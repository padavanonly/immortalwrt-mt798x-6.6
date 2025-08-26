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
#include "main.h"
#include "core.h"
#include "wsys.h"

static char *sta_type[] = {
	"normal", "group", "mld"
};

static void
mtk_wsys_trans_dump(struct mtk_bus_trans *trans, struct seq_file *s)
{

}

static void
mtk_wsys_chip_dump(struct mtk_chip_drv *drv, struct seq_file *s)
{

}

static void
mtk_wsys_mcu_dump(struct mtk_mcu_ctrl *mcu, struct seq_file *s)
{

}

static void
mtk_wsys_hwctrl_dump(struct mtk_hw_ctrl *hw_ctrl, struct seq_file *s)
{

}

static void
mtk_wsys_dev_dump(struct mtk_hw_dev *dev, struct seq_file *s)
{
	seq_printf(s, "==========hw_dev %d==========\n", dev->sys_idx.idx);
	seq_printf(s, "flags\t\t: %lu\n", dev->flags);
	seq_printf(s, "state\t\t: %lu\n", dev->state);
	mtk_wsys_trans_dump(dev->bus_trans, s);
	mtk_wsys_chip_dump(dev->chip_drv, s);
	mtk_wsys_mcu_dump(&dev->mcu_ctrl, s);
	mtk_wsys_hwctrl_dump(&dev->hw_ctrl, s);
}

static void
mtk_wsys_phy_dump(struct mtk_hw_phy *phy, struct seq_file *s)
{
	seq_printf(s, "==========hw_phy %d==========\n", phy->sys_idx.idx);
	seq_printf(s, "band_idx\t: %d\n", phy->band_idx);
	seq_printf(s, "wmm_ap_idx\t: %d\n", phy->wmm_ap);
	seq_printf(s, "wmm_sta_idx\t: %d\n", phy->wmm_sta);
	seq_printf(s, "hw\t\t: %p\n", phy->mac_hw);
	mtk_wsys_dev_dump(phy->hw_dev, s);
}

static void
mtk_wsys_bss_dump(struct mtk_hw_bss *bss, struct seq_file *s)
{
	seq_printf(s, "==========hw_bss %d==========\n", bss->sys_idx.idx);
	seq_printf(s, "fw_idx\t\t: %d\n", bss->fw_idx);
	seq_printf(s, "op mode\t\t: %d\n", bss->type);
	seq_printf(s, "wmm idx\t\t: %d\n", bss->wmm_idx);
	seq_printf(s, "omac idx\t: 0x%x\n", bss->omac_idx);
	seq_printf(s, "group wtbl\t: %d\n", bss->group->link_wcid);
	seq_printf(s, "mld addr idx\t: %d\n", bss->mld_addr_idx);
	seq_printf(s, "mld group idx\t: %d\n", bss->mld_cfg.mld_group_idx);
	seq_printf(s, "if addr\t\t: %pM\n", bss->if_addr);
	mtk_wsys_phy_dump(bss->hw_phy, s);
}

static void
mtk_wsys_sta_dump(struct mtk_hw_sta *sta, struct seq_file *s)
{
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;

	mld_sta_entry = mtk_wsys_mld_sta_entry_get(sta->mld_sta_idx);

	seq_printf(s, "==========hw_sta %d==========\n", sta->sys_idx.idx);
	seq_printf(s, "link wcid\t: %d\n", sta->link_wcid);
	seq_printf(s, "sta type\t: %s\n", sta_type[sta->sta_type]);
	seq_printf(s, "mld_sta_idx\t: %d\n", sta->mld_sta_idx);
	seq_printf(s, "mld_sta_entry\t: %p\n", mld_sta_entry);
	seq_printf(s, "tx_info\t\t: %x\n", sta->tx_info);
	mtk_wsys_bss_dump(sta->bss, s);
}

static int
mtk_wsys_bss_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_hw_bss *bss = (struct mtk_hw_bss *)sys_idx->data;

	mtk_wsys_bss_dump(bss, s);
	return 0;
}

static int
mtk_wsys_sta_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_hw_sta *sta = (struct mtk_hw_sta *)sys_idx->data;

	mtk_wsys_sta_dump(sta, s);
	return 0;
}

static int
mtk_wsys_mld_bss_dump(struct mtk_mld_bss_entry *mld_bss_entry, struct seq_file *s)
{
	int i;

	seq_printf(s, "==========mld_group_idx %d==========\n",
		mld_bss_entry->mld_group_idx.idx);
	seq_printf(s, "mld_addr\t: %pM\n", mld_bss_entry->mld_addr);
	seq_printf(s, "mld_type\t: %d\n", mld_bss_entry->mld_type);
	seq_printf(s, "remap_id\t: %d\n", mld_bss_entry->remap_id);
	seq_printf(s, "mat_idx\t\t: %d\n", mld_bss_entry->mat_idx);
	seq_printf(s, "ref_cnt\t\t: %d\n", mld_bss_entry->ref_cnt);

	for (i = 0 ; i < MLD_LINK_MAX; i++) {
		struct mtk_hw_bss *bss = mld_bss_entry->mld_bss[i];

		if (bss)
			mtk_wsys_bss_dump(bss, s);
	}
	return 0;
}

static int
mtk_wsys_mld_bss_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_mld_bss_entry *mld_bss_entry =
		(struct mtk_mld_bss_entry *)sys_idx->data;

	mtk_wsys_mld_bss_dump(mld_bss_entry, s);
	return 0;
}

static int
mtk_wsys_mld_sta_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;
	struct mtk_hw_sta *sta;
	int i;

	mt_rcu_read_lock();
	mld_sta_entry = (struct mtk_mld_sta_entry *)rcu_dereference(sys_idx->data);
	if (!mld_sta_entry) {
		mt_rcu_read_unlock();
		return 0;
	}
	seq_printf(s, "==========mld sta idx %d==========\n", mld_sta_entry->sys_idx->idx);
	seq_printf(s, "ref_cnt\t\t: %d\n", mld_sta_entry->ref_cnt);
	seq_printf(s, "primary\t\t: %d\n", mld_sta_entry->primary->link_wcid);
	seq_printf(s, "secondary\t: %d\n", mld_sta_entry->secondary->link_wcid);

	for (i = 0 ; i < MLD_LINK_MAX; i++) {
		sta = mld_sta_entry->link_sta[i];
		if (sta)
			mtk_wsys_sta_dump(sta, s);
	}

	seq_puts(s, "rx_pkt_cnt:\n");

	for (i = 0 ; i < MLD_LINK_MAX; i++)
		seq_printf(s, "link%d: %d\n", i, mld_sta_entry->rx_pkt_cnt[i]);
	mt_rcu_read_unlock();

	return 0;
}

static int
mtk_wsys_phy_debugfs(struct seq_file *s, void *data)
{
	struct mtk_idr_entry *sys_idx = (struct mtk_idr_entry *)s->private;
	struct mtk_hw_phy *phy = (struct mtk_hw_phy *)sys_idx->data;

	mtk_wsys_phy_dump(phy, s);
	return 0;
}

static int
mtk_wsys_phy_add(struct seq_file *s, void *data)
{
	struct mtk_bus_trans *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *hw_dev = to_hw_dev(trans);
	struct mtk_hw_phy *hw_phy = NULL;
	int ret;

	ret = mtk_interface_add_phy(hw_dev, &hw_phy);
	if (ret)
		goto err;

	seq_printf(s, "allocate success, new phy %p, idx %d", hw_phy, hw_phy->sys_idx.idx);
	return 0;
err:
	return -ENOMEM;
}

struct mtk_hw_bss *
mtk_wsys_bss_get(struct mtk_hw_dev *dev, u8 bss_idx)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;

	return (struct mtk_hw_bss *)mtk_idr_entry_find(&hw->bss_mgmt, bss_idx);
}

u32
mtk_wsys_free_uwtbl_num_get(struct mtk_hw_dev *dev)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	struct mtk_idr_mgmt *idrm = &hw->uwtbl_mgmt;

	return mtk_idr_free_pool_count(idrm);
}

static int
mtk_wsys_register_device_debugfs(struct mtk_idr_mgmt *idrm, struct mtk_hw_dev *dev)
{
	struct mtk_idr_entry *idr_entry = &dev->sys_idx;
	struct dentry *dir;
	char name[32];
	int ret;

	ret = snprintf(name, sizeof(name), "dev%d", dev->sys_idx.idx);
	if (ret < 0) {
		dev_err(dev->dev, "%s(): name parse error\n", __func__);
		name[0] = '\0';
	}
	dir = mt_debugfs_create_dir(name, idrm->debugfs_dir);
	if (!dir)
		goto err;
	idr_entry->dir = dir;

	mt_debugfs_create_devm_seqfile(dev->dev, "add_phy", dir, mtk_wsys_phy_add);

	/*bus debug*/
	if (dev->bus_trans->dma_ops->queues_read) {
		mt_debugfs_create_devm_seqfile(dev->dev, "queues", dir,
		mtk_bus_queues_read);
	}
	return 0;
err:
	return -EINVAL;
}

static int
mtk_wsys_mld_bss_init(struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt)
{
	struct mtk_mld_bss_entry *entry;
	int i, j;
	int ret;

	for (i = 0 ; i < WSYS_MLD_BSS_MAX; i++) {
		entry = &mld_bss_mgmt->entries[i];
		mutex_init(&entry->mutex);
		entry->ref_cnt = 0;
		entry->remap_id = 0;
		for (j = 0 ; j < MLD_LINK_MAX; j++)
			entry->mld_bss[j] = NULL;
		ret = mtk_idr_entry_register(&mld_bss_mgmt->base, &entry->mld_group_idx, entry);
		if (ret)
			goto err;
	}
	return 0;
err:
	return -ENOMEM;
}

static int
mtk_wsys_mld_bss_exit(struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt)
{
	struct mtk_mld_bss_entry *entry;
	int i, j;

	for (i = 0 ; i < WSYS_MLD_BSS_MAX; i++) {
		entry = &mld_bss_mgmt->entries[i];
		entry->ref_cnt = 0;
		entry->remap_id = 0;
		for (j = 0 ; j < MLD_LINK_MAX; j++)
			entry->mld_bss[j] = NULL;
		mutex_init(&entry->mutex);
		mtk_idr_entry_unregister(&mld_bss_mgmt->base, &entry->mld_group_idx);
	}
	return 0;
}

static int
mtk_wsys_mld_sta_init(struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt)
{
	struct mtk_idr_entry *sys_idx = NULL;
	struct mutex *lock = NULL;
	int i = 0;
	int ret = 0;

	for (i = 0 ; i < WSYS_MLD_STA_MAX; i++) {
		lock = &mld_sta_mgmt->ctrl[i].lock;
		mutex_init(lock);
		sys_idx = &mld_sta_mgmt->ctrl[i].sys_idx;
		ret = mtk_idr_entry_register(&mld_sta_mgmt->base, sys_idx, NULL);
		if (ret)
			goto err;
	}

	return 0;
err:
	return -ENOMEM;
}

static int
mtk_wsys_mld_sta_exit(struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt)
{
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_sta_mgmt->base;
	struct mtk_mld_sta_entry *entry = NULL;
	struct mtk_idr_entry *idr_entry = NULL;
	struct mutex *lock;
	int i;

	for (i = 0 ; i < WSYS_MLD_STA_MAX; i++) {
		entry = mtk_idr_entry_find(idrm, i);
		idr_entry = &mld_sta_mgmt->ctrl[i].sys_idx;
		mtk_idr_entry_unregister(&mld_sta_mgmt->base, idr_entry);
		if (entry) {
			mt_synchronize_rcu();
			kfree(entry);
		}

		lock = &mld_sta_mgmt->ctrl[i].lock;
		mutex_destroy(lock);
	}

	return 0;
}

/*
 *
 */
int
mtk_wsys_register_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt = &wsys->mld_bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_bss_mgmt->base;
	struct mtk_mld_bss_entry *entry = mtk_idr_entry_find(idrm, bss->mld_cfg.mld_group_idx);
	int ret = -EINVAL;

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != bss->mld_cfg.mld_group_idx)
		goto err;

	/* MLD Create */
	ret = mtk_wsys_register_mld(dev, &bss->mld_cfg);
	if (ret)
		goto err;

	/* MLD Add Link */
	ret = mtk_wsys_mld_register_bss(dev, bss, bss->mld_cfg.mld_group_idx);
	if (ret)
		goto err;

	return 0;
err:
	dev_err(dev->dev, "%s(): register mld bss err %d\n", __func__, ret);
	return ret;
}

/*
 *
 */
int
mtk_wsys_unregister_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	struct mtk_mld_bss_entry *entry = bss->mld_bss_entry;
	int ret = -EINVAL;

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != bss->mld_cfg.mld_group_idx)
		goto err;

	/* MLD Del Link */
	ret = mtk_wsys_mld_unregister_bss(dev, bss);
	if (ret)
		goto err;

	/* MLD Destroy */
	ret = mtk_wsys_unregister_mld(dev, bss->mld_cfg.mld_group_idx);
	if (ret)
		goto err;
	return 0;

err:
	dev_err(dev->dev, "%s(): unregister mld bss err %d\n", __func__, ret);
	return ret;
}

int
mtk_wsys_update_mld_bss_entry(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	struct mtk_mld_bss_entry *entry = bss->mld_bss_entry;
	u32 band_idx = hw_bss_get_band_idx(bss);

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != bss->mld_cfg.mld_group_idx)
		goto err;

	mutex_lock(&entry->mutex);

	/*remove original specific link bss*/
	if (entry->mld_bss[band_idx]) {
		entry->ref_cnt--;
		entry->mld_bss[band_idx] = NULL;
	}

	entry->mld_bss[band_idx] = bss;
	entry->ref_cnt++;
	mutex_unlock(&entry->mutex);
	return 0;
err:
	return -EINVAL;
}


/*
 *
 */
int
mtk_wsys_register_mld(struct mtk_hw_dev *dev, struct mtk_bss_mld_cfg *mld_cfg)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt = &wsys->mld_bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_bss_mgmt->base;
	struct mtk_mld_bss_entry *entry = mtk_idr_entry_find(idrm, mld_cfg->mld_group_idx);
	int ret = -EINVAL;

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != mld_cfg->mld_group_idx)
		goto err;

	/*mld entry not used*/
	if (!entry->ref_cnt) {
		entry->mld_type = mld_cfg->mld_type;
		memcpy(entry->mld_addr, mld_cfg->mld_addr, MAC_ADDR_LEN);

		ret = mtk_hwctrl_om_remap_alloc(dev, entry);
		if (ret)
			goto err;

		if (entry->mld_type == MTK_MLD_TYPE_MULTI) {
			ret = mtk_hwctrl_mat_tb_alloc(dev, entry->mld_addr,
				MAT_ADDR_TYPE_MLD, &entry->mat_idx);
			if (ret)
				goto err;
		}

		ret = mtk_idr_entry_register_debugfs(idrm, &entry->mld_group_idx,
			"mld_bss", mtk_wsys_mld_bss_debugfs);
		if (ret)
			goto err;
	} else {
		dev_err(dev->dev, "%s(): MLD already existed\n", __func__);
		// goto err;
	}

	return 0;
err:
	dev_err(dev->dev, "%s(): register mld err %d\n", __func__, ret);
	return ret;
}

/*
 *
 */
int
mtk_wsys_unregister_mld(struct mtk_hw_dev *dev, u32 mld_group_idx)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt = &wsys->mld_bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_bss_mgmt->base;
	struct mtk_mld_bss_entry *entry = mtk_idr_entry_find(idrm, mld_group_idx);
	int ret = -EINVAL;

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != mld_group_idx)
		goto err;

	if (!entry->ref_cnt) {
		mt_debugfs_remove_recursive(entry->mld_group_idx.relate_dir);
		entry->mld_group_idx.relate_dir = NULL;
		if (entry->mld_type == MTK_MLD_TYPE_MULTI)
			mtk_hwctrl_mat_tb_free(dev, entry->mat_idx);
		mtk_hwctrl_om_remap_free(dev, entry);
	} else {
		dev_err(dev->dev, "%s(): MLD not empty\n", __func__);
		// goto err;
	}

	return 0;
err:
	dev_err(dev->dev, "%s(): unregister mld err %d\n", __func__, ret);
	return ret;
}

/*
 *
 */
int
mtk_wsys_mld_register_bss(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss, u32 mld_group_idx)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_bss_mgmt *mld_bss_mgmt = &wsys->mld_bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_bss_mgmt->base;
	struct mtk_mld_bss_entry *entry = mtk_idr_entry_find(idrm, mld_group_idx);
	u32 band_idx = hw_bss_get_band_idx(bss);
	int ret = -EINVAL;

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != mld_group_idx)
		goto err;

	if (entry->mld_type == MTK_MLD_TYPE_SINGLE) {
		if (!memcmp(entry->mld_addr, bss->if_addr, MAC_ADDR_LEN))
			entry->mat_idx = bss->mld_addr_idx;
		else
			goto err;
	}

	mutex_lock(&entry->mutex);
	if (!entry->mld_bss[band_idx]) {
		entry->mld_bss[band_idx] = bss;
		entry->ref_cnt++;
	}
	bss->mld_cfg.mld_group_idx = mld_group_idx;
	bss->mld_bss_entry = entry;
	mutex_unlock(&entry->mutex);

	return 0;
err:
	dev_err(dev->dev, "%s(): register bss(%d) to mld(%d) err %d\n",
		__func__, bss->fw_idx, mld_group_idx, ret);
	return ret;
}

/*
 *
 */
int
mtk_wsys_mld_unregister_bss(struct mtk_hw_dev *dev, struct mtk_hw_bss *bss)
{
	struct mtk_mld_bss_entry *entry = bss->mld_bss_entry;
	u32 band_idx = hw_bss_get_band_idx(bss);

	if (!entry)
		goto err;

	if (entry->mld_group_idx.idx != bss->mld_cfg.mld_group_idx)
		goto err;

	if (entry->mld_bss[band_idx] == bss) {
		mutex_lock(&entry->mutex);
		entry->ref_cnt--;
		entry->mld_bss[band_idx] = NULL;
		bss->mld_bss_entry = NULL;
		mutex_unlock(&entry->mutex);
	}

	return 0;
err:
	dev_err(dev->dev, "%s(): unregister bss(%d) to mld(%d) err\n",
		__func__, bss->fw_idx, bss->mld_cfg.mld_group_idx);
	return -EINVAL;
}

int
mtk_wsys_update_mld_sta_mld_id(struct mtk_mld_sta_entry *entry, struct mtk_hw_sta *rm_sta)
{
	u8 i;
	struct mtk_hw_sta *avail_sta = NULL;

	if (!entry->ref_cnt)
		return 0;

	/* update pri/sec */
	if (rm_sta == entry->primary) {
		for (i = 0; i < MLD_LINK_MAX; i++) {
			avail_sta = entry->link_sta[i];
			if (avail_sta && avail_sta != rm_sta && avail_sta != entry->secondary)
				break;
			avail_sta = NULL;
		}
		if (avail_sta)
			entry->primary = avail_sta;
		else
			entry->primary = entry->secondary;

		pr_info("%s(): primary mld_id %d->%d\n", __func__,
			rm_sta->link_wcid, entry->primary->link_wcid);

	} else if (rm_sta == entry->secondary) {
		for (i = 0; i < MLD_LINK_MAX; i++) {
			avail_sta = entry->link_sta[i];
			if (avail_sta && avail_sta != rm_sta && avail_sta != entry->primary)
				break;
			avail_sta = NULL;
		}
		if (avail_sta)
			entry->secondary = avail_sta;
		else
			entry->secondary = entry->primary;

		pr_info("%s(): secondary mld_id %d->%d\n", __func__,
			rm_sta->link_wcid, entry->secondary->link_wcid);
	}

	return 0;
}

/*
 *
 */
struct mtk_idr_mgmt *
mtk_wsys_mld_sta_mgmt_idrm_get(void)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt = &wsys->mld_sta_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_sta_mgmt->base;

	return idrm;
}

/*
 *
 */
struct mtk_mld_sta_entry *
mtk_wsys_mld_sta_entry_get(u32 id)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt = &wsys->mld_sta_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &mld_sta_mgmt->base;

	if (idrm)
		return mtk_idr_entry_find(idrm, id);
	else
		return NULL;
}

/*
 *
 */
struct mtk_idr_entry *
mtk_wsys_mld_sta_entry_idr_entry_get(u32 id)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt = &wsys->mld_sta_mgmt;

	if (mld_sta_mgmt)
		return &mld_sta_mgmt->ctrl[id].sys_idx;
	else
		return NULL;
}

/*
 *
 */
int
mtk_wsys_mld_sta_entry_mld_id_get(struct mtk_hw_sta *hw_sta, u32 *pri_id, u32 *sec_id)
{
	int ret = -EINVAL;
	struct mtk_mld_sta_entry *mld_sta_entry = NULL;

	mt_rcu_read_lock();
	mld_sta_entry = mtk_wsys_mld_sta_entry_get(hw_sta->mld_sta_idx);
	if (!mld_sta_entry) {
		mt_rcu_read_unlock();
		return ret;
	}
	*pri_id = mld_sta_entry->primary->link_wcid;
	*sec_id = mld_sta_entry->secondary->link_wcid;
	mt_rcu_read_unlock();

	return 0;
}

/*
 *
 */
int
mtk_wsys_mld_sta_entry_alloc(struct mtk_hw_dev *dev, u32 mld_sta_idx,
	struct mtk_mld_sta_entry **new_entry)
{
	int ret = 0;
	struct mtk_mld_sta_entry *old_entry = NULL;
	struct mtk_mld_sta_entry *entry = NULL;

	old_entry = mtk_wsys_mld_sta_entry_get(mld_sta_idx);
	entry = kzalloc(sizeof(struct mtk_mld_sta_entry), GFP_KERNEL);
	if (!entry) {
		ret = -ENOMEM;
		goto err1;
	}

	if (!old_entry)
		entry->sys_idx = mtk_wsys_mld_sta_entry_idr_entry_get(mld_sta_idx);
	else
		memcpy(entry, old_entry, sizeof(struct mtk_mld_sta_entry));

	if (entry->sys_idx->idx != mld_sta_idx) {
		ret = -ENXIO;
		goto err2;
	}

	*new_entry = entry;

	return ret;
err2:
	kfree(entry);
err1:
	dev_err(dev->dev, "%s(): register mld sta err %d\n", __func__, ret);

	return ret;
}

/*
 *
 */
int
mtk_wsys_mld_sta_entry_free(struct mtk_mld_sta_entry *entry)
{
	int ret = 0;

	mt_synchronize_rcu();
	kfree(entry);

	return ret;
}

/*
 *
 */
struct mutex *
mtk_wsys_mld_sta_entry_lock(u32 id)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_mld_sta_mgmt *mld_sta_mgmt = &wsys->mld_sta_mgmt;

	if (mld_sta_mgmt)
		return &mld_sta_mgmt->ctrl[id].lock;
	else
		return NULL;
}

/*
 *
 */
int
mtk_wsys_mld_sta_entry_change_setup_link(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	int ret = 0;
	u32 wcid = sta->link_wcid;
	u32 mld_sta_idx = sta->mld_sta_idx;
	u32 band_idx = hw_sta_get_band_idx(sta);
	struct mtk_idr_mgmt *idrm = mtk_wsys_mld_sta_mgmt_idrm_get();
	struct mutex *lock = mtk_wsys_mld_sta_entry_lock(mld_sta_idx);
	struct mtk_mld_sta_entry *old_entry = NULL;
	struct mtk_mld_sta_entry *new_entry = NULL;

	if (!lock)
		goto err1;

	if (band_idx >= MAX_BAND_NUM)
		goto err1;

	if (!sta->bss || !sta->bss->mld_bss_entry) {
		ret = -ENODEV;
		goto err1;
	}

	mutex_lock(lock);

	ret = mtk_wsys_mld_sta_entry_alloc(dev, mld_sta_idx, &new_entry);
	if (ret) {
		mutex_unlock(lock);
		goto err1;
	}

	new_entry->setup_wcid = wcid;
	new_entry->setup_band = band_idx;
	rcu_assign_pointer(new_entry->sys_idx->data, new_entry);

	ret = mtk_idr_replace(idrm, mld_sta_idx, new_entry, (void **)&old_entry);
	if (ret) {
		mutex_unlock(lock);
		goto err2;
	}

	mutex_unlock(lock);

	mtk_wsys_mld_sta_entry_free(old_entry);

	return ret;

err2:
	rcu_assign_pointer(new_entry->sys_idx->data, NULL);
	kfree(new_entry);
err1:
	dev_err(dev->dev, "%s(): change setup link sta err %d\n", __func__, ret);

	return ret;
}

/*
 *
 */
int
mtk_wsys_register_mld_sta_entry(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	int ret = 0;
	u32 wcid = 0;
	u32 mld_sta_idx = sta->mld_sta_idx;
	u32 band_idx = hw_sta_get_band_idx(sta);
	struct mtk_idr_mgmt *idrm = mtk_wsys_mld_sta_mgmt_idrm_get();
	struct mutex *lock = mtk_wsys_mld_sta_entry_lock(mld_sta_idx);
	struct mtk_mld_sta_entry *old_entry = NULL;
	struct mtk_mld_sta_entry *new_entry = NULL;
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;

	if (!lock) {
		ret = -EINVAL;
		goto err1;
	}

	if (band_idx >= MAX_BAND_NUM) {
		ret = -EINVAL;
		goto err1;
	}

	if (!sta->bss || !sta->bss->mld_bss_entry) {
		ret = -ENODEV;
		goto err1;
	}

	mutex_lock(lock);

	ret = mtk_wsys_mld_sta_entry_alloc(dev, mld_sta_idx, &new_entry);
	if (ret) {
		mutex_unlock(lock);
		goto err1;
	}

	ret = mtk_idr_register(&hw->uwtbl_mgmt, &sta->link_wcid, sta);
	if (ret) {
		mutex_unlock(lock);
		goto err2;
	}

	wcid = sta->link_wcid;

	/* first connection is primary */
	if (!new_entry->ref_cnt) {
		ret = mtk_idr_entry_register_debugfs(idrm,
			new_entry->sys_idx, "mld_sta", mtk_wsys_mld_sta_debugfs);
		if (ret) {
			mutex_unlock(lock);
			goto err3;
		}

		new_entry->links[band_idx] = wcid;
		new_entry->mld_bss = sta->bss->mld_bss_entry;
		new_entry->primary = sta;
		new_entry->secondary = sta;
		new_entry->setup_wcid = wcid;
		new_entry->setup_band = band_idx;
	}

	/* second connection is secondary */
	if (new_entry->ref_cnt == 1)
		new_entry->secondary = sta;
	new_entry->link_sta[band_idx] = sta;
	new_entry->ref_cnt++;
	rcu_assign_pointer(new_entry->sys_idx->data, new_entry);

	ret = mtk_idr_replace(idrm,
		mld_sta_idx, new_entry, (void **)&old_entry);
	if (ret) {
		new_entry->ref_cnt--;
		new_entry->link_sta[band_idx] = NULL;
		mutex_unlock(lock);
		goto err4;
	}

	dev_info(dev->dev, "%s(): mld_sta_idx=%d, band=%d, cnt=%d, wcid=%d, hw_sta=%p\n",
		__func__, mld_sta_idx, band_idx, new_entry->ref_cnt, wcid, sta);

	mutex_unlock(lock);

	mtk_wsys_mld_sta_entry_free(old_entry);

	return ret;

err4:
	rcu_assign_pointer(new_entry->sys_idx->data, NULL);
	if (new_entry->ref_cnt == 0)
		mtk_idr_entry_unregister_debugfs(idrm, new_entry->sys_idx);
err3:
	mtk_idr_unregister(&hw->uwtbl_mgmt, wcid);
err2:
	kfree(new_entry);
err1:
	dev_err(dev->dev, "%s(): register mld sta err %d\n", __func__, ret);

	return ret;
}

/*
 *
 */
int
mtk_wsys_unregister_mld_sta_entry(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	int ret = 0;
	u32 mld_sta_idx = sta->mld_sta_idx;
	u32 band_idx = hw_sta_get_band_idx(sta);
	struct mtk_idr_mgmt *idrm = mtk_wsys_mld_sta_mgmt_idrm_get();
	struct mutex *lock = mtk_wsys_mld_sta_entry_lock(mld_sta_idx);
	struct mtk_mld_sta_entry *old_entry = NULL;
	struct mtk_mld_sta_entry *new_entry = NULL;
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;

	if (!lock) {
		ret = -EINVAL;
		goto err1;
	}

	if (band_idx >= MAX_BAND_NUM) {
		ret = -EINVAL;
		goto err1;
	}

	mutex_lock(lock);

	old_entry = mtk_wsys_mld_sta_entry_get(mld_sta_idx);
	if (!old_entry) {
		mutex_unlock(lock);
		ret = -ENODEV;
		goto err1;
	}

	if (old_entry->link_sta[band_idx] != sta) {
		dev_err(dev->dev, "%s(): band=%d, expt=%p, real=%p\n",
			__func__, band_idx, old_entry->link_sta[band_idx], sta);
		mutex_unlock(lock);
		ret = -ENXIO;
		goto err1;
	}

	mtk_idr_unregister(&hw->uwtbl_mgmt, sta->link_wcid);

	if (old_entry->ref_cnt == 1) {
		rcu_assign_pointer(old_entry->sys_idx->data, NULL);
		mtk_idr_entry_unregister_debugfs(idrm, old_entry->sys_idx);
		ret = mtk_idr_replace(idrm, mld_sta_idx, NULL, (void **)&old_entry);
		if (ret) {
			mutex_unlock(lock);
			goto err1;
		}

		dev_info(dev->dev, "%s(): mld_sta_idx=%d, band=%d, cnt=%d, wcid=%d, hw_sta=%p\n",
			__func__, mld_sta_idx, band_idx, 0, sta->link_wcid, sta);
	} else {
		ret = mtk_wsys_mld_sta_entry_alloc(dev, mld_sta_idx, &new_entry);
		if (ret) {
			mutex_unlock(lock);
			goto err1;
		}

		new_entry->ref_cnt--;
		new_entry->link_sta[band_idx] = NULL;
		rcu_assign_pointer(new_entry->sys_idx->data, new_entry);
		if (new_entry->ref_cnt > 0)
			mtk_wsys_update_mld_sta_mld_id(new_entry, sta);
		ret = mtk_idr_replace(idrm, mld_sta_idx, new_entry, (void **)&old_entry);
		if (ret) {
			mutex_unlock(lock);
			goto err2;
		}

		dev_info(dev->dev, "%s(): mld_sta_idx=%d, band=%d, cnt=%d, wcid=%d, hw_sta=%p\n",
			__func__, mld_sta_idx, band_idx, new_entry->ref_cnt, sta->link_wcid, sta);
	}

	mutex_unlock(lock);

	mtk_wsys_mld_sta_entry_free(old_entry);

	return ret;

err2:
	rcu_assign_pointer(new_entry->sys_idx->data, NULL);
	kfree(new_entry);
err1:
	dev_err(dev->dev, "%s(): unregister mld sta err %d\n", __func__, ret);

	return ret;
}

/*
 *
 */
int
mtk_wsys_register_device(struct mtk_hw_dev *dev)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_dev_mgmt *dev_mgmt = &wsys->dev_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) dev_mgmt;
	int ret;

	ret = mtk_idr_entry_register(idrm, &dev->sys_idx, dev);
	if (ret)
		goto err_idr_reg;

	ret = mtk_wsys_register_device_debugfs(idrm, dev);
	if (ret)
		goto err_dbgfs_reg;

	return 0;
err_dbgfs_reg:
	mtk_idr_entry_unregister(idrm, &dev->sys_idx);
err_idr_reg:
	return ret;
}

/*
 *
 */
void
mtk_wsys_unregister_device(struct mtk_hw_dev *dev)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_dev_mgmt *dev_mgmt = &wsys->dev_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) dev_mgmt;

	mtk_idr_entry_unregister(idrm, &dev->sys_idx);
}

/*
 *
 */
int
mtk_wsys_register_bss(struct mtk_hw_bss *hw_bss)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_bss_mgmt *bss_mgmt = &wsys->bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) bss_mgmt;
	int ret;

	ret = mtk_idr_entry_register(idrm, &hw_bss->sys_idx, hw_bss);
	if (ret)
		goto err;

	return mtk_idr_entry_register_debugfs(idrm, &hw_bss->sys_idx, "bss", mtk_wsys_bss_debugfs);
err:
	return -ENOMEM;
}

/*
 *
 */
void
mtk_wsys_unregister_bss(struct mtk_hw_bss *hw_bss)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_bss_mgmt *bss_mgmt = &wsys->bss_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) bss_mgmt;

	mtk_idr_entry_unregister(idrm, &hw_bss->sys_idx);
}

/*
 *
 */
int
mtk_wsys_register_sta(struct mtk_hw_sta *hw_sta)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_sta_mgmt *sta_mgmt = &wsys->sta_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) sta_mgmt;
	int ret;

	ret = mtk_idr_entry_register(idrm, &hw_sta->sys_idx, hw_sta);
	if (ret)
		goto err;

	return mtk_idr_entry_register_debugfs(idrm, &hw_sta->sys_idx, "sta", mtk_wsys_sta_debugfs);
err:
	return -ENOMEM;
}

/*
 *
 */
void
mtk_wsys_unregister_sta(struct mtk_hw_sta *hw_sta)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_sta_mgmt *sta_mgmt = &wsys->sta_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) sta_mgmt;

	mtk_idr_entry_unregister(idrm, &hw_sta->sys_idx);
}

/*
 *
 */
int
mtk_wsys_register_phy(struct mtk_hw_phy *hw_phy)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_phy_mgmt *phy_mgmt = &wsys->phy_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) phy_mgmt;
	struct mtk_hw_ctrl *hw_ctrl = &hw_phy->hw_dev->hw_ctrl;
	int ret;

	ret = mtk_idr_entry_register(idrm, &hw_phy->sys_idx, hw_phy);
	if (ret)
		goto err;

	ret = mtk_idr_entry_register_debugfs_with_parent(idrm,
		&hw_phy->sys_idx, "phy", hw_ctrl->phy_mgmt.mgmt.debugfs_dir,
		mtk_wsys_phy_debugfs);

	return mtk_idr_entry_register_debugfs(idrm, &hw_phy->sys_idx, "phy", mtk_wsys_phy_debugfs);
err:
	return -ENOMEM;
}

/*
 *
 */
void
mtk_wsys_unregister_phy(struct mtk_hw_phy *hw_phy)
{
	struct mtk_wsys_mgmt *wsys = mtk_hwifi_get_wsys_mgmt();
	struct mtk_wsys_phy_mgmt *phy_mgmt = &wsys->phy_mgmt;
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) phy_mgmt;

	mtk_idr_entry_unregister(idrm, &hw_phy->sys_idx);
}

/**
 * @brief Initial hw wifi system sub module
 *  prepare wsys_mgmt
 *
 * @param *drv global hwifi driver
 * @param *wsys_mgmt
 * @retval 0 init success
 * @retval -ENOMEM alloc resouce fail
 */
int
mtk_wsys_init(struct mtk_hwifi_drv *drv, struct mtk_wsys_mgmt *wsys)
{
	struct dentry *dir;
	int ret = 0;

	dir = mt_debugfs_create_dir("wsys", drv->debugfs_dir);
	if (!dir)
		goto err;

	wsys->debugfs_dir = dir;

	/*max card this driver support*/
	mtk_idrm_init(&wsys->dev_mgmt.base, "hw_dev", dir, 0, WSYS_CAR_MAX);
	/*max bss this driver can support*/
	mtk_idrm_init(&wsys->bss_mgmt.base, "hw_bss", dir, 0, WSYS_BSS_MAX);
	/*max sta this driver can support*/
	mtk_idrm_init(&wsys->sta_mgmt.base, "hw_sta", dir, 0, WSYS_STA_MAX);
	/*max radio band this driver can support*/
	mtk_idrm_init(&wsys->phy_mgmt.base, "hw_phy", dir, 0, WSYS_PHY_MAX);
	/*max mld bss group this driver can support*/
	mtk_idrm_init(&wsys->mld_bss_mgmt.base, "mld_bss", dir, 0, WSYS_MLD_BSS_MAX);
	mtk_wsys_mld_bss_init(&wsys->mld_bss_mgmt);
	/*max mld sta this driver can support*/
	mtk_idrm_init(&wsys->mld_sta_mgmt.base, "mld_sta", dir, 0, WSYS_MLD_STA_MAX);
	ret = mtk_wsys_mld_sta_init(&wsys->mld_sta_mgmt);
	if (ret)
		goto err;

	return 0;
err:
	mt_debugfs_remove_recursive(wsys->debugfs_dir);
	return -ENOMEM;
}

/**
 * @brief Exit hw wifi system sub module
 *  exit wsys_mgmt
 *
 * @param *wsys_mgmt
 */
void
mtk_wsys_exit(struct mtk_wsys_mgmt *wsys)
{
	mtk_wsys_mld_sta_exit(&wsys->mld_sta_mgmt);
	mtk_idrm_exit(&wsys->mld_sta_mgmt.base);
	mtk_wsys_mld_bss_exit(&wsys->mld_bss_mgmt);
	mtk_idrm_exit(&wsys->mld_bss_mgmt.base);
	mtk_idrm_exit(&wsys->dev_mgmt.base);
	mtk_idrm_exit(&wsys->bss_mgmt.base);
	mtk_idrm_exit(&wsys->sta_mgmt.base);
	mtk_idrm_exit(&wsys->phy_mgmt.base);
	mt_debugfs_remove_recursive(wsys->debugfs_dir);
}

/*
 *
 */
int
mtk_wsys_dev_debugfs_plugin(char *name, struct mtk_dbg_ops *ops)
{
	struct mtk_wsys_mgmt *wsys_mgmt = mtk_hwifi_get_wsys_mgmt();
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &wsys_mgmt->dev_mgmt;
	struct mtk_hw_dev *dev;
	u32 id;
	int ret = 0;

	mtk_idr_for_each_entry(idrm, dev, id) {
		if (!strcmp(dev->bus_trans->name, name)) {
			dev->dbg_ops = ops;
			if (ops->dbgfs_init)
				ops->dbgfs_init(dev);
			if (ops->dbgfs_alloc)
				ret = ops->dbgfs_alloc(dev);
		}
	}
	return ret;
}
EXPORT_SYMBOL(mtk_wsys_dev_debugfs_plugin);


/*
 *
 */
void
mtk_wsys_dev_debugfs_unplug(char *name)
{
	struct mtk_wsys_mgmt *wsys_mgmt = mtk_hwifi_get_wsys_mgmt();
	struct mtk_idr_mgmt *idrm = (struct mtk_idr_mgmt *) &wsys_mgmt->dev_mgmt;
	struct mtk_hw_dev *dev;
	u32 id;

	mtk_idr_for_each_entry(idrm, dev, id) {
		if (!strcmp(dev->bus_trans->name, name)) {
			if (dev->dbg_ops) {
				if (dev->dbg_ops->dbgfs_exit)
					dev->dbg_ops->dbgfs_exit(dev);
				if (dev->dbg_ops->dbgfs_free)
					dev->dbg_ops->dbgfs_free(dev);
				dev->dbg_ops = NULL;
			}
		}
	}
}
EXPORT_SYMBOL(mtk_wsys_dev_debugfs_unplug);
