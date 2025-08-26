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
/****************************************************************************
 ***************************************************************************/

/****************************************************************************
 *	Abstract:
 *
 *	All related CFG80211 P2P function body.
 *
 *	History:
 *
 ****************************************************************************/

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

#if defined(DOT11_EHT_BE)
#define MAX_NEI_NUM 4
#define BASIC_MULTI_LINK_ID 201
#endif

#ifdef GREENAP_SUPPORT
BOOLEAN greenap_get_allow_status(RTMP_ADAPTER *ad);
#endif

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	/* UCHAR lock_channel = CFG80211_getCenCh(pAd, Data); */
	UCHAR lock_channel = Data;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;

	if (RTMP_CFG80211_HOSTAPD_ON(pAd))
		return;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
			"invalid ioctl_if:%d\n", pObj->ioctl_if);
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	if (pAd->LatchRfRegs.Channel != lock_channel)
	{

		wlan_operate_set_prim_ch(wdev, lock_channel);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
			"Off-Channel Send Packet: From(%d)-To(%d)\n",
			pAd->LatchRfRegs.Channel, lock_channel);
	} else
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel);
}

#ifdef CONFIG_AP_SUPPORT

#ifdef DISABLE_HOSTAPD_PROBE_RESP
/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients for hostapd
	Parameters:
		apidx
	==========================================================================
 */
extern INT build_country_power_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
extern INT build_ch_switch_announcement_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);


VOID CFG80211_SyncPacketWpsIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen, UINT8 apidx, UINT8 *da)
{
	const UCHAR *wsc_ie  = NULL;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	EID_STRUCT *eid;
	const UINT WFA_OUI = 0x0050F2;

	wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4, pData, dataLen);
	if (wsc_ie != NULL) {

		eid = (EID_STRUCT *)wsc_ie;

		if (eid->Len + 2 <= 500) {
			NdisCopyMemory(pMbss->wdev.WscIEProbeResp.Value, wsc_ie, eid->Len + 2);
			pMbss->wdev.WscIEProbeResp.ValueLen = eid->Len + 2;
		}

	}
	return;
}


#endif /*DISABLE_HOSTAPD_PROBE_RESP */

BOOLEAN CFG80211_SyncBssWmmCap(RTMP_ADAPTER *pAd, UINT mbss_idx, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	EDCA_PARM *pBssEdca = NULL;
	UCHAR i;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);
	if (wmm_ie != NULL) {
		wdev->bWmmCapable = TRUE;
		pBssEdca = wlan_config_get_ht_edca(wdev);
		if (pBssEdca) {
			for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
				pBssEdca->Aifsn[i] = wmm_ie[10 + i * 4] & 0x0f;
				pBssEdca->Cwmin[i] = wmm_ie[11 + i * 4] & 0x0f;
				pBssEdca->Cwmax[i] = wmm_ie[11 + i * 4] >> 4;
				pBssEdca->Txop[i] =
					wmm_ie[12 + i * 4] | (wmm_ie[13 + i * 4] << 8);
			}
		} else
			return FALSE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_WMM, DBG_LVL_ERROR,
			"can't find the wmm ie, disable WmmCapable\n");
		wdev->bWmmCapable = FALSE;
	}
	pAd->ApCfg.MBSSID[mbss_idx].bWmmCapableOrg = wdev->bWmmCapable;
#ifdef DOT11_N_SUPPORT
	/*Sync with the HT relate info. In N mode, we should re-enable it */
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}

#define QOS_INFO_OFFSET 8
#define UAPSD_ENABLE 0x80
VOID CFG80211_SyncBssUAPSDCap(RTMP_ADAPTER *pAd, UINT mbss_idx, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	UCHAR qos_info = 0;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);
	if (wmm_ie != NULL) {
		qos_info = wmm_ie[QOS_INFO_OFFSET];

		if (qos_info & UAPSD_ENABLE)
			wdev->UapsdInfo.bAPSDCapable = TRUE;
		else
			wdev->UapsdInfo.bAPSDCapable = FALSE;
	}
}

BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;
	EDCA_PARM *pBssEdca = NULL;

	/* hex_dump("probe_rsp_in:", pData, dataLen); */
	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);

	if (wmm_ie != NULL) {
		UINT i;

		pBssEdca = wlan_config_get_ht_edca(wdev);
		if (pBssEdca) {
			/* WMM: sync from driver's EDCA parameter */
			for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
				wmm_ie[10 + (i * 4)] = (i << 5) +                                  /* b5-6 is ACI */
									   ((UCHAR)pBssEdca->bACM[i] << 4) +           /* b4 is ACM */
									   (pBssEdca->Aifsn[i] & 0x0f);                /* b0-3 is AIFSN */
				wmm_ie[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4) +                 /* b5-8 is CWMAX */
									   (pBssEdca->Cwmin[i] & 0x0f);                /* b0-3 is CWMIN */
				wmm_ie[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);          /* low byte of TXOP */
				wmm_ie[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);            /* high byte of TXOP */
			}
		}
		return TRUE;
	} else
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_WMM, DBG_LVL_DEBUG,
			"can't find the wmm ie\n");

	return FALSE;
}
#endif /* CONFIG_AP_SUPPORT */

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT) || defined(CCN67_SPLIT_MAC_SUPPORT)
VOID CFG80211_AuthRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct ieee80211_mgmt *mgmt = NULL;
	MLME_QUEUE_ELEM *mlme_entry = NULL;
	UINT8 apidx = 0;

	if (!pData) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				"pData is null\n");
		return;
	}

	mgmt = (struct ieee80211_mgmt *)pData;
	apidx = get_apidx_by_addr(pAd, mgmt->sa);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
		"[cfg80211] send auth resp to "MACSTR" (alg=%d, transaction=%d, status=%d)...\n",
		MAC2STR(mgmt->da), mgmt->u.auth.auth_alg, mgmt->u.auth.auth_transaction, mgmt->u.auth.status_code);

	if (MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_HOSTAPD_AUTH_RSP,
					Data, pData, 0, wdev, TRUE, &mlme_entry)) {
		RTMP_MLME_HANDLER(pAd);
		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&(mlme_entry->mlme_ack_done),
			RTMPMsecsToJiffies(500)))
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				"MLME Timeout!!\n");
	}
	else
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "enq fail\n");

	return;
}

VOID CFG80211_AssocRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct ieee80211_mgmt *mgmt = NULL;
	MLME_QUEUE_ELEM *mlme_entry = NULL;
	UINT8 apidx = 0;
	UCHAR isReassoc;

	if (!pData) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"pData is null\n");
		return;
	}

	mgmt = (struct ieee80211_mgmt *)pData;
	isReassoc = ieee80211_is_reassoc_resp(mgmt->frame_control);
	apidx = get_apidx_by_addr(pAd, mgmt->sa);

	if (isReassoc)
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"[cfg80211] send re-assoc resp to "MACSTR" (status=%d)...\n",
			MAC2STR(mgmt->da), mgmt->u.reassoc_resp.status_code);
	else
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"[cfg80211] send assoc resp to "MACSTR" (status=%d)...\n",
			MAC2STR(mgmt->da), mgmt->u.assoc_resp.status_code);

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "wdev is null\n");
		return;
	}

	if (MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_HOSTAPD_ASSOC_RESP_HANDLER,
					Data, pData, 0, wdev, TRUE, &mlme_entry)) {
		RTMP_MLME_HANDLER(pAd);
		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&(mlme_entry->mlme_ack_done),
			RTMPMsecsToJiffies(1000)))
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"MLME Timeout!!\n");
		}
	else
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "enq fail\n");

	return;
}

#endif /* HOSTAPD_11R_SUPPORT */

NDIS_STATUS CFG80211_LEGACY_BtmReqHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	struct BSS_TM_REQ *pPayload = NULL;
	BSS_ENTRY *pBssEntry = NULL;
	BSS_TABLE *ScanTab = NULL;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct ieee80211_mgmt *mgmt = NULL;
	UINT i = 0;
	UINT url_len = 0;
	ULONG bss_index = 0;
	UINT8 apidx = 0;
	BOOLEAN pref = 0;
#ifdef MBO_SUPPORT
	ULONG MboIndex = 0;
	ULONG MboLen = 0;
#endif /* MBO_SUPPORT */

	if (!pData) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"pData is null\n");
		return NDIS_STATUS_FAILURE;
	}

	if (Data > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"BTM Requset length is invalid\n");
		return NDIS_STATUS_FAILURE;
	}

	mgmt = (struct ieee80211_mgmt *)pData;
	apidx = get_apidx_by_addr(pAd, mgmt->sa);

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"wdev is null\n");
		return NDIS_STATUS_FAILURE;
	}

	pPayload = (struct BSS_TM_REQ *)(&mgmt->u);
	if (pPayload->action == BSS_TRANSITION_REQ)
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"[cfg80211] send btm request to "MACSTR".\n", MAC2STR(mgmt->da));
	else {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Not a btm request\n");
		return NDIS_STATUS_FAILURE;
	}

	if (pPayload->req_mode & BTM_TERMINATION_OFFSET &&
		pPayload->variable[i] == WNM_BSS_TERMINATION_SUBIE) {
		u32 reg_hw_tsf[2] = {0};  /* register hw tsf */
		UINT64 tsf_timer = 0;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"BSS Termination Duration (optional) Included\n");
		HW_GET_TSF(wdev, reg_hw_tsf);
		tsf_timer = (UINT64)reg_hw_tsf[0] + ((UINT64)reg_hw_tsf[1] << 32);
		tsf_timer = tsf_timer + (pPayload->variable[i + 2] * 1000000); /* sec to usec */
		NdisCopyMemory(&pPayload->variable[i + 2], &tsf_timer, 8);
		i += 2 + 10; /* Skip elemnt id, length, BSS Termination TSF and Duration*/
	}

	if (pPayload->req_mode & BTM_CANDIDATE_OFFSET) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Preferred Candidate List Included\n");
		pref = TRUE;
	}

	if (pPayload->req_mode & BTM_URL_OFFSET) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Session Information URL (optional) Included\n");
		url_len = pPayload->variable[i];
		if (url_len > URL_LEN) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"url_len is invalid\n");
			return NDIS_STATUS_FAILURE;
		}
		i += 1 + url_len; /* Skip URL length and url */
		if (i >= Data) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"url_len exceed Frame Len\n");
			return NDIS_STATUS_FAILURE;
		} else if (pref && (pPayload->variable[i] != IE_RRM_NEIGHBOR_REP)
				&& (pPayload->variable[i] != IE_VENDOR_SPECIFIC)) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Frame format error\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	while (pPayload->variable[i] == IE_RRM_NEIGHBOR_REP) {
		struct NEIGHBOR_REPORT_INFO *pNei_payload = NULL;
		UCHAR Nei_len = 0;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"BSS Transition Candidate List Entries (optional) is included\n");
		Nei_len = pPayload->variable[i + 1];
		i += 2; /* Skip elment id and length */

		pNei_payload = (struct NEIGHBOR_REPORT_INFO *)(&pPayload->variable[i]);
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Neighbor Report Entriy Bssid="MACSTR", Channel=%d\n",
			MAC2STR(pNei_payload->bssid), pNei_payload->channel);

		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		bss_index = BssTableSearch(ScanTab, pNei_payload->bssid, pNei_payload->channel);
		if (bss_index == BSS_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_WARN,
				"bss="MACSTR", not found\n",
				MAC2STR(pNei_payload->bssid));

			i += Nei_len;
			continue;
		}
		if (bss_index >= MAX_LEN_OF_BSS_TABLE) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_WARN,
				"bss index invalid\n");

			i += Nei_len;
			continue;
		}

		pBssEntry = &ScanTab->BssEntry[bss_index];
		if (pNei_payload->op_class == 0)
			pNei_payload->op_class = pBssEntry->RegulatoryClass;

		if (pBssEntry->Channel > 14) {
			if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) {
#ifdef DOT11_VHT_AC
				if (HAS_VHT_CAPS_EXIST(pBssEntry->ie_exists))
					pBssEntry->CondensedPhyType = 9;
				else
#endif /* DOT11_VHT_AC */
					pBssEntry->CondensedPhyType = 7;
			} else
				pBssEntry->CondensedPhyType = 4; /*OFDM case*/
		} else {
			if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) /*HT case*/
				pBssEntry->CondensedPhyType = 7;
			else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /*ERP case*/
				pBssEntry->CondensedPhyType = 6;
			else if (pBssEntry->SupRateLen > 4)/*OFDM case(1,2,5.5,11 for CCK 4 Rates)*/
				pBssEntry->CondensedPhyType = 4;
			/* no CCK's definition in spec. */
		}
		if (pNei_payload->phy_type == 0)
			pNei_payload->phy_type = pBssEntry->CondensedPhyType;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Neighbor Report Entriy->Bssid="MACSTR
			", BssidInfo=%u, Op_Class=%d, Channel=%d, Phy_Tpye=%d\n",
			MAC2STR(pNei_payload->bssid), pNei_payload->bssid_info,
			pNei_payload->op_class, pNei_payload->channel, pNei_payload->phy_type);

		i += Nei_len;
	}

#ifdef MBO_SUPPORT
	MboIndex = sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i;
	if (MboIndex < Data) {
		PUCHAR pBuf = (PUCHAR)pData;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
				"Check MBO Sub Element\n");

		MboLen = Data - MboIndex;

		if (pBuf[MboIndex + 1] != MboLen - 2 || pBuf[MboIndex] != IE_VENDOR_SPECIFIC
			|| pBuf[MboIndex + 5] != MBO_OCE_OUI_TYPE) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_WARN,
				"MBO IE is invalid\n");
			return NDIS_STATUS_FAILURE;
		}

		if ((sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i + MboLen) != Data) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"The BTM Request in invalid\n");
			return NDIS_STATUS_FAILURE;
		}
	} else if ((sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i) != Data) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"The BTM Request in invalid\n");
		return NDIS_STATUS_FAILURE;
	}
#endif /* MBO_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

#if defined(DOT11_EHT_BE)
VOID mlo_neighbor_per_sta_profie_set_entry(RTMP_ADAPTER *pAd,
	u8 *pData, UINT idx, struct wifi_dev *wdev,
	struct per_sta_profile_nei_entry *neighbor_table)
{
	u8 i = 0;
	u8 k = 0;
	u8 entry_idx = 0;
	BSS_TABLE *ScanTab = NULL;
	ULONG bss_index = 0;
	BSS_ENTRY *pBssEntry = NULL;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"%s ==>\n", __func__);

	/* Parse Neighbor Report IE, just save mlo neighbor's other link mac addr */
	while (pData[idx] == IE_RRM_NEIGHBOR_REP) {
		struct NEIGHBOR_REPORT_INFO *pNei_payload = NULL;
		UCHAR Nei_len = 0;

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"%s: BSS Transition Candidate List Entries (optional) is included\n", __func__);
		Nei_len = pData[idx + 1];
		idx += 2; /* Skip elment id and length */

		pNei_payload = (struct NEIGHBOR_REPORT_INFO *)(&pData[idx]);
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"%s: Neighbor Report Entriy Bssid="MACSTR", Channel=%d\n",
			__func__, MAC2STR(pNei_payload->bssid), pNei_payload->channel);

		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		bss_index = BssTableSearch(ScanTab, pNei_payload->bssid, pNei_payload->channel);
		if (bss_index == BSS_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"bss="MACSTR", not found\n",
				MAC2STR(pNei_payload->bssid));

			idx += Nei_len;
			continue;
		}
		if (bss_index >= MAX_LEN_OF_BSS_TABLE) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"bss index invalid\n");

			idx += Nei_len;
			continue;
		}

		pBssEntry = &ScanTab->BssEntry[bss_index];

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"%s: Neighbor Report Entriy->Bssid="MACSTR
			", BssidInfo=%u, Op_Class=%d, Channel=%d, Phy_Tpye=%d\n",
			__func__, MAC2STR(pNei_payload->bssid), pNei_payload->bssid_info,
			pNei_payload->op_class, pNei_payload->channel, pNei_payload->phy_type);

		if (pBssEntry->ml_capable) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"%s: Neighbor is a MLD, check whether it includes per-sta profile\n", __func__);

			COPY_MAC_ADDR(neighbor_table[entry_idx].setup_link_addr, pNei_payload->bssid);
			neighbor_table[entry_idx].setup_link_channel = pNei_payload->channel;

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"%s: pBssEntry->ml_info.mld_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, PRINT_MAC(pBssEntry->ml_info.mld_addr));
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"%s: pBssEntry->ml_info.link_num = %d\n", __func__, pBssEntry->ml_info.link_num);

			for (i = 0; i < pBssEntry->ml_info.link_num; i++) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
						"%s: setup link addr="MACSTR", non setup link addr= ="MACSTR"\n", __func__,
						MAC2STR(pBssEntry->ml_info.mld_addr),
						MAC2STR(pBssEntry->ml_info.sta_profiles[i].link_addr));
			}

			for (i = 0; i < pBssEntry->ml_info.link_num; i++) {
				if (!MAC_ADDR_EQUAL(pBssEntry->ml_info.sta_profiles[i].link_addr, ZERO_MAC_ADDR)
					&& !MAC_ADDR_EQUAL(pBssEntry->ml_info.sta_profiles[i].link_addr, pNei_payload->bssid)
					&& entry_idx < MAX_NEI_NUM && k < MAX_NON_SETUP_LINK) {
					COPY_MAC_ADDR(neighbor_table[entry_idx].non_setup_link_addr[k],
						pBssEntry->ml_info.sta_profiles[i].link_addr);
					MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
						"%s: setup link addr="MACSTR", non setup link addr= ="MACSTR"\n", __func__,
						MAC2STR(neighbor_table[entry_idx].setup_link_addr),
						MAC2STR(neighbor_table[entry_idx].non_setup_link_addr[k]));
					k++;
					entry_idx++;
				}
			}
		}

		idx += Nei_len;
	}
}

ULONG mlo_neighbor_per_sta_profie_search(RTMP_ADAPTER *pAd,
	struct per_sta_profile_nei_entry *neighbor_table,
	UCHAR *pBssid, struct wifi_dev *wdev)
{
	u8 i, j = 0;
	BSS_TABLE *ScanTab = NULL;
	ULONG bss_index = 0;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"%s ==>\n", __func__);

	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	for (i = 0; i < MAX_NEI_NUM; i++) {
		for (j = 0; j < MAX_NON_SETUP_LINK; j++) {
			if (MAC_ADDR_EQUAL(neighbor_table[i].non_setup_link_addr[j], pBssid)) {
				bss_index = BssTableSearch(ScanTab, neighbor_table[i].setup_link_addr,
					neighbor_table[i].setup_link_channel);
				return bss_index;
			}
		}
	}

	return (ULONG)BSS_NOT_FOUND;
}

VOID mlo_neighbor_per_sta_profie_clear(
	struct per_sta_profile_nei_entry *neighbor_table)
{
	u8 i, j = 0;

	for (i = 0; i < MAX_NEI_NUM; i++) {
		neighbor_table[i].setup_link_channel = 0;
		COPY_MAC_ADDR(neighbor_table[i].setup_link_addr, ZERO_MAC_ADDR);

		for (j = 0; j < MAX_NON_SETUP_LINK; j++) {
			COPY_MAC_ADDR(neighbor_table[i].non_setup_link_addr[j],
						ZERO_MAC_ADDR);
		}
	}
}

NDIS_STATUS CFG80211_MLO_BtmReqHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	struct BSS_TM_REQ *pPayload = NULL;
	BSS_ENTRY *pBssEntry = NULL;
	BSS_TABLE *ScanTab = NULL;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct ieee80211_mgmt *mgmt = NULL;
	PUCHAR pOutBuffer = NULL;
	PHEADER_802_11 pBtmReqHdr;
	UINT i = 0;
	UINT url_len = 0;
	ULONG bss_index = 0;
	UINT8 apidx = 0;
	BOOLEAN pref_mode = 0;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	ULONG MboIndex = 0;
	struct per_sta_profile_nei_entry neighbor_table[MAX_NEI_NUM];

	if (!pData) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"pData is null\n");
		return NDIS_STATUS_FAILURE;
	}

	if (Data > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"BTM Requset length is invalid\n");
		return NDIS_STATUS_FAILURE;
	}

	mgmt = (struct ieee80211_mgmt *)pData;
	apidx = get_apidx_by_addr(pAd, mgmt->sa);

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
			"wdev is null\n");
		return NDIS_STATUS_FAILURE;
	}

	pPayload = (struct BSS_TM_REQ *)(&mgmt->u);
	if (pPayload->action == BSS_TRANSITION_REQ)
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"[cfg80211] send mlo btm request to "MACSTR".\n", MAC2STR(mgmt->da));
	else {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"Not a btm request\n");
		return NDIS_STATUS_FAILURE;
	}

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"MlmeAllocateMemory fail\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Copy 80211 Header*/
	pBtmReqHdr = (PHEADER_802_11)pData;
	NdisMoveMemory(&pOutBuffer[FrameLen], pBtmReqHdr, sizeof(HEADER_802_11));
	FrameLen += sizeof(HEADER_802_11);

	/* Parse BTM Requset Mode */
	if (pPayload->req_mode & BTM_TERMINATION_OFFSET &&
		pPayload->variable[i] == WNM_BSS_TERMINATION_SUBIE) {
		u32 reg_hw_tsf[2] = {0};  /* register hw tsf */
		UINT64 tsf_timer = 0;

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"BSS Termination Duration (optional) Included\n");
		HW_GET_TSF(wdev, reg_hw_tsf);
		tsf_timer = (UINT64)reg_hw_tsf[0] + ((UINT64)reg_hw_tsf[1] << 32);
		tsf_timer = tsf_timer + (pPayload->variable[i + 2] * 1000000); /* sec to usec */
		NdisCopyMemory(&pPayload->variable[i + 2], &tsf_timer, 8);
		i += 2 + 10; /* Skip elemnt id, length, BSS Termination TSF and Duration*/
	}

	if (pPayload->req_mode & BTM_URL_OFFSET) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"Session Information URL (optional) Included\n");
		url_len = pPayload->variable[i];
		if (url_len > URL_LEN) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"url_len is invalid\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		}
		i += 1 + url_len; /* Skip URL length and url */
		if (i >= Data) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"url_len exceed Frame Len\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		} else if (pPayload->variable[i] != IE_RRM_NEIGHBOR_REP
			&& pPayload->variable[i] != IE_VENDOR_SPECIFIC) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"Frame format error\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		}
	}

	if (pPayload->req_mode & BTM_CANDIDATE_OFFSET) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"Preferred Candidate List Included\n");
		pref_mode = TRUE;
	}

	/* Copy BTM Request Payload except for Optional IE*/
	NdisMoveMemory(&pOutBuffer[FrameLen], pPayload, WNM_BTM_REQUEST_MADATORY_LEN);
	FrameLen += WNM_BTM_REQUEST_MADATORY_LEN;

	if (i > 0) {
		/* Copy BSS Termination Duaration IE & URL IE */
		NdisMoveMemory(&pOutBuffer[FrameLen], &pPayload->variable[0], i);
		FrameLen += i;
	}

	/* Check Per-Sta Profile and store non-setup link addr */
	mlo_neighbor_per_sta_profie_set_entry(pAd, pPayload->variable, i, wdev, neighbor_table);

	/* Parse Neighbor Report IE */
	while (pPayload->variable[i] == IE_RRM_NEIGHBOR_REP) {
		struct NEIGHBOR_REPORT_INFO *pNei_payload = NULL;
		BOOLEAN pref_ie = FALSE;
		UINT j = i;
		UINT nei_len_idx = 0;
		UCHAR Nei_len = 0;
		UCHAR Nei_payload_len = 0;
		UINT non_setup_link_flag = 0;
		UINT l = 0;

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"BSS Transition Candidate List Entries (optional) is included\n");
		Nei_len = pPayload->variable[i + 1];
		i += 2; /* Skip elment id and length */
		if ((sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i + Nei_len)
			> Data) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"Frame format error\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		}

		pNei_payload = (struct NEIGHBOR_REPORT_INFO *)(&pPayload->variable[i]);
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"Neighbor Report Entriy Bssid="MACSTR", Channel=%d\n",
			MAC2STR(pNei_payload->bssid), pNei_payload->channel);

		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		bss_index = BssTableSearch(ScanTab, pNei_payload->bssid, pNei_payload->channel);

		if (bss_index == BSS_NOT_FOUND || bss_index >= MAX_LEN_OF_BSS_TABLE) {
			bss_index = mlo_neighbor_per_sta_profie_search(pAd, neighbor_table,
				pNei_payload->bssid, wdev);
			if (bss_index == BSS_NOT_FOUND)
				non_setup_link_flag = TRUE;
		}

		if (!non_setup_link_flag) {
			if (bss_index == BSS_NOT_FOUND) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_WARN,
						"bss="MACSTR", not found\n",
						MAC2STR(pNei_payload->bssid));

				i += Nei_len;
				continue;
			}
			if (bss_index >= MAX_LEN_OF_BSS_TABLE) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_WARN,
						"bss index invalid\n");

				i += Nei_len;
				continue;
			}

			pBssEntry = &ScanTab->BssEntry[bss_index];
			if (pNei_payload->op_class == 0)
				pNei_payload->op_class = pBssEntry->RegulatoryClass;

			if (pBssEntry->Channel > 14) {
				if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) {
#ifdef DOT11_VHT_AC
					if (HAS_VHT_CAPS_EXIST(pBssEntry->ie_exists))
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				} else
					pBssEntry->CondensedPhyType = 4; /*OFDM case*/
			} else {
				if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) /*HT case*/
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /*ERP case*/
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)/*OFDM case*/
					pBssEntry->CondensedPhyType = 4;
				/* no CCK's definition in spec. */
			}
			if (pNei_payload->phy_type == 0)
				pNei_payload->phy_type = pBssEntry->CondensedPhyType;
		}

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
			"Neighbor Report Entriy->Bssid="MACSTR
			", BssidInfo=%u, Op_Class=%d, Channel=%d, Phy_Tpye=%d\n",
			MAC2STR(pNei_payload->bssid), pNei_payload->bssid_info,
			pNei_payload->op_class, pNei_payload->channel, pNei_payload->phy_type);

		i += sizeof(struct NEIGHBOR_REPORT_INFO);
		Nei_payload_len += sizeof(struct NEIGHBOR_REPORT_INFO);

		/* Copy Neighbor IE Payload, except for Optioanl Sub Element */
		NdisMoveMemory(&pOutBuffer[FrameLen], &pPayload->variable[j], 2 + sizeof(struct NEIGHBOR_REPORT_INFO));
		nei_len_idx = FrameLen + 1;
		FrameLen += 2 + sizeof(struct NEIGHBOR_REPORT_INFO);

		/* Parse Optional Sub Element BSS Candidate Preference */
		if (pPayload->variable[i] == IE_TSF_INFORMATION) {
			if (pPayload->variable[i + 1] != 4) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
					"tsf information len is invalid\n");
				if (pOutBuffer != NULL)
					MlmeFreeMemory((PVOID) pOutBuffer);
				return NDIS_STATUS_FAILURE;
			}
			i += 6;
			Nei_payload_len += 6;
			FrameLen += 6;
		}

		if (pPayload->variable[i] == IE_CONDENSED_COUNTRY_STRING) {
			if (pPayload->variable[i + 1] != 2) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
					"condensed country string len is invalid\n");
				if (pOutBuffer != NULL)
					MlmeFreeMemory((PVOID) pOutBuffer);
				return NDIS_STATUS_FAILURE;
			}
			i += 4;
			Nei_payload_len += 4;
			FrameLen += 4;
		}

		if (pPayload->variable[i] == IE_BSS_CANDIDATE_PREFERENCE) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"SubIE BSS Transition Candidate Preference is found\n");
			if (pPayload->variable[i + 1] != 1) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
					"candidate preference len is invalid\n");
				if (pOutBuffer != NULL)
					MlmeFreeMemory((PVOID) pOutBuffer);
				return NDIS_STATUS_FAILURE;
			}
			pref_ie = TRUE;

			/* Copy Optioanl Sub Element Candidate Preference */
			NdisMoveMemory(&pOutBuffer[FrameLen], &pPayload->variable[i], 3);
			i += 3;
			Nei_payload_len += 3;
			FrameLen += 3;
		}

		if (pPayload->variable[i] == BASIC_MULTI_LINK_ID) {
			non_setup_link_flag = TRUE;
			l =  pPayload->variable[i + 1] + 2;
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
					"SubIE Basic Multi Link ID\n");
			if (pPayload->variable[i + 1] < 9) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
						"Multi Link ID len is invalid - %s\n", __func__);
				if (pOutBuffer != NULL)
					MlmeFreeMemory((PVOID) pOutBuffer);
				return NDIS_STATUS_FAILURE;
			}
			/* Copy Optioanl Sub Element BASIC_MULTI_LINK_ID */
			NdisMoveMemory(&pOutBuffer[FrameLen], &pPayload->variable[i], l);
			i += l;
			Nei_payload_len += l;
			FrameLen += l;
		}

		if (Nei_payload_len < Nei_len) {
			i += Nei_len - Nei_payload_len;
			FrameLen += Nei_len - Nei_payload_len;
		}

		/* Insert Optional Sub Element IE, Basic Multi Link */
		if (!non_setup_link_flag && pBssEntry->ml_capable && pref_ie) {
			UCHAR ml_ie_len = pBssEntry->basic_multi_link_ie[1];

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"Neighbor is a MLD, need to add Basic Multi Linke IE\n");
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"Neighbor is a MLD, need to add Per STA Profile IE\n");

			/* By pass TBRE#4 Cert, set Basic Multi Link ID to 200 */
			pBssEntry->basic_multi_link_ie[0] = BASIC_MULTI_LINK_ID;

			/* Set to 0 of all subfileds of the Presence Bitmap filed
			*  except Link ID Info Present
			*/
			pBssEntry->basic_multi_link_ie[3] &= 0x1f;
			pBssEntry->basic_multi_link_ie[4] = 0;

			hex_dump("Neighbor's ML IE", pBssEntry->basic_multi_link_ie, ml_ie_len + 2);

			/* Remove Ext ID, so length will decrease by one */
			pBssEntry->basic_multi_link_ie[1] -= 1;
			ml_ie_len--;

			/* Skip Ext ID */
			NdisMoveMemory(&pOutBuffer[FrameLen], pBssEntry->basic_multi_link_ie, 2);
			NdisMoveMemory(&pOutBuffer[FrameLen + 2], &pBssEntry->basic_multi_link_ie[3], ml_ie_len);
			/* Update Neighbor Report Length */
			pOutBuffer[nei_len_idx] += ml_ie_len + 2;
			FrameLen += ml_ie_len + 2;
		} else if (!non_setup_link_flag && pBssEntry->ml_capable) {
			UCHAR ml_ie_len = pBssEntry->basic_multi_link_ie[1];
			UCHAR common_info_len = pBssEntry->basic_multi_link_ie[5];

			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"Neighbor is a MLD, need to add Basic Multi Linke IE\n");

			hex_dump("Neighbor's ML IE", pBssEntry->basic_multi_link_ie, ml_ie_len + 2);

			/* By pass TBRE#4 Cert, set Basic Multi Link ID to 200 */
			pBssEntry->basic_multi_link_ie[0] = BASIC_MULTI_LINK_ID;

			/* Clear Presence Bitmap */
			pBssEntry->basic_multi_link_ie[3] &= 0x0f;
			pBssEntry->basic_multi_link_ie[4] = 0;

			/* Remove Ext ID, so length will decrease by one */
			pBssEntry->basic_multi_link_ie[1] -= 1;

			/* Skip Ext ID */
			NdisMoveMemory(&pOutBuffer[FrameLen], pBssEntry->basic_multi_link_ie, 2);
			NdisMoveMemory(&pOutBuffer[FrameLen + 2], &pBssEntry->basic_multi_link_ie[3], 2 + common_info_len);
			/* Update Neighbor Report Length */
			pOutBuffer[nei_len_idx] += 4 + common_info_len;
			FrameLen += 4 + common_info_len;
		}
	}

#ifdef MBO_SUPPORT
	/* Insert MBO Sub Element */
	MboIndex = sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i;
	if (MboIndex < Data) {
		PUCHAR pBuf = (PUCHAR)pData;
		ULONG MboLen = 0;

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_NOTICE,
				"Insert MBO Sub Element\n");

		MboLen = Data - MboIndex;

		if (pBuf[MboIndex + 1] != MboLen - 2 || pBuf[MboIndex] != IE_VENDOR_SPECIFIC
			|| pBuf[MboIndex + 5] != MBO_OCE_OUI_TYPE) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_WARN,
				"MBO IE is invalid\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		}

		if ((sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i + MboLen) != Data) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
				"The BTM Request in invalid\n");
			if (pOutBuffer != NULL)
				MlmeFreeMemory((PVOID) pOutBuffer);
			return NDIS_STATUS_FAILURE;
		}

		hex_dump("MBO IE", &pBuf[MboIndex], MboLen);
		NdisMoveMemory(&pOutBuffer[FrameLen], &pBuf[MboIndex], MboLen);
		FrameLen += MboLen;
	} else if ((sizeof(HEADER_802_11) + WNM_BTM_REQUEST_MADATORY_LEN + i) != Data) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
			"The BTM Request in invalid\n");
		if (pOutBuffer != NULL)
			MlmeFreeMemory((PVOID) pOutBuffer);
		return NDIS_STATUS_FAILURE;
	}
#endif /* MBO_SUPPORT */

	hex_dump("pData", (CHAR *)pData, Data);
	hex_dump("pOutBuffer", pOutBuffer, FrameLen);

	if (FrameLen > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BTM, DBG_LVL_ERROR,
			"The BTM Request is too large\n");
		if (pOutBuffer != NULL)
			MlmeFreeMemory((PVOID) pOutBuffer);
		return NDIS_STATUS_FAILURE;
	}

	/* Transmit the BTM request frame */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);

	if (pOutBuffer != NULL)
		MlmeFreeMemory((PVOID) pOutBuffer);

	mlo_neighbor_per_sta_profie_clear(neighbor_table);

	return NDIS_STATUS_SUCCESS;
}
#endif


INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data_len)
{
	UCHAR	*pBuf = NULL;
	ULONG Data = Data_len;
	struct BSS_TM_REQ *pPayload = NULL;

	if (Data > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"frame size is illegal,len:%ld\n", Data);
	}

	if (pData != NULL) {
#ifdef CONFIG_AP_SUPPORT
		struct ieee80211_mgmt *mgmt;
#endif /* CONFIG_AP_SUPPORT */
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
		UINT8 apidx;
#endif

#ifdef RT_CFG80211_SUPPORT
		os_alloc_mem(NULL, (UCHAR **)&pBuf, Data);
		if (pBuf != NULL)
			NdisCopyMemory(pBuf, pData, Data);
		else {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"CFG_TX_STATUS: MEM ALLOC ERROR\n");
			return NDIS_STATUS_FAILURE;
		}
#endif
		CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);
#ifdef CONFIG_AP_SUPPORT
		mgmt = (struct ieee80211_mgmt *)pData;
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
		apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			INT offset = sizeof(HEADER_802_11) + 12;
#ifdef DISABLE_HOSTAPD_PROBE_RESP
#ifndef HOSTAPD_11R_SUPPORT
		UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
		CFG80211_SyncPacketWpsIe(pAd, pData + offset, Data - offset, apidx, mgmt->da);
		goto LabelOK;
#else

		CFG80211_SyncPacketWmmIe(pAd, pData + offset, Data - offset);
#endif
		}

		if ((ieee80211_is_auth(mgmt->frame_control)) && (mgmt->u.auth.auth_alg != AUTH_MODE_FT) &&
			(mgmt->u.auth.auth_alg != AUTH_MODE_SAE)
		) {
#ifdef RADIUS_MAC_AUTH_SUPPORT
			MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, mgmt->da);

			if (pEntry != NULL && pEntry->wdev->radius_mac_auth_enable) {
				if (mgmt->u.auth.status_code == MLME_SUCCESS) {
					pEntry->bAllowTraffic = TRUE;
				} else {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
			}
#endif
			goto LabelOK;
		}
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT) || defined(CCN67_SPLIT_MAC_SUPPORT)
		if (ieee80211_is_auth(mgmt->frame_control) &&
			((mgmt->u.auth.auth_alg == AUTH_MODE_FT) || (mgmt->u.auth.auth_alg == AUTH_MODE_SAE)
		)) {
			CFG80211_AuthRespHandler(pAd, pData, Data);
			MiniportMMRequest(pAd, 0, pData, Data, NULL);
			if (pBuf) {
				CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, COOkIE_VALUE,
							pBuf, Data, 1);
			}
			goto LabelOK;
		}
		if (ieee80211_is_reassoc_resp(mgmt->frame_control)
			|| ieee80211_is_assoc_resp(mgmt->frame_control)) {
			CFG80211_AssocRespHandler(pAd, pData, Data);
			if (pBuf) {
				CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, COOkIE_VALUE,
					pBuf, Data, 1);
			}
			goto LabelOK;
		}
#endif

		if (ieee80211_is_action(mgmt->frame_control)) {

			pPayload = (struct BSS_TM_REQ *)(&mgmt->u);

			if ((mgmt->u.action.category == CATEGORY_WNM)
					&& (pPayload->action == BSS_TRANSITION_REQ)) {
				NDIS_STATUS NStatus;
#if defined(DOT11_EHT_BE)
				MAC_TABLE_ENTRY * pEntry = NULL;
				UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif

				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_WMM, DBG_LVL_NOTICE,
					"Send WNM BTM Req Frame\n");

#if defined(DOT11_EHT_BE)
				pEntry = MacTableLookup(pAd, mgmt->da);

				if (pEntry && pEntry->mlo.mlo_en) {
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
						"for MLO Link\n");
					NStatus = CFG80211_MLO_BtmReqHandler(pAd, pData, Data);

					if (NStatus != NDIS_STATUS_SUCCESS)
						MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
							"Send MLO BTM request fail!!\n");

					if (pBuf)
						CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, COOkIE_VALUE,
								pBuf, Data, 1);

					goto LabelOK;
				} else
#endif
				{
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
						"for Non MLO Link\n");
					NStatus = CFG80211_LEGACY_BtmReqHandler(pAd, pData, Data);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
							"Send Legacy BTM request fail!!\n");

						CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, COOkIE_VALUE,
								pBuf, Data, 1);

						goto LabelOK;
					}
				}
			}
		}

#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
		if (ieee80211_is_deauth(mgmt->frame_control)) {
			PUCHAR pMac = (PUCHAR)mgmt->da;
			MAC_TABLE_ENTRY *pEntry;
			UCHAR broadMac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			USHORT reason_code = mgmt->u.deauth.reason_code;

			if (pMac == NULL)
				goto LabelOK;
			pEntry = MacTableLookup(pAd, pMac);
			if (pEntry) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO,
					"Deauth:"MACSTR", reason code %d\n", MAC2STR(pMac), reason_code);
				MlmeDeAuthAction(pAd, pEntry, reason_code, FALSE);
			} else if (MAC_ADDR_EQUAL(broadMac, pMac)) {
				ap_send_broadcast_deauth(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"Can't find pEntry\n");
			}
			goto LabelOK;
		}

		if (ieee80211_is_disassoc(mgmt->frame_control)) {
			PUCHAR pMac = (PUCHAR)mgmt->da;
			MAC_TABLE_ENTRY *pEntry;
			USHORT reason_code = mgmt->u.disassoc.reason_code;

			pEntry = MacTableLookup(pAd, pMac);
			if (pEntry) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO,
					"Disassoc:"MACSTR", reason code %d\n", MAC2STR(pMac), reason_code);
				APMlmeKickOutSta(pAd, pMac, pEntry->wcid, reason_code);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"Can't find pEntry\n");
			}
			goto LabelOK;
		}
#endif
#endif /* CONFIG_AP_SUPPORT */
		MiniportMMRequest(pAd, 0, pData, Data, NULL);
		/*dpp action frame need driver indicate TX status event*/
		if (ieee80211_is_action(mgmt->frame_control)) {
			UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);

			CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, COOkIE_VALUE,
					pBuf, Data, 1);
		}
	}
LabelOK:
	if (pBuf != NULL)
		os_free_mem(pBuf);

	return 0;
}

#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(
	RTMP_ADAPTER *pAd, struct _BSS_STRUCT *pMbss, struct wifi_dev *wdev,
	VOID *cfg, UCHAR *wpa_ie, UCHAR *rsn_ie)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;
	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;
	USHORT							Count;
	BOOLEAN bWPA = FALSE;
	BOOLEAN bWPA2 = FALSE;
	BOOLEAN bMix = FALSE;
	struct CMD_RTPRIV_IOCTL_80211_BEACON_CFG *pCfg = cfg;

	/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;
#ifdef DOT11W_PMF_SUPPORT
	wdev->SecConfig.PmfCfg.MFPC = 0;
	wdev->SecConfig.PmfCfg.MFPR = 0;
	wdev->SecConfig.PmfCfg.igtk_cipher = 0;
	wdev->SecConfig.PmfCfg.Desired_MFPC = 0;
	wdev->SecConfig.PmfCfg.Desired_MFPR = 0;
	wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 0;
#endif

#ifdef DOT1X_SUPPORT
	wdev->SecConfig.IEEE8021X = FALSE;
#endif /* DOT1X_SUPPORT */

	/* clear akm and cipher only if cfg provide from hostapd */
	if (pCfg) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"clear AuthMode and Cipher\n");
		CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
		CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
		CLEAR_GROUP_CIPHER(&wdev->SecConfig);
		if ((wpa_ie == NULL) && (rsn_ie == NULL)) { /* open case */
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* wdev->WepStatus = Ndis802_11WEPDisabled; */
			/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
			if (pCfg->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
				SET_AKM_OPEN(wdev->SecConfig.AKMMap);
			else if (pCfg->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
				SET_AKM_SHARED(wdev->SecConfig.AKMMap);

			if (pCfg->privacy) {
				SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
				SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
			} else {
				SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
				SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
			}
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"set AuthMode=%s,Cipher(%s %s)\n",
				GetAuthModeStr(wdev->SecConfig.AKMMap),
			    GetEncryModeStr(wdev->SecConfig.PairwiseCipher),
			    GetEncryModeStr(wdev->SecConfig.GroupCipher));
		}
	}

	if (wpa_ie != NULL) { /* wpapsk/tkipaes case */
		pEid = (PEID_STRUCT)wpa_ie;
		pTmp = (PUCHAR)pEid;

		if (os_equal_mem(pEid->Octet, WPA_OUI, 4)) {
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* SET_AKM_OPEN(wdev->SecConfig.AKMMap); */
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE, "WPA case\n");
			bWPA = TRUE;
			pTmp   += 11;

			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
			CLEAR_GROUP_CIPHER(&wdev->SecConfig);

			switch (*pTmp) {
			case 1:
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
						"Group Ndis802_11GroupWEP40Enabled\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled; */
				SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
				break;

			case 5:
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
						"Group Ndis802_11GroupWEP104Enabled\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled; */
				SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
				break;

			case 2:
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
						"Group Ndis802_11TKIPEnable\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable; */
				SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
				break;

			case 4:
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
						"Group Ndis802_11AESEnable\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11AESEnable; */
				SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
				break;

			default:
				break;
			}

			/* number of unicast suite*/
			pTmp   += 1;
			/* skip all unicast cipher suites*/
			/*Count = *(PUSHORT) pTmp;				*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pTmp += 3;
				TmpCipher = Ndis802_11WEPDisabled;

				switch (*pTmp) {
				case 1:
				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					TmpCipher = Ndis802_11WEPEnabled;
					break;

				case 2:
					TmpCipher = Ndis802_11TKIPEnable;
					break;

				case 4:
					TmpCipher = Ndis802_11AESEnable;
					break;

				default:
					break;
				}

				if (TmpCipher > PairCipher) {
					/* Move the lower cipher suite to PairCipherAux*/
					PairCipherAux = PairCipher;
					PairCipher	= TmpCipher;
				} else
					PairCipherAux = TmpCipher;

				pTmp++;
				Count--;
			}

			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"Auth Count in WPA = %d, we only parse the first for AKM\n", Count);
			pTmp   += 3; /* parse first AuthOUI for AKM */

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPA; */
				SET_AKM_WPA1(wdev->SecConfig.AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPAPSK; */
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"UNKNOWN AKM 0x%x IN WPA,please check!\n", *pTmp);
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"AuthMode = 0x%x\n", wdev->SecConfig.AKMMap);

			/* if (wdev->GroupKeyWepStatus == PairCipher) */
			if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
			   ) {
				/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
				/* pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus; */
				wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
					"WPA Mix TKIPAES\n");
				bMix = TRUE;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"wpa open/none case\n");
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* wait until wpa/wpa2 all not exist , then set open/none */
		}
	}

	bWPA2 = cfg80211_ap_parse_rsn_ie(pAd, &wdev->SecConfig, rsn_ie, TRUE, FALSE);

	if (bWPA2 && bWPA) {
		/* wdev->AuthMode = Ndis802_11AuthModeWPA1PSKWPA2PSK; */
		if (IS_AKM_WPA1(wdev->SecConfig.AKMMap) ||
			IS_AKM_WPA2(wdev->SecConfig.AKMMap)) {
			SET_AKM_WPA1(wdev->SecConfig.AKMMap);
			SET_AKM_WPA2(wdev->SecConfig.AKMMap);
			if (IS_AKM_WPA3(wdev->SecConfig.AKMMap))
				SET_AKM_WPA3(wdev->SecConfig.AKMMap);
		} else {
			SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
			SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
		}

		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else if (bWPA2) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"WPA2 case\n");
	} else if (bWPA) {
		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			if (IS_AKM_WPA1(wdev->SecConfig.AKMMap))
				SET_AKM_WPA1(wdev->SecConfig.AKMMap);
			else
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else {
		if (pCfg) {
			if (pCfg->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
				SET_AKM_OPEN(wdev->SecConfig.AKMMap);
			else if (pCfg->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
				SET_AKM_SHARED(wdev->SecConfig.AKMMap);
			SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			    "update AuthMode=%s, Cipher(%s %s)\n",
			    GetAuthModeStr(wdev->SecConfig.AKMMap),
			    GetEncryModeStr(wdev->SecConfig.PairwiseCipher),
			    GetEncryModeStr(wdev->SecConfig.GroupCipher));
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			    "skip updating cipher as no cfg from hostapd\n");
	}

#ifdef DOT1X_SUPPORT
	if (IS_AKM_WPA1(wdev->SecConfig.AKMMap) || IS_AKM_WPA2(wdev->SecConfig.AKMMap))
		wdev->SecConfig.IEEE8021X = TRUE;
#endif /* DOT1X_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"=>wpa(%d %d %d), AuthMode=%s, Cipher(%s %s), MFPC=%d, MFPR=%d, igtk_cipher=0x%x\n"
		, bWPA2, bWPA, bMix, GetAuthModeStr(wdev->SecConfig.AKMMap),
		GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher),
		wdev->SecConfig.PmfCfg.MFPC, wdev->SecConfig.PmfCfg.MFPR, wdev->SecConfig.PmfCfg.igtk_cipher);
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

