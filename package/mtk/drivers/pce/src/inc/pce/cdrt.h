/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _PCE_CDRT_H_
#define _PCE_CDRT_H_

#include <linux/platform_device.h>

#include "pce/cls.h"
#include "pce/pce.h"

enum cdrt_type {
	CDRT_DECRYPT,
	CDRT_ENCRYPT,

	__CDRT_TYPE_MAX,
};

struct cdrt_entry {
	u32 idx;
	enum cdrt_type type;
	struct cdrt_desc desc;
	struct cls_entry *cls;
};

int mtk_pce_cdrt_desc_write(struct cdrt_desc *desc, u32 idx);
int mtk_pce_cdrt_desc_read(struct cdrt_desc *desc, u32 idx);

int mtk_pce_cdrt_entry_write(struct cdrt_entry *cdrt);

struct cdrt_entry *mtk_pce_cdrt_entry_find(u32 cdrt_idx);
struct cdrt_entry *mtk_pce_cdrt_entry_alloc(enum cdrt_type type);
void mtk_pce_cdrt_entry_free(struct cdrt_entry *cdrt);

int mtk_pce_cdrt_enable(void);
void mtk_pce_cdrt_disable(void);

int mtk_pce_cdrt_init(struct platform_device *pdev);
void mtk_pce_cdrt_deinit(struct platform_device *pdev);
#endif /* _PCE_CDRT_H_ */
