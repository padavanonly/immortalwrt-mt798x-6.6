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
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"

/* DEV Info */
INT32 MtAsicSetDevMacByFw(
	RTMP_ADAPTER * pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT64 EnableFeature)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdDevInfoUpdate(pAd,
							   OwnMacIdx,
							   OwnMacAddr,
							   BandIdx,
							   Active,
							   EnableFeature);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtDevInfoUpdate(pAd,
							   OwnMacIdx,
							   OwnMacAddr,
							   BandIdx,
							   Active,
							   EnableFeature);
	}
}


/* BSS Info */
INT32 MtAsicSetBssidByFw(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdBssInfoUpdate(pAd, bss_info_argument);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtBssInfoUpdate(pAd, bss_info_argument);
	}
}

#ifdef DOT11_EHT_BE
INT32 MtAsicUpdateDscbInfo(
	struct _RTMP_ADAPTER *pAd,
	struct CMD_STATIC_PP_DSCB_T *dscb)
{
#ifdef WIFI_UNIFIED_COMMAND
#ifdef CFG_SUPPORT_FALCON_PP
	PP_CMD_T pp_cmd_cap;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	os_zero_mem(&pp_cmd_cap, sizeof(pp_cmd_cap));
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_DSCB_CTRL;
	if (pChipCap->uni_cmd_support) {
		return UniCmdPPCapCtrl(pAd, &pp_cmd_cap, dscb, NULL, NULL);
	} else
#endif
#endif /* WIFI_UNIFIED_COMMAND */
	return NDIS_STATUS_FAILURE;
}
#endif

/* STARec Info */
INT32 MtAsicSetStaRecByFw(
	RTMP_ADAPTER * pAd,
	STA_REC_CFG_T *pStaCfg)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecUpdate(pAd, pStaCfg);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecUpdate(pAd, pStaCfg);
	}

}

INT32 MtAsicUpdateStaRecBaByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecBaUpdate(pAd, StaRecBaCfg);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecBaUpdate(pAd, StaRecBaCfg);
	}
}


VOID MtSetTmrCRByFw(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	CmdExtSetTmrCR(pAd, enable, BandIdx);
}


VOID AsicAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout)
{
#ifdef WIFI_UNIFIED_COMMAND
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_SDO_ONLY)) {
		UniCmdSetSdoAutoBA(pAd, Enable, Timeout);
		return;
	}
#endif /* WIFI_UNIFIED_COMMAND */

	CmdAutoBATrigger(pAd, Enable, Timeout);
}


VOID MtAsicDelWcidTabByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO, "--->\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO, "wcid_idx(%d)\n", wcid_idx);

	if (wcid_idx == WCID_ALL)
		CmdExtWtblUpdate(pAd, 0, RESET_ALL_WTBL, NULL, 0);
	else
		CmdExtWtblUpdate(pAd, wcid_idx, RESET_WTBL_AND_SET, NULL, 0);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO, "<---\n");
}


#ifdef HTC_DECRYPT_IOT
/*
	Old Chip PATH (ex: MT7615 / MT7622 ) :
*/
VOID MtAsicSetWcidAAD_OMByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx,
	IN UCHAR value)
{
	UINT32 mask = 0xfffffff7;
	UINT32 val;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"wcid_idx(%d), value(%d)\n", wcid_idx, value);

	if (value) {
		val = 0x8;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	} else {
		val = 0x0;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO, "<---\n");
}

/*
	CONNAC F/W CMD PATH:
*/
INT32 MtAsicUpdateStaRecAadOmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT8 AadOm)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecAADOmUpdate(pAd, Wcid, AadOm);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecAADOmUpdate(pAd, Wcid, AadOm);
	}

}

#endif /* HTC_DECRYPT_IOT */

INT32 MtAsicUpdateStaRecPsmByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UINT8 Psm)
{
	INT32 ret;

#ifdef WIFI_UNIFIED_COMMAND
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		ret = UniCmdStaRecPsmUpdate(pAd, Wcid, Psm);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		ret = CmdExtStaRecPsmUpdate(pAd, Wcid, Psm);
	}

	return ret;
}

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID MtAsicSetWcid4Addr_HdrTransByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx,
	IN UCHAR IsEnable,
	IN UCHAR IsApcliEntry)
{

	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};

	rWtblHdrTrans.u2Tag = WTBL_HDR_TRANS;
	rWtblHdrTrans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
						"WCID %u ISenable %u\n",
						wcid_idx, IsEnable);
	/*Set to 1 */
	if (IsEnable) {
		rWtblHdrTrans.ucTd = 1;
		rWtblHdrTrans.ucFd = 1;
	} else if (IsApcliEntry) {
		rWtblHdrTrans.ucTd = 1;
		rWtblHdrTrans.ucFd = 0;
	} else {
		rWtblHdrTrans.ucTd = 0;
		rWtblHdrTrans.ucFd = 1;
	}
	rWtblHdrTrans.ucDisRhtr = 0;
	CmdExtWtblUpdate(pAd, wcid_idx, SET_WTBL, &rWtblHdrTrans, sizeof(CMD_WTBL_HDR_TRANS_T));

}
#endif
VOID MtAsicUpdateRxWCIDTableByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo)
{
	NDIS_STATUS					Status = NDIS_STATUS_SUCCESS;
	UCHAR						*pTlvBuffer = NULL;
	UCHAR						*pTempBuffer = NULL;
	UINT32						u4TotalTlvLen = 0;
	UCHAR						ucTotalTlvNumber = 0;
	/* Tag = 0, Generic */
	CMD_WTBL_GENERIC_T		rWtblGeneric = {0};
	/* Tage = 1, Rx */
	CMD_WTBL_RX_T				rWtblRx = {0};
#ifdef DOT11_N_SUPPORT
	/* Tag = 2, HT */
	CMD_WTBL_HT_T				rWtblHt = {0};
#ifdef DOT11_VHT_AC
	/* Tag = 3, VHT */
	CMD_WTBL_VHT_T			rWtblVht = {0};
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	/* Tag = 5, TxPs */
	CMD_WTBL_TX_PS_T			rWtblTxPs = {0};
#if defined(HDR_TRANS_TX_SUPPORT) || defined(HDR_TRANS_RX_SUPPORT)
	/* Tag = 6, Hdr Trans */
	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};
#endif /* HDR_TRANS_TX_SUPPORT */
	/* Tag = 7, Security Key */
	CMD_WTBL_SECURITY_KEY_T	rWtblSecurityKey = {0};
	/* Tag = 9, Rdg */
	CMD_WTBL_RDG_T			rWtblRdg = {0};
#ifdef TXBF_SUPPORT
	/* Tag = 12, BF */
	CMD_WTBL_BF_T           rWtblBf = {0};
#endif /* TXBF_SUPPORT */
	/* Tag = 13, SMPS */
	CMD_WTBL_SMPS_T			rWtblSmPs = {0};
	/* Tag = 16, SPE */
	CMD_WTBL_SPE_T          rWtblSpe = {0};
	/* Allocate TLV msg */
	Status = os_alloc_mem(pAd, (UCHAR **)&pTlvBuffer, MAX_BUF_SIZE_OF_WTBL_INFO);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"%d,%d,%d,%d,%d,%d,%d,%d,%d,"MACSTR"),%d,%d,%d,%d,%d,%d)\n",
		WtblInfo.Wcid,
		WtblInfo.Aid,
		WtblInfo.BssidIdx,
		WtblInfo.MacAddrIdx,
		WtblInfo.SmpsMode,
		WtblInfo.MaxRAmpduFactor,
		WtblInfo.MpduDensity,
		WtblInfo.WcidType,
		WtblInfo.aad_om,
		MAC2STR(WtblInfo.Addr),
		WtblInfo.CipherSuit,
		WtblInfo.PfmuId,
		WtblInfo.SupportHT,
		WtblInfo.SupportVHT,
		WtblInfo.SupportRDG,
		WtblInfo.SupportQoS);

	if ((Status != NDIS_STATUS_SUCCESS) || (pTlvBuffer == NULL))
		goto error;

	rWtblRx.ucRv   = WtblInfo.rv;
	rWtblRx.ucRca2 = WtblInfo.rca2;

	/* Manipulate TLV msg */
	if (WtblInfo.WcidType == MT_WCID_TYPE_BMCAST) {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = 0x0e;
		/* Tag = 1 */
		rWtblRx.ucRv = 1;
		rWtblRx.ucRca1 = 1;
		/* if (pAd->OpMode == OPMODE_AP) */
		{
			rWtblRx.ucRca2 = 1;
		}
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WTBL_CIPHER_NONE;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		if (pAd->OpMode == OPMODE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		}

#endif
	} else {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = WtblInfo.MacAddrIdx;
		rWtblGeneric.ucQos = (WtblInfo.SupportQoS) ? 1 : 0;
		rWtblGeneric.u2PartialAID = WtblInfo.Aid;
		rWtblGeneric.ucAadOm = WtblInfo.aad_om;

		/* Tag = 1 */
		if ((WtblInfo.WcidType == MT_WCID_TYPE_APCLI) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_REPEATER) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_AP) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST))
			rWtblRx.ucRca1 = 1;

		rWtblRx.ucRv = 1;
		rWtblRx.ucRca2 = 1;
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WtblInfo.CipherSuit;
		rWtblSecurityKey.ucRkv = (WtblInfo.CipherSuit != WTBL_CIPHER_NONE) ? 1 : 0;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		switch (WtblInfo.WcidType) {
		case MT_WCID_TYPE_AP:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
			break;

		case MT_WCID_TYPE_CLI:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
#if defined(A4_CONN) || defined(MBSS_AS_WDS_AP_SUPPORT)
			if (WtblInfo.a4_enable) {
				rWtblHdrTrans.ucFd = 1;
				rWtblHdrTrans.ucTd = 1;
			}
#endif /* A4_CONN */
			break;

		case MT_WCID_TYPE_APCLI:
		case MT_WCID_TYPE_REPEATER:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
#if defined(A4_CONN) || defined(APCLI_AS_WDS_STA_SUPPORT)
			if (WtblInfo.a4_enable) {
				rWtblHdrTrans.ucFd = 1;
				rWtblHdrTrans.ucTd = 1;
			}
#endif /* A4_CONN */
			break;

		case MT_WCID_TYPE_WDS:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"Unknown entry type(%d) do not support header translation\n",
				WtblInfo.WcidType);
			break;
		}

#endif /* HDR_TRANS_TX_SUPPORT */
#ifdef HDR_TRANS_RX_SUPPORT

		if (WtblInfo.DisRHTR)
			rWtblHdrTrans.ucDisRhtr = 1;
		else
			rWtblHdrTrans.ucDisRhtr = 0;

#endif /* HDR_TRANS_RX_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (WtblInfo.SupportHT) {
			/* Tag = 0 */
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucBafEn = 0;
			/* Tag = 2 */
			rWtblHt.ucHt = 1;
			rWtblHt.ucMm = WtblInfo.MpduDensity;
			rWtblHt.ucAf = WtblInfo.MaxRAmpduFactor;

			/* Tga = 9 */
			if (WtblInfo.SupportRDG) {
				rWtblRdg.ucR = 1;
				rWtblRdg.ucRdgBa = 1;
			}

			/* Tag = 13*/
			if (WtblInfo.SmpsMode == MMPS_DYNAMIC)
				rWtblSmPs.ucSmPs = 1;
			else
				rWtblSmPs.ucSmPs = 0;

#ifdef DOT11_VHT_AC

			/* Tag = 3 */
			if (WtblInfo.SupportVHT) {
				rWtblVht.ucVht = 1;
				/* ucDynBw for WTBL: 0 - not DynBW / 1 -DynBW */
				rWtblVht.ucDynBw = (WtblInfo.dyn_bw == BW_SIGNALING_DYNAMIC)?1:0;
			}

#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
	}

	/* Tag = 0 */
	os_move_mem(rWtblGeneric.aucPeerAddress, WtblInfo.Addr, MAC_ADDR_LEN);
	/* Tag = 5 */
	rWtblTxPs.ucTxPs = 0;
#ifdef TXBF_SUPPORT
	/* Tag = 0xc */
	rWtblBf.ucGid     = WtblInfo.gid;
	rWtblBf.ucPFMUIdx = WtblInfo.PfmuId;
	rWtblBf.ucTiBf    = WtblInfo.fgTiBf;
	rWtblBf.ucTeBf    = WtblInfo.fgTeBf;
	rWtblBf.ucTibfVht = WtblInfo.fgTibfVht;
	rWtblBf.ucTebfVht = WtblInfo.fgTebfVht;
#endif /* TXBF_SUPPORT */
	/* Tag = 0x10 */
	rWtblSpe.ucSpeIdx = WtblInfo.spe_idx;
	/* Append TLV msg */
	pTempBuffer = pTlvAppend(
					  pTlvBuffer,
					  (WTBL_GENERIC),
					  (sizeof(CMD_WTBL_GENERIC_T)),
					  &rWtblGeneric,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RX),
					  (sizeof(CMD_WTBL_RX_T)),
					  &rWtblRx,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_N_SUPPORT
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HT),
					  (sizeof(CMD_WTBL_HT_T)),
					  &rWtblHt,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RDG),
					  (sizeof(CMD_WTBL_RDG_T)),
					  &rWtblRdg,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SMPS),
					  (sizeof(CMD_WTBL_SMPS_T)),
					  &rWtblSmPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_VHT_AC
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_VHT),
					  (sizeof(CMD_WTBL_VHT_T)),
					  &rWtblVht,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_TX_PS),
					  (sizeof(CMD_WTBL_TX_PS_T)),
					  &rWtblTxPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#if defined(HDR_TRANS_RX_SUPPORT) || defined(HDR_TRANS_TX_SUPPORT)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HDR_TRANS),
					  (sizeof(CMD_WTBL_HDR_TRANS_T)),
					  &rWtblHdrTrans,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* HDR_TRANS_RX_SUPPORT || HDR_TRANS_TX_SUPPORT */
	if (WtblInfo.SkipClearPrevSecKey == FALSE)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SECURITY_KEY),
					  (sizeof(CMD_WTBL_SECURITY_KEY_T)),
					  &rWtblSecurityKey,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef TXBF_SUPPORT

	if (pAd->rStaRecBf.u2PfmuId != 0xFFFF) {
		pTempBuffer = pTlvAppend(
						  pTempBuffer,
						  (WTBL_BF),
						  (sizeof(CMD_WTBL_BF_T)),
						  &rWtblBf,
						  &u4TotalTlvLen,
						  &ucTotalTlvNumber);
	}

#endif /* TXBF_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SPE),
					  (sizeof(CMD_WTBL_SPE_T)),
					  &rWtblSpe,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	/* Send TLV msg*/
	if (WtblInfo.SkipClearPrevSecKey == FALSE || WtblInfo.IsReset == TRUE)
		CmdExtWtblUpdate(pAd, WtblInfo.Wcid, RESET_WTBL_AND_SET, pTlvBuffer, u4TotalTlvLen);
	else
		CmdExtWtblUpdate(pAd, WtblInfo.Wcid, SET_WTBL, pTlvBuffer, u4TotalTlvLen);
	/* Free TLV msg */
	if (pTlvBuffer)
		os_free_mem(pTlvBuffer);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO, "(Ret = %d)\n", Status);
}



VOID MtAsicUpdateBASessionByWtblTlv(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 *ba_range = cap->ppdu.ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"unknown tid(%d)\n", BaCtrl.Tid);
		return;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/*get ba win size from range */
			while (ba_range[idx] < BaCtrl.BaWinSize) {
				if (idx == 7)
					break;

				idx++;
			};

			if (ba_range[idx] > BaCtrl.BaWinSize)
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}
}


INT32 MtAsicUpdateBASessionByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_BA_CTRL_T BaCtrl)
{
	INT32				Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 *ba_range = cap->ppdu.ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"unknown tid(%d)\n", BaCtrl.Tid);
		return Status;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			rWtblBa.ucBandIdx = BaCtrl.band_idx;
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}

		/* TODO: Hanmin 7615, need rWtblBa.ucBaEn=0 for delete? */
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/* Get ba win size from range */
			while (BaCtrl.BaWinSize > ba_range[idx]) {
				if (idx == (MT_DMAC_BA_AGG_RANGE - 1))
					break;

				idx++;
			};

			if ((idx > 0) && (ba_range[idx] > BaCtrl.BaWinSize))
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}

	return Status;
}

/* offload BA winsize index calculation to FW */
INT32 MtAsicUpdateBASessionOffloadByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_BA_CTRL_T BaCtrl)
{
	INT32				Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T		rWtblBa = {0};

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"unknown tid(%d)\n", BaCtrl.Tid);
		return Status;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			rWtblBa.ucBandIdx = BaCtrl.band_idx;
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			/* After AX chip, need RX Ba Win size to determine BA_MODE in WTBL */
			rWtblBa.u2BaWinSize = BaCtrl.BaWinSize;
			Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}

		/* TODO: Hanmin 7615, need rWtblBa.ucBaEn=0 for delete? */
	} else {
		if (BaCtrl.isAdd) {
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.u2BaWinSize = BaCtrl.BaWinSize;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.u2BaWinSize = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}

	return Status;
}

VOID MtAsicAddRemoveKeyTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _ASIC_SEC_INFO *pInfo)
{
	CMD_WTBL_SECURITY_KEY_T *wtbl_security_key = NULL;
	UINT32 cmd_len = 0;
	INT32 ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef SW_CONNECT_SUPPORT
	struct _STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
		"wcid=%d, Operation=%d, Direction=%d\n",
		pInfo->Wcid, pInfo->Operation, pInfo->Direction);

#ifdef SW_CONNECT_SUPPORT
	if (IS_WCID_VALID(pAd, pInfo->Wcid))
		tr_entry = tr_entry_get(pAd, pInfo->Wcid);

	/* Skip to set key when wcid is Dummy Wcid, when S/W mode on */
	if (tr_entry && IS_NORMAL_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
	{
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			STA_REC_CFG_T StaRecCfg = {0};

			StaRecCfg.ucBssIndex = pInfo->BssIndex;
			StaRecCfg.u2WlanIdx = pInfo->Wcid;
			StaRecCfg.MuarIdx = 0;
			StaRecCfg.u8EnableFeature = STA_REC_INSTALL_KEY_FEATURE;
			memcpy(&(StaRecCfg.asic_sec_info), pInfo, sizeof(ASIC_SEC_INFO));
			UniCmdStaRecUpdate(pAd, &StaRecCfg);
			return;
		}
#endif /* WIFI_UNIFIED_COMMAND */

		ret = chip_fill_key_install_cmd(pAd->hdev_ctrl, pInfo, WTBL_SEC_KEY_METHOD, (VOID **)&wtbl_security_key, &cmd_len);

		if (ret != NDIS_STATUS_SUCCESS) {
			if (wtbl_security_key) {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
					"key install cmd failed free security key\n");
				os_free_mem(wtbl_security_key);
			}
			return;
		}

		CmdExtWtblUpdate(pAd, pInfo->Wcid, SET_WTBL, (PUCHAR)wtbl_security_key, cmd_len);

		os_free_mem(wtbl_security_key);
	}
}


VOID MtAsicSetSMPSByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR Smps)
{
	CMD_WTBL_SMPS_T	CmdWtblSmPs = {0};

	CmdWtblSmPs.u2Tag = WTBL_SMPS;
	CmdWtblSmPs.u2Length = sizeof(CMD_WTBL_SMPS_T);
	CmdWtblSmPs.ucSmPs = Smps;
	CmdExtWtblUpdate(pAd, Wcid, SET_WTBL, (PUCHAR)&CmdWtblSmPs, sizeof(CMD_WTBL_SMPS_T));
}

VOID MtAsicGetTxTscByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT32 pn_type_mask,
	OUT UCHAR * pTxTsc)
{
	CMD_WTBL_PN_T cmdWtblPn[MAX_TSC_TYPE];
	USHORT wcid = 0;
	UCHAR tsc_cnt = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	os_zero_mem(cmdWtblPn, MAX_TSC_TYPE * sizeof(CMD_WTBL_PN_T));

	if (pn_type_mask & TSC_TYPE_GTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_GTK_PN;
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_IGTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_IGTK_PN;
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_BIGTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_BIGTK_PN;
		tsc_cnt++;
	}
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdSetRecSecPnInfo(pAd, wcid, QUERY_WTBL, (uint8_t *)&cmdWtblPn, tsc_cnt * sizeof(CMD_WTBL_PN_T), tsc_cnt);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		CmdExtWtblUpdate(pAd, wcid, QUERY_WTBL, (PUCHAR)&cmdWtblPn, tsc_cnt * sizeof(CMD_WTBL_PN_T));

	tsc_cnt = 0;

	if (pn_type_mask & TSC_TYPE_GTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("gtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_IGTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("igtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_BIGTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("bigtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}
}

#ifdef ZERO_PKT_LOSS_SUPPORT
UINT8 mtf_read_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_RESULT_T SkipTxStatus;

	UniCmdWcidSkipTx(pAd, wcid, WTBL_OPERATION_QUERY, 0, (UINT32 *) &SkipTxStatus);
	return SkipTxStatus.fgWtblSkipTXEnable;
}

VOID mtf_update_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set)
{
	UniCmdWcidSkipTx(pAd, wcid, WTBL_OPERATION_SET, set, NULL);
}
#endif

VOID mt_wtbltlv_debug(RTMP_ADAPTER *pAd, UINT16 u2Wcid, UCHAR ucCmdId, UCHAR ucAtion, union _wtbl_debug_u *debug_u)
{
	/* tag 0 */
	if (ucCmdId == WTBL_GENERIC) {
		debug_u->wtbl_generic_t.u2Tag = WTBL_GENERIC;
		debug_u->wtbl_generic_t.u2Length = sizeof(CMD_WTBL_GENERIC_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

			os_move_mem(debug_u->wtbl_generic_t.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			debug_u->wtbl_generic_t.ucMUARIndex = 0x0;
			debug_u->wtbl_generic_t.ucSkipTx = 0;
			debug_u->wtbl_generic_t.ucCfAck = 0;
			debug_u->wtbl_generic_t.ucQos = 0;
			debug_u->wtbl_generic_t.ucMesh = 0;
			debug_u->wtbl_generic_t.ucAdm = 0;
			debug_u->wtbl_generic_t.u2PartialAID = 0;
			debug_u->wtbl_generic_t.ucBafEn = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

			os_move_mem(debug_u->wtbl_generic_t.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			debug_u->wtbl_generic_t.ucMUARIndex = 0x0e;
			debug_u->wtbl_generic_t.ucSkipTx = 1;
			debug_u->wtbl_generic_t.ucCfAck = 1;
			debug_u->wtbl_generic_t.ucQos = 1;
			debug_u->wtbl_generic_t.ucMesh = 1;
			debug_u->wtbl_generic_t.ucAdm = 1;
			debug_u->wtbl_generic_t.u2PartialAID = 32;
			debug_u->wtbl_generic_t.ucBafEn = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 1 */
	if (ucCmdId == WTBL_RX) {
		debug_u->wtbl_rx_t.u2Tag = WTBL_RX;
		debug_u->wtbl_rx_t.u2Length = sizeof(CMD_WTBL_RX_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_rx_t.ucRcid = 0;
			debug_u->wtbl_rx_t.ucRca1 = 0;
			debug_u->wtbl_rx_t.ucRca2 = 0;
			debug_u->wtbl_rx_t.ucRv = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_rx_t.ucRcid = 1;
			debug_u->wtbl_rx_t.ucRca1 = 1;
			debug_u->wtbl_rx_t.ucRca2 = 1;
			debug_u->wtbl_rx_t.ucRv = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 2 */
	if (ucCmdId == WTBL_HT) {
		debug_u->wtbl_ht_t.u2Tag = WTBL_HT;
		debug_u->wtbl_ht_t.u2Length = sizeof(CMD_WTBL_HT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_ht_t.ucHt = 0;
			debug_u->wtbl_ht_t.ucLdpc = 0;
			debug_u->wtbl_ht_t.ucAf = 0;
			debug_u->wtbl_ht_t.ucMm = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_ht_t.ucHt = 1;
			debug_u->wtbl_ht_t.ucLdpc = 1;
			debug_u->wtbl_ht_t.ucAf = 1;
			debug_u->wtbl_ht_t.ucMm = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 3 */
	if (ucCmdId == WTBL_VHT) {
		debug_u->wtbl_vht_t.u2Tag = WTBL_VHT;
		debug_u->wtbl_vht_t.u2Length = sizeof(CMD_WTBL_VHT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_vht_t.ucLdpcVht = 0;
			debug_u->wtbl_vht_t.ucDynBw = 0;
			debug_u->wtbl_vht_t.ucVht = 0;
			debug_u->wtbl_vht_t.ucTxopPsCap = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_vht_t.ucLdpcVht = 1;
			debug_u->wtbl_vht_t.ucDynBw = 1;
			debug_u->wtbl_vht_t.ucVht = 1;
			debug_u->wtbl_vht_t.ucTxopPsCap = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 4 */
	if (ucCmdId == WTBL_PEER_PS) {
		debug_u->wtbl_peer_ps_t.u2Tag = WTBL_PEER_PS;
		debug_u->wtbl_peer_ps_t.u2Length = sizeof(CMD_WTBL_PEER_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_peer_ps_t.ucDuIPsm = 0;
			debug_u->wtbl_peer_ps_t.ucIPsm = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_peer_ps_t.ucDuIPsm = 1;
			debug_u->wtbl_peer_ps_t.ucIPsm = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 5 */
	if (ucCmdId == WTBL_TX_PS) {
		debug_u->wtbl_tx_ps_t.u2Tag = WTBL_TX_PS;
		debug_u->wtbl_tx_ps_t.u2Length = sizeof(CMD_WTBL_TX_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_tx_ps_t.ucTxPs = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_tx_ps_t.ucTxPs = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 6 */
	if (ucCmdId == WTBL_HDR_TRANS) {
		debug_u->wtbl_hdr_trans_t.u2Tag = WTBL_HDR_TRANS;
		debug_u->wtbl_hdr_trans_t.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_hdr_trans_t.ucTd = 0;
			debug_u->wtbl_hdr_trans_t.ucFd = 0;
			debug_u->wtbl_hdr_trans_t.ucDisRhtr = 0;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_hdr_trans_t.ucTd = 1;
			debug_u->wtbl_hdr_trans_t.ucFd = 1;
			debug_u->wtbl_hdr_trans_t.ucDisRhtr = 1;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 QUERY_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 7 security */
	/* tag 8 BA */

	/* tag 9 */
	if (ucCmdId == WTBL_RDG) {
		debug_u->wtbl_rdg_t.u2Tag = WTBL_RDG;
		debug_u->wtbl_rdg_t.u2Length = sizeof(CMD_WTBL_RDG_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_rdg_t.ucRdgBa = 0;
			debug_u->wtbl_rdg_t.ucR = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_rdg_t.ucRdgBa = 1;
			debug_u->wtbl_rdg_t.ucR = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 10 */
	if (ucCmdId == WTBL_PROTECTION) {
		debug_u->wtbl_prot_t.u2Tag = WTBL_PROTECTION;
		debug_u->wtbl_prot_t.u2Length = sizeof(CMD_WTBL_PROTECTION_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_prot_t.ucRts = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_prot_t.ucRts = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 11 */
	if (ucCmdId == WTBL_CLEAR) {
		debug_u->wtbl_clear_t.u2Tag = WTBL_CLEAR;
		debug_u->wtbl_clear_t.u2Length = sizeof(CMD_WTBL_CLEAR_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_clear_t.ucClear = ((0 << 1) |
							 (1 << 1) |
							 (1 << 2) |
							 (1 << 3) |
							 (1 << 4) |
							 (1 << 5));
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_clear_t, sizeof(CMD_WTBL_CLEAR_T));
		} else if (ucAtion == 2) {
			/* query */
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 12 */
	if (ucCmdId == WTBL_BF) {
		debug_u->wtbl_bf_t.u2Tag = WTBL_BF;
		debug_u->wtbl_bf_t.u2Length = sizeof(CMD_WTBL_BF_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_bf_t.ucTiBf = 0;
			debug_u->wtbl_bf_t.ucTeBf = 0;
			debug_u->wtbl_bf_t.ucTibfVht = 0;
			debug_u->wtbl_bf_t.ucTebfVht = 0;
			debug_u->wtbl_bf_t.ucGid = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_bf_t.ucTiBf = 1;
			debug_u->wtbl_bf_t.ucTeBf = 1;
			debug_u->wtbl_bf_t.ucTibfVht = 1;
			debug_u->wtbl_bf_t.ucTebfVht = 1;
			debug_u->wtbl_bf_t.ucGid = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 13 */
	if (ucCmdId == WTBL_SMPS) {
		debug_u->wtbl_smps_t.u2Tag = WTBL_SMPS;
		debug_u->wtbl_smps_t.u2Length = sizeof(CMD_WTBL_SMPS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_smps_t.ucSmPs = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_smps_t.ucSmPs = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 14 */
	if (ucCmdId == WTBL_RAW_DATA_RW) {
		debug_u->wtbl_raw_data_rw_t.u2Tag = WTBL_RAW_DATA_RW;
		debug_u->wtbl_raw_data_rw_t.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			debug_u->wtbl_raw_data_rw_t.ucWhichDW = 0;
			debug_u->wtbl_raw_data_rw_t.u4DwMask = 0xffff00ff;
			debug_u->wtbl_raw_data_rw_t.u4DwValue = 0x12340078;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			debug_u->wtbl_raw_data_rw_t.ucWhichDW = 0;
			debug_u->wtbl_raw_data_rw_t.u4DwMask = 0xffff00ff;
			debug_u->wtbl_raw_data_rw_t.u4DwValue = 0x12345678;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 2) {
			/* query */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 QUERY_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"rWtblRawDataRw.u4DwValue(%x)\n",
					debug_u->wtbl_raw_data_rw_t.u4DwValue);
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 15 */
	if (ucCmdId == WTBL_PN) {
		debug_u->wtbl_pn_t.u2Tag = WTBL_PN;
		debug_u->wtbl_pn_t.u2Length = sizeof(CMD_WTBL_PN_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
			os_fill_mem(debug_u->wtbl_pn_t.aucPn, 6, 0xf);
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_pn_t, sizeof(CMD_WTBL_PN_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_pn_t, sizeof(CMD_WTBL_PN_T));
			hex_dump("WTBL_PN", debug_u->wtbl_pn_t.aucPn, 6);
		}
	}
}

VOID MtAsicUpdateRtsThldByFw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		UniCmdUpdateRTSThreshold(pAd, wdev, pkt_num, length);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		MT_RTS_THRESHOLD_T rts_thld = {0};

		rts_thld.band_idx = HcGetBandByWdev(wdev);
		rts_thld.pkt_num_thld = pkt_num;
		rts_thld.pkt_len_thld = length;
		{
			struct _EXT_CMD_UPDATE_PROTECT_T fw_rts;

			os_zero_mem(&fw_rts, sizeof(fw_rts));
			fw_rts.ucProtectIdx = UPDATE_RTS_THRESHOLD;
			fw_rts.ucDbdcIdx = rts_thld.band_idx;
			fw_rts.Data.rUpdateRtsThld.u4RtsPktLenThreshold = cpu2le32(rts_thld.pkt_len_thld);
			fw_rts.Data.rUpdateRtsThld.u4RtsPktNumThreshold = cpu2le32(rts_thld.pkt_num_thld);
			MtCmdUpdateProtect(pAd, &fw_rts);
		}
	}
}


INT MtAsicSetRDGByFw(RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg)
{
	struct _EXT_CMD_RDG_CTRL_T fw_rdg;

	memset(&fw_rdg, 0, sizeof(struct _EXT_CMD_RDG_CTRL_T));
	fw_rdg.u4TxOP = Rdg->Txop;
	fw_rdg.ucLongNav = Rdg->LongNav;
	fw_rdg.ucInit = Rdg->Init;
	fw_rdg.ucResp = Rdg->Resp;
	WCID_SET_H_L(fw_rdg.ucWlanIdxHnVer, fw_rdg.ucWlanIdxL, Rdg->WlanIdx);
	fw_rdg.ucBand = Rdg->BandIdx;
	MtCmdSetRdg(pAd, &fw_rdg);
	return TRUE;
}

UINT32 MtAsicGetWmmParamByFw(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 ret, Value = 0;
	MT_EDCA_CTRL_T EdcaCtrl;

	os_zero_mem(&EdcaCtrl, sizeof(MT_EDCA_CTRL_T));
	EdcaCtrl.ucTotalNum = 1;
	EdcaCtrl.ucAction = EDCA_ACT_GET;
	EdcaCtrl.rAcParam[0].ucAcNum = AcNum;
	ret = MtCmdGetEdca(pAd, &EdcaCtrl);

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		Value = EdcaCtrl.rAcParam[0].u2Txop;
		break;

	case WMM_PARAM_AIFSN:
		Value = EdcaCtrl.rAcParam[0].ucAifs;
		break;

	case WMM_PARAM_CWMIN:
		Value = EdcaCtrl.rAcParam[0].ucWinMin;
		break;

	case WMM_PARAM_CWMAX:
		Value = EdcaCtrl.rAcParam[0].u2WinMax;
		break;

	default:
		Value = 0xdeadbeef;
		break;
	}

	return Value;
}

INT MtAsicSetWmmParam(
	RTMP_ADAPTER *pAd,
	UCHAR idx,
	UINT32 AcNum,
	UINT32 EdcaType,
	UINT32 EdcaValue)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UCHAR index = 0;

	/* Could write any queue by FW */
	if ((AcNum < 4) && (idx < 4))
		index = (idx * 4) + AcNum;
	else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"Non-WMM Queue, WmmIdx/QueIdx=%d/%d!\n",
			idx, AcNum);
		index = AcNum;
	}

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));
	EdcaParam.ucTotalNum = 1;
	EdcaParam.ucAction = EDCA_ACT_SET;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)index;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop = (UINT16)EdcaValue;
		break;

	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax = (UINT16)EdcaValue;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"Error type=%d\n", EdcaType);
		break;
	}
	MtCmdEdcaParameterSet(pAd, &EdcaParam);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WIFI_UNIFIED_COMMAND
INT MtAsicUniCmdSetWmmParam(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR idx,
	UINT32 AcNum,
	UINT32 EdcaType,
	UINT32 EdcaValue)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR, "wdev is NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));
	EdcaParam.ucTotalNum = 1;
	EdcaParam.ucAction = EDCA_ACT_SET;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)AcNum;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop = (UINT16)EdcaValue;
		break;

	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax = (UINT16)EdcaValue;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			" Error type=%d\n", EdcaType);
		break;
	}

	MtUniCmdEdcaParameterSet(pAd, wdev, &EdcaParam);

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID MtAsicSetEdcaParm(RTMP_ADAPTER *pAd, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac = 0, index = 0;

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));

	if ((pEdcaParm != NULL) && (pEdcaParm->bValid != FALSE)) {
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;
		EdcaParam.ucTxModeValid = TRUE;
		EdcaParam.ucTxMode = tx_mode;

		for (ac = 0; ac < CMD_EDCA_AC_MAX;  ac++) {
			index = idx*4+ac;
			pAcParam = &EdcaParam.rAcParam[ac];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum =  asic_get_hwq_from_ac(pAd, idx, ac);
			pAcParam->ucAifs = pEdcaParm->Aifsn[ac];
			pAcParam->ucWinMin = pEdcaParm->Cwmin[ac];
			pAcParam->u2WinMax = pEdcaParm->Cwmax[ac];
			pAcParam->u2Txop = pEdcaParm->Txop[ac];
		}
	}
	MtCmdEdcaParameterSet(pAd, &EdcaParam);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID MtAsicUniCmdSetEdcaParm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac = 0;

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));

	if ((pEdcaParm != NULL) && (pEdcaParm->bValid != FALSE)) {
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;
		EdcaParam.ucTxModeValid = TRUE;
		EdcaParam.ucTxMode = tx_mode;

		for (ac = 0; ac < CMD_EDCA_AC_MAX;  ac++) {
			pAcParam = &EdcaParam.rAcParam[ac];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum = ac;
			pAcParam->ucAifs = pEdcaParm->Aifsn[ac];
			pAcParam->ucWinMin = pEdcaParm->Cwmin[ac];
			pAcParam->u2WinMax = pEdcaParm->Cwmax[ac];
			pAcParam->u2Txop = pEdcaParm->Txop[ac];
		}
	}

	MtUniCmdEdcaParameterSet(pAd, wdev, &EdcaParam);
}
#endif /* WIFI_UNIFIED_COMMAND */

INT MtAsicGetTsfTimeByFirmware(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	TSF_RESULT_T TsfResult;
	INT32 ret = NDIS_STATUS_FAILURE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	os_zero_mem(&TsfResult, sizeof(TSF_RESULT_T));

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		ret = UniCmdGetTsfTime(pAd, HwBssidIdx, &TsfResult);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdGetTsfTime(pAd, HwBssidIdx, &TsfResult);

	if (ret == NDIS_STATUS_SUCCESS) {
		*high_part = TsfResult.u4TsfBit63_32;
		*low_part = TsfResult.u4TsfBit0_31;
		return TRUE;
	}

	return FALSE;
}

INT MtAsicGetTsfDiffTime(
	RTMP_ADAPTER * pAd,
	UINT8 BssIdx0,
	UINT8 BssIdx1,
	UINT32 *Tsf0Bit0_31,
	UINT32 *Tsf0Bit63_32,
	UINT32 *Tsf1Bit0_31,
	UINT32 *Tsf1Bit63_32)
{
	struct TSF_DIFF_RESULT_T TsfDiffResult = {0};
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdGetTsfDiffTime(pAd, BssIdx0, BssIdx1, &TsfDiffResult);
#endif /* WIFI_UNIFIED_COMMAND */

	*Tsf0Bit0_31 = TsfDiffResult.u4Tsf0Bit0_31;
	*Tsf0Bit63_32 = TsfDiffResult.u4Tsf0Bit63_32;
	*Tsf1Bit0_31 = TsfDiffResult.u4Tsf1Bit0_31;
	*Tsf1Bit63_32 = TsfDiffResult.u4Tsf1Bit63_32;

	return TRUE;
}

VOID MtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx)
{
	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;

	MtCmdSlotTimeSet(pAd, (UINT8)SlotTime, (UINT8)SifsTime, (UINT8)RifsTime, (UINT16)EifsTime, BandIdx);
}
#ifdef WIFI_UNIFIED_COMMAND
VOID MtAsicUniCmdSetSlotTime(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, struct wifi_dev *wdev)
{
	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;

	MtUniCmdSlotTimeSet(pAd, (UINT16)SlotTime, (UINT16)SifsTime, (UINT16)RifsTime, (UINT16)EifsTime, wdev);
}
#endif /* WIFI_UNIFIED_COMMAND */

UINT32 MtAsicGetChBusyCntByFw(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
	UINT32 msdr16 = 0, ret;

	ret = MtCmdGetChBusyCnt(pAd, ch_idx, &msdr16);
	return msdr16;
}

UINT32 MtAsicGetCCACnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_MIB_PAIR Reg[UNI_CMD_MIB_MAX_PAIR];
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#else /* WIFI_UNIFIED_COMMAND */
	RTMP_MIB_PAIR Reg[1];
#endif /* !WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(Reg, sizeof(Reg));

	/* M0SDR9 */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		Reg[0].Counter = UNI_CMD_MIB_CNT_CCA_NAV_TX_TIME;
		UniCmdMib(pAd, BandIdx, Reg, 1);
	}
	else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		Reg[0].Counter = MIB_CNT_CCA_NAV_TX_TIME;
		MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 1);
	}

	return (UINT32)Reg[0].Value;
}

INT32 MtAsicSetMacTxRxByFw(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		ret = UniCmdSetMacTxRx(pAd, BandIdx, Enable);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = MtCmdSetMacTxRx(pAd, BandIdx, Enable);
	return ret;
}

INT32 MtAsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		ret = UniCmdRxvCtrl(pAd, BandIdx, ASIC_MAC_TXRX_RXV, Enable);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = MtCmdSetRxvFilter(pAd, BandIdx, Enable);
	return ret;
}

VOID MtAsicDisableSyncByFw(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
			"BssInfo idx (%d) is INIT currently!!!\n",
			wdev->bss_info_argument.ucBssIndex);
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_INITED;
	CmdSetSyncModeByBssInfoUpdate(pAd, &wdev->bss_info_argument);
}

VOID MtAsicEnableBssSyncByFw(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
			"BssInfo idx (%d) is INIT currently!!!\n",
			wdev->bss_info_argument.ucBssIndex);
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
	CmdSetSyncModeByBssInfoUpdate(pAd, &wdev->bss_info_argument);
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
/* STARec Info */
INT32 MtAsicSetAid(
	RTMP_ADAPTER *pAd,
	UINT16 Aid,
	UINT8 OmacIdx)
{
	return CmdETxBfAidSetting(pAd, Aid, 0, OmacIdx);
}
#endif

INT32 MtAsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		UNI_CMD_RX_HDR_TRAN_PARAM_T RxHdrTranParam;

		os_zero_mem(&RxHdrTranParam, sizeof(RxHdrTranParam));
		RxHdrTranParam.RxHdrTranEnable.fgEnable = En;
		RxHdrTranParam.RxHdrTranEnable.fgCheckBssid = ChkBssid;
		RxHdrTranParam.RxHdrTranEnable.ucTranslationMode = 0;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_ENABLE] = TRUE;

		RxHdrTranParam.RxHdrTranVlan.fgInsertVlan = InSVlan;
		RxHdrTranParam.RxHdrTranVlan.fgRemoveVlan = RmVlan;
		RxHdrTranParam.RxHdrTranVlan.fgUseQosTid = !SwPcP;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG] = TRUE;

		return UniCmdRxHdrTrans(pAd, &RxHdrTranParam);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdRxHdrTransUpdate(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);
	}
}

INT32 MtAsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		UNI_CMD_RX_HDR_TRAN_PARAM_T RxHdrTranParam;

		os_zero_mem(&RxHdrTranParam, sizeof(RxHdrTranParam));
		RxHdrTranParam.RxHdrTranBlackList.fgEnable = En;
		RxHdrTranParam.RxHdrTranBlackList.ucBlackListIdx = (UINT8)Index;
		RxHdrTranParam.RxHdrTranBlackList.u2EtherType = (UINT16)EthType;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG] = TRUE;

		return UniCmdRxHdrTrans(pAd, &RxHdrTranParam);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdRxHdrTransBLUpdate(pAd, Index, En, EthType);
	}
}

#ifdef VLAN_SUPPORT
INT32 mt_asic_update_vlan_id_by_fw(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		return UniCmdVLANUpdate(ad, band_idx, omac_idx, UNI_CMD_VLAN_OP_VID, vid);
#endif /* WIFI_UNIFIED_COMMAND */
	return cmd_vlan_update(ad, band_idx, omac_idx, BSS_INFO_SET_VLAN_ID, vid);
}

INT32 mt_asic_update_vlan_priority_by_fw(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		return UniCmdVLANUpdate(ad, band_idx, omac_idx, UNI_CMD_VLAN_OP_PCP, priority);
#endif /* WIFI_UNIFIED_COMMAND */
	return cmd_vlan_update(ad, band_idx, omac_idx, BSS_INFO_SET_VLAN_PRIORITY, priority);
}
#endif

#ifdef DOT11_EHT_BE
INT32 mtf_asic_sta_eml_op_update(
	struct _RTMP_ADAPTER *pAd,
	struct _STA_REC_CTRL_T *sta_rec_ctrl)
{
	struct _STA_REC_CFG StaCfg;
	UINT16 WlanIdx = sta_rec_ctrl->WlanIdx;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _STA_TR_ENTRY *tr_entry;

	os_zero_mem(&StaCfg, sizeof(struct _STA_REC_CFG));
	tr_entry = tr_entry_get(pAd, WlanIdx);
	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		entry = NULL;
	else
		entry = entry_get(pAd, WlanIdx);

	if (entry && !IS_ENTRY_NONE(entry)) {
		if (!entry->wdev) {
			ASSERT(entry->wdev);
			return NDIS_STATUS_FAILURE;
		}
		StaCfg.MuarIdx = entry->wdev->OmacIdx;
		StaCfg.ucBssIndex = entry->wdev->bss_info_argument.ucBssIndex;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = entry;
		UniCmdStaRecEmlOp(pAd, &StaCfg);
		return NDIS_STATUS_SUCCESS;
	}
	return NDIS_STATUS_FAILURE;
}

INT32 mtf_asic_tx_cap_update(
	struct _RTMP_ADAPTER *pAd,
	struct _STA_REC_CTRL_T *sta_rec_ctrl,
	BOOLEAN EnableAGGLimit)
{
	struct _STA_REC_CFG StaCfg;
	UINT16 WlanIdx = sta_rec_ctrl->WlanIdx;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _STA_TR_ENTRY *tr_entry;

	os_zero_mem(&StaCfg, sizeof(struct _STA_REC_CFG));
	tr_entry = tr_entry_get(pAd, WlanIdx);
	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		entry = NULL;
	else
		entry = entry_get(pAd, WlanIdx);

	if (entry && !IS_ENTRY_NONE(entry)) {
		if (!entry->wdev) {
			ASSERT(entry->wdev);
			return NDIS_STATUS_FAILURE;
		}
		StaCfg.MuarIdx = entry->wdev->OmacIdx;
		StaCfg.ucBssIndex = entry->wdev->bss_info_argument.ucBssIndex;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = entry;
		UniCmdStaRecTxCapUpdate(pAd, &StaCfg, EnableAGGLimit);
		return NDIS_STATUS_SUCCESS;
	}
	return NDIS_STATUS_FAILURE;
}

INT32 mtf_asic_at2lm_res_req(
	struct _RTMP_ADAPTER *pAd,
	struct AT2LM_RES_REQ_CTRL_T *req,
	struct AT2LM_RES_RSP_CTRL_T *rsp)
{
	return UniCmdMldAt2lmReq(pAd, req, rsp);
}

INT32 mtf_asic_nt2lm_req(
	struct _RTMP_ADAPTER *pAd,
	struct NT2LM_REQ_CTRL_T *req)
{
	return UniCmdPeerMldNt2lmReq(pAd, req);
}

INT32 mtf_asic_reconfig_rm_link_req(
	struct _RTMP_ADAPTER *pAd,
	struct RECONFIG_RM_LINK_REQ_CTRL_T *req)
{
	return UniCmdMldReconfigRmLinkReq(pAd, req);
}

INT mtf_asic_set_reconfig_tmr(
	RTMP_ADAPTER *pAd,
	struct RECONFIG_SET_TMR_CTRL_T *reconfig_ctrl)
{
	return UniCmdMldReconfigTmr(pAd, reconfig_ctrl);
}
#endif /* DOT11_EHT_BE */

INT32 MtAsicSetLpi(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT8 PsdLimit, UCHAR BandIdx)
{
	UINT32 ret = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		ret = UniCmdSetLpi(pAd, BandIdx, Enable, PsdLimit);
#endif /* WIFI_UNIFIED_COMMAND */

	return ret;
}

#ifdef TXRX_STAT_SUPPORT
UINT32 mt_asic_get_mib_txrx_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx)
{
	//UniCmdStatisticsGetTxRxStat(pAd, band_idx);
	/* for PER debug */
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	COUNTER_802_11 *pWlanCounters;
	/* Need sanity the max array in Rsp */
	RTMP_MIB_PAIR Reg[UNI_CMD_MIB_MAX_PAIR];

	os_zero_mem(Reg, sizeof(Reg));

	pWlanCounters = &pAd->WlanCounters;

	Reg[0].Counter = UNI_CMD_MIB_CNT_BA_CNT;
	Reg[1].Counter = UNI_CMD_MIB_CNT_BSS0_RTS_TX_CNT;
	Reg[2].Counter = UNI_CMD_MIB_CNT_BSS1_RTS_TX_CNT;
	Reg[3].Counter = UNI_CMD_MIB_CNT_BSS2_RTS_TX_CNT;
	Reg[4].Counter = UNI_CMD_MIB_CNT_BSS3_RTS_TX_CNT;
	Reg[5].Counter = UNI_CMD_MIB_CNT_BSS0_RTS_RETRY;
	Reg[6].Counter = UNI_CMD_MIB_CNT_BSS1_RTS_RETRY;
	Reg[7].Counter = UNI_CMD_MIB_CNT_BSS2_RTS_RETRY;
	Reg[8].Counter = UNI_CMD_MIB_CNT_BSS3_RTS_RETRY;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	pWlanCounters->RxBaCnt.QuadPart          = Reg[0].Value;
	pWlanCounters->RTSTotalCnt.QuadPart      = Reg[1].Value;
	pWlanCounters->RTSTotalCnt.QuadPart     += Reg[2].Value;
	pWlanCounters->RTSTotalCnt.QuadPart     += Reg[3].Value;
	pWlanCounters->RTSTotalCnt.QuadPart     += Reg[4].Value;
	pWlanCounters->RTSFailureCount.QuadPart  = Reg[5].Value;
	pWlanCounters->RTSFailureCount.QuadPart += Reg[6].Value;
	pWlanCounters->RTSFailureCount.QuadPart += Reg[7].Value;
	pWlanCounters->RTSFailureCount.QuadPart += Reg[8].Value;
	pWlanCounters->RTSSuccessCount.QuadPart  =
				pWlanCounters->RTSTotalCnt.QuadPart -
				pWlanCounters->RTSFailureCount.QuadPart;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxBaCnt                   = %lld\n", pWlanCounters->RxBaCnt.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RTSTotalCnt               = %lld\n", pWlanCounters->RTSTotalCnt.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RTSFailureCount           = %lld\n", pWlanCounters->RTSFailureCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RTSSuccessCount           = %lld\n", pWlanCounters->RTSSuccessCount.QuadPart);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_BSS0_FRAME_RETRY_2;
	Reg[1].Counter = UNI_CMD_MIB_CNT_BSS1_FRAME_RETRY_2;
	Reg[2].Counter = UNI_CMD_MIB_CNT_BSS2_FRAME_RETRY_2;
	Reg[3].Counter = UNI_CMD_MIB_CNT_BSS3_FRAME_RETRY_2;
	Reg[4].Counter = UNI_CMD_MIB_CNT_BSS0_ACK_FAIL; /* including BA miss cnt */
	Reg[5].Counter = UNI_CMD_MIB_CNT_BSS1_ACK_FAIL;
	Reg[6].Counter = UNI_CMD_MIB_CNT_BSS2_ACK_FAIL;
	Reg[7].Counter = UNI_CMD_MIB_CNT_BSS3_ACK_FAIL;
	Reg[8].Counter = UNI_CMD_MIB_CNT_AMPDU;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	pWlanCounters->MultipleRetryCount.QuadPart  = Reg[0].Value;
	pWlanCounters->MultipleRetryCount.QuadPart += Reg[1].Value;
	pWlanCounters->MultipleRetryCount.QuadPart += Reg[2].Value;
	pWlanCounters->MultipleRetryCount.QuadPart += Reg[3].Value;
	pWlanCounters->ACKFailureCount.QuadPart     = Reg[4].Value;
	pWlanCounters->ACKFailureCount.QuadPart    += Reg[5].Value;
	pWlanCounters->ACKFailureCount.QuadPart    += Reg[6].Value;
	pWlanCounters->ACKFailureCount.QuadPart    += Reg[7].Value;
	pWlanCounters->TxAmpduCnt.QuadPart          = Reg[8].Value;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"MultipleRetryCount        = %lld\n", pWlanCounters->MultipleRetryCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"ACKFailureCount           = %lld\n", pWlanCounters->ACKFailureCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxAmpduCnt                = %lld\n", pWlanCounters->TxAmpduCnt.QuadPart);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_TX_ABORT_CNT0;
	Reg[1].Counter = UNI_CMD_MIB_CNT_TX_ABORT_CNT1;
	Reg[2].Counter = UNI_CMD_MIB_CNT_TX_ABORT_CNT2;
	Reg[3].Counter = UNI_CMD_MIB_CNT_TX_ABORT_CNT3;
	Reg[4].Counter = UNI_CMD_MIB_CNT_MPDU_RETRY_DROP;
	Reg[5].Counter = UNI_CMD_MIB_CNT_RTS_DROP;
	Reg[6].Counter = UNI_CMD_MIB_CNT_BSS0_CTRL_FRAME_CNT;
	Reg[7].Counter = UNI_CMD_MIB_CNT_BSS1_CTRL_FRAME_CNT;
	Reg[8].Counter = UNI_CMD_MIB_CNT_BSS2_CTRL_FRAME_CNT;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	pWlanCounters->TxAbortCnt.QuadPart =
						(Reg[0].Value & 0xffff) + ((Reg[0].Value >> 16) & 0xffff);
	pWlanCounters->TxAbortCnt.QuadPart +=
						(Reg[1].Value & 0xffff) + ((Reg[1].Value >> 16) & 0xffff);
	pWlanCounters->TxAbortCnt.QuadPart +=
						(Reg[2].Value & 0xffff) + ((Reg[2].Value >> 16) & 0xffff);
	pWlanCounters->TxAbortCnt.QuadPart +=
						(Reg[3].Value & 0xffff) + ((Reg[3].Value >> 16) & 0xffff);
	pWlanCounters->TxMpduRetryDropCnt.QuadPart    = Reg[4].Value;
	pWlanCounters->TxRtsDropCnt.QuadPart          = Reg[5].Value;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxAbortCnt                = %lld\n", pWlanCounters->TxAbortCnt.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxMpduRetryDropCnt        = %lld\n", pWlanCounters->TxMpduRetryDropCnt.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxRtsDropCnt              = %lld\n", pWlanCounters->TxRtsDropCnt.QuadPart);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_BSS3_CTRL_FRAME_CNT;
	Reg[1].Counter = UNI_CMD_MIB_CNT_MBSS0_CTRL_FRAME_CNT;
	Reg[2].Counter = UNI_CMD_MIB_CNT_MBSS1_CTRL_FRAME_CNT;
	Reg[3].Counter = UNI_CMD_MIB_CNT_MBSS2_CTRL_FRAME_CNT;
	Reg[4].Counter = UNI_CMD_MIB_CNT_MBSS3_CTRL_FRAME_CNT;
	Reg[5].Counter = UNI_CMD_MIB_CNT_MBSS4_CTRL_FRAME_CNT;
	Reg[6].Counter = UNI_CMD_MIB_CNT_MBSS5_CTRL_FRAME_CNT;
	Reg[7].Counter = UNI_CMD_MIB_CNT_MBSS6_CTRL_FRAME_CNT;
	Reg[8].Counter = UNI_CMD_MIB_CNT_MBSS7_CTRL_FRAME_CNT;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_MBSS8_CTRL_FRAME_CNT;
	Reg[1].Counter = UNI_CMD_MIB_CNT_MBSS9_CTRL_FRAME_CNT;
	Reg[2].Counter = UNI_CMD_MIB_CNT_MBSS10_CTRL_FRAME_CNT;
	Reg[3].Counter = UNI_CMD_MIB_CNT_MBSS11_CTRL_FRAME_CNT;
	Reg[4].Counter = UNI_CMD_MIB_CNT_MBSS12_CTRL_FRAME_CNT;
	Reg[5].Counter = UNI_CMD_MIB_CNT_MBSS13_CTRL_FRAME_CNT;
	Reg[6].Counter = UNI_CMD_MIB_CNT_MBSS14_CTRL_FRAME_CNT;
	Reg[7].Counter = UNI_CMD_MIB_CNT_MBSS15_CTRL_FRAME_CNT;
	Reg[8].Counter = UNI_CMD_MIB_CNT_BCN_RX;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	/* ctrl frame cnt is accumulated in fw, so the last value is for all ctrl cnts */
	pWlanCounters->TxCtrlCnt.QuadPart    = Reg[7].Value;
	pWlanCounters->RxBeaconCnt.QuadPart  = Reg[8].Value;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxCtrlCnt                 = %lld\n", pWlanCounters->TxCtrlCnt.QuadPart);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_RX_FCS_OK;
	Reg[1].Counter = UNI_CMD_MIB_CNT_RX_FCS_ERR;
	Reg[2].Counter = UNI_CMD_MIB_CNT_RX_FIFO_OVERFLOW;
	Reg[3].Counter = UNI_CMD_MIB_CNT_RX_MPDU;
	Reg[4].Counter = UNI_CMD_MIB_CNT_AMDPU_RX_COUNT;
	Reg[5].Counter = UNI_CMD_MIB_CNT_RX_VALID_AMPDU_SF;
	Reg[6].Counter = UNI_CMD_MIB_CNT_DELIMITER_FAIL;
	Reg[7].Counter = UNI_CMD_MIB_CNT_RX_OUT_OF_RANGE_COUNT;
	Reg[8].Counter = UNI_CMD_MIB_CNT_RX_DROP_MPDU;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	pWlanCounters->RxFcsOkCount.QuadPart          = Reg[0].Value;
	pWlanCounters->RxFcsErrorCount.QuadPart       = Reg[1].Value;
	pWlanCounters->RxFifoFullCount.QuadPart       = Reg[2].Value;
	pWlanCounters->RxMpduCount.QuadPart           = Reg[3].Value;
	pWlanCounters->TxBaCnt.QuadPart               = Reg[4].Value;
	pWlanCounters->RxMpduInAmpduCount.QuadPart    = Reg[5].Value;
	pWlanCounters->RxDelimiterFailCount.QuadPart  = Reg[6].Value;
	pWlanCounters->RxOutOfRangeCount.QuadPart     = Reg[7].Value;
	pWlanCounters->RxFilterDropMpduCount.QuadPart = Reg[8].Value;

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_LEN_MISMATCH;
	Reg[1].Counter = UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID0;
	Reg[2].Counter = UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID1;
	Reg[3].Counter = UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID2;
	Reg[4].Counter = UNI_CMD_MIB_CNT_RX_DUP_DROP_BSSID3;
	Reg[5].Counter = UNI_CMD_MIB_CNT_BSS0_MGMT_RETRY;
	Reg[6].Counter = UNI_CMD_MIB_CNT_BSS1_MGMT_RETRY;
	Reg[7].Counter = UNI_CMD_MIB_CNT_BSS2_MGMT_RETRY;
	Reg[8].Counter = UNI_CMD_MIB_CNT_BSS3_MGMT_RETRY;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	pWlanCounters->RxPhy2MacLenMismatchCount.QuadPart = Reg[0].Value;
	/* the rx dup drop cnts are accumulated in fw, so the last value is for all cnts */
	pWlanCounters->RxDupDropCount.QuadPart            = Reg[4].Value;

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_MBSS0_MGMT_RETRY;
	Reg[1].Counter = UNI_CMD_MIB_CNT_MBSS1_MGMT_RETRY;
	Reg[2].Counter = UNI_CMD_MIB_CNT_MBSS2_MGMT_RETRY;
	Reg[3].Counter = UNI_CMD_MIB_CNT_MBSS3_MGMT_RETRY;
	Reg[4].Counter = UNI_CMD_MIB_CNT_MBSS4_MGMT_RETRY;
	Reg[5].Counter = UNI_CMD_MIB_CNT_MBSS5_MGMT_RETRY;
	Reg[6].Counter = UNI_CMD_MIB_CNT_MBSS6_MGMT_RETRY;
	Reg[7].Counter = UNI_CMD_MIB_CNT_MBSS7_MGMT_RETRY;
	Reg[8].Counter = UNI_CMD_MIB_CNT_MBSS8_MGMT_RETRY;

	UniCmdMib(pAd, ucBandIdx, Reg, 9);

	os_zero_mem(Reg, sizeof(Reg));
	Reg[0].Counter = UNI_CMD_MIB_CNT_MBSS9_MGMT_RETRY;
	Reg[1].Counter = UNI_CMD_MIB_CNT_MBSS10_MGMT_RETRY;
	Reg[2].Counter = UNI_CMD_MIB_CNT_MBSS11_MGMT_RETRY;
	Reg[3].Counter = UNI_CMD_MIB_CNT_MBSS12_MGMT_RETRY;
	Reg[4].Counter = UNI_CMD_MIB_CNT_MBSS13_MGMT_RETRY;
	Reg[5].Counter = UNI_CMD_MIB_CNT_MBSS14_MGMT_RETRY;
	Reg[6].Counter = UNI_CMD_MIB_CNT_MBSS15_MGMT_RETRY;

	UniCmdMib(pAd, ucBandIdx, Reg, 7);

	/* the mgmt retry cnts are accumulated in fw, so the last value is for all cnts */
	pWlanCounters->TxMgmtRetryCnt.QuadPart = Reg[6].Value;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"TxMgmtRetryCnt            = %lld\n", pWlanCounters->TxMgmtRetryCnt.QuadPart);

	os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
	MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBandIdx, 0, &rTxStatResult);
	pWlanCounters->TxTotalCnt.QuadPart += rTxStatResult.u4TotalTxCount;
	pWlanCounters->TransmittedFragmentCount.QuadPart +=
			(rTxStatResult.u4TotalTxCount -	rTxStatResult.u4TotalTxFailCount);
	pWlanCounters->FailedCount.QuadPart += rTxStatResult.u4TotalTxFailCount;

	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxFcsOkCount              = %lld\n", pWlanCounters->RxFcsOkCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxFcsErrorCount           = %lld\n", pWlanCounters->RxFcsErrorCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxFifoFullCount           = %lld\n", pWlanCounters->RxFifoFullCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxMpduCount               = %lld\n", pWlanCounters->RxMpduCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxMpduInAmpduCount        = %lld\n", pWlanCounters->RxMpduInAmpduCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxDelimiterFailCount      = %lld\n", pWlanCounters->RxDelimiterFailCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxOutOfRangeCount         = %lld\n", pWlanCounters->RxOutOfRangeCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxFilterDropMpduCount     = %lld\n", pWlanCounters->RxFilterDropMpduCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxPhy2MacLenMismatchCount = %lld\n", pWlanCounters->RxPhy2MacLenMismatchCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxDupDropCount            = %lld\n", pWlanCounters->RxDupDropCount.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxDataByte                = %lld\n", pWlanCounters->RxDataByte.QuadPart);
	MTWF_DBG(pAd, DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_DEBUG,
			"RxBeaconCnt               = %lld\n", pWlanCounters->RxBeaconCnt.QuadPart);

	return NDIS_STATUS_SUCCESS;
}

UINT32 mt_asic_get_all_rate_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucDirection)
{
	MtCmdGetAllRateInfo(pAd, ucBandIdx, ucDirection);
	return NDIS_STATUS_SUCCESS;
}

UINT32 mt_asic_get_stbc_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx)
{
	MtCmdGetStbcInfo(pAd, ucBandIdx);
	return NDIS_STATUS_SUCCESS;
}

UINT32 mt_asic_get_gi_cnts(RTMP_ADAPTER *pAd, UINT8 ucBandIdx)
{
	MtCmdGetGiInfo(pAd, ucBandIdx);
	return NDIS_STATUS_SUCCESS;
}

#endif /* TXRX_STAT_SUPPORT*/

