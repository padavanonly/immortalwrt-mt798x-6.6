/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MTK_UTILITY_H__
#define __MTK_UTILITY_H__

#include "os/mt_linux.h"

#define __bf_shf(x) (__builtin_ffsll(x) - 1)

#define field_prep(_mask, _val)						\
	({								\
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

#define field_get(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

#define SET_FIELD(_field, _val)	\
	({	\
		(((_val) << (_field##_SHIFT)) & (_field##_MASK));	\
	})

#define GET_FIELD(_field, _reg)	\
	({	\
		(((_reg) & (_field##_MASK)) >> (_field##_SHIFT));	\
	})

struct mtk_range {
	u32 start;
	u32 end;
};

struct mtk_idr_entry {
	int idx;
	void *data;
	struct dentry *dir;
	struct dentry *relate_dir;
	int (*read)(struct seq_file *s, void *data);
};

struct mtk_idr_mgmt {
	struct dentry *debugfs_dir;
	spinlock_t lock;
	struct idr id;
	u32 cnt;
	int high;
	int low;
};

int
mtk_idrm_init(struct mtk_idr_mgmt *idrm, char *name, struct dentry *parent, u32 low, u32 high);

void
mtk_idrm_exit(struct mtk_idr_mgmt *idrm);

void
mtk_idr_entry_unregister(struct mtk_idr_mgmt *_idrm, struct mtk_idr_entry *idr_entry);

int
mtk_idr_entry_register(struct mtk_idr_mgmt *_idrm, struct mtk_idr_entry *idr_entry, void *data);

int
mtk_idr_entry_register_debugfs(struct mtk_idr_mgmt *_idrm,
	struct mtk_idr_entry *idr_entry, const char *prefix,
	int (*read_fn)(struct seq_file *s, void *data));

int
mtk_idr_entry_register_debugfs_with_parent(struct mtk_idr_mgmt *idrm,
	struct mtk_idr_entry *idr_entry, const char *prefix, struct dentry *parent,
	int (*read_fn)(struct seq_file *s, void *data));

void
mtk_idr_entry_unregister_debugfs(struct mtk_idr_mgmt *idrm,
	struct mtk_idr_entry *idr_entry);

int
mtk_idr_register(struct mtk_idr_mgmt *idrm, u32 *idx, void *data);

void
mtk_idr_unregister(struct mtk_idr_mgmt *idrm, u32 idx);

/* caller shall handle old_data based on the return value */
long
mtk_idr_replace(struct mtk_idr_mgmt *idrm, u32 idx, void *data, void **old_data);

static inline void *
mtk_idr_entry_find(struct mtk_idr_mgmt *idrm, u32 id)
{
	void *data;

	spin_lock_bh(&idrm->lock);
	data = mt_idr_find(&idrm->id, id);
	spin_unlock_bh(&idrm->lock);
	return data;
}

#define mtk_idr_for_each_entry(_idrm, _entry, _id) \
	idr_for_each_entry(&(_idrm)->id, _entry, _id)

static inline u32
mtk_idr_count(struct mtk_idr_mgmt *idrm)
{
	u32 cnt;

	spin_lock_bh(&idrm->lock);
	cnt = idrm->cnt;
	spin_unlock_bh(&idrm->lock);
	return cnt;
}

static inline u32
mtk_idr_free_pool_count(struct mtk_idr_mgmt *idrm)
{
	u32 total = 0, cnt = 0;

	spin_lock_bh(&idrm->lock);
	if (idrm->high > idrm->low)
		total = idrm->high - idrm->low + 1;

	if (total > idrm->cnt)
		cnt = total - idrm->cnt;

	spin_unlock_bh(&idrm->lock);
	return cnt;
}

#endif
