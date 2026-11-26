// SPDX-License-Identifier: GPL-2.0-only
#include "rtl837x.h"

static void rtl837x_regmap_lock(void *ctx)
{
	struct rtl837x_priv *priv = ctx;

	mutex_lock(&priv->map_lock);
}

static void rtl837x_regmap_unlock(void *ctx)
{
	struct rtl837x_priv *priv = ctx;

	mutex_unlock(&priv->map_lock);
}

static int rtl837x_smi_wait_idle_locked(struct mii_bus *bus, int addr)
{
	int i, ret;

	for (i = 0; i < 64; i++) {
		ret = __mdiobus_read(bus, addr, RTL837X_MDIO_CTRL_REG);
		if (ret < 0)
			return ret;
		if (!(ret & RTL837X_MDIO_CMD_BUSY))
			return 0;
		udelay(5);
	}

	return -ETIMEDOUT;
}

static int rtl837x_regmap_read(void *ctx, unsigned int reg, unsigned int *val)
{
	struct rtl837x_priv *priv = ctx;
	struct mii_bus *bus = priv->bus;
	int ret, lo, hi;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	ret = rtl837x_smi_wait_idle_locked(bus, priv->mdio_addr);
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_ADDRESS_REG,
			      (u16)(reg >> 2));
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_CTRL_REG,
			      RTL837X_MDIO_READ_CMD);
	if (ret < 0)
		goto out;

	ret = rtl837x_smi_wait_idle_locked(bus, priv->mdio_addr);
	if (ret < 0)
		goto out;

	lo = __mdiobus_read(bus, priv->mdio_addr, RTL837X_MDIO_DATA_LOW_REG);
	if (lo < 0) {
		ret = lo;
		goto out;
	}

	hi = __mdiobus_read(bus, priv->mdio_addr, RTL837X_MDIO_DATA_HIGH_REG);
	if (hi < 0) {
		ret = hi;
		goto out;
	}

	*val = ((u32)(hi & 0xffff) << 16) | (u32)(lo & 0xffff);
	ret = 0;

out:
	mutex_unlock(&bus->mdio_lock);
	return ret;
}

static int rtl837x_regmap_write(void *ctx, unsigned int reg, unsigned int val)
{
	struct rtl837x_priv *priv = ctx;
	struct mii_bus *bus = priv->bus;
	int ret;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	ret = rtl837x_smi_wait_idle_locked(bus, priv->mdio_addr);
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_ADDRESS_REG,
			      (u16)(reg >> 2));
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_DATA_LOW_REG,
			      (u16)(val & 0xffff));
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_DATA_HIGH_REG,
			      (u16)((val >> 16) & 0xffff));
	if (ret < 0)
		goto out;

	ret = __mdiobus_write(bus, priv->mdio_addr, RTL837X_MDIO_CTRL_REG,
			      RTL837X_MDIO_WRITE_CMD);
	if (ret < 0)
		goto out;

	/* Soft reset clears SMI engine immediately; do not poll after SoC reset */
	if (reg != RTL837X_REG_RESET || !(val & RTL837X_RESET_SOC_BIT))
		ret = rtl837x_smi_wait_idle_locked(bus, priv->mdio_addr);

out:
	mutex_unlock(&bus->mdio_lock);
	return ret;
}

static const struct regmap_config rtl837x_mdio_regmap_cfg = {
	.reg_bits	= 16,
	.val_bits	= 32,
	.reg_stride	= 4,
	.max_register	= 0xfffc,
	.reg_read	= rtl837x_regmap_read,
	.reg_write	= rtl837x_regmap_write,
	.lock		= rtl837x_regmap_lock,
	.unlock		= rtl837x_regmap_unlock,
};

static const struct regmap_config rtl837x_mdio_regmap_nolock_cfg = {
	.reg_bits	= 16,
	.val_bits	= 32,
	.reg_stride	= 4,
	.max_register	= 0xfffc,
	.reg_read	= rtl837x_regmap_read,
	.reg_write	= rtl837x_regmap_write,
	.disable_locking = true,
};

int rtl837x_ita_exec(struct rtl837x_priv *priv, enum rtl837x_ita_table tbl,
		     bool write, u16 addr, u32 data_in[3], u32 data_out[3])
{
	u32 cmd, status;
	int i, ret;

	mutex_lock(&priv->map_lock);

	ret = regmap_read_poll_timeout(priv->map_nolock, RTL837X_TABLE_CTRL_REG,
				       status, !(status & RTL837X_TABLE_EXEC),
				       10, 10000);
	if (ret)
		goto out;

	/*
	 * RTLPlayground fix: always clear stale delete/method bits in
	 * RTL837X_TABLE_L2_CTRL_REG (0x5cb0) before table reads/writes so a
	 * prior L2 delete never corrupts subsequent CVLAN/LUT table walks.
	 */
	ret = regmap_write(priv->map_nolock, RTL837X_TABLE_L2_CTRL_REG, 0);
	if (ret)
		goto out;

	if (write && data_in) {
		for (i = 0; i < 3; i++) {
			ret = regmap_write(priv->map_nolock,
					   RTL837X_TABLE_WRITE_REG(i),
					   data_in[i]);
			if (ret)
				goto out;
		}
	}

	cmd = FIELD_PREP(RTL837X_TABLE_ADDR_MASK, addr) |
	      FIELD_PREP(RTL837X_TABLE_TYPE_MASK, tbl) |
	      (write ? RTL837X_TABLE_OP_WRITE : 0) |
	      RTL837X_TABLE_EXEC;

	ret = regmap_write(priv->map_nolock, RTL837X_TABLE_CTRL_REG, cmd);
	if (ret)
		goto out;

	ret = regmap_read_poll_timeout(priv->map_nolock, RTL837X_TABLE_CTRL_REG,
				       status, !(status & RTL837X_TABLE_EXEC),
				       10, 10000);
	if (ret)
		goto out;

	if (!write && data_out) {
		for (i = 0; i < 3; i++) {
			ret = regmap_read(priv->map_nolock,
					  RTL837X_TABLE_READ_REG(i),
					  &data_out[i]);
			if (ret)
				goto out;
		}
	}

out:
	mutex_unlock(&priv->map_lock);
	return ret;
}

int rtl837x_sds_reg_read(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			 u16 *val)
{
	u32 cmd, raw = 0;
	int ret;

	if (sds > 1 || page > 0x3f || reg > 0x1f)
		return -EINVAL;

	mutex_lock(&priv->map_lock);
	cmd = RTL837X_SDS_INDACS_TRIGGER |
	      FIELD_PREP(RTL837X_SDS_INDACS_REGAD, reg) |
	      FIELD_PREP(RTL837X_SDS_INDACS_PAGE, page) |
	      FIELD_PREP(RTL837X_SDS_INDACS_INDEX, sds);

	ret = regmap_write(priv->map_nolock, RTL837X_SDS_INDACS_CMD_REG, cmd);
	if (ret)
		goto out;

	ret = regmap_read_poll_timeout(priv->map_nolock,
				       RTL837X_SDS_INDACS_CMD_REG, cmd,
				       !(cmd & RTL837X_SDS_INDACS_TRIGGER),
				       10, 10000);
	if (ret)
		goto out;

	ret = regmap_read(priv->map_nolock, RTL837X_SDS_INDACS_READ_DATA_REG,
			  &raw);
	if (!ret)
		*val = (u16)(raw & 0xffff);

out:
	mutex_unlock(&priv->map_lock);
	return ret;
}

int rtl837x_sds_reg_write(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			  u16 val)
{
	u32 cmd;
	int ret;

	if (sds > 1 || page > 0x3f || reg > 0x1f)
		return -EINVAL;

	mutex_lock(&priv->map_lock);
	ret = regmap_write(priv->map_nolock, RTL837X_SDS_INDACS_WRITE_DATA_REG,
			   val);
	if (ret)
		goto out;

	cmd = RTL837X_SDS_INDACS_TRIGGER | RTL837X_SDS_INDACS_WR |
	      FIELD_PREP(RTL837X_SDS_INDACS_REGAD, reg) |
	      FIELD_PREP(RTL837X_SDS_INDACS_PAGE, page) |
	      FIELD_PREP(RTL837X_SDS_INDACS_INDEX, sds);

	ret = regmap_write(priv->map_nolock, RTL837X_SDS_INDACS_CMD_REG, cmd);
	if (ret)
		goto out;

	ret = regmap_read_poll_timeout(priv->map_nolock,
				       RTL837X_SDS_INDACS_CMD_REG, cmd,
				       !(cmd & RTL837X_SDS_INDACS_TRIGGER),
				       10, 10000);
out:
	mutex_unlock(&priv->map_lock);
	return ret;
}

int rtl837x_sds_reg_modify(struct rtl837x_priv *priv, u8 sds, u8 page, u8 reg,
			   u16 mask, u16 set)
{
	u16 val = 0;
	int ret;

	ret = rtl837x_sds_reg_read(priv, sds, page, reg, &val);
	if (ret)
		return ret;

	val = (val & ~mask) | (set & mask);
	return rtl837x_sds_reg_write(priv, sds, page, reg, val);
}

int rtl837x_phy_read_c45_locked(struct rtl837x_priv *priv, int port, int devad,
				int regnum)
{
	u32 cmd, sts = 0, val = 0;
	int ret;

	if (port < 0 || port >= RTL837X_MAX_PORTS)
		return -EINVAL;

	ret = regmap_write(priv->map_nolock, RTL837X_SMI_PHY_CTRL_2_REG,
			   ((u32)(regnum & 0xffff) << 16));
	if (ret)
		return ret;

	cmd = RTL837X_SMI_PHY_TRIGGER | RTL837X_SMI_PHY_C45 |
	      (((u32)devad & 0x1f) << 3) | (((u32)port & 0x1f) << 8);
	ret = regmap_write(priv->map_nolock, RTL837X_SMI_PHY_CTRL_0_REG, cmd);
	if (ret)
		return ret;

	ret = regmap_read_poll_timeout(priv->map_nolock,
				       RTL837X_SMI_PHY_CTRL_0_REG, sts,
				       !(sts & RTL837X_SMI_PHY_TRIGGER),
				       10, 5000);
	if (ret)
		return ret;
	if (sts & RTL837X_SMI_PHY_FAIL)
		return -EIO;

	ret = regmap_read(priv->map_nolock, RTL837X_SMI_PHY_CTRL_3_REG, &val);
	return ret ? ret : (int)(val & 0xffff);
}

int rtl837x_phy_write_c45_locked(struct rtl837x_priv *priv, int port, int devad,
				 int regnum, u16 val)
{
	u32 cmd, sts = 0;
	int ret;

	if (port < 0 || port >= RTL837X_MAX_PORTS)
		return -EINVAL;

	ret = regmap_write(priv->map_nolock, RTL837X_SMI_PHY_CTRL_1_REG,
			   BIT(port));
	if (ret)
		return ret;

	ret = regmap_write(priv->map_nolock, RTL837X_SMI_PHY_CTRL_2_REG,
			   ((u32)(regnum & 0xffff) << 16) | val);
	if (ret)
		return ret;

	cmd = RTL837X_SMI_PHY_TRIGGER | RTL837X_SMI_PHY_WRITE |
	      RTL837X_SMI_PHY_C45 | (((u32)devad & 0x1f) << 3);
	ret = regmap_write(priv->map_nolock, RTL837X_SMI_PHY_CTRL_0_REG, cmd);
	if (ret)
		return ret;

	ret = regmap_read_poll_timeout(priv->map_nolock,
				       RTL837X_SMI_PHY_CTRL_0_REG, sts,
				       !(sts & RTL837X_SMI_PHY_TRIGGER),
				       10, 5000);
	if (ret)
		return ret;

	return (sts & RTL837X_SMI_PHY_FAIL) ? -EIO : 0;
}

static int rtl837x_slave_mii_read_c22(struct mii_bus *bus, int addr, int regnum)
{
	struct rtl837x_priv *priv = bus->priv;
	int ret;

	/* Internal RTL8224 PHYs implement C22 via VEND2 OCP 0xa400 + reg*2 */
	mutex_lock(&priv->map_lock);
	ret = rtl837x_phy_read_c45_locked(priv, addr, MDIO_MMD_VEND2,
					  0xa400 + (regnum << 1));
	mutex_unlock(&priv->map_lock);
	return ret;
}

static int rtl837x_slave_mii_write_c22(struct mii_bus *bus, int addr,
				       int regnum, u16 val)
{
	struct rtl837x_priv *priv = bus->priv;
	int ret;

	mutex_lock(&priv->map_lock);
	ret = rtl837x_phy_write_c45_locked(priv, addr, MDIO_MMD_VEND2,
					   0xa400 + (regnum << 1), val);
	mutex_unlock(&priv->map_lock);
	return ret;
}

static int rtl837x_slave_mii_read_c45(struct mii_bus *bus, int addr, int devad,
				      int regnum)
{
	struct rtl837x_priv *priv = bus->priv;
	int ret;

	mutex_lock(&priv->map_lock);
	ret = rtl837x_phy_read_c45_locked(priv, addr, devad, regnum);
	mutex_unlock(&priv->map_lock);
	return ret;
}

static int rtl837x_slave_mii_write_c45(struct mii_bus *bus, int addr, int devad,
				       int regnum, u16 val)
{
	struct rtl837x_priv *priv = bus->priv;
	int ret;

	mutex_lock(&priv->map_lock);
	ret = rtl837x_phy_write_c45_locked(priv, addr, devad, regnum, val);
	mutex_unlock(&priv->map_lock);
	return ret;
}

int rtl837x_setup_internal_mdio(struct rtl837x_priv *priv)
{
	struct device_node *mnp;
	struct mii_bus *bus;
	int ret;

	bus = devm_mdiobus_alloc(priv->dev);
	if (!bus)
		return -ENOMEM;

	bus->name = "rtl837x-internal-mdio";
	bus->priv = priv;
	bus->parent = priv->dev;
	bus->read = rtl837x_slave_mii_read_c22;
	bus->write = rtl837x_slave_mii_write_c22;
	bus->read_c45 = rtl837x_slave_mii_read_c45;
	bus->write_c45 = rtl837x_slave_mii_write_c45;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s-mii", dev_name(priv->dev));

	mnp = of_get_child_by_name(priv->dev->of_node, "mdio");
	ret = devm_of_mdiobus_register(priv->dev, bus, mnp);
	if (mnp)
		of_node_put(mnp);
	if (ret)
		return ret;

	priv->slave_mii_bus = bus;
	return 0;
}

static void rtl837x_assert_reset(struct rtl837x_priv *priv)
{
	if (!priv->reset_gpio)
		return;

	if (gpiod_is_active_low(priv->reset_gpio))
		gpiod_set_value_cansleep(priv->reset_gpio, 1);
	else
		gpiod_set_value_cansleep(priv->reset_gpio, 0);
}

static int rtl837x_mdio_probe(struct mdio_device *mdiodev)
{
	struct device *dev = &mdiodev->dev;
	struct device_node *np = dev->of_node;
	struct rtl837x_priv *priv;
	struct regmap_config rc, rc_nl;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->map_lock);
	spin_lock_init(&priv->stats_lock);
	priv->dev = dev;
	priv->bus = mdiodev->bus;
	priv->mdio_addr = mdiodev->addr;
	priv->sds_rx_inv[0] = of_property_read_bool(np, "sds0-rx-swap");
	priv->sds_tx_inv[0] = of_property_read_bool(np, "sds0-tx-swap");
	priv->sds_rx_inv[1] = of_property_read_bool(np, "sds1-rx-swap");
	priv->sds_tx_inv[1] = of_property_read_bool(np, "sds1-tx-swap");
	priv->phy_mdi_reverse = of_property_read_bool(np, "phy-mdi-reverse");
	priv->phy_tx_pol_swap =
		of_property_read_bool(np, "phy-tx-polarity-swap");

	rc = rtl837x_mdio_regmap_cfg;
	rc.lock_arg = priv;
	priv->map = devm_regmap_init(dev, NULL, priv, &rc);
	if (IS_ERR(priv->map))
		return PTR_ERR(priv->map);

	rc_nl = rtl837x_mdio_regmap_nolock_cfg;
	priv->map_nolock = devm_regmap_init(dev, NULL, priv, &rc_nl);
	if (IS_ERR(priv->map_nolock))
		return PTR_ERR(priv->map_nolock);

	priv->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_ASIS);
	if (IS_ERR(priv->reset_gpio))
		return PTR_ERR(priv->reset_gpio);
	if (priv->reset_gpio) {
		if (gpiod_is_active_low(priv->reset_gpio)) {
			/* Active-low reset: logical 1 = reset, 0 = running */
			ret = gpiod_direction_output(priv->reset_gpio, 1);
			if (ret)
				return ret;
			msleep(50);
			gpiod_set_value_cansleep(priv->reset_gpio, 0);
			msleep(100);
		} else {
			/* Active-high DTS flag: physical HIGH = run, LOW = reset */
			ret = gpiod_direction_output(priv->reset_gpio, 1);
			if (ret)
				return ret;
			msleep(50);
			gpiod_set_value_cansleep(priv->reset_gpio, 0);
			msleep(50);
			gpiod_set_value_cansleep(priv->reset_gpio, 1);
			msleep(100);
		}
	}

	dev_set_drvdata(dev, priv);

	ret = rtl8372n_switch_detect_and_register(priv);
	if (ret) {
		rtl837x_assert_reset(priv);
		dev_set_drvdata(dev, NULL);
		return ret;
	}

	return 0;
}

static void rtl837x_mdio_remove(struct mdio_device *mdiodev)
{
	struct rtl837x_priv *priv = dev_get_drvdata(&mdiodev->dev);

	if (!priv)
		return;

	rtl8372n_switch_teardown_and_unregister(priv);
	rtl837x_assert_reset(priv);
	dev_set_drvdata(&mdiodev->dev, NULL);
}

static void rtl837x_mdio_shutdown(struct mdio_device *mdiodev)
{
	struct rtl837x_priv *priv = dev_get_drvdata(&mdiodev->dev);

	if (!priv)
		return;

	WRITE_ONCE(priv->stopping, true);
	cancel_delayed_work_sync(&priv->stats_work);
	if (priv->ds)
		dsa_switch_shutdown(priv->ds);
	dev_set_drvdata(&mdiodev->dev, NULL);
}

static const struct of_device_id rtl837x_mdio_of_match[] = {
	{ .compatible = "realtek,rtl8372n" },
	{ .compatible = "realtek,rtl8372n-dsa" },
	{ .compatible = "realtek,rtl8373" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl837x_mdio_of_match);

static struct mdio_driver rtl837x_mdio_driver = {
	.probe		= rtl837x_mdio_probe,
	.remove		= rtl837x_mdio_remove,
	.shutdown	= rtl837x_mdio_shutdown,
	.mdiodrv.driver = {
		.name		= "rtl8372n-dsa",
		.of_match_table	= rtl837x_mdio_of_match,
	},
};

static int __init rtl8372n_dsa_module_init(void)
{
	int ret;

	ret = rtl837x_tag_8021q_driver_register();
	if (ret)
		return ret;

	ret = mdio_driver_register(&rtl837x_mdio_driver);
	if (ret)
		rtl837x_tag_8021q_driver_unregister();

	return ret;
}
module_init(rtl8372n_dsa_module_init);

static void __exit rtl8372n_dsa_module_exit(void)
{
	mdio_driver_unregister(&rtl837x_mdio_driver);
	rtl837x_tag_8021q_driver_unregister();
}
module_exit(rtl8372n_dsa_module_exit);

MODULE_DESCRIPTION("Realtek RTL8372N/RTL8373 DSA Switch Driver");
MODULE_AUTHOR("StarField Xu / RTLPlayground Contributors");
MODULE_LICENSE("GPL");
