/*
 * Driver for MediaTek MT753x gigabit switch
 *
 * Copyright (C) 2018 MediaTek Inc. All Rights Reserved.
 *
 * Author: Landen Chao <landen.chao@mediatek.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MT753X_EXTPHY_H_
#define _MT753X_EXTPHY_H_
struct mt753x_extphy_id {
        u32 phy_id;
        u32 phy_id_mask;
	int (*init)(struct gsw_mt753x *gsw, int addr);
};
#endif
