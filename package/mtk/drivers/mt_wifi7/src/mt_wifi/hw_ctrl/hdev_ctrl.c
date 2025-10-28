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
/*
* local function
*/


/*
 *
*/
/*Only this function can use pAd*/
INT32 hdev_ctrl_init(RTMP_ADAPTER *pAd, INT type)
{
	struct hdev_ctrl  *ctrl = NULL;
	UINT32  ret;

	ret  =  os_alloc_mem(NULL, (UCHAR **)&ctrl, sizeof(struct hdev_ctrl));

	if (ctrl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"Allocate Hardware device Configure  fail!!\n");
		return -1;
	}

	os_zero_mem(ctrl, sizeof(struct hdev_ctrl));
	ctrl->priv  = (VOID *)pAd;
	pAd->hdev_ctrl = (VOID *)ctrl;

	hif_core_ops_register(ctrl, type);
	return 0;
}

/*
 *
*/
VOID hdev_ctrl_exit(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	hif_core_ops_unregister(pAd->hdev_ctrl, pAd->infType);
	/*exist hw resource*/
	hdev_resource_exit(ctrl);
	/*exist hdevcfg*/
	pAd->hdev_ctrl = NULL;
	os_free_mem(ctrl);
}

int
hc_get_bssidx(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (ctrl->hdev_ops && ctrl->hdev_ops->get_bssidx)
		return ctrl->hdev_ops->get_bssidx(wdev);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return -1;
	}
}

struct mtk_mac_dev *
hc_get_mac_dev(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!ad)
		return NULL;

	ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
	if (ctrl->hdev_ops && ctrl->hdev_ops->get_mac_dev)
		return ctrl->hdev_ops->get_mac_dev(ad);
	else
		return NULL;
}

struct mtk_mac_dev *
hc_get_mac_dev_by_bssidx(struct _RTMP_ADAPTER *ad, u8 bss_idx)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!ad)
		return NULL;

	ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
	if (ctrl->hdev_ops && ctrl->hdev_ops->get_mac_dev_by_bss)
		return ctrl->hdev_ops->get_mac_dev_by_bss(ad, bss_idx);
	else
		return NULL;
}

bool hc_init_rro_addr_elem_by_seid(PRTMP_ADAPTER ad, u16 seid)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

#ifdef WF_RESET_SUPPORT
	if (PD_GET_WF_RESET_IN_PROGRESS(ad->physical_dev) == TRUE)
		return TRUE;
#endif

	if (ctrl->hdev_ops && ctrl->hdev_ops->init_rro_addr_elem_by_seid)
		return ctrl->hdev_ops->init_rro_addr_elem_by_seid(ad, seid);
	else
		return FALSE;
}

int hc_set_wed_rmvl_per_sta(PRTMP_ADAPTER ad, u16 wcid, u8 remove_vlan)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!ad)
		return 0;

	ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
	if (ctrl->hdev_ops && ctrl->hdev_ops->set_wed_rmvl_per_sta)
		return ctrl->hdev_ops->set_wed_rmvl_per_sta(ad, wcid, remove_vlan);
	else
		return 0;
}

int hc_set_wed_pn_check(PRTMP_ADAPTER ad, u16 wcid, u16 se_id, u8 enable)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!ad)
		return 0;

	ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
	if (ctrl->hdev_ops && ctrl->hdev_ops->set_wed_pn_check)
		return ctrl->hdev_ops->set_wed_pn_check(ad, wcid, se_id, enable);
	else
		return 0;
}

INT32 AndesSendCmdMsg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	UINT32 result = 0;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;


#ifdef ERR_RECOVERY
	if (IsStopingPdma(&ad->ErrRecoveryCtl)) {
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"SER Period,Command type = %x, Extension command type = %x\n",
			msg->attr.type, msg->attr.ext_type);
		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}
#endif /* ERR_RECOVERY */

	if (in_interrupt() && IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "BUG: called from invalid context\n");
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "Command type = %x, Extension command type = %x\n",
			  msg->attr.type, msg->attr.ext_type);
		ASSERT(0);
		AndesForceFreeCmdMsg(msg);
	return NDIS_STATUS_FAILURE;
}
	if (ctrl->hdev_ops && ctrl->hdev_ops->mcu_tx) {
		result = ctrl->hdev_ops->mcu_tx(ad, msg);
		if (result != NDIS_STATUS_SUCCESS) {
			chip_dbg = hc_get_chip_dbg(ctrl);

			if (chip_dbg->show_cmd_timeout_info)
				chip_dbg->show_cmd_timeout_info(ad);
		}
		return result;
	}
	else {
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return NDIS_STATUS_FAILURE;
	}
}

/*
*
*/
VOID hdev_resource_init(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = ctrl->priv;

	if (ctrl->hdev_ops && ctrl->hdev_ops->set_hw_ops)
		ctrl->hdev_ops->set_hw_ops(ad);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
	HdevHwResourceInit(ctrl);
}

/*
*
*/
int hdev_resource_exit(struct hdev_ctrl *ctrl)
{
	struct _RTMP_ADAPTER *ad = ctrl->priv;

	if (ctrl->hdev_ops && ctrl->hdev_ops->clear_hw_ops)
		ctrl->hdev_ops->clear_hw_ops(ad);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
	HdevHwResourceExit(ctrl);
	return 0;
}

/*
*
*/
BOOLEAN rc_radio_equal(struct mtk_mac_phy *mac_phy, struct radio_res *res)
{
	struct freq_oper *oper = res->oper;

	/* if current action is for scan, always allow to switch channel */
	if (res->reason == REASON_NORMAL_SCAN)
		return FALSE;

	OS_SEM_LOCK(&mac_phy->lock);

	/*if the reason changes, need do phy updating*/
	if (mac_phy->reason != res->reason)
		goto err;

	/* if phy not active yet, need do phy updating */
	if (!mac_phy->state)
		goto err;

	if (mac_phy->chan != oper->prim_ch)
		goto err;

	if (mac_phy->bw != oper->bw)
		goto err;

	if (mac_phy->cen_chan != oper->cen_ch_1)
		goto err;

	if (mac_phy->chan2 != oper->cen_ch_2)
		goto err;

	if (mac_phy->ext_cha != oper->ext_cha)
		goto err;

	OS_SEM_UNLOCK(&mac_phy->lock);
	return TRUE;

err:
	OS_SEM_UNLOCK(&mac_phy->lock);
	return FALSE;
}


uint8_t
hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct mtk_mac_bss *mac_bss = NULL;
	struct mtk_mac_phy *mac_phy = NULL;
#if defined(ANTENNA_CONTROL_SUPPORT) || defined(ZERO_PKT_LOSS_SUPPORT) || defined(DFS_ADJ_BW_ZERO_WAIT)
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(ad, wdev);
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
	PDFS_PARAM pDfsParam = &ad->CommonCfg.DfsParameter;
#endif

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID)
		return TRUE;

	mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
	if (!mac_bss) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"wdev not ready, mac_bss is NULL!\n");
		return FALSE;
	}
	mac_phy = mac_bss->mac_phy;
	if (!mac_phy) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"Error! mac_phy is NULL!\n");
		return FALSE;
	}
	if (rc_radio_equal(mac_phy, res)) {
#ifdef ANTENNA_CONTROL_SUPPORT
		if (ad->bAntennaSetAPEnable && (mac_phy->rx_stream != res->oper->rx_stream))
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"current action for Antenna, always allow to switch channel!\n");
		else
#endif
		{
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"radio is equal, prim_ch=%d, bw=%d, rx_stream=%d!\n",
				res->oper->prim_ch, res->oper->bw, res->oper->rx_stream);
#ifdef ZERO_PKT_LOSS_SUPPORT
			if (ScanCtrl->state == OFFCHANNEL_SCAN_START) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN, "%s(): Same channel. Suspending TX!\n", __func__);
				HcSuspendMSDUTx(ad);
			}
			if (ScanCtrl->state == OFFCHANNEL_SCAN_COMPLETE) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN, "%s(): Same channel.Resuming TX!\n", __func__);
				HcUpdateMSDUTxAllow(ad);
			}
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
			if (!(pDfsParam->BW160ZeroWaitState == DFS_BW80_TX80RX80))
#endif
				return TRUE;
		}
	}

	if (ctrl->hdev_ops && ctrl->hdev_ops->set_chan)
		return ctrl->hdev_ops->set_chan(wdev, res);
	else {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return FALSE;
	}
}

/*
*
*/
uint16_t
HcAcquireGroupKeyWcid(RTMP_ADAPTER *ad, struct wifi_dev *wdev, BOOLEAN isVlan)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	uint16_t ret;
	struct _STA_TR_ENTRY *entry;
	int wcid;
	if (!HcIsRadioAcq(wdev))
		return WCID_INVALID;
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (wdev->tr_tb_idx == WCID_INVALID) {
		if (ctrl->hdev_ops && ctrl->hdev_ops->add_bmc_sta) {
			ret = ctrl->hdev_ops->add_bmc_sta(ad, wdev, isVlan);
			if (ret)
				return WCID_INVALID;
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"no hook api, please check.\n");
			return WCID_INVALID;
		}
	} else if (isVlan) {
		if (ctrl->hdev_ops && ctrl->hdev_ops->add_bmc_sta) {
			wcid = ctrl->hdev_ops->add_bmc_sta(ad, wdev, isVlan);
			ret = (uint16_t)wcid;
			if (wcid <= 0)
				return WCID_INVALID;
			else
				return ret;
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"no hook api, please check.\n");
			return WCID_INVALID;
		}
	}

	entry = tr_entry_get(ad, wdev->tr_tb_idx);
	/* wdev->tr_tb_idx is sw_wcid, need to return hw_wcid */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
	"wdev_idx=%d, tr_tb_idx=%u, wcid=%u\n", wdev->wdev_idx, wdev->tr_tb_idx, (entry ? entry->wcid : 0));

	if (entry)
		return entry->wcid;
	else
		return wdev->tr_tb_idx;
#else
	if (wdev->tr_tb_idx == WCID_INVALID) {
		if (ctrl->hdev_ops && ctrl->hdev_ops->add_bmc_sta) {
			wcid = ctrl->hdev_ops->add_bmc_sta(ad, wdev, isVlan);
			ret = (uint16_t)wcid;
			if (wcid <= 0)
				return WCID_INVALID;
			else
				return ret;
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"no hook api, please check.\n");
			return WCID_INVALID;
		}
	}

	entry = tr_entry_get(ad, wdev->tr_tb_idx);
	/* wdev->tr_tb_idx is sw_wcid, need to return hw_wcid */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
	"wdev_idx=%d, tr_tb_idx=%u, wcid=%u\n", wdev->wdev_idx, wdev->tr_tb_idx, (entry ? entry->wcid : 0));

	if (entry)
		return entry->wcid;
	else
		return wdev->tr_tb_idx;
#endif
}

uint16_t
HcAcquireGroupKeyWcid2(RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	if (!HcIsRadioAcq(wdev))
		return WCID_INVALID;

	return wdev->tr_tb_idx2;
}

/*
*
*/
INT
hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (!HcIsRadioAcq(wdev))
		return HC_STATUS_FAIL;

	if (ctrl->hdev_ops && ctrl->hdev_ops->radio_info)
		return ctrl->hdev_ops->radio_info(wdev, oper);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}

/*
*
*/
uint8_t HcIsRadioAcq(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (!ctrl)
		return FALSE;

	if (ctrl->hdev_ops && ctrl->hdev_ops->radio_state)
		return ctrl->hdev_ops->radio_state(wdev);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return FALSE;
	}
}

/*
*
*/
INT32 hw_start(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->start)
		return ctrl->hdev_ops->start(ad);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return NDIS_STATUS_FAILURE;
	}
}

/*
*
*/
INT32 hw_stop(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->stop)
		return ctrl->hdev_ops->stop(ad);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return NDIS_STATUS_FAILURE;
	}
}

/*
*
*/
INT32 HcAcquireRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->add_interface)
		return (ctrl->hdev_ops->add_interface(pAd, wdev) == 0
						? HC_STATUS_OK : HC_STATUS_FAIL);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}

/*
*
*/
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->remove_interface)
		return ctrl->hdev_ops->remove_interface(pAd, wdev);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return 0;
	}
}

#ifdef DOT11_EHT_BE
/*
*
*/
INT32 HcAcquireMldRsc(RTMP_ADAPTER *pAd, u32 mld_type, u32 mld_group_idx, u8 *mld_addr)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->add_mld)
		return (ctrl->hdev_ops->add_mld(pAd, mld_type, mld_group_idx, mld_addr) == 0
						? HC_STATUS_OK : HC_STATUS_FAIL);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}

/*
*
*/
INT32 HcReleaseMldRsc(RTMP_ADAPTER *pAd, u32 mld_group_idx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->remove_mld)
		return (ctrl->hdev_ops->remove_mld(pAd, mld_group_idx) == 0
						? HC_STATUS_OK : HC_STATUS_FAIL);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}

/*
*
*/
INT32 HcAcquireMldLinkRsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, u32 mld_group_idx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->mld_add_link)
		return (ctrl->hdev_ops->mld_add_link(pAd, wdev, mld_group_idx) == 0
						? HC_STATUS_OK : HC_STATUS_FAIL);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}

/*
*
*/
INT32 HcReleaseMldLinkRsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->mld_remove_link)
		return (ctrl->hdev_ops->mld_remove_link(pAd, wdev) == 0
						? HC_STATUS_OK : HC_STATUS_FAIL);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return HC_STATUS_FAIL;
	}
}
#endif /* DOT11_EHT_BE */

UCHAR HcGetBandByWdev(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (ctrl && ctrl->hdev_ops && ctrl->hdev_ops->get_link)
		return ctrl->hdev_ops->get_link(wdev);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return 0;
	}
}

/*
*
*/
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS state)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (!HcIsRadioAcq(wdev))
		return;

	if (ctrl->hdev_ops && ctrl->hdev_ops->set_radio_state)
		ctrl->hdev_ops->set_radio_state(wdev, state);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
}

int HcGetBssMld(struct wifi_dev *wdev, struct bss_mld *mld)
{
	int ret = 0;
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (ctrl->hdev_ops && ctrl->hdev_ops->get_bss_mld)
		ret = ctrl->hdev_ops->get_bss_mld(wdev, mld);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"not support !\n");
		ret = -1;
	}

	return ret;
}

/*
*
*/
int hc_get_peer_mld(RTMP_ADAPTER *ad, u16 wcid, struct peer_mld *mld)
{
	int ret = 0;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (ctrl->hdev_ops && ctrl->hdev_ops->get_peer_mld)
		ret = ctrl->hdev_ops->get_peer_mld(ad, wcid, mld);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"not support !\n");
		ret = -1;
	}

	return ret;
}

uint8_t hc_update_wdev(struct wifi_dev *wdev)
{
	int ret = 0;
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (ctrl->hdev_ops && ctrl->hdev_ops->update_wdev)
		ret = ctrl->hdev_ops->update_wdev(wdev);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"not support !\n");

		ret = 0;
	}

	return ret;
}


#ifdef DOT11_HE_AX
struct pe_control *hc_get_pe_ctrl(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct pe_control *pe_ctrl = NULL;

	if (HcIsRadioAcq(wdev))
		pe_ctrl = &ctrl->HwResourceCfg.PhyCtrl.pe_ctrl;
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"obj is not ready!!\n");
	return pe_ctrl;
}
#endif

/*
*
*/
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *ad, UCHAR channel, PHY_STATUS state)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (ctrl->hdev_ops)
		ctrl->hdev_ops->set_radio_state_by_channel(ad, channel, state);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = &ctrl->rdev;
	rdev->pRadioCtrl->CurStat = PHY_RADIOOFF;
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = &ctrl->rdev;
	rdev->pRadioCtrl->CurStat = PHY_INUSE;
}

/*
*
*/
u8 HcGetRadioCurStat(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (ctrl->hdev_ops)
		return ctrl->hdev_ops->get_radio_state(wdev);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hook api, please check.\n");
		return 0;
	}
}

/*
*
*/
uint8_t IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev)
{
	if (!HcIsRadioAcq(wdev)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"wdev_idx %d obj is not ready, return TRUE !!\n",
			wdev->wdev_idx);
		return TRUE;
	}

	if (HcGetRadioCurStat(wdev) == PHY_RADIOOFF)
		return TRUE;
	else
		return FALSE;
}

/*
*
*/
uint8_t IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	uint8_t AllSupportedBandsRadioOff = TRUE;

	rdev = &ctrl->rdev;

	if ((rdev->pRadioCtrl->CurStat == PHY_INUSE)
		&& (rdev->pRadioCtrl->CurStat != PHY_RADIOOFF)) {
		AllSupportedBandsRadioOff = FALSE;
	}

	return AllSupportedBandsRadioOff;
}

#ifdef GREENAP_SUPPORT
/*
*
*/
VOID HcSetGreenAPActive(
	RTMP_ADAPTER *pAd, BOOLEAN bGreenAPActive)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return;

	rdev = &ctrl->rdev;

	if (!rdev)
		return;

	rdev->pRadioCtrl->bGreenAPActive = bGreenAPActive;
}

/*
*
*/
BOOLEAN IsHcGreenAPActive(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return FALSE;

	rdev = &ctrl->rdev;

	if (!rdev)
		return FALSE;

	return rdev->pRadioCtrl->bGreenAPActive;
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
INT32 hc_radio_init(struct _RTMP_ADAPTER *pAd, UCHAR rfic, UCHAR dbdc_mode)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	rc_radio_init(ctrl, rfic, dbdc_mode);

	return ret;
}

INT32 hc_radio_exit(struct _RTMP_ADAPTER *pAd, UCHAR dbdc_mode)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	rc_radio_exit(ctrl, dbdc_mode);

	return ret;
}

/*
*	return: TRUE(trigger bpcc update suc)/FALSE(not trigger bpcc update)
*/
BOOLEAN prepare_bpcc_csa(RTMP_ADAPTER *pAd)
{
	BOOLEAN ret = FALSE;
	struct wifi_dev *wdev, *mlo_dev = NULL;
	struct DOT11_H *pDot11h = NULL, *pTmpDot11h = NULL;
	UCHAR i = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
	pDot11h = &pAd->Dot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"\x1b[41m pAd->Dot11_H is NULL !!\x1b[m\n");
		pAd->ApCfg.set_ch_async_flag = FALSE;
		return FALSE;
	}

#ifdef ZERO_PKT_LOSS_SUPPORT
	if (pAd->Zero_Loss_Enable)
		pAd->Dot11_H.ChnlSwitchState = SET_CHANNEL_COMMAND;
#endif /*ZERO_PKT_LOSS_SUPPORT*/

	/* loop all wdev to update csa in beacon */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev == NULL || !wdev->if_up_down_state || !WDEV_WITH_BCN_ABILITY(wdev))
			continue;

		pTmpDot11h = wdev->pDot11_H;

		if (pTmpDot11h == NULL)
			continue;

		if (pTmpDot11h->ChannelMode != CHAN_SILENCE_MODE) {

			pTmpDot11h->wdev_count++;
			pAd->ApCfg.set_ch_async_flag = TRUE;
#ifdef ZERO_PKT_LOSS_SUPPORT
			if (pAd->Zero_Loss_Enable)
				wdev->csa_count = pTmpDot11h->CSPeriod - 1;
			else
#endif
				wdev->csa_count = pTmpDot11h->CSPeriod;

			/* CSA count down start */
			if (wdev->csa_count != 0) {
				pTmpDot11h->csa_ap_bitmap |= (UINT32)(1 << wdev->func_idx);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						 "wdev(%d) csa_ap_bitmap = 0x%x\n",
						 wdev->wdev_idx, pTmpDot11h->csa_ap_bitmap);
			}

#ifdef DOT11_EHT_BE
			if (prepare_mlo_csa(pAd, wdev)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						 "wdev(%d) prepare_mlo_csa SUC\n", wdev->wdev_idx);
				if (!mlo_dev)
					mlo_dev = wdev;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						 "wdev(%d) NOT in MLD group.\n", wdev->wdev_idx);
				UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_CSA));
			}
#else
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_CSA));

#endif/*DOT11_EHT_BE*/

		}
	}

#ifdef DOT11_EHT_BE
	if (mlo_dev) {
		UpdateBeaconHandler_BPCC(pAd, mlo_dev, BCN_REASON(BCN_UPDATE_CSA), BCN_BPCC_CSA, TRUE);
		ret = TRUE;
	}
#endif/*DOT11_EHT_BE*/

	/*Send CSA cmd to FW and Wait CSA Event*/
	if (pDot11h->wdev_count != 0) {
		RTMPSetTimer(&pDot11h->CSAEventTimer, 4000);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"CSAEventTimer start(wdev_counter=%d)\n", pDot11h->wdev_count);
	}
#ifdef ZERO_PKT_LOSS_SUPPORT
	pAd->chan_switch_time[0] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/

	return ret;
}

/*
*
*/
BOOLEAN HcUpdateCsaCnt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN ret = TRUE;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
#ifdef DOT11_EHT_BE
	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_CSA) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_NOTICE,
				"bcn_bpcc_op_lock fail.\n");
		if (in_interrupt())
			bcn_bpcc_ct_switch(pAd, wdev, BCN_REASON(BCN_BPCC_CSA), BCN_UPDATE_CSA);
		else {
			ret = prepare_bpcc_csa(pAd);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_NOTICE, "ret = %d.\n", ret);
		}
	} else
#endif/*DOT11_EHT_BE*/
	{
		ret = prepare_bpcc_csa(pAd);
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_NOTICE, "ret = %d.\n", ret);
	}

#ifdef DOT11_EHT_BE
	if (!ret)
		bcn_bpcc_op_unlock(pAd, wdev, TRUE);

	bss_mngr_mld_sync_ml_probe_rsp(wdev);
#endif/*DOT11_EHT_BE*/

	return TRUE;
}

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx, ChIdx;
	CHANNEL_CTRL *pChCtrl = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev == NULL)
		MTWF_PRINT("Get Wdev Fail!");
	else {
		BandIdx = hc_get_hw_band_idx(pAd);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		if (pChCtrl->ChListNum == 0) {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d\n ",
				BandIdx,
				pChCtrl->ChListNum);
		} else {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m\n", BandIdx);
			MTWF_PRINT("ChannelListNum = %d\n", pChCtrl->ChListNum);
			MTWF_PRINT("ChGrpABandEn = %d\n Channel list information:\n",
				pChCtrl->ChGrpABandEn);
			MTWF_PRINT("Is40BWforbid = %d\n", pChCtrl->ch_cfg.bw40_forbid);
#ifdef CONFIG_6G_SUPPORT
			if (WMODE_CAP_6G(wdev->PhyMode))
				MTWF_PRINT(
				"Channel   Pwr0/1   Flags   DFSEnable PSC   ChPri   CacDoneTimestamp\n"
				);
			else
				MTWF_PRINT(
				"Channel   Pwr0/1   Flags   DFSEnable   ChPri   CacDoneTimestamp\n"
				);
#else

			MTWF_PRINT(
			"Channel   Pwr0/1   Flags   DFSEnable   ChPri   CacDoneTimestamp\n"
			);
#endif
			for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
#ifdef CONFIG_6G_SUPPORT
				if (WMODE_CAP_6G(wdev->PhyMode))
					MTWF_PRINT("#%-7d%4d/%d%8x%6d%10d%10u%20lu\n",
						pChCtrl->ChList[ChIdx].Channel,
						pChCtrl->ChList[ChIdx].Power,
						pChCtrl->ChList[ChIdx].Power2,
						pChCtrl->ChList[ChIdx].Flags,
						pChCtrl->ChList[ChIdx].DfsReq,
						pChCtrl->ChList[ChIdx].PSC_Ch,
						pChCtrl->ChList[ChIdx].Priority,
						pChCtrl->ChList[ChIdx].cac_done_timestamp);
				else
					MTWF_PRINT("#%-7d%4d/%d%8x%6d%16u%20lu\n",
						pChCtrl->ChList[ChIdx].Channel,
						pChCtrl->ChList[ChIdx].Power,
						pChCtrl->ChList[ChIdx].Power2,
						pChCtrl->ChList[ChIdx].Flags,
						pChCtrl->ChList[ChIdx].DfsReq,
						pChCtrl->ChList[ChIdx].Priority,
						pChCtrl->ChList[ChIdx].cac_done_timestamp);

#else
				MTWF_PRINT("#%-7d%4d/%d%8x%6d%16u%20lu\n",
					pChCtrl->ChList[ChIdx].Channel,
					pChCtrl->ChList[ChIdx].Power,
					pChCtrl->ChList[ChIdx].Power2,
					pChCtrl->ChList[ChIdx].Flags,
					pChCtrl->ChList[ChIdx].DfsReq,
					pChCtrl->ChList[ChIdx].Priority,
					pChCtrl->ChList[ChIdx].cac_done_timestamp);
#endif
			}
		}
	}
}

#ifdef GREENAP_SUPPORT
/*
 *
 */
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd)
{
	greenap_show(pAd);
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
void hc_show_edca_info(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	wmm_ctrl_show_entry(&ctrl->HwResourceCfg.wmm_ctrl);
}

/*
*
*/
void hc_show_radio_info(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	HdevCfgShow(ctrl);
}

/*
*
*/
void hc_show_hdev_obj(struct wifi_dev *wdev)
{
	if (!hdev_obj_state_ready(wdev->pHObj))
		return;

	HdevObjShow(wdev->pHObj);
}

/*
*
*/
void hc_set_txcmd_mode(VOID *ctrl, UCHAR txcmd_mode)
{
	struct hdev_ctrl *hctrl = (struct hdev_ctrl *) ctrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ctrl);

	if (txcmd_mode == HOBJ_TX_MODE_TXCMD
		&& (cap->asic_caps & fASIC_CAP_TXCMD)) {
		hctrl->HwResourceCfg.txcmd_mode = HOBJ_TX_MODE_TXCMD;
	}
}

uint8_t hc_get_txcmd_mode(VOID *ctrl)
{
	struct hdev_ctrl *hctrl = (struct hdev_ctrl *) ctrl;

	return hctrl->HwResourceCfg.txcmd_mode;
}

/*
*
*/
BOOLEAN HcAcquiredEdca(
	RTMP_ADAPTER *pAd, struct wifi_dev *wdev, EDCA_PARM *pEdca)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	/*hwifi will acquire resource when add interface*/
	if (ctrl->hdev_ops)
		return TRUE;
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"no hdev_ops, please check.\n");
		return FALSE;
	}
}

/*
*
*/
VOID HcReleaseEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	/*hwifi will acquire resource when add interface*/
	if (ctrl->hdev_ops)
		return;
}

/*
*  Need refine
*/


/*
* for Single Band Usage
*/
UCHAR  HcGetRadioChannel(RTMP_ADAPTER *pAd)
{
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(pAd);
	struct mtk_mac_phy *mac_phy = NULL;

	if (!mac_dev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_dev not found\n");
		return FALSE;
	}
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_phy not found\n");
		return FALSE;
	}
	return mac_phy->chan;
}

/*
*
*/
USHORT HcGetRadioPhyMode(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl.RadioCtrl.PhyMode;
}

USHORT HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl.RadioCtrl.PhyMode;
}

/*
*
*/
UCHAR HcGetRadioRfIC(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	return ctrl->HwResourceCfg.PhyCtrl.rf_band_cap;
}

/*
*
*/
uint8_t  HcIsRfSupport(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	if (pHwResource->PhyCtrl.rf_band_cap & RfIC)
		return TRUE;

	return FALSE;
}

/*
*
*/
uint8_t  HcIsRfRun(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev;

	rdev = &ctrl->rdev;

	if (WMODE_CAP_2G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_24GHZ))
		return TRUE;
	else if (WMODE_CAP_5G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_5GHZ))
		return TRUE;
	else if (WMODE_CAP_6G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_6GHZ))
		return TRUE;

	return FALSE;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
/*
*
*/
QLOAD_CTRL *HcGetQloadCtrlByRf(RTMP_ADAPTER *pAd, UINT32 RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	if (pHwResource->PhyCtrl.rf_band_cap & RfIC)
		return &pHwResource->PhyCtrl.QloadCtrl;

	return 0;
}

/*
*
*/
VOID *hc_get_qload_by_wdev(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl;
	HD_RESOURCE_CFG *hwres;

	if (!HcIsRadioAcq(wdev)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			"wdev=%d is not ready!\n", wdev->wdev_idx);
		return NULL;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	hwres = &ctrl->HwResourceCfg;

	return &hwres->PhyCtrl.QloadCtrl;
}


/*
*
*/
QLOAD_CTRL *HcGetQloadCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl.QloadCtrl;
}
#endif /*AP_QLOAD_SUPPORT*/

/*
*
*/
AUTO_CH_CTRL *HcGetAutoChCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource =  &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl.AutoChCtrl;
}
#endif /* CONFIG_AP_SUPPORT */

/*
*
*/
UCHAR HcGetBw(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct freq_oper oper;
	int ret;

	ret = hc_radio_query_by_wdev(wdev, &oper);

	if (ret != HC_STATUS_OK)
		return 0;

	return oper.bw;
}

/*
*
*/
UINT32 HcGetMgmtQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type)
{
	u8 band_idx = hc_get_hw_band_idx(pAd);
	u32 qid;

	switch (band_idx) {
	case BAND0:
		qid = TxQ_IDX_ALTX0;
		break;
	case BAND1:
		qid = TxQ_IDX_ALTX1;
		break;
	default:
		qid = TxQ_IDX_ALTX0;
		break;
	}
	if (pkt_type != TX_ALTX) {
		/* null/qos null frame need to use ac queue to get tx success cnt */
		UCHAR WmmIdx = HcGetWmmIdx(pAd, wdev);

		return asic_get_hwq_from_ac(pAd, WmmIdx, QID_AC_BE);
	}

	return qid;
}

/*
*
*/
UINT32 HcGetBcnQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	u8 band_idx = hc_get_hw_band_idx(pAd);
	u32 qid;

	switch (band_idx) {
	case BAND0:
		qid = TxQ_IDX_BCN0;
		break;
	case BAND1:
		qid = TxQ_IDX_BCN1;
		break;
	default:
		qid = TxQ_IDX_BCN0;
		break;
	}
	return qid;
}

UCHAR HcGetBandInfoByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	UCHAR BandIdx = BAND0;
	uint8_t Is2GRun = FALSE;
	uint8_t Is5GRun = FALSE;

	if (pAd->CommonCfg.dbdc_mode == FALSE)
		return BandIdx;

	Is2GRun = HcIsRfSupport(pAd, RFIC_24GHZ);
	Is5GRun = HcIsRfSupport(pAd, RFIC_5GHZ);
	if (Is2GRun && Is5GRun) {
		if (Channel > 14)
			return BAND1;

		return BandIdx;
	}

	ASSERT(FALSE);
	return BandIdx;
}

/*
*Use ChannelRange to get BandIdx
*When DBDC disabled, we return BAND0 as default
*When DBDC enabled, if channel > 14, we return BAND1,else we return BAND0
*/
UCHAR HcGetBandByChannelRange(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	UCHAR BandIdx = BAND0;
	BOOLEAN Is2GRun = FALSE;
	BOOLEAN Is5GRun = FALSE;

	if (pAd->CommonCfg.dbdc_mode == FALSE)
		return BandIdx;

	Is2GRun = HcIsRfSupport(pAd, RFIC_24GHZ);
	Is5GRun = HcIsRfSupport(pAd, RFIC_5GHZ);
	if (Is2GRun && Is5GRun) {
		if (Channel > 14)
			return BAND1;

		return BandIdx;
	}

	ASSERT(FALSE);
	return BandIdx;
}

/*
*
*/
inline UCHAR HcGetBandByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	return hc_get_hw_band_idx(pAd);
}

/*
*
*/
VOID HcCrossChannelCheck(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
	USHORT PhyMode = wdev->PhyMode;
	UCHAR WChannel = wdev->channel;

	/*check channel is belong to differet band*/
	if (WMODE_CAP_6G(PhyMode) && Channel <= CHANNEL_6G_MAX && WChannel <= CHANNEL_6G_MAX)
		return;

	if (WMODE_CAP_5G(PhyMode) && Channel > 14 && WChannel > 14)
		return;

	if (WMODE_CAP_2G(PhyMode) && Channel <= 14 && WChannel <= 14)
		return;

	/*is mixed mode, change default channel and */
	if (!WMODE_5G_ONLY(PhyMode) || !WMODE_2G_ONLY(PhyMode) || !WMODE_6G_ONLY(PhyMode)) {
		/*update wdev channel to new band*/
		wdev->channel = Channel;
		/*need change to other band*/
		hc_update_wdev(wdev);
	}

	return;
}

/*
*
*/
VOID HcReleaseGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, uint16_t wcid)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);

	if (ctrl->hdev_ops) {
		ctrl->hdev_ops->remove_bmc_sta(pAd, wdev, wcid);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support!\n");
}

/*
*
*/
UINT16 HcAcquireUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli, UINT16 mld_sta_idx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	INT16 ret = 0;

	if (!HcIsRadioAcq(wdev))
		return WCID_INVALID;

	if (ctrl->hdev_ops)
		ret = ctrl->hdev_ops->add_sta(pAd, wdev, is_A4, is_apcli, mld_sta_idx);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			" not support !\n");
		return WCID_INVALID;
	}
	if (ret < 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			" Alloc wcid fail !\n");
		return WCID_INVALID;
	}
	return ret;
}

UINT16 HcChangeSetupLink(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	INT16 ret = 0;

	if (ctrl->hdev_ops)
		ret = ctrl->hdev_ops->change_setup_link_sta(pAd, wcid);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			" not support !\n");
		return WCID_INVALID;
	}
	if (ret < 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
			" fail !\n");
		return WCID_INVALID;
	}
	return ret;
}


/*
*
*/
uint16_t HcReleaseUcastWcid(RTMP_ADAPTER *ad, struct wifi_dev *wdev, uint16_t wcid)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	if (!HcIsRadioAcq(wdev))
		return WCID_INVALID;

	if (ctrl->hdev_ops)
		return ctrl->hdev_ops->remove_sta(ad, wdev, wcid);
	else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support!\n");
		return WCID_INVALID;
	}
}

/*
*
*/
VOID HcWtblRecDump(RTMP_ADAPTER *pAd)
{
}

/*
*
*/
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad)
{
	struct freq_oper freq;
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(ad);
	struct mtk_mac_phy *mac_phy = NULL;

	if (!mac_dev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_dev not found\n");
		return FALSE;
	}
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_phy not found\n");
		return FALSE;
	}

	os_zero_mem(&freq, sizeof(freq));
	freq.bw = mac_phy->bw;
	freq.prim_ch = mac_phy->chan;
	freq.cen_ch_1 = mac_phy->cen_chan;
	freq.cen_ch_2 = mac_phy->chan2;
	AsicSwitchChannel(ad, hc_get_hw_band_idx(ad), &freq, FALSE);
	return TRUE;
}

/*
*
*/
VOID hc_set_rrm_init(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UCHAR band_idx = hc_get_hw_band_idx(ad);
	uint8_t ucTxPath = ad->Antenna.field.TxPath;
	uint8_t ucRxPath = ad->Antenna.field.RxPath;

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (ad->bAntennaSetAPEnable) {
			ucTxPath = ad->TxStream;
			ucRxPath = ad->RxStream;
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */


	AsicSetTxStream(wdev->sys_handle, ucTxPath, OPMODE_AP, TRUE, band_idx);
	AsicSetRxStream(wdev->sys_handle, ucRxPath, band_idx);
}


INT hc_radio_query(struct _RTMP_ADAPTER *ad, struct freq_oper *oper)
{
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(ad);
	struct mtk_mac_phy *mac_phy = NULL;

	if (!mac_dev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_dev not found\n");
		return HC_STATUS_FAIL;
	}
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_phy not found\n");
		return HC_STATUS_FAIL;
	}
	oper->bw = mac_phy->bw;
	oper->cen_ch_1 = mac_phy->cen_chan;
	oper->cen_ch_2 = mac_phy->chan2;
	oper->ext_cha = mac_phy->ext_cha;
	oper->prim_ch = mac_phy->chan;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
	return 0;
}



/*
 *
 */
VOID *hc_get_hdev_ctrl(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;
	struct hdev_ctrl *h_ctrl;

	ad = wdev->sys_handle;
	if (!ad)
		return ad;
	h_ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
	return h_ctrl;
}

/*
*
*/
inline struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return &ph_dev->chip_cap;
}
EXPORT_SYMBOL(hc_get_chip_cap);

/*
*
*/
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_ops;
}
EXPORT_SYMBOL(hc_get_chip_ops);

/*
*
*/
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	os_move_mem(&ctrl->chip_ops, ops, sizeof(*ops));
}

/*
*
*/
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2)
{
	os_move_mem(&ChCtrl->ChList[ChIdx], &pAd->TxPower[ChIdx2], sizeof(CHANNEL_TX_POWER));
	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx)
{
	ChCtrl->ChList[ChIdx].Flags |= ChannelListFlag;

	/* Check if 40MHz is allowed */
	if (Is40MHzForbid(ChCtrl))
		ChCtrl->ChList[ChIdx].Flags &= (~CHANNEL_40M_CAP);

	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	ChCtrl->ChListStat = ChListStat;
	return HC_STATUS_OK;
}

CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->ChCtrl;
}

UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd)
{
	CHANNEL_CTRL *pChCtrl;
	USHORT Phymode = 0;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	Phymode = HcGetRadioPhyModeByBandIdx(pAd);
	if (pAd->CommonCfg.DfsParameter.CACMemoEn
		&& hc_check_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE)
		&& WMODE_CAP_5G(Phymode)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"Skip Clear chCtrl (caller: %pS)\n",
			OS_TRACE);

		return HC_STATUS_OK;
	}
	os_zero_mem(pChCtrl, sizeof(CHANNEL_CTRL));

	return HC_STATUS_OK;
}

#ifdef CONFIG_AP_SUPPORT
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = NULL;
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0;
	pAutoChCtrl->AutoChSelCtrl.ChListNum = 0;
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	pAutoChCtrl->AutoChSelCtrl.RestoreDwell = 150;
	pAutoChCtrl->AutoChSelCtrl.ScanningDwell = 200;
	pAutoChCtrl->AutoChSelCtrl.ScanningChNum = 2;
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));

	return HC_STATUS_OK;
}
#endif

UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	return (ChCtrl->ChListStat == ChListStat);
}
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_dbg;
}

/*
*
*/
UINT32 hc_get_mac_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.mac_caps;
}

/*
*
*/
UINT32 hc_get_phy_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.phy_caps;
}

/*
*
*/
UINT32 hc_get_hif_type(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.hif_type;
}

/*
*
*/
UINT32 hc_get_asic_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.asic_caps;
}

/*
*
*/
VOID hc_set_mac_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.mac_caps |= caps;
}

/*
*
*/
VOID hc_set_phy_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.phy_caps |= caps;
}

/*
*
*/
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.asic_caps |= caps;
}

/*
*
*/
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.asic_caps &= ~(caps);
}

UCHAR hc_get_cur_rfic(struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl;
	struct radio_control *radio_ctrl;

	ctrl = hc_get_hdev_ctrl(wdev);

	radio_ctrl = ctrl->rdev.pRadioCtrl;
	return radio_ctrl->cur_rfic_type;
}

/*
*
*/
uint8_t hc_get_chip_bcn_max_num(struct _RTMP_ADAPTER *ad)
{
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.perBandBcnMaxNum;
}

/*
*
*/
void hc_set_chip_wtbl_num(void *hdev_ctrl, uint16_t max, uint16_t ucast)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	if (max)
		ph_dev->chip_cap.wtbl_max_entries = max;

	if (ucast)
		ph_dev->chip_cap.wtbl_ucast_entries = ucast;
}

/*
 * Used to indicate no WTBL entry is matched after HW search.
 * Different MAC arch could have particular value for it, and it's
 * defined in repsective header file. SW uses the unified value to
 * carry such information in RXBLK after processing the RXD.
 */
uint16_t hc_get_chip_wtbl_no_matched_idx(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.wtbl_no_matched;
}

#ifdef SW_CONNECT_SUPPORT
/* get the pure s/w tr_entry num , w/o hw part */
uint16_t hc_get_chip_sw_ucast_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.sw_ucast_max_entries;
}

/* set pure s/w u/c tr_entry low/high point , w/o hw part */
void hc_set_chip_sw_ucast_range_num(void *hdev_ctrl, uint16_t low, uint16_t high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.ucast_sw.start = low;
	ph_dev->chip_cap.ucast_sw.end = high;

	if (low && high && (high >= low))
		ph_dev->chip_cap.sw_ucast_max_entries = (high - low) + 1;
}

/* get pure s/w u/c tr_entry low/high point , w/o hw part */
void hc_get_chip_sw_ucast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	*low = ph_dev->chip_cap.ucast_sw.start;
	*high = ph_dev->chip_cap.ucast_sw.end;
}

/* get pure s/w tr_entry num of bcast, w/o hw part */
uint16_t hc_get_chip_sw_bcast_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.sw_bcast_max_entries;
}

/* set pure s/w tr_entry range of bcast, w/o hw part */
void hc_set_chip_sw_bcast_range_num(void *hdev_ctrl, uint16_t low, uint16_t high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.bcast_sw.start = low;
	ph_dev->chip_cap.bcast_sw.end = high;

	if (low && high && (high >= low))
		ph_dev->chip_cap.sw_bcast_max_entries = (high - low) + 1;
}

/* get pure s/w tr_entry range of bcast, w/o hw part */
void hc_get_chip_sw_bcast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	*low = ph_dev->chip_cap.bcast_sw.start;
	*high = ph_dev->chip_cap.bcast_sw.end;
}

/* get pure hw wtbl range of ucast from hwifi */
void hc_get_chip_hw_ucast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(ad);
	u16 hw_uwtbl_low = 0, hw_uwtbl_high = 0;

	if (mac_dev)
		mac_dev->ops->get_wtbl_idrm_range_num(&mac_dev->hw, &hw_uwtbl_low,
					&hw_uwtbl_high, MAC_STA_TYPE_NORMAL);

	*low = hw_uwtbl_low;
	*high = hw_uwtbl_high;
}

/* get pure hw wtbl range of bcast from hwifi */
void hc_get_chip_hw_bcast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(ad);
	u16 hw_grp_low = 0, hw_grp_high = 0;

	if (mac_dev)
		mac_dev->ops->get_wtbl_idrm_range_num(&mac_dev->hw, &hw_grp_low,
					&hw_grp_high, MAC_STA_TYPE_GROUP);

	*low = hw_grp_low;
	*high = hw_grp_high;
}

/* set sum of hw + s/w wtbl num */
void hc_set_chip_hw_sw_wtbl_num(void *hdev_ctrl, uint16_t hw, uint16_t sw)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.hw_sw_max_entries = (hw + sw);
}

/* set sum of hw + s/w ucast wtbl num */
void hc_set_chip_hw_sw_ucast_num(void *hdev_ctrl, uint16_t hw, uint16_t sw)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	ph_dev->chip_cap.hw_sw_max_ucast_entries = (hw + sw);
}

/* get bcast sw_wcid from hw_wcid , 1: 1 mapping  */
uint16_t hc_get_chip_sw_bcast_wcid_by_hw(void *hdev_ctrl, uint16_t hw_wcid)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;
	BOOLEAN found = FALSE;
	struct mtk_mac_sta *bmc_sta = NULL;
	/* search from low bcast sw_wcid */
	int sw_id = (int)(ph_dev->chip_cap.bcast_sw.start);

	os_idr_for_each_entry_continue(&(ph_dev->sw_group_mgmt), bmc_sta, sw_id) {
		if (bmc_sta->link_wcid == hw_wcid) {
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		return sw_id;
	else
		return WCID_INVALID;
}

BOOLEAN hc_is_sw_wcid(struct _RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if ((ph_dev->chip_cap.ucast_sw.start <= wcid) &&
		(wcid <= ph_dev->chip_cap.ucast_sw.end))
		return TRUE;

	return FALSE;
}

BOOLEAN hc_is_sw_sta_enable(struct _RTMP_ADAPTER *pAd)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	return ph_dev->bSwSta;
}

struct mtk_mac_sta *hc_is_dummy_sta_exist(struct _RTMP_ADAPTER *pAd, u8 band_idx)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if ((ph_dev->bSwSta) &&
		(pAd->dummy_obj.State == WTBL_STATE_SW_OCCUPIED))
		return pAd->dummy_obj.mac_sta;

	return NULL;
}

BOOLEAN hc_add_dummy_sta(struct _RTMP_ADAPTER *pAd, struct mtk_mac_sta *mac_sta)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	if (ph_dev->bSwSta) {
		if (pAd->dummy_obj.State == WTBL_STATE_NONE_OCCUPIED) {
			pAd->dummy_obj.mac_sta = mac_sta;
			pAd->dummy_obj.HwWcid = mac_sta->link_wcid;
			pAd->dummy_obj.State = WTBL_STATE_SW_OCCUPIED;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_NOTICE,
				"[ADD] bSwSta=%d, mac_sta=%p, (State=%d, mac_sta=%p, HwWcid=%d)\n",
				ph_dev->bSwSta, mac_sta, pAd->dummy_obj.State, pAd->dummy_obj.mac_sta, pAd->dummy_obj.HwWcid);
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"!!ERROR!! Double add!! please check.\n");
		}
	}

	return FALSE;
}

BOOLEAN hc_del_dummy_sta(struct _RTMP_ADAPTER *pAd, struct mtk_mac_sta *mac_sta)
{
	struct physical_device *ph_dev = pAd->physical_dev;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_NOTICE,
		"[DEL] bSwSta=%d, mac_sta=%p, (State=%d, mac_sta=%p, HwWcid=%d)\n",
		ph_dev->bSwSta, mac_sta, pAd->dummy_obj.State, pAd->dummy_obj.mac_sta, pAd->dummy_obj.HwWcid);

	if (ph_dev->bSwSta) {
		if (pAd->dummy_obj.State == WTBL_STATE_SW_OCCUPIED) {
			if (pAd->dummy_obj.mac_sta == mac_sta) {
				pAd->dummy_obj.mac_sta = NULL;
				pAd->dummy_obj.HwWcid = 0;
				pAd->dummy_obj.State = WTBL_STATE_NONE_OCCUPIED;
			} else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
					"!!ERROR!! mac_sta mis_match!! please check. pAd->dummy_obj.mac_sta=%p, mac_sta=%p\n",
					pAd->dummy_obj.mac_sta, mac_sta);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"!!ERROR!! Double del!! please check.\n");
		}
	}

	return FALSE;
}
#endif /* SW_CONNECT_SUPPORT */


/*
*
*/
uint16_t hc_get_chip_max_ba_win_sz(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	if (ph_dev->chip_cap.mac_caps & fMAC_CAP_BA_1024)
		return BA_WIN_SZ_1024;
	else if (ph_dev->chip_cap.mac_caps & fMAC_CAP_BA_512)
		return BA_WIN_SZ_512;
	else if (ph_dev->chip_cap.mac_caps & fMAC_CAP_BA_256)
		return BA_WIN_SZ_256;
	else
		return BA_WIN_SZ_64;
}

/*
*
*/
uint8_t hc_get_chip_wapi_sup(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.FlgIsHwWapiSup;
}

UINT32 hc_get_chip_tx_token_nums(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.tkn_info.token_tx_cnt;
}
EXPORT_SYMBOL(hc_get_chip_tx_token_nums);

UINT32 hc_get_chip_sw_tx_token_nums(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return (ph_dev->chip_cap.tkn_info.token_tx_cnt -
			ph_dev->chip_cap.tkn_info.hw_tx_token_cnt);
}
EXPORT_SYMBOL(hc_get_chip_sw_tx_token_nums);

UINT32 hc_get_chip_mac_rxd_size(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;

	return ph_dev->chip_cap.rx_hw_hdr_len;
}
EXPORT_SYMBOL(hc_get_chip_mac_rxd_size);

/*
*
*/
VOID *hc_get_hif_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->hif.cfg;
}
EXPORT_SYMBOL(hc_get_hif_ctrl);

#ifdef CUT_THROUGH
/*
*
*/
VOID *hc_get_ct_cb(void *hdev_ctrl)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	return pci_hif->PktTokenCb;
}
EXPORT_SYMBOL(hc_get_ct_cb);

/*
*
*/
VOID hc_set_ct_cb(void *hdev_ctrl, void *ct_cb)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	pci_hif->PktTokenCb = ct_cb;
}
#endif /*CUT_THROUGH*/

/*
*
*/
inline VOID *hc_get_os_cookie(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->cookie;
}

/*
*
*/
inline VOID *hc_get_mcu_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->mcu_ctrl;
}

/*
*
*/
inline struct _RTMP_ARCH_OP *hc_get_arch_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->arch_ops;
}

/*
*
*/
inline struct mt_io_ops *hc_get_io_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->io_ops;
}

/*
*
*/
inline void *hc_get_hdev_privdata(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->priv;
}

VOID *hc_get_hif_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->hif.ops;
}

#ifdef HWIFI_SUPPORT
VOID hc_set_hdev_ops(void *hdev_ctrl, struct mtk_hdev_ops *ops)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->hdev_ops = ops;
}
#endif

/*
* hif ops
*/

/*MCU related*/
VOID hif_mcu_init(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->mcu_init)
		ops->mcu_init(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"not support!\n");
}

VOID hif_mcu_exit(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->mcu_exit)
		ops->mcu_exit(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"not support!\n");
}

VOID hif_rx_event_process(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->rx_event_process)
		ops->rx_event_process(ad, msg);
	else
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"not support!\n");
}

VOID hif_mcu_fw_init(struct _RTMP_ADAPTER *ad)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_fw_init)
		ops->mcu_fw_init(ad->hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"not support!\n");
}

VOID hif_mcu_fw_exit(struct _RTMP_ADAPTER *ad)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_fw_exit)
		ops->mcu_fw_exit(ad->hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR, "not support!\n");
}

UCHAR *hif_get_tx_buf(void *hdev_ctrl, struct _TX_BLK *tx_blk, UCHAR resource_idx, UCHAR frame_type)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_buf)
		return ops->get_tx_buf(hdev_ctrl, tx_blk, resource_idx, frame_type);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO, "not support!\n");

	return NULL;
}

UINT32 hif_get_tx_resource_free_num(void *hdev_ctrl, uint8_t resource_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_resource_free_num)
		return ops->get_tx_resource_free_num(hdev_ctrl, resource_idx);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support!\n");

	return 0;
}

inline UINT32 hif_get_resource_idx(void *hdev_ctrl, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UCHAR q_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);
	uint8_t band_idx = 0;

	/* QM_V2 wdev may be NULL */
	if (wdev)
		band_idx = HcGetBandByWdev(wdev);

	if (ops->get_resource_idx)
		return ops->get_resource_idx(hdev_ctrl, band_idx, pkt_type, q_idx);

	return 0;
}

/*
*
*/
struct cmd_msg *hif_mcu_alloc_msg(
	struct _RTMP_ADAPTER *ad, unsigned int length, BOOLEAN bOldCmdFmt)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_alloc_msg)
		return ops->mcu_alloc_msg(ad, length);
	else
		return AndesAllocCmdMsgGe(ad, length, bOldCmdFmt);
}

#ifdef CONFIG_STA_SUPPORT
/*
*
*/
VOID hif_ps_poll_enq(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->ps_poll_enq)
		return ops->ps_poll_enq(ad, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support!\n");
}

/*
*
*/
VOID hif_sta_wakeup(struct _RTMP_ADAPTER *ad, uint8_t bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->sta_wakeup)
		return ops->sta_wakeup(ad, bFromTx, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support!\n");

}

/*
*
*/
VOID hif_sta_sleep_auto_wakeup(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->sta_sleep_auto_wakeup)
		return ops->sta_sleep_auto_wakeup(ad, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR, "not support !\n");
}
#endif /* CONFIG_STA_SUPPORT */
/*
*
*/
INT hif_cmd_thread(ULONG context)
{
	RTMP_ADAPTER *ad;
	RTMP_OS_TASK *task;
	int status = 0;
	struct hif_ops *ops;

	task = (RTMP_OS_TASK *)context;
	ad = (PRTMP_ADAPTER)task->priv;
	ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->cmd_thread)
		return ops->cmd_thread(context);

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "not support !\n");
	return status;
}

/*
*
*/
VOID hif_mcu_unlink_ackq(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_unlink_ackq)
		return ops->mcu_unlink_ackq(msg);

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "not support !\n");
}

/*
*
*/
uint8_t hif_get_tx_res_num(VOID *hdev_ctrl)
{
#ifndef HWIFI_SUPPORT
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_res_num)
		return ops->get_tx_res_num(hc_get_hif_ctrl(hdev_ctrl));

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "not support !\n");
#endif
	return 0;
}

/*
*
*/
uint8_t hif_get_rx_res_num(VOID *hdev_ctrl)
{
#ifndef HWIFI_SUPPORT
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_rx_res_num)
		return ops->get_rx_res_num(hc_get_hif_ctrl(hdev_ctrl));

	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "not support !\n");
#endif
	return 0;
}
