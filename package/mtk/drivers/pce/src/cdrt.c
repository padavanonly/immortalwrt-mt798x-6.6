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

#include "pce/cdrt.h"
#include "pce/internal.h"
#include "pce/netsys.h"

/* decrypt use cdrt idx 1 ~ 15, encrypt use cdrt idx 16 ~ 31 */
#define CDRT_ENC_MAX_ENTRY		16
#define CDRT_ENC_IDX_OFS		16
#define CDRT_DEC_MAX_ENTRY		15
#define CDRT_DEC_IDX_OFS		1

struct cdrt_hardware {
	DECLARE_BITMAP(enc_used, CDRT_ENC_MAX_ENTRY);
	DECLARE_BITMAP(dec_used, CDRT_DEC_MAX_ENTRY);
	struct cdrt_entry enc_tbl[CDRT_ENC_MAX_ENTRY];
	struct cdrt_entry dec_tbl[CDRT_DEC_MAX_ENTRY];
	spinlock_t lock;
};

struct cdrt_hardware cdrt_hw;

int mtk_pce_cdrt_desc_write(struct cdrt_desc *desc, u32 idx)
{
	struct fe_mem_msg msg;
	int ret;

	if (unlikely(!desc || idx >= FE_MEM_CDRT_MAX_INDEX / 3))
		return -EINVAL;

	memset(&msg.raw, 0, sizeof(msg.raw));

	/* write CDR 0 ~ 3 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_CDRT,
				  3 * idx);
	memcpy(&msg.raw, &desc->raw1, sizeof(desc->raw1));

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	/* write CDR 4 ~ 7 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_CDRT,
				  3 * idx + 1);
	memcpy(&msg.raw, &desc->raw2, sizeof(desc->raw2));

	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	/* write CDR 8 ~ 11 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_WRITE, FE_MEM_TYPE_CDRT,
				  3 * idx + 2);
	memcpy(&msg.raw, &desc->raw3, sizeof(desc->raw3));

	ret = mtk_pce_fe_mem_msg_send(&msg);

	return ret;
}

int mtk_pce_cdrt_desc_read(struct cdrt_desc *desc, u32 idx)
{
	struct fe_mem_msg msg;
	int ret;

	if (unlikely(!desc || idx >= FE_MEM_CDRT_MAX_INDEX / 3))
		return -EINVAL;

	memset(&msg.raw, 0, sizeof(msg.raw));

	/* read CDR 0 ~ 3 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_CDRT,
				  3 * idx);
	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	memcpy(&desc->raw1, &msg.raw, sizeof(desc->raw1));

	/* read CDR 4 ~ 7 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_CDRT,
				  3 * idx + 1);
	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	memcpy(&desc->raw2, &msg.raw, sizeof(desc->raw2));

	/* read CDR 8 ~ 11 */
	mtk_pce_fe_mem_msg_config(&msg, FE_MEM_CMD_READ, FE_MEM_TYPE_CDRT,
				  3 * idx + 2);
	ret = mtk_pce_fe_mem_msg_send(&msg);
	if (ret)
		return ret;

	memcpy(&desc->raw3, &msg.raw, sizeof(desc->raw3));

	return ret;
}

int mtk_pce_cdrt_entry_write(struct cdrt_entry *cdrt)
{
	if (unlikely(!cdrt))
		return -EINVAL;

	return mtk_pce_cdrt_desc_write(&cdrt->desc, cdrt->idx);
}
EXPORT_SYMBOL(mtk_pce_cdrt_entry_write);

struct cdrt_entry *mtk_pce_cdrt_entry_find(u32 cdrt_idx)
{
	struct cdrt_entry *cdrt;
	unsigned long flag;
	u32 idx;

	if (unlikely(!cdrt_idx || cdrt_idx > CDRT_DEC_MAX_ENTRY + CDRT_ENC_MAX_ENTRY))
		return ERR_PTR(-EINVAL);

	spin_lock_irqsave(&cdrt_hw.lock, flag);

	if (cdrt_idx < CDRT_DEC_IDX_OFS + CDRT_DEC_MAX_ENTRY) {
		idx = cdrt_idx - CDRT_DEC_IDX_OFS;
		if (!test_bit(idx, cdrt_hw.dec_used))
			return ERR_PTR(-ENODEV);

		cdrt = &cdrt_hw.dec_tbl[idx];
	} else {
		idx = cdrt_idx - CDRT_ENC_IDX_OFS;
		if (!test_bit(idx, cdrt_hw.enc_used))
			return ERR_PTR(-ENODEV);

		cdrt = &cdrt_hw.enc_tbl[idx];
	}

	spin_unlock_irqrestore(&cdrt_hw.lock, flag);

	return cdrt;
}
EXPORT_SYMBOL(mtk_pce_cdrt_entry_find);

static struct cdrt_entry *mtk_pce_cdrt_entry_encrypt_alloc(void)
{
	u32 idx;

	lockdep_assert_held(&cdrt_hw.lock);

	idx = find_first_zero_bit(cdrt_hw.enc_used, CDRT_ENC_MAX_ENTRY);
	if (idx == CDRT_ENC_MAX_ENTRY)
		return ERR_PTR(-ENOMEM);

	set_bit(idx, cdrt_hw.enc_used);

	return &cdrt_hw.enc_tbl[idx];
}

static struct cdrt_entry *mtk_pce_cdrt_entry_decrypt_alloc(void)
{
	u32 idx;

	lockdep_assert_held(&cdrt_hw.lock);

	idx = find_first_zero_bit(cdrt_hw.dec_used, CDRT_DEC_MAX_ENTRY);
	if (idx == CDRT_DEC_MAX_ENTRY)
		return ERR_PTR(-ENOMEM);

	set_bit(idx, cdrt_hw.dec_used);

	return &cdrt_hw.dec_tbl[idx];
}

struct cdrt_entry *mtk_pce_cdrt_entry_alloc(enum cdrt_type type)
{
	struct cdrt_entry *cdrt;
	unsigned long flag;

	if (type >= __CDRT_TYPE_MAX)
		return ERR_PTR(-EINVAL);

	spin_lock_irqsave(&cdrt_hw.lock, flag);

	if (type == CDRT_ENCRYPT)
		cdrt = mtk_pce_cdrt_entry_encrypt_alloc();
	else
		cdrt = mtk_pce_cdrt_entry_decrypt_alloc();

	spin_unlock_irqrestore(&cdrt_hw.lock, flag);

	return cdrt;
}
EXPORT_SYMBOL(mtk_pce_cdrt_entry_alloc);

void mtk_pce_cdrt_entry_free(struct cdrt_entry *cdrt)
{
	unsigned long flag;

	if (!cdrt)
		return;

	cdrt->cls = NULL;

	memset(&cdrt->desc.raw1, 0, sizeof(cdrt->desc.raw1));
	memset(&cdrt->desc.raw2, 0, sizeof(cdrt->desc.raw2));
	memset(&cdrt->desc.raw3, 0, sizeof(cdrt->desc.raw3));

	spin_lock_irqsave(&cdrt_hw.lock, flag);

	if (cdrt->type == CDRT_ENCRYPT)
		clear_bit(cdrt->idx, cdrt_hw.enc_used);
	else
		clear_bit(cdrt->idx, cdrt_hw.dec_used);

	spin_unlock_irqrestore(&cdrt_hw.lock, flag);
}
EXPORT_SYMBOL(mtk_pce_cdrt_entry_free);

static void mtk_pce_cdrt_clean_up(void)
{
	struct fe_mem_msg msg = {
		.cmd = FE_MEM_CMD_WRITE,
		.type = FE_MEM_TYPE_CDRT,
	};
	unsigned long flag;
	int ret = 0;
	u32 i;

	memset(&msg.raw, 0, sizeof(msg.raw));

	spin_lock_irqsave(&cdrt_hw.lock, flag);

	for (i = 0; i < FE_MEM_CDRT_MAX_INDEX; i++) {
		msg.index = i;

		ret = mtk_pce_fe_mem_msg_send(&msg);
		if (ret)
			return;

		/* TODO: clean up bit map? */
	}

	spin_unlock_irqrestore(&cdrt_hw.lock, flag);
}

int mtk_pce_cdrt_enable(void)
{
	mtk_pce_netsys_setbits(GLO_MEM_CFG, CDM_CDRT_EN);

	return 0;
}

void mtk_pce_cdrt_disable(void)
{
	mtk_pce_netsys_clrbits(GLO_MEM_CFG, CDM_CDRT_EN);
}

int mtk_pce_cdrt_init(struct platform_device *pdev)
{
	u32 i;

	spin_lock_init(&cdrt_hw.lock);

	mtk_pce_cdrt_clean_up();

	for (i = 0; i < CDRT_DEC_MAX_ENTRY; i++) {
		cdrt_hw.dec_tbl[i].idx = i + CDRT_DEC_IDX_OFS;
		cdrt_hw.dec_tbl[i].type = CDRT_DECRYPT;
	}

	for (i = 0; i < CDRT_ENC_MAX_ENTRY; i++) {
		cdrt_hw.enc_tbl[i].idx = i + CDRT_ENC_IDX_OFS;
		cdrt_hw.enc_tbl[i].type = CDRT_ENCRYPT;
	}

	return 0;
}

void mtk_pce_cdrt_deinit(struct platform_device *pdev)
{
	mtk_pce_cdrt_clean_up();
}
