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
	andes_mt.c
*/

#include	"rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include    "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#define PHY_MODE_CCK                        0
#define PHY_MODE_OFDM                       1
#define PHY_MODE_HTMIX                      2
#define PHY_MODE_HTGREENFIELD               3
#define PHY_MODE_VHT                        4
#define PHY_MODE_HESU                       8
#define PHY_MODE_HEEXTSU                    9
#define PHY_MODE_HETRIG                     10
#define PHY_MODE_HEMU                       11
#define PHY_MODE_HESU_REMAPPING             5
#define PHY_MODE_HEEXTSU_REMAPPING          6
#define PHY_MODE_HETRIG_REMAPPING           7
#define PHY_MODE_HEMU_REMAPPING             8
#define PHY_MODE_UNKNOWN_REMAPPING          9
#define TX_NSS_SHITF                        6
#define TX_NSS_MASK                         0x7
#define TX_HT_MCS_MASK                      0x3F
#define TX_NON_HT_MCS_MASK                  0xF
#define TX_MODE_SHIFT                       9
#define TX_MODE_MASK                        0xF
#define DCM_SHITF                           4
#define DCM_EN                              (1 << DCM_SHITF)
#define TX_BW_SHIFT                         13
#define TX_BW_MASK                          0x3

static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
}

static VOID EventChPrivilegeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "do nothing now\n");
}

#ifdef CONFIG_STA_SUPPORT

static VOID ExtEventRoamingDetectionHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_ROAMING_DETECT_RESULT_T *pExtEventRoaming =
		(struct _EXT_EVENT_ROAMING_DETECT_RESULT_T *)Data;
	pAd->StaCfg[0].PwrMgmt.bTriggerRoaming = TRUE;
	pExtEventRoaming->u4RoamReason = le2cpu32(pExtEventRoaming->u4RoamReason);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "FW LOG, ucBssidIdx = %d,  u4RoamReason = %d\n",
			  pExtEventRoaming->ucBssidIdx,
			  pExtEventRoaming->u4RoamReason);
}
#endif /*CONFIG_STA_SUPPORT*/

static VOID ExtEventBeaconLostHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = NULL;
#endif
	struct _EXT_EVENT_BEACON_LOSS_T *pExtEventBeaconLoss =
		(struct _EXT_EVENT_BEACON_LOSS_T *)Data;
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
			 "FW EVENT ("MACSTR"), Reason 0x%x\n",
			  MAC2STR(pExtEventBeaconLoss->aucBssid),
			  pExtEventBeaconLoss->ucReason);

	switch (pExtEventBeaconLoss->ucReason) {
#ifdef CONFIG_AP_SUPPORT

	case ENUM_BCN_LOSS_AP_DISABLE:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"  AP Beacon OFF!!!\n");
		break;

	case ENUM_BCN_LOSS_AP_SER_TRIGGER:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"  SER happened!!!\n");
		break;

	case ENUM_BCN_LOSS_AP_ERROR:
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"  Beacon lost - Error!!! Re-issue BCN_OFFLOAD cmd\n");
		/* update Beacon again if operating in AP mode. */
		wdev = wdev_search_by_address(pAd, pExtEventBeaconLoss->aucBssid);
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "Wrong Addr1 - "MACSTR"\n",
					 MAC2STR(pExtEventBeaconLoss->aucBssid));
			break;
		}

		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			break;

		/* do BCN_UPDATE_ALL_AP_RENEW when all BSS CSA done */
		if (pDot11h->csa_ap_bitmap == 0)
			UpdateBeaconHandler(pAd, get_default_wdev(pAd), BCN_REASON(BCN_UPDATE_ALL_AP_RENEW));
		else
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				" CSA is running, wait it done, ChannelMode=%d, csa_ap_bitmap=0x%x\n",
				pDot11h->ChannelMode, pDot11h->csa_ap_bitmap);
		break;
#endif
#ifdef CONFIG_STA_SUPPORT

	case ENUM_BCN_LOSS_STA: {
		UCHAR	i = 0;
		PSTA_ADMIN_CONFIG pStaCfg = NULL;

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "  Beacon lost - STA!!!\n");

		/* Find pStaCfg */
		for (i = 0; i < pAd->MSTANum; i++) {
			pStaCfg = &pAd->StaCfg[i];
			ASSERT(pStaCfg);

			if (pStaCfg->wdev.DevInfo.WdevActive) {
				if (NdisEqualMemory(pExtEventBeaconLoss->aucBssid, pStaCfg->Bssid, MAC_ADDR_LEN)) {
					MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
							 "Found StaCfg[%d] Bssid matching\n", i);
					break;
				}
			}
		}

		if (i == pAd->MSTANum) {
			ASSERT(0);
			return;
		}

		/* Upate pStaCfg */
		pStaCfg->PwrMgmt.bBeaconLost = TRUE;
		break;
	}

#endif

	default:
		break;
	}
}

#ifdef MT_DFS_SUPPORT
static VOID ExtEventRddReportHandler(RTMP_ADAPTER *pAd,
									 UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_RDD_REPORT_T *pExtEventRddReport =
		(struct _EXT_EVENT_RDD_REPORT_T *)Data;
	UCHAR rddidx = HW_RDD0;

	if (!pExtEventRddReport)
		return;

	rddidx = pExtEventRddReport->rdd_idx;

	/* update dbg pulse info */
	dfs_update_radar_info(pExtEventRddReport);

	if (pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en == TRUE)
		dfs_dump_radar_sw_pls_info(pAd, pExtEventRddReport);

	if (pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en == TRUE)
		dfs_dump_radar_hw_pls_info(pAd, pExtEventRddReport);

	if ((pAd->CommonCfg.DfsParameter.is_radar_emu == TRUE) ||
			(pExtEventRddReport->lng_pls_detected == TRUE) ||
			(pExtEventRddReport->cr_pls_detected == TRUE) ||
			(pExtEventRddReport->stgr_pls_detected == TRUE))
		WrapDfsRddReportHandle(pAd, rddidx);
}

#endif

static VOID ExtEventIdlePwrReportHandler(RTMP_ADAPTER *pAd,
									 UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_IDLE_PWR_GET_T *pExtEventIdlePwrReport =
		(struct _EXT_EVENT_IDLE_PWR_GET_T *)Data;

	if (!pExtEventIdlePwrReport)
		return;

	MTWF_PRINT("band: %d, obss percentage: %d, ipi percentage: %d\n",
			pExtEventIdlePwrReport->band_idx,
			pExtEventIdlePwrReport->obss_percent,
			pExtEventIdlePwrReport->ipi_percent);
}


#ifdef WIFI_SPECTRUM_SUPPORT
/*
	==========================================================================
	Description:
	Unsolicited extend event handler of wifi-spectrum.
	Return:
	==========================================================================
*/
static VOID ExtEventWifiSpectrumHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_SPECTRUM_RESULT_T *pResult = (EXT_EVENT_SPECTRUM_RESULT_T *)pData;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"FuncIndex = %d\n", pResult->u4FuncIndex);
	pResult->u4FuncIndex = le2cpu32(pResult->u4FuncIndex);
	switch (pResult->u4FuncIndex) {
	case SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, (VOID *)pData, Length);
		break;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Cmd queue raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
NTSTATUS WifiSpectrumRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	NTSTATUS ret = NDIS_STATUS_FAILURE;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->SpectrumEventRawDataHandler) {
		ops->SpectrumEventRawDataHandler(
			pAd, (UINT8 *)CMDQelmt->buffer, (UINT32)CMDQelmt->bufferlength);
		ret = NDIS_STATUS_SUCCESS;
	}

	return ret;
}

/*
	==========================================================================
	Description:
	Extend event raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
VOID ExtEventWifiSpectrumUnSolicitRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	UINT32 i, CapNode, Data;
	UINT8 msg_IQ[CAP_FILE_MSG_LEN], msg_Gain[CAP_FILE_MSG_LEN];
	INT32 retval, Status;
	INT16 I_0, Q_0, LNA, LPF;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 BankNum = pChipCap->SpectrumBankNum;
	RBIST_DESC_T *pSpectrumDesc = &pChipCap->pSpectrumDesc[0];
	EXT_EVENT_RBIST_DUMP_DATA_T *pSpectrumEvent = NULL;
	int ret;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pSpectrumEvent */
	pSpectrumEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);

	/* If file is closed, we need to drop this packet. */
	if ((pAd->pSrcf_IQ == NULL) || (pAd->pSrcf_Gain == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"\x1b[31m File is already closed!!\x1b[m\n");
		return;
	}

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it.*/
	if (pSpectrumEvent->u4PktNum > pAd->SpectrumEventCnt) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				pSpectrumEvent->u4PktNum, pAd->SpectrumEventCnt);
		return;
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	/* Dump I/Q/LNA/LPF data to file */
	for (i = 0; i < SPECTRUM_EVENT_DATA_SAMPLE; i++) {
		Data = le2cpu32(pSpectrumEvent->u4Data[i]);
		os_zero_mem(msg_IQ, CAP_FILE_MSG_LEN);
		os_zero_mem(msg_Gain, CAP_FILE_MSG_LEN);
		/* Parse I/Q/LNA/LPF data and dump these data to file */
		CapNode = Get_System_CapNode_Info(pAd);
		if ((CapNode == pChipCap->SpectrumWF0ADC) || (CapNode == pChipCap->SpectrumWF1ADC)
			|| (CapNode == pChipCap->SpectrumWF2ADC) || (CapNode == pChipCap->SpectrumWF3ADC)) { /* Dump 1-way RXADC */
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"Dump 1-way RXADC\n");

			if (pSpectrumDesc->ucADCRes == 10) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0x3FF);
				I_0 = ((Data & (0x3FF << 10)) >> 10);

				if (Q_0 >= 512)
					Q_0 -= 1024;

				if (I_0 >= 512)
					I_0 -= 1024;

				ret = snprintf(msg_IQ, CAP_FILE_MSG_LEN, "%+04d\t%+04d\n", I_0, Q_0);
				if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				ret = snprintf(msg_Gain, CAP_FILE_MSG_LEN, "%+04d\t%+04d\n", LNA, LPF);
				if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		} else if ((CapNode == pChipCap->SpectrumWF0FIIQ) || (CapNode == pChipCap->SpectrumWF1FIIQ)
				   || (CapNode == pChipCap->SpectrumWF2FIIQ) || (CapNode == pChipCap->SpectrumWF3FIIQ)
				   || (CapNode == pChipCap->SpectrumWF0FDIQ) || (CapNode == pChipCap->SpectrumWF1FDIQ)
				   || (CapNode == pChipCap->SpectrumWF2FDIQ) || (CapNode == pChipCap->SpectrumWF3FDIQ)) { /* Dump 1-way RXIQC */
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"Dump 1-way RXIQC\n");

			if (pSpectrumDesc->ucIQCRes == 12) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0xFFF);
				I_0 = ((Data & (0xFFF << 12)) >> 12);

				if (Q_0 >= 2048)
					Q_0 -= 4096;

				if (I_0 >= 2048)
					I_0 -= 4096;

				ret = snprintf(msg_IQ, CAP_FILE_MSG_LEN, "%+05d\t%+05d\n", I_0, Q_0);
				if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				ret = snprintf(msg_Gain, CAP_FILE_MSG_LEN, "%+04d\t%+04d\n", LNA, LPF);
				if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		}

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"0x%08x\n", Data);
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	/* Update SpectrumEventCnt */
	pAd->SpectrumEventCnt++;

	/* Update pSpectrumDesc */
	pSpectrumDesc = &pChipCap->pSpectrumDesc[pAd->SpectrumIdx];

	/* Check whether is the last FW event of data query in the same bank */
	if (pAd->SpectrumEventCnt == pSpectrumDesc->u4BankSize) {
		/* Check whether is the last bank or not */
		if ((pAd->SpectrumIdx + 1) == BankNum) {
			/* Print log to console to indicate data process done */
			{
				UINT32 TotalSize = 0;

				for (i = 0; i < BankNum; i++) {
					/* Update pSpectrumDesc */
					pSpectrumDesc = &pChipCap->pSpectrumDesc[i];
					/* Calculate total size  */
					TotalSize = TotalSize + pSpectrumDesc->u4BankSize;
				}
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"\x1b[31m: Dump %d K done !! \x1b[m\n", TotalSize);
			}
			/* Update status */
			pAd->SpectrumStatus = CAP_SUCCESS;
		}

		/* Reset SpectrumEventCnt */
		pAd->SpectrumEventCnt = 0;
		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Extend event I/Q data handler of wifi-spectrum.
	Return:
	==========================================================================
*/

VOID ExtEventWifiSpectrumUnSolicitIQDataHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	UINT32 Idxi, Type;
	UINT8 msg_IQ[CAP_FILE_MSG_LEN], msg_Gain[CAP_FILE_MSG_LEN], msg_Data[CAP_FILE_MSG_LEN];
	INT32 retval, Status, Data = 0, I = 0, Q = 0, LNA = 0, LPF = 0;
	RTMP_OS_FS_INFO osFSInfo;
	int ret;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_EVENT_ID_SPECTRUM_DATA_T *pSpectrumEvent = NULL;
#else
	EXT_EVENT_RBIST_DUMP_DATA_T * pSpectrumEvent = NULL;
#endif
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pSpectrumEvent */
#ifdef WIFI_UNIFIED_COMMAND
	pSpectrumEvent = (struct UNI_EVENT_ID_SPECTRUM_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);
	pSpectrumEvent->u4DataLen = le2cpu32(pSpectrumEvent->u4DataLen);
#else
	pSpectrumEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);
	pSpectrumEvent->u4DataLen = le2cpu32(pSpectrumEvent->u4DataLen);
#endif

	/* If file is closed, we need to drop this packet. */
	if ((pAd->pSrcf_IQ == NULL) || (pAd->pSrcf_Gain == NULL) || (pAd->pSrcf_InPhySniffer == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"\x1b[31m File is already closed!!\x1b[m\n");
		return;
	}

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it. */
	if (pSpectrumEvent->u4PktNum > pAd->SpectrumEventCnt) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"\x1b[31m Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				 pSpectrumEvent->u4PktNum, pAd->SpectrumEventCnt);
		return;
	}

	if (pSpectrumEvent->u4DataLen != 0) {
		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		Type = (pAd->SpectrumCapNode & WF_COMM_CR_CAP_NODE_TYPE_MASK) >> WF_COMM_CR_CAP_NODE_TYPE_SHFT;
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[32m : Capture Type = %d\x1b[m\n", Type);

		/* Dump data to file */
		for (Idxi = 0; Idxi < pSpectrumEvent->u4DataLen; Idxi++) {
			Data = (INT32)le2cpu32(pSpectrumEvent->u4Data[Idxi]);

			if (Type == CAP_INPHYSNIFFER_TYPE) {
				os_zero_mem(msg_Data, CAP_FILE_MSG_LEN);

				ret = snprintf(msg_Data, CAP_FILE_MSG_LEN, "%08x\n", Data);
				if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				retval = RtmpOSFileWrite(pAd->pSrcf_InPhySniffer, (RTMP_STRING *)msg_Data, strlen(msg_Data));
			} else {
				os_zero_mem(msg_IQ, CAP_FILE_MSG_LEN);
				os_zero_mem(msg_Gain, CAP_FILE_MSG_LEN);

				if ((Idxi % 4) == 0)
					I = Data;
				if ((Idxi % 4) == 1)
					Q = Data;
				if ((Idxi % 4) == 2)
					LPF = Data;
				if ((Idxi % 4) == 3) {
					LNA = Data;

					ret = snprintf(msg_IQ, CAP_FILE_MSG_LEN, "%+05d\t%+05d\n", I, Q);
					if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
							"snprintf error!\n");
						break;
					}
					retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));

					ret = snprintf(msg_Gain, CAP_FILE_MSG_LEN, "%+04d\t%+04d\n", LNA, LPF);
					if (os_snprintf_error(CAP_FILE_MSG_LEN, ret)) {
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
							"snprintf error!\n");
						break;
					}
					retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
				}
			}
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"%d\n", Data);
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

	/* Check whether is the last FW event or not */
	if ((pSpectrumEvent->u4DataLen == 0)
		&& (pSpectrumEvent->u4PktNum == pAd->SpectrumEventCnt)) {

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m : Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, pAd->SpectrumEventCnt);

		/* Reset SpectrumEventCnt */
		pAd->SpectrumEventCnt = 0;

		/* Update Spectrum overall status */
		pAd->SpectrumStatus = CAP_SUCCESS;

		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
	} else {
		/* Update SpectrumEventCnt */
		pAd->SpectrumEventCnt++;
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/*
	==========================================================================
	Description:
	Cmd queue raw data handler of ICAP.
	Return:
	==========================================================================
*/
NTSTATUS ICapRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	NTSTATUS ret = NDIS_STATUS_FAILURE;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->ICapEventRawDataHandler) {
		ops->ICapEventRawDataHandler(
				pAd, (UINT8 *)CMDQelmt->buffer, (UINT32)CMDQelmt->bufferlength);
		ret = NDIS_STATUS_SUCCESS;
	}

	return ret;
}

/*
	==========================================================================
	Description:
	Extend event of 96-bit raw data parser of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICap96BitDataParser(
	IN RTMP_ADAPTER *pAd)
{
	INT32 retval, Status;
	UINT32 i, j, StopPoint, CapNode;
	BOOLEAN Wrap;
	PUINT32 pTemp_L32Bit = NULL, pTemp_M32Bit = NULL, pTemp_H32Bit = NULL;
	RTMP_REG_PAIR RegStartAddr, RegStopAddr, RegWrap;
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Get RBIST start address */
	os_zero_mem(&RegStartAddr, sizeof(RegStartAddr));
	RegStartAddr.Register = RBISTCR2;
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		UniCmdMultipleMacRegAccessRead(pAd, &RegStartAddr, 1);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMacRegAccessRead(pAd, &RegStartAddr, 1);
	/* Get RBIST stop address */
	os_zero_mem(&RegStopAddr, sizeof(RegStopAddr));
	RegStopAddr.Register = RBISTCR9;
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		UniCmdMultipleMacRegAccessRead(pAd, &RegStopAddr, 1);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMacRegAccessRead(pAd, &RegStopAddr, 1);
	/* Calculate stop point */
	StopPoint = (RegStopAddr.Value - RegStartAddr.Value) / 4;
	/* Get RBIST wrapper */
	os_zero_mem(&RegWrap, sizeof(RegWrap));
	RegWrap.Register = RBISTCR0;
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		UniCmdMultipleMacRegAccessRead(pAd, &RegWrap, 1);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMacRegAccessRead(pAd, &RegWrap, 1);
	Wrap = ((RegWrap.Value & BIT(ICAP_WRAP)) >> ICAP_WRAP);
	MTWF_PRINT("\x1b[42m RBISTCR2: 0x%08x, RBISTCR9: 0x%08x, Wrap: %d \x1b[m\n",
			RegStartAddr.Value, RegStopAddr.Value, Wrap);

	/* Re-arrange each buffer by stop address and wrapper */
	if (!Wrap) {
		UINT32 Len, Offset;

		Len = ((BankNum / 3) * BankSmplCnt - StopPoint - 1) * sizeof(UINT32);
		Offset = StopPoint + 1;
		/* Set the rest of redundant data to zero */
		os_zero_mem((pAd->pL32Bit + Offset), Len);
		os_zero_mem((pAd->pM32Bit + Offset), Len);
		os_zero_mem((pAd->pH32Bit + Offset), Len);
	} else {
		UINT32 Len, Offset;

		/* Dynamic allocate memory for pTemp_L32Bit */
		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_L32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
			goto error;
		}

		/* Dynamic allocate memory for pTemp_M32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_M32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
			goto error;
		}

		/* Dynamic allocate memory for pTemp_H32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_H32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
			goto error;
		}

		/* Initialization of data buffer of Temp_L32Bit/Temp_M32Bit/Temp_H32Bit */
		os_zero_mem(pTemp_L32Bit, Len);
		os_zero_mem(pTemp_M32Bit, Len);
		os_zero_mem(pTemp_H32Bit, Len);
		os_move_mem(pTemp_L32Bit, pAd->pL32Bit, Len);
		os_move_mem(pTemp_M32Bit, pAd->pM32Bit, Len);
		os_move_mem(pTemp_H32Bit, pAd->pH32Bit, Len);

		for (i = 0; i < (Len / sizeof(UINT32)); i++) {
			/* Re-arrange data buffer of L32Bit/M32Bit/H32Bit */
			Offset = (StopPoint + 1 + i) % (Len / sizeof(UINT32));
			*(pAd->pL32Bit + i) = *(pTemp_L32Bit + Offset);
			*(pAd->pM32Bit + i) = *(pTemp_M32Bit + Offset);
			*(pAd->pH32Bit + i) = *(pTemp_H32Bit + Offset);
		}
	}

	/* Parse I/Q data and store these data to buffer */
	CapNode = Get_System_CapNode_Info(pAd);
	if (CapNode == pChipCap->ICapPackedADC) { /* 4-way ADC */
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[42m4-Way RXADC ------> \x1b[m\n");

		for (i = 0; i < (pChipCap->ICapADCIQCnt / 3); i++) {
			if (pICapDesc->ucADCRes == 4) {
				/* Parse I/Q data */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 8)) >> 8);         /* Parsing Q0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 4)) >> 4);     /* Parsing Q0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xF);                   /* Parsing Q0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 20)) >> 20);       /* Parsing I0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 16)) >> 16);   /* Parsing I0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 12)) >> 12);   /* Parsing I0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = (*(pAd->pM32Bit + i) & 0xF);                       /* Parsing Q1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 28)) >> 28);   /* Parsing Q1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 12)) >> 12);       /* Parsing I1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 8)) >> 8);     /* Parsing I1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 4)) >> 4);     /* Parsing I1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 24)) >> 24);       /* Parsing Q2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 20)) >> 20);   /* Parsing Q2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 16)) >> 16);   /* Parsing Q2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 4)) >> 4);         /* Parsing I2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_I_TYPE] = (*(pAd->pH32Bit + i) & 0xF);                   /* Parsing I2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);   /* Parsing I2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 16)) >> 16);       /* Parsing Q3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 12)) >> 12);   /* Parsing Q3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 8)) >> 8);     /* Parsing Q3 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 28)) >> 28);       /* Parsing I3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 24)) >> 24);   /* Parsing I3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 20)) >> 20);   /* Parsing I3 */

				/* Calculation of offset binary to decimal */
				for (j = 0; j < 3; j++) {
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 8;
				}
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[42m 4-Way IQC ------>  \x1b[m\n");

		for (i = 0; i < pChipCap->ICapIQCIQCnt; i++) {
			if (pICapDesc->ucIQCRes == 12) {
				/* Parse I/Q data */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xFFF);                  /* Parsing Q0 */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xFFF << 12)) >> 12);  /* Parsing I0 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xFF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] |= ((*(pAd->pM32Bit + i) & 0xF) << 8);		     /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 4)) >> 4);    /* Parsing I1 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 16)) >> 16);  /* Parsing Q2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);    /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] |= ((*(pAd->pH32Bit + i) & 0xFF) << 4);           /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 8)) >> 8);    /* Parsing Q3 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 20)) >> 20);  /* Parsing I3 */

				/* Calculation of two-complement to decimal */
				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 4096;
			}
		}
	}

	/* Print log to console to indicate data process done */
	{
		UINT32 TotalSize = 0;

		for (i = 0; i < BankNum; i++) {
			/* Update pICapDesc */
			pICapDesc = &pChipCap->pICapDesc[i];
			/* Calculate total size */
			TotalSize = TotalSize + pICapDesc->u4BankSize;
		}

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m : Dump %d K done !! \x1b[m\n", TotalSize);
	}
	/* Update ICap overall status */
	pAd->ICapStatus = CAP_SUCCESS;

error:
	if (pTemp_L32Bit != NULL)
		os_free_mem(pTemp_L32Bit);

	if (pTemp_M32Bit != NULL)
		os_free_mem(pTemp_M32Bit);

	if (pTemp_H32Bit != NULL)
		os_free_mem(pTemp_H32Bit);

	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Extend event 96-bit raw data handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICapUnSolicit96BitRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	INT32 retval, Status;
	UINT32 i, j;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = NULL;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pICapEvent */
	pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pICapEvent->u4FuncIndex = le2cpu32(pICapEvent->u4FuncIndex);
	pICapEvent->u4PktNum = le2cpu32(pICapEvent->u4PktNum);
	pICapEvent->u4Bank = le2cpu32(pICapEvent->u4Bank);

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it. */
	if (pICapEvent->u4PktNum > pAd->ICapEventCnt) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"\x1b[31mPacket out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				pICapEvent->u4PktNum, pAd->ICapEventCnt);
		return;
	}

	/* Dynamic allocate memory for L32Bit/M32Bit/H32Bit buffer */
	{
		UINT32 Len;

		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		if (pAd->pL32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pL32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"Not enough memory for dynamic allocating !!\n");
				goto error;
			}
		}

		if (pAd->pM32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pM32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"Not enough memory for dynamic allocating !!\n");
				goto error;
			}
		}

		if (pAd->pH32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pH32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
						"Not enough memory for dynamic allocating !!\n");
				goto error;
			}
		}

		/* Initialization of data buffer of L32Bit/M32Bit/H32Bit */
		if ((pAd->ICapL32Cnt == 0) && (pAd->ICapM32Cnt == 0)
			&& (pAd->ICapH32Cnt == 0)) {
			os_zero_mem(pAd->pL32Bit, Len);
			os_zero_mem(pAd->pM32Bit, Len);
			os_zero_mem(pAd->pH32Bit, Len);
		}
	}

	/* Store L32Bit, M32Bit, H32Bit data to each buffer */
	for (j = 0; j < (BankNum / 3); j++) {
		if (pICapEvent->u4Bank == pICapDesc->pLBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pL32Bit[pAd->ICapL32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapL32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pMBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pM32Bit[pAd->ICapM32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapM32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pHBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pH32Bit[pAd->ICapH32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapH32Cnt++;
			}
		}
	}

	/* Print ICap data to console for debugging purpose */
	for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[42m 0x%08x\x1b[m\n", le2cpu32(pICapEvent->u4Data[i]));
	}

	/* Update ICapEventCnt */
	pAd->ICapEventCnt++;
	/* Update pICapDesc */
	pICapDesc = &pChipCap->pICapDesc[pAd->ICapIdx];

	/* Check whether is the last FW event or not */
	if (pAd->ICapEventCnt == pICapDesc->u4BankSize) {
		/* Check whether is the last bank or not */
		if ((pAd->ICapIdx + 1) == BankNum)
			ExtEventICap96BitDataParser(pAd);

		/* Reset ICapEventCnt */
		pAd->ICapEventCnt = 0;
		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->ICapDumpDataDone);
	}

error:
	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Extend event I/Q data handler of ICAP.
	Return:
	==========================================================================
*/

VOID ExtEventICapUnSolicitIQDataHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	INT32 Status;
	UINT32 Idxi = 0, Idxj = 0, Idxk = 0, Idxz = 0, u4Cnt = 0;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = NULL;
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pICapEvent */
	pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pICapEvent->u4FuncIndex = le2cpu32(pICapEvent->u4FuncIndex);
	pICapEvent->u4PktNum = le2cpu32(pICapEvent->u4PktNum);
	pICapEvent->u4DataLen = le2cpu32(pICapEvent->u4DataLen);
	pICapEvent->u4SmplCnt = le2cpu32(pICapEvent->u4SmplCnt);
	pICapEvent->u4WFCnt = le2cpu32(pICapEvent->u4WFCnt);

	/* If we receive the packet which is out of sequence, we need to drop it */
	if (pICapEvent->u4PktNum > pAd->ICapEventCnt) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				pICapEvent->u4PktNum, pAd->ICapEventCnt);
		return;
	}

	if (pICapEvent->u4DataLen != 0) {
		u4Cnt = pAd->ICapDataCnt + pICapEvent->u4SmplCnt;
		for (Idxi = pAd->ICapDataCnt; Idxi < u4Cnt; Idxi++) {
			for (Idxj = 0; Idxj < pICapEvent->u4WFCnt; Idxj++) {
				pIQ_Array[Idxi].IQ_Array[Idxj][CAP_I_TYPE] = (INT32)le2cpu32(pICapEvent->u4Data[Idxk++]);
				pIQ_Array[Idxi].IQ_Array[Idxj][CAP_Q_TYPE] = (INT32)le2cpu32(pICapEvent->u4Data[Idxk++]);
			}
		}
		pAd->ICapDataCnt = Idxi;

		/* Print ICap data to console for debugging purpose */
		for (Idxz = 0; Idxz < pICapEvent->u4DataLen; Idxz++) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					"\x1b[42m Data[%d] : %d \x1b[m\n", Idxz, (INT32)le2cpu32(pICapEvent->u4Data[Idxz]));
		}

		/* Update ICapEventCnt */
		pAd->ICapEventCnt++;
	}

	/* Check whether is the last FW event or not */
	if ((pICapEvent->u4DataLen == 0)
		&& (pICapEvent->u4PktNum == pAd->ICapEventCnt)) {

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m: Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, pAd->ICapEventCnt);

		/* Reset ICapEventCnt */
		pAd->ICapEventCnt = 0;

		/* Reset ICapDataCnt */
		pAd->ICapDataCnt = 0;

		/* Update ICap overall status */
		pAd->ICapStatus = CAP_SUCCESS;

		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->ICapDumpDataDone);
	}

	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Extend event of querying data-captured status handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICapUnSolicitStatusHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_RBIST_ADDR_T *prICapGetEvent = (EXT_EVENT_RBIST_ADDR_T *)pData;
	/* save iCap result */
	/* send iCap result to QA tool */
	UINT32 *p;

	for (p = (UINT32 *)pData; p < ((UINT32 *)pData+8); p++)
		*p = le2cpu32(*p);
#ifdef CONFIG_ATE
	NdisMoveMemory(&(pAd->ATECtrl.icap_info), prICapGetEvent, sizeof(EXT_EVENT_RBIST_ADDR_T));
#endif /* CONFIG_ATE */
	MTWF_PRINT("%s: prICapGetEvent->u4StartAddr1 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr1);
	MTWF_PRINT("%s: prICapGetEvent->u4StartAddr2 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr2);
	MTWF_PRINT("%s: prICapGetEvent->u4StartAddr3 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr3);
	MTWF_PRINT("%s: prICapGetEvent->u4EndAddr = 0x%x\n",
			__func__, prICapGetEvent->u4EndAddr);
	MTWF_PRINT("%s: prICapGetEvent->u4StopAddr = 0x%x\n",
			__func__, prICapGetEvent->u4StopAddr);
	MTWF_PRINT("%s: prICapGetEvent->u4Wrap = 0x%x\n",
			__func__, prICapGetEvent->u4Wrap);
#ifdef CONFIG_ATE
	RTMP_OS_COMPLETE(&(pAd->ATECtrl.cmd_done));
#endif /* CONFIG_ATE */
}
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef PHY_ICS_SUPPORT
/*
	==========================================================================
	Description:
	Cmd queue raw data handler of PHY ICS.
	Return:
	==========================================================================
*/
NTSTATUS PhyIcsRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->PhyIcsEventRawDataHandler != NULL) {
		ops->PhyIcsEventRawDataHandler(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/*
	==========================================================================
	Description:
	Extend event data handler of PHY ICS.
	Return:
	==========================================================================
*/

VOID ExtEventPhyIcsUnSolicitDataHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#ifdef FW_LOG_DUMP
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
	UINT16 *serialID = &(FwLogCtrl->fw_log_serialID_count);
#endif
	UINT32 Idxz = 0;
	ULONG u4WifiSysTimes = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_EVENT_ID_PHY_ICS_DUMP_DATA_T *pIcsEvent = NULL;
#else
	EXT_EVENT_PHY_ICS_DUMP_DATA_T *pIcsEvent = NULL;
#endif

	UINT16 msg_len = 0;
	UINT8 *buffer = NULL;
	P_FW_BIN_LOG_HDR_T log_hdr;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pIcsEvent */
#ifdef WIFI_UNIFIED_COMMAND
	pIcsEvent = (struct UNI_EVENT_ID_PHY_ICS_DUMP_DATA_T *)pData;
#else
	pIcsEvent = (EXT_EVENT_PHY_ICS_DUMP_DATA_T *)pData;
#endif
	pIcsEvent->u4FuncIndex = le2cpu32(pIcsEvent->u4FuncIndex);
	pIcsEvent->u4PktNum = le2cpu32(pIcsEvent->u4PktNum);
	/* swap timestamp to alignment toucan's phy ics parsing rule(Big-endian) */
	pIcsEvent->u4WifiSysTimestamp = SWAP32(pIcsEvent->u4WifiSysTimestamp);
	u4WifiSysTimes = pIcsEvent->u4WifiSysTimestamp;
	pIcsEvent->u4DataLen = le2cpu32(pIcsEvent->u4DataLen);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
	       "PktNum = [%d], WifiSysTimestamp = [%lu]us, DataLen = [%d]\n",
	       pIcsEvent->u4PktNum,
	       u4WifiSysTimes,
	       pIcsEvent->u4DataLen);


	/* Print ICap data to console for debugging purpose */
	for (Idxz = 0; Idxz < pIcsEvent->u4DataLen; Idxz++) {
		pIcsEvent->u4Data[Idxz] = (INT32)SWAP32(pIcsEvent->u4Data[Idxz]);
	}


	if (pIcsEvent->u4DataLen == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"\x1b[31m Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, pIcsEvent->u4PktNum);
	}

	msg_len = pIcsEvent->u4DataLen * sizeof(UINT32) + sizeof(FW_BIN_LOG_HDR_T);

	if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS)
		return;
	log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

	/* prepare ICS header */
#ifdef FW_LOG_DUMP
	log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
	log_hdr->u1Version = FW_BIN_LOG_VERSION;
	log_hdr->u1Rsv = FW_BIN_LOG_RSV;
	log_hdr->u2SerialID = (*serialID)++;
	if (chip_dbg->get_lpon_frcr)
		log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
	else
		log_hdr->u4Timestamp = pIcsEvent->u4WifiSysTimestamp;
	log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_PHY_ICS;
	log_hdr->u2Length = pIcsEvent->u4DataLen * sizeof(UINT32);
#endif

	/* prepare ICS frame */
	NdisCopyMemory(buffer + sizeof(FW_BIN_LOG_HDR_T), pIcsEvent->u4Data, pIcsEvent->u4DataLen * sizeof(UINT32));

	if (msg_len) {
		if (pAd->PhyIcsFlag)
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)buffer, msg_len);

		os_free_mem(buffer);
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}
#endif /* PHY_ICS_SUPPORT */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID ExtEventThroughputBurst(RTMP_ADAPTER *pAd,
									UINT8 *Data, UINT32 Length)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	BOOLEAN fgEnable = FALSE;
	UCHAR pkt_num = 0;
	UINT32 length = 0;

	if (pObj->ioctl_if_type == INT_APCLI) {
#ifdef CONFIG_STA_SUPPORT
		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
#endif
		;
	} else {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
		;
	}

	if (!Data || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Data is NULL\n");
		return;
	}

	fgEnable = (BOOLEAN)(*Data);
	pAd->bDisableRtsProtect = fgEnable;

	if (pAd->bDisableRtsProtect) {
		pkt_num = MAX_RTS_PKT_THRESHOLD;
		length = MAX_RTS_THRESHOLD;
	} else {
		pkt_num = wlan_operate_get_rts_pkt_thld(wdev);
		length = wlan_operate_get_rts_len_thld(wdev);
	}

	HW_SET_RTS_THLD(pAd, wdev, pkt_num, length);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "%d\n", fgEnable);
}


static VOID ExtEventGBand256QamProbeResule(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T pResult;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 u2WlanIdx = WCID_INVALID;

	if (Data != NULL) {
		pResult = (P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T)(Data);
		u2WlanIdx = WCID_GET_H_L(pResult->ucWlanIdxHnVer, pResult->ucWlanIdxL);
		pEntry = entry_get(pAd, u2WlanIdx);

		if (!pEntry || IS_ENTRY_NONE(pEntry)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					 "pEntry is NONE\n");
			return;
		}

		if (pResult->ucResult == RA_G_BAND_256QAM_PROBE_SUCCESS)
			pEntry->fgGband256QAMSupport = TRUE;

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "Gband256QAMSupport = %d\n", pResult->ucResult);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				 " Data is NULL\n");
	}
}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

UINT max_line = 130;

static VOID ExtEventAssertDumpHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	struct _EXT_EVENT_ASSERT_DUMP_T *pExtEventAssertDump =
		(struct _EXT_EVENT_ASSERT_DUMP_T *)Data;

	if (max_line) {
		if (max_line == 130)
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "**************************************************\n\n");

		max_line--;
		pExtEventAssertDump->aucBuffer[Length] = 0;
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "%s\n", pExtEventAssertDump->aucBuffer);
	}

#ifdef FW_DUMP_SUPPORT

	if (!pAd->fw_dump_buffer) {
		os_alloc_mem(pAd, &pAd->fw_dump_buffer, pAd->fw_dump_max_size);
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;

		if (pAd->fw_dump_buffer) {
			if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_N9");
			else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_CR4");
			else
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "\0");
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "cannot alloc mem for FW dump\n");
		}
	}

	if (pAd->fw_dump_buffer) {
		if ((pAd->fw_dump_size + Length) <= pAd->fw_dump_max_size) {
			os_move_mem(pAd->fw_dump_buffer + pAd->fw_dump_size, Data, Length);
			pAd->fw_dump_size += Length;
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "FW dump size too big\n");
		}
	}

#endif
}

static VOID ExtEventPsSyncHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_PS_SYNC_T *pExtEventPsSync =
		(struct _EXT_EVENT_PS_SYNC_T *)Data;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	UINT16 wcid = WCID_GET_H_L(pExtEventPsSync->ucWtblIndexHnVer, pExtEventPsSync->ucWtblIndexL);
	struct qm_ctl *qm_ctl = PD_GET_QM_CTL(pAd->physical_dev);
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	NDIS_PACKET *pkt = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "PsSync Event from FW APPS WIdx=%d PSBit=%d len=%d\n",
			wcid, pExtEventPsSync->ucPsBit, Length);

	if (IS_WCID_VALID(pAd, wcid)) {
		tr_entry = tr_entry_get(pAd, wcid);
		OS_SEM_LOCK(&tr_entry->ps_sync_lock);
		tr_entry->ps_state = (pExtEventPsSync->ucPsBit == 0) ? FALSE : TRUE;

		if (tr_entry->ps_state == PWR_ACTIVE) {
			do {
				if (qm_ops->get_psq_pkt)
					pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (pkt) {
					UCHAR q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
					UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					struct wifi_dev *wdev = NULL;

					wdev = pAd->wdev_list[wdev_idx];

					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
				}
			} while (pkt);
		}
		OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);
	} else
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR, "wtbl index(%d) is invalid\n", wcid);
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
VOID ExtEventBfStatusRead(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_BF_STATUS_T *pExtEventBfStatus = (struct _EXT_EVENT_BF_STATUS_T *)Data;
	struct _EXT_EVENT_IBF_STATUS_T *pExtEventIBfStatus = (struct _EXT_EVENT_IBF_STATUS_T *)Data;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if ((Data == NULL) ||
		(pExtEventBfStatus == NULL) ||
		(pExtEventIBfStatus == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"Input data is invalid\n");
		return;
	}

	if (ops == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"ops is NULL\n");
		return;
	}

#if defined(CONFIG_ATE)

	if (ATE_ON(pAd))
#ifdef DOT11_HE_AX
		TxBf_Status_Update(pAd, Data, Length);
#else
		HQA_BF_INFO_CB(pAd, Data, Length);
#endif /* DOT11_HE_AX */
#endif /* defined(CONFIG_ATE) */

	switch (pExtEventBfStatus->ucBfDataFormatID) {
	case BF_PFMU_TAG:
		if (ops->dump_pfmu_tag)
			ops->dump_pfmu_tag(pAd,
					pExtEventBfStatus->fgBFer,
					pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_DATA:
		if (ops->dump_pfmu_data)
			ops->dump_pfmu_data(pAd,
					le2cpu16(pExtEventBfStatus->u2subCarrIdx),
					pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_PN:
		TxBfProfilePnPrint(pExtEventBfStatus->ucBw,
						   pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_MEM_ALLOC_MAP:
		TxBfProfileMemAllocMap(pExtEventBfStatus->aucBuffer);
		break;

	case BF_STAREC:
		StaRecBfRead(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_CAL_PHASE:
		if (ops->iBFPhaseCalReport)
			ops->iBFPhaseCalReport(pAd,
									pExtEventIBfStatus->ucGroup_L_M_H,
									pExtEventIBfStatus->ucGroup,
									pExtEventIBfStatus->u1DbdcBandIdx,
									pExtEventIBfStatus->ucStatus,
									pExtEventIBfStatus->ucPhaseCalType,
									pExtEventIBfStatus->aucBuffer);
		break;

	case BF_FBRPT_DBG_INFO:
		TxBfFbRptDbgInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_TXSND_INFO:
		TxBfTxSndInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_PLY_INFO:
		TxBfPlyInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_METRIC_INFO:
		HeRaMuMetricInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_TXCMD_CFG_INFO:
		TxBfTxCmdCfgInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_SND_CNT_INFO:
		TxBfSndCntInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	default:
		break;
	}
}
#endif /* MT_MAC && TXBF_SUPPORT */

VOID MecInfoAmsduEnPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT32 pu4Buf)
{
	P_MEC_EVENT_INFO_T pMecEvtInfo = (P_MEC_EVENT_INFO_T)pu4Buf;
	UINT8 u1AlgoEn;
	UINT16 u2WlanIdx;
	UINT16 wtbl_max_num = hc_get_chip_wtbl_max_num(pAd);
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	MTWF_PRINT("[+] AMSDU Algo Enable Status for all %u STAs (sync %u)\n",
		hc_get_chip_wtbl_max_num(pAd), WTBL_MAX_NUM(pAd));

	for (u2WlanIdx = 0; u2WlanIdx < (wtbl_max_num / 32); u2WlanIdx++) {
		MTWF_PRINT("     au4MecAlgoEnSta[%u]: 0x%08X\n", u2WlanIdx, pMecEvtInfo->rMecCtrl.au4MecAlgoEnSta[u2WlanIdx]);
	}

	MTWF_PRINT("[+] Connected STA AMSDU Algo Enable Status\n");

	for (u2WlanIdx = 1; VALID_UCAST_ENTRY_WCID(pAd, u2WlanIdx); u2WlanIdx++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, u2WlanIdx);

		if (!pEntry || pEntry->EntryType == ENTRY_NONE)
			continue;

#ifdef SW_CONNECT_SUPPORT
		tr_entry = tr_entry_get(pAd, u2WlanIdx);
		if (IS_SW_STA(tr_entry))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		u1AlgoEn = ((pMecEvtInfo->rMecCtrl.au4MecAlgoEnSta[u2WlanIdx>>0x5] & BIT(u2WlanIdx & 0x1F)) >> (u2WlanIdx & 0x1F));
		MTWF_PRINT("     WlanIdx %2u, AID %2u, AMSDU Algo Enable: %u\n",
			u2WlanIdx, pEntry->Aid, u1AlgoEn);
	}
}

VOID MecInfoAmsduThrPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT32 pu4Buf)
{
	P_MEC_EVENT_INFO_T pMecEvtInfo = (P_MEC_EVENT_INFO_T)pu4Buf;
	UINT16 u2WlanIdx;

	MTWF_PRINT("[+] PHY Rate Threshold for AMSDU Length Setting\n");

	for (u2WlanIdx = MEC_CTRL_BA_NUM_64; u2WlanIdx < MEC_CTRL_BA_NUM_MAX; u2WlanIdx++) {
		MTWF_PRINT("    BA %2u\n", ((u2WlanIdx == MEC_CTRL_BA_NUM_64)?64:256));
		MTWF_PRINT("     Num   Len    Threshold\n");
		MTWF_PRINT("      1    1.7K      0 Mbps\n");
		MTWF_PRINT("      2    3.3K   %4u Mbps\n",
			pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_1_DOT_7KB]);
		MTWF_PRINT("      3    4.8K   %4u Mbps\n",
			pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_3_DOT_3KB]);
		MTWF_PRINT("      4    6.3K   %4u Mbps\n",
			pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_4_DOT_8KB]);
	}
}

VOID ExtEventMecInfoRead(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MEC_INFO_T pExtEventMecInfo = (P_EXT_EVENT_MEC_INFO_T)Data;
	UINT16 wlanid;
	PMAC_TABLE_ENTRY pEntry = NULL;
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("MEC Ctrl Info:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"u2ReadType %2u\n", pExtEventMecInfo->u2ReadType);

	if (pExtEventMecInfo->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		pExtEventMecInfo->u2ReadType & MEC_EVENT_READ_TYPE_ALGO_EN)
		MecInfoAmsduEnPrint(pAd, pExtEventMecInfo->au4Buf);
	if (pExtEventMecInfo->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		pExtEventMecInfo->u2ReadType & MEC_EVENT_READ_TYPE_AMSDU_THR)
		MecInfoAmsduThrPrint(pAd, pExtEventMecInfo->au4Buf);
	if (pExtEventMecInfo->u2ReadType & MEC_EVENT_READ_TYPE_AMSDU_ERROR_NOTIFY) {
		//ERROR NOTIFY wlanid of sta is pExtEventMecInfo->u2wlanID
		wlanid = pExtEventMecInfo->u2wlanID;
		if (VALID_UCAST_ENTRY_WCID(pAd, wlanid)) {
			pEntry = entry_get(pAd, wlanid);

#ifdef SW_CONNECT_SUPPORT
			tr_entry = tr_entry_get(pAd, wlanid);
			if (IS_SW_STA(tr_entry))
				return;
#endif /* SW_CONNECT_SUPPORT */

			if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
				if (!pEntry->agg_err_flag) {
					pEntry->agg_err_flag = TRUE;
					MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					"STA(%d) Hit AMSDU_ERROR_NOTIFY!\n", wlanid);
				}
			}
		}
	}
}

static VOID ExtEventFwLog2HostHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	UCHAR *dev_name = NULL;
	UCHAR empty_name[] = " ";
#ifdef FW_LOG_DUMP
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
	UINT16 *serialID = &(FwLogCtrl->fw_log_serialID_count);
#endif

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "s2d_index = 0x%x\n",
			  event_rxd->fw_rxd_2.field.s2d_index);
	dev_name = RtmpOsGetNetDevName(pAd->net_dev);

	if ((dev_name == NULL) || strlen(dev_name) >= NET_DEV_NAME_MAX_LENGTH)
		dev_name = &empty_name[0];

	if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST) {
#ifdef FW_LOG_DUMP
		P_FW_BIN_LOG_HDR_T log_hdr = (P_FW_BIN_LOG_HDR_T)Data;
		if (log_hdr->u4MagicNum == FW_BIN_LOG_MAGIC_NUM) {
			log_hdr->u2SerialID = (*serialID)++;
			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
				RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)Data, Length);
			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
				fw_log_to_ethernet(pAd, Data, Length);
		} else
#endif /* FW_LOG_DUMP */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
		if (pAd->KtoFlashDebug)
			MTWF_PRINT("(%s): %s", dev_name, Data);
		else
#endif /*PRE_CAL_TRX_SET1_SUPPORT*/
			MTWF_PRINT("N9 LOG(%s): %s\n", dev_name, Data);
	} else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST) {
		MTWF_PRINT("CR4 LOG(%s): %s\n", dev_name, Data);
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "unknow MCU LOG(%s): %s", dev_name, Data);
	}
}

#ifdef COEX_SUPPORT
static VOID ExtEventBTCoexHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{

	UINT8 SubOpCode;
	MAC_TABLE_ENTRY *pEntry;
	struct _EVENT_EXT_COEXISTENCE_T *coext_event_t =
		(struct _EVENT_EXT_COEXISTENCE_T *)Data;
	SubOpCode = coext_event_t->ucSubOpCode;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "SubOpCode: 0x%x\n", coext_event_t->ucSubOpCode);
	hex_dump("Coex Event payload ", coext_event_t->aucBuffer, Length);

	if (SubOpCode == 0x01) {
		struct _EVENT_COEX_CMD_RESPONSE_T *CoexResp =
			(struct _EVENT_COEX_CMD_RESPONSE_T *)coext_event_t->aucBuffer;
		CoexResp->u4Status = le2cpu32(CoexResp->u4Status);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->Cmd_Resp=0x%x\n", CoexResp->u4Status);
	} else if (SubOpCode == 0x02) {
		struct _EVENT_COEX_REPORT_COEX_MODE_T *CoexReportMode =
			(struct _EVENT_COEX_REPORT_COEX_MODE_T *)coext_event_t->aucBuffer;
		CoexReportMode->u4SupportCoexMode = le2cpu32(CoexReportMode->u4SupportCoexMode);
		CoexReportMode->u4CurrentCoexMode = le2cpu32(CoexReportMode->u4CurrentCoexMode);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->SupportCoexMode=0x%x\n", CoexReportMode->u4SupportCoexMode);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->CurrentCoexMode=0x%x\n", CoexReportMode->u4CurrentCoexMode);
		pAd->BtCoexSupportMode = ((CoexReportMode->u4SupportCoexMode) & 0x3);
		pAd->BtCoexMode = ((CoexReportMode->u4CurrentCoexMode) & 0x3);
	} else if (SubOpCode == 0x03) {
		struct _EVENT_COEX_MASK_OFF_TX_RATE_T *CoexMaskTxRate =
			(struct _EVENT_COEX_MASK_OFF_TX_RATE_T *)coext_event_t->aucBuffer;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->MASK_OFF_TX_RATE=0x%x\n", CoexMaskTxRate->ucOn);
	} else if (SubOpCode == 0x04) {
		struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *CoexChgBaSize =
			(struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *)coext_event_t->aucBuffer;

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->Change_BA_Size ucOn=%d\n", CoexChgBaSize->ucOn);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->Change_BA_Size=0x%x\n", CoexChgBaSize->ucRXBASize);

		pEntry = entry_get(pAd, BSSID_WCID);
		if (pEntry && (CoexChgBaSize->ucOn == 1)) {
			struct BA_REC_ENTRY *pBAEntry = NULL;
			UCHAR Idx;
			UINT16 ba_tx_wsize = 0, ba_rx_wsize = 0;
			UINT16 max_ba_wsize = ba_get_default_max_ba_wsize(pEntry->wdev, pAd);
			Idx = pEntry->ba_info.RecWcidArray[0];

#ifdef DOT11_EHT_BE
			if (IS_ENTRY_MLO(pEntry)) {
				struct mld_entry_t *mld_entry;

				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pEntry->mld_entry);
				if (!mld_entry) {
					mt_rcu_read_unlock();
					return;
				}
				Idx = mld_entry->ba_info.RecWcidArray[0];
				mt_rcu_read_unlock();
			}
#endif /* DOT11_EHT_BE */

			pBAEntry = &pAd->BATable.BARecEntry[Idx];
			pAd->BtCoexBASize = CoexChgBaSize->ucRXBASize;
			ba_tx_wsize = wlan_config_get_ba_tx_wsize(pEntry->wdev);
			ba_rx_wsize = wlan_config_get_ba_rx_wsize(pEntry->wdev);

			if (pBAEntry->BAWinSize == 0)
				pBAEntry->BAWinSize = ba_rx_wsize;

			if (pAd->BtCoexBASize != 0) {
				if (pAd->BtCoexBASize < ba_rx_wsize)
					ba_rx_wsize = pAd->BtCoexBASize;

				if (pAd->BtCoexBASize < ba_tx_wsize)
					ba_tx_wsize = pAd->BtCoexBASize;

				wlan_config_set_ba_txrx_wsize(pEntry->wdev, ba_tx_wsize, ba_rx_wsize, max_ba_wsize);
				ba_ori_session_tear_down(pAd, BSSID_WCID, 0, FALSE);
				ba_rec_session_tear_down(pAd, BSSID_WCID, 0, FALSE);
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						 "COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexBASize);
			}
		}
	} else if (SubOpCode == 0x05) {
		struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *CoexLimitBeacon =
			(struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->COEX_LIMIT_BEACON_SIZE ucOn =%d\n", CoexLimitBeacon->ucOn);
		pAd->BtCoexBeaconLimit = CoexLimitBeacon->ucOn;
	} else if (SubOpCode == 0x06) {
		struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *CoexExtendBTORoam =
			(struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *)coext_event_t->aucBuffer;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "--->EVENT_COEX_EXTEND_BNCTO_ROAMING ucOn =%d\n",
				  CoexExtendBTORoam->ucOn);
	}
}
#endif /* COEX_SUPPORT */

#ifdef DOT11V_MBSSID_SUPPORT
BOOLEAN MtUpdateBcnToMcuV2(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void, BOOLEAN BcnSntReq, UCHAR UpdateReason)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	struct _BSS_INFO_ARGUMENT_T bss;

	memcpy(&bss, &wdev->bss_info_argument, sizeof(bss));
	bss.u8BssInfoFeature = BSS_INFO_OFFLOAD_PKT_FEATURE;
	bss.bBcnSntReq = BcnSntReq;
	bss.bUpdateReason = UpdateReason;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			"wdev(%d) bssIdx %d, OM 0x%x, Band %d\n",
			wdev->wdev_idx, bss.ucBssIndex, bss.OwnMacIdx, bss.ucBandIdx);

	AsicBssInfoUpdate(pAd, &bss);

	return TRUE;
}
#else
BOOLEAN MtUpdateBcnToMcu(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void, BOOLEAN BcnSntReq, UCHAR UpdateReason)
{
	BCN_BUF_STRUCT *bcn_buf = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_HE_AX
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;
#endif /* DOT11_HE_AX */
#endif
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	CMD_BCN_OFFLOAD_T_V2 *bcn_offload_v2 = NULL;
#endif
	CMD_BCN_OFFLOAD_T bcn_offload;
	BOOLEAN bSntReq = FALSE;
	UINT16 TimIELocation = 0, CsaIELocation = 0;
#ifdef DOT11_HE_AX
	UINT16 BccIELocation = 0;
#endif
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR UpdatePktType = PKT_V1_BCN;

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN) {
			os_alloc_mem(NULL, (PUCHAR *)&bcn_offload_v2, sizeof(*bcn_offload_v2));
			if (!bcn_offload_v2) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
						 "can not allocate bcn_offload\n");
				return FALSE;
			}
			os_zero_mem(bcn_offload_v2, sizeof(*bcn_offload_v2));
	} else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#endif

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "wdev is NULL!\n");
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
			os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	bcn_buf = &wdev->bcn_buf;

	if (!bcn_buf) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "bcn_buf is NULL!\n");
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	if ((UpdatePktType == PKT_V1_BCN)
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
		|| (UpdatePktType == PKT_V2_BCN)
#endif
	){
		pkt = bcn_buf->BeaconPkt;
		bSntReq = BcnSntReq;
		TimIELocation = bcn_buf->tim_ie_offset;
		CsaIELocation = bcn_buf->CsaIELocationInBeacon;
#ifdef DOT11_HE_AX
		BccIELocation = bcn_buf->bcc_ie_location;
#endif
	}

	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = bcn_buf->FrameLen + tx_hw_hdr_len;/* TXD & pkt content. */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN) {
		bcn_offload_v2->ucEnable = bSntReq;
		bcn_offload_v2->ucWlanIdx = 0;/* hardcode at present */
		bcn_offload_v2->ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload_v2->ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload_v2->u2PktLength = len;
		bcn_offload_v2->ucPktType = UpdatePktType;
		bcn_offload_v2->fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload_v2->u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload_v2->u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload_v2->ucCsaCount = wdev->csa_count;
#endif
		NdisCopyMemory(bcn_offload_v2->acPktContent, buf, len);
		MtCmdBcnV2OffloadSet(pAd, bcn_offload_v2);
} else {
		bcn_offload.ucEnable = bSntReq;
		bcn_offload.ucWlanIdx = 0;/* hardcode at present */
		bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload.u2PktLength = len;
		bcn_offload.ucPktType = UpdatePktType;
		bcn_offload.fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload.u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload.u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload.ucCsaCount = wdev->csa_count;
#endif
		NdisCopyMemory(bcn_offload.acPktContent, buf, len);
		MtCmdBcnOffloadSet(pAd, &bcn_offload);
}
#else
	{
		bcn_offload.ucEnable = bSntReq;
		bcn_offload.ucWlanIdx = 0;/* hardcode at present */
		bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload.u2PktLength = len;
		bcn_offload.ucPktType = UpdatePktType;
		bcn_offload.fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload.u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload.u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload.ucCsaCount = wdev->csa_count;
#ifdef DOT11_HE_AX
		if (BccIELocation)
			bcn_offload.ucBccCount = bss_color->u.ap_ctrl.bcc_count;
		bcn_offload.u2BccIePos = BccIELocation + tx_hw_hdr_len;
#endif /* DOT11_HE_AX */
#endif
		NdisCopyMemory(bcn_offload.acPktContent, buf, len);
		MtCmdBcnOffloadSet(pAd, &bcn_offload);
}
#endif /* BCN_V2_SUPPORT */
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				"BeaconPkt is NULL!\n");
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	return TRUE;
}
#endif /* DOT11V_MBSSID_SUPPORT */

static VOID ExtEventPretbttIntHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))      ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))   ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	RTMP_HANDLE_PRETBTT_INT_EVENT(pAd);
}

VOID EventThermalProtDutyNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_THERMAL_PROTECT_DUTY_NOTIFY *event_buf = NULL;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_DUTY_NOTIFY *)Data;

	if (!event_buf)
		return;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE,
		"(Thermal Protect) Duty Notify.\n");

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE,
		"band_idx: %d, level_idx: %d, duty_percent: %d\n",
		 event_buf->band_idx,
		 event_buf->level_idx,
		 event_buf->duty_percent);
	if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_TRIG && event_buf->temp)
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE,
			"Trigger Temp = %d\n", event_buf->temp);
	else if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_RESTORE)
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE,
			"Restore Temp = %d\n", event_buf->temp);
}

VOID EventThermalRadioNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_THERMAL_PROTECT_RADIO_NOTIFY *event_buf = NULL;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_RADIO_NOTIFY *)Data;

	if (!event_buf)
		return;

	MTWF_PRINT("(Thermal Protect) Radio Notify.\n");

	MTWF_PRINT("band_idx: %d, level_idx: %d\n",
		 event_buf->band_idx,
		 event_buf->level_idx);
	if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_TRIG && event_buf->temp)
		MTWF_PRINT("Trigger Temp = %d\n", event_buf->temp);
	else if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_RESTORE)
		MTWF_PRINT("Restore Temp = %d\n", event_buf->temp);
}

VOID EventThermalProtInfo(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_THERMAL_PROTECT_MECH_INFO *event_buf = NULL;
	UINT8 prot_type = 0;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_MECH_INFO *) Data;
	if (!event_buf)
		return;

	MTWF_PRINT("band_idx: %d\n", event_buf->band_idx);

	for (prot_type = THERMAL_PROTECT_TYPE_NTX_CTRL;
		prot_type < THERMAL_PROTECT_TYPE_NUM; prot_type++) {
		MTWF_PRINT("prot_type: %d, trig_type: %d\n",
			 event_buf->protect_type[prot_type],
			 event_buf->trigger_type[prot_type]);

		MTWF_PRINT("state: %d, enable: %d\n",
			 event_buf->state[prot_type],
			 event_buf->enable[prot_type]);

		MTWF_PRINT("trigger_temp: %d, restore_temp: %d\n",
			 event_buf->trigger_temp[prot_type],
			 event_buf->restore_temp[prot_type]);

		MTWF_PRINT("recheck_time: %d\n",
			 event_buf->recheck_time[prot_type]);

		MTWF_PRINT("--------------------------\n");
	}
}
VOID EventThermalProtDutyInfo(
	RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_THERMAL_PROTECT_DUTY_INFO *event_buf = NULL;

	MTWF_PRINT("%s\n", __func__);

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_DUTY_INFO *) Data;
	if (!event_buf)
		return;

	MTWF_PRINT("band_idx: %d\n", event_buf->band_idx);
	MTWF_PRINT("duty0: %d, duty1: %d, duty2: %d, duty3: %d\n",
		 event_buf->duty0, event_buf->duty1,
		 event_buf->duty2, event_buf->duty3);
}

VOID EventThermalProtectHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case THERMAL_PROTECT_EVENT_REASON_NOTIFY:
		EventThermalProtectReasonNotify(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_PROT_SHOW_INFO:
		EventThermalProtectInfo(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_DUTY_NOTIFY:
		EventThermalProtDutyNotify(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_RADIO_NOTIFY:
		EventThermalRadioNotify(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_MECH_INFO:
		EventThermalProtInfo(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_DUTY_INFO:
		EventThermalProtDutyInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventThermalProtectReasonNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
	UINT8 Reason;

	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;
	HLType = EvtThermalProtect->ucHLType;
	Reason = EvtThermalProtect->ucReason;
	MTWF_PRINT("%s: HLType: %d, CurrentTemp: %d, Reason: %d\n",
			  __func__, HLType, EvtThermalProtect->cCurrentTemp, Reason);

	if (Reason == THERAML_PROTECTION_REASON_RADIO) {
		RTMP_SET_THERMAL_RADIO_OFF(pAd);
		MTWF_PRINT("Radio Off due to too high temperature.\n");
	}
}

VOID EventThermalProtectInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{

	P_EXT_EVENT_THERMAL_PROT_ITEM_INFO_T prEventThermalProtectInfo;
	UINT16  u2AvrgPeriod[WH_TX_AVG_ADMIN_PERIOD_NUM] = {64, 1000};
	UINT8 u1ThermalProtItemIdx;

	/* Get pointer of Event Info Structure */
	prEventThermalProtectInfo = (P_EXT_EVENT_THERMAL_PROT_ITEM_INFO_T)Data;

	/* copy ThermalProt Item Info to Event Info */
	MTWF_PRINT("\n==================================================================================\n");
	MTWF_PRINT("                 Thermal Protect Information\n");
	MTWF_PRINT("                 Admit Duty Period = %d (us)\n", prEventThermalProtectInfo->u1AdmitPeriod * u2AvrgPeriod[prEventThermalProtectInfo->u1AvrgPeriod]);
	MTWF_PRINT("==================================================================================\n");
	for (u1ThermalProtItemIdx = 0; u1ThermalProtItemIdx < TX_DUTY_LEVEL_NUM; u1ThermalProtItemIdx++) {
		MTWF_PRINT("DutyLevel %3d       THERMAL PROTECT ADMIT TIME = %5d (us)    \n",
		u1ThermalProtItemIdx, prEventThermalProtectInfo->u2AdmitDutyLevel[u1ThermalProtItemIdx] * u2AvrgPeriod[prEventThermalProtectInfo->u1AvrgPeriod]/2);
	}
}

#define MAX_MSDU_SIZE 1544
static VOID ExtEventMaxAmsduLengthUpdate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T len_update =
		(P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T)Data;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UINT16 wcid = len_update->u2WlanIdx;
	UINT16 amsdu_len = len_update->u2AmsduLen;

#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	/* this is a temporary workaround to fix no amsdu at HT20 high rate */
	if (amsdu_len == 0)
		amsdu_len = MAX_MSDU_SIZE;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			 "ExtEventMaxAmsduLengthUpdate: wlan_idx = %d,\
			  amsdu_len = %d\n",
			  wcid, amsdu_len);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

#ifdef SW_CONNECT_SUPPORT
	tr_entry = tr_entry_get(pAd, wcid);
	if (IS_SW_STA(tr_entry))
		return;
#endif /* SW_CONNECT_SUPPORT */

	mac_entry = entry_get(pAd, wcid);

	if (mac_entry && (mac_entry->amsdu_limit_len != 0)) {
		mac_entry->amsdu_limit_len_adjust = (mac_entry->amsdu_limit_len < amsdu_len
						? mac_entry->amsdu_limit_len : amsdu_len);
	}
}

static VOID ExtEventBaTriggerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_CMD_BA_TRIGGER_EVENT_T prEventExtBaTrigger =
		(P_CMD_BA_TRIGGER_EVENT_T)Data;
	UINT16 u2WlanIdx = WCID_GET_H_L(prEventExtBaTrigger->ucWlanIdxHnVer, prEventExtBaTrigger->ucWlanIdxL);
	STA_TR_ENTRY *tr_entry;
	struct _MAC_TABLE_ENTRY *entry;
	struct _RTMP_ADAPTER *mac_ad;

	if (u2WlanIdx >= hc_get_chip_wtbl_max_num(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			 "RX P_CMD_BA_TRIGGER_EVENT_T: Invalid Wcid=%d\n",
			  u2WlanIdx);
		return;
	}
	entry = entry_get(pAd, u2WlanIdx);

	if (!entry || !IS_VALID_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			 "RX P_CMD_BA_TRIGGER_EVENT_T: Invalid entry\n");
		return;
	}
	mac_ad = (struct _RTMP_ADAPTER *)entry->pAd;
	tr_entry = tr_entry_get(pAd, u2WlanIdx);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			 "RX P_CMD_BA_TRIGGER_EVENT_T: Wcid=%d, Tid=%d\n",
			  u2WlanIdx, prEventExtBaTrigger->ucTid);
	ba_ori_session_start(mac_ad, tr_entry->wcid, prEventExtBaTrigger->ucTid);
}

static VOID ExtEventTmrCalcuInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TMR_CALCU_INFO_T ptmr_calcu_info;
	TMR_FRM_STRUC *p_tmr_frm;

	ptmr_calcu_info = (P_EXT_EVENT_TMR_CALCU_INFO_T)Data;
	p_tmr_frm = (TMR_FRM_STRUC *)ptmr_calcu_info->aucTmrFrm;
#ifdef CFG_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)p_tmr_frm, sizeof(TMR_FRM_STRUC));
#endif
	/*Tmr pkt comes to FW event, fw already take cares of the whole calculation.*/
	TmrReportParser(pAd, p_tmr_frm, TRUE, le2cpu32(ptmr_calcu_info->u4TOAECalibrationResult));
}

static VOID ExtEventCswNotifyHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_CSA_NOTIFY_T csa_notify_event = (P_EXT_EVENT_CSA_NOTIFY_T)Data;
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	UCHAR Index;
	struct wifi_dev *wdevEach = NULL;

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "bandIdx:%d, macIdx:%d\n",
		csa_notify_event->ucBandIdx, csa_notify_event->ucOwnMacIdx);
	wdev = wdev_search_by_band_omac_idx(pAd,
				csa_notify_event->ucBandIdx,
				csa_notify_event->ucOwnMacIdx);

	if (!wdev)
		return;
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	wdev->csa_count = csa_notify_event->ucChannelSwitchCount;
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE,
			"wdev:%s, wdev->csa_count:%d\n",  wdev->if_dev->name, csa_notify_event->ucChannelSwitchCount);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdevEach = pAd->wdev_list[Index];
		if (wdevEach == NULL  || !WDEV_WITH_BCN_ABILITY(wdevEach))
			continue;
		if (wdevEach->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdevEach) == HcGetBandByWdev(wdev))
			wdevEach->csa_count = wdev->csa_count;
	}

	if (pAd->CommonCfg.bIEEE80211H
		&& (pDot11h->ChannelMode == CHAN_SWITCHING_MODE)) {
#ifdef CONFIG_AP_SUPPORT
		pDot11h->CSCount = pDot11h->CSPeriod;
		ChannelSwitchingCountDownProc(pAd, wdev);
#endif /*CONFIG_AP_SUPPORT*/
	}
}

#ifdef DOT11_HE_AX
static VOID ExtEventBccNotifyHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_BCC_NOTIFY_T bcca_notify_event = (P_EXT_EVENT_BCC_NOTIFY_T)Data;
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, bcca_notify_event->ucBandIdx, bcca_notify_event->ucOwnMacIdx);

	if (!wdev)
		return;

	bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_HEOP, (UINT32)BCN_BPCC_HEOP_BSS_COLOR);
}

static VOID ExtEventMuruHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
#if defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT)

	UINT32 u4EventId = (*(UINT32 *)Data);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			 "u4EventId = %u, len = %u\n", u4EventId, Length);
#ifdef CFG_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

    switch (u4EventId) {
    case MURU_EVENT_TUNE_AP_MUEDCA:

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "MURU_EVENT_TUNE_AP_MUEDCA\n");
		muru_tune_ap_muedca_handler(pAd, Data, Length);

		break;
    case MURU_EVENT_GET_MURU_STATS_MODE_A:

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
				 "MURU_EVENT_GET_MURU_STATS_MODE_A\n");
		muru_statistic_handler(pAd, Data, Length);

		break;
    case MURU_EVENT_GET_MUMIMO_STATS_MODE_B:

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
		 "MURU_EVENT_GET_MUMIMO_STATS_MODE_B\n");
	muru_mimo_stat_handler(pAd, Data, Length);
	break;
    case MURU_EVENT_GET_DBG_STATS_MODE_C:

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
		 "MURU_EVENT_GET_DBG_STATS_MODE_C\n");
	muru_dbg_stat_handler(pAd, Data, Length);
	break;

    default:
		break;
	}
#endif /* defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT) */
}

#endif

static VOID ExtEventBssAcQPktNumHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_EVENT_BSS_ACQ_PKT_NUM_T prEventBssAcQPktNum =
		(P_EVENT_BSS_ACQ_PKT_NUM_T)Data;
	UINT8 i = 0;
	UINT32 sum[CR4_NUM_OF_WMM_AC] = {0};
	P_EVENT_PER_BSS_ACQ_PKT_NUM_T prPerBssInfo = NULL;
	UINT_16 thr;

#ifdef CFG_BIG_ENDIAN
	prEventBssAcQPktNum->u4BssMap = le2cpu32(prEventBssAcQPktNum->u4BssMap);
#endif
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			 "RX ExtEventBssAcQPktNumHandler: u4BssMap=0x%08X\n",
			  prEventBssAcQPktNum->u4BssMap);

	for (i = 0; (i < CR4_CFG_BSS_NUM) && (prEventBssAcQPktNum->u4BssMap & (1 << i)) ; i++) {
		prPerBssInfo = &prEventBssAcQPktNum->bssPktInfo[i];
#ifdef CFG_BIG_ENDIAN
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BK] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BK]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BE] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BE]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VI] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VI]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VO] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VO]);
#endif
		sum[WMM_AC_BE] += prPerBssInfo->au4AcqPktCnt[WMM_AC_BE];
		sum[WMM_AC_BK] += prPerBssInfo->au4AcqPktCnt[WMM_AC_BK];
		sum[WMM_AC_VI] += prPerBssInfo->au4AcqPktCnt[WMM_AC_VI];
		sum[WMM_AC_VO] += prPerBssInfo->au4AcqPktCnt[WMM_AC_VO];
	}

	if (pAd->CommonCfg.wifi_cert)
		thr = ONE_SECOND_NON_BE_PACKETS_CERT_THRESHOLD;
	else
		thr = ONE_SECOND_NON_BE_PACKETS_THRESHOLD;

	for (i = 0; i < CR4_NUM_OF_WMM_AC; i++) {
		if (sum[i] > thr)
			pAd->tx_one_second_ac_counter++;
	}

	mt_dynamic_wmm_be_tx_op(pAd, ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
}

#ifdef CONFIG_HOTSPOT_R2
static VOID ExtEventReprocessPktHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_CMD_PKT_REPROCESS_EVENT_T prReprocessPktEvt =
		(P_CMD_PKT_REPROCESS_EVENT_T)Data;
	PNDIS_PACKET pPacket = NULL;
	UINT8 Type = 0;
	UINT16 reprocessToken = 0;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct token_tx_pkt_queue *que = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cb == NULL)
		return;

	prReprocessPktEvt->u2MsduToken = le2cpu16(prReprocessPktEvt->u2MsduToken);
	reprocessToken = prReprocessPktEvt->u2MsduToken;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_WARN,
			 "- Reprocess Token ID = %d\n", reprocessToken);

	que = token_tx_get_queue_by_token_id(cb, reprocessToken);
	pPacket = token_tx_deq(pAd, que, reprocessToken, &Type);

	if (pPacket == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_ERROR,
			"Unexpected Reprocessing TOKEN ID %d, pkt_buf NULL!!!!!!!\n",
				 reprocessToken);
		return;
	}

	if (que->band_idx == 0)
		que->pkt_token[reprocessToken].Reprocessed = TRUE;
	else
		que->pkt_token[reprocessToken - cap->tkn_info.band0_token_cnt].Reprocessed = TRUE;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
		"- Band %d Token ID = %d reprocess set\n", que->band_idx, reprocessToken);

	if (hotspot_check_dhcp_arp(pAd, pPacket) == TRUE) {
		USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
		MAC_TABLE_ENTRY *pMacEntry = entry_get(pAd, Wcid);
		struct wifi_dev *wdev = NULL;

		if (pMacEntry)
			wdev = pMacEntry->wdev;

		if (!pMacEntry || !wdev) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_ERROR,
				"Abnormal: pMacEntry = %p, wdev = %p!!\n", pMacEntry, wdev);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			return;
		}
		RTMP_SET_PACKET_DIRECT_TX(pPacket, 1);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
			"- Driver Direct TX\n");
		send_data_pkt(pAd, wdev, pPacket);
	} else {
		if (Type == TOKEN_TX_DATA)
			RELEASE_NDIS_PACKET_IRQ(pAd, pPacket, NDIS_STATUS_SUCCESS);
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_ERROR,
				"Unexpected Reprocessing Mgmt!!\n");
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
	}
}

static VOID ExtEventGetHotspotCapabilityHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T prGetCapaEvt =
		(P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T)Data;
	UINT8 i = 0;

	for (i = 0; i < 2; i++) {
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[i].HotSpotCtrl;
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[i].WNMCtrl;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO, "======== BSSID %d  CR4 ======\n", i);
		hotspot_bssflag_dump(prGetCapaEvt->ucHotspotBssFlags[i]);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO, "======== BSSID %d  DRIVER ======\n", i);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
				 "pHSCtrl->HotSpotEnable = %d\n"
				  "pHSCtrl->ProxyARPEnable = %d\n"
				  "pHSCtrl->ASANEnable = %d\n"
				  "pHSCtrl->DGAFDisable = %d\n"
				  "pHSCtrl->QosMapEnable = %d\n"
				  , pHSCtrl->HotSpotEnable
				  , pWNMCtrl->ProxyARPEnable
				  , pHSCtrl->bASANEnable
				  , pHSCtrl->DGAFDisable
				  , pHSCtrl->QosMapEnable
				 );
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO, "======== BSSID %d END======\n", i);
	}
}
#endif /* CONFIG_HOTSPOT_R2 */

static VOID ext_event_get_cr4_multi_tx_statistics(RTMP_ADAPTER *pAd, UINT8 *data, UINT32 len)
{
	struct _EXT_EVENT_GET_CR4_MULTI_TX_STATISTICS_T *tx_statistics =
		(struct _EXT_EVENT_GET_CR4_MULTI_TX_STATISTICS_T *)data;
	UINT16 i = 0;

	for (i = 0 ; i < tx_statistics->number; i++) {
		P_EXT_EVENT_GET_CR4_TX_STATISTICS_T sta = &tx_statistics->stat_list[i];
		MAC_TABLE_ENTRY *entry = entry_get(pAd, sta->wlan_index);

		if (!entry) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_DEBUG,
				 "\x1b[42m Abnormal: entry is null !! \x1b[m\n");
			continue;
		}
#if defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_SNOOPING_OFFLOAD)
		entry->one_sec_M2U_TxBytes = le2cpu32(sta->one_sec_tx_bytes);
		entry->one_sec_M2U_TxPackets = le2cpu32(sta->one_sec_tx_cnts);
#else
		entry->OneSecTxBytes = le2cpu32(sta->one_sec_tx_bytes);
		entry->one_sec_tx_pkts = le2cpu32(sta->one_sec_tx_cnts);
		entry->AvgTxBytes = (entry->AvgTxBytes == 0) ?
					entry->OneSecTxBytes :
					((entry->AvgTxBytes + entry->OneSecTxBytes) >> 1);
		entry->avg_tx_pkts = (entry->avg_tx_pkts == 0) ?
				     entry->one_sec_tx_pkts :
				     ((entry->avg_tx_pkts + entry->one_sec_tx_pkts) >> 1);
#endif
	}
}

#ifdef IGMP_TVM_SUPPORT
static VOID ext_event_get_igmp_multicast_table(RTMP_ADAPTER *pAd, UINT8 *data, UINT32 len)
{
	P_EXT_EVENT_ID_IGMP_MULTICAST_SET_GET_T IgmpMulticastGet =
		(P_EXT_EVENT_ID_IGMP_MULTICAST_SET_GET_T)data;

	if (len < sizeof(EXT_EVENT_ID_IGMP_MULTICAST_SET_GET)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
			"Cmd Event length %u is less than required size %lu\n",
			len, sizeof(EXT_EVENT_ID_IGMP_MULTICAST_SET_GET));
		return;
	}

	switch (IgmpMulticastGet->ucRspType) {
	case MCAST_RSP_ENTRY_TABLE:
		IgmpSnoopingGetMulticastTable(pAd, IgmpMulticastGet->ucOwnMacIdx, &IgmpMulticastGet->RspData.McastTable);
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Cmd Event ID %u is not supported\n",
					IgmpMulticastGet->ucRspType);
	}
}
#endif /* IGMP_TVM_SUPPORT */

static VOID ExtEventEDCCA(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_EDCCA_T EDCCAresult = (P_EXT_EVENT_EDCCA_T)Data;

	switch (EDCCAresult->u1CmdIdx) {
	case GET_EDCCA_CTRL_EN:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "BAND%d: %d\n",
			EDCCAresult->u1BandIdx, EDCCAresult->i1CrVal[0]);
		break;
	case GET_EDCCA_CTRL_THRES:
		{
			UINT8 i = 0;

			for (i = 0 ; i < 3 ; i++) {
				if (!EDCCAresult->fginit)
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"BAND%d:BW%d0 %d\n",
						EDCCAresult->u1BandIdx, 1<<(i+1),  EDCCAresult->i1CrVal[i]);
				else {
					if (pAd->CommonCfg.u1EDCCAThreshold[i] == 0x7f)
						pAd->CommonCfg.u1EDCCAThreshold[i] = (UINT8) EDCCAresult->i1CrVal[i];
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
						"recording BAND%d:BW%d0 %d\n", EDCCAresult->u1BandIdx,
						1<<(i+1), EDCCAresult->i1CrVal[i]);
				}
			}
			break;
		}
	}
}

static VOID ExtEventNFAvgPwr(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT32 avgpwr[4];
	INT8 dBm[IPI_ANT_NUM];
	UINT8 i, msb, u1Shift = 0x1;
	UINT32 u4Mask = 0x000000FE;

	UINT8 u1Ipi_Idx = 0;
	UINT32 total = 0;
	INT_32 i4NF_Ipi;
	INT16 NF_Power[] = {-92, -89, -86, -83, -80, -75, -70, -65, -60, -55, -52};
	P_EXT_EVENT_ENABLE_NOISE_FLOOR_T prEventExtCmdResult = (P_EXT_EVENT_ENABLE_NOISE_FLOOR_T)Data;
	UCHAR band0_ipi_ant = pAd->Antenna.field.RxPath;
	INT32 band0_nf_total = 0;
	INT32 ret, left_buf_size;
	INT8 tmp_str[64] = {0};

	/*for invalid case, use default ipi_ant*/
	if (band0_ipi_ant == 0 || band0_ipi_ant > 4) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"invalid RxPath, use default band0_ipi_ant 2\n");
		band0_ipi_ant = 2;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"band0_ipi_ant:%d\n", band0_ipi_ant);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"Average Power, u1mode:%d\n", prEventExtCmdResult->u1mode);

	if (prEventExtCmdResult->u1mode == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "Idle Power\n");
		for (i = 0; i < 4; i++) {
			avgpwr[i] = prEventExtCmdResult->au4avgpwr[i];
			dBm[i] = (INT8)((avgpwr[i] & u4Mask) >> u1Shift);
			msb = ((avgpwr[i] & 0x00000100) >> 8);
			dBm[i] = (dBm[i] & 0x7f);
			dBm[i] = ((0x7f ^ dBm[i]) + 1);

			if (msb)
				dBm[i] = dBm[i] * (-1);

			left_buf_size = sizeof(tmp_str) - strlen(tmp_str);
			ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d ", dBm[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return;
			}
		}
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "%s\n", tmp_str);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "IPI\n");
		for (i = 0; i < IPI_ANT_NUM; i++) {
			total = 0;
			i4NF_Ipi = 0;
			for (u1Ipi_Idx = 0; u1Ipi_Idx <= 10; u1Ipi_Idx++) {
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG, "%d-",
					prEventExtCmdResult->au4avgPIHist[i][u1Ipi_Idx]);
				total += prEventExtCmdResult->au4avgPIHist[i][u1Ipi_Idx];
				i4NF_Ipi += (NF_Power[u1Ipi_Idx] * (INT32) prEventExtCmdResult->au4avgPIHist[i][u1Ipi_Idx]);
			}
			dBm[i] = (i4NF_Ipi/(INT32)total);

			left_buf_size = sizeof(tmp_str) - strlen(tmp_str);
			ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d ", dBm[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return;
			}
		}
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "%s\n", tmp_str);
	}

	for (i = 0; i < band0_ipi_ant; i++) {
		band0_nf_total += dBm[i];
	}


#ifdef OFFCHANNEL_SCAN_FEATURE
		if (pAd->ScanCtrl.OffChScan_Ongoing == FALSE)
#endif
		{
#ifdef NF_SUPPORT_V2
			pAd->Avg_NF = band0_nf_total / band0_ipi_ant;
#endif
		}

#ifdef OFFCHANNEL_SCAN_FEATURE
		if (pAd->ScanCtrl.OffChScan_Ongoing == TRUE) {
			pAd->ChannelInfo.AvgNF = band0_nf_total / band0_ipi_ant;
		}
#endif
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "\n");
}

static VOID ExtEventGetAllStaStats(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8 event_type = *Data;

	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

	switch (event_type) {
	case EVENT_PHY_ALL_TX_RX_RATE:
	{
#ifdef CONFIG_MAP_SUPPORT
		P_EXT_EVENT_TX_RATE_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_TX_RATE_RESULT_T)Data;
		union _HTTRANSMIT_SETTING LastTxRate;
		union _HTTRANSMIT_SETTING LastRxRate;
		union _HETRANSMIT_SETTING HELastTxRate, HELastRxRate = {0};

		for (Idx = 0; Idx < prEventExtCmdResult->ucStaNum; Idx++) {
			pEntry = entry_get(pAd,
				prEventExtCmdResult->rAllTxRateResult[Idx].ucWlanIdx);
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
/*For TX rate*/
				LastTxRate.word = LastRxRate.word = 0;
				HELastTxRate.word = HELastRxRate.word = 0;

				if (IS_HE_MODE(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE)) {
					HELastTxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE;
					HELastTxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.BW;
					HELastTxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ldpc ? 1 : 0;
					HELastTxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ShortGI ? 1 : 0;
					HELastTxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.STBC;
				} else {
					LastTxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE;
					LastTxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.BW;
					LastTxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ldpc ? 1 : 0;
					LastTxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ShortGI ? 1 : 0;
					LastTxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.STBC;
				}

				if (IS_HE_MODE(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE)) {
					HELastTxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					HELastTxRate.field.Nss = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.VhtNss;
					pEntry->map_LastTxRate = (UINT32)HELastTxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE  == MODE_VHT) {
					LastTxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.VhtNss - 1)
						& 0x3) << 4) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE  == MODE_OFDM) {
					LastTxRate.field.MCS =
						getLegacyOFDMMCSIndex(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS)
						& 0x0000003F;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				} else {
					LastTxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				}
/*For RX rate */
				if (IS_HE_MODE(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode)) {
					HELastRxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode;
					HELastRxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxBW;
					HELastRxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxCoding ? 1 : 0;
					HELastRxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxGi ? 1 : 0;
					HELastRxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxStbc;
				} else {
					LastRxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode;
					LastRxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxBW;
					LastRxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxCoding ? 1 : 0;
					LastRxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxGi ? 1 : 0;
					LastRxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxStbc;
				}

				if (IS_HE_MODE(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode)) {
					HELastRxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					HELastRxRate.field.Nss = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxNsts;
					pEntry->map_LastRxRate = (UINT32)HELastRxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode  == MODE_VHT) {
					LastRxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxNsts - 1)
						& 0x3) << 4) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode  == MODE_OFDM) {
					LastRxRate.field.MCS =
						getLegacyOFDMMCSIndex(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate)
						& 0x0000003F;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				} else {
					LastRxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				}

			}

		}
#endif
	}
	break;
	case EVENT_PHY_TX_STAT_PER_WCID:
	{
#ifdef EAP_STATS_SUPPORT
		P_EXT_EVENT_TX_STAT_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_TX_STAT_RESULT_T)Data;

		prEventExtCmdResult->u2StaNum = le2cpu16(prEventExtCmdResult->u2StaNum);
		for (Idx = 0; Idx < prEventExtCmdResult->u2StaNum; Idx++) {
			P_EXT_EVENT_ONE_TX_STAT_T rTxStatResult = &prEventExtCmdResult->rTxStatResult[Idx];

			rTxStatResult->u2WlanIdx = le2cpu16(rTxStatResult->u2WlanIdx);
			pEntry = entry_get(pAd, rTxStatResult->u2WlanIdx);
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			if (pEntry && pEntry->wdev && pEntry->Sst == SST_ASSOC &&
			(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)))
#else/*WIFI_IAP_STA_DUMP_FEATURE*/
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
#endif
			{
				pEntry->mpdu_attempts.QuadPart += le2cpu32(rTxStatResult->u4TotalTxCount);
				pEntry->mpdu_retries.QuadPart += le2cpu32(rTxStatResult->u4TotalTxFailCount);
			}

		}
#endif
	}
	break;

	case EVENT_PHY_RX_STAT:
	{
#ifdef EAP_STATS_SUPPORT
		P_EXT_EVENT_RX_STAT_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_RX_STAT_RESULT_T)Data;
		UCHAR       concurrent_bands = PD_GET_BAND_NUM(pAd->physical_dev);

		for (Idx = 0; Idx < concurrent_bands; Idx++) {
			P_EXT_EVENT_RX_STAT_T rRxStatResult = &prEventExtCmdResult->rRxStatResult[Idx];

			pAd->OneSecMibBucket.PdCount = le2cpu16(rRxStatResult->u2PhyRxPdCck) +
												le2cpu16(rRxStatResult->u2PhyRxPdOfdm);
			pAd->OneSecMibBucket.MdrdyCount = le2cpu16(rRxStatResult->u2PhyRxMdrdyCntCck) +
													le2cpu16(rRxStatResult->u2PhyRxMdrdyCntOfdm);
		}
#endif
	}
	break;
	case EVENT_PHY_TXRX_AIR_TIME:
	{
		int k;
		P_EXT_EVENT_TXRX_AIRTIME_INFO_T prEventExtCmdResult = (P_EXT_EVENT_TXRX_AIRTIME_INFO_T)Data;

		for (Idx = 0; Idx < prEventExtCmdResult->u2StaNum; Idx++) {
			pEntry = entry_get(
				pAd, prEventExtCmdResult->rTxRxAirTimeStat[Idx].u2WlanId);
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
				for (k = 0; k < 4; k++) {
					if (pEntry->TxRxTime[k][0] - pEntry->wrapArTxRxTime[k][0] > prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000) {
						/*Wrap around at FW has occurred*/
						/*Update the wrap around value to keep adding that*/
						pEntry->wrapArTxRxTime[k][0] = pEntry->TxRxTime[k][0];
						pEntry->TxRxTime[k][0] += prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000;
					} else {
						pEntry->TxRxTime[k][0] =
							pEntry->wrapArTxRxTime[k][0] + prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000;
					}
					if (pEntry->TxRxTime[k][1] - pEntry->wrapArTxRxTime[k][1] > prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000) {
						pEntry->wrapArTxRxTime[k][1] = pEntry->TxRxTime[k][1];
						pEntry->TxRxTime[k][1] += prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000;
					} else {
						pEntry->TxRxTime[k][1] =
							pEntry->wrapArTxRxTime[k][1] + prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000;
					}
					if (pEntry->TxRxTime[k][0] < pEntry->wrapArTxRxTime[k][0]) {
						pEntry->wrapArTxRxTime[k][0] = 0;
					}
				}
			}
		}
	}
	default:
		break;
	}
}

#ifdef TXRX_STAT_SUPPORT
static VOID ExtEventGetStaTxStat(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_STA_TX_STAT_RESULT_T *CmdStaTxStatResult = (EXT_EVENT_STA_TX_STAT_RESULT_T *)Data;
	UINT32 WcidIdx, i;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	UINT16 wtbl_max_num = hc_get_chip_wtbl_max_num(pAd);
	struct _RTMP_ADAPTER *mac_ad;
	struct _BSS_STRUCT *mbss;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pAd->ApCfg.MBSSID[i].stat_bss.Last1TxCnt = 0;
		pAd->ApCfg.MBSSID[i].stat_bss.Last1TxFailCnt = 0;
	}

	for (WcidIdx = 0; WcidIdx < wtbl_max_num; WcidIdx++) {
		pEntry = entry_get(pAd, WcidIdx);
#ifdef WIFI_IAP_STA_DUMP_FEATURE
		if (pEntry && pEntry->wdev &&  (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)) && pEntry->Sst == SST_ASSOC)
#else/*WIFI_IAP_STA_DUMP_FEATURE*/
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
#endif/*NO WIFI_IAP_STA_DUMP_FEATURE*/
		{
			pEntry->LastOneSecPER = 0;
		}
	}
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++) {
		mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, i);
		if (mac_ad) {
			ctrl = (struct hdev_ctrl *)mac_ad->hdev_ctrl;
			ctrl->rdev.pRadioCtrl->Last1TxCnt = 0;
			ctrl->rdev.pRadioCtrl->Last1TxFailCnt = 0;
		}
	}
	for (WcidIdx = 0; WcidIdx < wtbl_max_num; WcidIdx++) {
		pEntry = entry_get(pAd, WcidIdx);
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			mac_ad = (struct _RTMP_ADAPTER *)pEntry->pAd;
			ctrl = (struct hdev_ctrl *)mac_ad->hdev_ctrl;
			CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] =
				le2cpu32(CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx]);
			CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx] =
				le2cpu32(CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]);
			pEntry->LastOneSecTxTotalCountByWtbl = CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			pEntry->LastOneSecTxFailCountByWtbl = CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			pEntry->TxFailCountByWtbl += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/
			pEntry->TxSuccessByWtbl += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] -
						CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			ctrl->rdev.pRadioCtrl->TotalTxCnt +=
				CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			ctrl->rdev.pRadioCtrl->TotalTxFailCnt +=
				CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			if (ctrl->rdev.pRadioCtrl->TotalTxCnt
				&& ctrl->rdev.pRadioCtrl->TotalTxFailCnt)
				ctrl->rdev.pRadioCtrl->TotalPER =
					((100 * (ctrl->rdev.pRadioCtrl->TotalTxFailCnt))/
								ctrl->rdev.pRadioCtrl->TotalTxCnt);
			ctrl->rdev.pRadioCtrl->Last1TxCnt +=
				CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			ctrl->rdev.pRadioCtrl->Last1TxFailCnt +=
				CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			mbss = MBSS_GET(pEntry->pMbss);
			mbss->stat_bss.Last1TxCnt += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			mbss->stat_bss.Last1TxFailCnt += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			mbss->stat_bss.TxRetriedPacketCount.QuadPart += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			/*PER in percentage*/
			if (CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] && CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]) {
				pEntry->LastOneSecPER = ((100 * (CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]))/
													CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx]);
			}
		}
	}
}
#endif
static VOID ExtEventGetWtblTxCounter(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_WTBL_TX_COUNTER_RESULT_T *CmdWtblTxCounterResult = (EXT_EVENT_WTBL_TX_COUNTER_RESULT_T *)Data;

	UINT32 WcidIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	for (WcidIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, WcidIdx); WcidIdx++) {
		pEntry = entry_get(pAd, WcidIdx);
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
#ifdef CFG_BIG_ENDIAN
			pEntry->TxRetriedPktCount += cpu2le16(CmdWtblTxCounterResult->PerStaRetriedPktCnt[WcidIdx]);
#else
			pEntry->TxRetriedPktCount += CmdWtblTxCounterResult->PerStaRetriedPktCnt[WcidIdx];
#endif
		}

	}
}

static UINT_8 getTxNss(TX_MODE_RATE eTxRate)
{
    return ((eTxRate  >> TX_NSS_SHITF) & TX_NSS_MASK) + 1;
}

static UINT_8 getTxMode(TX_MODE_RATE eTxRate)
{
    /* TODO: implement it */
    return ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);
}

static UINT_8 getTxModeRemap(TX_MODE_RATE eTxRate, UINT16 u2RuIdx, UINT16 u2Direction)
{
	UINT_8 u1TxMod = ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);

	if (u2RuIdx > 0) {
		if (u2Direction == 0)
			return PHY_MODE_HEMU_REMAPPING;
		else
			return PHY_MODE_HETRIG_REMAPPING;
	} else {
		if (u1TxMod < PHY_MODE_HESU_REMAPPING) {
			return u1TxMod;
		} else if (u1TxMod == PHY_MODE_HESU) {
			return PHY_MODE_HESU_REMAPPING;
		}
	}

	return PHY_MODE_UNKNOWN_REMAPPING;

}

static UINT_8 getTxDcm(TX_MODE_RATE eTxRate)
{
    /*TODO: Implement it*/
    return (eTxRate & DCM_EN && (getTxMode(eTxRate) > PHY_MODE_VHT)) ? 1 : 0;
}

static UINT_8 getTxMcs(TX_MODE_RATE eTxRate)
{
    if (getTxMode(eTxRate) == PHY_MODE_HTMIX) {
		return eTxRate & TX_HT_MCS_MASK;
    } else {
	    return eTxRate & TX_NON_HT_MCS_MASK;
    }
}

static UINT_8 getTxBw(TX_MODE_RATE eTxRate, UINT_8 u1MaxBw)
{
	return u1MaxBw - ((eTxRate >> TX_BW_SHIFT) & TX_BW_MASK);
}

static UINT_8 getTone(UINT16 u2RuIdx)
{
	if (u2RuIdx <= 2)
		return 0;
	else if (u2RuIdx > 2 && u2RuIdx <= 6)
		return 1;
	else if (u2RuIdx > 6 && u2RuIdx <= 14)
		return 2;
	else if (u2RuIdx > 14 && u2RuIdx <= 22)
		return 3;
	else
		return 4;
}

static VOID ExtEventGetRuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_RU_RA_INFO_T *CmdRuRaInfo = (EXT_EVENT_RU_RA_INFO_T *)Data;
	UINT16 u2WlanIdx;
	UINT16 u2RuIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;
	UINT8  u1MaxBw = 0;
	CHAR * phyMode[10] = {"CCK", "OFDM", "HT-MIX", "HT-GF", "VHT", "HE-SU", "HE-EXT-SU", "HE-TB", "HE-MU", "UnKnown"};
	CHAR * dcm[2] = {"", "DCM"};
	CHAR * bw[4] = {"BW20", "BW40", "BW80", "BW160"};
	CHAR * tone[5] = {"996-tone", "484-tone", "242-tone", "<106-tone", "UnKnown"};
	PMAC_TABLE_ENTRY pEntry;


	u2WlanIdx   = CmdRuRaInfo->u2WlanIdx;
	u2RuIdx	    = CmdRuRaInfo->u2RuIdx;
	u2Direction = CmdRuRaInfo->u2Direction;
	u2DumpGroup = CmdRuRaInfo->u2DumpGroup;

	pEntry = entry_get(pAd, u2WlanIdx);
	if (!pEntry)
		return;
	u1MaxBw = pEntry->MaxHTPhyMode.field.BW;

	MTWF_PRINT("\nWLAN ID : %d\n", u2WlanIdx);
	MTWF_PRINT("RU Idx : %d", u2RuIdx);

	if (u2Direction == 0)
		MTWF_PRINT(" Downlink\n");
	else

		MTWF_PRINT(" Uplink\n");

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_PRINT("Group: Short-Term RA\n");
		MTWF_PRINT("\tCurrRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate)]);
		MTWF_PRINT("\tNoRateUpCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt);
		MTWF_PRINT("\tSuggestTxModeRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate)]);
		MTWF_PRINT("\tSuggestWF : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1SuggestWF);
		MTWF_PRINT("\tStartProbeUpMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS)]);
		MTWF_PRINT("\tIsProbeUpPeriod : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod);
		MTWF_PRINT("\tInitRateDownTotalCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt);
		MTWF_PRINT("\tInitRateDownOkCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownOkCnt);
		MTWF_PRINT("\tInitRateDownMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS)]);
		MTWF_PRINT("\tProbeDownPending : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgProbeDownPending);
		MTWF_PRINT("\tStSucceCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt);
		MTWF_PRINT("\tStTotalTxCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt);
		MTWF_PRINT("\tRuPrevRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate)]);
		MTWF_PRINT("\tStTotalPpduCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt);
		MTWF_PRINT("\tGI : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1Gi);
		MTWF_PRINT("\tRuTryupFailCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt);
		MTWF_PRINT("\tRuTryupCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupCnt);
		MTWF_PRINT("\tRuTryupCheck : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgRuTryupCheck);

		if (CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_PRINT("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10);
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_PRINT("Group: Long-Term RA\n");

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_PRINT("Group: Others\n");
}

static VOID ExtEventGetMuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_MU_RA_INFO_T *CmdMuRaInfo = (EXT_EVENT_MU_RA_INFO_T *)Data;
	UINT16 u2MuGroupIdx;
	UINT16 u2UserIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;

	u2MuGroupIdx = CmdMuRaInfo->u2MuGroupIdx;
	u2UserIdx    = CmdMuRaInfo->u2UserIdx;
	u2Direction  = CmdMuRaInfo->u2Direction;
	u2DumpGroup  = CmdMuRaInfo->u2DumpGroup;

	MTWF_PRINT("\nMU Group ID : %d\n", u2MuGroupIdx);
	MTWF_PRINT("User Idx : %d", u2UserIdx);

	if (u2Direction == 0)
		MTWF_PRINT(" Downlink\n");
	else
		MTWF_PRINT(" Uplink\n");

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_PRINT("Group: Short-Term RA\n");
		MTWF_PRINT("\tCurrRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate));
		MTWF_PRINT("\tNoRateUpCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt);
		MTWF_PRINT("\tSuggestTxModeRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate));
		MTWF_PRINT("\tSuggestWF : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1SuggestWF);
		MTWF_PRINT("\tStartProbeUpMCS : %x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS);
		MTWF_PRINT("\tIsProbeUpPeriod : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod);
		MTWF_PRINT("\tInitRateDownTotalCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt);
		MTWF_PRINT("\tInitRateDownOkCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownOkCnt);
		MTWF_PRINT("\tInitRateDownMCS : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
			    CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS,
			    getTxMode(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS));
		MTWF_PRINT("\tProbeDownPending : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgProbeDownPending);
		MTWF_PRINT("\tStSucceCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt);
		MTWF_PRINT("\tStTotalTxCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt);
		MTWF_PRINT("\tRuPrevRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate));
		MTWF_PRINT("\tStTotalPpduCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt);
		MTWF_PRINT("\tGI : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1Gi);
		MTWF_PRINT("\tRuTryupFailCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt);
		MTWF_PRINT("\tRuTryupCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupCnt);
		MTWF_PRINT("\tRuTryupCheck : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgRuTryupCheck);

		if (CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_PRINT("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10);
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_PRINT("Group: Long-Term RA\n");

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_PRINT("Group: Others\n");
}

static VOID HeRaEventDispatcher(
	RTMP_ADAPTER *pAd,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *) rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);
#ifdef CFG_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			 "u4EventId = %u, len = %u\n", u4EventId, len);

	switch (u4EventId) {
	case HERA_RU_RA_INFO_EVENT:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "HERA_RU_RA_INFO_EVENT\n");
		ExtEventGetRuRaInfo(pAd, pData, len);
		break;

	case HERA_MU_RA_INFO_EVENT:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "HERA_MU_RA_INFO_EVENT\n");
		ExtEventGetMuRaInfo(pAd, pData, len);
		break;

	default:
		break;
	}
}

static BOOLEAN IsUnsolicitedEvent(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_SEQ_NUM(event_rxd) == 0)                          ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_FW_LOG_2_HOST)	||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_THERMAL_PROTECT)  ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_ASSERT_DUMP) ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_PS_SYNC)  ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_HERA_INFO_CTRL))

		return TRUE;

	return FALSE;
}

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
static INT ExtEventPreCalStoreProc(RTMP_ADAPTER *pAd, UINT32 EventId, UINT8 *Data)
{
	UINT32 Offset = 0, IDOffset = 0, LenOffset = 0, HeaderSize = 0, CalDataSize = 0, BitMap = 0;
	UINT32 i, Length, TotalSize, ChGroupId = 0;
	UINT16 DoPreCal = 0;
	static int rf_count;
	int rf_countMax;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			 "\x1b[41m ------------> \x1b[m\n");
	rf_countMax = pAd->ChGrpMap == 0x1FF ? 9 : 6;

	switch (EventId) {
	case PRECAL_TXLPF: {
		TXLPF_CAL_INFO_T *prTxLPFGetEvent = (TXLPF_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXLPF_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXLPF_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxLPFGetEvent->u2BitMap = le2cpu16(prTxLPFGetEvent->u2BitMap);
		BitMap = prTxLPFGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM; i++)
			prTxLPFGetEvent->au4Data[i] = le2cpu32(prTxLPFGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXIQ: {
		TXIQ_CAL_INFO_T *prTxIQGetEvent = (TXIQ_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXIQ_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXIQ_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxIQGetEvent->u2BitMap = le2cpu16(prTxIQGetEvent->u2BitMap);
		BitMap = prTxIQGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxIQGetEvent->au4Data[i] = le2cpu32(prTxIQGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXDC: {
		TXDC_CAL_INFO_T *prTxDCGetEvent = (TXDC_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXDC_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXDC_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxDCGetEvent->u2BitMap = le2cpu16(prTxDCGetEvent->u2BitMap);
		BitMap = prTxDCGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxDCGetEvent->au4Data[i] = le2cpu32(prTxDCGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFI: {
		RXFI_CAL_INFO_T *prRxFIGetEvent = (RXFI_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFI_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFI_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFIGetEvent->u2BitMap = le2cpu16(prRxFIGetEvent->u2BitMap);
		BitMap = prRxFIGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*4; i++)
			prRxFIGetEvent->au4Data[i] = le2cpu32(prRxFIGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFD: {
		RXFD_CAL_INFO_T *prRxFDGetEvent = (RXFD_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFD_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFD_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFDGetEvent->u2BitMap = le2cpu16(prRxFDGetEvent->u2BitMap);
		BitMap = prRxFDGetEvent->u2BitMap;
		/* Update group ID */
		prRxFDGetEvent->u4ChGroupId = le2cpu32(prRxFDGetEvent->u4ChGroupId);
		ChGroupId = prRxFDGetEvent->u4ChGroupId;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[33m RXFD ChGroupId %d\x1b[m\n", ChGroupId);
		for (i = 0;
		i < (SCN_NUM*RX_SWAGC_LNA_NUM)+(SCN_NUM*RX_FDIQ_LPF_GAIN_NUM*RX_FDIQ_TABLE_SIZE*3);
		i++)
			prRxFDGetEvent->au4Data[i] = le2cpu32(prRxFDGetEvent->au4Data[i]);
	}
	break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[41m Not support this calibration item !!!! \x1b[m\n");
		break;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			 "\x1b[33m EventID = %d, Bitmp = %x\x1b[m\n", EventId, BitMap);
	/* Update offset parameter */
	IDOffset = LenOffset = Offset = pAd->PreCalWriteOffSet;
	LenOffset += 4; /*Skip ID field*/
	Offset += 8; /*Skip (ID + Len) field*/

	if (pAd->PreCalStoreBuffer != NULL) {
		/* Store ID */
		os_move_mem(pAd->PreCalStoreBuffer + IDOffset, &EventId, sizeof(EventId));
		/* Store header */
		os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, (ULONG)HeaderSize);
		Offset += HeaderSize; /*Skip header*/
		Data += HeaderSize; /*Skip header*/

		/* Store pre-cal data */
		if (EventId == PRECAL_RXFD) {
			if ((rf_count < rf_countMax) && (BitMap & (1 << ChGroupId))) {
				rf_count++;
				os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, CalDataSize);
				Offset += CalDataSize;
			}
		} else {
			int count = 0;
			int countMax = pAd->ChGrpMap == 0x1FF ? 8 : 5;

			for (i = 0; i < CHANNEL_GROUP_NUM; i++) {
				if (count > countMax)
					break;

				if (BitMap & (1 << i)) {
					count++;
					os_move_mem(pAd->PreCalStoreBuffer + Offset, Data + i * CalDataSize, CalDataSize);
					Offset += CalDataSize;
				}
			}
		}

		/* Update current temp buffer write-offset */
		pAd->PreCalWriteOffSet = Offset;
		/* Calculate buffer size (len + header + data) for each calibration item */
		Length = Offset - LenOffset;
		/* Store buffer size for each calibration item */
		os_move_mem(pAd->PreCalStoreBuffer + LenOffset, &Length, sizeof(Length));

		/* The last event ID - update calibration data to flash */
		if (EventId == PRECAL_RXFI) {
			TotalSize = pAd->PreCalWriteOffSet;
#ifdef RTMP_FLASH_SUPPORT
			/* Write pre-cal data to flash */
			if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_FLASH_MODE)
				RtmpFlashWrite(pAd->hdev_ctrl, pAd->PreCalStoreBuffer,
					get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, TotalSize);
#endif/* RTMP_FLASH_SUPPORT */
			if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_BIN_MODE)
				rtmp_cal_write_to_bin(pAd, pAd->PreCalStoreBuffer,
					get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, TotalSize);

			/* Raise DoPreCal bits */
			if (PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_FLASH_MODE
				|| PD_GET_E2P_ACCESS_MODE(pAd->physical_dev) == E2P_BIN_MODE) {
				RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);
				DoPreCal |= (1 << 2);
				RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);
			}

			/* Reset parameter */
			pAd->PreCalWriteOffSet = 0;
			rf_count = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
					 "\x1b[41m Pre-calibration done !! \x1b[m\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
				 "\x1b[41m PreCalStoreBuffer is NULL !! \x1b[m\n");
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
			 "\x1b[41m <------------ \x1b[m\n");
	return TRUE;
}

NTSTATUS PreCalTxLPFStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXLPF, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxIQStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXIQ, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxDCStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXDC, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFIStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFI, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFDStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFD, CMDQelmt->buffer);
	return 0;
}
static VOID ExtEventTxLPFCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXLPF_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXLPF_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXLPF, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxLpfInfo(pAd->rlmCalCache, Data, Length);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[42m Not store to flash !! \x1b[m\n");
	}

	return;
};

static VOID ExtEventTxIQCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXIQ_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXIQ_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXIQ, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxIqInfo(pAd->rlmCalCache, Data, Length);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[42m Not store to flash !! \x1b[m\n");
	}

	return;
};

static VOID ExtEventTxDCCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXDC_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXDC_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXDC, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxDcInfo(pAd->rlmCalCache, Data, Length);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[42m Not store to flash !! \x1b[m\n");
	}

	return;
};

static VOID ExtEventRxFICalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFI_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFI_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFI, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFiInfo(pAd->rlmCalCache, Data, Length);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[42m Not store to flash !! \x1b[m\n");
	}

	return;
};

static VOID ExtEventRxFDCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFD_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFD_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFD, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFdInfo(pAd->rlmCalCache, Data, Length);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				 "\x1b[42m Not store to flash !! \x1b[m\n");
	}

	return;
};
#endif  /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
/*For tx statistic info, the function is only for GET_TX_STAT_ENTRY_TX_CNT*/
static VOID event_get_tx_statistic_handle(struct _RTMP_ADAPTER *pAd, UINT8 *data, UINT32 Length)
{
	struct _EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T *p_event_hdr =
		(struct _EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T *)data;
	UINT16 wcid = 0;
	struct _MAC_TABLE_ENTRY *entry;
	struct _STA_TR_ENTRY *tr_entry;
	UINT32 tx_success = 0, i = 0;
	UINT16 num = 0;

	struct _EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T *p_temp = NULL;
	struct _EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T *p_event_data =
		(struct _EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T *)(p_event_hdr->aucTxStatisticResult);
	num = (Length - sizeof(struct _EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)) / sizeof(struct _EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T);

	for (i = 0; i < num; i++) {

		p_temp = p_event_data + i;

		wcid = le2cpu16(p_temp->u2WlanIdx);

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
			continue;

		entry = entry_get(pAd, wcid);

		if (!entry || IS_ENTRY_NONE(entry))
			continue;

		tr_entry = tr_entry_get(pAd, entry->tr_tb_idx);

		if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
			continue;

#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_STA(tr_entry))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		p_temp->u4EntryTxCount = le2cpu32(p_temp->u4EntryTxCount);
		p_temp->u4EntryTxFailCount = le2cpu32(p_temp->u4EntryTxFailCount);
		if (p_temp->u4EntryTxCount > 0) {
			tx_success = p_temp->u4EntryTxCount - p_temp->u4EntryTxFailCount;
			entry->TotalTxSuccessCnt += tx_success;
			entry->one_sec_tx_succ_pkts = tx_success;
			entry->ContinueTxFailCnt += p_temp->u4EntryTxFailCount;
			entry->TxStatRspCnt++;
			entry->tx_per = 100 * p_temp->u4EntryTxFailCount / p_temp->u4EntryTxCount;
		}

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"wcid(%d), TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n",
			wcid,
			p_temp->u4EntryTxCount,
			p_temp->u4EntryTxFailCount,
			entry->TotalTxSuccessCnt,
			(entry->TxStatRspCnt) ? "Valid" : "Invalid");
	}

}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

static VOID EventExtEventHandler(RTMP_ADAPTER *pAd, UINT8 ExtEID, UINT8 *Data,
								 UINT32 Length, EVENT_RXD *event_rxd)
{
	switch (ExtEID) {
	case EXT_EVENT_CMD_RESULT:
		EventExtCmdResult(NULL, Data, Length);
		break;

	case EXT_EVENT_ID_PS_SYNC:
		ExtEventPsSyncHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_FW_LOG_2_HOST:
		ExtEventFwLog2HostHandler(pAd, Data, Length, event_rxd);
		break;
#ifdef COEX_SUPPORT

	case EXT_EVENT_BT_COEX:
		ExtEventBTCoexHandler(pAd, Data, Length);
		break;
#endif /* COEX_SUPPORT */

	case EXT_EVENT_THERMAL_PROTECT:
		EventThermalProtectHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_PRETBTT_INT:
		ExtEventPretbttIntHandler(pAd, Data, Length);
		break;

#ifdef CONFIG_STA_SUPPORT
	case EXT_EVENT_ID_ROAMING_DETECTION_NOTIFICATION:
		ExtEventRoamingDetectionHandler(pAd, Data, Length);
		break;
#endif /*CONFIG_STA_SUPPORT*/

	case EXT_EVENT_BEACON_LOSS:
		ExtEventBeaconLostHandler(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	case EXT_EVENT_RA_THROUGHPUT_BURST:
		ExtEventThroughputBurst(pAd, Data, Length);
		break;

	case EXT_EVENT_G_BAND_256QAM_PROBE_RESULT:
		ExtEventGBand256QamProbeResule(pAd, Data, Length);
		break;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	case EXT_EVENT_ID_ASSERT_DUMP:
		ExtEventAssertDumpHandler(pAd, Data, Length, event_rxd);
		break;
#if defined(MT_MAC) && defined(TXBF_SUPPORT)

	case EXT_EVENT_ID_BF_STATUS_READ:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "EXT_EVENT_ID_BF_STATUS_READ\n");
		ExtEventBfStatusRead(pAd, Data, Length);
		break;
#endif /* MT_MAC && TXBF_SUPPORT */

	case EXT_EVENT_ID_MEC_INFO_READ:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "EXT_EVENT_ID_MEC_INFO_READ\n");
		ExtEventMecInfoRead(pAd, Data, Length);
		break;

#ifdef CONFIG_WLAN_SERVICE
	case EXT_EVENT_ID_RF_TEST:
	{
		struct service_test *serv_test;
		UINT32 en_log = 0;
		struct test_log_dump_cb *test_log_dump;
		struct test_operation *ops;

		serv_test = (struct service_test *)(pAd->serv.serv_handle);
		ops = serv_test->test_op;
		test_log_dump = &serv_test->test_log_dump[0];
		en_log = serv_test->en_log;

		if (ops->op_evt_rf_test_cb)
			ops->op_evt_rf_test_cb(serv_test->test_winfo, test_log_dump, en_log, Data, Length);
		break;
	}
#else
#ifdef CONFIG_ATE
	case EXT_EVENT_ID_RF_TEST:
		MT_ATERFTestCB(pAd, Data, Length);
		break;
#endif /* CONFIG_ATE */
#endif /* CONFIG_WLAN_SERVICE */

#ifdef MT_DFS_SUPPORT

	case EXT_EVENT_ID_RDD_REPORT:
		ExtEventRddReportHandler(pAd, Data, Length);
		break;
#endif

	case EXT_EVENT_ID_RDD_IPI_HIST_CTRL:
		ExtEventIdlePwrReportHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_MAX_AMSDU_LENGTH_UPDATE:
		ExtEventMaxAmsduLengthUpdate(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BA_TRIGGER:
		ExtEventBaTriggerHandler(pAd, Data, Length);
		break;

#ifdef WIFI_SPECTRUM_SUPPORT
	case EXT_EVENT_ID_WIFI_SPECTRUM:
		ExtEventWifiSpectrumHandler(pAd, Data, Length);
		break;
#endif /* WIFI_SPECTRUM_SUPPORT */

	case EXT_EVENT_CSA_NOTIFY:
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "rx EXT_EVENT_CSA_NOTIFY\n");
		ExtEventCswNotifyHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_TMR_CALCU_INFO:
		ExtEventTmrCalcuInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BSS_ACQ_PKT_NUM:
		ExtEventBssAcQPktNumHandler(pAd, Data, Length);
		break;
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)

	case EXT_EVENT_ID_TXLPF_CAL_INFO:
		ExtEventTxLPFCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXIQ_CAL_INFO:
		ExtEventTxIQCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXDC_CAL_INFO:
		ExtEventTxDCCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFI_CAL_INFO:
		ExtEventRxFICalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFD_CAL_INFO:
		ExtEventRxFDCalInfoHandler(pAd, Data, Length);
		break;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	case EXT_EVENT_ID_THERMAL_FEATURE_CTRL:
		EventThermalHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TX_POWER_FEATURE_CTRL:
		EventTxPowerHandler(pAd, Data, Length);
		break;
#ifdef CONFIG_HOTSPOT_R2

	case EXT_EVENT_ID_INFORM_HOST_REPROCESS_PKT:
		ExtEventReprocessPktHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_GET_CR4_HOTSPOT_CAPABILITY:
		ExtEventGetHotspotCapabilityHandler(pAd, Data, Length);
		break;
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef DABS_QOS
	case EXT_EVENT_ID_FASTPATH_RPT:
		ExtEventFastPathRptHandler(pAd, Data, Length);
		break;

#endif
		case EXT_EVENT_ID_HERA_INFO_CTRL:
			HeRaEventDispatcher(pAd, Data, Length);
			break;

	case EXT_EVENT_GET_CR4_TX_STATISTICS:
		ext_event_get_cr4_multi_tx_statistics(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	case EXT_EVENT_GET_TX_STATISTIC:
		event_get_tx_statistic_handle(pAd, Data, Length);
		break;
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
#ifdef DOT11_HE_AX
	case EXT_EVENT_ID_BCC_NOTIFY:
		ExtEventBccNotifyHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_MURU_CTRL:
		ExtEventMuruHandler(pAd, Data, Length);
		break;
#endif

	case EXT_EVENT_ID_TPC_INFO:
		ExtEvenTpcInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RX_STAT_INFO:
		EventRxvHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_ECC_RESULT:
		event_ecc_result(pAd, Data, Length);
		break;
#ifdef TXRX_STAT_SUPPORT
	case EXT_EVENT_ID_GET_STA_TX_STAT:
		ExtEventGetStaTxStat(pAd, Data, Length);
		break;
#endif
	case EXT_EVENT_ID_GET_ALL_STA_STATS:
		ExtEventGetAllStaStats(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_ENABLE_NOISEFLOOR:
		ExtEventNFAvgPwr(pAd, Data, Length);
		break;
	case EXT_EVENT_ID_EDCCA:
		ExtEventEDCCA(pAd, Data, Length);
		break;
#ifdef CFG_SUPPORT_FALCON_SR
	case EXT_EVENT_ID_SR_INFO:
		EventSrHandler(pAd, Data, Length);
		break;
#endif /*CFG_SUPPORT_FALCON_SR*/
	case EXT_EVENT_ID_PHY_STAT_INFO:
		EventPhyStatHandler(pAd, Data, Length);
		break;
#ifdef IGMP_TVM_SUPPORT
		case EXT_EVENT_ID_IGMP_MULTICAST_RESP:
		ext_event_get_igmp_multicast_table(pAd, Data, Length);
		break;
#endif	/* IGMP_TVM_SUPPORT */

	case EXT_EVENT_ID_RXFE_LOSS_COMP:
		EventRxFeCompHandler(pAd, Data, Length);
		break;

#ifdef WIFI_MD_COEX_SUPPORT
	case EXT_EVENT_ID_LTE_UNSAFE_CHN_REPORT:
		EventLteSafeChnHandler(pAd, Data, Length);
		break;
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
	case EXT_EVENT_ID_CSI_CTRL:
		ExtEventCSICtrl(pAd, Data, Length);
		break;
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	case EXT_EVENT_ID_TWT_RESUME_INFO:
		event_twt_resume_info(pAd, Data, Length);
		break;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	case EXT_EVENT_ID_GET_WTBL_TX_COUNTER:
		ExtEventGetWtblTxCounter(pAd, Data, Length);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Unknown Ext Event(%x)\n", ExtEID);
		break;
	}
}

static VOID EventExtGenericEventHandler(UINT8 *Data)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	if (EventExtCmdResult->u4Status == CMD_RESULT_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "CMD Success\n");
	} else if (EventExtCmdResult->u4Status == CMD_RESULT_NONSUPPORT) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "CMD Non-Support\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "CMD Fail!, EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	}
}

static VOID EventGenericEventHandler(UINT8 *Data)
{
	struct _INIT_EVENT_CMD_RESULT *EventCmdResult =
		(struct _INIT_EVENT_CMD_RESULT *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "EventCmdResult.ucCID = 0x%x\n",
			  EventCmdResult->ucCID);

	if (EventCmdResult->ucStatus == CMD_RESULT_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "CMD Success\n");
	} else if (EventCmdResult->ucStatus == CMD_RESULT_NONSUPPORT) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "CMD Non-Support\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "CMD Fail!, EventCmdResult.ucStatus = 0x%x\n",
				  EventCmdResult->ucStatus);
	}
}


static VOID GenericEventHandler(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	switch (EID) {
	case EXT_EVENT:
		EventExtGenericEventHandler(Data);
		break;

	case GENERIC_EVENT:
		EventGenericEventHandler(Data);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Unknown Event(%x)\n", EID);
		break;
	}
}

VOID UnsolicitedEventHandler(
	RTMP_ADAPTER *pAd, UINT8 EID, UINT8 ExtEID,
	UINT8 *Data, UINT32 Length, EVENT_RXD *event_rxd)
{
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "ExtEID=0x%x\n", ExtEID);

	switch (EID) {
	case EVENT_CH_PRIVILEGE:
		EventChPrivilegeHandler(pAd, Data, Length);
		break;

	case EXT_EVENT:
		EventExtEventHandler(pAd, ExtEID, Data, Length, event_rxd);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Unknown Event(%x)\n", EID);
		break;
	}
}


static BOOLEAN IsRspLenVariableAndMatchSpecificMinLen(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size <= GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0) && IS_CMD_MSG_LEN_VAR_FLAG_SET(msg))
		return TRUE;
	else
		return FALSE;
}



static BOOLEAN IsRspLenNonZeroAndMatchExpected(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size == GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0))
		return TRUE;
	else
		return FALSE;
}

static VOID HandlSeq0AndOtherUnsolicitedEvents(RTMP_ADAPTER *pAd,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UnsolicitedEventHandler(pAd,
							GET_EVENT_FW_RXD_EID(event_rxd),
							GET_EVENT_FW_RXD_EXT_EID(event_rxd),
							GET_EVENT_HDR_ADDR(net_pkt),
							GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd), event_rxd);
}


static void CompleteWaitCmdMsgOrFreeCmdMsg(struct cmd_msg *msg)
{
	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_COMPLETE(&msg->ack_done);
	else {
		DlListDel(&msg->list);
		AndesFreeCmdMsg(msg);
	}
}

static void FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(
	EVENT_RXD *event_rxd, struct cmd_msg *msg)
{
	/* Error occurs!!! dump info for debugging */
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "expect response len(%d), command response len(%zd) invalid\n",
			  msg->attr.ctrl.expect_size, GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x\n",
			  msg->attr.type, msg->attr.ext_type,
			  GET_EVENT_FW_RXD_EXT_EID(event_rxd));
	msg->attr.ctrl.expect_size = GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd);
}


static VOID HandleLayer1GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static void CallEventHookHandlerOrDumpErrorMsg(EVENT_RXD *event_rxd,
		struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	if (msg->attr.rsp.handler == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "rsp_handler is NULL!!!!(cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x)\n",
				  msg->attr.type, msg->attr.ext_type,
				  GET_EVENT_FW_RXD_EXT_EID(event_rxd));

		if (GET_EVENT_FW_RXD_EXT_EID(event_rxd) == 0) {
			HandleLayer1GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
									 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
									 GET_EVENT_HDR_ADDR(net_pkt));
		}
	} else {
		msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt),
							  GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
	}
}

static void FwDebugPurposeHandler(EVENT_RXD *event_rxd,
								  struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle FW debug purpose only */
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static VOID HandleNormalLayer1Events(EVENT_RXD *event_rxd,
									 struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static void EventLenVariableHandler(EVENT_RXD *event_rxd,
									struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle event len variable */
	HandleNormalLayer1Events(event_rxd, msg, net_pkt);
}


static void HandleLayer1Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handler normal layer 1 event */
	if (IsRspLenNonZeroAndMatchExpected(event_rxd, msg))
		HandleNormalLayer1Events(event_rxd, msg, net_pkt);
	else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
		/* hanle event len variable */
		EventLenVariableHandler(event_rxd, msg, net_pkt);
	} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
		/* hanle FW debug purpose only */
		FwDebugPurposeHandler(event_rxd, msg, net_pkt);
	} else
		FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
}

static VOID HandleLayer0GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static BOOLEAN IsNormalLayer0Events(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_EID(event_rxd) == MT_FW_START_RSP)            ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_RESTART_DL_RSP)          ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_TARGET_ADDRESS_LEN_RSP)  ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_PATCH_SEM_RSP)           ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == EVENT_ACCESS_REG))
		return TRUE;
	else
		return FALSE;
}

static void HandleLayer0Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handle layer0 generic event */
	if (GET_EVENT_FW_RXD_EID(event_rxd) == GENERIC_EVENT) {
		HandleLayer0GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
								 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
								 GET_EVENT_HDR_ADDR(net_pkt) - 4);
	} else {
		/* handle normal layer0 event */
		if (IsNormalLayer0Events(event_rxd)) {
#ifdef CFG_BIG_ENDIAN
			event_rxd->fw_rxd_2.word = cpu2le32(event_rxd->fw_rxd_2.word);
#endif
			if (msg->attr.rsp.handler != NULL)
				msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt) - 4, GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd) + 4);
		} else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
			/* hanle event len is variable */
			EventLenVariableHandler(event_rxd, msg, net_pkt);
		} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
			/* hanle FW debug purpose only */
			FwDebugPurposeHandler(event_rxd, msg, net_pkt);
		} else
			FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
	}
}

static VOID GetMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
									   unsigned long *flags)
{
	RTMP_SPIN_LOCK_IRQSAVE(&((*ctl)->ackq_lock), flags);
}

static VOID ReleaseMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
		unsigned long *flags)
{
	RTMP_SPIN_UNLOCK_IRQRESTORE(&((*ctl)->ackq_lock), flags);
}

static UINT8 GetEventFwRxdSequenceNumber(EVENT_RXD *event_rxd)
{
	return (UINT8)(GET_EVENT_FW_RXD_SEQ_NUM(event_rxd));
}

static VOID HandleSeqNonZeroNormalEvents(VOID *physical_dev,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UINT8 peerSeq;
	struct cmd_msg *msg, *msg_tmp;
	RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);
	struct MCU_CTRL *ctl = PD_GET_MCU_CTRL_PTR(pAd->physical_dev);
	unsigned long flags = 0;

	peerSeq = GetEventFwRxdSequenceNumber(event_rxd);
	GetMCUCtrlAckQueueSpinLock(&ctl, &flags);
	DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "msg->seq=%x, field.seq_num=%x, msg->attr.ctrl.expect_size=%d\n",
				  msg->seq, peerSeq, msg->attr.ctrl.expect_size);

		if (msg->seq == peerSeq) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					 "(seq=%d)\n", msg->seq);

			msg->receive_time_in_jiffies = jiffies;
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					 "CMD_ID(0x%x 0x%x),total spent %ld ms\n",
					  msg->attr.type, msg->attr.ext_type,
					  ((msg->receive_time_in_jiffies - msg->sending_time_in_jiffies) * 1000 / OS_HZ));
#ifdef WIFI_UNIFIED_COMMAND
			if (GET_EVENT_FW_RXD_OPTION(event_rxd) & UNI_CMD_OPT_BIT_1_UNI_EVENT) {
				if (msg->attr.rsp.handler != NULL)
					msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt),
										GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
			} else
#endif /* WIFI_UNIFIED_COMMAND */
			{
				if (GET_EVENT_FW_RXD_EID(event_rxd) == EXT_EVENT)
					HandleLayer1Events(event_rxd, msg, net_pkt);
				else
					HandleLayer0Events(event_rxd, msg, net_pkt);
			}

			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					 "need_wait=%d\n",
					  IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg));
			CompleteWaitCmdMsgOrFreeCmdMsg(msg);
			break;
		}
	}
	ReleaseMCUCtrlAckQueueSpinLock(&ctl, &flags);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID AndesMTRxProcessUniEvent(VOID *physical_dev, PNDIS_PACKET net_pkt)
{
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "seq_num=%d, cmd_id=%x\n",
			  GET_EVENT_FW_RXD_SEQ_NUM(event_rxd), GET_EVENT_FW_RXD_EID(event_rxd));

	if (GET_EVENT_FW_RXD_OPTION(event_rxd) & UNI_CMD_OPT_BIT_2_UNSOLICIT_EVENT)
		UniEventUnsolicitMainHandler(physical_dev, net_pkt);
	else
		HandleSeqNonZeroNormalEvents(physical_dev, event_rxd, net_pkt);
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID AndesMTRxProcessEvent(RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	EVENT_RXD *event_rxd = NULL;

	if (net_pkt == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"net_pkt is NULL!\n");
		return;
	}

	event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
	if (event_rxd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"event_rxd is NULL!\n");
		return;
	}

#ifdef WIFI_UNIFIED_COMMAND
	if (GET_EVENT_FW_RXD_OPTION(event_rxd) & UNI_CMD_OPT_BIT_1_UNI_EVENT) {
		AndesMTRxProcessUniEvent(pAd->physical_dev, net_pkt);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
#ifdef CONFIG_TRACE_SUPPORT
		TRACE_MCU_EVENT_INFO(GET_EVENT_FW_RXD_LENGTH(event_rxd),
							 GET_EVENT_FW_RXD_PKT_TYPE_ID(event_rxd),
							 GET_EVENT_FW_RXD_EID(event_rxd),
							 GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
							 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
							 GET_EVENT_HDR_ADDR(net_pkt),
							 GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
#endif
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "seq_num=%d, ext_eid=%x\n",
				  GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
				  GET_EVENT_FW_RXD_EXT_EID(event_rxd));

		if (IsUnsolicitedEvent(event_rxd))
			HandlSeq0AndOtherUnsolicitedEvents(pAd, event_rxd, net_pkt);
		else
			HandleSeqNonZeroNormalEvents(pAd, event_rxd, net_pkt);
	}
}

VOID AndesMTRxEventHandler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = PD_GET_MCU_CTRL_PTR(pAd->physical_dev);
	EVENT_RXD *event_rxd = (EVENT_RXD *)data;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

#ifdef CFG_BIG_ENDIAN
	event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);
#endif
	msg = AndesAllocCmdMsg(pAd, GET_EVENT_FW_RXD_LENGTH(event_rxd));

	if (!msg || !msg->net_pkt)
		return;

	AndesAppendCmdMsg(msg, (char *)data, GET_EVENT_FW_RXD_LENGTH(event_rxd));
	AndesMTRxProcessEvent(pAd, msg);
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

	RTMPFreeNdisPacket(pAd, msg->net_pkt);

#endif
	AndesFreeCmdMsg(msg);
}

VOID EventThermalHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case THERMAL_EVENT_THERMAL_SENSOR_BASIC_INFO:
		EventThermalSensorShowInfo(pAd, Data, Length);
		break;

	case THERMAL_EVENT_THERMAL_SENSOR_TASK_RESPONSE:
		EventThermalSensorTaskResp(pAd, Data, Length);
		break;

	default:
		break;
	}
}



VOID EventThermalSensorTaskResp(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T  prEventTheralSensorTaskResp;

	prEventTheralSensorTaskResp = (P_EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T)Data;

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "result fp: #%x\n", prEventTheralSensorTaskResp->u4FuncPtr);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "origin fp: #%p\n", ThermalTaskAction);

	if (prEventTheralSensorTaskResp->u4FuncPtr)
		ThermalTaskAction(pAd,
							prEventTheralSensorTaskResp->u4PhyIdx,
							prEventTheralSensorTaskResp->u4ThermalTaskProp,
							prEventTheralSensorTaskResp->u1ThermalAdc);
}

VOID EventThermalSensorShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T  prEventTheralSensorItem;
	UINT8 u1ThermalItemIdx;

	prEventTheralSensorItem = (P_EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T)Data;

	MTWF_PRINT("Thermal Task Num: %d\n\n", prEventTheralSensorItem->u1ThermoTaskNum);

	MTWF_PRINT("SensorTh: %d (Low), %d (High)\n\n", prEventTheralSensorItem->u1SensorThLow, prEventTheralSensorItem->u1SensorThHigh);

	MTWF_PRINT("==============================================================================================\n");
	MTWF_PRINT("  Item    Property    fgTrig    Threshold    FuncHandle    Data               \n");
	MTWF_PRINT("==============================================================================================\n");

	/* Thermal State Info Table */
	for (u1ThermalItemIdx = 0; u1ThermalItemIdx < prEventTheralSensorItem->u1ThermoTaskNum; u1ThermalItemIdx++) {
		MTWF_PRINT("  %d         %d         %d         %3d         #%x         #%x\n",
															u1ThermalItemIdx,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4ThermalTaskProp,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].fgTrigEn,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u1Thres,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4BoundHandle,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4Data
															);
	}
}

VOID EventTxPowerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TXPOWER_EVENT_SHOW_INFO:
		EventTxPowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE:
		EventTxPowerCompTable(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_EPA_STATUS:
		EventTxPowerEPAInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO:
		EventPowerTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TARGET_POWER_INFO_GET:
		break;

	case TXPOWER_EVENT_SHOW_ALL_RATE_TXPOWER_INFO:
		EventTxPowerAllRatePowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO:
		EventThermalCompTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO:
		EventTxvBbpPowerInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventTxPowerShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->txpower_show_info)
		ops->txpower_show_info(pAd, Data, Length);
}

VOID EventTxPowerAllRatePowerShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops && arch_ops->arch_txpower_all_rate_info)
		arch_ops->arch_txpower_all_rate_info(pAd, Data, Length);
}

VOID EventTxPowerEPAInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_EPA_STATUS_T  prEventTxPowerEPAInfo;

	prEventTxPowerEPAInfo = (P_EXT_EVENT_EPA_STATUS_T)Data;
	/* update EPA status */
	pAd->fgEPA = prEventTxPowerEPAInfo->fgEPA;
}

NTSTATUS EventTxvBbpPowerInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXV_BBP_POWER_INFO_T prEventTxvBbpPowerInfo;
	UINT8 ucBandIdx = 0;
	UINT8 ucAntIdx = 0;
	UINT8 ucAbsoTxvPower = 0;
	PUINT8 pu1AbsoBbpPower = NULL;
	UINT8 ucNegTxv = 0;
	PUINT8 pu1NegBbp = NULL;
	UINT8 ucWfNum = 0;

	/* get event info buffer contents */
	prEventTxvBbpPowerInfo = (P_EXT_EVENT_TXV_BBP_POWER_INFO_T)Data;

	if (Length != sizeof(EXT_EVENT_TXV_BBP_POWER_INFO_T))
		return STATUS_UNSUCCESSFUL;

	/*WF Path*/
	ucWfNum = prEventTxvBbpPowerInfo->ucWfNum;

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pu1AbsoBbpPower, ucWfNum);

	/*Check allocated memory*/
	if (!pu1AbsoBbpPower)
		return STATUS_UNSUCCESSFUL;

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pu1NegBbp, ucWfNum);

	/*Check allocated memory and Free memory*/
	if (!pu1NegBbp) {
		os_free_mem(pu1AbsoBbpPower);
		return STATUS_UNSUCCESSFUL;
	}

	/* initinal memory */
	os_zero_mem(pu1AbsoBbpPower, ucWfNum);
	os_zero_mem(pu1NegBbp, ucWfNum);

	if (prEventTxvBbpPowerInfo->cTxvPower < 0) {
		ucAbsoTxvPower = ~prEventTxvBbpPowerInfo->cTxvPower + 1;
		ucNegTxv = 1;
	} else
		ucAbsoTxvPower = prEventTxvBbpPowerInfo->cTxvPower;

	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] < 0) {
			*(pu1AbsoBbpPower + ucAntIdx) = ~prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] + 1;
			*(pu1NegBbp + ucAntIdx) = 1;
		} else
			*(pu1AbsoBbpPower + ucAntIdx)  = prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx];
	}

	ucBandIdx = prEventTxvBbpPowerInfo->ucBandIdx;

	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT("   Target TXV and BBP POWER INFO (per packet)\n");
	MTWF_PRINT("=============================================================================\n");

	/* get cTxvPower */
	if (prEventTxvBbpPowerInfo->cTxvPower % 2) {
	MTWF_PRINT("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1));
	} else {
	MTWF_PRINT("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1));
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* BBP POWER INFO */
	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] % 2) {
			MTWF_PRINT("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1));
		} else {
			MTWF_PRINT("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1));

		}
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* free allocated memory */
	os_free_mem(pu1AbsoBbpPower);
	os_free_mem(pu1NegBbp);

	return STATUS_SUCCESS;
}



 VOID EventThermalStateShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_STATE_INFO_T  prEventThermalStateInfo;
	UINT8  ucThermalItemIdx;
	CHAR * cThermalItem[THERMO_ITEM_NUM] = {"DPD_CAL          ", /*  0 */
												"OVERHEAT         ", /*  1 */
												"BB_HI            ", /*  2 */
												"BB_LO            ", /*  3 */
												"NTX_PROTECT_HI   ", /*  4 */
												"NTX_PROTECT_LO   ", /*  5 */
												"ADM_PROTECT_HI   ", /*  6 */
												"ADM_PROTECT_LO   ", /*  7 */
												"RF_PROTECT_HI    ", /*  8 */
												"_TSSI_COMP       ", /*  9 */
												"TEMP_COMP_N7_2G4 ", /* 10 */
												"TEMP_COMP_N6_2G4 ", /* 11 */
												"TEMP_COMP_N5_2G4 ", /* 12 */
												"TEMP_COMP_N4_2G4 ", /* 13 */
												"TEMP_COMP_N3_2G4 ", /* 14 */
												"TEMP_COMP_N2_2G4 ", /* 15 */
												"TEMP_COMP_N1_2G4 ", /* 16 */
												"TEMP_COMP_N0_2G4 ", /* 17 */
												"TEMP_COMP_P1_2G4 ", /* 18 */
												"TEMP_COMP_P2_2G4 ", /* 19 */
												"TEMP_COMP_P3_2G4 ", /* 20 */
												"TEMP_COMP_P4_2G4 ", /* 21 */
												"TEMP_COMP_P5_2G4 ", /* 22 */
												"TEMP_COMP_P6_2G4 ", /* 23 */
												"TEMP_COMP_P7_2G4 ", /* 24 */
												"TEMP_COMP_N7_5G  ", /* 25 */
												"TEMP_COMP_N6_5G  ", /* 26 */
												"TEMP_COMP_N5_5G  ", /* 27 */
												"TEMP_COMP_N4_5G  ", /* 28 */
												"TEMP_COMP_N3_5G  ", /* 29 */
												"TEMP_COMP_N2_5G  ", /* 30 */
												"TEMP_COMP_N1_5G  ", /* 31 */
												"TEMP_COMP_N0_5G  ", /* 32 */
												"TEMP_COMP_P1_5G  ", /* 33 */
												"TEMP_COMP_P2_5G  ", /* 34 */
												"TEMP_COMP_P3_5G  ", /* 35 */
												"TEMP_COMP_P4_5G  ", /* 36 */
												"TEMP_COMP_P5_5G  ", /* 37 */
												"TEMP_COMP_P6_5G  ", /* 38 */
												"TEMP_COMP_P7_5G  ", /* 39 */
												"DYNAMIC_G0       "	 /* 40 */
												};

	/* Get pointer of Event Info Structure */
	prEventThermalStateInfo = (P_EXT_EVENT_THERMAL_STATE_INFO_T)Data;

	MTWF_PRINT("Total Thermo Item Num: %d\n\n", prEventThermalStateInfo->ucThermoItemsNum);

	MTWF_PRINT("==================================================================================\n");
	MTWF_PRINT("        Item            Type       LowEn       HighEn      LowerBnd       UpperBnd\n");
	MTWF_PRINT("==================================================================================\n");

	/* Thermal State Info Table */
	for (ucThermalItemIdx = 0; ucThermalItemIdx < prEventThermalStateInfo->ucThermoItemsNum; ucThermalItemIdx++) {

		MTWF_PRINT("%s     %3d        %3d         %3d          %3d            %3d\n",
															cThermalItem[prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoItem],
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoType,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgLowerEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgUpperEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cLowerBound,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cUpperBound
															);
	}
}

VOID EventPowerTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
#define POWERITEM  18

	struct _EXT_EVENT_TXPOWER_BACKOFF_T *pow = NULL;
	UINT16 modeIdx;
	UINT16 i = 0, nTx = 1, nss = 1;
	struct sku2_def cPowerItem[POWERITEM] = {
		{"CCK           ", 0, 5, 0, 5},
		{"OFDM          ", 5, 5, 10, 4},
		{"RU26          ", 14, 15, 29, 15},
		{"RU52          ", 44, 15, 59, 15},
		{"RU26+52       ", 74, 15, 89, 15},
		{"RU106         ", 104, 15, 119, 15},
		{"RU106+32      ", 134, 15, 149, 15},
		{"RU242     bw20", 164, 15, 179, 15},
		{"RU484     bw40", 194, 15, 209, 15},
		{"RU242+484     ", 224, 15, 239, 15},
		{"RU996     bw80", 254, 15, 269, 15},
		{"RU484+996     ", 284, 15, 299, 15},
		{"RU242+484+996 ", 314, 15, 329, 15},
		{"RU996x2  bw160", 344, 15, 359, 15},
		{"RU996x2+484   ", 374, 15, 389, 15},
		{"RU996x3       ", 404, 15, 419, 15},
		{"RU996x3+484   ", 434, 15, 449, 15},
		{"RU996x4  bw320", 464, 15, 479, 15},
	};

	/* Get pointer of Event Info Structure */
	pow = (struct _EXT_EVENT_TXPOWER_BACKOFF_T *)Data;



	MTWF_PRINT("Phy Rate         Band:%d, Ch_Band:%d, lpi:%d\n  BF on/off    nTx/nsts\n",
			pow->u1BandIdx, pow->u1ChBand, pAd->CommonCfg.LpiEn);
	MTWF_PRINT("=================================\n");

	for (modeIdx = 0; modeIdx < POWERITEM; modeIdx++) {
		nTx = nss = 1;
		MTWF_PRINT("\033[1;33m%s\x1b[m\n", cPowerItem[modeIdx].mode_txpower);
		MTWF_PRINT("  BF OFF: ");
		for (i = 0; i < cPowerItem[modeIdx].bfoff_len; i++) {
			MTWF_PRINT("%dT%dss ", nTx, nss);

			if (nTx % 5 == 0) {
				nTx = nss + 1;
				nss++;
			} else
				nTx++;
		}

		MTWF_PRINT("\n         ");
		for (i = cPowerItem[modeIdx].bfoff_st;
				i < (cPowerItem[modeIdx].bfoff_st + cPowerItem[modeIdx].bfoff_len); i++)
			MTWF_PRINT("%5d ", pow->rBackOffTblInfo.i1PwrLimit[i]);

		if (modeIdx == 0) {
			/* cck no BF ON case */
			MTWF_PRINT("\n");
			continue;
		}

		nTx = 1, nss = 1;
		if (modeIdx == 1)
			nTx = 2;

		MTWF_PRINT("\n  BF ON : ");
		for (i = 0; i <  cPowerItem[modeIdx].bfon_len; i++) {
			MTWF_PRINT("%dT%dss ", nTx, nss);

			if (nTx % 5 == 0) {
				nTx = nss + 1;
				nss++;
			} else
				nTx++;
		}

		MTWF_PRINT("\n         ");
		for (i = cPowerItem[modeIdx].bfon_st;
				i < (cPowerItem[modeIdx].bfon_st + cPowerItem[modeIdx].bfon_len); i++)
			MTWF_PRINT("%5d ", pow->rBackOffTblInfo.i1PwrLimit[i]);

		MTWF_PRINT("\n");
	}
}

VOID EventThermalCompTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T prEventThermalCompTableInfo = NULL;
	UINT8 ucIdx = 0;
	CHAR * cPowerItem[THERMAL_TABLE_SIZE] = {"-7_Step_Number  ",
												"-6_Step_Number  ",
												"-5_Step_Number  ",
												"-4_Step_Number  ",
												"-3_Step_Number  ",
												"-2_Step_Number  ",
												"-1_Step_Number  ",
												" 0_Step_Number  ",
												" 1_Step_Number  ",
												" 2_Step_Number  ",
												" 3_Step_Number  ",
												" 4_Step_Number  ",
												" 5_Step_Number  ",
												" 6_Step_Number  ",
												" 7_Step_Number  "
											};

	/* Get pointer of Event Info Structure */
	prEventThermalCompTableInfo = (P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T)Data;

	/* Show Thermal Compensation Table */
	MTWF_PRINT("=========================================\n");
	MTWF_PRINT("       Thermal Compensation Table\n");
	MTWF_PRINT("=========================================\n");
	MTWF_PRINT("  Band Index: %d,  Channel Band: %s\n",
		prEventThermalCompTableInfo->ucBandIdx, (prEventThermalCompTableInfo->ucBand) ?
			((prEventThermalCompTableInfo->ucBand == 1) ? ("5G") : ("6G")) : ("2G"));
	MTWF_PRINT("-----------------------------------------\n");

	for (ucIdx = 0; ucIdx < THERMAL_TABLE_SIZE; ucIdx++)
		MTWF_PRINT("%s    = 0x%x\n", cPowerItem[ucIdx], prEventThermalCompTableInfo->cThermalComp[ucIdx]);

	MTWF_PRINT("------------------------------------------\n");

}


VOID EventTxPowerCompTable(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_BACKUP_T  prEventTxPowerCompTable;
	struct _RTMP_ADAPTER *mac_ad;

	prEventTxPowerCompTable = (P_EXT_EVENT_TXPOWER_BACKUP_T)Data;

	if (prEventTxPowerCompTable->ucBandIdx != hc_get_hw_band_idx(pAd))
		mac_ad = physical_device_get_mac_adapter_by_band(
			pAd->physical_dev, prEventTxPowerCompTable->ucBandIdx);
	else
		mac_ad = pAd;

	/* update power compensation value table */
	if (mac_ad)
		os_move_mem(mac_ad->CommonCfg.cTxPowerCompBackup,
			prEventTxPowerCompTable->cTxPowerCompBackup,
			sizeof(INT8) * SKU_TABLE_SIZE * SKU_TX_SPATIAL_STREAM_NUM);
	else
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_DEBUG,
			"Event from invalid band index(=%d)\n",
			prEventTxPowerCompTable->ucBandIdx);
}

VOID ExtEvenTpcInfoHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TPC_EVENT_DOWNLINK_TABLE:
		EventTpcDownLinkTbl(pAd, Data, Length);
		break;

	case TPC_EVENT_UPLINK_TABLE:
		EventTpcUpLinkTbl(pAd, Data, Length);
		break;
	default:
		break;
	}
}

VOID EventRxvHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  u1EventCategoryID;

	/* Get Event Category ID */
	u1EventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (u1EventCategoryID) {
	case TESTMODE_RXV_EVENT_RXV_REPORT:
		EventRxvReport(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventRxFeCompHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* FE Loss data parsing */
	if (ops && ops->get_RxFELossComp_data)
		ops->get_RxFELossComp_data(pAd, (VOID *)Data);
}

#ifdef WIFI_MD_COEX_SUPPORT
VOID EventLteSafeChnHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EVENT_LTE_SAFE_CHN_T *pEventLteSafeChn = NULL;

	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO,
			"pAd:%p, Data:%p, Length:%d\n", pAd, Data, Length);

	if (!pAd->LteSafeChCtrl.bEnabled || !pAd->idcState) {
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO,
				"unsafe channel feature is disabled.\n");
		return;
	}

	if (Data && Length >= sizeof(EVENT_LTE_SAFE_CHN_T)) {
		pEventLteSafeChn = (EVENT_LTE_SAFE_CHN_T *)Data;
		if (pEventLteSafeChn->ucVersion != FW_IDC_V3_VERSION) {
			MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
				"FW version %d error, please upgrade FW!!!\n",
				pEventLteSafeChn->ucVersion);
			return;
		}

		LteSafeChnEventHandle(pAd, pEventLteSafeChn->u4SafeChannelBitmask,
							 pEventLteSafeChn->u4PwrChannelBitmask);
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO, "end.\n");
	}
}

VOID ExtEventIdcEventHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct COEX_IDC_INFO *pEventIdcInfo = NULL;

	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO,
			"pAd:%p, Data:%p, Length:%d\n", pAd, Data, Length);

	if (Data && Length >= sizeof(struct COEX_IDC_INFO)) {
		pEventIdcInfo = (struct COEX_IDC_INFO *)Data;

		Coex_IDC_Info_Handle(pAd, pEventIdcInfo);
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_INFO, "end.\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
			"Error! Null pointer or length not match!\n");
	}
}

#endif

VOID EventTpcDownLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T  prTpcDownlinkInfoTbl;
	UINT8 i;

	prTpcDownlinkInfoTbl = (P_EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T)Data;

	MTWF_PRINT("TPC DOWNLINK INFO TABLE\n\n");
	MTWF_PRINT("AP INFO\n");
	MTWF_PRINT("===============================================================================\n");
	MTWF_PRINT("		DL Tx Type			Cmd Pwr Ctrl		DL Tc Pwr\n");
	MTWF_PRINT("===============================================================================\n");
	MTWF_PRINT("		MU MIMO				%3d					%3d\n",
		prTpcDownlinkInfoTbl->fgCmdPwrCtrl[TPC_DL_TX_TYPE_MU_MIMO],
		prTpcDownlinkInfoTbl->DlTxPwr[TPC_DL_TX_TYPE_MU_MIMO]);
	MTWF_PRINT("		OFDMA				%3d					%3d\n\n",
		prTpcDownlinkInfoTbl->fgCmdPwrCtrl[TPC_DL_TX_TYPE_MU_OFDMA],
		prTpcDownlinkInfoTbl->DlTxPwr[TPC_DL_TX_TYPE_MU_OFDMA]);

	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("===============================================================================\n");
	MTWF_PRINT("		WLAN		TxPwrAlpha MU_MIMO		TxPwrAlpha OFDMA\n");
	MTWF_PRINT("===============================================================================\n");
	for (i = 0; i < 32; i++)
		MTWF_PRINT("		%3d				%3d					%3d\n",
			prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].u2WlanId,
			prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].DlTxPwrAlpha[TPC_DL_TX_TYPE_MU_MIMO],
			prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].DlTxPwrAlpha[TPC_DL_TX_TYPE_MU_OFDMA]);
}

VOID EventTpcUpLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TPC_INFO_UPLINK_TABLE_T  prTpcUplinkInfoTbl;
	UINT8 i;

	prTpcUplinkInfoTbl = (P_EXT_EVENT_TPC_INFO_UPLINK_TABLE_T)Data;

	MTWF_PRINT("TPC UPLINK INFO TABLE\n\n");
	MTWF_PRINT("AP INFO: AP TX Power = %d\n", prTpcUplinkInfoTbl->u1ApTxPwr);
	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("===============================================================================\n");
	MTWF_PRINT("		WLAN		TargetRssi		PwrHeadRoom		MinPwrFlag\n");
	MTWF_PRINT("===============================================================================\n");
	for (i = 0; i < 32; i++)
		MTWF_PRINT("		%3d			%3d			%3d			%3d\n",
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].u2WlanId,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1TargetRssi,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1PwrHeadRoom,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.fgMinPwr);
}

VOID EventRxvReport(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T  prEventRxvReport;

	prEventRxvReport = (P_EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T)Data;

	MTWF_PRINT("====================================================================================\n");
	MTWF_PRINT("RXV Report                      \n");
	MTWF_PRINT("====================================================================================\n");
	/* RXV entry content */
	MTWF_PRINT("RxvCallBackType   = %d\n", prEventRxvReport->u1RxvCbEntryType);
	MTWF_PRINT("PostMd            = %d\n", prEventRxvReport->u2PostMd);
	MTWF_PRINT("PostRssiRu        = %d\n", prEventRxvReport->u2PostRssiRu);
	MTWF_PRINT("RxCeLtfSnr        = %d\n", prEventRxvReport->u1RxCeLtfSnr);
	MTWF_PRINT("TftFoe            = %d\n", prEventRxvReport->u4TftFoe);
	MTWF_PRINT("PostNoiseFloorRx0 = %d\n", prEventRxvReport->u1PostNoiseFloorRx0);
	MTWF_PRINT("PostNoiseFloorRx1 = %d\n", prEventRxvReport->u1PostNoiseFloorRx1);
	MTWF_PRINT("PostNoiseFloorRx2 = %d\n", prEventRxvReport->u1PostNoiseFloorRx2);
	MTWF_PRINT("PostNoiseFloorRx3 = %d\n", prEventRxvReport->u1PostNoiseFloorRx3);
	MTWF_PRINT("DecUserNum        = %d\n", prEventRxvReport->u1DecUserNum);
	MTWF_PRINT("UserRate          = %d\n", prEventRxvReport->u1UserRate);
	MTWF_PRINT("UserStreamNum     = %d\n", prEventRxvReport->u1UserStreamNum);
	MTWF_PRINT("UserRuAlloc       = %d\n", prEventRxvReport->u1UserRuAlloc);
	MTWF_PRINT("MuAid             = %d\n", prEventRxvReport->u2MuAid);
	MTWF_PRINT("RxFcsErr          = %d\n", prEventRxvReport->fgRxFcsErr);
	MTWF_PRINT("OfdmRu26Snr0    = %d\n", prEventRxvReport->u4OfdmRu26Snr0);
	MTWF_PRINT("OfdmRu26Snr1    = %d\n", prEventRxvReport->u4OfdmRu26Snr1);
	MTWF_PRINT("OfdmRu26Snr2    = %d\n", prEventRxvReport->u4OfdmRu26Snr2);
	MTWF_PRINT("OfdmRu26Snr3    = %d\n", prEventRxvReport->u4OfdmRu26Snr3);
	MTWF_PRINT("OfdmRu26Snr4    = %d\n", prEventRxvReport->u4OfdmRu26Snr4);
	MTWF_PRINT("OfdmRu26Snr5    = %d\n", prEventRxvReport->u4OfdmRu26Snr5);
	MTWF_PRINT("OfdmRu26Snr6    = %d\n", prEventRxvReport->u4OfdmRu26Snr6);
	MTWF_PRINT("OfdmRu26Snr7    = %d\n", prEventRxvReport->u4OfdmRu26Snr7);
	MTWF_PRINT("OfdmRu26Snr8    = %d\n", prEventRxvReport->u4OfdmRu26Snr8);
	MTWF_PRINT("OfdmRu26Snr9    = %d\n", prEventRxvReport->u4OfdmRu26Snr9);
	MTWF_PRINT("====================================================================================\n");
}

VOID event_ecc_result(RTMP_ADAPTER *ad, UINT8 *data, UINT32 length)
{
	EVENT_ECC_RES_T *event_rcc_res = (EVENT_ECC_RES_T *)data;

	MTWF_PRINT("ucEccCmdId = %d, ucIsResFail= %d\n", event_rcc_res->ucEccCmdId, event_rcc_res->ucIsResFail);

	hex_dump_always("Dqx", event_rcc_res->aucDqxBuffer, event_rcc_res->ucDqxDataLength);
	hex_dump_always("Dqy", event_rcc_res->aucDqyBuffer, event_rcc_res->ucDqyDataLength);
}


#ifdef LED_CONTROL_SUPPORT
INT AndesLedEnhanceOP(
	RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode){

	INT32 ret = 0, i = 0;
	struct cmd_msg *msg;
	CHAR *pos, *buf;
	UCHAR *p_pattern = NULL;
	UINT32 len = 0;
	UINT32 patt_len = 0;
	UINT8 led_band_sel = band;

	struct _CMD_ATTRIBUTE attr = {0};
	led_tx_blink_pattern tx_blink_pattern;
	led_pure_blink_pattern pure_blink_pattern;
	led_mix_tx_pure_blink_pattern mix_tx_pure_blink_pattern;
	led_control_event event;

#ifdef WIFI_UNIFIED_COMMAND
	{
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (pChipCap->uni_cmd_support)
			return UniCmdLedCtrl(pAd, led_idx, tx_over_blink,
								reverse_polarity, led_band_sel, blink_mode,
								off_time, on_time, led_control_mode);
	}
#endif /* WIFI_UNIFIED_COMMAND */

	len = sizeof(led_control_event);

	/*init led control event*/
	NdisZeroMemory(&event, len);

	/*init pattern specific structure*/
	NdisZeroMemory(&tx_blink_pattern, sizeof(led_tx_blink_pattern));
	NdisZeroMemory(&pure_blink_pattern, sizeof(led_pure_blink_pattern));
	NdisZeroMemory(&mix_tx_pure_blink_pattern, sizeof(led_mix_tx_pure_blink_pattern));

	/*fill common parameters*/
	event.led_ver = 2;
	event.led_idx = led_idx;
	event.reverse_polarity = reverse_polarity;

	/*handle led_control_mode*/
	switch (led_control_mode) {
	case LED_SOLID_ON:
		event.pattern_category = LED_CATEGORY_0_SOLID_ON;
		break;
	case LED_SOLID_OFF:
		event.pattern_category = LED_CATEGORY_1_SOLID_OFF;
		break;
	case LED_TX_BLINKING:
		event.pattern_category = LED_CATEGORY_2_TX_BLINK;
		event.band_select = band;
		tx_blink_pattern.led_combine = 0;
		tx_blink_pattern.blink_mode = blink_mode;
		tx_blink_pattern.tx_blink_on_time = 70 ; /* 70 ms */
		tx_blink_pattern.tx_blink_off_time = 30; /* 30 ms */

		p_pattern = (UCHAR *)&tx_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_tx_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_500MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 500;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 500;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_250MS_ON_250MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 250;
		pure_blink_pattern.s0_off_time = 250;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 250;
		pure_blink_pattern.s1_off_time = 250;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_170MS_ON_170MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 170;
		pure_blink_pattern.s0_off_time = 170;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 170;
		pure_blink_pattern.s1_off_time = 170;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_100MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 100;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 100;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_700MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1200;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 700;
		pure_blink_pattern.s1_total_time = 1200;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 700;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	/*WPS*/
	/* Cases needs timer are handled at previous stage.
	case LED_WPS_3_BLINKING_PER_SECOND_FOR_4_SECONDS:
		break;
	case LED_WPS_5S_ON_3S_OFF_THEN_BLINKING:
		break;
	case LED_WPS_5S_ON:
		break; */
	/*Generic fix blinking format*/
	case LED_GENERAL_FIX_BLINKING_FORMAT:
		event.pattern_category = LED_CATEGORY_2_TX_BLINK;
		event.band_select = band;
		tx_blink_pattern.led_combine = 0;
		tx_blink_pattern.blink_mode = blink_mode;
		tx_blink_pattern.tx_blink_on_time = on_time*10;
		tx_blink_pattern.tx_blink_off_time = off_time*10;

		p_pattern = (UCHAR *)&tx_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_tx_blink_pattern);
		break;
	default:
		break;
	}

	len += patt_len;
	msg = AndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	os_alloc_mem(pAd, (UCHAR **)&buf, len);

	if (buf == NULL)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(buf, len);
	pos = buf;


	NdisMoveMemory(pos, &event, sizeof(led_control_event));
	if (p_pattern != NULL)
		NdisMoveMemory(pos + sizeof(led_control_event), p_pattern, patt_len);

	hex_dump("led_send_fw_cmd: ", buf, len);

	/*Debug Log*/
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\nled_send_fw_cmd:\n");
	i = 0;
	while (i < sizeof(led_control_event)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "%02x", buf[i]);
		i++;
	}

	if (p_pattern != NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\npattern content:\n");
		i = 0;
		while (i < patt_len) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "%02x", p_pattern[i]);
			i++;
		}
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\n");
	} /*End of Debug Log*/

	AndesAppendCmdMsg(msg, (char *)buf, len);
	ret = AndesSendCmdMsg(pAd, msg);
	os_free_mem(buf);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"(ret = %d)\n", ret);
	return ret;

}

INT AndesLedGpioMap(RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type)
{
	INT32 ret = 0, i = 0;
	struct cmd_msg *msg;
	CHAR *buf;
	UINT32 len;

	struct _CMD_ATTRIBUTE attr = {0};

	/*init led control event*/
	led_control_event event;

	len = sizeof(led_control_event);
	NdisZeroMemory(&event, len);

	/*fill common parameters*/
	event.led_ver = 2;
	event.led_idx = led_index;
	event.pattern_category = LED_CATEGORY_5_GPIO_SETTING;
	event.rsvd_1 |= ctr_type;
	event.rsvd_1 |= map_index << 1;

	msg = AndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	os_alloc_mem(pAd, (UCHAR **)&buf, len);

	if (!buf) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	NdisZeroMemory(buf, len);
	NdisMoveMemory(buf, &event, sizeof(led_control_event));

	hex_dump("led_send_fw_cmd: ", buf, len);

	/*Debug Log*/
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "\nled_send_fw_cmd:\n");
	i = 0;
	while (i < sizeof(led_control_event)) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "%02x", buf[i]);
		i++;
	}
	/*End of Debug Log*/

	AndesAppendCmdMsg(msg, (char *)buf, len);
	AndesSendCmdMsg(pAd, msg);
	os_free_mem(buf);

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "\nled_%d map to gpio_%d...\n", led_index, map_index);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

#endif

#ifdef CFG_SUPPORT_CSI
ULONG NBytesAlign(ULONG len, ULONG nBytesAlign)
{
	if (nBytesAlign > 0xFFFF)
		return 0xFFFFFFFF;

	if (len%nBytesAlign == 0)
		return len;

	len += nBytesAlign - len%nBytesAlign;

	return len;
}

INT AndesCSICtrl(RTMP_ADAPTER *pAd, struct CMD_CSI_CONTROL_T *prCSICtrl)
{
	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};

	/*allocate mem*/
	msg = AndesAllocCmdMsg(pAd, sizeof(struct CMD_CSI_CONTROL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/*set attr*/
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CSI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	/*Debug Log*/
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
	"CSI send fw cmd:%d,%d,%d,%d,%d\n",
	prCSICtrl->BandIdx, prCSICtrl->ucMode, prCSICtrl->ucCfgItem, prCSICtrl->ucValue1, prCSICtrl->ucValue2);

	AndesAppendCmdMsg(msg, (char *)prCSICtrl, sizeof(struct CMD_CSI_CONTROL_T));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_NOTICE, "(ret = %d)\n", ret);
	return ret;
}

VOID ExtEventCSICtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT_16 *pru2Tmp = NULL;
	UINT_32 *p32tmp = NULL;
	INT_16 i2Idx = 0;
	UINT_32 rx_info = 0;
	UINT32 ret = 0;
	struct CSI_MAX_DATA_T *currnet_data = NULL;
	struct CSI_DATA_T *final_data = NULL;
	TLV_ELEMENT_T *prCSITlvData = NULL;
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	UINT8 *prBuf = Data;
	/* u2Offset is 8 bytes currently, tag 4 bytes + length 4 bytes */
	UINT_16 u2Offset = Offsetof(TLV_ELEMENT_T, aucbody);

	/*hex_dump_with_lvl("CSIEventDump:\n ", (UCHAR *)prBuf, (UINT)Length, DBG_LVL_ERROR);*/

	os_alloc_mem(NULL, (UCHAR **)&currnet_data, sizeof(struct CSI_MAX_DATA_T));

	if (!currnet_data) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"allocate memory for prCSIData failed!\n");
		return;
	}

	os_zero_mem(currnet_data, sizeof(struct CSI_DATA_T));

	while (Length >= u2Offset) {
		prCSITlvData = (struct TLV_ELEMENT *)prBuf;
		prCSITlvData->tag_type = le2cpu32(prCSITlvData->tag_type);
		prCSITlvData->body_len = le2cpu32(prCSITlvData->body_len);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_DEBUG,
			"tag_type=%d,body_len=%d,length=%d\n",
			prCSITlvData->tag_type, prCSITlvData->body_len, Length);

		switch (prCSITlvData->tag_type) {
		case CSI_EVENT_FW_VER:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid FW VER len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->FWVer = (UINT8)le2cpu32(*((UINT32 *)prCSITlvData->aucbody));
			prCSIInfo->FWVer = currnet_data->FWVer;
			break;
		case CSI_EVENT_CBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CBW len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucBw = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RSSI:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSSI len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->cRssi = (UINT8)le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_SNR:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid SNR len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucSNR = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_BAND:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid BAND len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucDbdcIdx = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			if ((currnet_data->ucDbdcIdx != BAND0) &&
				(currnet_data->ucDbdcIdx != BAND1) &&
				(currnet_data->ucDbdcIdx != BAND2)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid BAND IDX (%d)\n", currnet_data->ucDbdcIdx);
				goto out;
			}
			break;
		case CSI_EVENT_CSI_NUM:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI num len %u", prCSITlvData->body_len);
				goto out;
			}

			currnet_data->u2DataCount = (UINT16)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));

			/*at most BW80 per event,surpass BW80, data will be divided*/
			if (currnet_data->u2DataCount > CSI_BW80_TONE_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI count %u\n", currnet_data->u2DataCount);
				goto out;
			}

			break;
		case CSI_EVENT_CSI_I_DATA:

			if (prCSITlvData->body_len != sizeof(INT_16) * currnet_data->u2DataCount) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI I data len %u, csinum %u\n",
					prCSITlvData->body_len, currnet_data->u2DataCount);
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < currnet_data->u2DataCount; i2Idx++)
				currnet_data->ac2IData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_CSI_Q_DATA:

			if (prCSITlvData->body_len != sizeof(INT_16) * currnet_data->u2DataCount) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI Q data len %u, csinum %u\n",
					prCSITlvData->body_len, currnet_data->u2DataCount);
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < currnet_data->u2DataCount; i2Idx++)
				currnet_data->ac2QData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_DBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid DBW len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucDataBw = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_CH_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CH IDX len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucPrimaryChIdx = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TA:
			/*
			 * TA length is 8-byte long (MAC addr 6 bytes +
			 * 2 bytes padding), the 2-byte padding keeps
			 * the next Tag at a 4-byte aligned address.
			 */
			if (prCSITlvData->body_len != NBytesAlign(sizeof(currnet_data->aucTA), 4)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TA len %u", prCSITlvData->body_len);
				goto out;
			}
			os_move_mem(currnet_data->aucTA, prCSITlvData->aucbody, sizeof(currnet_data->aucTA));
			break;
		case CSI_EVENT_EXTRA_INFO:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Error len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->u4ExtraInfo = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RX_MODE:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Rx Mode len %u", prCSITlvData->body_len);
				goto out;
			}
			rx_info = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			currnet_data->ucRxMode = GET_CSI_RX_MODE(rx_info);
			currnet_data->rx_rate = GET_CSI_RATE(rx_info);
			break;
		case CSI_EVENT_RSVD1:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD1_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD1 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd1Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < currnet_data->ucRsvd1Cnt; i2Idx++)
				currnet_data->ai4Rsvd1[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD2:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD2_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD2 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd2Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < currnet_data->ucRsvd2Cnt; i2Idx++)
				currnet_data->au4Rsvd2[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD3:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD3 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->i4Rsvd3 = le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RSVD4:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD4 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd4 = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_H_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid chain_info len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->chain_info = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TX_RX_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TX_RX_IDX len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->Tx_Rx_Idx = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TS:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TS len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->u4TimeStamp = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_PKT_SN:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Segment Number len %u",
					prCSITlvData->body_len);
				goto out;
			}

			currnet_data->pkt_sn = (UINT_16)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_BW_SEG:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Segment Number len %u",
					prCSITlvData->body_len);
				goto out;
			}

			currnet_data->u4SegmentNum = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_REMAIN_LAST:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Remain Last len %u",
					prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRemainLast = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TR_STREAM:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TR stream number len %u",
					prCSITlvData->body_len);
				goto out;
			}
			currnet_data->tr_stream = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
				"Unsupported CSI tag %d\n", prCSITlvData->tag_type);
		};

			Length -= (u2Offset + prCSITlvData->body_len);

			if (Length >= u2Offset)
				prBuf += (u2Offset + prCSITlvData->body_len);
	}

	/*if protocal filter is open, filter pkt*/
	if (prCSIInfo->protocol_filter &&
		!(prCSIInfo->protocol_filter & currnet_data->ucRxMode))
		goto out;

	ret = wlanCheckCSISegmentData(pAd, currnet_data);

	/*event data error or event drop*/
	if (ret == CSI_CHAIN_ERR || ret == CSI_CHAIN_SEGMENT_ERR) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"CSI chain check error! Segment number=%d, Remain last=%d\n",
			currnet_data->u4SegmentNum, currnet_data->ucRemainLast);
		goto out;
	}

	if (ret == CSI_CHAIN_SEGMENT_FIRST || ret == CSI_CHAIN_SEGMENT_MIDDLE)
		goto out;
	else if (ret == CSI_CHAIN_COMPLETE)
		final_data = prepare_csi_data_to_push(pAd, currnet_data);
	else if (ret == CSI_CHAIN_SEGMENT_LAST) /*if all segments have been combined*/
		final_data = prepare_csi_data_to_push(pAd, &prCSIInfo->rCSISegmentTemp);

	if (!final_data)
		goto out;

	if (prCSIInfo->usr_offset)
		csi_timestamp_filter(pAd, final_data);	/*csi sw timestamp filter*/
	else
		wlanPushCSIData(pAd, final_data);

	if (prCSIInfo->CSI_report_mode == CSI_PROC)
		wake_up_interruptible(&(pAd->rCSIInfo.waitq));

out:
	if (currnet_data)
		os_free_mem(currnet_data);

	if (final_data)
		os_free_mem(final_data);

	return;
}
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID event_twt_resume_info(RTMP_ADAPTER *ad, UINT8 *data, UINT32 length)
{
	EVENT_TWT_RESUME_INFO_T *event = (EVENT_TWT_RESUME_INFO_T *)data;
	struct wifi_dev *wdev = NULL;
	struct twt_resume_info resume_info = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = event->wcid;

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return;

	entry = entry_get(ad, wcid);
	if (!entry)
		return;
	wdev = entry->wdev;
	resume_info.bssinfo_idx = event->bssinfo_idx;
	resume_info.wcid = wcid;
	resume_info.flow_id = event->flow_id;
	resume_info.idle = event->idle;

	twt_get_resume_event(wdev, &resume_info);
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
