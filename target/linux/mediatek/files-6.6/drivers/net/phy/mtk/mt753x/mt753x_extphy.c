/*
 * Driver for MediaTek MT7531 gigabit switch
 *
 * Copyright (C) 2018 MediaTek Inc. All Rights Reserved.
 *
 * Author: Landen Chao <landen.chao@mediatek.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/kernel.h>
#include <linux/mii.h>

#include "mt753x.h"
#include "mt753x_regs.h"
#include "mt753x_extphy.h"

int gpy211_init(struct gsw_mt753x *gsw, int addr)
{
	/* Enable rate adaption */
	gsw->mmd_write(gsw, addr, 0x1e, 0x8, 0x24e2);

	return 0;
}

static struct mt753x_extphy_id extphy_tbl[] = {
        {0x67c9de00, 0x0fffffff0, gpy211_init},
};

static u32 get_cl22_phy_id(struct gsw_mt753x *gsw, int addr)
{
	int phy_reg;
	u32 phy_id = 0;

	phy_reg = gsw->mii_read(gsw, addr, MII_PHYSID1);
	if (phy_reg < 0)
		return 0;
	phy_id = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = gsw->mii_read(gsw, addr, MII_PHYSID2);
	if (phy_reg < 0)
		return 0;

	phy_id |= (phy_reg & 0xffff);

	return phy_id;
}

static inline bool phy_id_is_match(u32 id, struct mt753x_extphy_id *phy)
{
	return ((id & phy->phy_id_mask) == (phy->phy_id & phy->phy_id_mask));
}

int extphy_init(struct gsw_mt753x *gsw, int addr)
{
	int i;
	u32 phy_id;
	struct mt753x_extphy_id *extphy;

	phy_id = get_cl22_phy_id(gsw, addr);
	for (i = 0; i < ARRAY_SIZE(extphy_tbl); i++) {
		extphy = &extphy_tbl[i];
		if(phy_id_is_match(phy_id, extphy))
			extphy->init(gsw, addr);
	}

	return 0;
}
