/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include "rt_config.h"
#include "hdev/hdev.h"
#include "os/hwifi_main.h"
#include "hdev/hdev.h"
#include "dot11_base.h"
#include "hw_ctrl/rro_cmm.h"

#ifdef CFG_MTK_SUPPORT_ANDROID_WMT
#include <connectivity_build_in_adapter.h>
#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
#include "connsys_logs_ctrl.h"
#endif /* CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH */
#endif /* CFG_MTK_SUPPORT_ANDROID_WMT */

#define hw_to_mac_dev(_hw) container_of(_hw, struct mtk_mac_dev, hw)

static u8 tid_to_ac[17] = {
	0, 1, 1, 0, 2, 2, 3, 3,
	0, 1, 1, 0, 2, 2, 3, 3,
	3
};

static inline struct mtk_mac_dev *
pad_to_mac_dev(struct _RTMP_ADAPTER *ad)
{
	struct os_cookie *handle = ad->OS_Cookie;

	return handle->mac_dev;
}

static bool
is_efuse_ready(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);

	return test_bit(MAC_EFUSE_READY, &mac_dev->hw.flags);
}

static int
mtk_mac_set_cap(struct _RTMP_ADAPTER *ad, struct mtk_mac_hw *hw)
{
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);

	if (test_bit(MAC_RX_OFFLOAD, &hw->flags))
		cap->tkn_info.feature |= TOKEN_RX;
	if (test_bit(MAC_BA_OFFLOAD, &hw->flags))
		cap->asic_caps |= fASIC_CAP_BA_OFFLOAD;
	if (test_bit(MAC_SW_AMSDU, &hw->flags)) {
		cap->qm = GENERIC_QM;
		/* XX = hw->cap_info.sw_max_amsdu_num; */
		/* XX = hw->cap_info.sw_max_amsdu_len; */
	}
	if (test_bit(MAC_HW_RRO, &hw->flags))
		cap->asic_caps |= fASIC_CAP_HW_RRO;
	if (test_bit(MAC_MULTI_BUS, &hw->flags))
		cap->asic_caps |= fASIC_CAP_TWO_PCIE;

	RTMP_CLEAR_FLAG(ad->physical_dev, PH_DEV_CHECK_ETH_TYPE);
	if (test_bit(MAC_PARSE_TX_PAYLOAD, &hw->flags))
		RTMP_SET_FLAG(ad->physical_dev, PH_DEV_CHECK_ETH_TYPE);

	if (test_bit(MAC_MAX_BA_WSIZE_SCENE_MLO, &hw->flags))
		cap->max_ba_wsize_scene_mlo = hw->cap_info.max_ba_wsize_scene_mlo;
	else
		cap->max_ba_wsize_scene_mlo = 0;

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
		"rx %d, ba :%d, rro_cap:%d, asic_caps: %d\n",
		test_bit(MAC_RX_OFFLOAD, &hw->flags),
		test_bit(MAC_BA_OFFLOAD, &hw->flags),
		cap->rro_cap,
		cap->asic_caps);
	return 0;
}

static inline int
hwifi_txq_init(struct mtk_mac_hw *hw, struct mtk_mac_bss *mac_bss,
	struct mtk_mac_sta *mac_sta, struct mtk_mac_txq **txq, u8 tid)
{
	struct mtk_mac_txq *mac_txq = NULL;

	mac_txq = kzalloc(sizeof(*mac_txq) + hw->txq_priv_size, GFP_KERNEL);
	if (!mac_txq)
		goto err;
	mac_txq->ac = tid_to_ac[tid];
	mac_txq->tid = tid;
	mac_txq->bss = mac_bss;
	mac_txq->sta = mac_sta;
	txq[tid] = mac_txq;
	return 0;
err:
	return -ENOMEM;
}


static u16
hwifi_add_sta(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
	BOOLEAN is_A4, BOOLEAN is_apcli, uint16_t mld_sta_idx)
{
	int ret, i;
	struct _STA_TR_ENTRY *sta;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct mtk_mac_sta *mac_sta = NULL;
	u32 sw_wcid = 0, hw_wcid = 0;

#ifdef SW_CONNECT_SUPPORT
	struct physical_device *ph_dev = ad->physical_dev;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	bool is_add_sw_dummy_path = false;

	if (!ph_dev)
		goto err;

	if (!cap)
		goto err;

	if (hc_is_sw_sta_enable(ad)) {
		u32 hw_free_sta_pool_num = mac_dev->ops->get_free_sta_pool_num(hw);

		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"hc_is_sw_sta_enable(ad) = %d, BAND_NUM = %d, hw_free_sta_pool_num = %u, band_idx=%d\n",
				hc_is_sw_sta_enable(ad), PD_GET_BAND_NUM(ad->physical_dev), hw_free_sta_pool_num, hw->band_idx);

		if (hw_free_sta_pool_num <= PD_GET_BAND_NUM(ad->physical_dev))
			is_add_sw_dummy_path = true;

		mac_sta = hc_is_dummy_sta_exist(ad, hw->band_idx);

		/* Not only check S/W dummy exist , but also to check the boundary hit,
		     to Prevent non dummy removed & won't be use again.
		*/
		if (mac_sta && is_add_sw_dummy_path) {
			/* reuse the dummy mac_sta for other tr_entry as pure s/w sta */
			ret = os_idr_register(&(ph_dev->sw_uwtbl_mgmt), &sw_wcid, mac_sta);

			if (ret) {
				/* S/W FULL */
				MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
					"!!!SW WTBL FULL!!! : band_idx=%d, ad->MacTab->Size=%d, ref_cnt=%d\n",
					hw->band_idx, ad->MacTab->Size, atomic_read(&mac_sta->sw_dummy_sta_ref_cnt));
				return  -ENOMEM;
			}

			sta = tr_entry_get(ad, sw_wcid);

			sta->mac_sta = mac_sta;
			sta->wcid = (u16)(sw_wcid);
			sta->wdev = wdev;

			sta->HwWcid = mac_sta->link_wcid;
			sta->bSwMainSta = (sta->HwWcid == sta->wcid) ? true : false;
			sta->bSw = true;

			atomic_inc(&mac_sta->sw_dummy_sta_ref_cnt);

			MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
				"SW CASE 2 : band_idx=%d, entry %p, sta :%p, link_wcid = %d, sw_wcid=%d, ref_cnt=%d\n",
				hw->band_idx, sta, mac_sta, mac_sta->link_wcid, sw_wcid, atomic_read(&mac_sta->sw_dummy_sta_ref_cnt));
			return sta->wcid;
		}
	}
#endif /* SW_CONNECT_SUPPORT */

	/* normal case for add sta */
	mac_sta = kzalloc(sizeof(*mac_sta) + hw->sta_priv_size, GFP_KERNEL);

	if (!mac_sta)
		goto err;

	/*create txq*/
	for (i = 0 ; i < TXQ_NUM; i++) {
		ret = hwifi_txq_init(hw, mac_bss, mac_sta, mac_sta->txq, i);
		if (ret)
			goto err;
	}
	/*TODO: MLD STA may request multi-link, same mld_sta_idx will query same hw reource*/
	mac_sta->sta_type = (mld_sta_idx == MLD_STA_NONE ? MAC_STA_TYPE_NORMAL : MAC_STA_TYPE_MLD);
	mac_sta->mld_sta_idx = mld_sta_idx;

	ret = mac_dev->ops->add_sta(hw, mac_bss, mac_sta);

	if (ret)
		goto err;

	sw_wcid = hw_wcid = mac_sta->link_wcid;

	/*update to original mac driver*/
	sta = tr_entry_get(ad, sw_wcid);

#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad)) {
		if (is_add_sw_dummy_path) {
			/* update the dummy mac_sta s/w sta */
			ret = os_idr_register(&(ph_dev->sw_uwtbl_mgmt), &sw_wcid, mac_sta);

			if (ret) {
				/* S/W FULL */
				MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
					"!!!SW WTBL FULL!!! : band_idx=%d, ad->MacTab->Size=%d, ref_cnt=%d\n",
					hw->band_idx, ad->MacTab->Size, atomic_read(&mac_sta->sw_dummy_sta_ref_cnt));
				goto err;
			}

			/* s/w entry of dummy */
			sta = tr_entry_get(ad, sw_wcid);
			sta->bSwMainSta = (hw_wcid == sw_wcid) ? true : false;
			sta->bSw = true;
			mac_sta->is_dummy_role = true;
			atomic_inc(&mac_sta->sw_dummy_sta_ref_cnt);
			hc_add_dummy_sta(ad, (void *)mac_sta);
		}
	}
#endif /* SW_CONNECT_SUPPORT */

	sta->mac_sta = mac_sta;
	sta->wcid = sw_wcid;
	sta->HwWcid = hw_wcid;
	sta->wdev = wdev;

#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad))
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"BN%d entry %p, sta :%p, HwWcid=%d, wcid = %d, ref_cnt=%d\n", hw->band_idx, sta, mac_sta, hw_wcid, sw_wcid, atomic_read(&mac_sta->sw_dummy_sta_ref_cnt));
	else
#endif  /* SW_CONNECT_SUPPORT */
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
			"entry %p, sta :%p, wcid = %d\n", sta, mac_sta, sw_wcid);

	return sta->wcid;

err:
	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "create sta fail\n");
	if (mac_sta) {
#ifdef SW_CONNECT_SUPPORT
		if (mac_sta->is_dummy_role == true)
			atomic_dec(&mac_sta->sw_dummy_sta_ref_cnt);

		if (atomic_read(&mac_sta->sw_dummy_sta_ref_cnt) == 0)
#endif /* SW_CONNECT_SUPPORT */
		{
			for (i = 0 ; i < TXQ_NUM; i++) {
				if (mac_sta->txq[i]) {
					MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
						"free mac txq !!!\n");
					kfree(mac_sta->txq[i]);
				}
			}

			MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
				"free mac_sta!!!\n");
			kfree(mac_sta);
		}
	}
	return -ENOMEM;
}

static void
hwifi_free_sta(struct rcu_head *head)
{
	struct mtk_mac_sta *mac_sta = container_of(head, struct mtk_mac_sta, rcu);
	int i;

	for (i = 0 ; i < TXQ_NUM; i++)
		kfree(mac_sta->txq[i]);
	kfree(mac_sta);
}

static u16
hwifi_remove_sta(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u16 sta_idx)
{
	struct _STA_TR_ENTRY *sta = tr_entry_get(ad, sta_idx);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct mtk_mac_sta *mac_sta = sta->mac_sta;
	int ret;
#ifdef SW_CONNECT_SUPPORT
	struct physical_device *ph_dev = ad->physical_dev;

	if (!ph_dev)
		goto err;
#endif /* SW_CONNECT_SUPPORT */

	if (!mac_sta)
		goto err;

#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad)) {
		if (hc_is_sw_wcid(ad, sta_idx)
			&& (atomic_read(&mac_sta->sw_dummy_sta_ref_cnt) > 0)) {
			os_idr_unregister(&ph_dev->sw_uwtbl_mgmt, sta_idx);
			atomic_dec(&mac_sta->sw_dummy_sta_ref_cnt);

			if (atomic_read(&mac_sta->sw_dummy_sta_ref_cnt) == 0) {
				hc_del_dummy_sta(ad, mac_sta);
				sta->bSw = false;
				sta->bSwMainSta = false;
				mac_sta->is_dummy_role = false;
			} else
				return 0;
		}

		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
				"BN%d, sta_idx=%u, tr_entry=%p, bSw=%d, mac_sta=%p, ref_cnt=%d\n",
				hw->band_idx, sta_idx, sta, sta->bSw, mac_sta, atomic_read(&mac_sta->sw_dummy_sta_ref_cnt));
	}
#endif /* SW_CONNECT_SUPPORT */

	rcu_assign_pointer(sta->mac_sta, NULL);

	ret = mac_dev->ops->remove_sta(hw, mac_bss, mac_sta);
	if (ret)
		goto err;

	mt_call_rcu(&mac_sta->rcu, hwifi_free_sta);
	return 0;
err:
	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "remove sta(%u) fail!!\n", sta_idx);
	return -EINVAL;
}

static void hwifi_init_radio(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_phy *mac_phy = &mac_dev->mac_phy;

	OS_NdisAllocateSpinLock(&mac_phy->lock);
	OS_SEM_LOCK(&mac_phy->lock);
	mac_phy->chan = 0;
	mac_phy->chan2 = 0;
	mac_phy->cen_chan = 0;
	mac_phy->bw = 0;
	mac_phy->ext_cha = EXTCHA_NOASSIGN;
	mac_phy->state = 0;
	OS_SEM_UNLOCK(&mac_phy->lock);
}

static int
hwifi_start(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct physical_device *ph_dev = (struct physical_device *)ad->physical_dev;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
	int ret;

#ifdef DOT11V_MBSSID_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
	BOOLEAN bWMDupBcn = TRUE;
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef FW_LOG_DUMP
	PFW_LOG_CTRL FwLogCtrl = &(ad->physical_dev->fw_log_ctrl);
	RTMP_STRING *fw_log_arg = FwLogCtrl->fw_log_arg;
#endif /* FW_LOG_DUMP */

	MTWF_DBG(ad, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_NOTICE, "\n");

	if (hwifi_get_inf_num(ad) == 0)
		HwCmdThreadInit(ad->physical_dev);


	mac_dev->ops->set_fw_mode(&mac_dev->hw, WM_CPU, ph_dev->TestModeEn);

	ret = mac_dev->ops->start(&mac_dev->hw);

	if (ret)
		goto err;

#ifdef FW_LOG_DUMP
	if (strlen(fw_log_arg)) {
		set_fw_log(ad, fw_log_arg);
		/* clear parameter */
		memset(fw_log_arg, 0, strlen(fw_log_arg));
	}
#endif /* FW_LOG_DUMP */

#ifdef DOT11V_MBSSID_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
	if (PD_GET_11V_BCN_DUP(ad->physical_dev))
		bWMDupBcn = FALSE; /* off wm 11v mbss bcn dup */

	MtUniCmdFwMbssBcnDup(ad, bWMDupBcn);
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* DOT11V_MBSSID_SUPPORT */

#ifndef CONFIG_CSO_SUPPORT
	MtUniCmdFwBasicConfig(ad);
#endif /* CONFIG_CSO_SUPPORT */

	/* update hw cap again due to hwifi update hw flags again. */
	ret = mtk_mac_set_cap(ad, &mac_dev->hw);
	if (ret)
		goto err;

	/*wifi system hook ops register*/
	ret = HWCtrlOpsReg(ad);
	if (ret)
		goto err;

#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheApply(ad, ad->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */

	/*mac init*/
	ret = WfMacInit(ad);

	if (ret)
		goto err;
	MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE, "MAC Init Done!\n");

	/*efuse system initial*/
	ret = WfEPROMSysInit(ad, !is_efuse_ready(ad), true);
	if (ret)
		goto err;

	hwifi_init_radio(ad);

#ifdef SINGLE_SKU_V2
	/* Load SKU table to Host Driver */
	RTMPSetSkuParam(ad);
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
	/* Load BF Backoff table to Host Driver */
	RTMPSetBackOffParam(ad);
#endif /* defined(MT_MAC) && defined(TXBF_SUPPORT) */
#endif /* SINGLE_SKU_V2 */
	MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_NOTICE, "EEPROM Init Done!\n");

	/* Only need to do once */
	if (hwifi_get_inf_num(ad) == 0) {
		MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_DEBUG,
				"1st interface up!\n");
		/* adjust mac tbl & wtbl per chip sku */
		if (ops->mactbl_wtbl_adjust) {
			ret = ops->mactbl_wtbl_adjust(ad);
			if (ret)
				goto err;
		}

#ifdef DOT11_EHT_BE
		if (PD_GET_MLO_CONFIG_OP_SET(ad->physical_dev)) {
			MTWF_DBG(ad, DBG_CAT_INIT, CATINTF_UP, DBG_LVL_DEBUG,
				"Set mlo config operation cmd to fw!\n");
			UniCmdMloConfigOperation(ad, MLO_CONFIG_STRBITMAP);
		}
		/* Config AAD LINK ADDR if MLO V1 enabled */
		UniCmdMloConfigOperation(ad, MLO_CONFIG_AADLINKADDR);
#endif /* DOT11_EHT_BE */
		physical_device_proc_handle_init(ad->physical_dev);
#ifndef RT_CFG80211_SUPPORT
		big_integer_lock_init();
#endif /* !RT_CFG80211_SUPPORT */
	}

	NdisZeroMemory(&ad->FragFrame, sizeof(FRAGMENT_FRAME));
	ad->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(ad, RX_BUFFER_NORMSIZE);
	return 0;
err:
	return ret;
}

static int
hwifi_stop(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	int ret;

	ret = mac_dev->ops->stop(&mac_dev->hw);
	if (ad->FragFrame.pFragPacket) {
		RELEASE_NDIS_PACKET(ad, ad->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		ad->FragFrame.pFragPacket = NULL;
	}
	if (ret)
		goto err;

	ret = WfEPROMSysExit(ad);
	if (ret)
		goto err;
	return 0;
err:
	return ret;
}

static int hwifi_update_wdev(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *)wdev->pHObj;

	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_WARN, "(caller:%pS), wdev(%d).\n",
			 OS_TRACE, wdev->wdev_idx);

	if (mac_bss) {
		wdev->OmacIdx = mac_bss->omac_idx;
		wdev->WmmIdx = mac_bss->wmm_idx;
		wdev->bss_info_argument.ucBssIndex = mac_bss->bss_idx;
	} else
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"mac_bss is NULL, wdev up_down_state is %d\n", wdev->if_up_down_state);
#ifdef GREENAP_SUPPORT
	greenap_suspend(ad, GREENAP_REASON_ACQUIRE_RADIO_FOR_WDEV);
#endif /* GREENAP_SUPPORT */

#ifdef EXT_BUILD_CHANNEL_LIST
	if (WMODE_CAP_6G(wdev->PhyMode))
		BuildChannelList(ad, wdev);
	else
		BuildChannelListEx(ad, wdev);
#else
	BuildChannelList(ad, wdev);
#endif
	UpdateDot11hForWdev(ad, wdev, TRUE);

	Set_Zwdfs_Proc(ad, wdev);

	/*re-init operation*/
	wlan_operate_init(wdev);

#ifdef GREENAP_SUPPORT
	greenap_resume(ad, GREENAP_REASON_ACQUIRE_RADIO_FOR_WDEV);
#endif /* GREENAP_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

static bool hwifi_init_rro_addr_elem_by_seid(struct _RTMP_ADAPTER *ad, u16 seid)
{
	bool ret = FALSE;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;

	ret = mac_dev->ops->init_rro_addr_elem_by_seid(hw, seid);
	return ret;
}

static bool
hwifi_if_type_sta(u32 wdev_type)
{
	bool sta_type = FALSE;

	if (wdev_type == WDEV_TYPE_STA ||
		wdev_type == WDEV_TYPE_ADHOC ||
		wdev_type == WDEV_TYPE_GC ||
		wdev_type == WDEV_TYPE_ATE_STA ||
		wdev_type == WDEV_TYPE_SERVICE_TXD)
		sta_type = TRUE;
	else
		sta_type = FALSE;

	return sta_type;
}

static int
hwifi_add_interface(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = NULL;
	int ret;

#ifdef DOT11_EHT_BE
#ifdef CONFIG_AP_SUPPORT
	struct _BSS_STRUCT *mbss = NULL;
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_EHT_BE */

	MTWF_DBG(ad, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_WARN, "(caller:%pS), wdev(%d,%s).\n",
			 OS_TRACE, wdev->wdev_idx, RtmpOsGetNetDevName(wdev->if_dev));

	mac_bss = kzalloc(sizeof(*mac_bss) + hw->bss_priv_size, GFP_KERNEL);
	if (!mac_bss)
		goto err_alloc_bss;

	mac_bss->if_cfg.mld_group_idx = MLD_GROUP_NONE;
	mac_bss->if_cfg.mld_type = MAC_MLD_TYPE_NONE;
#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(wdev->PhyMode)) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			mbss = &ad->ApCfg.MBSSID[wdev->func_idx];
			mac_bss->if_cfg.mld_group_idx = mbss->mld_grp_idx;
			// mac_bss->if_cfg.mld_type = mbss->mld_type;
			// NdisMoveMemory(mac_bss->if_cfg.mld_addr,
			// mbss->op_mld_addr, MAC_ADDR_LEN);
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (wdev->mld_dev) {
				mac_bss->if_cfg.mld_group_idx = wdev->mld_dev->mld_grp_idx;
				mac_bss->if_cfg.mld_type = MAC_MLD_TYPE_MULTI;
				NdisMoveMemory(mac_bss->if_cfg.mld_addr, wdev->mld_dev->mld_addr, MAC_ADDR_LEN);
			}
		}
	}
#endif /* DOT11_EHT_BE */
	NdisMoveMemory(mac_bss->if_cfg.if_addr, wdev->if_addr, MAC_ADDR_LEN);
	mac_bss->if_cfg.if_type = wdev->wdev_type;
	mac_bss->mac_phy = &mac_dev->mac_phy;
	ret = mac_dev->ops->add_interface(hw, mac_bss);
	if (ret)
		goto err_add_interface;

	wdev->pHObj = mac_bss;
	hwifi_update_wdev(wdev);
	return 0;
err_add_interface:
	MTWF_DBG(ad, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
		"create mac_bss succeed but func failed!\n");
	kfree(mac_bss);
err_alloc_bss:
	return -ENOMEM;
}

static int
hwifi_remove_interface(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *) wdev->pHObj;
	int ret;

	rcu_assign_pointer(wdev->pHObj, NULL);
	ret = mac_dev->ops->remove_interface(hw, mac_bss);
	if (ret)
		return ret;

	mt_synchronize_rcu();
	kfree(mac_bss);
	return 0;
}

static int
hwifi_add_bmc_sta(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, BOOLEAN isVlan)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct mtk_mac_sta *bmc_sta = NULL;
	struct _STA_TR_ENTRY *entry;
	bool if_type_sta = hwifi_if_type_sta(wdev->wdev_type);
	int ret, i;
	u32 sw_wcid = 0;

#ifdef SW_CONNECT_SUPPORT
	struct physical_device *ph_dev = ad->physical_dev;

	if (!ph_dev)
		return -EINVAL;
#endif /* SW_CONNECT_SUPPORT */

	if (if_type_sta)
		bmc_sta = kzalloc(sizeof(*bmc_sta) + hw->sta_priv_size * 2, GFP_KERNEL);
	else
		bmc_sta = kzalloc(sizeof(*bmc_sta) + hw->sta_priv_size, GFP_KERNEL);

	if (!bmc_sta)
		goto err_alloc_sta;

	for (i = 0; i < TXQ_NUM; i++) {
		ret = hwifi_txq_init(hw, mac_bss, bmc_sta, bmc_sta->txq, i);
		if (ret)
			goto err_init_txq;
	}

	bmc_sta->sta_type = MAC_STA_TYPE_GROUP;
	bmc_sta->mld_sta_idx = MLD_STA_NONE;

	ret = mac_dev->ops->add_sta(hw, mac_bss, bmc_sta);
	if (ret)
		goto err_add_sta;
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (!isVlan)
		wdev->tr_tb_idx = bmc_sta->link_wcid;
#else
	wdev->tr_tb_idx = bmc_sta->link_wcid;
#endif
	wdev->tr_tb_idx2 = if_type_sta ? bmc_sta->link_wcid2 : WCID_INVALID;

	sw_wcid = wdev->tr_tb_idx;

#ifdef SW_CONNECT_SUPPORT
	/* shift the bc tr_entry idx */
	if (hc_is_sw_sta_enable(ad)) {
		ret = os_idr_register(&(ph_dev->sw_group_mgmt), &(bmc_sta->sw_wcid), bmc_sta);

		if (ret) {
			/* S/W FULL */
			MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
				"!!!SW Group WTBL FULL!!! : band_idx=%d\n", hw->band_idx);
			goto err_invalid_idx;
		}

		if (bmc_sta->sw_wcid)
			sw_wcid = wdev->tr_tb_idx = bmc_sta->sw_wcid;

		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
			"(caller:%pS), wdev(%d). sw_wcid=%d, bmc_sta->link_wcid=%u\n",
			 OS_TRACE, wdev->wdev_idx, sw_wcid, bmc_sta->link_wcid);
	}
#endif /* SW_CONNECT_SUPPORT */

	if (bmc_sta->link_wcid >= WTBL_MAX_NUM(ad)) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"update wdev(%p) omac=%d, wcid=%d error\n",
			wdev, wdev->OmacIdx, bmc_sta->link_wcid);
		goto err_invalid_idx;
	}

	mac_bss->bmc_sta = bmc_sta;
	entry = tr_entry_get(ad, sw_wcid);
	entry->mac_sta = bmc_sta;
	entry->wcid = bmc_sta->link_wcid;
	entry->wcid2 = if_type_sta ? bmc_sta->link_wcid2 : WCID_INVALID;
	entry->wdev = wdev;
	entry->HwWcid = bmc_sta->link_wcid;

	return bmc_sta->link_wcid;
err_invalid_idx:
#ifdef SW_CONNECT_SUPPORT
	if (hc_is_sw_sta_enable(ad) && sw_wcid)
		os_idr_unregister(&(ph_dev->sw_group_mgmt), sw_wcid);
#endif /* SW_CONNECT_SUPPORT */
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (!isVlan)
		wdev->tr_tb_idx = WCID_INVALID;
#else
	wdev->tr_tb_idx = WCID_INVALID;
#endif
	wdev->tr_tb_idx2 = WCID_INVALID;
	mac_dev->ops->remove_sta(hw, mac_bss, bmc_sta);
err_add_sta:
	MTWF_DBG(ad, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
		"create bmc_sta succeed but func failed!\n");
err_init_txq:
	for (i -= 1; i >= 0; i--)
		kfree(bmc_sta->txq[i]);

	kfree(bmc_sta);
err_alloc_sta:
	return -ENOMEM;
}

static int
hwifi_remove_bmc_sta(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
	u16 sta_idx)
{
	int ret, i;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *) wdev->pHObj;
	struct _STA_TR_ENTRY *entry = tr_entry_get(ad, sta_idx);
	struct mtk_mac_sta *bmc_sta = entry->mac_sta;
#ifdef SW_CONNECT_SUPPORT
	struct physical_device *ph_dev = ad->physical_dev;

	if (hc_is_sw_sta_enable(ad) && ph_dev && bmc_sta->sw_wcid) {
		os_idr_unregister(&(ph_dev->sw_group_mgmt), bmc_sta->sw_wcid);

		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
			"(caller:%pS), wdev(%d). sta_idx=%u, sw_wcid=%d, bmc_sta->link_wcid=%u\n",
			 OS_TRACE, wdev->wdev_idx, sta_idx, bmc_sta->sw_wcid, bmc_sta->link_wcid);
	}
#endif /* SW_CONNECT_SUPPORT */

	rcu_assign_pointer(entry->mac_sta, NULL);
	rcu_assign_pointer(mac_bss->bmc_sta, NULL);
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (wdev->tr_tb_idx == sta_idx)
		wdev->tr_tb_idx = WCID_INVALID;
#else
	wdev->tr_tb_idx = WCID_INVALID;
#endif
	wdev->tr_tb_idx2 = WCID_INVALID;

	ret = mac_dev->ops->remove_sta(hw, mac_bss, bmc_sta);
	if (ret)
		goto err;

	mt_synchronize_rcu();

	for (i = 0; i < TXQ_NUM; i++)
		kfree(bmc_sta->txq[i]);

	kfree(bmc_sta);
	return 0;
err:
	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "remove sta(%u) fail!!\n", sta_idx);
	return -EINVAL;
}

static int
hwifi_add_mld(struct _RTMP_ADAPTER *ad, u32 mld_type, u32 mld_group_idx, u8 *mld_addr)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	int ret;

	if (!mld_addr) {
		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"Error: NULL MLD MAC addr (type:%d, idx:%d).\n",
			mld_type, mld_group_idx);
		ret = -EINVAL;
		goto err;
	}

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_NOTICE,
		"(caller:%pS) type(%d), idx(%d), addr(%pM).\n",
		OS_TRACE, mld_type, mld_group_idx, mld_addr);

	ret = mac_dev->ops->add_mld(hw, mld_type, mld_group_idx, mld_addr);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
hwifi_remove_mld(struct _RTMP_ADAPTER *ad, u32 mld_group_idx)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	int ret;

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_NOTICE,
		"(caller:%pS) idx(%d).\n", OS_TRACE, mld_group_idx);

	ret = mac_dev->ops->remove_mld(hw, mld_group_idx);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
hwifi_mld_add_link(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u32 mld_group_idx)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
	int ret;

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"(caller:%pS) if(%s), idx(%d).\n",
		OS_TRACE, RtmpOsGetNetDevName(wdev->if_dev), mld_group_idx);

	ret = mac_dev->ops->mld_add_link(hw, mac_bss, mld_group_idx);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static int
hwifi_mld_remove_link(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
	int ret;

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"(caller:%pS) if(%s).\n",
		OS_TRACE, RtmpOsGetNetDevName(wdev->if_dev));

	ret = mac_dev->ops->mld_remove_link(hw, mac_bss);
	if (ret)
		goto err;

	return 0;
err:
	return ret;
}

static bool
hwifi_radio_state(struct wifi_dev *wdev)
{
	struct mtk_mac_bss *mac_bss = wdev->pHObj;

	return (mac_bss && mac_bss->mac_phy) ? true : false;
}

static int
hwifi_radio_info(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct mtk_mac_bss *mac_bss = NULL;
	struct mtk_mac_phy *mac_phy = NULL;

	if (wdev == NULL || oper == NULL)
		return -1;

	mt_rcu_read_lock();
	mac_bss = rcu_dereference(wdev->pHObj);
	if (mac_bss != NULL) {
		mac_phy = rcu_dereference(mac_bss->mac_phy);
		if (mac_phy != NULL) {
			OS_SEM_LOCK(&mac_phy->lock);
			oper->bw = mac_phy->bw;
			oper->cen_ch_1 = mac_phy->cen_chan;
			oper->cen_ch_2 = mac_phy->chan2;
			oper->ext_cha = mac_phy->ext_cha;
			oper->prim_ch = mac_phy->chan;
			oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
			oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
			OS_SEM_UNLOCK(&mac_phy->lock);
		}
	}
	mt_rcu_read_unlock();
	return 0;
}

static BOOLEAN rcCheckIsTheSameBand(USHORT target_phymode, USHORT cur_phymode)
{
	    return (wmode_2_rfic(target_phymode) == wmode_2_rfic(cur_phymode));
}

static UCHAR get_cur_rfic_by_phymode(USHORT PhyMode)
{
	UCHAR rf_mode = 0;

	if (WMODE_CAP_6G(PhyMode))
		rf_mode = RFIC_6GHZ;
	else if (WMODE_CAP_5G(PhyMode))
		rf_mode = RFIC_5GHZ;
	else if (WMODE_CAP_2G(PhyMode))
		rf_mode = RFIC_24GHZ;

	return rf_mode;
}

static INT32 rcUpdatePhyMode(struct radio_dev *rdev, USHORT PhyMode)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	/*band is not changed or not*/
	if (rcCheckIsTheSameBand(PhyMode, pRadioCtrl->PhyMode)) {
		pRadioCtrl->PhyMode |= PhyMode;
	} else {
		/*band is changed*/
		pRadioCtrl->PhyMode = PhyMode;
	}
	pRadioCtrl->cur_rfic_type = get_cur_rfic_by_phymode(PhyMode);
	return ret;
}

#ifdef ZERO_PKT_LOSS_SUPPORT
/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPSuspendMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "wdev is NULL.\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO, "[%s] SCANNING, suspend MSDU transmission ...\n", wdev->if_dev->name);

	/*stop netif queue, to avoid packet from os*/
	RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
	/*give time to clear netif stack pkt buffer, else pkt drop*/
	udelay(500);

	for (i = 0; i < 3; i++) {
		MAC_TABLE_ENTRY *pEntry = NULL;

		if (pAd->ZeroLossSta[i].valid) {
			pEntry = MacTableLookup(pAd, pAd->ZeroLossSta[i].StaAddr);

			if (pEntry == NULL)
				continue;

			/*In case zero loss entry was created before station connection,
			 * update wcid in zero loss entry*/
			if (pEntry)
				pAd->ZeroLossSta[i].wcid = pEntry->wcid;

			if ((pAd->ZeroLossSta[i].wcid) && (!pAd->ZeroLossSta[i].ChnlSwitchSkipTx) &&
					(!AsicReadSkipTx(pAd, pAd->ZeroLossSta[i].wcid))) {
				if ((pEntry != NULL) && (pEntry->wdev != NULL) && (pEntry->wdev == wdev)) {
					MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
						"(): Tx Disable for wcid %d [%u ms]\n",
						pAd->ZeroLossSta[i].wcid, jiffies_to_msecs(jiffies));
					AsicUpdateSkipTx(pAd, pAd->ZeroLossSta[i].wcid, 1);
					//skip hw tx for wtbl entry
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO, "(): Tx Disable skipped for wcid %d\n", pAd->ZeroLossSta[i].wcid);
		}
	}
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPResumeMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "wdev is NULL.");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO, "[%s] SCAN done, resume MSDU transmission ...\n", wdev->if_dev->name);

	for (i = 0; i < 3; i++) {
		MAC_TABLE_ENTRY *pEntry = NULL;

		if (pAd->ZeroLossSta[i].valid) {
			pEntry = MacTableLookup(pAd, pAd->ZeroLossSta[i].StaAddr);

			if (pEntry == NULL)
				continue;

			if ((pAd->ZeroLossSta[i].wcid) && (!pAd->ZeroLossSta[i].ChnlSwitchSkipTx) &&
					(AsicReadSkipTx(pAd, pAd->ZeroLossSta[i].wcid))) {
				if ((pEntry->wdev != NULL) && (pEntry->wdev == wdev)) {
					MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
						"(): Tx enable for wcid %d [%u ms]\n",
						pAd->ZeroLossSta[i].wcid, jiffies_to_msecs(jiffies));
					AsicUpdateSkipTx(pAd, pAd->ZeroLossSta[i].wcid, 0);
					//resume hw tx for wtbl entry
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
					"(): Tx enable skipped for wcid %d\n", pAd->ZeroLossSta[i].wcid);
		}
	}

	RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
}

INT32 HcSuspendMSDUTx(struct _RTMP_ADAPTER *ad)
{
	INT32 i = 0, ret = 0;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct wifi_dev *wdev;

	if (!ad->Zero_Loss_Enable)
		return 0;
	/*update all of wdev*/
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];
		if (!wdev || !(wdev && wdev->wdev_type == WDEV_TYPE_AP))
			continue;
		RTMPSuspendMsduTransmission(ad, wdev);
	}
	ctrl->SuspendMsduTx = 1;
	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
				"():set suspended MSDU Tx\n");
	/*Disable Mac Level Tx enable = 0 */
	if (ad->ScanCtrl.Num_Of_Channels == 1) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
				"():set skip Mac Tx\n");
		UniCmdMacTxEnable(ad, 0, 0);
	}

	return ret;
}

INT32 HcUpdateMSDUTxAllow(struct _RTMP_ADAPTER *ad)
{
	INT32 i = 0, ret = 0;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct wifi_dev *wdev;
	BOOLEAN resume = 0;

	if (!ad->Zero_Loss_Enable)
		return 0;
	/*update all of wdev*/
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if (!wdev || !(wdev && wdev->wdev_type == WDEV_TYPE_AP))
			continue;
		if (ad->ScanCtrl.state != OFFCHANNEL_SCAN_START) {
			resume = 1;
			RTMPResumeMsduTransmission(ad, wdev);
		} else {
			if (ad->ScanCtrl.Num_Of_Channels == 1) {
				MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
					"%s():set skip Tx\n", __func__);
			}
			if (ctrl->SuspendMsduTx == 0)
				RTMPSuspendMsduTransmission(ad, wdev);
		}
	}
	if ((resume == 1) && (ctrl->SuspendMsduTx == 1)) {
		ctrl->SuspendMsduTx = 0;
		if (ad->ScanCtrl.Num_Of_Channels == 1) {
			MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
					"():set Mac Tx Enable\n");
			UniCmdMacTxEnable(ad, 1, 0);
		}
	}
	return ret;
}
#endif

static bool
hwifi_set_chan(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_ctrl *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct mtk_mac_phy *mac_phy = mac_bss->mac_phy;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(wdev->sys_handle);
	struct freq_oper *oper = res->oper;
	uint8_t scan = (res->reason == REASON_NORMAL_SCAN) ? TRUE:FALSE;
	int ret;
#if defined(ANTENNA_CONTROL_SUPPORT) || defined(ZERO_PKT_LOSS_SUPPORT)
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
#endif

	OS_SEM_LOCK(&mac_phy->lock);
	/*update to mac_phy*/
	mac_phy->chan = oper->prim_ch;
	mac_phy->chan2 = oper->cen_ch_2;
#ifdef DOT11_EHT_BE
	if (oper->bw == BW_320)
		mac_phy->cen_chan = oper->eht_cen_ch;
	else
#endif
		mac_phy->cen_chan = oper->cen_ch_1;
	mac_phy->bw = oper->bw;
	mac_phy->ext_cha = oper->ext_cha;
	mac_phy->reason = res->reason;
	ret = mac_dev->ops->config_phy(&mac_dev->hw, mac_phy);
	if (ret)
		goto err;
	OS_SEM_UNLOCK(&mac_phy->lock);
	rcUpdatePhyMode(&hdev_ctrl->rdev, wdev->PhyMode);
#ifdef ZERO_PKT_LOSS_SUPPORT
	ad->chan_switch_time[7] = jiffies_to_msecs(jiffies);
	/*before update channel suspend tx*/
	HcSuspendMSDUTx(wdev->sys_handle);
	ad->chan_switch_time[8] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	AsicSwitchChannel(wdev->sys_handle, mac_bss->band_idx, oper, scan);
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*after update channel resum tx*/
	HcUpdateMSDUTxAllow(wdev->sys_handle);
	ad->chan_switch_time[13] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	OS_SEM_LOCK(&mac_phy->lock);
#ifdef ANTENNA_CONTROL_SUPPORT
	if (ad->bAntennaSetAPEnable)
		mac_phy->rx_stream = ad->RxStream;
#endif
	OS_SEM_UNLOCK(&mac_phy->lock);
	return true;
err:
	OS_SEM_UNLOCK(&mac_phy->lock);
	return 0;
}

static void
hwifi_set_radio_state(struct wifi_dev *wdev, PHY_STATUS state)
{
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct mtk_mac_phy *mac_phy = mac_bss->mac_phy;

	OS_SEM_LOCK(&mac_phy->lock);
	mac_phy->state = state;
	OS_SEM_UNLOCK(&mac_phy->lock);
}

static void
hwifi_set_radio_state_by_channel(struct _RTMP_ADAPTER *ad, u8 channel, PHY_STATUS state)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_phy *mac_phy = &mac_dev->mac_phy;

	OS_SEM_LOCK(&mac_phy->lock);
	mac_phy->state = state;
	OS_SEM_UNLOCK(&mac_phy->lock);
}

static int
hwifi_set_wed_rmvl_per_sta(struct _RTMP_ADAPTER *ad, u16 wcid, u8 remove_vlan)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);

	return mac_dev->ops->set_pao_sta_info(&mac_dev->hw, wcid,
		WED_PAO_NUM_LEN_SET_TO_DEFAULT, WED_PAO_NUM_LEN_SET_TO_DEFAULT,
		remove_vlan, WED_PAO_HDRT_SET_TO_DEFAULT);
}


static int
hwifi_set_pn_check(struct _RTMP_ADAPTER *ad, u16 wcid, u16 se_id, u8 enable)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;

	return mac_dev->ops->set_pn_check(hw, wcid, se_id, enable);
}

static int
mcu_dest_trans(u16 mac_dest, u8 *dest)
{
	switch (mac_dest) {
	case HOST2N9:
		*dest = MAC_MCU_DEST_WM;
		break;
	case HOST2CR4:
		*dest = MAC_MCU_DEST_WA;
		break;
	case HOST2CR4N9:
		*dest = MAC_MCU_DEST_WA_WM;
		break;
	case HOST2WO:
	case HOST2ALL:
		*dest = HOST2WO;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
hwifi_mcu_tx(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	struct os_cookie *handle = ad->OS_Cookie;
	struct mtk_mac_dev *mac_dev = handle->mac_dev;
	struct _CMD_ATTRIBUTE *attr;
	struct sk_buff *skb;
	char *buf = NULL;
	int ret = NDIS_STATUS_SUCCESS;
	struct mtk_mac_mcu_msg mcu_msg = {0};

	if (msg == NULL || msg->net_pkt == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef WF_RESET_SUPPORT
	if (PD_GET_WF_RESET_IN_PROGRESS(ad->physical_dev) == TRUE)
		goto err;
#endif

	attr = &msg->attr;
	skb = msg->net_pkt;
	mcu_msg.cmd = attr->type,
	mcu_msg.ext_cmd = attr->ext_type,
	mcu_msg.data = skb->data,
	mcu_msg.len = skb->len,
	mcu_msg.is_wait = IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg),
	mcu_msg.action = IS_CMD_ATTR_SET_QUERY_FLAG_SET(msg->attr) ? MAC_MCU_ACT_SET : MAC_MCU_ACT_QUERY,
	mcu_msg.ack = ((IS_CMD_ATTR_NEED_FW_RSP_FLAG_SET(msg->attr)) && !(IS_CMD_MSG_NA_FLAG_SET(msg))) ? TRUE : FALSE,
#ifdef WIFI_UNIFIED_COMMAND
	mcu_msg.uni_cmd = IS_CMD_MSG_UNI_CMD_FLAG_SET(msg),
	mcu_msg.frag_num = msg->frag_num,
	mcu_msg.frag_total_num = msg->total_frag,
#else
	mcu_msg.uni_cmd = 0,
	mcu_msg.frag_num = 0,
	mcu_msg.frag_total_num = 0,
#endif /* WIFI_UNIFIED_COMMAND */

	/*TxPowerInfo show all rate power, the pkt length is 1262 for bellwether.*/
	os_alloc_mem(NULL, (UCHAR **)&buf, 2048);
	if (buf == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"can not allocate buf[1024]\n");
		goto err;
	}

#ifdef WIFI_UNIFIED_COMMAND
	if ((mcu_msg.cmd == UNI_CMD_ID_EFUSE_CONTROL) || (mcu_msg.cmd == UNI_CMD_ID_BF) || (mcu_msg.cmd == UNI_CMD_ID_TESTMODE_CTRL))
		mcu_msg.timeout = MCU_CMD_SPECIAL_TIMEOUT;
	else if (mcu_msg.cmd == UNI_CMD_ID_EEPROM_CONTROL)
		mcu_msg.timeout = MCU_CMD_EEPROM_TIMEOUT;
	else
		mcu_msg.timeout = MCU_CMD_TIMEOUT;
#endif /* WIFI_UNIFIED_COMMAND */

	mcu_msg.rx_data = buf;

	ret = mcu_dest_trans(attr->mcu_dest, &mcu_msg.dest);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"(ret=%d):cmd=0x%x,ext_cmd=0x%x"
		",is_wait=%d,ack=%d,dest=%d,act=%d,frag=%d/%d,uni=%d\n",
		ret, mcu_msg.cmd, mcu_msg.ext_cmd, mcu_msg.is_wait,
		mcu_msg.ack, mcu_msg.dest, mcu_msg.action, mcu_msg.frag_num,
		mcu_msg.frag_total_num, mcu_msg.uni_cmd);

	if (ret)
		goto err;

	WLAN_HOOK_CALL(WLAN_HOOK_TX_CMD_TO_HWIFI, ad, &mcu_msg);

	ret = mac_dev->ops->mcu_tx(&mac_dev->hw, &mcu_msg);

	if (ret) {
#ifdef WF_RESET_SUPPORT
		AndesStatusCheck(ad, ret);
#endif
		if (ret == -ETIMEDOUT && attr->rsp.to_handler)
			attr->rsp.to_handler(msg);
		goto err;
	}

	if (attr->rsp.handler && mcu_msg.rx_data)
		attr->rsp.handler(msg, mcu_msg.rx_data, mcu_msg.rx_len);

	if (buf)
		os_free_mem(buf);

	RTMPFreeNdisPacket(ad, msg->net_pkt);
	AndesFreeCmdMsg(msg);

	return ret;
err:
	if (buf)
		os_free_mem(buf);

	RTMPFreeNdisPacket(ad, msg->net_pkt);
	AndesFreeCmdMsg(msg);

#ifdef ERR_RECOVERY
	{
		/* skip cmd timeout in SER period */
		UINT32 curr_time = ser_get_time();

		MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				"SER_T0=%u\n", ser_get_t0(ad));
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				"curr_time=%u\n", curr_time);
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				"curr_time-SER_T0=%d (ms)\n",
				(curr_time - ser_get_t0(ad)));
		#define SER_TIMEOUT_ALLOWANCE 5000

		if ((curr_time - ser_get_t0(ad)) < (SER_TIMEOUT_ALLOWANCE)) {
			MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
					"Skip send cmd fail in SER period!! (%s)!!\n",
					RtmpOsGetNetDevName(ad->net_dev));
			return ret;
		}
	}
#endif /* ERR_RECOVERY */

	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
		"send cmd fail! ret %d\n", ret);
	return ret;
}

static int
hwifi_clear_hw_ops(struct _RTMP_ADAPTER *ad)
{
	return 0;
}

static u8
hwifi_get_radio_state(struct wifi_dev *wdev)
{
	PHY_STATUS ret = PHY_IDLE;
	struct mtk_mac_bss *bss = NULL;

	mt_rcu_read_lock();
	bss = rcu_dereference(wdev->pHObj);
	if (bss == NULL) {
		ret = PHY_RADIOOFF;
	} else {
		if (bss->mac_phy)
			ret = PHY_INUSE;
		else
			ret = PHY_RADIOOFF;
	}
	mt_rcu_read_unlock();
	return ret;
}

static u8
hwifi_get_link(struct wifi_dev *wdev)
{
	struct mtk_mac_bss *bss = wdev->pHObj;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev((struct _RTMP_ADAPTER *)wdev->sys_handle);

	if (bss)
		return bss->band_idx;

	return mac_dev->hw.band_idx;
}

static int
hwifi_set_link(struct wifi_dev *wdev, uint32_t band_idx)
{
	struct mtk_mac_dev *mac_dev =
		pad_to_mac_dev((struct _RTMP_ADAPTER *)wdev->sys_handle);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *) wdev->pHObj;
	int ret = NDIS_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"mac_bss(%p) %d => %d\n",
		mac_bss, mac_bss ? mac_bss->band_idx : -100, band_idx);

	if (mac_bss) {
		struct mtk_mac_bss_conf info;

		info.u.band_idx = band_idx;
		mac_dev->ops->change_bss(hw, mac_bss,
			&info, MAC_BSS_CHANGE_BAND);

		ret = hwifi_update_wdev(wdev);
		wdev->DevInfo.BandIdx = band_idx;
	}

	return ret;
}

static int
hwifi_get_bssidx(struct wifi_dev *wdev)
{
	struct mtk_mac_bss *bss = NULL;
	u32 bss_idx = 0;

	mt_rcu_read_lock();
	bss = rcu_dereference(wdev->pHObj);
	if (bss != NULL)
		bss_idx = bss->bss_idx;

	mt_rcu_read_unlock();

	return bss_idx;
}

static struct mtk_mac_dev *
hwifi_get_mac_dev(struct _RTMP_ADAPTER *ad)
{
	return pad_to_mac_dev(ad);
}

static struct mtk_mac_dev *
hwifi_get_mac_dev_by_bss(struct _RTMP_ADAPTER *ad, u8 bss_idx)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_dev *expected_mac_dev = NULL;
	struct mtk_mac_hw *expected_hw = NULL;

	expected_hw = mac_dev->ops->get_mac_hw(hw, bss_idx);
	if (expected_hw)
		expected_mac_dev = hw_to_mac_dev(expected_hw);
	else
		expected_mac_dev = pad_to_mac_dev(ad);

	return expected_mac_dev;
}

static u32
hwifi_get_wmmidx(struct wifi_dev *wdev)
{
	struct mtk_mac_bss *bss = NULL;
	u8 wmm_idx = 0;

	mt_rcu_read_lock();
	bss = rcu_dereference(wdev->pHObj);
	if (bss != NULL)
		wmm_idx =  bss->wmm_idx;

	mt_rcu_read_unlock();

	return wmm_idx;
}

static u32
hwifi_get_omac(struct wifi_dev *wdev)
{
	struct mtk_mac_bss *bss = NULL;
	u32 omac_idx = 0;

	mt_rcu_read_lock();
	bss = rcu_dereference(wdev->pHObj);
	if (bss != NULL)
		omac_idx =  bss->omac_idx;

	mt_rcu_read_unlock();

	return omac_idx;
}

static int
hwifi_get_bss_mld(struct wifi_dev *wdev, struct bss_mld *mld)
{
	struct mtk_mac_bss *bss = NULL;

	mt_rcu_read_lock();
	bss = rcu_dereference(wdev->pHObj);
	if (bss != NULL) {
		mld->mld_addr_idx = bss->mld_addr_idx;
		mld->mld_group_addr_idx = bss->mld_group_addr_idx;
		mld->mld_remap_idx = bss->mld_remap_idx;
	} else {
		mt_rcu_read_unlock();
		return -EPERM;
	}
	mt_rcu_read_unlock();

	return 0;
}

static int
hwifi_get_peer_mld(struct _RTMP_ADAPTER *ad, u16 wcid, struct peer_mld *mld)
{
	int ret = 0;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct _STA_TR_ENTRY *sta_tr_entry = tr_entry_get(ad, wcid);
	struct mtk_mac_sta *mac_sta = sta_tr_entry->mac_sta;

	if (!mac_sta) {
		ret = -EINVAL;
		goto err;
	}

	if (mac_dev->ops->get_mld_id)
		ret = mac_dev->ops->get_mld_id(hw, mac_sta);
	else
		ret = -EPERM;

	if (ret)
		goto err;

	mld->mld_primary_idx = mac_sta->mld_primary_idx;
	mld->mld_secondary_idx = mac_sta->mld_secondary_idx;

	return ret;
err:
	return ret;
}

static u8
hwifi_get_hw_band_idx(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;

	return hw->band_idx;
}

u16
hwifi_get_inf_num(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);

	return mac_dev->ops->get_inf_num(&mac_dev->hw);
}

void
hwifi_get_tx_token_num(struct _RTMP_ADAPTER *ad, u16 tx_token_num[], u8 max_src_num)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;

	if (mac_dev->ops->get_tx_token_num)
		mac_dev->ops->get_tx_token_num(hw, tx_token_num, max_src_num);
}

void
hwifi_get_rro_sp_page_num(struct _RTMP_ADAPTER *ad, u32 *page_num)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;

	if (mac_dev->ops->get_rro_sp_page_num)
		mac_dev->ops->get_rro_sp_page_num(hw, page_num);
}

void
hwifi_get_fw_info(struct _RTMP_ADAPTER *pAd, int mcu_type, char *fw_ver, char *build_date, char *fw_ver_long)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);

	if (mac_dev->ops->get_fw_info)
		mac_dev->ops->get_fw_info(&mac_dev->hw, mcu_type, fw_ver, build_date, fw_ver_long);
}

struct _RTMP_CHIP_CAP *
hwifi_get_chip_cap(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	ctrl = ad->hdev_ctrl;
	if (ctrl == NULL) {
		MTWF_PRINT("%s: hdev_ctrl is NULL\n", __func__);
		goto err;
	}
	cap = hc_get_chip_cap(ctrl);
	if (cap == NULL) {
		MTWF_PRINT("%s: cap is NULL\n", __func__);
		goto err;
	}

err:
	return cap;
}

void
hwifi_update_edca(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _EDCA_PARM *pEdca, bool reset)
{
	struct _EDCA_PARM *edca = NULL;

	edca = hwifi_get_edca(ad, wdev);
	if (edca) {
		if (reset)
			os_zero_mem(edca, sizeof(*edca));
		else
			os_move_mem(edca, pEdca, sizeof(*edca));
	}
}

void
hwifi_set_edca(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct mtk_mac_bss *mac_bss = wdev->pHObj;
	struct _EDCA_PARM *edca = hwifi_get_edca(ad, wdev);

	struct wmm_entry entry = {
		.dbdc_idx = mac_bss->band_idx,
		.ref_cnt = 1,
		.tx_mode = ctrl->HwResourceCfg.txcmd_mode, /*TXCMD mode*/
		.wmm_set = mac_bss->wmm_idx,
	};

	if (edca == NULL)
		return;

	os_move_mem(&entry.edca, edca, sizeof(*edca));
	entry.edca.bValid = true;

	AsicSetEdcaParm(ad, &entry, wdev);
}

struct _EDCA_PARM *
hwifi_get_edca(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = NULL;
	struct mtk_mac_bss *mac_bss = (struct mtk_mac_bss *) wdev->pHObj;
	struct wmm_entry *entry = NULL;

	ctrl = ad->hdev_ctrl;
	if (ctrl == NULL) {
		MTWF_PRINT("%s: hdev_ctrl is NULL\n", __func__);
		goto err;
	}

	if (mac_bss) {
		entry = wmm_ctrl_get_entry_by_idx(ctrl, mac_bss->wmm_idx);

		if (entry)
			return &entry->edca;
		else
			return NULL;
	}

err:
	return NULL;
}

static u32
hwifi_change_setup_link_sta(struct _RTMP_ADAPTER *ad, uint16_t wcid)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct _STA_TR_ENTRY *sta_tr_entry = tr_entry_get(ad, wcid);
	struct mtk_mac_sta *mac_sta = sta_tr_entry->mac_sta;

	if (!mac_sta)
		return 0;

	if (mac_dev->ops->change_setup_link_sta)
		return mac_dev->ops->change_setup_link_sta(&mac_dev->hw, mac_sta);

	return 0;
}

static struct mtk_hdev_ops hwifi_ops = {
	.start = hwifi_start,
	.stop = hwifi_stop,
	.add_interface = hwifi_add_interface,
	.remove_interface = hwifi_remove_interface,
	.add_bmc_sta = hwifi_add_bmc_sta,
	.remove_bmc_sta = hwifi_remove_bmc_sta,
	.add_mld = hwifi_add_mld,
	.remove_mld = hwifi_remove_mld,
	.mld_add_link = hwifi_mld_add_link,
	.mld_remove_link = hwifi_mld_remove_link,
	.add_sta = hwifi_add_sta,
	.remove_sta = hwifi_remove_sta,
	.radio_state = hwifi_radio_state,
	.radio_info = hwifi_radio_info,
	.set_chan = hwifi_set_chan,
	.mcu_tx = hwifi_mcu_tx,
	.set_hw_ops = WfSysPreInit,
	.clear_hw_ops = hwifi_clear_hw_ops,
	.get_radio_state = hwifi_get_radio_state,
	.get_link = hwifi_get_link,
	.set_link = hwifi_set_link,
	.get_bssidx = hwifi_get_bssidx,
	.get_mac_dev = hwifi_get_mac_dev,
	.get_mac_dev_by_bss = hwifi_get_mac_dev_by_bss,
	.set_radio_state = hwifi_set_radio_state,
	.set_radio_state_by_channel = hwifi_set_radio_state_by_channel,
	.get_wmm_idx = hwifi_get_wmmidx,
	.get_omac = hwifi_get_omac,
	.get_bss_mld = hwifi_get_bss_mld,
	.get_peer_mld = hwifi_get_peer_mld,
	.get_hw_band_idx = hwifi_get_hw_band_idx,
	.update_wdev = hwifi_update_wdev,
	.init_rro_addr_elem_by_seid = hwifi_init_rro_addr_elem_by_seid,
	.set_wed_rmvl_per_sta = hwifi_set_wed_rmvl_per_sta,
	.set_wed_pn_check = hwifi_set_pn_check,
	.change_setup_link_sta = hwifi_change_setup_link_sta,
};

static VOID hwifi_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	int ret = -EINVAL;

	if (mac_dev->ops->bus_io_read)
		ret = mac_dev->ops->bus_io_read(&mac_dev->hw, reg, val);

	if (ret)
		*val = 0;
}

static VOID hwifi_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	int ret;

	if (mac_dev->ops->bus_io_write)
		ret = mac_dev->ops->bus_io_write(&mac_dev->hw, reg, val);
}

static VOID hwifi_map_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	int ret = -EINVAL;

	if (mt_mac_cr_range_mapping(ad->physical_dev, &reg) == TRUE) {
		if (mac_dev->ops->bus_io_read)
			ret = mac_dev->ops->bus_io_read(&mac_dev->hw, reg, val);

		if (ret)
			*val = 0;
	} else if (ops->hif_io_remap_read32) {
		ops->hif_io_remap_read32(hdev_ctrl, reg, val);
	}
}

static VOID hwifi_map_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	int ret;

	if (mt_mac_cr_range_mapping(ad->physical_dev, &reg) == TRUE) {
		if (mac_dev->ops->bus_io_write)
			ret = mac_dev->ops->bus_io_write(&mac_dev->hw, reg, val);
	} else if (ops->hif_io_remap_write32) {
		ops->hif_io_remap_write32(hdev_ctrl, reg, val);
	}
}

/*temporal used, MAC driver should not read/write CR directly*/
static void hwifi_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	ops->hif_io_read32 = hwifi_io_read32;
	ops->hif_io_write32 = hwifi_io_write32;
	/*mac*/
	ops->mac_io_read32 = hwifi_map_io_read32;
	ops->mac_io_write32 = hwifi_map_io_write32;
	/*phy*/
	ops->phy_io_read32 = hwifi_io_read32;
	ops->phy_io_write32 = hwifi_io_write32;
	/*hw*/
	ops->hw_io_read32 = hwifi_map_io_read32;
	ops->hw_io_write32 = hwifi_map_io_write32;
	/*mcu*/
	ops->mcu_io_read32 = CmdIORead32;
	ops->mcu_io_write32 = CmdIOWrite32;
}

void
hwifi_update_hw_cap(void *ph_dev_obj, unsigned long hw_flags, void *mac_cap_info)
{
	RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ph_dev_obj);
	struct mtk_mac_cap_info *mac_cap = mac_cap_info;

	if (test_bit(MAC_SW_AMSDU, &hw_flags)) {
		cap->qm = GENERIC_QM;
		/* XX = hw->cap_info.sw_max_amsdu_num; */
		/* XX = hw->cap_info.sw_max_amsdu_len; */
	}

	if (test_bit(MAC_HW_RRO, &hw_flags)) {
		cap->asic_caps |= fASIC_CAP_HW_RRO;
		/* XX = hw->cap_info.rro_mode; */
	}
	if (test_bit(MAC_MULTI_BUS, &hw_flags))
		cap->asic_caps |= fASIC_CAP_TWO_PCIE;

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
		"hw_flags :%lx, asic_caps :%x, qm type :%d SW AMSDU num :%d ln :%d\n",
		hw_flags, cap->asic_caps, cap->qm,
		mac_cap->sw_max_amsdu_num, mac_cap->sw_max_amsdu_len);
}

int
hwifi_tx_mgmt(struct _RTMP_ADAPTER *ad, UCHAR *tmac_info,
	struct _MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct sk_buff *skb = RTPKT_TO_OSPKT(tx_blk->pPacket);
	struct wifi_dev *wdev = tx_blk->wdev;
	struct _STA_TR_ENTRY *entry = tx_blk->tr_entry;
	struct mtk_mac_sta *sta;
	struct mtk_mac_txq *txq;
	int ret;

	if (!entry || !entry->mac_sta)
		entry = tr_entry_get(ad, wdev->tr_tb_idx);

	mt_rcu_read_lock();
	sta = entry->mac_sta;
	if (!sta)
		goto err;

	txq = sta->txq[MGMT_TXQ];
	if (!txq)
		goto err;

	skb_pull(skb, cap->tx_hw_hdr_len);

	/*should apply rcu lock*/
	tx_blk->wifi_hdr_len = info->hdr_len;
	tx_blk->dot11_type = info->Type;
	tx_blk->dot11_subtype = info->SubType;
	tx_blk->Pid = info->PID;
	tx_blk->QueIdx = info->q_idx;
	tx_blk->UserPriority = info->TID;
	if (RTMP_GET_PACKET_WCID(tx_blk->pPacket) != 0)
		/* we shall use unicast wcid for any unicast frame. */
		tx_blk->Wcid = RTMP_GET_PACKET_WCID(tx_blk->pPacket);

	if (info->BM)
		tx_blk->TxFrameType = TX_BMC_FRAME;
	if (info->Ack)
		TX_BLK_SET_FLAG(tx_blk, fTX_bAckRequired);
	if (info->addba)
		TX_BLK_SET_FLAG(tx_blk, fTX_bAddBA);
	if (info->txpwr_offset)
		tx_blk->txpwr_offset = info->txpwr_offset;

	/*
	 * info->prot is assigned in PMF_PerformTxFrameAction()
	 * info->prot == 1 indicates it is a UNICAST_ROBUST_FRAME
	 * info->prot == 2 indicates it is a GROUP_ROBUST_FRAME and set MMIE flag
	 * info->prot == 3 indicates it is a GROUP_ROBUST_FRAME and
					driver encap mmie success in PMF_EncapBIPAction()
	 */
	if (info->prot != 1)
		SET_CIPHER_NONE(tx_blk->CipherAlg);

	if (info->prot == 2)
		TX_BLK_SET_FLAG(tx_blk, fTX_bMMIE);

	if ((tx_blk->dot11_type == FC_TYPE_DATA)
		&& (RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX)) {
		tx_blk->QueIdx = TxQ_IDX_AC1;
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO,
			"Send data frame, wcid=%d, QueIdx=%d\n",
			tx_blk->Wcid, tx_blk->QueIdx);
	}

	if (info->PsmBySw)
		TX_BLK_SET_FLAG2(tx_blk, fTX_bPsmBySw);

	WLAN_HOOK_CALL(WLAN_HOOK_TX_DRIVER_TO_HWIFI, ad, tx_blk);

	ret = mac_dev->ops->tx_queue(&mac_dev->hw, txq, tx_blk);

	mt_rcu_read_unlock();
	return ret;
err:
	mt_rcu_read_unlock();
	MTWF_DBG(ad, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
		"tx fail!\n");
	return -EINVAL;
}

int
hwifi_tx_data(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct wifi_dev *wdev = tx_blk->wdev;
	struct _STA_TR_ENTRY *entry = tx_blk->tr_entry;
	struct mtk_mac_bss *bss;
	struct mtk_mac_sta *sta;
	struct mtk_mac_txq *txq;
	int ret;

	if (!wdev || !entry)
		goto err;

	mt_rcu_read_lock();
	bss = wdev->pHObj;
	sta = entry->mac_sta;

	if (!sta || !bss)
		goto err;

	if (tx_blk->UserPriority < TXQ_NUM)
		txq = sta->txq[tx_blk->UserPriority];
	else
		txq = sta->txq[0];

	if (!txq)
		goto err;

	WLAN_HOOK_CALL(WLAN_HOOK_TX_DRIVER_TO_HWIFI, ad, tx_blk);

	ret = mac_dev->ops->tx_queue(&mac_dev->hw, txq, tx_blk);
	mt_rcu_read_unlock();
	return ret;
err:
	mt_rcu_read_unlock();
	MTWF_DBG(ad, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR, "tx fail!\n");
	return -EINVAL;
}

void
hwifi_show_mac_cap(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_cap_info *cap_info = &hw->cap_info;

	MTWF_PRINT("mac cap information:\n");
	MTWF_PRINT("\thif_txd_ver_sdo = %d\n", cap_info->hif_txd_ver_sdo);
	MTWF_PRINT("\trx_path_type = %d\n", cap_info->rx_path_type);
	MTWF_PRINT("\trro_bypass_type = %d\n", cap_info->rro_bypass_type);
	MTWF_PRINT("\ttxfreedone_path = %d\n", cap_info->txfreedone_path);
}

int
hwifi_init_hw_ring_setting(struct _RTMP_ADAPTER *ad)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct mtk_mac_hw *hw = &mac_dev->hw;
	struct mtk_mac_cap_info *mac_cap_info = &hw->cap_info;
	int ret = NDIS_STATUS_SUCCESS;

	/* Only need to do once */
	if (hwifi_get_inf_num(ad) != 0)
		return ret;

	if (test_bit(MAC_CHIP_OPTION, &hw->flags)) {
#ifdef WIFI_UNIFIED_COMMAND
		ret = uni_cmd_rro_rx_path(ad, mac_cap_info->rx_path_type);
		MTWF_PRINT("%s(%d): ret=0x%x, rx_path_type = %d\n",
			__func__, __LINE__, ret, mac_cap_info->rx_path_type);

		ret = uni_cmd_rro_bypass_mode(ad, mac_cap_info->rro_bypass_type);
		MTWF_PRINT("%s(%d): ret=0x%x, rro_bypass_type = %d\n",
			__func__, __LINE__, ret, mac_cap_info->rro_bypass_type);

		/* hw->cap_info.mld_dest_type; *//* TODO: may not need. @20220809 */

		ret = uni_cmd_rro_tx_free_done_path(ad, mac_cap_info->txfreedone_path);
		MTWF_PRINT("%s(%d): ret=0x%x, txfreedone_path = %d\n",
			__func__, __LINE__, ret, mac_cap_info->txfreedone_path);
#endif /* WIFI_UNIFIED_COMMAND */
	}

	/* XX = hw->cap_info.hif_txd_ver_sdo; */
	ret = MtCmdCr4Set(ad, WA_SET_OPTION_HW_PATH_HIF_VER, mac_cap_info->hif_txd_ver_sdo, 0);
	MTWF_PRINT("%s(%d): ret=0x%x, hif_txd_ver_sdo = %d\n",
		__func__, __LINE__, ret, mac_cap_info->hif_txd_ver_sdo);
	return ret;
}

int
hwifi_ser_handler(struct _RTMP_ADAPTER *pAd, u32 action, u32 status)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);

	return mac_dev->ops->ser_handler(&mac_dev->hw, action, status);
}

int
hwifi_dbg_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);

	if (!mac_dev->ops->get_dbg_info) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"%s(): null pointer: get_dbg_info\n", __func__);
		return -EOPNOTSUPP;
	}

	return mac_dev->ops->get_dbg_info(&mac_dev->hw, arg);
}

void
hwifi_core_ops_register(struct hdev_ctrl *ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(ctrl);

	ops->cmd_thread = RTPCICmdThread;
#ifdef CONFIG_STA_SUPPORT
	ops->sta_sleep_auto_wakeup = RT28xxPciStaAsicSleepAutoWakeup;
	ops->sta_wakeup = RT28xxPciStaAsicWakeup;
#endif /*CONFIG_STA_SUPPORT*/

	hc_set_hdev_ops(ctrl, &hwifi_ops);
	hwifi_io_ops_init(ctrl);
}

void
hwifi_core_ops_unregister(struct hdev_ctrl *ctrl)
{

}

static int
mtk_mac_ba_offload_trig_event(struct mtk_mac_hw *hw, u16 wcid, u8 tid)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;

	ba_ori_session_start(ad, wcid, tid);
	return 0;
}

static struct mtk_mac_sw_ops sw_ops = {
	.ba_trig_event = mtk_mac_ba_offload_trig_event,
};

struct mtk_mac_hw *
mtk_mac_alloc_hw(size_t size, struct mtk_mac_hw_ops *hw_ops)
{
	int ret;
	struct os_cookie *handle = NULL;
	struct _RTMP_ADAPTER *ad;
	struct mtk_mac_dev *mac_dev;
	struct mtk_mac_hw *hw;
	int priv_sz = ALIGN(sizeof(struct os_cookie), 4) +
		ALIGN(sizeof(struct mtk_mac_dev), 4) + size;

	os_alloc_mem(NULL, (UCHAR **)&handle, priv_sz);
	if (handle == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
				"can not allocate memory for handle!!\n");
		return NULL;
	}
	os_zero_mem(handle, priv_sz);
	handle->mac_dev = (struct mtk_mac_dev *) ((char *) handle + ALIGN(sizeof(*handle), 4));

	ret = RTMPAllocAdapterBlock(handle, (void **)&ad, RTMP_DEV_INF_HWIFI);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	/*assign value*/
	mac_dev = handle->mac_dev;
	mac_dev->ad = ad;
	mac_dev->ops = hw_ops;
	hw = &mac_dev->hw;
	hw->sw_ops = &sw_ops;
	hw->priv = ((u8 *) mac_dev) + ALIGN(sizeof(struct mtk_mac_dev), 4);
	return hw;
err:
	if (ad)
		RTMPFreeAdapter(ad);
	if (handle)
		os_free_mem(handle);
	return NULL;
}
EXPORT_SYMBOL(mtk_mac_alloc_hw);

void
mtk_mac_free_hw(struct mtk_mac_hw *hw)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	struct os_cookie *cookie;
	struct physical_device *ph_dev;

	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"ad is null. check.\n");
		return;
	}
	cookie = ad->OS_Cookie;
	ph_dev = ad->physical_dev;
	rcu_assign_pointer(cookie->mac_dev, NULL);
	rcu_assign_pointer(mac_dev->ad, NULL);
	rcu_assign_pointer(mac_dev->ops, NULL);
	mt_synchronize_rcu();
	RtmpRaDevCtrlExit(ad);
	physical_device_remove_main_device(ph_dev);
}
EXPORT_SYMBOL(mtk_mac_free_hw);

int
mtk_mac_rx_napi(struct mtk_mac_hw *hw,
	struct mtk_mac_sta *sta, struct sk_buff *skb)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	struct _RX_BLK rx_blk;
	int ret;

	os_zero_mem(&rx_blk, sizeof(struct _RX_BLK));
	ret = asic_trans_rxd_into_rxblk(ad, &rx_blk, skb);

	if (ret == 0)
		goto err;

	if (header_packet_process(ad, skb, &rx_blk) != NDIS_STATUS_SUCCESS)
		goto err;

	/* TODO: call netif_gro_receive for this napi instead */

	return rx_packet_process(ad, skb, &rx_blk);
err:
	dev_kfree_skb(skb);
	return 0;
}
EXPORT_SYMBOL(mtk_mac_rx_napi);

int
mtk_mac_rx(struct mtk_mac_hw *hw,
	struct mtk_mac_sta *sta, struct sk_buff *skb, struct _RX_BLK *rx_blk)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;

#ifdef WF_RESET_SUPPORT
	if (PD_GET_WF_RESET_IN_PROGRESS(ad->physical_dev) == TRUE)
		goto err;
#endif

#ifdef SNIFFER_RADIOTAP_SUPPORT
	if (MONITOR_ON(ad)) {
		announce_802_11_radiotap_packet(ad, skb, rx_blk);
		return 0;
	}
#endif
	if (fill_rx_pkt_info(ad, rx_blk))
		goto err;

	/* TODO: call netif_gro_receive for this napi instead */

	return rx_packet_process(ad, skb, rx_blk);

err:
	dev_kfree_skb(skb);
	return 0;
}
EXPORT_SYMBOL(mtk_mac_rx);

int
mtk_mac_rx_ics(struct mtk_mac_hw *hw, struct sk_buff *skb)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;

	ad = physical_device_get_first_up_mac_adapter(ad->physical_dev);
	if (ad == NULL)
		ad = mac_dev->ad;
#ifdef FW_LOG_DUMP
	dbg_log_wrapper(ad, DBG_LOG_PKT_TYPE_ICS, (uint8_t *)skb, 0);
#endif /* FW_LOG_DUMP */

	dev_kfree_skb(skb);

	return 0;
}
EXPORT_SYMBOL(mtk_mac_rx_ics);

int
mtk_mac_rx_unsolicited_event(
	struct mtk_mac_hw *hw,
	struct sk_buff *skb)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	EVENT_RXD *event_rxd;
	UCHAR *rxd;

	rxd = (UCHAR *)skb->data;
	event_rxd = (EVENT_RXD *)rxd;

	WLAN_HOOK_CALL(WLAN_HOOK_RX_EVENT_FROM_HWIFI, ad, skb);

	UnsolicitedEventHandler(ad,
		GET_EVENT_FW_RXD_EID(event_rxd),
		GET_EVENT_FW_RXD_EXT_EID(event_rxd),
		GET_EVENT_HDR_ADDR(skb),
		GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd), event_rxd);

	return 0;
}
EXPORT_SYMBOL(mtk_mac_rx_unsolicited_event);

int
mtk_mac_tx_status(struct mtk_mac_hw *hw, struct mtk_tx_status *tx_status)
{
#ifdef HWIFI_TXSTATS_SUPPORT
#if defined(EAP_STATS_SUPPORT) || defined(CFG_RED_SUPPORT) || defined(DABS_QOS)
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
#endif
#endif /* HWIFI_TXSTATS_SUPPORT */
#ifdef EAP_STATS_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
	ULONG now_time;
#endif

	if (!tx_status) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"invalid tx_status!\n");
		return -1;
	}

#ifdef HWIFI_TXSTATS_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
			 "===> tx_status=%p\n", tx_status);

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
			 "wcid[%d] qid[%d] from band:%d\n",
			 tx_status->wcid, tx_status->qid, tx_status->band_idx);
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
			 "stat:0x%x, tx times:%d, air delay:%d, transmit delay:%d\n",
			 tx_status->stat, tx_status->cnt,
			 tx_status->air_latency, tx_status->mac_latency);

#if defined(EAP_STATS_SUPPORT) || defined(CFG_RED_SUPPORT) || defined(DABS_QOS)
	if (VALID_UCAST_ENTRY_WCID(ad, tx_status->wcid)) {
#ifdef EAP_STATS_SUPPORT
		pEntry = entry_get(ad, tx_status->wcid);

		CalStaTxLatency(pEntry, tx_status->mac_latency,
				tx_status->air_latency, tx_status->cnt);
		if (tx_status->stat == 1) {//hw drop
			pEntry->mpdu_xretries.QuadPart++;
			pEntry->txm.fail_cnt++;
		}
		pEntry->txm.total_cnt++;

		NdisGetSystemUpTime(&now_time);
		if (RTMP_TIME_AFTER(now_time, pEntry->txm.last_calc_time + 300 / (1000 / OS_HZ))) {
			if (pEntry->txm.fail_cnt == pEntry->txm.total_cnt) {
				pEntry->txm.per_err_times += 1;
				pEntry->txm.contd_fail_cnt += pEntry->txm.fail_cnt;
				MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_DEBUG,
						"pEntry->wcid %d, pEntry->txm.per_err_times: %d, pEntry->txm.contd_fail_cnt: %ld\n",
						pEntry->wcid, pEntry->txm.per_err_times, pEntry->txm.contd_fail_cnt);
			} else {
				pEntry->txm.per_err_times = 0;
				pEntry->txm.contd_fail_cnt = 0;
			}

			pEntry->txm.fail_cnt = 0;
			pEntry->txm.total_cnt = 0;
			pEntry->txm.last_calc_time = now_time;
		}
#endif
#ifdef CFG_RED_SUPPORT
		acm_tx_cnt_update(ad, tx_status->qid, tx_status->wcid, 1,
				tx_status->stat);
#endif
#ifdef DABS_QOS
		dabs_latency_update(ad, tx_status->qid, tx_status->wcid, tx_status->mac_latency,
			tx_status->air_latency, tx_status->cnt);
#endif

	}
#endif /* EAP_STATS_SUPPORT || CFG_RED_SUPPORT || DABS_QOS */
#endif

#ifdef CONFIG_WIFI_SYSDVT
	adm_dvt_tx_status(tx_status);
#endif

	return 0;
}
EXPORT_SYMBOL(mtk_mac_tx_status);

int
mtk_mac_txs_handler(void *hw_dev, struct txs_info_t txs_info)
{
	struct physical_device *physical_dev;

	if (!hw_dev)
		return -1;

	physical_dev = physical_device_find_physical_dev(hw_dev);
	if (!physical_dev)
		return -1;

	TxsReceiveHandler(physical_dev, txs_info);

	return 0;
}
EXPORT_SYMBOL(mtk_mac_txs_handler);

void mtk_mac_dequeue_by_token(struct mtk_mac_hw *hw, UCHAR idx)
{
	struct mtk_mac_dev *mac_dev;
	struct _RTMP_ADAPTER *ad;
	struct tm_ops *tm_ops;

	if (!hw)
		return;

	mac_dev = hw_to_mac_dev(hw);

	ad = mac_dev->ad;
	if (!ad)
		return;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(ad->physical_dev, idx));
	tm_ops = PD_GET_TM_QM_OPS(ad->physical_dev);
	if ((tm_ops) &&
		((PD_GET_QM_FP_SWQ_LEN(ad->physical_dev, idx) +
		PD_GET_QM_FP_POST_SWQ_LEN(ad->physical_dev, idx)) > 0))
		tm_ops->schedule_task(ad, TX_DEQ_TASK, idx);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(ad->physical_dev, idx));
}
EXPORT_SYMBOL(mtk_mac_dequeue_by_token);

static mtk_pcie_soft_off_t mtk_pcie_soft_off;
static mtk_pcie_soft_on_t mtk_pcie_soft_on;
static mtk_trigger_whole_chip_reset_t mtk_trigger_whole_chip_reset;
static mtk_lookup_symbol_t mtk_lookup_symbol;

static int save_state(struct pci_dev *pdev, void *arg)
{
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
		"save state for pdev[%p]\n", pdev);
	return pci_save_state(pdev);
}

static int restore_state(struct pci_dev *pdev, void *arg)
{
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
		"restore state for pdev[%p]\n", pdev);
	pci_restore_state(pdev);
	return 0;
}

static bool process_pci_bus(struct pci_bus *bus, int op)
{
	int ret = 0;

	if (op == PCI_SAVE_STATE) {
		mt_pci_walk_bus(bus, save_state, NULL);
		ret = mtk_pcie_soft_off(bus);
		if (ret) {
			/* failure, roll-back */
			MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
				"call external func: mtk_pcie_soft_off fail, ret = %d\n", ret);
			mt_pci_walk_bus(bus, restore_state, NULL);
			return FALSE;
		}
	} else if (op == PCI_RESTORE_STATE) {
		ret = mtk_pcie_soft_on(bus);
		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
				"call external func: mtk_pcie_soft_on fail, ret = %d\n", ret);
			mt_pci_walk_bus(bus, restore_state, NULL);
			return FALSE;
		}
		mt_pci_walk_bus(bus, restore_state, NULL);
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"pci_state op[%d] error\n", op);
		return FALSE;
	}
	return TRUE;
}

static bool handle_pci_state(int op)
{
	int index;
	struct pci_bus *bus;

	for (index = 0; index < MAX_PCI_NUM; index++) {
		bus = pci_find_bus(index, 0);
		if (bus) {
			if (!process_pci_bus(bus, op)) {
				MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
					"process_pci_bus fail\n");
				return FALSE;
			}
		}
	}
	return TRUE;
}

static void *lookup_and_log(const char *name)
{
	void *func = (void *)mtk_lookup_symbol(name);

	if (func)
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
			"find %s function\n", name);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"failed to find %s function\n", name);
	return func;
}

static bool find_chip_reset_symbol(void)
{
	mtk_pcie_soft_off =
		(mtk_pcie_soft_off_t)lookup_and_log("mtk_pcie_soft_off");
	mtk_pcie_soft_on =
		(mtk_pcie_soft_on_t)lookup_and_log("mtk_pcie_soft_on");
	mtk_trigger_whole_chip_reset =
		(mtk_trigger_whole_chip_reset_t)lookup_and_log("mtk_trigger_whole_chip_reset");

	return mtk_pcie_soft_off && mtk_pcie_soft_on && mtk_trigger_whole_chip_reset;
}

static bool find_kernel_lookup_symbol(void)
{
#if (KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE)
	mtk_lookup_symbol = NULL;
#else
	mtk_lookup_symbol = mt_kallsyms_lookup_name;
#endif
	if (mtk_lookup_symbol) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_INFO,
				 "find mtk_lookup_symbol function\n");
		return TRUE;
	}
	return FALSE;
}

int mtk_mac_chip_reset(unsigned int chip_id)
{
	/* 0. find kernel api: kallsyms_lookup_name */
	if (!find_kernel_lookup_symbol()) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"get kernel lookup symbol fail\n");
		return -1;
	}

	/* 1. find chip reset symbol */
	if (!find_chip_reset_symbol()) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"get chip reset symbol fail\n");
		return -1;
	}

	/* 2. save pcie config space and trigger pcie link to L2 */
	if (!handle_pci_state(PCI_SAVE_STATE)) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"handle_pci_state(PCI_SAVE_STATE) fail\n");
		return -1;
	}

	/* 3. trigger whole chip reset */
	if (!mtk_trigger_whole_chip_reset(chip_id)) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"call external func: mtk_trigger_whole_chip_reset fail and restore pci state\n");
		handle_pci_state(PCI_RESTORE_STATE);
		return -1;
	}

	/* 4. pcie re-linkup and restore pcie config space */
	if (!handle_pci_state(PCI_RESTORE_STATE)) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"handle_pci_state(PCI_RESTORE_STATE) fail\n");
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(mtk_mac_chip_reset);

struct net_device *g_netdev;

#ifdef CFG_MTK_SUPPORT_ANDROID_WMT
static int hifAxiProbe(void)
{
	struct net_device *prNetDev = g_netdev;
	struct _RTMP_ADAPTER *pAd;
	int ret = 0;

	GET_PAD_FROM_NET_DEV(pAd, prNetDev);
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "\n");

	if (VIRTUAL_IF_INIT(pAd, prNetDev) != 0)
		return -1;
	if (VIRTUAL_IF_UP(pAd, prNetDev) != 0)
		return -1;

	pAd->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;
#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	consys_log_sync_status(pAd);
	consys_log_sync_time(pAd);
#endif

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,
		"ret: %d\n", ret);
	return ret;
}

static int hifAxiRemove(void)
{
	struct net_device *prNetDev = g_netdev;
	struct _RTMP_ADAPTER *pAd;

	GET_PAD_FROM_NET_DEV(pAd, prNetDev);
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "\n");

	if (prNetDev && pAd) {
		VIRTUAL_IF_DOWN(pAd, prNetDev);
		VIRTUAL_IF_DEINIT(pAd, prNetDev);
	}

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "\n");

	return 0;
}

static void register_conninfra_cb(RTMP_ADAPTER *pAd)
{
	struct MTK_WCN_WLAN_CB_INFO rWlanCb;

	rWlanCb.wlan_probe_cb = hifAxiProbe;
	rWlanCb.wlan_remove_cb = hifAxiRemove;
	mtk_wcn_wlan_reg(&rWlanCb);
}

static void unregister_conninfra_cb(RTMP_ADAPTER *pAd)
{
	mtk_wcn_wlan_unreg();
}
#endif

int
mtk_mac_rx_uni_unsolicited_event(
	VOID *physical_dev,
	struct sk_buff *skb)
{
#ifdef WIFI_UNIFIED_COMMAND
	WLAN_HOOK_CALL(WLAN_HOOK_RX_EVENT_FROM_HWIFI, NULL, skb);

	AndesMTRxProcessUniEvent(physical_dev, skb);
#endif /* WIFI_UNIFIED_COMMAND */

	return 0;
}
EXPORT_SYMBOL(mtk_mac_rx_uni_unsolicited_event);

int
mtk_mac_hw_register(void *hw_dev, struct mtk_mac_hw *hw)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_OS_NETDEV_OP_HOOK_ net_dev_ops;
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	struct net_device *net_dev = NULL;
	int ret = -ENOMEM;
#ifdef MTK_FE_RESET_RECOVER
	struct physical_device *device;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl;
	struct mtk_notifier_block *mtk_nb;
#endif

	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
		"hw_dev(=%p), chip=(0x%x), ad(=%p), band_idx(=%d)!\n",
		hw_dev, hw->chip_id, ad, hw->band_idx);
	if (physical_device_add_mac_adapter(hw_dev, hw) == FALSE) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
			"Fail to add mac adapter to the device list!\n");
		goto err;
	}

#ifdef MTK_FE_RESET_RECOVER
	device = ad->physical_dev;
	pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	mtk_nb = &pErrRecoveryCtrl->mtk_nb;
#endif /* MTK_FE_RESET_RECOVER */

	/*assign hw information, before RaDevCtrlInit*/
	ad->ChipID = hw->chip_id;
	/*init mac driver*/
	ad->infType = RTMP_DEV_INF_HWIFI;
	ad->band_idx = hw->band_idx;
	RtmpRaDevCtrlInit(ad, RTMP_DEV_INF_HWIFI);
	mtk_mac_set_cap(ad, hw);
	/*init main device*/
	net_dev = RtmpPhyNetDevInit(ad, &net_dev_ops);

	if (!net_dev)
		goto err;

	/*for avoid linux network manager rename device name ingnore set netdev parent in here*/
	/*SET_NETDEV_DEV(net_dev, hw->pdev);*/
	ret = RtmpOSNetDevAttach(ad->OpMode, net_dev, &net_dev_ops);

	if (ret) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR,
			"failed to call RtmpOSNetDevAttach(), rv=%d!\n", ret);
		goto err;
	}

#ifdef MTK_FE_RESET_RECOVER
	if (mtk_nb->priv == NULL)
		mtk_fe_reset_notifier_init(device);
#endif

	g_netdev = net_dev;

#ifdef CFG_MTK_SUPPORT_ANDROID_WMT
#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	consys_log_init(ad);
#endif /* CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH */
	register_conninfra_cb(ad);
#endif /* CFG_MTK_SUPPORT_ANDROID_WMT */



	return 0;
err:
	if (net_dev)
		RtmpOSNetDevFree(net_dev);
	return ret;
}
EXPORT_SYMBOL(mtk_mac_hw_register);

void
mtk_mac_hw_unregister(struct mtk_mac_hw *hw)
{
	struct mtk_mac_dev *mac_dev = hw_to_mac_dev(hw);
	struct _RTMP_ADAPTER *ad = mac_dev->ad;
	struct net_device *net_dev = ad->net_dev;
#ifdef MTK_FE_RESET_RECOVER
	struct physical_device *device = ad->physical_dev;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	struct mtk_notifier_block *mtk_nb = &pErrRecoveryCtrl->mtk_nb;

	if (mtk_nb->priv == ad)
		mtk_fe_reset_notifier_exit(device);
#endif

	MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_NOTICE,
		"chip=(0x%x), ad(=%p)!\n", hw->chip_id, ad);

#ifdef CFG_MTK_SUPPORT_ANDROID_WMT
	unregister_conninfra_cb(ad);
#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	consys_log_uninit(ad);
#endif /* CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH */
#endif /* CFG_MTK_SUPPORT_ANDROID_WMT */

	RtmpPhyNetDevExit(ad, net_dev);
#ifndef RT_CFG80211_SUPPORT
	RtmpOSNetDevFree(net_dev);
#endif
}
EXPORT_SYMBOL(mtk_mac_hw_unregister);

int
mtk_rx_ser_event(
	uint32_t chip_id,
	uint32_t ser_level,
	uint32_t ser_event,
	uint32_t hw_id)
{
#ifdef ERR_RECOVERY
	return ser_event_enq(chip_id, ser_level, ser_event, hw_id);
#else
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_HWIFI, DBG_LVL_ERROR, "ERR_RECOVERY OFF now.\n");
	return NDIS_STATUS_FAILURE;
#endif
}
EXPORT_SYMBOL(mtk_rx_ser_event);
