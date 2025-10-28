// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/reset.h>
#include <linux/hrtimer.h>
#include <linux/mii.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/phy.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#include "an8855.h"
#include "an8855_swconfig.h"
#include "an8855_regs.h"
#include "an8855_nl.h"

/* AN8855 driver version */
#define ARHT_AN8855_SWCFG_DRIVER_VER	"1.0.6"
#define ARHT_CHIP_NAME                  "an8855"
#define ARHT_PROC_DIR                   "air_sw"
#define ARHT_PROC_NODE_DEVICE           "device"

static u32 an8855_gsw_id;
struct list_head an8855_devs;
static DEFINE_MUTEX(an8855_devs_lock);
struct proc_dir_entry *proc_an8855_gsw_dir;

static struct an8855_sw_id *an8855_sw_ids[] = {
	&an8855_id,
};

u32 an8855_reg_read(struct gsw_an8855 *gsw, u32 reg)
{
	u32 high, low;

	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f, 0x4);
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x10, 0x0);

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x15,
			     ((reg >> 16) & 0xFFFF));
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x16,
			     (reg & 0xFFFF));

	low = gsw->host_bus->read(gsw->host_bus, gsw->smi_addr, 0x18);
	high = gsw->host_bus->read(gsw->host_bus, gsw->smi_addr, 0x17);

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f, 0x0);

	mutex_unlock(&gsw->host_bus->mdio_lock);

	return (high << 16) | (low & 0xffff);
}

void an8855_reg_write(struct gsw_an8855 *gsw, u32 reg, u32 val)
{
	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f, 0x4);
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x10, 0x0);

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x11,
			     ((reg >> 16) & 0xFFFF));
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x12,
			     (reg & 0xFFFF));

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x13,
			     ((val >> 16) & 0xFFFF));
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x14,
			     (val & 0xFFFF));

	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f, 0x0);

	mutex_unlock(&gsw->host_bus->mdio_lock);
}

int an8855_mii_read(struct gsw_an8855 *gsw, int phy, int reg)
{
	int val;

	if (phy < AN8855_NUM_PHYS)
		phy = (gsw->phy_base + phy) & AN8855_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);
	val = gsw->host_bus->read(gsw->host_bus, phy, reg);
	mutex_unlock(&gsw->host_bus->mdio_lock);

	return val;
}

void an8855_mii_write(struct gsw_an8855 *gsw, int phy, int reg, u16 val)
{
	if (phy < AN8855_NUM_PHYS)
		phy = (gsw->phy_base + phy) & AN8855_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);
	gsw->host_bus->write(gsw->host_bus, phy, reg, val);
	mutex_unlock(&gsw->host_bus->mdio_lock);
}

int an8855_mmd_read(struct gsw_an8855 *gsw, int addr, int devad, u16 reg)
{
	int val;

	if (addr < AN8855_NUM_PHYS)
		addr = (gsw->phy_base + addr) & AN8855_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, addr, 0x0d, devad);
	gsw->host_bus->write(gsw->host_bus, addr, 0x0e, reg);
	gsw->host_bus->write(gsw->host_bus, addr, 0x0d, devad | (0x4000));
	val = gsw->host_bus->read(gsw->host_bus, addr, 0xe);

	mutex_unlock(&gsw->host_bus->mdio_lock);

	return val;
}

void an8855_mmd_write(struct gsw_an8855 *gsw, int addr, int devad, u16 reg,
		      u16 val)
{
	if (addr < AN8855_NUM_PHYS)
		addr = (gsw->phy_base + addr) & AN8855_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, addr, 0x0d, devad);
	gsw->host_bus->write(gsw->host_bus, addr, 0x0e, reg);
	gsw->host_bus->write(gsw->host_bus, addr, 0x0d, devad | (0x4000));
	gsw->host_bus->write(gsw->host_bus, addr, 0x0e, val);

	mutex_unlock(&gsw->host_bus->mdio_lock);
}

static inline int an8855_get_duplex(const struct device_node *np)
{
	return of_property_read_bool(np, "full-duplex");
}

static void an8855_load_port_cfg(struct gsw_an8855 *gsw)
{
	struct device_node *port_np;
	struct device_node *fixed_link_node;
	struct an8855_port_cfg *port_cfg;
	u32 port;
#if (KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE)
	int ret;

#endif

	for_each_child_of_node(gsw->dev->of_node, port_np) {
		if (!of_device_is_compatible(port_np, "airoha,an8855-port"))
			continue;

		if (!of_device_is_available(port_np))
			continue;

		if (of_property_read_u32(port_np, "reg", &port))
			continue;

		switch (port) {
		case 5:
			port_cfg = &gsw->port5_cfg;
			break;
		default:
			continue;
		}

		if (port_cfg->enabled) {
			dev_info(gsw->dev, "duplicated node for port%d\n",
				 port_cfg->phy_mode);
			continue;
		}

		port_cfg->np = port_np;
#if (KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE)
		ret = of_get_phy_mode(port_np, &port_cfg->phy_mode);
		if (ret < 0) {
#else
		port_cfg->phy_mode = of_get_phy_mode(port_np);
		if (port_cfg->phy_mode < 0) {
#endif
			dev_info(gsw->dev, "incorrect phy-mode %d\n", port);
			continue;
		}

		fixed_link_node = of_get_child_by_name(port_np, "fixed-link");
		if (fixed_link_node) {
			u32 speed;

			port_cfg->force_link = 1;
			port_cfg->duplex = an8855_get_duplex(fixed_link_node);

			if (of_property_read_u32(fixed_link_node, "speed",
						 &speed)) {
				speed = 0;
				continue;
			}

			of_node_put(fixed_link_node);

			switch (speed) {
			case 10:
				port_cfg->speed = MAC_SPD_10;
				break;
			case 100:
				port_cfg->speed = MAC_SPD_100;
				break;
			case 1000:
				port_cfg->speed = MAC_SPD_1000;
				break;
			case 2500:
				port_cfg->speed = MAC_SPD_2500;
				break;

			default:
				dev_info(gsw->dev, "incorrect speed %d\n",
					 speed);
				continue;
			}
		}

		port_cfg->stag_on =
		    of_property_read_bool(port_cfg->np, "airoha,stag-on");
		port_cfg->enabled = 1;
	}
}

static void an8855_add_gsw(struct gsw_an8855 *gsw)
{
	mutex_lock(&an8855_devs_lock);
	gsw->id = an8855_gsw_id++;
	INIT_LIST_HEAD(&gsw->list);
	list_add_tail(&gsw->list, &an8855_devs);
	mutex_unlock(&an8855_devs_lock);
}

static void an8855_remove_gsw(struct gsw_an8855 *gsw)
{
	mutex_lock(&an8855_devs_lock);
	list_del(&gsw->list);
	mutex_unlock(&an8855_devs_lock);
}

struct gsw_an8855 *an8855_get_gsw(u32 id)
{
	struct gsw_an8855 *dev;

	mutex_lock(&an8855_devs_lock);

	list_for_each_entry(dev, &an8855_devs, list) {
		if (dev->id == id)
			return dev;
	}

	mutex_unlock(&an8855_devs_lock);

	return NULL;
}

struct gsw_an8855 *an8855_get_first_gsw(void)
{
	struct gsw_an8855 *dev;

	mutex_lock(&an8855_devs_lock);

	list_for_each_entry(dev, &an8855_devs, list)
		return dev;

	mutex_unlock(&an8855_devs_lock);

	return NULL;
}

void an8855_put_gsw(void)
{
	mutex_unlock(&an8855_devs_lock);
}

void an8855_lock_gsw(void)
{
	mutex_lock(&an8855_devs_lock);
}

static int an8855_hw_reset(struct gsw_an8855 *gsw)
{
	struct device_node *np = gsw->dev->of_node;
	int ret;

	gsw->reset_pin = of_get_named_gpio(np, "reset-gpios", 0);
	if (gsw->reset_pin < 0) {
		dev_info(gsw->dev, "No reset pin of switch\n");
		return 0;
	}

	ret = devm_gpio_request(gsw->dev, gsw->reset_pin, "an8855-reset");
	if (ret) {
		dev_info(gsw->dev, "Failed to request gpio %d\n",
			 gsw->reset_pin);
		return ret;
	}

	gpio_direction_output(gsw->reset_pin, 0);
	gpio_set_value(gsw->reset_pin, 0);
	usleep_range(100000, 150000);
	gpio_set_value(gsw->reset_pin, 1);
	usleep_range(100000, 150000);

	return 0;
}

static irqreturn_t an8855_irq_handler(int irq, void *dev)
{
	struct gsw_an8855 *gsw = dev;

	disable_irq_nosync(gsw->irq);

	schedule_work(&gsw->irq_worker);

	return IRQ_HANDLED;
}

static int an8855_proc_device_read(struct seq_file *seq, void *v)
{
	seq_printf(seq, "%s\n", ARHT_CHIP_NAME);

	return 0;
}

static int an8855_proc_device_open(struct inode *inode, struct file *file)
{
	return single_open(file, an8855_proc_device_read, 0);
}

#if (KERNEL_VERSION(5, 6, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops an8855_proc_device_fops = {
	.proc_open	= an8855_proc_device_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations an8855_proc_device_fops = {
	.owner	= THIS_MODULE,
	.open	= an8855_proc_device_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= single_release,
};
#endif

static int an8855_proc_device_init(struct gsw_an8855 *gsw)
{
	if (!proc_an8855_gsw_dir)
		proc_an8855_gsw_dir = proc_mkdir(ARHT_PROC_DIR, 0);

	proc_create(ARHT_PROC_NODE_DEVICE, 0400, proc_an8855_gsw_dir,
			&an8855_proc_device_fops);

	return 0;
}

static void an8855_proc_device_exit(void)
{
	remove_proc_entry(ARHT_PROC_NODE_DEVICE, 0);
}

static int an8855_probe(struct platform_device *pdev)
{
	struct gsw_an8855 *gsw;
	struct an8855_sw_id *sw;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mdio;
	struct mii_bus *mdio_bus;
	int ret = -EINVAL;
	struct chip_rev rev;
	struct an8855_mapping *map;
	int i;

	mdio = of_parse_phandle(np, "airoha,mdio", 0);
	if (!mdio)
		return -EINVAL;

	mdio_bus = of_mdio_find_bus(mdio);
	if (!mdio_bus)
		return -EPROBE_DEFER;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct gsw_an8855), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

	gsw->host_bus = mdio_bus;
	gsw->dev = &pdev->dev;

	dev_info(gsw->dev, "AN8855 Driver Version=%s\n",
			ARHT_AN8855_SWCFG_DRIVER_VER);

	/* Switch hard reset */
	if (an8855_hw_reset(gsw)) {
		dev_info(&pdev->dev, "reset switch fail.\n");
		goto fail;
	}

	/* Fetch the SMI address first */
	gsw->smi_addr = AN8855_DFL_SMI_ADDR;
	if (of_property_read_u32(np, "airoha,smi-addr", &gsw->new_smi_addr))
		gsw->new_smi_addr = AN8855_DFL_SMI_ADDR;

	/* Assign AN8855 interrupt pin */
	if (of_property_read_u32(np, "airoha,intr", &gsw->intr_pin))
		gsw->intr_pin = AN8855_DFL_INTR_ID;

	/* AN8855 surge enhancement */
	if (of_property_read_u32(np, "airoha,extSurge", &gsw->extSurge))
		gsw->extSurge = AN8855_DFL_EXT_SURGE;

	/* Get LAN/WAN port mapping */
	map = an8855_find_mapping(np);
	if (map) {
		an8855_apply_mapping(gsw, map);
		gsw->global_vlan_enable = 1;
		dev_info(gsw->dev, "LAN/WAN VLAN setting=%s\n", map->name);
	}

	/* Load MAC port configurations */
	an8855_load_port_cfg(gsw);

	/* Check for valid switch and then initialize */
	an8855_gsw_id = 0;
	for (i = 0; i < ARRAY_SIZE(an8855_sw_ids); i++) {
		if (!an8855_sw_ids[i]->detect(gsw, &rev)) {
			sw = an8855_sw_ids[i];

			gsw->name = rev.name;
			gsw->model = sw->model;

			dev_info(gsw->dev, "Switch is Airoha %s rev %d",
				 gsw->name, rev.rev);

			/* Initialize the switch */
			ret = sw->init(gsw);
			if (ret)
				goto fail;

			break;
		}
	}

	if (i >= ARRAY_SIZE(an8855_sw_ids)) {
		dev_err(gsw->dev, "No an8855 switch found\n");
		goto fail;
	}

	gsw->irq = platform_get_irq(pdev, 0);
	if (gsw->irq >= 0) {
		INIT_WORK(&gsw->irq_worker, an8855_irq_worker);

		ret = devm_request_irq(gsw->dev, gsw->irq, an8855_irq_handler,
				       0, dev_name(gsw->dev), gsw);
		if (ret) {
			dev_err(gsw->dev, "Failed to request irq %d\n",
				gsw->irq);
			goto fail;
		}
	}

	platform_set_drvdata(pdev, gsw);

	an8855_add_gsw(gsw);

	an8855_gsw_nl_init();

	an8855_proc_device_init(gsw);

	an8855_swconfig_init(gsw);

	if (sw->post_init)
		sw->post_init(gsw);

	if (gsw->irq >= 0)
		an8855_irq_enable(gsw);

	return 0;

fail:
	devm_kfree(&pdev->dev, gsw);

	return ret;
}

static int an8855_remove(struct platform_device *pdev)
{
	struct gsw_an8855 *gsw = platform_get_drvdata(pdev);

	if (gsw->irq >= 0)
		cancel_work_sync(&gsw->irq_worker);

#ifdef CONFIG_SWCONFIG
	an8855_swconfig_destroy(gsw);
#endif

	an8855_proc_device_exit();

	an8855_gsw_nl_exit();

	an8855_remove_gsw(gsw);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id an8855_ids[] = {
	{.compatible = "airoha,an8855"},
	{},
};

MODULE_DEVICE_TABLE(of, an8855_ids);

static struct platform_driver an8855_driver = {
	.probe = an8855_probe,
	.remove = an8855_remove,
	.driver = {
		   .name = "an8855",
		   .of_match_table = an8855_ids,
		   },
};

static int __init an8855_init(void)
{
	int ret;

	INIT_LIST_HEAD(&an8855_devs);
	ret = platform_driver_register(&an8855_driver);

	return ret;
}

module_init(an8855_init);

static void __exit an8855_exit(void)
{
	platform_driver_unregister(&an8855_driver);
}

module_exit(an8855_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Min Yao <min.yao@airoha.com>");
MODULE_VERSION(ARHT_AN8855_SWCFG_DRIVER_VER);
MODULE_DESCRIPTION("Driver for Airoha AN8855 Gigabit Switch");
