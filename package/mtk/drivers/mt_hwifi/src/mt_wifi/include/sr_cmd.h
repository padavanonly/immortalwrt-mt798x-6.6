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
	sr_cmd.h
*/

#ifndef __CMM_SR_CMD_H__
#define __CMM_SR_CMD_H__

#ifdef CFG_SUPPORT_FALCON_SR
/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "rt_config.h"
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    TYPES
 ******************************************************************************/

/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS SrInit(RTMP_ADAPTER *pAd);
NDIS_STATUS SrRstNav(IN RTMP_ADAPTER * pAd, IN struct _RX_BLK * pRxBlk);
NDIS_STATUS SrDisSrBfrConnected(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable);
NDIS_STATUS SrSwitchToApCliMode(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable);
NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrMbssInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
NDIS_STATUS SrMeshGetSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pMeshSrgBitmap);
NDIS_STATUS SrMeshGetSrMode(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pu1SrMode);
NDIS_STATUS SrMeshSrgBitMapControl(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 pMeshSrgBitmap);
VOID SrMeshSelfSrgBMChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, IN BOOLEAN fgPBssidUpd);
NDIS_STATUS SrMeshTopologyUpdate(IN PRTMP_ADAPTER pAd, IN PUINT_8 pTopologyUpdate, IN UINT8 Band_Idx);
VOID SrMeshTopologyUpdatePerBand(struct _RTMP_ADAPTER *pAd, UINT_8 u1DbdcIdx);
VOID SrMeshTopologyUpdatePeriodic(struct _RTMP_ADAPTER *pAd);
VOID SrMeshTopologyUpdateBcnRssi(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list);
NDIS_STATUS SrFindBHWcid(PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, PUINT_8 pBHMac);
NDIS_STATUS SrSetUplinkTrafficStatus(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 UlStatus);
NDIS_STATUS SrSetMapBalance(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value);
NDIS_STATUS SrCmdSetMeshUlMode(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value);
NDIS_STATUS SrSetSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx);
NDIS_STATUS SrMeshStaModeRptLockConfig(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 u1RptLock);
NDIS_STATUS SrSetMeshRemoteStaModeRpt(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 RemoteAPStaAllHe);
VOID SrMeshSrUpdateSTAMode(IN PRTMP_ADAPTER pAd, BOOL Assoc, UINT8 CurrStaIsHe);
NDIS_STATUS SrMeshSrEnable(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN UINT_8 SrEnable);
NDIS_STATUS SrBHMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx,
								IN PUINT_8 prSrBhMeshSrgBitmap);

/* For RTMP_PRIVATE_SUPPORT_PROC ap_cfg.c */
/** SET **/
/** SR_CMD_SET_SR_CAP_SREN_CTRL **/
NDIS_STATUS SetSrCapSrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CAP_ALL_CTRL **/
INT SetSrCapAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_PARA_ALL_CTRL	**/
INT SetSrParaAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT SetSrParaPdThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL **/
INT SetSrDropTa(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_GLO_VAR_STA_CTRL **/
INT SetSrSta(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL **/
INT SetSrStaInit(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_COND_ALL_CTRL **/
INT SetSrCondAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL **/
INT SetSrRcpiTblAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL **/
INT SetSrRcpiTblOfstAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_Q_CTRL_ALL_CTRL **/
INT SetSrQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_IBPD_ALL_CTRL **/
INT SetSrIBPDAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_ALL_CTRL **/
INT SetSrNRTAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_RESET_CTRL **/
INT SetSrNRTResetAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL **/
INT SetSrNRTCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL **/
INT SetSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_FRM_FILT_ALL_CTRL **/
INT SetSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL **/
INT SetSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_ESR_ENABLE **/
NDIS_STATUS SetSrCfgESrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_BF **/
NDIS_STATUS SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ATF **/
NDIS_STATUS SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_MODE **/
NDIS_STATUS SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS SetSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_TXC_QID **/
NDIS_STATUS SetSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_TXC_PATH **/
NDIS_STATUS SetSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_AC_METHOD **/
NDIS_STATUS SetSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS SetSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS SetSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS SetSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS SetSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_PROFILE **/
NDIS_STATUS SetSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_FNQ_ENABLE **/
NDIS_STATUS SetSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ENABLE **/
NDIS_STATUS SetSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE **/
NDIS_STATUS SetSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE **/
NDIS_STATUS SetSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
NDIS_STATUS SetSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS SetSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_RATE_OFFSET **/
NDIS_STATUS SetSrCfgSrRateOffset(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrMeshSDFlag(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_SRG_BITMAP **/
INT SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SIGA_FLAG_CTRL **/
INT SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL **/
INT SetSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT SetSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT SetSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_REMOTE_FH_RSSI **/
INT SetSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT SetSrMeshStaThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD UNI_CMD_SR_BH_MESH_SR_BITMAP**/
INT SetSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD UNI_CMD_SR_FH_MESH_SR_BITMAP**/
INT SetSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_SET_FORBIT_MESH_SR **/
INT SetSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_FORHIB_MESH_SR_RESET **/
INT SetSrMeshResetBhDLForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

/** SR_CMD_SET_REMOTE_BH_INFO **/
INT SetSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_BALANCE **/
INT SetSrMeshMapBalance(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_TOPO **/
INT SetSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_TRAFFIC_STATUS **/
INT SetSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
NDIS_STATUS SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SetMeshSRsd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_MESH_UL_MODE **/
INT SetSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS SrMeshApcliDetect(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS SetMeshMac(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** Driver Internal **/
INT SetSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT SetSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);


/** GET **/
/** SR_CMD_GET_SR_CAP_ALL_INFO **/
INT ShowSrCap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CAP_ESR_EN **/
INT ShowSrCapEsrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CAP_NON_SRG_EN **/
INT ShowSrCapNonSrgEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_APCLI_MODE **/
INT ShowSrApCliMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_TX_COUNT **/
INT ShowSrTxCount(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_PARA_ALL_INFO **/
INT ShowSrPara(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_IND_ALL_INFO **/
INT ShowSrInd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO **/
INT ShowSrInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_COND_ALL_INFO **/
INT ShowSrCond(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_RCPI_TBL_ALL_INFO **/
INT ShowSrRcpiTbl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO **/
INT ShowSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_Q_CTRL_ALL_INFO **/
INT ShowSrQCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_IBPD_ALL_INFO **/
INT ShowSrIBPD(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_NRT_ALL_INFO **/
INT ShowSrNRT(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_NRT_CTRL_ALL_INFO **/
INT ShowSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO **/
INT ShowSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_FRM_FILT_ALL_INFO **/
INT ShowSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO **/
INT ShowSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO **/
INT ShowSrInterPsDbgAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
INT ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
INT ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_BF **/
INT ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ATF **/
INT ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_MODE **/
INT ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
INT ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
INT ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_TXC_QUEUE **/
INT ShowSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_TXC_QID **/
INT ShowSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_TXC_PATH **/
INT ShowSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_AC_METHOD **/
INT ShowSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_PERIOD_THR **/
INT ShowSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD **/
INT ShowSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO **/
INT ShowSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO **/
INT ShowSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_PROFILE **/
INT ShowSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_FNQ_ENABLE **/
INT ShowSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
INT ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ENABLE **/
INT ShowSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE **/
INT ShowSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE **/
INT ShowSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
INT ShowSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_DABS_MODE **/
INT ShowSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CNT_ALL **/
INT ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SD_ALL **/
INT ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SRG_BITMAP **/
INT ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_MESH_SRG_BITMAP **/
INT ShowSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SIGA_FLAG_INFO **/
INT ShowSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO **/
INT ShowSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** UNI_CMD_SR_DOWNLINK_STA_THRESHOLD **/
INT ShowSrMeshstaTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_BH_MESH_SR_BITMAP **/
INT ShowSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_FH_MESH_SR_BITMAP **/
INT ShowSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** UNI_CMD_SR_SET_FORBIT_MESH_SR **/
INT ShowSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

/** SR_CMD_GET_REMOTE_BH_INFO **/
INT ShowSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MAP_TOPO **/
INT ShowSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshTopoUpdateParams(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MAP_TRAFFIC_STATUS **/
INT ShowSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MESH_PHASE **/
INT ShowSrMeshPhase(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** Driver Internal **/
INT ShowSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT ShowSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SUPPORT_MODE **/
INT ShowSrCfgSrSupportMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
INT SetCsrEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrCoordAPInfoRst(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrCoordAPUpdateMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrCoordAPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrFixNofiyPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrFixRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrCoordApUpdMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrCoordApInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrEventCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrFixNofiyPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrFixRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrStaUpdateMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCsrStaInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrStaUpdMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCsrStaInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSrRateOffset(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

/* Event Handler andes_mt.c */
VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length);

/* Global Variable */
#define SR_STA_NUM           32
#define SR_DROP_TA_NUM       16
#define SR_RCPITBL_MCS_NUM   14
#define SR_NRT_ROW_NUM       16
#define SR_BSSID_NUM         20
#define SR_SIGA_FLAG_RANGE   16
#define WH_SR_RCPITBL_MCS_NUM               14
#define RAM_BAND_NUM                    2


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
/** FW & DRV sync with wh_sr.h **/

/*Follow HAL ENUM_WH_RESP_TXPWR_MODE_T */
enum ENUM_WH_SR_RESP_TXPWR_MODE {
	WH_ENUM_SR_RESP_TXPWR_NO_RESTRIC = 0,
	WH_ENUM_SR_RESP_TXPWR_RESTRIC,
	WH_ENUM_SR_RESP_TXPWR_MODE_NUM
};

/*Follow HAL ENUM_SR_TXPWR_RESTRIC_MODE_T */
enum ENUM_WH_SR_TXPWR_RESTRIC_MODE {
	WH_ENUM_SR_TXPWR_RESTRIC_NO_RESTRIC = 0,
	WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_SPEC,
	WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_MTK,
	WH_ENUM_SR_TXPWR_RESTRIC_MODE_NUM
};

struct WH_SR_CAP_T_SR_V1 {
	/** RMAC */
	BOOLEAN fgSrEn;
	BOOLEAN fgSrgEn;
	BOOLEAN fgNonSrgEn;
	BOOLEAN fgSingleMdpuRtsctsEn;
	BOOLEAN fgHdrDurEn;
	BOOLEAN fgTxopDurEn;
	BOOLEAN fgNonSrgInterPpduPresv;
	BOOLEAN fgSrgInterPpduPresv;
	/** AGG */
	BOOLEAN fgSrRemTimeEn;
	BOOLEAN fgProtInSrWinDis;
	BOOLEAN fgTxCmdDlRateSelEn;
	/** MIB */
	BOOLEAN fgAmpduTxCntEn;
};

struct WH_SR_CAP_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	BOOLEAN fgSrEn;
	BOOLEAN fgSrgEn;
	BOOLEAN fgNonSrgEn;
	BOOLEAN fgSingleMdpuRtsctsEn;
	/* DW_2 */
	BOOLEAN fgHdrDurEn;
	BOOLEAN fgTxopDurEn;
	BOOLEAN fgNonSrgInterPpduPresv;
	BOOLEAN fgSrgInterPpduPresv;
	/* DW_3 */
	BOOLEAN fgSMpduNoTrigEn;
	BOOLEAN fgSrgBssidOrder;
	BOOLEAN fgCtsAfterRts;
	BOOLEAN fgSrpOldRxvEn;
	/* DW_4 */
	BOOLEAN fgSrpNewRxvEn;
	BOOLEAN fgSrpDataOnlyEn;
	/* AGG  */
	BOOLEAN fgFixedRateSrREn;
	BOOLEAN fgWtblSrREn;
	/* DW_5 */
	BOOLEAN fgSrRemTimeEn;
	BOOLEAN fgProtInSrWinDis;
	BOOLEAN fgTxCmdDlRateSelEn;
	/* MIB  */
	BOOLEAN fgAmpduTxCntEn;
};
/* SR Parameter */
struct WH_SR_PARA_T {
	/* RMAC */
	/* DW_1 */
	UINT_8 u1NonSrgPdThr;
	UINT_8 u1SrgPdThr;
	UINT_8 u1PeriodOfst;
	UINT_8 u1RcpiSourceSel;
	/* TMAC */
	/* DW_2 */
	UINT_16 u2ObssPdMin;
	UINT_16 u2ObssPdMinSrg;
	/* DW_3 */
	enum ENUM_WH_SR_RESP_TXPWR_MODE eRespTxPwrMode;
	/* DW_4 */
	enum ENUM_WH_SR_TXPWR_RESTRIC_MODE eTxPwrRestricMode;
	/* DW_5 */
	UINT_8 u1ObssTxPwrRef;
	UINT_8 RSV[3];
};

/* SR Indicator */
struct WH_SR_IND_T {
	/* RMAC */
	/* DW_1 */
	UINT_16 u2NonSrgVldCnt;
	UINT_16 u2SrgVldCnt;
	/* DW_2 */
	UINT_16 u2IntraBssPpduCnt;
	UINT_16 u2InterBssPpduCnt;
	/* DW_3 */
	UINT_16 u2NonSrgPpduVldCnt;
	UINT_16 u2SrgPpduVldCnt;
	/* MIB */
	/* DW_4 */
	UINT_32 u4SrAmpduMpduCnt;
	/* DW_5 */
	UINT_32 u4SrAmpduMpduAckedCnt;
};

/* SR Condition */
struct SR_COND_T_SR_V1 {
	BOOLEAN fgSrRcpiCckRateEn;
	BOOLEAN fgSrMacRcpiRateEn;
	BOOLEAN fgSrRxvRcpiRateEn;
	BOOLEAN fgSrRcpiHeRateEn;
	BOOLEAN fgSrRcpiVhtRateEn;
	BOOLEAN fgSrRcpiHtRateEn;
	BOOLEAN fgSrRcpiLgRateEn;
	BOOLEAN fgSrRxvEntry;
	BOOLEAN fgSrPeriodLimitEn;
	UINT_8  u1SrPeriodLimit;
	/** Reserve for 4-byte aligned*/
	UINT_8  u1Reserved[2];
};

struct WH_SR_COND_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	BOOLEAN fgSrRcpiSel;
	BOOLEAN fgSrRcpiCckRateEn;
	BOOLEAN fgSrMacRcpiRateEn;
	BOOLEAN fgSrRxvRcpiRateEn;
	/* DW_2 */
	BOOLEAN fgSrRcpiHeRateEn;
	BOOLEAN fgSrRcpiVhtRateEn;
	BOOLEAN fgSrRcpiHtRateEn;
	BOOLEAN fgSrRcpiLgRateEn;
	/* DW_3*/
	BOOLEAN fgSrRcpiEhtRateEn;
	BOOLEAN fgSrRxvEntry;
	BOOLEAN fgSrPeriodLimitEn;
	UINT_8  u1SrPeriodLimit;
};

/* SR Queue Control */

struct WH_SR_QUEUE_CTRL_T_SR_V1 {
	/** RMAC */
	BOOLEAN fgSrRxRptEn;
	/** ARB */
	BOOLEAN fgSrCw;
	BOOLEAN fgSrSuspend;
	/** Reserve for 4-byte aligned*/
	UINT_8  u1Reserved;
	/** ARB */
	UINT_32 u4SrBackOffMask;
};

struct WH_SR_QUEUE_CTRL_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	BOOLEAN fgSrRxRptEn;
	/* ARB */
	BOOLEAN fgSrCw;
	BOOLEAN fgSrSuspend;
	BOOLEAN fgSrDisSwAifsDis;
	/* DW_2 */
	UINT_32 u4SrBackOffMask;
	/* DW_3 */
	UINT_32 u4SrBackOffEnable;
};

/* SR RCPI Table */
struct WH_SR_RCPITBL_T {
	/* RMAC */
	/* DW_1 - DW_3 */
	UINT_8  u1RcpiTblMcs[WH_SR_RCPITBL_MCS_NUM];
};

/* SR RCPI Table Offset */
struct SR_RCPITBL_OFST_T_SR_V1 {
	UINT_16 u2RxBwRcpiOfst;
	UINT_16 u2StbcRcpiOfst;
	UINT_16 u2NumAntRcpiOfst;
	UINT_16 u2LdpcRcpiOfst;
	UINT_16 u2DcmRcpiOfst;
	UINT_16 u2MacRcpiOfst;
	UINT_16 u2SigRcpiOfst;
	/** Reserve for 4-byte aligned*/
	UINT_8  u1Reserved[6];
};

struct WH_SR_RCPITBL_OFST_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	UINT_16 u2RxBwRcpiOfst;
	UINT_16 u2StbcRcpiOfst;
	/* DW_2 */
	UINT_16 u2LdpcRcpiOfst;
	UINT_16 u2DcmRcpiOfst;
	/* DW_3 */
	UINT_16 u2MacRcpiOfst;
	UINT_16 u2SigRcpiOfst;
	/* DW_4 */
	UINT_16 u2BfRcpiOfst;
	UINT_16 u2RSV;
	/* DW_5 */
	UINT_32 u4NumAntRcpiOfst;
};

/* SR Inter Bss Ppdu Determination */
struct WH_SR_IBPD_T {
	/* RMAC */
	/* DW_1 */
	UINT_8 u1InterBssByHdrBssid;
	UINT_8 u1InterBssByMu;
	UINT_8 u1InterBssByPbssColor;
	UINT_8 u1InterBssByPaid;
	/* DW_2 */
	UINT_8 u1InterBssByBssColor;
	UINT_8 RSV[3];
};

/* SR Neighbor Rssi Table */

/** SRNRT*/
struct SR_NRT_T_SR_V1 {
	/** RMAC */
	UINT_8 u1TableIdx;
	UINT_32 u4NRTValue;
	/** Reserve for 4-byte aligned*/
	UINT_8 u1Reserved[3];
};

struct WH_SR_NRT_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	UINT_8 u1TableIdx;
	UINT_8 u1RaTaSel;
	BOOLEAN fgSwProtect;
	UINT_8 RSV[1];
	/* DW_2 */
	UINT_32 u4NRTValue;
};

/* SR Neighbor Rssi Table Control */

/** SRNRTCtrl*/
struct SR_NRT_CTRL_T_SR_V1 {
	/** RMAC */
	BOOLEAN fgSrtEn;
	BOOLEAN fgSrtSrpEn;
	BOOLEAN fgSrtAddrOrderEn;
	/** Reserve for 4-byte aligned*/
	UINT_8 u1Reserved[1];
	UINT_16 u2SrtInRcpiTh;
	UINT_16 u2SrtOutRcpiTh;
	UINT_16 u2SrtUsedCntTh;
	/** Reserve for 4-byte aligned*/
	UINT_8 u1Reserved2[2];
};

struct WH_SR_NRT_CTRL_T_SR_V2 {
	/* RMAC */
	/* DW_1 */
	BOOLEAN fgSrtEn;
	BOOLEAN fgSrtSrpEn;
	BOOLEAN fgSrtAddrOrderEn;
	BOOLEAN fgSrtByPassCtsAck;
	/* DW_2 */
	UINT_16 u2SrtInRcpiTh;
	UINT_16 u2SrtOutRcpiTh;
	/* DW_3 */
	UINT_16 u2SrtUsedCntTh;
	UINT_8 RSV[2];
};

/* SR Freeze Normal Queue Control */
struct WH_SR_FNQ_CTRL_T {
	/* RMAC */
	/* DW_1 */
	UINT_16 u2SrpCondDis;
	UINT_8  u1PeriodOfst;
	BOOLEAN fgHdrDurEn;
	/* DW_2 */
	BOOLEAN fgTxopDurEn;
	BOOLEAN fgSrpCfendRst;
	BOOLEAN fgSrpNavToutRst;
	UINT_8  RSV[1];
};

/* SR Inter Power Saving Control */
struct WH_SR_INTERPS_CTRL_T {
	/* RMAC */
	/* DW_1 */
	UINT_8  u1CondDis;
	UINT_8  u1DurAdj;
	UINT_8  u1DurLmt;
	UINT_8  u1EntryEn;
	/* DW_2 */
	BOOLEAN fgDurLmtEn;
	BOOLEAN fgInterpsEn;
	UINT_8  RSV[2];
};

/* SR Inter Power Saving Debug */
struct WH_SR_INTERPS_DBG_T {
	/* RMAC */
	/* DW_1 */
	UINT_8  u1Entry0Cnt;
	UINT_8  u1Entry1Cnt;
	UINT_8  u1Entry2Cnt;
	UINT_8  u1EntryLat;
};


/** End FW & DRV sync with wh_sr.h **/

/** FW & DRV sync with sr_cmm.h **/
enum ENUM_SR_RXRPT_SRC_T {
	ENUM_SR_RXRPT_SRC_RXRPT = 0,
	ENUM_SR_RXRPT_SRC_CMDRPT_TX,
	ENUM_SR_RXRPT_SRC_NUM
};
/** END FW & DRV sync with sr_cmm.h **/

/** FW & DRV sync with sr_if.h **/
enum ENUM_SR_STA_MODE_T {
	ENUM_SR_STA_MODE_AUTO = 0x0,
	ENUM_SR_STA_MODE_FIXED,
	ENUM_SR_STA_MODE_NUM
};

struct SR_STA_INFO_T {
	UINT_16 u2WlanId;
	UINT_8 u1Mode; /* ENUM_SR_STA_MODE_T */
	UINT_8 u1State; /* ENUM_SR_STA_STATE_T */
	UINT_8 u1SrRateOffset;
	UINT_8 u1SrRaTryCnt;
	UINT_8 u1SrRaRound;
	UINT_8 u1SrRaState;
	UINT_16 u2SrSucCnt;
	UINT_16 u2SrTtlTxCnt;
	UINT_32 u4Score;
	UINT_16 u2BadQuota;
	UINT_8 u1BadLevel;
	UINT_8 u1SrRate;
	UINT_16 u2AvgMPDULength[AC_NUM];
};

struct SR_DROP_TA_INFO_T {
	UINT_32 u4Address2;
	struct SR_STA_INFO_T rSrStaInfo[SR_STA_NUM];
};

/* SR CNT */
enum _ENUM_SR_ENTRY_T {
	ENUM_SR_ENTRY_NEWRXV = 0,
	ENUM_SR_ENTRY_ORIRXVVHT,
	ENUM_SR_ENTRY_ORIRXVHE,
	ENUM_SR_ENTRY_NEWMAC,
	ENUM_SR_ENTRY_ORIMAC,
	ENUM_SR_ENTRY_NUM
};

enum _ENUM_SR_NONSRG_PPDU_TYPE_T {
	ENUM_SR_NONSRG_OBSS_OR_NONHE = 0,
	ENUM_SR_NONSRG_MESH_HE_INTERNAL_NO_SR,
	ENUM_SR_NONSRG_MESH_HE,
	ENUM_SR_NONSRG_PPDU_TYPE_NUM
};

struct SR_CNT_ENTRY_T {
	UINT_16 u2EntryTtl;
	UINT_16 u2PeriodSuc;/* u2PeriodFail = u2EntryTtl  - u2PeriodSuc */
	UINT_16 u2GenTxcSuc;/* u2GenTxcFail = u2PeriodSuc - u2GenTxcSuc */
	UINT_16 u2SrTxSuc;  /* u2SrTxFail   = u2GenTxcSuc - u2SrTxSuc   */
};

struct SR_CNT_T {
	UINT_32 u4Rsv;
	UINT_16 u2EntryNoSrTtl;
	UINT_16 u2EntryFailTtl;
	struct SR_CNT_ENTRY_T rSrCntEntry[ENUM_SR_RXRPT_SRC_NUM][ENUM_SR_ENTRY_NUM][ENUM_SR_NONSRG_PPDU_TYPE_NUM];
};
/* END SR CNT */

/* SR SD (Scene Detection) */
struct SR_SD_T {
	UINT_16 u2RxrptRxvCnt;
	UINT_16 u2RxrptMacCnt;
	UINT_32 u4DeltaTime;
	UINT_32 u4HWSrPeriodRatio;
	UINT_32 u4SWSrPeriodRatio;
	UINT_32 u4SrTxAirtime;
	UINT_32 u4OBSSAirtime;
	UINT_32 u4MyTxAirtime;
	UINT_32 u4MyRxAirtime;
	UINT_32 u4ChannelBusyTime;
	UINT_32 u4TtlAirTime;
	UINT_32 u4TtlAirTimeRatio;
	UINT_32 u4OBSSAirTimeRatio;
	UINT_8  u1Rule;
	/* SRSD - OBSS Monitor */
	UINT_8  u1ModeMcsIdx;
	UINT_8  u1SrTxState;
	UINT_8  u1LowTrafficCnt;
	UINT_8  u1ContWeakChkPnt;
	UINT_8  u1ObssLongPktPnt;
	UINT_16 u2ObssLongPkt[3];
	UINT_16 u2MeshDropPkt;
	UINT_16 u2RxrptMcs[SR_RCPITBL_MCS_NUM];
	UINT_32 u4TxByteSum;
	UINT_32 u4TxdPgCnt;
	UINT_32 u4SrTxCnt;
	UINT_32 u4TxCnt;
	/*End - SRSD - OBSS Monitor*/
};
/* END SR SD (Scene Detection) */

/* SR SRG BITMAP */
struct SR_SRG_BITMAP_T {
	UINT_32 u4Color_31_0;
	UINT_32 u4Color_63_32;
	UINT_32 u4pBssid_31_0;
	UINT_32 u4pBssid_63_32;
};
/* END SR SRG BITMAP */

/** SR SIGA FLAG*/
struct SR_SIGA_FLAG_T {
	UINT_8 u1Bssid;
	UINT_8 u1reserved[3];
	UINT_8 u1SigaFlag[SR_BSSID_NUM];
};

/* END SR SIGA FLAG */
struct SR_MULTI_AP_MESH_SRG_BITMAP_T {
	struct SR_SRG_BITMAP_T rSrBHMeshSrgBitmap;
	struct SR_SRG_BITMAP_T rSrFHMeshSrgBitmap;
};

struct SR_MESH_SR_DL_STA_THRESHOLD {
	UINT_8 u1BSSID;
	INT_8 irssi;
	UINT_8 au1Reserved[2];
};

struct _SR_SIGA_AUTO_FLAG_T {
	UINT_8 u1SrSigaAutoFlag;
	UINT_8 au1Reserved[3];
};

enum _ENUM_SR_SELF_BM_MODE_T {
	ENUM_SR_SELF_BM_AUTO = 0,
	ENUM_SR_SELF_BM_MANUAL,
	ENUM_SR_SELF_BM_NUM
};

enum _ENUM_SR_TOPO_LOCK_T {
	ENUM_SR_TOPO_LOCK_AUTO = 0,
	ENUM_SR_TOPO_LOCK_MANUAL,
	ENUM_SR_TOPO_LOCK_NUM
};

enum _ENUM_SR_MESH_BH_TYPE_T {
	ENUM_BH_TYPE_NO_WIFI = 0,
	ENUM_BH_TYPE_WIFI,
	ENUM_BH_TYPE_NUM
};

enum _ENUM_MESH_RMT_FH_STAT_T {
	ENUM_RMT_FH_INVLD_BSSID = 0,
	ENUM_RMT_FH_SCAN_SUCCESS,
	ENUM_RMT_FH_SCAN_FAIL,
	ENUM_RMT_FH_SCAN_INTR_FAIL
};

struct _SET_REMOTE_FH_PARAMS {
	INT_8  i1Rssi;
	UINT_8 u1RemoteFhStat;
	UINT_8 au1Reserved[2];
};

struct _SET_REMOTE_BH_PARAMS {
	UINT_16 u2RemoteBhWcid;
	UINT_8  u1RemoteBhType;
	UINT_8  u1Reserved;
};

struct _SET_MAP_TOPO_PARAMS {
	UINT_8 u1MapDevCount;
	UINT_8 u1MapDevSrSupportMode;
	UINT_8 u1SelfRole;
	UINT_8 u1Reserved;
};

union _SR_MESH_TOPOLOGY_T {
	struct _SET_REMOTE_FH_PARAMS rRemoteFhParams;
	struct _SET_REMOTE_BH_PARAMS rRemoteBhParams;
	struct _SET_MAP_TOPO_PARAMS  rMapTopoParams;
};

/** SR UL TRAFFIC STATUS */
struct _SR_UL_STATUS_T {
	UINT_8 u1UlStatus;
	UINT_8 au1Reserved[3];
};
/* END  SR UL TRAFFIC STATUS */

struct _SR_CMD_MAP_BALANCE_T {
	UINT_8 u1MapBalance;
	UINT_8 au1Reserved[3];
};

struct _SR_CMD_MESH_UL_MODE_T {
	UINT_8 u1UlMode;
	UINT_8 au1Reserved[3];
};

struct _SR_CMD_STA_ALL_HE_T {
	UINT_8 u1StaAllHe;
	UINT_8 au1Reserved[3];
};

/** End FW & DRV sync with sr_if.h **/

/** FW & DRV sync with sr_cmd.c **/
/** SR Command */
enum ENUM_SR_CMD_SUBID {
	SR_CMD_Reserve = 0x0,
	SR_CMD_SET_SR_CFG_SR_ENABLE = 0x1,
	SR_CMD_GET_SR_CFG_SR_ENABLE = 0x2,
	SR_CMD_SET_SR_CFG_SR_SD_ENABLE = 0x3,
	SR_CMD_GET_SR_CFG_SR_SD_ENABLE = 0x4,
	SR_CMD_SET_SR_CFG_SR_MODE = 0x5,
	SR_CMD_GET_SR_CFG_SR_MODE = 0x6,
	SR_CMD_SET_SR_CFG_DISRT_ENABLE = 0x7,
	SR_CMD_GET_SR_CFG_DISRT_ENABLE = 0x8,
	SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI = 0x9,
	SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI = 0xA,
	SR_CMD_SET_SR_CFG_SR_BF = 0xB,
	SR_CMD_GET_SR_CFG_SR_BF = 0xC,
	SR_CMD_SET_SR_CFG_SR_ATF = 0xD,
	SR_CMD_GET_SR_CFG_SR_ATF = 0xE,
	SR_CMD_SET_SR_CFG_TXC_QUEUE = 0xF,
	SR_CMD_GET_SR_CFG_TXC_QUEUE = 0x10,
	SR_CMD_SET_SR_CFG_TXC_QID = 0x11,
	SR_CMD_GET_SR_CFG_TXC_QID = 0x12,
	SR_CMD_SET_SR_CFG_TXC_PATH = 0x13,
	SR_CMD_GET_SR_CFG_TXC_PATH = 0x14,
	SR_CMD_SET_SR_CFG_AC_METHOD = 0x15,
	SR_CMD_GET_SR_CFG_AC_METHOD = 0x16,
	SR_CMD_SET_SR_CFG_SR_PERIOD_THR = 0x17,
	SR_CMD_GET_SR_CFG_SR_PERIOD_THR = 0x18,
	SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD = 0x19,
	SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD = 0x1A,
	SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO = 0x1B,
	SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO = 0x1C,
	SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO = 0x1D,
	SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO = 0x1E,
	SR_CMD_SET_SR_CFG_PROFILE = 0x1F,
	SR_CMD_GET_SR_CFG_PROFILE = 0x20,
	SR_CMD_SET_SR_CFG_FNQ_ENABLE = 0x21,
	SR_CMD_GET_SR_CFG_FNQ_ENABLE = 0x22,
	SR_CMD_SET_SR_CFG_DPD_ENABLE = 0x23,
	SR_CMD_GET_SR_CFG_DPD_ENABLE = 0x24,
	SR_CMD_SET_SR_CFG_SR_TX_ENABLE = 0x25,
	SR_CMD_GET_SR_CFG_SR_TX_ENABLE = 0x26,
	SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE = 0x27,
	SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE = 0x28,
	SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x29,
	SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x2A,
	SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x2B,
	SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x2C,
	SR_CMD_SET_SR_CFG_SR_DABS_MODE = 0x2D,
	SR_CMD_GET_SR_CFG_SR_DABS_MODE = 0x2E,
	SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE = 0x2F,
	SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE = 0x30,
	/*Reseve for SR_CFG = 0x31*/
	/*Reseve for SR_CFG = 0x32*/
	/*Reseve for SR_CFG = 0x33*/
	/*Reseve for SR_CFG = 0x34*/
	/*Reseve for SR_CFG = 0x35*/
	/*Reseve for SR_CFG = 0x36*/
	/*Reseve for SR_CFG = 0x37*/
	/*Reseve for SR_CFG = 0x38*/
	/*Reseve for SR_CFG = 0x39*/
	/*Reseve for SR_CFG = 0x3A*/
	/*Reseve for SR_CFG = 0x3B*/
	/*Reseve for SR_CFG = 0x3C*/
	/*Reseve for SR_CFG = 0x3D*/
	/*Reseve for SR_CFG = 0x3E*/
	/*Reseve for SR_CFG = 0x3F*/
	/*Reseve for SR_CFG = 0x40*/
	/*Reseve for SR_CFG = 0x41*/
	/*Reseve for SR_CFG = 0x42*/
	/*Reseve for SR_CFG = 0x43*/
	/*Reseve for SR_CFG = 0x44*/
	/*Reseve for SR_CFG = 0x45*/
	/*Reseve for SR_CFG = 0x46*/
	/*Reseve for SR_CFG = 0x47*/
	/*Reseve for SR_CFG = 0x48*/
	/*Reseve for SR_CFG = 0x49*/
	/*Reseve for SR_CFG = 0x4A*/
	/*Reseve for SR_CFG = 0x4B*/
	/*Reseve for SR_CFG = 0x4C*/
	/*Reseve for SR_CFG = 0x4D*/
	/*Reseve for SR_CFG = 0x4E*/
	/*Reseve for SR_CFG = 0x4F*/
	/*Reseve for SR_CFG = 0x50*/
	/*Reseve for SR_CFG = 0x51*/
	/*Reseve for SR_CFG = 0x52*/
	/*Reseve for SR_CFG = 0x53*/
	/*Reseve for SR_CFG = 0x54*/
	/*Reseve for SR_CFG = 0x55*/
	/*Reseve for SR_CFG = 0x56*/
	/*Reseve for SR_CFG = 0x57*/
	/*Reseve for SR_CFG = 0x58*/
	/*Reseve for SR_CFG = 0x59*/
	/*Reseve for SR_CFG = 0x5A*/
	/*Reseve for SR_CFG = 0x5B*/
	/*Reseve for SR_CFG = 0x5C*/
	/*Reseve for SR_CFG = 0x5D*/
	/*Reseve for SR_CFG = 0x5E*/
	/*Reseve for SR_CFG = 0x5F*/
	/*Reseve for SR_CFG = 0x60*/
	/*Reseve for SR_CFG = 0x61*/
	/*Reseve for SR_CFG = 0x62*/
	/*Reseve for SR_CFG = 0x63*/
	/*Reseve for SR_CFG = 0x64*/
	/*Reseve for SR_CFG = 0x65*/
	/*Reseve for SR_CFG = 0x66*/
	/*Reseve for SR_CFG = 0x67*/
	/*Reseve for SR_CFG = 0x68*/
	/*Reseve for SR_CFG = 0x69*/
	/*Reseve for SR_CFG = 0x6A*/
	/*Reseve for SR_CFG = 0x6B*/
	/*Reseve for SR_CFG = 0x6C*/
	/*Reseve for SR_CFG = 0x6D*/
	/*Reseve for SR_CFG = 0x6E*/
	/*Reseve for SR_CFG = 0x6F*/
	/*Reseve for SR_CFG = 0x70*/
	/*Reseve for SR_CFG = 0x71*/
	/*Reseve for SR_CFG = 0x72*/
	/*Reseve for SR_CFG = 0x73*/
	/*Reseve for SR_CFG = 0x74*/
	/*Reseve for SR_CFG = 0x75*/
	/*Reseve for SR_CFG = 0x76*/
	/*Reseve for SR_CFG = 0x77*/
	/*Reseve for SR_CFG = 0x78*/
	/*Reseve for SR_CFG = 0x79*/
	/*Reseve for SR_CFG = 0x7A*/
	/*Reseve for SR_CFG = 0x7B*/
	/*Reseve for SR_CFG = 0x7C*/
	/*Reseve for SR_CFG = 0x7D*/
	/*Reseve for SR_CFG = 0x7E*/
	/*Reseve for SR_CFG = 0x7F*/
	SR_CMD_SET_SR_SRG_BITMAP = 0x80,
	SR_CMD_GET_SR_SRG_BITMAP = 0x81,
	SR_CMD_SET_SR_SRG_BITMAP_REFRESH = 0x82,
	SR_CMD_GET_SR_CNT_ALL = 0x83,
	SR_CMD_GET_SR_SD_ALL = 0x84,
	SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL = 0x85,
	SR_CMD_SET_SR_GLO_VAR_STA_CTRL = 0x86,
	SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL = 0x87,
	SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO = 0x88,
	SR_CMD_SET_SR_MESH_SRG_BITMAP = 0x89,
	SR_CMD_GET_SR_MESH_SRG_BITMAP = 0x8A,
	/*Reseve for SR SW Module = 0x8B*/
	/*Reseve for SR SW Module = 0x8C*/
	/*Reseve for SR SW Module = 0x8D*/
	/*Reseve for SR SW Module = 0x8E*/
	/*Reseve for SR SW Module = 0x8F*/
	/*Reseve for SR SW Module = 0x90*/
	/*Reseve for SR SW Module = 0x91*/
	/*Reseve for SR SW Module = 0x92*/
	/*Reseve for SR SW Module = 0x93*/
	/*Reseve for SR SW Module = 0x94*/
	/*Reseve for SR SW Module = 0x95*/
	/*Reseve for SR SW Module = 0x96*/
	/*Reseve for SR SW Module = 0x97*/
	/*Reseve for SR SW Module = 0x98*/
	/*Reseve for SR SW Module = 0x99*/
	/*Reseve for SR SW Module = 0x9A*/
	/*Reseve for SR SW Module = 0x9B*/
	/*Reseve for SR SW Module = 0x9C*/
	/*Reseve for SR SW Module = 0x9D*/
	/*Reseve for SR SW Module = 0x9E*/
	/*Reseve for SR SW Module = 0x9F*/
	/*Reseve for SR SW Module = 0xA0*/
	/*Reseve for SR SW Module = 0xA1*/
	/*Reseve for SR SW Module = 0xA2*/
	/*Reseve for SR SW Module = 0xA3*/
	/*Reseve for SR SW Module = 0xA4*/
	/*Reseve for SR SW Module = 0xA5*/
	/*Reseve for SR SW Module = 0xA6*/
	/*Reseve for SR SW Module = 0xA7*/
	/*Reseve for SR SW Module = 0xA8*/
	/*Reseve for SR SW Module = 0xA9*/
	/*Reseve for SR SW Module = 0xAA*/
	/*Reseve for SR SW Module = 0xAB*/
	/*Reseve for SR SW Module = 0xAC*/
	/*Reseve for SR SW Module = 0xAD*/
	/*Reseve for SR SW Module = 0xAE*/
	/*Reseve for SR SW Module = 0xAF*/
	/*Reseve for SR SW Module = 0xB0*/
	/*Reseve for SR SW Module = 0xB1*/
	/*Reseve for SR SW Module = 0xB2*/
	/*Reseve for SR SW Module = 0xB3*/
	/*Reseve for SR SW Module = 0xB4*/
	/*Reseve for SR SW Module = 0xB5*/
	/*Reseve for SR SW Module = 0xB6*/
	/*Reseve for SR SW Module = 0xB7*/
	/*Reseve for SR SW Module = 0xB8*/
	/*Reseve for SR SW Module = 0xB9*/
	/*Reseve for SR SW Module = 0xBA*/
	/*Reseve for SR SW Module = 0xBB*/
	/*Reseve for SR SW Module = 0xBC*/
	/*Reseve for SR SW Module = 0xBD*/
	/*Reseve for SR SW Module = 0xBE*/
	/*Reseve for SR SW Module = 0xBF*/
	SR_CMD_SET_SR_CAP_ALL_CTRL = 0xC0,
	SR_CMD_GET_SR_CAP_ALL_INFO = 0xC1,
	SR_CMD_SET_SR_PARA_ALL_CTRL = 0xC2,
	SR_CMD_GET_SR_PARA_ALL_INFO = 0xC3,
	SR_CMD_SET_SR_COND_ALL_CTRL = 0xC4,
	SR_CMD_GET_SR_COND_ALL_INFO = 0xC5,
	SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL = 0xC6,
	SR_CMD_GET_SR_RCPI_TBL_ALL_INFO = 0xC7,
	SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL = 0xC8,
	SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO = 0xC9,
	SR_CMD_SET_SR_Q_CTRL_ALL_CTRL = 0xCA,
	SR_CMD_GET_SR_Q_CTRL_ALL_INFO = 0xCB,
	SR_CMD_SET_SR_IBPD_ALL_CTRL = 0xCC,
	SR_CMD_GET_SR_IBPD_ALL_INFO = 0xCD,
	SR_CMD_SET_SR_NRT_ALL_CTRL = 0xCE,
	SR_CMD_GET_SR_NRT_ALL_INFO = 0xCF,
	SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL = 0xD0,
	SR_CMD_GET_SR_NRT_CTRL_ALL_INFO = 0xD1,
	SR_CMD_SET_SR_NRT_RESET_CTRL = 0xD2,
	SR_CMD_SET_SR_CAP_SREN_CTRL = 0xD3,
	SR_CMD_GET_SR_IND_ALL_INFO = 0xD4,
	SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL = 0xD5,
	SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO = 0xD6,
	SR_CMD_SET_SR_FRM_FILT_ALL_CTRL = 0xD7,
	SR_CMD_GET_SR_FRM_FILT_ALL_INFO = 0xD8,
	SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL = 0xD9,
	SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO = 0xDA,
	SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO = 0xDB,
	SR_CMD_SET_SR_SIGA_FLAG_CTRL = 0xDC,
	SR_CMD_GET_SR_SIGA_FLAG_INFO = 0xDD,
	SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL = 0xDE,
	SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO = 0xDF,
	SR_CMD_SET_REMOTE_FH_RSSI = 0xE0,
	SR_CMD_GET_REMOTE_FH_RSSI = 0xE1,
	SR_CMD_SET_REMOTE_BH_INFO = 0xE2,
	SR_CMD_GET_REMOTE_BH_INFO = 0xE3,
	SR_CMD_SET_MAP_TOPO = 0xE4,
	SR_CMD_GET_MAP_TOPO = 0xE5,
	SR_CMD_SET_MAP_TRAFFIC_STATUS = 0xE6,
	SR_CMD_GET_MAP_TRAFFIC_STATUS = 0xE7,
	SR_CMD_SET_MAP_BALANCE = 0xE8,
	SR_CMD_GET_MESH_PHASE = 0xE9,
	SR_CMD_SET_SR_MESH_SR_SD_CTRL = 0xEA,
	SR_CMD_SET_MESH_UL_MODE = 0xEB,
	/*Reseve for SR HW Module = 0xEC*/
	/*Reseve for SR HW Module = 0xED*/
	/*Reseve for SR HW Module = 0xEE*/
	/*Reseve for SR HW Module = 0xEF*/
	/*Reseve for SR HW Module = 0xF0*/
	/*Reseve for SR HW Module = 0xF1*/
	/*Reseve for SR HW Module = 0xF2*/
	/*Reseve for SR HW Module = 0xF3*/
	/*Reseve for SR HW Module = 0xF4*/
	/*Reseve for SR HW Module = 0xF5*/
	/*Reseve for SR HW Module = 0xF6*/
	/*Reseve for SR HW Module = 0xF7*/
	/*Reseve for SR HW Module = 0xF8*/
	/*Reseve for SR HW Module = 0xF9*/
	/*Reseve for SR HW Module = 0xFA*/
	/*Reseve for SR HW Module = 0xFB*/
	/*Reseve for SR HW Module = 0xFC*/
	/*Reseve for SR HW Module = 0xFD*/
	/*Reseve for SR HW Module = 0xFE*/
	/*Reseve for SR HW Module = 0xFF*/
};

struct SR_CMD_T {
	UINT_8  u1CmdSubId;
	UINT_8  u1ArgNum;
	UINT_8  u1DbdcIdx;
	UINT_8  u1Status;
	UINT_8  u1DropTaIdx;
	UINT_8  u1StaIdx;	/* #256STA */
	UINT_8  u1Rsv[2];
	UINT_32 u4Value;
};

struct SR_CMD_SR_CAP_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_CAP_T_SR_V1 rSrCap;
	/** Reserve for 8-byte aligned*/
	UINT_8 u4Rsv[8];
};

struct SR_CMD_SR_CAP_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_CAP_T_SR_V2 rSrCap;
};

struct SR_CMD_SR_PARA_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_PARA_T rSrPara;
};

struct SR_CMD_SR_IND_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_IND_T rSrInd;
};

struct SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
	UINT_8 u1Rsv;
	UINT_8 u1CurSrDropTaIdx;
	UINT_16 u2SrTtlTxCntThr;
	struct SR_DROP_TA_INFO_T rSrDropTaInfo;
};

struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
	struct SR_CMD_T rSrCmd;
	struct SR_GLOBAL_VAR_SINGLE_DROP_TA_T rSrGlobalVarSingleDropTa;
};

struct SR_CMD_SR_COND_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct SR_COND_T_SR_V1 rSrCond;
};

struct SR_CMD_SR_COND_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_COND_T_SR_V2 rSrCond;
};

struct SR_CMD_SR_RCPITBL_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_RCPITBL_T rSrRcpiTbl;
};

struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct SR_RCPITBL_OFST_T_SR_V1 rSrRcpiTblOfst;
};

struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
};

struct SR_CMD_SR_Q_CTRL_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_QUEUE_CTRL_T_SR_V1 rSrQCtrl;
	/** Reserve for 4-byte aligned*/
	UINT_8 u4Rsv[4];
};

struct SR_CMD_SR_Q_CTRL_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
};

struct SR_CMD_SR_IBPD_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_IBPD_T rSrIBPD;
};

struct SR_CMD_SR_NRT_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct SR_NRT_T_SR_V1 rSrNRT;
};

struct SR_CMD_SR_NRT_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_NRT_T_SR_V2 rSrNRT;
};

struct SR_CMD_SR_NRT_CTRL_T_SR_V1 {
	struct SR_CMD_T rSrCmd;
	struct SR_NRT_CTRL_T_SR_V1 rSrNRTCtrl;
};

struct SR_CMD_SR_NRT_CTRL_T_SR_V2 {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
};

struct SR_CMD_SR_SRG_BITMAP_T {
	struct SR_CMD_T rSrCmd;
	struct SR_SRG_BITMAP_T rSrSrgBitmap[RAM_BAND_NUM];
};

struct SR_CMD_SR_FNQ_CTRL_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_FNQ_CTRL_T rSrFNQCtrl;
};

struct SR_CMD_SR_FRM_FILT_T {
	struct SR_CMD_T rSrCmd;
	UINT_32 u4SrFrmFilt;
};

struct SR_CMD_SR_INTERPS_CTRL_T {
	struct SR_CMD_T rSrCmd;
	struct WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
};

struct SR_CMD_SR_SIGA_FLAG_T {
	struct SR_CMD_T rSrCmd;
	struct SR_SIGA_FLAG_T rSrSigaFlag;
};

struct _SR_CMD_SR_SIGA_AUTO_FLAG_T {
	struct SR_CMD_T rSrCmd;
	struct _SR_SIGA_AUTO_FLAG_T rSrSigaAutoFlag;
};

struct _SR_CMD_MESH_TOPOLOGY_T {
	struct SR_CMD_T rSrCmd;
	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T {
	struct SR_CMD_T rSrCmd;
	struct _SR_UL_STATUS_T rSrUlStatus;
};

struct _SR_CMD_SET_MAP_BALANCE_T {
	struct SR_CMD_T rSrCmd;
	struct _SR_CMD_MAP_BALANCE_T rSrMapBalance;
};

struct _SR_CMD_SET_MESH_UL_MODE_T {
	struct SR_CMD_T rSrCmd;
	struct _SR_CMD_MESH_UL_MODE_T rSrMeshUlMode;
};


/** End SR Command */
/** SR Event */
enum ENUM_SR_EVENT_SUBID {
	SR_EVENT_Reserve = 0x0,
	SR_EVENT_GET_SR_CFG_SR_ENABLE = 0x1,
	SR_EVENT_GET_SR_CFG_SR_SD_ENABLE = 0x2,
	SR_EVENT_GET_SR_CFG_SR_MODE = 0x3,
	SR_EVENT_GET_SR_CFG_DISRT_ENABLE = 0x4,
	SR_EVENT_GET_SR_CFG_DISRT_MIN_RSSI = 0x5,
	SR_EVENT_GET_SR_CFG_SR_BF = 0x6,
	SR_EVENT_GET_SR_CFG_SR_ATF = 0x7,
	SR_EVENT_GET_SR_CFG_TXC_QUEUE = 0x8,
	SR_EVENT_GET_SR_CFG_TXC_QID = 0x9,
	SR_EVENT_GET_SR_CFG_TXC_PATH = 0xA,
	SR_EVENT_GET_SR_CFG_AC_METHOD = 0xB,
	SR_EVENT_GET_SR_CFG_SR_PERIOD_THR = 0xC,
	SR_EVENT_GET_SR_CFG_QUERY_TXD_METHOD = 0xD,
	SR_EVENT_GET_SR_CFG_SR_SD_CG_RATIO = 0xE,
	SR_EVENT_GET_SR_CFG_SR_SD_OBSS_RATIO = 0xF,
	SR_EVENT_GET_SR_CFG_PROFILE = 0x10,
	SR_EVENT_GET_SR_CFG_FNQ_ENABLE = 0x11,
	SR_EVENT_GET_SR_CFG_DPD_ENABLE = 0x12,
	SR_EVENT_GET_SR_CFG_SR_TX_ENABLE = 0x13,
	SR_EVENT_GET_SR_CFG_SR_SD_OM_ENABLE = 0x14,
	SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x15,
	SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x16,
	SR_EVENT_GET_SR_CFG_SR_DABS_MODE = 0x17,
	SR_EVENT_GET_SR_CFG_MESH_REMOTE_STA_MODE = 0x18,
	/*Reseve for SR_CFG = 0x19*/
	/*Reseve for SR_CFG = 0x1A*/
	/*Reseve for SR_CFG = 0x1B*/
	/*Reseve for SR_CFG = 0x1C*/
	/*Reseve for SR_CFG = 0x1D*/
	/*Reseve for SR_CFG = 0x1E*/
	/*Reseve for SR_CFG = 0x1F*/
	/*Reseve for SR_CFG = 0x20*/
	/*Reseve for SR_CFG = 0x21*/
	/*Reseve for SR_CFG = 0x22*/
	/*Reseve for SR_CFG = 0x23*/
	/*Reseve for SR_CFG = 0x24*/
	/*Reseve for SR_CFG = 0x25*/
	/*Reseve for SR_CFG = 0x26*/
	/*Reseve for SR_CFG = 0x27*/
	/*Reseve for SR_CFG = 0x28*/
	/*Reseve for SR_CFG = 0x29*/
	/*Reseve for SR_CFG = 0x2A*/
	/*Reseve for SR_CFG = 0x2B*/
	/*Reseve for SR_CFG = 0x2C*/
	/*Reseve for SR_CFG = 0x2D*/
	/*Reseve for SR_CFG = 0x2E*/
	/*Reseve for SR_CFG = 0x2F*/
	/*Reseve for SR_CFG = 0x30*/
	/*Reseve for SR_CFG = 0x31*/
	/*Reseve for SR_CFG = 0x32*/
	/*Reseve for SR_CFG = 0x33*/
	/*Reseve for SR_CFG = 0x34*/
	/*Reseve for SR_CFG = 0x35*/
	/*Reseve for SR_CFG = 0x36*/
	/*Reseve for SR_CFG = 0x37*/
	/*Reseve for SR_CFG = 0x38*/
	/*Reseve for SR_CFG = 0x39*/
	/*Reseve for SR_CFG = 0x3A*/
	/*Reseve for SR_CFG = 0x3B*/
	/*Reseve for SR_CFG = 0x3C*/
	/*Reseve for SR_CFG = 0x3D*/
	/*Reseve for SR_CFG = 0x3E*/
	/*Reseve for SR_CFG = 0x3F*/
	/*Reseve for SR_CFG = 0x40*/
	/*Reseve for SR_CFG = 0x41*/
	/*Reseve for SR_CFG = 0x42*/
	/*Reseve for SR_CFG = 0x43*/
	/*Reseve for SR_CFG = 0x44*/
	/*Reseve for SR_CFG = 0x45*/
	/*Reseve for SR_CFG = 0x46*/
	/*Reseve for SR_CFG = 0x47*/
	/*Reseve for SR_CFG = 0x48*/
	/*Reseve for SR_CFG = 0x49*/
	/*Reseve for SR_CFG = 0x4A*/
	/*Reseve for SR_CFG = 0x4B*/
	/*Reseve for SR_CFG = 0x4C*/
	/*Reseve for SR_CFG = 0x4D*/
	/*Reseve for SR_CFG = 0x4E*/
	/*Reseve for SR_CFG = 0x4F*/
	/*Reseve for SR_CFG = 0x50*/
	/*Reseve for SR_CFG = 0x51*/
	/*Reseve for SR_CFG = 0x52*/
	/*Reseve for SR_CFG = 0x53*/
	/*Reseve for SR_CFG = 0x54*/
	/*Reseve for SR_CFG = 0x55*/
	/*Reseve for SR_CFG = 0x56*/
	/*Reseve for SR_CFG = 0x57*/
	/*Reseve for SR_CFG = 0x58*/
	/*Reseve for SR_CFG = 0x59*/
	/*Reseve for SR_CFG = 0x5A*/
	/*Reseve for SR_CFG = 0x5B*/
	/*Reseve for SR_CFG = 0x5C*/
	/*Reseve for SR_CFG = 0x5D*/
	/*Reseve for SR_CFG = 0x5E*/
	/*Reseve for SR_CFG = 0x5F*/
	/*Reseve for SR_CFG = 0x60*/
	/*Reseve for SR_CFG = 0x61*/
	/*Reseve for SR_CFG = 0x62*/
	/*Reseve for SR_CFG = 0x63*/
	/*Reseve for SR_CFG = 0x64*/
	/*Reseve for SR_CFG = 0x65*/
	/*Reseve for SR_CFG = 0x66*/
	/*Reseve for SR_CFG = 0x67*/
	/*Reseve for SR_CFG = 0x68*/
	/*Reseve for SR_CFG = 0x69*/
	/*Reseve for SR_CFG = 0x6A*/
	/*Reseve for SR_CFG = 0x6B*/
	/*Reseve for SR_CFG = 0x6C*/
	/*Reseve for SR_CFG = 0x6D*/
	/*Reseve for SR_CFG = 0x6E*/
	/*Reseve for SR_CFG = 0x6F*/
	/*Reseve for SR_CFG = 0x70*/
	/*Reseve for SR_CFG = 0x71*/
	/*Reseve for SR_CFG = 0x72*/
	/*Reseve for SR_CFG = 0x73*/
	/*Reseve for SR_CFG = 0x74*/
	/*Reseve for SR_CFG = 0x75*/
	/*Reseve for SR_CFG = 0x76*/
	/*Reseve for SR_CFG = 0x77*/
	/*Reseve for SR_CFG = 0x78*/
	/*Reseve for SR_CFG = 0x79*/
	/*Reseve for SR_CFG = 0x7A*/
	/*Reseve for SR_CFG = 0x7B*/
	/*Reseve for SR_CFG = 0x7C*/
	/*Reseve for SR_CFG = 0x7D*/
	/*Reseve for SR_CFG = 0x7E*/
	/*Reseve for SR_CFG = 0x7F*/
	SR_EVENT_GET_SR_SRG_BITMAP = 0x80,
	SR_EVENT_GET_SR_CNT_ALL = 0x81,
	SR_EVENT_GET_SR_SD_ALL = 0x82,
	SR_EVENT_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO = 0x83,
	SR_EVENT_GET_SR_MESH_SRG_BITMAP = 0x84,
	/*Reseve for SR SW Module = 0x85*/
	/*Reseve for SR SW Module = 0x86*/
	/*Reseve for SR SW Module = 0x87*/
	/*Reseve for SR SW Module = 0x88*/
	/*Reseve for SR SW Module = 0x89*/
	/*Reseve for SR SW Module = 0x8A*/
	/*Reseve for SR SW Module = 0x8B*/
	/*Reseve for SR SW Module = 0x8C*/
	/*Reseve for SR SW Module = 0x8D*/
	/*Reseve for SR SW Module = 0x8E*/
	/*Reseve for SR SW Module = 0x8F*/
	/*Reseve for SR SW Module = 0x90*/
	/*Reseve for SR SW Module = 0x91*/
	/*Reseve for SR SW Module = 0x92*/
	/*Reseve for SR SW Module = 0x93*/
	/*Reseve for SR SW Module = 0x94*/
	/*Reseve for SR SW Module = 0x95*/
	/*Reseve for SR SW Module = 0x96*/
	/*Reseve for SR SW Module = 0x97*/
	/*Reseve for SR SW Module = 0x98*/
	/*Reseve for SR SW Module = 0x99*/
	/*Reseve for SR SW Module = 0x9A*/
	/*Reseve for SR SW Module = 0x9B*/
	/*Reseve for SR SW Module = 0x9C*/
	/*Reseve for SR SW Module = 0x9D*/
	/*Reseve for SR SW Module = 0x9E*/
	/*Reseve for SR SW Module = 0x9F*/
	/*Reseve for SR SW Module = 0xA0*/
	/*Reseve for SR SW Module = 0xA1*/
	/*Reseve for SR SW Module = 0xA2*/
	/*Reseve for SR SW Module = 0xA3*/
	/*Reseve for SR SW Module = 0xA4*/
	/*Reseve for SR SW Module = 0xA5*/
	/*Reseve for SR SW Module = 0xA6*/
	/*Reseve for SR SW Module = 0xA7*/
	/*Reseve for SR SW Module = 0xA8*/
	/*Reseve for SR SW Module = 0xA9*/
	/*Reseve for SR SW Module = 0xAA*/
	/*Reseve for SR SW Module = 0xAB*/
	/*Reseve for SR SW Module = 0xAC*/
	/*Reseve for SR SW Module = 0xAD*/
	/*Reseve for SR SW Module = 0xAE*/
	/*Reseve for SR SW Module = 0xAF*/
	/*Reseve for SR SW Module = 0xB0*/
	/*Reseve for SR SW Module = 0xB1*/
	/*Reseve for SR SW Module = 0xB2*/
	/*Reseve for SR SW Module = 0xB3*/
	/*Reseve for SR SW Module = 0xB4*/
	/*Reseve for SR SW Module = 0xB5*/
	/*Reseve for SR SW Module = 0xB6*/
	/*Reseve for SR SW Module = 0xB7*/
	/*Reseve for SR SW Module = 0xB8*/
	/*Reseve for SR SW Module = 0xB9*/
	/*Reseve for SR SW Module = 0xBA*/
	/*Reseve for SR SW Module = 0xBB*/
	/*Reseve for SR SW Module = 0xBC*/
	/*Reseve for SR SW Module = 0xBD*/
	/*Reseve for SR SW Module = 0xBE*/
	/*Reseve for SR SW Module = 0xBF*/
	SR_EVENT_GET_SR_CAP_ALL_INFO = 0xC0,
	SR_EVENT_GET_SR_PARA_ALL_INFO = 0xC1,
	SR_EVENT_GET_SR_COND_ALL_INFO = 0xC2,
	SR_EVENT_GET_SR_RCPI_TBL_ALL_INFO = 0xC3,
	SR_EVENT_GET_SR_RCPI_TBL_OFST_ALL_INFO = 0xC4,
	SR_EVENT_GET_SR_Q_CTRL_ALL_INFO = 0xC5,
	SR_EVENT_GET_SR_IBPD_ALL_INFO = 0xC6,
	SR_EVENT_GET_SR_NRT_ALL_INFO = 0xC7,
	SR_EVENT_GET_SR_NRT_CTRL_ALL_INFO = 0xC8,
	SR_EVENT_GET_SR_IND_ALL_INFO = 0xC9,
	SR_EVENT_GET_SR_FNQ_CTRL_ALL_INFO = 0xCA,
	SR_EVENT_GET_SR_FRM_FILT_ALL_INFO = 0xCB,
	SR_EVENT_GET_SR_INTERPS_CTRL_ALL_INFO = 0xCC,
	SR_EVENT_GET_SR_INTERPS_DBG_ALL_INFO = 0xCD,
	SR_EVENT_GET_SR_SIGA_FLAG_ALL_INFO = 0xCE,
	SR_EVENT_GET_SR_SIGA_AUTO_FLAG_ALL_INFO = 0xCF,
	SR_EVENT_GET_REMOTE_FH_RSSI = 0xD0,
	SR_EVENT_GET_REMOTE_BH_INFO = 0xD1,
	SR_EVENT_GET_MAP_TOPO = 0xD2,
	SR_EVENT_GET_MAP_TRAFFIC_STATUS = 0xD3,
	SR_EVENT_SEND_MESH_UPLINK_TRAFFIC = 0xD4,
	SR_EVENT_GET_MESH_PHASE = 0xD5,
	/*Reseve for SR HW Module = 0xD6*/
	/*Reseve for SR HW Module = 0xD7*/
	/*Reseve for SR HW Module = 0xD8*/
	/*Reseve for SR HW Module = 0xD9*/
	/*Reseve for SR HW Module = 0xDA*/
	/*Reseve for SR HW Module = 0xDB*/
	/*Reseve for SR HW Module = 0xDC*/
	/*Reseve for SR HW Module = 0xDD*/
	/*Reseve for SR HW Module = 0xDE*/
	/*Reseve for SR HW Module = 0xDF*/
	/*Reseve for SR HW Module = 0xE0*/
	/*Reseve for SR HW Module = 0xE1*/
	/*Reseve for SR HW Module = 0xE2*/
	/*Reseve for SR HW Module = 0xE3*/
	/*Reseve for SR HW Module = 0xE4*/
	/*Reseve for SR HW Module = 0xE5*/
	/*Reseve for SR HW Module = 0xE6*/
	/*Reseve for SR HW Module = 0xE7*/
	/*Reseve for SR HW Module = 0xE8*/
	/*Reseve for SR HW Module = 0xE9*/
	/*Reseve for SR HW Module = 0xEA*/
	/*Reseve for SR HW Module = 0xEB*/
	/*Reseve for SR HW Module = 0xEC*/
	/*Reseve for SR HW Module = 0xED*/
	/*Reseve for SR HW Module = 0xEE*/
	/*Reseve for SR HW Module = 0xEF*/
	/*Reseve for SR HW Module = 0xF0*/
	/*Reseve for SR HW Module = 0xF1*/
	/*Reseve for SR HW Module = 0xF2*/
	/*Reseve for SR HW Module = 0xF3*/
	/*Reseve for SR HW Module = 0xF4*/
	/*Reseve for SR HW Module = 0xF5*/
	/*Reseve for SR HW Module = 0xF6*/
	/*Reseve for SR HW Module = 0xF7*/
	/*Reseve for SR HW Module = 0xF8*/
	/*Reseve for SR HW Module = 0xF9*/
	/*Reseve for SR HW Module = 0xFA*/
	/*Reseve for SR HW Module = 0xFB*/
	/*Reseve for SR HW Module = 0xFC*/
	/*Reseve for SR HW Module = 0xFD*/
	/*Reseve for SR HW Module = 0xFE*/
	/*Reseve for SR HW Module = 0xFF*/
};

struct SR_EVENT_T {
	UINT_8  u1EventSubId;
	UINT_8  u1ArgNum;
	UINT_8  u1DbdcIdx;
	UINT_8  u1Status;
	UINT_8  u1DropTaIdx;
	UINT_8  u1StaIdx;	/* #256STA */
	UINT_8  u1Rsv[2];
	UINT_32 u4Value;
};

enum ENUM_SR_EVENT_STATUS_T {
	SR_STATUS_SUCCESS = 0x0,
	SR_STATUS_SANITY_FAIL,
	SR_STATUS_CALL_MIDDLE_FAIL,
	SR_STATUS_SW_HW_VAL_NOT_SYNC,
	SR_STATUS_UNKNOWN,
	SR_STATUS_NUM
};

struct SR_EVENT_SR_CAP_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_CAP_T_SR_V1 rSrCap;
};

struct SR_EVENT_SR_CAP_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_CAP_T_SR_V2 rSrCap;
};

struct SR_EVENT_SR_PARA_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_PARA_T rSrPara;
};

struct SR_EVENT_SR_IND_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_IND_T rSrInd;
};

struct SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
	struct SR_EVENT_T rSrEvent;
	struct SR_GLOBAL_VAR_SINGLE_DROP_TA_T rSrGlobalVarSingleDropTa;
};

struct SR_EVENT_SR_COND_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct SR_COND_T_SR_V1 rSrCond;
};

struct SR_EVENT_SR_COND_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_COND_T_SR_V2 rSrCond;
};

struct SR_EVENT_SR_RCPITBL_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_RCPITBL_T rSrRcpiTbl;
};

struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct SR_RCPITBL_OFST_T_SR_V1 rSrRcpiTblOfst;
};

struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
};

struct SR_EVENT_SR_Q_CTRL_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_QUEUE_CTRL_T_SR_V1 rSrQCtrl;
};

struct SR_EVENT_SR_Q_CTRL_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
};

struct SR_EVENT_SR_IBPD_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_IBPD_T rSrIBPD;
};

struct SR_EVENT_SR_NRT_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct SR_NRT_T_SR_V1 rSrNRT[SR_NRT_ROW_NUM];
};

struct SR_EVENT_SR_NRT_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_NRT_T_SR_V2 rSrNRT[SR_NRT_ROW_NUM];
};

struct SR_EVENT_SR_NRT_CTRL_T_SR_V1 {
	struct SR_EVENT_T rSrEvent;
	struct SR_NRT_CTRL_T_SR_V1 rSrNRTCtrl;
};

struct SR_EVENT_SR_NRT_CTRL_T_SR_V2 {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
};

struct SR_EVENT_SR_CNT_T {
	struct SR_EVENT_T rSrEvent;
	struct SR_CNT_T rSrCnt[RAM_BAND_NUM];
};

struct SR_EVENT_SR_SD_T {
	struct SR_EVENT_T rSrEvent;
	struct SR_SD_T rSrSd[RAM_BAND_NUM];
};

struct SR_EVENT_SR_SRG_BITMAP_T {
	struct SR_EVENT_T rSrEvent;
	struct SR_SRG_BITMAP_T rSrSrgBitmap[RAM_BAND_NUM];
};

struct SR_EVENT_SR_FNQ_CTRL_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_FNQ_CTRL_T rSrFNQCtrl;
};

struct SR_EVENT_SR_FRM_FILT_T {
	struct SR_EVENT_T rSrEvent;
	UINT_32 u4SrFrmFilt;
};

struct SR_EVENT_SR_INTERPS_CTRL_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
};

struct SR_EVENT_SR_INTERPS_DBG_T {
	struct SR_EVENT_T rSrEvent;
	struct WH_SR_INTERPS_DBG_T rSrInterPsDbg;
};

struct SR_EVENT_SR_SIGA_T {
	struct SR_EVENT_T rSrEvent;
	struct SR_SIGA_FLAG_T rSrSigaFlag;
};

struct _SR_EVENT_SR_SIGA_AUTO_T {
	struct SR_EVENT_T rSrEvent;
};

struct _SR_EVENT_MESH_TOPOLOGY_T {
	struct SR_EVENT_T rSrEvent;
	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T {
	struct SR_EVENT_T rSrEvent;
	struct _SR_UL_STATUS_T rSrUlStatus;
};

struct _SR_EVENT_SR_MESH_PHASE_T {
	struct SR_EVENT_T rSrEvent;
};

struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T {
	struct SR_EVENT_T rSrEvent;
};

struct SHARING_STA_RA_T {
	UINT_8 u1TxPowerOffset;
	UINT_8 u1Mode; /* 0: Auto Power, 1: Fixed Power*/
	UINT_8 u1State; /* ENUM_SR_STA_STATE_T */
	UINT_8 u1CoSrPwrTryCnt;
	UINT_8 u1CoSrPwrRound;
	UINT_8 u1CoSrPwrState;
	UINT_16 u2CoSrSucCnt;
	UINT_16 u2CoSrTtlTxCnt;
	UINT_16 u2BadQuota;
	UINT_8 u1BadLevel;
	INT_8 i1CoSrPwr;
	INT_8 i1FixCoSRPwr;
};

struct SHARED_STA_RA_T {
	INT_8 i1CoSrRateOffset;
	UINT_8 u1Mode; /* 0: Auto Rate, 1: Fixed Rate*/
	UINT_8 u1State; /* ENUM_SR_STA_STATE_T */
	UINT_8 u1CoSrRaTryCnt;
	UINT_8 u1CoSrRaRound;
	UINT_8 u1CoSrRaState;
	UINT_16 u2CoSrSucCnt;
	UINT_16 u2CoSrTtlTxCnt;
	UINT_32 u4Score;
	UINT_16 u2BadQuota;
	UINT_8 u1BadLevel;
	UINT_8 u1CoSrRate;
	UINT_8 u1FixCoSrRate;
};
/** End SR Event */
/** End FW & DRV sync with sr_cmd.c **/
/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
/* For Command*/
NDIS_STATUS SrCmd(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_T *prSrcmd);
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap);
NDIS_STATUS SrCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_PARA_T *prSrCmdSrPara);
NDIS_STATUS SrCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd,
					    IN struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
					    prSrCmdSrGlobalVarSingleDropTa, IN UINT_32 u4DropTaIdx,
					    IN UINT_32 u4StaIdx);
NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond);
NDIS_STATUS SrCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl);
NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst);
NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl);
NDIS_STATUS SrCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_IBPD_T *prSrCmdSrIBPD);
NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT);
NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl);
NDIS_STATUS SrCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl);
NDIS_STATUS SrCmdSRUpdateFrmFilt(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_FRM_FILT_T *prSrCmdSrFrmFilt);
NDIS_STATUS SrCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl);
NDIS_STATUS SrCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_SRG_BITMAP_T *prSrCmdSrSrgBitmap);
NDIS_STATUS SrCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_SIGA_FLAG_T *rSrCmdSrSigaFlag);
NDIS_STATUS SrCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *rSrCmdSrSigaAutoFlag);
NDIS_STATUS SrCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo);
NDIS_STATUS SrSetRemoteFHRssi(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi, UINT_8 u1RemoteFhStat);
NDIS_STATUS SrSetMeshStaThreshold(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx,  UINT8 u1Bssid, INT_8 i1DlThreshold);
NDIS_STATUS SrSetMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 i1BhThreshold);
NDIS_STATUS SrSetMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 i1FhThreshold);
NDIS_STATUS SrSetRemoteAssocBHInfo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 BhType, UINT_16 Wcid);
NDIS_STATUS SrSetMAPTopo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 MapDevCount, UINT_8 MapDevSrSupportMode, UINT_8 SelfRole);
NDIS_STATUS SrCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus);
NDIS_STATUS SrCmdMeshMapBalanceSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance);
NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum);
/* For Check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum);
NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto);
/* For Print content */
VOID PrintSrCmd(IN struct SR_CMD_T *prSrCmd);
VOID PrintSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCap);
VOID PrintSrPara(IN struct WH_SR_PARA_T *prSrPara);
VOID PrintSrInd(IN struct WH_SR_IND_T *prSrInd);
VOID PrintSrGloVarSingleDropTa(IN struct SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx);
VOID PrintSrDropTaInfo(IN struct SR_DROP_TA_INFO_T *prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx);
VOID PrintSrStaInfo(IN struct SR_STA_INFO_T *prSrStaInfo, IN UINT_8 u1StaIdx);
VOID PrintSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCond);
VOID PrintSrRcpiTbl(IN struct WH_SR_RCPITBL_T *prSrRcpiTbl);
VOID PrintSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrRcpiTblOfst);
VOID PrintSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrQCtrl);
VOID PrintSrIBPD(IN struct WH_SR_IBPD_T *prSrIBPD);
VOID PrintSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRT);
VOID PrintSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRTCtrl);
VOID PrintSrFNQCtrl(IN struct WH_SR_FNQ_CTRL_T *prSrFNQCtrl);
VOID PrintSrFrmFilt(IN UINT_32 *pu4SrFrmFilt);
VOID PrintSrInterPsCtrl(IN struct WH_SR_INTERPS_CTRL_T *prSrInterPsCtrl);
VOID PrintSrInterPsDbg(IN struct WH_SR_INTERPS_DBG_T *prSrInterPsDbg);
VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN struct SR_SRG_BITMAP_T *prSrSrgBitmap);
VOID PrintSrSiga(IN PRTMP_ADAPTER pAD, IN UINT_8 u1DbdcIdx, IN struct SR_SIGA_FLAG_T *prSrSigaflag, IN BOOLEAN fgread);
VOID PrintSrMeshTopo(IN UINT_8 u1SubId, IN union _SR_MESH_TOPOLOGY_T *prSrMeshTopo);
VOID PrintSrCnt(IN UINT8 u1DbdcIdx, IN struct SR_CNT_T *prSrCnt, IN UINT8 u1PpduType);
VOID PrintSrSd(IN UINT8 u1DbdcIdx, IN struct SR_SD_T *prSrSd);

VOID PrintSrCmdSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap);
VOID PrintSrCmdSrPara(IN struct SR_CMD_SR_PARA_T *prSrCmdSrPara);
VOID PrintSrCmdSrGloVarSingleDropTa(IN struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
				    prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DropTaIdx,
				    IN UINT_8 u1StaIdx);
VOID PrintSrCmdSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond);
VOID PrintSrCmdSrRcpiTbl(IN struct SR_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl);
VOID PrintSrCmdSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst);
VOID PrintSrCmdSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl);
VOID PrintSrCmdSrIBPD(IN struct SR_CMD_SR_IBPD_T *prSrCmdSrIBPD);
VOID PrintSrCmdSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT);
VOID PrintSrCmdSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl);
VOID PrintSrCmdSrFNQCtrl(IN struct SR_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl);
VOID PrintSrCmdSrFrmFilt(IN struct SR_CMD_SR_FRM_FILT_T *prSrCmdSrFrmFilt);
VOID PrintSrCmdSrInterPsCtrl(IN struct SR_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl);
VOID PrintSrCmdSrSrgBitmap(IN struct SR_CMD_SR_SRG_BITMAP_T *prSrCmdSrSrgBitmap);
VOID PrintSrCmdSrSiga(IN PRTMP_ADAPTER pAD, IN struct SR_CMD_SR_SIGA_FLAG_T *prSrCmdSrSigaFlag);
VOID PrintSrCmdSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag);
VOID PrintSrCmdMeshTopo(IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo);
VOID PrintSrCmdSrUlStatus(IN PRTMP_ADAPTER pAD, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus);
VOID PrintSrCmdSrMapBalance(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance);
VOID PrintSrEvent(IN struct SR_EVENT_T *prSrEvent);
VOID PrintSrEventSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCap);
VOID PrintSrEventSrPara(IN struct SR_EVENT_SR_PARA_T *prSrEventSrPara);
VOID PrintSrEventSrInd(IN struct SR_EVENT_SR_IND_T  *prSrEventSrInd);
VOID PrintSrEventSrGloVarSingleDropTa(IN struct SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
				      prSrEventSrGlobalVarSingleDropT);
VOID PrintSrEventSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCond);
VOID PrintSrEventSrRcpiTbl(IN struct SR_EVENT_SR_RCPITBL_T *prSrEventSrRcpiTbl);
VOID PrintSrEventSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrRcpiTblOfst);
VOID PrintSrEventSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrQCtrl);
VOID PrintSrEventSrIBPD(IN struct SR_EVENT_SR_IBPD_T *prSrEventSrIBPD);
VOID PrintSrEventSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRT);
VOID PrintSrEventSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRTCtrl);
VOID PrintSrEventSrFNQCtrl(IN struct SR_EVENT_SR_FNQ_CTRL_T *prSrEventSrFNQCtrl);
VOID PrintSrEventSrFrmFilt(IN struct SR_EVENT_SR_FRM_FILT_T *prSrEventSrFrmFilt);
VOID PrintSrEventSrInterPsCtrl(IN struct SR_EVENT_SR_INTERPS_CTRL_T *prSrEventSrInterPsCtrl);
VOID PrintSrEventSrInterPsDbg(IN struct SR_EVENT_SR_INTERPS_DBG_T *prSrEventSrInterPsDbg);
VOID PrintSrEventSrCnt(IN struct SR_EVENT_SR_CNT_T *prSrEventSrCnt);
VOID PrintSrEventSrSd(IN struct SR_EVENT_SR_SD_T *prSrEventSrSd);
VOID PrintSrEventSrSrgBitmap(IN struct SR_EVENT_SR_SRG_BITMAP_T *prSrEventSrSrgBitmap);
VOID PrintSrEventSrSiga(IN PRTMP_ADAPTER pAD, IN struct SR_EVENT_SR_SIGA_T *prSrEventSrSigaFlag);
VOID PrintSrEventSrSigaAuto(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_SIGA_AUTO_T *prSrEventSrSigaAutoFlag);
VOID PrintSrEventMeshTopo(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_MESH_TOPOLOGY_T *prSrEventMeshTopology);
VOID PrintSrEventSrUlStatus(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus);
VOID PrintSrEventSrMeshPhase(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_MESH_PHASE_T *prSrEventSrMeshPhase);
VOID PrintSrEventSrRemoteAPStaAllHe(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *prSrEventSrRemoteAPStaAllHe);
VOID SrMeshSelfBssColorChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx);
VOID SrMeshSelfPBssidChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx);
VOID SrMeshSelfSrgInfoEvent(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx);
VOID ExtEventMeshUplinkTraffic(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus);
VOID SrMeshSrReportSTAMode(IN PRTMP_ADAPTER pAd, UINT8 u1BandIdx, UINT8 u1StaAllHe);

#endif				/* CFG_SUPPORT_FALCON_SR */
#endif				/* __CMM_SR_CMD_H__ */
