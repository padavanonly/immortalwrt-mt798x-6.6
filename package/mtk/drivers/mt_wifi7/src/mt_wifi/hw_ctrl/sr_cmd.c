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
	sr_cmd.c
*/
#include "rt_config.h"
/*For HWITS00021718*/
#include "mac_mt/fmac/mt_fmac.h"
#ifdef MT7990
#include "chip/mt7990_cr.h"
#endif
#ifdef MT7992
#include "chip/mt7992_cr.h"
#endif
#ifdef MT7993
#include "chip/mt7993_cr.h"
#endif

/*End - For HWITS00021718*/

#if defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993)
#ifdef CFG_SUPPORT_FALCON_SR
/*******************************************************************************
 *    MACRO
 ******************************************************************************/
#define IS_SR_V1(_pAd) (IS_MT7915(_pAd))
#define IS_SR_V2(_pAd) (IS_MT7986(_pAd) || IS_BELLWETHER(_pAd) || \
						IS_MT7990(_pAd) || IS_MT7992(_pAd) || \
						IS_MT7993(_pAd))

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
#define SR_PARA_RCPI_SRC_SEL_ANT_0          0x0
#define SR_PARA_RCPI_SRC_SEL_ANT_1          0x1
#define SR_PARA_RCPI_SRC_SEL_ANT_2          0x2
#define SR_PARA_RCPI_SRC_SEL_ANT_3          0x3
/* Decimal dbm*/
#define SR_PARA_PD_THR_MAX                  -62
#define SR_PARA_PD_THR_MIN                  -110
#define SR_PARA_PD_THR_DEFAULT              SR_PARA_PD_THR_MAX
#define SR_PARA_PD_MIN_MAX                  0
#define SR_PARA_PD_MIN_MIN                  -127
#define SR_PARA_PD_MIN_DEFAULT              -82
#define SR_PARA_PERIOD_OFST_MAX             63
#define SR_PARA_PERIOD_OFST_MIN             0
#define SR_PARA_PERIOD_OFST_DEFAULT         SR_PARA_PERIOD_OFST_MIN
#define SR_PARA_OBSS_TXPWR_REF_MAX          30
#define SR_PARA_OBSS_TXPWR_REF_MIN          0
#define SR_PARA_OBSS_TXPWR_REF_DEFAULT      21

#define SR_COND_PERIOD_LIMIT_MAX            255
#define SR_COND_PERIOD_LIMIT_MIN            0
#define SR_RCPI_MAX                         0
#define SR_RCPI_MIN                         -110
#define SR_RCPI_DEFAULT                     SR_RCPI_MAX

/** ARG Number**/

/* SR_V1 & SR_V2 Conflict Part*/
#define SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V1             12
#define SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2             20

#define SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V1            10
#define SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2            12

#define SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V1   7
#define SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2   8

#define SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V1          4
#define SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2          6

#define SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V1             2
#define SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2             4

#define SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V1        6
#define SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2        7

/* SR_V2 New Feature */
#define SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM              6
#define SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM              1
#define SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM          6

/* SR_V1 & SR_V2 Common Part */
#define SR_CMD_SET_DEFAULT_ARG_NUM                           1
#define SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM                  1
#define SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM                  9
#define SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM           2
#define SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM               4
#define SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM          2
#define SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM              SR_RCPITBL_MCS_NUM
#define SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM                  5
#define SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM                 1
#define SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM                     4
#define SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM                  5
#define SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM                4
#define SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM                 2
#define SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM                   3


#define SR_CMD_GET_DEFAULT_ARG_NUM                           1
#define SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM    2
#define SR_CMD_SET_SR_DL_STA_MESH_THRESHOLD_TH_ARG_NUM	2
#define SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM	4



#define SR_SCENE_DETECTION_TIMER_PERIOD_MS  500 /* ms */
#define SR_SCENE_DETECTION_OBSS_RSP_TIME_US 100 /* us */
#define SR_SCENE_DETECTION_OBSS_AIRTIME_THR 500 /* 50.0% */
#define SR_SCENE_DETECTION_CONGESTION_THR   800 /* 80.0% */
#define SR_SCENE_DETECTION_MAC_LENGTH       17
#define SR_BSSID_OMAC_OFFSET                12  /* Omac offset */


#define SR_MESH_SCAN_COUNT_MAX               3
#define SR_MESH_PRIMARY_INTERFACE	    0x0
#define SR_MESH_AP_CLIENT		    0x1
#define SR_MESH_P2PGO			    0x2
#define SR_MESH_P2PGC			    0x3

BOOLEAN fgmeshdetect = 1;
BOOLEAN fghavebeensend = 1;
struct sr_mesh_topology_params g_rTopologyUpdate;
UINT8 g_u1CSRCoordAPMode;
UINT8 g_u1CSRStaUpdMode;

static struct {
	RTMP_STRING *name;
	UINT8 u1srflag;
} *PENUM_WH_SR_SIGA_FLAG_T, ENUM_WH_SR_SIGA_FLAG_T[] = {
	{"PSR_DISALLOW", 0},
	{"SR_RESTRICTED", 13},
	{"SR_DELAYED", 14},
	{"PSR_AND_NON_SRG_OBSS_PD_PROHIBITED", 15},
	{NULL,}
};

enum _ENUM_COSR_RESET_TYPE_T {
	ENUM_COSR_RESET_TYPE_AP,
	ENUM_COSR_RESET_TYPE_STA,
	ENUM_COSR_RESET_TYPE_NUM
};


UINT_8 SRRcpiConv(IN INT_8 i1Dbm);
INT_8 SRDbmConv(IN UINT_8 u1Rcpi);

NDIS_STATUS SrInit(RTMP_ADAPTER *pAd)
{
	if (pAd == NULL)
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&pAd->SrMeshTopologyUpdateParams, sizeof(struct sr_mesh_topology_update_params));
	os_zero_mem(&pAd->SrSelfSrgBM, sizeof(struct SR_MESH_SRG_BITMAP_T));
	os_zero_mem(&pAd->SrSelfSrgBMMan, sizeof(struct SR_MESH_SRG_BITMAP_T));
	pAd->SrSelfSrgBMMode = 0;
	pAd->SrMeshTopoLock = 0;
	pAd->SrMeshUlMode = 0;

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrRstNav(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	UINT_8 u1DbdcIdx = BAND0;
	UINT_16 u2WlanIdx = 0;
	UINT_32 u4Addr = BN0_WF_RMAC_TOP_LUNVR_RESET_ADDR, u4Val = BN0_WF_RMAC_TOP_LUNVR_RESET_MASK;
	struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(pRxBlk->rmac_info);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	u1DbdcIdx = (rxd_grp0->rxd_1 & RXD_BN) ? BAND1 : BAND0;
	u2WlanIdx = (rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT;

	if (IS_MT7915(pAd)) {
		if (u1DbdcIdx == BAND1)
			u4Addr += 0x10000;

		IO_W_32(u4Addr, u4Val);
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"Band:%d WlanIdx:%d Addr:%x Val:%x\n", u1DbdcIdx, u2WlanIdx, u4Addr, u4Val);
	}

	return NDIS_STATUS_SUCCESS;
}
NDIS_STATUS SrSwitchToApCliMode(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;

	if (wdev != NULL) {
		u1DbdcIdx = HcGetBandByWdev(wdev);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"wdev == NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/*SR not enable via profile*/
	if (pAd->CommonCfg.SREnable == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"SREnable[%d] = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.SREnable);
		return NDIS_STATUS_SUCCESS;
	}

	/*Check DisSrBfrConnected avoid periodically send cmd*/
	if (pAd->CommonCfg.fgSrApCliMode == TRUE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"DisSrBfrConnected[%d] = %d == fgSrEnable = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.fgSrApCliMode, fgSrEnable);
		return NDIS_STATUS_SUCCESS;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
	"u1DbdcIdx = %d, fgSrEnable = %d\n", u1DbdcIdx, fgSrEnable);

	if (fgSrEnable == FALSE) {

		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_SR_APCLI_MODE, u1DbdcIdx, TRUE, TRUE);

		/*Update fgSrApCliMode*/
		pAd->CommonCfg.fgSrApCliMode = TRUE;
	}
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS SrDisSrBfrConnected(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	struct SR_CMD_T rSrCmd;

	if (wdev != NULL) {
		u1DbdcIdx = HcGetBandByWdev(wdev);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"wdev == NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/*SR not enable via profile*/
	if (pAd->CommonCfg.SREnable == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"SREnable[%d] = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.SREnable);
		return NDIS_STATUS_SUCCESS;
	}

	/*Check DisSrBfrConnected avoid periodically send cmd*/
	if (pAd->CommonCfg.DisSrBfrConnected == fgSrEnable) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"DisSrBfrConnected[%d] = %d == fgSrEnable = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.DisSrBfrConnected, fgSrEnable);
		return NDIS_STATUS_SUCCESS;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
	"u1DbdcIdx = %d, fgSrEnable = %d\n", u1DbdcIdx, fgSrEnable);

	if (fgSrEnable == FALSE) {

		/* Disable SRSDEnable First */
		os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = FALSE;

#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_SD_ENABLE, u1DbdcIdx, FALSE, TRUE);
#else
		Status = SrCmd(pAd, &rSrCmd);
#endif
		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx, FALSE);
			return NDIS_STATUS_FAILURE;
		}

		/* Disable SREnable */
		os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_ENABLE, u1DbdcIdx, FALSE, TRUE);
#else
		Status = SrCmd(pAd, &rSrCmd);
#endif
		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Set SREnable[%d]=%d Fail!\n", u1DbdcIdx, FALSE);
			return NDIS_STATUS_FAILURE;
		}
		/*Update DisSrBfrConnected*/
		pAd->CommonCfg.DisSrBfrConnected = FALSE;
	} else {
		/*Restore Profile Setting*/
		/* Set SREnable Part */
		os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = pAd->CommonCfg.SREnable;
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_ENABLE, u1DbdcIdx, pAd->CommonCfg.SREnable, TRUE);
#else
		Status = SrCmd(pAd, &rSrCmd);
#endif

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Set SREnable[%d]=%d Fail!\n",  u1DbdcIdx,
			  pAd->CommonCfg.SREnable);
			return NDIS_STATUS_FAILURE;
		}
		/* End - Set SREnable Part */

		/*Update DisSrBfrConnected*/
		pAd->CommonCfg.DisSrBfrConnected = pAd->CommonCfg.SREnable;

		/* Set SRSDEnable Part */
		os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = pAd->CommonCfg.SRSDEnable;
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_SD_ENABLE, u1DbdcIdx, pAd->CommonCfg.SRSDEnable, TRUE);
#else
		Status = SrCmd(pAd, &rSrCmd);
#endif


		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx,
			  pAd->CommonCfg.SRSDEnable);
			return NDIS_STATUS_FAILURE;
		}
		/* End - Set SRSDEnable Part */
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

	u1BandNum = 1;
	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SREnable = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

	u1BandNum = 1;
	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRMode = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

	u1BandNum = 1;
	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRSDEnable = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

	u1BandNum = 1;
	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRDPDEnable = simple_strtol(ptr, 0, 10);

	}
	return Status;
}


NDIS_STATUS SrMbssInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	struct SR_CMD_T rSrCmd;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	/* ap.c will call this command enable SR by profile */
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		 "Spatial Reuse initialize via profile.\n");

	/*Init DisSrBfrConnected */
	pAd->CommonCfg.DisSrBfrConnected = pAd->CommonCfg.SREnable;

	/* Set SREnable Part */
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SREnable;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_ENABLE, u1DbdcIdx, pAd->CommonCfg.SREnable, TRUE);
#else
	Status = SrCmd(pAd, &rSrCmd);
#endif

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		 "Set SREnable[%d]=%d Fail!\n",  u1DbdcIdx,
		  pAd->CommonCfg.SREnable);
	}
	/* End - Set SREnable Part */

	/* Set SRMode Part */
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_MODE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRMode;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_MODE, u1DbdcIdx, pAd->CommonCfg.SRMode, TRUE);
#else
	Status = SrCmd(pAd, &rSrCmd);
#endif

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		 "Set SRMode[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRMode);
	}
	/* End - Set SRMode Part */

	/* Set SRSDEnable Part */
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRSDEnable;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_SD_ENABLE, u1DbdcIdx, pAd->CommonCfg.SRSDEnable, TRUE);
#else
	Status = SrCmd(pAd, &rSrCmd);
#endif

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRSDEnable);
	}
	/* End - Set SRSDEnable Part */

	/* Set SRDPDEnable Part */
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_DPD_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRDPDEnable;
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_DPD_ENABLE, u1DbdcIdx, pAd->CommonCfg.SRDPDEnable, TRUE);
#else
		Status = SrCmd(pAd, &rSrCmd);
#endif

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		 "Set SRDPDEnable[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRDPDEnable);
	}
	/* End - Set SRDPDEnable Part */

	/* Init mac for test */
	COPY_MAC_ADDR(g_rTopologyUpdate.map_remote_bh_mac, pAd->CurrentAddress); /* from AP1 */
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrMeshGetSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pMeshSrgBitmap)
{
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap;

	if (band_idx >= MAX_NUM_BAND)
		return NDIS_STATUS_FAILURE;

	prSrgBitmap = &pAd->SrSelfSrgBM;

	os_move_mem(pMeshSrgBitmap, prSrgBitmap, sizeof(*prSrgBitmap));

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"Color:[63_32][%x]-[31_0][%x] Bssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrMeshGetSrMode(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pu1SrMode)
{
	if (band_idx >= MAX_NUM_BAND)
		return NDIS_STATUS_FAILURE;

	*pu1SrMode = pAd->CommonCfg.SRMode;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG, "SrMode:%u\n", *pu1SrMode);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrMeshSrEnable(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN UINT_8 SrEnable)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = band_idx;
	struct SR_CMD_T rSrCmd;

	pAd->CommonCfg.SREnable = SrEnable;
	/* Set SREnable Part */
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SREnable;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_CFG_SR_ENABLE, u1DbdcIdx, pAd->CommonCfg.SREnable, TRUE);
#else
	Status = SrCmd(pAd, &rSrCmd);
#endif

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "SrMode:%u\n", SrEnable);

	return Status;
}

NDIS_STATUS SrBHMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx,
								IN PUINT_8 prSrBhMeshSrgBitmap)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = band_idx;
	struct SR_MESH_SRG_BITMAP_T *prSrMeshSrgBitmap = (struct SR_MESH_SRG_BITMAP_T *)prSrBhMeshSrgBitmap;
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#endif

#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrMeshSrgBitmap.u2Tag = UNI_CMD_SR_BH_MESH_SR_BITMAP;
	rSrCmdSrMeshSrgBitmap.u2Length = sizeof(rSrCmdSrMeshSrgBitmap);
#else
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = prSrMeshSrgBitmap->u4Color_31_0;
	rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = prSrMeshSrgBitmap->u4Color_63_32;
	rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = prSrMeshSrgBitmap->u4pBssid_31_0;
	rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = prSrMeshSrgBitmap->u4pBssid_63_32;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrMeshSrgBitmap, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
#endif
		Status = NDIS_STATUS_FAILURE;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"Update Backhaul SR Mesh SRG Bitmap from Mesh System.\n");

	return Status;
}

NDIS_STATUS SrMeshSrgBitMapControl(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 pMeshSrgBitmap)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	struct SR_MESH_SRG_BITMAP_T *prSrMeshSrgBitmap = (struct SR_MESH_SRG_BITMAP_T *)pMeshSrgBitmap;
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#endif

	if (wdev != NULL && prSrMeshSrgBitmap != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (fgSet) {
#ifdef WIFI_UNIFIED_COMMAND
		os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct UNI_CMD_SR_SW_SRG_BITMAP_T));
#else
		os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
#endif
		/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
		rSrCmdSrMeshSrgBitmap.u2Tag = UNI_CMD_SR_SW_MESH_SRG_BITMAP;
		rSrCmdSrMeshSrgBitmap.u2Length = sizeof(rSrCmdSrMeshSrgBitmap);
#else
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = prSrMeshSrgBitmap->u4Color_31_0;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = prSrMeshSrgBitmap->u4Color_63_32;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = prSrMeshSrgBitmap->u4pBssid_31_0;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = prSrMeshSrgBitmap->u4pBssid_63_32;

#ifdef WIFI_UNIFIED_COMMAND
		if (UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrMeshSrgBitmap, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
#else
		if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
#endif
			Status = FALSE;

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
			"Update SR Mesh SRG Bitmap from Mesh System.\n");
	} else {
		struct SR_CMD_T rSrCmd;

		os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));

		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_GET_SR_MESH_SRG_BITMAP;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;

		Status = SrCmd(pAd, &rSrCmd);

		/* Please put your receiving function in the case SR_EVENT_GET_SR_MESH_SRG_BITMAP in EventSrHandler */

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
			"Get SR Mesh SRG Bitmap from FW.\n");
	}

	return Status;
}

NDIS_STATUS SrFindBHWcid(PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, PUINT_8 pBHMac)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	UINT_8 BhType = ENUM_BH_TYPE_NO_WIFI;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT_16 wcid = 0;
	struct wifi_dev *wdev = NULL;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(pBHMac));

	if (pBHMac) {
		if (!MAC_ADDR_EQUAL(pBHMac, BROADCAST_ADDR) && !MAC_ADDR_EQUAL(pBHMac, ZERO_MAC_ADDR)) {
			pEntry = MacTableLookup(pAd, pBHMac);

			if (pEntry) {
				wdev = pEntry->wdev;
				if (wdev) {
					if (HcGetBandByWdev(wdev) == u1DbdcIdx) {
						wcid = pEntry->wcid;
						BhType = ENUM_BH_TYPE_WIFI;
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
							"Band %d STA found with wcid:%u\n", u1DbdcIdx, wcid);
					}
				}
			}
		}
	}

	Status = SrSetRemoteAssocBHInfo(pAd, u1DbdcIdx, BhType, wcid);

	return Status;
}

NDIS_STATUS SrMeshTopologyUpdate(IN PRTMP_ADAPTER pAd, IN PUINT_8 pTopologyUpdate, IN UINT8 Band_Idx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct sr_mesh_topology_params *prSrMeshTopologyParams = (struct sr_mesh_topology_params *)pTopologyUpdate;
	struct sr_mesh_topology_update_params *prParams;

	if ((pAd->bMapR3Enable == 0) || (pAd->CommonCfg.SRMode == 0)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
			"MapR3Enable:%u SRMode:%u Band:%u\n",
			pAd->bMapR3Enable, pAd->CommonCfg.SRMode, Band_Idx);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
		"MapDevCnt:%u MapDevSrSupportMode:%u SelfRole:%u\n",
		prSrMeshTopologyParams->map_dev_count,
		prSrMeshTopologyParams->map_dev_sr_support_mode,
		prSrMeshTopologyParams->self_role);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
		"FH BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prSrMeshTopologyParams->map_remote_fh_bssid));

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
		"BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prSrMeshTopologyParams->map_remote_bh_mac));

	if (pAd->SrMeshTopoLock == ENUM_SR_TOPO_LOCK_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
			"Topolock:%u Manual for band:%u\n",
			pAd->SrMeshTopoLock, Band_Idx);
		return NDIS_STATUS_FAILURE;
	}

	prParams = &pAd->SrMeshTopologyUpdateParams;

	NdisCopyMemory(&prParams->topo_params, prSrMeshTopologyParams, sizeof(*prSrMeshTopologyParams));
	prParams->scan_start = TRUE;
	prParams->skip_scan = FALSE;
	prParams->wdev = wdev;
	prParams->scan_fail = TRUE;
	prParams->scan_rssi = -127;

	if (MAC_ADDR_EQUAL(prSrMeshTopologyParams->map_remote_fh_bssid, BROADCAST_ADDR)
		|| MAC_ADDR_EQUAL(prSrMeshTopologyParams->map_remote_fh_bssid, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
			"No need to scan for the band:%u!\n", Band_Idx);
		prParams->skip_scan = TRUE;
		prParams->scan_fail = TRUE;
	}

	if (prParams->skip_scan == TRUE)
		SrMeshTopologyUpdatePerBand(pAd, Band_Idx);
#endif

	return Status;
}

VOID SrMeshTopologyUpdatePeriodic(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 bandidx = 0;

	if (pAd->CommonCfg.SRMode == 1)
		SrMeshTopologyUpdatePerBand(pAd, bandidx);
}

VOID SrMeshTopologyUpdateBcnRssi(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list)
{
	RSSI_SAMPLE rssi_sample;
	struct sr_mesh_topology_update_params *prParams;
	CHAR RealRssi;

	prParams = &pAd->SrMeshTopologyUpdateParams;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"band %d : MAC Addr Given:%02X:%02X:%02X:%02X:%02X:%02X\nMAC Addr Bcn:%02X:%02X:%02X:%02X:%02X:%02X\n",
		HcGetBandByWdev(wdev),
		PRINT_MAC(prParams->topo_params.map_remote_fh_bssid),
		PRINT_MAC(ie_list->Addr2));

	if (MAC_ADDR_EQUAL(prParams->topo_params.map_remote_fh_bssid, ie_list->Addr2)) {
		rssi_sample.AvgRssi[0] = Elem->rssi_info.raw_rssi[0];
		rssi_sample.AvgRssi[1] = Elem->rssi_info.raw_rssi[1];
		rssi_sample.AvgRssi[2] = Elem->rssi_info.raw_rssi[2];
		rssi_sample.AvgRssi[3] = Elem->rssi_info.raw_rssi[3];
		RealRssi = RTMPAvgRssi(pAd, &rssi_sample);
		prParams->scan_rssi = RealRssi;
		prParams->scan_fail = FALSE;
		prParams->scan_count++;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			"band %d : MAC Addr Match: RSSI %d %02X:%02X:%02X:%02X:%02X:%02X\n",
			HcGetBandByWdev(wdev), prParams->scan_rssi, PRINT_MAC(ie_list->Addr2));
	}
}

VOID SrMeshTopologyUpdatePerBand(struct _RTMP_ADAPTER *pAd, UINT_8 u1DbdcIdx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct sr_mesh_topology_update_params *prParams;
	INT_8 Rssi = -127;
	UINT_8 u1RemoteFhStat = ENUM_RMT_FH_SCAN_FAIL;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"Band:%u for scan\n", u1DbdcIdx);

	prParams = &pAd->SrMeshTopologyUpdateParams;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"scan_fail:%u skip_scan:%u scan_count:%u\n",
		prParams->scan_fail, prParams->skip_scan, prParams->scan_count);

	if (pAd->SrMeshTopoLock == ENUM_SR_TOPO_LOCK_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			"Topolock:%u Manual for band:%u\n",
			pAd->SrMeshTopoLock, u1DbdcIdx);
		goto error;
	}

	if (prParams->scan_start == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			"Band:%u no topology update triggered yet\n", u1DbdcIdx);
		goto error;
	}

	if (prParams->skip_scan == TRUE) {
		u1RemoteFhStat = ENUM_RMT_FH_INVLD_BSSID;
		Rssi = -127;
	} else if (prParams->scan_fail == TRUE) {
		u1RemoteFhStat = ENUM_RMT_FH_SCAN_FAIL;
		Rssi = -127;
	} else {/* scan success && not skip scan */
		u1RemoteFhStat = ENUM_RMT_FH_SCAN_SUCCESS;
		Rssi = prParams->scan_rssi;
	}

	Status = SrSetRemoteFHRssi(pAd, u1DbdcIdx, Rssi, u1RemoteFhStat);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"SrSetRemoteFHRssi Failed for band:%u!\n",
			u1DbdcIdx);
		goto error;
	}

	Status = SrFindBHWcid(pAd, u1DbdcIdx,
		prParams->topo_params.map_remote_bh_mac);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"SrFindBHWcid Failed for band:%u!\n",
			u1DbdcIdx);
		goto error;
	}

	Status = IsInRange(prParams->topo_params.map_dev_sr_support_mode, 0, 0, 2);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Band:%u MapDevSrSupportMode:%u should be [0,2]!\n",
			u1DbdcIdx, prParams->topo_params.map_dev_sr_support_mode);
		goto error;
	}

	Status = SrSetMAPTopo(pAd, u1DbdcIdx,
		prParams->topo_params.map_dev_count,
		prParams->topo_params.map_dev_sr_support_mode,
		prParams->topo_params.self_role);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"SrSetMAPTopo Failed for band:%u MapDevCnt:%u MapDevSrSupportMode:%u SelfRole:%u!\n",
			u1DbdcIdx, prParams->topo_params.map_dev_count,
			prParams->topo_params.map_dev_sr_support_mode,
			prParams->topo_params.self_role);
		goto error;
	}

	Status = SrSetSrgBitmapRefresh(pAd, u1DbdcIdx);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"SrSetSrgBitmapRefresh Failed for band:%u!\n",
			u1DbdcIdx);
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"Update SR Mesh Topology from Mesh System.\n");

error:

	prParams->scan_count = 0;
	prParams->scan_fail = TRUE;
}

NDIS_STATUS SrSetSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_SW_SRG_BITMAP_REFRESH, u1DbdcIdx, 0, TRUE);
#else
	struct SR_CMD_T rSrCmd;
	struct SR_CMD_T *prSrcmd = &rSrCmd;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SRG_BITMAP_REFRESH;
	rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = 0;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_T));
	if (!msg) {
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrcmd, sizeof(struct SR_CMD_T));
	Status = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"(Status = %d)\n", Status);
#endif/* WIFI_UNIFIED_COMMAND */
	return Status;
}

NDIS_STATUS SetSrCapSrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 u4SrEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V1 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_SREN_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {

					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}

					if (Status == NDIS_STATUS_FAILURE)
						break;
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;

		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcapsren=[SrEn]\n");
		}
	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		UINT_8 isSet = TRUE;
#else
		struct SR_CMD_SR_CAP_T_SR_V2 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_SREN_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
				Status = UniSrCmd(pAd, UNI_CMD_SR_HW_CAP_SREN, u1DbdcIdx, u4SrEn, isSet);
#else
				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
				rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
				Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;

		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcapsren=[SrEn]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

INT SetSrCapAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrEn = 0, u4SrgEn = 0, u4NonSrgEn = 0;
	UINT_32 u4SingleMdpuRtsctsEn = 0, u4HdrDurEn = 0, u4TxopDurEn = 0;
	UINT_32 u4NonSrgInterPpduPresv = 0, u4SrgInterPpduPresv = 0, u4SMpduNoTrigEn = 0;
	UINT_32 u4SrgBssidOrder = 0, u4CtsAfterRts = 0, u4SrpOldRxvEn = 0;
	UINT_32 u4SrpNewRxvEn = 0, u4SrpDataOnlyEn = 0, u4FixedRateSrREn = 0;
	UINT_32 u4WtblSrREn = 0, u4SrRemTimeEn = 0, u4ProtInSrWinDis = 0;
	UINT_32  u4TxCmdDlRateSelEn = 0, u4AmpduTxCntEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V1 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_ALL_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 12);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					case 1:
						u4SrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgEn, u1ArgNum);
						break;
					case 2:
						u4NonSrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgEn, u1ArgNum);
						break;
					case 3:
						u4SingleMdpuRtsctsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SingleMdpuRtsctsEn, u1ArgNum);
						break;
					case 4:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 5:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 6:
						u4NonSrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgInterPpduPresv, u1ArgNum);
						break;
					case 7:
						u4SrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgInterPpduPresv, u1ArgNum);
						break;
					case 8:
						u4SrRemTimeEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRemTimeEn, u1ArgNum);
						break;
					case 9:
						u4ProtInSrWinDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4ProtInSrWinDis, u1ArgNum);
						break;
					case 10:
						u4TxCmdDlRateSelEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxCmdDlRateSelEn, u1ArgNum);
						break;
					case 11:
						u4AmpduTxCntEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4AmpduTxCntEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					rSrCmdSrCap.rSrCap.fgSrgEn = u4SrgEn;
					rSrCmdSrCap.rSrCap.fgNonSrgEn = u4NonSrgEn;
					rSrCmdSrCap.rSrCap.fgSingleMdpuRtsctsEn = u4SingleMdpuRtsctsEn;
					rSrCmdSrCap.rSrCap.fgHdrDurEn = u4HdrDurEn;
					rSrCmdSrCap.rSrCap.fgTxopDurEn = u4TxopDurEn;
					rSrCmdSrCap.rSrCap.fgNonSrgInterPpduPresv = u4NonSrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrgInterPpduPresv = u4SrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrRemTimeEn = u4SrRemTimeEn;
					rSrCmdSrCap.rSrCap.fgProtInSrWinDis = u4ProtInSrWinDis;
					rSrCmdSrCap.rSrCap.fgTxCmdDlRateSelEn = u4TxCmdDlRateSelEn;
					rSrCmdSrCap.rSrCap.fgAmpduTxCntEn = u4AmpduTxCntEn;

					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcap=[SrEn]-[SrgEn]-[NonSrgEn]-[SingleMdpuRtsctsEn]-[HdrDurEn]-[TxopDurEn]-[NonSrgInterPpduPresv]-[SrgInterPpduPresv]-[SrRemTimeEn]-[ProtInSrWinDis]-[TxCmdDlRateSelEn]-[AmpduTxCntEn]\n"
				 );
		}

	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_HW_CAP_T rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(struct UNI_CMD_SR_HW_CAP_T));

		/* Assign Cmd Id */
		rSrCmdSrCap.u2Tag = UNI_CMD_SR_HW_CAP;
		rSrCmdSrCap.u2Length = sizeof(struct UNI_CMD_SR_HW_CAP_T);
#else
		struct SR_CMD_SR_CAP_T_SR_V2 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_ALL_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 20);
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					case 1:
						u4SrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgEn, u1ArgNum);
						break;
					case 2:
						u4NonSrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgEn, u1ArgNum);
						break;
					case 3:
						u4SingleMdpuRtsctsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SingleMdpuRtsctsEn, u1ArgNum);
						break;
					case 4:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 5:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 6:
						u4NonSrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgInterPpduPresv, u1ArgNum);
						break;
					case 7:
						u4SrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgInterPpduPresv, u1ArgNum);
						break;
					case 8:
						u4SMpduNoTrigEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SMpduNoTrigEn, u1ArgNum);
						break;
					case 9:
						u4SrgBssidOrder = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgBssidOrder, u1ArgNum);
						break;
					case 10:
						u4CtsAfterRts = simple_strtol(value, 0, 10);
						Status = IsFlag(u4CtsAfterRts, u1ArgNum);
						break;
					case 11:
						u4SrpOldRxvEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpOldRxvEn, u1ArgNum);
						break;
					case 12:
						u4SrpNewRxvEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpNewRxvEn, u1ArgNum);
						break;
					case 13:
						u4SrpDataOnlyEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpDataOnlyEn, u1ArgNum);
						break;
					case 14:
						u4FixedRateSrREn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4FixedRateSrREn, u1ArgNum);
						break;
					case 15:
						u4WtblSrREn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4WtblSrREn, u1ArgNum);
						break;
					case 16:
						u4SrRemTimeEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRemTimeEn, u1ArgNum);
						break;
					case 17:
						u4ProtInSrWinDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4ProtInSrWinDis, u1ArgNum);
						break;
					case 18:
						u4TxCmdDlRateSelEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxCmdDlRateSelEn, u1ArgNum);
						break;
					case 19:
						u4AmpduTxCntEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4AmpduTxCntEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#else
				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						" Format Error! ArgNum = %d != %d\n",
						u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					rSrCmdSrCap.rSrCap.fgSrgEn = u4SrgEn;
					rSrCmdSrCap.rSrCap.fgNonSrgEn = u4NonSrgEn;
					rSrCmdSrCap.rSrCap.fgSingleMdpuRtsctsEn = u4SingleMdpuRtsctsEn;
					rSrCmdSrCap.rSrCap.fgHdrDurEn = u4HdrDurEn;
					rSrCmdSrCap.rSrCap.fgTxopDurEn = u4TxopDurEn;
					rSrCmdSrCap.rSrCap.fgNonSrgInterPpduPresv = u4NonSrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrgInterPpduPresv = u4SrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSMpduNoTrigEn = u4SMpduNoTrigEn;
					rSrCmdSrCap.rSrCap.fgSrgBssidOrder = u4SrgBssidOrder;
					rSrCmdSrCap.rSrCap.fgCtsAfterRts = u4CtsAfterRts;
					rSrCmdSrCap.rSrCap.fgSrpOldRxvEn = u4SrpOldRxvEn;
					rSrCmdSrCap.rSrCap.fgSrpNewRxvEn = u4SrpNewRxvEn;
					rSrCmdSrCap.rSrCap.fgSrpDataOnlyEn = u4SrpDataOnlyEn;
					rSrCmdSrCap.rSrCap.fgFixedRateSrREn = u4FixedRateSrREn;
					rSrCmdSrCap.rSrCap.fgWtblSrREn = u4WtblSrREn;
					rSrCmdSrCap.rSrCap.fgSrRemTimeEn = u4SrRemTimeEn;
					rSrCmdSrCap.rSrCap.fgProtInSrWinDis = u4ProtInSrWinDis;
					rSrCmdSrCap.rSrCap.fgTxCmdDlRateSelEn = u4TxCmdDlRateSelEn;
					rSrCmdSrCap.rSrCap.fgAmpduTxCntEn = u4AmpduTxCntEn;
#ifdef WIFI_UNIFIED_COMMAND
					Status = UniCmdSRUpdateCap(pAd, &rSrCmdSrCap, u1DbdcIdx);
#else
					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcap=[SrEn]-[SrgEn]-[NonSrgEn]-[SingleMdpuRtsctsEn]-[HdrDurEn]-[TxopDurEn]\n"
				 "-[NonSrgInterPpduPresv]-[SrgInterPpduPresv]-[SMpduNoTrigEn]-[SrgBssidOrder]-[CtsAfterRts]\n"
				 "-[SrpOldRxvEn]-[SrpNewRxvEn]-[SrpDataOnlyEn]-[FixedRateSrREn]-[WtblSrREn]\n"
				 "-[SrRemTimeEn]-[ProtInSrWinDis]-[TxCmdDlRateSelEn]-[AmpduTxCntEn]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT SetSrParaAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 i4NonSrgPdThr = 0, i4SrgPdThr = 0;
	UINT_32 u4PeriodOfst = 0, u4RcpiSourceSel = 0;
	INT32 i4ObssPdMin = 0, i4ObssPdMinSrg = 0;
	UINT_32 u4RespTxPwrMode = 0, u4TxPwrRestricMode = 0, u4ObssTxPwrRef = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_SR_CMD_SR_PARA_T rSrCmdSrPara;

	os_zero_mem(&rSrCmdSrPara, sizeof(struct UNI_SR_CMD_SR_PARA_T));
	rSrCmdSrPara.u2Tag = UNI_CMD_SR_HW_PARA;
	rSrCmdSrPara.u2Length = sizeof(struct UNI_SR_CMD_SR_PARA_T);
#else
	struct SR_CMD_SR_PARA_T rSrCmdSrPara;

	os_zero_mem(&rSrCmdSrPara, sizeof(struct SR_CMD_SR_PARA_T));

	/* Assign Cmd Id */
	rSrCmdSrPara.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_PARA_ALL_CTRL;
	rSrCmdSrPara.rSrCmd.u1ArgNum = SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM;
	rSrCmdSrPara.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 9);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					i4NonSrgPdThr = simple_strtol(value, 0, 10);
					i4NonSrgPdThr = i4NonSrgPdThr * (-1);
					Status =
					    IsInRange(i4NonSrgPdThr, u1ArgNum, SR_PARA_PD_THR_MIN,
						      SR_PARA_PD_THR_MAX);
					break;
				case 1:
					i4SrgPdThr = simple_strtol(value, 0, 10);
					i4SrgPdThr = i4SrgPdThr * (-1);
					Status =
					    IsInRange(i4SrgPdThr, u1ArgNum, SR_PARA_PD_THR_MIN,
						      SR_PARA_PD_THR_MAX);

					break;
				case 2:
					u4PeriodOfst = simple_strtol(value, 0, 16);
					Status =
					    IsInRange(u4PeriodOfst, u1ArgNum,
						      SR_PARA_PERIOD_OFST_MIN,
						      SR_PARA_PERIOD_OFST_MAX);
					break;
				case 3:
					u4RcpiSourceSel = simple_strtol(value, 0, 16);
					Status =
					    IsInRange(u4RcpiSourceSel, u1ArgNum,
						      SR_PARA_RCPI_SRC_SEL_ANT_0,
						      SR_PARA_RCPI_SRC_SEL_ANT_3);
					break;
				case 4:
					i4ObssPdMin = simple_strtol(value, 0, 10);
					i4ObssPdMin = i4ObssPdMin * (-1);
					Status =
					    IsInRange(i4ObssPdMin, u1ArgNum, SR_PARA_PD_MIN_MIN,
						      SR_PARA_PD_MIN_MAX);
					break;
				case 5:
					i4ObssPdMinSrg = simple_strtol(value, 0, 10);
					i4ObssPdMinSrg = i4ObssPdMinSrg * (-1);
					Status =
					    IsInRange(i4ObssPdMinSrg, u1ArgNum, SR_PARA_PD_MIN_MIN,
						      SR_PARA_PD_MIN_MAX);
					break;
				case 6:
					u4RespTxPwrMode = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4RespTxPwrMode, u1ArgNum,
						      WH_ENUM_SR_RESP_TXPWR_NO_RESTRIC,
						      WH_ENUM_SR_RESP_TXPWR_RESTRIC);
					break;
				case 7:
					u4TxPwrRestricMode = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4TxPwrRestricMode, u1ArgNum,
						      WH_ENUM_SR_TXPWR_RESTRIC_NO_RESTRIC,
						      WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_MTK);
					break;
				case 8:
					u4ObssTxPwrRef = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4ObssTxPwrRef, u1ArgNum,
						      SR_PARA_OBSS_TXPWR_REF_MIN,
						      SR_PARA_OBSS_TXPWR_REF_MAX);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG,
							CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != rSrCmdSrPara.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrPara.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
				rSrCmdSrPara.rSrPara.u1NonSrgPdThr = i4NonSrgPdThr;
				rSrCmdSrPara.rSrPara.u1SrgPdThr = i4SrgPdThr;
				rSrCmdSrPara.rSrPara.u1PeriodOfst = u4PeriodOfst;
				rSrCmdSrPara.rSrPara.u1RcpiSourceSel = u4RcpiSourceSel;
				rSrCmdSrPara.rSrPara.u2ObssPdMin = i4ObssPdMin;
				rSrCmdSrPara.rSrPara.u2ObssPdMinSrg = i4ObssPdMinSrg;
				rSrCmdSrPara.rSrPara.eRespTxPwrMode = u4RespTxPwrMode;
				rSrCmdSrPara.rSrPara.eTxPwrRestricMode = u4TxPwrRestricMode;
				rSrCmdSrPara.rSrPara.u1ObssTxPwrRef = u4ObssTxPwrRef;

#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdatePara(pAd, &rSrCmdSrPara, u1DbdcIdx);
#else
				Status = SrCmdSRUpdatePara(pAd, &rSrCmdSrPara);
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srpara=[NonSrgPdThr]-[SrgPdThr]-[PeriodOfst]-[RcpiSourceSel]-[ObssPdMin]-[ObssPdMinSrg]-[RespTxPwrMode]-[TxPwrRestricMode]-[ObssTxPwrRef]\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL **/
INT SetSrDropTa(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
#ifdef WIFI_UNIFIED_COMMAND
	UINT_32 u4DropTaIdx = 0, u4Address2 = 0;
	struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T *prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T));
#else
	UINT_32 u4DropTaIdx = 0, u4Address2 = 0, u4StaIdx = 0;
	struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
#endif

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T));
	prSrCmdSrGlobalVarSingleDropTa->u2Tag = UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO;
	prSrCmdSrGlobalVarSingleDropTa->u2Length = sizeof(struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T);
#else
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 2);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4Address2 = simple_strtol(value, 0, 16);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			prSrCmdSrGlobalVarSingleDropTa->u1DropTaIdx = u4DropTaIdx;
			prSrCmdSrGlobalVarSingleDropTa->u4Address2 = u4Address2;
#ifdef CFG_BIG_ENDIAN
			prSrCmdSrGlobalVarSingleDropTa->u4Address2 =
				cpu2le32(prSrCmdSrGlobalVarSingleDropTa->u4Address2);
#endif
				/** Set    GlobalVar **/
			Status =
				UniCmdSRUpdateGloVarSingleDropTa(pAd,
						prSrCmdSrGlobalVarSingleDropTa,
						u1DbdcIdx);
#else
			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.u4Address2 = u4Address2;
#ifdef CFG_BIG_ENDIAN
		prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.u4Address2 =
			cpu2le32(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.u4Address2);
#endif
				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
#endif//WIFI_UNIFIED_COMMAND
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 " iwpriv ra0 set srdropta=[u4DropTaIdx]-[u4Address2]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_CTRL **/
INT SetSrSta(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniCmdSRUpdateGloVarStaInfo(pAd, arg, SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM, FALSE);
#else
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4DropTaIdx = 0, u4StaIdx = 0, u4SrRateOffset = 0, u4WlanId = 0;

	struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_STA_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 4);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4StaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all STA SR_STA_NUM - 1**/
					Status =
					    IsInRange(u4StaIdx, u1ArgNum, 0, SR_STA_NUM - 1);
					break;
				case 2:
					u4WlanId = simple_strtol(value, 0, 10);
					Status = IsInRange(u4WlanId, u1ArgNum, 0, 256);
					break;
				case 3:
					u4SrRateOffset = simple_strtol(value, 0, 10);
					Status =
						IsInRange(u4SrRateOffset, u1ArgNum, 0, 11);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				/** Update GlobalVar **/
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1StaIdx = u4StaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId = u4WlanId;
#ifdef CFG_BIG_ENDIAN
	prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId =
		cpu2le16(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId);
#endif
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u1Mode =
				    ENUM_SR_STA_MODE_FIXED;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u1SrRateOffset =
				    u4SrRateOffset;

				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srsta=[u4DropTaIdx]-[u4StaIdx]-[u4WlanId]-[u4SrRateOffset]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
#endif
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL **/
INT SetSrStaInit(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniCmdSRUpdateGloVarStaInfo(pAd, arg, SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM, TRUE);
#else
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4DropTaIdx = 0, u4StaIdx = 0;

	struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 2);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support init all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4StaIdx = simple_strtol(value, 0, 10);
					/** Support init all STA SR_STA_NUM**/
					Status =
					    IsInRange(u4StaIdx, u1ArgNum, 0, SR_STA_NUM);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				/** Update GlobalVar **/
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1StaIdx = u4StaIdx;
				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srstainit=[u4DropTaIdx]-[u4StaIdx]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
#endif/* WIFI_UNIFIED_COMMAND */
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_COND_ALL_CTRL **/
INT SetSrCondAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRcpiSel      = 0, u4SrRcpiCckRateEn = 0, u4SrMacRcpiRateEn = 0, u4SrRxvRcpiRateEn = 0;
	UINT_32 u4SrRcpiHeRateEn = 0, u4SrRcpiVhtRateEn = 0, u4SrRcpiHtRateEn  = 0, u4SrRcpiLgRateEn  = 0;
	UINT_32 u4SrRxvEntry     = 0, u4SrPeriodLimitEn = 0, u4SrPeriodLimit   = 0, u4SrRcpiEhtRateEn;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_COND_T_SR_V1 rSrCmdSrCond;

		os_zero_mem(&rSrCmdSrCond, sizeof(struct SR_CMD_SR_COND_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCond.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_COND_ALL_CTRL;
		rSrCmdSrCond.rSrCmd.u1ArgNum = SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrCond.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 10);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRcpiCckRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiCckRateEn, u1ArgNum);
						break;
					case 1:
						u4SrMacRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrMacRcpiRateEn, u1ArgNum);
						break;
					case 2:
						u4SrRxvRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvRcpiRateEn, u1ArgNum);
						break;
					case 3:
						u4SrRcpiHeRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHeRateEn, u1ArgNum);
						break;
					case 4:
						u4SrRcpiVhtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiVhtRateEn, u1ArgNum);
						break;
					case 5:
						u4SrRcpiHtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHtRateEn, u1ArgNum);
						break;
					case 6:
						u4SrRcpiLgRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiLgRateEn, u1ArgNum);
						break;
					case 7:
						u4SrRxvEntry = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvEntry, u1ArgNum);
						break;
					case 8:
						u4SrPeriodLimitEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrPeriodLimitEn, u1ArgNum);
						break;
					case 9:
						u4SrPeriodLimit = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrPeriodLimit, u1ArgNum, SR_COND_PERIOD_LIMIT_MIN,
							      SR_COND_PERIOD_LIMIT_MAX);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCond.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCond.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCond.rSrCond.fgSrRcpiCckRateEn = u4SrRcpiCckRateEn;
					rSrCmdSrCond.rSrCond.fgSrMacRcpiRateEn = u4SrMacRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvRcpiRateEn = u4SrRxvRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHeRateEn = u4SrRcpiHeRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiVhtRateEn = u4SrRcpiVhtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHtRateEn = u4SrRcpiHtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiLgRateEn = u4SrRcpiLgRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvEntry = u4SrRxvEntry;
					rSrCmdSrCond.rSrCond.fgSrPeriodLimitEn = u4SrPeriodLimitEn;
					rSrCmdSrCond.rSrCond.u1SrPeriodLimit = u4SrPeriodLimit;

					Status = SrCmdSRUpdateCond(pAd, &rSrCmdSrCond);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcond=[SrRcpiCckRateEn]-[SrMacRcpiRateEn]-[SrRxvRcpiRateEn]-[SrRcpiHeRateEn]-[u4SrRcpiVhtRateEn]-[SrRcpiHtRateEn]-[SrRcpiLgRateEn]-[SrRxvEntry]-[SrPeriodLimitEn]-[SrPeriodLimit]\n");
		}
	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_COND_T_SR_V2 rSrCmdSrCond;

		os_zero_mem(&rSrCmdSrCond, sizeof(struct UNI_CMD_SR_COND_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCond.u2Tag = UNI_CMD_SR_HW_COND;
		rSrCmdSrCond.u2Length = sizeof(struct UNI_CMD_SR_COND_T_SR_V2);
#else
		struct SR_CMD_SR_COND_T_SR_V2 rSrCmdSrCond;

		os_zero_mem(&rSrCmdSrCond, sizeof(struct SR_CMD_SR_COND_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCond.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_COND_ALL_CTRL;
		rSrCmdSrCond.rSrCmd.u1ArgNum = SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrCond.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 12);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRcpiSel = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiSel, u1ArgNum);
						break;
					case 1:
						u4SrRcpiCckRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiCckRateEn, u1ArgNum);
						break;
					case 2:
						u4SrMacRcpiRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrMacRcpiRateEn, u1ArgNum);
						break;
					case 3:
						u4SrRxvRcpiRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRxvRcpiRateEn, u1ArgNum);
						break;
					case 4:
						u4SrRcpiHeRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHeRateEn, u1ArgNum);
						break;
					case 5:
						u4SrRcpiVhtRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiVhtRateEn, u1ArgNum);
						break;
					case 6:
						u4SrRcpiHtRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHtRateEn, u1ArgNum);
						break;
					case 7:
						u4SrRcpiLgRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRcpiLgRateEn, u1ArgNum);
						break;
					case 8:
						u4SrRcpiEhtRateEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRxvEntry, u1ArgNum);
						break;
					case 9:
						u4SrRxvEntry = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrRxvEntry, u1ArgNum);
						break;
					case 10:
						u4SrPeriodLimitEn = os_str_tol(value, 0, 10);
						Status = IsFlag(u4SrPeriodLimitEn, u1ArgNum);
						break;
					case 11:
						u4SrPeriodLimit = os_str_tol(value, 0, 16);
						Status = IsInRange(u4SrPeriodLimit, u1ArgNum, SR_COND_PERIOD_LIMIT_MIN,
							      SR_COND_PERIOD_LIMIT_MAX);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						" Format Error! ArgNum = %d != %d\n",
						u1ArgNum, SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#else
				if (u1ArgNum != rSrCmdSrCond.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCond.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
				rSrCmdSrCond.rSrCond.fgSrRcpiSel = u4SrRcpiSel;
				rSrCmdSrCond.rSrCond.fgSrRcpiCckRateEn = u4SrRcpiCckRateEn;
				rSrCmdSrCond.rSrCond.fgSrMacRcpiRateEn = u4SrMacRcpiRateEn;
				rSrCmdSrCond.rSrCond.fgSrRxvRcpiRateEn = u4SrRxvRcpiRateEn;
				rSrCmdSrCond.rSrCond.fgSrRcpiHeRateEn = u4SrRcpiHeRateEn;
				rSrCmdSrCond.rSrCond.fgSrRcpiVhtRateEn = u4SrRcpiVhtRateEn;
				rSrCmdSrCond.rSrCond.fgSrRcpiHtRateEn = u4SrRcpiHtRateEn;
				rSrCmdSrCond.rSrCond.fgSrRcpiLgRateEn = u4SrRcpiLgRateEn;
				rSrCmdSrCond.rSrCond.fgSrRcpiEhtRateEn = u4SrRcpiEhtRateEn;
				rSrCmdSrCond.rSrCond.fgSrRxvEntry = u4SrRxvEntry;
				rSrCmdSrCond.rSrCond.fgSrPeriodLimitEn = u4SrPeriodLimitEn;
				rSrCmdSrCond.rSrCond.u1SrPeriodLimit = u4SrPeriodLimit;
#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateCondV2(pAd, &rSrCmdSrCond, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateCond(pAd, &rSrCmdSrCond);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcond=[SrRcpiSel]-[SrRcpiCckRateEn]-[SrMacRcpiRateEn]-[SrRxvRcpiRateEn]-[SrRcpiHeRateEn]-[u4SrRcpiVhtRateEn]-[SrRcpiHtRateEn]-[SrRcpiLgRateEn]-[SrRxvEntry]-[SrPeriodLimitEn]-[SrPeriodLimit]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL **/
INT SetSrRcpiTblAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0, u1Index = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 i4RcpiTblMcs[SR_RCPITBL_MCS_NUM];
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_RCPITBL_T rSrCmdSrRcpiTbl;
#else
	struct SR_CMD_SR_RCPITBL_T rSrCmdSrRcpiTbl;
#endif
	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrRcpiTbl, sizeof(struct UNI_CMD_SR_RCPITBL_T));

	/* Assign Cmd Id */
	rSrCmdSrRcpiTbl.u2Tag = UNI_CMD_SR_HW_RCPI_TBL;
	rSrCmdSrRcpiTbl.u2Length = sizeof(struct UNI_CMD_SR_RCPITBL_T);
#else
	os_zero_mem(&rSrCmdSrRcpiTbl, sizeof(struct SR_CMD_SR_RCPITBL_T));

	/* Assign Cmd Id */
	rSrCmdSrRcpiTbl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL;
	rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM;
	rSrCmdSrRcpiTbl.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 12);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
					i4RcpiTblMcs[u1ArgNum] = simple_strtol(value, 0, 10);
					i4RcpiTblMcs[u1ArgNum] = i4RcpiTblMcs[u1ArgNum] * (-1);
					Status = IsInRange(i4RcpiTblMcs[u1ArgNum], u1ArgNum, SR_RCPI_MIN, SR_RCPI_MAX);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
#ifdef WIFI_UNIFIED_COMMAND
		if (u1ArgNum != SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				" Format Error! ArgNum = %d != %d\n",
				u1ArgNum, SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM);
			Status = NDIS_STATUS_FAILURE;
			break;
		}
#else
			if (u1ArgNum != rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
			for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
				rSrCmdSrRcpiTbl.rSrRcpiTbl.u1RcpiTblMcs[u1Index] = SRRcpiConv((INT_8)i4RcpiTblMcs[u1Index]);

#ifdef WIFI_UNIFIED_COMMAND
			Status = UniCmdSRUpdateRcpiTbl(pAd, &rSrCmdSrRcpiTbl, u1DbdcIdx);
#else
			Status = SrCmdSRUpdateRcpiTbl(pAd, &rSrCmdSrRcpiTbl);
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srrcpitbl=[RcpiTblMcs[0]]-...-[RcpiTblMcs[11]]\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

/** SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL **/
INT SetSrRcpiTblOfstAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4RxBwRcpiOfst = 0, u4StbcRcpiOfst = 0, u4NumAntRcpiOfst = 0;
	UINT_32 u4LdpcRcpiOfst = 0, u4DcmRcpiOfst  = 0, u4MacRcpiOfst = 0;
	UINT_32 u4SigRcpiOfst  = 0, u4BfRcpiOfst   = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 rSrCmdSrRcpiTblOfst;

		os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrRcpiTblOfst.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 7);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4RxBwRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4RxBwRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 1:
						u4StbcRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4StbcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 2:
						u4NumAntRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4NumAntRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 3:
						u4LdpcRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 4:
						u4DcmRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 5:
						u4MacRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 6:
						u4SigRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst = u4RxBwRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst = u4StbcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst = u4NumAntRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst = u4LdpcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst = u4DcmRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst = u4MacRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst = u4SigRcpiOfst;
#ifdef CFG_BIG_ENDIAN
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst);
#endif
					Status = SrCmdSRUpdateRcpiTblOfst(pAd, &rSrCmdSrRcpiTblOfst);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srrcpitblofst=[RxBwRcpiOfst]-[StbcRcpiOfst]-[NumAntRcpiOfst]-[LdpcRcpiOfst]-[DcmRcpiOfst]-[MacRcpiOfst]-[SigRcpiOfst]\n");
		}

	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2 rSrCmdSrRcpiTblOfst;

		os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrRcpiTblOfst.u2Tag = UNI_CMD_SR_HW_RCPI_TBL_OFST;
		rSrCmdSrRcpiTblOfst.u2Length = sizeof(struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2);
#else
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 rSrCmdSrRcpiTblOfst;

		os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrRcpiTblOfst.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 8);
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4RxBwRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4RxBwRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 1:
						u4StbcRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4StbcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 2:
						u4NumAntRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4NumAntRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 3:
						u4LdpcRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 4:
						u4DcmRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DcmRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 5:
						u4MacRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4MacRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 6:
						u4SigRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SigRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 7:
						u4BfRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4BfRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						" Format Error! ArgNum = %d != %d\n",
						u1ArgNum, SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#else
				if (u1ArgNum != rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst = u4RxBwRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst = u4StbcRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u4NumAntRcpiOfst = u4NumAntRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst = u4LdpcRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst = u4DcmRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst = u4MacRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst = u4SigRcpiOfst;
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst = u4BfRcpiOfst;

#ifdef CFG_BIG_ENDIAN
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst);
#endif

#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateRcpiTblOfstV2(pAd, &rSrCmdSrRcpiTblOfst, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateRcpiTblOfst(pAd, &rSrCmdSrRcpiTblOfst);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srrcpitblofst=[RxBwRcpiOfst]-[StbcRcpiOfst]-[NumAntRcpiOfst]-[LdpcRcpiOfst]-[DcmRcpiOfst]-[MacRcpiOfst]-[SigRcpiOfst]-[BfRcpiOfst]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_Q_CTRL_ALL_CTRL **/
INT SetSrQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRxRptEn = 0, u4SrCw = 0, u4SrSuspend = 0, u4SrDisSwAifsDis = 0;
	UINT_32 u4SrBackOffMask = 0;
	UINT_32 u4SrBackOffEnable = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_Q_CTRL_T_SR_V1 rSrCmdSrQCtrl;

		os_zero_mem(&rSrCmdSrQCtrl, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL;
		rSrCmdSrQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 4);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRxRptEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxRptEn, u1ArgNum);
						break;
					case 1:
						u4SrCw = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrCw, u1ArgNum);
						break;
					case 2:
						u4SrSuspend = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrSuspend, u1ArgNum);
					break;
					case 3:
						u4SrBackOffMask = simple_strtol(value, 0, 10);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrQCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrQCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrQCtrl.rSrQCtrl.fgSrRxRptEn = u4SrRxRptEn;
					rSrCmdSrQCtrl.rSrQCtrl.fgSrCw = u4SrCw;
					rSrCmdSrQCtrl.rSrQCtrl.fgSrSuspend = u4SrSuspend;
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask = u4SrBackOffMask;
#ifdef CFG_BIG_ENDIAN
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask
						= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask);
#endif
					Status = SrCmdSRUpdateQCtrl(pAd, &rSrCmdSrQCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 " iwpriv ra0 set srqctrl=[SrRxRptEn]-[SrCw]-[SrSuspend]-[SrBackOffMask]\n");
		}
	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_Q_CTRL_T_SR_V2 rSrCmdSrQCtrl;

		os_zero_mem(&rSrCmdSrQCtrl, sizeof(struct UNI_CMD_SR_Q_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrQCtrl.u2Tag = UNI_CMD_SR_HW_Q_CTRL;
		rSrCmdSrQCtrl.u2Length = sizeof(struct UNI_CMD_SR_Q_CTRL_T_SR_V2);
#else
		struct SR_CMD_SR_Q_CTRL_T_SR_V2 rSrCmdSrQCtrl;

		os_zero_mem(&rSrCmdSrQCtrl, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL;
		rSrCmdSrQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 6);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRxRptEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxRptEn, u1ArgNum);
						break;
					case 1:
						u4SrCw = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrCw, u1ArgNum);
						break;
					case 2:
						u4SrSuspend = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrSuspend, u1ArgNum);
						break;
					case 3:
						u4SrDisSwAifsDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrDisSwAifsDis, u1ArgNum);
						break;
					case 4:
						u4SrBackOffMask = simple_strtol(value, 0, 16);
						break;
					case 5:
						u4SrBackOffEnable = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2);
				Status = NDIS_STATUS_FAILURE;
				break;
				}
#else
				if (u1ArgNum != rSrCmdSrQCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						" Format Error! ArgNum = %d != %d\n",
						u1ArgNum, rSrCmdSrQCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
				rSrCmdSrQCtrl.rSrQCtrl.fgSrRxRptEn = u4SrRxRptEn;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrCw = u4SrCw;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrSuspend = u4SrSuspend;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrDisSwAifsDis = u4SrDisSwAifsDis;
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask = u4SrBackOffMask;
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable = u4SrBackOffEnable;

#ifdef CFG_BIG_ENDIAN
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask
					= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask);
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable
					= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable);
#endif

#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateQCtrl(pAd, &rSrCmdSrQCtrl, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateQCtrl(pAd, &rSrCmdSrQCtrl);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srqctrl=[SrRxRptEn]-[SrCw]-[SrSuspend]-[SrDisSwAifsDis]-[SrBackOffMask]-[u4SrBackOffEnable]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_IBPD_ALL_CTRL **/
INT SetSrIBPDAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4InterBssByHdrBssid = 0, u4InterBssByMu = 0, u4InterBssByPbssColor = 0;
	UINT_32 u4InterBssByPaid = 0, u4InterBssByBssColor = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_IBPD_T rSrCmdSrIBPD;
#else
	struct SR_CMD_SR_IBPD_T rSrCmdSrIBPD;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrIBPD, sizeof(struct UNI_CMD_SR_IBPD_T));

	/* Assign Cmd Id */
	rSrCmdSrIBPD.u2Tag = UNI_CMD_SR_HW_IBPD;
	rSrCmdSrIBPD.u2Length = sizeof(struct UNI_CMD_SR_IBPD_T);
#else
	os_zero_mem(&rSrCmdSrIBPD, sizeof(struct SR_CMD_SR_IBPD_T));

	/* Assign Cmd Id */
	rSrCmdSrIBPD.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_IBPD_ALL_CTRL;
	rSrCmdSrIBPD.rSrCmd.u1ArgNum = SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM;
	rSrCmdSrIBPD.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 5);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4InterBssByHdrBssid = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByHdrBssid, u1ArgNum, 0, BIT8);
					break;
				case 1:
					u4InterBssByMu = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByMu, u1ArgNum, 0, BIT8);
					break;
				case 2:
					u4InterBssByPbssColor = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByPbssColor, u1ArgNum, 0, BIT8);
					break;
				case 3:
					u4InterBssByPaid = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByPaid, u1ArgNum, 0, BIT8);
					break;
				case 4:
					u4InterBssByBssColor = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByBssColor, u1ArgNum, 0, BIT8);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != rSrCmdSrIBPD.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrIBPD.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
			rSrCmdSrIBPD.rSrIBPD.u1InterBssByHdrBssid = u4InterBssByHdrBssid;
			rSrCmdSrIBPD.rSrIBPD.u1InterBssByMu = u4InterBssByMu;
			rSrCmdSrIBPD.rSrIBPD.u1InterBssByPbssColor = u4InterBssByPbssColor;
			rSrCmdSrIBPD.rSrIBPD.u1InterBssByPaid = u4InterBssByPaid;
			rSrCmdSrIBPD.rSrIBPD.u1InterBssByBssColor = u4InterBssByBssColor;

#ifdef WIFI_UNIFIED_COMMAND
			Status = UniCmdSRUpdateIBPD(pAd, &rSrCmdSrIBPD, u1DbdcIdx);
#else
			Status = SrCmdSRUpdateIBPD(pAd, &rSrCmdSrIBPD);
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 " iwpriv ra0 set sribpd=[InterBssByHdrBssid]-[InterBssByMu]-[InterBssByPbssColor]-[InterBssByPaid]-[InterBssByBssColor]\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_NRT_ALL_CTRL **/
INT SetSrNRTAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4TableIdx = 0, u4RaTaSel = 0, u4SwProtect = 0, u4NRTValue = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_T_SR_V1 rSrCmdSrNRT;

		os_zero_mem(&rSrCmdSrNRT, sizeof(struct SR_CMD_SR_NRT_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrNRT.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_ALL_CTRL;
		rSrCmdSrNRT.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrNRT.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 2);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4TableIdx = simple_strtol(value, 0, 10);
						Status = IsInRange(u4TableIdx, u1ArgNum, 0, SR_NRT_ROW_NUM);
						break;
					case 1:
						u4NRTValue = simple_strtol(value, 0, 10);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRT.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRT.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRT.rSrNRT.u1TableIdx = u4TableIdx;
					rSrCmdSrNRT.rSrNRT.u4NRTValue = u4NRTValue;
#ifdef CFG_BIG_ENDIAN
					rSrCmdSrNRT.rSrNRT.u4NRTValue
						= cpu2le32(rSrCmdSrNRT.rSrNRT.u4NRTValue);
#endif
					Status = SrCmdSRUpdateNRT(pAd, &rSrCmdSrNRT);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srnrt=[TableIdx]-[NRTValue]\n");
		}
	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_NRT_T_SR_V2 rSrCmdSrNRT;

		os_zero_mem(&rSrCmdSrNRT, sizeof(struct UNI_CMD_SR_NRT_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRT.u2Tag = UNI_CMD_SR_HW_NRT;
		rSrCmdSrNRT.u2Length = sizeof(struct UNI_CMD_SR_NRT_T_SR_V2);
#else
		struct SR_CMD_SR_NRT_T_SR_V2 rSrCmdSrNRT;

		os_zero_mem(&rSrCmdSrNRT, sizeof(struct SR_CMD_SR_NRT_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRT.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_ALL_CTRL;
		rSrCmdSrNRT.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrNRT.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 4);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4TableIdx = simple_strtol(value, 0, 10);
						Status = IsInRange(u4TableIdx, u1ArgNum, 0, SR_NRT_ROW_NUM);
						break;
					case 1:
						u4RaTaSel = simple_strtol(value, 0, 10);
						Status = IsInRange(u4RaTaSel, u1ArgNum, 0, BIT2);
						break;
					case 2:
						u4SwProtect = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SwProtect, u1ArgNum);
						break;
					case 3:
						u4NRTValue = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						" Format Error! ArgNum = %d != %d\n",
						u1ArgNum, SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#else
				if (u1ArgNum != rSrCmdSrNRT.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRT.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
				rSrCmdSrNRT.rSrNRT.u1TableIdx = u4TableIdx;
				rSrCmdSrNRT.rSrNRT.u1RaTaSel = u4RaTaSel;
				rSrCmdSrNRT.rSrNRT.fgSwProtect = u4SwProtect;
				rSrCmdSrNRT.rSrNRT.u4NRTValue = u4NRTValue;
#ifdef CFG_BIG_ENDIAN
				rSrCmdSrNRT.rSrNRT.u4NRTValue
					= cpu2le32(rSrCmdSrNRT.rSrNRT.u4NRTValue);
#endif

#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateNRT(pAd, &rSrCmdSrNRT, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateNRT(pAd, &rSrCmdSrNRT);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srnrt=[TableIdx]-[RaTaSel]-[SwProtect]-[NRTValue]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_NRT_RESET_CTRL **/
INT SetSrNRTResetAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_NRT_RESET, SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_NRT_RESET_CTRL, SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srnrtreset=0\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL **/
INT SetSrNRTCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrtEn = 0, u4SrtSrpEn = 0, u4SrtAddrOrderEn = 0, u4SrtByPassCtsAck = 0;
	UINT_32 u4SrtInRcpiTh = 0, u4SrtOutRcpiTh = 0;
	UINT_32 u4SrtUsedCntTh = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_CTRL_T_SR_V1 rSrCmdSrNRTCtrl;

		os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrNRTCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL;
		rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrNRTCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 6);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtEn, u1ArgNum);
						break;
					case 1:
						u4SrtSrpEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtSrpEn, u1ArgNum);
						break;
					case 2:
						u4SrtAddrOrderEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtAddrOrderEn, u1ArgNum);
						break;
					case 3:
						u4SrtInRcpiTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtInRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 4:
						u4SrtOutRcpiTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtOutRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 5:
						u4SrtUsedCntTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtUsedCntTh, u1ArgNum, 0, BIT16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtEn = u4SrtEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtSrpEn = u4SrtSrpEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtAddrOrderEn = u4SrtAddrOrderEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh = u4SrtInRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh = u4SrtOutRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh = u4SrtUsedCntTh;
#ifdef CFG_BIG_ENDIAN
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh);
#endif
					Status = SrCmdSRUpdateNRTCtrl(pAd, &rSrCmdSrNRTCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srnrtctrl=[SrtEn]-[SrtSrpEn]-[SrtAddrOrderEn]-[SrtInRcpiTh]-[SrtOutRcpiTh]-[SrtUsedCntTh]\n");
		}
	} else if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_NRT_CTRL_T_SR_V2 rSrCmdSrNRTCtrl;

		os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(struct UNI_CMD_SR_NRT_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRTCtrl.u2Tag = UNI_CMD_SR_HW_NRT_CTRL;
		rSrCmdSrNRTCtrl.u2Length = sizeof(struct UNI_CMD_SR_NRT_CTRL_T_SR_V2);
#else
		struct SR_CMD_SR_NRT_CTRL_T_SR_V2 rSrCmdSrNRTCtrl;

		os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRTCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL;
		rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrNRTCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 7);
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtEn, u1ArgNum);
						break;
					case 1:
						u4SrtSrpEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtSrpEn, u1ArgNum);
						break;
					case 2:
						u4SrtAddrOrderEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtAddrOrderEn, u1ArgNum);
						break;
					case 3:
						u4SrtByPassCtsAck = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtByPassCtsAck, u1ArgNum);
						break;
					case 4:
						u4SrtInRcpiTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtInRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 5:
						u4SrtOutRcpiTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtOutRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 6:
						u4SrtUsedCntTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtUsedCntTh, u1ArgNum, 0, BIT16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
				if (u1ArgNum != SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#else
				if (u1ArgNum != rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				}
#endif
				rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtEn = u4SrtEn;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtSrpEn = u4SrtSrpEn;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtAddrOrderEn = u4SrtAddrOrderEn;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtByPassCtsAck = u4SrtByPassCtsAck;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh = u4SrtInRcpiTh;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh = u4SrtOutRcpiTh;
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh = u4SrtUsedCntTh;
#ifdef CFG_BIG_ENDIAN
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh
					= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh);
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh
					= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh);
				rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh
					= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh);
#endif

#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateNRTCtrl(pAd, &rSrCmdSrNRTCtrl, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateNRTCtrl(pAd, &rSrCmdSrNRTCtrl);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srnrtctrl=[SrtEn]-[SrtSrpEn]-[SrtAddrOrderEn]-[SrtByPassCtsAck]-[SrtInRcpiTh]-[SrtOutRcpiTh]-[SrtUsedCntTh]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL **/
INT SetSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrpCondDis = 0, u4PeriodOfst = 0, u4HdrDurEn = 0;
	UINT_32 u4TxopDurEn = 0, u4SrpCfendRst = 0, u4SrpNavToutRst = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_FNQ_CTRL_T rSrCmdSrFNQCtrl;

		os_zero_mem(&rSrCmdSrFNQCtrl, sizeof(struct UNI_CMD_SR_FNQ_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrFNQCtrl.u2Tag = UNI_CMD_SR_HW_FNQ;
		rSrCmdSrFNQCtrl.u2Length = sizeof(struct UNI_CMD_SR_FNQ_CTRL_T);

#else
		struct SR_CMD_SR_FNQ_CTRL_T rSrCmdSrFNQCtrl;

		os_zero_mem(&rSrCmdSrFNQCtrl, sizeof(struct SR_CMD_SR_FNQ_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrFNQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL;
		rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM;
		rSrCmdSrFNQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 6);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrpCondDis = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrpCondDis, u1ArgNum, 0, BIT16);
						break;
					case 1:
						u4PeriodOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4PeriodOfst, u1ArgNum, 0, BIT8);
						break;
					case 2:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 3:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 4:
						u4SrpCfendRst = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpCfendRst, u1ArgNum);
						break;
					case 5:
						u4SrpNavToutRst = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpNavToutRst, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
			rSrCmdSrFNQCtrl.rSrFNQCtrl.u2SrpCondDis = u4SrpCondDis;
			rSrCmdSrFNQCtrl.rSrFNQCtrl.u1PeriodOfst = u4PeriodOfst;
			rSrCmdSrFNQCtrl.rSrFNQCtrl.fgHdrDurEn = u4HdrDurEn;
			rSrCmdSrFNQCtrl.rSrFNQCtrl.fgTxopDurEn = u4TxopDurEn;
			rSrCmdSrFNQCtrl.rSrFNQCtrl.fgSrpCfendRst = u4SrpCfendRst;
			rSrCmdSrFNQCtrl.rSrFNQCtrl.fgSrpNavToutRst = u4SrpNavToutRst;

#ifdef WIFI_UNIFIED_COMMAND
			Status = UniCmdSRUpdateFNQCtrl(pAd, &rSrCmdSrFNQCtrl, u1DbdcIdx);
#else
			Status = SrCmdSRUpdateFNQCtrl(pAd, &rSrCmdSrFNQCtrl);
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srfnqctrl=[SrpCondDis]-[PeriodOfst]-[HdrDurEn]-[TxopDurEn]-[SrpCfendRst]-[SrpNavToutRst]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_FRM_FILT_ALL_CTRL **/
INT SetSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrFrmFilt = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		UINT_8 isSet = TRUE;
#else
		struct SR_CMD_SR_FRM_FILT_T rSrCmdSrFrmFilt;

		os_zero_mem(&rSrCmdSrFrmFilt, sizeof(struct SR_CMD_SR_FRM_FILT_T));

		/* Assign Cmd Id */
		rSrCmdSrFrmFilt.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_FRM_FILT_ALL_CTRL;
		rSrCmdSrFrmFilt.rSrCmd.u1ArgNum = SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM;
		rSrCmdSrFrmFilt.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
					switch (u1ArgNum) {
					case 0:
						u4SrFrmFilt = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			Status = UniSrCmd(pAd, UNI_CMD_SR_HW_FRMFILT, u1DbdcIdx, u4SrFrmFilt, isSet);
#else
			if (u1ArgNum != rSrCmdSrFrmFilt.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrFrmFilt.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			rSrCmdSrFrmFilt.u4SrFrmFilt = u4SrFrmFilt;
			Status = SrCmdSRUpdateFrmFilt(pAd, &rSrCmdSrFrmFilt);
#endif
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srfrmfilt=[SrFrmFilt]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL **/
INT SetSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4CondDis = 0, u4DurAdj   = 0, u4DurLmt = 0;
	UINT_32 u4EntryEn = 0, u4DurLmtEn = 0, u4InterpsEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_SR_INTERPS_CTRL_T rSrCmdSrInterPsCtrl;

		os_zero_mem(&rSrCmdSrInterPsCtrl, sizeof(struct UNI_CMD_SR_INTERPS_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrInterPsCtrl.u2Tag = UNI_CMD_SR_HW_INTERPS_CTRL;
		rSrCmdSrInterPsCtrl.u2Length = sizeof(struct UNI_CMD_SR_INTERPS_CTRL_T);
#else
		struct SR_CMD_SR_INTERPS_CTRL_T rSrCmdSrInterPsCtrl;

		os_zero_mem(&rSrCmdSrInterPsCtrl, sizeof(struct SR_CMD_SR_INTERPS_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrInterPsCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL;
		rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM;
		rSrCmdSrInterPsCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 6);
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4CondDis = simple_strtol(value, 0, 16);
						Status = IsInRange(u4CondDis, u1ArgNum, 0, BIT8);
						break;
					case 1:
						u4DurAdj = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DurAdj, u1ArgNum, 0, BIT8);
						break;
					case 2:
						u4DurLmt = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DurLmt, u1ArgNum, 0, BIT8);
						break;
					case 3:
						u4EntryEn = simple_strtol(value, 0, 16);
						Status = IsInRange(u4EntryEn, u1ArgNum, 0, BIT8);
						break;
					case 4:
						u4DurLmtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4DurLmtEn, u1ArgNum);
						break;
					case 5:
						u4InterpsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4InterpsEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_CFG,
								CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif

			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1CondDis = u4CondDis;
			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1DurAdj = u4DurAdj;
			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1DurLmt = u4DurLmt;
			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1EntryEn = u4EntryEn;
			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.fgDurLmtEn = u4DurLmtEn;
			rSrCmdSrInterPsCtrl.rSrInterPsCtrl.fgInterpsEn = u4InterpsEn;

#ifdef WIFI_UNIFIED_COMMAND
			Status = UniCmdSRUpdateInterPsCtrl(pAd, &rSrCmdSrInterPsCtrl, u1DbdcIdx);
#else
			Status = SrCmdSRUpdateInterPsCtrl(pAd, &rSrCmdSrInterPsCtrl);
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srinterpsctrl=[u4CondDis]-[DurAdj]-[DurLmt]-[EntryEn]-[DurLmtEn]-[InterpsEn]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
INT SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsren=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_ESR_ENABLE **/
INT SetSrCfgESrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_ENHANCE_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set sresren=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
INT SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrsden=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

/** SR_CMD_SET_SR_CFG_SR_BF **/
INT SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_BF, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_BF, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrbf=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

/** SR_CMD_SET_SR_CFG_SR_ATF **/
INT SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_ATF, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ATF, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsratf=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

/** SR_CMD_SET_SR_CFG_SR_MODE **/
INT SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_MODE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrmode=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
INT SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DISRT_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgdisrten=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
INT SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DISRT_MIN_RSSI, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgdisrtmin=62 (Range:0 ~ 110)\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_TXC_QUEUE **/
INT SetSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_QUEUE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QUEUE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgtxcq=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_TXC_QID **/
INT SetSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_QID, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QID, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgtxcqid=86\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_TXC_PATH **/
INT SetSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_PATH, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_PATH, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgtxcpath=1\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_AC_METHOD **/
INT SetSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_AC_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_AC_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgac=0\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_PERIOD_THR **/
INT SetSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_PERIOD_THR, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_PERIOD_THR, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrperiodthr=480\n");
	}

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD **/
INT SetSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_QUERY_TXD_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrquerytxd=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO **/
INT SetSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_CG_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrsdcg=800\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO **/
INT SetSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrsdobss=500\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_PROFILE **/
INT SetSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_PROFILE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_PROFILE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrprofile=3\n"
			 "SR_PROFILE_QUERY_TXD_TIME           BIT(0)\n"
			 "SR_PROFILE_SHOW_Q_LEN               BIT(1)\n"
			 "SR_PROFILE_RPT_HANDLE_TIME          BIT(2)\n"
			 "SR_PROFILE_GEN_TXC_TIME             BIT(3)\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_FNQ_ENABLE **/
INT SetSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_FNQ_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_FNQ_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcfgfnqen=1\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
INT SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DPD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DPD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgdpden=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ENABLE **/
INT SetSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrtxen=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE **/
INT SetSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgomen=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE **/
INT SetSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrtxalignen=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
INT SetSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrtxalignrssi=40\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_DABS_MODE **/
INT SetSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_DABS_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_DABS_MODE, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgdabsmode=1\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_RATE_OFFSET **/
INT SetSrCfgSrRateOffset(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SR_RATE_OFFSET, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srcfgsrrateoffset\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrMeshSDFlag(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_MESH_SR_SD_CTRL, SR_CMD_SET_DEFAULT_ARG_NUM);

	return Status ? FALSE : TRUE;
}


/** SR_CMD_SET_SR_SRG_BITMAP **/
INT SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrSrgBitmap;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrSrgBitmap, sizeof(rSrCmdSrSrgBitmap));

	/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrSrgBitmap.u2Tag = UNI_CMD_SR_SW_SRG_BITMAP;
	rSrCmdSrSrgBitmap.u2Length = sizeof(rSrCmdSrSrgBitmap);
#else
	rSrCmdSrSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SRG_BITMAP;
	rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM;
	rSrCmdSrSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 4);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;
				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;
				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;
				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
#ifndef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
			else {
				rSrCmdSrSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = u4Color_31_0;
				rSrCmdSrSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = u4Color_63_32;
				rSrCmdSrSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = u4pBssid_31_0;
				rSrCmdSrSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = u4pBssid_63_32;
#ifdef WIFI_UNIFIED_COMMAND
				Status = UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrSrgBitmap, u1DbdcIdx);
#else
				Status = SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrSrgBitmap);
#endif
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;

}

/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_SW_SRG_BITMAP_REFRESH, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_SRG_BITMAP_REFRESH, SR_CMD_SET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srsrgbmrefresh=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct UNI_CMD_SR_SW_SRG_BITMAP_T));
#else
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
#endif

	/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrMeshSrgBitmap.u2Tag = UNI_CMD_SR_SW_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.u2Length = sizeof(rSrCmdSrMeshSrgBitmap);
#else
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value && (u1ArgNum < 4); value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
								"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;
#ifndef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#else
			if (u1ArgNum != SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
#endif
			else {
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = u4Color_31_0;
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = u4Color_63_32;
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = u4pBssid_31_0;

				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = u4pBssid_63_32;
#ifdef WIFI_UNIFIED_COMMAND
				if (UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrMeshSrgBitmap, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
#else
				if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
#endif
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srmeshsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_SIGA_FLAG_CTRL **/
INT SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4readvalue;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0, u1Bssid, u1SigaFlag = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SIGA_FLAG_T rSrCmdSrSigaFlag;
#else
	struct SR_CMD_SR_SIGA_FLAG_T rSrCmdSrSigaFlag;
#endif

	if (wdev != NULL) {
		u1DbdcIdx = HcGetBandByWdev(wdev);
		u1Bssid = wdev->DevInfo.OwnMacIdx > 3 ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;
	} else
		return FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrSigaFlag, sizeof(struct UNI_CMD_SR_SIGA_FLAG_T));

	/* Assign Cmd Id */
	rSrCmdSrSigaFlag.u2Tag = UNI_CMD_SR_HW_SIGA_FLAG;
	rSrCmdSrSigaFlag.u2Length = sizeof(struct UNI_CMD_SR_SIGA_FLAG_T);
#else
	os_zero_mem(&rSrCmdSrSigaFlag, sizeof(struct SR_CMD_SR_SIGA_FLAG_T));

	/* Assign Cmd Id */
	rSrCmdSrSigaFlag.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SIGA_FLAG_CTRL;
	rSrCmdSrSigaFlag.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrSigaFlag.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value && (u1ArgNum < 1); value = rstrtok(NULL, "-"), u1ArgNum++) {
				u4readvalue = simple_strtol(value, 0, 10);

				switch (u1ArgNum) {

				case 0:
					if (IsInRange(u4readvalue, u1ArgNum, 0, SR_SIGA_FLAG_RANGE) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					u1SigaFlag = u4readvalue;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				rSrCmdSrSigaFlag.rSrSigaFlag.u1Bssid = u1Bssid;
				rSrCmdSrSigaFlag.rSrSigaFlag.u1SigaFlag[u1Bssid] = u1SigaFlag;
				if (UniCmdSRUpdateSiga(pAd, &rSrCmdSrSigaFlag, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
#else
			if (u1ArgNum != rSrCmdSrSigaFlag.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, rSrCmdSrSigaFlag.rSrCmd.u1ArgNum);
				Status = FALSE;
			} else {
				rSrCmdSrSigaFlag.rSrSigaFlag.u1Bssid = u1Bssid;
				rSrCmdSrSigaFlag.rSrSigaFlag.u1SigaFlag[u1Bssid] = u1SigaFlag;
				if (SrCmdSRUpdateSiga(pAd, &rSrCmdSrSigaFlag) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srsrgbm=[BssIDindx][SRSiga Flag]\n");
	}

	return Status;
}

/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SrMeshApcliDetect(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;// TRUE =
	UCHAR ucinputmac[MAC_ADDR_LEN];
	UINT_8 u1ArgNum = 0;
	struct sr_mesh_topology_params *prSrMeshTopologyParams = &g_rTopologyUpdate;

	if (arg) {
		if (strlen(arg) != SR_SCENE_DETECTION_MAC_LENGTH)
			return FALSE;
		do {
			for (u1ArgNum = 0; u1ArgNum < MAC_ADDR_LEN; u1ArgNum++) {
				AtoH(arg, &ucinputmac[u1ArgNum], 1);
				arg = arg + 3;
			}

			if (IS_EQUAL_MAC(ucinputmac, prSrMeshTopologyParams->map_remote_bh_mac))
				fgmeshdetect &= 1;
			else
				fgmeshdetect &= 0;

			return Status;
		} while (0);
	} else {

		Status = FALSE;
	}

	return Status;
}

/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SetMeshMac(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;// TRUE = 1
	UINT_8 u1ArgNum = 0;
	struct sr_mesh_topology_params *prSrMeshTopologyParams = &g_rTopologyUpdate;

	if (strlen(arg) != SR_SCENE_DETECTION_MAC_LENGTH)
		return FALSE;


	for (u1ArgNum = 0; u1ArgNum < MAC_ADDR_LEN; u1ArgNum++) {
		AtoH(arg, &prSrMeshTopologyParams->map_remote_bh_mac[u1ArgNum], 1);
		arg = arg + 3;
	}

	return Status;
}

NDIS_STATUS SetMeshSRsd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;

	RTMP_STRING MeshSdFlag[2] = "0";
	RTMP_STRING SCSEnable[2] = "0";

	if (arg) {
		if (fghavebeensend != fgmeshdetect) {
			fghavebeensend = fgmeshdetect;
			if (fgmeshdetect) {
				strlcpy(MeshSdFlag, "0", sizeof(MeshSdFlag)); //There is no unknown apcli
				strlcpy(SCSEnable, "1", sizeof(SCSEnable));  //scs on
			} else {
				strlcpy(MeshSdFlag, "1", sizeof(MeshSdFlag)); //There is a unknown apcli
				strlcpy(SCSEnable, "0", sizeof(SCSEnable));  //scs off

			}
			fgmeshdetect = 1;
			MeshSdFlag[1] = '\0';
			SCSEnable[1] = '\0';
		} else {
			/*No need to send fw command*/
			fgmeshdetect = 1;
			return Status;
		}

		Status = SetSrMeshSDFlag(pAd, MeshSdFlag);


		return Status;
	} else
		return FALSE;

}

/** SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL **/
INT SetSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)

{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4readvalue;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SIGA_AUTO_FLAG_T rSrCmdSrSigaAutoFlag;
#else
	struct _SR_CMD_SR_SIGA_AUTO_FLAG_T rSrCmdSrSigaAutoFlag;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	os_zero_mem(&rSrCmdSrSigaAutoFlag, sizeof(rSrCmdSrSigaAutoFlag));

#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrSigaAutoFlag.u2Tag = UNI_CMD_SR_HW_SIGA_AUTO_FLAG;
	rSrCmdSrSigaAutoFlag.u2Length = sizeof(rSrCmdSrSigaAutoFlag);
#else
	/* Assign Cmd Id */
	rSrCmdSrSigaAutoFlag.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL;
	rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrSigaAutoFlag.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				u4readvalue = simple_strtol(value, 0, 10);
				switch (u1ArgNum) {
				case 0:
					if (IsInRange(u4readvalue, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					rSrCmdSrSigaAutoFlag.rSrSigaAutoFlag.u1SrSigaAutoFlag = u4readvalue;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;


#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (UniCmdSRUpdateSigaAuto(pAd, &rSrCmdSrSigaAutoFlag, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
#else
			if (u1ArgNum != rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum);
				Status = FALSE;
			} else {
				if (SrCmdSRUpdateSigaAuto(pAd, &rSrCmdSrSigaAutoFlag) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 " iwpriv ra0 set srsigaauto=[SRSigaAuto_Flag]\n");
	}

	return Status;
}

/** Driver Internal **/
INT SetSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0, u1Mode = 0;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap = NULL;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value && (u1ArgNum < 5); value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u1Mode = simple_strtol(value, 0, 10);

					if (IsInRange(u1Mode, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 1:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 4:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM);
				Status = FALSE;
				break;
			}

			if (u1Mode == 1) {
				pAd->SrSelfSrgBMMode = ENUM_SR_SELF_BM_MANUAL;

				prSrgBitmap = &pAd->SrSelfSrgBMMan;
				prSrgBitmap->u4Color_31_0 = u4Color_31_0;
				prSrgBitmap->u4Color_63_32 = u4Color_63_32;
				prSrgBitmap->u4pBssid_31_0 = u4pBssid_31_0;
				prSrgBitmap->u4pBssid_63_32 = u4pBssid_63_32;

				SrMeshSelfSrgInfoEvent(pAd, u1DbdcIdx);
			} else
				pAd->SrSelfSrgBMMode = ENUM_SR_SELF_BM_AUTO;
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 " iwpriv ra0 set srselfsrginfo=[Mode]-[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	if (prSrgBitmap != NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			 "BandIdx:%u, Mode:%u BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
			 u1DbdcIdx, pAd->SrSelfSrgBMMode, prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
			 prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);
	}

	return Status;
}

/** Driver Internal **/
INT SetSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1TopoLock = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					u1TopoLock = simple_strtol(value, 0, 10);
					if (IsInRange(u1TopoLock, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				pAd->SrMeshTopoLock = u1TopoLock;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		 "BandIdx:%u, SrMeshTopoLock:%u\n",
		 u1DbdcIdx, pAd->SrMeshTopoLock);

	return Status;
}

/** SR_CMD_SET_REMOTE_FH_RSSI **/
INT SetSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT value;
	INT_8 Rssi;
	UINT32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (pAd->SrMeshTopoLock == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"SrMeshTopoLock:%d Auto for band:%d\n",
				pAd->SrMeshTopoLock, u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		rv = sscanf(arg, "%d", &value);

		if (rv > 0) {
			Rssi = (INT_8)value;
			if (SrSetRemoteFHRssi(pAd, u1DbdcIdx, Rssi, ENUM_RMT_FH_SCAN_SUCCESS) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"set %d\n", Rssi);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			" iwpriv ra0 set srmeshremotefhrssi=[Rssi]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_REMOTE_FH_RSSI **/
NDIS_STATUS SrSetRemoteFHRssi(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi, UINT_8 u1RemoteFhStat)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.u2Tag = UNI_CMD_SR_HW_REMOTE_FH_RSSI;
	rSrCmdMeshTopology.u2Length = sizeof(rSrCmdMeshTopology);
#else
	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_REMOTE_FH_RSSI;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteFhParams.i1Rssi = Rssi;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteFhParams.u1RemoteFhStat = u1RemoteFhStat;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology, u1DbdcIdx);
#else
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);
#endif
	return Status;
}

NDIS_STATUS SrSetMeshStaThresholdToFw(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_MESH_DL_STA_MESH_THRESHOLD_T *prSrCmdMeshDlThreshold, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_MESH_DL_STA_MESH_THRESHOLD_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshDlThreshold, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}


/** UNI_CMD_SR_DOWNLINK_STA_THRESHOLD **/
NDIS_STATUS SrSetMeshStaThreshold(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT8 u1Bssid, INT_8 i1DlThreshold)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_MESH_DL_STA_MESH_THRESHOLD_T rSrCmdMeshDlThreshold;

	os_zero_mem(&rSrCmdMeshDlThreshold, sizeof(rSrCmdMeshDlThreshold));

	/* Assign Cmd Id */
	rSrCmdMeshDlThreshold.u2Tag = UNI_CMD_SR_DOWNLINK_STA_THRESHOLD;
	rSrCmdMeshDlThreshold.u2Length = sizeof(rSrCmdMeshDlThreshold);
	rSrCmdMeshDlThreshold.rSrDLStaThrehsold.irssi = i1DlThreshold;
	rSrCmdMeshDlThreshold.rSrDLStaThrehsold.u1BSSID = u1Bssid;

	Status = SrSetMeshStaThresholdToFw(pAd, &rSrCmdMeshDlThreshold, u1DbdcIdx);

#endif
	return Status;
}

/** UNI_CMD_SR_DOWNLINK_STA_THRESHOLD **/
INT SetSrMeshStaThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	UINT_8 u1DbdcIdx = BAND0;
	CHAR *value;
	u_long i1Rssi;
	u_long u2Wlanid = 0;
	struct wifi_dev *wdev = NULL;
	UINT_8 u1Bssid = 0;


	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, ":"); value && (u1ArgNum < 2);
				value = rstrtok(NULL, ":"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:

					if (kstrtol(value, 10, (long *)&u2Wlanid)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						return FALSE;
					}
					wdev = pAd->wdev_list[u2Wlanid];
					if (wdev == NULL) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"Wdev is Null, WdevIdx	= %ld\n", u2Wlanid);
						return FALSE;
					}
					u1DbdcIdx = HcGetBandByWdev(wdev);
					u1Bssid = (UINT_8)(wdev->DevInfo.OwnMacIdx > SR_MESH_P2PGC ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx);

					break;

				case 1:
					if (kstrtol(value, 10, (long *)&i1Rssi)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						return FALSE;
					}
					MTWF_PRINT("BandIdx = %d, Bssid = %d, Rssi = %ld\n", u1DbdcIdx, u1Bssid,  i1Rssi);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetMeshStaThreshold(pAd, u1DbdcIdx, u1Bssid, i1Rssi) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			" iwpriv ra0 set srmeshremotefhrssi=[Rssi]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	u_long u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct UNI_CMD_SR_SW_SRG_BITMAP_T));
#else
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
#endif

	/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrMeshSrgBitmap.u2Tag = UNI_CMD_SR_FH_MESH_SR_BITMAP;
	rSrCmdSrMeshSrgBitmap.u2Length = sizeof(rSrCmdSrMeshSrgBitmap);
#else
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif


	if (arg) {
		do {
		/* parameter parsing */
		for (u1ArgNum = 0, value = rstrtok(arg, "-");
			value && (u1ArgNum < 4); value = rstrtok(NULL, "-"), u1ArgNum++) {

			if (Status == FALSE)
				break;

			switch (u1ArgNum) {

			case 0:
				if (kstrtol(value, 16, (long *)&u4Color_31_0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 1:
				if (kstrtol(value, 16, (long *)&u4Color_63_32)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 2:
				if (kstrtol(value, 16, (long *)&u4pBssid_31_0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 3:
				if (kstrtol(value, 16, (long *)&u4pBssid_63_32)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			default:{
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
					Status = FALSE;
					break;
				}
			}
		}

		if (Status == FALSE)
			break;
#ifndef WIFI_UNIFIED_COMMAND
		if (u1ArgNum != rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 " Format Error! ArgNum = %d != %d\n",
				  u1ArgNum, rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum);
			Status = NDIS_STATUS_FAILURE;
			break;
		}
#else
		if (u1ArgNum != SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 " Format Error! ArgNum = %d != %d\n",
				  u1ArgNum, SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM);
			Status = NDIS_STATUS_FAILURE;
			break;
		}
#endif
		else {
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = u4Color_31_0;
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = u4Color_63_32;
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = u4pBssid_31_0;

			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = u4pBssid_63_32;
#ifdef WIFI_UNIFIED_COMMAND
			if (UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrMeshSrgBitmap, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
#else
			if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
#endif
				Status = FALSE;
		}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srmeshsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	u_long u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SW_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#else
	struct SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct UNI_CMD_SR_SW_SRG_BITMAP_T));
#else
	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
#endif

	/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
	rSrCmdSrMeshSrgBitmap.u2Tag = UNI_CMD_SR_BH_MESH_SR_BITMAP;
	rSrCmdSrMeshSrgBitmap.u2Length = sizeof(rSrCmdSrMeshSrgBitmap);
#else
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif


	if (arg) {
		do {
		/* parameter parsing */
		for (u1ArgNum = 0, value = rstrtok(arg, "-");
			value && (u1ArgNum < 4); value = rstrtok(NULL, "-"), u1ArgNum++) {

			if (Status == FALSE)
				break;

			switch (u1ArgNum) {

			case 0:
				if (kstrtol(value, 16, (long *)&u4Color_31_0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 1:
				if (kstrtol(value, 16, (long *)&u4Color_63_32)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 2:
				if (kstrtol(value, 16, (long *)&u4pBssid_31_0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			case 3:
				if (kstrtol(value, 16, (long *)&u4pBssid_63_32)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"set wrong parameters\n");
					return FALSE;
				}
				break;

			default:{
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
					Status = FALSE;
					break;
				}

			}
		}

		if (Status == FALSE)
			break;
#ifndef WIFI_UNIFIED_COMMAND
		if (u1ArgNum != rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 " Format Error! ArgNum = %d != %d\n",
				  u1ArgNum, rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum);
			Status = NDIS_STATUS_FAILURE;
			break;
		}
#else
		if (u1ArgNum != SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 " Format Error! ArgNum = %d != %d\n",
				  u1ArgNum, SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM);
			Status = NDIS_STATUS_FAILURE;
			break;
		}
#endif
		else {
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 = u4Color_31_0;
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 = u4Color_63_32;
			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 = u4pBssid_31_0;

			rSrCmdSrMeshSrgBitmap.rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 = u4pBssid_63_32;
#ifdef WIFI_UNIFIED_COMMAND
			if (UniCmdSRUpdateSrgBitmap(pAd, rSrCmdSrMeshSrgBitmap, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
#else
			if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
#endif
				Status = FALSE;
		}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set srmeshsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}


/** UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
		NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcfgsren=1\n");
		}

		if (Status == NDIS_STATUS_SUCCESS)
			return TRUE;
		else
			return FALSE;

}



/** UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
		NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcfgsren=1\n");
		}

		if (Status == NDIS_STATUS_SUCCESS)
			return TRUE;
		else
			return FALSE;

}

/** UNI_CMD_SR_SET_FORBIT_MESH_SR **/
INT SetSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
		NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_SET_FORBIT_MESH_SR, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcfgsren=1\n");
		}

		if (Status == NDIS_STATUS_SUCCESS)
			return TRUE;
		else
			return FALSE;

}


/** UNI_CMD_SR_SET_FORBIT_MESH_SR **/
INT SetSrMeshResetBhDLForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
		NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_FORHIB_MESH_SR_RESET, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE);

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srcfgsren=1\n");
		}

		if (Status == NDIS_STATUS_SUCCESS)
			return TRUE;
		else
			return FALSE;

}

/** SR_CMD_SET_REMOTE_BH_INFO **/
INT SetSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_8 BhType;
	UINT_16 Wcid;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (pAd->SrMeshTopoLock == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"SrMeshTopoLock:%d Auto for band:%d\n",
				pAd->SrMeshTopoLock, u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 2);
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					BhType = simple_strtol(value, 0, 10);
					if (IsInRange(BhType, u1ArgNum, ENUM_BH_TYPE_NO_WIFI,
						ENUM_BH_TYPE_WIFI) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 1:
					Wcid = simple_strtol(value, 0, 10);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetRemoteAssocBHInfo(pAd, u1DbdcIdx, BhType, Wcid) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			" iwpriv ra0 set srmeshremotebhinfo=[BhType]-[Wcid]\n");
	}

	return Status;
}

/** SR_CMD_SET_REMOTE_BH_INFO **/
NDIS_STATUS SrSetRemoteAssocBHInfo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 BhType, UINT_16 Wcid)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
		struct UNI_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

		os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

		/* Assign Cmd Id */
		rSrCmdMeshTopology.u2Tag = UNI_CMD_SR_HW_REMOTE_BH_INFO;
		rSrCmdMeshTopology.u2Length = sizeof(rSrCmdMeshTopology);
#else
	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_REMOTE_BH_INFO;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteBhParams.u2RemoteBhWcid = Wcid;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteBhParams.u1RemoteBhType = BhType;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology, u1DbdcIdx);
#else
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);
#endif
	return Status;
}

/** SR_CMD_SET_MAP_BALANCE **/
INT SetSrMeshMapBalance(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1DbdcIdx = BAND0;
	UINT8 u1MapBalance;
	u_long input = 0;

	u1DbdcIdx = hc_get_hw_band_idx(pAd);

	if (arg) {
		if (kstrtol(arg, 10, (long *)&input)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		} else {
			u1MapBalance = (UINT8)input;
			if (SrSetMapBalance(pAd, u1DbdcIdx, u1MapBalance) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"set %d\n", u1MapBalance);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			" iwpriv ra0 set srmapbalance=[Map_Balance]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_MAP_TOPO **/
INT SetSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_8 MapDevCount = 0, MapDevSrSupportMode = 0, SelfRole = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (pAd->SrMeshTopoLock == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"SrMeshTopoLock:%u Auto for band:%u\n",
			pAd->SrMeshTopoLock, u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 3);
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					MapDevCount = simple_strtol(value, 0, 10);
					break;

				case 1:
					MapDevSrSupportMode = simple_strtol(value, 0, 10);
					if (IsInRange(MapDevSrSupportMode, u1ArgNum, 0, 2) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 2:
					SelfRole = simple_strtol(value, 0, 10);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetMAPTopo(pAd, u1DbdcIdx, MapDevCount, MapDevSrSupportMode, SelfRole) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			" iwpriv ra0 set srmeshtopo=[MapDevCount]-[MapMtkMeshSrOnly]-[SelfRole]\n");
	}

	return Status;
}

/** SR_CMD_SET_MAP_TOPO **/
NDIS_STATUS SrSetMAPTopo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 MapDevCount, UINT_8 MapDevSrSupportMode, UINT_8 SelfRole)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.u2Tag = UNI_CMD_SR_HW_MAP_TOPO;
	rSrCmdMeshTopology.u2Length = sizeof(rSrCmdMeshTopology);
#else

	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_TOPO;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1MapDevCount = MapDevCount;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1MapDevSrSupportMode = MapDevSrSupportMode;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1SelfRole = SelfRole;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology, u1DbdcIdx);
#else
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);
#endif
	return Status;
}

/** SR_CMD_SET_MAP_TRAFFIC_STATUS **/
INT SetSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1UlStatus = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				switch (u1ArgNum) {

				case 0:
					u1UlStatus = simple_strtol(value, 0, 10);
					if (IsInRange(u1UlStatus, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetUplinkTrafficStatus(pAd, u1DbdcIdx, u1UlStatus) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	return Status;
}

/** SR_CMD_SET_MESH_UL_MODE **/
INT SetSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1UlMode = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				switch (u1ArgNum) {

				case 0:
					u1UlMode = (UCHAR) os_str_tol(value, 0, 10);
					if (IsInRange(u1UlMode, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrCmdSetMeshUlMode(pAd, u1DbdcIdx, u1UlMode) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE)
		MTWF_PRINT(" iwpriv ra0 set srmeshulmode=[u1UlMode]\n");

	return Status;
}

NDIS_STATUS SrSetUplinkTrafficStatus(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx,
	IN UINT8 UlStatus
)
{
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_UL_TRAFFIC_STATUS_T rSrCmdSrUlStatus;

	os_zero_mem(&rSrCmdSrUlStatus, sizeof(rSrCmdSrUlStatus));

	/* Assign Cmd Id */
	rSrCmdSrUlStatus.u2Tag = UNI_CMD_SR_HW_MAP_TRAFFIC_STATUS;
	rSrCmdSrUlStatus.u2Length = sizeof(rSrCmdSrUlStatus);
	rSrCmdSrUlStatus.rSrUlStatus.u1UlStatus = UlStatus;

	return UniCmdMeshUplinkStatusSet(pAd, &rSrCmdSrUlStatus, BandIdx);
#else
	struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T rSrCmdSrUlStatus;

	os_zero_mem(&rSrCmdSrUlStatus, sizeof(rSrCmdSrUlStatus));

	/* Assign Cmd Id */
	rSrCmdSrUlStatus.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_TRAFFIC_STATUS;
	rSrCmdSrUlStatus.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrUlStatus.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdSrUlStatus.rSrUlStatus.u1UlStatus = UlStatus;

	return SrCmdMeshUplinkStatusSet(pAd, &rSrCmdSrUlStatus);
#endif
}

NDIS_STATUS SrCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaFlag not null */
	if (!prSrCmdSrUlStatus) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrUlStatus */
	/* PrintSrCmdSrUlStatus(pAd, prSrCmdSrUlStatus); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdSrUlStatus));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrUlStatus, sizeof(*prSrCmdSrUlStatus));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrSetMapBalance(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value)
{
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SET_MAP_BALANCE_T rSrCmdMapBalance;

	os_zero_mem(&rSrCmdMapBalance, sizeof(rSrCmdMapBalance));

	/* Assign Cmd Id */
	rSrCmdMapBalance.u2Tag = UNI_CMD_SR_HW_MAP_BALANCE;
	rSrCmdMapBalance.u2Length = sizeof(rSrCmdMapBalance);
	rSrCmdMapBalance.rSrMapBalance.u1MapBalance = Value;

	return UniCmdMeshMapBalance(pAd, &rSrCmdMapBalance, BandIdx);
#else
	struct _SR_CMD_SET_MAP_BALANCE_T rSrCmdMapBalance;

	os_zero_mem(&rSrCmdMapBalance, sizeof(rSrCmdMapBalance));

	/* Assign Cmd Id */
	rSrCmdMapBalance.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_BALANCE;
	rSrCmdMapBalance.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMapBalance.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdMapBalance.rSrMapBalance.u1MapBalance = Value;

	return SrCmdMeshMapBalanceSet(pAd, &rSrCmdMapBalance);
#endif
}

NDIS_STATUS SrCmdMeshMapBalanceSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMapBalance not null */
	if (!prSrCmdMapBalance) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMapBalance */
	/* PrintSrCmdSrMapBalance(pAd, prSrCmdMapBalance); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMapBalance));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMapBalance, sizeof(*prSrCmdMapBalance));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

VOID PrintSrCmdSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode)
{
	PrintSrCmd(&(prSrCmdMeshUlMode->rSrCmd));
	MTWF_PRINT("u1UlMode = %u\n",
		prSrCmdMeshUlMode->rSrMeshUlMode.u1UlMode);
}

NDIS_STATUS SrCmdMeshUlModeSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMeshUlMode not null */
	if (!prSrCmdMeshUlMode) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMeshUlMode */
	/* PrintSrCmdSrMeshUlMode(pAd, prSrCmdMeshUlMode); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMeshUlMode));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshUlMode, sizeof(*prSrCmdMeshUlMode));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshUlMode(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_HW_MESH_UL_MODE, BandIdx, Value, TRUE);
#else
	struct _SR_CMD_SET_MESH_UL_MODE_T rSrCmdMeshUlMode;

	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&rSrCmdMeshUlMode, sizeof(rSrCmdMeshUlMode));

	/* Assign Cmd Id */
	rSrCmdMeshUlMode.rSrCmd.u1CmdSubId = SR_CMD_SET_MESH_UL_MODE;
	rSrCmdMeshUlMode.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMeshUlMode.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdMeshUlMode.rSrMeshUlMode.u1UlMode = Value;

	Status = SrCmdMeshUlModeSet(pAd, &rSrCmdMeshUlMode);

#endif

	if (Status == NDIS_STATUS_SUCCESS)
		pAd->SrMeshUlMode = Value;

	return Status;
}

/** Driver Internal **/
INT SetSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1RptLock = 0;
	UINT8 u1DbdcIdx = hc_get_hw_band_idx(pAd);

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					u1RptLock = (UINT8) os_str_tol(value, 0, 10);
					if (IsInRange(u1RptLock, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrMeshStaModeRptLockConfig(pAd, u1DbdcIdx, u1RptLock) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		 "BandIdx:%u, _u1StaModeRptUnLock:%u\n",
		 u1DbdcIdx, pAd->StaModeRptUnLock);

	return Status;
}

NDIS_STATUS SrMeshStaModeRptLockConfig(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 u1RptLock)
{
	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	pAd->StaModeRptUnLock = u1RptLock ? 0 : 1;

	SrMeshSrUpdateSTAMode(pAd, FALSE, FALSE);

	return NDIS_STATUS_SUCCESS;
}

/** SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT SetSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1StaAllHe = 0;
	UINT8 u1DbdcIdx = hc_get_hw_band_idx(pAd);

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				switch (u1ArgNum) {

				case 0:
					u1StaAllHe =  (UINT8) os_str_tol(value, 0, 10);
					if (IsInRange(u1StaAllHe, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetMeshRemoteStaModeRpt(pAd, u1DbdcIdx, u1StaAllHe) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	return Status;
}

NDIS_STATUS SrSetMeshRemoteStaModeRpt(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 RemoteAPStaAllHe)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmd(pAd, UNI_CMD_SR_HW_REMOTE_STA_MODE, BandIdx, RemoteAPStaAllHe, TRUE);
#else
	struct SR_CMD_T rSrCmd;

	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&rSrCmd, sizeof(rSrCmd));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE;
	rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmd.u4Value = RemoteAPStaAllHe;

	Status = SrCmd(pAd, &rSrCmd);
#endif

	return Status;
}

/** SR_CMD_GET_SR_CAP_ALL_INFO **/
INT ShowSrCap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_CAP, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CAP_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srcap=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CAP_ESR_EN **/
INT ShowSrCapEsrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_ENHANCE_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show sresren=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT ShowSrCapNonSrgEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_NONSRG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srnonsrgen=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT ShowSrApCliMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SR_APCLI_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srapclimode=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT ShowSrTxCount(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_16 u2Wlanid = 0;

	struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T srCmdSrGlobalVarShowSrTxCount;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	/* Assign Cmd Id */
	os_zero_mem(&srCmdSrGlobalVarShowSrTxCount, sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T));

	srCmdSrGlobalVarShowSrTxCount.u2Tag = UNI_CMD_SR_SW_SR_TX_COUNT;
	srCmdSrGlobalVarShowSrTxCount.u2Length = sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T);

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					if (kstrtou16(arg, 10, &u2Wlanid)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						return FALSE;
					}
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
			if (u1ArgNum != SR_CMD_GET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"Format Error! ArgNum = %d != %d\n",
						u1ArgNum, SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			srCmdSrGlobalVarShowSrTxCount.u2WlanId = u2Wlanid;
			Status = UniCmdShowSwSrTxCount(pAd, srCmdSrGlobalVarShowSrTxCount, u1DbdcIdx);
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show sr_txcount=wlandid\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_PARA_ALL_INFO **/
INT ShowSrPara(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_PARA, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_PARA_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srpara=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_IND_ALL_INFO **/
INT ShowSrInd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_IND, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IND_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srind=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO **/
INT ShowSrInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0, u1DropTaIdx = 0, u1StaIdx = 0;
#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T srCmdSrGlobalVarSingleDropTa;
#else
	struct SR_CMD_T rSrCmd;
#endif

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	/* Assign Cmd Id */
#ifdef WIFI_UNIFIED_COMMAND
	os_zero_mem(&srCmdSrGlobalVarSingleDropTa, sizeof(struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	srCmdSrGlobalVarSingleDropTa.u2Tag = UNI_CMD_SR_GLOVAR_SINGLE_DROP_TA_INFO;
	srCmdSrGlobalVarSingleDropTa.u2Length = sizeof(struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T);
#else
	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));

	rSrCmd.u1CmdSubId = SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO;
	rSrCmd.u1ArgNum = SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
#endif

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 2);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u1DropTaIdx = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u1DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u1StaIdx = simple_strtol(value, 0, 10);
					Status = IsInRange(u1StaIdx, u1ArgNum, 0, SR_STA_NUM);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;
#ifdef WIFI_UNIFIED_COMMAND
			if (u1ArgNum != SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						"Format Error! ArgNum = %d != %d\n",
						u1ArgNum, SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			srCmdSrGlobalVarSingleDropTa.u1DropTaIdx = u1DropTaIdx;
			srCmdSrGlobalVarSingleDropTa.u1StaIdx = u1StaIdx;
			Status = UniCmdShowGloVarSingleDropTa(pAd, srCmdSrGlobalVarSingleDropTa, u1DbdcIdx);
#else
			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmd.u1DropTaIdx = u1DropTaIdx;
				rSrCmd.u1StaIdx = u1StaIdx;
				Status = SrCmd(pAd, &rSrCmd);
			}
#endif
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srinfo=0-0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_COND_ALL_INFO **/
INT ShowSrCond(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_COND, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_COND_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srcond=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_RCPI_TBL_ALL_INFO **/
INT ShowSrRcpiTbl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_RCPI_TBL, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srrcpitbl=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO **/
INT ShowSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_RCPI_TBL_OFST, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srrcpitblofst=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_Q_CTRL_ALL_INFO **/
INT ShowSrQCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_Q_CTRL, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_Q_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srqctrl=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_IBPD_ALL_INFO **/
INT ShowSrIBPD(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_IBPD, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IBPD_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show sribpd=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_NRT_ALL_INFO **/
INT ShowSrNRT(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_NRT, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srnrt=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_NRT_CTRL_ALL_INFO **/
INT ShowSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_NRT_CTRL, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srnrtctrl=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO **/
INT ShowSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_FNQ, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"iwpriv ra0 show srfnqctrl=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_FRM_FILT_ALL_INFO **/
INT ShowSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_FRMFILT, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_FRM_FILT_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"iwpriv ra0 show srfrmfilt=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO **/
INT ShowSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_INTERPS_CTRL, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"iwpriv ra0 show srinterpsctrl=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO **/
INT ShowSrInterPsDbgAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_INTERPS_DBG, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"iwpriv ra0 show srinterpsdbg=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
INT ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srcfgsren=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
INT ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrsden=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_BF **/
INT ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_BF, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_BF, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrbf=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_ATF **/
INT ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_ATF, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ATF, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsratf=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_MODE **/
INT ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_MODE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrmode=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
INT ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DISRT_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgdisrten=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
INT ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DISRT_MIN_RSSI, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgdisrtmin=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_QUEUE **/
INT ShowSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_QUEUE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QUEUE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgtxcq=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_QID **/
INT ShowSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_QID, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QID, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgtxcqid=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_PATH **/
INT ShowSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_TXC_PATH, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_PATH, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgtxcpath=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_AC_METHOD **/
INT ShowSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_AC_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_AC_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgac=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_PERIOD_THR **/
INT ShowSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_PERIOD_THR, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_PERIOD_THR, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrperiodthr=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD **/
INT ShowSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_QUERY_TXD_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrquerytxd=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO **/
INT ShowSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_CG_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrsdcg=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO **/
INT ShowSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrsdobss=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_PROFILE **/
INT ShowSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_PROFILE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_PROFILE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srcfgsrprofile=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_FNQ_ENABLE **/
INT ShowSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
#ifdef WIFI_UNIFIED_COMMAND
		Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_FNQ_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_FNQ_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 show srcfgfnqen=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Command not supported\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
INT ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_DPD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DPD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcfgdpden=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ENABLE **/
INT ShowSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcfgsrtxen=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE **/
INT ShowSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcfgomen=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE **/
INT ShowSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcfgsrtxalignen=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
INT ShowSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show  srcfgsrtxalignrssi=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_DABS_MODE **/
INT ShowSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_DABS_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_DABS_MODE, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcfgdabsmode=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_CNT_ALL **/
INT ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_SW_CNT, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CNT_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srcnt=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_SD_ALL **/
INT ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_SW_SD, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SD_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show  srsd=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_SRG_BITMAP **/
INT ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_SW_SRG_BITMAP, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
#else
	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SRG_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM);
#endif
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srsrgbm=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

/** SR_CMD_GET_SR_MESH_SRG_BITMAP **/
INT ShowSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_SW_MESH_SRG_BITMAP, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_MESH_SRG_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshsrgbm=0\n");
	}

	return Status;
}

/** SR_CMD_GET_SR_SIGA_FLAG_INFO **/
INT ShowSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_SIGA_FLAG, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_SIGA_FLAG_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srsiga=0\n");
	}

	return Status;
}

/** SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO **/
INT ShowSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_SIGA_AUTO_FLAG, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srsigaauto=0\n");
	}

	return Status;
}

/** Driver Internal **/
INT ShowSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT_8 u1ArgNum, u1Mode = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap = NULL;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					u1Mode = simple_strtol(value, 0, 10);
					if (IsInRange(u1Mode, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_GET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_GET_DEFAULT_ARG_NUM);
				Status = FALSE;
				break;
			}

			if (u1Mode == 1)
				prSrgBitmap = &pAd->SrSelfSrgBMMan;
			else
				prSrgBitmap = &pAd->SrSelfSrgBM;
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (prSrgBitmap != NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			"BandIdx:%u, Mode:%u BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
			u1BandIdx, u1Mode, prSrgBitmap->u4Color_63_32,
			prSrgBitmap->u4Color_31_0, prSrgBitmap->u4pBssid_63_32,
			prSrgBitmap->u4pBssid_31_0);
	}

	return Status;
}

/** Driver Internal **/
INT ShowSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"BandIdx:%u, SrMeshTopoLock=%u\n",
		u1BandIdx, pAd->SrMeshTopoLock);

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_REMOTE_FH_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_REMOTE_FH_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshstaTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_DOWNLINK_STA_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_REMOTE_FH_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_FH_MESH_SR_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_BH_MESH_SR_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}


/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_BH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_FH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_SET_FORBIT_MESH_SR, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_BH_INFO **/
INT ShowSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_REMOTE_BH_INFO, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_REMOTE_BH_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshremotebhinfo=0\n");
	}

	return Status;
}

/** SR_CMD_GET_MAP_TOPO **/
INT ShowSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_MAP_TOPO, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_MAP_TOPO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show  srmeshtopo=0\n");
	}

	return Status;
}

/** Driver Internal **/
INT ShowSrMeshTopoUpdateParams(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;
	struct sr_mesh_topology_update_params *prParams;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	prParams = &pAd->SrMeshTopologyUpdateParams;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"BandIdx:%u, map_dev_count=%u map_dev_sr_support_mode:%u self_role:%u\n",
		u1BandIdx, prParams->topo_params.map_dev_count,
		prParams->topo_params.map_dev_sr_support_mode, prParams->topo_params.self_role);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"AL MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_al_mac));

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"FH BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_fh_bssid));

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"FH SSID:%s SSIDLen:%u\n",
		prParams->topo_params.ssid, prParams->topo_params.ssid_len);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_bh_mac));

	return Status;
}

/** SR_CMD_GET_MAP_TRAFFIC_STATUS **/
INT ShowSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_MAP_TRAFFIC_STATUS, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_MAP_TRAFFIC_STATUS, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srmeshuplinkevent=0\n");
	}

	return Status;
}

/** SR_CMD_GET_MESH_PHASE **/
INT ShowSrMeshPhase(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_MESH_PHASE, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_MESH_PHASE, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srmeshphase=0\n");
	}

	return Status;
}

INT ShowSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	MTWF_PRINT("BandIdx:%u, SRMeshUlMode:%u\n",
		u1BandIdx, pAd->SrMeshUlMode);

	return Status;

}

/** Driver Internal **/
INT ShowSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1BandIdx = hc_get_hw_band_idx(pAd);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"BandIdx:%u, StaModeRptUnLock=%u\n",
		u1BandIdx, pAd->StaModeRptUnLock);

	return Status;
}

/** SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT ShowSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_HW_REMOTE_STA_MODE, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS)
#else
	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS)
#endif
	{
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srmeshallremotestahe=0\n");
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SUPPORT_MODE **/
INT ShowSrCfgSrSupportMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

#ifdef WIFI_UNIFIED_COMMAND
	Status = UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SUPPORT_MODE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 show srcfgsrsupportmode=0\n");
	}
	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
#else
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"not support Show SR supportmode cmd\n");
	return FALSE;
#endif
}

/* for set/show function*/
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrCap not null */
	if (!_prSrCmdSrCap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V1 *prSrCmdSrCap = (struct SR_CMD_SR_CAP_T_SR_V1 *)_prSrCmdSrCap;
		/* Print prSrCmdSrCap */
		PrintSrCmdSrCap(pAd, prSrCmdSrCap);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_CAP_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V2 *prSrCmdSrCap = (struct SR_CMD_SR_CAP_T_SR_V2 *)_prSrCmdSrCap;
		/* Print prSrCmdSrCap */
		PrintSrCmdSrCap(pAd, prSrCmdSrCap);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_CAP_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, sizeof(struct SR_CMD_SR_CAP_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_PARA_T *prSrCmdSrPara)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrPara not null */
	if (!prSrCmdSrPara) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrPara */
	PrintSrCmdSrPara(prSrCmdSrPara);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_PARA_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrPara, sizeof(struct SR_CMD_SR_PARA_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS
SrCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd,
				IN struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
				prSrCmdSrGlobalVarSingleDropTa, IN UINT_32 u4DropTaIdx,
				IN UINT_32 u4StaIdx)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrEventSrGlobalVar not null */
	if (!prSrCmdSrGlobalVarSingleDropTa) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrEventSrGlobalVar */
	PrintSrCmdSrGloVarSingleDropTa(prSrCmdSrGlobalVarSingleDropTa, u4DropTaIdx, u4StaIdx);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrGlobalVarSingleDropTa,
			  sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrCond not null */
	if (!_prSrCmdSrCond) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_COND_T_SR_V1 *prSrCmdSrCond = (struct SR_CMD_SR_COND_T_SR_V1 *)_prSrCmdSrCond;
		/* Print prSrCmdSrCond */
		PrintSrCmdSrCond(pAd, prSrCmdSrCond);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_COND_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCond, sizeof(struct SR_CMD_SR_COND_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_COND_T_SR_V2 *prSrCmdSrCond = (struct SR_CMD_SR_COND_T_SR_V2 *)_prSrCmdSrCond;
		/* Print prSrCmdSrCond */
		PrintSrCmdSrCond(pAd, prSrCmdSrCond);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_COND_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCond, sizeof(struct SR_CMD_SR_COND_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrRcpiTbl not null */
	if (!prSrCmdSrRcpiTbl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrRcpiTbl */
	PrintSrCmdSrRcpiTbl(prSrCmdSrRcpiTbl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_RCPITBL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTbl, sizeof(struct SR_CMD_SR_RCPITBL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrRcpiTblOfst not null */
	if (!_prSrCmdSrRcpiTblOfst) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 *prSrCmdSrRcpiTblOfst = (struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 *) _prSrCmdSrRcpiTblOfst;
		/* Print prSrCmdSrRcpiTblOfst */
		PrintSrCmdSrRcpiTblOfst(pAd, prSrCmdSrRcpiTblOfst);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 *prSrCmdSrRcpiTblOfst = (struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 *) _prSrCmdSrRcpiTblOfst;
		/* Print prSrCmdSrRcpiTblOfst */
		PrintSrCmdSrRcpiTblOfst(pAd, prSrCmdSrRcpiTblOfst);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, sizeof(struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrQCtrl not null */
	if (!_prSrCmdSrQCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_Q_CTRL_T_SR_V1 *prSrCmdSrQCtrl = (struct SR_CMD_SR_Q_CTRL_T_SR_V1 *)_prSrCmdSrQCtrl;
		/* Print prSrCmdSrQCtrl */
		PrintSrCmdSrQCtrl(pAd, prSrCmdSrQCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_Q_CTRL_T_SR_V2 *prSrCmdSrQCtrl = (struct SR_CMD_SR_Q_CTRL_T_SR_V2 *)_prSrCmdSrQCtrl;
		/* Print prSrCmdSrQCtrl */
		PrintSrCmdSrQCtrl(pAd, prSrCmdSrQCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, sizeof(struct SR_CMD_SR_Q_CTRL_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_IBPD_T *prSrCmdSrIBPD)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrIBPD not null */
	if (!prSrCmdSrIBPD) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrIBPD */
	PrintSrCmdSrIBPD(prSrCmdSrIBPD);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_IBPD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrIBPD, sizeof(struct SR_CMD_SR_IBPD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrNRT not null */
	if (!_prSrCmdSrNRT) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_T_SR_V1 *prSrCmdSrNRT = (struct SR_CMD_SR_NRT_T_SR_V1 *)_prSrCmdSrNRT;
		/* Print prSrCmdSrNRT */
		PrintSrCmdSrNRT(pAd, prSrCmdSrNRT);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_NRT_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, sizeof(struct SR_CMD_SR_NRT_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_NRT_T_SR_V2 *prSrCmdSrNRT = (struct SR_CMD_SR_NRT_T_SR_V2 *)_prSrCmdSrNRT;
		/* Print prSrCmdSrNRT */
		PrintSrCmdSrNRT(pAd, prSrCmdSrNRT);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_NRT_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, sizeof(struct SR_CMD_SR_NRT_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrNRTCtrl not null */
	if (!_prSrCmdSrNRTCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_CTRL_T_SR_V1 *prSrCmdSrNRTCtrl = (struct SR_CMD_SR_NRT_CTRL_T_SR_V1 *)_prSrCmdSrNRTCtrl;
		/* Print prSrCmdSrNRTCtrl */
		PrintSrCmdSrNRTCtrl(pAd, prSrCmdSrNRTCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_NRT_CTRL_T_SR_V2 *prSrCmdSrNRTCtrl = (struct SR_CMD_SR_NRT_CTRL_T_SR_V2 *)_prSrCmdSrNRTCtrl;
		/* Print prSrCmdSrNRTCtrl */
		PrintSrCmdSrNRTCtrl(pAd, prSrCmdSrNRTCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, sizeof(struct SR_CMD_SR_NRT_CTRL_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrFNQCtrl not null */
	if (!prSrCmdSrFNQCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrFNQCtrl */
	PrintSrCmdSrFNQCtrl(prSrCmdSrFNQCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_FNQ_CTRL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrFNQCtrl, sizeof(struct SR_CMD_SR_FNQ_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateFrmFilt(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_FRM_FILT_T *prSrCmdSrFrmFilt)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrFrmFilt not null */
	if (!prSrCmdSrFrmFilt) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrFrmFilt */
	PrintSrCmdSrFrmFilt(prSrCmdSrFrmFilt);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_FRM_FILT_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrFrmFilt, sizeof(struct SR_CMD_SR_FRM_FILT_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrInterPsCtrl not null */
	if (!prSrCmdSrInterPsCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrInterPsCtrl */
	PrintSrCmdSrInterPsCtrl(prSrCmdSrInterPsCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_INTERPS_CTRL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrInterPsCtrl, sizeof(struct SR_CMD_SR_INTERPS_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_SRG_BITMAP_T *prSrCmdSrSrgBitmap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSrgBitmap not null */
	if (!prSrCmdSrSrgBitmap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSrgBitmap */
	/* PrintSrCmdSrSrgBitmap(prSrCmdSrSrgBitmap); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSrgBitmap, sizeof(struct SR_CMD_SR_SRG_BITMAP_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_SR_SIGA_FLAG_T *prSrCmdSrSigaFlag)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaFlag not null */
	if (!prSrCmdSrSigaFlag) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSigaFlag */
	PrintSrCmdSrSiga(pAd, prSrCmdSrSigaFlag);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_SR_SIGA_FLAG_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaFlag, sizeof(struct SR_CMD_SR_SIGA_FLAG_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaAutoFlag not null */
	if (!prSrCmdSrSigaAutoFlag) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSigaAutoFlag */
	/* PrintSrCmdSrSigaAuto(pAd, prSrCmdSrSigaAutoFlag); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdSrSigaAutoFlag));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaAutoFlag, sizeof(*prSrCmdSrSigaAutoFlag));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMeshTopo not null */
	if (!prSrCmdMeshTopo) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMeshTopo */
	/* PrintSrCmdMeshTopo(prSrCmdMeshTopo); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMeshTopo));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshTopo, sizeof(*prSrCmdMeshTopo));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	struct SR_CMD_T rSrCmd;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmd, sizeof(struct SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = u1CmdSubId;
	rSrCmd.u1ArgNum = u1ArgNum;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					rSrCmd.u4Value = simple_strtol(value, 0, 10);
#ifdef CFG_BIG_ENDIAN
					rSrCmd.u4Value = cpu2le32(rSrCmd.u4Value);
#endif
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					 u1ArgNum, rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				Status = SrCmd(pAd, &rSrCmd);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


NDIS_STATUS
SrCmd(IN PRTMP_ADAPTER pAd, IN struct SR_CMD_T *prSrcmd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrcmd not null */
	if (!prSrcmd) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmd */
	if (prSrcmd->u1CmdSubId != SR_CMD_SET_SR_MESH_SR_SD_CTRL)
		PrintSrCmd(prSrcmd);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(struct SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrcmd, sizeof(struct SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

#ifdef WIFI_UNIFIED_COMMAND
NDIS_STATUS UniSrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum, UINT_8 isSet)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_8 argNum = u1ArgNum;

	UINT_32 u4Value;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 1);
				value = rstrtok(NULL, "-"), u1ArgNum++) {
				switch (u1ArgNum) {
				case 0:
					u4Value = os_str_tol(value, 0, 10);
#ifdef CFG_BIG_ENDIAN
					u4Value = cpu2le32(rSrCmd.u4Value);
#endif
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != argNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					 u1ArgNum, argNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			Status = UniSrCmd(pAd, u1CmdSubId, u1DbdcIdx, u4Value, isSet);
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}
#ifdef RT_CFG80211_SUPPORT
#endif

NDIS_STATUS
UniSrCmd(IN PRTMP_ADAPTER pAd, UINT_8 u1CmdSubId, UINT_8 u1DbdcIdx, UINT_32 u4Value, UINT_8 isSet)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T	 UniCmdSR;
	struct UNI_CMD_SR_CMD_T UniCmdSrCmd;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}


	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	os_zero_mem(&UniCmdSrCmd, sizeof(UniCmdSrCmd));

	UniCmdSR.u1BandIdx = u1DbdcIdx;
	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdSrCmd);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
#ifdef RT_CFG80211_SUPPORT
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	if (isSet)
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	else
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

		/* Step 4: Fill and append TLV parameters here */
	UniCmdSrCmd.u2Tag = u1CmdSubId;
	UniCmdSrCmd.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef CFG_BIG_ENDIAN
	UniCmdSrCmd.u2Tag	= cpu2le16(UniCmdSrCmd.u2Tag);
	UniCmdSrCmd.u2Length = cpu2le16(UniCmdSrCmd.u2Length);
#endif /* CFG_BIG_ENDIAN */
	UniCmdSrCmd.u4Value = u4Value;

	AndesAppendCmdMsg(msg, (char *)&UniCmdSrCmd, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;

}

NDIS_STATUS UniCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_SRG_BITMAP_T prSrCmdSrSrgBitmap, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;


	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(prSrCmdSrSrgBitmap);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&prSrCmdSrSrgBitmap, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T *prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SW_GLOVAR_DROPTA_INFO_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrGlobalVarSingleDropTa, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRStaInfo(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T *prSrCmdSrGlobalVarStaInfo,  IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrGlobalVarStaInfo, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateGloVarStaInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg, IN UINT_8 u1ArgTotal, IN BOOLEAN isStaInit)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4DropTaIdx = 0, u4StaIdx = 0, u4SrRateOffset = 0, u4WlanId = 0;

	struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T *prSrCmdSrGlobalVarStaInfo;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarStaInfo, sizeof(struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
	if (!prSrCmdSrGlobalVarStaInfo)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarStaInfo);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarStaInfo, sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarStaInfo->u2Tag = UNI_CMD_SR_SW_GLOVAR_STA_INFO;
	prSrCmdSrGlobalVarStaInfo->u2Length = sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_INFO_T);
	prSrCmdSrGlobalVarStaInfo->fgIsInit = isStaInit;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value && (u1ArgNum < 4);
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = os_str_tol(value, 0, 10);
					/** Not Support set all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
						IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4StaIdx = os_str_tol(value, 0, 10);
					/** Not Support set all STA SR_STA_NUM - 1**/
					Status =
						IsInRange(u4StaIdx, u1ArgNum, 0, SR_STA_NUM - 1);
					break;
				case 2:
					u4WlanId = os_str_tol(value, 0, 10);
					Status = IsInRange(u4WlanId, u1ArgNum, 0, 256);
					break;
				case 3:
					u4SrRateOffset = os_str_tol(value, 0, 10);
					Status =
						IsInRange(u4SrRateOffset, u1ArgNum, 0, 11);
					break;
				default:{
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
						 "set wrong parameters\n");
					Status = NDIS_STATUS_FAILURE;
					break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != u1ArgTotal) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, u1ArgTotal);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			/** Update GlobalVar **/
			prSrCmdSrGlobalVarStaInfo->u1DropTaIdx = u4DropTaIdx;
			prSrCmdSrGlobalVarStaInfo->u1StaIdx = u4StaIdx;
			prSrCmdSrGlobalVarStaInfo->u2WlanId = u4WlanId;
#ifdef CFG_BIG_ENDIAN
			prSrCmdSrGlobalVarStaInfo->u2WlanId =
					cpu2le16(prSrCmdSrGlobalVarStaInfo->u2WlanId);
#endif

			prSrCmdSrGlobalVarStaInfo->u1SrRateOffset = u4SrRateOffset;
			/** Set    GlobalVar **/
			Status = UniCmdSRStaInfo(pAd,
							prSrCmdSrGlobalVarStaInfo,
							u1DbdcIdx);
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				 "iwpriv ra0 set srsta=[u4DropTaIdx]-[u4StaIdx]-[u4WlanId]-[u4SrRateOffset]\n");
		}

		if (prSrCmdSrGlobalVarStaInfo)
			os_free_mem(prSrCmdSrGlobalVarStaInfo);
		return Status;


}

NDIS_STATUS UniCmdShowGloVarSingleDropTa(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T srCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SR_GLOBAL_VAR_SINGLE_DROP_TA_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&srCmdSrGlobalVarSingleDropTa, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdShowSwSrTxCount(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T srCmdSrGlobalVarShowSrTxCount, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SW_GLOVAR_STA_REC_INFO_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}


	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&srCmdSrGlobalVarShowSrTxCount, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}


NDIS_STATUS UniCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN struct UNI_CMD_SR_HW_CAP_T *prSrCmdSrCap, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_HW_CAP_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, struct UNI_SR_CMD_SR_PARA_T *prSrCmdSrPara, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_SR_CMD_SR_PARA_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrPara, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;

}

NDIS_STATUS UniCmdSRUpdateCondV2(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_COND_T_SR_V2 *rSrCmdSrCond, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_COND_T_SR_V2);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)rSrCmdSrCond, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_RCPITBL_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTbl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateRcpiTblOfstV2(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2 *prSrCmdSrRcpiTblOfst, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_RCPITBL_OFST_T_SR_V2);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_Q_CTRL_T_SR_V2 *prSrCmdSrQCtrl, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_Q_CTRL_T_SR_V2);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_IBPD_T *prSrCmdSrIBPD, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_IBPD_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrIBPD, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_NRT_T_SR_V2 *prSrCmdSrNRT, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_NRT_T_SR_V2);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_NRT_CTRL_T_SR_V2 *prSrCmdSrNRTCtrl, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_NRT_CTRL_T_SR_V2);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_FNQ_CTRL_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrFNQCtrl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_INTERPS_CTRL_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrInterPsCtrl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SIGA_FLAG_T *prSrCmdSrSigaFlag, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SIGA_FLAG_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaFlag, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_SIGA_AUTO_FLAG_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaAutoFlag, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, struct UNI_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopology, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_MESH_TOPOLOGY_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshTopology, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdMeshUplinkStatus, IN UINT_8 u1DbdcIdx)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SR_UL_TRAFFIC_STATUS_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshUplinkStatus, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdMeshMapBalance(IN PRTMP_ADAPTER pAd, struct UNI_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance, IN UINT_8 u1DbdcIdx)

{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdSR;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSR);
	os_zero_mem(&UniCmdSR, u4ComCmdSize);
	UniCmdSR.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_SET_MAP_BALANCE_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdSR, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)prSrCmdMapBalance, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdCsrRst(
	RTMP_ADAPTER * pAd,
	UINT32 csrCoordApInfoRst,
	UINT32 u4TargetIndex
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	struct UNI_CMD_CSR_RESET_AP_STA_INFO_T rCmdCsrRst;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	rCmdCsrRst.u2Tag = UNI_CMD_CSR_COORDAP_RESET;
	rCmdCsrRst.u2Length = sizeof(struct UNI_CMD_SR_CMD_T);
	rCmdCsrRst.u1ResetTarget = csrCoordApInfoRst;
	if (csrCoordApInfoRst == ENUM_COSR_RESET_TYPE_AP)
		rCmdCsrRst.u1ResetApIndex = u4TargetIndex;
	else if (csrCoordApInfoRst == ENUM_COSR_RESET_TYPE_STA)
		rCmdCsrRst.u1ResetApIndex = u4TargetIndex;

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_RESET_AP_STA_INFO_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&rCmdCsrRst, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCsrCmdSetCoordAPInfo(
	RTMP_ADAPTER * pAd,
	struct UNI_CMD_CSR_APINFO csrCoordApinfo
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;


	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	csrCoordApinfo.u2Tag = UNI_CMD_CSR_COORDAP_INFO;
	csrCoordApinfo.u2Length = sizeof(struct UNI_CMD_CSR_APINFO);

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_APINFO);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&csrCoordApinfo, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdSetCsrStaInfo(
	RTMP_ADAPTER * pAd,
	struct UNI_CMD_CSR_STAINFO rcsrStaInfo
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;


	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	rcsrStaInfo.u2Tag = UNI_CMD_CSR_STA_INFO;
	rcsrStaInfo.u2Length = sizeof(struct UNI_CMD_CSR_STAINFO);

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_STAINFO);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&rcsrStaInfo, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdCsrInfo(
	RTMP_ADAPTER * pAd,
	struct UNI_CMD_CSR_INFO_T rcsrInfo
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;


	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_INFO_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&rcsrInfo, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdCsrFixedPwr(
	RTMP_ADAPTER * pAd,
	struct UNI_CMD_CSR_FIXED_TXPOWER_T rCsrFixedPwr
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;


	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	rCsrFixedPwr.u2Tag = UNI_CMD_CSR_FIX_PWR;
	rCsrFixedPwr.u2Length = sizeof(struct UNI_CMD_CSR_FIXED_TXPOWER_T);

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_FIXED_TXPOWER_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&rCsrFixedPwr, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}

NDIS_STATUS UniCmdCsrFixedRate(
	RTMP_ADAPTER * pAd,
	struct UNI_CMD_CSR_FIXED_RATE_T rCsrFixedRate
)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	struct UNI_CMD_SR_T  UniCmdCsr;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;


	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	rCsrFixedRate.u2Tag = UNI_CMD_CSR_FIX_RATE;
	rCsrFixedRate.u2Length = sizeof(struct UNI_CMD_CSR_FIXED_RATE_T);

	u4ComCmdSize = sizeof(UniCmdCsr);
	os_zero_mem(&UniCmdCsr, u4ComCmdSize);
	UniCmdCsr.u1BandIdx = u1DbdcIdx;

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_CSR_FIXED_RATE_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SR);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdCsr, (u4ComCmdSize));

	AndesAppendCmdMsg(msg, (char *)&rCsrFixedRate, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
			"(ret = %d)\n", ret);

	return ret;
}


INT SetCsrEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_CSR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, TRUE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set cosrenable=1\n");
	}

	return Status;
}
INT SetCsrCoordAPInfoRst(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 csrCoordApInfoRst;
	UINT32 u4Recv = 0;
	INT Status = NDIS_STATUS_SUCCESS;
	UINT32 u4TargetIndex;

	csrCoordApInfoRst = os_str_tol(arg, 0, 10);

	u4Recv = sscanf(arg, "%d-%d", &csrCoordApInfoRst, &u4TargetIndex);

	if (u4Recv > 2) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of arguments\n");
		return FALSE;
	}

	Status = UniCmdCsrRst(pAd, csrCoordApInfoRst, u4TargetIndex);

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT SetCsrCoordAPUpdateMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 csrCoordApUpdateMode;
	INT Status = NDIS_STATUS_SUCCESS;

	csrCoordApUpdateMode = os_str_tol(arg, 0, 10);
	if (csrCoordApUpdateMode > 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"csrCoordApUpdateMode Auto Mode = 0, Manual Mode = 1\n"
			);
		return FALSE;
	}
	g_u1CSRCoordAPMode = csrCoordApUpdateMode;

	return Status;
}

INT SetCsrFixNofiyPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct UNI_CMD_CSR_FIXED_TXPOWER_T rCsrFixedTxPwr = {0};
	UINT32 u4Recv = 0;
	UINT32 staId = 0, apId = 0;
	UINT32 mode = 0, fixedTxPwr = 0;
	INT Status = TRUE;

	u4Recv = sscanf(arg, "%d-%d-%d-%d", &staId, &apId, &mode, &fixedTxPwr);
	if (u4Recv > 4) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of arguments\n");
		return FALSE;
	}
	rCsrFixedTxPwr.u1STAid = staId;
	rCsrFixedTxPwr.u1APid = apId;
	rCsrFixedTxPwr.u1Mode = mode;
	rCsrFixedTxPwr.u1FixedTXpower = fixedTxPwr;
	if (UniCmdCsrFixedPwr(pAd, rCsrFixedTxPwr) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 set CoSRFixNofiyPwr=staId-apId-mode-FixedTxPower\n");
	}
	return Status;
}

INT SetCsrFixRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct UNI_CMD_CSR_FIXED_RATE_T rCsrFixedRate = {0};
	UINT32 u4Recv = 0;
	UINT32 staId = 0, apId = 0;
	UINT32 mode = 0, fixedRate = 0;
	INT Status = TRUE;

	u4Recv = sscanf(arg, "%d-%d-%d-%d", &staId, &apId, &mode, &fixedRate);
	if (u4Recv > 4) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of arguments\n");
		return FALSE;
	}
	rCsrFixedRate.u1STAid = staId;
	rCsrFixedRate.u1APid = apId;
	rCsrFixedRate.u1Mode = mode;
	rCsrFixedRate.u1MCS = fixedRate;
	if (UniCmdCsrFixedRate(pAd, rCsrFixedRate) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 set CoSRFixRate=staId-apId-mode-mcs\n");
	}
	return Status;
}

INT SetCsrStaUpdateMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 csrStaUpdateMode;
	INT Status = NDIS_STATUS_SUCCESS;

	csrStaUpdateMode = os_str_tol(arg, 0, 10);
	if (csrStaUpdateMode > 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"csrStaUpdateMode Auto Mode = 0, Manual Mode = 1\n"
			);
		return FALSE;
	}
	g_u1CSRStaUpdMode = csrStaUpdateMode;

	return Status;
}

INT SetCsrCoordAPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *thisChar;
	UINT8 ArgIdx = 0;
	INT Status = TRUE;
	struct UNI_CMD_CSR_APINFO csrCoordApinfo;

	if (g_u1CSRCoordAPMode != 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"CSR coordAPMode is not set as Manual mode\n");
		return FALSE;
	}
	os_zero_mem(&csrCoordApinfo, sizeof(struct UNI_CMD_CSR_APINFO));
	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		switch (ArgIdx) {
		case 0: /* AP_ID. */
			csrCoordApinfo.apId = (UINT8) os_str_tol(thisChar, 0, 10);
			break;
		case 1: /* Status */
			csrCoordApinfo.CoorAPStatus = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 2: /* MAC Addr */
			if (strlen(thisChar) == 17) {
				CHAR *token;
				INT i = 0;

				token = rstrtok(thisChar, ":");

				while (token != NULL) {
					AtoH(token, (char *) &csrCoordApinfo.aCoorAPBSSID[i], 1);
					i++;
					if (i >= MAC_ADDR_LEN)
						break;
					token = rstrtok(NULL, ":");
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
					"wrong input xx:xx:xx:xx:xx:xx\n");
				Status = FALSE;
			}
			break;
		case 3: /* RSSI */
			csrCoordApinfo.u1Rssi = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		}
		ArgIdx++;
	}
	if (ArgIdx > 4) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
		"wrong Number of arguments\n");
		Status = FALSE;
	}
	if (UniCsrCmdSetCoordAPInfo(pAd, csrCoordApinfo) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 set CSRCoordAPInfo=0\n");
	}
	return Status;
}

INT SetCsrStaInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct UNI_CMD_CSR_STAINFO rCsrStaInfo;
	UINT_32 u4Recv = 0;
	UINT_32 support_11k = 0, wcid = 0, mldid = 0;
	UINT_32 plDiff_0 = 0, plDiff_1 = 0, plDiff_2 = 0;
	UINT_32 u1status = 0, candStaId = 0;

	if (g_u1CSRStaUpdMode != 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"CSR StaUpdMode is not set to Manual mode\n");
		return FALSE;
	}
	os_zero_mem(&rCsrStaInfo, sizeof(struct UNI_CMD_CSR_STAINFO));
	u4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d", &candStaId, &wcid, &mldid, &plDiff_0,
		&plDiff_1, &plDiff_2, &support_11k, &u1status);

	if (u4Recv != 8) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"iwpriv ra0 set CSRStaInfo=wcid-mldid-pldiff_0-pldiff_1-pldiff_2-11kSupport-Status\n");
		return FALSE;
	}
	rCsrStaInfo.u1Candstaid = candStaId;
	rCsrStaInfo.u1Wcid = wcid;
	rCsrStaInfo.u1MLDid = mldid;
	rCsrStaInfo.u1Pldiff[0] = plDiff_0;
	rCsrStaInfo.u1Pldiff[1] = plDiff_1;
	rCsrStaInfo.u1Pldiff[2] = plDiff_2;
	rCsrStaInfo.u180211ksupport = support_11k;
	rCsrStaInfo.u1status = u1status;

	if (UniCmdSetCsrStaInfo(pAd, rCsrStaInfo) != NDIS_STATUS_SUCCESS)
		return FALSE;
	else
		return TRUE;
}

INT ShowCsrEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_CSR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show cosrenable=1\n");
	}

	return Status;
}

INT ShowCsrCoordApUpdMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (g_u1CSRCoordAPMode == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"csrCoordApUpdateMode = Auto Mode\n"
			);
	} else if (g_u1CSRCoordAPMode == 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"csrCoordApUpdateMode = Manual Mode\n"
			);
	}
	return Status;
}

INT ShowCsrCoordApInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_CSR_COORDAP_INFO, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show CSRCoordAPInfo=0\n");
	}

	return Status;
}

INT ShowCsrInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;
	UINT_32 u4Recv = 0;
	UINT_32 staId = 0, apId;
	struct UNI_CMD_CSR_INFO_T rCsrInfo = {0};

	u4Recv = sscanf(arg, "%d-%d", &staId, &apId);
	if (u4Recv != 2) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of argument\n");
		return FALSE;
	}

	rCsrInfo.u2Tag = UNI_CMD_CSR_INFO;
	rCsrInfo.u2Length = sizeof(struct UNI_CMD_CSR_INFO_T);

	rCsrInfo.u1STAid = staId;
	rCsrInfo.u1APid = apId;
	if (UniCmdCsrInfo(pAd, rCsrInfo) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show cosrinfo=staId-apId\n");
	}
	return Status;
}

INT ShowCsrEventCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_CSR_EVENT_CNT, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show cosreventcnt=0\n");
	}

	return Status;
}

INT ShowCsrStaUpdMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (g_u1CSRStaUpdMode == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"CSRStaUpdMode = Auto Mode\n"
			);
	} else if (g_u1CSRStaUpdMode == 1) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"CSRStaUpdMode = Manual Mode\n"
			);
	}
	return Status;
}

INT ShowCsrStaInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_CSR_STA_INFO, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show CSRStaInfo=0\n");
	}

	return Status;
}

INT ShowSrRateOffset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;

	if (UniSrCmdShow(pAd, arg, UNI_CMD_SR_CFG_SR_SR_RATE_OFFSET, SR_CMD_GET_DEFAULT_ARG_NUM, FALSE) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show srrateoffset=0\n");
	}

	return Status;
}

INT ShowCsrFixNofiyPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;
	UINT_32 u4Recv = 0;
	UINT_32 staId = 0, apId;
	struct UNI_CMD_CSR_INFO_T rCsrFixPwr = {0};

	u4Recv = sscanf(arg, "%d-%d", &staId, &apId);
	if (u4Recv != 2) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of arguments\n");
		return FALSE;
	}
	rCsrFixPwr.u2Tag = UNI_CMD_CSR_FIX_PWR;
	rCsrFixPwr.u2Length = sizeof(struct UNI_CMD_CSR_INFO_T);

	rCsrFixPwr.u1STAid = staId;
	rCsrFixPwr.u1APid = apId;
	if (UniCmdCsrInfo(pAd, rCsrFixPwr) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show CoSRFixNofiyPwr=staId-apId\n");
	}
	return Status;
}

INT ShowCsrFixRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Status = TRUE;
	UINT_32 u4Recv = 0;
	UINT_32 staId = 0, apId;
	struct UNI_CMD_CSR_INFO_T rCsrFixRate = {0};

	u4Recv = sscanf(arg, "%d-%d", &staId, &apId);
	if (u4Recv != 2) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"wrong number of arguments\n");
		return FALSE;
	}
	rCsrFixRate.u2Tag = UNI_CMD_CSR_FIX_RATE;
	rCsrFixRate.u2Length = sizeof(struct UNI_CMD_CSR_INFO_T);

	rCsrFixRate.u1STAid = staId;
	rCsrFixRate.u1APid = apId;
	if (UniCmdCsrInfo(pAd, rCsrFixRate) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "iwpriv ra0 show cosrinfo=staId-apId\n");
	}
	return Status;
}

#endif/* WIFI_UNIFIED_COMMAND */

VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length)
{
	/* Event ID */
	UINT8 u1EventSubId;

	/* Get Event Category ID */
	u1EventSubId = *Data;

	/**Prevent legitimate but wrong ID **/
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
		 "u1EventSubId = %d\n", u1EventSubId);

	/* Event Handle for different Category ID */
	switch (u1EventSubId) {
	case SR_EVENT_GET_SR_CFG_SR_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_SD_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_MODE:
	case SR_EVENT_GET_SR_CFG_DISRT_ENABLE:
	case SR_EVENT_GET_SR_CFG_DISRT_MIN_RSSI:
	case SR_EVENT_GET_SR_CFG_SR_BF:
	case SR_EVENT_GET_SR_CFG_SR_ATF:
	case SR_EVENT_GET_SR_CFG_TXC_QUEUE:
	case SR_EVENT_GET_SR_CFG_TXC_QID:
	case SR_EVENT_GET_SR_CFG_TXC_PATH:
	case SR_EVENT_GET_SR_CFG_AC_METHOD:
	case SR_EVENT_GET_SR_CFG_SR_PERIOD_THR:
	case SR_EVENT_GET_SR_CFG_QUERY_TXD_METHOD:
	case SR_EVENT_GET_SR_CFG_SR_SD_CG_RATIO:
	case SR_EVENT_GET_SR_CFG_SR_SD_OBSS_RATIO:
	case SR_EVENT_GET_SR_CFG_PROFILE:
	case SR_EVENT_GET_SR_CFG_FNQ_ENABLE:
	case SR_EVENT_GET_SR_CFG_DPD_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_SD_OM_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR:
	case SR_EVENT_GET_SR_CFG_SR_DABS_MODE:
		PrintSrEvent((struct SR_EVENT_T *)Data);
		break;
	case SR_EVENT_GET_SR_SRG_BITMAP:
		PrintSrEventSrSrgBitmap((struct SR_EVENT_SR_SRG_BITMAP_T *)Data);
		break;
	case SR_EVENT_GET_SR_MESH_SRG_BITMAP:
		PrintSrEventSrSrgBitmap((struct SR_EVENT_SR_SRG_BITMAP_T *)Data);
		break;
	case SR_EVENT_GET_SR_CNT_ALL:
		PrintSrEventSrCnt((struct SR_EVENT_SR_CNT_T *)Data);
		break;
	case SR_EVENT_GET_SR_SD_ALL:
		PrintSrEventSrSd((struct SR_EVENT_SR_SD_T *)Data);
		break;
	case SR_EVENT_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO:
		PrintSrEventSrGloVarSingleDropTa((struct SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *)Data);
		break;
	case SR_EVENT_GET_SR_CAP_ALL_INFO:
		PrintSrEventSrCap(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_PARA_ALL_INFO:
		PrintSrEventSrPara((struct SR_EVENT_SR_PARA_T *)Data);
		break;
	case SR_EVENT_GET_SR_COND_ALL_INFO:
		PrintSrEventSrCond(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_ALL_INFO:
		PrintSrEventSrRcpiTbl((struct SR_EVENT_SR_RCPITBL_T *)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_OFST_ALL_INFO:
		PrintSrEventSrRcpiTblOfst(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_Q_CTRL_ALL_INFO:
		PrintSrEventSrQCtrl(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_IBPD_ALL_INFO:
		PrintSrEventSrIBPD((struct SR_EVENT_SR_IBPD_T *)Data);
		break;
	case SR_EVENT_GET_SR_NRT_ALL_INFO:
		PrintSrEventSrNRT(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_NRT_CTRL_ALL_INFO:
		PrintSrEventSrNRTCtrl(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_IND_ALL_INFO:
		PrintSrEventSrInd((struct SR_EVENT_SR_IND_T *)Data);
		break;
	case SR_EVENT_GET_SR_FNQ_CTRL_ALL_INFO:
		PrintSrEventSrFNQCtrl((struct SR_EVENT_SR_FNQ_CTRL_T *)Data);
		break;
	case SR_EVENT_GET_SR_FRM_FILT_ALL_INFO:
		PrintSrEventSrFrmFilt((struct SR_EVENT_SR_FRM_FILT_T *)Data);
		break;
	case SR_EVENT_GET_SR_INTERPS_CTRL_ALL_INFO:
		PrintSrEventSrInterPsCtrl((struct SR_EVENT_SR_INTERPS_CTRL_T *)Data);
		break;
	case SR_EVENT_GET_SR_INTERPS_DBG_ALL_INFO:
		PrintSrEventSrInterPsDbg((struct SR_EVENT_SR_INTERPS_DBG_T *)Data);
		break;
	case SR_EVENT_GET_SR_SIGA_FLAG_ALL_INFO:
		PrintSrEventSrSiga(pAd, (struct SR_EVENT_SR_SIGA_T *)Data);
		break;
	case SR_EVENT_GET_SR_SIGA_AUTO_FLAG_ALL_INFO:
		PrintSrEventSrSigaAuto(pAd, (struct _SR_EVENT_SR_SIGA_AUTO_T *)Data);
		break;
	case SR_EVENT_GET_REMOTE_FH_RSSI:
	case SR_EVENT_GET_REMOTE_BH_INFO:
	case SR_EVENT_GET_MAP_TOPO:
		PrintSrEventMeshTopo(pAd, (struct _SR_EVENT_MESH_TOPOLOGY_T *)Data);
		break;
	case SR_EVENT_GET_MAP_TRAFFIC_STATUS:
		PrintSrEventSrUlStatus(pAd, (struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data);
		break;
	case SR_EVENT_SEND_MESH_UPLINK_TRAFFIC:
		ExtEventMeshUplinkTraffic(pAd, (struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data);
		break;
	case SR_EVENT_GET_MESH_PHASE:
		PrintSrEventSrMeshPhase(pAd, (struct _SR_EVENT_SR_MESH_PHASE_T *)Data);
		break;
	case SR_EVENT_GET_SR_CFG_MESH_REMOTE_STA_MODE:
		PrintSrEventSrRemoteAPStaAllHe(pAd, (struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *)Data);
		break;
	default:
		break;
	}
}

/* for check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum)
{
	if (u4ArgVal != 0 && u4ArgVal != 1) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "ArgNum[%d] = %d is invalid Value! (ArgVal !=0 && ArgVal !=1)\n",
			  u1ArgNum, u4ArgVal);
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto)
{
	if (u4ArgVal < u4Valfrom || u4ArgVal > u4Valto) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			 "ArgNum[%d] = %d is invalid Value! (ArgVal < %d or ArgVal > %d)\n",
			  u1ArgNum, u4ArgVal, u4Valfrom, u4Valto);
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

VOID PrintSrCmd(IN struct SR_CMD_T *prSrCmd)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1CmdSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %d\n"
		  "u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
		  prSrCmd->u1CmdSubId, prSrCmd->u1ArgNum, prSrCmd->u1DbdcIdx,
		  prSrCmd->u1Status, prSrCmd->u1DropTaIdx, prSrCmd->u1StaIdx, prSrCmd->u4Value);
}

VOID PrintSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCap)
{
	if (IS_SR_V1(pAd)) {
		struct WH_SR_CAP_T_SR_V1 *prSrCap = (struct WH_SR_CAP_T_SR_V1 *)_prSrCap;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "fgSrEn                 = %x, fgSrgEn              = %x, fgNonSrgEn              = %x\n"
		  "fgSingleMdpuRtsctsEn   = %x, fgHdrDurEn           = %x, fgTxopDurEn             = %x\n"
		  "fgNonSrgInterPpduPresv = %x, fgSrgInterPpduPresv  = %x, fgSrRemTimeEn           = %x\n"
		  "fgProtInSrWinDis       = %x, fgTxCmdDlRateSelEn   = %x, fgAmpduTxCntEn          = %x\n",
		  prSrCap->fgSrEn, prSrCap->fgSrgEn, prSrCap->fgNonSrgEn,
		  prSrCap->fgSingleMdpuRtsctsEn, prSrCap->fgHdrDurEn, prSrCap->fgTxopDurEn,
		  prSrCap->fgNonSrgInterPpduPresv, prSrCap->fgSrgInterPpduPresv,
		  prSrCap->fgSrRemTimeEn, prSrCap->fgProtInSrWinDis, prSrCap->fgTxCmdDlRateSelEn,
		  prSrCap->fgAmpduTxCntEn);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_CAP_T_SR_V2 *prSrCap = (struct WH_SR_CAP_T_SR_V2 *)_prSrCap;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrEn = %x, fgSrgEn = %x, fgNonSrgEn = %x, fgSingleMdpuRtsctsEn = %x\n",
			prSrCap->fgSrEn, prSrCap->fgSrgEn, prSrCap->fgNonSrgEn,
			prSrCap->fgSingleMdpuRtsctsEn);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgHdrDurEn = %x, fgTxopDurEn = %x, fgNonSrgInterPpduPresv = %x, fgSrgInterPpduPresv = %x\n",
			prSrCap->fgHdrDurEn, prSrCap->fgTxopDurEn,
			prSrCap->fgNonSrgInterPpduPresv, prSrCap->fgSrgInterPpduPresv);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSMpduNoTrigEn = %x, fgSrgBssidOrder = %x, fgCtsAfterRts = %x, fgSrpOldRxvEn = %x\n",
			prSrCap->fgSMpduNoTrigEn, prSrCap->fgSrgBssidOrder,
			prSrCap->fgCtsAfterRts, prSrCap->fgSrpOldRxvEn);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrpNewRxvEn = %x, fgSrpDataOnlyEn = %x, fgFixedRateSrREn = %x, fgWtblSrREn = %x\n",
			prSrCap->fgSrpNewRxvEn, prSrCap->fgSrpDataOnlyEn,
			prSrCap->fgFixedRateSrREn, prSrCap->fgWtblSrREn);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrRemTimeEn = %x, fgProtInSrWinDis = %x, fgTxCmdDlRateSelEn = %x, fgAmpduTxCntEn = %x\n",
			prSrCap->fgSrRemTimeEn, prSrCap->fgProtInSrWinDis,
			prSrCap->fgTxCmdDlRateSelEn, prSrCap->fgAmpduTxCntEn);
	}
}

VOID PrintSrPara(IN struct WH_SR_PARA_T *prSrPara)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u1NonSrgPdThr = %x, u1SrgPdThr = %x, u1PeriodOfst = %x\n",
		prSrPara->u1NonSrgPdThr, prSrPara->u1SrgPdThr,
		prSrPara->u1PeriodOfst);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u1RcpiSourceSel = %x, u2ObssPdMin = %x, u2ObssPdMinSrg = %x\n",
		prSrPara->u1RcpiSourceSel, prSrPara->u2ObssPdMin,
		prSrPara->u2ObssPdMinSrg);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"eRespTxPwrMode = %x, eTxPwrRestricMode = %x, u1ObssTxPwrRef = %x\n",
		prSrPara->eRespTxPwrMode, prSrPara->eTxPwrRestricMode,
		prSrPara->u1ObssTxPwrRef);
}

VOID PrintSrInd(IN struct WH_SR_IND_T *prSrInd)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u2NonSrgVldCnt = %x, u2SrgVldCnt = %x\n"
		"u2IntraBssPpduCnt = %x, u2InterBssPpduCnt = %x\n",
		prSrInd->u2NonSrgVldCnt, prSrInd->u2SrgVldCnt,
		prSrInd->u2IntraBssPpduCnt, prSrInd->u2InterBssPpduCnt);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u2NonSrgPpduVldCnt = %x, u2SrgPpduVldCnt = %x\n"
		"u4SrAmpduMpduCnt = %x, u4SrAmpduMpduAckedCnt = %x\n",
		prSrInd->u2NonSrgPpduVldCnt, prSrInd->u2SrgPpduVldCnt,
		prSrInd->u4SrAmpduMpduCnt, prSrInd->u4SrAmpduMpduAckedCnt);
}

VOID PrintSrGloVarSingleDropTa(IN struct SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "SR Info - u1CurSrDropTaIdx = %d, u2SrTtlTxCntThr=%d\n",
		  prSrGlobalVarSingleDropTa->u1CurSrDropTaIdx,
		  prSrGlobalVarSingleDropTa->u2SrTtlTxCntThr);

	PrintSrDropTaInfo(&(prSrGlobalVarSingleDropTa->rSrDropTaInfo), u1DropTaIdx, u1StaIdx);

}

VOID PrintSrDropTaInfo(IN struct SR_DROP_TA_INFO_T *prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "    DropTa %2d - Address : %x\n",
		  u1DropTaIdx, prSrDropTaInfo->u4Address2);

	if (u1StaIdx == SR_STA_NUM) {
		for (u1StaIdx = 0; u1StaIdx < SR_STA_NUM; u1StaIdx++) {
			PrintSrStaInfo(&(prSrDropTaInfo->rSrStaInfo[u1StaIdx]), u1StaIdx);
		}
	} else {
		PrintSrStaInfo(&(prSrDropTaInfo->rSrStaInfo[u1StaIdx]), u1StaIdx);
	}

}

VOID PrintSrStaInfo(IN struct SR_STA_INFO_T *prSrStaInfo, IN UINT_8 u1StaIdx)
{
	UINT_32 per = prSrStaInfo->u2SrTtlTxCnt == 0 ? 0 : 1000 * (prSrStaInfo->u2SrTtlTxCnt - prSrStaInfo->u2SrSucCnt) / prSrStaInfo->u2SrTtlTxCnt;
	CHAR *mode[2] = {"AUTO", "FIXED"};
	CHAR *state[5] = {"Invailid", "Not Stable", "Good", "Bad", "Timeout"};
	CHAR *rastate[2] = {"Stable", "Active"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "        STA %2d\n"
		  "        u2WlanId       = %d, u1Mode       = %s,  u1State = %s\n",
		  u1StaIdx, prSrStaInfo->u2WlanId, mode[prSrStaInfo->u1Mode], state[prSrStaInfo->u1State]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "        u1SrRateOffset = %d, u1SrRaTryCnt = %x, u1SrRaRound = %x, u1SrRaState = %s\n",
		  prSrStaInfo->u1SrRateOffset, prSrStaInfo->u1SrRaTryCnt, prSrStaInfo->u1SrRaRound, rastate[prSrStaInfo->u1SrRaState]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "        u2SrSucCnt  = %x, u2SrTtlTxCnt = %x, PER = %d.%1d%%\n",
		  prSrStaInfo->u2SrSucCnt, prSrStaInfo->u2SrTtlTxCnt, per / 10, per % 10);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"        u4Score = %x, u2BadQuota = %d, u1BadLevel = %d, u1SrRate = %x\n",
		  prSrStaInfo->u4Score, prSrStaInfo->u2BadQuota, prSrStaInfo->u1BadLevel, prSrStaInfo->u1SrRate);

}

VOID PrintSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCond)
{
	if (IS_SR_V1(pAd)) {
		struct SR_COND_T_SR_V1 *prSrCond = (struct SR_COND_T_SR_V1 *)_prSrCond;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			  "fgSrRcpiCckRateEn = %x, fgSrMacRcpiRateEn = %x, fgSrRxvRcpiRateEn = %x\n"
			  "fgSrRcpiHeRateEn  = %x, fgSrRcpiVhtRateEn = %x, fgSrRcpiHtRateEn  = %x\n"
			  "fgSrRcpiLgRateEn  = %x, fgSrRxvEntry 	 = %x, fgSrPeriodLimitEn = %x\n"
			  "u1SrPeriodLimit	 = %x\n",
			  prSrCond->fgSrRcpiCckRateEn, prSrCond->fgSrMacRcpiRateEn, prSrCond->fgSrRxvRcpiRateEn,
			  prSrCond->fgSrRcpiHeRateEn, prSrCond->fgSrRcpiVhtRateEn, prSrCond->fgSrRcpiHtRateEn,
			  prSrCond->fgSrRcpiLgRateEn, prSrCond->fgSrRxvEntry, prSrCond->fgSrPeriodLimitEn,
			  prSrCond->u1SrPeriodLimit);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_COND_T_SR_V2 *prSrCond = (struct WH_SR_COND_T_SR_V2 *)_prSrCond;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrRcpiSel = %x, fgSrRcpiCckRateEn = %x\n"
			"fgSrMacRcpiRateEn = %x, fgSrRxvRcpiRateEn = %x\n",
			prSrCond->fgSrRcpiSel, prSrCond->fgSrRcpiCckRateEn,
			prSrCond->fgSrMacRcpiRateEn, prSrCond->fgSrRxvRcpiRateEn);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrRcpiHeRateEn = %x, fgSrRcpiVhtRateEn = %x\n"
			"fgSrRcpiHtRateEn = %x, fgSrRcpiLgRateEn = %x\n",
			prSrCond->fgSrRcpiHeRateEn, prSrCond->fgSrRcpiVhtRateEn,
			prSrCond->fgSrRcpiHtRateEn, prSrCond->fgSrRcpiLgRateEn);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrRxvEntry = %x, fgSrPeriodLimitEn = %x, u1SrPeriodLimit = %x\n",
			prSrCond->fgSrRxvEntry, prSrCond->fgSrPeriodLimitEn,
			prSrCond->u1SrPeriodLimit);
	}
}

VOID PrintSrRcpiTbl(IN struct WH_SR_RCPITBL_T *prSrRcpiTbl)
{
	UINT_8 u1Index;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "\n");
	for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "u1RcpiTblMcs[%d] = %x (%d dBm)\n", u1Index, prSrRcpiTbl->u1RcpiTblMcs[u1Index], SRDbmConv(prSrRcpiTbl->u1RcpiTblMcs[u1Index]));
}

VOID PrintSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		struct SR_RCPITBL_OFST_T_SR_V1 *prSrRcpiTblOfst = (struct SR_RCPITBL_OFST_T_SR_V1 *)_prSrRcpiTblOfst;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
			  "u2RxBwRcpiOfst = %x, u2StbcRcpiOfst = %x, u2NumAntRcpiOfst = %x\n"
			  "u2LdpcRcpiOfst = %x, u2DcmRcpiOfst  = %x, u2MacRcpiOfst	  = %x\n"
			  "u2SigRcpiOfst  = %x\n",
			  prSrRcpiTblOfst->u2RxBwRcpiOfst, prSrRcpiTblOfst->u2StbcRcpiOfst, prSrRcpiTblOfst->u2NumAntRcpiOfst,
			  prSrRcpiTblOfst->u2LdpcRcpiOfst, prSrRcpiTblOfst->u2DcmRcpiOfst, prSrRcpiTblOfst->u2MacRcpiOfst,
			  prSrRcpiTblOfst->u2SigRcpiOfst);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_RCPITBL_OFST_T_SR_V2 *prSrRcpiTblOfst = (struct WH_SR_RCPITBL_OFST_T_SR_V2 *)_prSrRcpiTblOfst;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u2RxBwRcpiOfst = %x, u2StbcRcpiOfst = %x, u4NumAntRcpiOfst = %x\n",
			prSrRcpiTblOfst->u2RxBwRcpiOfst, prSrRcpiTblOfst->u2StbcRcpiOfst,
			prSrRcpiTblOfst->u4NumAntRcpiOfst);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u2LdpcRcpiOfst = %x, u2DcmRcpiOfst = %x, u2MacRcpiOfst = %x\n",
			prSrRcpiTblOfst->u2LdpcRcpiOfst, prSrRcpiTblOfst->u2DcmRcpiOfst,
			prSrRcpiTblOfst->u2MacRcpiOfst);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u2SigRcpiOfst = %x, u2BfRcpiOfst = %x\n",
			prSrRcpiTblOfst->u2SigRcpiOfst, prSrRcpiTblOfst->u2BfRcpiOfst);
	}
}

VOID PrintSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct WH_SR_QUEUE_CTRL_T_SR_V1 *prSrQCtrl = (struct WH_SR_QUEUE_CTRL_T_SR_V1 *)_prSrQCtrl;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "fgSrRxRptEn = %x, fgSrCw = %x, fgSrSuspend = %x, u4SrBackOffMask = %x\n",
		  prSrQCtrl->fgSrRxRptEn, prSrQCtrl->fgSrCw, prSrQCtrl->fgSrSuspend, prSrQCtrl->u4SrBackOffMask);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_QUEUE_CTRL_T_SR_V2 *prSrQCtrl = (struct WH_SR_QUEUE_CTRL_T_SR_V2 *)_prSrQCtrl;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			  "fgSrRxRptEn = %x, fgSrCw = %x, fgSrSuspend = %x, fgSrDisSwAifsDis = %x\n"
			  "u4SrBackOffMask = %x, u4SrBackOffEnable = %x\n",
			  prSrQCtrl->fgSrRxRptEn, prSrQCtrl->fgSrCw, prSrQCtrl->fgSrSuspend,
			  prSrQCtrl->fgSrDisSwAifsDis, prSrQCtrl->u4SrBackOffMask,
			  prSrQCtrl->u4SrBackOffEnable);
	}
}

VOID PrintSrIBPD(IN struct WH_SR_IBPD_T *prSrIBPD)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1InterBssByHdrBssid = %x, u1InterBssByMu = %x, u1InterBssByPbssColor = %x\n"
		  "u1InterBssByPaid = %x, u1InterBssByBssColor = %x\n",
		  prSrIBPD->u1InterBssByHdrBssid, prSrIBPD->u1InterBssByMu,
		  prSrIBPD->u1InterBssByPbssColor, prSrIBPD->u1InterBssByPaid,
		  prSrIBPD->u1InterBssByBssColor);
}

VOID PrintSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRT)
{
	if (IS_SR_V1(pAd)) {
		struct SR_NRT_T_SR_V1 *prSrNRT = (struct SR_NRT_T_SR_V1 *)_prSrNRT;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "u1TableIdx = %x, u4NRTValue = %x\n",
		  prSrNRT->u1TableIdx, prSrNRT->u4NRTValue);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_NRT_T_SR_V2 *prSrNRT = (struct WH_SR_NRT_T_SR_V2 *)_prSrNRT;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			 "u1TableIdx = %d, u1RaTaSel = %x, fgSwProtect = %x, u4NRTValue = %x\n",
			  prSrNRT->u1TableIdx, prSrNRT->u1RaTaSel, prSrNRT->fgSwProtect, prSrNRT->u4NRTValue);
	}
}

VOID PrintSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct SR_NRT_CTRL_T_SR_V1 *prSrNRTCtrl = (struct SR_NRT_CTRL_T_SR_V1 *)_prSrNRTCtrl;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			  "fgSrtEn       = %x, fgSrtSrpEn     = %x, fgSrtAddrOrderEn = %x\n"
			  "u2SrtInRcpiTh = %x, u2SrtOutRcpiTh = %x, u2SrtUsedCntTh   = %x\n",
			  prSrNRTCtrl->fgSrtEn, prSrNRTCtrl->fgSrtSrpEn, prSrNRTCtrl->fgSrtAddrOrderEn,
			  prSrNRTCtrl->u2SrtInRcpiTh, prSrNRTCtrl->u2SrtOutRcpiTh, prSrNRTCtrl->u2SrtUsedCntTh);
	}

	if (IS_SR_V2(pAd)) {
		struct WH_SR_NRT_CTRL_T_SR_V2 *prSrNRTCtrl = (struct WH_SR_NRT_CTRL_T_SR_V2 *)_prSrNRTCtrl;

		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"fgSrtEn = %x, fgSrtSrpEn = %x, fgSrtAddrOrderEn = %x, fgSrtByPassCtsAck = %x\n",
			prSrNRTCtrl->fgSrtEn, prSrNRTCtrl->fgSrtSrpEn, prSrNRTCtrl->fgSrtAddrOrderEn,
			prSrNRTCtrl->fgSrtByPassCtsAck);
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u2SrtInRcpiTh = %x, u2SrtOutRcpiTh = %x, u2SrtUsedCntTh   = %x\n",
			prSrNRTCtrl->u2SrtInRcpiTh, prSrNRTCtrl->u2SrtOutRcpiTh,
			prSrNRTCtrl->u2SrtUsedCntTh);
	}
}

VOID PrintSrFNQCtrl(IN struct WH_SR_FNQ_CTRL_T *prSrFNQCtrl)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u2SrpCondDis = %x, u1PeriodOfst  = %x, fgHdrDurEn      = %x\n",
		prSrFNQCtrl->u2SrpCondDis, prSrFNQCtrl->u1PeriodOfst,
		prSrFNQCtrl->fgHdrDurEn);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"fgTxopDurEn  = %x, fgSrpCfendRst = %x, fgSrpNavToutRst = %x\n",
		prSrFNQCtrl->fgTxopDurEn, prSrFNQCtrl->fgSrpCfendRst,
		prSrFNQCtrl->fgSrpNavToutRst);
}

VOID PrintSrFrmFilt(IN UINT_32 *pu4SrFrmFilt)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "u4SrFrmFilt = %x\n", *pu4SrFrmFilt);
}

VOID PrintSrInterPsCtrl(IN struct WH_SR_INTERPS_CTRL_T *prSrInterPsCtrl)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1CondDis = %x, u1DurAdj   = %x, u1DurLmt    = %x\n"
		  "u1EntryEn = %x, fgDurLmtEn = %x, fgInterpsEn = %x\n",
		  prSrInterPsCtrl->u1CondDis, prSrInterPsCtrl->u1DurAdj, prSrInterPsCtrl->u1DurLmt,
		  prSrInterPsCtrl->u1EntryEn, prSrInterPsCtrl->fgDurLmtEn, prSrInterPsCtrl->fgInterpsEn);
}

VOID PrintSrInterPsDbg(IN struct WH_SR_INTERPS_DBG_T *prSrInterPsDbg)
{
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1Entry0Cnt = %x, u1Entry1Cnt   = %x, u1Entry2Cnt    = %x\n"
		  "u1EntryLat  = %x\n",
		  prSrInterPsDbg->u1Entry0Cnt, prSrInterPsDbg->u1Entry1Cnt, prSrInterPsDbg->u1Entry2Cnt,
		  prSrInterPsDbg->u1EntryLat);
}

VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN struct SR_SRG_BITMAP_T *prSrSrgBitmap)
{
	UINT_8 u1BitmapIdx = 0, u1ColorEn = 0, u1pBssidEn = 0;
	CHAR *enable[2] = {" ", "V"};
	/*UINT_32 u4Color = 0, u4pBssid = 0;*/

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Color - 31_0:%x, 63_32:%x pBssid - 31_0:%x, 63_32:%x\n",
		(prSrSrgBitmap + u1DbdcIdx)->u4Color_31_0, (prSrSrgBitmap + u1DbdcIdx)->u4Color_63_32,
		(prSrSrgBitmap + u1DbdcIdx)->u4pBssid_31_0, (prSrSrgBitmap + u1DbdcIdx)->u4pBssid_63_32);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "BIT  Color  pBssid\n");


	for (u1BitmapIdx = 0; u1BitmapIdx < 64; u1BitmapIdx++) {
		if (u1BitmapIdx < 32) {
			u1ColorEn = ((prSrSrgBitmap + u1DbdcIdx)->u4Color_31_0 & BIT(u1BitmapIdx)) >> u1BitmapIdx;
			u1pBssidEn = ((prSrSrgBitmap + u1DbdcIdx)->u4pBssid_31_0 & BIT(u1BitmapIdx)) >> u1BitmapIdx;
		} else {
			u1ColorEn = ((prSrSrgBitmap + u1DbdcIdx)->u4Color_63_32 & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
			u1pBssidEn = ((prSrSrgBitmap + u1DbdcIdx)->u4pBssid_63_32 & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
		}

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"%2d     %s      %s   \n",
		u1BitmapIdx, enable[u1ColorEn], enable[u1pBssidEn]);
	}
}

VOID PrintMeshDlStaTh(IN PRTMP_ADAPTER pAD, IN UINT_8 u1DbdcIdx, IN struct SR_MESH_SR_DL_STA_THRESHOLD *prSrMeshDlStaTh, IN BOOLEAN fgread)
{
	RTMP_STRING *flaginfo = "Reserved";

	if (fgread) {
		MTWF_DBG(pAD, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1BSSID = %2d, irssi = %2d (%s)\n",
			  prSrMeshDlStaTh->u1BSSID,
			 prSrMeshDlStaTh->irssi, flaginfo);

	}
}

VOID PrintSrSiga(IN PRTMP_ADAPTER pAD, IN UINT_8 u1DbdcIdx, IN struct SR_SIGA_FLAG_T *prSrSigaflag, IN BOOLEAN fgread)
{
	INT i1devidx;
	UINT8 u1Bssid;
	POS_COOKIE pObj = (POS_COOKIE) pAD->OS_Cookie;
	struct wifi_dev *counterdev;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAD, pObj->ioctl_if, pObj->ioctl_if_type);
	RTMP_STRING *flaginfo = "Reserved";

	if (fgread) {
		for (i1devidx = 0; i1devidx < WDEV_NUM_MAX; i1devidx++) {
			if (pAD->wdev_list[i1devidx] == NULL)
				break;
			counterdev = pAD->wdev_list[i1devidx];

			u1Bssid = counterdev->DevInfo.OwnMacIdx > 3 ? counterdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : counterdev->DevInfo.OwnMacIdx;
			flaginfo = "Reserved";
			if ((u1Bssid < SR_BSSID_NUM) && counterdev->if_up_down_state) {
				if (u1DbdcIdx == HcGetBandByWdev(counterdev)) {
					for (PENUM_WH_SR_SIGA_FLAG_T = ENUM_WH_SR_SIGA_FLAG_T; PENUM_WH_SR_SIGA_FLAG_T->name ; PENUM_WH_SR_SIGA_FLAG_T++) {
						if (PENUM_WH_SR_SIGA_FLAG_T->u1srflag == prSrSigaflag->u1SigaFlag[u1Bssid]) {
							flaginfo = PENUM_WH_SR_SIGA_FLAG_T->name;
							break;
						}
					}
					MTWF_DBG(pAD, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
					"Interface = %s  omac_indx = %2d, Flag = %2d (%s)\n",
					RtmpOsGetNetDevName(counterdev->if_dev),
					counterdev->DevInfo.OwnMacIdx,
					prSrSigaflag->u1SigaFlag[u1Bssid], flaginfo);
				}
			}
		}
	} else {
		u1Bssid = wdev->DevInfo.OwnMacIdx > 3 ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;

		if (u1Bssid < SR_BSSID_NUM) {
			for (PENUM_WH_SR_SIGA_FLAG_T = ENUM_WH_SR_SIGA_FLAG_T; PENUM_WH_SR_SIGA_FLAG_T->name ; PENUM_WH_SR_SIGA_FLAG_T++) {
				if (PENUM_WH_SR_SIGA_FLAG_T->u1srflag == prSrSigaflag->u1SigaFlag[u1Bssid]) {
					flaginfo = PENUM_WH_SR_SIGA_FLAG_T->name;
					break;
				}
			}
			MTWF_DBG(pAD, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
				"Interface = %s  omac_indx = %2d, Flag = %2d (%s)\n",
				RtmpOsGetNetDevName(wdev->if_dev),
				wdev->DevInfo.OwnMacIdx, prSrSigaflag->u1SigaFlag[u1Bssid], flaginfo);
		}
	}
}

VOID PrintSrMeshTopo(IN UINT_8 u1SubId, IN union _SR_MESH_TOPOLOGY_T *prSrMeshTopo)
{
	UINT_8 cmd_u1SubId = u1SubId;
	UINT_8 event_u1SubId = u1SubId;
	if (cmd_u1SubId == SR_CMD_SET_REMOTE_FH_RSSI || event_u1SubId == SR_EVENT_GET_REMOTE_FH_RSSI)
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: u1CmdSubId:%u i1Rssi:%d FHStat:%u\n",
			__func__, u1SubId, prSrMeshTopo->rRemoteFhParams.i1Rssi,
			prSrMeshTopo->rRemoteFhParams.u1RemoteFhStat);
	else if (cmd_u1SubId == SR_CMD_SET_REMOTE_BH_INFO || event_u1SubId == SR_EVENT_GET_REMOTE_BH_INFO)
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: u1CmdSubId:%u u2RemoteBhWcid:%u u1RemoteBhType:%u\n",
			__func__, u1SubId, prSrMeshTopo->rRemoteBhParams.u2RemoteBhWcid,
			prSrMeshTopo->rRemoteBhParams.u1RemoteBhType);
	else if (cmd_u1SubId == SR_CMD_SET_MAP_TOPO || event_u1SubId == SR_EVENT_GET_MAP_TOPO)
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: u1CmdSubId:%u u1MapDevCount:%u u1MapDevSrSupportMode:%u u1SelfRole:%u\n",
			__func__, u1SubId, prSrMeshTopo->rMapTopoParams.u1MapDevCount,
			prSrMeshTopo->rMapTopoParams.u1MapDevSrSupportMode,
			prSrMeshTopo->rMapTopoParams.u1SelfRole);
	else
		return;
}

VOID PrintSrCnt(IN UINT_8 u1DbdcIdx, IN struct SR_CNT_T *prSrCnt, IN UINT8 u1PpduType)
{
	UINT_8  u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT;
	UINT_8  u1SrEntry = ENUM_SR_ENTRY_NEWRXV;
	CHAR *srrxrptsrc[2] = {"RXRPT", "CMDRPT-TX"};

	UINT_16 au2RxrptTtl[ENUM_SR_RXRPT_SRC_NUM]     = {0};
	UINT_16	au2PeriodSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2PeriodFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2GenTxcSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2GenTxcFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2SrTxSucTtl[ENUM_SR_RXRPT_SRC_NUM]   = {0}, au2SrTxFailTtl[ENUM_SR_RXRPT_SRC_NUM]    = {0};

	if (u1PpduType == ENUM_SR_NONSRG_OBSS_OR_NONHE) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: Band%d Reuse NonSgr:OBSS Packets\n", __func__, u1DbdcIdx);
	} else if (u1PpduType == ENUM_SR_NONSRG_MESH_HE_INTERNAL_NO_SR) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: Band%d Reuse NonSgr:Mesh HE Internal No SR Packets\n", __func__, u1DbdcIdx);
	} else if (u1PpduType == ENUM_SR_NONSRG_MESH_HE) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: Band%d Reuse NonSgr:Mesh HE Packets\n", __func__, u1DbdcIdx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"%s: Arg. is Invaild! 0:OBSS 1:Mesh HE RESTR 2:Mesh HE\n", __func__);
		return;
	}

	for (u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT; u1SrRxrptSrc < ENUM_SR_RXRPT_SRC_NUM; u1SrRxrptSrc++) {

		for (u1SrEntry = ENUM_SR_ENTRY_NEWRXV; u1SrEntry < ENUM_SR_ENTRY_NUM; u1SrEntry++) {
			au2RxrptTtl[u1SrRxrptSrc]     += (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][u1SrEntry][u1PpduType].u2EntryTtl;
			au2PeriodSucTtl[u1SrRxrptSrc] += (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][u1SrEntry][u1PpduType].u2PeriodSuc;
			au2GenTxcSucTtl[u1SrRxrptSrc] += (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][u1SrEntry][u1PpduType].u2GenTxcSuc;
			au2SrTxSucTtl[u1SrRxrptSrc]   += (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][u1SrEntry][u1PpduType].u2SrTxSuc;
		}
		au2SrTxFailTtl[u1SrRxrptSrc] = au2GenTxcSucTtl[u1SrRxrptSrc] - au2SrTxSucTtl[u1SrRxrptSrc];
		au2GenTxcFailTtl[u1SrRxrptSrc] = au2PeriodSucTtl[u1SrRxrptSrc] - au2GenTxcSucTtl[u1SrRxrptSrc];
		au2PeriodFailTtl[u1SrRxrptSrc] = au2RxrptTtl[u1SrRxrptSrc] - au2PeriodSucTtl[u1SrRxrptSrc];

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += (prSrCnt + u1DbdcIdx)->u2EntryNoSrTtl;
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += (prSrCnt + u1DbdcIdx)->u2EntryFailTtl;
		}

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"-------------------------------------------------------------------\n");
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"SR Rxrpt Source : %s\n"
			"Total Rxrpt  = %4x,\n"
			"NewRxv = %4x, OriRxvVht = %4x, OriRxvHe = %4x,\n"
			"NewMac = %4x, OriMac = %4x,\n",
			srrxrptsrc[u1SrRxrptSrc],
			au2RxrptTtl[u1SrRxrptSrc],
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2EntryTtl,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2EntryTtl,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2EntryTtl,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2EntryTtl,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2EntryTtl);

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
				 "NoSr = %4x, Fail = %4x,\n"
				  "-------------------------------------------------------------------\n",
				  (prSrCnt + u1DbdcIdx)->u2EntryNoSrTtl,
				  (prSrCnt + u1DbdcIdx)->u2EntryFailTtl);
		} else {
			MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
				 "-------------------------------------------------------------------\n");
		}

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"Total Period Succ = %4x, Fail = %4x,\n"
			"NewRxv Succ = %4x, Fail = %4x,\n",
			au2PeriodSucTtl[u1SrRxrptSrc], au2PeriodFailTtl[u1SrRxrptSrc],
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2EntryTtl - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"OriRxvVht Succ = %4x, Fail = %4x,\n"
			"OriRxvHe Succ = %4x, Fail = %4x,\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2EntryTtl - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2EntryTtl - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"NewMac Succ = %4x, Fail = %4x,\n"
			"OriMac Succ = %4x, Fail = %4x,\n"
			"-------------------------------------------------------------------\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2EntryTtl - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2EntryTtl - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc);

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"Total Gen Txc Succ = %4x, Fail = %4x,\n"
			"NewRxv Succ = %4x, Fail = %4x,\n",
			au2GenTxcSucTtl[u1SrRxrptSrc], au2GenTxcFailTtl[u1SrRxrptSrc],
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"OriRxvVht Succ = %4x, Fail = %4x,\n"
			"OriRxvHe Succ = %4x, Fail = %4x,\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"NewMac Succ = %4x, Fail = %4x,\n"
			"OriMac Succ = %4x, Fail = %4x,\n"
			"-------------------------------------------------------------------\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc);


		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"Total SR Tx Succ = %4x, Fail = %4x,\n"
			"NewRxv Succ = %4x, Fail = %4x,\n",
			au2SrTxSucTtl[u1SrRxrptSrc], au2SrTxFailTtl[u1SrRxrptSrc],
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2SrTxSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"OriRxvVht Succ = %4x, Fail = %4x,\n"
			"OriRxvHe Succ = %4x, Fail = %4x,\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2SrTxSuc);
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"NewMac Succ = %4x, Fail = %4x,\n"
			"OriMac Succ = %4x, Fail = %4x,\n"
			"-------------------------------------------------------------------\n",
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2SrTxSuc,
			(prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc - (prSrCnt + u1DbdcIdx)->rSrCntEntry[u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2SrTxSuc);
	}
}

VOID PrintSrSd(IN UINT_8 u1DbdcIdx, IN struct SR_SD_T *prSrSd)
{
	UINT_8 u1McsIdx = 0;
	CHAR *srsdrules[4] = {"1 - NO CONNECTED", "2 - NO CONGESTION", "3 - NO INTERFERENCE", "4 - SR ON"};
	CHAR *srtxstate[9] = {"1 - ON OM INIT", "2 - OFF SRSD FAIL", "3 - ON SRSD PASS", "4 - ON LOW TPUT", "5 - ON OM START", "6 - ON CONT WEAK", "7 - ON CONT NORM", "8 - ON LONG PKT CHK", "9 - OFF ENV DETECTED"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "Band%d\n", u1DbdcIdx);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"-------------------------------------------------------------------\n");

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "Hit Rule = %s\n"
		  "Rxrpt Count = %d\n"
		  "RXV = %d\n"
		  "MAC = %d\n",
		  srsdrules[(prSrSd + u1DbdcIdx)->u1Rule],
		  (prSrSd + u1DbdcIdx)->u2RxrptRxvCnt + (prSrSd + u1DbdcIdx)->u2RxrptMacCnt,
		  (prSrSd + u1DbdcIdx)->u2RxrptRxvCnt,
		  (prSrSd + u1DbdcIdx)->u2RxrptMacCnt);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"-------------------------------------------------------------------\n");

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "Timer Period = %d(us)\n"
		  "HwSR Period Ratio = %d.%1d%%\n"
		  "SwSR Period Ratio = %d.%1d%%\n"
		  "Congestion Ratio  = %d.%1d%%\n",
		  SR_SCENE_DETECTION_TIMER_PERIOD_MS * 1000,
		  (prSrSd + u1DbdcIdx)->u4HWSrPeriodRatio / 10,
		  (prSrSd + u1DbdcIdx)->u4HWSrPeriodRatio % 10,
		  (prSrSd + u1DbdcIdx)->u4SWSrPeriodRatio / 10,
		  (prSrSd + u1DbdcIdx)->u4SWSrPeriodRatio % 10,
		  (prSrSd + u1DbdcIdx)->u4TtlAirTimeRatio / 10,
		  (prSrSd + u1DbdcIdx)->u4TtlAirTimeRatio % 10);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"-------------------------------------------------------------------\n");

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "Total Airtime = %d(us)\n"
		  "ChBusy = %d\n"
		  "SrTx = %d\n"
		  "OBSS = %d\n"
		  "Delta = %d\n"
		  "MyTx = %d\n"
		  "MyRx = %d\n"
		  "Interference Ratio = %d.%1d%%\n",
		  (prSrSd + u1DbdcIdx)->u4TtlAirTime,
		  (prSrSd + u1DbdcIdx)->u4ChannelBusyTime,
		  (prSrSd + u1DbdcIdx)->u4SrTxAirtime,
		  (prSrSd + u1DbdcIdx)->u4OBSSAirtime,
		  (prSrSd + u1DbdcIdx)->u4DeltaTime,
		  (prSrSd + u1DbdcIdx)->u4MyTxAirtime,
		  (prSrSd + u1DbdcIdx)->u4MyRxAirtime,
		  (prSrSd + u1DbdcIdx)->u4OBSSAirTimeRatio / 10,
		  (prSrSd + u1DbdcIdx)->u4OBSSAirTimeRatio % 10);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"-------------------------------------------------------------------\n");

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"OBSS Monitor\n"
		"Tx State = %s\n"
		"Tx Byte Sum = %d\n"
		"TxD PG Cnt = %d\n"
		"Low Tput Cnt = %d\n"
		"Contention Point = %d\n",
		srtxstate[(prSrSd + u1DbdcIdx)->u1SrTxState],
		(prSrSd + u1DbdcIdx)->u4TxByteSum,
		(prSrSd + u1DbdcIdx)->u4TxdPgCnt,
		(prSrSd + u1DbdcIdx)->u1LowTrafficCnt,
		(prSrSd + u1DbdcIdx)->u1ContWeakChkPnt);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Long Pkt Point = %d\n"
		"Pkt Len Short = %d\n"
		"Pkt Len Middle = %d\n"
		"Pkt Len Long = %d\n"
		"SR Tx  Cnt = %d\n"
		"Mode MCS = %d\n",
		(prSrSd + u1DbdcIdx)->u1ObssLongPktPnt,
		(prSrSd + u1DbdcIdx)->u2ObssLongPkt[0],
		(prSrSd + u1DbdcIdx)->u2ObssLongPkt[1],
		(prSrSd + u1DbdcIdx)->u2ObssLongPkt[2],
		(prSrSd + u1DbdcIdx)->u4SrTxCnt,
		(prSrSd + u1DbdcIdx)->u1ModeMcsIdx);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"-------------------------------------------------------------------\n"
		"OBSS Rate Distribution\n");
	for (u1McsIdx = 0; u1McsIdx < SR_RCPITBL_MCS_NUM; u1McsIdx++)
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "MCS[%2d] = %x\n", u1McsIdx, (prSrSd + u1DbdcIdx)->u2RxrptMcs[u1McsIdx]);

}

VOID PrintSrCmdSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V1 *prSrCmdSrCap = (struct SR_CMD_SR_CAP_T_SR_V1 *)_prSrCmdSrCap;

		PrintSrCmd(&(prSrCmdSrCap->rSrCmd));
		PrintSrCap(pAd, &(prSrCmdSrCap->rSrCap));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_CAP_T_SR_V2 *prSrCmdSrCap = (struct SR_CMD_SR_CAP_T_SR_V2 *)_prSrCmdSrCap;

		PrintSrCmd(&(prSrCmdSrCap->rSrCmd));
		PrintSrCap(pAd, &(prSrCmdSrCap->rSrCap));
	}

}

VOID PrintSrCmdSrPara(IN struct SR_CMD_SR_PARA_T *prSrCmdSrPara)
{
	PrintSrCmd(&(prSrCmdSrPara->rSrCmd));
	PrintSrPara(&(prSrCmdSrPara->rSrPara));
}

VOID PrintSrCmdSrGloVarSingleDropTa(IN struct SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
				    prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DropTaIdx,
				    IN UINT_8 u1StaIdx)
{
	PrintSrCmd(&(prSrCmdSrGlobalVarSingleDropTa->rSrCmd));
	PrintSrGloVarSingleDropTa(&(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa),
				  u1DropTaIdx, u1StaIdx);
}

VOID PrintSrCmdSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_COND_T_SR_V1 *prSrCmdSrCond = (struct SR_CMD_SR_COND_T_SR_V1 *)_prSrCmdSrCond;

		PrintSrCmd(&(prSrCmdSrCond->rSrCmd));
		PrintSrCond(pAd, &(prSrCmdSrCond->rSrCond));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_COND_T_SR_V2 *prSrCmdSrCond = (struct SR_CMD_SR_COND_T_SR_V2 *)_prSrCmdSrCond;

		PrintSrCmd(&(prSrCmdSrCond->rSrCmd));
		PrintSrCond(pAd, &(prSrCmdSrCond->rSrCond));
	}
}

VOID PrintSrCmdSrRcpiTbl(IN struct SR_CMD_SR_RCPITBL_T *prSrCmdSrRcpiTbl)
{
	PrintSrCmd(&(prSrCmdSrRcpiTbl->rSrCmd));
	PrintSrRcpiTbl(&(prSrCmdSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrCmdSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 *prSrCmdSrRcpiTblOfst = (struct SR_CMD_SR_RCPITBL_OFST_T_SR_V1 *)_prSrCmdSrRcpiTblOfst;

		PrintSrCmd(&(prSrCmdSrRcpiTblOfst->rSrCmd));
		PrintSrRcpiTblOfst(pAd, &(prSrCmdSrRcpiTblOfst->rSrRcpiTblOfst));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 *prSrCmdSrRcpiTblOfst = (struct SR_CMD_SR_RCPITBL_OFST_T_SR_V2 *)_prSrCmdSrRcpiTblOfst;

		PrintSrCmd(&(prSrCmdSrRcpiTblOfst->rSrCmd));
		PrintSrRcpiTblOfst(pAd, &(prSrCmdSrRcpiTblOfst->rSrRcpiTblOfst));
	}
}

VOID PrintSrCmdSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_Q_CTRL_T_SR_V1 *prSrCmdSrQCtrl = (struct SR_CMD_SR_Q_CTRL_T_SR_V1 *)_prSrCmdSrQCtrl;

		PrintSrCmd(&(prSrCmdSrQCtrl->rSrCmd));
		PrintSrQCtrl(pAd, &(prSrCmdSrQCtrl->rSrQCtrl));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_Q_CTRL_T_SR_V2 *prSrCmdSrQCtrl = (struct SR_CMD_SR_Q_CTRL_T_SR_V2 *)_prSrCmdSrQCtrl;

		PrintSrCmd(&(prSrCmdSrQCtrl->rSrCmd));
		PrintSrQCtrl(pAd, &(prSrCmdSrQCtrl->rSrQCtrl));
	}
}

VOID PrintSrCmdSrIBPD(IN struct SR_CMD_SR_IBPD_T *prSrCmdSrIBPD)
{
	PrintSrCmd(&(prSrCmdSrIBPD->rSrCmd));
	PrintSrIBPD(&(prSrCmdSrIBPD->rSrIBPD));
}

VOID PrintSrCmdSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_T_SR_V1 *prSrCmdSrNRT = (struct SR_CMD_SR_NRT_T_SR_V1 *)_prSrCmdSrNRT;

		PrintSrCmd(&(prSrCmdSrNRT->rSrCmd));
		PrintSrNRT(pAd, &(prSrCmdSrNRT->rSrNRT));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_NRT_T_SR_V2 *prSrCmdSrNRT = (struct SR_CMD_SR_NRT_T_SR_V2 *)_prSrCmdSrNRT;

		PrintSrCmd(&(prSrCmdSrNRT->rSrCmd));
		PrintSrNRT(pAd, &(prSrCmdSrNRT->rSrNRT));
	}
}

VOID PrintSrCmdSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct SR_CMD_SR_NRT_CTRL_T_SR_V1 *prSrCmdSrNRTCtrl = (struct SR_CMD_SR_NRT_CTRL_T_SR_V1 *)_prSrCmdSrNRTCtrl;

		PrintSrCmd(&(prSrCmdSrNRTCtrl->rSrCmd));
		PrintSrNRTCtrl(pAd, &(prSrCmdSrNRTCtrl->rSrNRTCtrl));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_CMD_SR_NRT_CTRL_T_SR_V2 *prSrCmdSrNRTCtrl = (struct SR_CMD_SR_NRT_CTRL_T_SR_V2 *)_prSrCmdSrNRTCtrl;

		PrintSrCmd(&(prSrCmdSrNRTCtrl->rSrCmd));
		PrintSrNRTCtrl(pAd, &(prSrCmdSrNRTCtrl->rSrNRTCtrl));
	}
}

VOID PrintSrCmdSrFNQCtrl(IN struct SR_CMD_SR_FNQ_CTRL_T *prSrCmdSrFNQCtrl)
{
	PrintSrCmd(&(prSrCmdSrFNQCtrl->rSrCmd));
	PrintSrFNQCtrl(&(prSrCmdSrFNQCtrl->rSrFNQCtrl));
}

VOID PrintSrCmdSrFrmFilt(IN struct SR_CMD_SR_FRM_FILT_T *prSrCmdSrFrmFilt)
{
	PrintSrCmd(&(prSrCmdSrFrmFilt->rSrCmd));
	PrintSrFrmFilt(&(prSrCmdSrFrmFilt->u4SrFrmFilt));
}

VOID PrintSrCmdSrInterPsCtrl(IN struct SR_CMD_SR_INTERPS_CTRL_T *prSrCmdSrInterPsCtrl)
{
	PrintSrCmd(&(prSrCmdSrInterPsCtrl->rSrCmd));
	PrintSrInterPsCtrl(&(prSrCmdSrInterPsCtrl->rSrInterPsCtrl));
}

VOID PrintSrCmdSrSrgBitmap(IN struct SR_CMD_SR_SRG_BITMAP_T *prSrCmdSrSrgBitmap)
{
	PrintSrCmd(&(prSrCmdSrSrgBitmap->rSrCmd));
	PrintSrSrgBitmap(prSrCmdSrSrgBitmap->rSrCmd.u1DbdcIdx, prSrCmdSrSrgBitmap->rSrSrgBitmap);
}

VOID PrintSrCmdSrSiga(IN PRTMP_ADAPTER pAD, IN struct SR_CMD_SR_SIGA_FLAG_T *prSrCmdSrSigaFlag)
{
	PrintSrCmd(&(prSrCmdSrSigaFlag->rSrCmd));
	PrintSrSiga(pAD, prSrCmdSrSigaFlag->rSrCmd.u1DbdcIdx, &(prSrCmdSrSigaFlag->rSrSigaFlag), FALSE);
}

VOID PrintSrCmdSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag)
{
	PrintSrCmd(&(prSrCmdSrSigaAutoFlag->rSrCmd));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"SrSigaAutoFlag = %d\n",
		prSrCmdSrSigaAutoFlag->rSrSigaAutoFlag.u1SrSigaAutoFlag);
}

VOID PrintSrCmdMeshTopo(IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo)
{
	PrintSrCmd(&(prSrCmdMeshTopo->rSrCmd));
	PrintSrMeshTopo(prSrCmdMeshTopo->rSrCmd.u1CmdSubId, &(prSrCmdMeshTopo->rSrCmdMeshTopo));
}

VOID PrintSrCmdSrUlStatus(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus)
{
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_INFO,
		"u1UlStatus = %d DbdcIdx:%u CmdSubId:%u\n",
		prSrCmdSrUlStatus->rSrUlStatus.u1UlStatus, prSrCmdSrUlStatus->rSrCmd.u1DbdcIdx, prSrCmdSrUlStatus->rSrCmd.u1CmdSubId);
}

VOID PrintSrCmdSrMapBalance(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance)
{
	PrintSrCmd(&(prSrCmdMapBalance->rSrCmd));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"MapBalance = %u\n",
		prSrCmdMapBalance->rSrMapBalance.u1MapBalance);
}

VOID PrintSrEvent(IN struct SR_EVENT_T *prSrEvent)
{
	CHAR *status[5] = {"SUCCESS", "SANITY_FAIL", "CALL_MIDDLE_FAIL", "SW_HW_VAL_NOT_SYNC", "UNKNOWN"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1EventSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %s\n"
			"u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
			prSrEvent->u1EventSubId, prSrEvent->u1ArgNum, prSrEvent->u1DbdcIdx, status[prSrEvent->u1Status],
			prSrEvent->u1DropTaIdx, prSrEvent->u1StaIdx, prSrEvent->u4Value);
}

VOID PrintSrEventSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCap)
{
	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_CAP_T_SR_V1 *prSrEventSrCap = (struct SR_EVENT_SR_CAP_T_SR_V1 *)_prSrEventSrCap;

		PrintSrEvent(&(prSrEventSrCap->rSrEvent));
		PrintSrCap(pAd, &(prSrEventSrCap->rSrCap));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_CAP_T_SR_V2 *prSrEventSrCap = (struct SR_EVENT_SR_CAP_T_SR_V2 *)_prSrEventSrCap;

		PrintSrEvent(&(prSrEventSrCap->rSrEvent));
		PrintSrCap(pAd, &(prSrEventSrCap->rSrCap));
	}
}

VOID PrintSrEventSrPara(IN struct SR_EVENT_SR_PARA_T *prSrEventSrPara)
{
	PrintSrEvent(&(prSrEventSrPara->rSrEvent));
	PrintSrPara(&(prSrEventSrPara->rSrPara));
}

VOID PrintSrEventSrInd(IN struct SR_EVENT_SR_IND_T  *prSrEventSrInd)
{
	PrintSrEvent(&(prSrEventSrInd->rSrEvent));
	PrintSrInd(&(prSrEventSrInd->rSrInd));
}


VOID PrintSrEventSrGloVarSingleDropTa(IN struct SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *
				      prSrEventSrGlobalVarSingleDropTa)
{
	UINT_8 u1DropTaIdx, u1StaIdx;

	u1DropTaIdx = prSrEventSrGlobalVarSingleDropTa->rSrEvent.u1DropTaIdx;
	u1StaIdx = prSrEventSrGlobalVarSingleDropTa->rSrEvent.u1StaIdx;

	PrintSrEvent(&(prSrEventSrGlobalVarSingleDropTa->rSrEvent));
	PrintSrGloVarSingleDropTa(&(prSrEventSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa),
				  u1DropTaIdx, u1StaIdx);
}

VOID PrintSrEventSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCond)
{
	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_COND_T_SR_V1 *prSrEventSrCond = (struct SR_EVENT_SR_COND_T_SR_V1 *)_prSrEventSrCond;

		PrintSrEvent(&(prSrEventSrCond->rSrEvent));
		PrintSrCond(pAd, &(prSrEventSrCond->rSrCond));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_COND_T_SR_V2 *prSrEventSrCond = (struct SR_EVENT_SR_COND_T_SR_V2 *)_prSrEventSrCond;

		PrintSrEvent(&(prSrEventSrCond->rSrEvent));
		PrintSrCond(pAd, &(prSrEventSrCond->rSrCond));
	}
}

VOID PrintSrEventSrRcpiTbl(IN struct SR_EVENT_SR_RCPITBL_T *prSrEventSrRcpiTbl)
{
	PrintSrEvent(&(prSrEventSrRcpiTbl->rSrEvent));
	PrintSrRcpiTbl(&(prSrEventSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrEventSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V1 *prSrEventSrRcpiTblOfst = (struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V1 *)_prSrEventSrRcpiTblOfst;

		PrintSrEvent(&(prSrEventSrRcpiTblOfst->rSrEvent));
		PrintSrRcpiTblOfst(pAd, &(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V2 *prSrEventSrRcpiTblOfst = (struct SR_EVENT_SR_RCPITBL_OFST_T_SR_V2 *)_prSrEventSrRcpiTblOfst;

		PrintSrEvent(&(prSrEventSrRcpiTblOfst->rSrEvent));
		PrintSrRcpiTblOfst(pAd, &(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
	}
}
VOID PrintSrEventSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_Q_CTRL_T_SR_V1 *prSrEventSrQCtrl = (struct SR_EVENT_SR_Q_CTRL_T_SR_V1 *)_prSrEventSrQCtrl;

		PrintSrEvent(&(prSrEventSrQCtrl->rSrEvent));
		PrintSrQCtrl(pAd, &(prSrEventSrQCtrl->rSrQCtrl));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_Q_CTRL_T_SR_V2 *prSrEventSrQCtrl = (struct SR_EVENT_SR_Q_CTRL_T_SR_V2 *)_prSrEventSrQCtrl;

		PrintSrEvent(&(prSrEventSrQCtrl->rSrEvent));
		PrintSrQCtrl(pAd, &(prSrEventSrQCtrl->rSrQCtrl));
	}
}

VOID PrintSrEventSrIBPD(IN struct SR_EVENT_SR_IBPD_T *prSrEventSrIBPD)
{
	PrintSrEvent(&(prSrEventSrIBPD->rSrEvent));
	PrintSrIBPD(&(prSrEventSrIBPD->rSrIBPD));
}

VOID PrintSrEventSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRT)
{
	UINT_8 u1Index;

	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_NRT_T_SR_V1 *prSrEventSrNRT = (struct SR_EVENT_SR_NRT_T_SR_V1 *)_prSrEventSrNRT;

		PrintSrEvent(&(prSrEventSrNRT->rSrEvent));
		for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
			PrintSrNRT(pAd, &(prSrEventSrNRT->rSrNRT[u1Index]));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_NRT_T_SR_V2 *prSrEventSrNRT = (struct SR_EVENT_SR_NRT_T_SR_V2 *)_prSrEventSrNRT;

		PrintSrEvent(&(prSrEventSrNRT->rSrEvent));
		for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
			PrintSrNRT(pAd, &(prSrEventSrNRT->rSrNRT[u1Index]));
	}
}

VOID PrintSrEventSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		struct SR_EVENT_SR_NRT_CTRL_T_SR_V1 *prSrEventSrNRTCtrl = (struct SR_EVENT_SR_NRT_CTRL_T_SR_V1 *)_prSrEventSrNRTCtrl;

		PrintSrEvent(&(prSrEventSrNRTCtrl->rSrEvent));
		PrintSrNRTCtrl(pAd, &(prSrEventSrNRTCtrl->rSrNRTCtrl));
	}

	if (IS_SR_V2(pAd)) {
		struct SR_EVENT_SR_NRT_CTRL_T_SR_V2 *prSrEventSrNRTCtrl = (struct SR_EVENT_SR_NRT_CTRL_T_SR_V2 *)_prSrEventSrNRTCtrl;

		PrintSrEvent(&(prSrEventSrNRTCtrl->rSrEvent));
		PrintSrNRTCtrl(pAd, &(prSrEventSrNRTCtrl->rSrNRTCtrl));
	}
}

VOID PrintSrEventSrFNQCtrl(IN struct SR_EVENT_SR_FNQ_CTRL_T *prSrEventSrFNQCtrl)
{
	PrintSrEvent(&(prSrEventSrFNQCtrl->rSrEvent));
	PrintSrFNQCtrl(&(prSrEventSrFNQCtrl->rSrFNQCtrl));
}

VOID PrintSrEventSrFrmFilt(IN struct SR_EVENT_SR_FRM_FILT_T *prSrEventSrFrmFilt)
{
	PrintSrEvent(&(prSrEventSrFrmFilt->rSrEvent));
	PrintSrFrmFilt(&(prSrEventSrFrmFilt->u4SrFrmFilt));
}

VOID PrintSrEventSrInterPsCtrl(IN struct SR_EVENT_SR_INTERPS_CTRL_T *prSrEventSrInterPsCtrl)
{
	PrintSrEvent(&(prSrEventSrInterPsCtrl->rSrEvent));
	PrintSrInterPsCtrl(&(prSrEventSrInterPsCtrl->rSrInterPsCtrl));
}

VOID PrintSrEventSrInterPsDbg(IN struct SR_EVENT_SR_INTERPS_DBG_T *prSrEventSrInterPsDbg)
{
	PrintSrEvent(&(prSrEventSrInterPsDbg->rSrEvent));
	PrintSrInterPsDbg(&(prSrEventSrInterPsDbg->rSrInterPsDbg));
}

VOID PrintSrEventSrCnt(IN struct SR_EVENT_SR_CNT_T *prSrEventSrCnt)
{
	PrintSrEvent(&(prSrEventSrCnt->rSrEvent));
	PrintSrCnt(prSrEventSrCnt->rSrEvent.u1DbdcIdx, prSrEventSrCnt->rSrCnt, prSrEventSrCnt->rSrEvent.u4Value);
}

VOID PrintSrEventSrSd(IN struct SR_EVENT_SR_SD_T *prSrEventSrSd)
{
	PrintSrEvent(&(prSrEventSrSd->rSrEvent));
	PrintSrSd(prSrEventSrSd->rSrEvent.u1DbdcIdx, prSrEventSrSd->rSrSd);
}

VOID PrintSrEventSrSrgBitmap(IN struct SR_EVENT_SR_SRG_BITMAP_T *prSrEventSrSrgBitmap)
{
	PrintSrEvent(&(prSrEventSrSrgBitmap->rSrEvent));
	PrintSrSrgBitmap(prSrEventSrSrgBitmap->rSrEvent.u1DbdcIdx, prSrEventSrSrgBitmap->rSrSrgBitmap);
}

VOID PrintSrEventSrSiga(IN PRTMP_ADAPTER pAD, IN struct SR_EVENT_SR_SIGA_T *prSrEventSrSigaFlag)
{
	PrintSrEvent(&(prSrEventSrSigaFlag->rSrEvent));
	PrintSrSiga(pAD, prSrEventSrSigaFlag->rSrEvent.u1DbdcIdx, &(prSrEventSrSigaFlag->rSrSigaFlag), TRUE);
}

VOID PrintSrEventSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_SIGA_AUTO_T *prSrEventSrSigaAutoFlag)
{
	PrintSrEvent(&(prSrEventSrSigaAutoFlag->rSrEvent));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"SrSigaAuto = %d\n",
		prSrEventSrSigaAutoFlag->rSrEvent.u4Value);
}

VOID PrintSrEventMeshTopo(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_MESH_TOPOLOGY_T *prSrEventMeshTopology)
{
	PrintSrEvent(&(prSrEventMeshTopology->rSrEvent));
	PrintSrMeshTopo(prSrEventMeshTopology->rSrEvent.u1EventSubId, &(prSrEventMeshTopology->rSrCmdMeshTopo));
}

VOID PrintSrEventSrUlStatus(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus)
{
	PrintSrEvent(&(prSrEventSrUlStatus->rSrEvent));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u1UlStatus = %d\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
}

VOID PrintSrEventSrMeshPhase(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_MESH_PHASE_T *prSrEventSrMeshPhase)
{
	PrintSrEvent(&(prSrEventSrMeshPhase->rSrEvent));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"SrMeshPhase = %d\n",
		prSrEventSrMeshPhase->rSrEvent.u4Value);
}

VOID PrintSrEventSrRemoteAPStaAllHe(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *prSrEventSrRemoteAPStaAllHe)
{
	PrintSrEvent(&(prSrEventSrRemoteAPStaAllHe->rSrEvent));
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"SrMeshRemoteAPStaAllHe = %u\n",
		prSrEventSrRemoteAPStaAllHe->rSrEvent.u4Value);
}

void PrintSharedStaRaInfo(IN PRTMP_ADAPTER pAd, IN struct SHARED_STA_RA_T *prCsrSharedInfo)
{
	UINT_32 per = prCsrSharedInfo->u2CoSrTtlTxCnt == 0 ? 0 : 1000 * (prCsrSharedInfo->u2CoSrTtlTxCnt - prCsrSharedInfo->u2CoSrSucCnt) / prCsrSharedInfo->u2CoSrTtlTxCnt;
	CHAR *mode[2] = {"AUTO", "FIXED"};
	CHAR *state[5] = {"Invalid", "Not Stable", "Good", "Bad", "Timeout"};
	CHAR *rastate[2] = {"Stable", "Active"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1Mode = %s, u1State = %s\n",
		  mode[prCsrSharedInfo->u1Mode], state[prCsrSharedInfo->u1State]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "CoSrRateOffset = %d, CoSrRaTryCnt = %x, CoSrRaRound = %x, CoSrRaState = %s\n",
		  prCsrSharedInfo->i1CoSrRateOffset, prCsrSharedInfo->u1CoSrRaTryCnt, prCsrSharedInfo->u1CoSrRaRound, rastate[prCsrSharedInfo->u1CoSrRaState]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "CoSrSucCnt = %x, CoSrTtlTxCnt = %x, PER = %d.%1d%%\n",
		  prCsrSharedInfo->u2CoSrSucCnt, prCsrSharedInfo->u2CoSrTtlTxCnt, per / 10, per % 10);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Score = %x, BadQuota = %d, BadLevel = %d, SrRate = %x,  FixCoSrRate = %x\n",
		  prCsrSharedInfo->u4Score, prCsrSharedInfo->u2BadQuota, prCsrSharedInfo->u1BadLevel, prCsrSharedInfo->u1CoSrRate, prCsrSharedInfo->u1FixCoSrRate);

}

void PrintSharingStaRaInfo(IN PRTMP_ADAPTER pAd, IN struct SHARING_STA_RA_T *prCsrSharingInfo)
{
	UINT_32 per = prCsrSharingInfo->u2CoSrTtlTxCnt == 0 ? 0 : 1000 * (prCsrSharingInfo->u2CoSrTtlTxCnt - prCsrSharingInfo->u2CoSrSucCnt) / prCsrSharingInfo->u2CoSrTtlTxCnt;
	CHAR *mode[2] = {"AUTO", "FIXED"};
	CHAR *state[5] = {"Invalid", "Not Stable", "Good", "Bad", "Timeout"};
	CHAR *rastate[2] = {"Stable", "Active"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		  "u1Mode = %s, u1State = %s\n",
		  mode[prCsrSharingInfo->u1Mode], state[prCsrSharingInfo->u1State]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "TxPowerOffset = %d, CoSrRaTryCnt = %x, CoSrRaRound = %x, CoSrRaState = %s\n",
		  prCsrSharingInfo->u1TxPowerOffset, prCsrSharingInfo->u1CoSrPwrTryCnt, prCsrSharingInfo->u1CoSrPwrRound, rastate[prCsrSharingInfo->u1CoSrPwrState]);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "CoSrSucCnt = %x, CoSrTtlTxCnt = %x, PER = %d.%1d%%\n",
		  prCsrSharingInfo->u2CoSrSucCnt, prCsrSharingInfo->u2CoSrTtlTxCnt, per / 10, per % 10);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"BadQuota = %d, BadLevel = %d, SrRate = %x,  FixCoSRPwr = %x\n",
		prCsrSharingInfo->u2BadQuota, prCsrSharingInfo->u1BadLevel, prCsrSharingInfo->i1CoSrPwr, prCsrSharingInfo->i1FixCoSRPwr);

}
VOID SrMeshSelfSrgBMChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, IN BOOLEAN fgPBssidUpd)
{
	UCHAR band_idx;

	band_idx = HcGetBandByWdev(wdev);

	if (fgPBssidUpd)
		SrMeshSelfPBssidChangeEvent(pAd, wdev, band_idx);

	SrMeshSelfBssColorChangeEvent(pAd, wdev, band_idx);

	if (pAd->SrSelfSrgBMMode == ENUM_SR_SELF_BM_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
			"SrSelfSrgBMMode:%u Manual for band:%u\n",
			pAd->SrSelfSrgBMMode, band_idx);
		return;
	}

	SrMeshSelfSrgInfoEvent(pAd, band_idx);
}

VOID SrMeshSelfBssColorChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap;
	UINT_8 bitmap[8];

	prSrgBitmap = &pAd->SrSelfSrgBM;

	prSrgBitmap->u4Color_31_0 = 0;
	prSrgBitmap->u4Color_63_32 = 0;

	bss_color_for_all_wdev(wdev, bitmap);

	prSrgBitmap->u4Color_31_0 = (bitmap[3] << 24) | (bitmap[2] << 16) | (bitmap[1] << 8) | bitmap[0];
	prSrgBitmap->u4Color_63_32 = (bitmap[7] << 24) | (bitmap[6] << 16) | (bitmap[5] << 8) | bitmap[4];

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"Color:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0);
}

VOID SrMeshSelfPBssidChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap;
	UCHAR index, par_bssid;
	struct wifi_dev *temp_wdev = NULL;

	prSrgBitmap = &pAd->SrSelfSrgBM;

	prSrgBitmap->u4pBssid_31_0 = 0;
	prSrgBitmap->u4pBssid_63_32 = 0;

	for (index = MAIN_MBSSID; index < pAd->ApCfg.BssidNum; index++) {
		temp_wdev = &pAd->ApCfg.MBSSID[index].wdev;

		if (band_idx != HcGetBandByWdev(temp_wdev))
			continue;

		if (WDEV_BSS_STATE(temp_wdev) != BSS_READY)
			continue;

		par_bssid = (temp_wdev->bssid[4] >> 7) | ((temp_wdev->bssid[5] & 0x1F) << 1);

		if (par_bssid < 32)
			prSrgBitmap->u4pBssid_31_0 |= (1 << par_bssid);
		else
			prSrgBitmap->u4pBssid_63_32 |= (1 << (par_bssid - 32));
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"PBssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);
}

VOID SrMeshSelfSrgInfoEvent(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	struct SR_MESH_SRG_BITMAP_T *prSrgBitmap;

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)

	struct wifi_dev *wdev = wdev_search_by_band_omac_idx(pAd, u1BandIdx, HW_BSSID_0);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"%s: wdev is NULL", __func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_DEBUG,
		"%s: SrSelfSrgBMMode:%u for band:%u and MAP_ENABLE:%u MAP_R3_ENABLE:%u SRMode:%u\n",
		__func__, pAd->SrSelfSrgBMMode, u1BandIdx, IS_MAP_ENABLE(pAd),
		IS_MAP_R3_ENABLE(pAd), pAd->CommonCfg.SRMode);
#endif

	if (pAd->SrSelfSrgBMMode == ENUM_SR_SELF_BM_MANUAL)
		prSrgBitmap = &pAd->SrSelfSrgBMMan;
	else
		prSrgBitmap = &pAd->SrSelfSrgBM;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_INFO,
		"Color:[63_32][%x]-[31_0][%x] Bssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
		wapp_send_sr_self_srg_bm_event(pAd, wdev, (PUINT8)prSrgBitmap);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

VOID ExtEventMeshUplinkTraffic(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, prSrEventSrUlStatus->rSrEvent.u1DbdcIdx, HW_BSSID_0);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
		"u1UlStatus = %u Band:%u\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus, prSrEventSrUlStatus->rSrEvent.u1DbdcIdx);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	wapp_send_uplink_traffic_event(pAd, wdev,
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

VOID SrMeshSrUpdateSTAMode(IN PRTMP_ADAPTER pAd, BOOL Assoc, UINT8 CurrStaIsHe)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 wcid, stacnt = 0;
	UCHAR band_idx;

	band_idx = hc_get_hw_band_idx(pAd);

	if (pAd->StaModeRptUnLock == 0)
		return;

	if (Assoc) {
		if (!CurrStaIsHe) {
			SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
			return;
		}
	}

	for (wcid = 0; wcid < WTBL_MAX_NUM(pAd); wcid++) {
		pEntry = entry_get(pAd, wcid);

		if (pEntry && !IS_ENTRY_NONE(pEntry) && !IS_ENTRY_MCAST(pEntry)) {

			if (pEntry->pAd != pAd)
				continue;

			stacnt++;

			if (!IS_HE_STA(pEntry->cap.modes)) {
				SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
				return;
			}
		}
	}

	if (stacnt)
		SrMeshSrReportSTAMode(pAd, band_idx, TRUE);
	else
		SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
}

VOID SrMeshSrReportSTAMode(IN PRTMP_ADAPTER pAd, UINT8 u1BandIdx, UINT8 u1StaAllHe)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, u1BandIdx, HW_BSSID_0);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
		"u1StaAllHe = %u Band:%u\n", u1StaAllHe, u1BandIdx);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode == 1))
		wapp_send_sta_mode_rpt_event(pAd, wdev, u1StaAllHe);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

UINT_8 SRRcpiConv(IN INT_8 i1Dbm)
{
	/*
	   dBm = (RCPI-220)/2
	   RCPI = (dBm * 2) + 220
		= (dBm * 2) + (110 * 2)
		= (dBm + 110) * 2
				  *dBm must be negative
	*/

	if (i1Dbm < SR_RCPI_MIN) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, " Err. i1Dbm:%d\n", i1Dbm);
		return (UINT_8)((SR_RCPI_MIN + 110) << 1);
	} else if (i1Dbm > SR_RCPI_MAX) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, " Err. i1Dbm:%d\n", i1Dbm);
		return (UINT_8)((SR_RCPI_MAX + 110) << 1);
	} else {
		return (UINT_8)((i1Dbm + 110) << 1);
	}

}

INT_8 SRDbmConv(IN UINT_8 u1Rcpi)
{
	/*
	   dBm = (RCPI-220)/2
	*/
	return (u1Rcpi >> 1) - 110;

}

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventMeshUplinkTraffic(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus = (struct UNI_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data;
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, u1DbdcIdx, HW_BSSID_0);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_DEBUG,
		"u1UlStatus = %u Band:%u\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus, u1DbdcIdx);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	wapp_send_uplink_traffic_event(pAd, wdev,
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

VOID UniSrEventSrUlStatus(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus = (struct UNI_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);
	MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"u1UlStatus = %d\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
}

VOID UniSrMeshTopo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{

	struct UNI_EVENT_MESH_TOPOLOGY_T *prSrMeshTopo = (struct UNI_EVENT_MESH_TOPOLOGY_T *)Data;

	if (prSrMeshTopo->u2Tag == UNI_EVENT_SR_HW_REMOTE_FH_RSSI)
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1CmdSubId:%u i1Rssi:%d FHStat:%u\n",
			prSrMeshTopo->u2Tag, prSrMeshTopo->rSrCmdMeshTopo.rRemoteFhParams.i1Rssi,
			prSrMeshTopo->rSrCmdMeshTopo.rRemoteFhParams.u1RemoteFhStat);
	else if (prSrMeshTopo->u2Tag == UNI_EVENT_SR_HW_REMOTE_BH_INFO)
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1CmdSubId:%u u2RemoteBhWcid:%u u1RemoteBhType:%u\n",
			prSrMeshTopo->u2Tag, prSrMeshTopo->rSrCmdMeshTopo.rRemoteBhParams.u2RemoteBhWcid,
			prSrMeshTopo->rSrCmdMeshTopo.rRemoteBhParams.u1RemoteBhType);
	else if (prSrMeshTopo->u2Tag == UNI_EVENT_SR_HW_MAP_TOPO)
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1CmdSubId:%u u1MapDevCount:%u u1MapDevSrSupportMode:%u u1SelfRole:%u\n",
			prSrMeshTopo->u2Tag, prSrMeshTopo->rSrCmdMeshTopo.rMapTopoParams.u1MapDevCount,
			prSrMeshTopo->rSrCmdMeshTopo.rMapTopoParams.u1MapDevSrSupportMode,
			prSrMeshTopo->rSrCmdMeshTopo.rMapTopoParams.u1SelfRole);
	else
		return;
}


VOID UniSrEventMeshStaThreshold(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_MESH_DL_STA_THRESHOLD_T *prSrEventMeshDlStaTh = (struct UNI_EVENT_SR_MESH_DL_STA_THRESHOLD_T  *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintMeshDlStaTh(pAd, u1DbdcIdx, &(prSrEventMeshDlStaTh->rSrMeshSrDLStaThreshold), TRUE);
}

VOID UniSrEventSrSiga(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_SIGA_T *prSrEventSrSigaFlag = (struct UNI_EVENT_SR_SIGA_T  *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrSiga(pAd, u1DbdcIdx, &(prSrEventSrSigaFlag->rSrSigaFlag), TRUE);
}

VOID UniPrintSrTxCount(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{

	struct UNI_EVENT_SR_SW_SR_TX_COUNTER_T *prSrTxCount = (struct UNI_EVENT_SR_SW_SR_TX_COUNTER_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"SRTxCount = %d, SRTxSucCount = %d\n", prSrTxCount->u4SrTotalTxCount, prSrTxCount->u4SrTotalSucTxCount);

}

VOID UniSrEventSrInterPsDbg(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_INTERPS_DBG_T *prSrEventSrInterPsDbg = (struct UNI_EVENT_SR_INTERPS_DBG_T  *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrInterPsDbg(&(prSrEventSrInterPsDbg->rSrInterPsDbg));
}

VOID UniSrEventSrInterPsCtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_INTERPS_CTRL_T *prSrEventSrInterPsCtrl = (struct UNI_EVENT_SR_INTERPS_CTRL_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrInterPsCtrl(&(prSrEventSrInterPsCtrl->rSrInterPsCtrl));
}

VOID UniSrEventSrFrmFilt(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_FRM_FILT_T *prSrEventSrFrmFilt = (struct UNI_EVENT_SR_FRM_FILT_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);
	PrintSrFrmFilt(&(prSrEventSrFrmFilt->u4SrFrmFilt));
}

VOID UniSrEventSrFNQCtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_FNQ_CTRL_T *prSrEventSrFNQCtrl = (struct UNI_EVENT_SR_FNQ_CTRL_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrFNQCtrl(&(prSrEventSrFNQCtrl->rSrFNQCtrl));
}

VOID UniSrEventSrInd(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_IND_T *prSrEventSrInd = (struct UNI_EVENT_SR_IND_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrInd(&(prSrEventSrInd->rSrInd));
}

VOID UniSrEventSrNRTCtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{

	struct UNI_EVENT_SR_NRT_CTRL_T_SR_V2 *prSrEventSrNRTCtrl = (struct UNI_EVENT_SR_NRT_CTRL_T_SR_V2 *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrNRTCtrl(pAd, &(prSrEventSrNRTCtrl->rSrNRTCtrl));
}

VOID UniSrEventSrNRT(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	UINT_8 u1Index;

	struct UNI_EVENT_SR_NRT_T_SR_V2 *prSrEventSrNRT = (struct UNI_EVENT_SR_NRT_T_SR_V2 *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
		PrintSrNRT(pAd, &(prSrEventSrNRT->rSrNRT[u1Index]));
}

VOID UniSrCmdSrIBPD(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_IBPD_T *prSrCmdSrIBPD = (struct UNI_EVENT_SR_IBPD_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrIBPD(&(prSrCmdSrIBPD->rSrIBPD));
}

VOID UniSrCmdSrQCtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{

	struct UNI_EVENT_SR_Q_CTRL_T_SR_V2 *prSrCmdSrQCtrl = (struct UNI_EVENT_SR_Q_CTRL_T_SR_V2 *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrQCtrl(pAd, &(prSrCmdSrQCtrl->rSrQCtrl));
}

VOID UniSrEventSrRcpiTblOfst(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_RCPITBL_OFST_T_SR_V2 *prSrEventSrRcpiTblOfst = (struct UNI_EVENT_SR_RCPITBL_OFST_T_SR_V2 *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrRcpiTblOfst(pAd, &(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
}

VOID UniSrEventSrRcpiTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_RCPITBL_T *prSrEventSrRcpiTbl = (struct UNI_EVENT_SR_RCPITBL_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrRcpiTbl(&(prSrEventSrRcpiTbl->rSrRcpiTbl));
}

VOID UniSrEventSrCond(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_COND_T_SR_V2 *prSrEventSrCond = (struct UNI_EVENT_SR_COND_T_SR_V2 *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrCond(pAd, &(prSrEventSrCond->rSrCond));
}

VOID UniSrEventSrPara(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_PARA_T *prSrEventSrPara = (struct UNI_EVENT_SR_PARA_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrPara(&(prSrEventSrPara->rSrPara));
}

void UniSrEventSrCap(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_HW_CAP_T *prSrEventSrCap = (struct UNI_EVENT_SR_HW_CAP_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1DbdcIdx = %d\n", u1DbdcIdx);

	PrintSrCap(pAd, &(prSrEventSrCap->rSrCap));
}
void UniSrEventGloverDrop(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *prSrEventGloverDrop = (struct UNI_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		 "SR Info - u1CurSrDropTaIdx = %d, u2SrTtlTxCntThr=%d\n",
		  prSrEventGloverDrop->u1CurSrDropTaIdx,
		  prSrEventGloverDrop->u2SrTtlTxCntThr);

	PrintSrDropTaInfo(&(prSrEventGloverDrop->rSrDropTaInfo), prSrEventGloverDrop->u1DropTaIdx, prSrEventGloverDrop->u1StaIdx);

}
void UniCsrEventFixPwr(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_CSR_FIXED_TXPOWER *prCsrFixedPwr = (struct UNI_EVENT_CSR_FIXED_TXPOWER *)Data;
	CHAR *mode[2] = {"AUTO", "FIXED"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Mode = %s\n"
		"Fixed Power = %d\n",
		mode[prCsrFixedPwr->u1Mode],
		prCsrFixedPwr->u1FixedTXpower);

}

void UniCsrEventFixRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_CSR_FIXED_RATE *prCsrFixedRate = (struct UNI_EVENT_CSR_FIXED_RATE *)Data;
	CHAR *mode[2] = {"AUTO", "FIXED"};

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Mode = %s\n"
		"Fixed Rate = %d\n",
		mode[prCsrFixedRate->u1Mode],
		prCsrFixedRate->u1FixedRate);

}

void UniCsrEventStaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_CSR_STA_INFO_T *prCsrStaInfo = (struct UNI_EVENT_CSR_STA_INFO_T *)Data;
	UINT_8 i = 0;

	for (i = 0; i < 8; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"STA Info\nWCID = %d\n"
			"MLD_ID = %d\n",
			prCsrStaInfo->wcid[i],
			prCsrStaInfo->mldId[i]);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"PL_Diff_0 = %d, PL_Diff_1 = %d, PL_Diff_2 = %d\n",
			prCsrStaInfo->plDiff[i][0], prCsrStaInfo->plDiff[i][1], prCsrStaInfo->plDiff[i][2]);

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"11k_Support = %u, Status = %u\n",
			(UINT_32)prCsrStaInfo->support_11k[i], (UINT_32)prCsrStaInfo->u1Status[i]);
	}
}

void UniCsrEventCnt(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_COSR_EVENT_REC_CNT_T *prCsrEventCnt = (struct UNI_EVENT_COSR_EVENT_REC_CNT_T *)Data;
	INT i = 0;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"Share to\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharingEventCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nSharing Tx Success\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharingAPTxSuccCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShare from\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedEventCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx STA Search Fail from\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxSTASrchFaiCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx Fail TxQ Empty\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxQEmptyCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx insufficient Tx Time\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPInsufTimeCnt[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx Fail Txc Enqueue Abort\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxCEnqAbortCnt[i]);
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx TxC Enqueue Success\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxCEnqCnt[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx Success\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxSuccCnt[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nShared Tx Acked\n");
	for (i = 0; i < COSR_COOR_AP_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
			"CAP_%d : %d, ",
			i, prCsrEventCnt->SharedAPTxAcked[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nSharing Boost\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"CAP : %d, ",
		prCsrEventCnt->u2SharingBoostEventCnt);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"\nSharing Boost Tx Success\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE,
		"CAP : %d, ",
		prCsrEventCnt->u2SharingBoostTxSuccCnt);

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_NOTICE, "\n");
}

void UniCsrEventCoSrInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_CSR_INFO_T *prCsrInfo = (struct UNI_EVENT_CSR_INFO_T *)Data;

	PrintSharedStaRaInfo(pAd, &prCsrInfo->rSharedStaRAInfo);
	PrintSharingStaRaInfo(pAd, &prCsrInfo->rSharingStaRAInfo);
}



void UniCsrEventCoordApInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_CSR_COORDAP_INFO_T *prCsrCoordApInfo = (struct UNI_EVENT_CSR_COORDAP_INFO_T *)Data;
	UINT_8 i = 0;

	for (i = 0; i < 3; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"CoordAp Info\nAPID = %d\n"
			"CoorAPStatus = %d\n",
			prCsrCoordApInfo->apId[i],
			prCsrCoordApInfo->CoorAPStatus[i]);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_SR, DBG_LVL_ERROR,
			"CoorAPBSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
			PRINT_MAC(prCsrCoordApInfo->aCoorAPBSSID[i]));

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"u1Rssi = %d\n",
			prCsrCoordApInfo->u1Rssi[i]);
	}
}

VOID UniSrEventSrSd(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_SD_T *prSrEventSrSd = (struct UNI_EVENT_SR_SD_T *)Data;

	PrintSrSd(u1DbdcIdx, prSrEventSrSd->rSrSd);
}

void UniPrintSrCnt(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_CNT_T *prSrEventSrCnt = (struct UNI_EVENT_SR_CNT_T *)Data;

	PrintSrCnt(u1DbdcIdx, prSrEventSrCnt->rSrCnt, prSrEventSrCnt->u4Value);
}

void UniSrSrgBitmap(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length, UINT8 u1DbdcIdx)
{
	struct UNI_EVENT_SR_SW_SRG_BITMAP_T *prSrEventSrgBitmap = (struct UNI_EVENT_SR_SW_SRG_BITMAP_T *)Data;
	UINT_8 u1BitmapIdx = 0, u1ColorEn = 0, u1pBssidEn = 0;
	CHAR *enable[2] = {" ", "V"};
	/*UINT_32 u4Color = 0, u4pBssid = 0;*/

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"Color - 31_0:%x, 63_32:%x pBssid - 31_0:%x, 63_32:%x\n",
		prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4Color_31_0, prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4Color_63_32,
		prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0, prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32);
	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR, "BIT  Color  pBssid\n");


	for (u1BitmapIdx = 0; u1BitmapIdx < 64; u1BitmapIdx++) {
		if (u1BitmapIdx < 32) {
			u1ColorEn = (prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4Color_31_0 & BIT(u1BitmapIdx)) >> u1BitmapIdx;
			u1pBssidEn = (prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4pBssid_31_0 & BIT(u1BitmapIdx)) >> u1BitmapIdx;
		} else {
			u1ColorEn = (prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4Color_63_32 & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
			u1pBssidEn = (prSrEventSrgBitmap->rSrSrgBitmap[u1DbdcIdx].u4pBssid_63_32 & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
		}

		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"%2d     %s      %s\n",
		u1BitmapIdx, enable[u1ColorEn], enable[u1pBssidEn]);
	}
}
void UniPrintSrEvent(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_SR_EVENT_T *prSrEvent = (struct UNI_EVENT_SR_EVENT_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			" u4Value = %d\n", prSrEvent->u4Value);
}

void UniPrintSrSupportmodeEvent(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_SR_EVENT_T *prSrEvent = (struct UNI_EVENT_SR_EVENT_T *)Data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
		"[%s] u4Value = %d\n", __func__, prSrEvent->u4Value);
	wapp_send_sr_support_mode_query_rsp(pAd, prSrEvent->u4Value);
}

void UniEventSRHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT8 u1DbdcIdx;
	struct UNI_EVENT_SR_T *prEventSrTlv;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_SR_T);
	struct _RTMP_ADAPTER *pAd = NULL;

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATCFG_SR, DBG_LVL_WARN,
			"invalid length = %d\n", Length);
		return;
	}

	prEventSrTlv = (struct UNI_EVENT_SR_T *)pData;
	u1DbdcIdx = prEventSrTlv->u1DbdcIdx;
	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;

	pAd = physical_device_get_mac_adapter_by_band(physical_dev, prEventSrTlv->u1DbdcIdx);
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATCFG_SR, DBG_LVL_ERROR,
			"Cannot find corresponding pAd(Band%d)\n", prEventSrTlv->u1DbdcIdx);
		return;
	}

	switch (*tag) {
	case UNI_EVENT_SR_CFG_SR_ENABLE:
	case UNI_EVENT_SR_CFG_SR_SD_ENABLE:
	case UNI_EVENT_SR_CFG_SR_MODE:
	case UNI_EVENT_SR_CFG_DISRT_ENABLE:
	case UNI_EVENT_SR_CFG_DISRT_MIN_RSSI:
	case UNI_EVENT_SR_CFG_SR_BF:
	case UNI_EVENT_SR_CFG_SR_ATF:
	case UNI_EVENT_SR_CFG_TXC_QUEUE:
	case UNI_EVENT_SR_CFG_TXC_QID:
	case UNI_EVENT_SR_CFG_TXC_PATH:
	case UNI_EVENT_SR_CFG_AC_METHOD:
	case UNI_EVENT_SR_CFG_SR_PERIOD_THR:
	case UNI_EVENT_SR_CFG_QUERY_TXD_METHOD:
	case UNI_EVENT_SR_CFG_SR_SD_CG_RATIO:
	case UNI_EVENT_SR_CFG_SR_SD_OBSS_RATIO:
	case UNI_EVENT_SR_CFG_PROFILE:
	case UNI_EVENT_SR_CFG_FNQ_ENABLE:
	case UNI_EVENT_SR_CFG_DPD_ENABLE:
	case UNI_EVENT_SR_CFG_SR_TX_ENABLE:
	case UNI_EVENT_SR_CFG_SR_SD_OM_ENABLE:
	case UNI_EVENT_SR_CFG_SR_TX_ALIGN_ENABLE:
	case UNI_EVENT_SR_CFG_SR_TX_ALIGN_RSSI_THR:
	case UNI_EVENT_SR_CFG_SR_DABS_MODE:
	case UNI_EVENT_CSR_CFG_ENABLE:
	case UNI_EVENT_SR_HW_NONSRG_SR_ENABLE:
	case UNI_EVENT_SR_HW_ESR_ENABLE:
	case UNI_EVENT_SR_SW_SR_APCLI_MODE:
	case UNI_EVENT_SR_SW_SR_RATE_OFFSET:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_CSR_FIX_PWR:
		UniCsrEventFixPwr(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_CSR_FIX_RATE:
		UniCsrEventFixRate(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_SRG_BITMAP:
		UniSrSrgBitmap(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_MESH_SRG_BITMAP:
		UniSrSrgBitmap(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_CNT:
		UniPrintSrCnt(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_SD:
		UniSrEventSrSd(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_GLOVAR_SINGLE_DROP_TA_INFO:
		UniSrEventGloverDrop(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_CSR_COORDAP_INFO:
		UniCsrEventCoordApInfo(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_CSR_STA_INFO:
		UniCsrEventStaInfo(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_CSR_EVENT_CNT:
		UniCsrEventCnt(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_CSR_INFO:
		UniCsrEventCoSrInfo(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_CAP:
		UniSrEventSrCap(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_PARA:
		UniSrEventSrPara(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_COND:
		UniSrEventSrCond(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_RCPI_TBL:
		UniSrEventSrRcpiTbl(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_RCPI_TBL_OFST:
		UniSrEventSrRcpiTblOfst(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_Q_CTRL:
		UniSrCmdSrQCtrl(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_IBPD:
		UniSrCmdSrIBPD(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_NRT:
		UniSrEventSrNRT(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_NRT_CTRL:
		UniSrEventSrNRTCtrl(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_IND:
		UniSrEventSrInd(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_FNQ:
		UniSrEventSrFNQCtrl(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_FRMFILT:
		UniSrEventSrFrmFilt(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_INTERPS_CTRL:
		UniSrEventSrInterPsCtrl(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_INTERPS_DBG:
		UniSrEventSrInterPsDbg(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_SIGA_FLAG:
		UniSrEventSrSiga(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_SIGA_AUTO_FLAG:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_HW_REMOTE_FH_RSSI:
	case UNI_EVENT_SR_HW_REMOTE_BH_INFO:
	case UNI_EVENT_SR_HW_MAP_TOPO:
		UniSrMeshTopo(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_UPLINK_STATUS:
		UniSrEventSrUlStatus(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SEND_MESH_UPLINK_TRAFFIC:
		UniEventMeshUplinkTraffic(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_HW_MAP_PHASE:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_HW_REMOTE_STA_MODE:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_SW_SR_TX_COUNTER:
		UniPrintSrTxCount(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_MESH_STA_RSSI_TH:
		UniSrEventMeshStaThreshold(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_BH_MESH_SRG_BITMAP:
		UniSrSrgBitmap(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_FH_MESH_SRG_BITMAP:
		UniSrSrgBitmap(pAd, tag, tags_len, u1DbdcIdx);
		break;
	case UNI_EVENT_SR_SW_BH_DL_MESH_SR_RSSI_TH:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_SW_FH_DL_MESH_SR_RSSI_TH:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_SW_BH_FORBID_BITMAP:
		UniPrintSrEvent(pAd, tag, tags_len);
		break;
	case UNI_EVENT_SR_SW_SR_SUPOORT_MODE:
		UniPrintSrSupportmodeEvent(pAd, tag, tags_len);
		break;
	}

}
#endif /* WIFI_UNIFIED_COMMAND */
#endif /* CFG_SUPPORT_FALCON_SR */
#endif /* defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993) */
