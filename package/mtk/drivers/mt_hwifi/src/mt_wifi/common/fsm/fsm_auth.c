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


const CHAR *AUTH_FSM_STATE_STR[AUTH_FSM_MAX_STATE] = {
	"AUTH_IDLE",
	"AUTH_WAIT_SEQ2",
	"AUTH_FSM_WAIT_SEQ4",
	"AUTH_FSM_WAIT_SAE"
};

const CHAR *AUTH_FSM_MSG_STR[AUTH_FSM_MAX_MSG] = {
	"MLME_AUTH_REQ",
	"PEER_AUTH_EVEN",
	"PEER_AUTH_ODD",
	"AUTH_TIMEOUT",
	"PEER_DEAUTH",
	"MLME_DEAUTH_REQ",
	"PEER_AUTH_REQ",
	"PEER_AUTH_CONF",
	"SAE_AUTH_REQ",
	"SAE_AUTH_RSP"
};

static VOID auth_fsm_msg_invalid_state(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	ULONG curr_state = wdev->auth_machine.CurrState;
	USHORT Status = MLME_STATE_MACHINE_REJECT;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			 "[%s %s]: [%s][%s] ====================> FSM MSG DROP\n",
			wdev->if_dev->name,
			(wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  AUTH_FSM_STATE_STR[curr_state],
			  AUTH_FSM_MSG_STR[Elem->MsgType]);
	/* inform cntl this error */
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
}

inline BOOLEAN auth_fsm_state_transition(struct wifi_dev *wdev, ULONG next_state, const char *caller)
{
	ULONG old_state = 0;

		old_state = wdev->auth_machine.CurrState;
		wdev->auth_machine.CurrState = next_state;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			 "AUTH [%s, TYPE:%d %s]: [%s] \t==============================================> [%s]  (by %s)\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  AUTH_FSM_STATE_STR[old_state],
			  AUTH_FSM_STATE_STR[next_state],
			  caller);
	return TRUE;
}

VOID auth_fsm_mlme_deauth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->mlme_deauth_req_action)
			auth_api->mlme_deauth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_deauth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->mlme_deauth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

VOID auth_fsm_mlme_auth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->mlme_auth_req_action)
			auth_api->mlme_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->mlme_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "auth_req", DBG_LVL_INFO, &tl);
}

static VOID auth_fsm_auth_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->auth_timeout_action)
			auth_api->auth_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->auth_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID auth_fsm_peer_deauth_action(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_deauth_action)
			auth_api->peer_deauth_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_deauth_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_deauth_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID auth_fsm_peer_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_req_action)
			auth_api->peer_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_auth_req", DBG_LVL_INFO, &tl);
}


static VOID auth_fsm_peer_auth_rsp_at_seq2_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_rsp_at_seq2_action)
			auth_api->peer_auth_rsp_at_seq2_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_rsp_at_seq2_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_rsp_at_seq2_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "peer_auth_rsp_at_seq2", DBG_LVL_INFO, &tl);
}


static VOID auth_fsm_peer_auth_rsp_at_seq4_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_rsp_at_seq4_action)
			auth_api->peer_auth_rsp_at_seq4_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_peer_auth_rsp_at_seq4_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_rsp_at_seq4_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "peer_auth_rsp_at_seq4", DBG_LVL_INFO, &tl);
}



static VOID auth_fsm_peer_auth_confirm_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_confirm_action)
			auth_api->peer_auth_confirm_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_confirm_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_confirm_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
static VOID auth_fsm_sae_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->sae_auth_req_action)
			auth_api->sae_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_sae_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->sae_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "sae_auth_req", DBG_LVL_INFO, &tl);
}


static VOID auth_fsm_sae_auth_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->sae_auth_rsp_action)
			auth_api->sae_auth_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_sae_auth_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->sae_auth_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "sae_auth_rsp", DBG_LVL_INFO, &tl);
}
#endif /*DOT11_SAE_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef HOSTAPD_11R_SUPPORT
static VOID hostapd_auth_req(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 Fr;
	ULONG MsgLen;
	struct wifi_dev *wdev = NULL;
	UINT32 apidx;
	BSS_STRUCT *pMbss;
	PEID_STRUCT eid_ptr;
	UCHAR *Ptr;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *mlo_ie = NULL;
	uint16_t mld_sta_idx = MLD_STA_NONE;
	UINT8 mld_addr[MAC_ADDR_LEN] = {0};
	struct ml_ie_info *ml_info = NULL;

	if (!Elem) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATMLME_AUTH, DBG_LVL_ERROR, "AUTH - %s, Elem is null\n", __func__);
		return;
	}

	Fr = (PFRAME_802_11)Elem->Msg;
	MsgLen = Elem->MsgLen;

	apidx = get_apidx_by_addr(pAd, Fr->Hdr.Addr1);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "%s(%d), wdev is null\n", __func__, __LINE__);
		return;
	}

	Ptr = &Fr->Octet[6];
	eid_ptr = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
		if (eid_ptr->Eid == IE_WLAN_EXTENSION &&
			eid_ptr->Len && eid_ptr->Octet[0] == EID_EXT_EHT_MULTI_LINK) {

			mlo_ie = (UCHAR *)eid_ptr;

			if (mlo_ie) {
				os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
				if (!ml_info) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
						"failed to allocate memory for parsing ML IE\n");
					return;
				}
				NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));

				eht_parse_multi_link_ie(wdev, mlo_ie, NULL, ml_info);

#ifdef RT_CFG80211_SUPPORT
				wdev->do_not_send_sta_del_event_flag = 1;
#endif
				mld_sta_idx = eht_build_multi_link_conn_req(wdev,
					SUBTYPE_AUTH, mlo_ie, NULL, NULL, Fr->Hdr.Addr2, mld_addr, 0);
#ifdef RT_CFG80211_SUPPORT
				wdev->do_not_send_sta_del_event_flag = 0;
#endif

				if (ml_info) {
					os_free_mem(ml_info);
					ml_info = NULL;
				}

				if (mld_sta_idx == MLD_STA_NONE) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
						"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
						RtmpOsGetNetDevName(wdev->if_dev), Fr->Hdr.Addr2);
					return;
				}
			}

			pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);

			if (!pEntry) {
				pEntry = MacTableInsertEntry(
					pAd, Fr->Hdr.Addr2,
					wdev, ENTRY_CLIENT, OPMODE_AP,
					TRUE, mld_sta_idx, mld_addr);
			}
			break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	if (Fr && MsgLen) {
		UINT32 freq = 0;
		PNET_DEV pNetDev;
		struct wifi_dev *pWdev = wdev_search_by_address(pAd, Fr->Hdr.Addr1);

		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;
		if (pWdev)
			pNetDev = pWdev->if_dev;
		else
			pNetDev = CFG80211_GetEventDevice(pAd);

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
			"%s: FT Auth Req Rx, send it to hostapd\n", __func__);
		CFG80211OS_RxMgmt(pNetDev, freq,
			(PUCHAR)Fr, MsgLen);
	}
}
#endif

VOID hostapd_auth_resp(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	AUTH_FRAME_INFO *auth_info;	/* auth info from hostapd */
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfo;
#endif /* DOT11R_FT_SUPPORT */
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
#ifdef DOT11_EHT_BE
	UINT8 *ml_ie;
	UINT8 mld_addr[MAC_ADDR_LEN] = {0};
#endif
	uint16_t mld_sta_idx = MLD_STA_NONE;
	ULONG Data = 0;
	struct ieee80211_mgmt *mgmt = NULL;
	UINT8 apidx = 0;

	if (!Elem) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "AUTH - Elem is null\n");
		return;
	}

	mgmt = (struct ieee80211_mgmt *)(Elem->Msg);
	Data = Elem->MsgLen;

	apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "AUTH - Data len:%ld\n", Data);

	os_alloc_mem(pAd, (UCHAR **)&auth_info, sizeof(AUTH_FRAME_INFO));

	if (auth_info == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				"Fail to allocate memory!\n");
		return;
	}

	os_zero_mem(auth_info, sizeof(AUTH_FRAME_INFO));

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	pEntry = MacTableLookup(pAd, mgmt->da);
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
#ifdef DOT11W_PMF_SUPPORT
		if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		goto SendAuth;
#endif /* DOT11W_PMF_SUPPORT */

		if (!RTMPEqualMemory(mgmt->sa, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_WARN,
				"AUTH - Bssid does not match\n");
		} else {
#ifdef DOT11_N_SUPPORT
			if (!ISEntryDelSync(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
					"ENTRY ALREADY EXIST, TERADOWN BLOCKACK SESSION\n");
				ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);
			}
#endif /* DOT11_N_SUPPORT */
		}
	}

#ifdef DOT11W_PMF_SUPPORT
SendAuth:
#endif /* DOT11W_PMF_SUPPORT */

#ifdef HOSTAPD_11R_SUPPORT
	pFtCfg = &wdev->FtCfg;
	if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
		&& (mgmt->u.auth.auth_alg == AUTH_MODE_FT)) {
		/* USHORT result; */

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da, wdev,
				ENTRY_CLIENT, OPMODE_AP, TRUE, MLD_STA_NONE, NULL);

		if (pEntry != NULL) {
			/* fill auth info from upper layer response */
			COPY_MAC_ADDR(auth_info->addr2, mgmt->da);
			COPY_MAC_ADDR(auth_info->addr1, wdev->if_addr);
			auth_info->auth_alg = mgmt->u.auth.auth_alg;
			auth_info->auth_seq = mgmt->u.auth.auth_transaction;
			auth_info->auth_status = mgmt->u.auth.status_code;

			/* os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO)); */
			pFtInfo = &(pEntry->Auth_FtInfo);
			{
				PEID_STRUCT eid_ptr;
				UCHAR *Ptr;
				UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
				/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

				NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

				/* Ptr = &Fr->Octet[6]; */
				Ptr = mgmt->u.auth.variable;
				eid_ptr = (PEID_STRUCT) Ptr;

				/* get variable fields from payload and advance the pointer */
				while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
					switch (eid_ptr->Eid) {
					case IE_FT_MDIE:
						FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
						break;

					case IE_FT_FTIE:
						FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
						break;

					case IE_FT_RIC_DATA:
						/* record the pointer of first RDIE. */
						if (pFtInfo->RicInfo.pRicInfo == NULL) {
							pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
							pFtInfo->RicInfo.Len = ((UCHAR *)mgmt + Data)
													- (UCHAR *)eid_ptr + 1;
						}

						if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;


					case IE_FT_RIC_DESCRIPTOR:
						if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;

					case IE_RSN:
						if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
							NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
							pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
						}
						break;

					default:
						break;
					}
				eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
				}
			}

			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				NdisMoveMemory(&pEntry->MdIeInfo, &pFtInfo->MdIeInfo, sizeof(FT_MDIE_INFO));

				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, */
				/* if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
					"AuthState:%d, Sst:%d\n", pEntry->AuthState, pEntry->Sst);
			}
#ifdef RADIUS_MAC_AUTH_SUPPORT
			else {
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
			}
#endif
		}
		os_free_mem(auth_info);
		return;
	} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_SAE_SUPPORT
	if (mgmt->u.auth.auth_alg == AUTH_MODE_SAE
			&& (mgmt->u.auth.status_code == MLME_SUCCESS
			|| mgmt->u.auth.status_code == MLME_SAE_HASH_TO_ELEMENT
			|| mgmt->u.auth.status_code == MLME_SAE_PUBLIC_KEY)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"SAE Auth Response Sequence %d \n", mgmt->u.auth.auth_transaction);

#ifdef DOT11_EHT_BE
		ml_ie = pAd->ApCfg.MBSSID[0].wdev.hostapd_peer_ml_ie;

		/* sanity the peer mlo-capability if peer existing */
		if (mgmt->u.auth.auth_transaction == 1) {
			MacTableAuthReqClearMloEntry(pAd, mgmt->da,
				(ml_ie[0] == IE_WLAN_EXTENSION && ml_ie[2] == EID_EXT_EHT_MULTI_LINK));
		}

		if (ml_ie[0] == IE_WLAN_EXTENSION && ml_ie[2] == EID_EXT_EHT_MULTI_LINK) {
			wdev->do_not_send_sta_del_event_flag = 1;
			 /*do not report sta del event to hostapd, or auth2 will fail */
			mld_sta_idx = eht_build_multi_link_conn_req(wdev, SUBTYPE_AUTH,
				ml_ie, NULL, NULL, mgmt->da, mld_addr, 0);
			ml_ie[0] = 0; /* clear ml ie */
			ml_ie[2] = 0;
			wdev->do_not_send_sta_del_event_flag = 0;

			if (mld_sta_idx == MLD_STA_NONE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
					"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
					RtmpOsGetNetDevName(wdev->if_dev), mgmt->da);
				goto auth_failure;
			}
		}
#endif
		pEntry = MacTableLookup(pAd, mgmt->da);

		if (!pEntry) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"pEntry search fail, mld_sta_idx = %d\n", mld_sta_idx);
			pEntry = MacTableInsertEntry(
				pAd, mgmt->da,
				wdev, ENTRY_CLIENT, OPMODE_AP,
				TRUE, mld_sta_idx, mld_addr);
		}

		if (pEntry) {
			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, */
				/* if it already in SST_ASSOC, it can not go back */

				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
			} else {
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
#endif
			}
		}
	} else
#endif
	if ((mgmt->u.auth.auth_alg == AUTH_MODE_OPEN) &&
		(!IS_AKM_SHARED(pMbss->wdev.SecConfig.AKMMap))) {
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da,
				wdev, ENTRY_CLIENT, OPMODE_AP,
				TRUE, MLD_STA_NONE, NULL);

		if (pEntry) {
			tr_entry = tr_entry_get(pAd, pEntry->wcid);
#ifdef DOT11W_PMF_SUPPORT
		if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
			|| (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED))
#endif /* DOT11W_PMF_SUPPORT */
			{
				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, if it already in SST_ASSOC, */
				/* it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
			}
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
				"pEntry created: auth state:%d, Sst:%d\n",
				 pEntry->AuthState, pEntry->Sst);
			/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, */
			/* auth_info.auth_alg, auth_info.auth_seq + 1, MLME_SUCCESS); */

		} else
			; /* MAC table full, what should we respond ????? */
	} else {
		/* wrong algorithm */
		/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, */
		/* auth_info.auth_alg, auth_info.auth_seq + 1, MLME_ALG_NOT_SUPPORT); */

		/* If this STA exists, delete it. */
		if (pEntry) {
#if defined(EM_PLUS_SUPPORT) && defined(WAPP_SUPPORT)
			wapp_send_sta_connect_rejected(pAd, pEntry->wdev, pEntry->Addr,
					pEntry->bssid,
					WAPP_AUTH, MLME_UNSPECIFY_FAILURE,
					REASON_4_WAY_TIMEOUT, 0);
#endif
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "AUTH - Alg=%d, Seq=%d\n",
				auth_info->auth_alg, auth_info->auth_seq);
	}

auth_failure:
	os_free_mem(auth_info);
}

#endif /*RT_CFG80211_SUPPORT*/


VOID auth_fsm_init(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AUTH_FSM_MAX_STATE, AUTH_FSM_MAX_MSG,
					 (STATE_MACHINE_FUNC)auth_fsm_msg_invalid_state, AUTH_FSM_IDLE, AUTH_FSM_BASE);
	/* cmm */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_AUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_AUTH_CONF, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_confirm_action);
	/* the first column */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);
	/* the second column */
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_rsp_at_seq2_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);
	/* the third column */
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_rsp_at_seq4_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	/* only for sta mode, todo: unify with ap mode */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_SAE_AUTH_REQ, (STATE_MACHINE_FUNC) auth_fsm_sae_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_SAE_AUTH_RSP, (STATE_MACHINE_FUNC) auth_fsm_sae_auth_rsp_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
#endif /*DOT11_SAE_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef HOSTAPD_11R_SUPPORT
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_HOSTAPD_AUTH_REQ, (STATE_MACHINE_FUNC) hostapd_auth_req);
#endif
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_HOSTAPD_AUTH_RSP, (STATE_MACHINE_FUNC) hostapd_auth_resp);
#endif /*RT_CFG80211_SUPPORT*/


	wdev->auth_machine.CurrState = AUTH_FSM_IDLE;
}

VOID auth_fsm_reset(struct wifi_dev *wdev)
{
	auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
}

