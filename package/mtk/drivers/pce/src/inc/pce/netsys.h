/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_NETSYS_H_
#define _PCE_NETSYS_H_

#include <linux/platform_device.h>

/* FE BASE */
#define FE_BASE					(0x0000)

/* PPE BASE */
#define PPE0_BASE				(0x2000)
#define PPE1_BASE				(0x2400)
#define PPE2_BASE				(0x2C00)

/* PPE */
#define PPE_TPORT_TBL_0				(0x0258)
#define PPE_TPORT_TBL_1				(0x025C)

/* FE_MEM */
#define GLO_MEM_CFG				(0x0600)
#define GLO_MEM_CTRL				(0x0604)
#define GLO_MEM_DATA_IDX(x)			(0x0608 + (0x4 * (x)))

/* GLO_MEM_CFG */
#define GDM_DIPFILTER_EN			(BIT(0))
#define GDM_CLS_EN				(BIT(1))
#define CDM_CDRT_EN				(BIT(2))
#define EIP197_LOCK_CACHES			(BIT(3))

/* GLO_MEM_CTRL */
#define GLO_MEM_CTRL_ADDR_SHIFT			(0)
#define GLO_MEM_CTRL_ADDR_MASK			GENMASK(19, 0)
#define GLO_MEM_CTRL_INDEX_SHIFT		(20)
#define GLO_MEM_CTRL_INDEX_MASK			GENMASK(29, 20)
#define GLO_MEM_CTRL_CMD_SHIFT			(30)
#define GLO_MEM_CTRL_CMD_MASK			GENMASK(31, 30)

/* TPORT setting etc. */
#define TPORT_IDX_MAX				(16)
#define TPORT_IDX_PER_REG			(8)
#define TS_CONFIG_MASK				(0xE746)
#define PSE_PORT_PPE_MASK			(BIT(PSE_PORT_PPE0) \
						| BIT(PSE_PORT_PPE1) \
						| BIT(PSE_PORT_PPE2))
#define PSE_PER_PORT_MASK			(0xF)
#define PSE_PER_PORT_BITS			(4)

enum pse_port {
	PSE_PORT_ADMA = 0,
	PSE_PORT_GDM1,
	PSE_PORT_GDM2,
	PSE_PORT_PPE0,
	PSE_PORT_PPE1,
	PSE_PORT_QDMA = 5,
	PSE_PORT_DISCARD = 7,
	PSE_PORT_WDMA0,
	PSE_PORT_WDMA1,
	PSE_PORT_TDMA = 10,
	PSE_PORT_EDMA0,
	PSE_PORT_PPE2,
	PSE_PORT_WDMA2,
	PSE_PORT_EIP197,
	PSE_PORT_GDM3 = 15,
	__PSE_PORT_MAX,
};

void mtk_pce_ppe_rmw(enum pse_port ppe, u32 reg, u32 mask, u32 val);
u32 mtk_pce_ppe_read(enum pse_port ppe, u32 reg);

void mtk_pce_netsys_write(u32 reg, u32 val);
void mtk_pce_netsys_setbits(u32 reg, u32 mask);
void mtk_pce_netsys_clrbits(u32 reg, u32 mask);
void mtk_pce_netsys_rmw(u32 reg, u32 mask, u32 val);
u32 mtk_pce_netsys_read(u32 reg);

int mtk_pce_netsys_init(struct platform_device *pdev);
void mtk_pce_netsys_deinit(struct platform_device *pdev);
#endif /* _PCE_NETSYS_H_ */
