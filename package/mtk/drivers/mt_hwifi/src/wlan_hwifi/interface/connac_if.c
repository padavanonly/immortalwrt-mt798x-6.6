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
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <os/hwifi_mac.h>
#include "core.h"
#include "connac_if.h"
#include "mac_if.h"
#include "bus.h"
#include <wlan_tr.h>

#define TOKEN_NUM_FOR_SCHEULE_DEQ 10

static int phy_alloc_mode = PHY_ALLOC_SINGLE;
module_param(phy_alloc_mode, int, 0644);
MODULE_PARM_DESC(phy_alloc_mode, "phy allocation mode");

static int
connac_if_get_phy_num(struct mtk_chip_drv *drv)
{
	if (phy_alloc_mode == PHY_ALLOC_MULTI)
		return drv->hw_caps->hwres->radio_num;
	return 1;
}

static int
connac_if_get_phy_size(struct mtk_chip_drv *drv)
{
	int phy_size = sizeof(struct mtk_hw_phy);

	return (phy_size * connac_if_get_phy_num(drv));
}

static int
connac_if_get_bss_size(struct mtk_mac_hw *hw)
{
	int bss_size = sizeof(struct mtk_hw_bss);
	int bss_ext_size = sizeof(struct connac_bss_wrap);

	bss_size *= hw->phy_num;
	return (bss_size + bss_ext_size);
}

static int
connac_if_get_sta_size(struct mtk_mac_hw *hw)
{
	int sta_size = sizeof(struct mtk_hw_sta);

	return (sta_size * hw->phy_num);
}

static struct mtk_hw_bss *
to_hw_bss(struct mtk_mac_bss *mac_bss)
{
	struct connac_bss_wrap *bss_wrap =
		(struct connac_bss_wrap *) mac_bss->drv_priv;

	return bss_wrap->cur_bss;
}

static int
connac_if_rx_indicate(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy, struct sk_buff_head *frames, struct napi_struct *napi)
{
	int ret = 0;

	dev_info(dev->dev, "%s()\n", __func__);
	return ret;
}

static int
connac_if_rx(struct mtk_hw_dev *hw_dev, struct mtk_hw_phy *hw_phy, struct sk_buff_head *rx_frames, struct napi_struct *napi)
{
	struct sk_buff *skb;
	struct mtk_rx_blk *rx_blk;
	int ret;

	while ((skb = __skb_dequeue(rx_frames)) != NULL) {

		rx_blk = (struct mtk_rx_blk *)skb->cb;
		hw_phy = mtk_hwctrl_phy_entry_find(hw_dev, rx_blk->band_idx);

		if (!hw_phy)
			goto free;

		ret = mtk_mac_rx_napi(hw_phy->mac_hw, NULL, skb);
		if (ret)
			goto free;
		continue;
free:
		dev_kfree_skb(skb);
	}
	return 0;
}

static int
connac_if_rx_event(struct mtk_hw_dev *hw_dev, struct sk_buff *skb)
{
	struct mtk_mac_hw *mac_hw = hw_dev->if_dev;

	return mtk_mac_rx_unsolicited_event(mac_hw, skb);
}

static int
connac_if_rx_uni_event(struct mtk_hw_dev *hw_dev, struct sk_buff *skb)
{
	void *physical_dev = mtk_find_physical_device(hw_dev);

	if (physical_dev)
		return mtk_mac_rx_uni_unsolicited_event(physical_dev, skb);
	else
		return mtk_mac_rx_uni_unsolicited_event(hw_dev->if_dev, skb);
}

static int
connac_if_rx_ser_event(
	u32 chip_id,
	u32 ser_level,
	u32 ser_event,
	u32 hw_id)
{
	return mtk_rx_ser_event(chip_id, ser_level, ser_event, hw_id);
}


static int
connac_if_tx_status(struct mtk_hw_dev *dev, struct mtk_hw_sta *hw_sta,
				void *tx_sts)
{
	u8 mld_link;
	u32 mld_idx;
	u32 mld_link_idx;
	struct mtk_hw_phy *hw_phy;
	u32 band_idx = hw_sta_get_band_idx(hw_sta);
	struct mtk_tx_status *_tx_sts = tx_sts;

	if (band_idx >= MAX_BAND_NUM) {
		dev_err(dev->dev, "%s(): band_idx not exist\n", __func__);
		return -EINVAL;
	}

	hw_phy = mtk_hdev_rx_get_phy(dev, &band_idx, &mld_idx, &mld_link_idx, &mld_link);
	_tx_sts->band_idx = band_idx;

	if (hw_phy)
		mtk_mac_tx_status(hw_phy->mac_hw, _tx_sts);

	return 0;
}

static int
connac_if_dequeue_by_tk_free(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry)
{
	struct mtk_hw_ctrl *hw_ctrl = &dev->hw_ctrl;
	struct mtk_hw_phy_mgmt *phy_mgmt = &hw_ctrl->phy_mgmt;
	struct mtk_hw_phy *hw_phy;
	struct mtk_tk_res_ctl *res_ctl;
	struct mtk_mac_hw *hw;
	u8 band_idx;

	if (!tk_entry)
		return -EINVAL;

	band_idx = tk_entry->band_idx;
	if (band_idx < MAX_BAND_NUM) {
		hw_phy = phy_mgmt->phys[band_idx];
		if (hw_phy) {
			hw = (struct mtk_mac_hw *)hw_phy->mac_hw;
			res_ctl = hw_phy->tk_res_ctl;
			if (atomic_read(&res_ctl->used) < TOKEN_NUM_FOR_SCHEULE_DEQ)
				mtk_mac_dequeue_by_token(hw, band_idx);
		}
	}

	return 0;
}


static int
connac_to_mcu_action(u8 mac_action, u8 *mcu_action)
{
	switch (mac_action) {
	case MAC_MCU_ACT_QUERY:
		*mcu_action = MCU_ACT_QUERY;
		break;
	case MAC_MCU_ACT_SET:
		*mcu_action = MCU_ACT_SET;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
connac_to_mcu_dest(u8 dest, u8 *out_dest)
{
	switch (dest) {
	case MAC_MCU_DEST_WM:
		*out_dest = MCU_DEST_WM;
		break;
	case MAC_MCU_DEST_WA:
		*out_dest = MCU_DEST_WA;
		break;
	case MAC_MCU_DEST_WA_WM:
		*out_dest = MCU_DEST_WA_WM;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
connac_to_mcu_txblk(struct mtk_mcu_txblk *mcu_txblk, struct mtk_mac_mcu_msg *mcu_msg, struct mtk_mcu_resp *resp)
{
	int ret;

	mcu_txblk->cmd = mcu_msg->cmd;
	mcu_txblk->ext_cmd = mcu_msg->ext_cmd;
	mcu_txblk->data = mcu_msg->data;
	mcu_txblk->len = mcu_msg->len;
	mcu_txblk->wait_resp = mcu_msg->is_wait;
	mcu_txblk->ack = mcu_msg->ack;
	mcu_txblk->resp = resp;
	mcu_txblk->uni_cmd = mcu_msg->uni_cmd;
	mcu_txblk->frag_num = mcu_msg->frag_num;
	mcu_txblk->frag_total_num = mcu_msg->frag_total_num;
	mcu_txblk->timeout = mcu_msg->timeout;
	resp->rx_data = mcu_msg->rx_data;
	resp->rx_len = 0;
	ret = connac_to_mcu_dest(mcu_msg->dest, &mcu_txblk->dest);
	if (ret)
		goto err;
	ret = connac_to_mcu_action(mcu_msg->action, &mcu_txblk->action);
	if (ret)
		goto err;

	return 0;
err:
	return -EINVAL;
}

static u8
connac_if_sta_type_trans(u8 sta_type)
{
	u8 type = MTK_STA_TYPE_NORMAL;

	switch (sta_type) {
	case MAC_STA_TYPE_NORMAL:
		type = MTK_STA_TYPE_NORMAL;
		break;
	case MAC_STA_TYPE_GROUP:
		type = MTK_STA_TYPE_GROUP;
		break;
	case MAC_STA_TYPE_MLD:
		type = MTK_STA_TYPE_MLD;
		break;
	}
	return type;
}

static int
connac_if_mcu_tx(struct mtk_mac_hw *hw, struct mtk_mac_mcu_msg *mcu_msg)
{
	int ret;
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_mcu_resp resp;
	struct mtk_mcu_txblk mcu_txblk = {0};

	ret = connac_to_mcu_txblk(&mcu_txblk, mcu_msg, &resp);

	mtk_dbg(MTK_MCU, "%s(ret=%d):len=%d,cmd=0x%x,ext_cmd=0x%x,\n",
		__func__, ret, mcu_txblk.len, mcu_txblk.cmd,
		mcu_txblk.ext_cmd);

	mtk_dbg(MTK_MCU, "%s:wait_rsp=%d,ack=%d,dest=%d,act=%d,\n",
		__func__, mcu_txblk.wait_resp, mcu_txblk.ack,
		mcu_txblk.dest, mcu_txblk.action);

	mtk_dbg(MTK_MCU, "%s:frag=%d/%d,uni=%d\n",
		__func__, mcu_txblk.frag_num, mcu_txblk.frag_total_num,
		mcu_txblk.uni_cmd);

	if (ret)
		goto out;

	ret = mtk_hdev_tx_cmd(hw_dev, &mcu_txblk);

	mtk_dbg(MTK_MCU, "%s(): resp %p, res_len :%d, ret: %d\n",
		__func__, resp.rx_data, resp.rx_len, ret);
	mcu_msg->rx_len = resp.rx_len;
out:
	return ret;
}

static u32
connac_if_mld_type_trans(u32 mld_type)
{
	u32 type;

	switch (mld_type) {
	case MAC_MLD_TYPE_NONE:
		type = MTK_MLD_TYPE_NONE;
		break;
	case MAC_MLD_TYPE_SINGLE:
		type = MTK_MLD_TYPE_SINGLE;
		break;
	case MAC_MLD_TYPE_MULTI:
		type = MTK_MLD_TYPE_MULTI;
		break;
	default:
		type = MTK_MLD_TYPE_NONE;
		break;
	}
	return type;
}

static u32
connac_if_type_trans(u32 bss_type)
{
	u32 type;

	switch (bss_type) {
	case WDEV_TYPE_AP:
	case WDEV_TYPE_ATE_AP:
	case WDEV_TYPE_SERVICE_TXC:
		type = MTK_IF_TYPE_AP;
	break;
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_ATE_STA:
	case WDEV_TYPE_SERVICE_TXD:
		type = MTK_IF_TYPE_STA;
	break;
	case WDEV_TYPE_WDS:
		type = MTK_IF_TYPE_WDS;
	break;
	case WDEV_TYPE_MESH:
		type = MTK_IF_TYPE_MESH;
	break;
	default:
		type = MTK_IF_TYPE_AP;
	break;
	}
	return type;
}

static int
connac_if_add_bss(struct mtk_mac_bss *mac_bss,
	struct mtk_hw_phy *hw_phy, struct mtk_hw_bss *hw_bss)
{
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bss_mld_cfg mld_cfg = {0};
	int ret;

	mld_cfg.mld_type = connac_if_mld_type_trans(mac_bss->if_cfg.mld_type);
	mld_cfg.mld_group_idx = mac_bss->if_cfg.mld_group_idx;
	memcpy(mld_cfg.mld_addr, mac_bss->if_cfg.mld_addr, MAC_ADDR_LEN);
	/*create a new bss*/
	hw_bss->mac_bss = mac_bss;
	ret = mtk_hdev_add_interface(hw_dev, hw_phy,
		hw_bss, mac_bss->if_cfg.if_addr,
		&mld_cfg);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
connac_if_config_bss(struct mtk_mac_bss *mac_bss, struct mtk_hw_bss *hw_bss)
{
	struct connac_bss_wrap *bss_wrap =
		(struct connac_bss_wrap *) mac_bss->drv_priv;
	struct mtk_mld_bss_entry *mld_bss_entry;
	struct mtk_hw_phy *hw_phy;
	struct mtk_hw_dev *hw_dev;
	int ret;

	if (!hw_bss)
		goto err;

	hw_phy = hw_bss->hw_phy;
	hw_dev = hw_phy->hw_dev;

	if (hw_bss->mld_bss_entry) {
		ret = mtk_hdev_update_interface(hw_dev, hw_bss);
		if (ret)
			goto err;
	}

	/*assign to mac layer struct, should not do this after modify mac driver*/
	mac_bss->band_idx = hw_phy->band_idx;
	mac_bss->omac_idx = hw_bss->omac_idx;
	mac_bss->wmm_idx = hw_bss->wmm_idx;

	mld_bss_entry = hw_bss->mld_bss_entry;
	if (mld_bss_entry) {
		mac_bss->mld_remap_idx = mld_bss_entry->remap_id;
		mac_bss->mld_group_addr_idx = mld_bss_entry->mat_idx;
	}

	mac_bss->mld_addr_idx = hw_bss->mld_addr_idx;
	mac_bss->bss_idx = hw_bss->fw_idx;

	bss_wrap->cur_bss = hw_bss;
	dev_info(hw_dev->dev,
		"%s(): fw_idx %d, omac_idx %x, wmm_idx %d, band_idx %d\n",
		__func__,
		hw_bss->fw_idx,
		hw_bss->omac_idx,
		hw_bss->wmm_idx,
		hw_phy->band_idx);

	return 0;
err:
	return -EINVAL;
}

static struct mtk_mac_hw *
connac_if_get_mac_hw(struct mtk_mac_hw *hw, u8 bss_idx)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_bss *expected_hw_bss = NULL;
	struct mtk_hw_phy *expected_hw_phy = NULL;
	struct mtk_mac_hw *expected_hw = NULL;

	expected_hw_bss = mtk_hdev_get_hw_bss(hw_dev, bss_idx);
	if (expected_hw_bss) {
		expected_hw_phy = expected_hw_bss->hw_phy;
		expected_hw = (struct mtk_mac_hw *)expected_hw_phy->mac_hw;
	}

	return expected_hw;
}

static int
connac_if_add_interface(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss)
{
	struct connac_bss_wrap *bss_wrap =
		(struct connac_bss_wrap *) mac_bss->drv_priv;
	struct mtk_hw_bss *hw_bss;
	struct mtk_hw_phy *hw_phy = hw->priv;
	u8 bss_type = connac_if_type_trans(mac_bss->if_cfg.if_type);
	int ret;
	int i;

	/*assign current info*/
	hw_bss = (struct mtk_hw_bss *) (
		(u8 *) bss_wrap + sizeof(*bss_wrap));

	for (i = 0; i < hw->phy_num; i++) {
		bss_wrap->bss[hw_phy->band_idx] = hw_bss;
		hw_bss->type = bss_type;

		ret = connac_if_add_bss(mac_bss, hw_phy, hw_bss);
		if (ret)
			goto err;

		/*default apply phy0 as default*/
		if (i == 0) {
			ret = connac_if_config_bss(mac_bss, hw_bss);
			if (ret)
				goto err;
		}

		hw_bss++;
		hw_phy++;
	}
	return 0;
err:
	return ret;
}

static int
connac_if_remove_interface(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss)
{
	struct connac_bss_wrap *bss_wrap =
		(struct connac_bss_wrap *) mac_bss->drv_priv;
	struct mtk_hw_bss *hw_bss;
	struct mtk_hw_phy *hw_phy;
	struct mtk_hw_dev *hw_dev;
	int i;

	for (i = 0 ; i < MAX_BAND_NUM; i++) {
		hw_bss = bss_wrap->bss[i];

		if (!hw_bss)
			continue;

		hw_phy = hw_bss->hw_phy;
		hw_dev = hw_phy->hw_dev;
		bss_wrap->bss[i] = NULL;
		mtk_hdev_remove_interface(hw_dev, hw_phy, hw_bss);
	}
	return 0;
}

static int
connac_if_change_bss(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss,
	struct mtk_mac_bss_conf *info, u32 change)
{
	struct connac_bss_wrap *bss_wrap =
		(struct connac_bss_wrap *) mac_bss->drv_priv;

	switch (change) {
	case MAC_BSS_CHANGE_BAND:
		return connac_if_config_bss(mac_bss,
			bss_wrap->bss[info->u.band_idx]);
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static int
connac_if_add_mld(struct mtk_mac_hw *hw, u32 mld_type, u32 mld_group_idx, u8 *mld_addr)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bss_mld_cfg mld_cfg = {0};
	int ret;

	mld_cfg.mld_type = connac_if_mld_type_trans(mld_type);
	mld_cfg.mld_group_idx = mld_group_idx;
	memcpy(mld_cfg.mld_addr, mld_addr, MAC_ADDR_LEN);

	ret = mtk_hdev_add_mld(hw_dev, &mld_cfg);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
connac_if_remove_mld(struct mtk_mac_hw *hw, u32 mld_group_idx)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	int ret;

	ret = mtk_hdev_remove_mld(hw_dev, mld_group_idx);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
connac_if_mld_add_link(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss, u32 mld_group_idx)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_bss *hw_bss = (struct mtk_hw_bss *) to_hw_bss(mac_bss);
	struct mtk_mld_bss_entry *mld_bss_entry;
	int ret;

	ret = mtk_hdev_mld_add_link(hw_dev, hw_bss, mld_group_idx);
	if (ret)
		goto err;

	mac_bss->if_cfg.mld_group_idx = hw_bss->mld_cfg.mld_group_idx;
	mac_bss->mld_addr_idx = hw_bss->mld_addr_idx;

	mld_bss_entry = hw_bss->mld_bss_entry;
	if (mld_bss_entry) {
		mac_bss->mld_remap_idx = mld_bss_entry->remap_id;
		mac_bss->mld_group_addr_idx = mld_bss_entry->mat_idx;
	}

	dev_info(hw_dev->dev, "%s(): grp %d, mat %d, mldmat %d, remap %d\n",
		__func__, mac_bss->if_cfg.mld_group_idx, mac_bss->mld_addr_idx,
		mac_bss->mld_group_addr_idx, mac_bss->mld_remap_idx);

	return 0;
err:
	return ret;
}

static int
connac_if_mld_remove_link(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_bss *hw_bss = (struct mtk_hw_bss *) to_hw_bss(mac_bss);
	int ret;

	ret = mtk_hdev_mld_remove_link(hw_dev, hw_bss);
	if (ret)
		goto err;

	mac_bss->if_cfg.mld_group_idx = hw_bss->mld_cfg.mld_group_idx;
	mac_bss->mld_addr_idx = hw_bss->mld_addr_idx;
	mac_bss->mld_remap_idx = OM_REMAP_IDX_NONE;
	mac_bss->mld_group_addr_idx = 0xff;

	dev_info(hw_dev->dev, "%s(): grp %d, mat %d, mldmat %d, remap %d\n",
		__func__, mac_bss->if_cfg.mld_group_idx, mac_bss->mld_addr_idx,
		mac_bss->mld_group_addr_idx, mac_bss->mld_remap_idx);

	return 0;
err:
	return ret;
}

static int
connac_if_add_sta(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss,
				struct mtk_mac_sta *mac_sta)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_bss *hw_bss = (struct mtk_hw_bss *) to_hw_bss(mac_bss);
	struct mtk_hw_sta *hw_sta = (struct mtk_hw_sta *) mac_sta->drv_priv;
	u8 bss_type = connac_if_type_trans(mac_bss->if_cfg.if_type);
	u8 sta_type = connac_if_sta_type_trans(mac_sta->sta_type);
	int i;
	int ret;

	ret = mtk_hdev_add_sta(hw_dev, hw_bss, hw_sta, sta_type, mac_sta->mld_sta_idx);
	if (ret) {
		dev_err(hw_dev->dev, "%s(): Add STA failed, ret = %d\n", __func__, ret);
		goto err_add_sta;
	}

	mac_sta->link_wcid = hw_sta->link_wcid;
	mac_sta->link_wcid2 = WSYS_STA_MAX;

	for (i = 0 ; i < TXQ_NUM; i++) {
		struct mtk_hw_txq *txq = (struct mtk_hw_txq *) mac_sta->txq[i]->drv_priv;

		ret = mtk_hdev_init_txq(hw_dev, hw_bss, hw_sta, i, txq);
		if (ret) {
			dev_err(hw_dev->dev, "%s(): Init TXQ failed, ret = %d\n", __func__, ret);
			goto err_init_txq;
		}

		spin_lock_init(&txq->hw_txq_lock);
	}

	if (sta_type == MTK_STA_TYPE_GROUP) {
		if (!hw_bss->group)
			hw_bss->group = hw_sta;

		if (bss_type == MTK_IF_TYPE_STA) {
			struct mtk_hw_sta *hw_sta_ext = hw_sta + 1;

			ret = mtk_hdev_add_sta(hw_dev, hw_bss, hw_sta_ext,
					sta_type, mac_sta->mld_sta_idx);
			if (ret) {
				dev_err(hw_dev->dev, "%s(): Add STA_EXT failed, ret = %d\n",
					__func__, ret);
				goto err_add_sta_ext;
			}

			mac_sta->link_wcid2 = hw_sta_ext->link_wcid;
		}
	}
	return 0;

err_add_sta_ext:
err_init_txq:
	mtk_hdev_remove_sta(hw_dev, hw_bss, hw_sta);
err_add_sta:
	return ret;
}

static int
connac_if_remove_sta(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss, struct mtk_mac_sta *mac_sta)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_sta *hw_sta = (struct mtk_hw_sta *) mac_sta->drv_priv;
	struct mtk_hw_bss *hw_bss = hw_sta->bss;
	int ret;

	if (mac_sta->link_wcid2 != WSYS_STA_MAX) {
		struct mtk_hw_sta *hw_sta_ext = hw_sta + 1;

		ret = mtk_hdev_remove_sta(hw_dev, hw_bss, hw_sta_ext);
		if (ret)
			dev_err(hw_dev->dev, "%s(): Remove STA2 should not failed, ret = %d\n",
				__func__, ret);
	}

	ret = mtk_hdev_remove_sta(hw_dev, hw_bss, hw_sta);
	if (ret)
		dev_err(hw_dev->dev, "%s(): Remove STA should not failed, ret = %d\n",
			__func__, ret);

	return ret;
}

static int
connac_if_start(struct mtk_mac_hw *hw)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_mac_cap_info *mac_cap_info = &hw->cap_info;
	int ret = 0;

	dev_info(hw_dev->dev, "%s()\n", __func__);

	if (test_bit(HWIFI_STATE_RUNNING, &hw_dev->state)) {
		set_bit(HWIFI_FLAG_EFUSE_READY, &hw_dev->flags);
		goto inf_setting;
	}

	ret = mtk_hdev_start(hw_dev);

	if (ret)
		goto end;

#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
	connac_if_md_coex_register_notifier(hw_dev);
	connac_if_md_coex_register_info(hw_dev);
#endif

inf_setting:
	mac_cap_info->hif_txd_ver_sdo = hw_dev->hif_txd_ver_sdo;

	if (test_bit(HWIFI_FLAG_RX_OFFLOAD, &hw_dev->flags))
		set_bit(MAC_RX_OFFLOAD, &hw->flags);

	if (test_bit(HWIFI_FLAG_BA_OFFLOAD, &hw_dev->flags))
		set_bit(MAC_BA_OFFLOAD, &hw->flags);

	if (test_bit(HWIFI_FLAG_MULTI_BUS, &hw_dev->flags))
		set_bit(MAC_MULTI_BUS, &hw->flags);

	if (test_bit(HWIFI_FLAG_IN_CHIP_RRO, &hw_dev->flags))
		set_bit(MAC_HW_RRO, &hw->flags);

	if (test_bit(HWIFI_FLAG_CHIP_OPTION, &hw_dev->flags)) {
		mac_cap_info->rx_path_type = hw_dev->rx_path_type;
		mac_cap_info->rro_bypass_type = hw_dev->rro_bypass_type;
		mac_cap_info->mld_dest_type = hw_dev->mld_dest_type;
		mac_cap_info->txfreedone_path = hw_dev->txfreedone_path;
		set_bit(MAC_CHIP_OPTION, &hw->flags);
	}

	if (test_bit(HWIFI_FLAG_PARSE_TX_PAYLOAD, &hw_dev->flags))
		set_bit(MAC_PARSE_TX_PAYLOAD, &hw->flags);

	if (test_bit(HWIFI_FLAG_EFUSE_READY, &hw_dev->flags))
		set_bit(MAC_EFUSE_READY, &hw->flags);
	else
		clear_bit(MAC_EFUSE_READY, &hw->flags);

	if (hw_dev->max_ba_wsize_scene_mlo) {
		mac_cap_info->max_ba_wsize_scene_mlo = hw_dev->max_ba_wsize_scene_mlo;
		set_bit(MAC_MAX_BA_WSIZE_SCENE_MLO, &hw->flags);
	}

end:
	return ret;
}

static int
connac_if_stop(struct mtk_mac_hw *hw)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	/*only when last interface stop can do device stop*/
	if (mtk_idr_count(&hw_dev->hw_ctrl.bss_mgmt) > 0)
		return 0;

	clear_bit(HWIFI_FLAG_EFUSE_READY, &hw_dev->flags);

#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
	connac_if_md_coex_unregister_info(hw_dev);
	connac_if_md_coex_unregister_notifier(hw_dev);
#endif

	return mtk_hdev_stop(hw_dev);
}

static int
connac_if_tx_check_resource(struct mtk_mac_hw *hw, struct mtk_mac_txq *txq,
	enum TX_RESOURCE_TYPE resource_type)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_txq *hw_txq = NULL;
	int ret = 0;

	if (txq) {
		hw_txq = (struct mtk_hw_txq *)txq->drv_priv;

		spin_lock(&hw_txq->hw_txq_lock);
		hw_txq->tx_pkt_info = (void *)&resource_type;
		ret = mtk_hdev_tx_check_resource(hw_phy, hw_txq);
		spin_unlock(&hw_txq->hw_txq_lock);
	}

	return ret;
}

static int
connac_if_tx_queue(struct mtk_mac_hw *hw, struct mtk_mac_txq *txq,
					struct _TX_BLK *tx_blk)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_mac_bss *mac_bss;
	struct mtk_mac_sta *mac_sta;
	struct mtk_hw_txq *hw_txq;
	struct mtk_hw_bss *hw_bss = NULL;
	struct mtk_hw_sta *hw_sta = NULL;
	int ret = 0;

	if (!txq) {
		dev_err(hw_dev->dev, "%s(): err txq!\n", __func__);
		goto err;
	}

	mac_bss = txq->bss;
	mac_sta = txq->sta;
	hw_txq = (struct mtk_hw_txq *)txq->drv_priv;

	if (!mac_bss || !mac_sta || !hw_txq) {
		dev_err(hw_dev->dev, "%s(): invalid input!\n", __func__);
		goto err;
	}

	hw_bss = (struct mtk_hw_bss *)to_hw_bss(mac_bss);
	hw_sta = (struct mtk_hw_sta *)mac_sta->drv_priv;

	if (!hw_bss || !hw_sta) {
		dev_err(hw_dev->dev, "%s(): err hw_bss or hw_sta!\n", __func__);
		goto err;
	}

	if (hw_sta != hw_txq->sta) {
		dev_err(hw_dev->dev, "%s(): not sync hw_sta!\n", __func__);
		goto err;
	}

	if (hw_sta->bss != hw_bss) {
		dev_err(hw_dev->dev, "%s(): not sync hw_bss!\n", __func__);
		goto err;
	}

	spin_lock(&hw_txq->hw_txq_lock);
	hw_txq->tx_pkt_info = (void *)tx_blk;

	ret = mtk_hdev_tx_data(hw_phy, hw_txq);

	spin_unlock(&hw_txq->hw_txq_lock);

	return ret;
err:
	return -EINVAL;
}

static int
connac_if_tx_kick(struct mtk_mac_hw *hw)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	return mtk_hdev_tx_kick(hw_dev);
}

static int
connac_if_config_phy(struct mtk_mac_hw *hw, struct mtk_mac_phy *mac_phy)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	dev_info(hw_dev->dev, "%s(): update channel to %d\n", __func__, mac_phy->chan);
	/*TODO: do some tx/rx stop change state*/
	return 0;
}

static int
connac_if_io_read(struct mtk_mac_hw *hw, u32 addr, u32 *val)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	if (!test_bit(BUS_TRANS_FLAG_START, &hw_dev->bus_trans->flag))
		return -EINVAL;

	*val = bus_dbg_read(hw_dev->bus_trans, addr);
	return 0;
}

static int
connac_if_io_write(struct mtk_mac_hw *hw, u32 addr, u32 val)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	if (!test_bit(BUS_TRANS_FLAG_START, &hw_dev->bus_trans->flag))
		return -EINVAL;

	bus_dbg_write(hw_dev->bus_trans, addr, val);
	return 0;
}

static int
connac_if_ser_handler(struct mtk_mac_hw *hw, u32 action, u32 status)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;
	int ret;
	bool enable = status;

	switch (action) {
	case SER_ACTION_IO_SWITCH:
		if (enable)
			set_bit(BUS_TRANS_FLAG_START,
				&hw_dev->bus_trans->flag);
		else
			clear_bit(BUS_TRANS_FLAG_START,
				&hw_dev->bus_trans->flag);
		return 0;
	case SER_ACTION_INT_TO_MCU:
		return mtk_bus_dma_int_to_mcu(trans, status);
	case SER_ACTION_SET_DMA_TK:
		if (enable) {
			ret = mtk_hdev_set_dma_tk(hw_dev, hw_phy, enable);
#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
			if (ret == 0)
				connac_if_md_coex_register_info(hw_dev);
#endif
			return ret;
		}
#ifdef CONFIG_HWIFI_MD_COEX_SUPPORT
		connac_if_md_coex_unregister_info(hw_dev);
#endif
		ret = mtk_hdev_set_dma_tk(hw_dev, hw_phy, enable);
		if (test_and_clear_bit(HWIFI_STATE_RESET_FAILED, &hw_dev->state))
			ret = -EIO;

		return ret;
	case SER_ACTION_PAUSE_TRXQ:
		/* Included in bus_init flow, not need to do again
		 * Can remove this action and the following API
		 * return mtk_bus_dma_pause_trxq(trans, enable);
		 */
		return 0;
	case SER_ACTION_SET_DMA_SW_RXQ:
		/* Included in bus_init/exit flow, not need to do again
		 * Can remove this action and the following API
		 * return mtk_hdev_set_dma_sw_rxq(hw_dev, hw_phy, enable);
		 */
		return 0;
	case SER_ACTION_HW_RESET:
		return mtk_hdev_hw_reset(hw_dev);
	case SER_ACTION_L1_START_END:
		if (enable)
			set_bit(HWIFI_STATE_RESET, &hw_dev->state);
		else
			clear_bit(HWIFI_STATE_RESET, &hw_dev->state);

		mtk_dbg(MTK_MCU, "%s(): SER action = %u, state:0x%lx\n",
			__func__, action, hw_dev->state);
		return 0;
	case SER_ACTION_SET_TRAFFIC:
		return mtk_hdev_set_traffic(hw_dev, enable);
	default:
		dev_err(hw_dev->dev, "%s(): action = %u not supported\n",
			__func__, action);
		return -EOPNOTSUPP;
	}

}

static int
connac_if_get_dbginfo(struct mtk_mac_hw *hw, char *arg)
{
	struct mtk_hw_phy *hw_phy = hw->priv;

	return mtk_dbg_info(MTK_IOCTL_PATH, hw_phy, arg);
}

static u16
connac_if_get_max_uwtbl_num(struct mtk_mac_hw *hw, bool all)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_ctrl *hw_ctrl = &hw_dev->hw_ctrl;

	if (all) /* unicast ( for max skus) + bcast */
		return (1) + mtk_hdev_get_idrm_high(hw_dev, &hw_ctrl->group_mgmt);
	else /* unicast max */
		return mtk_hdev_get_idrm_high(hw_dev, &hw_ctrl->uwtbl_mgmt);
}

static int
connac_if_set_max_uwtbl_sta_num(struct mtk_mac_hw *hw, u16 max_sta_num, u16 max_group_num)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_chip_drv *chip_drv = hw_phy->hw_dev->chip_drv;
	struct mtk_hwres_cap *hwres_caps = chip_drv->hw_caps->hwres;
	int ret;

	struct mtk_hw_ctrl *hw_ctrl = &hw_dev->hw_ctrl;

	dev_info(hw_dev->dev, "uwtbl.end from %u to %u\n",
		hwres_caps->uwtbl.end, max_sta_num);

	ret = mtk_hdev_update_idrm(hw_dev, &hw_ctrl->uwtbl_mgmt,
			hwres_caps->uwtbl.start, max_sta_num);
	if (ret) {
		dev_info(hw_dev->dev, "!!! update uwtbl fail  !!!\n");
		goto end;
	}

	dev_info(hw_dev->dev, "group.start from %u to %u\n",
		hwres_caps->group.start, (max_sta_num + 1));
	dev_info(hw_dev->dev, "group.end from %u to %u\n",
		hwres_caps->group.end, (max_sta_num + max_group_num));
	ret = mtk_hdev_update_idrm(hw_dev, &hw_ctrl->group_mgmt,
			(max_sta_num + 1), (max_sta_num + max_group_num));
	if (ret) {
		dev_info(hw_dev->dev, "!!! update group fail  !!!\n");
		goto end;
	}

	return 0;

end:
	return ret;
}

static u16
connac_if_get_inf_num(struct mtk_mac_hw *hw)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	return mtk_idr_count(&hw_dev->hw_ctrl.bss_mgmt);
}

void
connac_if_get_fw_info(struct mtk_mac_hw *hw, int mcu_type, char *fw_ver, char *build_date,
			char *fw_ver_long)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	mtk_hdev_get_fw_info(hw_dev, mcu_type, fw_ver, build_date, fw_ver_long);
}

void
connac_if_set_fw_mode(struct mtk_mac_hw *hw, int mcu_type, unsigned char fw_mode)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	mtk_hdev_set_fw_mode(hw_dev, mcu_type, fw_mode);
}

static int
connac_if_get_mld_id(struct mtk_mac_hw *hw, struct mtk_mac_sta *mac_sta)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_sta *hw_sta = (struct mtk_hw_sta *) mac_sta->drv_priv;
	u32 pri_id = 0;
	u32 sec_id = 0;
	int ret = -EINVAL;

	if (!hw_sta)
		return ret;

	ret = mtk_hdev_get_mld_id(hw_dev, hw_sta, &pri_id, &sec_id);
	if (ret)
		return ret;

	/*primary idx (uwtbl view) which is used for even TID*/
	mac_sta->mld_primary_idx = pri_id;
	/*secondary idx (uwtbl view) which is used for odd TID*/
	mac_sta->mld_secondary_idx = sec_id;

	return ret;
}

bool
connac_if_init_rro_addr_elem_by_seid(struct mtk_mac_hw *hw, u16 seid)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *bus_trans = hw_dev->bus_trans;

	/*Avoid access rro addr_elem during L1 SER*/
	if (test_bit(HWIFI_STATE_RESET, &hw_dev->state))
		return false;

	if (bus_trans->dma_ops->init_rro_addr_elem_by_seid)
		return bus_trans->dma_ops->init_rro_addr_elem_by_seid(bus_trans, seid);

	return false;
}

static int
connac_if_set_pao_sta_info(struct mtk_mac_hw *hw, u16 wcid,
		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;

	return mtk_bus_set_pao_sta_info(trans, wcid,
			max_amsdu_nums, max_amsdu_len, remove_vlan, hdrt_mode);
}

static int
connac_if_set_pn_check(struct mtk_mac_hw *hw, u16 wcid, u16 se_id, bool enable)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;

	return mtk_bus_set_pn_check(trans, wcid, se_id, enable);
}

static int
connac_if_set_particular_to_host(struct mtk_mac_hw *hw, bool enable)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;

	return mtk_bus_set_particular_to_host(trans, enable);
}

static int
connac_if_get_tx_token_num(struct mtk_mac_hw *hw, u16 tx_token_num[], u8 max_src_num)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;

	return mtk_bus_get_tx_token_num(trans, tx_token_num, max_src_num);
}

static u32
connac_if_get_free_sta_pool_num(struct mtk_mac_hw *hw)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;

	return mtk_hdev_get_free_sta_pool_num(hw_dev);
}

static void
connac_if_get_wtbl_idrm_range_num(struct mtk_mac_hw *hw, u16 *low, u16 *high,  u8 wtbl_type)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_ctrl *hw_ctrl = &hw_dev->hw_ctrl;
	int local_l = 0;
	int local_h = 0;

	if (wtbl_type == MTK_STA_TYPE_NORMAL) { /* uwtbl */
		local_l = mtk_hdev_get_idrm_low(hw_dev, &hw_ctrl->uwtbl_mgmt);
		local_h = mtk_hdev_get_idrm_high(hw_dev, &hw_ctrl->uwtbl_mgmt);
	}

	if (wtbl_type == MTK_STA_TYPE_GROUP) { /* group */
		local_l = mtk_hdev_get_idrm_low(hw_dev, &hw_ctrl->group_mgmt);
		local_h = mtk_hdev_get_idrm_high(hw_dev, &hw_ctrl->group_mgmt);
	}

	if (low && local_l)
		*low = local_l;

	if (high && local_h)
		*high = local_h;
}

static int
connac_if_change_setup_link_sta(struct mtk_mac_hw *hw, struct mtk_mac_sta *mac_sta)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_hw_sta *hw_sta = (struct mtk_hw_sta *) mac_sta->drv_priv;
	int ret;

	ret = mtk_hdev_change_setup_link_sta(hw_dev, hw_sta);
	if (ret)
		goto err;

	return 0;
err:
	dev_info(hw_dev->dev, "%s(): err\n", __func__);
	return ret;
}

static int connac_if_get_rro_sp_page_num(struct mtk_mac_hw *hw, u32 *page_num)
{
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_dev *hw_dev = hw_phy->hw_dev;
	struct mtk_bus_trans *trans = hw_dev->bus_trans;

	return mtk_bus_get_rro_sp_page_num(trans, page_num);
}

/*
 *struct mtk_mac_hw_ops {
 *	int (*add_interface)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss);
 *	int (*remove_interface)(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss);
 *	int (*add_sta)(struct mtk_mac_hw *hw, struct mtk_mac_bss *bss, struct mtk_mac_sta *sta);
 *	int (*remove_sta)(struct mtk_mac_hw *hw, struct mtk_mac_bss *bss, struct mtk_mac_sta *sta);
 *	int (*start)(struct mtk_mac_hw *hw);
 *	int (*stop)(struct mtk_mac_hw *hw);
 *	int (*mcu_tx)(struct mtk_mac_hw *hw, struct mtk_mac_mcu_msg *mcu_msg);
 *	int (*tx_pkt)(struct mtk_mac_hw *hw, struct sk_buff *skb);
 *	int (*tx_queue)(struct mtk_mac_hw *hw, struct mtk_mac_txq *txq);
 *	int (*config_phy)(struct mtk_mac_hw *hw, struct mtk_mac_phy *mac_phy);
 *	int (*bus_io_read)(struct mtk_mac_hw *hw, u32 addr, u32 *val);
 *	int (*bus_io_write)(struct mtk_mac_hw *hw, u32 addr, u32 val);
 *	int (*ser_handler)(struct mtk_mac_hw *hw, u32 action, u32 status);
 *	u16 (*get_max_uwtbl_num)(struct mtk_mac_hw *hw, bool all);
 *	int (*set_max_uwtbl_sta_num)(struct mtk_mac_hw *hw, u16 max_sta_num, u16 max_group_num);
 *	u16 (*get_inf_num)(struct mtk_mac_hw *hw);
 *	void (*set_fw_mode)(struct mtk_mac_hw *hw, unsigned char fw_mode);
 *	int (*set_pao_sta_info)(struct mtk_mac_hw *hw, u16 wcid,
 *		u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode);
 *	int (*set_pn_check)(struct mtk_mac_hw *hw, u16 wcid, u16 se_id, bool enable);
 *	int (*get_tx_token_num)(struct mtk_mac_hw *hw, u16 tx_token_num[], u8 max_src_num);
 *	u32 (*get_free_sta_pool_num)(struct mtk_mac_hw *hw);
 *	void (*get_wtbl_idrm_range_num)(struct mtk_mac_hw *hw, u16 *low, u16 *high,  u8 wtbl_type)
 *};
 */

struct mtk_mac_hw_ops hw_ops = {
	.mcu_tx = connac_if_mcu_tx,
	.add_interface = connac_if_add_interface,
	.remove_interface = connac_if_remove_interface,
	.add_mld = connac_if_add_mld,
	.remove_mld = connac_if_remove_mld,
	.mld_add_link = connac_if_mld_add_link,
	.mld_remove_link = connac_if_mld_remove_link,
	.add_sta = connac_if_add_sta,
	.remove_sta = connac_if_remove_sta,
	.start = connac_if_start,
	.stop = connac_if_stop,
	.tx_check_resource = connac_if_tx_check_resource,
	.tx_queue = connac_if_tx_queue,
	.tx_kick = connac_if_tx_kick,
	.config_phy = connac_if_config_phy,
	.bus_io_read = connac_if_io_read,
	.bus_io_write = connac_if_io_write,
	.change_bss = connac_if_change_bss,
	.ser_handler = connac_if_ser_handler,
	.get_max_uwtbl_num = connac_if_get_max_uwtbl_num,
	.set_max_uwtbl_sta_num = connac_if_set_max_uwtbl_sta_num,
	.get_inf_num = connac_if_get_inf_num,
	.get_mac_hw = connac_if_get_mac_hw,
	.get_fw_info = connac_if_get_fw_info,
	.set_fw_mode = connac_if_set_fw_mode,
	.get_mld_id = connac_if_get_mld_id,
	.init_rro_addr_elem_by_seid = connac_if_init_rro_addr_elem_by_seid,
	.set_pao_sta_info = connac_if_set_pao_sta_info,
	.set_pn_check = connac_if_set_pn_check,
	.set_particular_to_host = connac_if_set_particular_to_host,
	.get_tx_token_num = connac_if_get_tx_token_num,
	.get_dbg_info = connac_if_get_dbginfo,
	.get_free_sta_pool_num = connac_if_get_free_sta_pool_num,
	.get_wtbl_idrm_range_num = connac_if_get_wtbl_idrm_range_num,
	.change_setup_link_sta = connac_if_change_setup_link_sta,
	.get_rro_sp_page_num = connac_if_get_rro_sp_page_num,
};

static int
connac_if_chip_reset(unsigned int chip_id)
{
	return mtk_mac_chip_reset(chip_id);
}

static struct mtk_hw_phy *
connac_if_alloc_phy(struct mtk_hw_dev *hw_dev)
{
	struct mtk_mac_hw *hw;
	struct mtk_hw_phy *hw_phy;
	int priv_sz = sizeof(*hw_phy);

	dev_info(hw_dev->dev, "%s()\n", __func__);

	hw = mtk_mac_alloc_hw(priv_sz, &hw_ops);
	if (!hw)
		goto err;

	dev_info(hw_dev->dev, "%s(): hw:%p, priv:%p\n",
		__func__, hw, hw->priv);

	hw->pdev = hw_dev->dev;
	hw->phy_num = 1;
	hw_phy = hw->priv;
	hw_phy->mac_hw = hw;
	hw_phy->hw_dev = hw_dev;

	return hw_phy;
err:
	return NULL;
}

static int
connac_if_register_device_with_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	struct mtk_mac_hw *hw = hw_phy->mac_hw;
	struct mtk_chip_drv *chip_drv = dev->chip_drv;
	int ret;

	dev_info(dev->dev, "%s()\n", __func__);

	hw->chip_id = chip_drv->device_id;
	hw->bss_priv_size = connac_if_get_bss_size(hw);
	hw->sta_priv_size = connac_if_get_sta_size(hw);
	hw->txq_priv_size = sizeof(struct mtk_hw_txq);

	ret = mtk_hdev_add_phy(dev, hw_phy);
	if (ret)
		goto err;

	hw->band_idx = hw_phy->band_idx;

	ret = mtk_mac_hw_register(dev, hw);
	if (ret)
		goto err2;

	return 0;
err2:
	mtk_hdev_remove_phy(dev, hw_phy);
err:
	return -EINVAL;
}

static int
connac_if_unregister_device_single_phy(struct mtk_hw_dev *dev)
{
	struct mtk_hw_phy *hw_phy;
	u32 id;

	mtk_idr_for_each_entry(&dev->hw_ctrl.phy_mgmt.mgmt, hw_phy, id) {
		bool free = hw_phy->mac_hw != dev->if_dev;

		mtk_hdev_remove_phy(dev, hw_phy);

		mtk_mac_hw_unregister(hw_phy->mac_hw);
		if (free)
			mtk_mac_free_hw(hw_phy->mac_hw);
	}
	return 0;
}

static int
connac_if_unregister_device_multi_phy(struct mtk_hw_dev *dev)
{
	struct mtk_hw_phy *hw_phy;
	struct mtk_mac_hw *hw = dev->if_dev;
	u32 id;

	mtk_idr_for_each_entry(&dev->hw_ctrl.phy_mgmt.mgmt, hw_phy, id) {
		mtk_hdev_remove_phy(dev, hw_phy);
	}
	mtk_mac_hw_unregister(hw);
	return 0;
}
static int
connac_if_unregister_device_with_phy(struct mtk_hw_dev *dev)
{
	struct mtk_mac_hw *hw = dev->if_dev;

	if (hw->phy_num > 1)
		return connac_if_unregister_device_multi_phy(dev);
	else
		return connac_if_unregister_device_single_phy(dev);
}

static int
connac_if_add_phy(struct mtk_hw_dev *hw_dev, struct mtk_hw_phy **hw_phy)
{
	struct mtk_hw_phy *new_phy = connac_if_alloc_phy(hw_dev);

	if (!new_phy)
		goto err;

	*hw_phy = new_phy;
	return connac_if_register_device_with_phy(hw_dev, new_phy);
err:
	return -ENOMEM;
}

static struct mtk_hw_dev *
connac_if_alloc_device(struct mtk_chip_drv *drv,
	struct device *pdev, size_t size)
{
	struct mtk_mac_hw *hw;
	struct mtk_hw_phy *hw_phy;
	struct mtk_hw_dev *hw_dev;
	int phy_size = connac_if_get_phy_size(drv);
	int priv_sz = phy_size + size;

	dev_info(pdev, "%s()\n", __func__);

	hw = mtk_mac_alloc_hw(priv_sz, &hw_ops);
	if (!hw)
		goto err;

	dev_info(pdev, "%s(): hw:%p, priv:%p, phy_size = %d\n",
		__func__, hw, hw->priv, phy_size);

	hw->pdev = pdev;
	hw->phy_num = connac_if_get_phy_num(drv);
	hw_phy = hw->priv;
	hw_phy->mac_hw = hw;
	hw_dev = (struct mtk_hw_dev *)((char *) hw_phy + phy_size);
	hw_phy->hw_dev = hw_dev;
	hw_dev->if_dev = hw;
	return hw_dev;
err:
	return NULL;
}

static int
connac_if_free_device(struct mtk_hw_dev *dev)
{
	struct mtk_mac_hw *hw = dev->if_dev;

	dev_info(dev->dev, "%s()\n", __func__);
	mtk_mac_free_hw(hw);
	return 0;
}

static int
connac_if_add_ext_phy(struct mtk_hw_dev *dev, struct mtk_hw_phy *hw_phy)
{
	struct mtk_mac_hw *hw = dev->if_dev;
	int phy_size = sizeof(*hw_phy);
	int ret = 0;
	int i;

	for (i = 1; i < hw->phy_num; i++) {
		struct mtk_hw_phy *ext_phy = (struct mtk_hw_phy *)
			((char *)hw_phy + i * phy_size);

		ret = mtk_hdev_add_phy(dev, ext_phy);
		if (ret)
			break;
		ext_phy->mac_hw = hw;
		ext_phy->hw_dev = hw_phy->hw_dev;
	}
	return ret;
}

static int
connac_if_register_device(struct mtk_hw_dev *dev)
{
	struct mtk_mac_hw *hw = dev->if_dev;
	struct mtk_hw_phy *hw_phy = hw->priv;
	struct mtk_hw_phy *new_phy = hw_phy;
	struct mtk_chip_drv *chip_drv = dev->chip_drv;
	struct mtk_chip_hw_cap *hw_caps = chip_drv->hw_caps;
	struct mtk_mac_cap_info *mac_cap_info = &hw->cap_info;
	struct mtk_hw_cap_info *hw_cap_info = &hw_caps->cap_info;
	int i = 0, ret;

	if (hw_caps->mac_cap & BIT(CAP_PAO)) {
		set_bit(MAC_SW_AMSDU, &hw->flags);
		mac_cap_info->sw_max_amsdu_num = hw_cap_info->sw_max_amsdu_num;
		mac_cap_info->sw_max_amsdu_len = hw_cap_info->sw_max_amsdu_len;
	}

	/* Hwifi passes per card dev pointer to logan to add per card information in logan. */
	mtk_add_main_device(dev, chip_drv->device_id, hw->flags, mac_cap_info);

	while (true) {
		ret = connac_if_register_device_with_phy(dev, new_phy);
		if (ret)
			goto err;

		if (++i >= dev->band_num_sup)
			break;

		new_phy = connac_if_alloc_phy(dev);
		if (!new_phy) {
			ret = -ENOMEM;
			goto err;
		}
	}

	return connac_if_add_ext_phy(dev, hw_phy);
err:
	connac_if_unregister_device_with_phy(dev);
	return ret;
}

static int
connac_if_unregister_device(struct mtk_hw_dev *dev)
{
	/*
	 * Hwifi doesn't need to remove main device information of logan.
	 * Logan can remove own main device information directly.
	 */
	return connac_if_unregister_device_with_phy(dev);
}

static int
connac_if_ba_trig_event(struct mtk_hw_dev *dev, u16 wcid, u8 tid)
{
	struct mtk_mac_hw *hw = dev->if_dev;

	if (hw->sw_ops && hw->sw_ops->ba_trig_event)
		return hw->sw_ops->ba_trig_event(hw, wcid, tid);
	return -EOPNOTSUPP;
}

static struct mtk_interface connac_if = {
	.type = MTK_INTERFACE_CONNAC,
	.ops = {
		.alloc_device = connac_if_alloc_device,
		.free_device = connac_if_free_device,
		.register_device = connac_if_register_device,
		.unregister_device = connac_if_unregister_device,
		.rx_pkt = connac_if_rx,
		.rx_indicate_pkt = connac_if_rx_indicate,
		.tx_status = connac_if_tx_status,
		.rx_event = connac_if_rx_event,
		.rx_uni_event = connac_if_rx_uni_event,
		.add_phy = connac_if_add_phy,
		.rx_ser_event = connac_if_rx_ser_event,
		.dequeue_by_tk_free = connac_if_dequeue_by_tk_free,
		.ba_trig_event = connac_if_ba_trig_event,
		.chip_reset = connac_if_chip_reset,
	},
};

static int __init mtk_connac_if_init(void)
{
	return mtk_interface_register(&connac_if);
}

static void __exit mtk_connac_if_exit(void)
{
	mtk_interface_unregister(&connac_if);
}

module_init(mtk_connac_if_init);
module_exit(mtk_connac_if_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Connac3 Wi-Fi Module");
