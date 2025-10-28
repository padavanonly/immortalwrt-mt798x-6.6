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

#include "core.h"
#include "tk_mgmt.h"
#include "hw_ops.h"

#ifdef CONFIG_HWIFI_DBG
static int
mtk_tk_entry_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_tk_entry *tk_entry = (struct mtk_tk_entry *)sys_idx->data;
	struct mtk_hw_dev *dev = tk_entry->hw_dev;

	seq_printf(s, "toke_id:\t %d\n", tk_entry->sid.idx);
	seq_printf(s, "entry:\t %p\n", tk_entry->sid.data);
	/* TODO: skb_buff_head list */
	seq_printf(s, "physical_addr:\t %pad\n", &tk_entry->dma_addr);
	seq_printf(s, "txd_addr:\t %pad\n", &tk_entry->txd_ptr);
	seq_printf(s, "fbuf_size:\t %pad\n", &tk_entry->fbuf_size);
	seq_printf(s, "fbuf_dma_size:\t %pad\n", &tk_entry->fbuf_dma_size);
	if (dev->dbg_ops->txd_info)
		dev->dbg_ops->txd_info(dev, (u32 *)tk_entry->txd_ptr);
	return 0;
}

static int
mtk_rx_tk_entry_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_rx_tk_entry *tk_entry = (struct mtk_rx_tk_entry *)sys_idx->data;

	seq_printf(s, "toke_id:\t %d\n", tk_entry->sid.idx);
	seq_printf(s, "entry:\t %p\n", tk_entry->sid.data);
	seq_printf(s, "skb_addr:\t %p\n", tk_entry->pkt);
	seq_printf(s, "physical_addr:\t 0x%llx\n", tk_entry->pkt_pa);
	seq_printf(s, "virtual_addr:\t %p\n", tk_entry->pkt_va);
	seq_printf(s, "size:\t %u\n", tk_entry->pkt_size);
	return 0;
}
#endif

static ssize_t
mtk_tk_mgmt_write_debugfs(struct file *file,
					const char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *phy;
	u32 band, max_nums, rsv_nums;
	char buf[256] = {0};
	int ret, rc;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = sscanf(buf, "%u %u %u", &band, &max_nums, &rsv_nums);

	if (ret != 3) {
		ret = -EINVAL;
		goto done;
	}

	if (band < MAX_BAND_NUM) {
		phy = phy_mgmt->phys[band];
		if (phy) {
			ret = mtk_tk_res_ctl_req(phy, max_nums, rsv_nums);

			if (ret)
				goto done;
		} else {
			ret = -EINVAL;
			dev_err(dev->dev, "phy of band(%u) is not registered\n",
								band);
			goto done;
		}
	} else {
		ret = -EINVAL;
		dev_err(dev->dev, "band idx(%u) >= %u\n", band, MAX_BAND_NUM);
		goto done;
	}

	ret = count;
done:
	return ret;
}

static ssize_t
mtk_tk_mgmt_read_debugfs(struct file *file,
					char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_tk_mgmt *tk = &hw_ctrl->tk_mgmt[TK_MGMT_CMM];
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *phy;
	struct mtk_tk_res_ctl *res_ctl;
	struct mtk_tk_res *rsv_res;
	char buf[256] = {0};
	int len = 0, i;

	len += scnprintf(buf + len, sizeof(buf) - len,
					"total token cnt:%u\n",
					tk->base.cnt);

	for (i = 0; i < MAX_BAND_NUM; i++) {
		phy = phy_mgmt->phys[i];

		if (phy) {
			res_ctl = phy->tk_res_ctl;
			rsv_res = &res_ctl->rsv_res;
			len += scnprintf(buf + len, sizeof(buf) - len,
					"band(%d): max_nums = %u, rsv_num = %u\n",
					i, res_ctl->max_nums, rsv_res->max_nums);

		}
	}

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static const struct file_operations fops_mtk_tk_mgmt = {
	.read = mtk_tk_mgmt_read_debugfs,
	.write = mtk_tk_mgmt_write_debugfs,
	.open = simple_open,
};

static int
mtk_tx_tk_mgmt_debugfs(struct seq_file *s, void *data)
{
	void *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];
	struct mtk_tk_entry *tk_entry;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *phy;
	struct mtk_tk_res_ctl *res_ctl;
	struct mtk_tk_res *rsv_res;
	int id, i, cnt = 0;
	bool tk_bmc_exist = tk_bmc->max_tx_tk_nums ? true : false;

	spin_lock_bh(&tk->base.lock);
	if (tk_bmc_exist)
		spin_lock_bh(&tk_bmc->base.lock);

	seq_puts(s, "==============token pool==============\n");
	seq_puts(s, "token id:\n");
	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {
		if (!tk_entry->rdy)
			continue;

		seq_printf(s, " %d ", id);
		if (cnt % 10 == 0)
			seq_puts(s, "\n");
		cnt++;
	}
	seq_puts(s, "\n");
	seq_printf(s, "max token cnt: %u\n", tk->max_tx_tk_nums);
	seq_printf(s, "total token cnt: %u\n", tk->base.cnt);
	seq_printf(s, "used cnt: %d\n\n", cnt);

	if (!tk_bmc_exist)
		goto dump_per_phy_used;

	cnt = 0;
	seq_puts(s, "============bmc token pool============\n");
	seq_puts(s, "token id:\n");
	mtk_idr_for_each_entry(&tk_bmc->base, tk_entry, id) {
		if (!tk_entry->rdy)
			continue;

		seq_printf(s, " %d ", id);
		if (cnt % 10 == 0)
			seq_puts(s, "\n");
		cnt++;
	}
	seq_puts(s, "\n");
	seq_printf(s, "max token cnt: %u\n", tk_bmc->max_tx_tk_nums);
	seq_printf(s, "total token cnt: %u\n", tk_bmc->base.cnt);
	seq_printf(s, "used cnt: %d\n\n", cnt);

dump_per_phy_used:
	for (i = 0; i < MAX_BAND_NUM; i++) {
		phy = phy_mgmt->phys[i];
		if (phy) {
			res_ctl = phy->tk_res_ctl;
			rsv_res = &res_ctl->rsv_res;
			seq_printf(s, "band: %d\n", i);
			seq_printf(s, "\tmax_nums: %u\n", res_ctl->max_nums);
			seq_printf(s, "\trsv_nums: %u\n", rsv_res->max_nums);
			seq_printf(s, "\tused: %d\n", atomic_read(&res_ctl->used));
			seq_printf(s, "\trsv_used: %d\n", atomic_read(&rsv_res->used));
		}
	}

	if (tk_bmc_exist)
		spin_unlock_bh(&tk_bmc->base.lock);
	spin_unlock_bh(&tk->base.lock);

	return 0;
}

static int
mtk_rx_tk_mgmt_debugfs(struct seq_file *s, void *data)
{
	void *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;
	struct mtk_rx_tk_entry *tk_entry;
	int id, cnt = 0, total = 0;

	spin_lock_bh(&tk->base.lock);
	seq_puts(s, "toke id:\n");
	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {
		total++;

		seq_printf(s, " %d ", id);
		if (cnt%10 == 0)
			seq_puts(s, "\n");
		cnt++;
	}
	seq_puts(s, "\n");
	seq_printf(s, "token cnt: %u\n", tk->base.cnt);
	seq_printf(s, "total cnt: %d\n", total);
	seq_printf(s, "used cnt: %d\n", cnt);
	spin_unlock_bh(&tk->base.lock);
	return 0;
}

static struct mtk_tk_entry *
mtk_tk_alloc_entry(struct mtk_hw_dev *dev, struct mtk_tk_mgmt *tk_mgmt)
{
	struct mtk_tk_entry *tk_entry;
	u8 *txd_ptr;
	u16 fbuf_size;
	int size;
	dma_addr_t addr;

	fbuf_size = tk_mgmt->fbuf_size;
	size = L1_CACHE_ALIGN(fbuf_size + sizeof(*tk_entry));
	txd_ptr = kzalloc(size, GFP_ATOMIC);

	if (!txd_ptr) {
		dev_err(tk_mgmt->pdev, "%s(): Allocate failed\n", __func__);
		return NULL;
	}

	/*map a physical address*/
	addr = dma_map_single(tk_mgmt->pdev, txd_ptr, fbuf_size, DMA_TO_DEVICE);
	tk_entry = (struct mtk_tk_entry *) (txd_ptr + fbuf_size);
	tk_entry->dma_addr = addr;

	/*assign txd_ptr*/
	tk_entry->txd_ptr = txd_ptr;
	tk_entry->fbuf_size = fbuf_size;
	tk_entry->hw_dev = dev;
	tk_entry->tk_mgmt = tk_mgmt;
	tk_entry->rdy = false;
	return tk_entry;
}

static int
mtk_tk_mgmt_debugfs_register(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tx_tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_rx_tk_mgmt *rx_tk = &dev->hw_ctrl.rx_tk_mgmt;

	mt_debugfs_create_devm_seqfile(dev->dev, "statistic",
		tx_tk->base.debugfs_dir, mtk_tx_tk_mgmt_debugfs);

	mt_debugfs_create_file("tk_mgmt", 0644,
				tx_tk->base.debugfs_dir,
				dev, &fops_mtk_tk_mgmt);

	mt_debugfs_create_devm_seqfile(dev->dev, "statistic",
		rx_tk->base.debugfs_dir, mtk_rx_tk_mgmt_debugfs);
	return 0;
}

static void
mtk_tk_free_entry(struct mtk_tk_mgmt *tk_mgmt, struct mtk_tk_entry *tk_entry)
{
	if (!tk_entry || !tk_entry->txd_ptr)
		return;

	dma_unmap_single(tk_mgmt->pdev, tk_entry->dma_addr,
			tk_entry->fbuf_size, DMA_TO_DEVICE);
	kfree(tk_entry->txd_ptr);
}

static inline int
mtk_tk_entry_request_id(struct mtk_tk_mgmt *tk, struct mtk_tk_entry *tk_entry)
{
	int ret;

	ret = mtk_idr_entry_register(&tk->base, &tk_entry->sid, tk_entry);
	if (ret) {
		dev_err(tk->pdev, "%s(): Register idr failed\n", __func__);
		return ret;
	}
#ifdef CONFIG_HWIFI_DBG
	ret = mtk_idr_entry_register_debugfs(&tk->base,
		&tk_entry->sid, "", mtk_tk_entry_debugfs);
	if (ret) {
		dev_err(tk->pdev, "%s(): Register debugfs failed\n", __func__);
		mtk_idr_entry_unregister(&tk->base, &tk_entry->sid);
	}
#endif

	return ret;
}

/*
 *
 */
inline int
mtk_tk_unmap_tx_q(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	if (!tk_entry)
		return -EINVAL;

	mtk_hw_tx_skb_unmap(dev, tk_entry);
	return 0;
}

/*
 *
 */
inline struct sk_buff_head *
mtk_tk_get_tx_q(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	return &tk_entry->tx_q;
}

struct mtk_tk_entry *
mtk_tk_get_entry(struct mtk_hw_dev *dev, u32 id)
{
	struct mtk_tk_entry *tk_entry;
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	if (tk_bmc->max_tx_tk_nums &&
	    id >= tk_bmc->base.low && id <= tk_bmc->base.high) {
		tk = tk_bmc;
	} else if (id < tk->base.low || id > tk->base.high) {
		dbg_tk_free_tkid_oor_inc(tk);
		return NULL;
	}

	tk_entry = mtk_idr_entry_find(&tk->base, id);
	if (!tk_entry || !tk_entry->rdy) {
		dbg_tk_release_err_inc(tk);
		return NULL;
	}

	return tk_entry;
}

/*
 *
 */
int
mtk_tk_release_entry(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	struct mtk_tk_mgmt *tk = tk_entry->tk_mgmt;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *phy;
	struct mtk_tk_res_ctl *res_ctl;
	spinlock_t *lock = &tk->base.lock;
	struct list_head *tx_cache = &tk->tx_cache;
	struct mtk_tk_res *rsv_res;
	u8 band_idx;

	if (!tk_entry->rdy) {
		dbg_tk_release_err_inc(tk);
		dev_err(dev->dev, "%s(): Token id(%u) not ready\n",
			__func__, tk_entry->sid.idx);
		return -EINVAL;
	}

	band_idx = tk_entry->band_idx;

	if (band_idx < MAX_BAND_NUM) {
		phy = phy_mgmt->phys[band_idx];
		if (phy) {
			res_ctl = phy->tk_res_ctl;
			atomic_dec(&res_ctl->used);
		} else {
			dev_err(dev->dev, "%s: phy of band(%d) is not registered\n",
					__func__, tk_entry->band_idx);
			return -EINVAL;
		}
	} else {
		dev_err(dev->dev, "%s: band idx(%d) >= %d\n",
					__func__, tk_entry->band_idx, MAX_BAND_NUM);
			return -EINVAL;
	}

	/* clear run-time modified field during releasing time */
	tk_entry->rdy = false;
	tk_entry->pkt_pa = 0x0;
	tk_entry->tid = 0;
	tk_entry->is_fixed_rate = 0;
	tk_entry->is_prior = 0;
	tk_entry->is_sp = 0;
	tk_entry->hf = 0;
	tk_entry->amsdu_en = 0;
	if (tk_entry->is_rsv) {
		rsv_res = &res_ctl->rsv_res;
		atomic_dec(&rsv_res->used);
		lock = &rsv_res->lock;
		tx_cache = &rsv_res->cache;
	}
	spin_lock_bh(lock);
	list_add_tail(&tk_entry->list, tx_cache);
	dbg_tk_release_inc(tk);
	spin_unlock_bh(lock);

	return 0;
}

/*
 *
 */
static inline int
_mtk_tk_check_resource(struct mtk_hw_phy *phy, struct mtk_tk_mgmt *tk)
{
	struct mtk_tk_res_ctl *res_ctl = phy->tk_res_ctl;
	struct mtk_tk_res *rsv_res = &res_ctl->rsv_res;

	spin_lock_bh(&tk->base.lock);
	if ((atomic_read(&res_ctl->used) < res_ctl->max_nums - rsv_res->max_nums) &&
		!list_empty(&tk->tx_cache)) {
		spin_unlock_bh(&tk->base.lock);
		return 1;
	} else {
		spin_unlock_bh(&tk->base.lock);
		dbg_tk_request_err_inc(tk);
		return 0;
	}
}

int
mtk_tk_check_resource(struct mtk_hw_phy *phy)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];

	return _mtk_tk_check_resource(phy, tk);
}

int
mtk_tk_bmc_check_resource(struct mtk_hw_phy *phy)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	if (!tk_bmc->max_tx_tk_nums)
		return -EOPNOTSUPP;

	return _mtk_tk_check_resource(phy, tk_bmc);
}

static inline int
_mtk_tk_rsv_check_resource(struct mtk_hw_phy *phy, struct mtk_tk_mgmt *tk)
{
	struct mtk_tk_res_ctl *res_ctl = phy->tk_res_ctl;
	struct mtk_tk_res *rsv_res = &res_ctl->rsv_res;

	spin_lock_bh(&rsv_res->lock);
	if ((atomic_read(&rsv_res->used) < rsv_res->max_nums) &&
		!list_empty(&rsv_res->cache)) {
		spin_unlock_bh(&rsv_res->lock);
		return 1;
	}

	spin_unlock_bh(&rsv_res->lock);
	return 0;
}

int
mtk_tk_rsv_check_resource(struct mtk_hw_phy *phy)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_tk_res *rsv_res = &phy->tk_res_ctl->rsv_res;

	if (!rsv_res->max_nums)
		return -EOPNOTSUPP;

	return _mtk_tk_rsv_check_resource(phy, tk);
}

/*
 *
 */
int
mtk_tk_res_ctl_alloc(struct mtk_hw_phy *phy, struct mtk_hwres_radio_cap *radio_cap)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_tk_res_ctl *res_ctl;
	struct mtk_tk_res *rsv_res;
	int cnt = tk->base.high - tk->base.low + 1;

	if (radio_cap->max_tx_tk_nums > cnt) {
		dev_err(dev->dev, "%s: req_tk_nums: %d, total_tk_nums: %d",
			__func__, radio_cap->max_tx_tk_nums, cnt);
		return -EINVAL;
	}

	if (radio_cap->rsv_tx_tk_nums > radio_cap->max_tx_tk_nums) {
		dev_err(dev->dev, "%s: rsv_tx_tk_nums: %d, max_tx_tk_nums: %d",
			__func__,
			radio_cap->rsv_tx_tk_nums, radio_cap->max_tx_tk_nums);
		return -EINVAL;
	}

	res_ctl = kzalloc(sizeof(*res_ctl), GFP_ATOMIC);

	if(!res_ctl)
		return -ENOMEM;

	res_ctl->max_nums = radio_cap->max_tx_tk_nums;
	rsv_res = &res_ctl->rsv_res;
	rsv_res->max_nums = radio_cap->rsv_tx_tk_nums;

	phy->tk_res_ctl = res_ctl;

	return 0;
}

/*
 *
 */
int
mtk_tk_res_ctl_free(struct mtk_hw_phy *phy)
{
	kfree(phy->tk_res_ctl);

	return 0;
}

/*
 *
 */
int
mtk_tk_res_ctl_req(struct mtk_hw_phy *phy, u16 max_nums, u16 rsv_nums)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_tk_res_ctl *res_ctl;
	struct mtk_tk_res *rsv_res;

	if (max_nums > tk->base.cnt) {
		dev_err(dev->dev, "%s: req_tk_nums: %d, total_tk_nums: %d",
					__func__, max_nums, tk->base.cnt);
		return -EINVAL;
	}

	if (rsv_nums > max_nums) {
		dev_err(dev->dev, "%s: req_rsv_nums: %d, max_tk_nums: %d",
					__func__, rsv_nums, max_nums);
		return -EINVAL;
	}

	res_ctl = phy->tk_res_ctl;
	res_ctl->max_nums = max_nums;
	rsv_res = &res_ctl->rsv_res;
	rsv_res->max_nums = rsv_nums;

	return 0;
}

/*
 *
 */
static inline struct mtk_tk_entry *
_mtk_tk_request_entry(struct mtk_tk_mgmt *tk)
{
	struct mtk_tk_entry *tk_entry = NULL;

	spin_lock_bh(&tk->base.lock);
	if (!list_empty(&tk->tx_cache)) {
		tk_entry = list_first_entry(&tk->tx_cache, struct mtk_tk_entry, list);
		list_del(&tk_entry->list);
		skb_queue_head_init(&tk_entry->tx_q);
		dbg_tk_request_inc(tk);
	} else {
		dbg_tk_request_err_inc(tk);
	}
	spin_unlock_bh(&tk->base.lock);
	return tk_entry;
}

struct mtk_tk_entry *
mtk_tk_request_entry(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];

	return _mtk_tk_request_entry(tk);
}

struct mtk_tk_entry *
mtk_tk_bmc_request_entry(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	if (!tk_bmc->max_tx_tk_nums)
		return ERR_PTR(-EOPNOTSUPP);

	return _mtk_tk_request_entry(tk_bmc);
}

struct mtk_tk_entry *
mtk_tk_rsv_request_entry(struct mtk_hw_phy *phy)
{
	struct mtk_tk_res *tk_rsv_res = &phy->tk_res_ctl->rsv_res;
	struct mtk_tk_entry *tk_entry = NULL;

	if (!tk_rsv_res->max_nums)
		return ERR_PTR(-EOPNOTSUPP);

	spin_lock_bh(&tk_rsv_res->lock);
	if (!list_empty(&tk_rsv_res->cache)) {
		tk_entry = list_first_entry(&tk_rsv_res->cache, struct mtk_tk_entry, list);
		list_del(&tk_entry->list);
		skb_queue_head_init(&tk_entry->tx_q);
		atomic_inc(&tk_rsv_res->used);
	}
	spin_unlock_bh(&tk_rsv_res->lock);
	return tk_entry;
}


static int
mtk_tk_cache_init(struct mtk_hw_dev *dev, struct mtk_tk_mgmt *tk)
{
	int i = 0;
	int ret, id;
	struct mtk_tk_entry *tk_entry;

	INIT_LIST_HEAD(&tk->tx_cache);

	for (i = tk->base.low; i <= tk->base.high; i++) {
		tk_entry = mtk_tk_alloc_entry(dev, tk);
		if (!tk_entry) {
			dev_err(dev->dev, "%s(): Allocate tk_entry failed\n", __func__);
			goto err;
		}

		ret = mtk_tk_entry_request_id(tk, tk_entry);
		if (ret) {
			dev_err(dev->dev, "%s(): Request id failed\n", __func__);
			mtk_tk_free_entry(tk, tk_entry);
			goto err;
		}

		list_add_tail(&tk_entry->list, &tk->tx_cache);
	}

	return 0;
err:
	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {
		list_del(&tk_entry->list);
		mtk_idr_entry_unregister(&tk->base, &tk_entry->sid);
		mtk_tk_free_entry(tk, tk_entry);
	}

	return -ENOMEM;
}

static void
mtk_tk_cache_exit(struct mtk_hw_dev *dev, struct mtk_tk_mgmt *tk)
{
	struct mtk_tk_entry *tk_entry;
	struct sk_buff_head *tx_q;
	struct sk_buff *skb, *tmp;
	int id;

	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {

		if (!tk_entry->rdy)
			goto skip_release;

		mtk_tk_unmap_tx_q(dev, tk_entry);
		tx_q = mtk_tk_get_tx_q(dev, tk_entry);

		skb_queue_walk_safe(tx_q, skb, tmp) {
			__skb_unlink(skb, tx_q);
			dev_kfree_skb_any(skb);
		}

		mtk_tk_release_entry(dev, tk_entry);

skip_release:
		list_del(&tk_entry->list);
		mtk_idr_entry_unregister(&tk->base, &tk_entry->sid);
		mtk_tk_free_entry(tk, tk_entry);
	}
}

static int
mtk_tk_rsv_cache_init(struct mtk_hw_phy *phy)
{
	struct mtk_hw_dev *dev = phy->hw_dev;
	struct mtk_tk_res *rsv_res = &phy->tk_res_ctl->rsv_res;
	int i = 0;

	if (!rsv_res->max_nums)
		return 0;

	spin_lock_init(&rsv_res->lock);
	INIT_LIST_HEAD(&rsv_res->cache);

	for (i = 0; i < rsv_res->max_nums; i++) {
		struct mtk_tk_entry *tk_entry = mtk_tk_request_entry(dev);

		if (!tk_entry)
			return -ENOMEM;

		tk_entry->is_rsv = true;
		list_add_tail(&tk_entry->list, &rsv_res->cache);
	}

	return 0;
}

static int
mtk_tx_tk_init(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];

	return mtk_tk_cache_init(dev, tk);
}

static void
mtk_tx_tk_exit(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];

	mtk_tk_cache_exit(dev, tk);
}

static int
mtk_tx_tk_rsv_init(struct mtk_hw_dev *dev)
{
	struct mtk_hw_phy *hw_phy;
	int ret, id;

	mtk_idr_for_each_entry(&dev->hw_ctrl.phy_mgmt.mgmt, hw_phy, id) {
		ret = mtk_tk_rsv_cache_init(hw_phy);
		if (ret) {
			dev_err(dev->dev, "%s(): Tx reserved token init failed, ret = %d\n",
				__func__, ret);
			return -ENOMEM;
		}
	}

	return 0;
}

static void
mtk_tx_tk_rsv_exit(struct mtk_hw_dev *dev)
{
}

static int
mtk_tx_tk_mgmt_init(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	int ret;

	ret = mtk_idrm_init(&tk->base, "tx_tkinfo", dev->sys_idx.dir,
		res_cap->tx_token.start, res_cap->tx_token.end);

	if (ret)
		goto err;

	/* assign txd size for got txd_ptr,
	 * due to txd_ptr alloc ahead of tk_entry
	 * TODO: need to add 802.11 header + llc if needed
	 */
	tk->fbuf_size = dev->chip_drv->hw_caps->mtxd_sz;
	tk->pdev = dev->dev;
	tk->max_tx_tk_nums = res_cap->tx_token.end - res_cap->tx_token.start + 1;

	return 0;
err:
	return ret;
}

void
mtk_tx_tk_mgmt_exit(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk = &dev->hw_ctrl.tk_mgmt[TK_MGMT_CMM];

	mtk_idrm_exit(&tk->base);
}

static int
mtk_tx_tk_bmc_init(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	if (!tk_bmc->max_tx_tk_nums)
		return 0;
	return mtk_tk_cache_init(dev, tk_bmc);
}

static void
mtk_tx_tk_bmc_exit(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	mtk_tk_cache_exit(dev, tk_bmc);
}

static int
mtk_tx_tk_bmc_mgmt_init(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	int ret;

	if (res_cap->tx_token_bmc.end == 0) {
		tk_bmc->max_tx_tk_nums = 0;
		return 0;
	}

	ret = mtk_idrm_init(&tk_bmc->base, "tx_tkinfo_bmc", dev->sys_idx.dir,
		res_cap->tx_token_bmc.start, res_cap->tx_token_bmc.end);

	if (ret)
		goto err;

	/* assign txd size for got txd_ptr,
	 * due to txd_ptr alloc ahead of tk_entry
	 * TODO: need to add 802.11 header + llc if needed
	 */
	tk_bmc->fbuf_size = dev->chip_drv->hw_caps->mtxd_sz;
	tk_bmc->pdev = dev->dev;
	tk_bmc->max_tx_tk_nums = res_cap->tx_token_bmc.end - res_cap->tx_token_bmc.start + 1;

	return 0;
err:
	return ret;
}

void
mtk_tx_tk_bmc_mgmt_exit(struct mtk_hw_dev *dev)
{
	struct mtk_tk_mgmt *tk_bmc = &dev->hw_ctrl.tk_mgmt[TK_MGMT_BMC];

	mtk_idrm_exit(&tk_bmc->base);
}

static inline int
mtk_rx_tk_entry_update(struct mtk_rx_tk_mgmt *tk,
	struct mtk_rx_tk_entry *tk_entry,
	void *pkt, u32 len, void *va, dma_addr_t pa)
{
	tk_entry->pkt = pkt;
	tk_entry->pkt_size = len;
	tk_entry->pkt_va = va;
	tk_entry->pkt_pa = pa;
	tk_entry->pkt_pa_bk = pa;

	return 0;
}

static int
_mtk_rx_tk_request_entry(struct mtk_rx_tk_mgmt *tk, void *pkt,
	u32 len, void *va, dma_addr_t pa)
{
	struct mtk_rx_tk_entry *tk_entry = NULL;

	spin_lock_bh(&tk->base.lock);
	if (!list_empty(&tk->rx_cache)) {
		tk_entry = list_first_entry(&tk->rx_cache,
				struct mtk_rx_tk_entry, list);
		list_del(&tk_entry->list);
		dbg_rx_tk_request_inc(tk);
	} else {
		dbg_rx_tk_request_err_inc(tk);
		spin_unlock_bh(&tk->base.lock);
		goto err_request;
	}
	spin_unlock_bh(&tk->base.lock);

	mtk_rx_tk_entry_update(tk, tk_entry, pkt, len, va, pa);
	return tk_entry->sid.idx;

err_request:
	return -ENOMEM;
}

static void *
_mtk_rx_tk_release_entry(struct mtk_rx_tk_mgmt *tk, u32 id)
{
	struct mtk_rx_tk_entry *tk_entry;
	void *pkt;

	tk_entry = mtk_idr_entry_find(&tk->base, id);
	if (!tk_entry || unlikely(!tk_entry->pkt))
		return NULL;

	dma_unmap_single(tk->pdev, tk_entry->pkt_pa,
		tk_entry->pkt_size, DMA_FROM_DEVICE);
	pkt = tk_entry->pkt;
	tk_entry->pkt = NULL;

	spin_lock_bh(&tk->base.lock);
	list_add_tail(&tk_entry->list, &tk->rx_cache);
	dbg_rx_tk_release_inc(tk);
	spin_unlock_bh(&tk->base.lock);

	return pkt;
}

static int
_mtk_rx_tk_entry_query(struct mtk_rx_tk_mgmt *tk,
	u32 id, dma_addr_t *pa)
{
	struct mtk_rx_tk_entry *tk_entry;

	tk_entry = mtk_idr_entry_find(&tk->base, id);
	if (!tk_entry)
		return -EINVAL;

	if (pa)
		*pa = tk_entry->pkt_pa;

	if (unlikely(tk_entry->pkt_pa != tk_entry->pkt_pa_bk)) {
		pr_err("%s(): PA not match, dump tk_entry\n", __func__);
		pr_err("toke_id:\t %d\n", tk_entry->sid.idx);
		pr_err("entry:\t %p\n", tk_entry->sid.data);
		pr_err("skb_addr:\t %p\n", tk_entry->pkt);
		pr_err("physical_addr:\t %pad\n", &tk_entry->pkt_pa);
		pr_err("physical_addr_bk:\t %pad\n", &tk_entry->pkt_pa_bk);
		pr_err("virtual_addr:\t %p\n", tk_entry->pkt_va);
		pr_err("size:\t %u\n", tk_entry->pkt_size);
	}
	return 0;
}

static int
_mtk_rx_tk_entry_find(struct mtk_rx_tk_mgmt *tk,
	u32 id, void **pkt, void **va, dma_addr_t *pa)
{
	struct mtk_rx_tk_entry *tk_entry;

	tk_entry = mtk_idr_entry_find(&tk->base, id);

	if (!tk_entry)
		return -EINVAL;

	*pkt = tk_entry->pkt;
	*va = tk_entry->pkt_va;
	*pa = tk_entry->pkt_pa;

	return 0;
}

static struct mtk_rx_tk_ops rx_tk_ops = {
	.find = _mtk_rx_tk_entry_find,
	.query = _mtk_rx_tk_entry_query,
	.request = _mtk_rx_tk_request_entry,
	.release = _mtk_rx_tk_release_entry,
};

static struct mtk_rx_tk_entry *
mtk_rx_tk_alloc_entry(struct mtk_hw_dev *dev, struct mtk_rx_tk_mgmt *tk)
{
	struct mtk_rx_tk_entry *tk_entry;

	tk_entry = kzalloc(sizeof(*tk_entry), GFP_ATOMIC);
	if (!tk_entry)
		dev_err(tk->pdev, "%s(): Allocate failed\n", __func__);

	return tk_entry;
}

static inline void
mtk_rx_tk_free_entry(struct mtk_rx_tk_mgmt *tk, struct mtk_rx_tk_entry *tk_entry)
{
	kfree(tk_entry);
}

static inline int
mtk_rx_tk_entry_request_id(struct mtk_rx_tk_mgmt *tk, struct mtk_rx_tk_entry *tk_entry)
{
	int ret;

	ret = mtk_idr_entry_register(&tk->base, &tk_entry->sid, tk_entry);
	if (ret) {
		dev_err(tk->pdev, "%s(): Register idr failed\n", __func__);
		return ret;
	}

#ifdef CONFIG_HWIFI_DBG
	ret = mtk_idr_entry_register_debugfs(&tk->base,
		&tk_entry->sid, "", mtk_rx_tk_entry_debugfs);
	if (ret) {
		dev_err(tk->pdev, "%s(): Register debugfs failed\n", __func__);
		mtk_idr_entry_unregister(&tk->base, &tk_entry->sid);
	}
#endif

	return ret;
}

static int
mtk_rx_tk_cache_init(struct mtk_hw_dev *dev, struct mtk_rx_tk_mgmt *tk)
{
	int i = 0;
	int id;
	struct mtk_rx_tk_entry *tk_entry;

	INIT_LIST_HEAD(&tk->rx_cache);

	for (i = tk->base.low; i <= tk->base.high; i++) {
		tk_entry = mtk_rx_tk_alloc_entry(dev, tk);
		if (!tk_entry)
			goto err;

		if (mtk_rx_tk_entry_request_id(tk, tk_entry)) {
			mtk_rx_tk_free_entry(tk, tk_entry);
			goto err;
		}

		list_add_tail(&tk_entry->list, &tk->rx_cache);
	}

	return 0;
err:
	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {
		list_del(&tk_entry->list);
		mtk_rx_tk_free_entry(tk, tk_entry);
	}

	return -ENOMEM;
}

static int
mtk_rx_tk_init(struct mtk_hw_dev *dev)
{
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;

	return mtk_rx_tk_cache_init(dev, tk);
}

static void
mtk_rx_tk_exit(struct mtk_hw_dev *dev)
{
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;
	struct mtk_rx_tk_entry *tk_entry;
	void *pkt;
	int id;

	mtk_idr_for_each_entry(&tk->base, tk_entry, id) {
		if (!tk_entry->pkt)
			goto skip_release;

		pkt = _mtk_rx_tk_release_entry(tk, id);
		put_page(virt_to_head_page(pkt));

skip_release:
		list_del(&tk_entry->list);
		mtk_idr_entry_unregister(&tk->base, &tk_entry->sid);
		mtk_rx_tk_free_entry(tk, tk_entry);
	}
}

static int
mtk_rx_tk_mgmt_init(struct mtk_hw_dev *dev)
{
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	int ret;

	ret = mtk_idrm_init(&tk->base, "rx_tkinfo", dev->sys_idx.dir,
		res_cap->rx_token.start, res_cap->rx_token.end);

	if (ret)
		goto err;

	tk->pdev = dev->dev;
	tk->ops = &rx_tk_ops;

	return 0;
err:
	return ret;
}

static void
mtk_rx_tk_mgmt_exit(struct mtk_hw_dev *dev)
{
	struct mtk_rx_tk_mgmt *tk = &dev->hw_ctrl.rx_tk_mgmt;

	mtk_idrm_exit(&tk->base);
}

int
mtk_tk_init(struct mtk_hw_dev *dev)
{
	int ret;

	ret = mtk_tx_tk_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Tx token init failed, ret = %d\n",
			__func__, ret);
		goto err_tx_tk_init;
	}

	ret = mtk_tx_tk_rsv_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Tx reserved token init failed, ret = %d\n",
			__func__, ret);
		goto err_tx_tk_rsv_init;
	}

	ret = mtk_tx_tk_bmc_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Tx BMC token init failed, ret = %d\n",
			__func__, ret);
		goto err_tx_tk_bmc_init;
	}

	ret = mtk_rx_tk_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Rx token init failed, ret = %d\n",
			__func__, ret);
		goto err_rx_tk_init;
	}

	return 0;
err_rx_tk_init:
	mtk_tx_tk_bmc_exit(dev);
err_tx_tk_bmc_init:
	mtk_tx_tk_rsv_exit(dev);
err_tx_tk_rsv_init:
	mtk_tx_tk_exit(dev);
err_tx_tk_init:
	return ret;
}

void
mtk_tk_exit(struct mtk_hw_dev *dev)
{
	mtk_tx_tk_exit(dev);
	mtk_tx_tk_bmc_exit(dev);
	mtk_rx_tk_exit(dev);
}

/*
 *
 */
int
mtk_tk_mgmt_init(struct mtk_hw_dev *dev)
{
	int ret;

	ret = mtk_tx_tk_mgmt_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Tx token mgmt init failed, ret = %d\n",
			__func__, ret);
		goto err_tx_tk_init;
	}

	ret = mtk_tx_tk_bmc_mgmt_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Tx token BMC mgmt init failed, ret = %d\n",
			__func__, ret);
		goto err_tx_tk_bmc_init;
	}

	ret = mtk_rx_tk_mgmt_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Rx token mgmt init failed, ret = %d\n",
			__func__, ret);
		goto err_rx_tk_init;
	}

	ret = mtk_tk_mgmt_debugfs_register(dev);
	if (ret) {
		dev_err(dev->dev, "%s(): Token mgmt debugfs register failed, ret = %d\n",
			__func__, ret);
		goto err_tk_mgmt_reg;
	}

	return 0;
err_tk_mgmt_reg:
	mtk_rx_tk_mgmt_exit(dev);
err_rx_tk_init:
	mtk_tx_tk_bmc_mgmt_exit(dev);
err_tx_tk_bmc_init:
	mtk_tx_tk_mgmt_exit(dev);
err_tx_tk_init:
	return ret;
}

/*
 *
 */
void
mtk_tk_mgmt_exit(struct mtk_hw_dev *dev)
{
	mtk_tx_tk_mgmt_exit(dev);
	mtk_tx_tk_bmc_mgmt_exit(dev);
	mtk_rx_tk_mgmt_exit(dev);
}
