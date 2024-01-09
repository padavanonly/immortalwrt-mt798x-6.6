/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_CLS_H_
#define _PCE_CLS_H_

#include <linux/platform_device.h>

#include "pce/pce.h"

#define CLS_DESC_MASK_DATA(cdesc, field, mask, data)			\
	do {								\
		(cdesc)->field ## _m = (mask);				\
		(cdesc)->field = (data);				\
	} while (0)
#define CLS_DESC_DATA(cdesc, field, data)				\
	(cdesc)->field = (data)

struct cls_entry {
	u32 idx;
	struct cls_desc cdesc;
};

int mtk_pce_cls_enable(void);
void mtk_pce_cls_disable(void);

int mtk_pce_cls_init(struct platform_device *pdev);
void mtk_pce_cls_deinit(struct platform_device *pdev);

int mtk_pce_cls_desc_read(struct cls_desc *cdesc, u32 idx);
int mtk_pce_cls_desc_write(struct cls_desc *cdesc, u32 idx);

int mtk_pce_cls_entry_write(struct cls_entry *cls);

struct cls_entry *mtk_pce_cls_entry_alloc(void);
void mtk_pce_cls_entry_free(struct cls_entry *cls);
#endif /* _PCE_CLS_H_ */
