// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/lockdep.h>
#include <linux/spinlock.h>

#include "pce/cls.h"
#include "pce/internal.h"
#include "pce/netsys.h"

struct cls_hw {
	struct cls_entry cls_tbl[FE_MEM_CLS_MAX_INDEX];
	DECLARE_BITMAP(cls_used, FE_MEM_CLS_MAX_INDEX);
	spinlock_t lock;
};

struct cls_hw cls_hw;

int mtk_pce_cls_enable(void)
{
	mtk_pce_netsys_setbits(GLO_MEM_CFG, GDM_CLS_EN);

	return 0;
}

void mtk_pce_cls_disable(void)
{
	mtk_pce_netsys_clrbits(GLO_MEM_CFG, GDM_CLS_EN);
}

static void mtk_pce_cls_clean_up(void)
{
	struct fe_mem_msg msg = {
		.cmd = FE_MEM_CMD_WRITE,
		.type = FE_MEM_TYPE_CLS,
	};
	unsigned long flag;
	int ret = 0;
	u32 i = 0;

	memset(&msg.raw, 0, sizeof(msg.raw));

	spin_lock_irqsave(&cls_hw.lock, flag);

	/* clean up cls table */
	for (i = 0; i < FE_MEM_CLS_MAX_INDEX; i++) {
		msg.index = i;
		ret = mtk_pce_fe_mem_msg_send(&msg);
		if (ret)
			goto unlock;
	}

unlock:
	spin_unlock_irqrestore(&cls_hw.lock, flag);
}

int mtk_pce_cls_init(struct platform_device *pdev)
{
	u32 i;

	spin_lock_init(&cls_hw.lock);

	mtk_pce_cls_clean_up();

	for (i = 0; i < FE_MEM_CLS_MAX_INDEX; i++)
		cls_hw.cls_tbl[i].idx = i + 1;

	return 0;
}

void mtk_pce_cls_deinit(struct platform_device *pdev)
{
	mtk_pce_cls_clean_up();
}

int mtk_pce_cls_desc_read(struct cls_desc *cdesc, u32 idx)
{
	struct fe_mem_msg msg;
	int ret;

	if (unlikely(!cdesc || !idx || idx >= FE_MEM_CLS_MAX_INDEX))
		return -EINVAL;

	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_CLS, idx);

	memset(&msg.raw, 0, sizeof(msg.raw));

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	memcpy(cdesc, &msg.cdesc, sizeof(struct cls_desc));

	return ret;
}

int mtk_pce_cls_desc_write(struct cls_desc *cdesc, u32 idx)
{
	struct fe_mem_msg msg;

	if (unlikely(!cdesc || !idx || idx >= FE_MEM_CLS_MAX_INDEX))
		return -EINVAL;

	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_CLS, idx);

	memset(&msg.raw, 0, sizeof(msg.raw));
	memcpy(&msg.cdesc, cdesc, sizeof(struct cls_desc));

	return mtk_pce_fe_mem_msg_send(&msg);
}

int mtk_pce_cls_entry_write(struct cls_entry *cls)
{
	if (unlikely(!cls))
		return -EINVAL;

	return mtk_pce_cls_desc_write(&cls->cdesc, cls->idx);
}
EXPORT_SYMBOL(mtk_pce_cls_entry_write);

struct cls_entry *mtk_pce_cls_entry_alloc(void)
{
	struct cls_entry *cls;
	unsigned long flag;
	u32 idx;

	spin_lock_irqsave(&cls_hw.lock, flag);

	idx = find_first_zero_bit(cls_hw.cls_used, FE_MEM_CLS_MAX_INDEX);
	if (idx == FE_MEM_CLS_MAX_INDEX) {
		cls = ERR_PTR(-ENOMEM);
		goto unlock;
	}

	set_bit(idx, cls_hw.cls_used);

	cls = &cls_hw.cls_tbl[idx];

	memset(&cls->cdesc, 0, sizeof(cls->cdesc));

unlock:
	spin_unlock_irqrestore(&cls_hw.lock, flag);

	return cls;
}
EXPORT_SYMBOL(mtk_pce_cls_entry_alloc);

void mtk_pce_cls_entry_free(struct cls_entry *cls)
{
	unsigned long flag;

	if (!cls)
		return;

	spin_lock_irqsave(&cls_hw.lock, flag);
	clear_bit(cls->idx - 1, cls_hw.cls_used);
	spin_unlock_irqrestore(&cls_hw.lock, flag);
}
EXPORT_SYMBOL(mtk_pce_cls_entry_free);
