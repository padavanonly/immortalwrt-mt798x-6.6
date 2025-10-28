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
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

#ifndef APMT2_PEER_PROBE_REQ
#define APMT2_PEER_PROBE_REQ		0
#endif


/*******************************************************************************
 *                           P R I V A T E   F U N C T I O N S
 *******************************************************************************
 */

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT) || defined(CCN67_SPLIT_MAC_SUPPORT)
BOOLEAN CFG80211_AuthReqHandler(
	struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, void *pData, AUTH_FRAME_INFO *auth_info)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pData;
	ULONG MsgLen = pRxBlk->DataSize;
#ifdef BAND_STEERING
	HEADER_802_11 AuthReqHdr;
#endif /* BAND_STEERING */
	struct wifi_dev *wdev = NULL;
	UINT32 apidx;
	BSS_STRUCT *pMbss;

	apidx = get_apidx_by_addr(pAd, auth_info->addr1);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR, "%d, wdev is null\n", __LINE__);
		return FALSE;
	}
#ifdef DOT11R_FT_SUPPORT
	if (auth_info->auth_alg == AUTH_MODE_FT) {
		PEID_STRUCT eid_ptr;
		UCHAR *Ptr;
		UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
		PFT_INFO pFtInfo = &auth_info->FtInfo;

		NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

		Ptr = &Fr->Octet[6];
		eid_ptr = (PEID_STRUCT) Ptr;

	    /* get variable fields from payload and advance the pointer */
		while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
			switch (eid_ptr->Eid) {
			case IE_FT_MDIE:
				if (FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_NOTICE,
							"wrong IE_FT_MDIE\n");
					return FALSE;
				}
				break;

			case IE_FT_FTIE:
				if (FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_NOTICE,
							"wrong IE_FT_FTIE\n");
					return FALSE;
				}
				break;

			case IE_FT_RIC_DATA:
				/* record the pointer of first RDIE. */
				if (pFtInfo->RicInfo.pRicInfo == NULL) {
					pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
					pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen)
								- (UCHAR *)eid_ptr + 1;
				}

				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2)
					< MAX_RICIES_LEN) {
					NdisMoveMemory(
					&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
					&eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				} else {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_NOTICE,
						"wrong IE_FT_RIC_DATA\n");
					return FALSE;
				}
				break;

			case IE_FT_RIC_DESCRIPTOR:
				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2)
					< MAX_RICIES_LEN) {
					NdisMoveMemory(
					&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
					&eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				} else {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_NOTICE,
						"wrong IE_FT_RIC_DESCRIPTOR\n");
					return FALSE;
				}
				break;

			case IE_RSN:
				if (parse_rsn_ie(eid_ptr)) {
					if (NdisEqualMemory(
						&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
						NdisMoveMemory(
							pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
					}
				} else {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_NOTICE,
						"wrong IE_RSN\n");
					return FALSE;
				}
				break;

			default:
				break;
			}
			eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
			if (((UCHAR *)eid_ptr >= ((UCHAR *)Fr + MsgLen))
				|| ((UCHAR *)(eid_ptr+2) > ((UCHAR *)Fr + MsgLen)))
				break;
		}
		if (MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_HOSTAPD_AUTH_REQ,
								pRxBlk->DataSize, pData, 0, wdev, FALSE, NULL))
			RTMP_MLME_HANDLER(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
	NdisZeroMemory(&AuthReqHdr, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *)&(AuthReqHdr.FC), pRxBlk->FC, sizeof(FRAME_CONTROL));
	wdev = wdev_search_by_address(pAd, AuthReqHdr.Addr1);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG, "wdev is null\n");
		return FALSE;
	}
#ifdef CCN67_BS_SUPPORT
	if (PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
	if (pAd->ApCfg.BandSteering)
#endif
	{
		BOOLEAN bBndStrgCheck;
		ULONG MsgType = APMT2_PEER_AUTH_REQ;
		MLME_QUEUE_ELEM *elem = NULL;

		os_alloc_mem(NULL, (UCHAR **)&elem, sizeof(MLME_QUEUE_ELEM));

		if (!elem) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"allocate elem memory fail\n");
			return FALSE;
		}
		os_zero_mem(elem, sizeof(MLME_QUEUE_ELEM));
		elem->MsgType = MsgType;
		NdisMoveMemory(elem->rssi_info.raw_rssi, pRxBlk->rx_signal.raw_rssi, 4);
#ifdef CCN67_BS_SUPPORT
		bBndStrgCheck = BS_CheckConnectionReq(pAd, wdev, auth_info->addr2, elem, NULL);
#else
		bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, auth_info->addr2, elem, NULL);
#endif

		if (elem)
			os_free_mem(elem);
		if (bBndStrgCheck == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO,
				"AUTH - check failed.\n");
			return FALSE;
		}
	}
#endif /* (CCN67_BS_SUPPORT) || defined(BAND_STEERING) */
	return TRUE;
}
#endif /* HOSTAPD_11R_SUPPORT */


/*
 * Return:
 *	TRUE - forward frames to upper layer or drop
 *	FALSE - forward frames to driver state machine
 */
static BOOLEAN CFG80211_ProbeReqHandler(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _RX_BLK *pRxBlk,
	PNET_DEV pNetDev,
	UINT32 freq)
{
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
	PEER_PROBE_REQ_PARAM * ProbeReqParam = NULL;

	os_alloc_mem(NULL, (UCHAR **)&ProbeReqParam, sizeof(PEER_PROBE_REQ_PARAM));
	if (!ProbeReqParam) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
			"allocate ProbeReqParam memory fail\n");
		return FALSE;
	}
	os_zero_mem((PVOID) ProbeReqParam, sizeof(PEER_PROBE_REQ_PARAM));
#ifdef CCN67_BS_SUPPORT
	if (PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
	if (pAd->ApCfg.BandSteering)
#endif
	{
		if (PeerProbeReqSanity(pAd, (VOID *)pRxBlk->pData,
			pRxBlk->DataSize, ProbeReqParam) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"PeerProbeReqSanity fail!\n");
			os_free_mem(ProbeReqParam);
			return TRUE; /*drop when SSID sanity check fails in ProbeReq */
		}
	}
#endif
	if (IS_BROADCAST_MAC_ADDR(pRxBlk->Addr1)) {
		int apidx;
		/*deliver broadcast frame to all virtual interface */
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[apidx];
			struct wifi_dev *wdev = &mbss->wdev;

			if (wdev->if_dev == NULL)
				continue;

			/* other virtual bss may be not ready for processing this frame*/
			if (!netif_running(wdev->if_dev))
				continue;
#ifdef EM_PLUS_SUPPORT
			/* fail in ACL checking =>  drop probe req. */
			if (!ApCheckAccessControlList(pAd, pRxBlk->Addr2, apidx)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
						"Device "MACSTR" ACL for wdev %s, block probe!!\n",
						MAC2STR(pRxBlk->Addr2),
						RtmpOsGetNetDevName(wdev->if_dev));
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
				os_free_mem(ProbeReqParam);
#endif /* BAND_STEERING */
				return TRUE;
			}
#endif /* EM_PLUS_SUPPORT */

#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)

#ifdef CCN67_BS_SUPPORT
		if (PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
		if (pAd->ApCfg.BandSteering)
#endif
		{
			BOOLEAN bBndStrgCheck;
			ULONG MsgType = APMT2_PEER_PROBE_REQ;
			MLME_QUEUE_ELEM *elem = NULL;

			os_alloc_mem(NULL, (UCHAR **)&elem,
				sizeof(MLME_QUEUE_ELEM));
			if (!elem) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT,
					DBG_LVL_ERROR,
					"allocate elem memory fail\n");
				os_free_mem(ProbeReqParam);
				return FALSE;
			}
			os_zero_mem((PVOID) elem, sizeof(MLME_QUEUE_ELEM));
			elem->MsgType = MsgType;
			NdisMoveMemory(elem->rssi_info.raw_rssi,
				pRxBlk->rx_signal.raw_rssi,
				4);

#ifdef CCN67_BS_SUPPORT
			bBndStrgCheck = BS_CheckConnectionReq(pAd, wdev,
					ProbeReqParam->Addr2, elem, ProbeReqParam);
#else /*BAND_STEERING*/
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev,
								ProbeReqParam->Addr2, elem, ProbeReqParam);
#endif
			if (elem)
				os_free_mem(elem);
			if (bBndStrgCheck == FALSE) {
				os_free_mem(ProbeReqParam);
				return TRUE;
			}
		}

#endif /* (CCN67_BS_SUPPORT) || defined(BAND_STEERING) */

			CFG80211OS_RxMgmt(wdev->if_dev, freq,
				pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
		}
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
		os_free_mem(ProbeReqParam);
#endif /* BAND_STEERING */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		/*only send this probe req to hostapd update infos,
		peer the probe req and send probe rsp in the driver*/
		return FALSE;
#endif
	} else {
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
#ifdef CCN67_BS_SUPPORT
		if (wdev && PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
		if (wdev && pAd->ApCfg.BandSteering)
#endif
		{
			BOOLEAN bBndStrgCheck;
			ULONG MsgType = APMT2_PEER_PROBE_REQ;
			MLME_QUEUE_ELEM *elem = NULL;

			os_alloc_mem(NULL, (UCHAR **)&elem, sizeof(MLME_QUEUE_ELEM));

			if (!elem) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
					"allocate elem memory fail\n");
				os_free_mem(ProbeReqParam);
				return FALSE;
			}
			os_zero_mem((PVOID) elem, sizeof(MLME_QUEUE_ELEM));
			elem->MsgType = MsgType;
			NdisMoveMemory(elem->rssi_info.raw_rssi, pRxBlk->rx_signal.raw_rssi, 4);
#ifdef CCN67_BS_SUPPORT
			bBndStrgCheck = BS_CheckConnectionReq(pAd, wdev,
					ProbeReqParam->Addr2, elem, ProbeReqParam);
#else
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev,
					ProbeReqParam->Addr2, elem, ProbeReqParam);
#endif
			if (elem)
				os_free_mem(elem);
			if (bBndStrgCheck == FALSE) {
				os_free_mem(ProbeReqParam);
				return TRUE;
			}
		}
#endif /* CCN67_BS_SUPPORT */

		if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr1)) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO,
				"Invalid probe request frame!\n");
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
			os_free_mem(ProbeReqParam);
#endif /* BAND_STEERING */
			return TRUE; /*drop when received ProbeReq with A1=MULTICAST */
		}

#ifdef EM_PLUS_SUPPORT
		if (wdev) {
			struct _BSS_STRUCT *mbss = (struct _BSS_STRUCT *)wdev->func_dev;
			UCHAR mbss_idx = mbss->mbss_idx;

			/* fail in ACL checking =>  drop probe req. */
			if (!ApCheckAccessControlList(pAd, pRxBlk->Addr2, mbss_idx)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
						"Device "MACSTR" ACL for wdev %s, block probe!!\n",
						MAC2STR(pRxBlk->Addr2),
						RtmpOsGetNetDevName(wdev->if_dev));
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
				os_free_mem(ProbeReqParam);
#endif /* BAND_STEERING */
				return TRUE;
			}
		}
#endif /* EM_PLUS_SUPPORT */

		CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);

#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
		os_free_mem(ProbeReqParam);
#endif /* BAND_STEERING */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		/*only send this probe req to hostapd update infos,
		peer the probe req and send probe rsp in the driver*/
		return FALSE;
#endif
	}
}

/*
	Return:
		TRUE - forward frames to upper layer
		FALSE - forward frames to driver state machine
*/
static BOOLEAN CFG80211_AuthHandle(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _RX_BLK *pRxBlk,
	PNET_DEV pNetDev,
	struct _CFG80211_CONTROL *pCfg80211_ctrl,
	UINT32 freq)
{
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)  || defined(HOSTAPD_OWE_SUPPORT) || defined(CCN67_SPLIT_MAC_SUPPORT)
	AUTH_FRAME_INFO *auth_info = &(pCfg80211_ctrl->auth_info);
	struct ieee80211_mgmt *mgmt;

	mgmt = (struct ieee80211_mgmt *)pRxBlk->pData;
	COPY_MAC_ADDR(auth_info->addr1,  mgmt->da);
	COPY_MAC_ADDR(auth_info->addr2,  mgmt->sa);
	auth_info->auth_alg = le2cpu16(mgmt->u.auth.auth_alg);
	auth_info->auth_seq = le2cpu16(mgmt->u.auth.auth_transaction);
	auth_info->auth_status = le2cpu16(mgmt->u.auth.status_code);

#ifdef APCLI_CFG80211_SUPPORT
	if (wdev && (wdev->wdev_type == WDEV_TYPE_STA)) {
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if (IS_WPA_SUPPLICANT_UP(pStaCfg) &&
			auth_info->auth_alg == AUTH_MODE_SAE &&
			wdev->cntl_machine.CurrState == CNTL_WAIT_AUTH) {
			BOOLEAN Cancelled;
			PRALINK_TIMER_STRUCT pAuthTimer = &pStaCfg->MlmeAux.AuthTimerExt;

			RTMPCancelTimer(pAuthTimer, &Cancelled);

			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
				"[cfg80211] received auth from "MACSTR
				" (alg=%d, seq=%d, status=%d)...\n",
				MAC2STR(auth_info->addr2), auth_info->auth_alg, auth_info->auth_seq,
				auth_info->auth_status);
			CFG80211OS_RxMgmt(wdev->if_dev, freq,
				pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);

			if (!IS_SAE_COMMIT_STATUS_SUCCESS(auth_info->auth_status))
				cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

			return TRUE;
		}
		return FALSE;
	}
#endif /* APCLI_CFG80211_SUPPORT */
	if ((auth_info->auth_alg == AUTH_MODE_FT)
		|| (auth_info->auth_alg == AUTH_MODE_SAE)
	) {
		/* handle FT Auth or SAE Auth in hostapd */
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			"[cfg80211] received auth from "MACSTR" (alg=%d, seq=%d, status=%d)...\n",
			MAC2STR(auth_info->addr2), auth_info->auth_alg,
			auth_info->auth_seq, auth_info->auth_status);
		CFG80211_AuthReqHandler(pAd, pRxBlk, (void *)pRxBlk->FC, auth_info);
		if (auth_info->auth_alg == AUTH_MODE_SAE
		)
			CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
		return TRUE;
	}
	/* handle normal auth in driver */
	if (wdev &&
		(IS_AKM_OWE(wdev->SecConfig.AKMMap)
			|| IS_AKM_DPP(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
			|| IS_AKM_OWE(wdev->SecConfig_ext.AKMMap)
			|| IS_AKM_DPP(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
			))
		CFG80211OS_RxMgmt(
			wdev->if_dev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
#endif
	return FALSE;
}

/*
	Return:
		TRUE - forward frames to upper layer
		FALSE - forward frames to driver state machine
*/
static BOOLEAN CFG80211_AssocReqHandle(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _RX_BLK *pRxBlk)
{
#ifdef HOSTAPD_OWE_SUPPORT
	BOOLEAN to_hostapd = FALSE;

	if ((wdev && (IS_AKM_DPP(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
		|| IS_AKM_DPP(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
		)
#if defined(MAP_R3) && defined(CONFIG_MAP_SUPPORT) && defined(MTK_HOSTAPD_SUPPORT)
		&& !(IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
#endif /* MAP_R3 && CONFIG_MAP_SUPPORT && MTK_HOSTAPD_SUPPORT  */
	))
		to_hostapd = TRUE;
	if (wdev && (IS_AKM_OWE(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
			|| IS_AKM_OWE(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
			))
		to_hostapd = TRUE;

	if (to_hostapd) {
		if ((pRxBlk->FC == pRxBlk->pData)
			&& (pRxBlk->DataSize == pRxBlk->MPDUtotalByteCnt)) {
			if (MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_HOSTAPD_ASSOC_REQ_HANDLER,
				pRxBlk->DataSize, pRxBlk->FC, pRxBlk->wcid, wdev, FALSE, NULL))
				RTMP_MLME_HANDLER(pAd);
			else
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
					"enq fail\n");
		} else
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"assoc error: DataSize:%d, MPDUtotalByteCnt:%d\n",
				pRxBlk->DataSize, pRxBlk->MPDUtotalByteCnt);
		return TRUE;
	}
#endif /* HOSTAPD_OWE_SUPPORT */
	return FALSE;
}

/*
	Return:
		TRUE - forward frames to upper layer
		FALSE - forward frames to driver state machine
*/
static BOOLEAN CFG80211_ReAssocReqHandle(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _RX_BLK *pRxBlk)
{
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
	if (wdev && (IS_AKM_OWE(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
		|| IS_AKM_OWE(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
		|| IS_AKM_FT_WPA2PSK(wdev->SecConfig.AKMMap)
		|| IS_AKM_FT_WPA2(wdev->SecConfig.AKMMap)
		|| IS_AKM_FT_SAE_SHA256(wdev->SecConfig.AKMMap)
		|| IS_AKM_FT_SAE_EXT(wdev->SecConfig.AKMMap))) {
		if ((pRxBlk->FC == pRxBlk->pData)
			&& (pRxBlk->DataSize == pRxBlk->MPDUtotalByteCnt)) {
			if (MlmeEnqueueWithWdev(pAd,
				ASSOC_FSM, ASSOC_FSM_HOSTAPD_REASSOC_REQ_HANDLER,
				pRxBlk->DataSize, pRxBlk->FC, 0, wdev, FALSE, NULL))
				RTMP_MLME_HANDLER(pAd);
			else
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
					"enq fail\n");
		} else
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"reassoc error: DataSize:%d, MPDUtotalByteCnt:%d\n",
				pRxBlk->DataSize, pRxBlk->MPDUtotalByteCnt);
		return TRUE;
	}
#endif
	return FALSE;
}

/*
	Return:
		TRUE - forward frames to upper layer
		FALSE - forward frames to driver state machine
*/
static BOOLEAN CFG80211_ActionHandle(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _RX_BLK *pRxBlk,
	PNET_DEV pNetDev,
	UINT32 freq)
{
	if (!CFG80211_CheckActionFrameType(pAd, "RX",
		(PUCHAR)pRxBlk->pData, pRxBlk->MPDUtotalByteCnt))
		return FALSE;

	if (wdev) {
		CFG80211OS_RxMgmt(wdev->if_dev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
	} else {
		CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);

#ifdef APCLI_CFG80211_SUPPORT
		/*Add it to support DPP R1 */
		/*APCLI will handle the broadcast action for dpp*/
		if (IS_BROADCAST_MAC_ADDR(pRxBlk->Addr1)) {
			int idx = 0;
			/*deliver broadcast frame to all virtual interface */
			for (idx = 0; idx < MAX_MULTI_STA ; idx++) {
				struct wifi_dev *wdev = &pAd->StaCfg[idx].wdev;

				if (wdev->if_dev != NULL)
					CFG80211OS_RxMgmt(wdev->if_dev, freq,
						pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
			}
		}
#endif /* APCLI_CFG80211_SUPPORT */
	}
	return TRUE;
}

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, UCHAR FromWhichBSSID)
{
#ifdef CONFIG_AP_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_AP);
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_STA);
	}
}

BOOLEAN CFG80211_CheckWNMActionBSSColorReport(
	IN RTMP_ADAPTER * pAd,
	IN PUCHAR preStr,
	IN PUCHAR pData,
	IN UINT32 length)
{
	BOOLEAN bss_color_report_found = FALSE;
	int rest_len = (int)length;
	PUCHAR ptr = pData + (sizeof(HEADER_802_11) + 3);

	rest_len -= (sizeof(HEADER_802_11) + 3); /* Only parse element report */
	while (rest_len > 0) {
		u8 element_len = 0;
		struct WNM_EVT_REPORT_ELEMENT *pRpt = (struct WNM_EVT_REPORT_ELEMENT *)ptr;

		element_len = (u8)pRpt->Length;
		if (pRpt->Element_ID == IE_EVENT_REPORT) {
			if (((pRpt->EvtType == BSS_COLOR_COLLISION) || (pRpt->EvtType == BSS_COLOR_INUSE))
				&& (pRpt->EvtToken == 0) && (pRpt->EvtStatus == STATUS_SUCCESSFUL)) {
				bss_color_report_found = TRUE;
				MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO, "bss color event found: %d", pRpt->EvtType);
				break;
			} else if (pRpt->EvtType > BSS_COLOR_INUSE) {
				MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_WARN, "Invalid Event type %02X", pRpt->EvtType);
			}
		}

		ptr += (element_len + 2); /* +2 for element ID and length */
		rest_len -= (element_len + 2);
	}

	return bss_color_report_found;
}
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
BOOLEAN CFG80211_CheckActionFrameTypeDpp(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR preStr,
	IN PUCHAR pData,
	IN UINT32 length)
{
	struct dpp_gas_frame_list *gas_frame = NULL;
	struct dpp_gas_frame_list *gas_frame_tmp = NULL;
	BOOLEAN isDppFrame = FALSE;
	PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pData;

	if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
			(pFrame->Category == CATEGORY_PUBLIC)) {

		/* For Vendor specific DPP frames like
		 * auth, configuration result, peer dicsovery, etc.
		 * OUIType here corresponds to the subtype of
		 * action frame
		 */
		if ((pFrame->Action == ACTION_WIFI_DIRECT) &&
			(NdisEqualMemory(pFrame->OUI, DPP_OUI, OUI_LEN)) &&
			(pFrame->OUIType == WFA_DPP_SUBTYPE)) {
			isDppFrame = TRUE;

			/* If DPP config result received from mac
			 * on which GAS happened then clear the link list
			 * Token here corresponds to the type of the
			 * DPP action frame
			 */
			if (pFrame->Token == ACTION_DPP_CONF_RESULT) {
				OS_SEM_LOCK(&pAd->gas_frame_list_lock);
				if (!DlListEmpty(&pAd->dpp_gas_event_list) && pAd->is_dpp_gas_list_init == TRUE) {
					DlListForEachSafe(gas_frame, gas_frame_tmp, &pAd->dpp_gas_event_list,
							struct dpp_gas_frame_list, List) {
					if (NdisEqualMemory(gas_frame->sta_mac, pFrame->p80211Header.Addr2, MAC_ADDR_LEN)) {
						MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
							"Free gas list with "MACSTR"\n", MAC2STR(gas_frame->sta_mac));
						DlListDel(&gas_frame->List);
						os_free_mem(gas_frame);
						break;
					}
					}
				}
				OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
			}
		}

		/* For DPP related GAS frames check OUI field to
		 * identify DPP frame, except GAS Comebck request
		 */
		else if ((pFrame->Action == ACTION_GAS_INITIAL_REQ) ||
						(pFrame->Action == ACTION_GAS_INITIAL_RSP) ||
						(pFrame->Action == ACTION_GAS_COMEBACK_REQ) ||
						(pFrame->Action == ACTION_GAS_COMEBACK_RSP)) {
			GAS_FRAME *GASFrame = (GAS_FRAME *)pData;

			switch (pFrame->Action) {
			case ACTION_GAS_INITIAL_REQ:
			{
			if ((GASFrame->u.GAS_INIT_REQ.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
					NdisEqualMemory(&GASFrame->u.GAS_INIT_REQ.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
					GASFrame->u.GAS_INIT_REQ.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE) {
				isDppFrame = TRUE;
				os_alloc_mem(NULL, (UCHAR **)&gas_frame, sizeof(struct dpp_gas_frame_list));
				if (gas_frame == NULL) {
					MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
						"Fail to allocate memory!\n");
					goto fail;
				} else {
					NdisZeroMemory(gas_frame, sizeof(struct dpp_gas_frame_list));

					NdisCopyMemory(gas_frame->sta_mac, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
					gas_frame->dpp_gas_ongoing = TRUE;
					OS_SEM_LOCK(&pAd->gas_frame_list_lock);
					if (pAd->is_dpp_gas_list_init == FALSE) {
						MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_WARN,
							"[DPP]: Gas DL List Init!\n");
						DlListInit(&pAd->dpp_gas_event_list);
						pAd->is_dpp_gas_list_init = TRUE;
					}
					DlListAddTail(&pAd->dpp_gas_event_list, &gas_frame->List);
					OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
				}
			}
			}
			break;

			case ACTION_GAS_INITIAL_RSP:
			{
				if ((GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
						NdisEqualMemory(&GASFrame->u.GAS_INIT_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
						GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)
					isDppFrame = TRUE;
			}
			break;

			case ACTION_GAS_COMEBACK_REQ:
			{
				OS_SEM_LOCK(&pAd->gas_frame_list_lock);
				/* For GAS comeback req no field presents to
				 * identify DPP frame so use link list for GAS
				 * frame corresponding to peer mac address
				 */
				if (!DlListEmpty(&pAd->dpp_gas_event_list) && pAd->is_dpp_gas_list_init == TRUE) {
					DlListForEach(gas_frame, &pAd->dpp_gas_event_list,  struct dpp_gas_frame_list, List) {
					if (NdisEqualMemory(gas_frame->sta_mac, GASFrame->Hdr.Addr2, MAC_ADDR_LEN)) {
						MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO,
									"Found Matching Frame\n");
						break;
					}
					}
				} else {
					MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
							"no gas frame in list\n");
					OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
					goto fail;
				}


				if (gas_frame == NULL) {
					MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
							"DPP Gas list with "MACSTR" not present\n", MAC2STR(GASFrame->Hdr.Addr2));
					OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
					goto fail;
				}

				if (gas_frame->dpp_gas_ongoing)
					isDppFrame = TRUE;

				OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
			}
			break;

			case ACTION_GAS_COMEBACK_RSP:
			{
				if ((GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
						NdisEqualMemory(&GASFrame->u.GAS_CB_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
						GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)
					isDppFrame = TRUE;
			}
			break;

			default:
				break;
			}
		}

		else
			isDppFrame = FALSE;
	}

fail:
	return isDppFrame;
}
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */

BOOLEAN CFG80211_CheckActionFrameType(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR preStr,
	IN PUCHAR pData,
	IN UINT32 length)
{
	BOOLEAN isP2pFrame = FALSE;
	struct ieee80211_mgmt *mgmt;
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	BOOLEAN isDppFrame = FALSE;
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */

	mgmt = (struct ieee80211_mgmt *)pData;

	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s ProbeRsp Frame %d\n",
				preStr, pAd->LatchRfRegs.Channel);

			if (!mgmt->u.probe_resp.timestamp) {
#if (KERNEL_VERSION(5, 0, 0) < LINUX_VERSION_CODE)
				struct timespec64 tv;

				ktime_get_real_ts64(&tv);
				mgmt->u.probe_resp.timestamp = (((UINT64) tv.tv_sec * 1000000000) + tv.tv_nsec)/1000;
#else
				struct timeval tv;

				do_gettimeofday(&tv);
				mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_usec;
#endif
			}
		}
#ifdef HOSTAPD_11R_SUPPORT
		else if (ieee80211_is_auth(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s AUTH Frame\n", preStr);
		else if (ieee80211_is_reassoc_req(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s REASSOC Req Frame\n", preStr);
		else if (ieee80211_is_reassoc_resp(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s REASSOC Resp Frame\n", preStr);
#endif	 /* HOSTAPD_11R_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
		else if (ieee80211_is_assoc_resp(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s ASSOC Resp Frame\n", preStr);
#endif
		else if (ieee80211_is_disassoc(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s DISASSOC Frame\n", preStr);
		else if (ieee80211_is_deauth(mgmt->frame_control))
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
				"CFG80211_PKT: %s Deauth Frame on %d\n",
				preStr, pAd->LatchRfRegs.Channel);
		else if (ieee80211_is_action(mgmt->frame_control)) {
			PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pData;
#ifdef HOSTAPD_11K_SUPPORT

		if (mgmt->u.action.category == CATEGORY_RM &&
			pFrame->Action == RRM_MEASURE_REP) {
			isP2pFrame = TRUE;
			/* CFG80211_HandleP2pMgmtFrame is being used for sending all pkts
			 * to upper layer, so isP2pFrame variable is being used
			 */
		}
#endif

#ifdef HOSTAPD_11R_SUPPORT
		if (mgmt->u.action.category == CATEGORY_FT)
			isP2pFrame = TRUE;
#endif

#ifdef HOSTAPD_11V_BTM_SUPPORT
		if (mgmt->u.action.category == CATEGORY_WNM) {
			isP2pFrame = TRUE;
#ifdef DBG
			if (pFrame->Action == ACTION_BSS_TRANSITION_MANAGEMENT_REQUEST &&
				NdisEqualMemory(preStr, "RX", 2))
				/* Too many logs may fail Re-Assoc */
				Set_Debug_Proc(pAd, "0");
#endif /* DBG */
			if ((pFrame->Action == EVENT_REPORT) && CFG80211_CheckWNMActionBSSColorReport(pAd, preStr, pData, length)) {
				/* Frame is sent to driver to proceed bss color report,
				 * it would be sent to os if it includes other event reports. */
				isP2pFrame = FALSE;
			}
		}
#endif /* HOSTAPD_11V_BTM_SUPPORT */

#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
			if (mgmt->u.action.category == CATEGORY_WNM) {
				if ((pFrame->Action == EVENT_REPORT) && CFG80211_CheckWNMActionBSSColorReport(pAd, preStr, pData, length)) {
					/* Frame is sent to driver to proceed bss color report,
					 * it would be sent to os if it includes
					 * other event reports. */
					isP2pFrame = FALSE;
				} else {
					isP2pFrame = TRUE;
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s WNM Frame bypass\n", preStr);
				}
			}
		}
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */

#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
			isDppFrame = CFG80211_CheckActionFrameTypeDpp(pAd, preStr, pData, length);
			if (isDppFrame == TRUE) {
				isP2pFrame = FALSE;
				goto send_driver;
			}
		}
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */
			if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
				(pFrame->Category == CATEGORY_PUBLIC) &&
				(pFrame->Action == ACTION_WIFI_DIRECT)) {
				isP2pFrame = TRUE;
				if (NdisEqualMemory(pFrame->OUI, MTK_OUI, 3))
					isP2pFrame = FALSE;

				switch (pFrame->Subtype) {
				case GO_NEGOCIATION_REQ:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s GO_NEGOCIACTION_REQ %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case GO_NEGOCIATION_RSP:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s GO_NEGOCIACTION_RSP %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case GO_NEGOCIATION_CONFIRM:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s GO_NEGOCIACTION_CONFIRM %d\n",
						preStr,  pAd->LatchRfRegs.Channel);
					break;

				case P2P_PROVISION_REQ:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_PROVISION_REQ %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case P2P_PROVISION_RSP:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_PROVISION_RSP %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case P2P_INVITE_REQ:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_INVITE_REQ %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case P2P_INVITE_RSP:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_INVITE_RSP %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case P2P_DEV_DIS_REQ:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_DEV_DIS_REQ %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;

				case P2P_DEV_DIS_RSP:
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
						"CFG80211_PKT: %s P2P_DEV_DIS_RSP %d\n",
						preStr, pAd->LatchRfRegs.Channel);
					break;
				}
			} else if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
					   (pFrame->Category == CATEGORY_PUBLIC) &&
					   ((pFrame->Action == ACTION_GAS_INITIAL_REQ)	 ||
						(pFrame->Action == ACTION_GAS_INITIAL_RSP)	 ||
						(pFrame->Action == ACTION_GAS_COMEBACK_REQ) ||
						(pFrame->Action == ACTION_GAS_COMEBACK_RSP)))
				isP2pFrame = TRUE;
			else if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
				(pFrame->Category == CATEGORY_PD))
				isP2pFrame = TRUE;
#ifdef HOSTAPD_HS_R2_SUPPORT
			else if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION)
				&& (pFrame->Category == CATEGORY_QOS)
				&& pFrame->Action == ACTION_QOS_MAP_CONFIG) {
				PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd, pFrame->p80211Header.Addr1);

				if (pEntry != NULL) {
					PHOTSPOT_CTRL pHSCtrl =
						&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].HotSpotCtrl;

					if (pHSCtrl->QosMapEnable) {
						int i = 0;
						UCHAR explen = 0;
						UCHAR PoolID = 0;

						for (i = 0; i < 21; i++) {
							if (pHSCtrl->DscpException[i] == 0xffff)
								break;
							explen += 2;
						}
						pEntry->DscpExceptionCount = explen;
						memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pHSCtrl->DscpRange, 16);
						memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pHSCtrl->DscpException, 42);

						PoolID = hotspot_qosmap_add_pool(pAd, pEntry);
						hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, PoolID);
					}
				}
			}
#endif
			else
				MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
					"CFG80211_PKT: %s ACTION Frame with Channel%d\n",
					preStr, pAd->LatchRfRegs.Channel);
		} else
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"CFG80211_PKT: %s UNKNOWN MGMT FRAME TYPE\n", preStr);
	} else
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
			"CFG80211_PKT: %s UNKNOWN FRAME TYPE\n", preStr);

#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
send_driver:
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */
	return isP2pFrame;
}

BOOLEAN CFG80211_isCfg80211FrameType(RTMP_ADAPTER *pAd, UINT16 frameSubType)
{
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (frameSubType == SUBTYPE_PROBE_REQ) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATRX_MGMT, DBG_LVL_DEBUG,
				" PROBE REQ is registered, maybe report probe req to CFG80211\n");
			return TRUE;
		}
		return FALSE;
	}

	if ((frameSubType != SUBTYPE_AUTH)
		&& (frameSubType != SUBTYPE_ASSOC_REQ)
		&& (frameSubType != SUBTYPE_REASSOC_REQ)
		&& (frameSubType != SUBTYPE_ACTION)
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
		&& (frameSubType != SUBTYPE_DEAUTH)
		&& (frameSubType != SUBTYPE_DISASSOC)
#endif
	)
		return FALSE;

	return TRUE;
}

/*
* This function will check if indicate related mgmt frame to CFG80211 when received pkts.
* Probe Request/Auth/(Re)Assoc Req/Action frames will be indicated currently.
*/
BOOLEAN CFG80211_HandleMgmtFrame(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, UCHAR OpMode)
{
	UINT32 freq = 0;
	PNET_DEV pNetDev;
	HEADER_802_11 Header;
	PHEADER_802_11 pHeader = &Header;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	struct wifi_dev *pWdev = wdev_search_by_address(pAd, pRxBlk->Addr1);

	NdisZeroMemory(&Header, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *) &(pHeader->FC), pRxBlk->FC, sizeof(FRAME_CONTROL));

	if (!CFG80211_isCfg80211FrameType(pAd, pHeader->FC.SubType))
		goto driver_process;

	NdisCopyMemory(pHeader->Addr1, pRxBlk->Addr1, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr2, pRxBlk->Addr2, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr3, pRxBlk->Addr3, MAC_ADDR_LEN);
	pHeader->Duration = pRxBlk->Duration;

	MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
	freq /= 1000;
	if (pWdev) {
		pNetDev = pWdev->if_dev;
		OpMode = pWdev->wdev_type == WDEV_TYPE_AP ? OPMODE_AP : OPMODE_STA;
		if ((pWdev->wdev_type == WDEV_TYPE_AP)
			&& ap_rx_mgmt_drop(pAd, (FRAME_CONTROL *)pRxBlk->FC))
			return TRUE; /* drop packet */
	} else {
		pNetDev = CFG80211_GetEventDevice(pAd);
	}

	if ((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) &&
		(pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame == TRUE)
		&& CFG80211_ProbeReqHandler(pAd, pWdev, pRxBlk, pNetDev, freq))
		return TRUE;
	else if ((pHeader->FC.SubType == SUBTYPE_AUTH)
		&& CFG80211_AuthHandle(pAd, pWdev, pRxBlk, pNetDev, pCfg80211_ctrl, freq))
		return TRUE;
	else if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ)
		&& CFG80211_AssocReqHandle(pAd, pWdev, pRxBlk))
		return TRUE;
	else if ((pHeader->FC.SubType == SUBTYPE_REASSOC_REQ)
		&& CFG80211_ReAssocReqHandle(pAd, pWdev, pRxBlk))
		return TRUE;
	else if ((pHeader->FC.SubType == SUBTYPE_ACTION)
		&& CFG80211_ActionHandle(pAd, pWdev, pRxBlk, pNetDev, freq))
		return TRUE;
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
	else if (pHeader->FC.SubType == SUBTYPE_DEAUTH || pHeader->FC.SubType == SUBTYPE_DISASSOC) {
		if (pWdev) {
			CFG80211OS_RxMgmt(
				pWdev->if_dev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
		} else {
			CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
		}
	}
#endif

driver_process:
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
	"Mmgt Frame(Subtype: 0x%x, SN: %d) will be process by driver instead of CFG80211\n",
	pHeader->FC.SubType, pRxBlk->SN);
	return FALSE;
}


#endif /* RT_CFG80211_SUPPORT */

