/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	ftm.c

	Abstract:
	802.11mc FTM protocol responder side (AP) implementation.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	annie        2014.11.22   Initial version: FTM responder function

*/

#ifdef FTM_SUPPORT
#include "rt_config.h"

static struct wifi_dev *last_profile_read_wdev;

VOID ftm_set_last_wdev(struct wifi_dev *wdev)
{
	last_profile_read_wdev = wdev;
}
struct wifi_dev *ftm_get_last_wdev(void)
{
	return last_profile_read_wdev;
}

VOID ftm_switch_channel(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR Channel)
{
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->ignore_dfs = TRUE;
#endif /* MT_DFS_SUPPORT */
	/* apply channel directly*/
	wlan_operate_set_prim_ch(wdev, Channel);
#ifdef MT_DFS_SUPPORT
	pDfsParam->ignore_dfs = FALSE;
#endif /* MT_DFS_SUPPORT */

	ftm_req_fw_mc_or_burst(pAd, wdev);
}

VOID ftm_get_mac_entry_by_wcid(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT16 wcid,
	VOID *pAddr,
	struct _FTM_PEER_INFO *pPeerInfo)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				"wcid = %d\n", wcid);

	if (VALID_UCAST_ENTRY_WCID(pAd, wcid))
		pEntry = entry_get(pAd, wcid);

	if (pEntry && !IS_ENTRY_NONE(pEntry))
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"Get pEntry of wcid = %d\n", pEntry->wcid);
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"Can't find pEntry of %hu, create fake pEntry for FTM\n", wcid);

		pEntry = MacTableInsertEntry(
		pAd, pAddr, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE, MLD_STA_NONE, NULL);
		pPeerInfo->create_STAREC = 1;

		if (pEntry) {
			struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
			struct freq_oper *chan_oper = &bssinfo->chan_oper;

			UINT16 htCI;
			UINT32 vhtCI;
			UINT16 RxMcsMap, TxMcsMap;

			// STA_REC_BASIC

			// STA_REC_RA
			pEntry->RaEntry.fgRaValid = 1;
			pEntry->RaEntry.fgAutoTxRateSwitch = 1;
			pEntry->RaEntry.ucPhyMode = 0x4c;
			pEntry->RaEntry.ucChannel = chan_oper->prim_ch;
			pEntry->RaEntry.ucBBPCurrentBW = 1;
			pEntry->RaEntry.fgDisableCCK = 0;
			pEntry->RaEntry.fgHtCapMcs32 = 0;
			pEntry->RaEntry.fgHtCapInfoGF = 0;
			pEntry->RaEntry.aucHtCapMCSSet[0] = 0xff;
			pEntry->RaEntry.aucHtCapMCSSet[1] = 0xff;
			pEntry->RaEntry.aucHtCapMCSSet[2] = 0;
			pEntry->RaEntry.aucHtCapMCSSet[3] = 0;
			pEntry->RaEntry.ucMmpsMode = 3;
			pEntry->RaEntry.ucGband256QAMSupport = 2;
			pEntry->RaEntry.ucMaxAmpduFactor = 3;
			pEntry->RaEntry.fgAuthWapiMode = 0;
			pEntry->RaEntry.RateLen = 8;
			pEntry->RaEntry.ucSupportRateMode = 6;
			pEntry->RaEntry.ucSupportCCKMCS = 0;
			pEntry->RaEntry.ucSupportOFDMMCS = 0xff;
			pEntry->RaEntry.u4SupportHTMCS = 0xffff;
			pEntry->RaEntry.u2SupportVHTMCS1SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS2SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS3SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS4SS = 0;
			pEntry->RaEntry.force_op_mode = 0;
			pEntry->RaEntry.vhtOpModeChWidth = 0;
			pEntry->RaEntry.vhtOpModeRxNss = 0;
			pEntry->RaEntry.vhtOpModeRxNssType = 0;
			pEntry->RaEntry.ClientStatusFlags = 83886097;

			// STA_REC_HT_BASIC
			htCI = 16429;
			pEntry->HTCapability.HtCapInfo = *((struct GNU_PACKED _HT_CAP_INFO*) &htCI);

			// STA_REC_VHT_BASIC
			vhtCI = 0x33c939b1;
			pEntry->vht_cap_ie.vht_cap = *((struct GNU_PACKED _VHT_CAP_INFO*) &vhtCI);
			RxMcsMap = 0xfffa;
			pEntry->vht_cap_ie.mcs_set.rx_mcs_map = *((struct GNU_PACKED _VHT_MCS_MAP*) &RxMcsMap);
			TxMcsMap = 0xfffa;
			pEntry->vht_cap_ie.mcs_set.tx_mcs_map = *((struct GNU_PACKED _VHT_MCS_MAP*) &TxMcsMap);

			pEntry->sta_force_keep = TRUE;
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HT_CAPABLE);
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_CAPABLE);

			wifi_sys_conn_act_FTM(wdev, pEntry);
		}
	}
}


VOID ftm_mc_burst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 band_idx)
{
	// send uni cmd
	band_idx = hc_get_hw_band_idx(pAd);
	UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_MC_BURST, 0, band_idx, NULL);
}

VOID FtmMCBurst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	HW_SET_FTM_MC_BURST(pAd, wdev);
}

VOID ftm_req_fw_mc_or_burst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	UINT8 band_idx;
	UINT8 peer_num = wdev->FtmCtrl.LastAssignedISTA;
	struct _FTM_PEER_INFO *pPeerInfo;

	pPeerInfo = &wdev->FtmCtrl.iSTA_pinfo[peer_num];
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"pPeerInfo->status = %hhu\n", pPeerInfo->status);
	switch (pPeerInfo->status) {
	case FTMISTA_FIRST_BURST:
		band_idx = hc_get_hw_band_idx(pAd);
		UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_MC, pPeerInfo->trigger_mc_req, band_idx, NULL);
		break;
	case FTMISTA_IN_BURST:
		FtmMCBurst(pAd, wdev);
		break;
	case FTMISTA_WAIT_SWITCH_BACK:
	default:
		break;
	}
}

VOID ftm_create_mac_entry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	VOID *pAddr,
	struct _FTM_PEER_INFO *pPeerInfo)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	/* get pEntry */
	pEntry = MacTableLookup(pAd, pAddr);
	if (pEntry)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"Get pEntry of "MACSTR", wcid = %d\n", MAC2STR(pAddr), pEntry->wcid);
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
				"Can't find pEntry of "MACSTR", create fake pEntry for FTM\n", MAC2STR(pAddr));

		pEntry = MacTableInsertEntry(
		pAd, pAddr, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE, MLD_STA_NONE, NULL);
		pPeerInfo->create_STAREC = 1;

		if (pEntry) {
			struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
			struct freq_oper *chan_oper = &bssinfo->chan_oper;

			UINT16 htCI;
			UINT32 vhtCI;
			UINT16 RxMcsMap, TxMcsMap;

			// STA_REC_BASIC

			// STA_REC_RA
			pEntry->RaEntry.fgRaValid = 1;
			pEntry->RaEntry.fgAutoTxRateSwitch = 1;
			pEntry->RaEntry.ucPhyMode = 0x4c;
			pEntry->RaEntry.ucChannel = chan_oper->prim_ch;
			pEntry->RaEntry.ucBBPCurrentBW = 1;
			pEntry->RaEntry.fgDisableCCK = 0;
			pEntry->RaEntry.fgHtCapMcs32 = 0;
			pEntry->RaEntry.fgHtCapInfoGF = 0;
			pEntry->RaEntry.aucHtCapMCSSet[0] = 0xff;
			pEntry->RaEntry.aucHtCapMCSSet[1] = 0xff;
			pEntry->RaEntry.aucHtCapMCSSet[2] = 0;
			pEntry->RaEntry.aucHtCapMCSSet[3] = 0;
			pEntry->RaEntry.ucMmpsMode = 3;
			pEntry->RaEntry.ucGband256QAMSupport = 2;
			pEntry->RaEntry.ucMaxAmpduFactor = 3;
			pEntry->RaEntry.fgAuthWapiMode = 0;
			pEntry->RaEntry.RateLen = 8;
			pEntry->RaEntry.ucSupportRateMode = 6;
			pEntry->RaEntry.ucSupportCCKMCS = 0;
			pEntry->RaEntry.ucSupportOFDMMCS = 0xff;
			pEntry->RaEntry.u4SupportHTMCS = 0xffff;
			pEntry->RaEntry.u2SupportVHTMCS1SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS2SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS3SS = 0;
			pEntry->RaEntry.u2SupportVHTMCS4SS = 0;
			pEntry->RaEntry.force_op_mode = 0;
			pEntry->RaEntry.vhtOpModeChWidth = 0;
			pEntry->RaEntry.vhtOpModeRxNss = 0;
			pEntry->RaEntry.vhtOpModeRxNssType = 0;
			pEntry->RaEntry.ClientStatusFlags = 83886097;

			// STA_REC_HT_BASIC
			htCI = 16429;
			pEntry->HTCapability.HtCapInfo = *((struct GNU_PACKED _HT_CAP_INFO*) &htCI);

			// STA_REC_VHT_BASIC
			vhtCI = 0x33c939b1;
			pEntry->vht_cap_ie.vht_cap = *((struct GNU_PACKED _VHT_CAP_INFO*) &vhtCI);
			RxMcsMap = 0xfffa;
			pEntry->vht_cap_ie.mcs_set.rx_mcs_map = *((struct GNU_PACKED _VHT_MCS_MAP*) &RxMcsMap);
			TxMcsMap = 0xfffa;
			pEntry->vht_cap_ie.mcs_set.tx_mcs_map = *((struct GNU_PACKED _VHT_MCS_MAP*) &TxMcsMap);

			pEntry->sta_force_keep = TRUE;
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HT_CAPABLE);
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_CAPABLE);

			wifi_sys_conn_act_FTM(wdev, pEntry);
		}
	}
}

/*
========================================================================
Routine Description:
	Dubug purpose, dump hex buffer content

Arguments:
	str				- message name
	pSrcBufVA		- pointer to target buffer
	SrcBufLen		- length of target buffer (in byte)
	DbgLvl			- message level

Return Value:
	None

========================================================================
*/
VOID
FtmHexDump(
	IN char *str,
	IN UINT8 * pSrcBufVA,
	IN UINT SrcBufLen,
	IN UINT32 DbgLvl
)
{
#ifdef DBG

	if (DebugLevel < DbgLvl)
		return;

	hex_dump(str, pSrcBufVA, SrcBufLen);
#endif
}


/*
========================================================================
Routine Description:
	Dubug purpose, dump FTM parameter

Arguments:
	pAd				- WLAN control block pointer
	parm			- Target FTM parameter

Return Value:
	None

========================================================================
*/
BOOLEAN
FtmParmDump(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PARAMETER	parm,
	IN UINT32			DbgLvl
)
{
	if (!parm)
		return FALSE;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "  FTM Parameter Dump\n");
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    status: %d\n", parm->status);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    value: %d\n", parm->value);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    num_burst_exponent=%d\n", parm->num_burst_exponent);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    burst_duration=%d\n", parm->burst_duration);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    min_delta_ftm=%d\n", parm->min_delta_ftm);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    partial_tsf_timer=0x%04X\n", parm->partial_tsf_timer);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    asap_capable=%d\n", parm->asap_capable);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    asap=%d\n", parm->asap);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    ftms_per_burst=%d\n", parm->ftms_per_burst);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    ftm_format_and_bw=%d\n", parm->ftm_format_and_bw);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "	    burst_period=%d\n", parm->burst_period);
	return TRUE;
}


/*
========================================================================
Routine Description:
	Dubug purpose, dump FTM entry value

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM Peer Entry pointer

Return Value:
	if the pEntry is valid and alive (> FTMPEER_UNUSED), return TRUE.

========================================================================
*/
BOOLEAN
FtmEntryDump(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN UINT32			DbgLvl
)
{
	if (!pAd || !pEntry)
		return FALSE;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "[FTM Entry Dump] idx=%ld 0x%p\n", GET_FTM_PEER_IDX(pAd, pEntry), pEntry);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- FTM State: %d\n", pEntry->State);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Address: "MACSTR"\n", MAC2STR(pEntry->Addr));
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- PeerReqParm\n");
	FtmParmDump(pAd, &pEntry->PeerReqParm, DbgLvl);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- VerdictParm\n");
	FtmParmDump(pAd, &pEntry->VerdictParm, DbgLvl);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- FTM DialogToken: %d\n", pEntry->DialogToken);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- BurstCntDown: %d\n", pEntry->BurstCntDown);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- FtmCntDown: %d\n", pEntry->FtmCntDown);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- PendingPid: 0x%02X\n", pEntry->PendingPid);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- TransAndRetrans: %u\n", pEntry->TransAndRetrans);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "\n[Location IE]\n");
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Civic Report: %d\n", pEntry->bCivicMsmtReport);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- LCI Report: %d\n", pEntry->bLciMsmtReport);

	if (pEntry->State > FTMPEER_UNUSED)
		return TRUE;
	else
		return FALSE;
}


/*
========================================================================
Routine Description:
	Dump FTM LCI current setting

Arguments:
	pAd				- WLAN control block pointer
	pLci			- Target LCI
	DbgLvl			- debug level

Return Value:
	None

========================================================================
*/
VOID
FtmLciValueDump(
	IN PRTMP_ADAPTER	pAd,
	IN PLCI_FIELD		pLci,
	IN UINT32			DbgLvl
)
{
	int i;
	UINT64	u8b;

	if (!pLci)
		return;

	/* Latitude */
	u8b = ((UINT64)(pLci->field.Latitude_b2_b33) << 2) | (pLci->field.Latitude_b0_b1);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "[Latitude]\t0x%09llX\n", u8b);
	/* Longitude */
	u8b = ((UINT64)(pLci->field.Longitude_b2_b33) << 2) | (pLci->field.Longitude_b0_b1);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "[Longitude]\t0x%09llX\n", u8b);
	/* Altitude */
	u8b = (pLci->field.Altitude_b22_b29 << 22) | (pLci->field.Altitude_b0_b21);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "[Altitude]\t 0x%08llX\n\n", u8b);
	/* detail */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl,
		"This AP LCI header length: %d\n", pAd->pFtmCtrl->LciHdr.Length);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "LCI Value:\n");

	for (i = 0; i  < sizeof(LCI_FIELD); i++) {
		if ((i > 0) && (i % 4 == 0))
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, " ");

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "%02X ", pLci->byte[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, ("\n\n"));
	/* structure dump */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- LatitudeUncertainty: 0x%X\n", pLci->field.LatitudeUncertainty);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Latitude_b0_b1: 0x%X\n", pLci->field.Latitude_b0_b1);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Latitude_b2_b33: 0x%X\n", pLci->field.Latitude_b2_b33);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- LongitudeUncertainty: 0x%X\n", pLci->field.LongitudeUncertainty);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Longitude_b0_b1: 0x%X\n", pLci->field.Longitude_b0_b1);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Longitude_b2_b33: 0x%X\n", pLci->field.Longitude_b2_b33);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- AltitudeType: 0x%X\n", pLci->field.AltitudeType);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- AltitudeUncertainty: 0x%X\n", pLci->field.AltitudeUncertainty);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Altitude_b0_b21: 0x%X\n", pLci->field.Altitude_b0_b21);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Altitude_b22_b29: 0x%X\n", pLci->field.Altitude_b22_b29);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Datum: 0x%X\n", pLci->field.Datum);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- RegLocAgreement: 0x%X\n", pLci->field.RegLocAgreement);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- RegLocDSE: 0x%X\n", pLci->field.RegLocDSE);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- Dependent: 0x%X\n", pLci->field.Dependent);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DbgLvl, "- STAVersion: 0x%X\n", pLci->field.STAVersion);
	return;
}


/*
========================================================================
Routine Description:
	Convert min_delat_ftm parameter (unit: 100us) to Timer period (unit: 1ms)

Arguments:
	MinDelta, unit: 100us

Return Value:
	millisecond

========================================================================
*/
UINT32
FtmMinDeltaToMS(
	IN UINT32	MinDelta
)
{
	UINT32 ms;

	ms = MinDelta / 10;

	/* Take Ceiling */
	if (MinDelta % 10)
		ms++;

	return ms;
}


/*
========================================================================
Routine Description:
	Convert BurstTimeout parameter to Timer period (unit: 1ms)

Arguments:
	BurstTimeout

Return Value:
	millisecond

========================================================================
*/
UINT32
FtmBurstDurationToMS(
	IN UINT8	BurstDuration
)
{
	UINT32 ms;

	switch (BurstDuration) {
	case FTMBST_250US:
	case FTMBST_500US:
	case FTMBST_1MS:
		ms = 1;
		break;

	case FTMBST_2MS:
		ms = 2;
		break;

	case FTMBST_4MS:
		ms = 4;
		break;

	case FTMBST_8MS:
		ms = 8;
		break;

	case FTMBST_16MS:
		ms = 16;
		break;

	case FTMBST_32MS:
		ms = 32;
		break;

	case FTMBST_64MS:
		ms = 64;
		break;

	case FTMBST_128MS:
		ms = 128;
		break;

	default:
		ms = 200;
		break;
	}

	return ms;
}


/*
========================================================================
Routine Description:
	Get BurstDuration code by time (unit: 100 us)

Arguments:
	time						- unit: 100 us

Return Value:
	FTM_BURST_DURATION_ENCODING

========================================================================
*/
UINT8
FtmEncodeBurstDuration(
	IN UINT32	time
)
{
	UINT32 burst_duration;	/* FTM_BURST_DURATION_ENCODING */

	if (time > 640)
		burst_duration = FTMBST_128MS;
	else if (time > 320)
		burst_duration = FTMBST_64MS;
	else if (time > 160)
		burst_duration = FTMBST_32MS;
	else if (time > 80)
		burst_duration = FTMBST_16MS;
	else if (time > 40)
		burst_duration = FTMBST_8MS;
	else if (time > 20)
		burst_duration = FTMBST_4MS;
	else if (time > 10)
		burst_duration = FTMBST_2MS;
	else if (time > 5)
		burst_duration = FTMBST_1MS;
	else if (time > 2)
		burst_duration = FTMBST_500US;
	else
		burst_duration = FTMBST_250US;

	return burst_duration;
}

UINT8
FtmEncodeBurstDurationMS(
	IN UINT8	time
)
{
	UINT8 burst_duration;	/* FTM_BURST_DURATION_ENCODING */

	if (time > 64)
		burst_duration = FTMBST_128MS;
	else if (time > 32)
		burst_duration = FTMBST_64MS;
	else if (time > 16)
		burst_duration = FTMBST_32MS;
	else if (time > 8)
		burst_duration = FTMBST_16MS;
	else if (time > 4)
		burst_duration = FTMBST_8MS;
	else if (time > 2)
		burst_duration = FTMBST_4MS;
	else if (time > 1)
		burst_duration = FTMBST_2MS;
	else
		burst_duration = FTMBST_1MS;

	return burst_duration;
}

/*
========================================================================
Routine Description:
	Get min BurstTimeout code by min_delta_ftm and ftms_per_burst.

Arguments:
	min_delta_ftm			- unit: 100 us
	ftms_per_burst		- unit: number

Return Value:
	FTM_BURST_DURATION_ENCODING

========================================================================
*/
UINT8
FtmGetBurstDuration(
	IN UINT8	min_delta_ftm,
	IN UINT8	ftms_per_burst
)
{
	const UINT8 ASSUME_RETRY_CNT = 1;
	UINT32 ucTotalTime;
	UINT8 ucTotalTimeInMs;
	UINT8 burst_duration;	/* FTM_BURST_DURATION_ENCODING */

	/* The responding STA’s selection of the FTMs Per Burst subfield should be
		the same as the one requested by the initiating STA if the requested
		the Burst Duration subfield indicates no preference */

	/* burst duration = minD * (FTM+1) + 16us */
	ucTotalTime = min_delta_ftm * (ftms_per_burst + ASSUME_RETRY_CNT) + 1; /* unit: 0.1 ms */

	ucTotalTime /= 10; /* 0.1ms -> 1ms */
	ucTotalTimeInMs = (ucTotalTime < 255) ? ucTotalTime : 0;
	burst_duration = FtmEncodeBurstDurationMS(ucTotalTimeInMs);

	return burst_duration;
}


/*
========================================================================
Routine Description:
	Get minimum burst period by burst duration.

Arguments:
	burst_duration		- FTM_BURST_DURATION_ENCODING

Return Value:
	duration period (unit: 100 ms)

========================================================================
*/
UINT16
FtmGetMinBurstPeriod(
	IN UINT8	burst_duration
)
{
	UINT32 duraion = FtmBurstDurationToMS(burst_duration) / 100 + 1;

	return (UINT16)duraion;
}


/*
========================================================================
Routine Description:
	Lookup table of TOA/TOD base offset

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	the offset, in 01. ns

Note:
	Channel Delta		MT6630		MT7628
	20 - 20			452 ns		428 ns
	40 - 20			1020 ns		980 ns
	40 - 40			651 ns		599 ns

========================================================================
*/
UINT32
FtmGetTmrBaseOffset(
	IN PRTMP_ADAPTER	pAd
)
{
	return 0;
}


/*
========================================================================
Routine Description:
	Convert TOD/TOA unit from 0.25 ns to 0.1 ns and minus the base offset in TOA.

Arguments:
	pAd			- WLAN control block pointer
	pTOD		- pinter to TOD
	pTOD		- pinter to TOA

Return Value:
	Success or not

========================================================================
*/
BOOLEAN
FtmConvertTodToa(
	IN PRTMP_ADAPTER	pAd,
	IN UINT64		*pTOD,
	IN UINT64		*pTOA
)
{
	UINT32 base = FtmGetTmrBaseOffset(pAd);
	*pTOD = *pTOD * 5 / 2;
	*pTOA = *pTOA * 5 / 2;

	if (*pTOA < base) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"##### TOA is less than base\n");
		return FALSE;
	} else {
		*pTOA -= base;
		return TRUE;
	}
}



/*
========================================================================
Routine Description:
	Is x greater than y ?
	if the range is limited and unsigned, use half interval as valid compare.

Arguments:
	x, y			- the number for comparison
	min, max		- the range

Return Value:
	TRUE		- x >= y
	FALSE		- x < y

========================================================================
*/
inline BOOLEAN
FtmGTE(
	IN UINT8 x,
	IN UINT8 y,
	IN UINT8 min,
	IN UINT8 max
)
{
	UINT32 diff;
	BOOLEAN xIsLarger;

	if (x >= y) {
		diff = x - y;
		xIsLarger = TRUE;
	} else {
		diff = y - x;
		xIsLarger = FALSE;
	}

	/* if diff > half interval */
	if (diff > ((max - min + 1) >> 1))
		xIsLarger = !xIsLarger;

	return xIsLarger;
}


/*
========================================================================
Routine Description:
	skip prefix of hex string

Arguments:
	str			- pointer of input string

Return Value:
	output string

========================================================================
*/
RTMP_STRING *
FtmSkipHexPrefix(
	IN RTMP_STRING *str
)
{
	if (str != NULL) {
		if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
			str += 2;

		while (*str == '0')
			str++;
	}

	return str;
}


/*
* ========================================================================
*Routine Description:
*	get target MAC address from iwpriv command argument
*
*Arguments:
*	arg			- argument from iwpriv command
*
*Return Value:
*	BOOLEAN, success or not
*
========================================================================
*/
BOOLEAN
FtmGetTargetAddr(
	IN  PRTMP_ADAPTER	pAd,
	IN  RTMP_STRING		*arg
)
{
	RTMP_STRING *value;
	UINT8 macAddr[MAC_ADDR_LEN];
	INT i;
	PFTM_CTRL pFtm = pAd->pFtmCtrl;
	INT len = strlen(arg);

	if (len == 0)
		return TRUE; /* follow the previous setting in pFtm->Responder */

	/* Mac address acceptable format 00:01:02:03:04:05 length 17 */
	if (len != 17)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && i < MAC_ADDR_LEN; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;

		AtoH(value, (UINT8 *)&macAddr[i++], 1);
	}

	NdisCopyMemory(pFtm->Responder, macAddr, MAC_ADDR_LEN);
	return TRUE;
}


/*
*========================================================================
*Routine Description:
*	Get Tx PID for Tx packet and TMR report mapping. PID Range: 0x21~0x40.
*	Idea: PID is generated from FTM DialogToken.
*
*Arguments:
*	pAd			- WLAN control block pointer
*
*Return Value:
*	PID
*
*========================================================================
*/
inline UINT8
FtmGetNewPid(
	IN PRTMP_ADAPTER	pAd
)
{
	return (pAd->pFtmCtrl->DialogToken & MASK_PID_FTM) + PID_FTM_MIN;
}


/*
========================================================================
Routine Description:
	Add new node to FTM PidPendingQ.
	Link list member: "PidList" in FTM_PEER_ENTRY
	The target is pEntry->PendingPid.

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM Peer Entry pointer
	PID				- Tx Pending PID (is waiting for TMR)

Return Value:
	None

========================================================================
*/
VOID
FtmAddPidPendingNode(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN UINT8			PID
)
{
	PFTM_CTRL pFtm = pAd->pFtmCtrl;
	PFTM_PEER_ENTRY pNode, pNext;
	BOOLEAN bHit = FALSE;
	ULONG IrqFlags = 0;
	/* Check existed nodes */
	RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
	DlListForEachSafe(pNode, pNext, &pFtm->PidPendingQ, FTM_PEER_ENTRY, PidList) {
		if (pNode == pEntry) {
			if (!bHit)
				bHit = TRUE;
			else {
				/* more than one node with the same entry */
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
					"WARNING: Entry %ld is duplicated => remove old node with PID 0x%02X\n",
					GET_FTM_PEER_IDX(pAd, pNode), pNode->PendingPid);
				DlListDel(&pNode->PidList);
				ASSERT(FALSE);
			}
		}
	}
	RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);

	if (bHit) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
			"Entry %ld overwrite PID: 0x%02X -> 0x%02X\n",
			GET_FTM_PEER_IDX(pAd, pEntry), pEntry->PendingPid, PID);
		/* Update new PID */
		RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
		pEntry->PendingPid = PID;
		RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);
	} else {
		/* Insert new node: pEntry */
		RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
		pEntry->PendingPid = PID;
		DlListAddTail(&pFtm->PidPendingQ, &pEntry->PidList);
		RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);
	}

	/* Reset TMR report content */
	NdisZeroMemory(&pEntry->Tmr, sizeof(TMR_NODE));
	pEntry->bGotTmr = FALSE;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "Tx: after add node\n");
}


/*
========================================================================
Routine Description:

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM Peer Entry pointer

Return Value:
	None

========================================================================
*/
INT
FtmDeqPidPendingNode(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PEER_ENTRY	pEntry
)
{
	ULONG IrqFlags = 0;
	INT ret = NDIS_STATUS_FAILURE;

	if (pEntry->PidList.Next) {
		RTMP_IRQ_LOCK(&pAd->pFtmCtrl->PidPendingQLock, IrqFlags);
		DlListDel(&pEntry->PidList);
		pEntry->PendingPid = FTMPID_NOT_WAITING;
		RTMP_IRQ_UNLOCK(&pAd->pFtmCtrl->PidPendingQLock, IrqFlags);
		ret = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"idx=%ld, ERROR! no PendingPid!\n", GET_FTM_PEER_IDX(pAd, pEntry));
		ret = NDIS_STATUS_FAILURE;
	}

	return ret;
}


/*
========================================================================
Routine Description:
	Search for the node with specific PID from FTM PidPendingQ.

Arguments:
	pAd			- WLAN control block pointer
	PID

Return Value:
	Target FTM entry

========================================================================
*/
PFTM_PEER_ENTRY
FtmGetPidPendingNode(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8	PID
)
{
	PFTM_CTRL pFtm = pAd->pFtmCtrl;
	PFTM_PEER_ENTRY pEntry = NULL;
	PFTM_PEER_ENTRY pNode, pNext;
	ULONG IrqFlags = 0;

	RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
	DlListForEachSafe(pNode, pNext, &pFtm->PidPendingQ, FTM_PEER_ENTRY, PidList) {
		if (pNode->PendingPid == PID) {
			pEntry = pNode;
			break;
		}
	}
	RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);
	return pEntry;
}


VOID
FtmNeighborTableInit(
	IN PRTMP_ADAPTER	pAd
)
{
	int idx;
	PFTM_NEIGHBORS pNeighbor;

	for (idx = 0; idx  < MAX_FTM_TBL_SIZE; idx++) {
		pNeighbor = &pAd->pFtmCtrl->FtmNeighbor[idx];
		pNeighbor->LciZ.SubElement = LCI_RPTID_Z;
		pNeighbor->LciZ.Length = sizeof(Z_ELEMENT) - 2;
		pNeighbor->CivicHdr.SubElement = CIVIC_RPTID_CIVIC;
		pNeighbor->CivicHdr.Length = 0;
	}
}


/*
========================================================================
Routine Description:
	Initialize FTM struct and initiator entry table

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

========================================================================
*/
VOID
FtmMgmtInit(
	IN PRTMP_ADAPTER	pAd
)
{
	PFTM_CTRL pFtm = NULL;
	ULONG IrqFlags = 0;
	UINT8 i;
	UINT8 TestVector_LCI[] = {	/* Test Plan 0.0.6 */
		0x52, 0x83, 0x4d, 0x12,  0xef, 0xd2, 0xb0, 0x8b,
		0x9b, 0x4b, 0xf1, 0xcc,  0x2c, 0x00, 0x00, 0x41
	};

	if (os_alloc_mem(NULL, (UINT8 **)&pFtm, sizeof(FTM_CTRL)) != NDIS_STATUS_SUCCESS)
		return;

	pAd->pFtmCtrl = pFtm;
	NdisZeroMemory(pFtm, sizeof(FTM_CTRL));
	/* FTM Default Setting */
	pFtm->asap = FTM_DEFAULT_ASAP;
	pFtm->min_delta_ftm = FTM_DEFAULT_MIN_DELTA_FTM;
	pFtm->ftms_per_burst = FTM_DEFAULT_FTMS_PER_BURST;
	pFtm->num_burst_exponent = FTM_DEFAULT_NUM_BURST_EXP;
	pFtm->burst_duration = FTM_DEFAULT_BURST_DURATION;
	pFtm->burst_period = FTM_DEFAULT_BURST_PERIOD;
	pFtm->LciHdr.SubElement = LCI_RPTID_LCI;
	pFtm->LciHdr.Length = sizeof(LCI_FIELD);
	pFtm->LciZ.SubElement = LCI_RPTID_Z;
	pFtm->LciZ.Length = sizeof(Z_ELEMENT) - 2;
	pFtm->CivicHdr.SubElement = CIVIC_RPTID_CIVIC;
	pFtm->CivicHdr.Length = 0;
	/* Set Test Vector for test plan 0.0.6 */
	NdisCopyMemory(pFtm->LciField.byte, TestVector_LCI, sizeof(LCI_FIELD));
	pFtm->Civic.CountryCode[0] = 'U';	/* US, Ref: http://zh.wikipedia.org/wiki/ISO_3166-1 */
	pFtm->Civic.CountryCode[1] = 'S';
	pFtm->Civic.CA_Type = 2;
	pFtm->Civic.CA_Length = MAX_CIVIC_CA_VALUE_LENGTH;

	for (i = 0; i  < MAX_CIVIC_CA_VALUE_LENGTH; i++)
		pFtm->CA_Value[i] = i + 1;

	pFtm->CivicHdr.Length = sizeof(LOCATION_CIVIC) + pFtm->Civic.CA_Length;
	/* Clear Variables */
	pFtm->DialogToken = 1;
	pFtm->LatestJoinPeer = 0;
	NdisAllocateSpinLock(pAd, &pFtm->PidPendingQLock);
	RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
	DlListInit(&pFtm->PidPendingQ);
	RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);
	FtmPeerTableInit(pAd);
	/* init Neighbor Table */
	FtmNeighborTableInit(pAd);
	pFtm->bSetLciRpt = TRUE;
	pFtm->bSetZRpt = TRUE;
	pFtm->bSetCivicRpt = TRUE;
	pFtm->bSetLciReq = FALSE;
	pFtm->bSetCivicReq = FALSE;
	pFtm->MinimumApCount = 3;
	pFtm->RandomizationInterval = 20;

	FtmMgmtInitByWdev(pAd);
}

VOID
FtmInitISTAPeerInfo(
	struct _FTM_PEER_INFO *pPeerInfo)
{
	pPeerInfo->num_burst_exponent = 0; /* single burst */
	pPeerInfo->burst_duration = 15; /* no perference */
	pPeerInfo->min_delta_ftm = FTM_DEFAULT_MIN_DELTA_FTM;
	pPeerInfo->partial_tsf = 0;
	pPeerInfo->ptsf_no_perference = 1;
	pPeerInfo->asap = FTM_DEFAULT_ASAP;
	pPeerInfo->ftms_per_burst = 0; /* no perference */
	pPeerInfo->fmt_and_bw = 0; /* no perference */
	pPeerInfo->preamble = 0;
	pPeerInfo->bandwidth = 0;
	pPeerInfo->burst_period = 0; /* no perference */
	pPeerInfo->status = FTMISTA_UNUSED;
	pPeerInfo->distanceMM = 0;
	pPeerInfo->create_STAREC = 0;
	pPeerInfo->trigger_mc_req = 0;
}

VOID
FtmInitRSTAPeerInfo(
	struct _FTM_PEER_INFO *pPeerInfo)
{
	pPeerInfo->num_burst_exponent = 0; /* single burst */
	pPeerInfo->burst_duration = 0; /* recalculate: FTM*(3+1)*min_D+16us */
	pPeerInfo->min_delta_ftm = FTM_DEFAULT_MIN_DELTA_FTM;
	pPeerInfo->partial_tsf = 0;
	pPeerInfo->ptsf_no_perference = 1;
	pPeerInfo->asap = FTM_DEFAULT_ASAP;
	pPeerInfo->ftms_per_burst = FTM_DEFAULT_FTMS_PER_BURST;
	pPeerInfo->fmt_and_bw = 0; /* RSTA BW <= ISTA, Phymode can’t be different */
	pPeerInfo->preamble = 0;
	pPeerInfo->bandwidth = 0;
	pPeerInfo->burst_period = 0; /* period >= RSTA's duration */
	pPeerInfo->status = FTMISTA_UNUSED;
	pPeerInfo->distanceMM = 0;
	pPeerInfo->create_STAREC = 0;
	pPeerInfo->trigger_mc_req = 0;
}

VOID
FtmMgmtInitByWdev(
	IN PRTMP_ADAPTER	pAd
)
{
	UCHAR i;
	struct wifi_dev *wdev;
	PFTM_CTRL pFtm = NULL;
	int idx;
	PFTM_PEER_ENTRY	pEntry;
	struct _FTM_PEER_INFO *pPeerInfo;

	/* Get any proper wdev on the band;  */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			pFtm = &wdev->FtmCtrl;
			pFtm->FTMResp = 0;
			pFtm->FTMInit = 0;
			pFtm->FTMRole = FTM_DEFAULT_ROLE;
			pFtm->LastAssignedRSTA = 0;
			pFtm->LastAssignedISTA = 0;

			/* init iSTA */
			for (idx = 0; idx < MAX_FTM_TBL_SIZE; idx++) {
				pPeerInfo = pFtm->iSTA_pinfo + idx;
				FtmInitISTAPeerInfo(pPeerInfo);
			}

			/* init rSTA */
			for (idx = 0; idx < MAX_FTM_TBL_SIZE; idx++) {
				pPeerInfo = pFtm->rSTA_pinfo + idx;
				FtmInitRSTAPeerInfo(pPeerInfo);
			}

			for (idx = 0; idx < MAX_FTM_TBL_SIZE; idx++) {
				pEntry = pFtm->FtmPeer + idx;
				pEntry->State = FTMPEER_UNUSED;
				pEntry->pAd = (PVOID)pAd;
			}
		} else
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"wdev is NULL\n");
	}
}


/*
========================================================================
Routine Description:
	De-init FTM struct and initiator entry table at exiting

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

========================================================================
*/
VOID
FtmMgmtExit(
	IN PRTMP_ADAPTER	pAd
)
{
	PFTM_CTRL pFtm = pAd->pFtmCtrl;
	PFTM_PEER_ENTRY	pEntry;
	BOOLEAN Cancelled;
	ULONG IrqFlags = 0;
	int idx;

	for (idx = 0; idx  < MAX_FTM_TBL_SIZE; idx++) {
		pEntry = pAd->pFtmCtrl->FtmPeer + idx;
		pEntry->State = FTMPEER_UNUSED;
	}

	RTMP_IRQ_LOCK(&pFtm->PidPendingQLock, IrqFlags);
	DlListInit(&pFtm->PidPendingQ);
	RTMP_IRQ_UNLOCK(&pFtm->PidPendingQLock, IrqFlags);
	NdisFreeSpinLock(&pFtm->PidPendingQLock);
	RTMPCancelTimer(&pFtm->FtmReqTimer, &Cancelled);
	os_free_mem(pFtm);
	pAd->pFtmCtrl = NULL;
}


/*
========================================================================
Routine Description:
	Initialize FTM initiator entry table

Arguments:
	pAd			- WLAN control block pointer

Return Value:
	None

========================================================================
*/
VOID
FtmPeerTableInit(
	IN PRTMP_ADAPTER	pAd
)
{
	int idx;
	PFTM_PEER_ENTRY	pEntry;

	for (idx = 0; idx  < MAX_FTM_TBL_SIZE; idx++) {
		pEntry = pAd->pFtmCtrl->FtmPeer + idx;
		pEntry->State = FTMPEER_UNUSED;
		pEntry->pAd = (PVOID)pAd;
	}
}


/*
========================================================================
Routine Description:
	Initialize a FTM initiator entry and assign MAC Address

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM initiator entry pointer
	Addr			- MAC adress of target initiator

Return Value:
	None

========================================================================
*/
VOID
FtmEntrySetValid(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PEER_ENTRY	pEntry,
	IN UINT8	*Addr
)
{
	/* Clear all members by setting to default value */
	pEntry->bLciMsmtReq = FALSE;
	pEntry->bCivicMsmtReq = FALSE;
	pEntry->bLciMsmtReport = FALSE;
	pEntry->bCivicMsmtReport = FALSE;
	NdisZeroMemory(&pEntry->PeerReqParm, sizeof(FTM_PARAMETER));
	NdisZeroMemory(&pEntry->VerdictParm, sizeof(FTM_PARAMETER));
	pEntry->DialogToken = 0;
	pEntry->LciToken = 0;
	pEntry->CivicToken = 0;
	pEntry->BurstCntDown = 0;
	pEntry->FtmCntDown = 0;
	pEntry->FollowUpToken = 0;
	NdisZeroMemory(&pEntry->FollowUpTmr, sizeof(TMR_NODE));
	pEntry->bNeedTmr = FALSE;
	NdisZeroMemory(&pEntry->PidList, sizeof(DL_LIST));
	pEntry->PendingPid = FTMPID_NOT_WAITING;
	pEntry->TransAndRetrans = 0;
	pEntry->bGotTmr = FALSE;
	/* Set Address and state */
	NdisCopyMemory(pEntry->Addr, Addr, MAC_ADDR_LEN);
	pEntry->State = FTMPEER_IDLE;
}


/*
========================================================================
Routine Description:
	Search a FTM entry by Addr.

Arguments:
	pAd			- WLAN control block pointer
	Addr			- MAC adress of target initiator

Return Value:
	FTM entry pointer; NULL means search failed.

========================================================================
*/
PFTM_PEER_ENTRY
FtmEntrySearch(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev		*wdev,
	IN UINT8	*Addr
)
{
	FTM_PEER_ENTRY *pEntry;
	int HashIdx = FTM_TBL_HASH_INDEX(Addr);
	int idx = HashIdx;

	do {
		pEntry = wdev->FtmCtrl.FtmPeer + idx;

		if (pEntry->State && NdisEqualMemory(pEntry->Addr, Addr, MAC_ADDR_LEN))
			return pEntry;

		INCREASE_IDX(idx, 0, MAX_FTM_TBL_SIZE);
	} while (idx != HashIdx);

	return NULL;
}


static struct _FTM_FMT_MAP ftmFmtMap[] = {
	{FTM_BW_NO_PREFERENCE, LOC_PREAMBLE_INVALID, LOC_MEAS_BW_20},
	{FTM_BW_NONHT_BW5, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_5},
	{FTM_BW_NONHT_BW10, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_10},
	{FTM_BW_NONHT_BW20, LOC_PREAMBLE_LEGACY, LOC_MEAS_BW_20},
	{FTM_BW_HT_BW20, LOC_PREAMBLE_HT, LOC_MEAS_BW_20},
	{FTM_BW_VHT_BW20, LOC_PREAMBLE_VHT, LOC_MEAS_BW_20},
	{FTM_BW_HT_BW40, LOC_PREAMBLE_HT, LOC_MEAS_BW_40},
	{FTM_BW_VHT_BW40, LOC_PREAMBLE_VHT, LOC_MEAS_BW_40},
	{FTM_BW_VHT_BW80, LOC_PREAMBLE_VHT, LOC_MEAS_BW_80},
	{FTM_BW_VHT_BW80_80, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_VHT_BW160_2RFLO, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_VHT_BW160_1RFLO, LOC_PREAMBLE_VHT, LOC_MEAS_BW_160},
	{FTM_BW_DMG_BW2160, LOC_PREAMBLE_INVALID, LOC_MEAS_BW_20},
};

UCHAR
FtmGetBWSetting(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR format_and_bw)
{
	USHORT wmode = wdev->PhyMode;
	UCHAR bw = wlan_operate_get_bw(wdev);

	UINT8 preamble, bandwidth;
	int chWidth;
	UCHAR fmt_bw;
	int i;

	preamble = LOC_PREAMBLE_INVALID;
	bandwidth = LOC_MEAS_BW_20;
	for (i = 0; i < ARRAY_SIZE(ftmFmtMap); i++) {
		if (format_and_bw == ftmFmtMap[i].ftm_fmt_and_bw) {
			preamble = ftmFmtMap[i].loc_cfg_preamble;
			bandwidth = ftmFmtMap[i].loc_cfg_bandwidth;
			break;
		}
	}

	/* nego phymode */
	if (wmode == WMODE_INVALID)
		preamble = LOC_PREAMBLE_INVALID;
#ifdef DOT11_EHT_BE
	else if (WMODE_CAP_BE(wmode))
		;
#endif /* DOT11_EHT_BE */
#ifdef DOT11_HE_AX
	else if (WMODE_CAP_AX(wmode)) {
		if (preamble & LOC_PREAMBLE_EHT)
			preamble = LOC_PREAMBLE_HE;
	}
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	else if (WMODE_CAP_AC(wmode)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE))
			preamble = LOC_PREAMBLE_VHT;
	}
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
	else if (WMODE_CAP_N(wmode)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE|LOC_PREAMBLE_VHT))
			preamble = LOC_PREAMBLE_HT;
	}
#endif
	else if (wmode & (WMODE_A|WMODE_B|WMODE_G)) {
		if (preamble & (LOC_PREAMBLE_EHT|LOC_PREAMBLE_HE|LOC_PREAMBLE_VHT|LOC_PREAMBLE_HT))
			preamble = LOC_PREAMBLE_LEGACY;
	}

	/* nego bandwidth */
	// mapping
	switch (bw) {
	case BW_20:
	{
		if (bandwidth & (LOC_MEAS_BW_40|LOC_MEAS_BW_80|LOC_MEAS_BW_160|LOC_MEAS_BW_320))
			chWidth = WIFI_CHAN_WIDTH_20;
		break;
	}
	case BW_40:
	{
		if (bandwidth & (LOC_MEAS_BW_80|LOC_MEAS_BW_160|LOC_MEAS_BW_320))
			chWidth = WIFI_CHAN_WIDTH_40;
		break;
	}
	case BW_80:
	{
		if (bandwidth & (LOC_MEAS_BW_160|LOC_MEAS_BW_320)) {
			chWidth = LOC_MEAS_BW_80;
			bandwidth = LOC_MEAS_BW_160;
		}
		break;
	}
	case BW_160:
	{
		if (bandwidth & (LOC_MEAS_BW_320)) {
			chWidth = WIFI_CHAN_WIDTH_160;
			bandwidth = LOC_MEAS_BW_160;
		}
		break;
	}
	case BW_10:
	{
		chWidth = WIFI_CHAN_WIDTH_10;
		bandwidth = LOC_MEAS_BW_10;
		break;
	}
	case BW_5:
	{
		chWidth = WIFI_CHAN_WIDTH_5;
		bandwidth = LOC_MEAS_BW_5;
		break;
	}
	case BW_8080:
	{
		if (bandwidth & (LOC_MEAS_BW_320)) {
			chWidth = WIFI_CHAN_WIDTH_80P80;
			bandwidth = LOC_MEAS_BW_160;
		}
		break;
	}
	default:
	{
		chWidth = WIFI_CHAN_WIDTH_INVALID;
		bandwidth = LOC_MEAS_BW_20;
		break;
	}
	}

	fmt_bw = FTM_BW_HT_BW20;
	for (i = 0; i < ARRAY_SIZE(ftmFmtMap); i++) {
		if (preamble == ftmFmtMap[i].loc_cfg_preamble
			&& bandwidth == ftmFmtMap[i].loc_cfg_bandwidth) {
			fmt_bw = ftmFmtMap[i].ftm_fmt_and_bw;
		}
	}

	return fmt_bw;
}


/*
========================================================================
Routine Description:
	If FTM entry search hits, return the entry.
	Otherwise, get a vacancy entry to assign it, and return the new FTM entry.

Arguments:
	pAd			- WLAN control block pointer
	Addr			- MAC adress of target initiator

Return Value:
	FTM entry pointer; NULL means table full and assign failed.

========================================================================
*/
PFTM_PEER_ENTRY
FtmEntryGet(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev		*wdev,
	IN UINT8	*Addr
)
{
	FTM_PEER_ENTRY *pEntry;
	int HashIdx, idx;

	pEntry = FtmEntrySearch(pAd, wdev, Addr);

	if (pEntry != NULL)
		return pEntry;

	/* Find an empty entry and occupy it */
	HashIdx = FTM_TBL_HASH_INDEX(Addr);
	idx = HashIdx;

	do {
		pEntry = wdev->FtmCtrl.FtmPeer + idx;

		if (pEntry->State == FTMPEER_UNUSED) {
			FtmEntrySetValid(pAd, pEntry, Addr);
			wdev->FtmCtrl.LatestJoinPeer = idx;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
					 "FTM %d_["MACSTR"]\n",
					  idx, MAC2STR(pEntry->Addr));
			return pEntry;
		}

		INCREASE_IDX(idx, 0, MAX_FTM_TBL_SIZE);
	} while (idx != HashIdx);

	return NULL;
}


static VOID
FTMParseParam(
	IN PFTM_PEER_ENTRY		pEntry
)
{
	int i = 0;

	pEntry->VerdictPeerInfo.num_burst_exponent = pEntry->PeerReqParm.num_burst_exponent;
	pEntry->VerdictPeerInfo.burst_duration = pEntry->PeerReqParm.burst_duration;
	pEntry->VerdictPeerInfo.min_delta_ftm = pEntry->PeerReqParm.min_delta_ftm;
	pEntry->VerdictPeerInfo.partial_tsf = pEntry->PeerReqParm.partial_tsf_timer;
	pEntry->VerdictPeerInfo.ptsf_no_perference = pEntry->PeerReqParm.tsf_no_preference;
	pEntry->VerdictPeerInfo.asap = pEntry->PeerReqParm.asap;
	pEntry->VerdictPeerInfo.ftms_per_burst = pEntry->PeerReqParm.ftms_per_burst;
	pEntry->VerdictPeerInfo.fmt_and_bw = pEntry->PeerReqParm.ftm_format_and_bw;
	for (i = 0; i < ARRAY_SIZE(ftmFmtMap); i++) {
		if (pEntry->PeerReqParm.ftm_format_and_bw == ftmFmtMap[i].ftm_fmt_and_bw) {
			pEntry->VerdictPeerInfo.preamble = ftmFmtMap[i].loc_cfg_preamble;
			pEntry->VerdictPeerInfo.bandwidth = ftmFmtMap[i].loc_cfg_bandwidth;
			break;
		}
	}
	pEntry->VerdictPeerInfo.burst_period = pEntry->PeerReqParm.burst_period;
}

static VOID
FTMNegoParseParam(
	IN PFTM_PEER_ENTRY		pEntry
)
{
	int i = 0;

	pEntry->VerdictPeerInfo.num_burst_exponent = pEntry->VerdictParm.num_burst_exponent;
	pEntry->VerdictPeerInfo.burst_duration = pEntry->VerdictParm.burst_duration;
	pEntry->VerdictPeerInfo.min_delta_ftm = pEntry->VerdictParm.min_delta_ftm;
	pEntry->VerdictPeerInfo.partial_tsf = pEntry->VerdictParm.partial_tsf_timer;
	pEntry->VerdictPeerInfo.ptsf_no_perference = pEntry->VerdictParm.tsf_no_preference;
	pEntry->VerdictPeerInfo.asap = pEntry->VerdictParm.asap;
	pEntry->VerdictPeerInfo.ftms_per_burst = pEntry->VerdictParm.ftms_per_burst;
	pEntry->VerdictPeerInfo.fmt_and_bw = pEntry->VerdictParm.ftm_format_and_bw;
	for (i = 0; i < ARRAY_SIZE(ftmFmtMap); i++) {
		if (pEntry->VerdictParm.ftm_format_and_bw == ftmFmtMap[i].ftm_fmt_and_bw) {
			pEntry->VerdictPeerInfo.preamble = ftmFmtMap[i].loc_cfg_preamble;
			pEntry->VerdictPeerInfo.bandwidth = ftmFmtMap[i].loc_cfg_bandwidth;
			break;
		}
	}
	pEntry->VerdictPeerInfo.burst_period = pEntry->VerdictParm.burst_period;
}

/*
========================================================================
Routine Description:
	Decide the FTM Verdict Parameter and response by Tx FTM frame.

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer

Return Value:
	None

========================================================================
*/
VOID
FtmParameterNego(
	IN PRTMP_ADAPTER		pAd,
	IN struct wifi_dev		*wdev,
	IN PFTM_PEER_ENTRY		pEntry
)
{
	struct _FTM_PEER_INFO *pPeerInfo;
	void *pPeerInfoArray;
	UINT8 peer_num = 0;

	pPeerInfoArray = &wdev->FtmCtrl.rSTA_pinfo;
	if (pPeerInfoArray == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pPeerInfoArray is NULL\n");
		return;
	}

	pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + peer_num;
	if (pPeerInfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pPeerInfo is NULL\n");
		return;
	}

	pEntry->State = FTMPEER_NEGO;
	/* Obtain VerdictParm */
	NdisCopyMemory(&pEntry->VerdictParm, &pEntry->PeerReqParm, sizeof(FTM_PARAMETER));
	pEntry->VerdictParm.status = FTMSTATUS_SUCCESS;

	/* asap_capable: always set to 1 */
	pEntry->VerdictParm.asap_capable = 1;

	/* num_burst_exponent */
	{
		if (pEntry->PeerReqParm.num_burst_exponent == FTMBST_NO_PREFERENCE)
			pEntry->VerdictParm.num_burst_exponent = FTM_DEFAULT_NUM_BURST_EXP;
	}

	/* min_delta_ftm */
	{
		pEntry->VerdictParm.min_delta_ftm = pEntry->PeerReqParm.min_delta_ftm;

		if (pEntry->PeerReqParm.min_delta_ftm == FTM_DELTA_NO_PREFERENCE)
			pEntry->VerdictParm.min_delta_ftm = pPeerInfo->min_delta_ftm;

		/* 802.11mc D4.3: 10.24.6.3 (p.1797):
		 * The responding STA's selection of the Min Delta FTM field value shall be
		 * greater than or equal to the value requested by the initiating STA. */
		else if (pEntry->PeerReqParm.min_delta_ftm < pPeerInfo->min_delta_ftm)
			pEntry->VerdictParm.min_delta_ftm = pPeerInfo->min_delta_ftm;
	}

	/* ftms_per_burst */
	if (pEntry->PeerReqParm.ftms_per_burst == 1) {
		/* single packet cannot resultin T1~T4 */
		pEntry->PeerReqParm.ftms_per_burst = 2;
	}

	/* burst_duration & ftms_per_burst */
	if (pEntry->PeerReqParm.ftms_per_burst == FTM_NUM_NO_PREFERENCE) {
		/* 802.11mc D4.3: 10.24.6.3 (p.1797):
		The responding STA's selection of the Burst Duration field value should be less than or
		equal to the one requested by the initiating STA if the requested FTMs per Burst field
		value is set to a value indicating no preference, subject the recommendations below and
		the responding STA's policy on the maximum and minimum Burst Duration field values.
		*/
		UCHAR ftm_cnt = 2;

		if (pEntry->PeerReqParm.burst_duration != FTMBST_NO_PREFERENCE) {
			pEntry->VerdictParm.burst_duration = pEntry->PeerReqParm.burst_duration;

			/* (burst duration / minD) >= FTM */
			if (pEntry->VerdictParm.min_delta_ftm != 0)
				ftm_cnt = (FtmBurstDurationToMS(pEntry->VerdictParm.burst_duration) * 10 - 1) / pEntry->VerdictParm.min_delta_ftm;

			if (ftm_cnt >= FTM_DEFAULT_FTMS_PER_BURST)
				pEntry->VerdictParm.ftms_per_burst = FTM_DEFAULT_FTMS_PER_BURST;
			else if (ftm_cnt < 2)
				pEntry->VerdictParm.ftms_per_burst = 2;
			else
				pEntry->VerdictParm.ftms_per_burst = ftm_cnt;

		} else {
			pEntry->VerdictParm.ftms_per_burst = FTM_DEFAULT_FTMS_PER_BURST;
			pEntry->VerdictParm.burst_duration = FtmGetBurstDuration(pEntry->VerdictParm.min_delta_ftm, pEntry->VerdictParm.ftms_per_burst);
		}
	} else {
		/* Note: pEntry->PeerReqParm.burst_duration might be 15, means FTMBST_NO_PREFERENCE */
		if (pEntry->PeerReqParm.burst_duration == FTMBST_NO_PREFERENCE) {
			/* The responding STA’s selection of the FTMs Per Burst subfield should be
				the same as the one requested by the initiating STA if the requested
				the Burst Duration subfield indicates no preference */

			pEntry->VerdictParm.ftms_per_burst = pEntry->PeerReqParm.ftms_per_burst;
			pEntry->VerdictParm.burst_duration = FtmGetBurstDuration(pEntry->VerdictParm.min_delta_ftm, pEntry->VerdictParm.ftms_per_burst);
		} else {
			UINT8 ASSUME_RETRY_CNT = 1;

			// iSTA had set ftm_nums and burst duration, check minD
			pEntry->VerdictParm.ftms_per_burst = pEntry->PeerReqParm.ftms_per_burst;
			pEntry->VerdictParm.burst_duration = pEntry->PeerReqParm.burst_duration;

			if (pEntry->VerdictParm.min_delta_ftm * (pEntry->VerdictParm.ftms_per_burst + ASSUME_RETRY_CNT) + 1 > FtmBurstDurationToMS(pEntry->VerdictParm.burst_duration) * 10)
				pEntry->VerdictParm.status = FTMSTATUS_REQ_INCAPABLE;
		}
	}

	/* burst_period */
	if (pEntry->PeerReqParm.num_burst_exponent == 0) {
		/* Single Burst */
		/* Note: if pEntry->PeerReqParm.ftms_per_burst is 0, then burst_period must be 0 */
		pEntry->VerdictParm.burst_period = 0;
	} else {
		/* Multi Burst */
		UINT16 min_burst_period = FtmGetMinBurstPeriod(pEntry->VerdictParm.burst_duration);

		if (pEntry->VerdictParm.burst_period < min_burst_period)
			pEntry->VerdictParm.burst_period = min_burst_period;
	}

	/* Partial TFS, unit: ms */
	{
		UINT32 hTsf = 0, lTsf = 0;
		UINT32 PartialTSF;
		/* Get current TSF */
		AsicGetTsfTime(pAd, &hTsf, &lTsf, HW_BSSID_0);
		/* partial_tsf_timer: b[25:10] of TSF */
		PartialTSF = (lTsf & 0x03FFFC00) >> 10;

		if (pEntry->PeerReqParm.asap) {
			/* workaround with Marvell: prevent to send too early, 2015.04.17 */
			if (PartialTSF >= 1)
				PartialTSF -= 1;
		} else {
			PartialTSF += FTM_DEFAULT_PTSF_DELTA;

			/* no less than peer's request */
			if ((pEntry->PeerReqParm.tsf_no_preference == 0) &&
				(PartialTSF < pEntry->PeerReqParm.partial_tsf_timer)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
					"Follow up initiator's PTSF: 0x%04X -> 0x%04X\n",
					PartialTSF, pEntry->PeerReqParm.partial_tsf_timer);
				PartialTSF = pEntry->PeerReqParm.partial_tsf_timer;
			}
		}

		pEntry->VerdictParm.partial_tsf_timer = PartialTSF;
	}
	/* format and bandwidth */
	pEntry->VerdictParm.ftm_format_and_bw = FtmGetBWSetting(pAd, wdev, pEntry->PeerReqParm.ftm_format_and_bw);
	/* zero all reserved field */
	pEntry->VerdictParm.rsv_1 = 0;
	pEntry->VerdictParm.rsv_2 = 0;
	pEntry->FtmCntDown = pEntry->VerdictParm.ftms_per_burst;
	pEntry->BurstCntDown = (1 << pEntry->VerdictParm.num_burst_exponent);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
		"Dump FTM Parameters, FTM_Request Rx\n");
	FtmEntryDump(pAd, wdev, pEntry, DBG_LVL_DEBUG);

	FTMNegoParseParam(pEntry);

	/* Tx FTM with Verdict FTM parameter */
	if (pEntry->VerdictParm.status == FTMSTATUS_SUCCESS)
		SendFTM(pAd, wdev, pEntry->Addr, FTMTX_START);
	else
		SendFTM(pAd, wdev, pEntry->Addr, FTMTX_REJECT);
}


/*
========================================================================
Routine Description:
	Response FTM frame after nego done.

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer

Return Value:
	None

========================================================================
*/
VOID
FtmEntryNegoDoneAction(
	IN PRTMP_ADAPTER		pAd,
	IN PFTM_PEER_ENTRY		pEntry
)
{
	/* Update State */
	if (pEntry->VerdictParm.asap) {
		pEntry->State = FTMPEER_MEASURING_IN_BURST;
		CNT_DOWN_DECREASE(pEntry->FtmCntDown);
	} else {
		pEntry->State = FTMPEER_MEASURING_WAIT_TRIGGER;
	}
}


/*
========================================================================
Routine Description:
	Maintain the FTM/Burst countdown value and do related action.

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer

Return Value:
	BOOLEAN
	- TRUE: implies keep going sending FTM

========================================================================
*/
BOOLEAN
FtmEntryCntDownAction(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev		*wdev,
	IN PFTM_PEER_ENTRY	pEntry
)
{
	/* FtmCntDown */
	CNT_DOWN_DECREASE(pEntry->FtmCntDown);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "   FtmCntDown: %d\n", pEntry->FtmCntDown);

	if (pEntry->FtmCntDown)
		return TRUE;

	/* BurstCntDown */
	CNT_DOWN_DECREASE(pEntry->BurstCntDown);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "   BurstCntDown: %d\n", pEntry->BurstCntDown);

	if (pEntry->BurstCntDown) {
		pEntry->FtmCntDown = pEntry->VerdictParm.ftms_per_burst;
		pEntry->State = FTMPEER_MEASURING_WAIT_TRIGGER;
		return FALSE;
	} else {
		/* Finished. */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "T1\n");
		FtmEntryTerminate(pAd, wdev, pEntry, FALSE);
		return FALSE;
	}
}


/*
========================================================================
Routine Description:
	FTM modify procedure: terminate and restart

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer
	pRxParm		- FTM patameter in Rx packet buffer

Return Value:
	None

========================================================================
*/
VOID
FtmParmModifyProcedure(
	IN PRTMP_ADAPTER		pAd,
	IN struct wifi_dev		*wdev,
	IN PFTM_PEER_ENTRY		pEntry,
	IN PFTM_PARAMETER		pRxParm
)
{
	UINT8 TargetAddr[MAC_ADDR_LEN];

	NdisCopyMemory(TargetAddr, pEntry->Addr, MAC_ADDR_LEN);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "T2\n");
	FtmEntryTerminate(pAd, wdev, pEntry, FALSE);
	FtmEntryGet(pAd, wdev, TargetAddr);
	NdisCopyMemory(&pEntry->PeerReqParm, pRxParm, sizeof(FTM_PARAMETER));
	FtmParameterNego(pAd, wdev, pEntry);
}


/*
========================================================================
Routine Description:
	Handle the FTM request with start/continue/modify.

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer
	bTxFTM		- if TRUE, send FTM frame to stop the initiator

Return Value:
	TRUE: Termination Success
	FALSE: Termination Failed

========================================================================
*/
BOOLEAN
FtmEntryTerminate(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN BOOLEAN			bTxFTM
)
{
	if (!pEntry)
		return FALSE;

	if (pEntry->State == FTMPEER_UNUSED)
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			 "Terminate: bTxFTM=%d, "MACSTR"\n",
			  bTxFTM, MAC2STR(pEntry->Addr));

	if (bTxFTM)
		SendFTM(pAd, wdev, pEntry->Addr, FTMTX_STOP);

	FtmDeqPidPendingNode(pAd, pEntry);
	pEntry->BurstCntDown = 0;
	pEntry->FtmCntDown = 0;
	pEntry->State = FTMPEER_UNUSED;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "Terminate: after del node\n");
	return TRUE;
}


/*
========================================================================
Routine Description:
	Handle the FTM request with start/continue/modify.

Arguments:
	pAd			- WLAN control block pointer
	pEntry		- FTM entry pointer
	pFtmReq		- Rx packet payload (bypass 802.11 header)
	FtmReqLen	- Rx packet length (bypass 802.11 header)

Return Value:
	None

========================================================================
*/
BOOLEAN
FtmRequestHandler(
	IN PRTMP_ADAPTER		pAd,
	IN struct wifi_dev		*wdev,
	IN PFTM_PEER_ENTRY		pEntry,
	IN PFTM_REQUEST_FRAME	pFtmReq,
	IN UINT32				FtmReqLen
)
{
	PEID_STRUCT pEId;
	MEASUREMENT_REQ *pMsmtReq = NULL;
	MEASUREMENT_REQ *pLciReq = NULL;
	MEASUREMENT_REQ *pCivicReq = NULL;
	FTM_PARAMETER *pRxParm = NULL;
	UINT8 *pBuf;
	INT BufLen;

	pEntry->bLciMsmtReq = FALSE;
	pEntry->bCivicMsmtReq = FALSE;
	pBuf = (UINT8 *)(pFtmReq->Variable);
	BufLen = FtmReqLen - sizeof(FTM_REQUEST_FRAME);
	pEId = (PEID_STRUCT)pBuf;

	while (BufLen >= pEId->Len) {
		switch (pEId->Eid) {
		case IE_MEASUREMENT_REQUEST:
			pMsmtReq = (MEASUREMENT_REQ *)pEId;

			if (parse_measurement_ie(pMsmtReq->Length)) {	/* offest of "Type" */
				switch (pMsmtReq->Type) {
				case MSMT_LCI_REQ:
					pEntry->bLciMsmtReq = TRUE;
					pEntry->LciToken = pMsmtReq->Token;
					pLciReq = pMsmtReq;
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
							 "IE_MEASUREMENT_REQUEST[%d] Length:%d Type:0x%02X Token:%u\n",
							  IE_MEASUREMENT_REQUEST, pLciReq->Length, pLciReq->Type, pEntry->LciToken);
					break;

				case MSMT_LOCATION_CIVIC_REQ:
					pEntry->bCivicMsmtReq = TRUE;
					pEntry->CivicToken = pMsmtReq->Token;
					pCivicReq = pMsmtReq;
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
							 "IE_MEASUREMENT_REQUEST[%d] Length:%d Type:0x%02X  Token:%u\n",
							  IE_MEASUREMENT_REQUEST, pCivicReq->Length, pCivicReq->Type, pEntry->CivicToken);
					break;
				}
			} else
				return FALSE;

			break;

		case IE_FTM_PARM:
			/* FTM parameter parse and handle */
			pRxParm = (FTM_PARAMETER *)(pEId->Octet);
			break;
		}

		/* Go to next IE */
		pBuf += (pEId->Len + 2);
		BufLen -= (pEId->Len + 2);
		pEId = (PEID_STRUCT)pBuf;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
		"pEntry->State=%hhu\n", pEntry->State);
	switch (pEntry->State) {
	case FTMPEER_IDLE:
		if (pRxParm) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				"Rx Trigger FTMReq !!\n");
			NdisCopyMemory(&pEntry->PeerReqParm, pRxParm, sizeof(FTM_PARAMETER));
		}

		FTMParseParam(pEntry);
		SendFTM(pAd, wdev, pEntry->Addr, FTMTX_START);
		// calculate burst duration/ period
		break;

	case FTMPEER_MEASURING_WAIT_TRIGGER:
		if (!pRxParm) {
			pEntry->State = FTMPEER_MEASURING_IN_BURST;
			SendFTM(pAd, wdev, pEntry->Addr, FTMTX_ONGOING);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
					 "warning, pRxParm should not be set here ! Go to modify procedure\n");
			/* Modify FTM */
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "M1\n");
			FtmParmModifyProcedure(pAd, wdev, pEntry, pRxParm);
		}

		break;

	case FTMPEER_MEASURING_IN_BURST:
		if (pRxParm) {
			/* Modify FTM */
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "M2\n");
			NdisCopyMemory(&pEntry->PeerReqParm, pRxParm, sizeof(FTM_PARAMETER));
			FTMParseParam(pEntry);
			SendFTM(pAd, wdev, pEntry->Addr, FTMTX_START);
			// calculate burst duration/ period
		} else {
			SendFTM(pAd, wdev, pEntry->Addr, FTMTX_ONGOING);
		}

		break;
	}
	return TRUE;
}


VOID
FtmSetInvalidToaTod(
	IN TMR_NODE * pTmr
)
{
	pTmr->tod = 0xFFFFFFFFFFFF;
	pTmr->toa = 0xFFFFFFFFFFFF;
	pTmr->HwReport.TmrD6.field.ToD32 = 0xFFFF;
	pTmr->HwReport.TmrD6.field.ToA32 = 0xFFFF;
	pTmr->HwReport.ToD0 = 0xFFFFFFFF;
	pTmr->HwReport.ToA0 = 0xFFFFFFFF;
}


/*
========================================================================
Routine Description:
	Fillout TOD, TOA, TOD Error and TOA Error field in Tx FTM frame.

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM entry pointer
	pFtmFr			- FTM frame packet buffer

Return Value:
	None

========================================================================
*/
VOID
FtmFilloutToaTod(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PEER_ENTRY	pEntry,
	IN PFTM_FRAME		pFtmFr
)
{
	TMR_FRM_STRUC	*pTmr = &pEntry->FollowUpTmr.HwReport;
	/* TOD, TOA */
	NdisCopyMemory(pFtmFr->TOD, (UINT8 *)&pEntry->FollowUpTmr.tod, 6);
	NdisCopyMemory(pFtmFr->TOA, (UINT8 *)&pEntry->FollowUpTmr.toa, 6);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			 "   [Tx]  TOD:0x%012llX  TOA:0x%012llX\n", pEntry->FollowUpTmr.tod, pEntry->FollowUpTmr.toa);
	/* Max Error */
	pFtmFr->TODError.MaxError = 150;	/* 150 * 0.2 ns = 30 ns */
	pFtmFr->TOAError.MaxError = 150;

	if (pTmr->TmrD6.field.ToD32 > pTmr->TmrD6.field.ToA32) {
		pFtmFr->TODError.NotConti = 1;
		pFtmFr->TOAError.NotConti = 1;
	}
}


ULONG FtmApendFtmHeader(
	PRTMP_ADAPTER		pAd,
	FTM_PEER_ENTRY		*pEntry,
	UINT8				*pOutBuffer,
	UINT8				DialogToken,
	UINT8				FollowUpToken,
	BOOLEAN				bFillTodToa)
{
	ULONG				TmpLen = 0;
	FTM_FRAME			FtmFr;

	NdisZeroMemory(&FtmFr, sizeof(FTM_FRAME));
	FtmFr.Category = CATEGORY_PUBLIC;
	FtmFr.Action = ACTION_FTM;
	FtmFr.DialogToken = DialogToken;
	FtmFr.FollowUpDialogToken = FollowUpToken;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
		"Token:0x%02X  F:0x%02X\n", FtmFr.DialogToken, FtmFr.FollowUpDialogToken);

	if (bFillTodToa == TRUE)
		FtmFilloutToaTod(pAd, pEntry, &FtmFr);

	MakeOutgoingFrame(pOutBuffer, &TmpLen,
					  sizeof(FTM_FRAME),  &FtmFr,
					  END_OF_ARGS);
	return TmpLen;
}


ULONG FtmApendLciReport(
	PRTMP_ADAPTER		pAd,
	FTM_PEER_ENTRY		*pEntry,
	UINT8				*pOutBuffer)
{
	PFTM_CTRL			pFtm = pAd->pFtmCtrl;
	MEASUREMENT_REPORT LciRpt;
	ULONG				FrameLen = 0;
	ULONG				TmpLen = 0;
	PMEASURE_REPORT_MODE pRptMode = NULL;

	NdisZeroMemory(&LciRpt, sizeof(MEASUREMENT_REPORT));
	pEntry->bLciMsmtReport = TRUE;
	LciRpt.ID = IE_MEASUREMENT_REPORT;
	LciRpt.Length = sizeof(MEASUREMENT_REPORT) - 2;
	LciRpt.Token = pEntry->LciToken;
	pRptMode = (PMEASURE_REPORT_MODE)&LciRpt.ReportMode;
	pRptMode->field.Late = 0;
	pRptMode->field.Incapable = 0;
	pRptMode->field.Refused = 0;
	LciRpt.Type = MSMT_LCI_RPT;
	/* 0. LCI */
	LciRpt.Length += (sizeof(MSMT_RPT_SUBELEMENT) + pFtm->LciHdr.Length);

	/* 4. Z element */
	if (pAd->pFtmCtrl->bSetZRpt) {
		LciRpt.Length += (pFtm->LciZ.Length + 2);
		pFtm->LciZ.Floor.field.ExpectedToMove = 0;
	}

	/* 6. Usage Rules/Policy */
	pFtm->LciUsage.SubElement = LCI_RPTID_USAGE_RULES;
	pFtm->LciUsage.RulesAndPolicy.field.RetransAllowed = 1;
	pFtm->LciUsage.RulesAndPolicy.field.RetExpiresPresent = 0;
	pFtm->LciUsage.RulesAndPolicy.field.LocationPolicy = 0;
	pFtm->LciUsage.Length = pFtm->LciUsage.RulesAndPolicy.field.RetExpiresPresent ? \
							(sizeof(USAGE_SUBELEMENT) - 2) :
							(sizeof(USAGE_SUBELEMENT) - 2 \
							 - sizeof(pFtm->LciUsage.RetExpires));
	LciRpt.Length += (pFtm->LciUsage.Length + 2);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(MEASUREMENT_REPORT), &LciRpt,
					  sizeof(MSMT_RPT_SUBELEMENT), &pFtm->LciHdr,
					  pFtm->LciHdr.Length, &pFtm->LciField,
					  END_OF_ARGS);

	if (pAd->pFtmCtrl->bSetZRpt) {
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  sizeof(Z_ELEMENT), &pFtm->LciZ,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					  (pFtm->LciUsage.Length + 2), &pFtm->LciUsage,
					  END_OF_ARGS);
	FrameLen += TmpLen;
	return  FrameLen;
}


ULONG FtmApendCivicReport(
	PRTMP_ADAPTER		pAd,
	FTM_PEER_ENTRY		*pEntry,
	UINT8				*pOutBuffer)
{
	PFTM_CTRL			pFtm = pAd->pFtmCtrl;
	MEASUREMENT_REPORT CivicRpt;
	UINT8 CivicLocationType;
	ULONG				FrameLen = 0;
	ULONG				TmpLen = 0;
	PMEASURE_REPORT_MODE pRptMode = NULL;

	NdisZeroMemory(&CivicRpt, sizeof(MEASUREMENT_REPORT));
	pEntry->bCivicMsmtReport = TRUE;
	CivicRpt.ID = IE_MEASUREMENT_REPORT;
	CivicRpt.Length = sizeof(MEASUREMENT_REPORT) - 2;
	CivicRpt.Token = pEntry->CivicToken;
	pRptMode = (PMEASURE_REPORT_MODE)&CivicRpt.ReportMode;
	pRptMode->field.Late = 0;
	pRptMode->field.Incapable = 0;
	pRptMode->field.Refused = 0;
	CivicRpt.Type = MSMT_LOCATION_CIVIC_RPT;
	/* Location Civic Report */
	CivicLocationType = CIVIC_TYPE_IETF_RFC4776_2006;
	CivicRpt.Length += sizeof(CivicLocationType);
	CivicRpt.Length += (sizeof(MSMT_RPT_SUBELEMENT) + pFtm->CivicHdr.Length);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(MEASUREMENT_REPORT), &CivicRpt,
					  sizeof(CivicLocationType), &CivicLocationType,
					  sizeof(MSMT_RPT_SUBELEMENT), &pFtm->CivicHdr,
					  END_OF_ARGS);

	if (pFtm->CivicHdr.Length) {
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  sizeof(LOCATION_CIVIC), &pFtm->Civic,
						  pFtm->Civic.CA_Length, pFtm->CA_Value,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	return FrameLen;
}


ULONG FtmApendFtmParam(
	PRTMP_ADAPTER		pAd,
	FTM_PEER_ENTRY     *pEntry,
	UINT8				*pOutBuffer)
{
	ULONG			TmpLen = 0;
	FTM_PARM_IE		FtmParm;

	NdisZeroMemory(&FtmParm, sizeof(FTM_PARM_IE));
	FtmParm.ID = IE_FTM_PARM;
	FtmParm.Length = sizeof(FTM_PARAMETER);
	NdisCopyMemory(&FtmParm.p, &pEntry->VerdictParm, sizeof(FTM_PARAMETER));
	MakeOutgoingFrame(pOutBuffer, &TmpLen,
					  sizeof(FTM_PARM_IE), &FtmParm,
					  END_OF_ARGS);
	return TmpLen;
}


VOID FtmFrameKickOut(
	PRTMP_ADAPTER   pAd,
	struct wifi_dev	*wdev,
	FTM_PEER_ENTRY  *pEntry,
	UINT8			*pOutBuffer,
	ULONG			FrameLen)
{
	FtmHexDump("FtmFrameKickOut", pOutBuffer, FrameLen, DBG_LVL_DEBUG);
	/* Packet send out */
	pEntry->bNeedTmr = TRUE;
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
	pOutBuffer = NULL;
	/* Update Dialog Token (this is also the follow-up token of next pair) */
	pEntry->DialogToken = wdev->FtmCtrl.DialogToken;
	/* 802.11mc D3.0: 10.24.6.4 (p.1721):
	Dialog Tokens field values of consecutive Fine Timing Measurement frames
	shall, excluding retries, be consecutive, except when the value wraps around to 1.
	*/
	INCREASE_IDX(wdev->FtmCtrl.DialogToken, 1, 256);
}

/*
========================================================================
Routine Description:
	Tx a FTM packet to start/continue a FTM procedure

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM entry pointer
	Reason			- FTMTX_START or FTMTX_ONGOING
	bReportLci		- BOOLEAN, report LCI Location IE or not
	bReportCivic		- BOOLEAN, report Civic Location IE or not

Return Value:
	None

========================================================================
*/
VOID
FtmTxForResponse(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN PFTM_PEER_ENTRY	pEntry,
	IN CHAR				Reason,
	IN BOOLEAN			bReportLci,
	IN BOOLEAN			bReportCivic
)
{
	PUINT8				pOutBuffer = NULL;
	ULONG				FrameLen = 0;
	HEADER_802_11		FtmHdr;
	NDIS_STATUS			NStatus;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	NdisZeroMemory(&FtmHdr, sizeof(HEADER_802_11));
	/* Construct 802.11 header */
	FtmHdr.FC.FrDs = 1;
	MgtMacHeaderInit(pAd, &FtmHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[BSS0].wdev.if_addr,
					 pAd->ApCfg.MBSSID[BSS0].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &FtmHdr,
					  END_OF_ARGS);

	switch (Reason) {
	case FTMTX_START: {
		/* case 1: START and ASAP */
		/* case 2: START and non-ASAP */
		pEntry->bLciMsmtReport = FALSE;
		pEntry->bCivicMsmtReport = FALSE;
		/* Construct FTM frame */
		FrameLen += FtmApendFtmHeader(pAd,
									  pEntry,
									  (pOutBuffer + FrameLen),
									  wdev->FtmCtrl.DialogToken,
									  0,
									  FALSE);

		/* Construct LCIMsmtReport */
		if (pEntry->bLciMsmtReq && bReportLci) {
			FrameLen += FtmApendLciReport(pAd,
										  pEntry,
										  (pOutBuffer + FrameLen));
		}

		/* Construct CivicRpt */
		if (pEntry->bCivicMsmtReq && bReportCivic) {
			FrameLen += FtmApendCivicReport(pAd,
											pEntry,
											(pOutBuffer + FrameLen));
		}

		/* Construct FTM Parameter */
		FrameLen += FtmApendFtmParam(pAd,
									 pEntry,
									 (pOutBuffer + FrameLen));
		FtmFrameKickOut(pAd, wdev, pEntry, pOutBuffer, FrameLen);
		pOutBuffer = NULL;
		break;
	}

	case FTMTX_ONGOING: {
		/* case 3: ONGOING and 1st FTM */
		/* case 4: ONGOING and not 1st FTM */
		if (pEntry->FtmCntDown == pEntry->VerdictParm.ftms_per_burst) {
			/* Construct FTM frame */
			FrameLen += FtmApendFtmHeader(pAd,
										  pEntry,
										  (pOutBuffer + FrameLen),
										  wdev->FtmCtrl.DialogToken,
										  0,
										  FALSE);
		} else if (pEntry->FtmCntDown == 1) {
			/* The final one FTM */
			FrameLen += FtmApendFtmHeader(pAd,
										  pEntry,
										  (pOutBuffer + FrameLen),
										  0,
										  pEntry->FollowUpToken,
										  TRUE);
		} else {
			/* Construct FTM frame */
			FrameLen += FtmApendFtmHeader(pAd,
										  pEntry,
										  (pOutBuffer + FrameLen),
										  wdev->FtmCtrl.DialogToken,
										  pEntry->FollowUpToken,
										  TRUE);
		}

		FtmFrameKickOut(pAd, wdev, pEntry, pOutBuffer, FrameLen);
		pOutBuffer = NULL;
		break;
	}

	case FTMTX_REJECT: {
		FrameLen += FtmApendFtmHeader(pAd,
										pEntry,
										(pOutBuffer + FrameLen),
										0,
										0,
										FALSE);

		FrameLen += FtmApendFtmParam(pAd,
									 pEntry,
									 (pOutBuffer + FrameLen));

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
		MlmeFreeMemory(pOutBuffer);
		pOutBuffer = NULL;
		break;
	}

	default:
		break;
	}

	if (pOutBuffer != NULL)
		MlmeFreeMemory(pOutBuffer);
}


/*
========================================================================
Routine Description:
	Tx a FTM packet to stop a FTM procedure

Arguments:
	pAd				- WLAN control block pointer
	pEntry			- FTM entry pointer

Return Value:
	None

========================================================================
*/
VOID
FtmTxForStop(
	IN PRTMP_ADAPTER	pAd,
	IN PFTM_PEER_ENTRY	pEntry
)
{
	PUINT8			pOutBuffer = NULL;
	ULONG			FrameLen = 0;
	HEADER_802_11   FtmHdr;
	FTM_FRAME		FtmFr;
	NDIS_STATUS		NStatus;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	NdisZeroMemory(&FtmHdr, sizeof(HEADER_802_11));
	NdisZeroMemory(&FtmFr, sizeof(FTM_FRAME));
	/* Construct 802.11 header */
	FtmHdr.FC.FrDs = 1;
	MgtMacHeaderInit(pAd, &FtmHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[BSS0].wdev.if_addr,
					 pAd->ApCfg.MBSSID[BSS0].wdev.bssid);
	/* Construct FTM frame */
	FtmFr.Category = CATEGORY_PUBLIC;
	FtmFr.Action = ACTION_FTM;
	FtmFr.DialogToken = 0;
	FtmFilloutToaTod(pAd, pEntry, &FtmFr);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &FtmHdr,
					  sizeof(FTM_FRAME),  &FtmFr,
					  END_OF_ARGS);
	/* Packet send out */
	pEntry->bNeedTmr = TRUE;
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
}

/*
    ==========================================================================
    Description:
	this function is for ftm to switch channel.
	this function switch channel without csa and deauth.
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT FtmSwitchChannel(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	UCHAR channel,
	enum FTM_ISTA_ACTION action)
{
	UCHAR band_idx;

	band_idx = HcGetBandByWdev(wdev);
	if (!IsValidChannel(mac_ad, channel, wdev)) {
		MTWF_DBG(mac_ad, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
			"This channel is out of channel list\n");
		return NDIS_STATUS_FAILURE;
	}

	if (wdev->channel == channel) {
		MTWF_DBG(mac_ad, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
			"The channel %d not changed.\n", channel);
		ftm_req_fw_mc_or_burst(mac_ad, wdev);
		return NDIS_STATUS_FAILURE;
	}

	if (action == ACTION_GO_TO) {
		/*To do set channel, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(mac_ad, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
			MTWF_DBG(mac_ad, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
				"TakeChannelOpCharge fail for SET channel!!\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	/* apply channel directly*/
	wdev->channel = channel;
	HW_SET_FTM_CHANNEL(mac_ad, wdev, wdev->channel);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
		"CtrlChannel(%d), CentralChannel(%d)\n",
		channel, wlan_operate_get_cen_ch_1(wdev));

	/*if channel setting is DONE, release ChannelOpCharge here*/
	if (action == ACTION_SWITCH_BACK)
		ReleaseChannelOpCharge(mac_ad, wdev, CH_OP_OWNER_SET_CHN);

	return NDIS_STATUS_SUCCESS;
}

/*
========================================================================
Routine Description:
	Tx a FTM packet

Arguments:
	pAd				- WLAN control block pointer
	Addr				- MAC address of Target FTM peer
	Reason			- FTMTX_ONGOING or FTMTX_STOP

Return Value:
	None

========================================================================
*/
VOID
SendFTM(
	IN PRTMP_ADAPTER	pAd,
	IN struct wifi_dev	*wdev,
	IN UINT8			*Addr,
	IN CHAR				Reason
)
{
	PFTM_PEER_ENTRY		pEntry;
	UINT8 band_idx;

	if (!Addr || !pAd)
		return;

	pEntry = FtmEntrySearch(pAd, wdev, Addr);

	if (!pEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"#%d: FTM_peer_entry search failed !\n", __LINE__);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			 "Send FTM frame to "MACSTR", Reason=%d\n",
			  MAC2STR(Addr), Reason);

	switch (Reason) {
	case FTMTX_START:
		pEntry->State = FTMPEER_MEASURING_IN_BURST;
		band_idx = hc_get_hw_band_idx(pAd);
		UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_RSP, 1, band_idx, pEntry);
		break;

	case FTMTX_ONGOING:
		band_idx = hc_get_hw_band_idx(pAd);
		UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_RSP, 2, band_idx, pEntry);
		break;

	case FTMTX_REJECT:
		FtmTxForResponse(pAd, wdev, pEntry, Reason, FALSE, FALSE);
		pEntry->State = FTMPEER_UNUSED;
		break;

	case FTMTX_STOP:
		FtmTxForStop(pAd, pEntry);
		break;
	}
}


/*
========================================================================
Routine Description:
	Parse the received FTM request frame and response it.

Arguments:
	pAd				- WLAN control block pointer
	Elem				- received packet

Return Value:
	None

========================================================================
*/
VOID
ReceiveFTMReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem
)
{
	FTM_PEER_ENTRY *pEntry;
	FTM_REQUEST_FRAME *pFtmReq;
	PFRAME_802_11 Fr;
	BOOLEAN result;
	struct wifi_dev *wdev;
	UINT8 band_idx;

	if (!Elem)
		return;

	if (Elem->MsgLen < (LENGTH_802_11 + sizeof(FTM_REQUEST_FRAME))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"ACTION - FTM Request Frame length=%ld is too short!\n", Elem->MsgLen);
		return;
	}

	wdev = Elem->wdev;
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"wdev is NULL!\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "ACTION - Rx FTM Request Frame\n");
	FtmHexDump("FTMRequestFrame", Elem->Msg, Elem->MsgLen, DBG_LVL_DEBUG);
	Fr = (PFRAME_802_11)Elem->Msg;
	pEntry = FtmEntryGet(pAd, wdev, Fr->Hdr.Addr2);

	if (!pEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"ACTION - FTM Request Frame, no available entry to handle\n");
		return;
	}

	pFtmReq = (FTM_REQUEST_FRAME *) &Elem->Msg[LENGTH_802_11];

	/* get or create STAREC */
	ftm_get_mac_entry_by_wcid(pAd, wdev, Elem->Wcid, Fr->Hdr.Addr2, &pEntry->VerdictPeerInfo);

	switch (pFtmReq->Trigger) {
	case FTM_TRIGGER_START_OR_CONTI:
		result = FtmRequestHandler(pAd, wdev, pEntry, pFtmReq, Elem->MsgLen - LENGTH_802_11);
		if (result == FALSE)
			return;
		break;

	case FTM_TRIGGER_STOP:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "T4\n");
		band_idx = hc_get_hw_band_idx(pAd);
		UniCmdFTM(pAd, wdev, UNI_CMD_LOC_TAG_RANGE_REQ_RSP, 0, band_idx, pEntry);
		break;
	}
}


/*
========================================================================
Routine Description:
	Parse the received FTM frame.

Arguments:
	pAd				- WLAN control block pointer
	Elem				- received packet

Return Value:
	None

========================================================================
*/
VOID
ReceiveFTM(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	FTM_FRAME *pFtm;
	MEASUREMENT_REPORT *pLCIMsmtReport = NULL;
	MEASUREMENT_REPORT *pLocationCivicMsmtRport = NULL;
	FTM_PARAMETER *parm = NULL;
	FTM_PEER_ENTRY *pEntry = NULL;
	PFRAME_802_11   Fr = NULL;
	UINT8 *pBuf;
	UINT16 SN;
	struct wifi_dev *wdev;

	if (!Elem)
		return;

	if (Elem->MsgLen < (LENGTH_802_11 + sizeof(FTM_FRAME))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"ACTION - FTM Frame length=%ld is too short! (sizeof(FTM_FRAME)=%lu)(sum=%lu)\n",
			Elem->MsgLen, sizeof(FTM_FRAME), (LENGTH_802_11 + sizeof(FTM_FRAME)));
		return;
	}

	wdev = Elem->wdev;
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"wdev is NULL!\n");
		return;
	}

	NdisCopyMemory((UINT8 *)&SN, &Elem->Msg[22], 2);
	Fr = (PFRAME_802_11)Elem->Msg;
	pEntry = FtmEntryGet(pAd, wdev, Fr->Hdr.Addr2);

	if (!pEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"ACTION - FTM Frame, no available entry to handle\n");
		return;
	}

	pFtm = (FTM_FRAME *) &Elem->Msg[LENGTH_802_11];
	pBuf = (UINT8 *)(pFtm->Variable);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "ACTION - Rx FTM Frame\n");
	FtmHexDump("FTMFrame", Elem->Msg, Elem->MsgLen, DBG_LVL_DEBUG);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "FTM Content\n");
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  Category=%d\n", pFtm->Category);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  Action=%d\n", pFtm->Action);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "   Token:0x%02X FollowUp:0x%02X SN:0x%04X(%d)\n", pFtm->DialogToken, pFtm->FollowUpDialogToken, SN, Fr->Hdr.Sequence);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  TOD: "MACSTR"\n", MAC2STR(pFtm->TOD));
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  TOA: "MACSTR"\n", MAC2STR(pFtm->TOA));
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  TODError: NotConti=%d, MaxError=%d\n", pFtm->TODError.NotConti, pFtm->TODError.MaxError);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "	  TOAError: NotConti=%d, MaxError=%d\n", pFtm->TOAError.NotConti, pFtm->TOAError.MaxError);
	NdisCopyMemory((UINT8 *)&pEntry->Tmr.tod, pFtm->TOD, 6);
	NdisCopyMemory((UINT8 *)&pEntry->Tmr.toa, pFtm->TOA, 6);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
			 "   [Rx]  TOD:0x%012llX  TOA:0x%012llX\n", pEntry->Tmr.tod, pEntry->Tmr.toa);

	if (*pBuf == IE_MEASUREMENT_REPORT) {
		pLCIMsmtReport = (MEASUREMENT_REPORT *)pBuf;
		pBuf += (pLCIMsmtReport->Length + 2);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				 "IE_MEASUREMENT_REPORT[%d] Length:%d\n",
				  IE_MEASUREMENT_REPORT, pLCIMsmtReport->Length);
	}

	if (*pBuf == IE_MEASUREMENT_REPORT) {
		pLocationCivicMsmtRport = (MEASUREMENT_REPORT *)pBuf;
		pBuf += (pLocationCivicMsmtRport->Length + 2);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				 "IE_MEASUREMENT_REPORT[%d] Length:%d\n",
				  IE_MEASUREMENT_REPORT, pLocationCivicMsmtRport->Length);
	}

	if (*pBuf == IE_FTM_PARM) {
		parm = (FTM_PARAMETER *)(pBuf + 2);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "Dump FTM Parameters, FTM Rx\n");
		FtmParmDump(pAd, parm, DBG_LVL_INFO);
	}

	if (pAd->pFtmCtrl->WaitForNego) {
		pAd->pFtmCtrl->WaitForNego = FALSE;
	}
}


VOID FtmSendCivicToDaemon(IN PRTMP_ADAPTER pAd)
{
	/* construct anqp location event to daemon , using pAd->pFtmCtrl->Civic */
	PUINT8				pOutBuffer = NULL;
	ULONG				FrameLen = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PNET_DEV NetDev = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.if_dev;
	UINT8 CivicLocationType = CIVIC_TYPE_IETF_RFC4776_2006;
	INT NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(CivicLocationType), &CivicLocationType,
					  sizeof(MSMT_RPT_SUBELEMENT), &pAd->pFtmCtrl->CivicHdr,
					  sizeof(LOCATION_CIVIC), &pAd->pFtmCtrl->Civic,
					  pAd->pFtmCtrl->Civic.CA_Length, pAd->pFtmCtrl->CA_Value,
					  END_OF_ARGS);
	SendLocationElementEvent(NetDev, pOutBuffer, FrameLen, AP_CIVIC_LOCATION);
	MlmeFreeMemory(pOutBuffer);
}

VOID FtmSendLciToDaemon(IN PRTMP_ADAPTER pAd)
{
	/* construct anqp location event to daemon , using pAd->pFtmCtrl->Civic */
	PUINT8				pOutBuffer = NULL;
	ULONG				FrameLen = 0, TmpLen = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PNET_DEV NetDev = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.if_dev;
	INT NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(MSMT_RPT_SUBELEMENT), &pAd->pFtmCtrl->LciHdr,
					  pAd->pFtmCtrl->LciHdr.Length, &pAd->pFtmCtrl->LciField,
					  END_OF_ARGS);

	if (pAd->pFtmCtrl->bSetZRpt) {
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  sizeof(Z_ELEMENT), &pAd->pFtmCtrl->LciZ,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	pAd->pFtmCtrl->LciUsage.SubElement = LCI_RPTID_USAGE_RULES;
	/* pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetransAllowed = 1;  //should follow profile settings */
	pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetExpiresPresent = 0;
	pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.LocationPolicy = 0;
	pAd->pFtmCtrl->LciUsage.Length = pAd->pFtmCtrl->LciUsage.RulesAndPolicy.field.RetExpiresPresent ? \
									 (sizeof(USAGE_SUBELEMENT) - 2) :
									 (sizeof(USAGE_SUBELEMENT) - 2 \
									  - sizeof(pAd->pFtmCtrl->LciUsage.RetExpires));
	MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					  (pAd->pFtmCtrl->LciUsage.Length + 2), &pAd->pFtmCtrl->LciUsage,
					  END_OF_ARGS);
	FrameLen += TmpLen;
	SendLocationElementEvent(NetDev, pOutBuffer, FrameLen, AP_GEOSPATIAL_LOCATION);
	MlmeFreeMemory(pOutBuffer);
}

INT Set_FtmLciValue_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	PLCI_FIELD pLci = &pAd->pFtmCtrl->LciField;

	if (strlen(arg) != (sizeof(LCI_FIELD) * 2)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR, "(X) illegal size: LCI should be %lu hex bytes\n", sizeof(LCI_FIELD));
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "example:\n");
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "iwpriv ra0 set FtmLciValue=52834d12efd2b08b9b4bf1cc2c000041\n");
		return TRUE;
	}

	/* update */
	AtoH(arg, pLci->byte, sizeof(LCI_FIELD));
	/* dump all */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN, "Update LciField\n");
	FtmLciValueDump(pAd, pLci, DBG_LVL_WARN);
	return TRUE;
}

INT Set_FtmLciLat_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	PLCI_FIELD pLci = &pAd->pFtmCtrl->LciField;

	Set_FtmLciLat_Proc(pAd, arg, pLci);
	return TRUE;
}

INT Set_FtmLciLng_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	PLCI_FIELD pLci = &pAd->pFtmCtrl->LciField;

	Set_FtmLciLng_Proc(pAd, arg, pLci);
	return TRUE;
}

INT Set_FtmLciAlt_ThisAP_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	PLCI_FIELD pLci = &pAd->pFtmCtrl->LciField;

	Set_FtmLciAlt_Proc(pAd, arg, pLci);
	return TRUE;
}

INT Set_FtmLciLat_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg,
	IN PLCI_FIELD pLci
)
{
	int arg_len;
	UINT8 high = 0;
	UINT32 low = 0;

	arg = FtmSkipHexPrefix(arg);
	arg_len = strlen(arg);

	if (arg_len > 9) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR, "(X) illegal size of Latitude: exceed 34 bits\n");
		return TRUE;
	}

	if (arg_len == 9) {
		AtoH(arg, &high, 1);
		high = (high >> 4) & 3;	/* take bit 33 and 32 */
		arg++;
	}

	low = os_str_tol(arg, 0, 16);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
		"Updated Latitude: 0x%02X %08X\n\n", high, low);
	/* update */
	pLci->field.Latitude_b0_b1 = low & 0x3;
	pLci->field.Latitude_b2_b33 = ((UINT32)high << 30) | (low >> 2);
	/* dump all */
	FtmLciValueDump(pAd, pLci, DBG_LVL_WARN);
	return TRUE;
}


INT Set_FtmLciLng_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg,
	IN PLCI_FIELD pLci
)
{
	int arg_len;
	UINT8 high = 0;
	UINT32 low = 0;

	arg = FtmSkipHexPrefix(arg);
	arg_len = strlen(arg);

	if (arg_len > 9) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR, "(X) illegal size of Longitude: exceed 34 bits\n");
		return TRUE;
	}

	if (arg_len == 9) {
		AtoH(arg, &high, 1);
		high = (high >> 4) & 3;	/* take bit 33 and 32 */
		arg++;
	}

	low = os_str_tol(arg, 0, 16);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
		"Updated Longitude: 0x%02X %08X\n\n", high, low);
	/* update */
	pLci->field.Longitude_b0_b1 = low & 0x3;
	pLci->field.Longitude_b2_b33 = ((UINT32)high << 30) | (low >> 2);
	/* dump all */
	FtmLciValueDump(pAd, pLci, DBG_LVL_WARN);
	return TRUE;
}


INT Set_FtmLciAlt_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg,
	IN PLCI_FIELD pLci
)
{
	UINT32 value;

	arg = FtmSkipHexPrefix(arg);
	if (strlen(arg) > 8) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR, "(X) illegal size of Altitude: exceed 30 bits\n");
		return TRUE;
	}

	value = os_str_tol(arg, 0, 16);
	value &= 0x3FFFFFFF;	/* bit31 and bit30 are invalid, truncate them */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN, "Updated Altitude: 0x%08X\n\n", value);
	/* update */
	pLci->field.Altitude_b0_b21 = value & 0x3FFFFF;	/* mask bit0 ~ bit21 */
	pLci->field.Altitude_b22_b29 = value >> 22;
	/* dump all */
	FtmLciValueDump(pAd, pLci, DBG_LVL_WARN);
	return TRUE;
}


INT Set_FtmLciKnown_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 flag = os_str_tol(arg, 0, 10);

	if (flag) {
		pAd->pFtmCtrl->LciHdr.Length = sizeof(LCI_FIELD);
		FtmSendLciToDaemon(pAd);
	} else
		pAd->pFtmCtrl->LciHdr.Length = 0;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"%d => LciHdr.Length=%d\n", flag, pAd->pFtmCtrl->LciHdr.Length);
	return TRUE;
}


INT Set_FtmLciFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 flag = os_str_tol(arg, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"Request:%d->%d, Report:%d->%d\n",
		pAd->pFtmCtrl->bSetLciReq, flag, pAd->pFtmCtrl->bSetLciRpt, flag);
	pAd->pFtmCtrl->bSetLciReq = flag;
	pAd->pFtmCtrl->bSetLciRpt = flag;
	return TRUE;
}


INT Set_FtmCivicKnown_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 flag = os_str_tol(arg, 0, 10);

	if (flag) {
		pAd->pFtmCtrl->CivicHdr.Length = sizeof(LOCATION_CIVIC) + pAd->pFtmCtrl->Civic.CA_Length;
		FtmSendCivicToDaemon(pAd);
	} else
		pAd->pFtmCtrl->CivicHdr.Length = 0;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"%d => CivicHdr.Length=%d\n", flag, pAd->pFtmCtrl->CivicHdr.Length);
	return TRUE;
}


INT Set_FtmCivicFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 flag = os_str_tol(arg, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"Request:%d->%d, Report:%d->%d\n",
		pAd->pFtmCtrl->bSetCivicReq, flag, pAd->pFtmCtrl->bSetCivicRpt, flag);
	pAd->pFtmCtrl->bSetCivicReq = flag;
	pAd->pFtmCtrl->bSetCivicRpt = flag;
	return TRUE;
}


INT Set_FtmZFlag_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 flag = os_str_tol(arg, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"%d -> %d\n", pAd->pFtmCtrl->bSetZRpt, flag);
	pAd->pFtmCtrl->bSetZRpt = flag;
	return TRUE;
}


INT Show_FtmEntry_Proc(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev	*wdev,
	IN RTMP_STRING *arg
)
{
	UINT8 EntryIdx = 0;

	if (arg)
		EntryIdx = os_str_tol(arg, 0, 10);
	else
		EntryIdx = wdev->FtmCtrl.LatestJoinPeer;

	FtmEntryDump(pAd, wdev, &wdev->FtmCtrl.FtmPeer[EntryIdx], DBG_LVL_INFO);
	return TRUE;
}


INT Show_FtmLciValue_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg
)
{
	FtmLciValueDump(pAd, &pAd->pFtmCtrl->LciField, DBG_LVL_INFO);
	return TRUE;
}

void FtmMapSigmaCmdToLocLCI(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, PLCI_FIELD pLci)
{
	/*lci,		120d3649bc0312c32e6e2e01010f330b00000100000001*/
	/*tmpbuf     [0                                       ~						45]*/
	/*   Latitude uncertainty (1 byte)		6
		Latitude (5 bytes)					34 to set Lat
		Longitude uncertainty (1 byte)		6
		Longitude (5 bytes)				34 to set Lng
		Altitude type (1 byte)				4
		Altitude uncertainty (1 byte)		6
		Altitude (4 bytes)					30 to set Alt
		Datum (1 byte)					3
		RegLocAgreement (1 byte)			1
		RegLocDSE (1 byte)				1
		Dependent state (1 byte)			1
		Version (1 byte)					2

	*/

	int i = 0;
	UINT8 LciValue = 0;
	CHAR LciArg[20];
	/* map sigma 23 byte to LCI 16 byte */
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "\n");
	for (i = 0; i < 46; i++) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "\033[1;33m%c\033[0m", tmpbuf[i]);
	}
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "\n");

	AtoH(tmpbuf, &LciValue, 1);
	pLci->field.LatitudeUncertainty = LciValue & 0x3F;
	NdisZeroMemory(LciArg, 20);
	NdisCopyMemory(LciArg, tmpbuf + 2, 10);
	LciArg[0] = '0'; /* no need of bit 36~39 */
	Set_FtmLciLat_Proc(pAd, LciArg, pLci);

	AtoH(tmpbuf + 12, &LciValue, 1);
	pLci->field.LongitudeUncertainty = LciValue & 0x3F;
	NdisZeroMemory(LciArg, 20);
	NdisCopyMemory(LciArg, tmpbuf + 14, 10);
	LciArg[0] = '0'; /* no need of bit 36~39 */
	Set_FtmLciLng_Proc(pAd, LciArg, pLci);

	AtoH(tmpbuf + 24, &LciValue, 1);
	pLci->field.AltitudeType = LciValue & 0x0F;

	AtoH(tmpbuf + 26, &LciValue, 1);
	pLci->field.AltitudeUncertainty = LciValue & 0x3F;

	NdisZeroMemory(LciArg, 20);
	NdisCopyMemory(LciArg, tmpbuf + 28, 8);
	Set_FtmLciAlt_Proc(pAd, LciArg, pLci);

	AtoH(tmpbuf + 36, &LciValue, 1);
	pLci->field.Datum = LciValue & 0x07;

	AtoH(tmpbuf + 38, &LciValue, 1);
	pLci->field.RegLocAgreement = LciValue & 0x01;

	AtoH(tmpbuf + 40, &LciValue, 1);
	pLci->field.RegLocDSE = LciValue & 0x01;

	AtoH(tmpbuf + 42, &LciValue, 1);
	pLci->field.Dependent = LciValue & 0x01;

	AtoH(tmpbuf + 44, &LciValue, 1);
	pLci->field.STAVersion = LciValue & 0x03;

	FtmLciValueDump(pAd, pLci, DBG_LVL_WARN);
	return;
}


VOID FtmProfileNeighborApParse(RTMP_ADAPTER *pAd, UINT NeighborIdx, RTMP_STRING *tmpbuf)
{
	RTMP_STRING *NeighborInfo = NULL, *Nvalue = NULL;
	UINT8 j = 0, k = 0;
	UCHAR *tmpNeighborInfo;

	if (os_alloc_mem(pAd, (UINT8 **)&tmpNeighborInfo, 1024) != NDIS_STATUS_SUCCESS || !tmpNeighborInfo)
		return;

	pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborValid = TRUE;

	for (j = 0, NeighborInfo = rstrtok(tmpbuf, ";"); NeighborInfo && j < MAX_CIVIC_CA_VALUE_LENGTH; NeighborInfo = rstrtok(NULL, ";"), j++) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
				"NeighborInfo: %s\n", NeighborInfo);

		if (strncmp(NeighborInfo, "Bssid:", 6) == 0) {
			NdisZeroMemory(tmpNeighborInfo, 1024);
			NdisCopyMemory(tmpNeighborInfo, NeighborInfo, 1024);
		} else {
			CHAR *pch = NULL;

			if ((strstr(NeighborInfo, "PHYtype") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborPhyType = (UINT8)os_str_tol(pch + 1, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "NeighborPhyType: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborPhyType);
			} else if ((strstr(NeighborInfo, "FTMinBssidInfo") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborFTMCap = (UINT8)os_str_tol(pch + 1, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "NeighborFTMCap: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborFTMCap);
			} else if ((strstr(NeighborInfo, "OpChannel") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborChannel = (UINT8)os_str_tol(pch + 1, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "NeighborChannel: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborChannel);
			} else if ((strncmp(NeighborInfo, "LocCivicAddr:", 13) == 0)) {
				pch = strchr(NeighborInfo, ':');

				for (j = 0; j < MAX_CIVIC_CA_VALUE_LENGTH && *(pch + 1 + j * 2) != 0; j++)
					AtoH(pch + 1 + j * 2, &pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].CA_Value[j], 1);

				/* hex_dump_my("NeighborLocCA",pAd->pFtmCtrl->CA_Value,MAX_CIVIC_CA_VALUE_LENGTH); */
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "LocCivicAddr: %s\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].CA_Value);
			} else if ((strstr(NeighborInfo, "LocCivicAddrType") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Type = (UINT8)os_str_tol(pch + 1, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "LocCivicAddrType: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Type);
			} else if ((strstr(NeighborInfo, "LocCivicAddrLength") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length = (UINT8)os_str_tol(pch + 1, 0, 10);

				if (pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length >= MAX_CIVIC_CA_VALUE_LENGTH)
					pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length = MAX_CIVIC_CA_VALUE_LENGTH;

				if (pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length == 0)
					pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].CivicHdr.Length = 0;
				else
					pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].CivicHdr.Length = sizeof(LOCATION_CIVIC) + pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length;

				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "LocCivicAddrLength: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].Civic.CA_Length);
			} else if ((strstr(NeighborInfo, "lci") != 0) || (strstr(NeighborInfo, "LCI") != 0)) {
				UCHAR LciField[50]; /* sigma lci is either 1 byte or 46 byte long */

				pch = strchr(NeighborInfo, ':');

				if (strlen(pch + 1) == 46) {
					NdisCopyMemory(LciField, pch + 1, strlen(pch + 1));
					pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciHdr.Length = sizeof(LCI_FIELD);
					FtmMapSigmaCmdToLocLCI(pAd, LciField, &pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciField);
				} else
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "Neighbor lci length: %lu, remain unknown\n", strlen(pch + 1));
			} else if ((strstr(NeighborInfo, "FloorInfoZ") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.Floor.word = (UINT16)os_str_tol(pch + 1, 0, 10);
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].bSetNeighbotZRpt = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "FloorInfoZ: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.Floor.word);
			} else if ((strstr(NeighborInfo, "HeightAboveFloorZ") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.HeightAboveFloor = (UINT16)os_str_tol(pch + 1, 0, 10);
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].bSetNeighbotZRpt = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "HeightAboveFloorZ: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.HeightAboveFloor);
			} else if ((strstr(NeighborInfo, "HeightAboveFloorUncZ") != 0)) {
				pch = strchr(NeighborInfo, ':');
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.HeightUncertainty = (UINT8)os_str_tol(pch + 1, 0, 10);
				pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].bSetNeighbotZRpt = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "HeightAboveFloorUncZ: %d\n", pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].LciZ.HeightUncertainty);
			}
		}
	}

	for (j = 0, Nvalue = rstrtok(tmpNeighborInfo, ":"); Nvalue && j < MAX_CIVIC_CA_VALUE_LENGTH; Nvalue = rstrtok(NULL, ":"), j++) {
		if ((strlen(Nvalue) != 2) || (!isxdigit(*Nvalue)) || (!isxdigit(*(Nvalue + 1))))
			continue;  /*Invalid */

		AtoH(Nvalue, &pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborBSSID[k++], 1);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG, "Neighbor[%d]: "MACSTR"\n"
			 , NeighborIdx, MAC2STR(pAd->pFtmCtrl->FtmNeighbor[NeighborIdx].NeighborBSSID));

	os_free_mem(tmpNeighborInfo);
}

INT Send_ANQP_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UCHAR *Buf, *buf_anqp, *Pos;
	/* RTMP_STRING *value; */
	GAS_FRAME *GASFrame;
	UINT32 FrameLen = 0;
	UINT16 tmpLen = 0, tmp = 0; /* ,i; */
	static UINT32 Token;
	UINT32 anqp_req_len = 0;
	struct anqp_frame *anqp_req;
	UCHAR PeerMACAddr[MAC_ADDR_LEN] = {0x00, 0x0C, 0x43, 0xE1, 0x76, 0x28};
	ULONG choice = os_str_tol(arg, 0, 10);

	printk("%s  choice  %ld\n", __func__, choice);

	/* NdisZeroMemory(PeerMACAddr, MAC_ADDR_LEN); */
	/* query_ap_geospatial_location  , query_ap_civic_location ,query_ap_location_public_uri */
	if (choice == 1) {
		os_alloc_mem(NULL, (UCHAR **)&buf_anqp, sizeof(*anqp_req) + 2);
		if (buf_anqp != NULL) {
			anqp_req = (struct anqp_frame *)buf_anqp;
			anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
			anqp_req_len += 2;
			anqp_req->length = cpu2le16(2);
			anqp_req_len += 2;
			Pos = anqp_req->variable;
			tmp = cpu2le16(ROAMING_CONSORTIUM_LIST);
			/* tmp = cpu2le16(AP_GEOSPATIAL_LOCATION); */
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;
			/*
			tmp = cpu2le16(AP_CIVIC_LOCATION);
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;

			tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;

			anqp_req_len += 6;
			*/
			anqp_req_len += 2;
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"Fail to allocate memory!\n");
			return FALSE;
		}
	} else {
		os_alloc_mem(NULL, (UCHAR **)&buf_anqp, sizeof(*anqp_req) + 6);
		if (buf_anqp != NULL) {
			anqp_req = (struct anqp_frame *)buf_anqp;
			anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
			anqp_req_len += 2;
			anqp_req->length = cpu2le16(6);
			anqp_req_len += 2;
			Pos = anqp_req->variable;
			tmp = cpu2le16(AP_GEOSPATIAL_LOCATION);
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;
			tmp = cpu2le16(AP_CIVIC_LOCATION);
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;
			tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
			NdisMoveMemory(Pos, &tmp, 2);
			Pos += 2;
			anqp_req_len += 6;
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					"Fail to allocate memory!\n");
			return FALSE;
		}
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASFrame) + anqp_req_len);

	if (!Buf)
		goto error0;

	NdisZeroMemory(Buf, sizeof(*GASFrame) + anqp_req_len);
	GASFrame = (GAS_FRAME *)Buf;
	ActHeaderInit(pAd, &GASFrame->Hdr, PeerMACAddr, pAd->CurrentAddress,
				  PeerMACAddr);
	FrameLen += sizeof(HEADER_802_11);
	GASFrame->Category = CATEGORY_PUBLIC;
	GASFrame->u.GAS_INIT_REQ.Action = ACTION_GAS_INIT_REQ;
	GASFrame->u.GAS_INIT_REQ.DialogToken = Token++; /* Event->u.GAS_REQ_DATA.DialogToken; */
	FrameLen += 3;
	Pos = GASFrame->u.GAS_INIT_REQ.Variable;
	*Pos++ = IE_ADVERTISEMENT_PROTO;
	*Pos++ = 2; /* Length field */
	*Pos++ = 0; /* Query response info field */
	*Pos++ = ACCESS_NETWORK_QUERY_PROTOCOL; /* Advertisement Protocol ID field */
	tmpLen = cpu2le16(anqp_req_len);
	NdisMoveMemory(Pos, &tmpLen, 2);
	Pos += 2;
	FrameLen += 6;
	NdisMoveMemory(Pos, buf_anqp, anqp_req_len);
	FrameLen += anqp_req_len;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"Location Anqp Req to "MACSTR"\n", MAC2STR(PeerMACAddr));
	MiniportMMRequest(pAd, 0, Buf, FrameLen, NULL);
	os_free_mem(Buf);
	os_free_mem(buf_anqp);
	return TRUE;
error0:
	os_free_mem(buf_anqp);
	return FALSE;
}

INT Send_NeighborReq_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT8 DialogToken = RandomByte(pAd);
	UINT8 MeasurementToken = RandomByte(pAd);

	if (FtmGetTargetAddr(pAd, arg) == FALSE)
		return FALSE;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "\n");
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"allocate memory failed\n");
		return FALSE;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->pFtmCtrl->Responder,
					pAd->ApCfg.MBSSID[BSS0].wdev.if_addr, pAd->ApCfg.MBSSID[BSS0].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &ActHdr, END_OF_ARGS);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_RM, RRM_NEIGHTBOR_REQ);

	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	{
		ULONG TempLen;
		UCHAR ElemetnID = 38, measurementType = MSMT_LCI_RPT, length = 5, MeasurementMode = 0;

		MakeOutgoingFrame((pOutBuffer + FrameLen),	&TempLen,
						  1,				&ElemetnID,
						  1,				&length,
						  1,				&MeasurementToken,
						  1,				&MeasurementMode,
						  1,				&measurementType,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen, NULL);

	if (pOutBuffer) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"sent out Neighbor Req.\n");
		MlmeFreeMemory(pOutBuffer);
	}

	return TRUE;
}

INT Set_FtmRMRandomizationInterval_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg
)
{
	UINT16 value = os_str_tol(arg, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			 "RandomizationInterval %d\n", pAd->pFtmCtrl->RandomizationInterval);
	pAd->pFtmCtrl->RandomizationInterval = value;
	return TRUE;
}
INT Set_FtmRMMinimumApCount_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg
)
{
	UINT8 value = os_str_tol(arg, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"MinimumApCount %d\n", pAd->pFtmCtrl->MinimumApCount);
	pAd->pFtmCtrl->MinimumApCount = value;
	return TRUE;
}

INT Send_RadioMeasurement_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT8 DialogToken = RandomByte(pAd);
	UINT8 MeasurementToken = RandomByte(pAd);
	UINT  i = 0;
	PFTM_CTRL	pFtm = pAd->pFtmCtrl;
	BSS_STRUCT *pMbss;
	RRM_BSSID_INFO BssidInfo;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 CondensedPhyType = (WMODE_CAP_6G(wdev->PhyMode) || WMODE_CAP_5G(wdev->PhyMode)) ? MODE_HE_EXT_SU : MODE_HE_24G; /* 7:2G, 9:5G */
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, "\n");

	if (FtmGetTargetAddr(pAd, arg) == FALSE)
		return FALSE;

	pMbss = &pAd->ApCfg.MBSSID[BSS0];
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"allocate memory failed\n");
		return FALSE;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pFtm->Responder, pMbss->wdev.if_addr, pMbss->wdev.bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &ActHdr, END_OF_ARGS);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_RM, RRM_MEASURE_REQ);
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	{
		ULONG TempLen;
		UCHAR ElementID = 38, measurementType = MSMT_FTM_RANGE_REQ, MeasurementMode = 0;
		UCHAR length = 6 + 15 * pFtm->MinimumApCount;
		UINT16 NumberOfRepetitions = 0;

		MakeOutgoingFrame((pOutBuffer + FrameLen),	&TempLen,
						  2,				&NumberOfRepetitions,
						  1,				&ElementID,
						  1,				&length,
						  1,				&MeasurementToken,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
		MakeOutgoingFrame((pOutBuffer + FrameLen),	&TempLen,
						  1,				&MeasurementMode,
						  1,				&measurementType,
						  2,				&pFtm->RandomizationInterval,
						  1,				&pFtm->MinimumApCount,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* our own info */
	BssidInfo.word = 0;
	BssidInfo.field.APReachAble = 3;
	BssidInfo.field.Security = 0; /* rrm to do. */
	BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
	BssidInfo.field.SpectrumMng = (pMbss->CapabilityInfo & (1 << 8)) ? 1 : 0;
	BssidInfo.field.Qos = (pMbss->CapabilityInfo & (1 << 9)) ? 1 : 0;
	BssidInfo.field.APSD = (pMbss->CapabilityInfo & (1 << 11)) ? 1 : 0;
#ifdef DOT11K_RRM_SUPPORT
	BssidInfo.field.RRM = (pMbss->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
#endif
	BssidInfo.field.DelayBlockAck = (pMbss->CapabilityInfo & (1 << 14)) ? 1 : 0;
	BssidInfo.field.ImmediateBA = (pMbss->CapabilityInfo & (1 << 15)) ? 1 : 0;
	BssidInfo.field.FTM = 1;
	COPY_MAC_ADDR(NeighborRepInfo.Bssid, pMbss->wdev.bssid);
	NeighborRepInfo.BssidInfo = BssidInfo.word;
	NeighborRepInfo.RegulatoryClass = get_regulatory_class(pAd, pMbss->wdev.channel, pMbss->wdev.PhyMode, &pMbss->wdev);
	NeighborRepInfo.ChNum = pMbss->wdev.channel;
	NeighborRepInfo.PhyType = CondensedPhyType;
	RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
							sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);

	/* neighbor info */
	for (i = 0; i < (pFtm->MinimumApCount - 1); i++) {
		PFTM_NEIGHBORS pFtmNeighbor = &pAd->pFtmCtrl->FtmNeighbor[i];

		BssidInfo.word = 0;
		BssidInfo.field.APReachAble = 3;
		BssidInfo.field.Security = 0; /* rrm to do. */
		BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
		BssidInfo.field.SpectrumMng = (pMbss->CapabilityInfo & (1 << 8)) ? 1 : 0;
		BssidInfo.field.Qos = (pMbss->CapabilityInfo & (1 << 9)) ? 1 : 0;
		BssidInfo.field.APSD = (pMbss->CapabilityInfo & (1 << 11)) ? 1 : 0;
#ifdef DOT11K_RRM_SUPPORT
		BssidInfo.field.RRM = (pMbss->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
#endif
		BssidInfo.field.DelayBlockAck = (pMbss->CapabilityInfo & (1 << 14)) ? 1 : 0;
		BssidInfo.field.ImmediateBA = (pMbss->CapabilityInfo & (1 << 15)) ? 1 : 0;
		BssidInfo.field.FTM = pFtmNeighbor->NeighborFTMCap;
		COPY_MAC_ADDR(NeighborRepInfo.Bssid, pFtmNeighbor->NeighborBSSID);
		NeighborRepInfo.BssidInfo = BssidInfo.word;
		NeighborRepInfo.RegulatoryClass = pFtmNeighbor->NeighborOpClass;
		NeighborRepInfo.ChNum = pFtmNeighbor->NeighborChannel;
		NeighborRepInfo.PhyType = pFtmNeighbor->NeighborPhyType;
		RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
								sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);
	}

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen, NULL);

	if (pOutBuffer) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"sent out RadioMeasurement Req. FrameLen:%ld\n", FrameLen);
		hex_dump("RM OUT", pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
	}

	return TRUE;
}

#endif /* FTM_SUPPORT */
