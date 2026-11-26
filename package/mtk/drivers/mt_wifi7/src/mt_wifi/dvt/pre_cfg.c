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
    pre_cfg.c

    Abstract:
    This is pre cfg feature used to do DVT purpose.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef PRE_CFG_SUPPORT
#include "rt_config.h"

/*******************************************************************************
 * Global Parameters
 *******************************************************************************/

/*
 * These global arrays show precfg adctions and their handler function
 */

struct PRECFG_CMD ap_set_pre_cfg_tbl[] = {
	{"mld_link_add",	pre_cfg_add_peer_mld_sta_info},
	{"add",				pre_cfg_add_peer_sta},
	{"del",				pre_cfg_del_peer_sta},
	{"connect",			pre_cfg_connect_peer_sta},
	{"disconnect",		pre_cfg_disconnect_peer_sta},
};

struct PRECFG_CMD sta_set_pre_cfg_tbl[] = {
	{"mld_link_add",	pre_cfg_add_peer_mld_ap_info},
	{"add",				pre_cfg_add_peer_ap},
	{"del",				pre_cfg_del_peer_ap},
	{"connect",			pre_cfg_connect_peer_ap},
	{"disconnect",		pre_cfg_disconnect_peer_ap},
};

/*
 * This global array keeps parameters for fake negotiation
 * Format:
 *     {param Name, value, is config done}
 */

enum PRECFG_NEGO_PARA {
	PRECFG_NEGO_AID,
	PRECFG_NEGO_BW,
	PRECFG_NEGO_BAWIN,
	PRECFG_NEGO_NSS,
};

struct PRECFG_FAKE_NEGO_PARAM g_arPrecfgFakeNegoParam[] = {
	{"AID",		0, FALSE}, /* For struct _STA_ADMIN_CONFIG.StaActive.Aid (USHORT) */
	{"BW",		0, FALSE},
	{"BA_WIN",	0, FALSE},
	{"NSS",		0, FALSE},
};

/*******************************************************************************
 * Functions
 *******************************************************************************/


#ifdef WIFI_UNIFIED_COMMAND
VOID UniCmdPreCfgResultRsp(
	IN struct cmd_msg *msg,
	IN char *payload,
	IN UINT16 payload_len)
{
	struct UNI_EVENT_CMD_RESULT_T *UniCmdResult = (struct UNI_EVENT_CMD_RESULT_T *)payload;

	UniCmdResult->u2CID = le2cpu16(UniCmdResult->u2CID);
	UniCmdResult->u4Status = le2cpu32(UniCmdResult->u4Status);

	MTWF_PRINT("UniCmdResult.ucCID = 0x%x\n", UniCmdResult->u2CID);

	if (UniCmdResult->u4Status != 0 || (UniCmdResult->u2CID != msg->attr.type)) {
		MTWF_PRINT("BUG::UniCmdResult.u4Status = 0x%x cid: = 0x%x\n",
				UniCmdResult->u4Status, UniCmdResult->u2CID);
	} else {
		MTWF_PRINT("UniCmdResult.u4Status = 0x%x\n", UniCmdResult->u4Status);
	}
}
#endif /* WIFI_UNIFIED_COMMAND */


NTSTATUS PreCfgStaRecComm(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf)
{
	struct UNI_CMD_STAREC_INFO_T *pCmdStaRecUpdate = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pCmdStaRecUpdate = (struct UNI_CMD_STAREC_INFO_T *)(*pCmdBuf);
	pCmdStaRecUpdate->ucBssInfoIdx = pPreCfgEntry->StaRecCfg.ucBssIndex;
	WCID_SET_H_L(pCmdStaRecUpdate->ucWlanIdxHnVer,
		pCmdStaRecUpdate->ucWlanIdxL, pPreCfgEntry->StaRecCfg.u2WlanIdx);
	pCmdStaRecUpdate->ucMuarIdx = pPreCfgEntry->StaRecCfg.MuarIdx;
	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}
	(*pCmdBuf) += sizeof(struct UNI_CMD_STAREC_INFO_T);

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecSend(IN struct _RTMP_ADAPTER *pAd,
								IN UINT16 u2TLVTotalNumber,
								IN UCHAR * pCmdData,
								IN UINT32 CmdSize)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_STAREC_INFO_T *pCmdStaRecUpdate = (struct UNI_CMD_STAREC_INFO_T *)(pCmdData);

	pCmdStaRecUpdate->u2TotalElementNum = cpu2le16(u2TLVTotalNumber);
	pCmdStaRecUpdate->ucAppendCmdTLV = TRUE;
	Ret = UniCmdPreCfgSetCmdByRaw(pAd, HOST2CR4N9, UNI_CMD_ID_STAREC_INFO,
									pCmdData, CmdSize);

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return Ret;
}

VOID PreCfgRandomAmsduSend(IN struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl)
{
	struct UNI_CMD_STAREC_INFO_T *pCmdStaRecUpdate = (struct UNI_CMD_STAREC_INFO_T *)(pPreCfgTimerCmdCtrl->pCmdBuf);
	P_CMD_STAREC_AMSDU_T pCmdStaRecAmsdu = (P_CMD_STAREC_AMSDU_T)(pCmdStaRecUpdate->aucTlvBuffer);

	struct hdev_ctrl *ctrl = pPreCfgTimerCmdCtrl->pAd->hdev_ctrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ctrl);
	UINT8 max_amsdu_nums;
	UCHAR rand_num;

	max_amsdu_nums = cap->hw_max_amsdu_nums;
	rand_num = RandomByte(pPreCfgTimerCmdCtrl->pAd);
	pCmdStaRecAmsdu->ucMaxAmsduNum = (rand_num % max_amsdu_nums) + 1;
	pCmdStaRecAmsdu->ucMaxMpduSize = rand_num % 3;

	RTMP_SET_PRECFG_CMD(pPreCfgTimerCmdCtrl);
}

INT32 UniCmdPreCfgBandConfig(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgRadioOn,
	UINT8 ucDbdcIdx
)
{
	struct cmd_msg *msg;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4ComCmdSize = 0;
	UINT8  *pTempBuf = NULL;
	UINT8  *pNextHeadBuf = NULL;

	P_UNI_CMD_BAND_CONFIG_T pBandCfg;
	P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T pBandCfgRadioOnOff;

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_BAND_CONFIG_T);
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T);

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;
	pBandCfg = (P_UNI_CMD_BAND_CONFIG_T) pNextHeadBuf;
	pBandCfg->ucDbdcIdx = ucDbdcIdx;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 3: Filled in parameters of UNI_CMD_GET_SPECTRUM_DATA*/
	pBandCfgRadioOnOff = (P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T) pNextHeadBuf;
	pBandCfgRadioOnOff->u2Tag = cpu2le16(UNI_CMD_BAND_CONFIG_RADIO_ONOFF);
	pBandCfgRadioOnOff->u2Length = cpu2le16(sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T));
	pBandCfgRadioOnOff->fgRadioOn = fgRadioOn;

	/* Step 4: Send data packet*/
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BAND_CONFIG);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)pTempBuf, u4CmdNeedMaxBufSize);
	Ret = chip_cmd_tx(pAd, msg);

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(ret = %d)\n", Ret);
	return Ret;
}

VOID PreCfgRandomRadioOnOff(IN struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl)
{
	UCHAR rand_num;

	rand_num = RandomByte(pPreCfgTimerCmdCtrl->pAd);
	if ((rand_num%2) == 1) {
		UniCmdPreCfgBandConfig(pPreCfgTimerCmdCtrl->pAd, TRUE,
			HcGetBandByChannel(pPreCfgTimerCmdCtrl->pAd,
			pPreCfgTimerCmdCtrl->pPreCfgEntry->pMacEntry->wdev->channel));
		MTWF_DBG(pPreCfgTimerCmdCtrl->pAd, DBG_CAT_TEST, CATTEST_PRECFG,
			DBG_LVL_ERROR, "==>(ON)\n");
	} else {
		UniCmdPreCfgBandConfig(pPreCfgTimerCmdCtrl->pAd, FALSE,
			HcGetBandByChannel(pPreCfgTimerCmdCtrl->pAd,
			pPreCfgTimerCmdCtrl->pPreCfgEntry->pMacEntry->wdev->channel));
		MTWF_DBG(pPreCfgTimerCmdCtrl->pAd, DBG_CAT_TEST, CATTEST_PRECFG,
			DBG_LVL_ERROR, "==> (OFF)\n");
	}
}

VOID PreCfgTimerSend(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl = (struct PRECFG_TIMER_CMD_CTRL *) FunctionContext;

	pPreCfgTimerCmdCtrl->pfTimerHandler(pPreCfgTimerCmdCtrl);
}

DECLARE_TIMER_FUNCTION(PreCfgTimerSend);
BUILD_TIMER_FUNCTION(PreCfgTimerSend);

NTSTATUS PreCfgStaRecBasicPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecBasicUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	P_CMD_STAREC_COMMON_T pStaRecCommon = (P_CMD_STAREC_COMMON_T)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	pStaRecCommon->u2Tag = UNI_CMD_STAREC_BASIC;
	pStaRecCommon->u2Length = sizeof(CMD_STAREC_COMMON_T);
	pStaRecCommon->u4ConnectionType = cpu2le32(pStaRecCfg->ConnectionType);
	pStaRecCommon->ucConnectionState = pStaRecCfg->ConnectionState;
	pStaRecCommon->u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2;
	pStaRecCommon->u2AID = cpu2le16(pMacEntry->Aid);
	os_move_mem(pStaRecCommon->aucPeerMacAddr, pMacEntry->Addr, MAC_ADDR_LEN);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_BASIC (PRE_CFG_UNI_CMD_STAREC_BASIC)\n");

	if (pStaRecCfg->IsNewSTARec)
		pStaRecCommon->u2ExtraInfo |= STAREC_COMMON_EXTRAINFO_NEWSTAREC;
	pStaRecCommon->u2ExtraInfo = cpu2le16(pStaRecCommon->u2ExtraInfo);

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		pStaRecCommon->ucIsQBSS =
			CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE) ? TRUE : FALSE;
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/*
		EDCA_PARM *pEdca = hwifi_get_edca(pAd, pMacEntry->wdev);

		if (pEdca)
			pStaRecCommon->ucIsQBSS = pEdca->bValid;
		*/
		pStaRecCommon->ucIsQBSS =
			CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE) ?
				TRUE : FALSE;
	}
#ifdef CFG_BIG_ENDIAN
	pStaRecCommon->u2Tag = cpu2le16(pStaRecCommon->u2Tag);
	pStaRecCommon->u2Length = cpu2le16(pStaRecCommon->u2Length);
#endif

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(CMD_STAREC_COMMON_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s: aucPeerMacAddr = ("MACSTR")!\n", __func__, MAC2STR(pStaRecCommon->aucPeerMacAddr));
	MTWF_PRINT("%s: (CMD_STAREC_COMMON_T), u4ConnectionType = %d, ucConnectionState = %d, ucIsQBSS = %d, u2AID = %d\n",
			__func__,  le2cpu32(pStaRecCommon->u4ConnectionType),  pStaRecCommon->ucConnectionState,
			pStaRecCommon->ucIsQBSS, le2cpu16(pStaRecCommon->u2AID));

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}


NTSTATUS PreCfgStaRecWtblPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecWtblUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	UCHAR *pNext = (UCHAR *)(*pCmdBuf);
	struct UNI_CMD_STAREC_AADOM_T *pAADOM = NULL;
	P_UNI_CMD_STAREC_PHY_INFO_T pPhyInfo = NULL;
	struct UNI_CMD_STAREC_HDRT_T *pHDRT = NULL;
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pAADOM = (struct UNI_CMD_STAREC_AADOM_T *)pNext;
	pNext += sizeof(*pAADOM);
	pHDRT = (struct UNI_CMD_STAREC_HDRT_T *)pNext;
	pNext += sizeof(*pHDRT);
	pPhyInfo = (P_UNI_CMD_STAREC_PHY_INFO_T)pNext;
	pNext += sizeof(*pPhyInfo);
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	pAADOM->u2Tag = UNI_CMD_STAREC_AAD_OM;
	pAADOM->u2Length = sizeof(struct UNI_CMD_STAREC_AADOM_T);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_AAD_OM (PRE_CFG_UNI_CMD_STAREC_AAD_OM)\n");

	if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)
		&& CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET))
		pAADOM->ucAadOm = 1;

	pHDRT->u2Tag = UNI_CMD_STAREC_HDRT;
	pHDRT->u2Length = sizeof(struct UNI_CMD_STAREC_HDRT_T);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_HDRT (PRE_CFG_UNI_CMD_STAREC_HDRT)\n");

	if (!IS_BELLWETHER(pAd))
		pHDRT->ucHdrtMode = 1;
	else
		pHDRT->ucHdrtMode = 0;

	pPhyInfo->u2Tag = UNI_CMD_STAREC_PHY_INFO;
	pPhyInfo->u2Length = sizeof(UNI_CMD_STAREC_PHY_INFO_T);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_PHY_INFO (PRE_CFG_UNI_CMD_STAREC_PHY_INFO)\n");

	pPhyInfo->ucAmpduParam = ((pMacEntry->MpduDensity & 0x7) << 2);
	if (IS_VHT_STA(pMacEntry)) {
		if (pMacEntry->MaxRAmpduFactor >= 0x3)
			pPhyInfo->ucAmpduParam |= 0x3;
		else
			pPhyInfo->ucAmpduParam |= (pMacEntry->MaxRAmpduFactor & 0x3);
		pPhyInfo->ucVhtMaxAmpduLen = (pMacEntry->MaxRAmpduFactor & 0x7);
	} else {
		pPhyInfo->ucAmpduParam |= (pMacEntry->MaxRAmpduFactor & 0x3);
		pPhyInfo->ucVhtMaxAmpduLen = 0;
	}

#ifdef CFG_BIG_ENDIAN
	pAADOM->u2Tag = cpu2le16(pAADOM->u2Tag);
	pAADOM->u2Length = cpu2le16(pAADOM->u2Length);
	pHDRT->u2Tag = cpu2le16(pHDRT->u2Tag);
	pHDRT->u2Length = cpu2le16(pHDRT->u2Length);
	pPhyInfo->u2Tag = cpu2le16(pPhyInfo->u2Tag);
	pPhyInfo->u2Length = cpu2le16(pPhyInfo->u2Length);
#endif

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += (sizeof(struct UNI_CMD_STAREC_AADOM_T) +
				   sizeof(struct UNI_CMD_STAREC_HDRT_T) +
				   sizeof(UNI_CMD_STAREC_PHY_INFO_T));
	*u2TLVNumber += 3;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHwAmsduPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHwAmsduUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	P_CMD_STAREC_AMSDU_T pCmdStaRecAmsdu = (P_CMD_STAREC_AMSDU_T)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	pCmdStaRecAmsdu->u2Tag = UNI_CMD_STAREC_HW_AMSDU;
	pCmdStaRecAmsdu->u2Length = sizeof(CMD_STAREC_AMSDU_T);
	pCmdStaRecAmsdu->ucMaxMpduSize = pMacEntry->AMsduSize;
	pCmdStaRecAmsdu->ucMaxAmsduNum = cap->hw_max_amsdu_nums;
	pCmdStaRecAmsdu->ucAmsduEnable = TRUE;
#ifdef CFG_BIG_ENDIAN
	pCmdStaRecAmsdu->u2Tag = cpu2le16(pCmdStaRecAmsdu->u2Tag);
	pCmdStaRecAmsdu->u2Length = cpu2le16(pCmdStaRecAmsdu->u2Length);
#endif
	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(CMD_STAREC_AMSDU_T);
	*u2TLVNumber += 1;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_HW_AMSDU (PRE_CFG_UNI_CMD_STAREC_HW_AMSDU)\n");

	MTWF_PRINT("%s: ucMaxMpduSize = %d, ucMaxAmsduNum = %d\n", __func__,
			pCmdStaRecAmsdu->ucMaxMpduSize, pCmdStaRecAmsdu->ucMaxAmsduNum);

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHtBasicPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		*((UINT16 *)&pMacEntry->HTCapability.HtCapInfo) = 0x9ef;
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		*((UINT16 *)&pMacEntry->HTCapability.HtCapInfo) = 0x9ef;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHtBasicUpdate(IN struct _RTMP_ADAPTER *pAd,
											IN struct PRECFG_CTRL *pPreCfgCtrl,
											IN struct PRECFG_ENTRY *pPreCfgEntry,
											IN BOOLEAN fgConnect,
											IN UCHAR **pCmdBuf,
											OUT UINT16 *u2TLVNumber)
{
	P_CMD_STAREC_HT_INFO_T pCmdStaRecHtInfo = (P_CMD_STAREC_HT_INFO_T)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	pCmdStaRecHtInfo->u2Tag = UNI_CMD_STAREC_HT_BASIC;
	pCmdStaRecHtInfo->u2Length = sizeof(CMD_STAREC_HT_INFO_T);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_HT_BASIC (PRE_CFG_UNI_CMD_STAREC_HT_BASIC)\n");

	if (sizeof(pCmdStaRecHtInfo->u2HtCap) >= sizeof(pMacEntry->HTCapability.HtCapInfo))
		os_move_mem(&(pCmdStaRecHtInfo->u2HtCap),
					&(pMacEntry->HTCapability.HtCapInfo),
					sizeof(pMacEntry->HTCapability.HtCapInfo));
	else
		os_move_mem(&(pCmdStaRecHtInfo->u2HtCap),
					&(pMacEntry->HTCapability.HtCapInfo),
					sizeof(pCmdStaRecHtInfo->u2HtCap));

#ifdef CFG_BIG_ENDIAN
	pCmdStaRecHtInfo->u2Tag = cpu2le16(pCmdStaRecHtInfo->u2Tag);
	pCmdStaRecHtInfo->u2Length = cpu2le16(pCmdStaRecHtInfo->u2Length);
	pCmdStaRecHtInfo->u2HtCap = cpu2le16(pCmdStaRecHtInfo->u2HtCap);
#endif
	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(CMD_STAREC_HT_INFO_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s: u2HtCap = 0x%x\n", __func__, pCmdStaRecHtInfo->u2HtCap);
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecVhtBasicPrepare(IN struct _RTMP_ADAPTER *pAd,
											IN struct PRECFG_CTRL *pPreCfgCtrl,
											IN struct PRECFG_ENTRY *pPreCfgEntry,
											IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		*((UINT32 *)&pMacEntry->vht_cap_ie.vht_cap) = 0x33c001b1;
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		*((UINT32 *)&pMacEntry->vht_cap_ie.vht_cap) = 0x33c001b1;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecVhtBasicUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	P_CMD_STAREC_VHT_INFO_T pCmdStaRecVHtInfo = (P_CMD_STAREC_VHT_INFO_T)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	pCmdStaRecVHtInfo->u2Tag = UNI_CMD_STAREC_VHT_BASIC;
	pCmdStaRecVHtInfo->u2Length = sizeof(CMD_STAREC_VHT_INFO_T);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_VHT_BASIC (PRE_CFG_UNI_CMD_STAREC_VHT_BASIC)\n");

	/* FIXME: may need separate function to compose the payload */
	if (sizeof(pCmdStaRecVHtInfo->u4VhtCap) >= sizeof(pMacEntry->vht_cap_ie.vht_cap))
		os_move_mem(&(pCmdStaRecVHtInfo->u4VhtCap),
				&(pMacEntry->vht_cap_ie.vht_cap),
				sizeof(pMacEntry->vht_cap_ie.vht_cap));
	else
		os_move_mem(&(pCmdStaRecVHtInfo->u4VhtCap),
				&(pMacEntry->vht_cap_ie.vht_cap),
				sizeof(pCmdStaRecVHtInfo->u4VhtCap));
	if (sizeof(pCmdStaRecVHtInfo->u2VhtRxMcsMap) >= sizeof(pMacEntry->vht_cap_ie.mcs_set.rx_mcs_map))
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtRxMcsMap),
				&(pMacEntry->vht_cap_ie.mcs_set.rx_mcs_map),
				sizeof(pMacEntry->vht_cap_ie.mcs_set.rx_mcs_map));
	else
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtRxMcsMap),
				&(pMacEntry->vht_cap_ie.mcs_set.rx_mcs_map),
				sizeof(pCmdStaRecVHtInfo->u2VhtRxMcsMap));
	if (sizeof(pCmdStaRecVHtInfo->u2VhtTxMcsMap) >= sizeof(pMacEntry->vht_cap_ie.mcs_set.tx_mcs_map))
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtTxMcsMap),
				&(pMacEntry->vht_cap_ie.mcs_set.tx_mcs_map),
				sizeof(pMacEntry->vht_cap_ie.mcs_set.tx_mcs_map));
	else
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtTxMcsMap),
				&(pMacEntry->vht_cap_ie.mcs_set.tx_mcs_map),
				sizeof(pCmdStaRecVHtInfo->u2VhtTxMcsMap));

	if (IS_VHT_STA(pMacEntry) && !IS_HE_2G_STA(pMacEntry->cap.modes)) {
		UCHAR ucRTSBWSig = wlan_config_get_vht_bw_sig(pMacEntry->wdev);

		/* for StaRec: 0-disable DynBW, 1-static BW, 2 Dynamic BW */
		pCmdStaRecVHtInfo->ucRTSBWSig = ucRTSBWSig;
	}
	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(CMD_STAREC_VHT_INFO_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s: u4VhtCap = 0x%x, u2VhtRxMcsMap = 0x%x, u2VhtTxMcsMap = 0x%x\n",
			__func__, pCmdStaRecVHtInfo->u4VhtCap, pCmdStaRecVHtInfo->u2VhtRxMcsMap,
			pCmdStaRecVHtInfo->u2VhtTxMcsMap);
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHeBasicPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		pMacEntry->aucHeMacCapInfo[0] = 0x3;
		pMacEntry->aucHeMacCapInfo[1] = 0x8;
		pMacEntry->aucHeMacCapInfo[2] = 0x0;
		pMacEntry->aucHeMacCapInfo[3] = 0x1a;
		pMacEntry->aucHeMacCapInfo[4] = 0x3;
		pMacEntry->aucHeMacCapInfo[5] = 0x8;

		pMacEntry->aucHePhyCapInfo[0] = 0x4;
		pMacEntry->aucHePhyCapInfo[1] = 0x70;
		pMacEntry->aucHePhyCapInfo[2] = 0xce;
		pMacEntry->aucHePhyCapInfo[3] = 0x12;
		pMacEntry->aucHePhyCapInfo[4] = 0x6d;
		pMacEntry->aucHePhyCapInfo[5] = 0x0;
		pMacEntry->aucHePhyCapInfo[6] = 0xf3;
		pMacEntry->aucHePhyCapInfo[7] = 0x16;
		pMacEntry->aucHePhyCapInfo[8] = 0x11;
		pMacEntry->aucHePhyCapInfo[9] = 0x7f;
		pMacEntry->aucHePhyCapInfo[10] = 0x0;
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		pMacEntry->aucHeMacCapInfo[0] = 0x5;
		pMacEntry->aucHeMacCapInfo[1] = 0x0;
		pMacEntry->aucHeMacCapInfo[2] = 0x8;
		pMacEntry->aucHeMacCapInfo[3] = 0x1a;
		pMacEntry->aucHeMacCapInfo[4] = 0x5;
		pMacEntry->aucHeMacCapInfo[5] = 0x0;

		pMacEntry->aucHePhyCapInfo[0] = 0x4;
		pMacEntry->aucHePhyCapInfo[1] = 0x20;
		pMacEntry->aucHePhyCapInfo[2] = 0xe;
		pMacEntry->aucHePhyCapInfo[3] = 0x92;
		pMacEntry->aucHePhyCapInfo[4] = 0x6f;
		pMacEntry->aucHePhyCapInfo[5] = 0x1b;
		pMacEntry->aucHePhyCapInfo[6] = 0xaf;
		pMacEntry->aucHePhyCapInfo[7] = 0x10;
		pMacEntry->aucHePhyCapInfo[8] = 0x11;
		pMacEntry->aucHePhyCapInfo[9] = 0xc;
		pMacEntry->aucHePhyCapInfo[10] = 0x0;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecHeBasicUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	UINT32 i;
	struct CMD_STAREC_HE_BASIC_T *he_basic = (struct CMD_STAREC_HE_BASIC_T *)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	he_basic->u2Tag = UNI_CMD_STAREC_HE_BASIC;
	he_basic->u2Length = sizeof(struct CMD_STAREC_HE_BASIC_T);
	/*mac info*/
	if (sizeof(he_basic->aucHeMacCapInfo) >= sizeof(pMacEntry->aucHeMacCapInfo))
		os_move_mem(&he_basic->aucHeMacCapInfo[0], &pMacEntry->aucHeMacCapInfo[0],
					sizeof(pMacEntry->aucHeMacCapInfo));
	else
		os_move_mem(&he_basic->aucHeMacCapInfo[0], &pMacEntry->aucHeMacCapInfo[0],
					sizeof(he_basic->aucHeMacCapInfo));

	/*phy info*/
	if (sizeof(he_basic->aucHePhyCapInfo) >= sizeof(pMacEntry->aucHePhyCapInfo))
		os_move_mem(&he_basic->aucHePhyCapInfo[0], &pMacEntry->aucHePhyCapInfo[0],
					sizeof(pMacEntry->aucHePhyCapInfo));
	else
		os_move_mem(&he_basic->aucHePhyCapInfo[0], &pMacEntry->aucHePhyCapInfo[0],
					sizeof(he_basic->aucHePhyCapInfo));

	/* packet extension */
	he_basic->ucPktExt = 2;	/* force Packet Extension as 16 us by default */

	/*MAX NSS MCS*/
	for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
		he_basic->au2RxMaxNssMcs[CMD_HE_MCS_BW80] |= (pMacEntry->cap.rate.he80_rx_nss_mcs[i] << (i * 2));
		he_basic->au2RxMaxNssMcs[CMD_HE_MCS_BW160] |= (pMacEntry->cap.rate.he160_rx_nss_mcs[i] << (i * 2));
		he_basic->au2RxMaxNssMcs[CMD_HE_MCS_BW8080] |= (pMacEntry->cap.rate.he8080_rx_nss_mcs[i] << (i * 2));
	}

#ifdef CFG_BIG_ENDIAN
	he_basic->u2Tag = cpu2le16(he_basic->u2Tag);
	he_basic->u2Length = cpu2le16(he_basic->u2Length);
	for (i = 0 ; i < CMD_HE_MCS_BW_NUM ; i++)
		he_basic->au2RxMaxNssMcs[i] = cpu2le16(he_basic->au2RxMaxNssMcs[i]);
#endif

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(struct CMD_STAREC_HE_BASIC_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecEhtBasicPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		pMacEntry->eht_mac_cap = 0x6;
		pMacEntry->eht_phy_cap = 0xc2128fe000dc8;
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		pMacEntry->eht_mac_cap = 0x3;
		pMacEntry->eht_phy_cap = 0xc6120fe1b0de8;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecEhtBasicUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct CMD_STAREC_EHT_BASIC_T *pCmdStaRecEthBasic =
								(struct CMD_STAREC_EHT_BASIC_T *)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	/* Fill TLV format */
	pCmdStaRecEthBasic->u2Tag = UNI_CMD_STAREC_EHT_BASIC;
	pCmdStaRecEthBasic->u2Length = sizeof(struct CMD_STAREC_EHT_BASIC_T);
	pCmdStaRecEthBasic->ucTidBitmap = 0xFF;
	pCmdStaRecEthBasic->u2EhtMacCap = pMacEntry->eht_mac_cap;
	pCmdStaRecEthBasic->u8EhtPhyCap = pMacEntry->eht_phy_cap;
	pCmdStaRecEthBasic->u8EhtPhyCapExt = pMacEntry->eht_phy_cap_ext;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_EHT_BASIC (PRE_CFG_UNI_CMD_STAREC_EHT_BASIC)\n");
	if (pMacEntry->MaxHTPhyMode.field.BW == BW_20) {
		if (sizeof(pCmdStaRecEthBasic->aucMscMap20MHzSta) >= sizeof(struct eht_txrx_mcs_nss_20))
			NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap20MHzSta,
							(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only,
							sizeof(struct eht_txrx_mcs_nss_20));
		else
			NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap20MHzSta,
							(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only,
							sizeof(pCmdStaRecEthBasic->aucMscMap20MHzSta));
	} else {
		if (pMacEntry->MaxHTPhyMode.field.BW > BW_20) {
			if (sizeof(pCmdStaRecEthBasic->aucMscMap80MHz) >= sizeof(struct eht_txrx_mcs_nss))
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap80MHz,
							(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[0],
							sizeof(struct eht_txrx_mcs_nss));
			else
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap80MHz,
							(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[0],
							sizeof(pCmdStaRecEthBasic->aucMscMap80MHz));
		}
		if (pMacEntry->MaxHTPhyMode.field.BW >= BW_160) {
			if (sizeof(pCmdStaRecEthBasic->aucMscMap160MHz) >= sizeof(struct eht_txrx_mcs_nss))
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap160MHz,
								(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[1],
								sizeof(struct eht_txrx_mcs_nss));
			else
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap160MHz,
								(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[1],
								sizeof(pCmdStaRecEthBasic->aucMscMap160MHz));
		}
		if (pMacEntry->MaxHTPhyMode.field.BW == BW_320) {
			if (sizeof(pCmdStaRecEthBasic->aucMscMap320MHz) >= sizeof(struct eht_txrx_mcs_nss))
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap320MHz,
								(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[2],
								sizeof(struct eht_txrx_mcs_nss));
			else
				NdisMoveMemory((UINT8 *)pCmdStaRecEthBasic->aucMscMap320MHz,
								(UINT8 *)&pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[2],
								sizeof(pCmdStaRecEthBasic->aucMscMap320MHz));
		}
	}
#ifdef CFG_BIG_ENDIAN
	pCmdStaRecEthBasic->u2Tag = cpu2le16(pCmdStaRecEthBasic->u2Tag);
	pCmdStaRecEthBasic->u2Length = cpu2le16(pCmdStaRecEthBasic->u2Length);
#endif

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(struct CMD_STAREC_EHT_BASIC_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s: ucTidBitmap = 0x%x, u2EhtMacCap = 0x%x, u8EhtPhyCap = 0x%llx\n",
			__func__, pCmdStaRecEthBasic->ucTidBitmap, pCmdStaRecEthBasic->u2EhtMacCap,
			pCmdStaRecEthBasic->u8EhtPhyCap);
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMldTeardownPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMldTeardownUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct CMD_STAREC_MLD_TEARDOWN_T *pCmdStaRecMldTeardown =
								(struct CMD_STAREC_MLD_TEARDOWN_T *)(*pCmdBuf);

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pCmdStaRecMldTeardown->u2Tag = UNI_CMD_STAREC_MLD_TEARDOWN;
	pCmdStaRecMldTeardown->u2Length = sizeof(struct CMD_STAREC_MLD_TEARDOWN_T);
#ifdef CFG_BIG_ENDIAN
	pCmdStaRecMldTeardown->u2Tag = cpu2le16(pCmdStaRecMldTeardown->u2Tag);
	pCmdStaRecMldTeardown->u2Length = cpu2le16(pCmdStaRecMldTeardown->u2Length);
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"=> UNI_CMD_STAREC_MLD_TEARDOWN (PRE_CFG_UNI_CMD_STAREC_MLD_TEARDOWN)\n");

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(struct CMD_STAREC_MLD_TEARDOWN_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}


NTSTATUS PreCfgStaRecRaPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	P_RA_ENTRY_INFO_T pRaEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;
	pRaEntry = &pMacEntry->RaEntry;
	os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T));

	pRaEntry->u2Wcid = pMacEntry->wcid;
	pRaEntry->fgAutoTxRateSwitch = pMacEntry->bAutoTxRateSwitch;
	pRaEntry->ucPhyMode = pMacEntry->wdev->PhyMode;
	pRaEntry->ucChannel = pMacEntry->wdev->channel;
	/* use the maximum bw capability */
	pRaEntry->ucBBPCurrentBW = HcGetBw(pAd, pMacEntry->wdev);
	pRaEntry->fgDisableCCK = FALSE;
	pRaEntry->fgHtCapMcs32 = (pMacEntry->HTCapability.MCSSet[4] & 0x1) ? TRUE : FALSE;
	pRaEntry->fgHtCapInfoGF = pMacEntry->HTCapability.HtCapInfo.GF;
	pRaEntry->aucHtCapMCSSet[0] = pMacEntry->HTCapability.MCSSet[0];
	pRaEntry->aucHtCapMCSSet[1] = pMacEntry->HTCapability.MCSSet[1];
	pRaEntry->aucHtCapMCSSet[2] = pMacEntry->HTCapability.MCSSet[2];
	pRaEntry->aucHtCapMCSSet[3] = pMacEntry->HTCapability.MCSSet[3];
	pRaEntry->ucMmpsMode = pMacEntry->MmpsMode;
	pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_ENABLE;
	pRaEntry->ucMaxAmpduFactor = pMacEntry->MaxRAmpduFactor;
	pRaEntry->RateLen = pMacEntry->RateLen;
	pRaEntry->ucSupportRateMode = pMacEntry->SupportRateMode;
	pRaEntry->ucSupportCCKMCS = pMacEntry->SupportCCKMCS;
	pRaEntry->ucSupportOFDMMCS = pMacEntry->SupportOFDMMCS;
	pRaEntry->u4SupportHTMCS = pMacEntry->SupportHTMCS;
	pRaEntry->u2SupportVHTMCS1SS = pMacEntry->SupportVHTMCS1SS;
	pRaEntry->u2SupportVHTMCS2SS = pMacEntry->SupportVHTMCS2SS;
	pRaEntry->u2SupportVHTMCS3SS = pMacEntry->SupportVHTMCS3SS;
	pRaEntry->u2SupportVHTMCS4SS = pMacEntry->SupportVHTMCS4SS;
	pRaEntry->force_op_mode = pMacEntry->force_op_mode;
	pRaEntry->vhtOpModeChWidth = pMacEntry->operating_mode.ch_width;
	pRaEntry->vhtOpModeRxNss = pMacEntry->operating_mode.rx_nss;
	pRaEntry->vhtOpModeRxNssType = pMacEntry->operating_mode.rx_nss_type;
	pRaEntry->AvgRssiSample[0] = pMacEntry->RssiSample.AvgRssi[0];
	pRaEntry->AvgRssiSample[1] = pMacEntry->RssiSample.AvgRssi[1];
	pRaEntry->AvgRssiSample[2] = pMacEntry->RssiSample.AvgRssi[2];
	pRaEntry->AvgRssiSample[3] = pMacEntry->RssiSample.AvgRssi[3];
	pRaEntry->fgAuthWapiMode = FALSE;
	pRaEntry->ClientStatusFlags = pMacEntry->ClientStatusFlags;
	pRaEntry->MaxPhyCfg.MODE = pMacEntry->MaxHTPhyMode.field.MODE;
	pRaEntry->MaxPhyCfg.STBC = pMacEntry->MaxHTPhyMode.field.STBC;
	pRaEntry->MaxPhyCfg.ShortGI = pMacEntry->MaxHTPhyMode.field.ShortGI;
	pRaEntry->MaxPhyCfg.BW = pMacEntry->MaxHTPhyMode.field.BW;
	pMacEntry->MaxHTPhyMode.field.ldpc = pMacEntry->operating_mode.no_ldpc ? 0 : 1;
	pRaEntry->MaxPhyCfg.ldpc = pMacEntry->MaxHTPhyMode.field.ldpc;
	if (pRaEntry->MaxPhyCfg.MODE >= MODE_VHT) {
		pRaEntry->MaxPhyCfg.MCS = pMacEntry->MaxHTPhyMode.field.MCS & 0xf;
		pRaEntry->MaxPhyCfg.VhtNss = ((pMacEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	} else {
		pRaEntry->MaxPhyCfg.MCS = pMacEntry->MaxHTPhyMode.field.MCS;
		pRaEntry->MaxPhyCfg.VhtNss = 0;
	}
	pRaEntry->TxPhyCfg.MODE = pMacEntry->HTPhyMode.field.MODE;
	pRaEntry->TxPhyCfg.STBC = pMacEntry->HTPhyMode.field.STBC;
	pRaEntry->TxPhyCfg.ShortGI = pMacEntry->HTPhyMode.field.ShortGI;
	pRaEntry->TxPhyCfg.BW = pMacEntry->HTPhyMode.field.BW;
	pRaEntry->TxPhyCfg.ldpc = pMacEntry->HTPhyMode.field.ldpc;
	if (pRaEntry->TxPhyCfg.MODE >= MODE_VHT) {
		pRaEntry->TxPhyCfg.MCS = pMacEntry->HTPhyMode.field.MCS & 0xf;
		pRaEntry->TxPhyCfg.VhtNss = ((pMacEntry->HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	} else {
		pRaEntry->TxPhyCfg.MCS = pMacEntry->HTPhyMode.field.MCS;
		pRaEntry->TxPhyCfg.VhtNss = 0;
	}

	if (IS_HE_6G_STA(pMacEntry->cap.modes)) {
		struct _RA_PHY_CFG_T *tx_phy_cfg = &pRaEntry->TxPhyCfg;
		struct _RA_PHY_CFG_T *max_phy_cfg = &pRaEntry->MaxPhyCfg;
		struct caps_info *cap = &pMacEntry->cap;

		/*phy conifugration*/
		tx_phy_cfg->STBC = 1;
		tx_phy_cfg->ShortGI = 1.;
		tx_phy_cfg->ldpc = cap->he_phy_cap & HE_LDPC ? 1 : 0;
		tx_phy_cfg->MCS = 13;

		max_phy_cfg->STBC = 1;
		max_phy_cfg->ShortGI = 1;
		max_phy_cfg->ldpc = cap->he_phy_cap & HE_LDPC ? 1 : 0;
		max_phy_cfg->MCS = 13;
		/*update smps*/
		pRaEntry->ucMmpsMode = pMacEntry->MmpsMode;
	}
	pRaEntry->TxPhyCfg.BW = pRaEntry->ucBBPCurrentBW < pMacEntry->cap.ch_bw.he_ch_width ?
						pRaEntry->ucBBPCurrentBW : pMacEntry->cap.ch_bw.he_ch_width;
	pRaEntry->MaxPhyCfg.BW = pRaEntry->ucBBPCurrentBW < pMacEntry->cap.ch_bw.he_ch_width ?
						pRaEntry->ucBBPCurrentBW : pMacEntry->cap.ch_bw.he_ch_width;
	pRaEntry->fgRaValid = TRUE;
	raWrapperEntryRestore(pAd, pMacEntry, pRaEntry);

	if (IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		/* Nothing */
	} else if (IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		/* Nothing */
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecRaUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct _STAREC_AUTO_RATE_T *pCmdStaRecAutoRate =
								(struct _STAREC_AUTO_RATE_T *)(*pCmdBuf);
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (pMacEntry) {
		StaRecAutoRateParamSet(&pMacEntry->RaEntry, pCmdStaRecAutoRate);
		pCmdStaRecAutoRate->u2Tag = UNI_CMD_STAREC_RA;
		pCmdStaRecAutoRate->u2Length = sizeof(struct _STAREC_AUTO_RATE_T);
#ifdef CFG_BIG_ENDIAN
		pCmdStaRecAutoRate->u2Tag = cpu2le16(pCmdStaRecAutoRate->u2Tag);
		pCmdStaRecAutoRate->u2Length = cpu2le16(pCmdStaRecAutoRate->u2Length);
#endif
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"=> UNI_CMD_STAREC_RA (PRE_CFG_UNI_CMD_STAREC_RA)\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(struct _STAREC_AUTO_RATE_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMuruInfoPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMuruInfoUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (pMacEntry) {
		UniCmdStaRecUpdateMuruInfo(pAd, pStaRecCfg, NULL, (void *)(*pCmdBuf));
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(CMD_STAREC_MURU_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMldSetupPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecMldSetupUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct CMD_STAREC_MLD_SETUP_T *pCmdStaRecMldSetup =
		(struct CMD_STAREC_MLD_SETUP_T *)(*pCmdBuf);

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (pMacEntry) {
		UniCmdStaRecMldSetup(pAd, pStaRecCfg, NULL, (void *)(*pCmdBuf));
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += pCmdStaRecMldSetup->u2Length;
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecEhtMldPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecEhtMldUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (pMacEntry) {
		UniCmdStaRecEhtMld(pAd, pStaRecCfg, NULL, (void *)(*pCmdBuf));
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Moved to next & Add TLV Number */
	(*pCmdBuf) += sizeof(struct CMD_STAREC_EHT_MLD_T);
	*u2TLVNumber += 1;

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecInstallKeyPrepare(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_VALID_ENTRY(pMacEntry)) {
		pMacEntry->SecConfig.PairwiseKeyId = 0;
		pMacEntry->SecConfig.GroupKeyId = 1;
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfgStaRecInstallKeyUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect,
	IN UCHAR **pCmdBuf,
	OUT UINT16 *u2TLVNumber)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_VALID_ENTRY(pMacEntry)) {
		ASIC_SEC_INFO Ptk_Info = {0}, Gtk_Info = {0};
		UINT32 encryMode;

		encryMode = GET_PAIRWISE_CIPHER(&pMacEntry->SecConfig);
		os_zero_mem(&Ptk_Info, sizeof(ASIC_SEC_INFO));
		os_zero_mem(&Gtk_Info, sizeof(ASIC_SEC_INFO));

		if (IS_CIPHER_WEP(encryMode)) {
			/* APCLI set key */
			UINT_8 au1KeyPatternPtk[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
			UINT_8 au1KeyPatternGtk[5] = {0x99, 0x99, 0x99, 0x99, 0x99};

			os_move_mem(Ptk_Info.Key.Key, &au1KeyPatternPtk[0], LEN_WEP40);
			os_move_mem(Gtk_Info.Key.Key, &au1KeyPatternGtk[0], LEN_WEP40);
			Ptk_Info.Key.KeyLen = LEN_WEP40;
			Gtk_Info.Key.KeyLen = LEN_WEP40;
		} else {
			/* APCLI set key */
			UINT_8 au1KeyPatternPtk[32] = {
				0xae, 0xaf, 0xa1, 0xee,
				0xe9, 0xc6, 0xb2, 0x9c,
				0x9c, 0xe7, 0x91, 0x82,
				0xde, 0x5f, 0x4b, 0x15,
				0x11, 0x11, 0x11, 0x11,
				0x11, 0x11, 0x11, 0x11,
				0x11, 0x11, 0x11, 0x11,
				0x11, 0x11, 0x11, 0x11,
			};

			UINT_8 au1KeyPatternGtk[32] = {
				0xf7, 0xe6, 0x70, 0xeb,
				0x31, 0x74, 0x46, 0x7f,
				0xdb, 0xcb, 0x8f, 0xab,
				0x3f, 0x55, 0xe2, 0x4b,
				0x99, 0x99, 0x99, 0x99,
				0x99, 0x99, 0x99, 0x99,
				0x99, 0x99, 0x99, 0x99,
				0x99, 0x99, 0x99, 0x99,
			};
			os_move_mem(Ptk_Info.Key.Key, &au1KeyPatternPtk[0], (LEN_TK + LEN_TK2));
			os_move_mem(Gtk_Info.Key.Key, &au1KeyPatternGtk[0], (LEN_TK + LEN_TK2));
		}

		/* APCLI set PTK */
		Ptk_Info.Operation  = SEC_ASIC_ADD_PAIRWISE_KEY;
		Ptk_Info.Direction  = SEC_ASIC_KEY_BOTH;
		Ptk_Info.Wcid       = pMacEntry->wcid;
		Ptk_Info.BssIndex   = pMacEntry->func_tb_idx;
		Ptk_Info.Cipher     = pMacEntry->SecConfig.PairwiseCipher;
		Ptk_Info.KeyIdx     = pMacEntry->SecConfig.PairwiseKeyId;
		os_move_mem(&Ptk_Info.PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
		WPAInstallKey(pAd, &Ptk_Info, FALSE, TRUE);

		/* APCLI set GTK */
		Gtk_Info.Operation  = SEC_ASIC_ADD_GROUP_KEY;
		Gtk_Info.Direction  = SEC_ASIC_KEY_RX;
		Gtk_Info.Wcid       = pMacEntry->wdev->bss_info_argument.bmc_wlan_idx;
		Gtk_Info.BssIndex   = BSS0;
		Gtk_Info.Cipher     = pMacEntry->SecConfig.PairwiseCipher;
		Gtk_Info.KeyIdx     = pMacEntry->SecConfig.PairwiseKeyId;
		os_move_mem(&Gtk_Info.PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
		WPAInstallKey(pAd, &Gtk_Info, FALSE, TRUE);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMacEntry is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

static struct PRECFG_CFG_SET PreCfgAllCfgSet[PRECFG_CFGSET_MAX] = {
	{
		.u2CfgSetID = PRECFG_CFGSET_BASIC,
		.u8CmdFeature = BASIC_CONNECT_FLAGS,
		.pucDescription = "Basic Connection Configuration",
		.pfCommHandler = PreCfgStaRecComm,
		.pfSendHandler = PreCfgStaRecSend,
		.pfTimerHandler = NULL,
		.pfHandler = {
			{
				/* UNI_CMD_STAREC_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_BASIC,
				.pfPrepareHandler = PreCfgStaRecBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecBasicUpdate
			},
			{
				/*  UNI_CMD_STAREC_AAD_OM,
					UNI_CMD_STAREC_HDRT,
					UNI_CMD_STAREC_PHY_INFO */
				.u8CmdFeature = (PRE_CFG_UNI_CMD_STAREC_AAD_OM |
								PRE_CFG_UNI_CMD_STAREC_HDRT |
								PRE_CFG_UNI_CMD_STAREC_PHY_INFO),
				.pfPrepareHandler = PreCfgStaRecWtblPrepare,
				.pfUpdateHandler = PreCfgStaRecWtblUpdate
			},
			{
				/* UNI_CMD_STAREC_HW_AMSDU */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_HW_AMSDU,
				.pfPrepareHandler = PreCfgStaRecHwAmsduPrepare,
				.pfUpdateHandler = PreCfgStaRecHwAmsduUpdate
			},
			{
				/* UNI_CMD_STAREC_HT_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_HT_BASIC,
				.pfPrepareHandler = PreCfgStaRecHtBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecHtBasicUpdate
			},
			{
				/* UNI_CMD_STAREC_VHT_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_VHT_BASIC,
				.pfPrepareHandler = PreCfgStaRecVhtBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecVhtBasicUpdate
			},
			{
				/* UNI_CMD_STAREC_HE_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_HE_BASIC,
				.pfPrepareHandler = PreCfgStaRecHeBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecHeBasicUpdate
			},
			{
				/* UNI_CMD_STAREC_EHT_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_EHT_BASIC,
				.pfPrepareHandler = PreCfgStaRecEhtBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecEhtBasicUpdate
			},
			{
				/* UNI_CMD_STAREC_MLD_TEARDOWN */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_MLD_TEARDOWN,
				.pfPrepareHandler = PreCfgStaRecMldTeardownPrepare,
				.pfUpdateHandler = PreCfgStaRecMldTeardownUpdate
			},
			{
				/* UNI_CMD_STAREC_MURU */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_MURU_INFO,
				.pfPrepareHandler = PreCfgStaRecMuruInfoPrepare,
				.pfUpdateHandler = PreCfgStaRecMuruInfoUpdate
			},
			TAG_END
		},
	},
	{
		.u2CfgSetID = PRECFG_CFGSET_RA,
		.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_RA,
		.pucDescription = "RA Configuration",
		.pfCommHandler = PreCfgStaRecComm,
		.pfSendHandler = PreCfgStaRecSend,
		.pfTimerHandler = NULL,
		.pfHandler = {
			{
				/* UNI_CMD_STAREC_RA */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_RA,
				.pfPrepareHandler = PreCfgStaRecRaPrepare,
				.pfUpdateHandler = PreCfgStaRecRaUpdate
			},
			TAG_END
		},
	},
	{
		.u2CfgSetID = PRECFG_CFGSET_MLD,
		.u8CmdFeature = (PRE_CFG_UNI_CMD_STAREC_MLD_SETUP | PRE_CFG_UNI_CMD_STAREC_EHT_MLD | PRE_CFG_UNI_CMD_STAREC_EHT_BASIC),
		.pucDescription = "MLD Configuration",
		.pfCommHandler = PreCfgStaRecComm,
		.pfSendHandler = PreCfgStaRecSend,
		.pfTimerHandler = NULL,
		.pfHandler = {
			{
				/* UNI_CMD_STAREC_MLD_SETUP */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_MLD_SETUP,
				.pfPrepareHandler = PreCfgStaRecMldSetupPrepare,
				.pfUpdateHandler = PreCfgStaRecMldSetupUpdate
			},
			{
				/* UNI_CMD_STAREC_EHT_MLD */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_EHT_MLD,
				.pfPrepareHandler = PreCfgStaRecEhtMldPrepare,
				.pfUpdateHandler = PreCfgStaRecEhtMldUpdate
			},
			{
				/* UNI_CMD_STAREC_EHT_BASIC */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_EHT_BASIC,
				.pfPrepareHandler = PreCfgStaRecEhtBasicPrepare,
				.pfUpdateHandler = PreCfgStaRecEhtBasicUpdate
			},
			TAG_END
		},
	},
	{
		.u2CfgSetID = PRECFG_CFGSET_SEC,
		.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_INSTALL_KEY_V2,
		.pucDescription = "AuthMode & EncrypType Configuration",
		.pfCommHandler = NULL,
		.pfSendHandler = NULL,
		.pfTimerHandler = NULL,
		.pfHandler = {
			{
				/* UNI_CMD_STAREC_INSTALL_KEY_V2 */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_INSTALL_KEY_V2,
				.pfPrepareHandler = PreCfgStaRecInstallKeyPrepare,
				.pfUpdateHandler = PreCfgStaRecInstallKeyUpdate
			},
			TAG_END
		},
	},
	{
		.u2CfgSetID = PRECFG_CFGSET_RAND_AMSDU,
		.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_HW_AMSDU,
		.pucDescription = "Random Amsdu Configuration",
		.pfCommHandler = PreCfgStaRecComm,
		.pfSendHandler = NULL,
		.pfTimerHandler = PreCfgRandomAmsduSend,
		.pfHandler = {
			{
				/* UNI_CMD_STAREC_HW_AMSDU */
				.u8CmdFeature = PRE_CFG_UNI_CMD_STAREC_HW_AMSDU,
				.pfPrepareHandler = PreCfgStaRecHwAmsduPrepare,
				.pfUpdateHandler = PreCfgStaRecHwAmsduUpdate
			},
			TAG_END
		},
	},
	{
		.u2CfgSetID = PRECFG_CFGSET_RAND_RADIO,
		.u8CmdFeature = PRE_CFG_NONE,
		.pucDescription = "Random Radio On & Off",
		.pfCommHandler = NULL,
		.pfSendHandler = NULL,
		.pfTimerHandler = PreCfgRandomRadioOnOff,
		.pfHandler = {
			TAG_END
		},
	}
};

NTSTATUS pre_cfg_peer_cap_default_by_wdev(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pWdev,
	IN struct _MAC_TABLE_ENTRY *pMacEntry,
	IN struct _STA_REC_CFG *pStaRecCfg
	)
{
	UINT32 i;
	struct ppdu_caps *ppdu_cap = (struct ppdu_caps *)wlan_config_get_ppdu_caps(pWdev);
	enum ASIC_CAP asic_caps = wlan_config_get_asic_caps(pWdev);
	enum PHY_CAP phy_caps = wlan_config_get_phy_caps(pWdev);
	struct he_mcs_info *mcs = NULL;
	UINT8 rx_nss = wlan_config_get_rx_stream(pWdev);
	UINT8 nss_num = 0x4;

	if (rx_nss > HE_MAX_SUPPORT_STREAM)
		rx_nss = HE_MAX_SUPPORT_STREAM;

	if (g_arPrecfgFakeNegoParam[PRECFG_NEGO_AID].fgIsCfgDone)
		pMacEntry->Aid = g_arPrecfgFakeNegoParam[PRECFG_NEGO_AID].u4Value;

	if (g_arPrecfgFakeNegoParam[PRECFG_NEGO_BW].fgIsCfgDone) {
		pMacEntry->cap.ch_bw.he_ch_width = g_arPrecfgFakeNegoParam[PRECFG_NEGO_BW].u4Value;
		pMacEntry->operating_mode.ch_width = g_arPrecfgFakeNegoParam[PRECFG_NEGO_BW].u4Value;
	}

	if (g_arPrecfgFakeNegoParam[PRECFG_NEGO_NSS].fgIsCfgDone)
		nss_num = g_arPrecfgFakeNegoParam[PRECFG_NEGO_NSS].u4Value;

	pMacEntry->fgPreCfgRunning = TRUE;
	pMacEntry->sta_force_keep = TRUE;

	pMacEntry->HTCapability.MCSSet[0] = 255;
	pMacEntry->HTCapability.MCSSet[1] = 255;
	pMacEntry->HTCapability.MCSSet[2] = 255;
	pMacEntry->HTCapability.MCSSet[3] = 255;
	pMacEntry->HTCapability.HtCapInfo.GF = 0;
	pMacEntry->MmpsMode = MMPS_DISABLE;
	pMacEntry->bAutoTxRateSwitch = pWdev->bAutoTxRateSwitch;
	pMacEntry->MaxRAmpduFactor = 0x7;
	pMacEntry->RateLen = 8;
	pMacEntry->SupportRateMode = 0xe;
	pMacEntry->SupportOFDMMCS = 0xff;
	pMacEntry->SupportHTMCS = 0xffffffff;

	if (IS_ENTRY_CLIENT(pMacEntry)) {
		pMacEntry->HTCapability.MCSSet[4] = 1;
		pMacEntry->ClientStatusFlags = 0x1d9d00f1;
		pMacEntry->MaxHTPhyMode.field.MCS = 0x3b;
	} else {
		pMacEntry->HTCapability.MCSSet[4] = 0;
		pMacEntry->ClientStatusFlags = 0x1d9d00f9;
		pMacEntry->MaxHTPhyMode.field.MCS = 0x39;
	}
	pMacEntry->MaxHTPhyMode.field.STBC = 1;
	pMacEntry->MaxHTPhyMode.field.ShortGI = 1;
	pMacEntry->MaxHTPhyMode.field.BW = BW_160;
	pMacEntry->MaxHTPhyMode.field.ldpc = 1;

	pMacEntry->MinHTPhyMode.word = pWdev->MinHTPhyMode.word;
	pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
	pMacEntry->AMsduSize = 1;

// Max NSS number
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs10_11_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs12_13_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs0_9_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs10_11_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs12_13_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs0_9_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs10_11_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs12_13_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss = nss_num;
	pMacEntry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss = nss_num;

	pMacEntry->SupportVHTMCS1SS = 0x0;
	pMacEntry->SupportVHTMCS2SS = 0x0;
	pMacEntry->SupportVHTMCS3SS = 0x0;
	pMacEntry->SupportVHTMCS4SS = 0x0;

	switch (nss_num) {
	case 4:
		pMacEntry->SupportVHTMCS1SS = 0x3ff;
		pMacEntry->SupportVHTMCS2SS = 0x3ff;
		pMacEntry->SupportVHTMCS3SS = 0x3ff;
		pMacEntry->SupportVHTMCS4SS = 0x3ff;
		break;

	case 3:
		pMacEntry->SupportVHTMCS1SS = 0x3ff;
		pMacEntry->SupportVHTMCS2SS = 0x3ff;
		pMacEntry->SupportVHTMCS3SS = 0x3ff;
		break;

	case 2:
		pMacEntry->SupportVHTMCS1SS = 0x3ff;
		pMacEntry->SupportVHTMCS2SS = 0x3ff;
		break;

	case 1:
		pMacEntry->SupportVHTMCS1SS = 0x3ff;
		break;

	default:
		break;
	}

	mcs = &pStaRecCfg->he_sta.max_nss_mcs;
	for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
		// 0x2: Support max MCS, 0x3: Don't support
		mcs->bw80_mcs[i] = i < nss_num ? 0x2:0x3;
		mcs->bw160_mcs[i] = i < nss_num ? 0x2:0x3;
		mcs->bw8080_mcs[i] = 0x3;
		*((UINT16 *)&pMacEntry->vht_cap_ie.mcs_set.rx_mcs_map) |= i < nss_num ?
			(0x2 << (i * 2)):(0x3 << (i * 2));
		*((UINT16 *)&pMacEntry->vht_cap_ie.mcs_set.tx_mcs_map) |= i < nss_num ?
			(0x2 << (i * 2)):(0x3 << (i * 2));
	}

	os_move_mem(&pMacEntry->cap.rate.he80_rx_nss_mcs[0], &mcs->bw80_mcs[0],
					sizeof(pMacEntry->cap.rate.he80_rx_nss_mcs));
	os_move_mem(&pMacEntry->cap.rate.he160_rx_nss_mcs[0], &mcs->bw160_mcs[0],
					sizeof(pMacEntry->cap.rate.he160_rx_nss_mcs));
	os_move_mem(&pMacEntry->cap.rate.he8080_rx_nss_mcs[0], &mcs->bw8080_mcs[0],
					sizeof(pMacEntry->cap.rate.he8080_rx_nss_mcs));

	//CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);

	/* HT part */
	if (WMODE_CAP_N(pWdev->PhyMode)) {
		//CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HT_CAPABLE);
		pMacEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pMacEntry->HTPhyMode.field.MODE = MODE_HTMIX;
		MTWF_PRINT("%s: Support HT\n", __func__);
	}

	/* VHT part */
	if (WMODE_CAP_AC(pWdev->PhyMode)) {
		//CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_VHT_CAPABLE);
		pMacEntry->MaxHTPhyMode.field.MODE = MODE_VHT;
		pMacEntry->HTPhyMode.field.MODE = MODE_VHT;
		MTWF_PRINT("%s: Support VHT\n", __func__);
	} else {
		PreCfgAllCfgSet[PRECFG_CFGSET_BASIC].u8CmdFeature &= ~PRE_CFG_UNI_CMD_STAREC_VHT_BASIC;
	}

	/* HE part */
	if (WMODE_CAP_AX(pWdev->PhyMode)) {
		//CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HE_CAPABLE);
		pMacEntry->cap.modes |= (HE_24G_SUPPORT | HE_5G_SUPPORT | HE_6G_SUPPORT);
		pMacEntry->MaxHTPhyMode.field.MODE = MODE_HE;
		pMacEntry->HTPhyMode.field.MODE = MODE_HE;
		MTWF_PRINT("%s: Support HE\n", __func__);
	} else {
		PreCfgAllCfgSet[PRECFG_CFGSET_BASIC].u8CmdFeature &= ~PRE_CFG_UNI_CMD_STAREC_HE_BASIC;
	}

	/* BE part */
	if (WMODE_CAP_BE(pWdev->PhyMode)) {
		pMacEntry->cap.modes |= (EHT_24G_SUPPORT | EHT_5G_SUPPORT | EHT_6G_SUPPORT);
		pMacEntry->MaxHTPhyMode.field.MODE = MODE_EHT;
		pMacEntry->HTPhyMode.field.MODE = MODE_EHT;
		MTWF_PRINT("%s: Support BE\n", __func__);
	} else {
		PreCfgAllCfgSet[PRECFG_CFGSET_BASIC].u8CmdFeature &= ~PRE_CFG_UNI_CMD_STAREC_EHT_BASIC;
	}

#ifdef DOT11_HE_AX
	NdisZeroMemory(&pStaRecCfg->he_sta, sizeof(pStaRecCfg->he_sta));
	pStaRecCfg->he_sta.mac_info.max_ampdu_len_exp = ppdu_cap->he_max_ampdu_len_exp;
	pStaRecCfg->he_sta.mac_info.amsdu_in_ampdu_support = ppdu_cap->tx_amsdu_support;
	pStaRecCfg->he_sta.mac_info.htc_support = 0;
	pStaRecCfg->he_sta.mac_info.bqr_support = 0;
	pStaRecCfg->he_sta.mac_info.bsr_support = 0;
	pStaRecCfg->he_sta.mac_info.om_support = 0;
	pStaRecCfg->he_sta.mac_info.om_support = 0;
	pStaRecCfg->he_sta.mac_info.trigger_frame_mac_pad_dur = 0;
	pStaRecCfg->he_sta.phy_info.dual_band_support = (asic_caps & fASIC_CAP_DBDC)?1:0;
	if (WMODE_CAP_AX_2G(pWdev->PhyMode))
		pStaRecCfg->he_sta.phy_info.ch_width_set |= SUPP_40M_CW_IN_24G_BAND;
	else if (WMODE_CAP_AX_5G(pWdev->PhyMode))
		pStaRecCfg->he_sta.phy_info.ch_width_set |= SUPP_160M_CW_IN_5G_6G_BAND;
	pStaRecCfg->he_sta.phy_info.bw20_242tone = (phy_caps & fPHY_CAP_BW20_242TONE)?1:0;
	pStaRecCfg->he_sta.phy_info.punctured_preamble_rx = 0;
	pStaRecCfg->he_sta.phy_info.device_class = 0;
	pStaRecCfg->he_sta.phy_info.ldpc_support = (wlan_config_get_he_ldpc(pWdev) > 0)?1:0;
	pStaRecCfg->he_sta.phy_info.stbc_support = (wlan_config_get_he_tx_stbc(pWdev) || wlan_config_get_he_rx_stbc(pWdev))?1:0;
	pStaRecCfg->he_sta.phy_info.gi_cap = 1;
	pStaRecCfg->he_sta.phy_info.dcm_max_nss_tx = 0;
	pStaRecCfg->he_sta.phy_info.dcm_cap_tx = 0;
	pStaRecCfg->he_sta.phy_info.dcm_max_nss_rx = 0;
	pStaRecCfg->he_sta.phy_info.dcm_cap_rx = 0;
	pStaRecCfg->he_sta.phy_info.dcm_max_ru = 0;
	pStaRecCfg->he_sta.phy_info.tx_le_ru242_1024qam = 1;
	pStaRecCfg->he_sta.phy_info.rx_le_ru242_1024qam = 1;
	pStaRecCfg->he_sta.phy_info.triggered_cqi_feedback_support = 0;
	pStaRecCfg->he_sta.phy_info.partial_bw_ext_range_support = 0;

	/*max nss mcs*/
/*	mcs = &pStaRecCfg->he_sta.max_nss_mcs;
	for (i = 0 ; i < DOT11AX_MAX_STREAM; i++) {
		mcs->bw80_mcs[i] = 0xff;
		mcs->bw8080_mcs[i] = 0xff;
		mcs->bw160_mcs[i] = 0xff;
		for (j = 0; (j < rx_nss) && (j < HE_MAX_SUPPORT_STREAM); j++) {
			mcs->bw8080_mcs[j] = 0x3;
			mcs->bw80_mcs[j] = 0x2;
			mcs->bw160_mcs[j] = 0x2;
		}
	}*/
#endif /* DOT11_HE_AX */

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

struct PRECFG_CFG_SET *PreCfg_Lookup_ConfigSet(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 u2CfgSetID)
{
	UINT32 i;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;

	if (u2CfgSetID >= PRECFG_CFGSET_MAX)
		return NULL;

	for (i = 0; i < PRECFG_CFGSET_MAX; i++) {
		pPrecfgCfgSet = &PreCfgAllCfgSet[i];
		if (pPrecfgCfgSet->u2CfgSetID == u2CfgSetID)
			break;
	}

	return pPrecfgCfgSet;
}

NTSTATUS pre_cfg_comm_fw_update(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN BOOLEAN fgConnect)
{
	UINT32 i, j, u4RealUsedBufSize = 0;
	UINT16 u2TLVNumber = 0, u2TLVTotalNumber = 0;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;
	struct PRE_CFG_TAG_HANDLE *pfHandler = NULL;
	UCHAR *pCmdBuf = NULL, *pNextHeadBuf = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;
	if (IS_VALID_ENTRY(pMacEntry)) {
		if (os_alloc_mem(pAd, (UCHAR **)&pCmdBuf, PRE_CFG_CMD_BUF_MAX) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Allocate memory failed, please check!\n");
			return NDIS_STATUS_FAILURE;
		}

		for (i = 0; i < pPreCfgEntry->u2CfgSetCount; i++) {
			os_zero_mem(pCmdBuf, PRE_CFG_CMD_BUF_MAX);
			pNextHeadBuf = pCmdBuf;
			u2TLVTotalNumber = 0;
			pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pAd, pPreCfgEntry->u2ArrCfgSetInput[i]);
			if (pPrecfgCfgSet == NULL)
				continue;
			if (fgConnect) {
				pPreCfgEntry->StaRecCfg.u8EnableFeature = pPrecfgCfgSet->u8CmdFeature;
			} else {
				pPreCfgEntry->StaRecCfg.u8EnableFeature = PRE_CFG_UNI_CMD_STAREC_BASIC;
				if (pMacEntry->mlo.mlo_en)
					pPreCfgEntry->StaRecCfg.u8EnableFeature |= PRE_CFG_UNI_CMD_STAREC_MLD_TEARDOWN;
			}

			if (pPrecfgCfgSet->pfCommHandler != NULL) {
				Ret = pPrecfgCfgSet->pfCommHandler(pAd, pPreCfgCtrl, pPreCfgEntry,
											fgConnect, &pNextHeadBuf);
				if (Ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"pfCommHandler return fail!\n");
					goto err;
				}
			}

			pfHandler = &pPrecfgCfgSet->pfHandler[0];
			for (j = 0; j < PRE_CFG_TAG_MAX; j++, pfHandler++) {
				if (pfHandler->u8CmdFeature == PRE_CFG_TAG_END) // Mean to end
					break;

				if ((pPreCfgEntry->StaRecCfg.u8EnableFeature & pfHandler->u8CmdFeature) &&
					pfHandler->pfUpdateHandler) {
					u2TLVNumber = 0;
					Ret = pfHandler->pfUpdateHandler(pAd, pPreCfgCtrl, pPreCfgEntry,
											fgConnect, &pNextHeadBuf, &u2TLVNumber);
					if (Ret == NDIS_STATUS_SUCCESS)
						u2TLVTotalNumber += u2TLVNumber;
					else
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"u8CmdFeature = 0x%llx is failure!\n", pfHandler->u8CmdFeature);
				}
			}

			u4RealUsedBufSize = (pNextHeadBuf - pCmdBuf);
			MTWF_PRINT("%s: u2TLVTotalNumber = %d, u4RealUsedBufSize = %d\n",
						__func__, u2TLVTotalNumber, u4RealUsedBufSize);

			if ((u2TLVTotalNumber > 0) && (u4RealUsedBufSize > 0)) {
				if (pPrecfgCfgSet->pfSendHandler != NULL) {
					Ret = pPrecfgCfgSet->pfSendHandler(pAd, u2TLVTotalNumber, pCmdBuf, u4RealUsedBufSize);
					if (Ret != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"pfSendHandler return fail!\n");
						goto err;
					}
				}
			}
			if ((pPrecfgCfgSet->pfTimerHandler != NULL) && (fgConnect)) {
				struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl;

				os_alloc_mem(NULL, (UCHAR **)&pPreCfgTimerCmdCtrl, sizeof(*pPreCfgTimerCmdCtrl));
				if (pPreCfgTimerCmdCtrl == NULL) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Allocate memory failed, please check!\n");
					Ret = NDIS_STATUS_FAILURE;
					goto err;
				}
				NdisZeroMemory(pPreCfgTimerCmdCtrl, sizeof(*pPreCfgTimerCmdCtrl));
				DlListInit(&pPreCfgTimerCmdCtrl->List);
				DlListAddTail(&pPreCfgCtrl->TimerList, &pPreCfgTimerCmdCtrl->List);
				pPreCfgTimerCmdCtrl->pAd = pAd;
				pPreCfgTimerCmdCtrl->u2TLVTotalNumber = u2TLVTotalNumber;
				pPreCfgTimerCmdCtrl->u4RealUsedBufSize = u4RealUsedBufSize;
				pPreCfgTimerCmdCtrl->u2CfgSetIdx = pPreCfgEntry->u2ArrCfgSetInput[i];
				pPreCfgTimerCmdCtrl->pfTimerHandler = pPrecfgCfgSet->pfTimerHandler;
				pPreCfgTimerCmdCtrl->pPreCfgEntry = pPreCfgEntry;
				os_alloc_mem(pAd, (UCHAR **)&pPreCfgTimerCmdCtrl->pCmdBuf, PRE_CFG_CMD_BUF_MAX);
				if (pPreCfgTimerCmdCtrl->pCmdBuf == NULL) {
					if (pPreCfgTimerCmdCtrl)
						os_free_mem(pPreCfgTimerCmdCtrl);
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Allocate memory failed, please check!\n");
					Ret = NDIS_STATUS_FAILURE;
					goto err;
				}
				os_zero_mem(pPreCfgTimerCmdCtrl->pCmdBuf, PRE_CFG_CMD_BUF_MAX);
				NdisCopyMemory(pPreCfgTimerCmdCtrl->pCmdBuf, pCmdBuf, u4RealUsedBufSize);
				os_zero_mem(&pPreCfgTimerCmdCtrl->PreCfgTimer, sizeof(RALINK_TIMER_STRUCT));
				RTMPInitTimer(pAd, &pPreCfgTimerCmdCtrl->PreCfgTimer, GET_TIMER_FUNCTION(PreCfgTimerSend), pPreCfgTimerCmdCtrl, TRUE);
				RTMPSetTimer(&pPreCfgTimerCmdCtrl->PreCfgTimer, 3000);

				MTWF_PRINT("%s: Add Config Set (ID:%d) in periodic timer\n", __func__, pPreCfgTimerCmdCtrl->u2CfgSetIdx);
			}
			MTWF_PRINT("%s: \x1b[1;33mConfig Set ID = %d, Description: %s Done!\x1b[m\n", __func__,
						pPreCfgEntry->u2ArrCfgSetInput[i], pPrecfgCfgSet->pucDescription);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
err:
	if (pCmdBuf)
		os_free_mem(pCmdBuf);

	return Ret;
}

NTSTATUS pre_cfg_peer_sta_sys_init(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN UCHAR *pMacAddr,
	IN UINT16 u2MldStaIdx,
	IN UINT16 *pu2CfgSetInput,
	IN UINT16 u2CfgSetCount)
{
	UINT32 i, j, k;
	BSS_STRUCT *pMbss = pPreCfgCtrl->PreCfgData.ApData.pMbss;
	struct ml_ie_info *pMlInfo = &pPreCfgCtrl->PreCfgData.ApData.ml_info;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;
	struct PRE_CFG_TAG_HANDLE *pfHandler = NULL;
	struct PRECFG_ENTRY *pNewPreCfgEntry = NULL;
	struct _RTMP_ADAPTER *pCurrAd = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	UINT16 mld_sta_idx = u2MldStaIdx;

	pMacEntry = MacTableLookup(pAd, pMacAddr);
	if (pMacEntry != NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"The mac address ("MACSTR") already exist!\n", MAC2STR(pMacAddr));
		return NDIS_STATUS_FAILURE;
	}

	if (mld_sta_idx != MLD_STA_NONE) {
		pMlInfo->link_id = mld_sta_idx;
		pMlInfo->mld_caps = 0x1;
		COPY_MAC_ADDR(pMlInfo->mld_addr, pMacAddr);
		mld_sta_idx = pre_cfg_mld_sta_add(pMbss->wdev.if_dev, pMlInfo, pMacAddr);
		if (!BMGR_VALID_MLD_STA(mld_sta_idx)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"mld_sta_idx(%d) is invalid!\n", mld_sta_idx);
			return NDIS_STATUS_FAILURE;
		}
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			if (pMlInfo->link[i].active != TRUE)
				continue;

			pMacEntry = MacTableLookup(pAd, pMlInfo->link[i].link_addr);

			if (IS_VALID_ENTRY(pMacEntry) != TRUE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"pMacEntry is invalid!\n");
				return NDIS_STATUS_FAILURE;
			}

			pMacEntry->fgPreCfgRunning = TRUE;
			pMacEntry->sta_force_keep = TRUE;
			/* set aid=1024 for MLD */
			pMacEntry->Aid = 0x400;
			if (os_alloc_mem(pAd, (UCHAR **)&pNewPreCfgEntry, sizeof(*pNewPreCfgEntry)) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Allocate memory failed, please check!\n");
				MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacAddr);
				return NDIS_STATUS_FAILURE;
			}

			pCurrAd = (struct _RTMP_ADAPTER *)pMacEntry->wdev->sys_handle;
			os_zero_mem(pNewPreCfgEntry, sizeof(*pNewPreCfgEntry));

			SET_PRE_CFG_ENTRY_PEER(pNewPreCfgEntry, PRE_CFG_ENTRY_PEER_STA);
			pNewPreCfgEntry->pPreCtrl = pPreCfgCtrl;
			pNewPreCfgEntry->pMacEntry = pMacEntry;
			pNewPreCfgEntry->mld_sta_idx = mld_sta_idx;
			DlListInit(&pNewPreCfgEntry->List);
			os_move_mem(pNewPreCfgEntry->u2ArrCfgSetInput, pu2CfgSetInput, sizeof(pu2CfgSetInput[0])*u2CfgSetCount);
			pNewPreCfgEntry->u2CfgSetCount = u2CfgSetCount;
			pNewPreCfgEntry->fgValid = TRUE;
			pNewPreCfgEntry->fgConnect = FALSE;
			DlListAddTail(&pPreCfgCtrl->PreCfgData.ApData.EntryList, &pNewPreCfgEntry->List);
			NdisMoveMemory(&pMacEntry->SecConfig, &pMbss->wdev.SecConfig, sizeof(struct _SECURITY_CONFIG));
			pre_cfg_peer_cap_default_by_wdev(pCurrAd, &pMbss->wdev, pMacEntry, &pNewPreCfgEntry->StaRecCfg);

			for (j = 0; j < u2CfgSetCount; j++) {
				pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pCurrAd, pu2CfgSetInput[j]);
				if (pPrecfgCfgSet == NULL)
					continue;
				pfHandler = &pPrecfgCfgSet->pfHandler[0];

				for (k = 0; k < PRE_CFG_TAG_MAX; k++, pfHandler++) {
					if ((pPrecfgCfgSet->u8CmdFeature & pfHandler->u8CmdFeature) == 0)
						continue;

					if (pfHandler->pfPrepareHandler == NULL)
						continue;

					Ret = pfHandler->pfPrepareHandler(pCurrAd, pPreCfgCtrl, pNewPreCfgEntry, TRUE);
					if (Ret != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pCurrAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"u8CmdFeature = 0x%llx is failure!\n", pfHandler->u8CmdFeature);
					}
				}
			}
			MTWF_PRINT("%s: Pre cfg add Mac Entry("MACSTR") successfully!\n",
						__func__, MAC2STR(pMacEntry->Addr));
		}
	} else {

		/* ToDo : PreCfg align device normal flow security setting, and only
		 *        handle key nego */
		//if (pPreCfgCtrl->PreCfgData.ApData.fgSecurity == 0) {
		//	Set_SecAuthMode_Proc(pAd, "OPEN");
		//	Set_SecEncrypType_Proc(pAd, "NONE");
		//}

		//APSecInit(pAd, &pMbss->wdev);
		//restart_ap(&pMbss->wdev);

		pMacEntry = MacTableInsertEntry(pAd, pMacAddr, &pMbss->wdev,
									ENTRY_CLIENT, OPMODE_AP, TRUE, mld_sta_idx, NULL);
		if (IS_VALID_ENTRY(pMacEntry)) {
			pMacEntry->fgPreCfgRunning = TRUE;
			pMacEntry->sta_force_keep = TRUE;
			if (os_alloc_mem(pAd, (UCHAR **)&pNewPreCfgEntry, sizeof(*pNewPreCfgEntry)) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Allocate memory failed, please check!\n");
				MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacAddr);
				return NDIS_STATUS_FAILURE;
			}
			os_zero_mem(pNewPreCfgEntry, sizeof(*pNewPreCfgEntry));

			SET_PRE_CFG_ENTRY_PEER(pNewPreCfgEntry, PRE_CFG_ENTRY_PEER_STA);
			pNewPreCfgEntry->pPreCtrl = pPreCfgCtrl;
			pNewPreCfgEntry->pMacEntry = pMacEntry;
			pNewPreCfgEntry->mld_sta_idx = mld_sta_idx;
			DlListInit(&pNewPreCfgEntry->List);
			os_move_mem(pNewPreCfgEntry->u2ArrCfgSetInput, pu2CfgSetInput, sizeof(pu2CfgSetInput[0])*u2CfgSetCount);
			pNewPreCfgEntry->u2CfgSetCount = u2CfgSetCount;
			pNewPreCfgEntry->fgValid = TRUE;
			pNewPreCfgEntry->fgConnect = FALSE;
			DlListAddTail(&pPreCfgCtrl->PreCfgData.ApData.EntryList, &pNewPreCfgEntry->List);
			NdisMoveMemory(&pMacEntry->SecConfig, &pMbss->wdev.SecConfig, sizeof(struct _SECURITY_CONFIG));
			pre_cfg_peer_cap_default_by_wdev(pAd, &pMbss->wdev, pMacEntry, &pNewPreCfgEntry->StaRecCfg);
			for (i = 0; i < u2CfgSetCount; i++) {
				pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pAd, pu2CfgSetInput[i]);
				if (pPrecfgCfgSet == NULL)
					continue;
				pfHandler = &pPrecfgCfgSet->pfHandler[0];
				for (j = 0; j < PRE_CFG_TAG_MAX; j++, pfHandler++) {
					if ((pPrecfgCfgSet->u8CmdFeature & pfHandler->u8CmdFeature) &&
						pfHandler->pfPrepareHandler) {
						Ret = pfHandler->pfPrepareHandler(pAd, pPreCfgCtrl, pNewPreCfgEntry, TRUE);
						if (Ret != NDIS_STATUS_SUCCESS) {
							MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"u8CmdFeature = 0x%llx is failure!\n", pfHandler->u8CmdFeature);
						}
					}
				}
			}
			MTWF_PRINT("%s: Pre cfg add Mac Entry("MACSTR") successfully!\n",
				__func__, MAC2STR(pMacEntry->Addr));
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"pMacEntry is invalid!\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_peer_sta_connect(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	STA_TR_ENTRY *pTrEntry = NULL;
	struct _MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *pWdev = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;

	if (IS_VALID_ENTRY(pMacEntry)) {
		pWdev = pMacEntry->wdev;
		pTrEntry = tr_entry_get(pAd, pMacEntry->tr_tb_idx);

		if (pMacEntry->EntryState != ENTRY_STATE_NONE) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry->EntryState is incorrect!!\n");
			return NDIS_STATUS_FAILURE;
		}

		if (pPreCfgCtrl->PeerOp.peer_sys_conn_act) {
			Ret = pPreCfgCtrl->PeerOp.peer_sys_conn_act(pAd, pPreCfgCtrl, pPreCfgEntry);
			if (Ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Failed to peer_sys_conn_act!\n");
				return Ret;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"peer_sys_conn_act is NULL hook!\n");
			return NDIS_STATUS_FAILURE;
		}
		pPreCfgEntry->fgConnect = TRUE;
		pMacEntry->Sst = SST_ASSOC;
		pMacEntry->AuthState = AS_AUTH_OPEN;
		pMacEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
		pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
		pMacEntry->sta_rec_valid = TRUE;
		pTrEntry->PortSecured = WPA_802_1X_PORT_SECURED;
		CheckBMCPortSecured(pAd, pMacEntry, TRUE);

		/*indicate mac entry under sync to hw*/
		pMacEntry->EntryState = ENTRY_STATE_SYNC;
		MSDU_FORBID_CLEAR(pWdev, MSDU_FORBID_CONNECTION_NOT_READY);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_peer_sta_disconnect(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *pTrEntry = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;
	if (IS_VALID_ENTRY(pMacEntry)) {
		pTrEntry = tr_entry_get(pAd, pMacEntry->tr_tb_idx);

		if (pMacEntry->EntryState != ENTRY_STATE_SYNC) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry->EntryState is incorrect!!\n");
			return NDIS_STATUS_FAILURE;
		}

		if (pPreCfgCtrl->PeerOp.peer_sys_disconn_act) {
			Ret = pPreCfgCtrl->PeerOp.peer_sys_disconn_act(pAd, pPreCfgCtrl, pPreCfgEntry, pMacEntry);
			if (Ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Failed to peer_sys_disconn_act!\n");
				return Ret;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"peer_sys_disconn_act is NULL hook!\n");
			return NDIS_STATUS_FAILURE;
		}
		pPreCfgEntry->fgConnect = FALSE;
		pTrEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		CheckBMCPortSecured(pAd, pMacEntry, FALSE);

		/*indicate mac entry under sync to hw*/
		pMacEntry->EntryState = ENTRY_STATE_NONE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (DlListLen(&pPreCfgCtrl->TimerList) != 0) {
		UINT8 cancelled;
		struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl, *pPreCfgTimerCmdCtrlTmp;

		DlListForEachSafe(pPreCfgTimerCmdCtrl, pPreCfgTimerCmdCtrlTmp, &pPreCfgCtrl->TimerList, struct PRECFG_TIMER_CMD_CTRL, List) {
			if (pPreCfgTimerCmdCtrl) {
				MTWF_PRINT("%s: Delete Config Set (ID: %d) in periodic timer\n", __func__, pPreCfgTimerCmdCtrl->u2CfgSetIdx);
				RTMPCancelTimer(&pPreCfgTimerCmdCtrl->PreCfgTimer, &cancelled);
				DlListDel(&pPreCfgTimerCmdCtrl->List);
				os_free_mem(pPreCfgTimerCmdCtrl);
			}
		}
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}


NTSTATUS pre_cfg_peer_sta_sys_conn_act(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	struct wifi_dev *pWdev = NULL;
	struct _MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;
	pWdev = pMacEntry->wdev;
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;

	pStaRecCfg->ucBssIndex = pWdev->bss_info_argument.ucBssIndex;
	pStaRecCfg->MuarIdx = pWdev->bss_info_argument.OwnMacIdx;
	pStaRecCfg->u2WlanIdx = pMacEntry->wcid;
	pStaRecCfg->ConnectionType = pMacEntry->ConnectionType;
	pStaRecCfg->ConnectionState = STATE_PORT_SECURE;
	pStaRecCfg->IsNewSTARec = TRUE;
	pStaRecCfg->pEntry = pMacEntry;

	if (pPreCfgCtrl->PeerOp.peer_fw_update) {
		Ret = pPreCfgCtrl->PeerOp.peer_fw_update(pAd, pPreCfgCtrl, pPreCfgEntry, TRUE);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_fw_update!\n");
			return Ret;
		}
	}  else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"peer_fw_update is NULL hook!\n");
		return NDIS_STATUS_FAILURE;
	}
	//chip_ra_init(pAd, pMacEntry);

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_peer_sta_sys_disconn_act(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN struct _MAC_TABLE_ENTRY *pMacEntry)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	struct _STA_REC_CFG *pStaRecCfg = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_STA(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;
	pStaRecCfg->ConnectionState = STATE_DISCONNECT;

	if (pPreCfgCtrl->PeerOp.peer_fw_update) {
		Ret = pPreCfgCtrl->PeerOp.peer_fw_update(pAd, pPreCfgCtrl, pPreCfgEntry, FALSE);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_fw_update!\n");
			return Ret;
		}
	}  else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"peer_fw_update is NULL hook!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}


#ifdef APCLI_SUPPORT
NTSTATUS pre_cfg_peer_ap_sys_init(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN UCHAR *pMacAddr,
	IN UINT16 u2MldStaIdx,
	IN UINT16 *pu2CfgSetInput,
	IN UINT16 u2CfgSetCount)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct PRECFG_ENTRY *pNewPreCfgEntry = NULL;

	pMacEntry = MacTableLookup(pAd, pMacAddr);
	if (pMacEntry != NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"The mac address ("MACSTR") already exist!\n", MAC2STR(pMacAddr));
		return NDIS_STATUS_FAILURE;
	}

	if (MAC_ADDR_EQUAL(pPreCfgCtrl->PreCfgData.StaData.ApMacAddr, pMacAddr)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"The mac address ("MACSTR") is already created!\n", MAC2STR(pMacAddr));
		return NDIS_STATUS_FAILURE;
	}

	if (os_alloc_mem(pAd, (UCHAR **)&pNewPreCfgEntry, sizeof(*pNewPreCfgEntry)) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Allocate memory failed, please check!\n");
		return NDIS_STATUS_FAILURE;
	}
	pPreCfgCtrl->PreCfgData.StaData.u2MldStaIdx = u2MldStaIdx;
	COPY_MAC_ADDR(pPreCfgCtrl->PreCfgData.StaData.ApMacAddr, pMacAddr);
	os_zero_mem(pNewPreCfgEntry, sizeof(*pNewPreCfgEntry));

	SET_PRE_CFG_ENTRY_PEER(pNewPreCfgEntry, PRE_CFG_ENTRY_PEER_AP);
	pNewPreCfgEntry->pPreCtrl = pPreCfgCtrl;
	os_move_mem(pNewPreCfgEntry->u2ArrCfgSetInput, pu2CfgSetInput, sizeof(pu2CfgSetInput[0])*u2CfgSetCount);
	pNewPreCfgEntry->u2CfgSetCount = u2CfgSetCount;
	pNewPreCfgEntry->fgValid = TRUE;
	pNewPreCfgEntry->fgConnect = FALSE;
	pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry = pNewPreCfgEntry;

	return NDIS_STATUS_SUCCESS;
}

NTSTATUS PreCfg_Load_Config_Set(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	UINT32 u4CfgSetIdx = 0, j = 0;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;
	struct PRE_CFG_TAG_HANDLE *pfHandler = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	for (u4CfgSetIdx = 0; u4CfgSetIdx < pPreCfgEntry->u2CfgSetCount; u4CfgSetIdx++) {
		pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pAd, pPreCfgEntry->u2ArrCfgSetInput[u4CfgSetIdx]);
		if (pPrecfgCfgSet == NULL)
			continue;

		pfHandler = &pPrecfgCfgSet->pfHandler[0];
		for (j = 0; j < PRE_CFG_TAG_MAX; j++, pfHandler++) {
			if ((pPrecfgCfgSet->u8CmdFeature & pfHandler->u8CmdFeature) &&
				pfHandler->pfPrepareHandler) {
				if (pfHandler->pfPrepareHandler(pAd, pPreCfgCtrl, pPreCfgEntry, TRUE) != NDIS_STATUS_SUCCESS) {
					Ret = NDIS_STATUS_FAILURE;
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"u8CmdFeature = 0x%llx is failure!\n", pfHandler->u8CmdFeature);
				}
			}
		}
	}

	return Ret;
}

NTSTATUS PreCfg_Do_LinkUp(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (pPreCfgCtrl->PeerOp.sys_link_up) {
		Ret = pPreCfgCtrl->PeerOp.sys_link_up(pAd, pPreCfgCtrl, pPreCfgEntry, pPreCfgEntry->pMacEntry);
		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to sys_link_up!\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"sys_link_up is NULL hook!\n");
		return NDIS_STATUS_FAILURE;
	}

	return Ret;
}

NTSTATUS PreCfg_Do_Connection(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry
)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	STA_TR_ENTRY *pTrEntry = tr_entry_get(pAd, pPreCfgEntry->pMacEntry->tr_tb_idx);
	struct _STA_ADMIN_CONFIG *pStaCfg = pPreCfgCtrl->PreCfgData.StaData.pStaCfg;

	if (pPreCfgCtrl->PeerOp.peer_sys_conn_act) {
		Ret = pPreCfgCtrl->PeerOp.peer_sys_conn_act(pAd, pPreCfgCtrl, pPreCfgEntry);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Failed to peer_sys_conn_act!\n");
			return Ret;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"peer_sys_conn_act is NULL hook!\n");
		return NDIS_STATUS_FAILURE;
	}

	//sta_mld_ra_init(pAd, pWdev, pMacEntry);

	pPreCfgEntry->fgConnect = TRUE;

	pPreCfgEntry->pMacEntry->Sst = SST_ASSOC;
	pPreCfgEntry->pMacEntry->AuthState = AS_AUTH_OPEN;
	pPreCfgEntry->pMacEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
	pPreCfgEntry->pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	pPreCfgEntry->pMacEntry->sta_rec_valid = TRUE;
	pPreCfgEntry->pMacEntry->EntryState = ENTRY_STATE_SYNC;
	pPreCfgEntry->pMacEntry->wdev->PortSecured = WPA_802_1X_PORT_SECURED;
	MSDU_FORBID_CLEAR(pPreCfgEntry->pMacEntry->wdev, MSDU_FORBID_CONNECTION_NOT_READY);
	RTMP_OS_NETDEV_CARRIER_ON(pPreCfgEntry->pMacEntry->wdev->if_dev);

	pStaCfg->PrivacyFilter = pPreCfgEntry->pMacEntry->PrivacyFilter;
	pStaCfg->wdev.bLinkUpDone = TRUE;
	STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
	NdisGetSystemUpTime(&pStaCfg->ApcliInfStat.ApCliLinkUpTime);
	NdisMoveMemory(pStaCfg->Bssid, pPreCfgCtrl->PreCfgData.StaData.ApMacAddr, MAC_ADDR_LEN);
	NdisMoveMemory(pStaCfg->MlmeAux.Bssid, pPreCfgCtrl->PreCfgData.StaData.ApMacAddr, MAC_ADDR_LEN);

	pTrEntry->PortSecured = pPreCfgEntry->pMacEntry->wdev->PortSecured;
	pStaCfg->ApcliInfStat.Enable = TRUE;
	pStaCfg->ApcliInfStat.Valid = TRUE;
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

	return Ret;
}

NTSTATUS pre_cfg_peer_ap_connect(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	UINT32 k;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *pWdev = NULL;
	struct _STA_ADMIN_CONFIG *pStaCfg = NULL;
	struct _RTMP_ADAPTER *pCurrAd = NULL;
	struct ml_ie_info *pMlInfo = &pPreCfgCtrl->PreCfgData.StaData.ml_info;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	pStaCfg = pPreCfgCtrl->PreCfgData.StaData.pStaCfg;
	if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Pre Cfg STA is already connected!!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (pPreCfgCtrl->PreCfgData.StaData.u2MldStaIdx != MLD_STA_NONE) {
		pMlInfo->link_id = pPreCfgCtrl->PreCfgData.StaData.u2MldStaIdx;
		COPY_MAC_ADDR(pMlInfo->mld_addr, pPreCfgCtrl->PreCfgData.StaData.ApMacAddr);
		pre_cfg_mld_ap_add(&pStaCfg->wdev, pMlInfo);

		for (k = 0; k < BSS_MNGR_MAX_BAND_NUM; k++) {
			if (!pMlInfo->link[k].active)
				continue;

			pMacEntry = MacTableLookup(pAd, pMlInfo->link[k].link_addr);
			if (!IS_VALID_ENTRY(pMacEntry)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"pMacEntry is invalid!\n");
				continue;
			}
			/* set aid=1024 for MLD */
			pMacEntry->Aid = 0x400;

			pWdev = pMacEntry->wdev;
			pCurrAd = (struct _RTMP_ADAPTER *)pWdev->sys_handle;

			pPreCfgEntry->pMacEntry = pMacEntry;
			if (pPreCfgCtrl->PreCfgData.StaData.fgSecurity == 0) {
				SetWdevAuthMode(&pWdev->SecConfig, "OPEN");
				SetWdevEncrypMode(&pWdev->SecConfig, "NONE");
			}
			NdisMoveMemory(&pMacEntry->SecConfig, &pMacEntry->wdev->SecConfig, sizeof(struct _SECURITY_CONFIG));
			pre_cfg_peer_cap_default_by_wdev(pCurrAd, &pStaCfg->wdev, pMacEntry, &pPreCfgEntry->StaRecCfg);
			pStaCfg->StaActive.Aid = pMacEntry->Aid;

			/* Do sanity check and then linkup */
			if (PreCfg_Do_LinkUp(pCurrAd, pPreCfgCtrl, pPreCfgEntry) != NDIS_STATUS_SUCCESS)
				goto err;

			/* Load required PreCfg Configuration Set */
			PreCfg_Load_Config_Set(pCurrAd, pPreCfgCtrl, pPreCfgEntry);

			/* Do connection and update to WM through in-band command */
			if (PreCfg_Do_Connection(pCurrAd, pPreCfgCtrl, pPreCfgEntry) != NDIS_STATUS_SUCCESS)
				goto err;

		}
	} else {
		if (pPreCfgCtrl->PreCfgData.StaData.fgSecurity == 0) {
			SetWdevAuthMode(&pStaCfg->wdev.SecConfig, "OPEN");
			SetWdevEncrypMode(&pStaCfg->wdev.SecConfig, "NONE");
		}

		pMacEntry = MacTableInsertEntry(pAd,
									pPreCfgCtrl->PreCfgData.StaData.ApMacAddr,
									&pStaCfg->wdev,
									ENTRY_INFRA,
									OPMODE_STA,
									TRUE,
									pPreCfgCtrl->PreCfgData.StaData.u2MldStaIdx,
									NULL);

		if (IS_VALID_ENTRY(pMacEntry)) {
			pCurrAd = (struct _RTMP_ADAPTER *)pMacEntry->wdev->sys_handle;

			pPreCfgEntry->pMacEntry = pMacEntry;
			NdisMoveMemory(&pMacEntry->SecConfig, &pMacEntry->wdev->SecConfig, sizeof(struct _SECURITY_CONFIG));
			pre_cfg_peer_cap_default_by_wdev(pCurrAd, &pStaCfg->wdev, pMacEntry, &pPreCfgEntry->StaRecCfg);
			pStaCfg->StaActive.Aid = pMacEntry->Aid;

			/* Do sanity check and then linkup */
			if (PreCfg_Do_LinkUp(pCurrAd, pPreCfgCtrl, pPreCfgEntry) != NDIS_STATUS_SUCCESS)
				goto err;

			/* Load required PreCfg Configuration Set */
			PreCfg_Load_Config_Set(pCurrAd, pPreCfgCtrl, pPreCfgEntry);

			/* Do connection and update to WM through in-band command */
			if (PreCfg_Do_Connection(pCurrAd, pPreCfgCtrl, pPreCfgEntry) != NDIS_STATUS_SUCCESS)
				goto err;

		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"pMacEntry is invalid!\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;

err:
	MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);

	return NDIS_STATUS_FAILURE;
}

NTSTATUS pre_cfg_peer_ap_disconnect(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *pWdev = NULL;
	struct _STA_ADMIN_CONFIG *pStaCfg = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}

	pStaCfg = pPreCfgCtrl->PreCfgData.StaData.pStaCfg;
	if (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Pre Cfg STA is already disconnected!!\n");
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;
	if (IS_VALID_ENTRY(pMacEntry)) {
		pWdev = pMacEntry->wdev;

		if (pPreCfgCtrl->PeerOp.sys_link_down) {
			Ret = pPreCfgCtrl->PeerOp.sys_link_down(pAd, pPreCfgCtrl, pPreCfgEntry);
			if (Ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Failed to sys_link_down!\n");
				return NDIS_STATUS_FAILURE;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"sys_link_down is NULL hook!\n");
			return NDIS_STATUS_FAILURE;
		}
		RTMP_OS_NETDEV_CARRIER_OFF(pWdev->if_dev);
		pMacEntry->fgPreCfgRunning = FALSE;
		pPreCfgEntry->fgConnect = FALSE;
		pPreCfgEntry->pMacEntry = NULL;
		pWdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pStaCfg->wdev.bLinkUpDone = FALSE;
		pStaCfg->ApcliInfStat.Enable = FALSE;
		pStaCfg->ApcliInfStat.Valid = FALSE;
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
		MSDU_FORBID_SET(pWdev, MSDU_FORBID_CONNECTION_NOT_READY);
		MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (DlListLen(&pPreCfgCtrl->TimerList) != 0) {
		UINT8 cancelled;
		struct PRECFG_TIMER_CMD_CTRL *pPreCfgTimerCmdCtrl, *pPreCfgTimerCmdCtrlTmp;

		DlListForEachSafe(pPreCfgTimerCmdCtrl, pPreCfgTimerCmdCtrlTmp, &pPreCfgCtrl->TimerList, struct PRECFG_TIMER_CMD_CTRL, List) {
			if (pPreCfgTimerCmdCtrl) {
				MTWF_PRINT("%s: Delete Config Set (ID : %d) in periodic timer\n", __func__, pPreCfgTimerCmdCtrl->u2CfgSetIdx);
				RTMPCancelTimer(&pPreCfgTimerCmdCtrl->PreCfgTimer, &cancelled);
				DlListDel(&pPreCfgTimerCmdCtrl->List);
				os_free_mem(pPreCfgTimerCmdCtrl);
			}
		}
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_sys_sta_link_up(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry,
	IN struct _MAC_TABLE_ENTRY *pMacEntry)
{
	struct wifi_dev *pWdev = NULL;
	struct _STA_ADMIN_CONFIG *pStaCfg = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}

	if (IS_VALID_ENTRY(pMacEntry)) {
		pWdev = pMacEntry->wdev;
		pStaCfg = pPreCfgCtrl->PreCfgData.StaData.pStaCfg;

		HW_SET_SLOTTIME(pAd, TRUE, pWdev->channel, pWdev);
		if (wdev_do_linkup(pWdev, pMacEntry) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to wdev_do_linkup!\n");
			return NDIS_STATUS_FAILURE;
		}
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_sys_sta_link_down(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *pWdev = NULL;
	struct _STA_ADMIN_CONFIG *pStaCfg = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;
	if (IS_VALID_ENTRY(pMacEntry)) {
		pWdev = pMacEntry->wdev;
		pStaCfg = pPreCfgCtrl->PreCfgData.StaData.pStaCfg;

		if (wdev_do_linkdown(pWdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to wdev_do_linkdown!\n");
			return NDIS_STATUS_FAILURE;
		}
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS pre_cfg_peer_ap_sys_conn_act(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct PRECFG_CTRL *pPreCfgCtrl,
	IN struct PRECFG_ENTRY *pPreCfgEntry)
{
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;
	struct _STA_REC_CFG *pStaRecCfg = NULL;
	struct wifi_dev *pWdev = NULL;
	struct _MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct _STA_REC_CTRL_T *strec = NULL;

	if (!IS_PRE_CFG_ENTRY_VALID(pPreCfgEntry) ||
		!IS_PRE_CFG_ENTRY_PEER_AP(pPreCfgEntry)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg entry is invalid!\n");

		return NDIS_STATUS_FAILURE;
	}
	pMacEntry = pPreCfgEntry->pMacEntry;
	pWdev = pMacEntry->wdev;
	pStaRecCfg = &pPreCfgEntry->StaRecCfg;

	pStaRecCfg->ucBssIndex = pWdev->bss_info_argument.ucBssIndex;
	pStaRecCfg->MuarIdx = pWdev->bss_info_argument.OwnMacIdx;
	pStaRecCfg->u2WlanIdx = pMacEntry->wcid;
	pStaRecCfg->ConnectionType = pMacEntry->ConnectionType;
	pStaRecCfg->ConnectionState = STATE_PORT_SECURE;
	pStaRecCfg->IsNewSTARec = TRUE;
	pStaRecCfg->pEntry = pMacEntry;

/* Setup STAREC */
	tr_entry = tr_entry_get(pAd, pMacEntry->wcid);
	strec = &tr_entry->StaRec;
	strec->BssIndex = pWdev->bss_info_argument.ucBssIndex;
	strec->WlanIdx = pMacEntry->wcid;
	strec->ConnectionType = pMacEntry->ConnectionType;
	strec->ConnectionState = STATE_PORT_SECURE;
	strec->IsNewSTARec = TRUE;
	strec->priv = tr_entry;

	if (pPreCfgCtrl->PeerOp.peer_fw_update) {
		Ret = pPreCfgCtrl->PeerOp.peer_fw_update(pAd, pPreCfgCtrl, pPreCfgEntry, TRUE);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_fw_update!\n");
			return Ret;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"peer_fw_update is NULL hook!\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("%s ====> Done!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}
#endif /* APCLI_SUPPORT */

BOOLEAN pre_cfg_interface_init(
	IN struct _RTMP_ADAPTER *pAd,
	IN INT8 InfIdx,
	IN BOOLEAN fgApRole)
{
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;

	if (fgApRole) {
		BSS_STRUCT *pMbss = NULL;

		if (!VALID_MBSS(pAd, InfIdx))
			return FALSE;

		pMbss = &pAd->ApCfg.MBSSID[InfIdx];
		pPreCfgCtrl = &pMbss->PreCfgCtrl;
		os_zero_mem(pPreCfgCtrl, sizeof(*pPreCfgCtrl));
		pPreCfgCtrl->u1PreCfgType = PRE_CFG_ON_AP;
		pPreCfgCtrl->pPriv = (VOID *)pAd;
		pPreCfgCtrl->PreCfgData.ApData.pMbss = pMbss;
		pPreCfgCtrl->PeerOp.peer_sys_init = pre_cfg_peer_sta_sys_init;
		pPreCfgCtrl->PeerOp.peer_fw_update = pre_cfg_comm_fw_update;
		pPreCfgCtrl->PeerOp.peer_connect = pre_cfg_peer_sta_connect;
		pPreCfgCtrl->PeerOp.peer_disconnect = pre_cfg_peer_sta_disconnect;
		pPreCfgCtrl->PeerOp.peer_sys_conn_act = pre_cfg_peer_sta_sys_conn_act;
		pPreCfgCtrl->PeerOp.peer_sys_disconn_act = pre_cfg_peer_sta_sys_disconn_act;
		pPreCfgCtrl->PeerOp.sys_link_up = NULL; /* Only need for STA */
		pPreCfgCtrl->PeerOp.sys_link_down = NULL; /* Only need for STA */
		pPreCfgCtrl->fgEnable = TRUE;
		pPreCfgCtrl->PreCfgData.ApData.fgSecurity = 0;
		DlListInit(&pPreCfgCtrl->PreCfgData.ApData.EntryList);
		DlListInit(&pPreCfgCtrl->TimerList);
	}
#ifdef APCLI_SUPPORT
	else {
		PSTA_ADMIN_CONFIG pStaCfg;

		if (InfIdx >= MAX_MULTI_STA)
			return FALSE;

		pStaCfg = &pAd->StaCfg[InfIdx];
		pPreCfgCtrl = &pStaCfg->PreCfgCtrl;
		os_zero_mem(pPreCfgCtrl, sizeof(*pPreCfgCtrl));
		pPreCfgCtrl->u1PreCfgType = PRE_CFG_ON_STA;
		pPreCfgCtrl->pPriv = (VOID *)pAd;
		pPreCfgCtrl->PreCfgData.StaData.pStaCfg		= pStaCfg;
		pPreCfgCtrl->PeerOp.peer_sys_init			= pre_cfg_peer_ap_sys_init;
		pPreCfgCtrl->PeerOp.peer_fw_update			= pre_cfg_comm_fw_update;
		pPreCfgCtrl->PeerOp.peer_connect			= pre_cfg_peer_ap_connect;
		pPreCfgCtrl->PeerOp.peer_disconnect			= pre_cfg_peer_ap_disconnect;
		pPreCfgCtrl->PeerOp.peer_sys_conn_act		= pre_cfg_peer_ap_sys_conn_act;
		pPreCfgCtrl->PeerOp.peer_sys_disconn_act	= NULL;
		pPreCfgCtrl->PeerOp.sys_link_up				= pre_cfg_sys_sta_link_up;
		pPreCfgCtrl->PeerOp.sys_link_down			= pre_cfg_sys_sta_link_down;
		pPreCfgCtrl->fgEnable = TRUE;
		pPreCfgCtrl->PreCfgData.StaData.fgSecurity = 0;
		DlListInit(&pPreCfgCtrl->TimerList);
	}
#else
	else
		return FALSE;
#endif

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_INFO,
			"Pre cfg init successfully : %s (%d) !\n",
			(fgApRole)?"AP":"STA", InfIdx);

	return TRUE;
}

BOOLEAN pre_cfg_interface_deinit(
	IN struct _RTMP_ADAPTER *pAd,
	IN INT8 InfIdx,
	IN BOOLEAN fgApRole)
{
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;

	if (fgApRole) {
		BSS_STRUCT *pMbss = NULL;

		if (!VALID_MBSS(pAd, InfIdx))
			return FALSE;

		pMbss = &pAd->ApCfg.MBSSID[InfIdx];
		pPreCfgCtrl = &pMbss->PreCfgCtrl;

		if (pPreCfgCtrl->fgEnable) {
			pre_cfg_free_peer_all_sta(pAd, pMbss);
			os_zero_mem(pPreCfgCtrl, sizeof(*pPreCfgCtrl));
		}
	}
#ifdef APCLI_SUPPORT
	else {
		PSTA_ADMIN_CONFIG pStaCfg;
		struct PRECFG_ENTRY *pPreCfgEntry = NULL;

		if (InfIdx >= MAX_MULTI_STA)
			return FALSE;

		pStaCfg = &pAd->StaCfg[InfIdx];
		pPreCfgCtrl = &pStaCfg->PreCfgCtrl;
		if (pPreCfgCtrl->fgEnable) {
			pPreCfgEntry = pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry;
			if (pPreCfgEntry &&
				pPreCfgEntry->fgConnect &&
				pPreCfgCtrl->PeerOp.peer_disconnect)
				pPreCfgCtrl->PeerOp.peer_disconnect(pAd, pPreCfgCtrl, pPreCfgEntry);
			if (pPreCfgEntry != NULL)
				os_free_mem(pPreCfgEntry);
			os_zero_mem(pPreCfgCtrl, sizeof(*pPreCfgCtrl));
		}
	}
#else
	else
		return FALSE;
#endif

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_INFO,
			"Pre cfg de-init successfully : %s (%d) !\n",
			(fgApRole)?"AP":"STA", InfIdx);

	return TRUE;
}

struct PRECFG_ENTRY *pre_cfg_lookup_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _BSS_STRUCT *pMbss,
	IN UINT16 u2Wcid)
{
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;

	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMbss is NULL!\n");
		return NULL;
	}
	pPreCfgCtrl = &pMbss->PreCfgCtrl;

	if (!pPreCfgCtrl->fgEnable) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg is disabled!\n");
		return NULL;
	}

	if (VALID_UCAST_ENTRY_WCID(pAd, u2Wcid)) {
		struct _MAC_TABLE_ENTRY *pMacEntry = NULL;

		DlListForEach(pPreCfgEntry, &pPreCfgCtrl->PreCfgData.ApData.EntryList, struct PRECFG_ENTRY, List) {
			if (!pPreCfgEntry->fgValid)
				continue;
			pMacEntry = pPreCfgEntry->pMacEntry;
			if (IS_VALID_ENTRY(pMacEntry) && (pMacEntry->wcid == u2Wcid))
				return pPreCfgEntry;
		}
	}

	return NULL;
}

VOID pre_cfg_free_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _BSS_STRUCT *pMbss,
	IN UINT16 u2Wcid)
{
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL, *pPreCfgTmpEntry = NULL;
	struct _MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMbss is NULL!\n");
		return;
	}
	pPreCfgCtrl = &pMbss->PreCfgCtrl;

	if (DlListLen(&pPreCfgCtrl->PreCfgData.ApData.EntryList) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"No any peer sta in pre cfg entry list!\n");
		return;
	}

	DlListForEachSafe(pPreCfgEntry, pPreCfgTmpEntry, &pPreCfgCtrl->PreCfgData.ApData.EntryList, struct PRECFG_ENTRY, List) {
		if (!pPreCfgEntry->fgValid)
				continue;
		pMacEntry = pPreCfgEntry->pMacEntry;
		if (IS_VALID_ENTRY(pMacEntry) && (pMacEntry->wcid == u2Wcid)) {
			DlListDel(&pPreCfgEntry->List);
			os_free_mem(pPreCfgEntry);
			break;
		}
	}
}

VOID pre_cfg_free_peer_all_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _BSS_STRUCT *pMbss)
{
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL, *pPreCfgTmpEntry = NULL;
	struct _MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"pMbss is NULL!\n");
		return;
	}
	pPreCfgCtrl = &pMbss->PreCfgCtrl;

	if (DlListLen(&pPreCfgCtrl->PreCfgData.ApData.EntryList) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"No any peer sta in pre cfg entry list!\n");
		return;
	}

	DlListForEachSafe(pPreCfgEntry, pPreCfgTmpEntry, &pPreCfgCtrl->PreCfgData.ApData.EntryList, struct PRECFG_ENTRY, List) {
		if (pPreCfgEntry->fgValid && pPreCfgEntry->fgConnect) {
			pMacEntry = pPreCfgEntry->pMacEntry;
			pMacEntry->fgPreCfgRunning = FALSE;
			MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
		}
		DlListDel(&pPreCfgEntry->List);
		os_free_mem(pPreCfgEntry);
	}
}

INT pre_cfg_add_peer_mld_sta_info(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UINT32 mac_len = 0;
	UCHAR *pStrTemp = NULL;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	BSS_STRUCT *pMbss = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct ml_ie_info *pMlInfo = NULL;
	UINT16 u2linkId = BSS_MNGR_MAX_BAND_NUM;
	BOOLEAN bActive = FALSE;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pMbss = &pAd->ApCfg.MBSSID[u1InfIdx];
	pPreCfgCtrl = &pMbss->PreCfgCtrl;
	pMlInfo = &pPreCfgCtrl->PreCfgData.ApData.ml_info;

	pStrTemp = strsep(&arg, "-");
	pStrMacAddr = pStrTemp;
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing u2linkId!\n");
		return FALSE;
	}
	pStrTemp = strsep(&arg, "-");

	if (pStrTemp)
		u2linkId = simple_strtol(pStrTemp, 0, 10);

	if (u2linkId >= BSS_MNGR_MAX_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid u2linkId (%d)\n", u2linkId);
		return FALSE;
	}

	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing bActive!\n");
		return FALSE;
	}
	bActive = simple_strtol(arg, 0, 10);

	MTWF_PRINT("%s: MacAddr = "MACSTR", u2linkId = %d, bActive = %d!\n", __func__,
			MAC2STR(PeerMacAddr), u2linkId, bActive);

	pMlInfo->link[u2linkId].link_id = u2linkId;
	COPY_MAC_ADDR(pMlInfo->link[u2linkId].link_addr, PeerMacAddr);
	pMlInfo->link[u2linkId].active = bActive;

	return TRUE;
}

INT pre_cfg_add_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UINT16 u2CfgSetCount = 0, u2CfgSetID = 0;
	UINT16 u2ArrCfgSetInput[PRECFG_CFG_SET_BUF_MAX] = {0};
	UINT32 mac_len = 0;
	UCHAR *pStrTemp = NULL;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	BSS_STRUCT *pMbss = NULL;
	long tempValue = 0;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;
	UINT16 u2MldStaIdx;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pMbss = &pAd->ApCfg.MBSSID[u1InfIdx];
	pPreCfgCtrl = &pMbss->PreCfgCtrl;

	pStrTemp = strsep(&arg, "-");
	pStrMacAddr = pStrTemp;
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	pStrTemp = strsep(&arg, "-");
	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing Config Set Index!\n");
		return FALSE;
	}

	if (kstrtol(pStrTemp, 16, &tempValue)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Fail to get MLD STA Index!\n");
		return FALSE;
	}

	u2MldStaIdx = (UINT16)tempValue;

	do {
		pStrTemp = strsep(&arg, ",");
		if ((pStrTemp != NULL) && (kstrtol(pStrTemp, 10, &tempValue) == 0)) {
			u2CfgSetID = (UINT16)tempValue;
			pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pAd, u2CfgSetID);
			if (pPrecfgCfgSet == NULL) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"pPrecfgCfgSet is NULL!\n");
				return FALSE;
			}
			u2ArrCfgSetInput[u2CfgSetCount++] = u2CfgSetID;
		}
	} while (arg && (strlen(arg) > 0));

	MTWF_PRINT("%s: MacAddr = "MACSTR", u2MldStaIdx = 0x%x!\n", __func__,
			MAC2STR(PeerMacAddr), u2MldStaIdx);

	for (i = 0; i < u2CfgSetCount; i++) {
		if (u2ArrCfgSetInput[i] == PRECFG_CFGSET_SEC)
			pPreCfgCtrl->PreCfgData.ApData.fgSecurity = 1;
		MTWF_PRINT("%s: Config Set Input[%d] = %d\n", __func__, i, u2ArrCfgSetInput[i]);
	}

	if (pPreCfgCtrl->PeerOp.peer_sys_init) {
		Ret = pPreCfgCtrl->PeerOp.peer_sys_init(pAd, pPreCfgCtrl, PeerMacAddr,
								u2MldStaIdx, u2ArrCfgSetInput, u2CfgSetCount);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_sys_init!\n");
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"peer_sys_init is NULL hook!\n");
		return FALSE;
	}

	return TRUE;
}

INT pre_cfg_del_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	UINT32 mac_len = 0;
	BSS_STRUCT *pMbss = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	struct ml_ie_info *pMlInfo = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pMbss = &pAd->ApCfg.MBSSID[u1InfIdx];
	pPreCfgCtrl = &pMbss->PreCfgCtrl;
	pMlInfo = &pPreCfgCtrl->PreCfgData.ApData.ml_info;
	pStrMacAddr = arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	pMacEntry = MacTableLookup(pAd, PeerMacAddr);
	if (IS_VALID_ENTRY(pMacEntry)) {
		pPreCfgEntry = pre_cfg_lookup_peer_sta(pAd, pMbss, pMacEntry->wcid);
		if (pPreCfgEntry == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg look up Mac Entry("MACSTR") failed!\n", MAC2STR(PeerMacAddr));
			return FALSE;
		}

		if (pPreCfgEntry->mld_sta_idx != MLD_STA_NONE) {
			for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
				if (pMlInfo->link[i].active) {
					pMacEntry = MacTableLookup(pAd, pMlInfo->link[i].link_addr);
					if (IS_VALID_ENTRY(pMacEntry)) {
						pMacEntry->fgPreCfgRunning = FALSE;
						pre_cfg_free_peer_sta(pAd, pMbss, pMacEntry->wcid);
						bss_mngr_mld_disconn_op(pMacEntry->wdev,
									pPreCfgEntry->mld_sta_idx,
									MLD_DISC_OP_DEL_STA_N_ACT);
						MTWF_PRINT("%s: Pre cfg del Mac Entry("MACSTR") successfully!\n",
									__func__, MAC2STR(pMlInfo->link[i].link_addr));
					}
				}
			}
			os_zero_mem(pMlInfo, sizeof(*pMlInfo));
		} else {
			pMacEntry->fgPreCfgRunning = FALSE;
			pre_cfg_free_peer_sta(pAd, pMbss, pMacEntry->wcid);
			MacTableDeleteEntry(pAd, pMacEntry->wcid, PeerMacAddr);
			MTWF_PRINT("%s: Pre cfg del Mac Entry("MACSTR") successfully!\n",
						__func__, MAC2STR(PeerMacAddr));
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Please add entry first!\n");
		return FALSE;
	}

	return TRUE;
}

INT pre_cfg_connect_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	UINT32 mac_len = 0;
	BSS_STRUCT *pMbss = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	struct ml_ie_info *pMlInfo = NULL;
	struct _RTMP_ADAPTER *pCurrAd = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pMbss = &pAd->ApCfg.MBSSID[u1InfIdx];
	pPreCfgCtrl = &pMbss->PreCfgCtrl;
	pMlInfo = &pPreCfgCtrl->PreCfgData.ApData.ml_info;

	pStrMacAddr = strsep(&arg, "-");
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	pMacEntry = MacTableLookup(pAd, PeerMacAddr);
	if (IS_VALID_ENTRY(pMacEntry)) {
		pPreCfgEntry = pre_cfg_lookup_peer_sta(pAd, pMbss, pMacEntry->wcid);
		if (pPreCfgEntry == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg look up Mac Entry("MACSTR") failed!\n", MAC2STR(PeerMacAddr));
			return FALSE;
		}

		if (pPreCfgEntry->fgConnect) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Already connected!\n");
			return FALSE;
		}

		if (pPreCfgEntry->mld_sta_idx != MLD_STA_NONE) {
			for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
				if (pMlInfo->link[i].active) {
					pMacEntry = MacTableLookup(pAd, pMlInfo->link[i].link_addr);
					if (IS_VALID_ENTRY(pMacEntry)) {
						pPreCfgEntry = pre_cfg_lookup_peer_sta(pAd, pMbss, pMacEntry->wcid);
						pCurrAd = (struct _RTMP_ADAPTER *)pMacEntry->wdev->sys_handle;
						if (pPreCfgEntry == NULL) {
							MTWF_DBG(pCurrAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"Pre cfg look up Mac Entry("MACSTR") failed!\n", MAC2STR(PeerMacAddr));
							return FALSE;
						}

						if (pPreCfgEntry->fgConnect) {
							MTWF_DBG(pCurrAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"Already connected!\n");
							return FALSE;
						}

						if (pPreCfgCtrl->PeerOp.peer_connect) {
							Ret = pPreCfgCtrl->PeerOp.peer_connect(pCurrAd, pPreCfgCtrl, pPreCfgEntry);
							if (Ret != NDIS_STATUS_SUCCESS) {
								MTWF_DBG(pCurrAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
										"Failed to peer_connect!\n");
								return FALSE;
							}
						} else {
							MTWF_DBG(pCurrAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"peer_connect is NULL hook!\n");
							return FALSE;
						}
						MTWF_PRINT("%s: Pre cfg connect to Mac Entry("MACSTR") successfully!\n",
									__func__, MAC2STR(pMacEntry->Addr));
					} else {
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"pMacEntry is invalid!\n");
						return FALSE;
					}
				}
			}
		} else {
			if (pPreCfgCtrl->PeerOp.peer_connect) {
				Ret = pPreCfgCtrl->PeerOp.peer_connect(pAd, pPreCfgCtrl, pPreCfgEntry);
				if (Ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Failed to peer_connect!\n");
					return FALSE;
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"peer_connect is NULL hook!\n");
				return FALSE;
			}
			MTWF_PRINT("%s: Pre cfg connect to Mac Entry("MACSTR") successfully!\n",
						__func__, MAC2STR(PeerMacAddr));
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return FALSE;
	}

	return TRUE;
}

INT pre_cfg_disconnect_peer_sta(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	UINT32 mac_len = 0;
	BSS_STRUCT *pMbss = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	struct ml_ie_info *pMlInfo = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pMbss = &pAd->ApCfg.MBSSID[u1InfIdx];
	pPreCfgCtrl = &pMbss->PreCfgCtrl;
	pMlInfo = &pPreCfgCtrl->PreCfgData.ApData.ml_info;

	pStrMacAddr = strsep(&arg, "-");
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	pMacEntry = MacTableLookup(pAd, PeerMacAddr);
	if (IS_VALID_ENTRY(pMacEntry)) {
		pPreCfgEntry = pre_cfg_lookup_peer_sta(pAd, pMbss, pMacEntry->wcid);
		if (pPreCfgEntry == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Pre cfg look up Mac Entry("MACSTR") failed!\n", MAC2STR(PeerMacAddr));
			return FALSE;
		}

		if (!pPreCfgEntry->fgConnect) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Already disconnected!\n");
			return FALSE;
		}

		if (pPreCfgEntry->mld_sta_idx != MLD_STA_NONE) {
			for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
				if (pMlInfo->link[i].active) {
					pMacEntry = MacTableLookup(pAd, pMlInfo->link[i].link_addr);
					if (IS_VALID_ENTRY(pMacEntry)) {
						pPreCfgEntry = pre_cfg_lookup_peer_sta(pAd, pMbss, pMacEntry->wcid);
						if (pPreCfgEntry == NULL) {
							MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
								"Pre cfg look up Mac Entry("MACSTR") failed!\n", MAC2STR(PeerMacAddr));
							return FALSE;
						}

						if (!pPreCfgEntry->fgConnect) {
							MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"Already disconnected!\n");
							return FALSE;
						}

						if (pPreCfgCtrl->PeerOp.peer_disconnect) {
							Ret = pPreCfgCtrl->PeerOp.peer_disconnect(pAd, pPreCfgCtrl, pPreCfgEntry);
							if (Ret != NDIS_STATUS_SUCCESS) {
								MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
										"Failed to peer_disconnect!\n");
								return FALSE;
							}
							MTWF_PRINT("%s: Pre cfg disconnect to Mac Entry("MACSTR") successfully!\n",
										__func__, MAC2STR(pMacEntry->Addr));
						} else {
							MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"peer_disconnect is NULL hook!\n");
							return FALSE;
						}
					} else {
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
									"pMacEntry is invalid!\n");
						return FALSE;
					}
				}
			}
		} else {
			if (pPreCfgCtrl->PeerOp.peer_disconnect) {
				Ret = pPreCfgCtrl->PeerOp.peer_disconnect(pAd, pPreCfgCtrl, pPreCfgEntry);
				if (Ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Failed to peer_disconnect!\n");
					return FALSE;
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"peer_disconnect is NULL hook!\n");
				return FALSE;
			}
			MTWF_PRINT("%s: Pre cfg disconnect to Mac Entry("MACSTR") successfully!\n",
						__func__, MAC2STR(PeerMacAddr));
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return FALSE;
	}

	return TRUE;
}

#ifdef APCLI_SUPPORT
INT pre_cfg_add_peer_mld_ap_info(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UINT32 mac_len = 0;
	UCHAR *pStrTemp = NULL;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct ml_ie_info *pMlInfo = NULL;
	UINT16 u2linkId = BSS_MNGR_MAX_BAND_NUM;
	BOOLEAN bActive = FALSE;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pStaCfg = &pAd->StaCfg[u1InfIdx];
	pPreCfgCtrl = &pStaCfg->PreCfgCtrl;
	pMlInfo = &pPreCfgCtrl->PreCfgData.StaData.ml_info;

	pStrTemp = strsep(&arg, "-");
	pStrMacAddr = pStrTemp;
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing u2linkId!\n");
		return FALSE;
	}
	pStrTemp = strsep(&arg, "-");

	if (pStrTemp)
		u2linkId = simple_strtol(pStrTemp, 0, 10);

	if (u2linkId >= BSS_MNGR_MAX_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid u2linkId (%d)\n", u2linkId);
		return FALSE;
	}

	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing bActive!\n");
		return FALSE;
	}
	bActive = simple_strtol(arg, 0, 10);

	MTWF_PRINT("%s: MacAddr = "MACSTR", u2linkId = %d, bActive = %d!\n", __func__,
			MAC2STR(PeerMacAddr), u2linkId, bActive);

	pMlInfo->link[u2linkId].link_id = u2linkId;
	COPY_MAC_ADDR(pMlInfo->link[u2linkId].link_addr, PeerMacAddr);
	pMlInfo->link[u2linkId].active = bActive;

	return TRUE;
}

INT pre_cfg_add_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	UINT8 i = 0;
	UINT16 u2CfgSetCount = 0, u2CfgSetID = 0;
	UINT16 u2ArrCfgSetInput[PRECFG_CFG_SET_BUF_MAX] = {0};
	UINT32 mac_len = 0;
	UCHAR *pStrTemp = NULL;
	UCHAR *pStrMacAddr = NULL;
	UCHAR PeerMacAddr[MAC_ADDR_LEN];
	long tempValue = 0;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_CFG_SET *pPrecfgCfgSet = NULL;
	UINT16 u2MldStaIdx;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	if (!arg || strlen(arg) == 0)
		return FALSE;

	MTWF_PRINT("%s: arg: %s\n", __func__, arg);

	pStaCfg = &pAd->StaCfg[u1InfIdx];
	pPreCfgCtrl = &pStaCfg->PreCfgCtrl;

	pStrTemp = strsep(&arg, "-");
	pStrMacAddr = pStrTemp;
	if (!pStrMacAddr)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(pStrMacAddr);
	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(pStrMacAddr, "00:00:00:00:00:00")) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"invalid mac setting\n");
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++, pStrMacAddr += 3)
		AtoH(pStrMacAddr, &PeerMacAddr[i], 1);

	pStrTemp = strsep(&arg, "-");
	if (strlen(arg) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Missing Config Set Index!\n");
		return FALSE;
	}

	if (kstrtol(pStrTemp, 16, &tempValue)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"Fail to get MLD STA Index!\n");
		return FALSE;
	}

	u2MldStaIdx = (UINT16)tempValue;

	do {
		pStrTemp = strsep(&arg, ",");
		if ((pStrTemp != NULL) && (kstrtol(pStrTemp, 10, &tempValue) == 0)) {
			u2CfgSetID = (UINT16)tempValue;
			pPrecfgCfgSet = PreCfg_Lookup_ConfigSet(pAd, u2CfgSetID);
			if (pPrecfgCfgSet == NULL) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"pPrecfgCfgSet is NULL!\n");
				return FALSE;
			}
			u2ArrCfgSetInput[u2CfgSetCount++] = u2CfgSetID;
		}
	} while (arg && (strlen(arg) > 0));

	MTWF_PRINT("%s: MacAddr = "MACSTR", u2MldStaIdx = 0x%x!\n",
			__func__, MAC2STR(PeerMacAddr), u2MldStaIdx);

	for (i = 0; i < u2CfgSetCount; i++) {
		if (u2ArrCfgSetInput[i] == PRECFG_CFGSET_SEC)
			pPreCfgCtrl->PreCfgData.StaData.fgSecurity = 1;
		MTWF_PRINT("%s: Config Set Input[%d] = %d\n", __func__, i, u2ArrCfgSetInput[i]);
	}

	if (pPreCfgCtrl->PeerOp.peer_sys_init) {
		Ret = pPreCfgCtrl->PeerOp.peer_sys_init(pAd, pPreCfgCtrl, PeerMacAddr,
									u2MldStaIdx, u2ArrCfgSetInput, u2CfgSetCount);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_sys_init!\n");
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"peer_sys_init is NULL hook!\n");
		return FALSE;
	}

	pPreCfgCtrl->PreCfgData.StaData.fgValid = TRUE;
	MTWF_PRINT("%s: Pre cfg add Mac Entry("MACSTR") successfully!\n",
				__func__, MAC2STR(PeerMacAddr));

	return TRUE;
}

INT pre_cfg_del_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UCHAR PeerApMacAddr[MAC_ADDR_LEN];
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	pStaCfg = &pAd->StaCfg[u1InfIdx];
	pPreCfgCtrl = &pStaCfg->PreCfgCtrl;

	if (!pPreCfgCtrl->PreCfgData.StaData.fgValid) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Please add entry first!\n");
		return FALSE;
	}
	pPreCfgEntry = pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry;
	if (pPreCfgEntry != NULL) {
		pMacEntry = pPreCfgEntry->pMacEntry;
		COPY_MAC_ADDR(PeerApMacAddr, pPreCfgCtrl->PreCfgData.StaData.ApMacAddr);

		if (IS_VALID_ENTRY(pMacEntry)) {
			if (pPreCfgCtrl->PeerOp.peer_disconnect) {
				Ret = pPreCfgCtrl->PeerOp.peer_disconnect(pAd, pPreCfgCtrl, pPreCfgEntry);
				if (Ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Failed to peer_disconnect!\n");
					return FALSE;
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"peer_disconnect is NULL hook!\n");
				return FALSE;
			}
		}

		os_free_mem(pPreCfgEntry);
		pPreCfgCtrl->PreCfgData.StaData.fgValid = FALSE;
		pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry = NULL;
		os_zero_mem(pPreCfgCtrl->PreCfgData.StaData.ApMacAddr, MAC_ADDR_LEN);
		MTWF_PRINT("%s: Pre cfg del Mac Entry("MACSTR") successfully!\n",
				__func__, MAC2STR(PeerApMacAddr));
	}

	return TRUE;
}

INT pre_cfg_connect_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	UCHAR *pPeerApMacAddr = NULL;
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	pStaCfg = &pAd->StaCfg[u1InfIdx];
	pPreCfgCtrl = &pStaCfg->PreCfgCtrl;
	pPreCfgEntry = pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry;
	if (!pPreCfgCtrl->PreCfgData.StaData.fgValid) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Please add entry first!\n");
		return FALSE;
	}

	if (pPreCfgEntry->fgConnect) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Already connected!\n");
		return FALSE;
	}

	pPeerApMacAddr = pPreCfgCtrl->PreCfgData.StaData.ApMacAddr;

	if (pPreCfgCtrl->PeerOp.peer_connect) {
		Ret = pPreCfgCtrl->PeerOp.peer_connect(pAd, pPreCfgCtrl, pPreCfgEntry);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Failed to peer_connect!\n");
			return FALSE;
		}

		MTWF_PRINT("%s: Pre cfg connect to Mac Entry("MACSTR") successfully!\n",
				__func__, MAC2STR(pPeerApMacAddr));
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"peer_connect is NULL hook!\n");
		return FALSE;
	}

	return TRUE;
}

INT pre_cfg_disconnect_peer_ap(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN UINT8 u1InfIdx)
{
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_ENTRY *pPreCfgEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UCHAR PeerApMacAddr[MAC_ADDR_LEN];
	NTSTATUS Ret = NDIS_STATUS_SUCCESS;

	pStaCfg = &pAd->StaCfg[u1InfIdx];
	pPreCfgCtrl = &pStaCfg->PreCfgCtrl;
	pPreCfgEntry = pPreCfgCtrl->PreCfgData.StaData.pPreCfgEntry;
	if (!pPreCfgCtrl->PreCfgData.StaData.fgValid) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Please add entry first!\n");
		return FALSE;
	}

	if (!pPreCfgEntry->fgConnect) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"Already disconnected!\n");
		return FALSE;
	}

	pMacEntry = pPreCfgEntry->pMacEntry;
	if (IS_VALID_ENTRY(pMacEntry)) {
		COPY_MAC_ADDR(PeerApMacAddr, pMacEntry->Addr);
		if (pPreCfgCtrl->PeerOp.peer_disconnect) {
			Ret = pPreCfgCtrl->PeerOp.peer_disconnect(pAd, pPreCfgCtrl, pPreCfgEntry);
			if (Ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						"Failed to peer_disconnect!\n");
				return FALSE;
			}

			MTWF_PRINT("%s: Pre cfg disconnect to Mac Entry("MACSTR") successfully!\n",
					__func__, MAC2STR(PeerApMacAddr));
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"peer_disconnect is NULL hook!\n");
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pMacEntry is invalid!\n");
		return FALSE;
	}

	return TRUE;
}

#endif /* APCLI_SUPPORT */

struct pre_cfg_proc_tbl pre_cfg_tbl[] = {
	{PRE_CFG_ON_AP, "ap", ap_set_pre_cfg_tbl},
#ifdef APCLI_SUPPORT
	{PRE_CFG_ON_STA, "sta", sta_set_pre_cfg_tbl},
#endif /* APCLI_SUPPORT */
};

INT	_Set_PreCfg_Peer_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg,
	IN INT8 InfIdx,
	IN BOOLEAN fgApRole)
{
	UINT32 i, j, array_size = 0;
	UCHAR *role = NULL, *action = NULL;
	struct PRECFG_CTRL *pPreCfgCtrl = NULL;
	struct PRECFG_CMD *pCmdTbl = NULL;

	if (fgApRole) {
		BSS_STRUCT *pMbss = NULL;

		if (!VALID_MBSS(pAd, InfIdx))
			return FALSE;

		pMbss = &pAd->ApCfg.MBSSID[InfIdx];
		pPreCfgCtrl = &pMbss->PreCfgCtrl;

		if (!pPreCfgCtrl->fgEnable ||
			(pPreCfgCtrl->u1PreCfgType != PRE_CFG_ON_AP)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pre cfg isn't correctly initialized!\n");
			return FALSE;
		}
	}
#ifdef APCLI_SUPPORT
	else {
		PSTA_ADMIN_CONFIG pStaCfg;

		if (InfIdx >= MAX_MULTI_STA)
			return FALSE;

		pStaCfg = &pAd->StaCfg[InfIdx];
		pPreCfgCtrl = &pStaCfg->PreCfgCtrl;

		if (!pPreCfgCtrl->fgEnable ||
			(pPreCfgCtrl->u1PreCfgType != PRE_CFG_ON_STA)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					"pre cfg isn't correctly initialized!\n");
			return FALSE;
		}
	}
#else
	else
		return FALSE;
#endif

	role = strsep(&arg, "-");
	if ((role != NULL) && (strlen(role) > 0)) {
		MTWF_PRINT("%s: role: %s, arg: %s\n", __func__, role, arg);

		for (i = 0; i < ARRAY_SIZE(pre_cfg_tbl); i++) {
			if (strcmp((pre_cfg_tbl + i)->target, role) != 0)
				continue;

			MTWF_PRINT("%s: match for role (%d)!\n",
						__func__, (pre_cfg_tbl + i)->u1PreCfgType);

			/* Sanity check - Role of config target */
			if (pPreCfgCtrl->u1PreCfgType != (pre_cfg_tbl + i)->u1PreCfgType) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Please use correct wifi interface to config ap or sta!\n");
				return FALSE;
			}

			pCmdTbl = (pre_cfg_tbl + i)->cmd_tbl;

			/* Sanity check - Input argument */
			if ((pCmdTbl == NULL) || (strlen(arg) <= 0)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Need to add more options for operation!\n");
				return FALSE;
			}

#ifdef APCLI_SUPPORT
			if ((pre_cfg_tbl + i)->u1PreCfgType == PRE_CFG_ON_STA)
				array_size = ARRAY_SIZE(sta_set_pre_cfg_tbl);
			else
#endif /* APCLI_SUPPORT */
				array_size = ARRAY_SIZE(ap_set_pre_cfg_tbl);

			action = strsep(&arg, "-");

			/* Sanity check - Precfg action */
			if ((action == NULL) || (strlen(action) <= 0)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
							"Invalid PreCfg action input!\n");
				return FALSE;
			}

			MTWF_PRINT("%s: array_size: %d, action: %s, arg: %s\n",
						__func__, array_size, action, arg);

			for (j = 0; j < array_size; j++, pCmdTbl++) {
				if ((pCmdTbl == NULL) || (strcmp(pCmdTbl->action, action) != 0))
					continue;

				MTWF_PRINT("%s: match for action!\n", __func__);

				if (pCmdTbl->action_handler &&
					pCmdTbl->action_handler(pAd, arg, InfIdx) != TRUE)
					return FALSE;
			}
		}
	}

	return TRUE;
}

VOID _Show_PreCfg_Help_Proc(VOID)
{
	MTWF_PRINT
		("\tPlease follow correct input as below:\n");
	MTWF_PRINT
		("\tIf Peer is STA:\n");
		MTWF_PRINT
			("\t\tAdd Fake MLD Link for Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=ap-mld_link_add-");
		MTWF_PRINT
			("[MAC]-[MLD Link ID]-[Active Status]\n");
		MTWF_PRINT
			("\t\tAdd Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=ap-add-");
		MTWF_PRINT
			("[Peer's MAC]-[MLD Index]-[Configure Set ID 0],[Configure Set ID 1]...\n");
		MTWF_PRINT
			("\t\tDelete Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=ap-del-");
		MTWF_PRINT
			("[Peer's MAC]-[MLD Index]\n");
		MTWF_PRINT
			("\t\tConnect to Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=ap-connect-[Peer's MAC]\n");
		MTWF_PRINT
			("\t\tDisconnect to Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=ap-disconnect-[Peer's MAC]\n");

		MTWF_PRINT
			("\t\tExample:\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-add-06:0C:43:26:60:28-0xffff-0,1\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-connect-06:0C:43:26:60:28\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-disconnect-06:0C:43:26:60:28\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-del-06:0C:43:26:60:28\n");

		MTWF_PRINT
			("\t\tMLO Example:\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-mld_link_add-");
		MTWF_PRINT
			("06:0C:43:26:60:08-0-1\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-mld_link_add-");
		MTWF_PRINT
			("06:0C:43:26:50:10-1-1\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-add-06:0C:43:26:60:08-0-0,1\n");
		MTWF_PRINT
			("\t\t\tiwpriv ra0 set precfg_peer=ap-connect-06:0C:43:26:60:08\n");

	MTWF_PRINT
		("\tIf Peer is AP:\n");
		MTWF_PRINT
			("\t\tAdd Fake MLD Link for Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=sta-mld_link_add-");
		MTWF_PRINT
			("[MAC]-[MLD Link ID]-[Active Status]\n");
		MTWF_PRINT
			("\t\tAdd Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=sta-add-");
		MTWF_PRINT
			("[Peer's MAC]-[MLD Index]-[Configure Set ID 0],[Configure Set ID 1]...\n");
		MTWF_PRINT
			("\t\tDelete Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=sta-del\n");
		MTWF_PRINT
			("\t\tConnect to Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=sta-connect\n");
		MTWF_PRINT
			("\t\tDisconnect to Peer:\n");
		MTWF_PRINT
			("\t\t\tiwpriv $(inf_name) set precfg_peer=sta-disconnect\n");

		MTWF_PRINT
			("\t\tExample:\n");
		MTWF_PRINT
			("\t\t\tiwpriv apcli0 set precfg_peer=sta-add-");
		MTWF_PRINT
			("00:0C:43:26:60:30-0xffff-0,1\n");
		MTWF_PRINT
			("\t\t\tiwpriv apcli0 set precfg_peer=sta-connect\n");
		MTWF_PRINT
			("\t\t\tiwpriv apcli0 set precfg_peer=sta-disconnect\n");
		MTWF_PRINT
			("\t\t\tiwpriv apcli0 set precfg_peer=sta-del\n");

		MTWF_PRINT
			("\t\tMLO Example:\n");
		MTWF_PRINT
			("\t\t\tTBD\n");
}

INT	Set_PreCfg_Peer_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	INT ret;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;

	if ((strlen(arg) == 0) || (strstr(arg, "-") == NULL))
		goto err;

	if (pObj->ioctl_if_type == INT_APCLI)
		ret = _Set_PreCfg_Peer_Proc(pAd, arg, ifIndex, FALSE);
	else
		ret = _Set_PreCfg_Peer_Proc(pAd, arg, ifIndex, TRUE);
	if (ret != TRUE)
		goto err;

	return ret;

err:
	_Show_PreCfg_Help_Proc();
	return TRUE;
}

/******************************************************************************
 * Set_Precfg_Negotiation_Param_Proc:
 *     This function is used to set up parameters for fake negotiation
 ******************************************************************************/

VOID Show_Precfg_Negotiation_Param_Proc(
	VOID)
{
	UINT8 u1Idx = 0;

	MTWF_PRINT("Show current fake nego parameters:\n");

	for (u1Idx = 0; u1Idx < ARRAY_SIZE(g_arPrecfgFakeNegoParam); u1Idx++) {
		MTWF_PRINT("[Config Done : %s][%s][0x%x]\n",
			g_arPrecfgFakeNegoParam[u1Idx].fgIsCfgDone ? "Y" : "N",
			g_arPrecfgFakeNegoParam[u1Idx].pParamStr,
			g_arPrecfgFakeNegoParam[u1Idx].u4Value);
	}
}

VOID Set_Clear_Precfg_Negotiation_Param_Proc(
	VOID)
{
	UINT8 u1Idx = 0;

	MTWF_PRINT("Clear fake nego parameters\n");
	for (u1Idx = 0; u1Idx < ARRAY_SIZE(g_arPrecfgFakeNegoParam); u1Idx++) {
		g_arPrecfgFakeNegoParam[u1Idx].u4Value = 0x0;
		g_arPrecfgFakeNegoParam[u1Idx].fgIsCfgDone = FALSE;
	}
	Show_Precfg_Negotiation_Param_Proc();
}

INT Set_PreCfg_Negotiation_Param_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	INT			ret			= FALSE;
	//POS_COOKIE	pObj		= (POS_COOKIE) pAd->OS_Cookie;
	//UINT32		ifIndex		= pObj->ioctl_if;
	PUCHAR		param_str	= NULL;
	PUCHAR		val_str		= NULL;
	UINT8		u1Idx		= 0;
	UINT32		u4Val		= 0;

	/* Command sanity check */
	if (arg == NULL)
		goto INVALID_CMD;

	param_str = strsep(&arg, "-");
	val_str = strsep(&arg, "-");
	if ((param_str == NULL) || (val_str == NULL))
		goto INVALID_CMD;

	/* Handle parameter setting */
	u4Val = os_str_toul(val_str, 0, 10);

	for (u1Idx = 0; u1Idx < ARRAY_SIZE(g_arPrecfgFakeNegoParam); u1Idx++) {
		if (strcmp(g_arPrecfgFakeNegoParam[u1Idx].pParamStr, param_str) == 0) {
			MTWF_PRINT("%s() Set %s as %s\n", __func__, param_str, val_str);
			g_arPrecfgFakeNegoParam[u1Idx].u4Value = u4Val;
			g_arPrecfgFakeNegoParam[u1Idx].fgIsCfgDone = TRUE;
		}
	}
	Show_Precfg_Negotiation_Param_Proc();
	return ret;

INVALID_CMD:
	ret = TRUE;
	MTWF_PRINT("%s() Invalid input. (Set %s as %s)\n", __func__, param_str, val_str);
	return ret;
}

#endif /* PRE_CFG_SUPPORT */
