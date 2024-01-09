// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/refcount.h>
#include <linux/spinlock.h>

#include "pce/dipfilter.h"
#include "pce/internal.h"
#include "pce/netsys.h"

struct dipfilter_entry {
	struct dip_desc ddesc;
	refcount_t refcnt;
	u32 index;
};

struct dipfilter_hw {
	struct dipfilter_entry dip_entries[FE_MEM_DIPFILTER_MAX_IDX];
	DECLARE_BITMAP(dip_tbl, FE_MEM_DIPFILTER_MAX_IDX);
	spinlock_t lock;
};

struct dipfilter_hw dip_hw;

int mtk_pce_dipfilter_enable(void)
{
	mtk_pce_netsys_setbits(GLO_MEM_CFG, GDM_DIPFILTER_EN);

	return 0;
}

void mtk_pce_dipfilter_disable(void)
{
	mtk_pce_netsys_clrbits(GLO_MEM_CFG, GDM_DIPFILTER_EN);
}

/*
 * find registered dipfilter info
 * return index on matched dipfilter info
 * return NULL on not found
 */
static struct dipfilter_entry *__mtk_pce_dip_info_find(struct dip_desc *target)
{
	struct dip_desc *ddesc;
	u32 idx = 0;

	lockdep_assert_held(&dip_hw.lock);

	/* can check this before acquiring lock */
	if (!target
	    || target->tag == DIPFILTER_DISABLED
	    || target->tag >= __DIPFILTER_TAG_MAX) {
		PCE_NOTICE("dip_desc is not enabled or invalid\n");
		return NULL;
	}

	for_each_set_bit(idx, dip_hw.dip_tbl, FE_MEM_DIPFILTER_MAX_IDX) {
		ddesc = &dip_hw.dip_entries[idx].ddesc;

		if (target->tag != ddesc->tag)
			continue;

		if (ddesc->tag == DIPFILTER_IPV4
		    && ddesc->ipv4 == target->ipv4) {
			return &dip_hw.dip_entries[idx];
		} else if (ddesc->tag == DIPFILTER_IPV6
			   && !memcmp(ddesc->ipv6, target->ipv6, sizeof(u32) * 4)) {
			return &dip_hw.dip_entries[idx];
		}
	}

	return NULL;
}

int mtk_pce_dipfilter_desc_read(struct dip_desc *ddesc, u32 idx)
{
	struct fe_mem_msg msg;
	unsigned long flag;
	int ret;

	if (!ddesc || idx > FE_MEM_DIPFILTER_MAX_IDX)
		return -EINVAL;

	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_DIPFILTER, idx);
	memset(&msg.raw, 0, sizeof(msg.raw));

	spin_lock_irqsave(&dip_hw.lock, flag);

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		goto unlock;

	memcpy(ddesc, &msg.ddesc, sizeof(struct dip_desc));

unlock:
	spin_unlock_irqrestore(&dip_hw.lock, flag);

	return ret;
}

static int __mtk_pce_dipfilter_entry_add(struct dip_desc *ddesc)
{
	struct fe_mem_msg fmsg;
	struct dipfilter_entry *dip_entry;
	int ret;
	u32 idx;

	lockdep_assert_held(&dip_hw.lock);

	/* find available entry */
	idx = find_first_zero_bit(dip_hw.dip_tbl, FE_MEM_DIPFILTER_MAX_IDX);
	if (idx == FE_MEM_DIPFILTER_MAX_IDX) {
		PCE_NOTICE("dipfilter full\n");
		return -EBUSY;
	}

	/* prepare fe_mem message */
	mtk_pce_fe_mem_msg_config(&fmsg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_DIPFILTER, idx);

	memset(&fmsg.raw, 0, sizeof(fmsg.raw));
	memcpy(&fmsg.raw, ddesc, sizeof(struct dip_desc));

	/* send fe_mem message */
	ret = mtk_pce_fe_mem_msg_send(&fmsg);
	if (ret) {
		PCE_NOTICE("fe_mem send dipfilter desc failed\n");
		return ret;
	}

	/* record installed dipfilter data */
	dip_entry = &dip_hw.dip_entries[idx];
	memcpy(&dip_entry->ddesc, ddesc, sizeof(struct dip_desc));

	refcount_set(&dip_entry->refcnt, 1);

	set_bit(idx, dip_hw.dip_tbl);

	return 0;
}

int mtk_pce_dipfilter_entry_add(struct dip_desc *ddesc)
{
	struct dipfilter_entry *dip_entry;
	unsigned long flag;
	int ret = 0;

	if (unlikely(!ddesc))
		return 0;

	spin_lock_irqsave(&dip_hw.lock, flag);

	dip_entry = __mtk_pce_dip_info_find(ddesc);
	if (dip_entry) {
		refcount_inc(&dip_entry->refcnt);
		goto unlock;
	}

	ret = __mtk_pce_dipfilter_entry_add(ddesc);

unlock:
	spin_unlock_irqrestore(&dip_hw.lock, flag);

	return ret;
}
EXPORT_SYMBOL(mtk_pce_dipfilter_entry_add);

int mtk_pce_dipfilter_entry_del(struct dip_desc *ddesc)
{
	struct fe_mem_msg fmsg;
	struct dipfilter_entry *dip_entry;
	unsigned long flag;
	int ret;

	if (!ddesc)
		return 0;

	spin_lock_irqsave(&dip_hw.lock, flag);

	dip_entry = __mtk_pce_dip_info_find(ddesc);
	if (!dip_entry)
		goto unlock;

	if (!refcount_dec_and_test(&dip_entry->refcnt))
		/* dipfilter descriptor is still in use */
		return 0;

	mtk_pce_fe_mem_msg_config(&fmsg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_DIPFILTER,
				  dip_entry->index);
	memset(&fmsg.raw, 0, sizeof(fmsg.raw));

	ret = mtk_pce_fe_mem_msg_send(&fmsg);
	if (ret) {
		PCE_NOTICE("fe_mem send dipfilter desc failed\n");
		return ret;
	}

	memset(&dip_entry->ddesc, 0, sizeof(struct dip_desc));
	clear_bit(dip_entry->index, dip_hw.dip_tbl);

unlock:
	spin_unlock_irqrestore(&dip_hw.lock, flag);

	return 0;
}
EXPORT_SYMBOL(mtk_pce_dipfilter_entry_del);

static void mtk_pce_dipfilter_clean_up(void)
{
	struct fe_mem_msg fmsg = {
		.cmd = FE_MEM_CMD_WRITE,
		.type = FE_MEM_TYPE_DIPFILTER,
	};
	unsigned long flag;
	int ret = 0;
	u32 i = 0;

	memset(&fmsg.raw, 0, sizeof(fmsg.raw));

	spin_lock_irqsave(&dip_hw.lock, flag);

	/* clear all dipfilter desc on init */
	for (i = 0; i < FE_MEM_DIPFILTER_MAX_IDX; i++) {
		fmsg.index = i;
		ret = mtk_pce_fe_mem_msg_send(&fmsg);
		if (ret)
			goto unlock;

		dip_hw.dip_entries[i].index = i;
	}

unlock:
	spin_unlock_irqrestore(&dip_hw.lock, flag);
}

int mtk_pce_dipfilter_init(struct platform_device *pdev)
{
	spin_lock_init(&dip_hw.lock);

	mtk_pce_dipfilter_clean_up();

	return 0;
}

void mtk_pce_dipfilter_deinit(struct platform_device *pdev)
{
	mtk_pce_dipfilter_clean_up();
}
