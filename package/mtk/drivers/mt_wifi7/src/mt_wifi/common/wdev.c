/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"

BOOLEAN is_testmode_wdev(UINT32 wdev_type)
{
	BOOLEAN ret = FALSE;

	if (wdev_type == WDEV_TYPE_ATE_AP ||
		wdev_type == WDEV_TYPE_ATE_STA ||
		wdev_type == WDEV_TYPE_SERVICE_TXC ||
		wdev_type == WDEV_TYPE_SERVICE_TXD)
		ret = TRUE;

	return ret;
}

/*define extern function*/
INT wdev_edca_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	EDCA_PARM *edca;

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
#ifdef WDS_SUPPORT
	case WDEV_TYPE_WDS:
#endif /*WDS_SUPPORT*/
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT

	case WDEV_TYPE_STA: {
		struct _STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(ad, wdev);

		edca = &sta_cfg->MlmeAux.APEdcaParm;
	}
	break;
#endif /*CONFIG_STA_SUPPORT*/

	default:
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
	}

	hwifi_update_edca(ad, wdev, edca, FALSE);
	return TRUE;
}

/*define global function*/
struct wifi_dev *get_default_wdev(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(ad->OpMode) {
		return &ad->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(ad->OpMode) {
		return &ad->StaCfg[MAIN_MSTA_ID].wdev;
	}
#endif
	return NULL;
}

/*define local function*/
INT wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN,
					 "unregister wdev(type:%d, idx:%d) from wdev_list\n",
					  wdev->wdev_type, wdev->wdev_idx);
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
				 "Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
				  wdev, wdev->wdev_type, wdev->wdev_idx);
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO, "Dump wdev_list:\n");

		for (idx = 0; idx < WDEV_NUM_MAX; idx++)
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO, "Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]);
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? 0 : -1);
}

#ifdef MAC_REPEATER_SUPPORT
/*
	Description:
	for record Rx Pkt's wlanIdx, TID, Seq.
	it is used for checking if there is the same A2 send to different A1.
	according the record. trigger the scoreboard update.
*/
static VOID RxTrackingInit(struct wifi_dev *wdev)
{
	UCHAR j;
	RX_TRACKING_T *pTracking = NULL;
	RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;

	pTracking = &wdev->rx_tracking;
	pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

	pTracking->TriggerNum = 0;

	pTaTidSeqMapEntry->RxDWlanIdx = 0xff;
	pTaTidSeqMapEntry->MuarIdx = 0xff;
	for (j = 0; j < 8; j++) {
		pTaTidSeqMapEntry->TID_SEQ[j] = 0xffff;
	}
	pTaTidSeqMapEntry->LatestTID = 0xff;
}
#endif

INT32 wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN,
					 "wdev(type:%d) already registered and idx(%d) %smatch\n",
					  wdev->wdev_type, wdev->wdev_idx,
					  ((idx != wdev->wdev_idx) ? "mis" : ""));
			break;
		}

		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
#ifdef MAC_REPEATER_SUPPORT
			RxTrackingInit(wdev);
#endif /* MAC_REPEATER_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO,
				"Assign wdev_idx=%d with OmacIdx = %d\n",
				idx,
				wdev->OmacIdx);
			break;
		}
	}


	wdev->wdev_idx = idx;

	if (idx < WDEV_NUM_MAX)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO, "Assign wdev_idx=%d\n", idx);

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


/*
*
*/
VOID BssInfoArgumentLink(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	union _HTTRANSMIT_SETTING HTPhyMode;
	UINT16 BMCWlanIdxTmp, BMCWlanIdx2Tmp;
	BOOLEAN isVlan = FALSE;
#ifdef CONFIG_STA_SUPPORT
	struct _STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(ad, wdev);
#endif /*CONFIG_STA_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = wdev->func_dev;
#endif

	bssinfo->OwnMacIdx = wdev->OmacIdx;
	bssinfo->ucBandIdx = wdev->DevInfo.BandIdx;
	bssinfo->ucBssIndex = hc_get_bssidx(wdev);
	os_move_mem(bssinfo->Bssid, wdev->bssid, MAC_ADDR_LEN);
	bssinfo->CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
	bssinfo->ucPhyMode = wdev->PhyMode;
	bssinfo->ucPhyModeExt = (wdev->PhyMode >> 8);
	bssinfo->priv = (VOID *)wdev;
	hc_radio_query_by_wdev(wdev, &bssinfo->chan_oper);
#ifdef DOT11_EHT_BE
	bssinfo->mld_info.mld_group_idx = MLD_GROUP_NONE;
#endif /* DOT11_EHT_BE */

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_STA;
		BMCWlanIdxTmp = HcAcquireGroupKeyWcid(ad, wdev, isVlan);
		BMCWlanIdx2Tmp = HcAcquireGroupKeyWcid2(ad, wdev);
		if (BMCWlanIdxTmp == BMCWlanIdx2Tmp)
			MTWF_DBG(ad, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"BMCWlanIdx (%d) cannot equal to BMCWlanIdx2 (%d)!!\n",
				BMCWlanIdxTmp, BMCWlanIdx2Tmp);
		else {
			bssinfo->bmc_wlan_idx = (BMCWlanIdx2Tmp > BMCWlanIdxTmp) ? BMCWlanIdxTmp : BMCWlanIdx2Tmp;
			ad->BMCWlanIdx = bssinfo->bmc_wlan_idx;
			bssinfo->bmc_wlan_idx2 = (BMCWlanIdx2Tmp > BMCWlanIdxTmp) ? BMCWlanIdx2Tmp : BMCWlanIdxTmp;
			ad->BMCWlanIdx2 = bssinfo->bmc_wlan_idx2;
			MTWF_DBG(ad, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"BMCWlanIdx=%d, BMCWlanIdx2=%d, bssinfo->bmc_wlan_idx=%d, bssinfo->bmc_wlan_idx2=%d\n",
				BMCWlanIdxTmp, BMCWlanIdx2Tmp, bssinfo->bmc_wlan_idx, bssinfo->bmc_wlan_idx2);
		}
		TRTableInsertMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
#ifdef CONFIG_KEEP_ALIVE_OFFLOAD
		/* STA LP offload */
		bssinfo->rBssInfoPm.ucKeepAliveEn = TRUE;
		bssinfo->rBssInfoPm.ucKeepAlivePeriod = KEEP_ALIVE_INTERVAL_IN_SEC;
#endif
		bssinfo->rBssInfoPm.ucBeaconLossReportEn = TRUE;
		bssinfo->rBssInfoPm.ucBeaconLossCount = BEACON_OFFLOAD_LOST_TIME;
#ifdef	CONFIG_STA_SUPPORT
		if (sta_cfg) {
			bssinfo->bcn_period = sta_cfg->MlmeAux.BeaconPeriod;
			bssinfo->dtim_period = sta_cfg->MlmeAux.DtimPeriod;
			bssinfo->dbm_to_roam = sta_cfg->dBmToRoam;
#ifdef DOT11V_MBSSID_SUPPORT
			bssinfo->max_bssid_indicator =
				sta_cfg->MlmeAux.max_bssid_indicator;
			bssinfo->mbssid_index = sta_cfg->MlmeAux.mbssid_index;
#endif
		}
#ifdef UAPSD_SUPPORT
		uapsd_config_get(wdev, &bssinfo->uapsd_cfg);
#endif /*UAPSD_SUPPORT*/
#endif /*CONFIG_STA_SUPPORT*/
#ifdef DOT11_EHT_BE
		wifi_sys_bss_query_mld(wdev, &bssinfo->mld_info);
#endif /* DOT11_EHT_BE */
		break;

	case WDEV_TYPE_ADHOC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_IBSS;
		bssinfo->u4ConnectionType = CONNECTION_IBSS_ADHOC;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev, isVlan);
		bssinfo->bmc_wlan_idx2 = HcAcquireGroupKeyWcid2(ad, wdev);
		break;

	case WDEV_TYPE_WDS:
		bssinfo->bssinfo_type = WDS;
		bssinfo->NetworkType = NETWORK_WDS;
		bssinfo->u4ConnectionType = CONNECTION_WDS;
		break;

	case WDEV_TYPE_GO:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GO;
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev, isVlan);
		bssinfo->bmc_wlan_idx2 = HcAcquireGroupKeyWcid2(ad, wdev);
		MgmtTableSetMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
		break;

	case WDEV_TYPE_GC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_P2P;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GC;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev, isVlan);
		bssinfo->bmc_wlan_idx2 = HcAcquireGroupKeyWcid2(ad, wdev);
		break;

	case WDEV_TYPE_AP:
	default:
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev, isVlan);
		ad->BMCWlanIdx = bssinfo->bmc_wlan_idx;
		bssinfo->bmc_wlan_idx2 = 0xFFFF;
		ad->BMCWlanIdx2 = bssinfo->bmc_wlan_idx2;
		MgmtTableSetMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_AP;
#ifdef CONFIG_AP_SUPPORT
		bssinfo->bcn_period = ad->CommonCfg.BeaconPeriod;

		if (pMbss)
			bssinfo->dtim_period = pMbss->DtimPeriod;
#endif /*CONFIG_AP_SUPPORT*/
		break;
	}

	wdev_edca_acquire(ad, wdev);
	bssinfo->WmmIdx = HcGetWmmIdx(ad, wdev);
	/* Get a specific Tx rate for BMcast frame */
	os_zero_mem(&HTPhyMode, sizeof(union _HTTRANSMIT_SETTING));

	if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
		WMODE_CAP_2G(wdev->PhyMode)
#ifdef GN_MIXMODE_SUPPORT
		&& (!(ad->CommonCfg.GNMixMode))
#endif /*GN_MIXMODE_SUPPORT*/
	    ) {
		HTPhyMode.field.MODE = MODE_CCK;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = RATE_1;
		if (ad->CommonCfg.bSeOff != TRUE) {
			bssinfo->FixedRateIdx = FR_CCK_SPE0x18_1M;
			bssinfo->mloFixedRateIdx = FR_OFDM_SPE0x18_6M;
		} else {
			bssinfo->FixedRateIdx = FR_CCK_1M;
			bssinfo->mloFixedRateIdx = FR_OFDM_6M;
		}
	} else {
		HTPhyMode.field.MODE = MODE_OFDM;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = MCS_RATE_6;
		if (ad->CommonCfg.bSeOff != TRUE) {
			bssinfo->FixedRateIdx = FR_OFDM_SPE0x18_6M;
			bssinfo->mloFixedRateIdx = FR_OFDM_SPE0x18_6M;
		} else {
			bssinfo->FixedRateIdx = FR_OFDM_6M;
			bssinfo->mloFixedRateIdx = FR_OFDM_6M;
		}
	}

#ifdef MCAST_RATE_SPECIFIC
	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		bssinfo->McTransmit = (wdev->channel > 14) ? (wdev->rate.MCastPhyMode_5G) : (wdev->rate.MCastPhyMode);
		bssinfo->BcTransmit = (wdev->channel > 14) ? (wdev->rate.MCastPhyMode_5G) : (wdev->rate.MCastPhyMode);
#else
		bssinfo->McTransmit = wdev->rate.mcastphymode;
		bssinfo->BcTransmit = wdev->rate.mcastphymode;
#endif

		if ((wdev->channel > 14)
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		&& (wdev->rate.MCastPhyMode_5G.field.MODE == MODE_CCK)
#else
		&& (wdev->rate.mcastphymode.field.MODE == MODE_CCK)
#endif
		) {
			bssinfo->McTransmit = HTPhyMode;
			bssinfo->BcTransmit = HTPhyMode;
		}
	} else
#endif /* MCAST_RATE_SPECIFIC */
	{
		bssinfo->McTransmit = HTPhyMode;
		bssinfo->BcTransmit = HTPhyMode;
	}

	bssinfo->FixedRateBW = BW_20;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	raWrapperConfigSet(ad, wdev, &bssinfo->ra_cfg);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

#ifdef DOT11_HE_AX
	fill_bssinfo_he(wdev, bssinfo);
#endif /*DOT11_HE_AX*/

#ifdef BCN_PROTECTION_SUPPORT
	os_move_mem(&bssinfo->bcn_prot_cfg, &wdev->SecConfig.bcn_prot_cfg, sizeof(struct bcn_protection_cfg));
#endif
	bssinfo->bss_state = BSS_INITED;
}


VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;

	hwifi_update_edca(pAd, wdev, NULL, TRUE);
#ifdef DOT11_HE_AX
	bcolor_release_entry(pAd, bss->bss_color.color);
#endif
	os_zero_mem(bss, sizeof(wdev->bss_info_argument));
	WDEV_BSS_STATE(wdev) = BSS_INIT;
}

INT wdev_ops_register(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
					  struct wifi_dev_ops *ops, UCHAR wmm_detect_method)
{
	wdev->wdev_ops = ops;

	if (wmm_detect_method == WMM_DETECT_METHOD1)
		ops->detect_wmm_traffic = mt_detect_wmm_traffic;
	else if (wmm_detect_method == WMM_DETECT_METHOD2)
		ops->detect_wmm_traffic = detect_wmm_traffic;

	/* register wifi mlme callback function */
	wifi_mlme_ops_register(wdev);
	return TRUE;
}

/**
 * @pAd
 * @wdev wifi device
 * @wdev_type wifi device type
 * @IfDev pointer to interface NET_DEV
 * @func_idx  _STA_TR_ENTRY index for BC/MC packet
 * @func_dev function device
 * @sys_handle pointer to pAd
 *
 * Initialize a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	INT32 wdev_idx = 0;

	wdev->wdev_type = wdev_type;
	wdev->client_num = 0;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = WCID_INVALID;
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;
	wdev->bAllowBeaconing = FALSE;
	wdev->radio_off_req = FALSE;
	wdev->btwt_id = 0;
	wdev->multicast_cnt = 0;
	wdev->MarkToClose = FALSE;
	RTMP_OS_INIT_COMPLETION(&wdev->ScanInfo.scan_complete);

	RTMP_SEM_EVENT_INIT(&wdev->wdev_op_lock, &pAd->RscSemMemList);
	wdev->wdev_op_lock_flag = FALSE;
	NdisZeroMemory(wdev->dbg_wdev_op_lock_caller, sizeof(wdev->dbg_wdev_op_lock_caller));

	wdev_idx = wdev_idx_reg(pAd, wdev);

	init_vie_ctrl(wdev);

	if (wdev_idx < 0)
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO, "(caller:%pS), wdev(%d)\n",
			 OS_TRACE, wdev->wdev_idx);

	return TRUE;
}

VOID wdev_init_for_bound_wdev(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	wdev->wdev_type = wdev_type;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = WCID_INVALID;
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;
	wdev->bAllowBeaconing = FALSE;
	wdev->radio_off_req = FALSE;
}


INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	/* band info (from pAd) that will used for mac address calculating */
	BOOLEAN isVlan = FALSE;
	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
	case WDEV_TYPE_GO:
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_AP);
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN, "(): wdevId%d = "MACSTR"\n",
				 wdev->wdev_idx, MAC2STR(wdev->if_addr));

		if (wdev->if_dev) {
			dev_addr_set(wdev->if_dev, wdev->if_addr);
			//NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
			//			   wdev->if_addr, MAC_ADDR_LEN);
		}

		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN, "wdevId%d = "MACSTR"\n",
				wdev->wdev_idx, MAC2STR(wdev->if_addr));

		if (wdev->if_dev)
			dev_addr_set(wdev->if_dev, wdev->if_addr);
			//NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev), wdev->if_addr, MAC_ADDR_LEN);
		break;
#endif
	default:
		break;
	}

	if (HcAcquireRadioForWdev(pAd, wdev) != HC_STATUS_OK)
		return FALSE;

	HcAcquireGroupKeyWcid(pAd, wdev, isVlan);

	return TRUE;
}


/**
 * @param pAd
 * @param wdev wifi device
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO, "(caller:%pS), wdev(%d)\n",
			 OS_TRACE, wdev->wdev_idx);

	wdev->client_num = 0;
	deinit_vie_ctrl(wdev);
	RTMP_SEM_EVENT_DESTORY(&wifi_dev->wdev_op_lock);
	RTMP_OS_COMPLETE_ALL(&wdev->ScanInfo.scan_complete);

	if (wdev->wdev_type != WDEV_TYPE_REPEATER) {
		wlan_operate_exit(wdev);
	}
	wdev_idx_unreg(pAd, wdev);
	return TRUE;
}


/**
 * @param pAd
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_config_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			wdev->channel = 0;
			wdev->PhyMode = 0;
		}
	}

	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT

/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *WdevSearchByBssid(RTMP_ADAPTER *pAd, UCHAR *Address)
{
    UINT16 Index;
    struct wifi_dev *wdev;

    NdisAcquireSpinLock(&pAd->WdevListLock);
	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(Address, wdev->bssid)) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
		"can not find registered wdev\n");
	return NULL;
}
#endif

VOID wdev_fsm_init(struct wifi_dev *wdev)
{
	sync_fsm_ops_init(wdev);
	cntl_state_machine_init(wdev, &wdev->cntl_machine, wdev->cntl_func);
	auth_fsm_init((PRTMP_ADAPTER)wdev->sys_handle, wdev, &wdev->auth_machine, wdev->auth_func);
	assoc_fsm_init((PRTMP_ADAPTER)wdev->sys_handle, wdev, &wdev->assoc_machine, wdev->assoc_func);
}

/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *address)
{
	UINT16 Index;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif
	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(address, wdev->if_addr)) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
#ifdef MAC_REPEATER_SUPPORT

	/* if we cannot found wdev from A2, it might comes from Rept entry.
	 * cause rept must bind the bssid of apcli_link,
	 * search A3(Bssid) to find the corresponding wdev.
	 */
	if (pAd->ApCfg.bMACRepeaterEn) {
		rept_entry = lookup_rept_entry(pAd, address);

		if (rept_entry)
			return &rept_entry->wdev;
	}

#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_DEBUG, "can not find registered wdev\n");
	return NULL;
}

struct wifi_dev *wdev_search_by_omac_idx(RTMP_ADAPTER *pAd, UINT8 BssIndex)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (wdev->OmacIdx == BssIndex) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR, "can not find registered wdev\n");
	return NULL;
}

struct wifi_dev *wdev_search_by_band_omac_idx(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 omac_idx)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (wdev->DevInfo.BandIdx == band_idx && wdev->OmacIdx == omac_idx) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR, "can not find registered wdev\n");
	return NULL;
}

inline struct wifi_dev *wdev_search_by_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	struct wifi_dev *wdev = NULL;
	UINT32 wdev_idx = RTMP_GET_PACKET_WDEV(pkt);

	wdev = pAd->wdev_list[wdev_idx];

	if (!wdev)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR, "error: wdev(wdev_idx = %d) is null from pkt\n", wdev_idx);

	return wdev;
}

inline struct wifi_dev *wdev_search_by_idx(RTMP_ADAPTER *pAd, UINT32 idx)
{
	struct wifi_dev *wdev = NULL;

	if (idx < WDEV_NUM_MAX)
		wdev = pAd->wdev_list[idx];

	if (!wdev)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR, "error: wdev(wdev_idx = %d) is null from idx\n", idx);

	return wdev;
}

inline struct wifi_dev *wdev_search_by_wcid(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct wifi_dev *wdev = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;

	if (!IS_WCID_VALID(pAd, wcid))
		return NULL;

	tr_entry = tr_entry_get(pAd, wcid);
	if (tr_entry)
		wdev = tr_entry->wdev;
	else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN,
				 "can not a valid wdev by wcid (%u)\n",
				  wcid);
	}

	return wdev;
}

struct wifi_dev *wdev_search_by_netdev(RTMP_ADAPTER *pAd, VOID *pDev)
{
	UCHAR i = 0;
	struct net_device *pNetDev = (struct net_device *)pDev;
	struct wifi_dev *wdev = NULL;

	if (pNetDev != NULL) {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev = pAd->wdev_list[i];

			if (wdev == NULL)
				continue;

			if (wdev->if_dev == pNetDev)
				return wdev;
#ifdef CONFIG_VLAN_GTK_SUPPORT
			else if (CFG80211_MatchVlandev(wdev, pNetDev))
				return wdev;
#endif
		}
	}

	return wdev;
}


void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2)
{
	UCHAR ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR vht_bw;
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_EHT_BE
	UCHAR eht_bw;
#endif /*DOT11_EHT_BE*/
	UCHAR ext_cha;
	UCHAR stbc;
	UCHAR ldpc;
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR ba_en;
	UCHAR mpdu_density;
#ifdef TXBF_SUPPORT
	UCHAR txbf;
#endif
	UCHAR max_mpdu_len;

	/*update configure*/
	if (wlan_config_get_ext_cha(dev1) == EXTCHA_NOASSIGN) {
		ext_cha = wlan_config_get_ext_cha(dev2);
		wlan_config_set_ext_cha(dev1, ext_cha);
	}

#ifdef TXBF_SUPPORT
	txbf = wlan_config_get_etxbf(dev2);
	wlan_config_set_etxbf(dev1, txbf);
	txbf = wlan_config_get_itxbf(dev2);
	wlan_config_set_itxbf(dev1, txbf);
#endif
	stbc = wlan_config_get_ht_stbc(dev2);
	wlan_config_set_ht_stbc(dev1, stbc);
	ldpc = wlan_config_get_ht_ldpc(dev2);
	wlan_config_set_ht_ldpc(dev1, ldpc);
	mpdu_density = wlan_config_get_min_mpdu_start_space(dev2);
	wlan_config_set_min_mpdu_start_space(dev1, mpdu_density);
	stbc = wlan_config_get_vht_stbc(dev2);
	wlan_config_set_vht_stbc(dev1, stbc);
	ldpc = wlan_config_get_vht_ldpc(dev2);
	wlan_config_set_vht_ldpc(dev1, ldpc);

	ht_bw = wlan_config_get_ht_bw(dev2);
	vht_bw = wlan_config_get_vht_bw(dev2);
	wlan_config_set_ht_bw(dev1, ht_bw);
	wlan_config_set_vht_bw(dev1, vht_bw);
	wlan_config_set_cen_ch_2(dev1, wlan_config_get_cen_ch_2(dev2));
#ifdef DOT11_EHT_BE
	eht_bw = wlan_config_get_eht_bw(dev2);
	wlan_config_set_eht_bw(dev1, eht_bw);
	tx_stream = wlan_config_get_eht_tx_nss(dev2);
	wlan_config_set_eht_tx_nss(dev1,
		min(wlan_config_get_eht_tx_nss(dev1), tx_stream));
	rx_stream = wlan_config_get_eht_rx_nss(dev2);
	wlan_config_set_eht_rx_nss(dev1,
		min(wlan_config_get_eht_rx_nss(dev1), rx_stream));
#endif /*DOT11_EHT_BE*/
	tx_stream = wlan_config_get_tx_stream(dev2);
	wlan_config_set_tx_stream(dev1, tx_stream);
	rx_stream = wlan_config_get_rx_stream(dev2);
	wlan_config_set_rx_stream(dev1, rx_stream);

	/* HT_BAWinSize */
	wlan_config_set_ba_txrx_wsize(dev1,
		wlan_config_get_ba_tx_wsize(dev2),
		wlan_config_get_ba_rx_wsize(dev2),
		ba_get_default_max_ba_wsize(dev2, NULL));

	/* VHT Max mpdu length */
	max_mpdu_len = wlan_config_get_vht_max_mpdu_len(dev2);
	wlan_config_set_vht_max_mpdu_len(dev1, max_mpdu_len);

#ifdef DOT11_HE_AX
	tx_stream = wlan_config_get_he_tx_nss(dev2);
	wlan_config_set_he_tx_nss(dev1,
		min(wlan_config_get_he_tx_nss(dev1), tx_stream));
	rx_stream = wlan_config_get_he_rx_nss(dev2);
	wlan_config_set_he_rx_nss(dev1,
		min(wlan_config_get_he_rx_nss(dev1), rx_stream));
#ifdef WIFI_TWT_SUPPORT
	/* STA/APCLI TWT */
	wlan_config_set_he_twt_support(dev1, wlan_config_get_he_twt_support(dev2));
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	ba_en = wlan_config_get_ba_enable(dev2);
	wlan_config_set_ba_enable(dev1, (ba_en > 0) ? ba_en : 0);
	dev1->channel = dev2->channel;
	dev1->quick_ch_change = dev2->quick_ch_change;
	wlan_config_set_ch_band(dev1, dev2->PhyMode);
	dev1->bWmmCapable = dev2->bWmmCapable;
	wlan_operate_update_ht_cap(dev1);

#ifdef MCAST_RATE_SPECIFIC
	/* temporary soluation due to WDS interface acutally reference */
	if (dev1->wdev_type == WDEV_TYPE_WDS) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		if (dev1->channel > 14)
			dev1->rate.MCastPhyMode_5G = dev2->rate.MCastPhyMode_5G;
		else
			dev1->rate.MCastPhyMode = dev2->rate.MCastPhyMode;
#else
		dev1->rate.mcastphymode = dev2->rate.mcastphymode;
#endif
	}
#endif
}


void wdev_sync_prim_ch(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UCHAR i = 0;
	struct wifi_dev *tdev;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = ad->wdev_list[i];

		if (tdev && (band_idx == HcGetBandByWdev(tdev)))
			tdev->channel = wdev->channel;

		if (tdev && ((tdev->wdev_type == WDEV_TYPE_AP) || (tdev->wdev_type == WDEV_TYPE_STA))
			&& ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_STA))) {
			tdev->quick_ch_change = wdev->quick_ch_change;
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"QuickChannel:%d; tdev_type:%d,tdev_idx:%d; wdev_type:%d,wdev_idx:%d==>\n",
			tdev->quick_ch_change, tdev->wdev_type, tdev->wdev_idx, wdev->wdev_type, wdev->wdev_idx);
		}
	}
}

void DoActionBeforeDownIntf(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
		"caller:%pS, wdev(%d)", OS_TRACE, wdev->BssIdx);

	wdev->MarkToClose = TRUE;

	wait_scan_stop(pAd, wdev);
}

void DoActionAfterDownIntf(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	wdev->MarkToClose = FALSE;
}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
#ifdef DOT11_N_SUPPORT
void wdev_sync_ht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ADD_HTINFO *add_ht_info)
{
	UCHAR mbss_idx = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	BOOLEAN adjustBW = FALSE;
	struct wifi_dev *mbss_wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"Entry Bw %d Ch %d==>\n", add_ht_info->RecomWidth, add_ht_info->ExtChanOffset);

	/*Moving all same band Soft AP interfaces to new BW proposed by RootAP */
	for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
		mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		if (mbss_wdev == NULL)
			continue;

		if (HcIsRadioAcq(mbss_wdev) && (band_idx == HcGetBandByWdev(mbss_wdev)))
			/* Same Band */
			adjustBW = TRUE;
		else if ((mbss_wdev->wdev_type == WDEV_TYPE_AP) &&
				(mbss_wdev->if_up_down_state == 0) &&
				(((mbss_wdev->channel <= 14) && (wdev->channel <= 14))))
			/* Different Band */
			adjustBW = TRUE;

		if (adjustBW) {
			wlan_config_set_ht_bw(mbss_wdev,
				add_ht_info->RecomWidth);
			wlan_config_set_ext_cha(mbss_wdev,
				add_ht_info->ExtChanOffset);

			/* Reset for other wdev's */
			adjustBW = FALSE;
		}
	}
}
#endif

#ifdef DOT11_VHT_AC
void wdev_sync_vht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR bw, UINT8 channel)
{
	UCHAR mbss_idx = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	BOOLEAN adjustCh = FALSE, adjustBw = TRUE;
	struct wifi_dev *mbss_wdev;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"Entry bw %d ch %d==>\n", bw, channel);

	if (bw >= VHT_BW_160)
		adjustBw = FALSE;

	/*Moving all same band Soft AP interfaces to new BW proposed by RootAP */
	for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
		mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		if (mbss_wdev == NULL)
			continue;

		if (HcIsRadioAcq(mbss_wdev) && (band_idx == HcGetBandByWdev(mbss_wdev)))
			/* Same Band */
			adjustCh = TRUE;
		else if ((mbss_wdev->wdev_type == WDEV_TYPE_AP) &&
				(mbss_wdev->if_up_down_state == 0) &&
				(((mbss_wdev->channel > 14) && (wdev->channel > 14))))
			/* Different Band */
			adjustCh = TRUE;

		if (adjustCh) {
			wlan_operate_set_cen_ch_2(mbss_wdev, channel);

			if (adjustBw)
				wlan_operate_set_vht_bw(mbss_wdev, bw);

			/* Reset for other wdev's */
			adjustCh = FALSE;
		}
	}
}
#endif

BOOLEAN IS_SYNC_BW_POLICY_VALID(struct _RTMP_ADAPTER *pAd, BOOLEAN isHTPolicy, UCHAR policy)
{
	BOOLEAN status = FALSE;

	if (isHTPolicy) {
		if (1 & (pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncHTSupport >> policy))
			status = TRUE;
	} else {
		if (1 & (pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncVHTSupport >> policy))
			status = TRUE;
	}

	return status;
}
#endif

VOID phy_sync_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *ap_wdev = NULL;

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		int mbss_idx;

		/*for 5G+5G case choose both phymode & func_idx the same first.*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode && wdev->func_idx == mbss_idx) {
				ap_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
				update_att_from_wdev(wdev, ap_wdev);
				break;
			}
		}

		if (ap_wdev)
			return;

		/*original rule*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode) {
				update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[mbss_idx].wdev);
				break;
			}
		}
	} else {
		/* align phy mode to BSS0 by default */
		if (wdev->PhyMode == WMODE_INVALID)
			wdev->PhyMode = pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode;
		update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[BSS0].wdev);
	}
}

VOID wdev_if_up_down(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN if_up_down_state)
{
	if (wdev == NULL)
		return;

	wdev->if_up_down_state = if_up_down_state;

	if (if_up_down_state == FALSE) { /* clear counter in inf down */
		wdev->rx_drop_long_len = 0;
		wdev->multicast_cnt = 0;
	}
}

void wdev_sync_ch_by_rfic(struct _RTMP_ADAPTER *ad, UCHAR rfic, UCHAR ch)
{
	UCHAR i;
	struct wifi_dev *dev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		dev = ad->wdev_list[i];

		if (dev &&
			(wmode_2_rfic(dev->PhyMode) & rfic) &&
			(dev->channel != ch)
		   )
			dev->channel = ch;
	}
}

/*
* wifi sys operate action, must used in dispatch level task, do not use in irq/tasklet/timer context
*/

INT wdev_do_open(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->open)
		return wdev->wdev_ops->open(wdev);

	return FALSE;
}

INT wdev_do_close(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->close)
		return wdev->wdev_ops->close(wdev);

	return FALSE;
}

INT wdev_do_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef DOT11_EHT_BE
	if (entry && entry->mlo.mlo_en) {
		/* mlo case */
		if (wdev->wdev_type == WDEV_TYPE_STA)
			if (!sta_mld_link_up(wdev, entry))
				return TRUE;
	}
#endif /* DOT11_EHT_BE */

	if (wdev->wdev_ops && wdev->wdev_ops->linkup)
		return wdev->wdev_ops->linkup(wdev, entry);

	return FALSE;
}

INT wdev_do_linkdown(struct wifi_dev *wdev)
{
#ifdef DOT11_EHT_BE
	if (wdev->wdev_type == WDEV_TYPE_STA)
		sta_mld_link_down(wdev);
#endif /* DOT11_EHT_BE */

	if (wdev->wdev_ops && wdev->wdev_ops->linkdown)
		return wdev->wdev_ops->linkdown(wdev);

	return FALSE;
}

INT wdev_do_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef DOT11_EHT_BE
	/* mlo case */
	if (entry && entry->mlo.mlo_en) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			UINT16 mld_sta_idx;
			struct mld_entry_t *mld_entry;

			if (!entry->mlo.is_setup_link_entry)
				return TRUE;

			mt_rcu_read_lock();
			mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
					"mld_entry is NULL\n");
				return FALSE;
			}
			mld_sta_idx = mld_entry->mld_sta_idx;
			mt_rcu_read_unlock();

			return (bss_mngr_mld_conn_act(wdev, mld_sta_idx) == NDIS_STATUS_SUCCESS) ?
				TRUE : FALSE;
		} else if (wdev->wdev_type == WDEV_TYPE_STA)
			if (!sta_mld_connect_act(wdev, entry))
				return TRUE;
	}
#endif /* DOT11_EHT_BE */

	if (wdev->wdev_ops && wdev->wdev_ops->conn_act)
		return wdev->wdev_ops->conn_act(wdev, entry);

	return FALSE;
}

INT wdev_do_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef DOT11_EHT_BE
	/* mlo case */
	if (entry && entry->mlo.mlo_en) {
		if (wdev->wdev_type == WDEV_TYPE_STA)
			if (sta_mld_disconnect_act(wdev, entry))
				return TRUE;
	}
#endif /* DOT11_EHT_BE */

	if (entry && wdev->wdev_ops && wdev->wdev_ops->disconn_act)
		return wdev->wdev_ops->disconn_act(wdev, entry);

	return FALSE;
}

VOID wdev_phy_default_init(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UINT8 band_idx = hc_get_hw_band_idx(ad);
	UCHAR cfg_mode, cfg_ch;
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	CHANNEL_CTRL *pChCtrl;
	struct _RTMP_ADAPTER *band_ad;
	/*
	 * Default Band0: 2G, Band1: 5G, Band2: 6G
	 */
	switch (band_idx) {
	case BAND1:
		cfg_ch = 36;
		if (chip_cap->phy_caps & fPHY_CAP_EHT) {
			cfg_mode = PHY_11BE_5G;
			break;
		}
		if (chip_cap->phy_caps & fPHY_CAP_HE) {
			cfg_mode = PHY_11AX_5G;
			break;
		}
		cfg_mode = PHY_11VHT_N_A_MIXED;
		break;
	case BAND2:
		cfg_ch = 37;
		if (chip_cap->phy_caps & fPHY_CAP_EHT) {
			cfg_mode = PHY_11BE_6G;
			break;
		}
		if (chip_cap->phy_caps & fPHY_CAP_HE) {
			cfg_mode = PHY_11AX_6G;
			break;
		}
		cfg_mode = PHY_11VHT_N_A_MIXED;
		break;
	case BAND0:
	default:
		cfg_ch = 6;
		if (chip_cap->phy_caps & fPHY_CAP_EHT) {
			cfg_mode = PHY_11BE_24G;
			break;
		}
		if (chip_cap->phy_caps & fPHY_CAP_HE) {
			cfg_mode = PHY_11AX_24G;
			break;
		}
		cfg_mode = PHY_11VHT_N_ABG_MIXED;
		break;
	}
	band_ad = physical_device_get_mac_adapter_by_band(ad->physical_dev, band_idx);
	wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
	wlan_config_set_ch_band(wdev, wdev->PhyMode);
	wdev->bWmmCapable = 1;
	wdev->channel = cfg_ch;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_HE_AX
	if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
		wlan_config_set_he6g_op_present(wdev, 1);
	else
		wlan_config_set_he6g_op_present(wdev, 0);
#endif /* #ifdef DOT11_HE_AX */
#endif /* CONFIG_AP_SUPPORT */
	if (band_ad) {
		pChCtrl = hc_get_channel_ctrl(band_ad->hdev_ctrl);
		if (pChCtrl)
			pChCtrl->ch_cfg.boot_chan = wdev->channel;
		MTWF_DBG(band_ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
			"PhyMode = 0x%x, Channel = %d\n", wdev->PhyMode, wdev->channel);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		SET_APCLI_SYNC_PEER_DEAUTH_ENBL(band_ad, FALSE);
#endif
	}
	wlan_config_set_ext_cha(wdev, EXTCHA_BELOW);
}

VOID wdev_ap_default_init(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	UINT i;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	wdev_phy_default_init(ad, wdev);
#ifdef UAPSD_SUPPORT
	UAPSD_INFO_INIT(&wdev->UapsdInfo);
#ifdef RT_CFG80211_SUPPORT
	wdev->SecConfig.PairwiseKeyId_set_flag = 0;
#endif /* RT_CFG80211_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	MAP_Init(ad, wdev, WDEV_TYPE_AP);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DPP_R2_SUPPORT
	NdisZeroMemory(wdev->DPPCfg.cce_ie_buf, 6);
	wdev->DPPCfg.cce_ie_len = 0;
#endif /* DPP_R2_SUPPORT */
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	SET_V10_OLD_CHNL_VALID(wdev, FALSE);
#endif /* DFS_VENDOR10_CUSTOM_FEATURE*/

#ifdef DOT1X_SUPPORT
	/* dot1x related per BSS */
	wdev->SecConfig.IEEE8021X = FALSE;
#ifndef RT_CFG80211_SUPPORT
	wdev->SecConfig.radius_srv_num = 0;
	wdev->SecConfig.NasIdLen = 0;
	wdev->SecConfig.PreAuth = FALSE;
#ifdef RADIUS_MAC_ACL_SUPPORT
	NdisZeroMemory(&wdev->SecConfig.RadiusMacAuthCache, sizeof(RT_802_11_RADIUS_ACL));
	/* Default Timeout Value for 1xDaemon Radius ACL Cache */
	wdev->SecConfig.RadiusMacAuthCacheTimeout = 30;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
#endif /* DOT1X_SUPPORT */

	/* VLAN related */
#ifdef VLAN_SUPPORT
	wdev->bVLAN_Tag = 0;
	wdev->VLAN_VID = 0;
	wdev->VLAN_Priority = 0;
	wdev->VLAN_Policy[TX_VLAN] = VLAN_TX_ALLOW;
	wdev->VLAN_Policy[RX_VLAN] = 0;
#endif /* VLAN_SUPPORT */
#ifdef CONFIG_VLAN_GTK_SUPPORT
	INIT_LIST_HEAD(&wdev->vlan_gtk_list);
	wdev->vlan_cnt = 0;
#endif /* CONFIG_VLAN_GTK_SUPPORT */

	/* Default MCS as AUTO*/
	wdev->bAutoTxRateSwitch = TRUE;
	wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
	wdev->bcn_buf.stop_tx = STOP_BCN_TX_DISABLE;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	wdev->eap.eap_mgmrate_en = 0x00;
	wdev->eap.mgmphymode[MGMT_TYPE].field.MODE = MODE_OFDM;
	wdev->eap.mgmphymode[MGMT_TYPE].field.MCS = MCS_RATE_6;
	wdev->eap.mgmphymode[MGMT_TYPE].field.BW = BW_20;
	if ((wdev->channel < 14) && (wdev->channel != 0)) {
		wdev->eap.mgmphymode[MGMT_TYPE].field.MODE = MODE_CCK;
		wdev->eap.mgmphymode[MGMT_TYPE].field.MCS = RATE_11;
		wdev->eap.mgmphymode[MGMT_TYPE].field.BW = BW_20;
	}
	for (i = BCN_TYPE; i < MGMT_MAX_TYPE; i++)
		wdev->eap.mgmphymode[i].word = wdev->eap.mgmphymode[MGMT_TYPE].word;

	wdev->eap.eap_hprirate_en = 0x00;
	for (i = HIGHPRI_ARP; i < HIGHPRI_MAX_TYPE; i++) {
		wdev->eap.hpriphymode[i].field.MODE = MODE_OFDM;
		wdev->eap.hpriphymode[i].field.MCS = MCS_RATE_6;
		wdev->eap.hpriphymode[i].field.BW = BW_20;
	}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	wdev->rate.MCastPhyMode.field.MODE = MODE_OFDM;
	wdev->rate.MCastPhyMode.field.MCS = MCS_RATE_6;
	wdev->rate.MCastPhyMode_5G.field.MODE = MODE_OFDM;
	wdev->rate.MCastPhyMode_5G.field.MCS = MCS_RATE_6;
#else
	wdev->rate.mcastphymode.field.MODE = MODE_OFDM;
	wdev->rate.mcastphymode.field.MCS = MCS_RATE_6;
#endif
#endif /* MCAST_RATE_SPECIFIC */
	ie_filter_init(ad, wdev);
#ifdef WSC_INCLUDED
	ApWscDefaultInit(ad, wdev);
#endif /* WSC_INCLUDED */
#endif /* CONFIG_AP_SUPPORT */
}

VOID wdev_sta_default_init(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef CONFIG_STA_SUPPORT
	wdev_phy_default_init(ad, wdev);
#ifdef CONFIG_MAP_SUPPORT
	MAP_Init(ad, wdev, WDEV_TYPE_STA);
#endif /* CONFIG_MAP_SUPPORT */
	wdev->bAutoTxRateSwitch = TRUE;
	wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
#ifdef VLAN_SUPPORT
	wdev->bVLAN_Tag = 0;
	wdev->VLAN_VID = 0;
	wdev->VLAN_Priority = 0;
	wdev->VLAN_Policy[TX_VLAN] = VLAN_TX_ALLOW;
	wdev->VLAN_Policy[RX_VLAN] = 0;
#endif /* VLAN_SUPPORT */
	wdev->UapsdInfo.bAPSDCapable = FALSE;
#ifdef APCLI_CFG80211_SUPPORT
#ifdef DOT1X_SUPPORT
	wdev->SecConfig.IEEE8021X = FALSE;
#endif /* DOT1X_SUPPORT */
#endif /* APCLI_CFG80211_SUPPORT */
#ifdef UAPSD_SUPPORT
	wdev->UapsdInfo.bAPSDCapable = FALSE;
#endif /* UAPSD_SUPPORT */
	wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	wdev->bLinkUpDone = FALSE;
	atomic_set(&wdev->bGotEapolPkt, FALSE);

	if (!wdev->pEapolPktFromAP) {
		os_alloc_mem(NULL,
			(UCHAR **)&wdev->pEapolPktFromAP,
			sizeof(*wdev->pEapolPktFromAP));

		if (!wdev->pEapolPktFromAP)
			MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
				"pEapolPktFromAP alloc fail!!!\n");
		else
			os_zero_mem(wdev->pEapolPktFromAP, sizeof(struct _RX_BLK));
	} else
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_WARN,
			"non-NULL pEapolPktFromAP 0x%p\n",
			wdev->pEapolPktFromAP);
#ifdef WSC_INCLUDED
	StaWscDefaultInit(ad, wdev);
#endif /* WSC_INCLUDED */
#endif /* CONFIG_STA_SUPPORT */
}

