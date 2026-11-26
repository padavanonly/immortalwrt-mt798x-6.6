// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>

#include "pce/cdrt.h"
#include "pce/cls.h"
#include "pce/dipfilter.h"
#include "pce/internal.h"
#include "pce/netsys.h"
#include "pce/pce.h"

struct netsys_hw {
	void __iomem *base;
	spinlock_t lock;
	u32 fe_mem_limit[__FE_MEM_TYPE_MAX];
};

static struct netsys_hw netsys = {
	.fe_mem_limit = {
		[FE_MEM_TYPE_TS_CONFIG] = FE_MEM_TS_CONFIG_MAX_INDEX,
		[FE_MEM_TYPE_DIPFILTER] = FE_MEM_DIPFILTER_MAX_IDX,
		[FE_MEM_TYPE_CLS] = FE_MEM_CLS_MAX_INDEX,
		[FE_MEM_TYPE_CDRT] = FE_MEM_CDRT_MAX_INDEX,
	},
};

void mtk_pce_ppe_rmw(enum pse_port ppe, u32 reg, u32 mask, u32 val)
{
	if (ppe == PSE_PORT_PPE0)
		mtk_pce_netsys_rmw(PPE0_BASE + reg, mask, val);
	else if (ppe == PSE_PORT_PPE1)
		mtk_pce_netsys_rmw(PPE1_BASE + reg, mask, val);
	else if (ppe == PSE_PORT_PPE2)
		mtk_pce_netsys_rmw(PPE2_BASE + reg, mask, val);
}

u32 mtk_pce_ppe_read(enum pse_port ppe, u32 reg)
{
	if (ppe == PSE_PORT_PPE0)
		return mtk_pce_netsys_read(PPE0_BASE + reg);
	else if (ppe == PSE_PORT_PPE1)
		return mtk_pce_netsys_read(PPE1_BASE + reg);
	else if (ppe == PSE_PORT_PPE2)
		return mtk_pce_netsys_read(PPE2_BASE + reg);

	return 0;
}

void mtk_pce_netsys_write(u32 reg, u32 val)
{
	writel(val, netsys.base + reg);
}

void mtk_pce_netsys_setbits(u32 reg, u32 mask)
{
	writel(readl(netsys.base + reg) | mask, netsys.base + reg);
}

void mtk_pce_netsys_clrbits(u32 reg, u32 mask)
{
	writel(readl(netsys.base + reg) & (~mask), netsys.base + reg);
}

void mtk_pce_netsys_rmw(u32 reg, u32 mask, u32 val)
{
	writel((readl(netsys.base + reg) & (~mask)) | val, netsys.base + reg);
}

u32 mtk_pce_netsys_read(u32 reg)
{
	return readl(netsys.base + reg);
}

static inline void mtk_pce_fe_mem_config(enum fe_mem_type type, u32 idx)
{
	/* select memory index to program */
	mtk_pce_netsys_rmw(GLO_MEM_CTRL,
		   GLO_MEM_CTRL_ADDR_MASK,
		   FIELD_PREP(GLO_MEM_CTRL_ADDR_MASK, idx));

	/* select memory type to program */
	mtk_pce_netsys_rmw(GLO_MEM_CTRL,
		   GLO_MEM_CTRL_INDEX_MASK,
		   FIELD_PREP(GLO_MEM_CTRL_INDEX_MASK, type));
}

static inline void mtk_pce_fe_mem_start(enum fe_mem_cmd cmd)
{
	/* trigger start */
	mtk_pce_netsys_rmw(GLO_MEM_CTRL,
		   GLO_MEM_CTRL_CMD_MASK,
		   FIELD_PREP(GLO_MEM_CTRL_CMD_MASK, cmd));
}

static inline void mtk_pce_fe_mem_wait_transfer_done(void)
{
	while (FIELD_GET(GLO_MEM_CTRL_CMD_MASK, mtk_pce_netsys_read(GLO_MEM_CTRL)))
		udelay(10);
}

static int mtk_pce_fe_mem_write(enum fe_mem_type type, u32 *raw_data, u32 idx)
{
	unsigned long flag;
	u32 i = 0;

	spin_lock_irqsave(&netsys.lock, flag);

	mtk_pce_fe_mem_config(type, idx);

	/* place data */
	for (i = 0; i < FE_MEM_DATA_WLEN; i++)
		mtk_pce_netsys_write(GLO_MEM_DATA_IDX(i), raw_data[i]);

	mtk_pce_fe_mem_start(FE_MEM_CMD_WRITE);

	mtk_pce_fe_mem_wait_transfer_done();

	spin_unlock_irqrestore(&netsys.lock, flag);

	return 0;
}

static int mtk_pce_fe_mem_read(enum fe_mem_type type, u32 *raw_data, u32 idx)
{
	unsigned long flag;
	u32 i = 0;

	spin_lock_irqsave(&netsys.lock, flag);

	mtk_pce_fe_mem_config(type, idx);

	mtk_pce_fe_mem_start(FE_MEM_CMD_READ);

	mtk_pce_fe_mem_wait_transfer_done();

	/* read data out */
	for (i = 0; i < FE_MEM_DATA_WLEN; i++)
		raw_data[i] = mtk_pce_netsys_read(GLO_MEM_DATA_IDX(i));

	spin_unlock_irqrestore(&netsys.lock, flag);

	return 0;
}

int mtk_pce_fe_mem_msg_send(struct fe_mem_msg *msg)
{
	if (unlikely(!msg))
		return -EINVAL;

	if (msg->cmd >= __FE_MEM_CMD_MAX) {
		PCE_ERR("invalid fe_mem_cmd: %u\n", msg->cmd);
		return -EPERM;
	}

	if (msg->type >= __FE_MEM_TYPE_MAX) {
		PCE_ERR("invalid fe_mem_type: %u\n", msg->type);
		return -EPERM;
	}

	if (msg->index >= netsys.fe_mem_limit[msg->type]) {
		PCE_ERR("invalid FE_MEM index: %u, type: %u, max: %u\n",
		       msg->index, msg->type, netsys.fe_mem_limit[msg->type]);
		return -EPERM;
	}

	switch (msg->cmd) {
	case FE_MEM_CMD_WRITE:
		return mtk_pce_fe_mem_write(msg->type, msg->raw, msg->index);
	case FE_MEM_CMD_READ:
		return mtk_pce_fe_mem_read(msg->type, msg->raw, msg->index);
	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL(mtk_pce_fe_mem_msg_send);

int mtk_pce_netsys_init(struct platform_device *pdev)
{
	struct device_node *fe_mem = NULL;
	struct resource res;
	int ret = 0;

	fe_mem = of_parse_phandle(pdev->dev.of_node, "fe_mem", 0);
	if (!fe_mem) {
		PCE_ERR("can not find fe_mem node\n");
		return -ENODEV;
	}

	if (of_address_to_resource(fe_mem, 0, &res))
		return -ENXIO;

	/* map fe_mem */
	netsys.base = devm_ioremap(&pdev->dev, res.start, resource_size(&res));
	if (!netsys.base)
		return -ENOMEM;

	spin_lock_init(&netsys.lock);

	of_node_put(fe_mem);

	return ret;
}

void mtk_pce_netsys_deinit(struct platform_device *pdev)
{
	/* nothing to deinit right now */
}

int mtk_pce_enable(void)
{
	mtk_pce_cls_enable();

	mtk_pce_dipfilter_enable();

	mtk_pce_cdrt_enable();

	return 0;
}
EXPORT_SYMBOL(mtk_pce_enable);

void mtk_pce_disable(void)
{
	mtk_pce_cdrt_disable();

	mtk_pce_dipfilter_disable();

	mtk_pce_cls_disable();
}
EXPORT_SYMBOL(mtk_pce_disable);
