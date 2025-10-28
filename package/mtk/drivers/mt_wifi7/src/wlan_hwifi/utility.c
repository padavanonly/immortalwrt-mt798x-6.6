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

#include <linux/kernel.h>
#include "core.h"

static int
mtk_idr_debugfs_open(struct inode *inode, struct file *file)
{
	struct mtk_idr_entry *idr_entry = inode->i_private;

	return single_open(file, idr_entry->read, idr_entry);
}

static const struct file_operations mtk_idr_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = mtk_idr_debugfs_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct dentry *
mtk_debugfs_create_file(const char *name, struct mtk_idr_mgmt *idrm,
	struct mtk_idr_entry *idr_entry, struct dentry *parent)
{
	if (IS_ERR(parent))
		return ERR_PTR(-ENOENT);

	return mt_debugfs_create_file(name, 0644, parent, idr_entry, &mtk_idr_debugfs_fops);
}

int
mtk_idr_entry_register(struct mtk_idr_mgmt *idrm, struct mtk_idr_entry *idr_entry, void *data)
{
	int ret;
	spin_lock_bh(&idrm->lock);
	idr_entry->idx = mt_idr_alloc(&idrm->id, data, idrm->low, idrm->high+1, GFP_ATOMIC);
	idr_entry->dir = NULL;
	idr_entry->data = data;

	if (idr_entry->idx < idrm->low) {
		ret = -ENOMEM;
		goto err;
	}

	idrm->cnt++;
	ret = 0;
err:
	spin_unlock_bh(&idrm->lock);
	return ret;
}

void
mtk_idr_entry_unregister(struct mtk_idr_mgmt *idrm, struct mtk_idr_entry *idr_entry)
{
	if (idr_entry && idr_entry->idx >= 0) {
		spin_lock_bh(&idrm->lock);
		mt_idr_remove(&idrm->id, idr_entry->idx);
		idrm->cnt--;
		spin_unlock_bh(&idrm->lock);

		mt_debugfs_remove_recursive(idr_entry->dir);
		mt_debugfs_remove_recursive(idr_entry->relate_dir);
	}
}

int
mtk_idr_register(struct mtk_idr_mgmt *idrm, u32 *idx, void *data)
{
	int ret;

	spin_lock_bh(&idrm->lock);
	ret = mt_idr_alloc(&idrm->id, data, idrm->low, idrm->high+1, GFP_ATOMIC);
	if (ret < idrm->low) {
		ret = -ENOMEM;
		goto err;
	}
	*idx = ret;
	idrm->cnt++;
	ret = 0;
err:
	spin_unlock_bh(&idrm->lock);
	return ret;
}

void
mtk_idr_unregister(struct mtk_idr_mgmt *idrm, u32 idx)
{
	spin_lock_bh(&idrm->lock);
	mt_idr_remove(&idrm->id, idx);
	idrm->cnt--;
	spin_unlock_bh(&idrm->lock);
}

long
mtk_idr_replace(struct mtk_idr_mgmt *idrm, u32 idx, void *data, void **old_data)
{
	void *ptr;
	long ret;

	spin_lock_bh(&idrm->lock);
	ptr = idr_replace(&idrm->id, data, idx);
	spin_unlock_bh(&idrm->lock);

	if (IS_ERR(ptr)) {
		ret = -PTR_ERR(ptr);
		goto err;
	}

	*old_data = ptr;
	return 0;
err:
	return ret;
}

int
mtk_idrm_init(struct mtk_idr_mgmt *idrm, char *name, struct dentry *parent, u32 low, u32 high)
{
	struct dentry *dir;

	spin_lock_init(&idrm->lock);
	idr_init(&idrm->id);

	spin_lock_bh(&idrm->lock);
	idrm->cnt = 0;
	idrm->low = low;
	idrm->high = high;
	spin_unlock_bh(&idrm->lock);

	if (parent && name) {
		dir = mt_debugfs_create_dir(name, parent);
		if (!dir)
			return -ENOMEM;
		idrm->debugfs_dir = dir;
	}
	return 0;
}

void
mtk_idrm_exit(struct mtk_idr_mgmt *idrm)
{
	spin_lock_bh(&idrm->lock);
	idr_destroy(&idrm->id);
	spin_unlock_bh(&idrm->lock);
	spin_lock_init(&idrm->lock);

	mt_debugfs_remove_recursive(idrm->debugfs_dir);
}

int
mtk_idr_entry_register_debugfs_with_parent(struct mtk_idr_mgmt *idrm,
	struct mtk_idr_entry *idr_entry, const char *prefix, struct dentry *parent,
	int (*read_fn)(struct seq_file *s,
			void *data))
{
	char name[32];
	int ret;

	if (!idr_entry || !parent)
		return -EINVAL;

	ret = snprintf(name, sizeof(name), "%s%d", prefix, idr_entry->idx);
	if (ret < 0) {
		pr_err("%s(): name parse error\n", __func__);
		name[0] = '\0';
	}
	idr_entry->read = read_fn;
	idr_entry->relate_dir = mtk_debugfs_create_file(
		name, idrm, idr_entry, parent);
	return idr_entry->relate_dir ? 0 : -ENOMEM;
}

int
mtk_idr_entry_register_debugfs(struct mtk_idr_mgmt *idrm,
	struct mtk_idr_entry *idr_entry, const char *prefix,
	int (*read_fn)(struct seq_file *s,
			void *data))
{
	return mtk_idr_entry_register_debugfs_with_parent(idrm, idr_entry,
		prefix, idrm->debugfs_dir, read_fn);
}


void
mtk_idr_entry_unregister_debugfs(struct mtk_idr_mgmt *idrm, struct mtk_idr_entry *idr_entry)
{
	if (idr_entry && idr_entry->idx >= 0) {
		mt_debugfs_remove_recursive(idr_entry->dir);
		idr_entry->dir = NULL;
		mt_debugfs_remove_recursive(idr_entry->relate_dir);
		idr_entry->relate_dir = NULL;
	}
}

