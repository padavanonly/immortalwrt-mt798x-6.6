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
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"


struct _auth_api_ops sta_auth_api;

#ifdef MAC_REPEATER_SUPPORT
#define IS_APCLI_RPT_IFINDEX_INVALID(ad, wdev, idx) (\
		IS_OPMODE_AP(ad)\
		&& (wdev)->wdev_type == WDEV_TYPE_STA\
		&& (idx) >= (hc_get_chip_cap((ad)->hdev_ctrl)->MaxRepeaterNum))
#else
#define IS_APCLI_RPT_IFINDEX_INVALID(ad, wdev, idx) (\
		IS_OPMODE_AP(ad)\
		&& (wdev)->wdev_type == WDEV_TYPE_STA\
		&& (idx) >= MAX_APCLI_NUM)

#endif /* MAC_REPEATER_SUPPORT */

BOOLEAN sta_send_auth_req(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem,
	IN PRALINK_TIMER_STRUCT pAuthTimer,
	IN RTMP_STRING *pSMName,
	IN USHORT SeqNo,
	IN PUCHAR pNewElement,
	IN ULONG ElementLen)
{
	USHORT Alg, Seq, Status;
	UCHAR Addr[6];
	ULONG Timeout;
	HEADER_802_11 AuthHdr;
	BOOLEAN TimerCancelled;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, tmp = 0;
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	USHORT ifIndex = wdev->func_idx;
	ASSERT(pStaCfg);
	ASSERT(wdev);
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	if (!pStaCfg)
		return FALSE;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return FALSE;

	/* Block all authentication request durning WPA block period */
	if (pStaCfg->bBlockAssoc == TRUE) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				 "%s - Block Auth request durning WPA block period!\n",
				  pSMName);
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		Status = MLME_STATE_MACHINE_REJECT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
		return FALSE;
	} else if (MlmeAuthReqSanity(pAd, Elem->wdev, Elem->Msg, Elem->MsgLen, Addr, &Timeout, &Alg)) {
		/* reset timer */
		RTMPCancelTimer(pAuthTimer, &TimerCancelled);
		COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, Addr);
		pStaCfg->MlmeAux.Alg = Alg;
		Seq = SeqNo;
		Status = MLME_SUCCESS;
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
					 "%s - MlmeAuthReqAction(Alg:%d) allocate memory failed\n",
					  pSMName, Alg);
			auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
			return FALSE;
		}

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			 "send auth req to "MACSTR" (alg=%d, seq=%d, status=%d)...\n",
			 MAC2STR(pStaCfg->MlmeAux.Bssid), Alg, Seq, Status);

		MgtMacHeaderInitExt(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr, wdev->if_addr,
							pStaCfg->MlmeAux.Bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
						  &AuthHdr, 2, &Alg, 2, &Seq, 2, &Status,
						  END_OF_ARGS);

		if (pNewElement && ElementLen) {
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
							  ElementLen, pNewElement, END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11_EHT_BE
		/* build multi-link element */
		if (WMODE_CAP_BE(wdev->PhyMode) && !IS_APCLI_DISABLE_MLO(wdev) && wdev->mld_dev
#ifdef APCLI_CFG80211_SUPPORT
			&& !(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
#endif
		)
			FrameLen += eht_add_auth_req_ies(wdev, pOutBuffer, FrameLen);
#endif /* DOT11_EHT_BE */

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory(pOutBuffer);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			"Set auth Timeout(%ld)ms\n", Timeout);
		RTMPSetTimer(pAuthTimer, Timeout);
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
			"%s sanity check fail\n", pSMName);
		return FALSE;
	}

	return TRUE;
}

VOID sta_mlme_deauth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *pInfo; /* snowpin for cntl mgmt */
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	pInfo = (MLME_DISCONNECT_STRUCT *) Elem->Msg; /* snowpin for cntl mgmt */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer); /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				 "AUTH - MlmeDeauthReqAction() allocate memory fail\n");
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		Status = MLME_FAIL_NO_RESOURCE;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DEAUTH_CONF, Status);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
		"send de-auth req to "MACSTR" (Reason=%d)...\n",
		MAC2STR(pStaCfg->MlmeAux.Bssid), pInfo->reason);
	MgtMacHeaderInitExt(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pInfo->addr,
						wdev->if_addr,
						pStaCfg->MlmeAux.Bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
					  &DeauthHdr, 2, &pInfo->reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DeauthReason = pInfo->reason;
	COPY_MAC_ADDR(pStaCfg->DeauthSta, pInfo->addr);
	auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
	Status = MLME_SUCCESS;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DEAUTH_CONF, Status);
	/* send wireless event - for deauthentication */
	RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL, BSS0, 0);
	return;
}



VOID sta_mlme_auth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	PRALINK_TIMER_STRUCT pAuthTimer = NULL;
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
		"wdev(type=%d,wdev_idx=%d,wdev_func_idx=%d\n",
		wdev->wdev_type, wdev->wdev_idx, wdev->func_idx);
	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		PREPEATER_CLIENT_ENTRY rept = (PREPEATER_CLIENT_ENTRY) wdev->func_dev;
		pAuthTimer = &rept->ApCliAuthTimer;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pAuthTimer = &pStaCfg->MlmeAux.AuthTimer;

	if (sta_send_auth_req(pAd, Elem, pAuthTimer, "AUTH", 1, NULL, 0))
		auth_fsm_state_transition(wdev, AUTH_FSM_WAIT_SEQ2, __func__);
	else {
		USHORT Status;

		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
	}

	return;
}

VOID sta_class2_error_action(struct wifi_dev *wdev, UCHAR *pAddr)
{
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Reason = REASON_CLS2ERR;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			 "AUTH - Class 2 error, Send DEAUTH frame...\n");
	MgtMacHeaderInitExt(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAddr,
						pStaCfg->wdev.if_addr,
						pStaCfg->MlmeAux.Bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
					  &DeauthHdr, 2, &Reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DeauthReason = Reason;
	COPY_MAC_ADDR(pStaCfg->DeauthSta, pAddr);
}

static VOID sta_auth_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	ASSERT(wdev);

	if (!wdev)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "AUTH - AuthTimeoutAction\n");
	auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
}

VOID sta_auth_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE, "AuthTimeout\n");
	MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_AUTH_TIMEOUT, 0, NULL, 0, pContext->wdev, FALSE, NULL);
	RTMP_MLME_HANDLER(pAd);
}

DECLARE_TIMER_FUNCTION(sta_auth_timeout);
BUILD_TIMER_FUNCTION(sta_auth_timeout);

#ifdef APCLI_CFG80211_SUPPORT
VOID sta_auth_timeout_ext(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_NOTICE, "%s(): reset ctrl state to idle\n", __func__);
	cntl_fsm_state_transition(pContext->wdev, CNTL_IDLE, __func__);
}

DECLARE_TIMER_FUNCTION(sta_auth_timeout_ext);
BUILD_TIMER_FUNCTION(sta_auth_timeout_ext);
#endif /* APCLI_CFG80211_SUPPORT */

VOID sta_peer_deauth_action(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	USHORT Reason;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
#ifdef DOT11_EHT_BE
	struct wifi_dev *mlo_setup_wdev = get_sta_mld_setup_wdev(wdev->mld_dev);
#endif
	USHORT ifIndex = wdev->func_idx;
	UINT link_down_type = 0;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

#ifdef WSC_INCLUDED
	{
		struct wifi_dev *wdev = &pStaCfg->wdev;
		WSC_CTRL *wsc_ctrl = &wdev->WscControl;
		BOOLEAN Cancelled;

		if (wsc_ctrl->WscState == WSC_STATE_WAIT_EAPFAIL) {
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&wsc_ctrl->WscEAPHandshakeCompleted,
								DISASSOC_WAIT_EAP_SUCCESS))
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
					"Time out!!!\n");
		}

		if (wsc_ctrl->WscState == WSC_STATE_WAIT_DISCONN) {
			wsc_ctrl->WscState = WSC_STATE_OFF;
			if (wsc_ctrl->EapolTimerRunning == TRUE) {
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
				wsc_ctrl->EapMsgRunning = FALSE;
			}
		}
	}
#endif /* WSC_INCLUDED */

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason)) {
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			"receive de-auth from "MACSTR", reason=%d\n",
			MAC2STR(Addr2), Reason);
		if ((INFRA_ON(pStaCfg)
#ifdef DOT11_EHT_BE
			|| mlo_setup_wdev != NULL
#endif
			) && MAC_ADDR_EQUAL(pStaCfg->Bssid, Addr2)) {
#ifdef DOT11_EHT_BE
			if (mlo_setup_wdev != NULL) {
				pAd = (PRTMP_ADAPTER)mlo_setup_wdev->sys_handle;
				if (pAd  == NULL) {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
							"mlo setuplink pad is null.\n");
					return;
				}
				pStaCfg = GetStaCfgByWdev(pAd, mlo_setup_wdev);
				if (pStaCfg == NULL) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
							"mlo can't find the setuplink sta cfg is null.\n");
					return;
				}

				if (pStaCfg->ApcliInfStat.Valid == FALSE) {
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
							"receive de-auth from " MACSTR
							", reason=%d, but setup link is not valid yet, do not action FSM\n",
							MAC2STR(Addr2), Reason);
							return;
				}
				wdev = mlo_setup_wdev;
				NdisMoveMemory(Addr2, pStaCfg->Bssid, MAC_ADDR_LEN);
				ifIndex = mlo_setup_wdev->func_idx;
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				"MLO forword to setuplink wdev("MACSTR")\n",
				MAC2STR(mlo_setup_wdev->if_addr));
			}
#endif

			if (Reason == REASON_4_WAY_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			if (Reason == REASON_GROUP_KEY_HS_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
									  IW_GROUP_HS_TIMEOUT_EVENT_FLAG,
									  NULL, 0, 0);

			/* send wireless event - for deauthentication */
			RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL, BSS0, 0);

#ifdef CONFIG_OWE_SUPPORT
			if (Elem->wdev->wdev_type == WDEV_TYPE_STA)
				sta_reset_owe_parameters(pAd, ifIndex);
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			do {
				UCHAR if_addr[6];
				INT CachedIdx;
#ifndef RT_CFG80211_SUPPORT
				SAE_INSTANCE *pSaeIns = NULL;
				struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);
#endif /* RT_CFG80211_SUPPORT */
				UINT32 sec_akm = 0;

				if (IS_AKM_SAE(wdev->SecConfig.AKMMap)
					|| IS_AKM_OWE(wdev->SecConfig.AKMMap)) {

					if (IS_AKM_SAE_SHA256(wdev->SecConfig.AKMMap))
						SET_AKM_SAE_SHA256(sec_akm);
					else if (IS_AKM_SAE_EXT(wdev->SecConfig.AKMMap))
						SET_AKM_SAE_EXT(sec_akm);
					else if (IS_AKM_OWE(wdev->SecConfig.AKMMap))
						SET_AKM_OWE(sec_akm);

#ifdef MAC_REPEATER_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
					REPEATER_CLIENT_ENTRY *rept_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
					COPY_MAC_ADDR(if_addr, rept_entry->CurrentAddress);
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					NdisCopyMemory(if_addr, pStaCfg->wdev.if_addr, MAC_ADDR_LEN);
				}
				CachedIdx = sta_search_pmkid_cache(pAd, Addr2, ifIndex, wdev,
					sec_akm, pStaCfg->Ssid, pStaCfg->SsidLen);

				if (CachedIdx != INVALID_PMKID_IDX) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
									" Delete pmkid on de-auth\n");
						sta_delete_pmkid_cache(pAd, Addr2, ifIndex, wdev,
							sec_akm, pStaCfg->Ssid, pStaCfg->SsidLen);
				}
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
				pSaeIns = search_sae_instance(sae_cfg, if_addr, Addr2);
				if (pSaeIns != NULL) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
							" Delete Existing sae instance on de-auth\n");
					delete_sae_instance(pSaeIns);
				}
#endif
				}
			} while (0);
#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */
			cntl_fsm_state_transition(wdev, CNTL_WAIT_DEAUTH, __func__);
			if (cntl_auth_assoc_conf(wdev, CNTL_MLME_DEAUTH_CONF, Reason)
				== FALSE) {
			link_down_type |= LINK_REQ_FROM_AP;
			LinkDown(pAd, link_down_type, wdev, Elem);
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
				 "AUTH_RSP - PeerDeauthAction() sanity check fail\n");
	}
}


VOID sta_peer_auth_rsp_at_seq2_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Seq, Status, RemoteStatus, Alg;
	UCHAR iv_hdr[4];
	UCHAR *ChlgText = NULL;
	UCHAR *CyperChlgText = NULL;
	ULONG c_len = 0;
	HEADER_802_11 AuthHdr;
	BOOLEAN TimerCancelled;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Status2;
	UCHAR ChallengeIe = IE_CHALLENGE_TEXT;
	UCHAR len_challengeText = CIPHER_TEXT_LEN;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	RALINK_TIMER_STRUCT *auth_timer = NULL;

	ASSERT(pStaCfg);
	ASSERT(wdev);
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		auth_timer = &rept->ApCliAuthTimer;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		auth_timer = &pStaCfg->MlmeAux.AuthTimer;

	os_alloc_mem(NULL, (UCHAR **) &ChlgText, CIPHER_TEXT_LEN);

	if (ChlgText == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "alloc mem fail\n");
		return;
	}

	os_alloc_mem(NULL, (UCHAR **) &CyperChlgText, CIPHER_TEXT_LEN + 8 + 8);

	if (CyperChlgText == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "CyperChlgText Allocate memory fail!!!\n");
		os_free_mem(ChlgText);
		return;
	}

	if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, (PCHAR)ChlgText)) {
		if (MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, Addr2) && Seq == 2) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
				 "received auth resp from "MACSTR
				 " (alg=%d, seq=%d, status=%d)...\n",
				 MAC2STR(Addr2), Alg, Seq, Status);
			RTMPCancelTimer(auth_timer, &TimerCancelled);

			if (Status == MLME_SUCCESS) {
				/* Authentication Mode "LEAP" has allow for CCX 1.X */
				if (pStaCfg->MlmeAux.Alg == Ndis802_11AuthModeOpen) {
					pEntry = MacTableLookup2(pAd, Addr2, wdev);
#if defined(DOT11_SAE_SUPPORT) && !defined(RT_CFG80211_SUPPORT)
					if (IS_AKM_SAE(pStaCfg->AKMMap)) {
						UCHAR if_addr[MAC_ADDR_LEN];
						UCHAR pmkid[LEN_PMKID];
						UCHAR pmk[LEN_PMK];
						UINT32 sec_akm = 0;
						struct __SAE_CFG *sae_cfg =
							PD_GET_SAE_CFG_PTR(pAd->physical_dev);

						if (IS_AKM_SAE_SHA256(pStaCfg->AKMMap))
							SET_AKM_SAE_SHA256(sec_akm);
						else if (IS_AKM_SAE_EXT(pStaCfg->AKMMap))
							SET_AKM_SAE_EXT(sec_akm);

						NdisCopyMemory(if_addr, pStaCfg->wdev.if_addr, MAC_ADDR_LEN);
						if (sae_get_pmk_cache(sae_cfg, if_addr,
							pStaCfg->MlmeAux.Bssid, FALSE, pmkid, pmk))
							sta_add_pmkid_cache(pAd, pStaCfg->MlmeAux.Bssid, pmkid, pmk, LEN_PMK,  pStaCfg->wdev.func_idx, wdev, sec_akm
								, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
						if (pEntry)
							NdisCopyMemory(&pEntry->SecConfig.sae_cap, &wdev->SecConfig.sae_cap, sizeof(struct sae_capability));
					}
#endif
					if (pEntry) {
						pEntry->SecConfig.rsnxe_len = pStaCfg->MlmeAux.rsnxe_len;
						NdisMoveMemory(pEntry->SecConfig.rsnxe_content, pStaCfg->MlmeAux.rsnxe_content, pStaCfg->MlmeAux.rsnxe_len);
					}

					auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
					cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
				} else {
					PSEC_KEY_INFO  pKey;
					UINT default_key = wdev->SecConfig.PairwiseKeyId;

					pKey = &wdev->SecConfig.WepKey[default_key];
					/* 2. shared key, need to be challenged */
					Seq++;
					RemoteStatus = MLME_SUCCESS;
					/* Get an unused nonpaged memory */
					NStatus =
						MlmeAllocateMemory(pAd,
										   &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(NULL,
							DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
							"AUTH - allocate memory fail\n");
						auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
						Status2 = MLME_FAIL_NO_RESOURCE;
						cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, Status2);
						goto LabelOK;
					}

					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
						"send auth req to "MACSTR
						" (alg=%d, seq=%d, status=%d)...\n",
						MAC2STR(Addr2), Alg, Seq, RemoteStatus);
					MgtMacHeaderInitExt(pAd, &AuthHdr,
										SUBTYPE_AUTH, 0, Addr2,
										wdev->if_addr,
										pStaCfg->MlmeAux.Bssid);
					AuthHdr.FC.Wep = 1;
#ifdef MAC_REPEATER_SUPPORT
					if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
						REPEATER_CLIENT_ENTRY *rept_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
						COPY_MAC_ADDR(AuthHdr.Addr2, rept_entry->CurrentAddress);
					}
#endif
					/* TSC increment */
					INC_TX_TSC(pKey->TxTsc, LEN_WEP_TSC);
					/* Construct the 4-bytes WEP IV header */
					RTMPConstructWEPIVHdr(default_key, pKey->TxTsc, iv_hdr);
					Alg = cpu2le16(*(USHORT *) &Alg);
					Seq = cpu2le16(*(USHORT *) &Seq);
					RemoteStatus = cpu2le16(*(USHORT *) &RemoteStatus);
					/* Construct message text */
					MakeOutgoingFrame(CyperChlgText, &c_len,
									  2, &Alg,
									  2, &Seq,
									  2, &RemoteStatus,
									  1, &ChallengeIe,
									  1, &len_challengeText,
									  len_challengeText,
									  ChlgText,
									  END_OF_ARGS);

					if (RTMPSoftEncryptWEP(iv_hdr,
										   pKey,
										   CyperChlgText, c_len) == FALSE) {
						MlmeFreeMemory(pOutBuffer);
						auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
						Status2 = MLME_FAIL_NO_RESOURCE;
						cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
						goto LabelOK;
					}

					/* Update the total length for 4-bytes ICV */
					c_len += LEN_ICV;
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof
									  (HEADER_802_11),
									  &AuthHdr,
									  LEN_WEP_IV_HDR,
									  iv_hdr, c_len,
									  CyperChlgText,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
					MlmeFreeMemory(pOutBuffer);
					RTMPSetTimer(auth_timer, AUTH_TIMEOUT);

					auth_fsm_state_transition(wdev, AUTH_FSM_WAIT_SEQ4, __func__);
				}
			} else {
				pStaCfg->AuthFailReason = Status;
				COPY_MAC_ADDR(pStaCfg->AuthFailSta, Addr2);
				auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
				cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			"received auth resp seq#2, sanity check fail\n");
	}

LabelOK:

	if (ChlgText != NULL)
		os_free_mem(ChlgText);

	if (CyperChlgText != NULL)
		os_free_mem(CyperChlgText);

	return;
}


VOID sta_peer_auth_rsp_at_seq4_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Alg, Seq, Status;
	/*    CHAR          ChlgText[CIPHER_TEXT_LEN]; */
	CHAR *ChlgText = NULL;
	BOOLEAN TimerCancelled;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	RALINK_TIMER_STRUCT *auth_timer = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "--->\n");

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		auth_timer = &rept->ApCliAuthTimer;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		auth_timer = &pStaCfg->MlmeAux.AuthTimer;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **) &ChlgText, CIPHER_TEXT_LEN);

	if (ChlgText == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "ChlgText Allocate memory fail!!!\n");
		return;
	}

	if (PeerAuthSanity
		(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status,
		 ChlgText)) {
		if (MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, Addr2) && Seq == 4) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
				"received auth resp from "MACSTR" (alg=%d, seq=%d, status=%d)...\n",
				MAC2STR(Addr2), Alg, Seq, Status);
			RTMPCancelTimer(auth_timer, &TimerCancelled);

			if (Status != MLME_SUCCESS) {
				pStaCfg->AuthFailReason = Status;
				COPY_MAC_ADDR(pStaCfg->AuthFailSta, Addr2);
				RTMPSendWirelessEvent(pAd, IW_SHARED_WEP_FAIL,
									  NULL, BSS0, 0);
			}

			auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			"received auth resp seq#4, sanity check fail\n");
	}

	if (ChlgText != NULL)
		os_free_mem(ChlgText);
}

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
/*
    ==========================================================================
    Description:

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID sta_sae_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
#ifndef RT_CFG80211_SUPPORT
	STA_ADMIN_CONFIG *pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	/* SAE_MLME_AUTH_REQ_STRUCT *AuthReq = (SAE_MLME_AUTH_REQ_STRUCT *)Elem->Msg; */
	MLME_AUTH_REQ_STRUCT *AuthReq = (MLME_AUTH_REQ_STRUCT *)Elem->Msg;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	UCHAR if_addr[ETH_ALEN];
	UCHAR *pSae_cfg_group = NULL;
	INT cache_idx = 0;
	UINT32 sec_akm = 0;
	struct _SECURITY_CONFIG *sec_cfg;
	struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "==>()\n");
	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, AuthReq->Addr);

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *pReptEntry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;

		pSae_cfg_group = &pReptEntry->sae_cfg_group;
		COPY_MAC_ADDR(if_addr, pReptEntry->CurrentAddress);
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pSae_cfg_group = &pStaCfg->sae_cfg_group;
		COPY_MAC_ADDR(if_addr, pStaCfg->wdev.if_addr);
	}

	sec_cfg = &wdev->SecConfig;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER)
		; /* do not need to redrive pt */
	else
#endif
	if (pStaCfg->MlmeAux.sae_conn_type) {
		struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

		sae_derive_pt(sae_cfg, sec_cfg->PSK, pStaCfg->MlmeAux.Ssid,
					pStaCfg->MlmeAux.SsidLen, NULL, &sec_cfg->pt_list);
	} else
		sae_pt_list_deinit(&sec_cfg->pt_list);

	if (pStaCfg->MlmeAux.sae_conn_type == SAE_CONNECTION_TYPE_SAEPK) {
#ifdef MAC_REPEATER_SUPPORT
		/* todo: check saepk in rsnxe */
		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				" not support saepk for repeater now, change to h2e\n");
			pStaCfg->MlmeAux.sae_conn_type = SAE_CONNECTION_TYPE_H2E;
		} else
#endif
		if (sec_cfg->sae_cap.sae_pk_en == SAE_PK_DISABLE ||
			FALSE == sae_pk_init(pAd,
								 &sec_cfg->sae_pk,
								 pStaCfg->MlmeAux.Ssid,
								 pStaCfg->MlmeAux.SsidLen,
								 SAE_PK_ROLE_SUPPLICANT,
								 sec_cfg->PSK)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				" sae_pk_en(%d) is disable or sae_pk_init fail, change to h2e\n",
					sec_cfg->sae_cap.sae_pk_en);
			pStaCfg->MlmeAux.sae_conn_type = SAE_CONNECTION_TYPE_H2E;
		}
	}
	if (pStaCfg->MlmeAux.sae_conn_type == SAE_CONNECTION_TYPE_H2E && sec_cfg->sae_cap.sae_pk_only_en == 1) {
		USHORT Status;
		Status = MLME_UNSPECIFY_FAIL;
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, Status);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"SAE PK Only case\n");
		return;
	}

	if (IS_AKM_SAE(wdev->SecConfig.AKMMap) && wdev->cntl_machine.CurrState == CNTL_WAIT_AUTH) {
		if (IS_AKM_SAE_SHA256(wdev->SecConfig.AKMMap))
			SET_AKM_SAE_SHA256(sec_akm);
		else if (IS_AKM_SAE_EXT(wdev->SecConfig.AKMMap))
			SET_AKM_SAE_EXT(sec_akm);

		cache_idx = sta_search_pmkid_cache(pAd, pStaCfg->MlmeAux.Bssid, ifIndex, wdev,
			sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
		if (cache_idx != INVALID_PMKID_IDX) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
							"(%s)[%d]: pmkid exist,delete cache_idx(%d)\n",
							__func__, __LINE__, cache_idx);
			sta_delete_pmkid_cache(pAd, pStaCfg->MlmeAux.Bssid, ifIndex, wdev,
				sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
		}
	}

	if (sae_auth_init(pAd, sae_cfg, wdev->if_addr, AuthReq->Addr,
		pStaCfg->MlmeAux.Bssid, wdev, *pSae_cfg_group, pStaCfg->MlmeAux.sae_conn_type))
		auth_fsm_state_transition(wdev, AUTH_FSM_WAIT_SAE, __func__);
	else {
		USHORT Status;
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, Status);
	}
#endif /* RT_CFG80211_SUPPORT */
}


/*
    ==========================================================================
    Description:

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID sta_sae_auth_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
#ifndef RT_CFG80211_SUPPORT
	STA_ADMIN_CONFIG *pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	FRAME_802_11 *Fr = (FRAME_802_11 *)Elem->Msg;
	struct wifi_dev *wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif
	USHORT seq;
	USHORT status;
	USHORT mlme_status;
	UCHAR *pmk;
	UCHAR sae_conn_type;
	struct __SAE_CFG *sae_cfg = PD_GET_SAE_CFG_PTR(pAd->physical_dev);

	ASSERT(pStaCfg);
	ASSERT(wdev);

	if (!pStaCfg)
		return;

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	NdisMoveMemory(&seq,    &Fr->Octet[2], 2);
	NdisMoveMemory(&status, &Fr->Octet[4], 2);
	if (FALSE == sae_handle_auth(pAd, sae_cfg, Elem->Msg, Elem->MsgLen,
						  wdev, seq, status, &pmk, &sae_conn_type, NULL)) {
		mlme_status = MLME_UNSPECIFY_FAIL;
		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, mlme_status);
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
		if ((IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd) && wdev)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "WPA3failReconfig\n");
			wapp_send_trigger_reconfig(pAd, wdev);
		}
#endif
#endif
	} else if (pmk != NULL) {
		MAC_TABLE_ENTRY *pEntry = NULL;
		hex_dump_with_lvl("pmk:", (char *)pmk, LEN_PMK, DBG_LVL_INFO);
#ifdef MAC_REPEATER_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			pReptEntry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
			if (pReptEntry) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "%s Move pmk to rept_PMK\n", __func__);
				NdisMoveMemory(pReptEntry->rept_PMK, pmk, LEN_PMK);
				mlme_status = MLME_SUCCESS;
			} else
				mlme_status = MLME_UNSPECIFY_FAIL;
		} else
#endif /*MAC_REPEATER_SUPPORT*/
		{
			pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);

			if (pEntry) {
				NdisMoveMemory(pEntry->SecConfig.PMK, pmk, LEN_PMK);
#ifdef SUPP_SAE_SUPPORT
				mtk_cfg80211_event_connect_params(pAd, pmk, LEN_PMK);
#endif
				pEntry->SecConfig.sae_conn_type = sae_conn_type;
				NdisCopyMemory(&pEntry->SecConfig.sae_cap, &wdev->SecConfig.sae_cap, sizeof(struct sae_capability));
				pEntry->SecConfig.rsnxe_len = pStaCfg->MlmeAux.rsnxe_len;
				NdisMoveMemory(pEntry->SecConfig.rsnxe_content, pStaCfg->MlmeAux.rsnxe_content, pStaCfg->MlmeAux.rsnxe_len);
				mlme_status = MLME_SUCCESS;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, " Security AKM = 0x%x, PairwiseCipher = 0x%x, GroupCipher = 0x%x\n",
						 pEntry->SecConfig.AKMMap, pEntry->SecConfig.PairwiseCipher, pEntry->SecConfig.GroupCipher);
			} else
				mlme_status = MLME_UNSPECIFY_FAIL;
		}

		auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(wdev, CNTL_MLME_AUTH_CONF, mlme_status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "**************Unhandled ************\n");
	}
#endif /* RT_CFG80211_SUPPORT */
}
#endif /* DOT11_SAE_SUPPORT */


VOID sta_auth_init(struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	sta_auth_api.mlme_deauth_req_action		= sta_mlme_deauth_req_action;
	sta_auth_api.mlme_auth_req_action			= sta_mlme_auth_req_action;
	sta_auth_api.auth_timeout_action			= sta_auth_timeout_action;
	sta_auth_api.peer_deauth_action				= sta_peer_deauth_action;
	sta_auth_api.peer_auth_rsp_at_seq2_action	= sta_peer_auth_rsp_at_seq2_action;
	sta_auth_api.peer_auth_rsp_at_seq4_action	= sta_peer_auth_rsp_at_seq4_action;
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	sta_auth_api.sae_auth_req_action		= sta_sae_auth_req_action;
	sta_auth_api.sae_auth_rsp_action		= sta_sae_auth_rsp_action;
#endif
	wdev->auth_api = &sta_auth_api;
	wdev->auth_machine.CurrState = AUTH_FSM_IDLE;

	/* if Timer is not init yet, init it */
	if (!pStaCfg->MlmeAux.AuthTimer.Valid) {
		pStaCfg->MlmeAux.AuthTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.AuthTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.AuthTimer,
					  GET_TIMER_FUNCTION(sta_auth_timeout), &pStaCfg->MlmeAux.AuthTimerFuncContext, FALSE);
	}

#ifdef APCLI_CFG80211_SUPPORT
	/* if Timer is not init yet, init it */
	if (!pStaCfg->MlmeAux.AuthTimerExt.Valid) {
		pStaCfg->MlmeAux.AuthTimerFuncContextExt.pAd = pAd;
		pStaCfg->MlmeAux.AuthTimerFuncContextExt.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.AuthTimerExt,
					  GET_TIMER_FUNCTION(sta_auth_timeout_ext), &pStaCfg->MlmeAux.AuthTimerFuncContextExt, FALSE);
	}
#endif /* APCLI_CFG80211_SUPPORT */
}

VOID sta_auth_send_deauth_frame(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct _HEADER_802_11 DeauthHdr;
	uint8_t *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT reason;
	struct _STA_ADMIN_CONFIG *sta_cfg;
	USHORT ifIndex = wdev->func_idx;

	ASSERT(wdev);
	sta_cfg = GetStaCfgByWdev(pAd, wdev);
	if (!sta_cfg) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "sta_cfg is NULL!\n");
		return;
	}
	ASSERT(sta_cfg);

	if (IS_APCLI_RPT_IFINDEX_INVALID(pAd, wdev, ifIndex))
		return;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			"DEAUTH - allocate memory fail\n");
		return;
	}

	reason = REASON_DEAUTH_STA_LEAVING;
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			 "DEAUTH - Send DE-AUTH request, reason=%d\n",
			  reason);
	MgtMacHeaderInitExt(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0,
		sta_cfg->Bssid,
		wdev->if_addr,
		sta_cfg->Bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
		&DeauthHdr, 2, &reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
	sta_cfg->DeauthReason = reason;
	/* send wireless event - for deauthentication */
	RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL, BSS0, 0);
#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_MODE_EN(pAd))
		slave_bh_event(pAd, wdev, FALSE);
#endif /* DFS_SLAVE_SUPPORT */
}

