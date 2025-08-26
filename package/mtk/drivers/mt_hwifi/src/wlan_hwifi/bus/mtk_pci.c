/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include "mtk_pci_dma.h"
#include "chips.h"

static void
pci_device_dump(const struct pci_device_id *id)
{
	pr_info("vendor: %x, device: %x\n", id->vendor, id->device);
	pr_info("subvendor: %x, subdevice: %x\n", id->subvendor, id->subdevice);
	pr_info("class: %x, class_mask: %x\n", id->class, id->class_mask);
	pr_info("driver_data : %p\n", (void *)id->driver_data);
}

static int
pci_get_gen_width(struct pci_dev *pdev, enum pcie_gen *gen,
		enum pcie_link_width *width)
{
	enum pci_bus_speed speed;

#if (KERNEL_VERSION(4, 17, 0) > LINUX_VERSION_CODE)
	pcie_get_minimum_link(pdev, &speed, width);
#else
	pcie_bandwidth_available(pdev, NULL, &speed, width);
#endif

	switch (speed) {
	case PCIE_GEN1:
		*gen = PCIE_GEN1;
		break;
	case PCIE_GEN2:
		*gen = PCIE_GEN2;
		break;
	case PCIE_GEN3:
		*gen = PCIE_GEN3;
		break;
	case PCIE_GEN4:
		*gen = PCIE_GEN4;
		break;
	default:
		dev_err(&pdev->dev, "%s(): Unknown speed = 0x%x\n",
			__func__, speed);
		return -EOPNOTSUPP;
	}

	return 0;
}

static int
pci_probe(struct pci_dev *pdev,
		const struct pci_device_id *id)
{
	int ret = 0;
	struct pci_trans *trans = NULL;
	struct pci_driver *drv = pdev->driver;
	struct mtk_chip *chip = container_of((void *)drv, struct mtk_chip, priv_drv);
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;
	struct pci_chip_profile *p = (struct pci_chip_profile *)bus_cfg->profile;
	void __iomem * const *iomap = NULL;

	ret = pcim_enable_device(pdev);
	if (ret)
		goto err;

	ret = pcim_iomap_regions(pdev, BIT(0), pci_name(pdev));
	if (ret)
		goto err;

	pci_set_master(pdev);

	if (p->non_coherent_dma_addr_size > 32) {
		if (!dma_set_mask(&pdev->dev, DMA_BIT_MASK(p->non_coherent_dma_addr_size))) {
			dev_info(&pdev->dev, "Using %dbit DMA for streaming map\n",
					p->non_coherent_dma_addr_size);
		} else if (!dma_set_mask(&pdev->dev, DMA_BIT_MASK(32))) {
			dev_info(&pdev->dev, "Using 32bit DMA for streaming map\n");
		} else {
			dev_err(&pdev->dev, "No usable streaming DMA configuration, aborting\n");
			goto err;
		}
	} else {
		if (!dma_set_mask(&pdev->dev, DMA_BIT_MASK(32))) {
			dev_info(&pdev->dev, "Using 32bit DMA for streaming map\n");
		} else {
			dev_err(&pdev->dev, "No usable streaming DMA configuration, aborting\n");
			goto err;
		}
	}

	if (p->coherent_dma_addr_size > 32) {
		if (!dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(p->coherent_dma_addr_size))) {
			dev_info(&pdev->dev, "Using %dbit DMA for coherent map\n",
					p->coherent_dma_addr_size);
		} else if (!dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32))) {
			dev_info(&pdev->dev, "Using 32bit DMA for coherent map\n");
		} else {
			dev_err(&pdev->dev, "No usable coherent DMA configuration, aborting\n");
			goto err;
		}
	} else {
		if (!dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32))) {
			dev_info(&pdev->dev, "Using 32bit DMA for coherent map\n");
		} else {
			dev_err(&pdev->dev, "No usable coherent DMA configuration, aborting\n");
			goto err;
		}
	}

	trans = mtk_bus_alloc_trans(&pdev->dev,
		drv, &pci_mmio_ops, &pci_dma_ops);

	if (!trans)
		goto err;

	pci_set_drvdata(pdev, trans);
	/* master switch of PCIe interrupt enable */
	iomap = pcim_iomap_table(pdev);
	if (!iomap)
		goto err;

	trans->regs = iomap[0];
	trans->id = id->device;
	trans->pdev = pdev;

	ret = pci_get_gen_width(pdev, &trans->pci_gen, &trans->pci_lane);
	if (ret)
		goto err;

	ret = mtk_bus_register_trans(trans);
	if (ret)
		goto err;

	pci_device_dump(id);
	return 0;
err:
	if (trans)
		mtk_bus_free_trans(trans);
	return -ENOMEM;
}

static void
pci_remove(struct pci_dev *pdev)
{
	struct pci_trans *trans = pci_get_drvdata(pdev);

	mtk_bus_unregister_trans(trans);
	mtk_bus_free_trans(trans);
}

static int
pci_drv_register(void *bus_drv, void *priv_drv)
{
	struct pci_driver *drv = priv_drv;
	struct mtk_chip *chip = container_of(priv_drv, struct mtk_chip, priv_drv);
	struct mtk_bus_cfg *bus_cfg = &chip->drv->bus_cfg;

	if (!bus_cfg->id_table || !bus_cfg->name)
		goto err;

	drv->name = bus_cfg->name;
	drv->id_table = (struct pci_device_id *) bus_cfg->id_table;
	drv->probe = pci_probe;
	drv->remove = pci_remove;
	return pci_register_driver(drv);
err:
	return -ENOMEM;
}

static int
pci_drv_unregister(void *bus_drv, void *priv_drv)
{
	struct pci_driver *drv = (struct pci_driver *) priv_drv;

	pci_unregister_driver(drv);
	return 0;
}

static int
pci_drv_dump_q_layout(struct seq_file *s, const struct pci_queue_desc *desc, u8 idx)
{
	seq_printf(s, "========== queue idx %d ==========\n", idx);
	seq_printf(s, "queue info\t: %s\n", desc->q_info);
	seq_printf(s, "queue attr\t: %u\n", desc->q_attr);
	seq_printf(s, "queue size\t: %u\n", desc->q_size);
	seq_printf(s, "band idx bitmap\t: 0x%x\n", desc->band_idx_bmp);
	seq_printf(s, "desc size\t: %u\n", desc->desc_size);
	seq_printf(s, "hw_desc_base\t: 0x%x\n", desc->hw_desc_base);
	seq_printf(s, "hw_int_mask\t: 0x%x\n", desc->hw_int_mask);

	return 0;
}

static int
pci_drv_dump_rxq_layout(struct seq_file *s, const struct pci_rx_queue_desc *desc, u8 idx)
{
	pci_drv_dump_q_layout(s, &desc->cmm, idx);
	seq_printf(s, "rx_buf_size\t: %u\n", desc->rx_buf_size);
	return 0;
}

static int
pci_drv_profile_dump(struct seq_file *s, void *profile)
{
	struct pci_chip_profile *pci_prof = profile;
	int i;

	seq_printf(s, "lp_fw_own_reg_addr\t: 0x%x\n", pci_prof->lp_fw_own_reg_addr);
	seq_printf(s, "rxq_num: %d\n", pci_prof->rxq_num);
	for (i = 0 ; i < pci_prof->rxq_num; i++)
		pci_drv_dump_rxq_layout(s,
			&pci_prof->queue_layout.rx_queue_layout[i], i);
	seq_printf(s, "txq_num: %u\n", pci_prof->txq_num);
	for (i = 0 ; i < pci_prof->txq_num; i++)
		pci_drv_dump_q_layout(s,
			(struct pci_queue_desc *)&pci_prof->queue_layout.tx_queue_layout[i], i);

	return 0;
}


static struct mtk_bus_driver pci_bus = {
	.bus_type = MTK_BUS_PCI,
	.bus_sub_type = MTK_BUS_NUM,
	.ctl_size = sizeof(struct pci_trans),
	.ops = {
		.drv_register = pci_drv_register,
		.drv_unregister = pci_drv_unregister,
		.drv_profile_dump = pci_drv_profile_dump,
	},
};

static int __init mtk_pci_init(void)
{
	return mtk_bus_driver_register(&pci_bus);
}

static void __exit mtk_pci_exit(void)
{
	mtk_bus_driver_unregister(&pci_bus);
}


module_init(mtk_pci_init);
module_exit(mtk_pci_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Hardware Wi-Fi Driver");

