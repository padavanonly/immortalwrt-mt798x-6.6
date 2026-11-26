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

#include <linux/etherdevice.h>
#include "hwres_mgmt.h"
#include "core.h"
#include "hw_ops.h"
#include "tk_mgmt.h"

static int
mtk_mat_tb_debugfs(struct seq_file *s, void *data)
{
	void *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	struct mtk_mat_tb_mgmt *mat_tb_mgmt = &dev->hw_ctrl.mat_tb_mgmt;
	struct mtk_mat_tb_entry *tmp_entry = NULL;
	u32 id;

	seq_puts(s, "===============mat tb===============\n");

	mtk_idr_for_each_entry(&mat_tb_mgmt->mld_addr_mgmt, tmp_entry, id) {
		seq_printf(s, "===== entry %d =====\n", id);
		if (tmp_entry) {
			seq_printf(s, "addr type\t: %d\n", tmp_entry->addr_type);
			seq_printf(s, "mac addr\t: %pM\n", tmp_entry->mac_addr);
			// seq_printf(s, "assigned to\t: %d\n", tmp_entry->mld_group_idx);
		}
	}

	mtk_idr_for_each_entry(&mat_tb_mgmt->link_addr_mgmt, tmp_entry, id) {
		seq_printf(s, "===== entry %d =====\n", id);
		if (tmp_entry) {
			seq_printf(s, "addr type\t: %d\n", tmp_entry->addr_type);
			seq_printf(s, "mac addr\t: %pM\n", tmp_entry->mac_addr);
			// seq_printf(s, "assigned to\t: %d\n", tmp_entry->hw_bss_idx);
		}
	}
	return 0;
}

static int
mtk_mat_tb_init(struct mtk_mat_tb_mgmt *mat_tb_mgmt, u32 tb_size)
{
	u32 mem_size = tb_size * sizeof(struct mtk_mat_tb_entry);

	pr_info("%s(): tb_size %d, mem_size %d\n", __func__, tb_size, mem_size);

	mat_tb_mgmt->entries = kzalloc(mem_size, GFP_KERNEL);

	if (!mat_tb_mgmt->entries)
		goto err;

	return 0;
err:
	return -ENOMEM;
}

static void
mtk_mat_tb_exit(struct mtk_mat_tb_mgmt *mat_tb_mgmt)
{
	kfree(mat_tb_mgmt->entries);
}

static int
mtk_mat_tb_mgmt_init(struct mtk_hw_dev *dev)
{
	struct mtk_mat_tb_mgmt *mat_tb_mgmt = &dev->hw_ctrl.mat_tb_mgmt;
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	u32 tb_size = res_cap->link_addr.end + 1;
	int ret;

	ret = mtk_mat_tb_init(mat_tb_mgmt, tb_size);

	if (ret)
		goto err;

	ret = mtk_idrm_init(&mat_tb_mgmt->mld_addr_mgmt, "mat_mld", dev->sys_idx.dir,
		res_cap->mld_addr.start,
		res_cap->mld_addr.end);

	if (ret)
		goto err;

	ret = mtk_idrm_init(&mat_tb_mgmt->link_addr_mgmt, "mat_non_mld", dev->sys_idx.dir,
		res_cap->link_addr.start,
		res_cap->link_addr.end);

	if (ret)
		goto err;

	mt_debugfs_create_devm_seqfile(dev->dev, "matinfo", dev->sys_idx.dir, mtk_mat_tb_debugfs);

	return 0;
err:
	return ret;
}

static void
mtk_mat_tb_mgmt_exit(struct mtk_hw_dev *dev)
{
	struct mtk_mat_tb_mgmt *mat_tb_mgmt = &dev->hw_ctrl.mat_tb_mgmt;

	mtk_idrm_exit(&mat_tb_mgmt->link_addr_mgmt);
	mtk_idrm_exit(&mat_tb_mgmt->mld_addr_mgmt);
	mtk_mat_tb_exit(mat_tb_mgmt);
}

static inline void *
mtk_mat_tb_find(struct mtk_mat_tb_mgmt *mat_tb_mgmt, u8 *mac_addr, u8 addr_type)
{
	struct mtk_mat_tb_entry *tmp_entry = NULL, *mat_tb_entry = NULL;
	struct mtk_idr_mgmt *idr_mgmt;
	u32 id;

	if (addr_type == MAT_ADDR_TYPE_MLD)
		idr_mgmt = &mat_tb_mgmt->mld_addr_mgmt;
	else
		idr_mgmt = &mat_tb_mgmt->link_addr_mgmt;

	mtk_idr_for_each_entry(idr_mgmt, tmp_entry, id) {
		if (tmp_entry && !memcmp(tmp_entry->mac_addr, mac_addr, MAC_ADDR_LEN)) {
			mat_tb_entry = tmp_entry;
			break;
		}
	}

	return mat_tb_entry;
}

int
mtk_hwctrl_mat_tb_alloc(struct mtk_hw_dev *dev, u8 *mac_addr, u8 addr_type, u32 *mat_idx)
{
	struct mtk_mat_tb_mgmt *mat_tb_mgmt = &dev->hw_ctrl.mat_tb_mgmt;
	struct mtk_mat_tb_entry *mat_tb_entry, *old_mat_tb_entry;
	struct mtk_idr_mgmt *idr_mgmt;
	u32 mat_tb_idx;
	int ret = -EINVAL;

	if (!mac_addr)
		goto err;

	switch (addr_type) {
	case MAT_ADDR_TYPE_MLD:
		idr_mgmt = &mat_tb_mgmt->mld_addr_mgmt;
		break;
	case MAT_ADDR_TYPE_NON_MLD:
		idr_mgmt = &mat_tb_mgmt->link_addr_mgmt;
		break;
	default:
		goto err;
	}

	if (mtk_mat_tb_find(mat_tb_mgmt, mac_addr, addr_type))
		goto err;
	else {
		/* acquire MAT table index */
		ret = mtk_idr_register(idr_mgmt, &mat_tb_idx, NULL);

		if (ret)
			goto err;

		/* update MAT table entry */
		mat_tb_entry = &mat_tb_mgmt->entries[mat_tb_idx];
		mat_tb_entry->mat_tb_idx = mat_tb_idx;
		mat_tb_entry->addr_type = addr_type;
		memcpy(mat_tb_entry->mac_addr, mac_addr, MAC_ADDR_LEN);

		ret = mtk_idr_replace(idr_mgmt, mat_tb_idx,
			mat_tb_entry, (void **)&old_mat_tb_entry);

		if (ret)
			goto err_replace_idr;

		*mat_idx = mat_tb_idx;
	}

	return 0;

err_replace_idr:
	mtk_idr_unregister(idr_mgmt, mat_tb_idx);
err:
	dev_err(dev->dev, "%s(): alloc mat tb idx err %d (addr_type %d)\n",
		__func__, ret, addr_type);
	return ret;
}

int
mtk_hwctrl_mat_tb_free(struct mtk_hw_dev *dev, u32 mat_idx)
{
	struct mtk_mat_tb_mgmt *mat_tb_mgmt = &dev->hw_ctrl.mat_tb_mgmt;
	struct mtk_mat_tb_entry *old_mat_tb_entry = NULL;
	struct mtk_idr_mgmt *idr_mgmt;
	int ret;

	if (mat_idx <= mat_tb_mgmt->mld_addr_mgmt.high)
		idr_mgmt = &mat_tb_mgmt->mld_addr_mgmt;
	else
		idr_mgmt = &mat_tb_mgmt->link_addr_mgmt;

	if (mtk_idr_entry_find(idr_mgmt, mat_idx)) {
		ret = mtk_idr_replace(idr_mgmt, mat_idx, NULL, (void **)&old_mat_tb_entry);

		if (ret)
			goto err;

		if (old_mat_tb_entry)
			memset(old_mat_tb_entry, 0, sizeof(struct mtk_mat_tb_entry));

		mtk_idr_unregister(idr_mgmt, mat_idx);
	}
	return 0;
err:
	return ret;
}

static struct mtk_idr_mgmt *
mtk_hwctrl_omac_mgmt_find(struct mtk_hw_bss *hw_bss)
{
	struct mtk_idr_mgmt *idr_mgmt = NULL;
	struct mtk_hw_phy *phy = hw_bss->hw_phy;

	switch (hw_bss->type) {
	case MTK_IF_TYPE_MONITOR:
	case MTK_IF_TYPE_AP:
	case MTK_IF_TYPE_MESH:
	case MTK_IF_TYPE_ADHOC:
		if ((mtk_idr_entry_find(&phy->omac_mgmt, 0) != NULL) ||
			(mtk_idr_entry_find(&phy->ext_omac_mgmt, hw_bss->omac_idx) != NULL))
			idr_mgmt = &phy->ext_omac_mgmt;
		else
			idr_mgmt = &phy->omac_mgmt;
		break;
	case MTK_IF_TYPE_STA:
		idr_mgmt = &phy->omac_mgmt;
		break;
	case MTK_IF_TYPE_REPT:
		idr_mgmt = &phy->rept_omac_mgmt;
		break;
	case MTK_IF_TYPE_WDS:
		idr_mgmt = &phy->omac_mgmt;
		break;
	default:
		dev_info(phy->hw_dev->dev, "%s(): err bss type!\n", __func__);
		break;
	}
	return idr_mgmt;
}

static int
mtk_hwctrl_omac_alloc(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	struct mtk_idr_mgmt *idr_mgmt = mtk_hwctrl_omac_mgmt_find(hw_bss);

	if (!idr_mgmt)
		goto err;

	/* by default, the AP uses OMAC #0 */
	if (hw_bss->type == MTK_IF_TYPE_WDS) {
		hw_bss->omac_idx = 0;
		return 0;
	} else {
		return mtk_idr_register(idr_mgmt, &hw_bss->omac_idx, hw_bss);
	}

err:
	return -EINVAL;
}

static int
mtk_hwctrl_omac_free(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	struct mtk_hw_phy *phy = hw_bss->hw_phy;
	struct mtk_idr_mgmt *idr_mgmt = hw_bss->omac_idx == 0 ?
		&phy->omac_mgmt : mtk_hwctrl_omac_mgmt_find(hw_bss);

	if (!idr_mgmt)
		goto err;

	mtk_idr_unregister(idr_mgmt, hw_bss->omac_idx);
	return 0;
err:
	return -EINVAL;
}

static int
mtk_hwctrl_wmm_get(struct mtk_hw_phy *phy, struct mtk_hw_bss *hw_bss)
{
	switch (hw_bss->type) {
	case MTK_IF_TYPE_MONITOR:
	case MTK_IF_TYPE_AP:
	case MTK_IF_TYPE_MESH:
	case MTK_IF_TYPE_ADHOC:
	case MTK_IF_TYPE_WDS:
	case MTK_IF_TYPE_REPT:
		hw_bss->wmm_idx = phy->wmm_ap;
		break;
	case MTK_IF_TYPE_STA:
		hw_bss->wmm_idx = phy->wmm_sta;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int
mtk_hwctrl_om_remap_alloc(struct mtk_hw_dev *dev, struct mtk_mld_bss_entry *mld_bss_entry)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	int ret = -EINVAL;

	switch (mld_bss_entry->mld_type) {
	case MTK_MLD_TYPE_MULTI:
		ret = mtk_idr_register(&hw->mld_remap_mgmt,
				&mld_bss_entry->remap_id, mld_bss_entry);
		if (ret)
			goto err;

		break;
	case MTK_MLD_TYPE_SINGLE:
		mld_bss_entry->remap_id = OM_REMAP_IDX_NONE;
		break;
	default:
		goto err;
	}

	return 0;
err:
	dev_err(dev->dev, "%s(): alloc agg remap idx err %d\n", __func__, ret);
	return ret;

}

int
mtk_hwctrl_om_remap_free(struct mtk_hw_dev *dev, struct mtk_mld_bss_entry *mld_bss_entry)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	int ret = -EINVAL;

	switch (mld_bss_entry->mld_type) {
	case MTK_MLD_TYPE_MULTI:
		if (mtk_idr_entry_find(&hw->mld_remap_mgmt, mld_bss_entry->remap_id))
			mtk_idr_unregister(&hw->mld_remap_mgmt, mld_bss_entry->remap_id);
		break;
	case MTK_MLD_TYPE_SINGLE:
		break;
	default:
		goto err;
	}

	return 0;
err:
	return ret;
}

static int
mtk_hwctrl_mld_bss_alloc(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	int ret;

	/*alloc an idx for mdp doing header translation*/
	ret = mtk_hwctrl_mat_tb_alloc(dev, hw_bss->if_addr,
		MAT_ADDR_TYPE_NON_MLD, &hw_bss->mld_addr_idx);
	if (ret)
		goto err;

	if (hw_bss->mld_cfg.mld_group_idx != MLD_GROUP_NONE) {
		if (hw_bss->type != MTK_IF_TYPE_AP) {
			ret = mtk_wsys_register_mld_bss_entry(dev, hw_bss);
			if (ret)
				goto err;
		}
	}
	return 0;
err:
	return ret;
}

static int
mtk_hwctrl_mld_bss_free(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	int ret;

	if (hw_bss->mld_cfg.mld_group_idx != MLD_GROUP_NONE) {
		if (hw_bss->type != MTK_IF_TYPE_AP) {
			ret = mtk_wsys_unregister_mld_bss_entry(dev, hw_bss);
			if (ret)
				goto err;
		}
	}

	ret = mtk_hwctrl_mat_tb_free(dev, hw_bss->mld_addr_idx);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

int
mtk_hwctrl_sta_alloc(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta, u8 sta_type)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	int ret = -EINVAL;

	switch (sta_type) {
	case MTK_STA_TYPE_GROUP:
		ret = mtk_idr_register(&hw->group_mgmt, &sta->link_wcid, sta);
		dev_info(dev->dev, "%s(G): mld_sta_idx=%d, wcid=%d, hw_sta=%p\n",
			__func__, sta->mld_sta_idx, sta->link_wcid, sta);
		break;
	case MTK_STA_TYPE_NORMAL:
		ret = mtk_idr_register(&hw->uwtbl_mgmt, &sta->link_wcid, sta);
		dev_info(dev->dev, "%s(N): mld_sta_idx=%d, wcid=%d, hw_sta=%p\n",
			__func__, sta->mld_sta_idx, sta->link_wcid, sta);
		break;
	case MTK_STA_TYPE_MLD:
		ret = mtk_wsys_register_mld_sta_entry(dev, sta);
		break;
	}

	return ret;
}

int
mtk_hwctrl_sta_free(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta)
{
	int ret = 0;
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;

	switch (sta->sta_type) {
	case MTK_STA_TYPE_GROUP:
		dev_info(dev->dev, "%s(G): mld_sta_idx=%d, wcid=%d, hw_sta=%p\n",
			__func__, sta->mld_sta_idx, sta->link_wcid, sta);
		mtk_idr_unregister(&hw->group_mgmt, sta->link_wcid);
		break;
	case MTK_STA_TYPE_NORMAL:
		dev_info(dev->dev, "%s(N): mld_sta_idx=%d, wcid=%d, hw_sta=%p\n",
			__func__, sta->mld_sta_idx, sta->link_wcid, sta);
		mtk_idr_unregister(&hw->uwtbl_mgmt, sta->link_wcid);
		break;
	case MTK_STA_TYPE_MLD:
		ret = mtk_wsys_unregister_mld_sta_entry(dev, sta);
		break;
	default:
		goto err;
	}
	return ret;
err:
	dev_err(dev->dev, "%s(): err %d\n", __func__, ret);
	return -EINVAL;
}

int
mtk_hwctrl_bss_alloc(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	struct mtk_hw_phy *phy = hw_bss->hw_phy;
	struct mtk_hw_bss *tmp_bss;
	u32 id;
	bool find = false;
	int ret;

	/*
	 * fw bss info idx, check same mac_bss need to share same fw_idx,
	 * for band_auto, same fw_idx means share the same bss data
	 * structure in fw
	 */
	mtk_idr_for_each_entry(&hw->bss_mgmt, tmp_bss, id) {

		if (tmp_bss->mac_bss == hw_bss->mac_bss) {
			find = true;
			hw_bss->fw_idx = tmp_bss->fw_idx;
			hw_bss->mld_addr_idx = tmp_bss->mld_addr_idx;
		}
	}

	if (!find) {
		/*there no match mac_bss, we need to alloc a new fw_idx */
		ret = mtk_idr_register(&hw->bss_mgmt, &hw_bss->fw_idx, hw_bss);
		if (ret) {
			dev_err(dev->dev, "%s(): Register idr failed, ret = %d\n", __func__, ret);
			goto err_idr_reg;
		}
	}

	ret = mtk_hwctrl_omac_alloc(dev, hw_bss);
	if (ret) {
		dev_err(dev->dev, "%s(): Allocate omac failed, ret = %d\n", __func__, ret);
		goto err_omac_alloc;
	}

	ret = mtk_hwctrl_mld_bss_alloc(dev, hw_bss);
	if (ret) {
		dev_err(dev->dev, "%s(): Allocate mld_bss failed, ret = %d\n", __func__, ret);
		goto err_mld_bss_alloc;
	}

	ret = mtk_hwctrl_wmm_get(phy, hw_bss);
	if (ret) {
		dev_err(dev->dev, "%s(): Get wmm failed, ret = %d\n", __func__, ret);
		goto err_wmm_get;
	}

	return 0;
err_wmm_get:
	mtk_hwctrl_mld_bss_free(dev, hw_bss);
err_mld_bss_alloc:
	mtk_hwctrl_omac_free(dev, hw_bss);
err_omac_alloc:
	if (!find)
		mtk_idr_unregister(&hw->bss_mgmt, hw_bss->fw_idx);
err_idr_reg:
	return ret;
}

int
mtk_hwctrl_bss_free(struct mtk_hw_dev *dev, struct mtk_hw_bss *hw_bss)
{
	struct mtk_hw_ctrl *hw = &dev->hw_ctrl;
	int ret;

	ret = mtk_hwctrl_mld_bss_free(dev, hw_bss);
	if (ret)
		goto err;

	ret = mtk_hwctrl_omac_free(dev, hw_bss);
	if (ret)
		goto err;

	if (mtk_idr_entry_find(&hw->bss_mgmt, hw_bss->fw_idx))
		mtk_idr_unregister(&hw->bss_mgmt, hw_bss->fw_idx);

	return 0;
err:
	dev_err(dev->dev, "%s(): err %d\n", __func__, ret);
	return ret;
}

/*
 *
 */
int
mtk_hwctrl_phy_alloc(struct mtk_hw_dev *dev, struct mtk_hw_phy *phy)
{
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hwres_radio_cap *radio_cap;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	int i, ret;

	ret = mtk_idr_register(&phy_mgmt->mgmt, &phy->sys_idx.idx, phy);
	if (ret) {
		dev_err(dev->dev, "%s(): Register idr failed, ret = %d\n", __func__, ret);
		goto err_idr_reg;
	}

	for (i = 0; i < res_cap->radio_num; i++) {
		radio_cap = &res_cap->radio_cap[i];

		if ((dev->band_idx_bmp_sup & BIT(radio_cap->band_idx)) &&
		    !(dev->band_idx_bmp_used & BIT(radio_cap->band_idx))) {
			dev->band_idx_bmp_used |= BIT(radio_cap->band_idx);
			break;
		}
	}

	if (i >= res_cap->radio_num) {
		ret = -EINVAL;
		dev_err(dev->dev, "%s(): All radio are occupied\n", __func__);
		goto err_res_cap;
	}

	phy->band_idx = radio_cap->band_idx;

	ret = mtk_idrm_init(&phy->omac_mgmt, "omac", phy->sys_idx.dir,
		radio_cap->omac.start,
		radio_cap->omac.end);
	if (ret) {
		dev_err(dev->dev, "%s(): Init omac idrm failed, ret = %d\n", __func__, ret);
		goto err_omac_init;
	}

	ret = mtk_idrm_init(&phy->ext_omac_mgmt, "ext-omac", phy->sys_idx.dir,
		radio_cap->ext_omac.start,
		radio_cap->ext_omac.end);
	if (ret) {
		dev_err(dev->dev, "%s(): Init ext-omac idrm failed, ret = %d\n", __func__, ret);
		goto err_ext_omac_init;
	}

	ret = mtk_idrm_init(&phy->rept_omac_mgmt, "rept", phy->sys_idx.dir,
			radio_cap->rept_omac.start,
			radio_cap->rept_omac.end);
	if (ret) {
		dev_err(dev->dev, "%s(): Init rept idrm failed, ret = %d\n", __func__, ret);
		goto err_rept_init;
	}

	phy->wmm_ap = radio_cap->wmm_set.start;
	phy->wmm_sta = radio_cap->wmm_set.end;

	ret = mtk_tk_res_ctl_alloc(phy, radio_cap);
	if (ret) {
		dev_err(dev->dev, "%s(): Allocate res_ctl failed, ret = %d\n", __func__, ret);
		goto err_res_ctl_alloc;
	}

	spin_lock_init(&phy->mlo_rx_lock);
	__skb_queue_head_init(&phy->msdu_q);
	phy->msdu_rx_blk = NULL;

	phy_mgmt->phys[phy->band_idx] = phy;
	return 0;
err_res_ctl_alloc:
	mtk_idrm_exit(&phy->rept_omac_mgmt);
err_rept_init:
	mtk_idrm_exit(&phy->ext_omac_mgmt);
err_ext_omac_init:
	mtk_idrm_exit(&phy->omac_mgmt);
err_omac_init:
	dev->band_idx_bmp_used &= ~BIT(phy->band_idx);
err_res_cap:
	mtk_idr_unregister(&phy_mgmt->mgmt, phy->sys_idx.idx);
err_idr_reg:
	dev_info(dev->dev, "%s(): err %d\n", __func__, ret);
	return ret;
}

/*
 *
 */
int
mtk_hwctrl_phy_free(struct mtk_hw_dev *dev, struct mtk_hw_phy *phy)
{
	struct mtk_hw_ctrl *hw_ctrl = &phy->hw_dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;

	rcu_assign_pointer(phy_mgmt->phys[phy->band_idx], NULL);
	mt_synchronize_rcu();

	mtk_idrm_exit(&phy->omac_mgmt);
	mtk_idrm_exit(&phy->ext_omac_mgmt);
	mtk_idrm_exit(&phy->rept_omac_mgmt);

	mtk_tk_res_ctl_free(phy);

	mtk_idr_unregister(&phy_mgmt->mgmt, phy->sys_idx.idx);
	return 0;
}

/*
 *
 */
int
mtk_hwctrl_init_device(struct mtk_hw_dev *dev)
{
	struct mtk_hwres_cap *res_cap = dev->chip_drv->hw_caps->hwres;
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	int ret;

	ret = mtk_tk_mgmt_init(dev);

	if (ret)
		goto end;

	ret = mtk_idrm_init(&hw_ctrl->uwtbl_mgmt, "uwtbl", dev->sys_idx.dir,
		res_cap->uwtbl.start, res_cap->uwtbl.end);

	if (ret)
		goto end;

	ret = mtk_idrm_init(&hw_ctrl->group_mgmt, "group", dev->sys_idx.dir,
		res_cap->group.start, res_cap->group.end);

	if (ret)
		goto end;

	ret = mtk_idrm_init(&hw_ctrl->phy_mgmt.mgmt, "phy", dev->sys_idx.dir,
		0, res_cap->radio_num);

	if (ret)
		goto end;

	ret = mtk_idrm_init(&hw_ctrl->bss_mgmt, "bss", dev->sys_idx.dir,
			res_cap->bss.start,
			res_cap->bss.end);

	if (ret)
		goto end;

	ret = mtk_idrm_init(&hw_ctrl->mld_remap_mgmt, "mld_remap", dev->sys_idx.dir,
		res_cap->mld_remap.start,
		res_cap->mld_remap.end);

	if (ret)
		goto end;

	ret = mtk_mat_tb_mgmt_init(dev);

	if (ret)
		goto end;

	return 0;
end:
	return ret;
}

/*
 *
 */
void
mtk_hwctrl_exit_device(struct mtk_hw_dev *dev)
{
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;

	mtk_tk_exit(dev);
	mtk_tk_mgmt_exit(dev);
	mtk_mat_tb_mgmt_exit(dev);
	mtk_idrm_exit(&hw_ctrl->mld_remap_mgmt);
	mtk_idrm_exit(&hw_ctrl->bss_mgmt);
	mtk_idrm_exit(&hw_ctrl->phy_mgmt.mgmt);
	mtk_idrm_exit(&hw_ctrl->uwtbl_mgmt);
	mtk_idrm_exit(&hw_ctrl->group_mgmt);
}
