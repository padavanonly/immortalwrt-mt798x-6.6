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
	cmm_cmd.c

	Abstract:
	All command related API.

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
	Paul Lin    06-25-2004  created
*/

#include "rt_config.h"


#ifdef DBG_STARVATION
static void cmdq_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct _CmdQElmt *cmd = container_of(starv, struct _CmdQElmt, starv);
	struct _CmdQ *cmdq = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (!log)
		return;

	log->qsize = cmdq->size;
	log->id = cmd->command;
	entry->log = log;
}

static void cmdq_starv_block_init(struct starv_log *ctrl, struct _CmdQ *cmdq)
{
	struct starv_dbg_block *block = &cmdq->block;

	strncpy(block->name, "cmdq", sizeof(block->name));
	block->priv = cmdq;
	block->ctrl = ctrl;
	block->timeout = 100;
	block->timeout_fn = cmdq_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}
#endif /*DBG_STARVATION*/


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTInitializeCmdQ(
	IN	PCmdQ	cmdq)
{
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTThreadDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt * pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL) {
		cmdq->head = cmdq->head->next;
		cmdq->size--;

		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
NDIS_STATUS RTEnqueueInternalCmd(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength)
{
	NDIS_STATUS	status;
	ULONG	flag = 0;
	PCmdQElmt	cmdqelmt = NULL;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			"---> NIC is not exist!!\n");
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(CmdQElmt));

	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(cmdqelmt, sizeof(CmdQElmt));

	if (InformationBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, InformationBufferLength);

		if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL)) {
			os_free_mem(cmdqelmt);
			return NDIS_STATUS_RESOURCES;
		} else {
			NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
			cmdqelmt->bufferlength = InformationBufferLength;
		}
	} else {
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = FALSE;

	if (cmdqelmt != NULL) {
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->CmdQLock, &flag);

		if ((pAd->CmdQ.size < MAX_LEN_OF_CMD_QUEUE) &&
			(pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)) {
#ifdef DBG_STARVATION
			starv_dbg_init(&pAd->CmdQ.block, &cmdqelmt->starv);
			starv_dbg_get(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
			EnqueueCmd((&pAd->CmdQ), cmdqelmt);
			status = NDIS_STATUS_SUCCESS;
		} else
			status = NDIS_STATUS_FAILURE;

		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CmdQLock, &flag);

		if (status == NDIS_STATUS_FAILURE) {
			if (cmdqelmt->buffer)
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		} else
			RTCMDUp(&pAd->cmdQTask);
	}

	return status;
}




/*Define common Cmd Thread*/



#ifdef CONFIG_STA_SUPPORT
static NTSTATUS SetPortSecuredHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	struct wifi_dev *wdev, *wdev_tmp;

	wdev_tmp = (struct wifi_dev *) CMDQelmt->buffer;
	wdev = wdev_search_by_address(pAd, wdev_tmp->if_addr);

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
			"caller:%pS,wdev(if)addr="MACSTR") is NULL\n", OS_TRACE, MAC2STR(wdev_tmp->if_addr));
		return NDIS_STATUS_FAILURE;
	}

	STA_PORT_SECURED_BY_WDEV(pAd, wdev);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT */




#ifdef CONFIG_AP_SUPPORT
#ifdef MBO_SUPPORT
VOID MBO_Send_Disassoc(PRTMP_ADAPTER pAd, UCHAR apidx, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	int i = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	struct wifi_dev *wdev_bss;

	wdev_bss = &pAd->ApCfg.MBSSID[apidx].wdev;

	if ((apidx < pAd->ApCfg.BssidNum)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
			"Send DISASSOC frame(%d) with ra%d \n", Reason, apidx);

		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pMacEntry = entry_get(pAd, i);
			if (!pMacEntry)
				continue;
			if (pMacEntry->wdev != wdev_bss)
				continue;
			if (IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef CONFIG_AP_SUPPORT
				/* Send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO,
							DBG_LVL_INFO,
							" MlmeAllocateMemory fail	..\n");
						return;
					}

					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
						": Send DISASSOC frame (Reason=%d) to "MACSTR"\n",
						Reason, MAC2STR(pMacEntry->Addr));

					/* 802.11 Header */
					NdisZeroMemory(&DisassocHdr, sizeof(HEADER_802_11));
					DisassocHdr.FC.Type = FC_TYPE_MGMT;
					DisassocHdr.FC.SubType = SUBTYPE_DISASSOC;
					DisassocHdr.FC.ToDs = 0;
					DisassocHdr.FC.Wep = 0;
					COPY_MAC_ADDR(DisassocHdr.Addr1, pMacEntry->Addr);
					COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
					COPY_MAC_ADDR(DisassocHdr.Addr3, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
							sizeof(HEADER_802_11), &DisassocHdr,
							2, &Reason,
							END_OF_ARGS);

					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
					MlmeFreeMemory(pOutBuffer);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
		}
    }
}

static NTSTATUS BssTerminate(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR apidx;

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));

#ifdef FT_R1KH_KEEP
	pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */

	/*
	Inorder to pass MBO Cert 4.2.5.4, APUT should send out a DISASSOC frame when Termination timer times out
	*/
	MBO_Send_Disassoc(pAd, apidx, REASON_NO_LONGER_VALID);

	APStop(pAd, &pAd->ApCfg.MBSSID[apidx], AP_BSS_OPER_BY_RF);
	/* MlmeRadioOff(pAd, wdev); */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO, "==>(OFF)\n");

	return NDIS_STATUS_SUCCESS;
}
#endif

static NTSTATUS _802_11_CounterMeasureHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MAC_TABLE_ENTRY *pEntry;

		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}
	return NDIS_STATUS_SUCCESS;
}

#if defined(RACTRL_LIMIT_MAX_PHY_RATE) || defined(CONFIG_RA_PHY_RATE_SUPPORT)
static NTSTATUS UpdteMaxRA(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	struct _STA_REC_CTRL_T *strec;

	pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	strec = &tr_entry->StaRec;

	strec->EnableFeature = 0;
	if (pEntry->update_he_maxra == true)
		strec->EnableFeature |= STA_REC_HE_BASIC_FEATURE;
	if (pEntry->update_eht_maxra == true)
		strec->EnableFeature |= STA_REC_EHT_BASIC_FEATURE;
	AsicStaRecUpdate(pAd, strec);
	WifiSysRaInit(pAd, pEntry);

	pEntry->update_he_maxra = false;
	pEntry->update_eht_maxra = false;

	return NDIS_STATUS_SUCCESS;
}
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */



static NTSTATUS ApSoftReStart(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	BSS_STRUCT *pMbss;
	UCHAR apidx;

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	pMbss = &pAd->ApCfg.MBSSID[apidx];

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "cmd> ApSoftReStart: apidx = %d\n", apidx);
	APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
	APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);

	return NDIS_STATUS_SUCCESS;
}

#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
static NTSTATUS CmdMlmeRstStateMacHandler(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
		PWSC_CTRL pWscControl = (PWSC_CTRL)CMDQelmt->buffer;
		struct wifi_dev *wdev = NULL;

		if (pWscControl != NULL)
			wdev = (struct wifi_dev *)pWscControl->wdev;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			"cmd> CMDTHREAD_MLME_RESET_STATE_MACHINE\n");
		if (wdev != NULL)
			RTMP_MLME_RESET_STATE_MACHINE(pAd, wdev);

		return NDIS_STATUS_SUCCESS;
}

static NTSTATUS ApCliPbcApFoundHandler(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PSTA_ADMIN_CONFIG pApCliTab = NULL;
	UCHAR channel = 0;
	BOOLEAN apcliEn;
	UCHAR apidx = 0;

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	pApCliTab = &pAd->StaCfg[apidx];

	channel = pApCliTab->MlmeAux.Channel;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		"cmd> channel=%d CMDTHREAD_APCLI_PBC_AP_FOUND!\n", channel);
	/* XXX: Check if channel change is required */
	rtmp_set_channel(pAd, &pApCliTab->wdev, channel);

	/* Bring down ApCli If */
	apcliEn = pApCliTab->ApcliInfStat.Enable;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		"cmd>  CMDTHREAD_APCLI_PBC_AP_FOUND!apcliEn=%d\n", apcliEn);
	if (apcliEn == TRUE && pApCliTab->ApcliInfStat.Valid == TRUE) {
		pApCliTab->ApcliInfStat.Enable = FALSE;
		ApCliIfDown(pAd);
	}

	if (pApCliTab->ApcliInfStat.Valid == FALSE &&
		pApCliTab->ApcliInfStat.Enable == FALSE)
		pApCliTab->ApcliInfStat.Enable = apcliEn;

	/* Change WPS State */
	if (pApCliTab->ApcliInfStat.Enable) {
		pApCliTab->wdev.WscControl.WscState = WSC_STATE_START;
		pApCliTab->wdev.WscControl.WscStatus = STATUS_WSC_START_ASSOC;
	}

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS ApCliSetChannel(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PSTA_ADMIN_CONFIG pApCliTab = (PSTA_ADMIN_CONFIG)CMDQelmt->buffer;
	UCHAR channel = 0;

	channel = pApCliTab->MlmeAux.Channel;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		"cmd> channel=%d CMDTHREAD_APCLI_PBC_TIMEOUT!\n", channel);
	rtmp_set_channel(pAd, &pApCliTab->wdev, channel);

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_INCLUDED */

static NTSTATUS CmdApCliIfDown(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR apidx = 0;
	BOOLEAN apcliEn;
#if defined(CONFIG_MAP_SUPPORT) && defined(WSC_INCLUDED)
	WSC_CTRL *wsc_ctrl = NULL;
#endif /* CONFIG_MAP_SUPPORT */

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	apcliEn = pAd->StaCfg[apidx].ApcliInfStat.Enable;
#if defined(CONFIG_MAP_SUPPORT) && defined(WSC_INCLUDED)
	wsc_ctrl = &pAd->StaCfg[apidx].wdev.WscControl;
#endif /* CONFIG_MAP_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		"cmd>  CMDTHREAD_APCLI_IF_DOWN! apidx=%u, apcliEn=%d\n", apidx, apcliEn);

	/* bring apcli interface down first */
	if (apcliEn == TRUE) {
		pAd->StaCfg[apidx].ApcliInfStat.Enable = FALSE;
		ApCliIfDown(pAd);
	}
#if defined(CONFIG_MAP_SUPPORT) && defined(WSC_INCLUDED)
	if (wsc_ctrl && !IS_MAP_TURNKEY_ENABLE(pAd) &&
			wsc_ctrl->WscState == WSC_STATE_OFF)
#endif /* CONFIG_MAP_SUPPORT */
		pAd->StaCfg[apidx].ApcliInfStat.Enable = apcliEn;

	return NDIS_STATUS_SUCCESS;
}

#ifdef WSC_AP_SUPPORT
static NTSTATUS CmdWscApCliLinkDown(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR apidx = 0;

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		"cmd>  CMDTHREAD_WSC_APCLI_LINK_DOWN! apidx=%u\n", apidx);
	WscApCliLinkDownById(pAd, apidx);
	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_AP_SUPPORT */

#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static NTSTATUS SetPSMBitHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		PPSM_BIT_CTRL_T	 pPsmBitCtrl = (PPSM_BIT_CTRL_T)CMDQelmt->buffer;

		MlmeSetPsmBit(pAd, pPsmBitCtrl->pStaCfg, pPsmBitCtrl->psm_val);
	}
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS QkeriodicExecutHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	StaQuickResponeForRateUpExec(NULL, pAd, NULL, NULL);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT*/


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS ChannelRescanHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	/*SUPPORT RTMP_CHIP ONLY, Single Band*/
	UCHAR Channel;
	struct wifi_dev *wdev;
	AUTO_CH_CTRL *pAutoChCtrl;
	UCHAR apidx;
	BSS_STRUCT *pMbss;

	NdisMoveMemory(&apidx, CMDQelmt->buffer, sizeof(UCHAR));
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "cmd> get wdev fail!\n");
		return NDIS_STATUS_FAILURE;
	}
	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	Channel = APAutoSelectChannel(pAd, wdev, TRUE, pAutoChCtrl->pChannelInfo->IsABand);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "cmd> Re-scan channel!\n");
	if (!pAd->ApCfg.auto_ch_score_flag) {
		wdev->channel = Channel;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"cmd> Switch to %d!\n", Channel);
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT*/

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
static NTSTATUS RegHintHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RegHint11DHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT11D(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ScanEnd(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_SCAN_END(pAd, FALSE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ConnResultInfom(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	CFG80211_CONN_RESULT_INFORM(pAd, pAd->StaCfg[0].MlmeAux.Bssid, 0,
		pAd->StaCfg[0].ReqVarIEs, pAd->StaCfg[0].ReqVarIELen,
		CMDQelmt->buffer, CMDQelmt->bufferlength, WLAN_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef CONFIG_STA_SUPPORT
static NTSTATUS sta_deauth_act_handle(struct _RTMP_ADAPTER *ad, struct _CmdQElmt *elem)
{
	struct wifi_dev *wdev_tmp = (struct wifi_dev *) elem->buffer;
	/*wdev_tmp is just a copy of real wdev, so get real wdev pointer by if_addr*/
	struct wifi_dev *wdev = wdev_search_by_address(ad, wdev_tmp->if_addr);
	struct _STA_ADMIN_CONFIG *sta_cfg = NULL;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
		"caller:%pS,wdev(if)addr="MACSTR"\n", OS_TRACE, MAC2STR(wdev_tmp->if_addr));

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
			"caller:%pS,wdev(if)addr="MACSTR") is NULL\n", OS_TRACE, MAC2STR(wdev_tmp->if_addr));
		return NDIS_STATUS_FAILURE;
	}
	sta_cfg = GetStaCfgByWdev(ad, wdev);

	cntl_disconnect_request(wdev,
								CNTL_DEAUTH,
								sta_cfg->Bssid,
								REASON_DEAUTH_STA_LEAVING);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS sta_deassoc_act_handle(struct _RTMP_ADAPTER *ad, struct _CmdQElmt *elem)
{
	struct wifi_dev *wdev_tmp = (struct wifi_dev *) elem->buffer;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept;
#endif /* MAC_REPEATER_SUPPORT */
	/*wdev_tmp is just a copy of real wdev, so get real wdev pointer by if_addr*/
	struct wifi_dev *wdev = wdev_search_by_address(ad, wdev_tmp->if_addr);
	struct _STA_ADMIN_CONFIG *sta_cfg = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
			"caller:%pS,wdev(if)addr="MACSTR") is NULL\n", OS_TRACE, MAC2STR(wdev_tmp->if_addr));
		return NDIS_STATUS_FAILURE;
	}
	sta_cfg = GetStaCfgByWdev(ad, wdev);

	switch (wdev->wdev_type) {
#ifdef MAC_REPEATER_SUPPORT
	case WDEV_TYPE_REPEATER:
		rept = &ad->ApCfg.pRepeaterCliPool[wdev->func_idx];
		cntl_disconnect_request(&rept->wdev,
							CNTL_DISASSOC,
							rept->wdev.bssid,
							REASON_DISASSOC_STA_LEAVING);
		break;
#endif /* MAC_REPEATER_SUPPORT */
	case WDEV_TYPE_STA:
		cntl_disconnect_request(&sta_cfg->wdev,
							CNTL_DISASSOC,
							sta_cfg->Bssid,
							REASON_DISASSOC_STA_LEAVING);
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"unknown wdev_type\n");
		break;
	}
	return NDIS_STATUS_SUCCESS;
}

#endif /*CONFIG_STA_SUPPORT*/

#ifdef DOT11_EHT_BE
static NTSTATUS MldApReconfigHandler(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	struct RECONFIG_TMR_TO_CTRL_T *reconfig_to_ctrl = CMDQelmt->buffer;
	struct reconfig_tmr_to_t reconfig_to_info = {0};
	UINT8 link;

	reconfig_to_info.args.reconfig_mode = MLD_RECONFIG_MODE_COUNTDOWN;
	COPY_MAC_ADDR(reconfig_to_info.mld_addr, reconfig_to_ctrl->aucMldAddr);
	reconfig_to_info.fw_mld_idx = reconfig_to_ctrl->ucFwMldIdx;
	reconfig_to_info.to_link_id_bmap = reconfig_to_ctrl->ucToLinkIdBmap;
	for (link = 0; link < CFG_WIFI_RAM_BAND_NUM; link++) {
		if (reconfig_to_ctrl->ucToLinkIdBmap & BIT(link))
			reconfig_to_info.fw_bss_idx[link] = reconfig_to_ctrl->aucFwBssIdxLink[link];
	}

	return eht_ap_mld_exec_link_reconfiguration(&reconfig_to_info);
}

static NTSTATUS MldApAt2lmToHandler(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	int ret = 0;
	struct AT2LM_TMR_TO_CTRL_T *at2lm_tmr_to = CMDQelmt->buffer;

	ret = at2lm_tsf_expiry(at2lm_tmr_to->aucMldAddr,
		at2lm_tmr_to->ucFwMldIdx,
		at2lm_tmr_to->ucAt2lmId,
		at2lm_tmr_to->ucType);

	return (ret == 0) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
}

#endif /* DOT11_EHT_BE */

typedef NTSTATUS (*CMDHdlr)(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt);


typedef struct {
	UINT32 CmdID;
	CMDHdlr CmdHdlr;
} MT_CMD_TABL_T;

static MT_CMD_TABL_T CMDHdlrTable[] = {

	/*STA related*/
#ifdef CONFIG_STA_SUPPORT
	{CMDTHREAD_SET_PSM_BIT, SetPSMBitHdlr},
	{CMDTHREAD_QKERIODIC_EXECUT, QkeriodicExecutHdlr},
	{CMDTHREAD_SET_PORT_SECURED, SetPortSecuredHdlr},
#endif
	/*AP related*/
#ifdef CONFIG_AP_SUPPORT
	{CMDTHREAD_CHAN_RESCAN, ChannelRescanHdlr},
	{CMDTHREAD_802_11_COUNTER_MEASURE, _802_11_CounterMeasureHdlr},
	{CMDTHREAD_AP_RESTART, ApSoftReStart},
#ifdef APCLI_SUPPORT
	{CMDTHREAD_APCLI_IF_DOWN, CmdApCliIfDown},
#ifdef WSC_INCLUDED
	{CMDTHREAD_APCLI_PBC_TIMEOUT, ApCliSetChannel},
	{CMDTHREAD_APCLI_PBC_AP_FOUND, ApCliPbcApFoundHandler},
	{CMDTHREAD_MLME_RESET_STATE_MACHINE, CmdMlmeRstStateMacHandler},
#endif /* WSC_INCLUDED */
#ifdef WSC_AP_SUPPORT
	{CMDTHREAD_WSC_APCLI_LINK_DOWN, CmdWscApCliLinkDown},
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif
	/*CFG 802.11*/
#if  defined(LINUX) && defined(RT_CFG80211_SUPPORT)
	{CMDTHREAD_REG_HINT, RegHintHdlr},
	{CMDTHREAD_REG_HINT_11D, RegHint11DHdlr},
	{CMDTHREAD_SCAN_END, RT_Mac80211_ScanEnd},
	{CMDTHREAD_CONNECT_RESULT_INFORM, RT_Mac80211_ConnResultInfom},
#endif
#ifdef PHY_ICS_SUPPORT
	{CMDTHRED_PHY_ICS_DUMP_RAW_DATA, PhyIcsRawDataHandler},
#endif /* PHY_ICS_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	{CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, WifiSpectrumRawDataHandler},
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	{CMDTHRED_ICAP_DUMP_RAW_DATA, ICapRawDataHandler},
#endif/* INTERNAL_CAPTURE_SUPPORT */
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	{CMDTHRED_PRECAL_TXLPF, PreCalTxLPFStoreProcHandler},
	{CMDTHRED_PRECAL_TXIQ, PreCalTxIQStoreProcHandler},
	{CMDTHRED_PRECAL_TXDC, PreCalTxDCStoreProcHandler},
	{CMDTHRED_PRECAL_RXFI, PreCalRxFIStoreProcHandler},
	{CMDTHRED_PRECAL_RXFD, PreCalRxFDStoreProcHandler},
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */
#ifdef CONFIG_AP_SUPPORT
	{CMDTHRED_DOT11H_SWITCH_CHANNEL, Dot11HCntDownTimeoutAction},
#endif /* CONFIG_AP_SUPPORT */
#ifdef MT_DFS_SUPPORT
	{CMDTHRED_DFS_CAC_TIMEOUT, DfsChannelSwitchTimeoutAction},
	{CMDTHRED_DFS_RADAR_DETECTED_SW_CH, DfsSwitchChAfterRadarDetected},
	{CMDTHRED_DFS_AP_RESTART, DfsAPRestart},
#endif
#ifdef CONFIG_STA_SUPPORT
	{CMDTHRED_STA_DEAUTH_ACT, sta_deauth_act_handle},
	{CMDTHRED_STA_DEASSOC_ACT, sta_deassoc_act_handle},
#endif /* CONFIG_STA_SUPPORT */
#ifdef FW_LOG_DUMP
	{CMDTHRED_FW_LOG_OPEN_CLOSE_FILE, fw_log_open_close_file},
	{CMDTHRED_FW_LOG_TO_FILE, fw_log_write_file},
	{CMDTHRED_FW_LOG_ALLOC_MEMORY, fw_log_alloc_memory},
	{CMDTHRED_FW_LOG_TO_MEMORY, fw_log_write_memory},
#endif
#ifdef MBO_SUPPORT
	{CMDTHREAD_BSS_TERM, BssTerminate},
#endif
#if defined(RACTRL_LIMIT_MAX_PHY_RATE) || defined(CONFIG_RA_PHY_RATE_SUPPORT)
	{CMDTHREAD_UPDATE_MAXRA, UpdteMaxRA},
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
#ifdef WIFI_MD_COEX_SUPPORT
	{CMDTHREAD_LTE_SAFE_CHN_CHG, LteSafeChannelChangeProcess},
#endif
	{CMDTHREAD_APCLI_MLO_CSA_SWITCH_CHANNEL, ApcliMloCsaSwitchChannelHandler},
#ifdef MT_DFS_SUPPORT
	{CMDTHREAD_DROP_RADAR_EVENT, DropRadarEventHandler},
#endif /* MT_DFS_SUPPORT */
#ifdef DOT11_EHT_BE
	{CMDTHREAD_MLD_RECONFIG_TIMEOUT, MldApReconfigHandler},
	{CMDTHREAD_MLD_AT2LM_TIMEOUT, MldApAt2lmToHandler},
#endif /* DOT11_EHT_BE */
	{CMDTHREAD_BSS_RECONF_SM, MBSS_Reconfig_SM_Handler},
	{CMDTHREAD_FORCE_SCAN_STOP, scan_stop_handle},
#ifdef ZERO_PKT_LOSS_SUPPORT
	{CMDTHREAD_LAST_BCN_TX_SWITCH_CHANNEL, LastBcnTxChannelSwitch},
	{CMDTHREAD_DISABLE_ZERO_LOSS_STA_TRAFFIC, DisableZeroLossStaTraffic},
#endif
	{CMDTHREAD_END_CMD_ID, NULL}
};

static inline CMDHdlr ValidCMD(IN PCmdQElmt CMDQelmt)
{
	SHORT CMDIndex = CMDQelmt->command;
	SHORT CurIndex = 0;
	USHORT CMDHdlrTableLength = sizeof(CMDHdlrTable) / sizeof(MT_CMD_TABL_T);
	CMDHdlr Handler = NULL;

	if (CMDIndex > CMDTHREAD_END_CMD_ID) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"CMD(%x) is out of boundary\n", CMDQelmt->command);
		return NULL;
	}

	for (CurIndex = 0; CurIndex < CMDHdlrTableLength; CurIndex++) {
		if (CMDHdlrTable[CurIndex].CmdID == CMDIndex) {
			Handler = CMDHdlrTable[CurIndex].CmdHdlr;
			break;
		}
	}

	if (Handler == NULL)
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
			"No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command);

	return Handler;
}


VOID CMDHandler(RTMP_ADAPTER *pAd)
{
	PCmdQElmt		cmdqelmt;
	NTSTATUS		ntStatus;
	CMDHdlr		Handler = NULL;
	UINT32		process_cnt = 0;
	UINT		cmdq_size = 0;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
			"pAd is NULL.\n");
		return;
	}
	NdisAcquireSpinLock(&pAd->CmdQLock);
	cmdq_size = pAd->CmdQ.size;
	NdisReleaseSpinLock(&pAd->CmdQLock);
	while (cmdq_size > 0) {

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"System halted, exit CMDHandler!(CmdQ.size = %d)\n",
				cmdq_size);
			break;
		}

		/* For worst case, avoid process CmdQ too long which cause RCU_sched stall */
		process_cnt++;
		if ((!in_interrupt()) && (process_cnt >= CMD_QUEUE_SCH)) {/*process_cnt-16*/
			process_cnt = 0;
			OS_SCHEDULE();
		}

		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		cmdq_size = pAd->CmdQ.size;
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (cmdqelmt == NULL)
			break;


		Handler = ValidCMD(cmdqelmt);

		if (Handler)
			ntStatus = Handler(pAd, cmdqelmt);

#ifdef DBG_STARVATION
		starv_dbg_put(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/

		if (cmdqelmt->CmdFromNdis == TRUE) {
			if (cmdqelmt->buffer != NULL)
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		} else {
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(cmdqelmt->buffer);

			os_free_mem(cmdqelmt);
		}
	}	/* end of while */
}

void RtmpCmdQExit(RTMP_ADAPTER *pAd)
{
	/* WCNCR00034259: unify CmdQ init and exit. But cleanup is done by
	 */
#ifdef DBG_STARVATION
	unregister_starv_block(&pAd->CmdQ.block);
#endif /*DBG_STARVATION*/
	return;
}

void RtmpCmdQInit(RTMP_ADAPTER *pAd)
{
	/* WCNCR00034259: moved from RTMP{Init, Alloc}TxRxRingMemory() */
	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);
	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);
#ifdef DBG_STARVATION
	cmdq_starv_block_init(&pAd->starv_log_ctrl, &pAd->CmdQ);
#endif /*DBG_STARVATION*/
}

/*
 * ========================================================================
 * Routine Description:
 *   Create kernel threads & tasklets.
 *
 * Arguments:
 *   *net_dev			Pointer to wireless net device interface
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS
 *	NDIS_STATUS_FAILURE
 * ========================================================================
 */
NDIS_STATUS	 RtmpMgmtTaskInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;
	char task_name[TASK_NAME_LEN] = {0};
	int ret = 0;
	u8 band_idx = hc_get_hw_band_idx(pAd);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"invalid band index(=%d)\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}

#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK)) {
		/* Creat TimerQ Thread, We need init timerQ related structure before create the timer thread. */
		RtmpTimerQInit(pAd);
		pTask = &pAd->timerTask;
		ret = snprintf(task_name, TASK_NAME_LEN,
			"RtmpTimerTask_%d%d", PD_GET_DEVICE_IDX(pAd->physical_dev), band_idx);
		if (os_snprintf_error(TASK_NAME_LEN, ret))
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"(line.%d) snprintf error!\n", __LINE__);
		MTWF_PRINT("%s(%d): task_name is %s\n", __func__, __LINE__, task_name);
		RTMP_OS_TASK_INIT(pTask, task_name, pAd);
		status = RtmpOSTaskAttach(pTask, RtmpTimerQThread, (ULONG)pTask);

		if (status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
					 KERN_WARNING "%s: unable to start RtmpTimerQThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /*RTMP_TIMER_TASK_SUPPORT*/
	/* WCNCR00034259: init CmdQ resources before run thread */
	RtmpCmdQInit(pAd);
	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	os_zero_mem(task_name, TASK_NAME_LEN);
	ret = snprintf(task_name, TASK_NAME_LEN,
		"RtmpCmdQTask_%d%d", PD_GET_DEVICE_IDX(pAd->physical_dev), band_idx);
	if (os_snprintf_error(TASK_NAME_LEN, ret))
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
			"(line.%d) snprintf error!\n", __LINE__);
	MTWF_PRINT("%s(%d): task_name is %s\n", __func__, __LINE__, task_name);
	RTMP_OS_TASK_INIT(pTask, task_name, pAd);
	status = RtmpOSTaskAttach(pTask, hif_cmd_thread, (ULONG)pTask);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				 KERN_WARNING "%s: unable to start RTUSBCmdThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
	/* start the crediential write task first. */
	status = WscThreadInit(pAd);
#endif /* WSC_INCLUDED */
	return status;
}



/*
 * ========================================================================
 * Routine Description:
 *   Close kernel threads.
 *
 * Arguments:
 *	*pAd				the raxx interface data pointer
 *
 * Return Value:
 *   NONE
 * ========================================================================
 */
VOID RtmpMgmtTaskExit(
	IN RTMP_ADAPTER *pAd)
{
	INT			ret;
	RTMP_OS_TASK	*pTask;

	MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_INFO, "\n pAd=%p\n", pAd);

	RtmpCmdQExit(pAd); /* WCNCR00034259: unify CmdQ init and exit */
	/* Terminate cmdQ thread */
	pTask = &pAd->cmdQTask;
	if (RTMP_OS_TASK_LEGALITY(pTask)) {
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);
		ret = RtmpOSTaskKill(pTask);

		if (ret == NDIS_STATUS_FAILURE)
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"kill command task failed!\n");

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	/* We need clear timerQ related structure before exits of the timer thread. */
#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK)) {
		RtmpTimerQExit(pAd);
		/* Terminate timer thread */
		pTask = &pAd->timerTask;
		ret = RtmpOSTaskKill(pTask);

		if (ret == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"kill timer task failed!\n");
		}
	}
#endif /*RTMP_TIMER_TASK_SUPPORT*/

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */
}

