/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_DIPFILTER_H_
#define _PCE_DIPFILTER_H_

#include <linux/platform_device.h>

#include "pce/pce.h"

int mtk_pce_dipfilter_enable(void);
void mtk_pce_dipfilter_disable(void);

int mtk_pce_dipfilter_desc_read(struct dip_desc *ddesc, u32 idx);

int mtk_pce_dipfilter_entry_add(struct dip_desc *ddesc);
int mtk_pce_dipfilter_entry_del(struct dip_desc *ddesc);

int mtk_pce_dipfilter_init(struct platform_device *pdev);
void mtk_pce_dipfilter_deinit(struct platform_device *pdev);
#endif /* _PCE_DIPFILTER_H_ */
