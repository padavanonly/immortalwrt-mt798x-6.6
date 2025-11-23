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
/****************************************************************************
 ****************************************************************************

    Module Name:
	cmm_info.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#include "hdev/hdev_basic.h"
#include "hdev/hdev.h"
#include "rtmp.h"
#include "rtmp_def.h"
#ifdef HWIFI_SUPPORT
#include "os/hwifi_main.h"
#endif /* HWIFI_SUPPORT */
#include "mgmt/be_internal.h"
#include "chlist.h"
#include "action.h"
#ifdef RT_CFG80211_SUPPORT
#include "mtk_vendor_nl80211.h"
#endif

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */
#define MIN(a, b)					((a) < (b) ? (a) : (b))
INT UNII4BandSupportRegions[] = {25, 26};
INT MCSMappingRateTable[] = {
	2,  4, 11, 22, 12,  18,  24,  36, 48,  72,  96, 108, 109, 110, 111, 112,/* CCK and OFDM */
	13, 26, 39, 52, 78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
	39, 78, 117, 156, 234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
	43, 87, 130, 173, 260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */

	/*for 11ac:20 Mhz 800ns GI*/
	6,  13, 19, 26,  39,  52,  58,  65,  78,  90,     /*1ss mcs 0~8*/
	13, 26, 39, 52,  78,  104, 117, 130, 156, 180,     /*2ss mcs 0~8*/
	19, 39, 58, 78,  117, 156, 175, 195, 234, 260,   /*3ss mcs 0~9*/
	26, 52, 78, 104, 156, 208, 234, 260, 312, 360,     /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 800ns GI*/
	13,	27,	40,	54,	 81,  108, 121, 135, 162, 180,   /*1ss mcs 0~9*/
	27,	54,	81,	108, 162, 216, 243, 270, 324, 360,   /*2ss mcs 0~9*/
	40,	81,	121, 162, 243, 324, 364, 405, 486, 540,  /*3ss mcs 0~9*/
	54,	108, 162, 216, 324, 432, 486, 540, 648, 720, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 800ns GI*/
	29,	58,	87,	117, 175, 234, 263, 292, 351, 390,   /*1ss mcs 0~9*/
	58,	117, 175, 243, 351, 468, 526, 585, 702, 780, /*2ss mcs 0~9*/
	87,	175, 263, 351, 526, 702, 0,	877, 1053, 1170, /*3ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 800ns GI*/
	58,	117, 175, 234, 351, 468, 526, 585, 702, 780, /*1ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*2ss mcs 0~9*/
	175, 351, 526, 702, 1053, 1404, 1579, 1755, 2160, 0, /*3ss mcs 0~8*/
	234, 468, 702, 936, 1404, 1872, 2106, 2340, 2808, 3120, /*4ss mcs 0~9*/

	/*for 11ac:20 Mhz 400ns GI*/
	7,	14,	21,	28,  43,  57,   65,	 72,  86,  100,    /*1ss mcs 0~8*/
	14,	28,	43,	57,	 86,  115,  130, 144, 173, 200,    /*2ss mcs 0~8*/
	21,	43,	65,	86,	 130, 173,  195, 216, 260, 288,  /*3ss mcs 0~9*/
	28,	57,	86,	115, 173, 231,  260, 288, 346, 400,    /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 400ns GI*/
	15,	30,	45,	60,	 90,  120,  135, 150, 180, 200,  /*1ss mcs 0~9*/
	30,	60,	90,	120, 180, 240,  270, 300, 360, 400,  /*2ss mcs 0~9*/
	45,	90,	135, 180, 270, 360,  405, 450, 540, 600, /*3ss mcs 0~9*/
	60,	120, 180, 240, 360, 480,  540, 600, 720, 800, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 400ns GI*/
	32,	65,	97,	130, 195, 260,  292, 325, 390, 433,  /*1ss mcs 0~9*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*2ss mcs 0~9*/
	97,	195, 292, 390, 585, 780,  0,	 975, 1170, 1300, /*3ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 400ns GI*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*1ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*2ss mcs 0~9*/
	195, 390, 585, 780, 1170, 1560,	1755, 1950, 2340, 0, /*3ss mcs 0~8*/
	260, 520, 780, 1040, 1560, 2080,	2340, 2600, 3120, 3466, /*4ss mcs 0~9*/

	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
}; /* 3*3 */

#ifdef DOT11_EHT_BE
#define MAX_NUM_EHT_BANDWIDTHS 5
#define MAX_NUM_EHT_SPATIAL_STREAMS 16
#define MAX_NUM_EHT_MCS_ENTRIES 14
UINT16 eht_mcs_phyrate_mapping_table[MAX_NUM_EHT_BANDWIDTHS][MAX_NUM_EHT_MCS_ENTRIES] = {
	/*20 Mhz 1SS*/
	{8, 17, 25, 34, 51, 68, 77, 86, 103, 114, 129, 143, 154, 172},
	/*40 Mhz 1 SS */
	{17, 34, 51, 68, 103, 137, 154, 172, 206, 229, 258, 286, 309, 344},
	/*80 Mhz 1 SS */
	{36, 72, 108, 144, 216, 288, 324, 360, 432, 480, 540, 600, 648, 720},
	/*160 Mhz 1 SS */
	{72, 144, 216, 288, 432, 576, 648, 720, 864, 960, 1080, 1201, 1297, 1441},
	/*320 Mhz 1 SS */
	{144, 288, 432, 576, 864, 1152, 1297, 1441, 1729, 1921, 2161, 2401, 2594, 2882}
};
#endif /* DOT11_EHT_BE */


#ifdef DOT11_HE_AX
#define MAX_NUM_HE_BANDWIDTHS 4
#define MAX_NUM_HE_SPATIAL_STREAMS 4
#define MAX_NUM_HE_MCS_ENTRIES 12
UINT16 he_mcs_phyrate_mapping_table[MAX_NUM_HE_BANDWIDTHS][MAX_NUM_HE_SPATIAL_STREAMS][MAX_NUM_HE_MCS_ENTRIES] = {
	{ /*20 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 8,
			 17,
			 25,
			 34,
			 51,
			 68,
			 77,
			 86,
			 103,
			 114,
			 129,
			 143
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 17,
			 34,
			 51,
			 68,
			 103,
			 137,
			 154,
			 172,
			 206,
			 229,
			 258,
			 286
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 25,
			 51,
			 77,
			 103,
			 154,
			 206,
			 232,
			 258,
			 309,
			 344,
			 387,
			 430
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 34,
			 68,
			 103,
			 137,
			 206,
			 275,
			 309,
			 344,
			 412,
			 458,
			 516,
			 573
		}
	},
	{ /*40 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 17,
			 34,
			 51,
			 68,
			 103,
			 137,
			 154,
			 172,
			 206,
			 229,
			 258,
			 286
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 34,
			 68,
			 103,
			 137,
			 206,
			 275,
			 309,
			 344,
			 412,
			 458,
			 516,
			 573

		},
		/* 3 SS */
		{
			/* DCM 0 */
			 51,
			 103,
			 154,
			 206,
			 309,
			 412,
			 464,
			 516,
			 619,
			 688,
			 774,
			 860

		},
		/* 4 SS */
		{
			/* DCM 0 */
			 68,
			 137,
			 206,
			 275,
			 412,
			 550,
			 619,
			 688,
			 825,
			 917,
			 1032,
			 1147
		}
	},
	{ /*80 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 36,
			 72,
			 108,
			 144,
			 216,
			 288,
			 324,
			 360,
			 432,
			 480,
			 540,
			 600
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 72,
			 144,
			 216,
			 288,
			 432,
			 576,
			 648,
			 720,
			 864,
			 960,
			 1080,
			 1201
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 108,
			 216,
			 324,
			 432,
			 648,
			 864,
			 972,
			 1080,
			 1297,
			 1441,
			 1621,
			 1801
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 144,
			 288,
			 432,
			 576,
			 864,
			 1152,
			 1297,
			 1141,
			 1729,
			 1921,
			 2161,
			 2401
		}
	},
	{ /*160 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 72,
			 144,
			 216,
			 288,
			 432,
			 576,
			 648,
			 720,
			 864,
			 960,
			 1080,
			 1201
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 144,
			 288,
			 432,
			 576,
			 864,
			 1152,
			 1297,
			 1441,
			 1729,
			 1921,
			 2161,
			 2401
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 216,
			 432,
			 648,
			 864,
			 1297,
			 1729,
			 1945,
			 2161,
			 2594,
			 2882,
			 3242,
			 3602
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 288,
			 576,
			 864,
			 1152,
			 1729,
			 2305,
			 2594,
			 2882,
			 3458,
			 3843,
			 4323,
			 4803
		},
	}

};

#endif /* DOT11_HE_AX */

VOID show_tpinfo_host(RTMP_ADAPTER *pAd, UINT32 option, UINT32 param0, UINT32 param1);
#ifdef CONFIG_DBG_QDISC
extern void os_system_tx_queue_dump(PNET_DEV dev);
#endif

static UINT32 debug_lvl = DBG_LVL_OFF;
static UINT32 debug_cat = DBG_CAT_ALL;

static VOID fw_ram_ver_to_file(char *ramVerStr)
{
	char *filePath = "/tmp/log/FW_version.txt";
	RTMP_OS_FD_EXT srcf;
	INT8 Ret;

	os_file_open(filePath, (RTMP_OS_FD_EXT *)&srcf, O_WRONLY|O_CREAT|O_TRUNC, 0);
	if (srcf.Status) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
			"Open file \"%s\" failed!\n", filePath);
		return;
	}

	Ret = os_file_write(srcf, ramVerStr, strlen(ramVerStr));
	if (Ret) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
			"Write file \"%s\" failed! Ret=%d\n",
			filePath, Ret);
	}

	Ret = os_file_close(srcf);
	if (Ret) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
			"Close file \"%s\" failed! Ret=%d\n",
			filePath, Ret);
	}
}

/*
    ==========================================================================
    Description:
	Get Driver version.

    Return:
    ==========================================================================
*/
INT show_driverinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef HWIFI_SUPPORT
	char fw_ver[16] = {0};
	char build_date[16] = {0};
	char fw_ver_long[128] = {0};
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		MTWF_PRINT("Driver version: %s\n", AP_DRIVER_VERSION);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MTWF_PRINT("Driver version: %s\n", STA_DRIVER_VERSION);
#endif /* CONFIG_STA_SUPPORT */

	MTWF_PRINT("FW ver: 0x%x, HW ver: 0x%x, CHIP ID: 0x%x\n", pAd->FWVersion, pAd->HWVersion, pAd->ChipID);

#ifndef HWIFI_SUPPORT
	show_patch_info(pAd);
	show_fw_info(pAd);
#else
	hwifi_get_fw_info(pAd, WM_CPU, fw_ver, build_date, fw_ver_long);
	MTWF_PRINT("WM - FW ver: %s, build date: %s\n", fw_ver, build_date);
	fw_ram_ver_to_file(fw_ver_long);
	hwifi_get_fw_info(pAd, WA_CPU, fw_ver, build_date, fw_ver_long);
	MTWF_PRINT("WA - FW ver: %s, build date: %s\n", fw_ver, build_date);
	hwifi_get_fw_info(pAd, DSP_CPU, fw_ver, build_date, fw_ver_long);
	MTWF_PRINT("DSP- FW ver: %s, build date: %s\n", fw_ver, build_date);
#endif /* !HWIFI_SUPPORT */

	return TRUE;
}

INT	Set_Cmm_WirelessMode_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT	success = TRUE;
	LONG cfg_mode = os_str_tol(arg, 0, 10);
	USHORT wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	UINT32 i = 0;
	struct wifi_dev *TmpWdev = NULL;
#endif
	CHANNEL_CTRL *pChCtrl;
	INT32 IfIdx = pObj->ioctl_if;
#ifdef MT_DFS_SUPPORT
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (BandIdx >= RDD_BAND_NUM) {
		success = FALSE;
		MTWF_PRINT("%s: Invalid BandIdx: %d!!\n", __func__, BandIdx);
		goto error;
	}
#endif

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		success = FALSE;
		goto error;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!VALID_MBSS(pAd, IfIdx)) {
			success = FALSE;
			MTWF_PRINT("%s: Invalid IfIdx: %d!!\n", __func__, IfIdx);
			goto error;
		}

		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		if (WMODE_CAP_6G(wmode) && (!WMODE_CAP_6G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		} else if (WMODE_CAP_5G(wmode) && (!WMODE_CAP_5G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		} else if (WMODE_CAP_2G(wmode) && (!WMODE_CAP_2G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		}
		wdev->PhyMode = wmode;
#ifdef MBSS_SUPPORT
		success = RT_CfgSetMbssWirelessMode(pAd, arg);

		if (!success)
			goto error;

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;

			/*update WmmCapable*/
			if (!wmode_band_equal(TmpWdev->PhyMode, wmode))
				continue;

			TmpWdev->bWmmCapable = pAd->ApCfg.MBSSID[i].bWmmCapableOrg;
		}

		MTWF_PRINT("%s::(BSS%d=%d)\n",
			__func__, IfIdx, wdev->PhyMode);
#else
		success = RT_CfgSetWirelessMode(pAd, arg, wdev);

		if (!success)
			goto error;

#endif /*MBSS_SUPPORT*/
		hc_update_wdev(wdev);
		/* Change channel state to NONE */
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
#ifdef MT_DFS_SUPPORT
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
		BuildChannelList(pAd, wdev);
#endif
		RTMPUpdateRateInfo(wmode, &wdev->rate);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		rtmpeapupdaterateinfo(wmode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		RTMPSetPhyMode(pAd, wdev, wmode);
#ifdef WIFI_TWT_SUPPORT
#ifdef BCN_EXTCAP_VAR_LEN
		wlan_config_set_ext_cap_length(wdev, cfg_mode);
#endif /* BCN_EXTCAP_VAR_LEN */
#endif /* WIFI_TWT_SUPPORT */
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UINT8 ba_en = 1;
		PSTA_ADMIN_CONFIG pStaCfg;
		SCAN_INFO *ScanInfo = NULL;
		BSS_TABLE *ScanTab = NULL;

		if ((IfIdx >= pAd->MSTANum) || IfIdx < 0) {
			success = FALSE;
			MTWF_PRINT("%s: Invalid IfIdx: %d!!\n", __func__, IfIdx);
			goto error;
		}

		pStaCfg = &pAd->StaCfg[IfIdx];
		wdev = &pAd->StaCfg[IfIdx].wdev;
		if (WMODE_CAP_6G(wmode) && (!WMODE_CAP_6G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		} else if (WMODE_CAP_5G(wmode) && (!WMODE_CAP_5G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		} else if (WMODE_CAP_2G(wmode) && (!WMODE_CAP_2G(wdev->PhyMode))) {
			MTWF_PRINT("%s():phymode changed from %d to %d fail!\n",
				__func__, wdev->PhyMode, wmode);
			return FALSE;
		}

		wdev->PhyMode = wmode;
		ScanInfo = &wdev->ScanInfo;
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		success = RT_CfgSetWirelessMode(pAd, arg, wdev);

		if (!success)
			goto error;

		hc_update_wdev(wdev);
		/* Change channel state to NONE */
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef MT_DFS_SUPPORT
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
		BuildChannelList(pAd, wdev);
		RTMPUpdateRateInfo(wmode, &wdev->rate);
		RTMPSetPhyMode(pAd, wdev, wmode);
		BssTableInit(ScanTab);
		ScanInfo->LastScanTime = 0;
#ifdef DOT11_N_SUPPORT
		ba_en = (WMODE_CAP_N(wmode)) ? 1 : 0;
		wlan_config_set_ba_enable(wdev, ba_en);
#endif /* DOT11_N_SUPPORT */

		/* Set AdhocMode rates*/
		if (pStaCfg->BssType == BSS_ADHOC) {
			MlmeUpdateTxRates(pAd, FALSE, 0);
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	return success;
error:
	MTWF_PRINT("%s::parameters out of range\n", __func__);
	return success;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
    ==========================================================================
    Description:
	Set Wireless Mode for MBSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

/*
    ==========================================================================
    Description:
	Set Wireless Mode
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}

#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan = (UCHAR) os_str_tol(arg, 0, 10);
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan %d\n",
		pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan);
	return TRUE;
}
#endif

/*
 *  ==========================================================================
 *  Description:
 *	Set Probe_Rsp's times
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
*/

INT Set_Probe_Rsp_Times_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{

	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 ProbeRspTimes = (UINT8) os_str_tol(arg, 0, 10);

	if ((ProbeRspTimes > 10) || (ProbeRspTimes < 1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Set_PROBE_RSP_TIMES_Proc! INVALID, ProbeRspTimes(%d) should be <1~10>\n",
				 ProbeRspTimes);
		return FALSE;
	}

	cap->ProbeRspTimes = ProbeRspTimes;

	MTWF_PRINT("Set_PROBE_RSP_TIMES_Proc! ProbeRspTimes = %d\n", cap->ProbeRspTimes);

	return TRUE;
}


/*
    ==========================================================================
    Description:
	Set phy channel for debugging/testing
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT set_phy_channel_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	INT32 recv = 0;
	struct freq_oper freq;
	UCHAR band_idx = 0;
	/* ch parameters */
	UINT32 ch_band = 0;
	UINT32 ht_bw = 0;
	UINT32 vht_bw = 0;
	UINT32 bw = 0;
	UINT32 ext_cha = 0;
	UINT32 prim_ch = 0;
	UINT32 cen_ch_1 = 0;
	UINT32 cen_ch_2 = 0;
	UINT32 rx_stream = 0;
	UINT32 ap_bw = 0;
	UINT32 ap_cen_ch = 0;
	UINT32 scan = 0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev == NULL! if_type %d, if_idx = %d\n", pObj->ioctl_if_type, if_idx);
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"band_idx = %d\n", band_idx);

	os_zero_mem(&freq, sizeof(freq));
	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
							&(ch_band), &(ht_bw), &(vht_bw),
							&(bw), &(ext_cha), &(prim_ch),
							&(cen_ch_1), &(cen_ch_2), &(rx_stream),
							&(ap_bw), &(ap_cen_ch), &(scan));

		if (recv != 12) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"ch_band-ht_bw-vht_bw-"
					"bw-ext_cha-prim_ch-"
					"cen_ch_1-cen_ch_2-rx_stream-"
					"ap_bw-ap_cen_ch-scan\n");
			return TRUE;
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"ch_band = %d\n ht_bw = %d\n vht_bw = %d\n"
					"bw = %d\n ext_cha = %d\n prim_ch = %d\n cen_ch_1 = %d\n"
					"cen_ch_2 = %d\n rx_stream = %d\n ap_bw = %d\n ap_cen_ch = %d scan = %d\n",
					ch_band, ht_bw, vht_bw,
					bw, ext_cha, prim_ch, cen_ch_1,
					cen_ch_2, rx_stream, ap_bw, ap_cen_ch, scan);

		freq.ch_band = ch_band;
		freq.ht_bw = ht_bw;
		freq.vht_bw = vht_bw;
		freq.bw = bw;
		freq.ext_cha = ext_cha;
		freq.prim_ch = prim_ch;
		freq.cen_ch_1 = cen_ch_1;
		freq.cen_ch_2 = cen_ch_2;
		freq.rx_stream = rx_stream;
		freq.ap_bw = ap_bw;
		freq.ap_cen_ch = ap_cen_ch;

		AsicSwitchChannel(pAd, band_idx, &freq, scan);
	}

	return TRUE;
}

static BOOLEAN cmm_utl_get_first_bit(UINT_32 *flags, UCHAR *pBit)
{
	int	mask;
	int bit;

	for (bit = 31; bit >= 0; bit--) {
		mask = 1 << (bit);

		if (mask & *flags) {
			*pBit = bit;
			return TRUE;
		}
	}

	return FALSE;
}

static int cmm_utl_is_bit_set(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "error !!!!\n");
		return 0;
	}

	mask = 1 << (bit);
	return ((mask & *flags) != 0);
}

static void cmm_utl_set_bit(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "error !!!!\n");
		return;
	}

	mask = 1 << (bit);
	*flags |= mask;
}

static void cmm_utl_clear_bit(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "error !!!!\n");
		return;
	}

	mask = 1 << (bit);
	*flags &= (~mask);
}

/**
* ChOpTimeout - The ChannelOp timeout process.
* @pAd: pointer of the RTMP_ADAPTER
*
* The function is used for TakeChannelOpCharge timeout handling.
*/
VOID ChOpTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pContext->pAd;
	UCHAR BandIdx = pContext->BandIdx;
	UCHAR owner_bit = CH_OP_OWNER_IDLE;

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
		"for band:%d\n!!\n", BandIdx);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
		"pAd=%p, done=%x, ownBitMask(%p)=0x%x, ChOpWaitBitMask=0x%x\n ",
		pAd, pAd->ChOpCtrl.ChOpDone.done, &pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
	/* Clear owner bitmask to free operation flag */
	NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
	while (cmm_utl_get_first_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, &owner_bit)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"owner=%d hold the ChOpCharge too long! Force release now!!\n", owner_bit);
		cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner_bit);
	}
	NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);

	pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
	RTMP_OS_COMPLETE_ALL(&pAd->ChOpCtrl.ChOpDone);
}


/**
* ChannelOpCtrlInit - Init Channel Operation Control DB.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID ChannelOpCtrlInit(IN PRTMP_ADAPTER	pAd)
{
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "pAd=%p\n", pAd);

	NdisAllocateSpinLock(pAd, &pAd->ChOpCtrl.ChOpLock);
	RTMP_OS_INIT_COMPLETION(&pAd->ChOpCtrl.ChOpDone);
	pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
	pAd->ChOpCtrl.ChOpOwnerBitMask = 0;
	pAd->ChOpCtrl.ChOpWaitBitMask = 0;
}

/**
* ChannelOpCtrlDeinit - Deinit Channel Operation Control DB.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID ChannelOpCtrlDeinit(IN PRTMP_ADAPTER	pAd)
{
	BOOLEAN Cancelled;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "pAd=%p\n", pAd);

	if (pAd->ChOpCtrl.ChOpTimerRunning) {
		RTMPReleaseTimer(&pAd->ChOpCtrl.ChOpTimer, &Cancelled);
		pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
	}

	NdisFreeSpinLock(&pAd->ChOpCtrl.ChOpLock);
	RTMP_OS_EXIT_COMPLETION(&pAd->ChOpCtrl.ChOpDone);
	RTMP_OS_INIT_COMPLETION(&pAd->ChOpCtrl.ChOpDone);
}

/**
* TakeChannelOpCharge - Try to take charge of the channel operation.
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
* @owner: the owner who triggered the channel operation
* @wait: whether need wait for current op done
*
* The return value is - TRUE if take charge succeeded, FALSE if failed.
*
* The function should be used with ReleaseChannelOpCharge in pairs.
*/
BOOLEAN TakeChannelOpCharge(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR owner, BOOLEAN wait)
{
	BOOLEAN bCharge = FALSE, line_up = FALSE;
	UINT32 waitCnt = 0, waitTime = 10000; /* 10 sec */
	UCHAR wait_owner = CH_OP_OWNER_IDLE;
	UCHAR BandIdx = DBDC_BAND0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! wdev is NULL and return!!\n");
		return FALSE;
	}
	BandIdx = HcGetBandByWdev(wdev);

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Try to Take Channel Op Charge for band:%d\n!!\n", BandIdx);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. pAd=%p, owner=%d, wait=%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n",
		OS_TRACE, pAd, owner, wait, pAd->ChOpCtrl.ChOpDone.done,
		pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask,
		pAd->ChOpCtrl.ChOpTimerRunning);

	do {
		NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
		if (pAd->ChOpCtrl.ChOpOwnerBitMask == CH_OP_OWNER_IDLE) {
			if (cmm_utl_get_first_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, &wait_owner)) {
				if (wait_owner > owner) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"Higher priority owner wait, just wait. ChOpWaitBitMask=0x%x\n",
						pAd->ChOpCtrl.ChOpWaitBitMask);
					line_up = TRUE;
				}
			}

			if (!line_up) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						 "caller:%pS. TakeCharge succeed!\n ", OS_TRACE);
				cmm_utl_set_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner);
				cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
				bCharge = TRUE;
			}
		}

		if (!bCharge && wait) {
			if (waitCnt >= CH_OP_MAX_TRY_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						 "caller:%pS. Wait ChnOpCharge timeout!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
						 OS_TRACE,
						 pAd->ChOpCtrl.ChOpDone.done,
						 pAd->ChOpCtrl.ChOpOwnerBitMask,
						 pAd->ChOpCtrl.ChOpWaitBitMask);
				cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
				NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);
				break;
			}

			cmm_utl_set_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
		}
		NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);
		if (!bCharge && wait) {
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ChOpCtrl.ChOpDone, RTMPMsecsToJiffies(waitTime))) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"caller:%pS. Wait complete timeout!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
					OS_TRACE, pAd->ChOpCtrl.ChOpDone.done,
					pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
			}

			waitCnt++;
		}
	} while (!bCharge && wait);

	if (bCharge) {
		RTMP_OS_INIT_COMPLETION(&pAd->ChOpCtrl.ChOpDone);
		pAd->ChOpCtrl.ChOpTimerFuncContex.pAd = pAd;
		pAd->ChOpCtrl.ChOpTimerFuncContex.BandIdx = BandIdx;
		RTMPInitTimer(pAd, &pAd->ChOpCtrl.ChOpTimer, GET_TIMER_FUNCTION(ChOpTimeout),
			&pAd->ChOpCtrl.ChOpTimerFuncContex, FALSE);
		RTMPSetTimer(&pAd->ChOpCtrl.ChOpTimer, CH_OP_MAX_HOLD_TIME);
		pAd->ChOpCtrl.ChOpTimerRunning = TRUE;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. owner=%d, wait=%d, result:%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, owner, wait, bCharge, pAd->ChOpCtrl.ChOpDone.done,
		pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask,
		pAd->ChOpCtrl.ChOpTimerRunning);

	return bCharge;
}

/**
* TakeChannelOpChargeByBand - Try to take charge of the channel operation.
* @pAd: pointer of the RTMP_ADAPTER
* @BandIdx: Which band should take charge
* @owner: the owner who triggered the channel operation
* @wait: whether need wait for current op done
*
* The return value is - TRUE if take charge succeeded, FALSE if failed.
*
* The function should be used with ReleaseChannelOpCharge in pairs.
*/
BOOLEAN TakeChannelOpChargeByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, UCHAR owner, BOOLEAN wait)
{
	BOOLEAN bCharge = FALSE, line_up = FALSE;
	UINT32 waitCnt = 0, waitTime = 10000; /* 10 sec */
	UCHAR wait_owner = CH_OP_OWNER_IDLE;

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Try to TakeChannelOpCharge for band:%d\n!!\n", BandIdx);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. pAd=%p, owner=%d, wait=%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, pAd, owner, wait, pAd->ChOpCtrl.ChOpDone.done,
		pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask,
		pAd->ChOpCtrl.ChOpTimerRunning);

	do {
		NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
		if (pAd->ChOpCtrl.ChOpOwnerBitMask == CH_OP_OWNER_IDLE) {
			if (cmm_utl_get_first_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, &wait_owner)) {
				if (wait_owner > owner) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"Higher priority owner wait, just wait. ChOpWaitBitMask=0x%x\n ",
						pAd->ChOpCtrl.ChOpWaitBitMask);
					line_up = TRUE;
				}
			}

			if (!line_up) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"caller:%pS. TakeCharge succeed!\n ", OS_TRACE);
				cmm_utl_set_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner);
				cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
				bCharge = TRUE;
			}
		}

		if (!bCharge && wait) {
			if (waitCnt >= CH_OP_MAX_TRY_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"caller:%pS. Wait ChnOpCharge timeout!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
					OS_TRACE, pAd->ChOpCtrl.ChOpDone.done,
					pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
				cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
				NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);
				break;
			}

			cmm_utl_set_bit(&pAd->ChOpCtrl.ChOpWaitBitMask, owner);
		}
		NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);
		if (!bCharge && wait) {
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ChOpCtrl.ChOpDone, RTMPMsecsToJiffies(waitTime))) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"caller:%pS. Wait complete timeout!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
					OS_TRACE, pAd->ChOpCtrl.ChOpDone.done,
					pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
			}

			waitCnt++;
		}
	} while (!bCharge && wait);

	if (bCharge) {
		RTMP_OS_INIT_COMPLETION(&pAd->ChOpCtrl.ChOpDone);
		pAd->ChOpCtrl.ChOpTimerFuncContex.pAd = pAd;
		pAd->ChOpCtrl.ChOpTimerFuncContex.BandIdx = BandIdx;
		RTMPInitTimer(pAd, &pAd->ChOpCtrl.ChOpTimer, GET_TIMER_FUNCTION(ChOpTimeout),
			&pAd->ChOpCtrl.ChOpTimerFuncContex, FALSE);
		RTMPSetTimer(&pAd->ChOpCtrl.ChOpTimer, CH_OP_MAX_HOLD_TIME);
		pAd->ChOpCtrl.ChOpTimerRunning = TRUE;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. owner=%d, wait=%d, result:%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, owner, wait, bCharge,
		pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpWaitBitMask, pAd->ChOpCtrl.ChOpTimerRunning);

	return bCharge;
}

/**
* ReleaseChannelOpCharge - Release the charge of channel operation.
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
* @owner: the owner who triggered the channel operation

* The function should be used with TakeChannelOpCharge in pairs.
*/
VOID ReleaseChannelOpCharge(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR owner)
{
	UINT32 cur_own_bitmask;
	BOOLEAN cleared = FALSE, Cancelled = FALSE;
	UCHAR BandIdx = DBDC_BAND0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! wdev is NULL and return!!\n");
		return;
	}
	BandIdx = HcGetBandByWdev(wdev);

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Try to Release Channel Op Charge for band:%d\n!!\n", BandIdx);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. pAd=%p, owner=%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, pAd, owner,
		pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpWaitBitMask, pAd->ChOpCtrl.ChOpTimerRunning);

	NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
	if (cmm_utl_is_bit_set(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "the ChOpCharge of owner=%d is released now!!\n ", owner);
		cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner);
		cleared = TRUE;
	}

	cur_own_bitmask = pAd->ChOpCtrl.ChOpOwnerBitMask;
	NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);

	if (cleared) {
		if (cur_own_bitmask != CH_OP_OWNER_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"caller:%pS. Wait ChnOpCharge timeout!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
				OS_TRACE,
				pAd->ChOpCtrl.ChOpDone.done,
				pAd->ChOpCtrl.ChOpOwnerBitMask,
				pAd->ChOpCtrl.ChOpWaitBitMask);
		} else {
			RTMP_OS_COMPLETE_ALL(&pAd->ChOpCtrl.ChOpDone);
			RTMPReleaseTimer(&pAd->ChOpCtrl.ChOpTimer, &Cancelled);
			pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "success. Cancelled: %d.\n ", Cancelled);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"caller:%pS. Try to release an owner_bit_not_set ChnOpCharge!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
			OS_TRACE, pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. owner=%d, result:%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, owner, cleared,
		pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpWaitBitMask, pAd->ChOpCtrl.ChOpTimerRunning);
}

/**
* ReleaseChannelOpChargeByBand - Release the charge of channel operation.
* @pAd: pointer of the RTMP_ADAPTER
* @BandIdx: Which band should release charge
* @owner: the owner who triggered the channel operation

* The function should be used with TakeChannelOpCharge in pairs.
*/
VOID ReleaseChannelOpChargeByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, UCHAR owner)
{
	UINT32 cur_own_bitmask;
	BOOLEAN cleared = FALSE, Cancelled = FALSE;

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Try to ReleaseChannelOpCharge for band:%d\n!!\n", BandIdx);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. pAd=%p, owner=%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, pAd, owner,
		pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpWaitBitMask, pAd->ChOpCtrl.ChOpTimerRunning);

	NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
	if (cmm_utl_is_bit_set(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"the ChOpCharge of owner=%d is released now!!\n ", owner);
		cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner);
		cleared = TRUE;
	}

	cur_own_bitmask = pAd->ChOpCtrl.ChOpOwnerBitMask;
	NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);

	if (cleared) {
		if (cur_own_bitmask != CH_OP_OWNER_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					 "caller:%pS. ChnOpCharge NOT IDEL after release!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
					 OS_TRACE,
					 pAd->ChOpCtrl.ChOpDone.done,
					 pAd->ChOpCtrl.ChOpOwnerBitMask,
					 pAd->ChOpCtrl.ChOpWaitBitMask);
		} else {
			RTMP_OS_COMPLETE_ALL(&pAd->ChOpCtrl.ChOpDone);
			RTMPReleaseTimer(&pAd->ChOpCtrl.ChOpTimer, &Cancelled);
			pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"success. Cancelled: %d.\n ", Cancelled);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"caller:%pS. Try to release an owner_bit_not_set ChnOpCharge!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
			OS_TRACE, pAd->ChOpCtrl.ChOpDone.done,
			pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"caller:%pS. owner=%d, result:%d, done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x, ChOpTimerRunning=%d\n ",
		OS_TRACE, owner, cleared,
		pAd->ChOpCtrl.ChOpDone.done, pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpWaitBitMask, pAd->ChOpCtrl.ChOpTimerRunning);
}


/**
* ReleaseChannelOpChargeForCurrentOwner - Release the charge of channel operation for current owner.
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
*/
VOID ReleaseChannelOpChargeForCurrentOwner(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR owner_bit = CH_OP_OWNER_IDLE;
	BOOLEAN isEmpty = FALSE;
	BOOLEAN cleared = FALSE, Cancelled = FALSE;
	UCHAR BandIdx = DBDC_BAND0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! wdev is NULL and return!!\n");
		return;
	}
	BandIdx = HcGetBandByWdev(wdev);

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"Try to Release Channel Op Charge For Current Owner for band:%x\n!!\n", BandIdx);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"pAd=%p, done=%x, ownBitMask(%p)=0x%x, ChOpWaitBitMask=0x%x\n ",
		pAd, pAd->ChOpCtrl.ChOpDone.done, &pAd->ChOpCtrl.ChOpOwnerBitMask,
		pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);

	/* Clear current owner bit to free operation flag */
	NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
	if (cmm_utl_get_first_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, &owner_bit)) {
		if ((owner_bit == CH_OP_OWNER_PARTIAL_SCAN) || (owner_bit == CH_OP_OWNER_SCAN)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"the ChOpCharge of owner=%d should not be released here!!\n", owner_bit);
			NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);
			return;
		}
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "the ChOpCharge of owner=%d is released now!!\n ", owner_bit);
		cmm_utl_clear_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, owner_bit);
		cleared = TRUE;
	}

	isEmpty = pAd->ChOpCtrl.ChOpOwnerBitMask ? FALSE : TRUE;
	NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);

	if (!isEmpty)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "Own bitmask(0x%x) not empty after release!!\n ",
				 pAd->ChOpCtrl.ChOpOwnerBitMask);
	if (cleared) {
		RTMP_OS_COMPLETE_ALL(&pAd->ChOpCtrl.ChOpDone);
		RTMPReleaseTimer(&pAd->ChOpCtrl.ChOpTimer, &Cancelled);
		pAd->ChOpCtrl.ChOpTimerRunning = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"success. Cancelled: %d.\n", Cancelled);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"caller:%pS. Try to release an empty owner_bit_mask!! done=%x, ownBitMask=0x%x, ChOpWaitBitMask=0x%x\n ",
			OS_TRACE, pAd->ChOpCtrl.ChOpDone.done,
			pAd->ChOpCtrl.ChOpOwnerBitMask, pAd->ChOpCtrl.ChOpWaitBitMask);
	}
}

UCHAR GetCurrentChannelOpOwner(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR owner_bit = CH_OP_OWNER_IDLE;
	UCHAR BandIdx = DBDC_BAND0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! wdev is NULL and return!!\n");
		return owner_bit;
	}
	BandIdx = HcGetBandByWdev(wdev);

	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"error! invalid BandIdx and return!!\n");
		return owner_bit;
	}

	NdisAcquireSpinLock(&pAd->ChOpCtrl.ChOpLock);
	if (cmm_utl_get_first_bit(&pAd->ChOpCtrl.ChOpOwnerBitMask, &owner_bit)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"success. CurrentChannelOpOwner: %d(caller: %pS).\n ", owner_bit, OS_TRACE);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"No owner taking ChannelOpCharge now.\n ");
	NdisReleaseSpinLock(&pAd->ChOpCtrl.ChOpLock);

	return owner_bit;
}

INT StaRecBFUpdateAdjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN fgStatus = FALSE;
	STA_REC_CFG_T StaCfg;

	os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
	StaCfg.MuarIdx = 0;
	StaCfg.ConnectionState = TRUE;
	StaCfg.ConnectionType = 0;
	StaCfg.u8EnableFeature = (1 << STA_REC_BF);
	StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	StaCfg.u2WlanIdx = pEntry->wcid;
	StaCfg.pEntry = pEntry;

#ifdef WIFI_UNIFIED_COMMAND
	if (UniCmdStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE)
		fgStatus = TRUE;
#else
	if (CmdExtStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE)
		fgStatus = TRUE;
#endif
	return fgStatus;
}

UINT8 get_peer_max_bw_cap(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	UINT8 peer_max_bw = BW_20;
#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(wdev->PhyMode) && IS_HT_STA(pEntry))
		peer_max_bw = pEntry->cap.ch_bw.ht_support_ch_width_set;
#endif

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode) && IS_VHT_STA(pEntry))
		peer_max_bw = vht_bw_2_rf_bw(pEntry->cap.ch_bw.vht_support_ch_width_set);
#endif

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode) && IS_HE_STA(pEntry->cap.modes)) {
		peer_max_bw = he_bw_2_rf_bw(peer_max_bw_cap(pEntry->cap.ch_bw.he_ch_width));
#ifdef DOT11_EHT_BE
		if (WMODE_CAP_6G(wdev->PhyMode) && IS_EHT_STA(pEntry->cap.modes) && (peer_max_bw == BW_160) && (pEntry->cap.eht_phy_cap && EHT_320M_6G))
			peer_max_bw = BW_320;
#endif
	}
#endif

	return peer_max_bw;
}

VOID UpdatedRaBfInfoBwByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT_8 BW)
{
	MAC_TABLE_ENTRY *pEntry;
	UINT_32 i;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "wdev_idx:%d, BW:%d.\n",
			 wdev->wdev_idx, BW);
	/* begein from 1 due to index 0 is a dummy entry */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		UINT8 peer_max_bw = BW_20;
		UINT8 operating_bw = BW_20;

		pEntry = entry_get(pAd, i);

		/*check all MacTable entries */
		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		if (pEntry->wdev != wdev)
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;


		peer_max_bw = get_peer_max_bw_cap(wdev, pEntry);
		operating_bw = (peer_max_bw > BW) ? BW : peer_max_bw;

		if (operating_bw == pEntry->RaEntry.MaxPhyCfg.BW)
			continue;

		pEntry->MaxHTPhyMode.field.BW = operating_bw;
		pEntry->RaEntry.MaxPhyCfg.BW = operating_bw;
		pEntry->operating_mode.ch_width = operating_bw;
		pEntry->RaEntry.vhtOpModeChWidth = operating_bw;
		pEntry->rStaRecBf.ucCBW = operating_bw;
		pEntry->rStaRecBf.uciBfDBW = operating_bw;
		StaRecBFUpdateAdjust(pAd, pEntry);
		WifiSysRaInit(pAd, pEntry);
	}
}
#ifdef CONFIG_EAP_QUICK_CHANNEL_SWITCH_WITHOUT_CSA
/*
    ==========================================================================
    Description:
	this function can quickly switch channel, but there are something to note:
	1. this function switch channel without csa and deauth.
	2. this function should be called on both ap and apcli at the same time to
		ensure the connection.
	3. the upper application should control when this functions are called.
	4. this function can not support switching to radar channels.
	5. this function can not switch channel that the BW is 160M in 5G band.
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_QuickChSwitch_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UCHAR band_idx;
	UCHAR Channel = (UCHAR) os_str_tol(arg, 0, 10);
	INT32 success = FALSE;
	UINT32 dbg_lvl_ori = DebugLevel;
	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__, pObj->ioctl_if_type, if_idx);
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	if (!IsValidChannel(pAd, Channel, wdev)) {
		MTWF_PRINT("%s: This channel is out of channel list\n", __func__);
		return FALSE;
	}

	if (wdev->channel == Channel) {
		MTWF_PRINT("%s: The channel %d not changed.\n ", __func__, Channel);
		return FALSE;
	}
	/* not support change to dfs channel*/
	if (RadarChannelCheck(pAd, Channel)) {
		MTWF_PRINT("%s: Switch to radar channel is not supported!!\n", __func__);
		return FALSE;
	}

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_PRINT("%s: TakeChannelOpCharge fail for SET channel!!\n", __func__);
		return FALSE;
	}

	set_dbg_lvl_all(DBG_LVL_ERROR);
	/* apply channel directly*/
	wdev->channel = Channel;
	wlan_operate_set_prim_ch(wdev, Channel);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"CtrlChannel(%d), CentralChannel(%d)\n",
		Channel, wlan_operate_get_cen_ch_1(wdev));

	set_dbg_lvl_all(dbg_lvl_ori);

	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success;
}
#endif

/*
    ==========================================================================
    Description:
	Set Channel
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UCHAR Channel = (UCHAR) os_str_tol(arg, 0, 10);
	INT32 success = FALSE;
	UINT_8 ch_idx;
#ifdef CONFIG_AP_SUPPORT
	INT ret = 0;
#endif /* CONFIG_AP_SUPPORT */
#ifdef TR181_SUPPORT
	UCHAR old_channel;
	struct hdev_ctrl *ctrl;
#endif /*TR181_SUPPORT*/
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	UCHAR OriChannel;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__, pObj->ioctl_if_type, if_idx);
		return FALSE;
	}
#ifdef TR181_SUPPORT
	old_channel = wdev->channel;
	ctrl = pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	OriChannel = wdev->channel;
#endif

#ifdef WIFI_MD_COEX_SUPPORT
	if (!IsChannelSafe(pAd, Channel)) {
		if (IsPwrChannelSafe(pAd, Channel)) {
			MTWF_PRINT("caller:%pS. The channel %d is power backoff channel\n ",
					 OS_TRACE, Channel);
		} else {
			MTWF_PRINT("caller:%pS. The channel %d is in unsafe channel list!!\n ",
					 OS_TRACE, Channel);
			return FALSE;
		}
	}
#endif
	if (!IsValidChannel(pAd, Channel, wdev)) {
		MTWF_PRINT("%s: This channel is out of channel list\n", __func__);
		return FALSE;
	}
	if (wdev->channel == Channel) {
		MTWF_PRINT("%s: The channel %d not changed.\n ", __func__, Channel);
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	if (pAd->CommonCfg.bIEEE80211H == TRUE) {
		if (CheckNonOccupancyChannel(pAd, wdev, Channel) == FALSE) {
			MTWF_PRINT("%s: Can not update channel(%d), Its a NonOccupancy channel!\n",
				__func__, Channel);
			return FALSE;
		}
		/*If set ch36~ch48 BW160, Need to check ch52~ch64 NOP*/
		if (pAd->CommonCfg.DfsParameter.band_bw == BW_160 && IS_CH_BETWEEN(Channel, 36, 48)) {
			for (ch_idx = 52; ch_idx <= 64;) {
				if (CheckNonOccupancyChannel(pAd, wdev, ch_idx) == FALSE) {
					MTWF_PRINT("%s: Can't update channel(%d),",
					__func__, Channel);
					MTWF_PRINT("Its a NonOccupancy channel!\n");
					return FALSE;
				}
				ch_idx = ch_idx + 4;
			}
		}
	}
#endif

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_PRINT("%s: TakeChannelOpCharge fail for SET channel!!\n", __func__);
		return FALSE;
	}

	MTWF_PRINT("Channel(%d), Cert(%d), Quick(%d)\n",
			  Channel, pAd->CommonCfg.wifi_cert, wdev->quick_ch_change);
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	pAd->CommonCfg.DfsParameter.ByPassCac = DfsZwBypassCac(pAd, wdev, OriChannel, Channel);
	if (!pAd->CommonCfg.DfsParameter.ByPassCac
	&& (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE)) {
		DfsDedicatedExamineSetNewCh(pAd, wdev, Channel);
		DedicatedZeroWaitStop(pAd, TRUE);
	}
#endif

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE)
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_SET_CHANNEL;
#endif

#ifdef TR181_SUPPORT
	success = rtmp_set_channel(pAd, wdev, Channel);

	if (success && (old_channel != Channel)) {
		if (ctrl) {
			ctrl->rdev.pRadioCtrl->ManualChannelChangeCount++;
			ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
			MTWF_PRINT("success = %d	Manual:%d Total:%d\n",
				success, ctrl->rdev.pRadioCtrl->ManualChannelChangeCount,
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount);
		}
	}
#else

#ifdef DFS_CAC_R2
	if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
		success = rtmp_set_channel(pAd, wdev, Channel);
		if (success == FALSE)
			wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, FALSE);
		/*return success; after set channel finished,then return iwpriv.*/
	} else
#endif
	success = rtmp_set_channel(pAd, wdev, Channel);
	if (success == FALSE)
		return success;
#endif
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.iwpriv_event_flag = TRUE;
	if (pAd->ApCfg.set_ch_async_flag == TRUE) {
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
		if (ret)
			MTWF_PRINT("%s: wait channel setting success.\n", __func__);
		else {
			MTWF_PRINT("%s: wait channel setting timeout.\n", __func__);
			pAd->ApCfg.set_ch_async_flag = FALSE;
		}
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;
#endif /* CONFIG_AP_SUPPORT */
	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success;
}

/*
    ==========================================================================
    Description:
	Enable or Disable 2G Channel Switch Anouncement
    Return:
	TRUE
    ==========================================================================
*/
INT	Set_SeamlessCSA_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR temp;

	temp = (UCHAR) simple_strtol(arg, 0, 10);

	if (temp != 0)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else
		pAd->CommonCfg.bIEEE80211H = FALSE;

	MTWF_PRINT("%s::(%d)\n", __func__, pAd->CommonCfg.bIEEE80211H);

	return TRUE;
}

INT	Set_MaxChSwitchTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG temp;

	if (kstrtol(arg, 10, &temp) == 0)
		pAd->Dot11_H.MaxChannelSwitchTime = (UINT16)temp;
	else
		pAd->Dot11_H.MaxChannelSwitchTime = 0;

	MTWF_PRINT("%s:: set MaxChSwitchTime = %d\n", __func__, pAd->Dot11_H.MaxChannelSwitchTime);

	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_csa_2g(RTMP_ADAPTER *pAd, UCHAR csa_2g)
{
	if (csa_2g == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (csa_2g == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return -EINVAL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"set CSASupportFor2G=%d\n", pAd->CommonCfg.bIEEE80211H);
	return 0;
}
#endif
#ifdef CONVERTER_MODE_SWITCH_SUPPORT


/*
*    ==========================================================================
*    Description:
*	Enable/disable AP Beacons in Vendor10 Converter mode
*    Return:
*	TRUE if all parameters are OK, FALSE otherwise
*    ==========================================================================
*/
INT Set_V10ConverterMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR APBeaconEn  = (UCHAR) os_str_tol(arg, 0, 10);
	INT32 success = TRUE;	/*FALSE = 0*/
	UCHAR idx = 0;

	if (APBeaconEn) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
			pMbss->APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;
			if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
				pMbss->wdev.bAllowBeaconing = TRUE;
				if (wdev_do_linkup(&pMbss->wdev, NULL) != TRUE)
					MTWF_PRINT("%s: link up fail!!\n", __func__);
			}
#ifdef VOW_SUPPORT
			vow_mbss_init(pAd, &pMbss->wdev);
#endif
		}
	} else {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];

			if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
				pMbss->wdev.bAllowBeaconing = FALSE;
				if (wdev_do_linkdown(&pMbss->wdev) != TRUE)
					MTWF_PRINT("%s: link down fail!!\n", __func__);
			}
			pMbss->APStartPseduState = AP_STATE_START_AFTER_APCLI_CONNECTION;
		}
	}
	return success;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_Set_V10ConverterMode(RTMP_ADAPTER *pAd, UCHAR APBeaconEn)
{
	INT32 success = TRUE;	/*FALSE = 0*/
	UCHAR idx = 0;

	if (APBeaconEn) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];

			pMbss->APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;

			if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
				pMbss->wdev.bAllowBeaconing = TRUE;
				if (wdev_do_linkup(&pMbss->wdev, NULL) != TRUE)
					MTWF_PRINT("%s: link up fail!!\n", __func__);
			}
#ifdef VOW_SUPPORT
			vow_mbss_init(pAd, &pMbss->wdev);
#endif
		}
	} else {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];

			if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
				pMbss->wdev.bAllowBeaconing = FALSE;
				if (wdev_do_linkdown(&pMbss->wdev) != TRUE)
					MTWF_PRINT("%s: link down fail!!\n", __func__);
			}
			pMbss->APStartPseduState = AP_STATE_START_AFTER_APCLI_CONNECTION;
		}
	}
	return success;
}
#endif /* RT_CFG80211_SUPPORT */
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/

/*
*    ==========================================================================
*    Description:
*	Enable/disable quick Channel Switch feature
*    Return:
*	TRUE if all parameters are OK, FALSE otherwise
*    ==========================================================================
*/
INT Set_Quick_Channel_Switch_En_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Bandidx = 0, QuickChannelEn = 0;
	UCHAR i = 0;
	INT32 success = TRUE;	/*FALSE = 0*/
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	struct wifi_dev *tdev;

	if (!wdev)
		return FALSE;
	Bandidx = HcGetBandByWdev(wdev);
	QuickChannelEn = (UCHAR) os_str_tol(arg, 0, 10);

	if (QuickChannelEn > QUICK_CH_SWICH_ENABLE)
		QuickChannelEn = QUICK_CH_SWICH_ENABLE;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && HcIsRadioAcq(tdev) && (Bandidx == HcGetBandByWdev(tdev)))
				tdev->quick_ch_change = QuickChannelEn;
	}

	MTWF_PRINT("%s(): Bandidx(%d) Quick Channel Switch Enable = %d\n", __func__, Bandidx, QuickChannelEn);
	return success;
}


#ifdef CONFIG_AP_SUPPORT

VOID ap_update_rf_ch_for_mbss(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct freq_oper *OperCh)
{

	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct _BSS_INFO_ARGUMENT_T bss;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"wdev_idx:%d, ch:%d, bw:%d, rx_stream:%d.\n",
		 wdev->wdev_idx, OperCh->prim_ch, OperCh->bw, OperCh->rx_stream);
	/* update freq_oper into bss_info */
	NdisCopyMemory(&bss_info->chan_oper, OperCh, sizeof(struct freq_oper));

	/* copy to tmp bss info entry and update down to fw */
	NdisCopyMemory(&bss, bss_info, sizeof(struct _BSS_INFO_ARGUMENT_T));
	bss.u8BssInfoFeature = BSS_INFO_RF_CH_FEATURE;
	if (arch_ops->archSetBssid)
		arch_ops->archSetBssid(ad, &bss);
	else {
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"there is no lower layer implementation.\n");
	}
}

void ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR i = 0;
	struct wifi_dev *tdev;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "(caller:%pS), wdev_idx:%d.\n",
			 OS_TRACE, wdev->wdev_idx);

	/* first loop for disconnect sta */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev
			&& HcIsRadioAcq(tdev)
			&& (band_idx == HcGetBandByWdev(tdev))) {
			if (tdev->wdev_type == WDEV_TYPE_AP) {
				/* AP send deauth to disconnect STA when
					1. need cac; 2. CSA disabled; */
#ifdef CONFIG_MAP_SUPPORT
				pAd->disconnect_all_sta = 0;
#endif
				if (NeedDoDfsCac(pAd, tdev)
					|| !pAd->CommonCfg.bIEEE80211H)
#ifdef CONFIG_MAP_SUPPORT
				{
					pAd->disconnect_all_sta = 1;
#endif
					MacTableResetNonMapWdev(pAd, tdev);
#ifdef ZERO_PKT_LOSS_SUPPORT
					/*10ms delay to send deauth
					before switch channel*/
				if (pAd->Zero_Loss_Enable)
					RtmpusecDelay(10000);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
#ifdef CONFIG_MAP_SUPPORT
					pAd->disconnect_all_sta = 0;
				}
#endif
				if (wlan_config_get_ch_band(tdev) == CMD_CH_BAND_5G) {
#ifdef MT_DFS_SUPPORT /* Jelly20150217 */
					WrapDfsRadarDetectStop(pAd);
					/* Zero wait hand off recovery for CAC period + interface down case */
					DfsZeroHandOffRecovery(pAd, tdev);
#endif
				}
			} else if (tdev->wdev_type == WDEV_TYPE_STA) {
#ifdef CONFIG_STA_SUPPORT
				/*delete rootap entry to avoid false connections
				1):apcli actively set channel,driver will send disconnect;
				2):apcli receive CSA and radar channel, and mesh enable with CAC require,driver will send disconnect;
				3):other than that, apcli will not send disconnect */
		if ((GetCurrentChannelOpOwner(pAd, wdev) != CH_OP_OWNER_PEER_CSA)
#ifdef DFS_ADJ_BW_ZERO_WAIT
			&& ((IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) == TRUE)
				&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == DFS_BW160_TX160RX160 && pAd->PeerApccfs1 != 0)
#endif
#ifdef CONFIG_MAP_SUPPORT
			&& (!IS_MAP_ENABLE(pAd) ||
			 (IS_MAP_ENABLE(pAd) && (RadarChannelCheck(pAd, wdev->channel) && (wdev->cac_not_required == FALSE))))
#endif
#ifdef MT_DFS_SUPPORT
			&& (!(pAd->CommonCfg.DfsParameter.OrigCh == tdev->channel))
#endif
				) {
					UCHAR ifIdex;
					struct wifi_dev *pdev;
					PSTA_ADMIN_CONFIG pApCliEntry;
					MAC_TABLE_ENTRY *pRootApMacEntry;

					for (ifIdex = 0; ifIdex < MAX_APCLI_NUM; ifIdex++) {
						pApCliEntry = &pAd->StaCfg[ifIdex];
						if (pApCliEntry) {
							pdev = &pApCliEntry->wdev;
							if (band_idx != HcGetBandByWdev(pdev))
								continue;
							if (pApCliEntry->ApcliInfStat.Valid == FALSE)
								continue;
							pRootApMacEntry = GetAssociatedAPByWdev(pAd, pdev);
							if ((pRootApMacEntry != NULL) && IS_ENTRY_MLO(pRootApMacEntry))
								continue;
							MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN,
								DBG_LVL_WARN,
								"%s Send DeAuth to "MACSTR"\n",
								(char *)pApCliEntry->wdev.if_dev->name, MAC2STR(pApCliEntry->Bssid));
							pApCliEntry->ApcliInfStat.Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
							cntl_disconnect_request(&pApCliEntry->wdev, CNTL_DISASSOC,
									pApCliEntry->Bssid, REASON_DISASSOC_STA_LEAVING);
#ifdef ZERO_PKT_LOSS_SUPPORT
							/*10ms delay to send deauth
							before switch channel*/
				if (pAd->Zero_Loss_Enable)
					RtmpusecDelay(10000);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
						}
					}
				}
#endif /* CONFIG_STA_SUPPORT */
			}
		}
	}

#ifdef ZERO_PKT_LOSS_SUPPORT
	/*Moved inside condition in above code, to avoid 10ms delay, if no disconnection */
	if (!(pAd->Zero_Loss_Enable))
#endif /*ZERO_PKT_LOSS_SUPPORT*/
		/*10ms delay to ensure that there is enough time
		to send deauth before switch channel*/
		RtmpusecDelay(10000);
#ifdef ZERO_PKT_LOSS_SUPPORT
	pAd->chan_switch_time[5] = jiffies_to_msecs(jiffies);
#endif /*ZERO_PKT_LOSS_SUPPORT*/

	/* second loop for switch channel and Enable Beacon */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pStaCfg = NULL;
		tdev = pAd->wdev_list[i];
		if (tdev
			&& HcIsRadioAcq(tdev)
			&& (band_idx == HcGetBandByWdev(tdev))) {
			PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"Wlan_operate_init and switch channel for : %s\n", (char *)tdev->if_dev->name);

			if (pAd->CommonCfg.bIEEE80211H) {
				pBcnCheckInfo->BcnInitedRnd = pAd->Mlme.PeriodicRound;
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
					 "\t BcnInitedRnd = %ld\n", pBcnCheckInfo->BcnInitedRnd);
			}

			/* update new channel setting to hw */
			wlan_operate_init(tdev);

			if (pAd->CommonCfg.bIEEE80211H) {
				UpdateBeaconHandler(pAd, tdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
#ifdef DOT11_EHT_BE
				remove_mlo_csa(pAd, tdev);
#endif
				/* APCLI: Notify connected AP that BW may change due to dfs*/
				if ((tdev->wdev_type == WDEV_TYPE_STA) && WMODE_CAP_AC(tdev->PhyMode)) {
					pStaCfg = GetStaCfgByWdev(pAd, tdev);
					if (pStaCfg != NULL && INFRA_ON(pStaCfg))
						StaSendVhtOmnAction(pAd, tdev);
				}
			}

			/* If the cfg.bw changed, need to de-auth and force re-negotiate */
			if (pAd->Dot11_H.disconn_after_ch_switch == TRUE) {
				if (tdev->wdev_type == WDEV_TYPE_AP)
					ap_send_broadcast_deauth(pAd, tdev);
				else if (tdev->wdev_type == WDEV_TYPE_STA) {
					pStaCfg = GetStaCfgByWdev(pAd, tdev);
					if (!pStaCfg || !INFRA_ON(pStaCfg))
						continue;
					cntl_disconnect_request(&pStaCfg->wdev,
										CNTL_DISASSOC,
										pStaCfg->Bssid,
										REASON_DISASSOC_STA_LEAVING);
				}
				pAd->Dot11_H.disconn_after_ch_switch = FALSE;
			}
		}
	}

#ifdef DOT11_EHT_BE
	/* consider channel change case */
	if (WMODE_CAP_BE(wdev->PhyMode)) {
		if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_CH_CHANGED) == TRUE)
			UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_CH_CHANGED, TRUE);
	} else {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tdev = pAd->wdev_list[i];
			if (tdev
				&& HcIsRadioAcq(tdev)
				&& (band_idx == HcGetBandByWdev(tdev))
				&& WMODE_CAP_BE(tdev->PhyMode)) {
				if (bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_CH_CHANGED) == TRUE) {
					UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_CH_CHANGED, TRUE);
					break;
				}
			}
		}
	}
#endif

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd))
		wapp_send_ch_change_rsp_map_r3(pAd, wdev, wdev->channel);
#endif
#endif
	/* Trigger overlapping scan for 2.4G */
	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
		UINT8 idx;
		BSS_STRUCT *pCurMbss = NULL;

		/* Reset ht coex result */
		pAd->CommonCfg.BssCoexScanLastResult.LastScanTime = 0;
		pAd->CommonCfg.BssCoexScanLastResult.bNeedFallBack = FALSE;
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			if (wdev->channel <  14) {
				/* check MBSS work on the same RF(channel) */
				if (pCurMbss->wdev.channel == wdev->channel) {
					ap_over_lapping_scan(pAd, pCurMbss);
					break;
				}
			}
		}
	}
#ifdef MGMT_TXPWR_CTRL
	/* Update tx power for each channel change */
	wdev->bPwrCtrlEn = FALSE;
	wdev->TxPwrDelta = 0;
	wdev->mgmt_txd_txpwr_offset = 0;
	/* Get EPA info by Tx Power info cmd*/
	pAd->ApCfg.MgmtTxPwr = 0;
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, HcGetBandByWdev(wdev));
	if (wdev->MgmtTxPwr) {
	/* wait until TX Pwr event rx*/
		RtmpusecDelay(50);
		update_mgmt_frame_power(pAd, wdev);
	}
#endif
}
#endif


/*
*	==========================================================================
*	Description:
*		This routine reset the entire MAC table. All packets pending in
*		the power-saving queues are freed here.
*	==========================================================================
*/
VOID MacTableResetNonMapWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "wdev_idx:%d.\n",
			 wdev->wdev_idx);

	/* TODO:Carter, check why start from 1 */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = entry_get(pAd, i);

		if (pMacEntry->wdev != wdev)
			continue;
#ifdef CONFIG_MAP_SUPPORT
		if ((IS_MAP_TURNKEY_ENABLE(pAd)) &&
			((pMacEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) &&
			(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS))))
			continue;
#endif
		/* MLO STA should not be deleteted MAC entry*/
		if ((IS_ENTRY_MLO(pMacEntry)) && pAd->CommonCfg.bIEEE80211H)
			continue;

		if (IS_ENTRY_CLIENT(pMacEntry)) {
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*	send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
							" MlmeAllocateMemory fail  ..\n");
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"Send DeAuth (Reason=%d) to "MACSTR"\n",
							 Reason, MAC2STR(pMacEntry->Addr));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 wdev->if_addr,
									 wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef WSC_AP_SUPPORT
		{
			BOOLEAN Cancelled;

			RTMPCancelTimer(&mbss->wdev.WscControl.EapolTimer, &Cancelled);
			mbss->wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(mbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			mbss->wdev.WscControl.EapMsgRunning = FALSE;
		}
#endif /* WSC_AP_SUPPORT */
		mbss->StaCount = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
}

#ifdef CONFIG_MAP_SUPPORT

/*
*    ==========================================================================
*    Description:
*	Set Channel quickly without AP start/stop
*    Return:
*	TRUE if all parameters are OK, FALSE otherwise
*    ==========================================================================
*/
INT Set_Map_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	INT32 success = FALSE;	/*FALSE = 0*/
	UINT32 i;
	INT ret = 0;
#ifdef TR181_SUPPORT
	UCHAR old_channel;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
#endif
	UCHAR Channel = 0;
#ifdef MAP_R2
	UCHAR cac_req;
	UCHAR dev_role;
	RTMP_STRING *token;
#endif

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"wdev == NULL! if_type %d, if_idx = %d\n",
				 pObj->ioctl_if_type,
				 if_idx);
		return FALSE;
	}

	for (i = 0; i < MAX_BEACON_NUM; i++) {
		pAd->ApCfg.MBSSID[i].wdev.cac_not_required = FALSE;
	}
#ifdef MAP_R2
	token = rstrtok(arg, ":");
	if (token)
		Channel = os_str_tol(token, 0, 10);
	token = rstrtok(NULL, ":");
	if (token) {
		cac_req = os_str_tol(token, 0, 10);
#ifdef MT_DFS_SUPPORT
		if (cac_req == 0 && pAd->CommonCfg.DfsParameter.bDfsEnable) {
			for (i = 0; i < MAX_BEACON_NUM; i++) {
				if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
					pAd->ApCfg.MBSSID[i].wdev.cac_not_required = TRUE;
			}
		}
#endif
	}
	token = rstrtok(NULL, ":");
	if (token) {
		dev_role = os_str_tol(token, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"dev_role %d\n", dev_role);
		for (i = 0; i < MAX_BEACON_NUM; i++) {
			if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
				pAd->ApCfg.MBSSID[i].wdev.dev_role = dev_role;
		}
	}
#else
	Channel = (UCHAR) os_str_tol(arg, 0, 10);
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "\n");

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsDedicatedExamineSetNewCh(pAd, wdev, Channel);
	DedicatedZeroWaitStop(pAd, TRUE);
#endif

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"TakeChannelOpCharge fail for SET channel!!\n");
		return FALSE;
	}
	pAd->ApCfg.iwpriv_event_flag = TRUE;

#ifdef TR181_SUPPORT
	success = rtmp_set_channel(pAd, wdev, Channel);

	old_channel = wdev->channel;
	if (success && (old_channel != Channel)) {
		if (ctrl) {
			ctrl->rdev.pRadioCtrl->ManualChannelChangeCount++;
			ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"success = %d	Manual:%d Total:%d\n",
				success, ctrl->rdev.pRadioCtrl->ManualChannelChangeCount,
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount);
		}
	}
#else
	success = rtmp_set_channel(pAd, wdev, Channel);
#endif

	if (pAd->ApCfg.set_ch_async_flag == TRUE) {
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"wait channel setting success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"wait channel setting timeout.\n");
			pAd->ApCfg.set_ch_async_flag = FALSE;
		}
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;

	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success;

}

#ifdef MAP_TS_TRAFFIC_SUPPORT
INT Set_MapTS_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);

	if (pAd->bTSEnable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_PRINT("%s MAP TS is already %s\n", __func__, enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable)
		pAd->bTSEnable = FALSE;
	else
		pAd->bTSEnable = TRUE;

	MTWF_PRINT("%s: MAP TS is %s\n", __func__, pAd->bTSEnable?"enabled":"disabled");

	return TRUE;
}
#endif

#ifdef MAP_R2
INT Set_Map_Bh_Primary_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 vid = (UINT16) os_str_tol(arg, 0, 10);

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__,
			pObj->ioctl_if_type,
			if_idx);
		return FALSE;
	}

	wdev->MAPCfg.primary_vid = vid;
	MTWF_PRINT("%s: %s default vid=%d\n", __func__, wdev->if_dev->name, vid);

	return TRUE;
}

INT Set_Map_Bh_Primary_Pcp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR pcp = (UCHAR) os_str_tol(arg, 0, 10);

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__, pObj->ioctl_if_type, if_idx);
		return FALSE;
	}

	wdev->MAPCfg.primary_pcp = pcp;
	MTWF_PRINT("%s default pcp=%d\n", wdev->if_dev->name, pcp);

	return TRUE;
}

INT Set_Map_Bh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 vid;
	UINT32 index = 0, offset = 0;
	RTMP_STRING *p = NULL;

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__,
			pObj->ioctl_if_type,
			if_idx);
		return FALSE;
	}

	RTMPZeroMemory(wdev->MAPCfg.vids, sizeof(wdev->MAPCfg.vids));
	wdev->MAPCfg.vid_num = 0;

	while (1) {
		p = strsep(&arg, ",");
		if (!p || *p == '\0')
			break;
		vid = (UINT16) os_str_tol(p, 0, 10);
		if (vid >= INVALID_VLAN_ID || vid == 0) {
			MTWF_PRINT("%s: %s invalid vid=%d\n", __func__, wdev->if_dev->name, vid);
			continue;
		}

		index = vid / (sizeof(UINT32) * 8);
		offset = vid % (sizeof(UINT32) * 8);

		if (!(wdev->MAPCfg.vids[index] & BIT(offset))) {
			wdev->MAPCfg.vids[index] |= BIT(offset);
			wdev->MAPCfg.vid_num++;
		}
		MTWF_PRINT("%s: %s bh vid vlan id=%d\n", __func__, wdev->if_dev->name, vid);
	}

	MTWF_PRINT("%s: %s total vid_num=%d\n", __func__, wdev->if_dev->name, wdev->MAPCfg.vid_num);

	return TRUE;
}

INT Set_Map_Fh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 vid = (UINT16) os_str_tol(arg, 0, 10);

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
				__func__, pObj->ioctl_if_type, if_idx);
		return FALSE;
	}

	wdev->MAPCfg.fh_vid = vid;
	MTWF_PRINT("%s: %s fh vid=%d\n", __func__, wdev->if_dev->name, vid);

	return TRUE;
}

INT Set_Map_Transparent_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 vid = 0;
	UINT32 index = 0, offset = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	RTMP_STRING *p = NULL;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev == NULL! if_type %d, if_idx = %d\n",
			__func__, pObj->ioctl_if_type, if_idx);
		return FALSE;
	}

	RTMPZeroMemory(wdev->MAPCfg.bitmap_trans_vlan, sizeof(wdev->MAPCfg.bitmap_trans_vlan));

	while (1) {
		p = strsep(&arg, ",");
		if (!p || *p == '\0')
			break;
		vid = (UINT16) os_str_tol(p, 0, 10);
		if (vid >= INVALID_VLAN_ID || vid == 0) {
			MTWF_PRINT("%s: %s invalid vid=%d\n", __func__, wdev->if_dev->name, vid);
			continue;
		}

		index = vid / (sizeof(UINT32) * 8);
		offset = vid % (sizeof(UINT32) * 8);

		wdev->MAPCfg.bitmap_trans_vlan[index] |= BIT(offset);
		MTWF_PRINT("%s: %s transparent vlan id=%d\n", __func__, wdev->if_dev->name, vid);
	}

	return TRUE;
}
#endif
#endif

BOOLEAN mt_validate_dfs_channel_for_cac(RTMP_ADAPTER *pAdapter, struct wifi_dev *wdev)
{
	BSS_ENTRY *bss;
	UINT i = 0;
	PBSS_TABLE ScanTab = NULL;
	if ((wdev->wdev_type != WDEV_TYPE_STA) &&
	(wdev->wdev_type != WDEV_TYPE_REPEATER))
		return TRUE;
	ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);
	if (ScanTab->BssNr == 0)
		return TRUE;
	for (i = 0; i < ScanTab->BssNr; i++) {
		bss = &ScanTab->BssEntry[i];
		if (bss->Channel == wdev->channel)
			return FALSE;
	}
	return TRUE;
}

void ap_send_csa_action_frame(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	PUCHAR pch_sw_frm_buf = NULL;
	NDIS_STATUS NStatus = NDIS_STATUS_FAILURE;
	ULONG chnl_switch_frm_len;
	HEADER_802_11 ActHdr;
	struct DOT11_H *pDot11h = NULL;
	UINT16 ie_size = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, BROADCAST_ADDR, wdev->if_addr, wdev->bssid);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pch_sw_frm_buf);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "allocate memory failed\n");
		return;
	}
	NdisMoveMemory(pch_sw_frm_buf, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	chnl_switch_frm_len = sizeof(HEADER_802_11);
	InsertActField(pAd, (pch_sw_frm_buf + chnl_switch_frm_len), &chnl_switch_frm_len, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	pDot11h = wdev->pDot11_H;

	if (!pDot11h) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pDot11h is NULL!!!\n");
		MlmeFreeMemory(pch_sw_frm_buf);
		return;
	}

	build_channel_switch_relatd_ie(pAd, wdev, pch_sw_frm_buf + chnl_switch_frm_len, &ie_size);
	chnl_switch_frm_len += ie_size;

#ifdef DOT11W_PMF_SUPPORT
	/* Add BIP MIC if PMF is enabled */
	if (!PMF_AddBIPMIC(pAd, wdev, (pch_sw_frm_buf+chnl_switch_frm_len), &chnl_switch_frm_len)) {
		MlmeFreeMemory(pch_sw_frm_buf);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "PMF_AddBIPMIC failed!\n");
		return;
	}
#endif /* DOT11W_PMF_SUPPORT */
	if (MiniportMMRequest(pAd, QID_AC_BK, pch_sw_frm_buf, chnl_switch_frm_len, NULL))
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Tx_1 pkt failed!\n");

	MlmeFreeMemory(pch_sw_frm_buf);
}

static BOOLEAN set_new_chn_bw_for_csa(struct wifi_dev *wdev)
{
	struct freq_cfg fcfg;
	struct freq_oper oper_dev;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	if (!pDot11h)
		return FALSE;

	os_zero_mem(&fcfg, sizeof(fcfg));
	os_zero_mem(&oper_dev, sizeof(oper_dev));

	phy_freq_get_cfg(wdev, &fcfg);
	if (!phy_freq_adjust(wdev, &fcfg, &oper_dev)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "phy_freq_adjust failed!");
		return FALSE;
	}

	pDot11h->csa_chn_info.new_bw = oper_dev.bw;
	pDot11h->csa_chn_info.new_ch = wdev->channel;
#ifdef DOT11_EHT_BE
	if (oper_dev.bw == BW_320)
		pDot11h->csa_chn_info.new_cench1 = oper_dev.eht_cen_ch;
	else
#endif
		pDot11h->csa_chn_info.new_cench1 = oper_dev.cen_ch_1;

	pDot11h->csa_chn_info.new_cench2 = oper_dev.cen_ch_2;

	return TRUE;
}


BOOLEAN DoActionBeforeCsa(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *tdev;
	UCHAR i = 0;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	if (!pDot11h)
		return FALSE;

	if (!set_new_chn_bw_for_csa(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"set_new_chn_bw_for_csa failed!\n");
		return FALSE;
	}

	/* loop all wdev to update csa in beacon */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];

		if (tdev == NULL || !WDEV_WITH_BCN_ABILITY(tdev) || !tdev->pDot11_H)
			continue;

		ap_send_csa_action_frame(pAd, tdev);
	}

	return TRUE;
}

BOOLEAN	perform_channel_change(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
#ifdef OFFCHANNEL_SCAN_FEATURE
	OFFCHANNEL_SCAN_MSG Rsp;
#endif
	USHORT PhyMode;
	UCHAR OriChannel;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	PNET_DEV ndev_ap_if = NULL;
	UCHAR i = 0;
#endif
	struct DOT11_H *pDot11h = NULL;

#if (defined(CONFIG_RCSA_SUPPORT) || defined(MT_DFS_SUPPORT))
	CHANNEL_CTRL *pChCtrl = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif
	struct freq_oper oper = {0};

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"(caller:%pS), Channel(%d), quick_ch_change:%d.\n",
		 OS_TRACE, Channel, wdev->quick_ch_change);
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.set_ch_async_flag = FALSE;
#endif /* CONFIG_AP_SUPPORT */
	pDot11h = wdev->pDot11_H;
#ifdef MT_DFS_SUPPORT
	pDfsParam->OrigCh = wdev->channel;
#endif
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "wdev->pDot11_H is NULL.\n");
		return FALSE;
	}

	if (!VALID_MBSS(pAd, wdev->func_idx)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"mbss idx check fail!(%d)\n", wdev->func_idx);
		return FALSE;
	}

	hc_radio_query_by_wdev(wdev, &oper);
	if (wdev->channel != oper.prim_ch) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"Channel Mismatch!, radio channel is:%d, wdev->channel is:%d\n",
			oper.prim_ch, wdev->channel);
		return FALSE;
	}
	PhyMode = wdev->PhyMode;
	OriChannel = wdev->channel;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
		pStaCfg->MlmeAux.Channel = Channel;
		/*apply channel directly*/
		wlan_operate_set_prim_ch(wdev, Channel);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"CtrlChannel(%d), CentralChannel(%d)\n",
			Channel, wlan_operate_get_cen_ch_1(wdev));
	}
#endif /* CONFIG_STA_SUPPORT */

	/*used for not support MCC*/
	wdev->channel = Channel;
	wdev_sync_prim_ch(wdev->sys_handle, wdev);

#ifdef MT_DFS_SUPPORT
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

		if ((pAd->CommonCfg.dbdc_mode == TRUE) && (RadarChannelCheck(pAd, wdev->channel)) && (pChCtrl->ChListNum == 0)) {
			pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
			DfsBuildChannelList(pAd, wdev);
		}
#endif

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	} else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
		/* for APCLI, find first BSS with same channel */
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			if ((pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel) &&
					(pAd->ApCfg.MBSSID[i].wdev.if_up_down_state != 0)) {
				pMbss = &pAd->ApCfg.MBSSID[i];
				break;
			}
		}
	}

	if (pMbss != NULL)
		ndev_ap_if = pMbss->wdev.if_dev;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
			pAd->ApCfg.ApCliAutoConnectChannelSwitching = TRUE;

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

#ifdef DFS_SLAVE_SUPPORT
	if (SLAVE_BEACON_STOPPED(pAd) && pMbss) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"[DFS-SLAVE][%s] beaconing off do channel switch\n", __func__);
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i] && (pAd->wdev_list[i]->wdev_type == WDEV_TYPE_AP) &&
				(pAd->wdev_list[i]->channel == wdev->channel)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
					"[DFS-SLAVE] Enqueue SwitchCmd for func_idx = %d\n",
					pAd->wdev_list[i]->func_idx);
				RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &pAd->wdev_list[i]->func_idx, sizeof(UCHAR));
				break;
			}
		}
		goto lable_ok;
	}
#endif /* DFS_SLAVE_SUPPORT */

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (pAd->CommonCfg.bIEEE80211H == TRUE && !wdev->bcn_buf.stop_tx) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "CSA enabled.\n");
			pDot11h->org_ch = OriChannel;

			if (pMbss == NULL) {
				 /*AP Interface is not present and CLI wants to change channel*/
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"Only Change CLI Channel to %d!\n", wdev->channel);
#ifdef DFS_SLAVE_SUPPORT
				if (SLAVE_MODE_EN(pAd)) {
					if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) {
						pDot11h->ChannelMode = CHAN_SILENCE_MODE;
						pDot11h->InServiceMonitorCount = 0;
					}
					WrapDfsRadarDetectStop(pAd);
					wlan_operate_set_prim_ch(wdev, wdev->channel);
					MtCmdSetDfsTxStart(pAd, HcGetBandByWdev(wdev));
				} else
					wlan_operate_set_prim_ch(wdev, wdev->channel);
#else
				wlan_operate_set_prim_ch(wdev, wdev->channel);
#endif /* DFS_SLAVE_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
				pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif
				return TRUE;
			}

#ifdef CONFIG_MAP_SUPPORT
#ifdef MT_DFS_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd) && !(mt_validate_dfs_channel_for_cac(pAd, wdev)) &&
					pAd->CommonCfg.DfsParameter.bDfsEnable) {
				for (i = 0; i < MAX_BEACON_NUM; i++) {
					if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
						pAd->ApCfg.MBSSID[i].wdev.cac_not_required = TRUE;
				}
			}
#endif /* MT_DFS_SUPPORT */
#endif

			if (((pDot11h->ChannelMode == CHAN_SILENCE_MODE) && (!bss_mngr_is_wdev_in_mlo_group(wdev)))
				|| ((ndev_ap_if != NULL) && (!RTMP_OS_NETDEV_STATE_RUNNING(ndev_ap_if)))) {

				pDot11h->ChannelMode = CHAN_SWITCHING_MODE;
				if (wdev->quick_ch_change == QUICK_CH_SWICH_DISABLE) {

					if (pMbss != NULL)
						APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef MT_DFS_SUPPORT
					if (DfsStopWifiCheck(pAd, wdev)) {
						MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
							"Stop AP Startup\n");
						return FALSE;
					}
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
					if (pMbss != NULL) {
						memcpy(Rsp.ifrn_name, pAd->ScanCtrl.if_name, IFNAMSIZ);
						Rsp.Action = DRIVER_CHANNEL_SWITCH_SUCCESSFUL;
						Rsp.data.operating_ch_info.channel = HcGetRadioChannel(pAd);
						MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
							" 2.4G channel to switch = %d\n",
							Rsp.data.operating_ch_info.channel);
						Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
						Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
						Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
						Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
						if ((pAd->Antenna.field.TxPath == 4) && (pAd->Antenna.field.RxPath == 4))
							Rsp.data.operating_ch_info.is4x4Mode = 1;
						else
							Rsp.data.operating_ch_info.is4x4Mode = 0;
						wapp_send_event_offchannel_info(pAd,
								(UCHAR *)&Rsp,
								sizeof(OFFCHANNEL_SCAN_MSG));
					}
#endif

					if (pMbss != NULL)
						APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);

#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_TURNKEY_ENABLE(pAd)) {
						if (pMbss && pMbss->wdev.cac_not_required) {
							for (i = 0; i < MAX_BEACON_NUM; i++) {
								if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
									pAd->ApCfg.MBSSID[i].wdev.cac_not_required = FALSE;
							}
							pDot11h->RDCount = pDot11h->cac_time;
						}
					}
#endif
				} else
					ap_phy_rrm_init_byRf(pAd, wdev);
			}
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
			/* When this Procuct is in CNV mode or */
			/* REP mode and if ApCli is not connected */
			/* then we do not send CSA, and so we should imitate the event functionality here */
			else if (pMbss->APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
				UINT i = 0;

				for (i = 0; i < WDEV_NUM_MAX; i++) {
					if (pAd->wdev_list[i] && (pAd->wdev_list[i]->wdev_type == WDEV_TYPE_AP)) {
						MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_INFO,
							"Type = %d, func_idx = %d\n",
							pAd->wdev_list[i]->wdev_type, pAd->wdev_list[i]->func_idx);
						RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL,
							&pAd->wdev_list[i]->func_idx, sizeof(UCHAR));
						break;
					}
				}
			}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT*/

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
			else if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd) && IS_MAP_CERT_ENABLE(pAd)
				&& (wdev->quick_ch_change != QUICK_CH_SWICH_DISABLE &&
					(!RadarChannelCheck(pAd, wdev->channel)))) {
				ap_phy_rrm_init_byRf(pAd, wdev);
			}
#endif
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
			/* Do not send CSA(do not change inband channel) when set channel */
			else if (pDfsParam->SetOutBandChStat == OUTB_SET_CH_CAC) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"Set outband channel only\n");
				zero_wait_dfs_switch_ch(pAd, wdev, RDD_DEDICATED_IDX);
			}
#endif /*BACKGROUND_SCAN_SUPPORT*/

			else {
#ifdef ZERO_PKT_LOSS_SUPPORT
				/*adds delay in channel switch, enable only if needed*/
				if (pAd->Zero_Loss_Enable && pAd->Csa_Action_Frame_Enable) {
					/*send Broadcast CSA/Ext_CSA action frame for main bss
					* **add loop for all MBSS if needed
					*/
					NotifyBroadcastChSwAnn(pAd, wdev, CHAN_SWITCHING_MODE, Channel);
					NotifyBroadcastExtChSwAnn(pAd, wdev, CHAN_SWITCHING_MODE, Channel);
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_NOTICE,
					   "NotifyChSwAnnToConnectedSTAs\n");
					RtmpusecDelay(10000); /*delay for actionframe send*/
				}
#endif /*ZERO_PKT_LOSS_SUPPORT*/
				pDot11h->CSCount = 0;
				if (!DoActionBeforeCsa(pAd, wdev)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"DoActionBeforeCsa failed!\n");
					return FALSE;
				}

				pDot11h->ChannelMode = CHAN_SWITCHING_MODE;
#ifdef CONFIG_RCSA_SUPPORT
				if (pDfsParam->bRCSAEn && pDfsParam->fSendRCSA) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"Sending RCSA NewChannel:%d\n", Channel);
					notify_channel_switch_to_backhaulAP(pAd, wdev, Channel, pDfsParam->ChSwMode);
					pDfsParam->fSendRCSA = FALSE;
				}
#endif
				if (!HcUpdateCsaCnt(pAd, wdev)) {
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_TURNKEY_ENABLE(pAd))
						wdev->map_indicate_channel_change = 1;
#endif
					return TRUE;
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"CSA disabled.\n");
			if (wdev->quick_ch_change != QUICK_CH_SWICH_DISABLE)
				ap_phy_rrm_init_byRf(pAd, wdev);
			else {
				APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
				APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
			}
#ifdef DFS_ZEROWAIT_SUPPORT
			if (pAd->ApCfg.bChSwitchNoCac == 1)
				pAd->ApCfg.bChSwitchNoCac = 0;
#endif
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DFS_SLAVE_SUPPORT
lable_ok:
#endif /* DFS_SLAVE_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "::(Channel=%d)\n", Channel);
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd) || IS_MAP_BS_ENABLE(pAd))
		wdev->map_indicate_channel_change = 1;

	if (IS_MAP_BS_ENABLE(pAd) && (pAd->CommonCfg.bIEEE80211H == 0)) {
		wapp_send_ch_change_rsp(pAd, wdev, Channel);
		wdev->map_indicate_channel_change = 0;
	}
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
INT UNII4BandSupport(RTMP_ADAPTER *pAd)
{

	INT Region_Count = sizeof(UNII4BandSupportRegions) / sizeof(int);
	INT i;

	for (i = 0; i < Region_Count; i++) {
		if (pAd->CommonCfg.CountryRegionForABand == UNII4BandSupportRegions[i])
			return TRUE;
	}

	return FALSE;
}
INT	rtmp_set_channel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
	INT32 Success = TRUE;
	UCHAR OriChannel;
	UCHAR  new_bw, old_bw;
	UCHAR Ch;
	struct DOT11_H *pDot11h = NULL;
	BOOLEAN RestoreChAfterNopEnd = pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd;
	BOOLEAN WaitChNop = pAd->CommonCfg.DfsParameter.bDfsWaitCfgChNop;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "wdev == NULL!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"(caller:%pS), Channel(%d), quick_ch_change:%d.\n",
		 OS_TRACE, Channel, wdev->quick_ch_change);

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pDot11h == NULL!\n");
		return FALSE;
	}

	if (IsHcRadioCurStatOffByWdev(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"IsHcRadioCurStatOffByWdev true!\n");
		return FALSE;
	}

	OriChannel = wdev->channel;
	new_bw = wlan_config_get_bw(wdev);
	old_bw = wlan_operate_get_bw(wdev);

	RTMP_UPDATE_CHANNEL_INFO(pAd, Channel2Index(pAd, wdev->channel));

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE
	&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == DFS_SET_CHANNEL
	&& pAd->CommonCfg.DfsParameter.CacCtrl != NO_NEED_CAC)
		Adj_ZeroWait_Status_Update(pAd, wdev, &new_bw, &Channel);
#endif /*DFS_ADJ_BW_ZERO_WAIT*/

#ifdef MT_DFS_SUPPORT
	if (pAd->CommonCfg.BandSelBand == BAND_SELECT_BAND_5G) {
		update_cac_ctrl_status(pAd, wdev, OriChannel, old_bw, Channel, new_bw);
	}
#endif /*MT_DFS_SUPPORT*/

	/*165 channel can only work at 20M*/
	if ((Channel == 165) && WMODE_CAP_5G(wdev->PhyMode)) {
		if (wlan_operate_get_ht_bw(wdev) || wlan_operate_get_vht_bw(wdev)) {
			if (UNII4BandSupport(pAd) == FALSE)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"Warning: 165 Channel must have to call back to HtBw/VhtBw 20M!\n");
		}
	}

	/* check if this channel is valid*/
	if (!IsValidChannel(pAd, Channel, wdev) &&
		(pAd->CommonCfg.DfsParameter.bDfsWaitCfgChNop != TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			Channel = FirstChannel(pAd, wdev);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					 "This channel is out of channel list, set as the first channel(%d)\n ", Channel);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			Success = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"This channel is out of channel list, nothing to do!\n ");
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	if (IS_CH_BETWEEN(Channel, 36, 48) && (new_bw == BW_160)) {
		if (RestoreChAfterNopEnd == FALSE && WaitChNop == FALSE) {
			/* Allow ch36~ch48 if ch52~ch64 in NOP */
			for (Ch = 52; Ch <= 64; Ch += 4) {
				if (CheckNonOccupancyChannel(pAd, wdev, Ch) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"Adjust bw(%d) to bw(%d) for channel(%d), and no need CAC\n",
							BW_160, BW_80, Channel);
					pAd->CommonCfg.DfsParameter.CacCtrl = NO_NEED_CAC;
					pAd->CommonCfg.DfsParameter.band_bw = BW_80;
					new_bw = BW_80;
					break;
				}
			}
		}
	}

	if ((OriChannel == Channel)
	&& (IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) == FALSE)) {
		if ((Channel == 36 && (pAd->CommonCfg.DfsParameter.inband_ch_stat == DFS_INB_DFS_RADAR_OUTB_CAC_DONE)) ||
			pAd->CommonCfg.DfsParameter.MapZwFlag) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"DFS inband radar trigger, set to CH36.\n ");
				pAd->CommonCfg.DfsParameter.MapZwFlag = FALSE;
		} else if (pAd->CommonCfg.DfsParameter.bNoAvailableCh == TRUE &&
				   pAd->CommonCfg.DfsParameter.DfsNopExpireSetChPolicy != DFS_NOP_EXPIRE_SET_CH_POLICY_NOT_RESTORE) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"Use channel(%d) & bw(%d), but not available now\n",
					Channel, pAd->CommonCfg.DfsParameter.band_bw);
		} else if ((pAd->CommonCfg.DfsParameter.bDfsWaitCfgChNop == TRUE ||
					pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd == TRUE) &&
				   (pAd->CommonCfg.DfsParameter.DfsNopExpireSetChPolicy != DFS_NOP_EXPIRE_SET_CH_POLICY_NOT_RESTORE)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Setting to channel(%d) & bw(%d)\n",
					Channel, pAd->CommonCfg.DfsParameter.band_bw);
			if (pAd->CommonCfg.DfsParameter.bDfsWaitCfgChNop)
				pAd->CommonCfg.DfsParameter.bDfsWaitCfgChNop = FALSE;
			if (pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"Setting ZwChBootUp to TRUE\n");
				pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd = FALSE;
				if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault)
					pAd->CommonCfg.DfsParameter.ZwChBootUp = TRUE;
			}
		} else if (new_bw != old_bw) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Setting to channel(%d) & bw(%d)\n",
					Channel, new_bw);
		} else {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "The channel %d NOT changed.\n ", OriChannel);
			return TRUE;
		}
	}

	if (pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"Setting ZwChBootUp to TRUE\n");
		pAd->CommonCfg.DfsParameter.bDfsRestoreCfgChAfterNopEnd = FALSE;
		if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault)
			pAd->CommonCfg.DfsParameter.ZwChBootUp = TRUE;
	}

	if (pAd->CommonCfg.bIEEE80211H == TRUE) {
		if (CheckNonOccupancyChannel(pAd, wdev, Channel) == FALSE) {
			if (pAd->CommonCfg.DfsParameter.DfsNopExpireSetChPolicy != DFS_NOP_EXPIRE_SET_CH_POLICY_NOT_RESTORE &&
				(RestoreChAfterNopEnd == TRUE || WaitChNop == TRUE))
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"Restore ch(%d) after NOP or waiting NOP, restore policy(%d)\n",
					wdev->channel, pAd->CommonCfg.DfsParameter.DfsNopExpireSetChPolicy);
			else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Can not update channel(%d), restoring old channel(%d)\n",
					wdev->channel, OriChannel);
				return FALSE;
			}
		}
	}

#ifdef WIFI_MD_COEX_SUPPORT
	if (!IsChannelSafe(pAd, Channel)) {
		if (IsPwrChannelSafe(pAd, Channel))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					 "caller:%pS. The channel %d is power backoff channel\n ",
					 OS_TRACE, Channel);
		else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					 "caller:%pS. The channel %d is in unsafe channel list!!\n ",  OS_TRACE, Channel);
	}
#endif

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	 pAd->CommonCfg.DfsParameter.ByPassCac = DfsZwBypassCac(pAd, wdev, OriChannel, Channel);
	 MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"DFS-inband status %d, DFS-outband status %d\n",
					pAd->CommonCfg.DfsParameter.inband_ch_stat,
					pAd->CommonCfg.DfsParameter.SetOutBandChStat);

	if (wdev->quick_ch_change && !pAd->CommonCfg.DfsParameter.ByPassCac
	&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == FALSE) {

		if (RadarChannelCheck(pAd, Channel)
			|| (pAd->CommonCfg.DfsParameter.CacCtrl == NEED_RESTART && !IS_MAP_TURNKEY_ENABLE(pAd))) {

			if (pAd->CommonCfg.DfsParameter.CACMemoEn
				&& dfs_cac_op(pAd, wdev, CAC_DONE_CHECK, Channel)) {

				if (pAd->CommonCfg.DfsParameter.bPreCacEn) {
					pAd->CommonCfg.DfsParameter.SetOutBandChStat = OUTB_SET_CH_DEFAULT;
					dfs_pre_cac_start_detect(pAd, wdev);
				} else
					pAd->CommonCfg.DfsParameter.inband_ch_stat = DFS_INB_CH_SWITCH_CH;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
					"DFS-Pre CAC done, skip zero_waite dfs\n");
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"Set to new dfsch, zwdfs engine reset!\n");
				if (pAd->CommonCfg.DfsParameter.inband_ch_stat != DFS_INB_DFS_RADAR_OUTB_CAC_DONE &&
					(pAd->CommonCfg.DfsParameter.inband_ch_stat != DFS_INB_DFS_RADAR_OUTB_CAC_DONE_QUICK)) {
					DfsDedicatedExamineSetNewCh(pAd, wdev, Channel);
					DedicatedZeroWaitStop(pAd, TRUE);
				}

				/* CAC of new DFS ch X will be checked by dedicated RX */
				zero_wait_dfs_update_ch(pAd, wdev, OriChannel, &Channel);
			}
		} else if (pAd->CommonCfg.DfsParameter.inband_ch_stat == DFS_OUTB_CH_CAC) {
			if (pAd->CommonCfg.DfsParameter.bPreCacEn)
				dfs_pre_cac_detect_next_channel(pAd, wdev);
			else
				pAd->CommonCfg.DfsParameter.OutBandCh = 0;
		} else if (((pAd->CommonCfg.DfsParameter.inband_ch_stat != DFS_INB_DFS_RADAR_OUTB_CAC_DONE) &&
			(pAd->CommonCfg.DfsParameter.inband_ch_stat != DFS_INB_DFS_RADAR_OUTB_CAC_DONE_QUICK) &&
			Channel != 36)) {

			pAd->CommonCfg.DfsParameter.inband_ch_stat = DFS_INB_CH_INIT;
			if (pAd->CommonCfg.DfsParameter.bPreCacEn)
				dfs_pre_cac_detect_next_channel(pAd, wdev);
			else
				pAd->CommonCfg.DfsParameter.OutBandCh = 0;
		}
	}
#endif
#endif
	if (Success) {
		Success = perform_channel_change(pAd, wdev, Channel);
#ifdef MT_DFS_SUPPORT
		DfsBuildChannelList(pAd, wdev);
#endif
	}

	return Success;
}

/*
    ==========================================================================
    Description:
	Set Short Slot Time Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;
	UCHAR BandIdx = 0;
#ifdef CONFIG_AP_SUPPORT
	struct  wifi_dev *wdev;
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */

	retval = RT_CfgSetShortSlot(pAd, arg, BandIdx);

	if (retval == TRUE)
		MTWF_PRINT("%s::(ShortSlot=%d)\n", __func__, pAd->CommonCfg.bUseShortSlotTime);

	return retval;
}

/*
    ==========================================================================
    Description:
	Set Tx power
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_TxPower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG    TxPower;
	INT     status = FALSE;
	UINT8   BandIdx = 0;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
	BandIdx = hc_get_hw_band_idx(pAd);
	MTWF_PRINT("%s: BandIdx = %d\n", __func__, BandIdx);
	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	TxPower = simple_strtol(arg, 0, 10);

	if (TxPower <= 100) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pAd->CommonCfg.ucTxPowerPercentage = TxPower;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pAd->CommonCfg.ucTxPowerDefault = TxPower;
			pAd->CommonCfg.ucTxPowerPercentage =
				pAd->CommonCfg.ucTxPowerDefault;
		}
#endif /* CONFIG_STA_SUPPORT */
		status = TRUE;
	} else
		status = FALSE;

	MTWF_PRINT("%s: BandIdx: %d, (TxPowerPercentage=%d)\n",
		__func__, BandIdx, pAd->CommonCfg.ucTxPowerPercentage);
	return status;
}

/*
    ==========================================================================
    Description:
	Set 11B/11G Protection
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BGProtection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32       apidx = pObj->ioctl_if;

	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev) {
		MTWF_PRINT("%s: Invalid apidx=%d!\n", __func__, apidx);
		return FALSE;
	}

	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*AUTO*/
		pAd->CommonCfg.UseBGProtection = 0;
		break;

	case 1: /*Always On*/
		pAd->CommonCfg.UseBGProtection = 1;
		break;

	case 2: /*Always OFF*/
		pAd->CommonCfg.UseBGProtection = 2;
		break;

	default:  /*Invalid argument */
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		ApUpdateCapabilityAndErpIe(pAd, &pAd->ApCfg.MBSSID[apidx]);
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_PRINT("%s::(BGProtection=%ld)\n",
		__func__, pAd->CommonCfg.UseBGProtection);
	return TRUE;
}

/*
 *  ==========================================================================
 *  Description:
 *	Set Set_McastBcastMcs_CCK
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
*/
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
INT Set_McastBcastMcs_CCK(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Mcs)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	BOOLEAN isband5g, tmp_band;
	union _HTTRANSMIT_SETTING *pTransmit;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "Invalid device interface!\n");
		return FALSE;
	}

	if (Mcs > 15) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "Mcs must be in range of 0 to 15\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	isband5g = (wdev->channel > 14) ? TRUE : FALSE;
	pTransmit = (isband5g) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);

	if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11)) {
		pTransmit->field.MCS = Mcs;
	} else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
		return FALSE;
	}

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		tmp_band = (wdev->channel > 14) ? TRUE : FALSE;

		if (tmp_band != isband5g)
			continue;

		NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
		bss_info_argument.bss_state = BSS_ACTIVE;
		bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		bss_info_argument.u8BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;

		memmove(&bss_info_argument.BcTransmit, pTransmit, sizeof(union _HTTRANSMIT_SETTING));
		memmove(&bss_info_argument.McTransmit, pTransmit, sizeof(union _HTTRANSMIT_SETTING));

		if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
					 "Fail to apply the bssinfo, BSSID=%d!\n", i);
	}

	return TRUE;
}
#endif

/*
    ==========================================================================
    Description:
	Set TxPreamble
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RT_802_11_PREAMBLE	Preamble;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	RT_802_11_PREAMBLE	OldPreamble = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	union _HTTRANSMIT_SETTING *pTransmit = NULL;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	BOOLEAN isband5g = FALSE;
	UCHAR MCS = 0;
#endif

	Preamble = (RT_802_11_PREAMBLE)os_str_tol(arg, 0, 10);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

	if (Preamble == Rt802_11PreambleAuto)
		return FALSE;

#endif /* CONFIG_AP_SUPPORT */

	switch (Preamble) {
	case Rt802_11PreambleShort:
		pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
#endif /* CONFIG_STA_SUPPORT */
		break;

	case Rt802_11PreambleLong:
#ifdef CONFIG_STA_SUPPORT
	case Rt802_11PreambleAuto:
		/*
			If user wants AUTO, initialize to LONG here, then change according to AP's
			capability upon association
		*/
#endif /* CONFIG_STA_SUPPORT */
		pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
#endif /* CONFIG_STA_SUPPORT */
		break;

	default: /*Invalid argument */
		return FALSE;
	}

#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	/* MCS Switch based on new Preamble */
	if (wdev) {
		isband5g = (wdev->channel > 14) ? TRUE : FALSE;
		pTransmit = (isband5g) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);

		MTWF_PRINT("%s: Old MCS %d Old Preamble %d New Preamble %d\n",
			__func__, MCS, OldPreamble, Preamble));

		if (pTransmit && (pTransmit->field.MODE == MODE_CCK)) {
			MCS = pTransmit->field.MCS;

		if ((MCS) && (Preamble == Rt802_11PreambleShort) && (OldPreamble == Rt802_11PreambleLong))
			/* New Preamble = Short : Old Preamble = Long */
			MCS--;
		else if (((Preamble == Rt802_11PreambleLong) && (OldPreamble == Rt802_11PreambleShort))
#ifdef CONFIG_STA_SUPPORT
			|| ((pAd->OpMode == OPMODE_STA) &&
				(Preamble == Rt802_11PreambleAuto)
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))))
#else

#endif
				/* New Preamble = Long/Auto : Old Preamble = Short */
				MCS++;

				if ((pTransmit->field.MCS != MCS) || (Preamble != OldPreamble))
					Set_McastBcastMcs_CCK(pAd, MCS);
		}
	}
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */

	MTWF_PRINT("%s::(TxPreamble=%ld)\n", __func__, pAd->CommonCfg.TxPreamble);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set RTS Threshold
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
static VOID set_rts_len_thld(struct wifi_dev *wdev, UINT32 length)
{
	wlan_operate_set_rts_len_thld(wdev, length);
}

INT Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 length;

	if (arg == NULL) {
		MTWF_PRINT("Usage:\niwpriv raN set RTSThreshold=[length]\n");
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	length = os_str_tol(arg, 0, 10);
	set_rts_len_thld(wdev, length);
	MTWF_PRINT ("%s: set wdev%d rts length threshold=%d(0x%x)\n", __func__, wdev->wdev_idx, length, length);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Fragment Threshold
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 frag_thld;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);

	if (!arg)
		return FALSE;

	if (!wdev)
		return FALSE;

	frag_thld = os_str_tol(arg, 0, 10);

	if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
		frag_thld = MAX_FRAG_THRESHOLD;
	else if ((frag_thld % 2) == 1)
		frag_thld -= 1;

	wlan_operate_set_frag_thld(wdev, frag_thld);
	MTWF_PRINT ("%s::set wdev%d FragThreshold=%d)\n", __func__, wdev->wdev_idx, frag_thld);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set TxBurst
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG TxBurst;

	TxBurst = os_str_tol(arg, 0, 10);

	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_PRINT("%s::(TxBurst=%d)\n", __func__, pAd->CommonCfg.bEnableTxBurst);
	return TRUE;
}

INT Set_MaxTxPwr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR IfIdx;
	UCHAR MaxTxPwr = 0;
	CHANNEL_CTRL *pChCtrl;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	IfIdx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[IfIdx].wdev;
	else if (pObj->ioctl_if_type == INT_MSTA)
		wdev = &pAd->StaCfg[IfIdx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("%s: pObj->ioctl_if_type = %d!!\n", __func__, pObj->ioctl_if_type);
		return FALSE;
	}

	MaxTxPwr = (UCHAR) simple_strtol(arg, 0, 10);

	if ((MaxTxPwr > 0) && (MaxTxPwr < 0xff)) {
		pAd->MaxTxPwr = MaxTxPwr;
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
		BuildChannelList(pAd, wdev);
#endif
		MTWF_PRINT("Set MaxTxPwr = %d\n", MaxTxPwr);
		return TRUE;
	}

	MTWF_PRINT("ERROR: wrong power announced(MaxTxPwr=%d)\n", MaxTxPwr);
	return FALSE;

}

/*
    ==========================================================================
    Description:
	Set IEEE80211H.
	This parameter is 1 when needs radar detection, otherwise 0
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG ieee80211h;

	ieee80211h = os_str_tol(arg, 0, 10);

	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0) { /*Disable*/
		pAd->CommonCfg.bIEEE80211H = FALSE;
#ifdef BACKGROUND_SCAN_SUPPORT
		pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#endif
#ifdef MT_DFS_SUPPORT
		pAd->CommonCfg.DfsParameter.bDfsEnable = FALSE;
#ifdef MT_BAND4_DFS_SUPPORT /*302502*/
		pAd->CommonCfg.DfsParameter.band4DfsEnable = FALSE;
#endif
		UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
		MTWF_PRINT("%s: Disable DFS/Zero wait=%d/%d\n",
			__func__,
			IS_SUPPORT_MT_DFS(pAd),
			IS_SUPPORT_MT_ZEROWAIT_DFS(pAd));
#endif
	} else {
		MTWF_PRINT("%s: Invalid argument:%ld\n", __func__, ieee80211h);
		return FALSE;  /*Invalid argument */
	}

	MTWF_PRINT("%s: (IEEE80211H=%d)\n", __func__, pAd->CommonCfg.bIEEE80211H);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ieee80211h(RTMP_ADAPTER *pAd, UCHAR ieee80211h)
{
	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return -EINVAL;

	MTWF_PRINT("%s::(IEEE80211H=%d)\n", __func__, pAd->CommonCfg.bIEEE80211H);
	return 0;
}
#endif

#ifdef EXT_BUILD_CHANNEL_LIST
/*
    ==========================================================================
    Description:
	Set Country Code.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("%s can only be used when interface is down.\n", __func__);
		return TRUE;
	}

	if (strlen(arg) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	{
		UCHAR CountryCode[3] = {0};

		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_PRINT("%s::(bCountryFlag=%d, CountryCode=%s)\n",
			__func__,
			pAd->CommonCfg.bCountryFlag,
			CountryCode);
	}

	return TRUE;
}
/*
    ==========================================================================
    Description:
	Set Ext DFS Type
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR	 *pDfsType = &pAd->CommonCfg.DfsType;

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("%s can only be used when interface is down.\n", __func__);
		return TRUE;
	}

	if (!strcmp(arg, "CE"))
		*pDfsType = CE;
	else if (!strcmp(arg, "FCC"))
		*pDfsType = FCC;
	else if (!strcmp(arg, "JAP"))
		*pDfsType = JAP;
	else if (!strcmp(arg, "KR"))
		*pDfsType = KR;
	else
		MTWF_PRINT("%s: Unsupported DFS type:%s (Legal types are: CE, FCC, JAP)\n",
			__func__, arg);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Add new channel list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CH_DESP		inChDesp;
	PCH_REGION pChRegion = NULL;

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("%s can only be used when interface is down.\n", __func__);
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_PRINT("%s: CountryCode is not configured or not valid\n", __func__);
			return TRUE;
		}
	}
	/* Parsing the arg, IN:arg; OUT:inChRegion */
	{
		UCHAR strBuff[64], count = 0;
		PUCHAR	pStart, pEnd, tempIdx, tempBuff[5];

		if (strlen(arg) < 64)
			NdisCopyMemory(strBuff, arg, strlen(arg));

		pStart = rtstrchr(strBuff, '[');

		if (pStart != NULL) {
			pEnd = rtstrchr(pStart++, ']');

			if (pEnd != NULL) {
				tempBuff[count++] = pStart;

				for (tempIdx = pStart; tempIdx != pEnd; tempIdx++) {
					if (*tempIdx == ',') {
						*tempIdx = '\0';
						tempBuff[count++] = ++tempIdx;
					}
				}

				*(pEnd) = '\0';

				if (count != 5) {
					MTWF_PRINT("Input Error. Too more or too less parameters.\n");
					return TRUE;
				} else {
					inChDesp.FirstChannel = (UCHAR) os_str_tol(tempBuff[0], 0, 10);
					inChDesp.NumOfCh = (UCHAR) os_str_tol(tempBuff[1], 0, 10);
					inChDesp.MaxTxPwr = (UCHAR) os_str_tol(tempBuff[2], 0, 10);
					inChDesp.Geography = (!strcmp(tempBuff[3], "BOTH") ? BOTH : (!strcmp(tempBuff[3], "IDOR") ? IDOR : ODOR));
					inChDesp.DfsReq = (!strcmp(tempBuff[4], "TRUE") ? TRUE : FALSE);
				}
			} else {
				MTWF_PRINT("%s: Missing End \"]\"\n", __func__);
				return TRUE;
			}
		} else {
			MTWF_PRINT("%s: Invalid input format.\n", __func__);
			return TRUE;
		}
	}
	/* Add entry to Channel List*/
	{
		UCHAR EntryIdx;
		PCH_DESP pChDesp = NULL;
		UCHAR CountryCode[3] = {0};

		if (pAd->CommonCfg.pChDesp == NULL) {
			os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE * sizeof(CH_DESP));
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			if (pChDesp) {
				for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
					if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
						MTWF_PRINT("%s: Table is full.\n", __func__);
						return TRUE;
					}

					NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
				}

				/* Copy the NULL entry*/
				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			} else {
				MTWF_PRINT("%s: os_alloc_mem failded.\n", __func__);
				return FALSE;
			}
		} else {
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
				if (EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
					MTWF_PRINT("Table is full.\n");
					return TRUE;
				}
			}
		}

		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_PRINT("%s: Add channel lists {%u, %u, %u, %s, %s} to %s.\n", __func__,
				 inChDesp.FirstChannel,
				 inChDesp.NumOfCh,
				 inChDesp.MaxTxPwr,
				 (inChDesp.Geography == BOTH) ? "BOTH" : (inChDesp.Geography == IDOR) ?  "IDOR" : "ODOR",
				 (inChDesp.DfsReq == TRUE) ? "TRUE" : "FALSE",
				 CountryCode);
		NdisCopyMemory(&pChDesp[EntryIdx], &inChDesp, sizeof(CH_DESP));
		pChDesp[++EntryIdx].FirstChannel = 0;
	}
	return TRUE;
}

INT Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCH_REGION	pChRegion = NULL;
	UCHAR		EntryIdx, CountryCode[3] = {0};
	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_PRINT("CountryCode is not configured or not valid\n");
			return TRUE;
		}
	}
	NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);

	if (pAd->CommonCfg.DfsType == MAX_RD_REGION)
		pAd->CommonCfg.DfsType = pChRegion->op_class_region;

	MTWF_PRINT("=========================================\n");
	MTWF_PRINT("CountryCode:%s\n", CountryCode);
	MTWF_PRINT("DfsType:%s\n",
			 (pAd->CommonCfg.DfsType == KR) ? "KR" :
			 (pAd->CommonCfg.DfsType == JAP) ? "JAP" :
			 ((pAd->CommonCfg.DfsType == FCC) ? "FCC" : "CE"));

	if (pAd->CommonCfg.pChDesp != NULL) {
		PCH_DESP pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

		for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			MTWF_PRINT("%u. {%3u, %2u, %2u, %s, %5s}.\n",
					 EntryIdx,
					 pChDesp[EntryIdx].FirstChannel,
					 pChDesp[EntryIdx].NumOfCh,
					 pChDesp[EntryIdx].MaxTxPwr,
					 (pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" : "ODOR",
					 (pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE");
		}
	} else {
		MTWF_PRINT("Default channel list table:\n");

		for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			MTWF_PRINT("%u. {%3u, %2u, %2u, %s, %5s}.\n",
					 EntryIdx,
					 pChRegion->pChDesp[EntryIdx].FirstChannel,
					 pChRegion->pChDesp[EntryIdx].NumOfCh,
					 pChRegion->pChDesp[EntryIdx].MaxTxPwr,
					 (pChRegion->pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChRegion->pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" :
					 "ODOR",
					 (pChRegion->pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE");
		}
	}

	MTWF_PRINT("=========================================\n");
	return TRUE;
}

INT Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR EntryIdx, TargetIdx, NumOfEntry;
	PCH_REGION	pChRegion = NULL;
	PCH_DESP pChDesp = NULL;

	TargetIdx = os_str_tol(arg, 0, 10);

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("%s can only be used when interface is down.\n", __func__);
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0) {
				pChRegion = &ChRegion[loop];
				break;
			}

			loop++;
		}

		if (pChRegion == NULL) {
			MTWF_PRINT("%s: CountryCode is not configured or not valid\n", __func__);
			return TRUE;
		}
	}

	if (pAd->CommonCfg.pChDesp == NULL) {
		os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE * sizeof(CH_DESP));

		if (pAd->CommonCfg.pChDesp) {
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

			for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
				if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table*/
					MTWF_PRINT("%s: Table is full.\n", __func__);
					return TRUE;
				}

				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			}

			/* Copy the NULL entry*/
			NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
		} else {
			MTWF_PRINT("%s: os_alloc_mem failded.\n", __func__);
			return FALSE;
		}
	} else
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

	if (!strcmp(arg, "default")) {
		MTWF_PRINT("%s: Default table used.\n", __func__);

		if (pAd->CommonCfg.pChDesp != NULL)
			os_free_mem(pAd->CommonCfg.pChDesp);

		pAd->CommonCfg.pChDesp = NULL;
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
	} else if (!strcmp(arg, "all")) {
		MTWF_PRINT("%s: Remove all entries.\n", __func__);

		for (EntryIdx = 0; EntryIdx < MAX_PRECONFIG_DESP_ENTRY_SIZE; EntryIdx++)
			NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP));
	} else if (TargetIdx < (MAX_PRECONFIG_DESP_ENTRY_SIZE - 1)) {
		for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++) {
			if (EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE - 2)) { /* Keep an NULL entry in the end of table */
				MTWF_PRINT("%s: Last entry should be NULL.\n", __func__);
				pChDesp[EntryIdx].FirstChannel = 0;
				return TRUE;
			}
		}

		NumOfEntry = EntryIdx;

		if (TargetIdx >= NumOfEntry) {
			MTWF_PRINT("%s: Out of table range.\n", __func__);
			return TRUE;
		}

		for (EntryIdx = TargetIdx; EntryIdx < NumOfEntry; EntryIdx++)
			NdisCopyMemory(&pChDesp[EntryIdx], &pChDesp[EntryIdx + 1], sizeof(CH_DESP));

		NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP)); /*NULL entry*/
		MTWF_PRINT("%s: Entry %u deleted.\n", __func__, TargetIdx);
	} else
		MTWF_PRINT("%s: Entry not found.\n", __func__);

	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST  */

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj;
	INT32       IfIdx;

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pAd == NULL!\n");
		return TRUE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	IfIdx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
				pWscControl = &pAd->StaCfg[IfIdx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"IF(apcli%d):: This command is from apcli interface now.\n", IfIdx);
		} else
#endif /* APCLI_SUPPORT */
		{
			if (VALID_MBSS(pAd, IfIdx))
				pWscControl = &pAd->ApCfg.MBSSID[IfIdx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"IF(ra%d):: This command is from ra interface now.\n", IfIdx);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
			pWscControl = &pAd->StaCfg[IfIdx].wdev.WscControl;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (pWscControl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"pWscControl == NULL! (IfIdx=%d).\n", IfIdx);
		return FALSE;
	}

	if (pWscControl->WscEnrollee4digitPinCode) {
		pWscControl->WscEnrolleePinCodeLen = 4;
		pWscControl->WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
	} else {
		pWscControl->WscEnrolleePinCodeLen = 8;
		pWscControl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, IfIdx);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		":: Enrollee PinCode\t\t%08u\n",
		pWscControl->WscEnrolleePinCode);
	return TRUE;
}

INT Set_WscVendorPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32       IfIdx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
				pWscControl = &pAd->StaCfg[IfIdx].wdev.WscControl;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"for apcli(%d)\n", IfIdx);
		} else
#endif /* APCLI_SUPPORT */
		{
			if (VALID_MBSS(pAd, IfIdx))
				pWscControl = &pAd->ApCfg.MBSSID[IfIdx].wdev.WscControl;
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"for ra%d!\n", IfIdx);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
			pWscControl = &pAd->StaCfg[IfIdx].wdev.WscControl;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (!pWscControl) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			"pWscControl NULL, (IfIdx=%d)\n",
			IfIdx);
		return FALSE;
	}
	else
		return RT_CfgSetWscPinCode(pAd, arg, pWscControl);
}
#endif /* WSC_INCLUDED */


#ifdef DBG
INT rx_temp_dbg;

static PCHAR cat_str[32] = {
	"MISC",		/* DBG_CAT_MISC     0 */
	"INIT",		/* DBG_CAT_INIT     1 */
	"HW",		/* DBG_CAT_HW       2 */
	"FW",		/* DBG_CAT_FW       3 */
	"HIF",		/* DBG_CAT_HIF      4 */
	"FPGA",		/* DBG_CAT_FPGA     5 */
	"TEST",		/* DBG_CAT_TEST     6 */
	"RA",		/* DBG_CAT_RA       7 */
	"AP",		/* DBG_CAT_AP       8 */
	"CLIENT",	/* DBG_CAT_CLIENT   9 */
	"TX",		/* DBG_CAT_TX       10 */
	"RX",		/* DBG_CAT_RX       11 */
	"CFG",		/* DBG_CAT_CFG      12 */
	"MLME",		/* DBG_CAT_MLME     13 */
	"PROTO",	/* DBG_CAT_PROTO    14 */
	"SEC",		/* DBG_CAT_SEC      15 */
	"PS",		/* DBG_CAT_PS       16 */
	"POWER",	/* DBG_CAT_POWER    17 */
	"COEX",		/* DBG_CAT_COEX     18 */
	"P2P",		/* DBG_CAT_P2P      19 */
	"TOKEN",	/* DBG_CAT_TOKEN    20 */
	"CMW",		/* DBG_CAT_CMW      21 */
	"BF",		/* DBG_CAT_BF       22 */
	"CHN",		/* DBG_CAT_CHN      23 */
	"MLO",		/* DBG_CAT_MLO      24 */
	"CCN34",	/* DBG_CAT_CCN34    25 */
	"INTF",		/* DBG_CAT_INTF     26 */
	"ANDLINK",	/* DBG_CAT_ANDLINK  27 */
	"CFG80211",	/* DBG_CAT_CFG80211 28 */
};

static PCHAR sub_cat_str[32][32] = {
	/* DBG_CAT_MISC		0 */ {"MISC"},
	/* DBG_CAT_INIT		1 */ {"MISC", "INTF", "TRCTRL", "MLME", "EEPROM"},
	/* DBG_CAT_HW		2 */ {"MISC", "SA", "SER", "PHY", "MAC", "RXV", "RXPHY", "CMD_CTRL", "HDEV_CTRL", "EEPROM", "GREEPAP"},
	/* DBG_CAT_FW		3 */ {"MISC", "DEVINFO", "BSSINFO", "STAREC", "MLD", "EHT", "FR_TABLE", "COUNTER_INFO", "11V_MBSS", "EFUSE", "DBGINFO", "SR", "PHY", "EAP", "SDO"},
	/* DBG_CAT_HIF		4 */ {"MISC", "PCI", "USB", "HWIFI", "IO"},
	/* DBG_CAT_FPGA		5 */ {"MISC"},
	/* DBG_CAT_TEST		6 */ {"MISC", "RFEATURE", "PRECFG", "ATE", "BF", "MDVT", "SDO", "PROFILING", "RVR", "SANITY", "DIAG", "OOM"},
	/* DBG_CAT_RA		7 */ {"MISC", "CFG"},
	/* DBG_CAT_AP		8 */ {"MISC", "MBSS", "WDS", "BCN", "DISC", "KPLIVE", "11V_MBSS", "TR181", "DISCON", "ACL", "COSR", "MNT", "WAPP", "STEERING", "IDS", "QBSS", "SEC", "BMC_SN", "VOW", "RECONF_FLOW", "BPCC"},
	/* DBG_CAT_CLIENT	9 */ {"MISC", "ADHOC", "APCLI", "MESH", "LP", "ROAMING"},
	/* DBG_CAT_TX		10 */ {"MISC", "TMAC", "TXOP", "FRAG_TX", "VLAN", "MGMT", "DATA", "SEC_TX", "WMM", "DABS_QOS", "4ADDR", "VOW", "CNT", "RED", "PREAMBLE", "POWER"},
	/* DBG_CAT_RX		11 */ {"MISC", "FRAG_RX", "SEC_RX", "AMSDU", "AMPDU", "802_11D", "802_3D", "MGMT", "CTRL", "RXINFO", "CNT", "VOW"},
	/* DBG_CAT_CFG		12 */ {"MISC", "TX", "INTF", "CMD", "DBGLOG", "PROFILE", "SR", "PP", "VENDOR"},
	/* DBG_CAT_MLME		13 */ {"MISC", "WTBL", "TXOP", "AIFS", "RX_DROP", "IE_INFO", "AUTH", "ASSOC", "CNTL", "SYNC", "STAT", "ROUTINE", "DOT1X", "BSSENTRY"},
	/* DBG_CAT_PROTO	14 */ {"MISC", "ACM", "BA", "TDLS", "WNM", "IGMP", "MAT", "RRM", "DFS", "FT", "SCAN", "FTM", "OCE", "TWT", "COLOR", "EHT", "ACTION", "HS_R2", "HT", "VHT", "HE", "DPP", "AFC", "MBO", "MAP", "QOS_MAP", "BND_STRG", "WHC", "BTM"},
	/* DBG_CAT_SEC		15 */ {"MISC", "KEY", "WPS", "WAPI", "PMF", "SAE", "SUITEB", "OWE", "ECC", "BCNPROT", "OCV", "MLO", "ALG", "PMK", "WEP", "WPA", "AES", "TKIP"},
	/* DBG_CAT_PS		16 */ {"MISC", "UAPSD", "CFG", "LP", "GREENAP"},
	/* DBG_CAT_POWER	17 */ {"MISC", "SKU"},
	/* DBG_CAT_COEX		18 */ {"MISC", "MD"},
	/* DBG_CAT_P2P		19 */ {"MISC"},
	/* DBG_CAT_TOKEN	20 */ {"MISC", "INFO", "PROFILE", "TRACE"},
	/* DBG_CAT_CMW		21 */ {"MISC"},
	/* DBG_CAT_BF		22 */ {"MISC", "IWCMD", "ASSOC", "SOUND", "CFG", "EVENT_INFO"},
	/* DBG_CAT_CHN		23 */ {"MISC", "ACS", "DFS", "SCAN", "UNSAFE", "CHN", "SCS", "CSI"},
	/* DBG_CAT_MLO		24 */ {"MISC", "BMGR", "CONN", "AUTH", "ASSOC", "ENTRY", "WDS", "CFG", "BTM", "AT2LM", "RECONFIG", "BPCC"},
	/* DBG_CAT_CCN34	25 */ {"MISC"},
	/* DBG_CAT_INTF		26 */ {"MISC", "UP", "DOWN", "UPDATE", "CTRL"},
	/* DBG_CAT_ANDLINK	27 */ {"MISC"},
	/* DBG_CAT_CFG80211	28 */ {"MISC", "AP", "STA", "CMM", "INTF"},
};

/*
    ==========================================================================
    Description:
      Get debug level by Category and SubCategory

    Return:
      Debug Level
    ==========================================================================
*/
UINT32 get_dbg_lvl_by_cat_subcat(UINT32 dbg_cat, UINT32 dbg_sub_cat)
{
	UINT32 i;
	UINT32 dbg_lvl = DBG_LVL_OFF;

	for (i = DBG_LVL_MAX; i != 0; i--) {
		if (DebugSubCategory[i][dbg_cat] & dbg_sub_cat) {
			dbg_lvl = i;
			break;
		}
	}

	return dbg_lvl;
}

/*
    ==========================================================================
    Description:
      Apply debug level to all Category and SubCategory

    Return:
      NA
    ==========================================================================
*/
void set_dbg_lvl_all(UINT32 dbg_lvl)
{
	UINT32 i, j;

	DebugLevel = dbg_lvl;
	for (i = 0; i <= DBG_LVL_MAX; i++) {
		for (j = 0; j < 32; j++)
			DebugSubCategory[i][j] =
				(i <= dbg_lvl) ? DBG_SUBCAT_EN_ALL_MASK : 0;
	}

	return;
}

/*
    ==========================================================================
    Description:
      Apply debug level to specific Category

    Return:
      NA
    ==========================================================================
*/
void set_dbg_lvl_cat(UINT32 dbg_lvl, UINT32 dbg_cat)
{
	UINT32 i;

	for (i = 0; i <= DBG_LVL_MAX; i++) {
		DebugSubCategory[i][dbg_cat] =
			(i <= dbg_lvl) ? DBG_SUBCAT_EN_ALL_MASK : 0;
	}

	return;
}

/*
    ==========================================================================
    Description:
      Apply debug level to specific Category and SubCategory

    Return:
      NA
    ==========================================================================
*/
void set_dbg_lvl_cat_subcat(UINT32 dbg_lvl, UINT32 dbg_cat, UINT32 dbg_sub_cat_mask)
{
	UINT32 i;

	for (i = 0; i <= DBG_LVL_MAX; i++) {
		if (i <= dbg_lvl)
			DebugSubCategory[i][dbg_cat] |= (dbg_sub_cat_mask);
		else
			DebugSubCategory[i][dbg_cat] &= ~(dbg_sub_cat_mask);
	}

	return;
}

/*
    ==========================================================================
    Description:
	For Debug information
	Change DebugLevel
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 dbg_lvl = 0;
	UINT32 dbg_cat = 0;
	UINT32 dbg_sub_cat = 0;
	UINT32 i;
	UINT32 j;
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;
	UCHAR para = 3;
	UCHAR show_usage_para = 0;

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	str = strsep(&arg, ":");

	if (arg == NULL) {
		para = 1;
		if (rtstrcasecmp(str, "?") == TRUE)
			show_usage_para = 1;
	} else {
		str2 = strsep(&arg, ":");
		if (arg == NULL) {
			para = 2;
			if (rtstrcasecmp(str2, "?") == TRUE)
				show_usage_para = 2;
		} else if (rtstrcasecmp(arg, "?") == TRUE)
			show_usage_para = 3;
	}

	dbg_lvl = os_str_tol(str, 0, 10);
	if (para >= 2)
		dbg_cat = os_str_tol(str2, 0, 10);
	if (para >= 3)
		dbg_sub_cat = os_str_tol(arg, 0, 10);

	if (show_usage_para == 1) {
		MTWF_PRINT("usage and current state:\n");
		for (j = 0; j < 32; j++) {
			if (cat_str[j] == NULL)
				break;
			for (i = DBG_LVL_MAX; i != 0; i--)
				if (DebugSubCategory[i][j] != 0) {
					MTWF_PRINT("%2d:%s(L%d", j, cat_str[j], i);
					if (DebugSubCategory[i][j] != DBG_SUBCAT_EN_ALL_MASK)
						MTWF_PRINT("*");
					MTWF_PRINT(")\t");
					break;
				}
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	} else if (show_usage_para == 2) {
		MTWF_PRINT("usage and current state for DebugLevel %d:\n", dbg_lvl);
		for (j = 0; j < 32; j++) {
			if (cat_str[j] == NULL)
				break;
			MTWF_PRINT("%2d:%s(0x%08x)\t", j, cat_str[j], DebugSubCategory[dbg_lvl][j]);
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	} else if (show_usage_para == 3) {
		MTWF_PRINT("usage and current state for DebugLevel %d, Category %d(%s):\n", dbg_lvl, dbg_cat, cat_str[dbg_cat]);
		for (j = 0; j < 32; j++) {
			if (sub_cat_str[dbg_cat][j] == NULL)
				break;
			MTWF_PRINT("%2d:%s(", j, sub_cat_str[dbg_cat][j]);
			if (DebugSubCategory[dbg_lvl][dbg_cat] & (0x1 << j))
				MTWF_PRINT("on)\t");
			else
				MTWF_PRINT("off)\t");
			if ((j + 1) % 4 == 0)
				MTWF_PRINT("\n");
		}

		MTWF_PRINT("\n");
		return TRUE;
	}

	if (dbg_lvl <= DBG_LVL_MAX) {
		if (para == 1) {
			set_dbg_lvl_all(dbg_lvl);
		} else if (para == 2) {
			if (dbg_cat > DBG_CAT_MAX)
				goto format_error;
			else
				set_dbg_lvl_cat(dbg_lvl, dbg_cat);
#if defined(CONFIG_WLAN_SERVICE)
			if (dbg_cat == 6) {	/* test mode */
				extern INT32 serv_dbg_lvl;

				serv_dbg_lvl = dbg_lvl;
			}
#endif	/* CONFIG_WLAN_SERVICE */
		} else if (para == 3) {
			if (dbg_sub_cat > 31)
				goto format_error;
			else
				set_dbg_lvl_cat_subcat(dbg_lvl, dbg_cat, (0x1 << dbg_sub_cat));
		}
	} else
		goto format_error;

	MTWF_PRINT("%s(): (DebugLevel = %d)\n", __func__, DebugLevel);
	return TRUE;

format_error:
	MTWF_PRINT("Format error! correct format:\n");
	MTWF_PRINT("iwpriv ra0 set Debug=[DebugLevel]:[DebugCat]:[DebugSubCat]\n");
	MTWF_PRINT("\t[DebugLevel]:0~6 or ?\n");
	MTWF_PRINT("\t[DebugCat]:0~31 or ?, optional\n");
	MTWF_PRINT("\t[DebugSubCat]:0~31 or ?, optional\n");
	MTWF_PRINT("EX: 1.iwpriv ra0 set Debug=2\n");
	MTWF_PRINT("\t DebugSubCategory[0~2][0~31] = 0xffffffff, DebugSubCategory[3~6][0~31] = 0\n");
	MTWF_PRINT("    2.iwpriv ra0 set Debug=4:5\n");
	MTWF_PRINT("\t DebugSubCategory[0~4][5] = 0xffffffff, DebugSubCategory[5~6][5] = 0\n");
	MTWF_PRINT("    3.iwpriv ra0 set Debug=3:10:7\n");
	MTWF_PRINT("\t DebugSubCategory[0~3][10] |= (0x1 << 7), DebugSubCategory[4~6][10] &= ~(0x1 << 7)\n");
	MTWF_PRINT("    4.iwpriv ra0 set Debug=?\n");
	MTWF_PRINT("\t query category list and current debuglevel value for each category\n");
	MTWF_PRINT("    5.iwpriv ra0 set Debug=3:?\n");
	MTWF_PRINT("\t query category list and current subcategory bitmap value for each category at DebugLevel 3\n");
	MTWF_PRINT("    6.iwpriv ra0 set Debug=2:8:?\n");
	MTWF_PRINT("\t query subcategory list and current subcategory on/off state for category 8 at DebugLevel 2\n");

	return FALSE;
}
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
INT Set_Max_ProbeRsp_IE_Cnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ap_probe_rsp_vendor_ie_max_count;

	ap_probe_rsp_vendor_ie_max_count = simple_strtol(arg, 0, 10);

	pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count = ap_probe_rsp_vendor_ie_max_count;

	MTWF_PRINT("%s: ap_probe_rsp_vendor_ie_max_count: %d\n",
		__func__, ap_probe_rsp_vendor_ie_max_count);
	return TRUE;
}
#endif /*CUSTOMER_VENDOR_IE_SUPPORT*/

#ifdef RATE_PRIOR_SUPPORT
INT Set_RatePrior_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG RatePrior;
	INT idx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PBLACK_STA pBlackSta = NULL, tmp;

	RatePrior = simple_strtol(arg, 0, 10);
	if (RatePrior == 1) {
		pAd->LowRateCtrl.RatePrior = 1;
		pAd->LowRateCtrl.LowRateRatioThreshold = 2;
		pAd->LowRateCtrl.LowRateCountPeriod = 5;
		pAd->LowRateCtrl.TotalCntThreshold = 50;
		pAd->LowRateCtrl.BlackListTimeout = 30;
	} else {
		pAd->LowRateCtrl.RatePrior = 0;
		/*clear the list*/
		OS_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
		DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
			MTWF_PRINT("Remove from blklist, "MACSTR"\n", MAC2STR(pBlackSta->Addr));
			tmp = pBlackSta;
			pBlackSta = DlListEntry(pBlackSta->List.Prev, BLACK_STA, List);
			DlListDel(&(tmp->List));
			os_free_mem(tmp);
		}
		OS_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
		/*clear entry info*/
		for (idx = 1; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
			pEntry = entry_get(pAd, idx);
			if (pEntry != NULL) {
				pEntry->McsTotalRxCount = 0;
				pEntry->McsLowRateRxCount = 0;
			}
		}
}

	MTWF_PRINT("%s: RatePrior: %s\n", __func__, RatePrior == 1 ? "Enable" : "Disable");
	return TRUE;
}

INT Set_BlackListTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT BlackListTimeout;

	BlackListTimeout = simple_strtol(arg, 0, 10);

	if (BlackListTimeout <= 0) {
		MTWF_PRINT("ERROR! BlackListTimeout value: %d\n",
		BlackListTimeout);
		return FALSE;
	}

	pAd->LowRateCtrl.BlackListTimeout = BlackListTimeout;
	MTWF_PRINT("%s: BlackListTimeout: %d\n", __func__, BlackListTimeout);
	return TRUE;
}


INT Set_LowRateRatio_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT LowRateRatioThreshold;

	LowRateRatioThreshold = simple_strtol(arg, 0, 10);
	if (LowRateRatioThreshold <= 0) {
		MTWF_PRINT("%s: ERROR! LowRateRatioThreshold value: %d\n",
			__func__,
			LowRateRatioThreshold);
		return FALSE;
	}

	pAd->LowRateCtrl.LowRateRatioThreshold = LowRateRatioThreshold;
	MTWF_PRINT("%s: LowRateRatioThreshold: %d\n", __func__, LowRateRatioThreshold);
	return TRUE;
}

INT Set_LowRateCountPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT LowRateCountPeriod;

	LowRateCountPeriod = simple_strtol(arg, 0, 10);
	if (LowRateCountPeriod <= 0) {
		MTWF_PRINT("%s: ERROR! LowRateCountPeriod value: %d\n",
			__func__,
			LowRateCountPeriod);
		return FALSE;
	}

	pAd->LowRateCtrl.LowRateCountPeriod = LowRateCountPeriod;
	MTWF_PRINT("%s: LowRateCountPeriod: %d\n",
		__func__, LowRateCountPeriod);
	return TRUE;
}

INT Set_TotalCntThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT TotalCntThreshold;

	TotalCntThreshold = simple_strtol(arg, 0, 10);
	if (TotalCntThreshold <= 0) {
		MTWF_PRINT("%s: ERROR! TotalCntThreshold value: %d\n",
			__func__, TotalCntThreshold);
		return FALSE;
	}

	pAd->LowRateCtrl.TotalCntThreshold = TotalCntThreshold;
	MTWF_PRINT("%s: TotalCntThreshold: %d \n",
		__func__, TotalCntThreshold);
	return TRUE;
}

#endif /*RATE_PRIOR_SUPPORT*/

/*
    ==========================================================================
    Description:
	Change DebugCategory
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DebugCategory_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 category = (UINT32)os_str_tol(arg, 0, 16);

	DebugCategory = category;
	MTWF_PRINT("%s(): Set DebugCategory = 0x%x\n", __func__, DebugCategory);
	return TRUE;
}

#ifdef DBG
#ifdef DBG_ENHANCE
/**
* Change Debug option. Enable or disable to print below information:
* debug category and level, Wi-Fi interface name, current thread ID,
* function name and line number.
*/
INT Set_DebugOption_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN prtCatLvl = 0;
	BOOLEAN prtIntfName = 0;
	BOOLEAN prtThreadId = 0;
	BOOLEAN prtFuncLine = 0;
	char *str;

	if (arg == NULL || strlen(arg) == 0)
		goto usage;

	str = strsep(&arg, ":");
	prtCatLvl = os_str_tol(str, 0, 10);

	if (arg != NULL) {
		str = strsep(&arg, ":");
		prtIntfName = os_str_tol(str, 0, 10);
	} else
		goto usage;

	if (arg != NULL) {
		str = strsep(&arg, ":");
		prtThreadId = os_str_tol(str, 0, 10);
	} else
		goto usage;

	if (arg != NULL) {
		str = strsep(&arg, ":");
		prtFuncLine = os_str_tol(str, 0, 10);
	} else
		goto usage;

	MTWF_PRINT("DebugOption=%d:%d:%d:%d\n", prtCatLvl, prtIntfName,
				prtThreadId, prtFuncLine);

	mtwf_dbg_option(prtCatLvl, prtIntfName, prtThreadId, prtFuncLine);

	return TRUE;

usage:
	MTWF_PRINT("Format error! correct format:\n");
	MTWF_PRINT("iwpriv ra0 set DebugOption=<CatLvl>:<Intf>:<ThreadId>:<FuncLine>\n");
	MTWF_PRINT("Enable (1) or disable (0) to print these in debug log:\n");
	MTWF_PRINT("  CatLvl: print the debug category and level\n");
	MTWF_PRINT("  Intf: print the Wi-Fi interface name\n");
	MTWF_PRINT("  ThreadId: print current thread ID\n");
	MTWF_PRINT("  FuncLine: print function name and line number\n");
	MTWF_PRINT("For example:\n");
	MTWF_PRINT("Enable all options : iwpriv ra0 set DebugOption=1:1:1:1\n");
	MTWF_PRINT("Disable all options: iwpriv ra0 set DebugOption=0:0:0:0\n");
	MTWF_PRINT("Enable Intf only   : iwpriv ra0 set DebugOption=0:1:0:0\n");

	return FALSE;
}
#endif /* DBG_ENHANCE */
#endif /* DBG */

static BOOLEAN ascii2hex(RTMP_STRING *in, UINT32 *out)
{
	UINT32 hex_val, val;
	CHAR *p, asc_val;

	hex_val = 0;
	p = (char *)in;

	while ((*p) != 0) {
		val = 0;
		asc_val = *p;

		if ((asc_val >= 'a') && (asc_val <= 'f'))
			val = asc_val - 87;
		else if ((*p >= 'A') && (asc_val <= 'F'))
			val = asc_val - 55;
		else if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		hex_val = (hex_val << 4) + val;
		p++;
	}

	*out = hex_val;
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Read / Write MAC
    Arguments:
	pAd                    Pointer to our adapter
	wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
	       2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *seg_str, *addr_str, *val_str, *range_str;
	RTMP_STRING *mpool, *msg;
	RTMP_STRING *arg, *ptr;
	UINT32 macVal = 0, max_len = 4096;
	BOOLEAN bFromUI, is_write, is_range;
	UINT32 IdMac, map_addr, mac_s = 0, mac_e = 0;
	BOOLEAN IsFound;
	INT ret;
	UINT LeftBufSize;

	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (4096 + 256 + 12));

	if (!mpool)
		return;

	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;
	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	arg = (RTMP_STRING *)((ULONG)(msg + 4096 + 3) & (ULONG)~0x03);
	memset(msg, 0x00, max_len);
	memset(arg, 0x00, 256);

	if (wrq->u.data.length > 1) {
#ifdef LINUX
		INT Status = NDIS_STATUS_SUCCESS;

		Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#else
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#endif /* LINUX */
		arg[255] = 0x00;
	}

	ptr = arg;

	if ((ptr != NULL) && (strlen(ptr) > 0)) {
		while ((*ptr != 0) && (*ptr == 0x20)) /* remove space */
			ptr++;

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"after trim space, ptr len=%zu, pointer(%p)=%s!\n",
			strlen(ptr), ptr, ptr);
	}

	{
		while ((seg_str = strsep((char **)&ptr, ",")) != NULL) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"seg_str[%zu]=%s\n", strlen(seg_str), seg_str);
			is_write = FALSE;
			addr_str = seg_str;
			val_str = NULL;
			val_str = strchr(seg_str, '=');

			if (val_str != NULL) {
				*val_str++ = 0;
				is_write = 1;
			} else
				is_write = 0;

			if (addr_str) {
				range_str = strchr(addr_str, '-');

				if (range_str != NULL) {
					*range_str++ = 0;
					is_range = 1;
				} else
					is_range = 0;

				if ((ascii2hex(addr_str, &mac_s) == FALSE)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"Invalid MAC CR Addr, str=%s\n", addr_str);
					break;
				}

				if (is_range) {
					if (ascii2hex(range_str, &mac_e) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"Invalid Range End MAC CR Addr[0x%x], str=%s\n",
						mac_e, range_str);
						break;
					}

					if (mac_e < mac_s) {
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"Invalid Range MAC Addr[%s - %s]=>[0x%x - 0x%x]\n",
						addr_str, range_str, mac_s, mac_e);
						break;
					}
				} else
					mac_e = mac_s;
			}

			if (val_str) {
				if ((strlen(val_str) == 0) || ascii2hex(val_str, &macVal) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"Invalid MAC value[0x%s]\n", val_str);
					break;
				}
			}

			if (is_write) {
#ifdef WIFI_UNIFIED_COMMAND
				/* WF_PHY_DSP CR via WM */
				if ((mac_s & 0x86000000) == 0x86000000) {
					RTMP_REG_PAIR dsp_reg;

					dsp_reg.Register = mac_s;
					dsp_reg.Value = macVal;
					UniCmdMultipleMacRegAccessWrite(pAd, &dsp_reg, 1);
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
						"dsp_addr=0x%08x, val=0x%08x\n",
						mac_s, macVal);
				} else
#endif
					RTMP_IO_WRITE32(pAd->hdev_ctrl, mac_s, macVal);

				/* call mt_mac_cr_range_mapping here is only for debugging purpose */
				map_addr = mac_s;
				IsFound = mt_mac_cr_range_mapping(pAd->physical_dev, &map_addr);
				LeftBufSize = max_len - strlen(msg);
				ret = snprintf(msg + strlen(msg), LeftBufSize, "[0x%04x]:%08x  ", map_addr, macVal);
				if (os_snprintf_error(LeftBufSize, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					os_free_mem(mpool);
					return;
				}
				MTWF_PRINT("MacAddr=0x%x, MacValue=0x%x, IsRemap=%d\n", map_addr, macVal, !IsFound);
			} else {
				for (IdMac = mac_s; IdMac <= mac_e; IdMac += 4) {
#ifdef WIFI_UNIFIED_COMMAND
					/* WF_PHY_DSP CR via WM */
					if ((mac_s & 0x86000000) == 0x86000000) {
						RTMP_REG_PAIR dsp_reg;

						dsp_reg.Register = IdMac;
						dsp_reg.Value = 0;
						UniCmdMultipleMacRegAccessRead(pAd, &dsp_reg, 1);
						macVal = dsp_reg.Value;
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
								"dsp_addr=0x%08x, val=0x%08x\n",
								IdMac, macVal);
					} else
#endif
						RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);

					/* call mt_mac_cr_range_mapping here is only for debugging purpose */
					map_addr = IdMac;
					IsFound = mt_mac_cr_range_mapping(
						pAd->physical_dev, &map_addr);
					LeftBufSize = max_len - strlen(msg);
					ret = snprintf(msg + strlen(msg), LeftBufSize, "[0x%04x]:%08x  ", map_addr, macVal);
					if (os_snprintf_error(LeftBufSize, ret)) {
						MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
									"Snprintf failed!\n");
						os_free_mem(mpool);
						return;
					}
					MTWF_PRINT("MacAddr=0x%x, MacValue=0x%x, IsRemap=%d\n", map_addr, macVal, !IsFound);
				}
			}

			if (ptr)
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "NextRound: ptr[%zu]=%s\n", strlen(ptr), ptr);
		}
	}

	if (strlen(msg) == 1) {
		LeftBufSize = max_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "===>Error command format!");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			os_free_mem(mpool);
			return;
		}
	}

#ifdef LINUX
	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"copy_to_user() fail\n");

#endif /* LINUX */

	os_free_mem(mpool);
}

#endif /* DBG */

#ifdef PER_PKT_CTRL_FOR_CTMR
INT set_host_pkt_ctrl_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	pAd->PerPktCtrlEnable =  os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"per_pkt_ctrl_enable is %d\n", pAd->PerPktCtrlEnable);
	return TRUE;
}
#endif

#ifdef DOT11_HE_AX
INT set_color_dbg(RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 wdev_idx, action, value;

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	if (sscanf(arg, "%d:%d:%d", &wdev_idx, &action, &value) != 3)
		goto format_error;

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_ERROR,
			"wdev_index is out of range\n");
		return FALSE;
	}
	if (action > BSS_COLOR_DBG_MAX) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_ERROR,
			"action is out of range\n");
		return FALSE;
	}

	set_bss_color_dbg(ad, (UINT8)wdev_idx, (UINT8)action, (UINT8)value);
	return TRUE;

format_error:
	MTWF_PRINT("iwpriv ra0 set color_dbg=[wdev_idx]:[action]:[value]\n");
	MTWF_PRINT("[action] 1:occupy, 2:setperiod, 3:trigger, 4: change\n");
	MTWF_PRINT("         5:assign manually 6:change manually\n");
	return FALSE;
}


/*
 *   ==========================================================================
 *   Description:
 *   Set TXOP Duration based RTS Threshold
 *   Return:
 *   TRUE if all parameters are OK, FALSE otherwise
 *   ==========================================================================
 */
VOID set_txop_dur_rts_thld(struct wifi_dev *wdev, UINT16 txop_dur_thld)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	BOOLEAN ie_update = FALSE;

	if (wlan_config_get_he_txop_dur_rts_thld(wdev) != txop_dur_thld)
		ie_update = TRUE;

	if (wlan_operate_get_he_txop_dur_rts_thld(wdev) != txop_dur_thld)
		ie_update = TRUE;

	if (ie_update) {
		if ((wdev->wdev_type == WDEV_TYPE_AP) &&
			(bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_HEOP) == FALSE))
			MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

		wlan_config_set_he_txop_dur_rts_thld(wdev, txop_dur_thld);
		wlan_operate_set_he_txop_dur_rts_thld(wdev, txop_dur_thld);
		HW_SET_PROTECT(ad, wdev, PROT_TXOP_DUR_BASE, 0, 0);
		if (wdev->wdev_type == WDEV_TYPE_AP)
			UpdateBeaconHandler_BPCC(ad, wdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_HEOP, TRUE);
	}
}

INT set_txop_duration_prot_threshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 txop_dur_thld;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO,
			 CATPROTO_HE,
			 DBG_LVL_ERROR,
			 "Usage:\niwpriv raN set txop_rts_thld=[txop_duration_threshold]\n");
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	txop_dur_thld = os_str_tol(arg, 0, 10);
	if (txop_dur_thld > MAX_TXOP_DURATION_RTS_THRESHOLD) {
		MTWF_DBG(pAd, DBG_CAT_PROTO,
			 CATPROTO_HE,
			 DBG_LVL_ERROR,
			 "incorrect value:%d\n", txop_dur_thld);
		return FALSE;
	}

	set_txop_dur_rts_thld(wdev, txop_dur_thld);
	MTWF_PRINT("%s: set wdev%d txop_duration rts threshold=%d (0x%x)\n",
			__func__,
			wdev->wdev_idx,
			txop_dur_thld,
			txop_dur_thld);
	return TRUE;
}

#endif

#ifdef RANDOM_PKT_GEN
INT Set_TxCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->set_txctrl_proc)
		return chip_dbg->set_txctrl_proc(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

VOID regular_pause_umac(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->regular_pause_umac)
		chip_dbg->regular_pause_umac(pAd->hdev_ctrl);
}
#endif
#ifdef CSO_TEST_SUPPORT
INT32 CsCtrl;
INT Set_CsCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	CsCtrl = os_str_tol(arg, 0, 10);

	if (CsCtrl & BIT0)
		MTWF_PRINT("IPV4 checksum overwrite enable!\n");

	if (CsCtrl & BIT1)
		MTWF_PRINT("TCP checksum overwrite enable!\n");

	if (CsCtrl & BIT2)
		MTWF_PRINT("UDP checksum overwrite enable!\n");

	if (CsCtrl & BIT3)
		MTWF_PRINT("Tx Debug log enable!\n");

	if (CsCtrl & BIT4)
		MTWF_PRINT("Rx Debug log enable!\n");

	return TRUE;
}
#endif

INT	Show_WifiSysInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	wifi_sys_dump(pAd);
	return TRUE;
}

INT Show_DescInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef RTMP_MAC_PCI
	INT32 i, resource_idx;
	RXD_STRUC *pRxD;
	TXD_STRUC *pTxD;
#ifdef CFG_BIG_ENDIAN
	RXD_STRUC *pDestRxD, RxD;
	TXD_STRUC *pDestTxD, TxD;
#endif /* CFG_BIG_ENDIAN */
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring;
	struct hif_pci_rx_ring *rx_ring;
	PUCHAR pDMAHeaderBufVA;
	RTMP_STRING *dir;

	if (arg != NULL && strlen(arg)) {
		dir = strsep(&arg, ":");
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg)) {
		resource_idx = os_str_tol(arg, 0, 10);
	} else {
		goto err;
	}

	if (rtstrcasecmp(dir, "tx")) {
		if (resource_idx < hif->tx_res_num) {
			UINT16 tx_ring_size;
			tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
			tx_ring_size = tx_ring->ring_size;
			MTWF_PRINT("Tx Ring %d ---------------------------------\n", resource_idx);

			for (i = 0; i < tx_ring->ring_size; i++) {
				pDMAHeaderBufVA = (UCHAR *)tx_ring->Cell[i].DmaBuf.AllocVa;
#ifdef CFG_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)tx_ring->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				if (pDMAHeaderBufVA)
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
				else
					MTWF_PRINT("pkt is null\n");

#else
				pTxD = (TXD_STRUC *)tx_ring->Cell[i].AllocVa;
#endif /* CFG_BIG_ENDIAN */
				MTWF_PRINT("Desc #%d\n", i);

				if (pTxD)
					dump_txd(pAd, pTxD);
				else
					MTWF_PRINT("TXD null!!\n");

				if (pDMAHeaderBufVA) {
					asic_dump_tmac_info(pAd, pDMAHeaderBufVA);
					MTWF_PRINT("pkt physical address = %x\n",
							 (UINT32)tx_ring->Cell[i].PacketPa);
#ifdef CFG_BIG_ENDIAN
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
				} else
					MTWF_PRINT("pkt is null\n");
			}
		} else {
			MTWF_PRINT("Tx resource_idx %d out of range\n", resource_idx);
		}
	} else if (rtstrcasecmp(dir, "rx")) {
		if (resource_idx < hif->rx_res_num) {
			UINT16 RxRingSize;
			rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
			RxRingSize = rx_ring->ring_size;
			MTWF_PRINT("Rx Ring %d ---------------------------------\n", resource_idx);

			for (i = 0; i < RxRingSize; i++) {
#ifdef CFG_BIG_ENDIAN
				pDestRxD = (RXD_STRUC *)rx_ring->Cell[i].AllocVa;
				RxD = *pDestRxD;
				pRxD = &RxD;
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
				pRxD = (RXD_STRUC *)rx_ring->Cell[i].AllocVa;
#endif /* CFG_BIG_ENDIAN */
				MTWF_PRINT("Desc #%d\n", i);

				if (pRxD) {
					dump_rxd(pAd, pRxD);
					MTWF_PRINT("pRxD->DDONE = %x\n", pRxD->DDONE);
				} else
					MTWF_PRINT("RXD null!!\n");
			}
		} else {
			MTWF_PRINT("Rx resource_idx %d out of range\n", resource_idx);
		}
	} else {
		goto err;
	}
	return TRUE;

err:
	MTWF_PRINT ("Usage:   iwpriv $(inf_name) show descinfo=$(tx/rx):$(resource_idx)\n");
	MTWF_PRINT ("Example: iwpriv ra0 show descinfo=tx:0\n");
#endif /* RTMP_MAC_PCI */
	return FALSE;
}

#ifdef TXRX_STAT_SUPPORT
static VOID reset_txrx_stat_cnt(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	RADIO_CTRL *pRadioCtrl = ctrl->rdev.pRadioCtrl;

	asic_get_mib_txrx_cnts(pAd, hc_get_hw_band_idx(pAd));

	pAd->WlanCounters.LastRxTotByteCount  = pAd->WlanCounters.RxTotByteCount;
	pAd->WlanCounters.LastTxMgmtRetryCnt  = pAd->WlanCounters.TxMgmtRetryCnt;
	pAd->WlanCounters.LastRxDupDropCount  = pAd->WlanCounters.RxDupDropCount;
	pAd->WlanCounters.LastRxBaCnt         = pAd->WlanCounters.RxBaCnt;
	pAd->WlanCounters.LastRTSTotalCnt     = pAd->WlanCounters.RTSTotalCnt;
	pAd->WlanCounters.LastRTSFailureCount = pAd->WlanCounters.RTSFailureCount;
	pAd->WlanCounters.LastTxBaCnt         = pAd->WlanCounters.TxBaCnt;
	pAd->WlanCounters.LastRxFcsOkCount    = pAd->WlanCounters.RxFcsOkCount;
	pAd->WlanCounters.LastACKFailureCount = pAd->WlanCounters.ACKFailureCount;
	pAd->WlanCounters.LastTxAmpduCnt      = pAd->WlanCounters.TxAmpduCnt;
	pAd->WlanCounters.LastTxAbortCnt      = pAd->WlanCounters.TxAbortCnt;
	pAd->WlanCounters.LastTxRtsDropCnt    = pAd->WlanCounters.TxRtsDropCnt;
	pAd->WlanCounters.LastTxCtrlCnt       = pAd->WlanCounters.TxCtrlCnt;
	pAd->WlanCounters.LastRxBeaconCnt     = pAd->WlanCounters.RxBeaconCnt;
	pAd->WlanCounters.LastMultipleRetryCount = pAd->WlanCounters.MultipleRetryCount;
	pAd->WlanCounters.LastRxMpduInAmpduCount = pAd->WlanCounters.RxMpduInAmpduCount;
	pAd->WlanCounters.LastRxOutOfRangeCount  = pAd->WlanCounters.RxOutOfRangeCount;
	pAd->WlanCounters.LastTxMpduRetryDropCnt = pAd->WlanCounters.TxMpduRetryDropCnt;
	pAd->WlanCounters.LastRxDelimiterFailCount  = pAd->WlanCounters.RxDelimiterFailCount;
	pAd->WlanCounters.LastRxFilterDropMpduCount = pAd->WlanCounters.RxFilterDropMpduCount;
	pAd->WlanCounters.LastRxPhy2MacLenMismatchCount = pAd->WlanCounters.RxPhy2MacLenMismatchCount;
	pAd->WlanCounters.LastRxFcsErrorCount = pAd->WlanCounters.RxFcsErrorCount;
	pAd->WlanCounters.LastRxFifoFullCount = pAd->WlanCounters.RxFifoFullCount;
	pAd->WlanCounters.LastRxMpduCount = pAd->WlanCounters.RxMpduCount;

	/*============= Clear BSS Stats ================*/
	pAd->ApCfg.MBSSID[apidx].TxCount = 0;
	pAd->ApCfg.MBSSID[apidx].RxCount = 0;
	pAd->ApCfg.MBSSID[apidx].TransmittedByteCount = 0;
	pAd->ApCfg.MBSSID[apidx].ReceivedByteCount = 0;
	pAd->ApCfg.MBSSID[apidx].RxErrorCount = 0;
	pAd->ApCfg.MBSSID[apidx].RxDropCount = 0;
	pAd->ApCfg.MBSSID[apidx].TxErrorCount = 0;
	pAd->ApCfg.MBSSID[apidx].TxDropCount = 0;
	pAd->ApCfg.MBSSID[apidx].ucPktsTx = 0;
	pAd->ApCfg.MBSSID[apidx].ucPktsRx = 0;
	pAd->ApCfg.MBSSID[apidx].mcPktsTx = 0;
	pAd->ApCfg.MBSSID[apidx].mcPktsRx = 0;
	pAd->ApCfg.MBSSID[apidx].bcPktsTx = 0;
	pAd->ApCfg.MBSSID[apidx].bcPktsRx = 0;
	pRadioCtrl->RxDataPacketCount.QuadPart = 0;
	pRadioCtrl->RxDataPacketByte.QuadPart = 0;
	pRadioCtrl->TxDataPacketCount.QuadPart = 0;
	pRadioCtrl->TxDataPacketByte.QuadPart = 0;
	pRadioCtrl->TxMulticastDataPacket.QuadPart = 0;
	pRadioCtrl->TxUnicastDataPacket.QuadPart = 0;
	pRadioCtrl->TxBroadcastDataPacket.QuadPart = 0;
	pRadioCtrl->TxMgmtPacketCount.QuadPart = 0;
	pRadioCtrl->RxMgmtPacketCount.QuadPart = 0;
	pRadioCtrl->TxMgmtPacketByte.QuadPart = 0;
	pRadioCtrl->RxMgmtPacketByte.QuadPart = 0;
	pRadioCtrl->TxBeaconPacketCount.QuadPart = pAd->BcnCheckInfo.totalbcncnt;
	pRadioCtrl->RxDecryptionErrorCount.QuadPart = 0;
	pRadioCtrl->RxMICErrorCount.QuadPart = 0;

	/*============= Clear Rate Report ================*/
	NdisZeroMemory(&pAd->TxRxRateReport, sizeof(struct COUNTER_TXRX_RATE_REPORT));

}
#endif /* TXRX_STAT_SUPPORT */

/*
    ==========================================================================
    Description:
	Reset statistics counter

    Arguments:
	pAd            Pointer to our adapter
	arg

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 ucBand = hc_get_hw_band_idx(pAd);
	LARGE_INTEGER RxFcsErrorCount, RxFifoFullCount, RxMpduCount;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "==>\n");
	/* add the most up-to-date h/w raw counters into software counters*/
	NICUpdateRawCountersNew(pAd);

	RxFcsErrorCount = pAd->WlanCounters.RxFcsErrorCount;
	RxFifoFullCount = pAd->WlanCounters.RxFifoFullCount;
	RxMpduCount = pAd->WlanCounters.RxMpduCount;

	NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
	pAd->WlanCounters.LastRxFcsErrorCount = RxFcsErrorCount;
	pAd->WlanCounters.LastRxFifoFullCount = RxFifoFullCount;
	pAd->WlanCounters.LastRxMpduCount = RxMpduCount;

	NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
	NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));
	pAd->mcli_ctl.last_tx_cnt = 0;
	pAd->mcli_ctl.last_tx_fail_cnt = 0;

	pAd->WlanCounters.APAckCount.QuadPart = 0;
	pAd->WlanCounters.APCliAckCount.QuadPart = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	{
		/* clear TX success/fail count in MCU */
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBand, 0, &rTxStatResult);
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef CONFIG_WLAN_SERVICE
	/* Clear TX success count in ATE mode */
	if (ATE_ON(pAd)) {
		struct service *serv = &pAd->serv;
		struct service_test *serv_test = (struct service_test *)serv->serv_handle;

		os_zero_mem(&serv_test->test_rx_statistic[TESTMODE_GET_BAND_IDX(pAd)], sizeof(struct test_rx_stat));
		TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), ATE_TXDONE_CNT, 0);
	}
#endif /* CONFIG_WLAN_SERVICE */
#ifdef TXBF_SUPPORT
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->FlgHwTxBfCap) {
		int i;
		struct _MAC_TABLE_ENTRY *entry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			entry = entry_get(pAd, i);
			NdisZeroMemory(&entry->TxBFCounters, sizeof(entry->TxBFCounters));
		}
	}
}
#endif /* TXBF_SUPPORT */
#ifdef TXRX_STAT_SUPPORT
	reset_txrx_stat_cnt(pAd);
#endif /* TXRX_STAT_SUPPORT */

	return TRUE;
}

#ifdef WIFI_UNIFIED_COMMAND
INT Set_AckCnt_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR val  = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return -EINVAL;

	/* Enable RX could receive ack pkt to Host */
	if (val == 1) {
		UniCmdSetRxAckToHost(pAd, wdev, TRUE);
		pAd->AckCntEnable = val;
	}

	/* Disable RX could receive ack pkt to Host */
	else if (val == 0) {
		UniCmdSetRxAckToHost(pAd, wdev, FALSE);
		pAd->AckCntEnable = val;
	}

	else
		return FALSE;

	return TRUE;
}
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WIFI_UNIFIED_COMMAND
INT Set_Bar_Retry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR val  = os_str_tol(arg, 0, 10);
	/* Set BAR retry cnt limit */
	UniCmdSetBarRetry(pAd, (UINT8)val);

	return TRUE;
}
#endif /* WIFI_UNIFIED_COMMAND */


BOOLEAN RTMPCheckStrPrintAble(
	IN  CHAR *pInPutStr,
	IN  UCHAR strLen)
{
	UCHAR i = 0;

	for (i = 0; i < strLen; i++) {
		if ((pInPutStr[i] < 0x20) || (pInPutStr[i] > 0x7E))
			return FALSE;
	}

	return TRUE;
}

/*
	========================================================================

	Routine Description:
		Remove WPA Key process

	Arguments:
		pAd					Pointer to our adapter
		pBuf							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
#ifdef CONFIG_STA_SUPPORT
VOID RTMPSetDesiredRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, LONG Rates)
{
	NDIS_802_11_RATES aryRates;

	memset(&aryRates, 0x00, sizeof(NDIS_802_11_RATES));

	switch (wdev->PhyMode) {
	case (UCHAR)(WMODE_A): /* A only*/
		switch (Rates) {
		case 6000000: /*6M*/
			aryRates[0] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 9000000: /*9M*/
			aryRates[0] = 0x12; /* 9M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 12000000: /*12M*/
			aryRates[0] = 0x18; /* 12M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 18000000: /*18M*/
			aryRates[0] = 0x24; /* 18M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 24000000: /*24M*/
			aryRates[0] = 0x30; /* 24M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_4;
			break;

		case 36000000: /*36M*/
			aryRates[0] = 0x48; /* 36M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_5;
			break;

		case 48000000: /*48M*/
			aryRates[0] = 0x60; /* 48M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_6;
			break;

		case 54000000: /*54M*/
			aryRates[0] = 0x6c; /* 54M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_7;
			break;

		case -1: /*Auto*/
		default:
			aryRates[0] = 0x6c; /* 54Mbps*/
			aryRates[1] = 0x60; /* 48Mbps*/
			aryRates[2] = 0x48; /* 36Mbps*/
			aryRates[3] = 0x30; /* 24Mbps*/
			aryRates[4] = 0x24; /* 18M*/
			aryRates[5] = 0x18; /* 12M*/
			aryRates[6] = 0x12; /* 9M*/
			aryRates[7] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			break;
		}

		break;

	case (UCHAR)(WMODE_B | WMODE_G): /* B/G Mixed*/
	case (UCHAR)(WMODE_B): /* B only*/
	case (UCHAR)(WMODE_A | WMODE_B | WMODE_G): /* A/B/G Mixed*/
	default:
		switch (Rates) {
		case 1000000: /*1M*/
			aryRates[0] = 0x02;
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 2000000: /*2M*/
			aryRates[0] = 0x04;
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 5000000: /*5.5M*/
			aryRates[0] = 0x0b; /* 5.5M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 11000000: /*11M*/
			aryRates[0] = 0x16; /* 11M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 6000000: /*6M*/
			aryRates[0] = 0x0c; /* 6M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_0;
			break;

		case 9000000: /*9M*/
			aryRates[0] = 0x12; /* 9M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_1;
			break;

		case 12000000: /*12M*/
			aryRates[0] = 0x18; /* 12M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_2;
			break;

		case 18000000: /*18M*/
			aryRates[0] = 0x24; /* 18M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_3;
			break;

		case 24000000: /*24M*/
			aryRates[0] = 0x30; /* 24M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_4;
			break;

		case 36000000: /*36M*/
			aryRates[0] = 0x48; /* 36M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_5;
			break;

		case 48000000: /*48M*/
			aryRates[0] = 0x60; /* 48M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_6;
			break;

		case 54000000: /*54M*/
			aryRates[0] = 0x6c; /* 54M*/
			wdev->DesiredTransmitSetting.field.MCS = MCS_7;
			break;

		case -1: /*Auto*/
		default:
			if (wdev->PhyMode == WMODE_B) {
				/*B Only*/
				aryRates[0] = 0x16; /* 11Mbps*/
				aryRates[1] = 0x0b; /* 5.5Mbps*/
				aryRates[2] = 0x04; /* 2Mbps*/
				aryRates[3] = 0x02; /* 1Mbps*/
			} else {
				/*(B/G) Mixed or (A/B/G) Mixed*/
				aryRates[0] = 0x6c; /* 54Mbps*/
				aryRates[1] = 0x60; /* 48Mbps*/
				aryRates[2] = 0x48; /* 36Mbps*/
				aryRates[3] = 0x30; /* 24Mbps*/
				aryRates[4] = 0x16; /* 11Mbps*/
				aryRates[5] = 0x0b; /* 5.5Mbps*/
				aryRates[6] = 0x04; /* 2Mbps*/
				aryRates[7] = 0x02; /* 1Mbps*/
			}

			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			break;
		}

		break;
	}

	NdisZeroMemory(wdev->rate.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisMoveMemory(wdev->rate.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			 " RTMPSetDesiredRates (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
			  wdev->rate.DesireRate[0], wdev->rate.DesireRate[1],
			  wdev->rate.DesireRate[2], wdev->rate.DesireRate[3],
			  wdev->rate.DesireRate[4], wdev->rate.DesireRate[5],
			  wdev->rate.DesireRate[6], wdev->rate.DesireRate[7]);
	/* Changing DesiredRate may affect the MAX TX rate we used to TX frames out*/
	MlmeUpdateTxRates(pAd, FALSE, 0);
}
#endif /* CONFIG_STA_SUPPORT */

#if defined(CONFIG_STA_SUPPORT)
NDIS_STATUS RTMPWPARemoveKeyProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_REMOVE_KEY pKey;
	ULONG					KeyIdx;
	NDIS_STATUS			Status = NDIS_STATUS_FAILURE;
	BOOLEAN	bTxKey;		/* Set the key as transmit key*/
	BOOLEAN	bPairwise;		/* Indicate the key is pairwise key*/
	BOOLEAN	bKeyRSC;		/* indicate the receive  SC set by KeyRSC value.*/
	/* Otherwise, it will set by the NIC.*/
	BOOLEAN	bAuthenticator; /* indicate key is set by authenticator.*/
	INT		i;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "--->\n");
	pKey = (PNDIS_802_11_REMOVE_KEY) pBuf;
	KeyIdx = pKey->KeyIndex & 0xff;
	/* Bit 31 of Add-key, Tx Key*/
	bTxKey = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	/* Bit 30 of Add-key PairwiseKey*/
	bPairwise = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
	/* Bit 29 of Add-key KeyRSC*/
	bKeyRSC = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	/* Bit 28 of Add-key Authenticator*/
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

	/* 1. If bTx is TRUE, return failure information*/
	if (bTxKey == TRUE)
		return NDIS_STATUS_INVALID_DATA;

	/* 2. Check Pairwise Key*/
	if (bPairwise) {
		/* a. If BSSID is broadcast, remove all pairwise keys.*/
		/* b. If not broadcast, remove the pairwise specified by BSSID*/
		for (i = 0; i < SHARE_KEY_NUM; i++) {
#ifdef CONFIG_STA_SUPPORT
			if (MAC_ADDR_EQUAL(pAd->SharedKey[BSS0][i].BssId, pKey->BSSID)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
					"(KeyIdx=%d)\n", i);
				pAd->SharedKey[BSS0][i].KeyLen = 0;
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
				AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)i);
				Status = NDIS_STATUS_SUCCESS;
				break;
			}
#endif/*CONFIG_STA_SUPPORT*/
		}
	}
	/* 3. Group Key*/
	else {
		/* a. If BSSID is broadcast, remove all group keys indexed*/
		/* b. If BSSID matched, delete the group key indexed.*/
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"(KeyIdx=%ld)\n", KeyIdx);
		pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx);
		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;
}
#endif /* defined(CONFIG_STA_SUPPORT)*/

#ifdef CONFIG_STA_SUPPORT
/*
	========================================================================

	Routine Description:
		Remove All WPA Keys

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPWPARemoveAllKeys(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	ASIC_SEC_INFO Info = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);

	if (!pEntry)
		return;

	/* clear the cipher flag to prevent Rx ICV Error log flooding */
	if (!IS_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher))
		SET_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher);

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
	Info.Wcid = pEntry->wcid;
	HW_ADDREMOVE_KEYTABLE(pAd, &Info);
}
#endif /* CONFIG_STA_SUPPORT */

/*
	========================================================================
	Routine Description:
		Change NIC PHY mode. Re-association may be necessary

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID RTMPSetPhyMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, USHORT phymode)
{
	INT i;
	UCHAR Channel = (wdev->channel) ? wdev->channel : HcGetRadioChannel(pAd);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/* sanity check user setting*/
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (Channel == pChCtrl->ChList[i].Channel)
			break;
	}

	if (i == pChCtrl->ChListNum) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

		if (Channel != 0)
			Channel = FirstChannel(pAd, wdev);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		Channel = FirstChannel(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"channel out of range, use first ch=%d\n",
			Channel);
		wdev->channel = Channel;
		wlan_operate_set_prim_ch(wdev, wdev->channel);
	}

	MlmeUpdateTxRatesWdev(pAd, FALSE, wdev);
	/* CFG_TODO */
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
}

VOID RTMPUpdateRateInfo(
	USHORT phymode,
	struct dev_rate_info *rate
)
{
	struct legacy_rate *legacy_rate = &rate->legacy_rate;

	NdisZeroMemory(legacy_rate->sup_rate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(legacy_rate->ext_rate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	switch (phymode) {
	case (WMODE_B):
		legacy_rate->sup_rate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate */
		legacy_rate->sup_rate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate */
		legacy_rate->sup_rate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate */
		legacy_rate->sup_rate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate */
		legacy_rate->sup_rate_len = 4;
		legacy_rate->ext_rate_len = 0;
		rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
		/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_CCK;  This MODE is only FYI. not use*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_0;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_CCK;
		break;

	/*
		In current design, we will put supported/extended rate element in
		beacon even we are 11n-only mode.
		Or some 11n stations will not connect to us if we do not put
		supported/extended rate element in beacon.
	*/
	case (WMODE_G):
	case (WMODE_B | WMODE_G):
	case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
	case (WMODE_GN):
	case (WMODE_G | WMODE_GN):
	case (WMODE_B | WMODE_G | WMODE_GN):
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN | WMODE_AC):
#endif
#ifdef DOT11_HE_AX
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G):
#endif
#ifdef DOT11_EHT_BE
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_BE_24G):
#endif
		legacy_rate->sup_rate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[4]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[5]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[6]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate_len = 8;
		legacy_rate->ext_rate[0]  = 0x0C;	  /* 6 mbps, in units of 0.5 Mbps*/
		legacy_rate->ext_rate[1]  = 0x18;	  /* 12 mbps, in units of 0.5 Mbps*/
		legacy_rate->ext_rate[2]  = 0x30;	  /* 24 mbps, in units of 0.5 Mbps*/
		legacy_rate->ext_rate[3]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
		legacy_rate->ext_rate_len = 4;
		rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[4]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[5]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[6]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[7]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[8]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[9]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[10] = 96;    /* 48 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[11] = 108;   /* 54 mbps, in units of 0.5 Mbps*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_0;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_CCK;
		break;

	case (WMODE_A):
#ifdef DOT11_N_SUPPORT
	case (WMODE_AN):
	case (WMODE_A | WMODE_AN):
	case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	case (WMODE_A | WMODE_AN | WMODE_AC):
	case (WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G):
	case (WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G):
#endif
#ifdef DOT11_EHT_BE
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_BE_5G):
	case (WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G | WMODE_BE_6G):
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G	| WMODE_BE_5G | WMODE_BE_6G):
#endif
		legacy_rate->sup_rate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
		legacy_rate->sup_rate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
		legacy_rate->sup_rate_len = 8;
		legacy_rate->ext_rate_len = 0;
		rate->DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
		rate->DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/
		/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_OFDM;  This MODE is only FYI. not use*/
		/*update MlmeTransmit rate*/
		rate->MlmeTransmit.field.MCS = MCS_RATE_6;
		rate->MlmeTransmit.field.BW = BW_20;
		rate->MlmeTransmit.field.MODE = MODE_OFDM;
		break;
	default:
		break;
	}
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
VOID rtmpeapupdaterateinfo(
	USHORT phymode,
	struct dev_rate_info *rate,
	struct dev_eap_info *eap
)
{
	INT i;
	struct legacy_rate *eap_legacy_rate = &eap->eap_legacy_rate;
	struct legacy_rate *legacy_rate = &rate->legacy_rate;

	if (!WMODE_CAP_AX(phymode))
		eap->eap_hesuprate_en = FALSE;
	if (!WMODE_CAP_AC(phymode))
		eap->eap_vhtsuprate_en = FALSE;
	if (!WMODE_CAP_N(phymode))
		eap->eap_htsuprate_en = FALSE;

	if (eap->eap_suprate_en != TRUE)
		return;

	NdisZeroMemory(legacy_rate->sup_rate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(legacy_rate->ext_rate, MAX_LEN_OF_SUPPORTED_RATES);

	switch (phymode) {
	case (WMODE_B):
		for (i = 0; i < eap_legacy_rate->sup_rate_len; i++)
			legacy_rate->sup_rate[i] = eap_legacy_rate->sup_rate[i] | 0x80;

		legacy_rate->sup_rate_len = eap_legacy_rate->sup_rate_len;
		break;
	case (WMODE_G):
	case (WMODE_B | WMODE_G):
	case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
	case (WMODE_GN):
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC):
	case (WMODE_B | WMODE_G | WMODE_GN):
	case (WMODE_G | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_EHT_BE
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_BE_24G):
	case (WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_AX_6G
				| WMODE_BE_24G | WMODE_BE_6G):
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_A
				| WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G
				| WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G):
#endif /* DOT11_EHT_BE */
		for (i = 0; i < eap_legacy_rate->sup_rate_len; i++)
			legacy_rate->sup_rate[i] = eap_legacy_rate->sup_rate[i];

		legacy_rate->sup_rate_len = eap_legacy_rate->sup_rate_len;

		for (i = 0; i < eap_legacy_rate->ext_rate_len; i++)
			legacy_rate->ext_rate[i] = eap_legacy_rate->ext_rate[i];

		legacy_rate->ext_rate_len = eap_legacy_rate->ext_rate_len;

		break;

	case (WMODE_A):
#ifdef DOT11_N_SUPPORT
	case (WMODE_A | WMODE_AN):
	case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
	case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	case (WMODE_A | WMODE_AN | WMODE_AC):
	case (WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G):
#endif
#ifdef DOT11_EHT_BE
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_BE_5G):
	case (WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G | WMODE_BE_6G):
	case (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G
			| WMODE_BE_5G | WMODE_BE_6G):
#endif /* DOT11_EHT_BE */
		for (i = 0; i < eap_legacy_rate->sup_rate_len; i++)
			legacy_rate->sup_rate[i] = eap_legacy_rate->sup_rate[i];

		legacy_rate->sup_rate_len = eap_legacy_rate->sup_rate_len;
		legacy_rate->ext_rate_len = 0;
		break;
#ifdef DOT11_HE_AX
	case (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G):
		for (i = 0; i < eap_legacy_rate->sup_rate_len; i++)
			legacy_rate->sup_rate[i] = eap_legacy_rate->sup_rate[i];

		legacy_rate->sup_rate_len = eap_legacy_rate->sup_rate_len;

		for (i = 0; i < eap_legacy_rate->ext_rate_len; i++)
			legacy_rate->ext_rate[i] = eap_legacy_rate->ext_rate[i];

		legacy_rate->ext_rate_len = eap_legacy_rate->ext_rate_len;

		break;
#endif

	default:
		break;
	}

}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef GN_MIXMODE_SUPPORT
VOID RTMPUpdateGNRateInfo(
	USHORT phymode,
	struct dev_rate_info *rate
	)
{
	struct legacy_rate *legacy_rate = &rate->legacy_rate;

	NdisZeroMemory(legacy_rate->sup_rate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(legacy_rate->ext_rate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

	legacy_rate->sup_rate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
	legacy_rate->sup_rate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
	legacy_rate->sup_rate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
	legacy_rate->sup_rate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
	legacy_rate->sup_rate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
	legacy_rate->sup_rate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
	legacy_rate->sup_rate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
	legacy_rate->sup_rate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
	legacy_rate->sup_rate_len = 8;
	legacy_rate->ext_rate_len = 0;

	rate->DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
	rate->DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/

	/*update MlmeTransmit rate*/
	rate->MlmeTransmit.field.MCS = MCS_RATE_6;
	rate->MlmeTransmit.field.BW = BW_20;
	rate->MlmeTransmit.field.MODE = MODE_OFDM;

}
#endif /* GN_MIXMODE_SUPPORT */

/* WTBL Test */
INT set_assign_wcid_proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT16 assignWcid;

	assignWcid = os_str_tol(arg, 0, 10);
	pAd->assignWcid = assignWcid;

	MTWF_PRINT("assignWcid = %d\n", assignWcid);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Parse encryption type
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
    ==========================================================================
*/
RTMP_STRING *GetEncryptType(CHAR enc)
{
	if (enc == Ndis802_11WEPDisabled)
		return "NONE";

	if (enc == Ndis802_11WEPEnabled)
		return "WEP";

	if (enc == Ndis802_11TKIPEnable)
		return "TKIP";

	if (enc == Ndis802_11AESEnable)
		return "AES";

	if (enc == Ndis802_11TKIPAESMix)
		return "TKIPAES";

	else
		return "UNKNOW";
}

RTMP_STRING *GetAuthMode(CHAR auth)
{
	if (auth == Ndis802_11AuthModeOpen)
		return "OPEN";

	if (auth == Ndis802_11AuthModeShared)
		return "SHARED";

	if (auth == Ndis802_11AuthModeAutoSwitch)
		return "AUTOWEP";

	if (auth == Ndis802_11AuthModeWPA)
		return "WPA";

	if (auth == Ndis802_11AuthModeWPAPSK)
		return "WPAPSK";

	if (auth == Ndis802_11AuthModeWPANone)
		return "WPANONE";

	if (auth == Ndis802_11AuthModeWPA2)
		return "WPA2";

	if (auth == Ndis802_11AuthModeWPA2PSK)
		return "WPA2PSK";

	if (auth == Ndis802_11AuthModeWPA1WPA2)
		return "WPA1WPA2";

	if (auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		return "WPA1PSKWPA2PSK";

	return "UNKNOW";
}

/*
    ==========================================================================
    Description:
	Get site survey results
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) UI needs to wait 4 seconds after issue a site survey command
			2.) iwpriv ra0 get_site_survey
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
/*
 * Max string length of site survey data = 140
 * 140 = No (4) + Channel (4) + SSID (33) + Bssid (20) + Security (23) + Signal (9) +
 *       WiressMode (11) + ExtCh (7) + MaxBW (5) + NetworkType (3) + SSID_LEN (8) + BcnRept (10) +
 *		 MWDSCap (8)
 */
#define	LINE_LEN	(4+4+33+20+23+8+9+11+7+3+8+10+8)	/* No+Channel+SSID+Bssid+Security+RSSI+Signal+WiressMode+ExtCh+NetworkType+LEN+BcnRept+MWDSCap*
*/
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
#define	WPS_LINE_LEN	(4+5)	/* WPS+DPID*/
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
#define DOT11R_LINE_LEN	(5+9+10)	/* MDId+FToverDS+RsrReqCap*/
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef TR181_SUPPORT
#define TR181_LINE_LEN	(4+5+(3*12)+(3*12)+4+4+2)	/* Mode+Rssi+SupRate+ExtRate+BCN+DTIM */
#endif /* TR181_SUPPORT */

UCHAR GetNeighborAPOperatingBw(
	IN	BSS_ENTRY * pBss)
{
	UCHAR op_bw;

	/*BW_20 check*/
	if (pBss->Channel == pBss->CentralChannel)
		return BW_20;

	op_bw = BW_40;

	/*VHT 80M/160M/80M+80M check*/
	if (HAS_VHT_OP_EXIST(pBss->ie_exists)) {
		UCHAR op_vht_bw = pBss->vht_op_ie.vht_op_info.ch_width;
		UCHAR op_vht_ccfs1 = pBss->vht_op_ie.vht_op_info.ccfs_1;
		UCHAR op_vht_ccfs0 = pBss->vht_op_ie.vht_op_info.ccfs_0;

		/*follow latest spec, 80M/160M/80M+80M, op_vht_bw is 1*/
		/*need check ccfs1/ccfs0*/
		if (op_vht_bw == VHT_BW_80) {
			op_bw = BW_80;
			if (op_vht_ccfs1 != 0) {
				if (((op_vht_ccfs1 - op_vht_ccfs0) == 8) ||
					((op_vht_ccfs0 - op_vht_ccfs1) == 8))
					op_bw = BW_160;
				else if (((op_vht_ccfs1 - op_vht_ccfs0) > 16) ||
					((op_vht_ccfs0 - op_vht_ccfs1) > 16))
					op_bw = BW_8080;
			}
		}
	} else if (HAS_VHT_CAPS_EXIST(pBss->ie_exists)) {
		if (pBss->vht_cap_ie.vht_cap.ch_width == 0)
			op_bw = BW_80;
		else
			op_bw = BW_160;
	}

	/*6G HE op IE check*/
#ifdef DOT11_HE_AX
	if (HAS_HE_OP_EXIST(pBss->ie_exists) &&
		(pBss->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT)) {
		UCHAR op_6g_bw = pBss->he6g_opinfo.ctrl & HE_6G_OP_CONTROL_CH_WIDTH_MASK;
		UCHAR op_6g_ccfs1 = pBss->he6g_opinfo.ccfs_1;
		UCHAR op_6g_ccfs0 = pBss->he6g_opinfo.ccfs_0;

		if (op_6g_bw == HE_BW_80)
			op_bw = BW_80;
		else if (op_6g_bw == HE_BW_160) {
			op_bw = BW_160;
			if (((op_6g_ccfs1 - op_6g_ccfs0) > 16) ||
				((op_6g_ccfs0 - op_6g_ccfs1) > 16))
				op_bw = BW_8080;
		}
	}
#endif

	/*EHT op IE check*/
#ifdef DOT11_EHT_BE
	if (HAS_EHT_OP_EXIST(pBss->ie_exists) &&
		GET_DOT11BE_OP_PARAM_HAS_OP_INFO(pBss->eht_op.op_parameters)) {
		UCHAR op_eht_bw = GET_DOT11BE_OP_CTRL_CH_BW(pBss->eht_op.op_info.control);

		if (op_eht_bw == EHT_OP_CH_BW320)
			op_bw = BW_320;
		else if (op_eht_bw == EHT_OP_CH_BW160)
			op_bw = BW_160;
		else if (op_eht_bw == EHT_OP_CH_BW80)
			op_bw = BW_80;
		else if (op_eht_bw == EHT_OP_CH_BW40)
			op_bw = BW_40;
		else if (op_eht_bw == EHT_OP_CH_BW20)
			op_bw = BW_20;
	}
#endif

	return op_bw;
}

UCHAR GetNeighborAPMaxBwCap(
	IN	BSS_ENTRY * pBss)
{
	UCHAR max_bw = BW_20;

	/*HT 40M check*/
	if (HAS_HT_CAPS_EXIST(pBss->ie_exists)) {
		if (pBss->HtCapability.HtCapInfo.ChannelWidth == HT_BW_40)
			max_bw = BW_40;
	}

	/*VHT 40/80/160M check*/
	if (HAS_VHT_CAPS_EXIST(pBss->ie_exists)) {
		if (pBss->vht_cap_ie.vht_cap.ch_width == 0) {
			if (HAS_VHT_OP_EXIST(pBss->ie_exists)) {
				if (pBss->vht_op_ie.vht_op_info.ch_width == 0)
					max_bw = BW_40;
				else
					max_bw = BW_80;
			}
		} else
			max_bw = BW_160;
	}

	/*HE BW check*/
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(pBss->ie_exists)) {
		UCHAR he_bw;
		UCHAR he_rf_bw;

		he_bw = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(pBss->he_caps.phy_cap.phy_capinfo_1));
		he_rf_bw = he_bw_2_rf_bw(he_bw);
		if (he_rf_bw > max_bw)
			max_bw = he_rf_bw;
	}
#endif

	/*EHT 320M check*/
#ifdef DOT11_EHT_BE
	if (HAS_EHT_CAPS_EXIST(pBss->ie_exists)) {
		if ((pBss->eht_caps.phy_cap.phy_capinfo_1 & DOT11BE_PHY_CAP_320M_6G)
			&& (max_bw == BW_160))
			max_bw = BW_320;
	}
#endif

	return max_bw;
}

VOID RTMPCommSiteSurveyData(
	IN  RTMP_STRING *msg,
	IN  BSS_ENTRY * pBss,
	IN  UINT32 MsgLen,
	IN  BOOLEAN RawSSID)
{
	INT         Rssi = 0;
	UINT        Rssi_Quality = 0;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	CHAR		Ssid[100] = {0};
	RTMP_STRING SecurityStr[32] = {0};
	INT ret;
	UINT LeftBufSize;
	//UCHAR max_bw;

	/*Channel*/
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%-4d", pBss->Channel);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	/*SSID*/
	NdisZeroMemory(Ssid, sizeof(Ssid));
	if (RawSSID)
		NdisMoveMemory(Ssid, pBss->Ssid, pBss->SsidLen);
	else
	{
	if (RTMPCheckStrPrintAble((PCHAR)pBss->Ssid, pBss->SsidLen))
		NdisMoveMemory(Ssid, pBss->Ssid, pBss->SsidLen);
	else {
		INT idx = 0;

		LeftBufSize = sizeof(Ssid);
		ret = snprintf(Ssid, LeftBufSize, "%s", "0x");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}

		for (idx = 0; idx < pBss->SsidLen; idx++) {
			LeftBufSize = sizeof(Ssid) - 2 - (idx * 2);
			ret = snprintf(Ssid + 2 + (idx * 2), LeftBufSize, "%02X", (UCHAR)pBss->Ssid[idx]);
			if (os_snprintf_error(LeftBufSize, ret)) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				return;
			}
		}
	}
	}
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%-33s ", Ssid);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	/*BSSID*/
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%02x:%02x:%02x:%02x:%02x:%02x   ",
			pBss->Bssid[0],
			pBss->Bssid[1],
			pBss->Bssid[2],
			pBss->Bssid[3],
			pBss->Bssid[4],
			pBss->Bssid[5]);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	/*Security*/
	RTMPZeroMemory(SecurityStr, 32);
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(SecurityStr, LeftBufSize, "%s/%s", GetAuthModeStr(pBss->AKMMap), GetEncryModeStr(pBss->PairwiseCipher));
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%-23s", SecurityStr);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	/* Rssi*/
	Rssi = (INT)pBss->Rssi;
	ret = snprintf(msg + strlen(msg), MsgLen - strlen(msg), "%-8d", Rssi + 0x100);
	if (os_snprintf_error(LeftBufSize, ret)) {
 		//MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Snprintf failed!\n");
 		return;
 	}
	if (Rssi >= -50)
		Rssi_Quality = 100;
	else if (Rssi >= -80)    /* between -50 ~ -80dbm*/
		Rssi_Quality = (UINT)(24 + ((Rssi + 80) * 26) / 10);
	else if (Rssi >= -90)   /* between -80 ~ -90dbm*/
		Rssi_Quality = (UINT)(((Rssi + 90) * 26) / 10);
	else    /* < -84 dbm*/
		Rssi_Quality = 0;

	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%-9d", Rssi_Quality);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}

	/* Wireless Mode*/
	wireless_mode = NetworkTypeInUseSanity(pBss);

	if (wireless_mode == Ndis802_11FH ||
		wireless_mode == Ndis802_11DS) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "b");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "a");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_N) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "a/n");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_AC) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "a/n/ac");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "b/g");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24_N) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "b/g/n");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24_HE) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "b/g/n/ax");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_HE) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "a/n/ac/ax");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24_EHT) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "b/g/n/ax/be");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_EHT) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "a/n/ac/ax/be");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-11s", "unknown");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	}

	/* Ext Channel*/
	if (HAS_HT_OP_EXIST(pBss->ie_exists)) {
		if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) {
			LeftBufSize = MsgLen - strlen(msg);
			ret = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", " ABOVE");
			if (os_snprintf_error(LeftBufSize, ret)) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				return;
			}
		} else if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW) {
			LeftBufSize = MsgLen - strlen(msg);
			ret = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", " BELOW");
			if (os_snprintf_error(LeftBufSize, ret)) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				return;
			}
		} else {
			LeftBufSize = MsgLen - strlen(msg);
			ret = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", " NONE");
			if (os_snprintf_error(LeftBufSize, ret)) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				return;
			}
		}
	} else {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", " NONE");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	}

	/*MaxBW
	max_bw = GetNeighborAPOperatingBw(pBss);
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", get_bw_str(max_bw, BW_FROM_OID));
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	*/

	/*Network Type		*/
	if (pBss->BssType == BSS_ADHOC) {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-3s", " Ad");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	} else {
		LeftBufSize = MsgLen - strlen(msg);
		ret = snprintf(msg + strlen(msg), LeftBufSize, "%-3s", " In");
		if (os_snprintf_error(LeftBufSize, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			return;
		}
	}

	/* SSID Length */
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, " %-8d", pBss->SsidLen);
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	LeftBufSize = MsgLen - strlen(msg);
	ret = snprintf(msg + strlen(msg), LeftBufSize, "\n");
	if (os_snprintf_error(LeftBufSize, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		return;
	}
	return;
}

static BOOLEAN ascii2int(RTMP_STRING *in, UINT32 *out)
{
	UINT32 decimal_val, val;
	CHAR *p, asc_val;

	decimal_val = 0;
	p = (char *)in;

	while ((*p) != 0) {
		val = 0;
		asc_val = *p;

		if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		decimal_val = (decimal_val * 10) + val;
		p++;
	}

	*out = decimal_val;
	return TRUE;
}

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_get_scan_result(struct wiphy *wiphy, RTMP_ADAPTER *pAd, SCAN_CTRL *ScanCtrl, UINT32 start_idx)
{
	RTMP_STRING *msg;
	INT i = 0;
	INT Status = 0;
	INT max_len = LINE_LEN;
	UINT LeftBufSize;
	UINT32 bss_start_idx = start_idx;
	BSS_ENTRY *pBss;
	UINT32 TotalLen, BufLen;
	BSS_TABLE *ScanTab = &ScanCtrl->ScanTab;
	struct sk_buff *skb;
	UINT32 msg_len = 0;
	int ret;

	if (!ScanTab) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "ScanTab is null\n");
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
	max_len += WPS_LINE_LEN;
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	max_len += DOT11R_LINE_LEN;
#endif /* DOT11R_FT_SUPPORT */
#endif /*CONFIG_STA_SUPPORT */

#ifdef TR181_SUPPORT
	max_len += TR181_LINE_LEN;
#endif /* TR181_SUPPORT */


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"bss_start_idx is %d\n", bss_start_idx);

	TotalLen = sizeof(CHAR) * ((MAX_LEN_OF_BSS_TABLE) * max_len) + 100;
	BufLen = IW_SCAN_MAX_DATA;
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"msg memory alloc fail!\n");
		return FALSE;
	}

	memset(msg, 0, TotalLen);

	if (ScanTab->BssNr == 0) {
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg, LeftBufSize, "No BssInfo\n");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		goto SEND;
	}

	if (bss_start_idx > (ScanTab->BssNr - 1)) {
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg, LeftBufSize, "BssInfo Idx(%d) is out of range(0~%d)\n",
				bss_start_idx, (ScanTab->BssNr - 1));
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		goto SEND;
	}

	Status = snprintf(msg, TotalLen, "%s", "\n");
	if (os_snprintf_error(TotalLen, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize, "Total=%-4d", ScanTab->BssNr);
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize, "%s", "\n");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#ifdef DPA_T
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize,
	"%-4s%-4s%-33s%-20s%-33s%-9s%-11s%-7s%-3s\n",
	"Ch", "Len", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH", " NT");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#else /* DPA_T */
#ifdef CUSTOMER_MAXBITRATE_SUPPORT
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize,
	"%-4s%-4s%-34s%-20s%-23s%-9s%-11s%-7s%-3s%-8s%-16s\n",
	"No", "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH", " NT",
	" SSID_Len", "MaxBitRate");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#else
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize,
	"%-4s%-4s%-34s%-20s%-23s%-9s%-11s%-7s%-5s%-3s%-8s\n",
	"No", "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH",
	"BW", " NT", " SSID_Len");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif
#endif /* !DPA_T */
#ifdef WSC_INCLUDED
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s%-5s\n", " WPS", " DPID");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* WSC_INCLUDED */
#if 0
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-5s%-5s%-12s%-12s%-4s%-s",
		" Mode", " Rssi", " SupRate", " ExtRate", " BCN", " DTIM");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-10s\n", " BcnRept");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#ifdef MWDS
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-8s\n", " MWDSCap");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* MWDS */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-5s%-9s%-10s\n", " MDId", " FToverDS", " RsrReqCap");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	BssTableSortByRssi(ScanTab,FALSE);
	for (i = bss_start_idx; i < ScanTab->BssNr; i++) {
		pBss = &ScanTab->BssEntry[i];

		if (pBss->Channel == 0)
			break;

		if ((strlen(msg) + max_len) >= BufLen)
			break;

		/*No*/
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4d", i);
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		RTMPCommSiteSurveyData(msg, pBss, TotalLen);
#ifdef WSC_INCLUDED

		/*WPS*/
		if (pBss->WpsAP & 0x01) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s", " YES");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s", "  NO");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}

		if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PIN) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " PIN");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PBC) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " PBC");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " ");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}

#endif /* WSC_INCLUDED */
#ifdef TR181_SUPPORT
		/* radio mode */
		LeftBufSize = TotalLen - strlen(msg);
		switch (pBss->BssType) {
		case 0:
			Status = snprintf(msg + strlen(msg), LeftBufSize,
				"%-4s", " Ad");
			break;
		case 1:
			Status = snprintf(msg + strlen(msg), LeftBufSize,
				"%-4s", " In");
			break;
		case 2:
			Status = snprintf(msg + strlen(msg), LeftBufSize,
				"%-4s", " An");
			break;
		case 3:
			Status = snprintf(msg + strlen(msg), LeftBufSize,
				"%-4s", " MO");
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Error bss type!\n");
			break;
		}
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}

		/* Rssi */
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5d", pBss->Rssi);
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}

		/*SupRate and ExtRate*/
		{
			int j;

			for (j = 0; j < pBss->SupRateLen; j++) {
				if (pBss->SupRate[j]) {
					LeftBufSize = TotalLen - strlen(msg);
					Status = snprintf(msg + strlen(msg), LeftBufSize, "%-2x/",
						pBss->SupRate[j]);
					if (os_snprintf_error(LeftBufSize, Status)) {
						MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
						goto ERROR;
					}
				}
			}

			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, " ");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}

			for (j = 0; j < pBss->ExtRateLen; j++) {
				if (pBss->ExtRate[j]) {
					LeftBufSize = TotalLen - strlen(msg);
					Status = snprintf(msg + strlen(msg), LeftBufSize, "%-2x/",
						pBss->ExtRate[j]);
					if (os_snprintf_error(LeftBufSize, Status)) {
						MTWF_DBG(pAd,
							DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
						goto ERROR;
					}
				}
			}
		}

		/* Beacon Period */
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, " %-4d", pBss->BeaconPeriod);
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}

		/* Dtim Period */
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4d", pBss->DtimPeriod);
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
#endif /* End of TR181_SUPPORT */
#ifndef MWDS
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-7s\n", pBss->FromBcnReport ? " YES" : " NO");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
#else
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", pBss->FromBcnReport ? " YES" : " NO");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}

		if (pBss->bSupportMWDS) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4s\n", " YES");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4s\n", " NO");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}
#endif /* MWDS */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT

		if (pBss->bHasMDIE) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, " %02x%02x", pBss->FT_MDIE.MdId[0], pBss->FT_MDIE.MdId[1]);
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}

			if (pBss->FT_MDIE.FtCapPlc.field.FtOverDs) {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-9s", " TRUE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			} else {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-9s", " FALSE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			}

			if (pBss->FT_MDIE.FtCapPlc.field.RsrReqCap) {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-10s\n",
				" TRUE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			} else {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-10s\n",
				" FALSE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			}
		}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	}

SEND:

	msg_len = strlen(msg);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_len + 1);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		goto ERROR;
	}

	if (nla_put_string(skb, MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT, msg)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		goto ERROR;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"reply msg failed\n");
		goto ERROR;
	}

	os_free_mem((PUCHAR)msg);
	return TRUE;
ERROR:
	os_free_mem((PUCHAR)msg);
	return FALSE;
}
#endif

VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *msg;
	INT		i = 0;
	INT			WaitCnt;
	INT		Status = 0;
	INT         max_len = LINE_LEN;
	UINT LeftBufSize;
	RTMP_STRING *this_char;
	UINT32		bss_start_idx;
	BSS_ENTRY *pBss;
	UINT32 TotalLen, BufLen = IW_SCAN_MAX_DATA;
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	UINT32 IfIdx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAdapter, pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);

	if (wdev == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"wdev is null! IfIdx: %d.\n", IfIdx);
		return;
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
	max_len += WPS_LINE_LEN;
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	max_len += DOT11R_LINE_LEN;
#endif /* DOT11R_FT_SUPPORT */
#endif /*CONFIG_STA_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&this_char, wrq->u.data.length + 1);
	if (!this_char) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Allocate memory fail!!!\n");
		return;
	}

	if (copy_from_user(this_char, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"copy_from_user() fail!!!\n");
		os_free_mem(this_char);
		return;
	}
	this_char[wrq->u.data.length] = 0;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%s(): Before check, this_char = %s\n"
			 , __func__, this_char);

	if (ascii2int(this_char, &bss_start_idx) == FALSE)
		bss_start_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%s(): After check, this_char = %s, out = %d\n"
		, __func__, this_char, bss_start_idx);
	TotalLen = sizeof(CHAR) * ((MAX_LEN_OF_BSS_TABLE) * max_len) + 100;
	BufLen = IW_SCAN_MAX_DATA;
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n");
		os_free_mem(this_char);
		return;
	}

	memset(msg, 0, TotalLen);

	if (ScanTab->BssNr == 0) {
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg, LeftBufSize, "No BssInfo\n");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n",
				 wrq->u.data.length);
		os_free_mem(this_char);
		os_free_mem((PUCHAR)msg);
		return;
	}

	if (bss_start_idx > (ScanTab->BssNr - 1)) {
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg, LeftBufSize, "BssInfo Idx(%d) is out of range(0~%d)\n",
				bss_start_idx, (ScanTab->BssNr - 1));
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"wrq->u.data.length = %d\n",
			wrq->u.data.length);
		os_free_mem((PUCHAR)msg);
		os_free_mem(this_char);
		return;
	}

	Status = snprintf(msg, TotalLen, "%s", "\n");
	if (os_snprintf_error(TotalLen, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize, "Total=%-4d", ScanTab->BssNr);
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize, "%s", "\n");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg), LeftBufSize,
			"%-4s%-4s%-34s%-20s%-23s%-8s%-9s%-11s%-7s%-3s%-8s\n",
			"No", "Ch", "SSID", "BSSID", "Security", "Rssi","Siganl(%)","W-Mode", " ExtCH", " NT", " SSID_Len");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}

#ifdef WSC_INCLUDED
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s%-5s\n", " WPS", " DPID");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* WSC_INCLUDED */
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-10s\n", " BcnRept");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#if 0
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-8s\n", " MWDSCap");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* MWDS */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	LeftBufSize = TotalLen - strlen(msg);
	Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-5s%-9s%-10s\n", " MDId", " FToverDS", " RsrReqCap");
	if (os_snprintf_error(LeftBufSize, Status)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Snprintf failed!\n");
		goto ERROR;
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	WaitCnt = 0;
#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (IfIdx < pAdapter->MSTANum)
			pAdapter->StaCfg[IfIdx].bSkipAutoScanConn = TRUE;
		else
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				" invalid IfIdx=%d.\n", IfIdx);
	}
#endif /* CONFIG_STA_SUPPORT */

	/*Before geting scan result, need check SCAN/PARTIAL_SCAN first*/
	while ((GetCurrentChannelOpOwner(pAdapter, wdev) == CH_OP_OWNER_SCAN
		|| GetCurrentChannelOpOwner(pAdapter, wdev) == CH_OP_OWNER_PARTIAL_SCAN)
		&& (WaitCnt++ < 20)) {
		MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"Current scan/partialscan is ongoing, need wait scan done!\n");
		OS_WAIT(3000);
	}
	BssTableSortByRssi(ScanTab,FALSE);
	for (i = bss_start_idx; i < ScanTab->BssNr; i++) {
		pBss = &ScanTab->BssEntry[i];

		if (pBss->Channel == 0)
			break;

		if ((strlen(msg) + max_len) >= BufLen)
			break;

		/*No*/
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4d", i);
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
		if (wrq->u.data.flags & 0x1)
                        RTMPCommSiteSurveyData(msg, pBss, TotalLen, true);
                else
                        RTMPCommSiteSurveyData(msg, pBss, TotalLen, false);
#ifdef WSC_INCLUDED

		/*WPS*/
		if (pBss->WpsAP & 0x01) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s", " YES");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, "%-4s", "  NO");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}

		if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PIN) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " PIN");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PBC) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " PBC");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-5s", " ");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}

#endif /* WSC_INCLUDED */
#ifndef MWDS
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-7s\n", pBss->FromBcnReport ? " YES" : " NO");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}
#else
		LeftBufSize = TotalLen - strlen(msg);
		Status = snprintf(msg + strlen(msg), LeftBufSize, "%-7s", pBss->FromBcnReport ? " YES" : " NO");
		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"Snprintf failed!\n");
			goto ERROR;
		}

		if (pBss->bSupportMWDS) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4s\n", " YES");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		} else {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%-4s\n", " NO");
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}
		}

#endif /* MWDS */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT

		if (pBss->bHasMDIE) {
			LeftBufSize = TotalLen - strlen(msg);
			Status = snprintf(msg + strlen(msg) - 1, LeftBufSize, " %02x%02x", pBss->FT_MDIE.MdId[0], pBss->FT_MDIE.MdId[1]);
			if (os_snprintf_error(LeftBufSize, Status)) {
				MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
							"Snprintf failed!\n");
				goto ERROR;
			}

			if (pBss->FT_MDIE.FtCapPlc.field.FtOverDs) {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-9s", " TRUE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			} else {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize, "%-9s", " FALSE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			}

			if (pBss->FT_MDIE.FtCapPlc.field.RsrReqCap) {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize,
									"%-10s\n", " TRUE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			} else {
				LeftBufSize = TotalLen - strlen(msg);
				Status = snprintf(msg + strlen(msg), LeftBufSize,
									"%-10s\n", " FALSE");
				if (os_snprintf_error(LeftBufSize, Status)) {
					MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
								"Snprintf failed!\n");
					goto ERROR;
				}
			}
		}

#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (IfIdx < pAdapter->MSTANum)
			pAdapter->StaCfg[IfIdx].bSkipAutoScanConn = FALSE;
		else
			MTWF_DBG(pAdapter, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"invalid IfIdx=%d.\n", IfIdx);
	}
#endif /* CONFIG_STA_SUPPORT */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"wrq->u.data.length = %d\n",
		wrq->u.data.length);
	os_free_mem((PUCHAR)msg);
	os_free_mem(this_char);
	return;
ERROR:
	os_free_mem((PUCHAR)msg);
	os_free_mem(this_char);
	return;
}
#endif

USHORT RTMPGetLastTxRate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
	union _HTTRANSMIT_SETTING lastTxRate;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
	lastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
	lastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
	lastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	lastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	lastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

	if (lastTxRate.field.MODE >= MODE_VHT)
		lastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
	else if (lastTxRate.field.MODE == MODE_OFDM)
		lastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		lastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

#else
	lastTxRate.word = pEntry->HTPhyMode.word;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return lastTxRate.word;
}

INT Set_DynamicAGG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 rv = 0;
	UINT32 aggEn, dn_th, up_th, kp_idx;

	if (!arg)
		goto Error;

	rv = sscanf(arg, "%u-%u-%u-%u", &aggEn, &dn_th, &up_th, &kp_idx);

	if (rv != 4)
		goto Error;

	pAd->aggManualEn = aggEn > 0 ? TRUE : FALSE;
	pAd->per_dn_th = (UINT8)dn_th;
	pAd->per_up_th = (UINT8)up_th;
	pAd->winsize_kp_idx = kp_idx;

	MTWF_PRINT("%s: aggManualEn = %u, per[dn/up] = [%u/%u], kp_idx = %u\n",
		__func__, pAd->aggManualEn, pAd->per_dn_th, pAd->per_up_th, pAd->winsize_kp_idx);


	return TRUE;

Error:
	MTWF_PRINT("error input parameter!\n");

	MTWF_PRINT("iwpriv ra0 set DynamicAGG=[aggEn]-[dn_th]-[up_th]-[kp_idx]\n");
	MTWF_PRINT("[aggEn]: Force Manual Adjust STA AGG\n");
	MTWF_PRINT("[dn_th]: STA PER Down Threshold	(per>dn_th,agg--)\n");
	MTWF_PRINT("[up_th]: STA PER UP Threshold	(per<up_th,agg++)\n");
	MTWF_PRINT("[kp_idx]:STA BA_WINSIZE Keep Index\n");

	return TRUE;
}

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*
		The BASetup inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/

	if (strlen(arg) <
		19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = (UCHAR) os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token && (i < MAC_ADDR_LEN);
				token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_PRINT("\n"MACSTR"\n", MAC2STR(mac));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_PRINT("\nSetup BA Session: Tid = %d\n", tid);
			ba_ori_session_start(pAd, pEntry->wcid, tid);
		}

		return TRUE;
	}

	return FALSE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ba_setup(RTMP_ADAPTER *pAd, UCHAR *mac_addr, UCHAR tid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct BA_INFO *ba_info;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, mac_addr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, mac_addr, NULL);
#endif

	if (!IS_VALID_ENTRY(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s():Can't find STA, MAC("MACSTR"), TID = %d\n",
			__func__, MAC2STR(mac_addr), tid);
		return -EINVAL;
	}

	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_NOTICE,
		"%s():Setup Ori BA session: TID = %d, MAC("MACSTR")\n",
		__func__, tid, MAC2STR(mac_addr));

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return -EINVAL;
		}
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	ba_info->AutoTest = TRUE;
	ba_ori_session_start(pAd, tr_entry->wcid, tid);

#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return 0;
}
#endif

INT	Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ba_decline;

	ba_decline = os_str_tol(arg, 0, 10);
	wlan_config_set_ba_decline(wdev, ba_decline);

	MTWF_PRINT("(BADecline=%d)\n", wlan_config_get_ba_decline(wdev));
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT cfg80211_badecline_status(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	struct sk_buff *skb;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	UINT8 status;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	status = wlan_config_get_ba_decline(wdev);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO, status)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}

INT	mtk_cfg80211_set_ba_decline(RTMP_ADAPTER *pAd, UCHAR ba_decline)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	wlan_config_set_ba_decline(wdev, ba_decline);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_NOTICE,
		"(BADecline=%d)\n", wlan_config_get_ba_decline(wdev));
	return 0;
}
#endif

INT	Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	UINT16 wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*
		The BAOriTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/
	if (strlen(arg) <
		19) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		/* another acceptable format wcid-tid */
		token = strchr(arg, DASH);

		if ((token != NULL) && (strlen(token) > 1)) {
			tid = os_str_tol((token + 1), 0, 10);

			if (tid > (NUM_OF_TID - 1)) {
				MTWF_PRINT("tid=%d is wrong\n\r", tid);
				return FALSE;
			}

			*token = '\0';
			wcid = os_str_tol(arg, 0, 10);

			if (wcid >= 128) {
				MTWF_PRINT("wcid=%d is wrong\n\r", wcid);
				return FALSE;
			}

			MTWF_PRINT("tear down ori ba,wcid=%d,tid=%d\n\r", wcid, tid);
			ba_ori_session_tear_down(pAd, wcid, tid, FALSE);
			return TRUE;
		}

		return FALSE;
	}

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token && (i < MAC_ADDR_LEN);
				token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_PRINT("\n"MACSTR"-%02x", MAC2STR(mac), tid);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_PRINT("\nTear down Ori BA Session: Tid = %d\n", tid);
			ba_ori_session_tear_down(pAd, pEntry->wcid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ba_ori_teardown(RTMP_ADAPTER *pAd, UCHAR *mac_addr, UCHAR tid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct BA_INFO *ba_info;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, (PUCHAR) mac_addr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, (PUCHAR) mac_addr, NULL);
#endif

	if (!IS_VALID_ENTRY(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s():Can't find STA, MAC("MACSTR"), TID = %d\n",
			__func__, MAC2STR(mac_addr), tid);
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_NOTICE,
		"%s():Tear down Ori BA session: TID = %d, MAC("MACSTR")\n",
		__func__, tid, MAC2STR(mac_addr));

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return FALSE;
		}
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	ba_info->AutoTest = FALSE;
	ba_ori_session_tear_down(pAd, pEntry->wcid, tid, FALSE);

#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return 0;
}
#endif

INT	Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], tid;
	UINT16 wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*
		The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the tid value.
	*/
	if (strlen(arg) <
		19) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		/* another acceptable format wcid-tid */
		token = strchr(arg, DASH);

		if ((token != NULL) && (strlen(token) > 1)) {
			tid = os_str_tol((token + 1), 0, 10);

			if (tid > (NUM_OF_TID - 1)) {
				MTWF_PRINT("tid=%d is wrong\n\r", tid);
				return FALSE;
			}

			*token = '\0';
			wcid = os_str_tol(arg, 0, 10);

			if (wcid >= 128) {
				MTWF_PRINT("wcid=%d is wrong\n\r", wcid);
				return FALSE;
			}

			MTWF_PRINT("tear down rec ba,wcid=%d,tid=%d\n\r", wcid, tid);
			ba_rec_session_tear_down(pAd, wcid, tid, FALSE);
			return TRUE;
		}

		return FALSE;
	}

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		tid = os_str_tol((token + 1), 0, 10);

		if (tid > (NUM_OF_TID - 1))
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token && (i < MAC_ADDR_LEN);
				token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_PRINT("\n"MACSTR"-%02x", MAC2STR(mac), tid);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, (PUCHAR) mac, NULL);
#endif

		if (pEntry) {
			MTWF_PRINT("\nTear down Rec BA Session: Tid = %d\n", tid);
			ba_rec_session_tear_down(pAd, pEntry->wcid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ba_rec_teardown(RTMP_ADAPTER *pAd, UCHAR *mac_addr, UCHAR tid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, (PUCHAR) mac_addr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, (PUCHAR) mac_addr, NULL);
#endif


	if (pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_NOTICE,
			"%s():Tear down Rec BA session: TID = %d, MAC("MACSTR")\n",
			__func__, tid, MAC2STR(mac_addr));
		ba_rec_session_tear_down(pAd, pEntry->wcid, tid, FALSE);
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s():Can't find STA, MAC("MACSTR"), TID = %d\n",
			__func__, MAC2STR(mac_addr), tid);
		return -EINVAL;
	}
	return 0;
}
#endif

INT	Set_HtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG HtBw;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	HtBw = os_str_tol(arg, 0, 10);

	if ((HtBw != BW_40) && (HtBw != BW_20))
		return FALSE;  /*Invalid argument */

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

			if (HtBw == BW_40) {
				wlan_config_set_ht_bw(tdev, BW_40);
				wlan_operate_set_ht_bw(tdev, HT_BW_40, wlan_operate_get_ext_cha(tdev));
			} else {
				wlan_config_set_ht_bw(tdev, BW_20);
				wlan_operate_set_ht_bw(tdev, HT_BW_20, EXTCHA_NONE);
			}
			SetCommonHtVht(pAd, tdev);
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_HEOP, TRUE);
		}
	}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Update Beacon to Reflect BW Changes */
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd))
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
#endif /* BW_VENDOR10_CUSTOM_FEATURE */

	MTWF_PRINT("Set_HtBw_Proc::(HtBw=%d)\n", wlan_config_get_ht_bw(wdev));
	return TRUE;
}

INT	Set_HtMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bAutoRate = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	UCHAR HtMcs = MCS_AUTO, Mcs_tmp, ValidMcs = 15;
#ifdef DOT11_VHT_AC
	RTMP_STRING *mcs_str, *ss_str;
	UCHAR ss = 0, mcs = 0;
#endif /* DOT11_VHT_AC */
	UINT32       IfIdx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev){
		MTWF_PRINT("wdev is null! IfIdx: %d.\n", IfIdx);
		return FALSE;
	}

#ifdef DOT11_VHT_AC
	ss_str = arg;
	mcs_str = rtstrchr(arg, ':');

	if (mcs_str != NULL) {
		*mcs_str = 0;
		mcs_str++;
		MTWF_PRINT("%s(): ss_str=%s, mcs_str=%s\n",
				 __func__, ss_str, mcs_str);

		if (strlen(ss_str) && strlen(mcs_str)) {
			mcs = os_str_tol(mcs_str, 0, 10);
			ss = os_str_tol(ss_str, 0, 10);

			if ((ss <= wlan_operate_get_tx_stream(wdev)) && (mcs <= 7))
				HtMcs = ((ss - 1) << 4) | mcs;
			else {
				HtMcs = MCS_AUTO;
				ss = 0;
			}

			MTWF_PRINT("%s(): %dSS-MCS%d, Auto=%s\n",
					 __func__, ss, mcs,
					 (HtMcs == MCS_AUTO && ss == 0) ? "TRUE" : "FALSE");
			Set_FixedTxMode_Proc(pAd, "VHT");
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		Mcs_tmp = os_str_tol(arg, 0, 10);

		if (Mcs_tmp <= ValidMcs || Mcs_tmp == 32)
			HtMcs = Mcs_tmp;
		else
			HtMcs = MCS_AUTO;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		wdev->DesiredTransmitSetting.field.MCS = HtMcs;
		MTWF_PRINT("%s::(HtMcs=%d) for ra%d\n", __func__,
			wdev->DesiredTransmitSetting.field.MCS, IfIdx);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		MAC_TABLE_ENTRY *pEntry = NULL;

		wdev = &pAd->StaCfg[IfIdx].wdev;
		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		ASSERT(pEntry);
		if (!pEntry) {
			MTWF_PRINT("pEntry is null!\n");
			return FALSE;
		}
		wdev->DesiredTransmitSetting.field.MCS = HtMcs;
		wdev->bAutoTxRateSwitch = (HtMcs == MCS_AUTO) ? TRUE : FALSE;
		MTWF_PRINT("%s::(HtMcs=%d, bAutoTxRateSwitch = %d)\n", __func__,
				 wdev->DesiredTransmitSetting.field.MCS, wdev->bAutoTxRateSwitch);

		if ((!WMODE_CAP_N(wdev->PhyMode)) ||
			(pEntry->HTPhyMode.field.MODE < MODE_HTMIX)) {
			if ((wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
				(HtMcs <= 3) &&
				(wdev->DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_CCK))
				RTMPSetDesiredRates(pAd, wdev, (LONG) (RateIdToMbps[HtMcs] * 1000000));
			else if ((wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
					 (HtMcs <= 7) &&
					 (wdev->DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_OFDM))
				RTMPSetDesiredRates(pAd, wdev, (LONG) (RateIdToMbps[HtMcs + 4] * 1000000));
			else
				bAutoRate = TRUE;

			if (bAutoRate) {
				wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
				RTMPSetDesiredRates(pAd, wdev, -1);
			}

			MTWF_PRINT("%s::(FixedTxMode=%d)\n", __func__,
					 wdev->DesiredTransmitSetting.field.FixedTxMode);
		}

		if (ADHOC_ON(pAd))
			return TRUE;
	}
#endif /* CONFIG_STA_SUPPORT */
	SetCommonHtVht(pAd, wdev);

	return TRUE;
}

INT	Set_HtGi_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG HtGi;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	HtGi = os_str_tol(arg, 0, 10);

	if ((HtGi != GI_400) && (HtGi != GI_800))
		return FALSE;

	wlan_config_set_ht_gi(wdev, HtGi);
	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("%s::(ShortGI=%d)\n", __func__, wlan_config_get_ht_gi(wdev));
	return TRUE;
}

INT	Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Size;

	Size = os_str_tol(arg, 0, 10);

	if (Size <= 0 || Size >= 64)
		Size = 8;

	pAd->CommonCfg.TxBASize = Size - 1;
	MTWF_PRINT("%s::(TxBASize= %d)\n", __func__, Size);
	return TRUE;
}

INT	Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 1)
		pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	else
		pAd->CommonCfg.HT_DisallowTKIP = FALSE;

	MTWF_PRINT("%s::%s\n", __func__,
			 (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "enabled" : "disabled");
	return TRUE;
}

INT	Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
	else if (Value == HTMODE_MM)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
	else
		return FALSE; /*Invalid argument */

	wlan_config_set_ht_mode(wdev, Value);
	wlan_operate_loader_greenfield(wdev, Value);

	SetCommonHtVht(pAd, wdev);
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	MTWF_PRINT("(HtOpMode=%d)\n", pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ht_op_mode(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 ht_op_mode)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	if (ht_op_mode == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE = HTMODE_GF;
	else if (ht_op_mode == HTMODE_MM)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE = HTMODE_MM;
	else
		return FALSE; /*Invalid argument */

	wlan_config_set_ht_mode(wdev, ht_op_mode);
	wlan_operate_loader_greenfield(wdev, ht_op_mode);

	SetCommonHtVht(pAd, wdev);
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_NOTICE,
		"mwctl:set bss(%d) HT_OP_MODE=%s\n",
		inf_idx, pAd->CommonCfg.RegTransmitSetting.field.HTMODE ? "GF":"MM");

	return TRUE;
}
#endif

INT	Set_HtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 1) {
		MTWF_PRINT("%s: Invalid arguments!\n", __func__);
		return FALSE;
	}

	wlan_config_set_ht_ldpc(wdev, (UCHAR)Value);
	wlan_operate_set_ht_ldpc(wdev, (UCHAR)Value);

	MTWF_PRINT("%s:(HtLdpc=%d)\n", __func__, wlan_config_get_ht_ldpc(wdev));
	return TRUE;
}

INT	Set_HtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_ht_stbc(wdev, STBC_USE);
	else if (Value == STBC_NONE)
		wlan_config_set_ht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(HtStbc=%d)\n", wlan_config_get_ht_stbc(wdev));
	return TRUE;
}

/*configure useage*/
INT	set_extcha_for_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR value)
{
	value = value ? EXTCHA_ABOVE : EXTCHA_BELOW;
	wlan_config_set_ext_cha(wdev, value);
	SetCommonHtVht(pAd, wdev);
	return TRUE;
}

INT	Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ext_cha;

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	Value = os_str_tol(arg, 0, 10);

	if (Value != 0 && Value != 1)
		return FALSE;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev)))
			set_extcha_for_wdev(pAd, tdev, Value);
	}
	ext_cha = wlan_config_get_ext_cha(wdev);
	MTWF_PRINT("(HtExtcha=%d)\n", ext_cha);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT	mtk_cfg80211_set_ht_extcha(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	UCHAR bandidx;
	UCHAR i;
	struct wifi_dev *tdev;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	if (value != 0 && value != 1)
		return FALSE;

	bandidx = HcGetBandByWdev(wdev);
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (bandidx == HcGetBandByWdev(tdev)))
			set_extcha_for_wdev(pAd, tdev, value);
	}
	MTWF_PRINT("(HtExtcha=%d)\n",
		wlan_config_get_ext_cha(wdev));
	return TRUE;
}
#endif
INT	Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	Value = os_str_tol(arg, 0, 10);

	if (!wdev)
		return FALSE;

	if (Value <= 7)
		wlan_config_set_min_mpdu_start_space(wdev, Value);
	else
		wlan_config_set_min_mpdu_start_space(wdev, INTERVAL_NO_RESTRICTION);

	SetCommonHtVht(pAd, NULL);

	Value = wlan_config_get_min_mpdu_start_space(wdev);
	MTWF_PRINT("Set_HtMpduDensity_Proc::(HtMpduDensity=%d)\n", (UCHAR)Value);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_mpdu_density(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	if (value <= 7)
		wlan_config_set_min_mpdu_start_space(wdev, value);
	else
		wlan_config_set_min_mpdu_start_space(wdev, INTERVAL_NO_RESTRICTION);

	MTWF_PRINT("(HtMpduDensity=%d)\n",
		wlan_config_get_min_mpdu_start_space(wdev));
	return TRUE;
}
#endif

INT	Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 val = 0, max_ba_wsize = 0;

	if (!wdev)
		return FALSE;

	max_ba_wsize = ba_get_default_max_ba_wsize(wdev, pAd);

	val = os_str_tol(arg, 0, 10);
	if (val == 0 || val > max_ba_wsize)
		return FALSE;

	wlan_config_set_ba_txrx_wsize(wdev, val, val, max_ba_wsize);
	SetCommonHtVht(pAd, wdev);

	MTWF_PRINT("(HtBaWinSize=%d)\n", val);

	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT	mtk_cfg80211_set_ht_ba_wsize(RTMP_ADAPTER *pAd, USHORT ba_wsize)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	UINT16 max_ba_wsize = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	max_ba_wsize = ba_get_default_max_ba_wsize(wdev, pAd);

	if (ba_wsize == 0 || ba_wsize > BA_WIN_SZ_1024)
		return -EINVAL;

	wlan_config_set_ba_txrx_wsize(wdev, ba_wsize, ba_wsize, max_ba_wsize);
	SetCommonHtVht(pAd, wdev);

	MTWF_PRINT("(HtBaWinSize=%d)\n", ba_wsize);

	return 0;
}
#endif

INT	Set_HtRdg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value != 0 && IS_ASIC_CAP(pAd, fASIC_CAP_RDG))
		pAd->CommonCfg.bRdg = TRUE;
	else
		pAd->CommonCfg.bRdg = FALSE;

	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(HtRdg=%d)\n", pAd->CommonCfg.bRdg);
	return TRUE;
}

INT	Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->bLinkAdapt = FALSE;
	else if (Value == 1)
		pAd->bLinkAdapt = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_PRINT("(HtLinkAdapt=%d)\n", pAd->bLinkAdapt);
	return TRUE;
}

INT	Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
	wlan_config_set_amsdu_en(wdev, Value);
	SetCommonHtVht(pAd, wdev);

	MTWF_PRINT("(HtAmsdu=%d)\n", wlan_config_get_amsdu_en(wdev));
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_amsdu_en(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	wlan_config_set_amsdu_en(wdev, value);
	SetCommonHtVht(pAd, wdev);

	MTWF_PRINT("(HtAmsdu=%d)\n", wlan_config_get_amsdu_en(wdev));
	return TRUE;
}

INT mtk_cfg80211_get_amsdu_status(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, struct wiphy *wiphy)
{
	UINT8 amsdu_status;
	struct sk_buff *skb;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	amsdu_status = wlan_config_get_amsdu_en(wdev);
	MTWF_PRINT("(HtAmsdu=%d)\n", amsdu_status);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN, amsdu_status)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}


INT mtk_cfg80211_get_bss_isolation_status(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	struct sk_buff *skb;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	UINT8 status;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	if (pAd->ApCfg.MBSSID[apidx].IsolateInterStaTraffic)
		status = 1;
	else
		status = 0;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_ISOLATION, status)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}


INT mtk_cfg80211_set_bss_isolate_en(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return FALSE;
		}
	} else
		return FALSE;

	if (value == 1)
		pAd->ApCfg.MBSSID[inf_idx].IsolateInterStaTraffic = TRUE;
	else if (value == 0)
		pAd->ApCfg.MBSSID[inf_idx].IsolateInterStaTraffic = FALSE;

	MTWF_PRINT("(bss_isolate=%ld)\n", pAd->ApCfg.MBSSID[inf_idx].IsolateInterStaTraffic);
	return TRUE;
}

#endif

INT	Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ba_en;

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
	ba_en = Value;
	wlan_config_set_ba_enable(wdev, ba_en);
	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(HtAutoBa=%d)\n", ba_en);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT cfg80211_ba_auto_status(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	struct sk_buff *skb;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	UINT8 status;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	status = wlan_config_get_ba_enable(wdev);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO, status)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_BA, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}

INT	mtk_cfg80211_set_ht_auto_ba(RTMP_ADAPTER *pAd, UCHAR ba_en)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	wlan_config_set_ba_enable(wdev, ba_en);
	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(HtAutoBa=%d)\n", ba_en);
	return 0;
}
#endif

INT	Set_HtProtect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);
	wlan_config_set_ht_protect_en(wdev, Value);
	MTWF_PRINT("(HtProtect=%d)\n", Value);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ht_protect(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	wlan_config_set_ht_protect_en(wdev, value);

	MTWF_PRINT("(HtProtect=%d)\n", value);
	return TRUE;
}
#endif


INT	Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6], mode;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/*
		The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
			=>The six 2 digit hex-decimal number previous are the Mac address,
			=>The seventh decimal number is the mode value.
	*/
	if (strlen(arg) <
		19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and mode value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		mode = os_str_tol((token + 1), 0, 10);

		if (mode > MMPS_DISABLE)
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token && (i < MAC_ADDR_LEN);
				token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_PRINT("\n"MACSTR"-%02x", MAC2STR(mac), mode);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, mac);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, mac, NULL);
#endif

		if (pEntry) {
			MTWF_PRINT("\nSendSMPSAction SMPS mode = %d\n", mode);
			SendSMPSAction(pAd, pEntry->wcid, mode);
		}

		return TRUE;
	}

	return FALSE;
}

INT	Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	UCHAR mmps = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 3)
		Value = 3;
	wlan_config_set_mmps(wdev, Value);
	SetCommonHtVht(pAd, wdev);
	mmps = wlan_config_get_mmps(wdev);

	MTWF_PRINT("(MIMOPS mode=%d)\n", mmps);
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
/*
    ==========================================================================
    Description:
	Set Tx Stream number
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG	Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32       apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;

	if (!wdev){
		MTWF_PRINT("wdev is null! apidx: %d.\n", apidx);
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apidx];

	ucTxPath = MIN(MCS_NSS_CAP(pAd)->max_nss,
		MCS_NSS_CAP(pAd)->max_path[MAX_PATH_TX]);


#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable)
			ucTxPath = pAd->TxStream;
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	Value = os_str_tol(arg, 0, 10);

	if ((Value >= 1) && (Value <= ucTxPath)) {
		wlan_config_set_tx_stream(wdev, Value);
		wlan_operate_set_tx_stream(wdev, Value);
	} else {
		wlan_config_set_tx_stream(wdev, ucTxPath);
		wlan_operate_set_tx_stream(wdev, ucTxPath);
	}

	SetCommonHtVht(pAd, wdev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif
	MTWF_PRINT("(Tx Stream=%d)\n", wlan_operate_get_tx_stream(wdev));
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT	mtk_cfg80211_Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, ULONG value)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32       apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;

	if (!wdev) {
		MTWF_PRINT("wdev is null! apidx: %d.\n", apidx);
		return -EFAULT;
	}

	pMbss = &pAd->ApCfg.MBSSID[apidx];

	ucTxPath = MIN(MCS_NSS_CAP(pAd)->max_nss,
		MCS_NSS_CAP(pAd)->max_path[MAX_PATH_TX]);

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable)
			ucTxPath = pAd->TxStream;
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	if ((value >= 1) && (value <= ucTxPath)) {
		wlan_config_set_tx_stream(wdev, value);
		wlan_operate_set_tx_stream(wdev, value);
	} else {
		wlan_config_set_tx_stream(wdev, ucTxPath);
		wlan_operate_set_tx_stream(wdev, ucTxPath);
	}

	SetCommonHtVht(pAd, wdev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif
	MTWF_PRINT("(Tx Stream=%d)\n", wlan_operate_get_tx_stream(wdev));
	return 0;
}

#endif

/*
    ==========================================================================
    Description:
	Set Rx Stream number
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG	Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32       apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucRxPath = pAd->Antenna.field.RxPath;

	if (!wdev){
		MTWF_PRINT("wdev is null! apidx: %d.\n", apidx);
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apidx];

	ucRxPath = MIN(MCS_NSS_CAP(pAd)->max_nss,
		MCS_NSS_CAP(pAd)->max_path[MAX_PATH_RX]);

	Value = os_str_tol(arg, 0, 10);

	if ((Value >= 1) && (Value <= ucRxPath)) {
		wlan_config_set_rx_stream(wdev, Value);
		wlan_operate_set_rx_stream(wdev, Value);
	} else {
		wlan_config_set_rx_stream(wdev, ucRxPath);
		wlan_operate_set_rx_stream(wdev, ucRxPath);
	}

	SetCommonHtVht(pAd, wdev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif
	MTWF_PRINT("(Rx Stream=%d)\n", wlan_operate_get_rx_stream(wdev));
	return TRUE;
}

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		greenap_proc(pAd, FALSE);
	else if (Value == 1)
		greenap_proc(pAd, TRUE);
	else
		return FALSE; /*Invalid argument*/

	MTWF_PRINT("(greenap_cap=%d)\n", greenap_get_capability(pAd));
	return TRUE;
}
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT set_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0) {
		mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND0, FALSE, FALSE);
		if (pAd->CommonCfg.dbdc_mode)
			mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND1, FALSE, FALSE);
		set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
	} else if (Value == 1) {
		set_pcie_aspm_dym_ctrl_cap(pAd, TRUE);
	} else {
		return FALSE; /*Invalid argument*/
	}

	MTWF_PRINT("%s=%d\n", __func__, get_pcie_aspm_dym_ctrl_cap(pAd));

	return TRUE;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT set_twt_support_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	POS_COOKIE	obj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT32 para_id = 0;
	UINT32 value = 0;
	INT para_num = 0;
	UINT32 before_value = 0;
	UINT32 new_value = 0;

	wdev = get_wdev_by_ioctl_idx_and_iftype(ad, obj->ioctl_if, obj->ioctl_if_type);

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	para_num = sscanf(arg, "%d:%d", &para_id, &value);
	if (para_num > 2) {
		MTWF_PRINT("Invalid format.\n");
		return FALSE;
	}

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"para_num=%d,para_id=%d,value=%d\n",
		para_num, para_id, value);

	new_value = value;
	/* para_id=0(TWTSupport), para_id=1(TWTInfoFrame) */
	if (para_id > 2)
		goto format_error;

	if (para_id == 0 && para_num != 1)
		goto format_error;

	if (para_id == 1 && (new_value >= TWT_PROFILE_SUPPORT_TYPE_NUM || para_num != 2))
		goto format_error;

	if (para_id == 2 && (new_value >= 2 || para_num != 2))
		goto format_error;

	switch (para_id) {
	case 0:
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:if=%d,if_type=%d,TWTSupport=%d\n",
			__func__, obj->ioctl_if, obj->ioctl_if_type,
			wlan_config_get_he_twt_support(wdev));
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:if=%d,if_type=%d,TWTInfoFrame=%d\n",
			__func__, obj->ioctl_if, obj->ioctl_if_type,
			wlan_config_get_he_twt_info_frame(wdev));
		break;
	case 1:
		before_value = wlan_config_get_he_twt_support(wdev);

		if (new_value < TWT_PROFILE_SUPPORT_TYPE_NUM)
			wlan_config_set_he_twt_support(wdev, new_value);

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:if=%d,if_type=%d,TWTSupport=%d->%d\n",
			__func__, obj->ioctl_if, obj->ioctl_if_type,
			before_value, wlan_config_get_he_twt_support(wdev));
		break;
	case 2:
		before_value = wlan_config_get_he_twt_info_frame(wdev);

		wlan_config_set_he_twt_info_frame(wdev, new_value);

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:if=%d,if_type=%d,TWTInfoFrame=%d->%d\n",
			__func__, obj->ioctl_if, obj->ioctl_if_type,
			before_value, wlan_config_get_he_twt_info_frame(wdev));
		break;
	/*default: Fix Coverity: para_id only 0/1/2.
		break;*/
	}

	return TRUE;

format_error:
	if (para_id == 0)
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"iwpriv IF set twtsupport=0 to show current status\n");
	else if (para_id == 1)
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"iwpriv IF set twtsupport=1:x (x=0:3 enable/disable b/i TWT)\n");
	else if (para_id == 2)
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"iwpriv IF set twtsupport=2:x (x=0/1 enable/disable TWTInfoFrame)\n");

	return FALSE;
}

INT set_twt_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	static struct TWT_AGRT_PARA_T twt_agrt_para_local = {0};

	POS_COOKIE	obj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	UINT32 para_id = 0;
	UINT32 value[8] = {0};
	INT para_num = 0;
	INT i = 0;

	UINT32 current_tsf[2] = {0};

	wdev = get_wdev_by_ioctl_idx_and_iftype(ad, obj->ioctl_if, obj->ioctl_if_type);

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	para_num = sscanf(arg, "%d:%d:%d:%d:%d:%d:%d:%d:%d", &para_id, &value[0],
		&value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7]);
	if (para_num <= 0)
		goto format_error;
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"para_num=%d, para_id=%d\n",
		para_num, para_id);

	/* para_id=0(dump), para_id=1(write) */
	if ((para_id <= 3) && (para_num != 1))
		goto format_error;

	/* para_id=6~23 case */
	if (((para_id >= 6) && (para_id < 24)) && (para_num != 2))
		goto format_error;

	/* para_id=24 case */
	if ((para_id == 24) && (para_num != 9))
		goto format_error;

	/* handle parameters assignment */
	switch (para_id) {
	case 0: /* dump twt parameters */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_tbl_idx=%d\n", twt_agrt_para_local.agrt_tbl_idx);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_ctrl_flag=%d\n", twt_agrt_para_local.agrt_ctrl_flag);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "own_mac_idx=%d\n", twt_agrt_para_local.own_mac_idx);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "flow_id=%d\n", twt_agrt_para_local.flow_id);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "peer_id_grp_id=%d\n", twt_agrt_para_local.peer_id_grp_id);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_sp_duration=%d\n", twt_agrt_para_local.agrt_sp_duration);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "bss_idx=%d\n", twt_agrt_para_local.bss_idx);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_sp_start_tsf_low=%d\n", twt_agrt_para_local.agrt_sp_start_tsf_low);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_sp_start_tsf_high=%d\n", twt_agrt_para_local.agrt_sp_start_tsf_high);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_sp_wake_intvl_mantissa=%d\n", twt_agrt_para_local.agrt_sp_wake_intvl_mantissa);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_sp_wake_intvl_exponent=%d\n", twt_agrt_para_local.agrt_sp_wake_intvl_exponent);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "is_role_ap=%d\n", twt_agrt_para_local.is_role_ap);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_para_bitmap=%d\n", twt_agrt_para_local.agrt_para_bitmap);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "persistence=%d\n", twt_agrt_para_local.persistence);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "ntbtt_before_reject=%d\n", twt_agrt_para_local.ntbtt_before_reject);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "grp_member_cnt=%d\n", twt_agrt_para_local.grp_member_cnt);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "agrt_tbl_idx_h=%d\n", twt_agrt_para_local.agrt_tbl_idx_h);
#ifdef DOT11_EHT_BE
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "tid_dl_bitmap=%d\n", twt_agrt_para_local.tid_dl_bitmap);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "tid_ul_bitmap=%d\n", twt_agrt_para_local.tid_ul_bitmap);
#else
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "reserved_d=%d\n", twt_agrt_para_local.reserved_d);
#endif /* DOT11_EHT_BE */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"sta_list[%d,%d,%d,%d,%d,%d,%d,%d]\n",
			twt_agrt_para_local.sta_list[0],
			twt_agrt_para_local.sta_list[1],
			twt_agrt_para_local.sta_list[2],
			twt_agrt_para_local.sta_list[3],
			twt_agrt_para_local.sta_list[4],
			twt_agrt_para_local.sta_list[5],
			twt_agrt_para_local.sta_list[6],
			twt_agrt_para_local.sta_list[7]);
		break;

	case 1:
		mt_asic_twt_agrt_update(wdev, &twt_agrt_para_local);
		break;

	case 2:
		twt_dump_resource(wdev);
		break;

	case 3:
		twt_get_current_tsf(wdev, current_tsf);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"wdev->OmacIdx(%d),current_tsf(0x%.8x, 0x%.8x)\n",
			wdev->OmacIdx, current_tsf[1], current_tsf[0]);
		break;

	case 4:
	{
		/* S1G action=11(twt information) */
		if (value[0] == CATE_S1G_ACTION_TWT_INFO) {
			struct _MLME_QUEUE_ELEM *elem = NULL;
			struct frame_twt_information *frame_in = NULL;
			UINT8 subfiled_size = 0;
			UINT64 tsf_64 = 0;

			os_alloc_mem(NULL, (UCHAR **)&(elem), sizeof(MLME_QUEUE_ELEM));

			if (elem) {
				os_zero_mem(elem, sizeof(MLME_QUEUE_ELEM));
				elem->wdev = wdev;
				elem->Wcid = value[1];
				frame_in = (struct frame_twt_information *)&elem->Msg;
				os_zero_mem(frame_in, sizeof(struct frame_twt_information));
				SET_TWT_INFO_FLOW_ID(frame_in->twt_info, value[2]);
				SET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(frame_in->twt_info, value[3]);
				subfiled_size = GET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(frame_in->twt_info);
				SET_TWT_INFO_ALL_TWT(frame_in->twt_info, value[4]);
				if (subfiled_size) {
					twt_get_current_tsf(wdev, current_tsf);
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"%s:current_tsf(msb:0x%.8x,lsb:0x%.8x)\n",
						__func__, current_tsf[1], current_tsf[0]);
					tsf_64 = current_tsf[1];
					tsf_64 = (tsf_64 << 32) + current_tsf[0]; /* 64bits tsf */
					tsf_64 += (value[5] * 1000); /* add x ms = x*1000 us */
					if (subfiled_size == 1) /* 32bits tsf */
						 tsf_64 = tsf_64 & 0x00000000ffffffff;
					else if (subfiled_size == 2) /* 48bits tsf */
						tsf_64 = tsf_64 & 0x0000ffffffffffff;
					frame_in->next_twt[0] = (UINT32)(tsf_64 & 0xffffffff);
					frame_in->next_twt[1] = (UINT32)(tsf_64 >> 32);
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"%s:current_tsf+%dms->subfiled_size(%d:%s),next_twt(msb:0x%.8x,lsb:0x%.8x)\n",
						__func__,
						value[5],
						subfiled_size,
						(subfiled_size == 1 ? "32bits" : (subfiled_size == 2 ? "48bits" : "64bits")),
						frame_in->next_twt[1],
						frame_in->next_twt[0]);
				}
				peer_twt_info_frame_action(ad, elem);
				os_free_mem(elem);
			} else {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"para_id(%d) fail\n", para_id);
			}
		} else if (value[0] == 12) {
			struct twt_resume_info resume_info = {0};
			struct _MAC_TABLE_ENTRY *entry = NULL;
			UINT16 wcid = value[1];
			UINT8 flow_id = value[2];
			UINT8 idle = value[3];

			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"%s:got event,sub_cmd=%d,wcid=%d,flow_id=%d,idle=%d\n",
				__func__, value[0], wcid, flow_id, idle);

			entry = entry_get(ad, wcid);
			wdev = entry->wdev;
			resume_info.bssinfo_idx = wdev->bss_info_argument.ucBssIndex;
			resume_info.wcid = wcid;
			resume_info.flow_id = flow_id;
			resume_info.idle = idle;

			twt_get_resume_event(wdev, &resume_info);
		}

		break;
	}
	case 5:
	{
		UINT16 wcid = value[0];
		UINT8 flow_id = value[1];

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:force to teardown iTWT wcid=%d,flow_id=%d\n",
			__func__, wcid, flow_id);

		twt_teardown_itwt(wdev, wcid, flow_id);
		break;
	}
	case 6:
		twt_agrt_para_local.agrt_tbl_idx = value[0];
		break;

	case 7:
		twt_agrt_para_local.agrt_ctrl_flag = value[0];
		break;

	case 8:
		twt_agrt_para_local.own_mac_idx = value[0];
		break;

	case 9:
		twt_agrt_para_local.flow_id = value[0];
		break;

	case 10:
		twt_agrt_para_local.peer_id_grp_id = value[0];
		break;

	case 11:
		twt_agrt_para_local.agrt_sp_duration = value[0];
		break;

	case 12:
		twt_agrt_para_local.bss_idx = value[0];
		break;

	case 13:
		twt_agrt_para_local.agrt_sp_start_tsf_low = value[0];
		break;

	case 14:
		twt_agrt_para_local.agrt_sp_start_tsf_high = value[0];
		break;

	case 15:
		twt_agrt_para_local.agrt_sp_wake_intvl_mantissa = value[0];
		break;

	case 16:
		twt_agrt_para_local.agrt_sp_wake_intvl_exponent = value[0];
		break;

	case 17:
		twt_agrt_para_local.is_role_ap = value[0];
		break;

	case 18:
		twt_agrt_para_local.agrt_para_bitmap = value[0];
		break;

	case 19:
		twt_agrt_para_local.persistence = value[0];
		break;

	case 20:
		twt_agrt_para_local.ntbtt_before_reject = value[0];
		break;

	case 21:
		twt_agrt_para_local.grp_member_cnt = value[0];
		break;

	case 22:
		twt_agrt_para_local.agrt_tbl_idx_h = value[0];
		break;

	case 23:
#ifdef DOT11_EHT_BE
		twt_agrt_para_local.tid_dl_bitmap = value[0];
		twt_agrt_para_local.tid_ul_bitmap = value[0];
#else
		twt_agrt_para_local.reserved_d = value[0];
#endif /* DOT11_EHT_BE */
		break;

	case 24:
		for (i = 0; i < ARRAY_SIZE(twt_agrt_para_local.sta_list); i++)
			twt_agrt_para_local.sta_list[i] = value[i];
		break;

	case 25: /* allocate twt h/w resource */
	{
		struct TWT_AGRT_MGMT_T agrt_mgmt = {0};

		mt_asic_twt_agrt_alloc(wdev, &twt_agrt_para_local, &agrt_mgmt);
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:hw_res_alloc::sts=%d,wcid=%d,res_id=%d\n",
			__func__, agrt_mgmt.sts, agrt_mgmt.wcid, agrt_mgmt.res_id);
		break;
	}
	case 26: /* free twt h/w resource */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:hw_res_free::wcid=%d,res_id=%d\n",
			__func__, twt_agrt_para_local.peer_id_grp_id, twt_agrt_para_local.agrt_tbl_idx);
		mt_asic_twt_agrt_free(wdev, &twt_agrt_para_local);
		break;

	default:
		break;
	}

	return TRUE;

format_error:
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Format error! correct format:\n");
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"iwpriv ra0 set twt=[para_id]:[value]\n");
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"\t 1. Dump twt parameters, para_id=0\n");
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"\t 2. Write twt parameters to FW, para_id=1\n");
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"\t 2. Update twt parameters, para_id=6~24:value\n");

	return FALSE;
}

INT set_btwt_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	POS_COOKIE	obj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	UINT32 para_id = 0;
	UINT32 value[15] = {0};
	INT para_num = 0;
	UINT8 band = 0;
	struct twt_ctrl_btwt btwt_para = {0};
	struct twt_ctrl_btwt *p_btwt_para = NULL;

	wdev = get_wdev_by_ioctl_idx_and_iftype(ad, obj->ioctl_if, obj->ioctl_if_type);

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	if (arg == NULL || strlen(arg) == 0)
		goto format_error;

	para_num = sscanf(arg, "%d", &para_id);
	if (para_num < 0)
		return FALSE;

	para_num = sscanf(arg, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", &para_id,
			&value[0], &value[1], &value[2], &value[3], &value[4], &value[5],
			&value[6], &value[7], &value[8], &value[9], &value[10], &value[11],
			&value[12], &value[13], &value[14]);

	if (para_num < 0)
		return FALSE;

	MTWF_PRINT("%s:para_num=%d, para_id=%d\n",
		__func__, para_num, para_id);

	/* para_id = 0(list), cmd_format=0 */
	if ((para_id == 0) && (para_num != 1))
		goto format_error;

	/* para_id = 1(acquire), cmd_format=1:value[0~11] */
	if ((para_id == 1) && (para_num != 13))
		goto format_error;

	/* para_id = 2(release), cmd_format=2:btwt_id */
	if ((para_id == 2) && (para_num != 2))
		goto format_error;

	/* para_id = 4(test), cmd_format=4:value[0:10] */
	if ((para_id == 4) && !(para_num == 12 || para_num == 6 || para_num == 7))
		goto format_error;

	/* para_id = 5 cmd_format=5:value[0~7] */
	if ((para_id == 5) && (para_num != 9))
		goto format_error;

#ifdef DOT11_EHT_BE
	/* para_id = 6(test), cmd_format=6:value[0:14] */
	if ((para_id == 6) && !(para_num == 13 || para_num == 16))
		goto format_error;
#endif /* DOT11_EHT_BE */

	band = HcGetBandByWdev(wdev);

	/* handle parameters assignment */
	switch (para_id) {
	case 0: /* list */
	{
		UINT32 btwt_id_bitmap = 0;

		twt_dump_btwt_elem(wdev, &btwt_id_bitmap);
		break;
	}
	case 1: /* acquire */
		/*
		para_id  = 1(acquire)
		value[0] = b.TWT ID
		value[1] = SP
		value[2] = Mantissa
		value[3] = Exponent
		value[4] = Trigger
		value[5] = Flow type
		value[6] = Protect
		value[7] = Twt setup cmd (accept=4/alternate=5/reject=7)
		value[8] = b.TWT Persistence
		value[9] = Twt info. Disable
		value[10] = Wake duration unit
		value[11] = b.TWT recommendation
		*/
		p_btwt_para = &btwt_para;
		p_btwt_para->band = band;
		p_btwt_para->btwt_id = value[0];
		p_btwt_para->agrt_sp_duration = value[1];
		p_btwt_para->agrt_sp_wake_intvl_mantissa = value[2];
		p_btwt_para->agrt_sp_wake_intvl_exponent = value[3];
		if (value[4])
			SET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_TRIGGER);
		if (value[5] == 0)
			SET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE);
		if (value[6])
			SET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_PROTECT);
		p_btwt_para->twt_setup_cmd = value[7];
		p_btwt_para->persistence = value[8];
		p_btwt_para->twt_info_frame_dis = value[9];
		p_btwt_para->wake_dur_unit = value[10];
		p_btwt_para->btwt_recommendation = value[11];

		MTWF_PRINT(" Acquire btwt element\n");
		MTWF_PRINT(" band=%d\n", p_btwt_para->band);
		MTWF_PRINT(" btwt_id=%d\n", p_btwt_para->btwt_id);
		MTWF_PRINT(" sp=%d\n", p_btwt_para->agrt_sp_duration);
		MTWF_PRINT(" man=%d\n", p_btwt_para->agrt_sp_wake_intvl_mantissa);
		MTWF_PRINT(" exp=%d\n", p_btwt_para->agrt_sp_wake_intvl_exponent);
		MTWF_PRINT(" trig=%d\n", GET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_TRIGGER));
		MTWF_PRINT(" f_type=%d\n", GET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) ? FALSE : TRUE);
		MTWF_PRINT(" protect=%d\n", GET_AGRT_PARA_BITMAP(p_btwt_para, TWT_AGRT_PARA_BITMAP_IS_PROTECT));
		MTWF_PRINT(" setup_cmd=%d\n", p_btwt_para->twt_setup_cmd);
		MTWF_PRINT(" peristence=%d\n", p_btwt_para->persistence);
		MTWF_PRINT(" twt_info_frame_dis=%d\n", p_btwt_para->twt_info_frame_dis);
		MTWF_PRINT(" wake_dur_unit=%d\n", p_btwt_para->wake_dur_unit);
		MTWF_PRINT(" rec=%d\n", p_btwt_para->btwt_recommendation);
		twt_acquire_btwt_node(wdev, p_btwt_para);

		break;

	case 2: /* release */
		/*
		para_id  = 2(release)
		value[0] = b.TWT ID
		*/
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%s:Release btwt element,band=%d,btwt_id=%d\n",
			__func__, band, value[0]);
		if (value[0] == ALL_BTWT_ID) {
			UINT32 btwt_id_bitmap = 0;
			UINT8 i = 0;

			twt_dump_btwt_elem(wdev, &btwt_id_bitmap);
			for (i = 0; i < TWT_BTWT_ID_NUM; i++) {
				if (btwt_id_bitmap & (1 << i)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN, "%s:Release->btwt_id=%d\n", __func__, i);
					twt_remove_btwt_resouce(wdev, i);
				}
			}
		} else
			twt_remove_btwt_resouce(wdev, value[0]);

		break;
	case 4:
		/*
		AP get STA join btwt_id action frame (simluate)
			para_id  = 4
			value[0] = twt_action_type=6
			value[1] = wcid=1
			value[2] = nego_type=3
			value[3] = twt_setup_cmd=0
			value[4] = trigger=1
			value[5] = flow_type=0
			value[6] = rec=0
			value[7] = exponent=10
			value[8] = sp_duration=64
			value[9] = mantissa=32
			value[10]= btwt_id=1
		*/
		if (value[0] == CATE_S1G_ACTION_TWT_SETUP) {
			UINT8 nego_type = value[2];

			if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
				struct _MAC_TABLE_ENTRY *entry = NULL;
				struct frame_btwt_setup *frame = NULL;
				struct btwt_para_set *btwt_para = NULL;
				struct btwt_para_set_fix *btwt_para_fix = NULL;
				struct _MLME_QUEUE_ELEM *elem = NULL;
				UINT8 btwt_element_num = 1;
				UINT8 frame_len = sizeof(struct frame_btwt_setup) + sizeof(struct btwt_para_set_fix) * btwt_element_num;
				UINT16 wcid = value[1];

				if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
								"Error.Invalid uc wcid=%d\n", wcid);
					return FALSE;
				}

				if (os_alloc_mem(ad, (UCHAR **)&(elem), sizeof(MLME_QUEUE_ELEM)) != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						" allocate memory failed, please check\n");
					return FALSE;
				}

				os_zero_mem(elem, frame_len);

				entry = entry_get(ad, wcid);

				if (elem) {
					elem->wdev = wdev;
					elem->Wcid = wcid;
					/* build btwt action frame */
					frame = (struct frame_btwt_setup *)&elem->Msg;
					ActHeaderInit(ad, &frame->hdr, wdev->if_addr, entry->Addr, entry->wdev->bssid);
					frame->category = CATEGORY_UNPROT_S1G;
					frame->s1g_action = CATE_S1G_ACTION_TWT_SETUP;
					frame->token = 123;
					frame->elem_id = IE_TWT;
					frame->len = 1 + sizeof(struct btwt_para_set_fix) * btwt_element_num; /* 1=control */
					/* btwt control */
					frame->control |= SET_TWT_CTRL_NEGO_TYPE(nego_type) |
						SET_TWT_CTRL_INFO_FRM_DIS(1) |
						SET_TWT_CTRL_WAKE_DUR_UNIT(0);
					/* btwt para */
					btwt_para = (struct btwt_para_set *)&frame->btwt_para[0];
					btwt_para_fix = &btwt_para->btwt_para_fix;
					btwt_para_fix->req_type |= SET_TWT_RT_REQUEST(1) |
						SET_TWT_RT_SETUP_CMD(value[3]) |
						SET_TWT_RT_TRIGGER(value[4]) |
						SET_TWT_RT_IMPLICIT_LAST(1) |
						SET_TWT_RT_FLOW_TYPE(value[5]) |
						SET_TWT_RT_BTWT_REC(value[6]) |
						SET_TWT_RT_WAKE_INTVAL_EXP(value[7]);
					btwt_para_fix->target_wake_time = 0;
					btwt_para_fix->duration = value[8];
					btwt_para_fix->mantissa = value[9];
					btwt_para_fix->btwt_info |= SET_BTWT_INFO_BTWT_ID(value[10]) |
						SET_BTWT_INFO_BTWT_P(0);

					MTWF_PRINT("%s:*STA join btwt_id=%d request (simulate)\n", __func__, value[10]);
					MTWF_PRINT("%s:wcid=%d\n", __func__, value[1]);
					MTWF_PRINT("%s:nego_type=%d\n", __func__, value[2]);
					MTWF_PRINT("%s:twt_setup_cmd=%d\n", __func__, value[3]);
					MTWF_PRINT("%s:trigger=%d\n", __func__, value[4]);
					MTWF_PRINT("%s:flow_type=%d\n", __func__, value[5]);
					MTWF_PRINT("%s:rec=%d\n", __func__, value[6]);
					MTWF_PRINT("%s:exponent=%d\n", __func__, value[7]);
					MTWF_PRINT("%s:sp_duration=%d\n", __func__, value[8]);
					MTWF_PRINT("%s:mantissa=%d\n", __func__, value[9]);
					MTWF_PRINT("%s:btwt_id=%d\n", __func__, value[10]);
					MTWF_PRINT("%s:frame_len=%d\n", __func__, frame_len);
					MTWF_PRINT("(%s:A1=%pM,A2=%pM,A3=%pM\n", __func__, frame->hdr.Addr1, frame->hdr.Addr2, frame->hdr.Addr3);

					peer_twt_action(ad, elem);
					os_free_mem(elem);
				}
			}
		} else if (value[0] == CATE_S1G_ACTION_TWT_TEARDOWN) {
			UINT8 initiate_role = value[1];
			UINT8 btwt_id = value[2];
			UINT8 nego_type = value[3];
			BOOLEAN teardown_all_twt = value[4] ? TRUE : FALSE;

			/*
			AP initiate mlem to terdwon btwt or teardown_all_twt
				para_id  = 4
				value[0] = twt_action_type=7
				value[1] = 1 (AP initiate mlem to terdwon btwt or teardown_all_twt)
				value[2] = btwt_id
				value[3] = nego_type=3
				value[4] = teardown_all_twt
			STA initiate teardown action frame to leave btwt_id (simluate)
				para_id  = 4
				value[0] = twt_action_type=7
				value[1] = 0 (STA initiate teardown action frame to terdwon btwt_id)
				value[2] = btwt_id
				value[3] = nego_type=3
				value[4] = teardown_all_twt
				value[5] = wcid
			*/
			if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
				if (initiate_role == 1) { /* AP initiate */
					MTWF_PRINT("%s:*AP teardown btwt with btwt_id=%d,teardown_all_twt=%d\n",
						__func__, btwt_id, teardown_all_twt);

					twt_tardown_btwt(wdev, NULL, nego_type, btwt_id, teardown_all_twt);
				} else if (initiate_role == 0) { /* STA initiate */
					struct _MAC_TABLE_ENTRY *entry = NULL;
					struct _MLME_QUEUE_ELEM *elem = NULL;
					struct frame_btwt_teardown *frame = NULL;
					UINT8 frame_len = sizeof(struct frame_twt_teardown);
					UINT16 wcid = value[5];

					if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
								"Error.Invalid uc wcid=%d\n", wcid);
						return FALSE;
					}

					if (os_alloc_mem(ad, (UCHAR **)&(elem), sizeof(MLME_QUEUE_ELEM)) != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
							" allocate memory failed, please check\n");
						return FALSE;
					}

					os_zero_mem(elem, frame_len);

					if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
						os_free_mem(elem);
						return FALSE;
					}
					entry = entry_get(ad, wcid);

					if (elem) {
						elem->wdev = wdev;
						elem->Wcid = wcid;
						/* build btwt action frame */
						frame = (struct frame_btwt_teardown *)&elem->Msg;
						ActHeaderInit(ad, &frame->hdr, wdev->if_addr, entry->Addr, entry->wdev->bssid);
						frame->category = CATEGORY_UNPROT_S1G;
						frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
						frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(btwt_id) |
							SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
							SET_BTWT_FLOW_TEARDOWN_ALL_TWT(teardown_all_twt);

						MTWF_PRINT("%s:*STA teardown btwt with btwt_id=%d,teardown_all_twt=%d,wcid=%d (simulate)\n",
							__func__, btwt_id, teardown_all_twt, wcid);
						MTWF_PRINT("%s:A1=%pM,A2=%pM,A3=%pM,frame_len=%d\n",
							__func__, frame->hdr.Addr1, frame->hdr.Addr2, frame->hdr.Addr3, frame_len);

						peer_twt_action(ad, elem);
						os_free_mem(elem);
					}
				}
			}
		}
		break;
	case 5: /* teardown specific scheduled STA */
	{
		/*
		para_id  = 5
		value[0~5] = aa:bb:cc:dd:ee:ff
		value[6] = nego_type
		value[7] = btwt_id (1~31:specific BTWT_ID, 255: all BTWT_ID(1~31)
		*/
		UCHAR peer_addr[MAC_ADDR_LEN] = {0};
		UINT8 nego_type = value[6];
		UINT8 btwt_id = value[7];

		peer_addr[0] = (UCHAR)value[0];
		peer_addr[1] = (UCHAR)value[1];
		peer_addr[2] = (UCHAR)value[2];
		peer_addr[3] = (UCHAR)value[3];
		peer_addr[4] = (UCHAR)value[4];
		peer_addr[5] = (UCHAR)value[5];

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%s: Teardown peer(%pM) with nt=%d,btwt_id=%d\n",
			__func__, peer_addr, nego_type, btwt_id);

		twt_tardown_btwt(wdev,
			peer_addr,
			nego_type,
			btwt_id,
			(btwt_id == ALL_BTWT_ID) ? TRUE : FALSE);

		break;
	}
#ifdef DOT11_EHT_BE
	case 6:
	{
		if (!wlan_config_get_eht_restricted_twt(wdev)) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Need to enable rTWT for this test!\n");
			return FALSE;
		}
		/*
		AP get STA join btwt_id action frame using rTWT (simluate)
			para_id  = 6
			value[0] = twt_action_type=6
			value[1] = wcid=1
			value[2] = nego_type=3
			value[3] = twt_setup_cmd=2
			value[4] = trigger=1
			value[5] = flow_type=1
			value[6] = rec=4
			value[7] = exponent=10
			value[8] = sp_duration=64
			value[9] = mantissa=32
			value[10]= btwt_id=1
			value[11]= rTWT traffic info present
			value[12]= traffic info ctrl
			value[13]= tid_dl_bitmap
			value[14]= tid_ul_bitmap
		*/
		if (value[0] == CATE_S1G_ACTION_TWT_SETUP) {
			UINT8 nego_type = value[2];

			if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
				struct _MAC_TABLE_ENTRY *entry = NULL;
				struct frame_btwt_setup *frame = NULL;
				struct btwt_para_set *btwt_para = NULL;
				struct btwt_para_set_fix *btwt_para_fix = NULL;
				struct _MLME_QUEUE_ELEM *elem = NULL;
				UINT8 btwt_element_num = 1;
				UINT8 frame_len = sizeof(struct frame_btwt_setup) + sizeof(struct btwt_para_set_fix) * btwt_element_num;
				UINT16 wcid = value[1];
				UINT32 current_tsf[2] = {0};
				UINT64 twt_current_tsf = 0;
				UINT64 twt_interval = 0;
				UINT64 future_target_wake_time = 0;

				if (!VALID_UCAST_ENTRY_WCID(ad, wcid)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
							"Error.Invalid uc wcid=%d\n", wcid);
					return FALSE;
				}

				if (os_alloc_mem(ad, (UCHAR **)&(elem), sizeof(MLME_QUEUE_ELEM)) != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						" allocate memory failed, please check\n");
					return FALSE;
				}

				os_zero_mem(elem, frame_len);

				entry = entry_get(ad, wcid);

				if (elem) {
					elem->wdev = wdev;
					elem->Wcid = wcid;
					/* build btwt action frame */
					frame = (struct frame_btwt_setup *)&elem->Msg;
					ActHeaderInit(ad, &frame->hdr, wdev->if_addr, entry->Addr, entry->wdev->bssid);
					frame->category = CATEGORY_UNPROT_S1G;
					frame->s1g_action = CATE_S1G_ACTION_TWT_SETUP;
					frame->token = 123;
					frame->elem_id = IE_TWT;
					frame->len = 1 + sizeof(struct btwt_para_set_fix) * btwt_element_num; /* 1=control */
					if (value[11])
						frame->len += (sizeof(struct rtwt_para_ext_set) * btwt_element_num);
					/* btwt control */
					frame->control |= (SET_TWT_CTRL_NEGO_TYPE(nego_type) |
										SET_TWT_CTRL_INFO_FRM_DIS(1) |
										SET_TWT_CTRL_WAKE_DUR_UNIT(0));
					/* btwt para */
					btwt_para = (struct btwt_para_set *)&frame->btwt_para[0];
					btwt_para_fix = &btwt_para->btwt_para_fix;
					btwt_para_fix->req_type |= (SET_TWT_RT_REQUEST(1) |
												SET_TWT_RT_SETUP_CMD(value[3]) |
												SET_TWT_RT_TRIGGER(value[4]) |
												SET_TWT_RT_IMPLICIT_LAST(1) |
												SET_TWT_RT_FLOW_TYPE(value[5]) |
												SET_TWT_RT_BTWT_REC(value[6]) |
												SET_TWT_RT_WAKE_INTVAL_EXP(value[7]));
					twt_get_current_tsf(wdev, current_tsf);
					twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
					twt_interval = ((UINT64)value[9]) << value[7];
					future_target_wake_time = twt_current_tsf + twt_interval;
					btwt_para_fix->target_wake_time = (UINT16)((future_target_wake_time >> 10) & 0xffff);
					btwt_para_fix->duration = value[8];
					btwt_para_fix->mantissa = value[9];
					btwt_para_fix->btwt_info |= (SET_BTWT_INFO_BTWT_ID(value[10]) |
												SET_BTWT_INFO_RTWT_TRAFFIC_INFO_PRESENT(value[11]) |
												SET_BTWT_INFO_RTWT_SCHEDULE_INFO(0) |
												SET_BTWT_INFO_BTWT_P(255));
					if (value[11]) {
						btwt_para->rtwt_ext_set.traffic_info_ctrl = value[12];
						btwt_para->rtwt_ext_set.tid_dl_bitmap = value[13];
						btwt_para->rtwt_ext_set.tid_ul_bitmap = value[14];
					}

					MTWF_PRINT("%s:*STA join btwt_id=%d request using rTWT (simulate)\n", __func__, value[10]);
					MTWF_PRINT("%s:wcid=%d\n", __func__, value[1]);
					MTWF_PRINT("%s:nego_type=%d\n", __func__, value[2]);
					MTWF_PRINT("%s:twt_setup_cmd=%d\n", __func__, value[3]);
					MTWF_PRINT("%s:trigger=%d\n", __func__, value[4]);
					MTWF_PRINT("%s:flow_type=%d\n", __func__, value[5]);
					MTWF_PRINT("%s:rec=%d\n", __func__, value[6]);
					MTWF_PRINT("%s:exponent=%d\n", __func__, value[7]);
					MTWF_PRINT("%s:sp_duration=%d\n", __func__, value[8]);
					MTWF_PRINT("%s:mantissa=%d\n", __func__, value[9]);
					MTWF_PRINT("%s:btwt_id=%d\n", __func__, value[10]);
					MTWF_PRINT("%s:rTWT traffic info present=%d\n", __func__, value[11]);
					MTWF_PRINT("%s:traffic info ctrl=%d\n", __func__, value[12]);
					MTWF_PRINT("%s:DL TID bitmap=%d\n", __func__, value[13]);
					MTWF_PRINT("%s:UL TID bitmap=%d\n", __func__, value[13]);
					MTWF_PRINT("%s:frame_len=%d\n", __func__, frame_len);
					MTWF_PRINT("(%s:A1=%pM,A2=%pM,A3=%pM\n", __func__, frame->hdr.Addr1, frame->hdr.Addr2, frame->hdr.Addr3);

					peer_twt_action(ad, elem);
					os_free_mem(elem);
				}
			}
		}
		break;
	}
#endif /* DOT11_EHT_BE */

	default:
		break;
	}

	return TRUE;

format_error:
	MTWF_PRINT("Cmd format error! correct format:\n");
	if (para_id == 0) {
		MTWF_PRINT("iwpriv IF set btwt=0\n");
	} else if (para_id == 1) {
		MTWF_PRINT("iwpriv IF set btwt=1:btwt_id:sp:man:exp:trig:f_type:setup_cmd:peristence:twt_info_dis:wake_dur_unit:rec\n");
	} else if (para_id == 2) {
		MTWF_PRINT("iwpriv IF set btwt=2:btwt_id\n");
	} else if (para_id == 4) {
		MTWF_PRINT("iwpriv IF set btwt=4:value[0:4] or 4:value[0:5] or 4:value[0:10]\n");
	} else if (para_id == 5) {
		MTWF_PRINT("iwpriv IF set btwt=5:value[0:7]\n");
	}
#ifdef DOT11_EHT_BE
	else if (para_id == 6) {
		MTWF_PRINT("iwpriv IF set btwt=6:value[0:11] or 6:value[0:14]\n");
	}
#endif /* DOT11_EHT_BE */

	return FALSE;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

INT	Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->WIFItestbed.bShortGI = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bShortGI = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(ForceShortGI=%d)\n", pAd->WIFItestbed.bShortGI);
	return TRUE;
}

INT	Set_ForceGF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->WIFItestbed.bGreenField = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bGreenField = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(ForceGF=%d)\n", pAd->WIFItestbed.bGreenField);
	return TRUE;
}

INT	Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bMIMOPSEnable = FALSE;
	else if (Value == 1)
		pAd->CommonCfg.bMIMOPSEnable = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_PRINT("(HtMimoPs=%d)\n", pAd->CommonCfg.bMIMOPSEnable);
	return TRUE;
}

#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	UCHAR bBssCoexEnable = os_str_tol(pParam, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	pAd->CommonCfg.bBssCoexEnable = ((bBssCoexEnable == 1) ? TRUE : FALSE);
	MTWF_PRINT("Set bBssCoexEnable=%d!\n", pAd->CommonCfg.bBssCoexEnable);

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd) && pAd->CommonCfg.bBssCoexEnable) {
		/* Disable BSS Coex Enable Fields */
		pAd->CommonCfg.bBssCoexEnable = FALSE;
	}
#endif

	if ((pAd->CommonCfg.bBssCoexEnable == FALSE)
		&& pAd->CommonCfg.bRcvBSSWidthTriggerEvents) {
		/* switch back 20/40 */
		MTWF_PRINT("Set bBssCoexEnable:  Switch back 20/40.\n");
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

		if ((HcIsRfSupport(pAd, RFIC_24GHZ)) && (wlan_config_get_ht_bw(wdev) == BW_40))
			wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_config_get_ext_cha(wdev));
	}

	return TRUE;
}

INT Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	pAd->CommonCfg.BssCoexApCntThr = os_str_tol(pParam, 0, 10);
	MTWF_PRINT("Set BssCoexApCntThr=%d!\n", pAd->CommonCfg.BssCoexApCntThr);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_coex_cnt_thr(RTMP_ADAPTER *pAd, u8 value)
{
	pAd->CommonCfg.BssCoexApCntThr = value;
	MTWF_PRINT("Set BssCoexApCntThr=%d!\n", pAd->CommonCfg.BssCoexApCntThr);
	return TRUE;
}
#endif

#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
INT	Set_VhtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *tdev;
	UCHAR Bandidx = 0;
	UCHAR i = 0;
	ULONG vht_cw;
	UCHAR vht_bw, vht_bw_max_cap = VHT_BW_8080;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/* Sanity : VHT_BW_8080 only for MT7915 chips,  */
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_BW160C_STD))
		vht_bw_max_cap = VHT_BW_160;

	if (!wdev)
		return FALSE;

	Bandidx = HcGetBandByWdev(wdev);
	vht_cw = os_str_tol(arg, 0, 10);

	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) {
		MTWF_PRINT("(error Channel = %d)\n", wdev->channel);
		return FALSE;
	}

	if (vht_cw > vht_bw_max_cap)
		MTWF_PRINT("vht_cw=%lu, should be <= %u, correct to %u!\n", vht_cw, vht_bw_max_cap, VHT_BW_2040);


	if (vht_cw <= vht_bw_max_cap)
		vht_bw = vht_cw;
	else
		vht_bw = VHT_BW_2040;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

#ifdef CONFIG_MAP_SUPPORT
			if (!IS_MAP_ENABLE(pAd)) {
				MTWF_PRINT("Set VHT BW CONFIG");
				wlan_config_set_vht_bw(tdev, vht_bw);
			}
#else
			wlan_config_set_vht_bw(tdev, vht_bw);
#endif
			if (WMODE_CAP_AC(tdev->PhyMode))
				wlan_operate_set_vht_bw(tdev, vht_bw);
			SetCommonHtVht(pAd, tdev);
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_HEOP, TRUE);
		}
	}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Update Beacon to Reflect BW Changes */
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd))
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
#endif /* BW_VENDOR10_CUSTOM_FEATURE */
	MTWF_PRINT("(VHT_BW=%d)\n", vht_bw);
	return TRUE;
}

INT set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR *bwsig_str[] = {"NONE", "STATIC", "DYNAMIC"};
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ULONG bw_signal = os_str_tol(arg, 0, 10);

	if (bw_signal > BW_SIGNALING_DYNAMIC)
		bw_signal = BW_SIGNALING_DISABLE;
	wlan_config_set_vht_bw_sig(wdev, bw_signal);

	if (bw_signal > BW_SIGNALING_DISABLE) {
		struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T ExtCmdRtsSigTaCfg = {0};
		struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T ExtCmdSchDetDisCfg = {0};

		ExtCmdRtsSigTaCfg.Enable = TRUE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RTS_SIGTA_EN_FEATURE, &ExtCmdRtsSigTaCfg);

		ExtCmdSchDetDisCfg.Disable = FALSE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_SCH_DET_DIS_FEATURE, &ExtCmdSchDetDisCfg);

		/*Set RTS Threshold to a lower Value */
		/* Otherwise RTS Packets are not send - TGAC 5.2.67A*/
		set_rts_len_thld(wdev, 500);
	} else {
		struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T ExtCmdRtsSigTaCfg = {0};
		struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T ExtCmdSchDetDisCfg = {0};

		ExtCmdRtsSigTaCfg.Enable = FALSE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RTS_SIGTA_EN_FEATURE, &ExtCmdRtsSigTaCfg);

		ExtCmdSchDetDisCfg.Disable = TRUE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_SCH_DET_DIS_FEATURE, &ExtCmdSchDetDisCfg);

		/*Set RTS Threshold to a lower Value */
		/* Otherwise RTS Packets are not send - TGAC 5.2.67A*/
		set_rts_len_thld(wdev, 500);
	}

	MTWF_PRINT("vht_bw_signal = %s\n", bwsig_str[bw_signal]);

	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_vht_bw_signaling(
	RTMP_ADAPTER * pAd,
	u32 inf_idx,
	INT inf_type,
	UCHAR value
)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return -EINVAL;

	if (value > BW_SIGNALING_DYNAMIC)
		value = BW_SIGNALING_DISABLE;
	wlan_config_set_vht_bw_sig(wdev, value);

	if (value > BW_SIGNALING_DISABLE) {

		UniCmdConfigSetRtsSignalEn(pAd, wdev, TRUE);
		UniCmdConfigSetSchDetDis(pAd, wdev, FALSE);
		/*Set RTS Threshold to a lower Value */
		/* Otherwise RTS Packets are not send - TGAC 5.2.67A*/
		set_rts_len_thld(wdev, 500);
	} else {

		UniCmdConfigSetRtsSignalEn(pAd, wdev, FALSE);
		UniCmdConfigSetSchDetDis(pAd, wdev, TRUE);
		/*Set RTS Threshold to a lower Value */
		/* Otherwise RTS Packets are not send - TGAC 5.2.67A*/
		set_rts_len_thld(wdev, 500);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
		"set_vht_bw_signaling=%d\n", wlan_config_get_vht_bw_sig(wdev));
	return TRUE;
}
#endif

INT	Set_VhtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value > 1) {
		MTWF_PRINT("Invalid arguments!\n");
		return FALSE;
	}

	wlan_config_set_vht_ldpc(wdev, (UCHAR)Value);
	wlan_operate_set_vht_ldpc(wdev, (UCHAR)Value);

	MTWF_PRINT("%s:(VhtLdpc=%d)\n",	__func__, wlan_config_get_vht_ldpc(wdev));
	return TRUE;
}

INT	Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	Value = os_str_tol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_vht_stbc(wdev, STBC_USE);
	else if (Value == STBC_NONE)
		wlan_config_set_vht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd, wdev);
	MTWF_PRINT("(VhtStbc=%d)\n", wlan_config_get_vht_stbc(wdev));
	return TRUE;
}

INT	Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bNonVhtDisallow = FALSE;
	else
		pAd->CommonCfg.bNonVhtDisallow = TRUE;

	MTWF_PRINT("(bNonVhtDisallow=%d)\n", pAd->CommonCfg.bNonVhtDisallow);
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_disallow_vht(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);

	if (!wdev)
		return FALSE;

	if (value == 0)
		pAd->CommonCfg.bNonVhtDisallow = FALSE;
	else
		pAd->CommonCfg.bNonVhtDisallow = TRUE;

	MTWF_PRINT("(bNonVhtDisallow=%d)\n",  pAd->CommonCfg.bNonVhtDisallow);
	return TRUE;
}
#endif

#endif /* DOT11_VHT_AC */

#ifdef ETH_CONVERT_SUPPORT
INT Set_EthConvertMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/*
		Dongle mode: it means use our default MAC address to connect to AP, and
					support multiple internal PCs connect to Internet via this default MAC
		Clone mode : it means use one specific MAC address to connect to remote AP, and
					just the node who owns the MAC address can connect to Internet.
		Hybrid mode: it means use some specific MAC address to connecto to remote AP, and
					support mulitple internal PCs connect to Internet via this specified MAC address.
	*/
	if (rtstrcasecmp(arg, "dongle") == TRUE) {
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_DONGLE;
		NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
		pAd->EthConvert.CloneMacVaild = TRUE;
	} else if (rtstrcasecmp(arg, "clone") == TRUE) {
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_CLONE;
		pAd->EthConvert.CloneMacVaild = FALSE;
	} else if (rtstrcasecmp(arg, "hybrid") == TRUE) {
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_HYBRID;
		pAd->EthConvert.CloneMacVaild = FALSE;
	} else {
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_DISABLE;
		pAd->EthConvert.CloneMacVaild = FALSE;
	}

	pAd->EthConvert.macAutoLearn = FALSE;
	MTWF_PRINT("EthConvertMode=%d!\n", pAd->EthConvert.ECMode);
	return TRUE;
}

INT Set_EthCloneMac_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
	extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
	/*
		If the input is the zero mac address, it means use our default(from EEPROM) MAC address as out-going
		   MAC address.
		If the input is the broadcast MAC address, it means use the source MAC of first packet forwarded by
		   our device as the out-going MAC address.
		If the input is any other specific valid MAC address, use it as the out-going MAC address.
	*/
	pAd->EthConvert.macAutoLearn = FALSE;

	if (strlen(arg) == 0) {
		NdisZeroMemory(&pAd->EthConvert.EthCloneMac[0], MAC_ADDR_LEN);
		goto done;
	}

	if (rtstrmactohex(arg, (RTMP_STRING *) &pAd->EthConvert.EthCloneMac[0]) == FALSE)
		goto fail;

done:
	MTWF_PRINT("CloneMac = "MACSTR"\n", MAC2STR(pAd->EthConvert.EthCloneMac));

	if (NdisEqualMemory(&pAd->EthConvert.EthCloneMac[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Use our default Mac address for cloned MAC!\n");
		NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
		pAd->EthConvert.CloneMacVaild = TRUE;
	} else if (NdisEqualMemory(&pAd->EthConvert.EthCloneMac[0], &BROADCAST_ADDR[0], MAC_ADDR_LEN)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Use first frowarded Packet's source Mac for cloned MAC!\n");
		NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
		pAd->EthConvert.CloneMacVaild = FALSE;
		pAd->EthConvert.macAutoLearn = TRUE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Use user assigned spcific Mac address for cloned MAC!\n");
		pAd->EthConvert.CloneMacVaild = TRUE;
	}

	MTWF_PRINT("After ajust, CloneMac = "MACSTR"\n", MAC2STR(pAd->EthConvert.EthCloneMac));
	return TRUE;
fail:
	MTWF_PRINT("wrong Mac Address format or length!\n");
	NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	return FALSE;
}
#endif /* ETH_CONVERT_SUPPORT */

INT	Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT	fix_tx_mode = RT_CfgSetFixedTxPhyMode(arg);
	INT32       IfIdx = pObj->ioctl_if;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (VALID_MBSS(pAd, IfIdx))
			wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return FALSE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
			wdev = &pAd->StaCfg[IfIdx].wdev;
		else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return FALSE;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		wdev->DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;

	MTWF_PRINT("%s():(FixedTxMode=%d)\n", __func__, fix_tx_mode);
	return TRUE;
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = os_str_tol(arg, 0, 10);
#ifdef RTMP_MAC_PCI

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#endif /* RTMP_MAC_PCI */
		{
			MTWF_PRINT("Can not switch operate mode on interface up !!\n");
			return FALSE;
		}

	if (Value == 0)
		pAd->OpMode = OPMODE_STA;
	else if (Value == 1)
		pAd->OpMode = OPMODE_AP;
	else
		return FALSE; /*Invalid argument*/

	MTWF_PRINT("(OpMode=%s)\n", pAd->OpMode == 1 ? "AP Mode" : "STA Mode");
	return TRUE;
}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR LongRetryLimit = (UCHAR)os_str_tol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
	MTWF_PRINT("(LongRetryLimit=0x%x)\n",
			 LongRetryLimit);
	return TRUE;
}

INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ShortRetryLimit = (UCHAR)os_str_tol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
	MTWF_PRINT("(ShortRetryLimit=0x%x)\n",
			 ShortRetryLimit);
	return TRUE;
}

INT Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return RT_CfgSetAutoFallBack(pAd, arg);
}

#ifdef MEM_ALLOC_INFO_SUPPORT
INT Show_MemInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT show = 0;
	UINT64 option = 0;
	CHAR *param;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	param = rstrtok(arg, "-");

	if (param) {
		show = os_str_tol(param, 0, 10);
		param = rstrtok(NULL, "-");

		if (param)
			option = os_str_tol(param, 0, 16);
	}
	ShowMemAllocInfo(show, option);
	return TRUE;
err:
	ShowMemAllocInfo(MAX_TYPE, 0);
	return TRUE;
}

INT Show_PktInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT show = 0;
	UINT64 option = 0;
	CHAR *param;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	param = rstrtok(arg, "-");

	if (param) {
		show = os_str_tol(param, 0, 10);
		param = rstrtok(NULL, "-");

		if (param)
			option = os_str_tol(param, 0, 16);
	}
	ShowPktAllocInfo(show, option);
	return TRUE;
err:
	ShowPktAllocInfo(MAX_TYPE, 0);
	return TRUE;
}
#endif /* MEM_ALLOC_INFO_SUPPORT */

INT	Show_SSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UCHAR	ssid_str[33];
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32       IfIdx = pObj->ioctl_if;
	INT ret;

	NdisZeroMemory(&ssid_str[0], 33);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (VALID_MBSS(pAd, IfIdx))
			NdisMoveMemory(&ssid_str[0],
						pAd->ApCfg.MBSSID[IfIdx].Ssid,
						pAd->ApCfg.MBSSID[IfIdx].SsidLen);
		else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
			NdisMoveMemory(&ssid_str[0],
						pAd->StaCfg[IfIdx].Ssid,
						pAd->StaCfg[IfIdx].SsidLen);
		else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	ret = snprintf(pBuf, BufLen, "\t%s", ssid_str);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

static VOID GetWirelessMode(USHORT PhyMode, UCHAR *pBuf, UCHAR BufLen)
{
	INT ret;

	switch (PhyMode) {
	case (WMODE_B | WMODE_G):
		ret = snprintf(pBuf, BufLen, "\t11B/G");
		break;

	case (WMODE_B):
		ret = snprintf(pBuf, BufLen, "\t11B");
		break;

	case (WMODE_A):
		ret = snprintf(pBuf, BufLen, "\t11A");
		break;

	case (WMODE_A | WMODE_B | WMODE_G):
		ret = snprintf(pBuf, BufLen, "\t11A/B/G");
		break;

	case (WMODE_G):
		ret = snprintf(pBuf, BufLen, "\t11G");
		break;
#ifdef DOT11_N_SUPPORT

	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		ret = snprintf(pBuf, BufLen, "\t11A/B/G/N");
		break;

	case (WMODE_GN):
		ret = snprintf(pBuf, BufLen, "\t11N only with 2.4G");
		break;

	case (WMODE_G | WMODE_GN):
		ret = snprintf(pBuf, BufLen, "\t11G/N");
		break;

	case (WMODE_A | WMODE_AN):
		ret = snprintf(pBuf, BufLen, "\t11A/N");
		break;

	case (WMODE_B | WMODE_G | WMODE_GN):
		ret = snprintf(pBuf, BufLen, "\t11B/G/N");
		break;

	case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
		ret = snprintf(pBuf, BufLen, "\t11A/G/N");
		break;

	case (WMODE_AN):
		ret = snprintf(pBuf, BufLen, "\t11N only with 5G");
		break;
#endif /* DOT11_N_SUPPORT */

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknow Value(%d)", PhyMode);
		break;
	}

	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
}

INT	Show_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	GetWirelessMode(wdev->PhyMode, pBuf, BufLen);
	return 0;
}

INT	Show_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bEnableTxBurst ? "TRUE" : "FALSE");
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	switch (pAd->CommonCfg.TxPreamble) {
	case Rt802_11PreambleShort:
		ret = snprintf(pBuf, BufLen, "\tShort");
		break;

	case Rt802_11PreambleLong:
		ret = snprintf(pBuf, BufLen, "\tLong");
		break;

	case Rt802_11PreambleAuto:
		ret = snprintf(pBuf, BufLen, "\tAuto");
		break;

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknown Value(%lu)", pAd->CommonCfg.TxPreamble);
		break;
	}

	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%u", pAd->CommonCfg.ucTxPowerPercentage);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT ret = 0;

	if (WMODE_CAP_6G(wdev->PhyMode))
		ret = snprintf(pBuf, BufLen, "\t6G Band: %d\n", wdev->channel);
	else if (WMODE_CAP_2G(wdev->PhyMode))
		ret = snprintf(pBuf, BufLen, "\t2.4G Band: %d\n", wdev->channel);
	else if (WMODE_CAP_5G(wdev->PhyMode))
		ret = snprintf(pBuf, BufLen, "\t5G Band: %d\n", wdev->channel);

	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	switch (pAd->CommonCfg.UseBGProtection) {
	case 1: /*Always On*/
		ret = snprintf(pBuf, BufLen, "\tON");
		break;

	case 2: /*Always OFF*/
		ret = snprintf(pBuf, BufLen, "\tOFF");
		break;

	case 0: /*AUTO*/
		ret = snprintf(pBuf, BufLen, "\tAuto");
		break;

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknow Value(%lu)", pAd->CommonCfg.UseBGProtection);
		break;
	}
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 oper_len_thld;
	UINT32 conf_len_thld;
	INT ret;

	if (!wdev)
		return 0;

	conf_len_thld = wlan_config_get_rts_len_thld(wdev);
	oper_len_thld = wlan_operate_get_rts_len_thld(wdev);
	ret = snprintf(pBuf, BufLen, "\tRTSThreshold:: conf=%d, oper=%d", conf_len_thld, oper_len_thld);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UINT32 conf_frag_thld;
	UINT32 oper_frag_thld;
	POS_COOKIE pobj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pobj->ioctl_if, pobj->ioctl_if_type);
	INT ret;

	if (!wdev)
		return 0;

	conf_frag_thld = wlan_config_get_frag_thld(wdev);
	oper_frag_thld = wlan_operate_get_frag_thld(wdev);
	ret = snprintf(pBuf, BufLen, "\tFrag thld:: conf=%u, oper=%u", conf_frag_thld, oper_frag_thld);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

#ifdef DOT11_N_SUPPORT
INT	Show_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT ret;

	if (wlan_config_get_ht_bw(wdev) == BW_40)
		ret = snprintf(pBuf, BufLen, "\t40 MHz");
	else
		ret = snprintf(pBuf, BufLen, "\t20 MHz");

	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32       IfIdx = pObj->ioctl_if;
	INT ret;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (VALID_MBSS(pAd, IfIdx))
			wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		else {
			MTWF_PRINT(" invalid IfIdx=%d.\n", IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum)
			wdev = &pAd->StaCfg[IfIdx].wdev;
		else {
			MTWF_PRINT(" invalid IfIdx=%d.\n", IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (wdev) {
		ret = snprintf(pBuf, BufLen, "\t%u", wdev->DesiredTransmitSetting.field.MCS);
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
	}

	return 0;
}

INT	Show_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ht_gi;
	UCHAR *msg[3] = {"GI_800", "GI_400", "GI_Unknown"};
	INT ret;

	if (!wdev)
		return 0;

	ht_gi = wlan_config_get_ht_gi(wdev);

	if (ht_gi > GI_400)
		ht_gi = 2; /*Unknown GI*/

	ret = snprintf(pBuf, BufLen, "\ti%s", msg[ht_gi]);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	switch (pAd->CommonCfg.RegTransmitSetting.field.HTMODE) {
	case HTMODE_GF:
		ret = snprintf(pBuf, BufLen, "\tGF");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case HTMODE_MM:
		ret = snprintf(pBuf, BufLen, "\tMM");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	default:
		snprintf(pBuf, BufLen, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
		break;
	}

	return 0;
}

INT	Show_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ext_cha;
	INT ret;

	ext_cha = wlan_config_get_ext_cha(wdev);

	switch (ext_cha) {
	case EXTCHA_BELOW:
		ret = snprintf(pBuf, BufLen, "\tBelow");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case EXTCHA_ABOVE:
		ret = snprintf(pBuf, BufLen, "\tAbove");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknow Value(%u)", ext_cha);
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;
	}

	return 0;
}

INT	Show_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR mpdu_density = 0;
	INT ret;

	if (wdev)
		mpdu_density = wlan_config_get_min_mpdu_start_space(wdev);
	ret = snprintf(pBuf, BufLen, "\t%u", mpdu_density);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 ba_tx_wsize = 0, ba_rx_wsize = 0;
	INT ret;

	if (!wdev)
		return 0;
	ba_tx_wsize = wlan_config_get_ba_tx_wsize(wdev);
	ba_rx_wsize = wlan_config_get_ba_rx_wsize(wdev);
	ret = snprintf(pBuf, BufLen, "\t%u %u", ba_tx_wsize, ba_rx_wsize);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);

	return 0;
}

INT	Show_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bRdg ? "TRUE" : "FALSE");
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR amsdu_en = 0;
	INT ret;

	if (wdev)
		amsdu_en = wlan_config_get_amsdu_en(wdev);
	ret = snprintf(pBuf, BufLen, "\t%s", (amsdu_en) ? "TRUE" : "FALSE");
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ba_en = 1;
	INT ret;

	if (wdev)
		ba_en = wlan_config_get_ba_enable(wdev);

	ret = snprintf(pBuf, BufLen, "\t%s", (ba_en) ? "TRUE" : "FALSE");
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}
#endif /* DOT11_N_SUPPORT */

INT	Show_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegion);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegionForABand);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.CountryCode);
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

INT	Show_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32       IfIdx = pObj->ioctl_if;
	INT ret;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (VALID_MBSS(pAd, IfIdx)) {
			ret = snprintf(pBuf, BufLen, "\t%s", pAd->ApCfg.MBSSID[IfIdx].wdev.bWmmCapable ? "TRUE" : "FALSE");
			if (os_snprintf_error(BufLen, ret))
				MTWF_PRINT("%s: snprintf error!\n", __func__);
		} else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (IfIdx >= 0 && IfIdx < pAd->MSTANum) {
			ret = snprintf(pBuf, BufLen, "\t%s", pAd->StaCfg[IfIdx].wdev.bWmmCapable ? "TRUE" : "FALSE");
			if (os_snprintf_error(BufLen, ret))
				MTWF_PRINT("%s: snprintf error!\n", __func__);
		} else {
			MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
			return 0;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	return 0;
}

INT	Show_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	INT ret;

	ret = snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bIEEE80211H ? "TRUE" : "FALSE");
	if (os_snprintf_error(BufLen, ret))
		MTWF_PRINT("%s: snprintf error!\n", __func__);
	return 0;
}

#ifdef CONFIG_STA_SUPPORT
INT	Show_NetworkType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	INT ret;

	if (IfIdx < pAd->MSTANum)
		pStaCfg = &pAd->StaCfg[IfIdx];
	else {
		MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
		return 0;
	}

	switch (pStaCfg->BssType) {
	case BSS_ADHOC:
		ret = snprintf(pBuf, BufLen, "\tAdhoc");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case BSS_INFRA:
		ret = snprintf(pBuf, BufLen, "\tInfra");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case BSS_ANY:
		ret = snprintf(pBuf, BufLen, "\tAny");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case BSS_MONITOR:
		ret = snprintf(pBuf, BufLen, "\tMonitor");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknow Value(%d)", pStaCfg->BssType);
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;
	}

	return 0;
}

#ifdef WSC_STA_SUPPORT
INT	Show_WpsPbcBand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	INT ret;

	PSTA_ADMIN_CONFIG pStaCfg;
	if (IfIdx < pAd->MSTANum)
		pStaCfg = &pAd->StaCfg[IfIdx];
	else {
		MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
		return 0;
	}

	switch (pStaCfg->wdev.WscControl.WpsApBand) {
	case PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST:
		ret = snprintf(pBuf, BufLen, "\t2.4G");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST:
		ret = snprintf(pBuf, BufLen, "\t5G");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	case PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION:
		ret = snprintf(pBuf, BufLen, "\tAuto");
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;

	default:
		ret = snprintf(pBuf, BufLen, "\tUnknow Value(%d)", pStaCfg->wdev.WscControl.WpsApBand);
		if (os_snprintf_error(BufLen, ret))
			MTWF_PRINT("%s: snprintf error!\n", __func__);
		break;
	}

	return 0;
}
#endif /* WSC_STA_SUPPORT */

INT	Show_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	int ret, left_buf_size;

	PSTA_ADMIN_CONFIG pStaCfg;
	if (IfIdx < pAd->MSTANum)
		pStaCfg = &pAd->StaCfg[IfIdx];
	else {
		MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
		return 0;
	}

	if ((pStaCfg->WpaPassPhraseLen >= 8) &&
		(pStaCfg->WpaPassPhraseLen < 64)) {
		ret = snprintf(pBuf, BufLen, "\tWPAPSK = %s", pStaCfg->WpaPassPhrase);
		if (os_snprintf_error(BufLen, ret)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			return 0;
		}
	} else {
		INT idx;

		ret = snprintf(pBuf, BufLen, "\tWPAPSK = ");
		if (os_snprintf_error(BufLen, ret)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			return 0;
		}

		for (idx = 0; idx < 32; idx++) {
			left_buf_size = BufLen - strlen(pBuf);
			ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "%02X", pStaCfg->WpaPassPhrase[idx]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				return 0;
			}
		}
	}

	return 0;
}

INT	Show_AutoReconnect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg;
	UINT32       IfIdx = pObj->ioctl_if;
	int ret;

	if (IfIdx < pAd->MSTANum) {
		pStaCfg = &pAd->StaCfg[IfIdx];
		ret = snprintf(pBuf, BufLen, "\tAutoReconnect = %d", pStaCfg->bAutoReconnect);
		if (os_snprintf_error(BufLen, ret)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			return 1;
		}
	}
	else {
		MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
	}

	return 0;
}

#endif /* CONFIG_STA_SUPPORT */

INT	Show_STA_RAInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	int ret, left_buf_size;

	ret = snprintf(pBuf, BufLen, "\n");
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
#ifdef TXBF_SUPPORT
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "CommonCfg.ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
	left_buf_size = BufLen - strlen(pBuf);
	ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		return 1;
	}
#endif /* TXBF_SUPPORT // */
	return 0;
}

static UINT32 Get_All_Connected_Sta_Rate(RTMP_ADAPTER *pAd, UINT32 ent_type)
{
	INT i = 0;
	struct wifi_dev *pwdev = NULL;
	UCHAR band_idx = 0;
	UINT32 connected_sta_index = 0;
	INT loop_cnt = 0;
	UINT32 loop = 0;
	struct physical_device *ph_dev = (struct physical_device *)pAd->physical_dev;

	if (!ph_dev)
		return connected_sta_index;

	if (!(ph_dev->RxRateResultPair))
		return connected_sta_index;

	/* Get connected station rx rate info, use multi sta get by group*/
	os_zero_mem(ph_dev->RxRateResultPair,
		(sizeof(struct _PHY_STATE_RX_RATE_PAIR) * WTBL_MAX_NUM(pAd)));

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);
#ifdef SW_CONNECT_SUPPORT
		STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, i);

		if (IS_SW_STA(tr_entry))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		if ((pEntry->EntryType != ent_type) || (pEntry->EntryType != ENTRY_CLIENT) || (pEntry->Sst != SST_ASSOC))
			continue;

		pwdev = pEntry->wdev;
		if (pwdev)
			band_idx = HcGetBandByWdev(pwdev);
		ph_dev->RxRateResultPair[connected_sta_index].ucPhyStateInfoCatg = UNI_CMD_PER_STA_CONTENTION_RX_PHYRATE;
		ph_dev->RxRateResultPair[connected_sta_index].ucBandIdx = band_idx;
		ph_dev->RxRateResultPair[connected_sta_index].u2Wcid = pEntry->wcid;
		connected_sta_index++;
		loop_cnt++;
		/* get MAX_STA_LIST_NUM sta rxrate by group(once time) */
		if (loop_cnt == MAX_STA_LIST_NUM) {
			MtCmdPhyGetMutliRxRate(pAd, &(ph_dev->RxRateResultPair[loop * MAX_STA_LIST_NUM]), MAX_STA_LIST_NUM);
			loop++;
			loop_cnt = 0;
		}
	}

	/* the rest sta rx rate */
	if (loop_cnt != 0)
		MtCmdPhyGetMutliRxRate(pAd, &(ph_dev->RxRateResultPair[loop * MAX_STA_LIST_NUM]), loop_cnt);

	return connected_sta_index;
}

static BOOLEAN Get_Connected_Sta_RxRate(RTMP_ADAPTER *pAd, UINT32 u4MaxConnectedNum, UINT16 u2Wcid, EXT_EVENT_PHY_STATE_RX_RATE *prRxRateInfo)
{
	INT i = 0;
	BOOLEAN find = FALSE;
	struct physical_device *ph_dev = (struct physical_device *)pAd->physical_dev;

	if (!ph_dev)
		return find;

	if (!(ph_dev->RxRateResultPair))
		return find;

	for (i = 0; i < u4MaxConnectedNum; i++) {
		if (u2Wcid == ph_dev->RxRateResultPair[i].u2Wcid) {
			os_move_mem(prRxRateInfo, &(ph_dev->RxRateResultPair[i].rRxStatResult), sizeof(EXT_EVENT_PHY_STATE_RX_RATE));
			find = TRUE;
		}
	}
	return find;
}

#ifdef RT_CFG80211_SUPPORT
INT  get_mac_entry_trx_statistics(PMAC_TABLE_ENTRY pEntry, UCHAR *tx_phymode,
	UCHAR *rx_phymode, UCHAR *tx_bw, UCHAR *rx_bw, UCHAR *tx_nss, UCHAR *rx_nss,
	UCHAR *tx_mcs, UCHAR *rx_mcs, ULONG *tx_rate, ULONG *rx_rate, UCHAR *tx_sgi,
	UCHAR *rx_sgi, UCHAR *snr)
{
	UCHAR phy_mode, rate, bw, sgi, stbc;
	UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
	UCHAR nss;
	UCHAR nss_r = 0;
	UINT32 RawData;
	UINT32 lastTxRate;
	UINT32 lastRxRate = pEntry->LastRxRate;
	UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);
	ULONG DataRate, DataRate_r;
	struct _RTMP_ADAPTER *pAd;
	UINT32 MaxConnectedNum;
	UCHAR u1Snr[4] = {0};
	INT32 i, snr_len;
	UINT32 snr_sum = 0;

	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
	union _HTTRANSMIT_SETTING LastTxRate;
	union _HTTRANSMIT_SETTING LastRxRate;

	NdisZeroMemory(&rRxStatResult, sizeof(rRxStatResult));
	NdisZeroMemory(&rTxStatResult, sizeof(rTxStatResult));
	pAd = (struct _RTMP_ADAPTER *)pEntry->pAd;

	if (!pAd)
		return -1;

	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
	LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
	LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
	LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

	if (LastTxRate.field.MODE >= MODE_VHT)
		LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
	else if (LastTxRate.field.MODE == MODE_OFDM)
		LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

	lastTxRate = (UINT32)(LastTxRate.word);
	LastRxRate.word = (USHORT)lastRxRate;
	RawData = lastTxRate;
	phy_mode = rTxStatResult.rEntryTxRate.MODE;
	rate = RawData & 0x3F;
	bw = (RawData >> 7) & 0x7; // max bw enum is 7
	sgi = rTxStatResult.rEntryTxRate.ShortGI;
	stbc = ((RawData >> 10) & 0x1);
	nss = rTxStatResult.rEntryTxRate.VhtNss;
	MaxConnectedNum = Get_All_Connected_Sta_Rate(pAd, pEntry->EntryType);

	/* First, get rxrate from result, if not find, then get by wcid */
	if (Get_Connected_Sta_RxRate(pAd, MaxConnectedNum, pEntry->wcid, &rRxStatResult) != TRUE)
		MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, (UINT32 *)&rRxStatResult);

	LastRxRate.field.MODE = rRxStatResult.u1RxMode;
	LastRxRate.field.BW = rRxStatResult.u1BW;
	LastRxRate.field.ldpc = rRxStatResult.u1Coding;
	LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
	LastRxRate.field.STBC = rRxStatResult.u1Stbc;

	if (LastRxRate.field.MODE >= MODE_VHT)
		LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
	else if (LastRxRate.field.MODE == MODE_OFDM)
		LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate & 0xF);
	else
		LastRxRate.field.MCS = rRxStatResult.u1RxRate;

	phy_mode_r = rRxStatResult.u1RxMode;
	rate_r = rRxStatResult.u1RxRate & 0x3F;
	bw_r = rRxStatResult.u1BW;
	sgi_r = rRxStatResult.u1Gi;
	stbc_r = rRxStatResult.u1Stbc;

	*tx_phymode = phy_mode;
	*rx_phymode = phy_mode_r;
	*tx_bw = bw;
	*rx_bw = bw_r;
	*tx_sgi = sgi;
	*rx_sgi = sgi_r;

#ifdef DOT11_VHT_AC
	if (phy_mode >= MODE_VHT)
		rate = rate & 0xF;
#endif /* DOT11_VHT_AC */
	*tx_nss = nss;
	*tx_mcs = rate;

	UniCmdPerStaGetSNR(pAd, pEntry->wcid, u1Snr);
	snr_len = MCS_NSS_CAP(pAd)->max_path[MAX_PATH_RX];
	*snr = 0;
	for (i = 0; i < 4 && i < snr_len; i++)
		snr_sum += u1Snr[i];

	*snr = (UCHAR)(snr_sum / i);

#ifdef DOT11_VHT_AC
	if (phy_mode_r >= MODE_VHT) {
		nss_r = (rRxStatResult.u1RxNsts + 1) / (rRxStatResult.u1Stbc + 1);
		rate_r = rate_r & 0xF;
	}
#endif /* DOT11_VHT_AC */
	if (phy_mode_r == MODE_OFDM) {
		rate_r = rate_r & 0xF;
		if (rate_r == TMI_TX_RATE_OFDM_6M)
			LastRxRate.field.MCS = 0;
		else if (rate_r == TMI_TX_RATE_OFDM_9M)
			LastRxRate.field.MCS = 1;
		else if (rate_r == TMI_TX_RATE_OFDM_12M)
			LastRxRate.field.MCS = 2;
		else if (rate_r == TMI_TX_RATE_OFDM_18M)
			LastRxRate.field.MCS = 3;
		else if (rate_r == TMI_TX_RATE_OFDM_24M)
			LastRxRate.field.MCS = 4;
		else if (rate_r == TMI_TX_RATE_OFDM_36M)
			LastRxRate.field.MCS = 5;
		else if (rate_r == TMI_TX_RATE_OFDM_48M)
			LastRxRate.field.MCS = 6;
		else if (rate_r == TMI_TX_RATE_OFDM_54M)
			LastRxRate.field.MCS = 7;
		else
			LastRxRate.field.MCS = 0;
		rate_r = LastRxRate.field.MCS;
	} else if (phy_mode_r == MODE_CCK) {
		rate_r = rate_r & 0x7;
		if (rate_r == TMI_TX_RATE_CCK_1M_LP)
			LastRxRate.field.MCS = 0;
		else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
			LastRxRate.field.MCS = 1;
		else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
			LastRxRate.field.MCS = 2;
		else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
			LastRxRate.field.MCS = 3;
		else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
			LastRxRate.field.MCS = 1;
		else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
			LastRxRate.field.MCS = 2;
		else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
			LastRxRate.field.MCS = 3;
		else
			LastRxRate.field.MCS = 0;
		rate_r = LastRxRate.field.MCS;
	}
	*rx_nss = nss_r;
	*rx_mcs = rate_r;

	if (phy_mode >= MODE_HE) {
#ifdef DOT11_EHT_BE
		if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
			get_rate_eht((rate & 0xf), bw, nss, 0, &DataRate);
		else
#endif
			get_rate_he((rate & 0xf), bw, nss, 0, &DataRate);
		if (sgi == 1)
			DataRate = (DataRate * 967) >> 10;
		if (sgi == 2)
			DataRate = (DataRate * 870) >> 10;
	} else {
		getRate(LastTxRate, &DataRate);
	}

	if (phy_mode_r >= MODE_HE) {
#ifdef DOT11_EHT_BE
		if ((phy_mode_r == MODE_EHT) || (phy_mode_r == MODE_EHT_ER_SU) || (phy_mode_r == MODE_EHT_TB) || (phy_mode_r == MODE_EHT_MU))
			get_rate_eht((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
		else
#endif
			get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
		if (sgi_r == 1)
			DataRate_r = (DataRate_r * 967) >> 10;
		if (sgi_r == 2)
			DataRate_r = (DataRate_r * 870) >> 10;
	} else {
		getRate(LastRxRate, &DataRate_r);
	}
	*tx_rate = DataRate;
	*rx_rate = DataRate_r;

	return 0;
}

INT get_station_info_from_mac_entry(struct wifi_dev *wdev, UINT32 ent_type, const UCHAR *mac_addr,
	struct station_information *sta)
{
	int i;
	/* Get connected station rx rate info, use multi sta get by group*/
	PMAC_TABLE_ENTRY pEntry;
	union _HTTRANSMIT_SETTING PhyMode;
	struct _RTMP_ADAPTER *ad;
	UCHAR tx_sgi, rx_sgi, snr;
	int rssi_len;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap;
#endif
	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	for (i = 1; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		pEntry = entry_get(ad, i);
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		/* dump MacTable entries which match the EntryType and current band*/
		if (pEntry->EntryType == ent_type && wdev == pEntry->wdev && MAC_ADDR_EQUAL(mac_addr, pEntry->Addr))
			break;
	}

	if (!VALID_UCAST_ENTRY_WCID(ad, i))
		return -1;
#ifdef DOT11_EHT_BE
	sta->mlo_enable = pEntry->mlo.mlo_en;
	if (sta->mlo_enable) {
		/*don't need to report non-setup link entry, it will be done by the setup link*/
		if (!pEntry->mlo.is_setup_link_entry)
			return -1;
	}
#endif
	PhyMode = pEntry->MaxHTPhyMode;
	rssi_len = MCS_NSS_CAP(ad)->max_path[MAX_PATH_RX];
	sta->rssi[0] = -127;
	sta->rssi[1] = -127;
	sta->rssi[2] = -127;
	sta->rssi[3] = -127;
	rtmp_get_rssi(ad, pEntry->wcid, sta->rssi, rssi_len);
	UniCmdPerStaGetSNR(ad, pEntry->wcid, sta->snr);
	COPY_MAC_ADDR(sta->mac, pEntry->Addr);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (cap->fgRateAdaptFWOffload == TRUE/*&& (pEntry->bAutoTxRateSwitch == TRUE)*/) {
		get_mac_entry_trx_statistics(pEntry, &(sta->tx_phymode), &(sta->rx_phymode), &(sta->tx_bw),  &(sta->rx_bw),
			&(sta->tx_nss), &(sta->rx_nss), &(sta->tx_mcs), &(sta->rx_mcs), &(sta->tx_rate), &(sta->rx_rate),
			&tx_sgi, &rx_sgi, &snr);
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	sta->cap_phymode = pEntry->MaxHTPhyMode.field.MODE;
	sta->rx_bytes = pEntry->RxBytes;
	sta->tx_bytes = pEntry->TxBytes;
	sta->rx_packets = pEntry->RxPackets.QuadPart;
	sta->tx_packets = pEntry->TxPackets.QuadPart;

#ifdef DOT11_EHT_BE
	if (sta->mlo_enable) {
		struct mld_entry_t *mld;
		PMAC_TABLE_ENTRY pEntry_link;
		int n, j;
		UCHAR tmp_u8;

		COPY_MAC_ADDR(sta->mld_mac, pEntry->mlo.mld_addr);
		mld = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
		if (mld && mld->valid) {
			for (j = 0, n = 0; j < MLD_LINK_MAX && n < 3; j++) {
				pEntry_link = mld->link_entry[j];
				if (pEntry_link) {
					COPY_MAC_ADDR(sta->mlo_link[n].link_address,
						pEntry_link->Addr);
					sta->mlo_link[n].valid = 1;
					sta->mlo_link[n].rx_bytes = pEntry_link->RxBytes;
					sta->mlo_link[n].tx_bytes = pEntry_link->TxBytes;
					sta->mlo_link[n].rx_packets = pEntry_link->RxPackets.QuadPart;
					sta->mlo_link[n].tx_packets = pEntry_link->TxPackets.QuadPart;
					if (pEntry_link->pAd) {
						rssi_len = MCS_NSS_CAP(((struct _RTMP_ADAPTER *)pEntry_link->pAd))->max_path[MAX_PATH_RX];
						rtmp_get_rssi(pEntry_link->pAd, pEntry_link->wcid, sta->mlo_link[n].rssi, rssi_len);
						UniCmdPerStaGetSNR(pEntry_link->pAd, pEntry_link->wcid, sta->mlo_link[n].snr);
						get_mac_entry_trx_statistics(pEntry, &tmp_u8, &tmp_u8, &tmp_u8, &tmp_u8,
								&tmp_u8, &tmp_u8, &tmp_u8, &tmp_u8,
								&(sta->mlo_link[n].tx_rate), &(sta->mlo_link[n].rx_rate), &tx_sgi, &rx_sgi, &snr);
					}
					n++;
				}
			}
		}
	}
#endif
	return 0;
}

#endif

static INT dump_mac_table(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
	INT i, j;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	ULONG max_DataRate = 0;
	INT sta_cnt = 0;
	INT sta_cnt_band[MAX_BAND_NUM] = {0};
	INT apcli_cnt = 0;
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	ADD_HT_INFO_IE *addht;
	CHAR rssi[MAX_RSSI_LEN];
	UINT8 u1Snr[MAX_RSSI_LEN] = {0};
	UINT8 Antenna = 0;
	int ret, left_buf_size;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	struct wifi_dev *pwdev = NULL;
	UCHAR band_idx = 0;
	UINT32 MaxConnectedNum = 0;

	struct entry_type_str_map {
		UINT32 type;
		UCHAR str[6];
	} type_str_map[] = {
		{ENTRY_AP,	"AP"},
		{ENTRY_INFRA,	"AP"},
		{ENTRY_GC,	"GC"},
		{ENTRY_ADHOC,	"ADHOC"},
		{ENTRY_APCLI,	"APCLI"},
		{ENTRY_DLS,	"DLS"},
		{ENTRY_CLIENT,	"STA"},
		{ENTRY_REPEATER, "REPT"},
		{0,		""}
	};

	for (i = 0; i < ARRAY_SIZE(rssi); i++)
		rssi[i] = -127;

	MTWF_PRINT("\n");
#ifdef CONFIG_HOTSPOT_R2
	MTWF_PRINT(
			"\n%-18s%-6s%-5s%-7s%-9s%-4s%-4s%-7s%-20s%-20s%-14s%-10s%-15s%-13s%-10s%-7s%-12s%-7s\n",
		   "MAC", "MODE", "AID", "WCID", "BSS/BN", "PSM",
		   "WMM", "MIMOPS", "RSSI0/1/2/3", "SNR0/1/2/3", "PhMd", "BW", "MCS", "SGI",
		   "STBC",      "Idle", "Rate",     "QosMap");
#else
	MTWF_PRINT(
			"\n%-18s%-6s%-5s%-7s%-9s%-4s%-4s%-7s%-20s%-20s%-12s%-9s%-14s%-13s%-10s%-7s%-10s\n",
		   "MAC", "MODE", "AID", "WCID", "BSS/BN", "PSM",
		   "WMM", "MIMOPS", "RSSI0/1/2/3", "SNR0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
		   "STBC(T/R)", "Idle", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef MWDS
	MTWF_PRINT("%-8s\n", "MWDSCap");
#endif /* MWDS */


	/* Get connected station rx rate info, use multi sta get by group*/
	MaxConnectedNum = Get_All_Connected_Sta_Rate(pAd, ent_type);

	/* Wcid 0 is not used for user, don't dump it */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);
		union _HTTRANSMIT_SETTING PhyMode = pEntry->MaxHTPhyMode;
		struct _RTMP_ADAPTER *ad = NULL;
		int rssi_len;

		if ((ent_type == ENTRY_NONE)) {
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE)
				continue;
		} else {
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
				&& (pEntry->Sst != SST_ASSOC))
				continue;

#ifdef MAC_REPEATER_SUPPORT

			if (bReptCli == FALSE) {
				/* only dump the apcli entry which not a RepeaterCli */
				if (IS_REPT_LINK_UP(pEntry->pReptCli))
					continue;
			}

#endif /* MAC_REPEATER_SUPPORT */
		}

		if (IS_ENTRY_CLIENT(pEntry)) {
			pwdev = pEntry->wdev;
			if (pwdev)
				band_idx = HcGetBandByWdev(pwdev);
			sta_cnt++;
			sta_cnt_band[band_idx]++;
		}

		if (IS_ENTRY_PEER_AP(pEntry))
			apcli_cnt++;

		DataRate = 0;
		getRate(pEntry->HTPhyMode, &DataRate);
		MTWF_PRINT(MACSTR, MAC2STR(pEntry->Addr));
		MTWF_PRINT(" ");

		for (j = 0; type_str_map[j].type != 0; j++) {
			if (type_str_map[j].type == pEntry->EntryType) {
				MTWF_PRINT("%-6s", type_str_map[j].str);
				break;
			}
		}
		MTWF_PRINT("%-5d", (int)pEntry->Aid);
		MTWF_PRINT("%-7d", (int)pEntry->wcid);
		MTWF_PRINT("%4d/%-4d", (int)pEntry->func_tb_idx, band_idx);
		MTWF_PRINT("%-4d", (int)pEntry->PsMode);
		MTWF_PRINT("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
		MTWF_PRINT("%-7d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
		ad = pEntry->wdev->sys_handle;
		rssi_len = MCS_NSS_CAP(ad)->max_path[MAX_PATH_RX];
		rtmp_get_rssi(pAd, pEntry->wcid, rssi, rssi_len);
		ret = 0;
		for (j = 0; j < 4; j++) {
			if (j < rssi_len)
				ret += snprintf(tmp_str + ret, temp_str_len, "%d", rssi[j]);
			else
				ret += snprintf(tmp_str + ret, temp_str_len, "--");
			if (j < 4 - 1)
				ret += snprintf(tmp_str + ret, temp_str_len, "/");
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
			}
		}
		MTWF_PRINT("%-20s", tmp_str);

		UniCmdPerStaGetSNR(pAd, pEntry->wcid, u1Snr);
		ret = 0;
		for (j = 0; j < 4; j++) {
			if (j < rssi_len)
				ret += snprintf(tmp_str + ret, temp_str_len, "%d", u1Snr[j]);
			else
				ret += snprintf(tmp_str + ret, temp_str_len, "--");
			if (j < 4 - 1)
				ret += snprintf(tmp_str + ret, temp_str_len, "/");
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
			}
		}
		MTWF_PRINT("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

		if (cap->fgRateAdaptFWOffload == TRUE/*&& (pEntry->bAutoTxRateSwitch == TRUE)*/) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
			UCHAR nss;
			UCHAR nss_r = 0;
			UINT32 RawData;
			UINT32 lastTxRate;
			UINT32 lastRxRate = pEntry->LastRxRate;
			UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);

			EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
			EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
			union _HTTRANSMIT_SETTING LastTxRate;
			union _HTTRANSMIT_SETTING LastRxRate;

			NdisZeroMemory(&rRxStatResult, sizeof(rRxStatResult));
			NdisZeroMemory(&rTxStatResult, sizeof(rTxStatResult));

			MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
			LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
			LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
			LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
			LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
			LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

			if (LastTxRate.field.MODE >= MODE_VHT)
				LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
			else if (LastTxRate.field.MODE == MODE_OFDM)
				LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
			else
				LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

			lastTxRate = (UINT32)(LastTxRate.word);
			LastRxRate.word = (USHORT)lastRxRate;
			RawData = lastTxRate;
			phy_mode = rTxStatResult.rEntryTxRate.MODE;
			rate = RawData & 0x3F;
			bw = (RawData >> 7) & 0x7; // max bw enum is 7
			sgi = rTxStatResult.rEntryTxRate.ShortGI;
			stbc = ((RawData >> 10) & 0x1);
			nss = rTxStatResult.rEntryTxRate.VhtNss;

			/* First, get rxrate from result, if not find, then get by wcid */
			if (Get_Connected_Sta_RxRate(pAd, MaxConnectedNum, pEntry->wcid, &rRxStatResult) != TRUE)
				MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, (UINT32 *)&rRxStatResult);

			LastRxRate.field.MODE = rRxStatResult.u1RxMode;
			LastRxRate.field.BW = rRxStatResult.u1BW;
			LastRxRate.field.ldpc = rRxStatResult.u1Coding;
			LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
			LastRxRate.field.STBC = rRxStatResult.u1Stbc;

			if (LastRxRate.field.MODE >= MODE_VHT)
				LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
			else if (LastRxRate.field.MODE == MODE_OFDM)
				LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate & 0xF);
			else
				LastRxRate.field.MCS = rRxStatResult.u1RxRate;

			phy_mode_r = rRxStatResult.u1RxMode;
			rate_r = rRxStatResult.u1RxRate & 0x3F;
			bw_r = rRxStatResult.u1BW;
			sgi_r = rRxStatResult.u1Gi;
			stbc_r = rRxStatResult.u1Stbc;

			ret = snprintf(tmp_str, temp_str_len, "%s/%s", get_phymode_str(phy_mode), get_phymode_str(phy_mode_r));
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
			MTWF_PRINT("%-14s", tmp_str);
			ret = snprintf(tmp_str, temp_str_len, "%s/%s", get_bw_str_stainfo(bw, BW_FROM_TXRX_INFO), get_bw_str_stainfo(bw_r, BW_FROM_TXRX_INFO));
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
			MTWF_PRINT("%-10s", tmp_str);
#ifdef DOT11_VHT_AC

			if (phy_mode >= MODE_VHT) {
				rate = rate & 0xF;
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d/", nss, rate);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			} else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d/", rate);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			}

#ifdef DOT11_VHT_AC

			if (phy_mode_r >= MODE_VHT) {
				nss_r = (rRxStatResult.u1RxNsts + 1) / (rRxStatResult.u1Stbc + 1);
				rate_r = rate_r & 0xF;
				left_buf_size = temp_str_len - strlen(tmp_str);
				ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%dS-M%d", nss_r, rate_r);
				if (os_snprintf_error(left_buf_size, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if (phy_mode_r >= MODE_HTMIX) {
					left_buf_size = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d", rate_r);
					if (os_snprintf_error(left_buf_size, ret)) {
						MTWF_PRINT("%s: final_name snprintf error!\n",
							__func__);
					}
				} else
#endif
					if (phy_mode_r == MODE_OFDM) {
						rate_r = rate_r & 0xF;
						if (rate_r == TMI_TX_RATE_OFDM_6M)
							LastRxRate.field.MCS = 0;
						else if (rate_r == TMI_TX_RATE_OFDM_9M)
							LastRxRate.field.MCS = 1;
						else if (rate_r == TMI_TX_RATE_OFDM_12M)
							LastRxRate.field.MCS = 2;
						else if (rate_r == TMI_TX_RATE_OFDM_18M)
							LastRxRate.field.MCS = 3;
						else if (rate_r == TMI_TX_RATE_OFDM_24M)
							LastRxRate.field.MCS = 4;
						else if (rate_r == TMI_TX_RATE_OFDM_36M)
							LastRxRate.field.MCS = 5;
						else if (rate_r == TMI_TX_RATE_OFDM_48M)
							LastRxRate.field.MCS = 6;
						else if (rate_r == TMI_TX_RATE_OFDM_54M)
							LastRxRate.field.MCS = 7;
						else
							LastRxRate.field.MCS = 0;

				left_buf_size = temp_str_len - strlen(tmp_str);
				ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d", LastRxRate.field.MCS);
				if (os_snprintf_error(left_buf_size, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}

			} else if (phy_mode_r == MODE_CCK) {
				rate_r = rate_r & 0x7;
				if (rate_r == TMI_TX_RATE_CCK_1M_LP)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
					LastRxRate.field.MCS = 3;
				else
					LastRxRate.field.MCS = 0;

				left_buf_size = temp_str_len - strlen(tmp_str);
				ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d", LastRxRate.field.MCS);
				if (os_snprintf_error(left_buf_size, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			}

			MTWF_PRINT("%-15s", tmp_str);
			ret = snprintf(tmp_str, temp_str_len, "%s/%s",
				 get_gi_str(phy_mode, sgi),
				 get_gi_str(phy_mode_r, sgi_r));
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
			MTWF_PRINT("%-13s", tmp_str);
			ret = snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
			MTWF_PRINT("%-10s", tmp_str);

			if (phy_mode >= MODE_HE) {
#ifdef DOT11_EHT_BE
				if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
					get_rate_eht((rate & 0xf), bw, nss, 0, &DataRate);
				else
#endif
					get_rate_he((rate & 0xf), bw, nss, 0, &DataRate);
				if (sgi == 1)
					DataRate = (DataRate * 967) >> 10;
				if (sgi == 2)
					DataRate = (DataRate * 870) >> 10;
			} else {
				getRate(LastTxRate, &DataRate);
			}

			if (phy_mode_r >= MODE_HE) {
#ifdef DOT11_EHT_BE
				if ((phy_mode_r == MODE_EHT) || (phy_mode_r == MODE_EHT_ER_SU) || (phy_mode_r == MODE_EHT_TB) || (phy_mode_r == MODE_EHT_MU))
					get_rate_eht((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
				else
#endif
					get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
				if (sgi_r == 1)
					DataRate_r = (DataRate_r * 967) >> 10;
				if (sgi_r == 2)
					DataRate_r = (DataRate_r * 870) >> 10;
			} else {
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_PRINT("%-12s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-9s", get_bw_str(pEntry->HTPhyMode.field.BW, BW_FROM_OID));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1),
						 (pEntry->HTPhyMode.field.MCS & 0xf));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			}
			else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d", pEntry->HTPhyMode.field.MCS);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				}
			}

			MTWF_PRINT("%-14s", tmp_str);
			MTWF_PRINT("%-13d", pEntry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-10d", pEntry->HTPhyMode.field.STBC);
		}

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
		/* If TWT agreement is present for this STA, add maximum TWT wake up interval in sta idle timeout. */
		if (GET_PEER_ITWT_FID_BITMAP(pEntry)) {
			MTWF_PRINT("%-7d", (int)((pEntry->StaIdleTimeout + pEntry->twt_ctrl.twt_interval_max) - pEntry->NoDataIdleCount));
		} else
#endif
		{
			MTWF_PRINT("%-7d", (pEntry->StaIdleTimeout >= pEntry->NoDataIdleCount)?(int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount):0);
		}

		ret = snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
		}
		MTWF_PRINT("%-12s", tmp_str);
#ifdef CONFIG_HOTSPOT_R2
		MTWF_PRINT("%-7d", (int)pEntry->QosMapSupport);
#endif
		MTWF_PRINT("%d,%d,%d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
			   (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry->QosMapSupport) {
			int k = 0;

			MTWF_PRINT("DSCP Exception:\n");

			for (k = 0; k < pEntry->DscpExceptionCount / 2; k++)
				MTWF_PRINT("[Value: %4d] [UP: %4d]\n", pEntry->DscpException[k] & 0xff, (pEntry->DscpException[k] >> 8) & 0xff);

			MTWF_PRINT("DSCP Range:\n");

			for (k = 0; k < 8; k++)
				MTWF_PRINT("[UP :%3d][Low Value: %4d] [High Value: %4d]\n", k, pEntry->DscpRange[k] & 0xff,
					   (pEntry->DscpRange[k] >> 8) & 0xff);
		}

#endif
#ifdef MWDS

		if (IS_ENTRY_PEER_AP(pEntry)) {
			if (pEntry->func_tb_idx < MAX_APCLI_NUM) {
				if (pAd->StaCfg[pEntry->func_tb_idx].MlmeAux.bSupportMWDS)
					MTWF_PRINT("%-8s", "YES");
				else
					MTWF_PRINT("%-8s", "NO");
			}
		} else {
			if (pEntry->bSupportMWDS)
				MTWF_PRINT("%-8s", "YES");
			else
				MTWF_PRINT("%-8s", "NO");
		}

#endif /* MWDS */
		/* +++Add by shiang for debug */
		MTWF_PRINT("%26s%49s%-12s", "MaxCap: ", " ", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
		MTWF_PRINT("%-9s", get_bw_str(pEntry->MaxHTPhyMode.field.BW, BW_FROM_OID));
		if (pEntry->he_ie_flag && (pEntry->ht_ie_flag || pEntry->vht_ie_flag)) {
			MTWF_PRINT("\n%34s%49s%-12s", "sta-max-cap : ", " ", "HE");
			MTWF_PRINT("%-9s\n", get_bw_str(pEntry->sta_he_ch_bw, BW_FROM_OID));
		} else if (pEntry->ht_ie_flag && !pEntry->vht_ie_flag && !pEntry->he_ie_flag) {
			MTWF_PRINT("\n%34s%49s%-12s", "sta-max-cap : ", " ", "HT_MM");
			MTWF_PRINT("%-9s\n", get_bw_str(pEntry->cap.ch_bw.ht_support_ch_width_set, BW_FROM_OID));
		} else if (pEntry->ht_ie_flag && pEntry->vht_ie_flag && !pEntry->he_ie_flag) {
			MTWF_PRINT("\n%34s%49s%-12s", "sta-max-cap : ", " ", "VHT");
			MTWF_PRINT("%-9s\n", get_bw_str(pEntry->sta_vht_ch_bw, BW_FROM_OID));
		} else {
			MTWF_PRINT("\n%34s%49s%-12s\n", "sta-max-cap : ", " ", "not HE/VHT/HT");
		}
#ifdef DOT11_VHT_AC

		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_VHT) {
			ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((pEntry->MaxHTPhyMode.field.MCS >> 4) + 1),
					 (pEntry->MaxHTPhyMode.field.MCS & 0xf));
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
		}
		else
#endif /* DOT11_VHT_AC */
		{
			ret = snprintf(tmp_str, temp_str_len, "%d", pEntry->MaxHTPhyMode.field.MCS);
			if (os_snprintf_error(temp_str_len, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			}
		}

		MTWF_PRINT("%-14s", tmp_str);
		MTWF_PRINT("%-13d", pEntry->MaxHTPhyMode.field.ShortGI);
		MTWF_PRINT("%-10d", pEntry->MaxHTPhyMode.field.STBC);
#ifdef DOT11_VHT_AC
		if ((pEntry->fgGband256QAMSupport) && ((pEntry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
		                                       (pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD))) {
			if ((PhyMode.field.MCS == MCS_7) || (PhyMode.field.MCS == MCS_15) ||
			    (PhyMode.field.MCS == MCS_23) || (PhyMode.field.MCS == MCS_31)) {
				PhyMode.field.MODE = MODE_VHT;
				/*for spec VHT mode bw20M or (bw160M and 3*3) maxMCS is 8*/
				if (((PhyMode.field.BW == 0) && (PhyMode.field.MCS != MCS_23)) ||
				    ((PhyMode.field.BW == 3) && (PhyMode.field.MCS == MCS_23)))
					PhyMode.field.MCS = MCS_8;
				/*bw40M or bw80M or (bw20M and 3*3) maxMCS is 9*/
				else
					PhyMode.field.MCS = MCS_9;
				/*get antenna VHT from HT*/
				Antenna = ((pEntry->MaxHTPhyMode.field.MCS >> 3) & 0x3);
				PhyMode.field.MCS |= (Antenna << 4);
			}
		}
#endif /* DOT11_VHT_AC */
		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HE)
#ifdef DOT11_EHT_BE
			if (pEntry->MaxHTPhyMode.field.MODE == MODE_EHT)
				get_rate_eht((pEntry->MaxHTPhyMode.field.MCS & 0xf), pEntry->MaxHTPhyMode.field.BW,
					((pEntry->MaxHTPhyMode.field.MCS >> 4) & 0x3) + 1, 0, &max_DataRate);
			else
#endif
				get_rate_he((pEntry->MaxHTPhyMode.field.MCS & 0xf), pEntry->MaxHTPhyMode.field.BW,
					((pEntry->MaxHTPhyMode.field.MCS >> 4) & 0x3) + 1, 0, &max_DataRate);
		else
			getRate(PhyMode, &max_DataRate);
		MTWF_PRINT("%-7s", "-");
		MTWF_PRINT("%-10d\n", (int)max_DataRate);
#ifdef DOT11_N_SUPPORT
		addht = wlan_operate_get_addht(pEntry->wdev);
		MTWF_PRINT("%18sHT Operating Mode:%d", " ", addht->AddHtInfo2.OperaionMode);
#endif /* DOT11_N_SUPPORT */
#ifdef HTC_DECRYPT_IOT
		MTWF_PRINT(" HTC_ICVErr:%d", pEntry->HTC_ICVErrCnt);
		MTWF_PRINT(" HTC_AAD_OM_Force:%s", pEntry->HTC_AAD_OM_Force ? "YES" : "NO");
#endif /* HTC_DECRYPT_IOT */
		MTWF_PRINT(" wdev:%d", (int)pEntry->wdev->wdev_idx);
		MTWF_PRINT(" i/btwt=%d,0x%.2x/%d,0x%.8x\n",
			IS_STA_SUPPORT_TWT(pEntry), GET_PEER_ITWT_FID_BITMAP(pEntry),
			IS_STA_SUPPORT_BTWT(pEntry), GET_PEER_BTWT_ID_BITMAP(pEntry));
		/* ---Add by shiang for debug */
		MTWF_PRINT(" Time:%04d:%02d:%02d", pEntry->StaConnectTime/3600, pEntry->StaConnectTime/60%60, pEntry->StaConnectTime%60);
		MTWF_PRINT("\n\n");
#ifdef IGMP_SNOOP_SUPPORT
		MTWF_PRINT("M2uPackets: %-12lld", pEntry->M2U_TxPackets);
		MTWF_PRINT("M2UBytes: %-12lld\n", pEntry->M2U_TxBytes);
#endif
#ifdef MLR_SUPPORT
		if (pEntry->vendor_ie.mtk_mlr_cap || pEntry->MlrMode)
			MTWF_PRINT("MLR CAPs: 0x%x, Mode %d, State %d\n",
			pEntry->vendor_ie.mtk_mlr_cap, pEntry->MlrMode, pEntry->MlrCurState);
#endif
	}

	MTWF_PRINT("sta_cnt=%d (", sta_cnt);
	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		if (band_idx == (MAX_BAND_NUM-1))
			MTWF_PRINT("%d)\n", sta_cnt_band[band_idx]);
		else
			MTWF_PRINT("%d/", sta_cnt_band[band_idx]);
	}

	MTWF_PRINT("apcli_cnt=%d\n", apcli_cnt);
#ifdef HTC_DECRYPT_IOT
	MTWF_PRINT("HTC_ICV_Err_TH=%d\n", pAd->HTC_ICV_Err_TH);
#endif /* HTC_DECRYPT_IOT */
	return TRUE;
}

INT Show_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;
	INT ret = TRUE;
	RTMP_STRING *rate_str = NULL;
	RTMP_STRING *psm_str = NULL;
	RTMP_STRING *bss_str = NULL;
	CHAR *pch = NULL;
	UINT32 check_aid = 0;
	UINT32 check_bssidx = 0;

	MTWF_PRINT("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg));

	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;

		rate_str = strstr(arg, "rate:");
		if (rate_str) {
			pch = strchr(rate_str, ':');
			if (pch) {
				check_aid = (UINT32)os_str_tol(pch + 1, 0, 10);

				MTWF_PRINT("%s check_aid:%d RATE\n", __func__, check_aid);
				if (check_aid)
					ret = entrytb_traversal(pAd, traversal_func_dump_entry_rate_by_aid, (void *)&check_aid);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"rate_str parse fail\n");
			}
			return ret;
		}

		psm_str = strstr(arg, "psm:");
		if (psm_str) {
			pch = strchr(psm_str, ':');
			if (pch) {
				check_aid = (UINT32)os_str_tol(pch + 1, 0, 10);

				MTWF_PRINT("%s check_aid:%d PSM\n", __func__, check_aid);

				if (check_aid)
					ret = entrytb_traversal(pAd, traversal_func_dump_entry_psm_by_aid, (void *)&check_aid);
				return ret;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"psm_str parse fail\n");
				return ret;
			}
		}

		bss_str = strstr(arg, "bss:");
		if (bss_str) {
			entrytb_bss_idx_search_t bss_search;

			os_zero_mem(&bss_search, sizeof(entrytb_bss_idx_search_t));
			pch = strchr(bss_str, ':');

			if (pch)
				check_bssidx = (UINT32)os_str_tol(pch + 1, 0, 10);
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"bss_str parse fail\n");
				return ret;
			}

			bss_search.bss_idx = check_bssidx;
			bss_search.need_print_field_name = 1;

			MTWF_PRINT("%s check_bssidx:%d associated entries\n", __func__, check_bssidx);
			ret = entrytb_traversal(pAd, traversal_func_dump_entry_associated_to_bss, (void *)&bss_search);

			return ret;
		}
	}

	MTWF_PRINT("Dump MacTable entries info, EntType=0x%x\n", ent_type);
	return dump_mac_table(pAd, ent_type, FALSE);
}

INT Show_Mib_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		struct wifi_dev *wdev = NULL;
		POS_COOKIE pObj = NULL;
		UCHAR BandIdx = 0;

		MTWF_PRINT("show mib info statistic:\n");
		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"pObj is NULL\n");
			return FALSE;
		}

		if (arg != NULL && strlen(arg)) {
			pAd->partial_mib_show_en = os_str_tol(arg, 0, 10);
		} else {
			goto err;
		}

		if (pAd->partial_mib_show_en == 1) {
			INT32       apidx = pObj->ioctl_if;
			if (VALID_MBSS(pAd, apidx))
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Invalid apidx: %d!\n", apidx);
				goto err;
			}

			BandIdx = HcGetBandByWdev(wdev);

			MTWF_PRINT("%s RX FCS Error Count         = %d\n", __func__,
				pAd->WlanCounters.RxFcsErrorCount.u.LowPart);
			MTWF_PRINT("%s RX FIFO Overflow Count     = %d\n", __func__,
				pAd->WlanCounters.RxFifoFullCount.u.LowPart);
			MTWF_PRINT("%s RX MPDU Count              = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.RxMpduCount.QuadPart);
			MTWF_PRINT("%s Channel Idle Count         = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.ChannelIdleCount.QuadPart);
			MTWF_PRINT("%s CCA NAV TX Time            = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.CcaNavTxTime.QuadPart);
			MTWF_PRINT("%s RX MDRDY Count             = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.RxMdrdyCount.QuadPart);
			MTWF_PRINT("%s S CCA Time                 = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.SCcaTime.QuadPart);
			MTWF_PRINT("%s P ED Time                  = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.PEdTime.QuadPart);
			MTWF_PRINT("%s RX Total Byte Count        = %ld\n", __func__,
				(ULONG)pAd->WlanCounters.RxTotByteCount.QuadPart);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;

#ifdef CONFIG_AP_SUPPORT
err:
	MTWF_PRINT("invalid input, should be enable of disable show mib\n");
	return TRUE;
#endif /* CONFIG_AP_SUPPORT */
}

INT show_hwifi_dbgcmd_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (hwifi_dbg_info(pAd, arg)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"%s(): process failed\n", __func__);
		return FALSE;
	}

	return TRUE;
}

#ifdef ACL_BLK_COUNT_SUPPORT
INT Show_ACLRejectCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;

		if (arg && strlen(arg)) {
			if (rtstrcasecmp(arg, "1") == TRUE) {
				int count;

				if (pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy == 2) {
					MTWF_PRINT("ACL: Policy=%lu(0:Dis,1:White,2:Black),ACL: Num=%lu\n",
							pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy,
							pAd->ApCfg.MBSSID[apidx].AccessControlList.Num);
					for (count = 0; count < pAd->ApCfg.MBSSID[apidx].AccessControlList.Num; count++) {
						MTWF_PRINT("MAC:"MACSTR" , Reject_Count: %lu\n",
						MAC2STR(pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Addr),
						pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Reject_Count);
					}
				} else {
					MTWF_PRINT(
					"ERR:Now Policy=%lu(0:Disable,1:White List,2:Black List)\n",
					pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy);
				}
			}
		}
		return TRUE;
	}
#endif/*ACL_BLK_COUNT_SUPPORT*/

INT Show_PSTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
		"arg=%s\n", (arg == NULL ? "" : arg));

	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;
	}

	MTWF_PRINT("Dump MacTable entries info, EntType=0x%x\n", ent_type);
	if (chip_dbg->dump_ps_table)
		return chip_dbg->dump_ps_table(pAd->hdev_ctrl, ent_type, FALSE);
	else
		return FALSE;
}

INT Show_BaTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tok;
	ULONG first_index, second_index;

	tok = rstrtok(arg, ":");

	if (tok) {
		first_index = os_str_toul(tok, NULL, 10);

		switch (first_index) {
		case 0:
			while (tok) {
				tok = rstrtok(NULL, ":");
				if (tok) {
					second_index = os_str_toul(tok, NULL, 10);
					ba_resource_dump_all(pAd, second_index);
				} else {
					ba_resource_dump_all(pAd, 0);
				}
			}
			break;
		case 1:
			ba_reordering_resource_dump_all(pAd);
			break;
		case 2:
			while (tok) {
				tok = rstrtok(NULL, ":");
				if (tok) {
					second_index = os_str_toul(tok, NULL, 10);
					ba_reodering_resource_dump(pAd, second_index);
				}
			}
			break;
		case 3:
			ba_resource_raw_dump(pAd);
			break;
		}
	} else {
		ba_resource_dump_all(pAd, 0);
	}

	MTWF_PRINT("Dump BaTable info arg = %s\n", arg);
	return TRUE;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
INT show_client_idle_time(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i = 0;

	if (IS_SUPPORT_V10_DFS(pAd) == FALSE)
		MTWF_PRINT("%s(): Feature Not Supported\n", __func__);
	else
		MTWF_PRINT("%s(): Client Idle Time\n", __func__);

	MTWF_PRINT("==============================================\n");
	MTWF_PRINT("WCID Client MAC Address   Idle Period(seconds)\n");

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			MTWF_PRINT("%d    ", pEntry->wcid);
			MTWF_PRINT(MACSTR, MAC2STR(pEntry->Addr));
			MTWF_PRINT("      %ld\n", pEntry->LastRxTimeCount);
		}
	}
	return TRUE;
}
#endif
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
INT show_current_rssi(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT i = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if ((pObj->ioctl_if_type != INT_APCLI) || (ifIndex >= MAX_APCLI_NUM)) {
		MTWF_PRINT("Invalid Interface Type %d Number %d\n", pObj->ioctl_if_type, ifIndex);
		return FALSE;
	} else
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	if (IS_VENDOR10_RSSI_VALID(wdev) == FALSE) {
		MTWF_PRINT("RSSI Feature Disable\n");
		return FALSE;
	}

	MTWF_PRINT("=====================================\n");
	MTWF_PRINT("WCID      Peer MAC      RSSI Strength\n");

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

			if ((IS_VALID_ENTRY(pEntry)) && (pEntry->wdev->wdev_type == WDEV_TYPE_STA || pEntry->wdev->wdev_type == WDEV_TYPE_REPEATER)
			&& (pEntry->func_tb_idx < MAX_APCLI_NUM)) {
			MTWF_PRINT("%d    ", pEntry->wcid);
			MTWF_PRINT(MACSTR, MAC2STR(pEntry->Addr));

			MTWF_PRINT(" %d", pEntry->CurRssi);

			if (pEntry->CurRssi >= -50)
				MTWF_PRINT("   High\n");
			else if (pEntry->CurRssi >= -80)    /* between -50 ~ -80dbm*/
				MTWF_PRINT("   Medium\n");
			else if (pEntry->CurRssi >= -90)   /* between -80 ~ -90dbm*/
				MTWF_PRINT("   Low\n");
			else    /* < -84 dbm*/
				MTWF_PRINT("   Poor\n");
		}
	}

	MTWF_PRINT("=====================================\n");

	if (i > WTBL_MAX_NUM(pAd)) {
		MTWF_PRINT("No Entry Found\n");
		return FALSE;
	}
	return TRUE;
}
#endif

#ifdef MT_MAC
INT show_wtbl_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, start, end, idx = -1;
	/* WTBL_ENTRY wtbl_entry; */

	asic_dump_wtbl_base_info(pAd);

	if (arg == NULL)
		return TRUE;

	MTWF_PRINT("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg));

	if (strlen(arg)) {
		idx = os_str_toul(arg, NULL, 10);
		start = end = idx;
	} else {
		start = 0;
		end = pAd->mac_ctrl.wtbl_entry_cnt[0] - 1;
	}

	MTWF_PRINT("Dump WTBL entries info, start=%d, end=%d, idx=%d\n",
			 start, end, idx);

	for (i = start; i <= end; i++) {
		asic_dump_wtbl_info(pAd, i);
	}

	return TRUE;
}

INT show_wtbl_mlo_omac_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);
	INT idx = -1;

	if (arg == NULL)
		return TRUE;

	MTWF_PRINT("%s(): arg=%s\n", __func__, (arg == NULL ? "" : arg));

	if (strlen(arg))
		idx = os_str_toul(arg, NULL, 10);

	MTWF_PRINT("Dump WTBL entries info idx=%d\n", idx);

	chip_dbg->dump_wtbl_mlo_omac(pAd, idx);

	return TRUE;
}

INT show_wtbltlv_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UINT16 u2Wcid = 0;
	UCHAR ucCmdId = 0;
	UCHAR ucAction = 0;
	union _wtbl_debug_u debug_u;

	NdisZeroMemory(&debug_u, sizeof(union _wtbl_debug_u));

	MTWF_PRINT("%s::param=%s\n", __func__, arg);

	if (arg == NULL)
		goto error;

	Param = rstrtok(arg, ":");

	if (Param != NULL)
		u2Wcid = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");

	if (Param != NULL)
		ucCmdId = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");

	if (Param != NULL)
		ucAction = os_str_tol(Param, 0, 10);
	else
		goto error;

	MTWF_PRINT("%s():Wcid(%d), CmdId(%d), Action(%d)\n",  __func__, u2Wcid, ucCmdId, ucAction);
	mt_wtbltlv_debug(pAd, u2Wcid, ucCmdId, ucAction, &debug_u);
	return TRUE;
error:
	MTWF_PRINT("%s: param = %s not correct\n", __func__, arg);
	MTWF_PRINT("%s: iwpriv ra0 show wtbltlv=Wcid,CmdId,Action\n", __func__);
	return 0;
}

INT show_amsdu_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef DBG_AMSDU
	UINT8 slot_index;
	UINT32 i;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *mac_table_entry = NULL;
#endif

	if (tr_ctl->amsdu_type == TX_SW_AMSDU) {
		MTWF_PRINT("TX AMSDU Usage\n");
#ifdef DBG_AMSDU
		for (i = 0; IS_WCID_VALID(pAd, i); i++) {
			tr_entry = tr_entry_get(pAd, i);
			mac_table_entry = entry_get(pAd, i);
			if (!IS_ENTRY_NONE(tr_entry)) {
				MTWF_PRINT("tr_entry index = %d, amsdu_limit_len_adjust = %d\n",
					   i, mac_table_entry->amsdu_limit_len_adjust);
				MTWF_PRINT("%-10s%-12s%-12s%-12s%-12s%-12s%-12s%-12s%-12s\n",
					   "TimeSlot",
					   "amsdu_1", "amsdu_2", "amsdu_3", "amsdu_4",
					   "amsdu_5", "amsdu_6", "amsdu_7", "amsdu_8");

				for (slot_index = 0; slot_index < TIME_SLOT_NUMS; slot_index++) {
					MTWF_PRINT("%-10d", slot_index);
					MTWF_PRINT("%-12d%-12d%-12d%-12d%-12d%-12d%-12d%-12d\n",
						    tr_entry->amsdu_1_rec[slot_index],
						    tr_entry->amsdu_2_rec[slot_index],
						    tr_entry->amsdu_3_rec[slot_index],
						    tr_entry->amsdu_4_rec[slot_index],
						    tr_entry->amsdu_5_rec[slot_index],
						    tr_entry->amsdu_6_rec[slot_index],
						    tr_entry->amsdu_7_rec[slot_index],
						    tr_entry->amsdu_8_rec[slot_index]);
				}
			}
		}
#endif
	} else if (tr_ctl->amsdu_type == TX_HW_AMSDU) {
		if (chip_dbg->dump_ple_amsdu_count_info)
			return chip_dbg->dump_ple_amsdu_count_info(pAd->hdev_ctrl);
	}

	return TRUE;
}

INT show_mib_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->dump_mib_info)
		return chip_dbg->dump_mib_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowChCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowChCtrlInfo(pAd);
	return TRUE;
}

INT32 ShowFreqList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"FreqListNum = %d\n", pChCtrl->FreqListNum);

	for (i = 0; (i < pChCtrl->FreqListNum) && (pChCtrl->FreqListNum <= MAX_NUM_OF_CHANNELS); i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Freq = %d, channel = %d\n", pChCtrl->FreqList[i],
			FreqToChannel(pChCtrl->FreqList[i]));
	}

	return TRUE;
}

#ifdef GREENAP_SUPPORT
INT32 ShowGreenAPProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowGreenAPInfo(pAd);
	return TRUE;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 show_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("\tflag_pcie_aspm_dym_ctrl_cap=%d\n",
		get_pcie_aspm_dym_ctrl_cap(pAd));

	return TRUE;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 show_twt_support_cap_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i = 0;
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &pAd->StaCfg[i].wdev;
			if (wdev) {
				MTWF_PRINT("\t STA_%d, twt_support on wf_cfg=%d\n",
					i,
					wlan_config_get_he_twt_support(wdev));
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if (wdev) {
				MTWF_PRINT("\t AP_%d, twt_support on wf_cfg=%d\n",
					i,
					wlan_config_get_he_twt_support(wdev));
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

static UINT16 txop_to_ms(UINT16 *txop_level)
{
	UINT16 ms = (*txop_level) >> 5;

	ms += ((*txop_level) & (1 << 4)) ? 1 : 0;
	return ms;
}

static void dump_txop_level(UINT16 *txop_level, UINT32 len)
{
	UINT32 prio;

	for (prio = 0; prio < len; prio++) {
		UINT16 ms = txop_to_ms(txop_level + prio);

		MTWF_PRINT(" {%x:0x%x(%ums)} ", prio, *(txop_level + prio), ms);
	}

	MTWF_PRINT("\n");
}

static void dump_tx_burst_info(struct _RTMP_ADAPTER *pAd)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	EDCA_PARM *edca_param = NULL;
	UINT32 idx = 0;
	UCHAR wmm_idx = 0;
	UCHAR bss_idx = 0xff;

	MTWF_PRINT("[%s]\n", __func__);

	do {
		if (wdev[idx] == NULL)
			break;

		if (bss_idx != wdev[idx]->bss_info_argument.ucBssIndex) {
			edca_param = hwifi_get_edca(pAd, wdev[idx]);

			if (edca_param == NULL)
				break;

			wmm_idx = HcGetWmmIdx(pAd, wdev[idx]);
			MTWF_PRINT("<bss_%x>\n", wdev[idx]->bss_info_argument.ucBssIndex);
			MTWF_PRINT(" |-[wmm_idx]: %x\n", wmm_idx);
			MTWF_PRINT(" |-[bitmap]: %08x\n", wdev[idx]->bss_info_argument.prio_bitmap);
			MTWF_PRINT(" |-[prio:level]:");
			dump_txop_level(wdev[idx]->bss_info_argument.txop_level, MAX_PRIO_NUM);
			bss_idx = wdev[idx]->bss_info_argument.ucBssIndex;
		}

		MTWF_PRINT(" |---<wdev_%x>\n", idx);
		MTWF_PRINT("      |-[bitmap]: %08x\n", wdev[idx]->prio_bitmap);
		MTWF_PRINT("      |-[prio:level]:");
		dump_txop_level(wdev[idx]->txop_level, MAX_PRIO_NUM);
		idx++;
	} while (idx < WDEV_NUM_MAX);
}

INT32 show_tx_burst_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	dump_tx_burst_info(pAd);
	return TRUE;
}

INT32 show_wifi_sys(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	wifi_sys_dump(pAd);
	return TRUE;
}

INT32 show_wmm_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	hc_show_edca_info(pAd->hdev_ctrl);
	return TRUE;
}

INT32 ShowTmacInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_tmac_info)
		return chip_dbg->show_tmac_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowAggInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_agg_info)
		return chip_dbg->show_agg_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowArbInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_arb_info)
		return chip_dbg->show_arb_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT ShowManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/*UINT32 txop = 0;*/

	MTWF_PRINT("CURRENT: ManualTxOP = %d\n", pAd->CommonCfg.ManualTxop);
	MTWF_PRINT("       : bEnableTxBurst = %d\n", pAd->CommonCfg.bEnableTxBurst);
	MTWF_PRINT("       : MacTab.Size = %d\n", pAd->MacTab->Size);
	MTWF_PRINT("       : RDG_ACTIVE = %d\n", RTMP_TEST_FLAG(pAd,
			 fRTMP_ADAPTER_RDG_ACTIVE));
/*
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &txop);
	MTWF_PRINT("       : AC0 TxOP = 0x%x\n", GET_AC0LIMIT(txop));
	MTWF_PRINT("       : AC1 TxOP = 0x%x\n", GET_AC1LIMIT(txop));
*/
	return TRUE;
}

INT show_dmasch_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_dmasch_info)
		return chip_dbg->show_dmasch_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowPseInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_pse_info)
		return chip_dbg->show_pse_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT32 ShowPseData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UINT8 StartFID, FrameNums;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	Param = rstrtok(arg, ",");

	if (Param != NULL)
		StartFID = os_str_tol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ",");

	if (Param != NULL)
		FrameNums = os_str_tol(Param, 0, 10);
	else
		goto error;

	if (chip_dbg->show_pse_data)
		return chip_dbg->show_pse_data(pAd->hdev_ctrl, StartFID, FrameNums);
	else
		return FALSE;

error:
	MTWF_PRINT("param = %s not correct\n", arg);
	MTWF_PRINT("iwpriv ra0 show psedata=startfid,framenums\n");
	return 0;
}

INT ShowPLEInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP | fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD))
		return FALSE;
	if (chip_dbg->show_ple_info)
		return chip_dbg->show_ple_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_pause_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 i, x, ac_idx;
	UINT8  band_num = PD_GET_BAND_NUM(pAd->physical_dev);
	UINT8 *pu8Raw = NULL;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	UINT32 QueryCR = 0;
	struct pause_bitmap_ *ppause_bitmap = &(pAd->physical_dev->pause_bitmap);

	/* iwpriv ra0 show pauseinfo=[QueryCR] */
	if (arg != NULL)
		QueryCR = os_str_toul(arg, 0, 10);

	if (QueryCR) {
		if (chip_dbg->get_pause_by_band)
			for (i = 0; i < band_num; i++) {
				if (chip_dbg->get_pause_by_band(pAd->hdev_ctrl, arg, i) != TRUE) {
					MTWF_PRINT("BandIdx = %d can't show_pause_proc() by CR!\n", i);
				return FALSE;
			}
		}
	} else {
		if (get_sta_pause_by_cmd(pAd) != TRUE) {
			MTWF_PRINT("can't show_pause_proc() by CMD!\n");
			return FALSE;
		}
	}

	for (i = 0; i < band_num; i++) {
		pu8Raw = (UINT8 *)ppause_bitmap->pause[i];
		ac_idx = 0;

		for (x = 0; x < DRR_MAX_DW_ALL_AC(pAd); x++) {
			if (!(x % DRR_MAX_DW_PER_AC(pAd)))
				MTWF_PRINT("BN[%u][%s]AC[%d]\n", i, QueryCR ? "CR" : "CMD", ac_idx);
			MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
										(x % DRR_MAX_DW_PER_AC(pAd)),
										pu8Raw[x * 4 + 3],
										pu8Raw[x * 4 + 2],
										pu8Raw[x * 4 + 1],
										pu8Raw[x * 4]);

			if ((x % DRR_MAX_DW_PER_AC(pAd)) ==  (DRR_MAX_DW_PER_AC(pAd) - 1))
				ac_idx++;
		}

		pu8Raw = (UINT8 *)ppause_bitmap->twt_pause[i];
		MTWF_PRINT("BN[%u][%s][TWT]\n", i, QueryCR ? "CR" : "CMD");
		for (x = 0; x < DRR_MAX_DW_TWT(pAd); x++) {
			MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
										x,
										pu8Raw[x * 4 + 3],
										pu8Raw[x * 4 + 2],
										pu8Raw[x * 4 + 1],
										pu8Raw[x * 4]);
		}
	}

	return TRUE;
}

INT show_drr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_drr_info)
		return chip_dbg->show_drr_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_TXD_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT fid;

	if (arg == NULL)
		return FALSE;

	if (strlen(arg) == 0)
		return FALSE;

	fid = simple_strtol(arg, 0, 16);
	return ShowTXDInfo(pAd, fid);
}

INT32 get_sta_pause_by_cmd(RTMP_ADAPTER *pAd)
{
	UINT8 i;
	UINT8  band_num = PD_GET_BAND_NUM(pAd->physical_dev);
	RTMP_ADAPTER *ad = NULL;

	for (i = 0; i < band_num; i++) {
		MTWF_PRINT("%s: BandIdx = %d\n", __func__, i);
		ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, i);
		if (ad == NULL || UniCmdGetStaPause(ad, i) != TRUE)
			return FALSE;
	}

	return TRUE;
}

#define DUMP_MEM_SIZE 64
#define TOTAL_PAIR DUMP_MEM_SIZE >> 2
INT ShowTXDInfo(RTMP_ADAPTER *pAd, UINT fid)
{
	INT i = 0;
	UINT8 data[DUMP_MEM_SIZE];
	UINT32 Addr = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef WIFI_UNIFIED_COMMAND
	INT32 Ret = NDIS_STATUS_SUCCESS;
#endif /* WIFI_UNIFIED_COMMAND */

	if (fid >= cap->fid_invalid_id)
		return FALSE;

	os_zero_mem(data, DUMP_MEM_SIZE);
	Addr = 0xa << 28 | fid << cap->fid_shift; /* TXD addr: 0x{a}{fid}{0000}*/

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		UINT32 TotalPair = (DUMP_MEM_SIZE / 4);
		struct _RTMP_REG_PAIR RegPair[TOTAL_PAIR];
		struct _RTMP_REG_PAIR *pNextRegPair = NULL;

		os_zero_mem(RegPair, sizeof(RegPair));
		for (i = 0; i < TotalPair; i++) {
			pNextRegPair = &RegPair[i];
			pNextRegPair->Register = Addr + (i * 4);
		}

		Ret = UniCmdMultipleMacRegAccessRead(pAd, RegPair, TotalPair);
		if (Ret == NDIS_STATUS_SUCCESS) {
			pNextRegPair = &RegPair[0];
			for (i = 0; i < DUMP_MEM_SIZE; i = i + 4) {
				os_move_mem(&data[i], &pNextRegPair->Value, sizeof(pNextRegPair->Value));
				MTWF_PRINT("DW%02d: 0x%02x%02x%02x%02x\n", i / 4,
						data[i + 3], data[i + 2], data[i + 1], data[i]);
				pNextRegPair++;
			}
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		MtCmdMemDump(pAd, Addr, &data[0]);

		for (i = 0; i < DUMP_MEM_SIZE; i = i + 4)
			MTWF_PRINT("DW%02d: 0x%02x%02x%02x%02x\n", i / 4,
					data[i + 3], data[i + 2], data[i + 1], data[i]);
	}
	asic_dump_tmac_info(pAd, &data[0]);

	return TRUE;
}
INT show_mem_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	INT32 Ret = NDIS_STATUS_SUCCESS;
#endif /* WIFI_UNIFIED_COMMAND */
	UINT Addr = os_str_tol(arg, 0, 16);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		UINT32 TotalPair = (DUMP_MEM_SIZE / 4);
		struct _RTMP_REG_PAIR RegPair[TOTAL_PAIR];
		struct _RTMP_REG_PAIR *pNextRegPair = NULL;

		os_zero_mem(RegPair, sizeof(RegPair));

		for (i = 0; i < TotalPair; i++) {
			pNextRegPair = &RegPair[i];
			pNextRegPair->Register = Addr + (i * 4);
		}

		Ret = UniCmdMultipleMacRegAccessRead(pAd, RegPair, TotalPair);
		if (Ret == NDIS_STATUS_SUCCESS) {
			for (i = 0; i < TotalPair; i++) {
				pNextRegPair = &RegPair[i];
				MTWF_PRINT("addr 0x%08x: 0x%08x\n", pNextRegPair->Register, pNextRegPair->Value);
			}
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		UINT8 data[DUMP_MEM_SIZE];

		os_zero_mem(data, DUMP_MEM_SIZE);
		MtCmdMemDump(pAd, Addr, &data[0]);

		for (i = 0; i < DUMP_MEM_SIZE; i = i + 4)
		MTWF_PRINT("addr 0x%08x: 0x%02x%02x%02x%02x\n", Addr + i, data[i + 3],
				 data[i + 2], data[i + 1], data[i]);
	}

	return TRUE;
}
INT show_protect_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_protect_info)
		return chip_dbg->show_protect_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_cca_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_cca_info)
		return chip_dbg->show_cca_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;

}

#endif /*MT_MAC*/


INT Show_sta_tr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	STA_TR_ENTRY *tr_entry;

	for (idx = 0; IS_WCID_VALID(pAd, idx); idx++) {
		tr_entry = tr_entry_get(pAd, idx);

		if (IS_VALID_ENTRY(tr_entry))
			TRTableEntryDump(pAd, idx, __func__, __LINE__);
	}

	return TRUE;
}

INT show_stainfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate = 0, irqflags;
	UCHAR mac_addr[MAC_ADDR_LEN];
	RTMP_STRING *token;
	CHAR sep[1] = {':'};
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;

	MTWF_PRINT("%s(): Input string=%s\n",
			 __func__, arg);

	for (i = 0, token = rstrtok(arg, &sep[0]); token && (i < MAC_ADDR_LEN);
			token = rstrtok(NULL, &sep[0]), i++) {
		MTWF_PRINT("%s(): token(len=%zu) =%s\n",
				 __func__, strlen(token), token);

		if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
			return FALSE;

		AtoH(token, (&mac_addr[i]), 1);
	}

	MTWF_PRINT("%s(): i= %d\n", __func__, i);

	if (i != 6)
		return FALSE;

	MTWF_PRINT("\nAddr "MACSTR"\n", MAC2STR(mac_addr));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, (UCHAR *)mac_addr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, (UCHAR *)mac_addr, NULL);
#endif

	if (!pEntry)
		return FALSE;

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Invalid MAC address!\n");
		return FALSE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("EntryType : %d\n", pEntry->EntryType);
	MTWF_PRINT("Entry Capability:\n");
	MTWF_PRINT("\tPhyMode:%-10s\n", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
	MTWF_PRINT("\tBW:%-6s\n", get_bw_str(pEntry->MaxHTPhyMode.field.BW, BW_FROM_OID));
	MTWF_PRINT("\tDataRate:\n");
#ifdef DOT11_VHT_AC

	if (pEntry->MaxHTPhyMode.field.MODE >= MODE_VHT)
		MTWF_PRINT("%dS-M%d", ((pEntry->MaxHTPhyMode.field.MCS >> 4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
		MTWF_PRINT(" %-6d", pEntry->MaxHTPhyMode.field.MCS);

	MTWF_PRINT(" %-6d", pEntry->MaxHTPhyMode.field.ShortGI);
	MTWF_PRINT(" %-6d\n", pEntry->MaxHTPhyMode.field.STBC);
	MTWF_PRINT("Entry Operation Features\n");
	MTWF_PRINT("\t%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");
	DataRate = 0;
	getRate(pEntry->HTPhyMode, &DataRate);
	MTWF_PRINT("\t%-4d", (int)pEntry->Aid);
	MTWF_PRINT("%-4d", (int)pEntry->func_tb_idx);
	MTWF_PRINT("%-4d", (int)pEntry->PsMode);
	MTWF_PRINT("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
	MTWF_PRINT("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
	MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[0]);
	MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[1]);
	MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[2]);
	MTWF_PRINT("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
	MTWF_PRINT("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW, BW_FROM_OID));
#ifdef DOT11_VHT_AC

	if (pEntry->HTPhyMode.field.MODE >= MODE_VHT)
		MTWF_PRINT("%dS-M%d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
		MTWF_PRINT("%-6d", pEntry->HTPhyMode.field.MCS);

	MTWF_PRINT("%-6d", pEntry->HTPhyMode.field.ShortGI);
	MTWF_PRINT("%-6d", pEntry->HTPhyMode.field.STBC);
#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
	/* If TWT agreement is present for this STA, add maximum TWT wake up interval in sta idle timeout. */
	if (GET_PEER_ITWT_FID_BITMAP(pEntry)) {
		MTWF_PRINT("%-7d", (int)((pEntry->StaIdleTimeout + pEntry->twt_ctrl.twt_interval_max) - pEntry->NoDataIdleCount));
	} else
#endif
	{
		MTWF_PRINT("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
	}

	MTWF_PRINT("%-7d", (int)DataRate);
	MTWF_PRINT("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
		   (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
	MTWF_PRINT("\n");
	ASSERT(pEntry->wcid <= GET_MAX_UCAST_NUM(pAd));
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	MTWF_PRINT("Entry TxRx Info\n");
	MTWF_PRINT("\tEntryType : %d\n", tr_entry->EntryType);
	MTWF_PRINT("\tHookingWdev : %p\n", tr_entry->wdev);
	MTWF_PRINT("\tIndexing : FuncTd=%d, WCID=%d\n", tr_entry->func_tb_idx, tr_entry->wcid);
	MTWF_PRINT("Entry TxRx Features\n");
	MTWF_PRINT("\tIsCached, PortSecured, PsMode, LockTx, VndAth\n");
	MTWF_PRINT("\t%d\t%d\t%d\t%d\t%d\n", tr_entry->isCached, tr_entry->PortSecured,
		   tr_entry->PsMode, tr_entry->LockEntryTx,
		   tr_entry->bIAmBadAtheros);
	MTWF_PRINT("\t%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", "TxQId", "PktNum", "QHead", "QTail", "EnQCap", "DeQCap", "PktSeq");

#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_ext_t *mld_entry_ext = NULL;

		if (!mld_entry_ext_get(pEntry, &mld_entry_ext))
			seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
	} else
#endif /* DOT11_EHT_BE */
	{
		seq_ctrl = &tr_entry->seq_ctrl;
	}

	for (i = 0; i < WMM_QUE_NUM;  i++) {
		RTMP_IRQ_LOCK(&tr_entry->txq_lock[i], irqflags);
		MTWF_PRINT("\t%d %6d  %p  %6p %d %d %d\n",
			   i,
			   tr_entry->tx_queue[i].Number,
			   tr_entry->tx_queue[i].Head,
			   tr_entry->tx_queue[i].Tail,
			   tr_entry->enq_cap, tr_entry->deq_cap,
			   seq_ctrl ? seq_ctrl->TxSeq[i] : 0);
		RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[i], irqflags);
	}

	RTMP_IRQ_LOCK(&tr_entry->ps_queue_lock, irqflags);
	MTWF_PRINT("\tpsQ %6d  %p  %p %d %d  NoQ:%d\n",
		   tr_entry->ps_queue.Number,
		   tr_entry->ps_queue.Head,
		   tr_entry->ps_queue.Tail,
		   tr_entry->enq_cap, tr_entry->deq_cap,
		   seq_ctrl ? seq_ctrl->NonQosDataSeq : 0);
	RTMP_IRQ_UNLOCK(&tr_entry->ps_queue_lock, irqflags);
	MTWF_PRINT("\n");
	return TRUE;
}

extern INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_devinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR *pstr;

	MTWF_PRINT("Device MAC\n");

	if (pAd->OpMode == OPMODE_AP)
		pstr = "AP";
	else if (pAd->OpMode == OPMODE_STA)
		pstr = "STA";
	else
		pstr = "Unknown";

	MTWF_PRINT("Operation Mode: %s\n", pstr);
	MTWF_PRINT("PD_GET_HW_WTBL_SUPPORT: %x\n", PD_GET_HW_WTBL_SUPPORT(pAd->physical_dev));
#ifdef SW_CONNECT_SUPPORT
	MTWF_PRINT("SwStaEnable: %d\n",
		hc_is_sw_sta_enable(pAd));
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("WTBL_MAX_NUM: %u, WTBL_MAX_NUM_ONLY_HW: %u\n",
		WTBL_MAX_NUM(pAd), WTBL_MAX_NUM_ONLY_HW(pAd));
	MTWF_PRINT("GET_MAX_UCAST_NUM: %u, GET_MAX_UCAST_NUM_ONLY_HW: %u\n",
		GET_MAX_UCAST_NUM(pAd), GET_MAX_UCAST_NUM_ONLY_HW(pAd));
	show_radio_info_proc(pAd, arg);
	return TRUE;
}

INT show_wdev_info(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev)
			break;
	}

	if (idx >= WDEV_NUM_MAX) {
		MTWF_PRINT("ERR! Cannot found required wdev(%p)!\n", wdev);
		return FALSE;
	}

	MTWF_PRINT("WDEV Instance(%d) Info:\n", idx);
	return TRUE;
}


CHAR *wdev_type_str[] = {"AP", "STA", "ADHOC", "WDS", "MESH", "GO", "GC", "APCLI", "REPEATER", "P2P_DEVICE", "SERVICE", "ATE", "Unknown"};

RTMP_STRING *wdev_type2str(int type)
{
	switch (type) {
	case WDEV_TYPE_AP:
		return wdev_type_str[0];

	case WDEV_TYPE_STA:
		return wdev_type_str[1];

	case WDEV_TYPE_ADHOC:
		return wdev_type_str[2];

	case WDEV_TYPE_WDS:
		return wdev_type_str[3];

	case WDEV_TYPE_MESH:
		return wdev_type_str[4];

	case WDEV_TYPE_GO:
		return wdev_type_str[5];

	case WDEV_TYPE_GC:
		return wdev_type_str[6];

	/*case WDEV_TYPE_APCLI:
		return wdev_type_str[7];*/

	case WDEV_TYPE_REPEATER:
		return wdev_type_str[8];

	case WDEV_TYPE_P2P_DEVICE:
		return wdev_type_str[9];

	case WDEV_TYPE_SERVICE_TXC:
	case WDEV_TYPE_SERVICE_TXD:
		return wdev_type_str[10];

	case WDEV_TYPE_ATE_AP:
	case WDEV_TYPE_ATE_STA:
		return wdev_type_str[11];

	default:
		return wdev_type_str[12];
	}
}


INT show_sysinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	UINT32 total_size = 0, cntr_size;
	struct wifi_dev *wdev;
	UCHAR ext_cha;
#ifdef CONFIG_STA_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	BSS_TABLE *ScanTab = NULL;
	if (IfIdx < pAd->MSTANum)
		pStaCfg = &pAd->StaCfg[IfIdx];
	else {
		MTWF_PRINT("%s: invalid IfIdx=%d.\n", __func__, IfIdx);
		return FALSE;
	}

	wdev = &pStaCfg->wdev;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#endif

	MTWF_PRINT("Device Instance\n");

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_PRINT ("\tWDEV %02d:", idx);

		if (pAd->wdev_list[idx]) {
			UCHAR *str = NULL;

			wdev = pAd->wdev_list[idx];
			MTWF_PRINT("\n\t\tName/Type:%s/%s\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev),
					 wdev_type2str(wdev->wdev_type));
			MTWF_PRINT("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx);
			MTWF_PRINT("\t\tMacAddr:"MACSTR"\n", MAC2STR(wdev->if_addr));
			MTWF_PRINT("\t\tBSSID:"MACSTR"\n", MAC2STR(wdev->bssid));
			str = wmode_2_str(wdev->PhyMode);

			if (str) {
				MTWF_PRINT("\t\tPhyMode:%s\n", str);
				os_free_mem(str);
			}

			ext_cha = wlan_config_get_ext_cha(wdev);
			MTWF_PRINT("\t\tChannel:%d,ExtCha:%d\n", wdev->channel, ext_cha);
			MTWF_PRINT("\t\tClient Number:%d\n", wdev->client_num);
			MTWF_PRINT("\t\tPortSecured/ForbidTx: %d(%sSecured)/%lx\n",
					 wdev->PortSecured,
					 (wdev->PortSecured == WPA_802_1X_PORT_SECURED ? "" : "Not"),
					 wdev->forbid_data_tx);
			MTWF_PRINT("\t\tEdcaIdx:%d\n", wdev->EdcaIdx);
			MTWF_PRINT("\t\tif_dev:0x%p\tfunc_dev:[%d]0x%p\tsys_handle:0x%p\n",
					 wdev->if_dev, wdev->func_idx, wdev->func_dev, wdev->sys_handle);
			MTWF_PRINT("\t\tIgmpSnoopEnable:%d\n", wdev->IgmpSnoopEnable);
#ifdef LINUX

			if (wdev->if_dev) {
				UINT idx, q_num;
				UCHAR *mac_str = RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev);

				MTWF_PRINT("\t\tOS NetDev status(%s[%d]-"MACSTR"):\n",
						  RtmpOsGetNetDevName(wdev->if_dev),
						  RtmpOsGetNetIfIndex(wdev->if_dev),
						  MAC2STR(mac_str));
				MTWF_PRINT("\t\t\tdev->state: 0x%lx\n", RtmpOSGetNetDevState(wdev->if_dev));
				MTWF_PRINT("\t\t\tdev->flag: 0x%x\n", RtmpOSGetNetDevFlag(wdev->if_dev));
				q_num = RtmpOSGetNetDevQNum(wdev->if_dev);

				for (idx = 0; idx < q_num; idx++) {
					MTWF_PRINT("\t\t\tdev->queue[%d].state: 0x%lx\n", idx,
							  RtmpOSGetNetDevQState(wdev->if_dev, idx));
				}
				MTWF_PRINT("\t\t\trx_drop_long_len: 0x%x\n", wdev->rx_drop_long_len);
			}

#endif /* LINUX */
		} else
			MTWF_PRINT ("\n");
	}

	MTWF_PRINT("Memory Statistics:\n");
	MTWF_PRINT("\tsize>\n");
	MTWF_PRINT("\t\tpAd = \t\t%zu bytes\n\n", sizeof(*pAd));
	MTWF_PRINT("\t\t\tCommonCfg = \t%zu bytes\n", sizeof(pAd->CommonCfg));
	if (pAd->physical_dev) {
		MTWF_PRINT("PD_GET_HW_WTBL_MAX(device)=%u, sizeof(struct _MAC_TABLE_ENTRY)=%u, Total Entry Size=%u \n",
			PD_GET_HW_WTBL_MAX(pAd->physical_dev), (UINT16)(sizeof(struct _MAC_TABLE_ENTRY)), (UINT16)(sizeof(struct _MAC_TABLE_ENTRY) * PD_GET_HW_WTBL_MAX(pAd->physical_dev)));
	}
	total_size += sizeof(pAd->CommonCfg);
#ifdef CONFIG_AP_SUPPORT
	MTWF_PRINT("\t\t\tApCfg = \t%zu bytes\n", sizeof(pAd->ApCfg));
	total_size += sizeof(pAd->ApCfg);
	MTWF_PRINT("\t\t\t\tMBSSID = \t%zu B (PerMBSS =%zu B, Total MBSS Num= %d)\n",
			 sizeof(pAd->ApCfg.MBSSID), sizeof(struct _BSS_STRUCT), MAX_BEACON_NUM);
#ifdef APCLI_SUPPORT
	MTWF_PRINT("\t\t\t\t\tAPCLI = \t%zu bytes (PerAPCLI =%zu bytes, Total APCLI Num= %d)\n",
			  sizeof(pAd->StaCfg), sizeof(struct _STA_ADMIN_CONFIG), MAX_APCLI_NUM);
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_MAC_PCI
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	INT idx2;

	cntr_size = 0;
	for (idx = 0; idx < hif->tx_res_num; idx++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, idx);

		cntr_size += tx_ring->desc_ring.AllocSize;
		cntr_size += tx_ring->buf_space.AllocSize;
	}
	MTWF_PRINT("\t\t\tTxRing = \t%d bytes\n", cntr_size);
	total_size += cntr_size;
	cntr_size = 0;
	for (idx = 0; idx < hif->rx_res_num; idx++) {
		struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, idx);
		cntr_size += rx_ring->desc_ring.AllocSize;
		for (idx2 = 0; idx2 < rx_ring->ring_size; idx2++)
			cntr_size += rx_ring->Cell[idx2].DmaBuf.AllocSize;
	}
	MTWF_PRINT("\t\t\tRxRing = \t%d bytes\n", cntr_size);
	total_size += cntr_size;
}
#endif /* RTMP_MAC_PCI */
	MTWF_PRINT("\t\t\tMlme = \t%zu bytes\n", sizeof(pAd->Mlme));
	total_size += sizeof(pAd->Mlme);
#ifdef CONFIG_STA_SUPPORT
	MTWF_PRINT("\t\t\tMlmeAux = \t%zu bytes\n", sizeof(pStaCfg->MlmeAux));
	total_size += sizeof(pStaCfg->MlmeAux);
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("\t\t\tMacTab = \t%zu bytes\n", sizeof(*pAd->MacTab));
	total_size += sizeof(pAd->MacTab);
	MTWF_PRINT("\t\t\tBA Control = \t%zu bytes\n", sizeof(pAd->physical_dev->ba_ctl));
	total_size += sizeof(pAd->physical_dev->ba_ctl);
	cntr_size = sizeof(pAd->Counters8023) + sizeof(pAd->WlanCounters) +
				sizeof(pAd->RalinkCounters) + /* sizeof(pAd->DrsCounters) */ +
				sizeof(pAd->PrivateInfo);
	MTWF_PRINT("\t\t\tCounter** = \t%d bytes\n", cntr_size);
	total_size += cntr_size;
#ifdef CONFIG_STA_SUPPORT
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	MTWF_PRINT("\t\t\tScanTab = \t%zu bytes\n", sizeof(ScanTab));
	total_size += sizeof(ScanTab);
#endif
#endif
	MTWF_PRINT("\tsize> Total = \t\t%d bytes, Others = %zu bytes\n\n",
			 total_size, sizeof(*pAd) - total_size);
	return TRUE;
}

void wifi_dump_info(void)
{
	RTMP_ADAPTER *pAd = NULL;
	UCHAR idx = 0;
	MTWF_PRINT("%s--------------------\n", __func__);
	for (idx = 0 ; idx < MAX_NUM_OF_INF; idx++) {
#ifdef MULTI_INF_SUPPORT
		pAd = adapt_list[idx];
#endif
		if (pAd) {
			show_tpinfo_host(pAd, WFDMA_INFO, 0, 0);
			show_tpinfo_host(pAd, COUNTER_INFO, FALSE, 0);
			show_trinfo_proc(pAd, "");
			ShowPLEInfo(pAd, "");
			ShowPseInfo(pAd, "");
			Show_PSTable_Proc(pAd, "");
			show_swqinfo(pAd, "");
#ifdef ERR_RECOVERY
			ShowSerProc2(pAd, "");
#endif
		}
	}
}
EXPORT_SYMBOL(wifi_dump_info);

#ifdef CONFIG_TP_DBG
INT Set_TPDbg_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT dbg;
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;

	dbg = simple_strtol(arg, 0, 10);
	tp_dbg->debug_flag = dbg;
	if (!(dbg & TP_DEBUG_TIMING)) {
		memset(tp_dbg->TRDoneTimesRec, 0x0, sizeof(tp_dbg->TRDoneTimesRec));
		memset(tp_dbg->TRDoneInterval, 0x0, sizeof(tp_dbg->TRDoneInterval));
	}
	MTWF_PRINT("%s(): (TPDebugLevel = %d)\n", __func__, dbg);

	return TRUE;
}

INT show_TPDbg_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tp_dbg_slot_idx;
	UCHAR dump = 0;
	UCHAR dbg_detail_lvl = DBG_LVL_INFO;
	UCHAR time_slot_num = TP_DBG_TIME_SLOT_NUMS;
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;

	if (arg != NULL)
		dump = os_str_toul(arg, 0, 16);

	if (dump == 1)
		dbg_detail_lvl = DBG_LVL_OFF;
	else if (dump == 2)
		dbg_detail_lvl = DBG_LVL_INFO;
	else if (dump == 3) {
		/* show less information for MSP debugging */
		dbg_detail_lvl = DBG_LVL_OFF;
		time_slot_num = TP_DBG_TIME_SLOT_NUMS/2;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, dbg_detail_lvl,
		"\n\tTimeSlot \tTxIsr \tRxIsr/Rx1Isr/RxDlyIsr \t\tTxIoRead/TxIoWrite"
		" \tRxIoRead/RxIoWrite \tRx1IoRead/Rx1IoWrite\n");

	for (tp_dbg_slot_idx = 0; tp_dbg_slot_idx < time_slot_num; tp_dbg_slot_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, dbg_detail_lvl,
				"\t%d \t\t%d \t%5d/%6d/%8d \t\t%8d/%9d \t%8d/%9d \t%9d/%10d\n",
				tp_dbg_slot_idx, tp_dbg->IsrTxCntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRxCntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRx1CntRec[tp_dbg_slot_idx],
				tp_dbg->IsrRxDlyCntRec[tp_dbg_slot_idx],
				tp_dbg->IoReadTxRec[tp_dbg_slot_idx],
				tp_dbg->IoWriteTxRec[tp_dbg_slot_idx],
				tp_dbg->IoReadRxRec[tp_dbg_slot_idx],
				tp_dbg->IoWriteRxRec[tp_dbg_slot_idx],
				tp_dbg->IoReadRx1Rec[tp_dbg_slot_idx],
				tp_dbg->IoWriteRx1Rec[tp_dbg_slot_idx]);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, dbg_detail_lvl,
		"\n\tTimeSlot \tRx0Cnt_A/Cnt_B/Cnt_C/Cnt_D "
		"\t\tRx1Cnt_A/Cnt_B/Cnt_C/Cnt_D \t\tTRdone \t\tTRdoneInterval\n");

	for (tp_dbg_slot_idx = 0; tp_dbg_slot_idx < time_slot_num; tp_dbg_slot_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, dbg_detail_lvl,
			"\t%d \t\t%8d/%5d/%5d/%5d \t\t%8d/%5d/%5d/%5d \t\t%d \t\t%d\n",
			tp_dbg_slot_idx, tp_dbg->MaxProcessCntRxRecA[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRxRecB[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRxRecC[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRxRecD[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRx1RecA[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRx1RecB[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRx1RecC[tp_dbg_slot_idx],
			tp_dbg->MaxProcessCntRx1RecD[tp_dbg_slot_idx],
			tp_dbg->TRDoneTimesRec[tp_dbg_slot_idx],
			tp_dbg->TRDoneInterval[tp_dbg_slot_idx]);
	}
	return TRUE;
}
#endif /* CONFIG_TP_DBG */

static VOID show_counter_info(RTMP_ADAPTER *pAd, UINT32 en_rx_profiling)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	UINT8 i, j;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _rx_profiling *rx_rate_rc = NULL;
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT16 counting_range = qm_parm->max_data_que_num / (queue_deep_cnt_steps-1);

	/* TX */
	MTWF_PRINT("\nTX Counter\n");
	MTWF_PRINT("\tamsdu type = %s\n", tr_ctl->amsdu_type ? "TX_HW_AMSDU" : "TX_SW_AMSDU");
	MTWF_PRINT("\tnet_if_stop_cnt = %d\n", tr_cnt->net_if_stop_cnt);
	if (tr_cnt->net_if_stop_cnt != 0) {
		UCHAR que_idx = FP_QUE0;
		struct wifi_dev *wdev_block = NULL;
		struct fp_tx_flow_control *flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(pAd->physical_dev);

		for (que_idx = FP_QUE0; que_idx < FP_QUE_NUM; que_idx++) {
			OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
			DlListForEach(wdev_block, &flow_ctl->TxBlockDevList[que_idx], struct wifi_dev, tx_block_list) {
				if (wdev_block)
					MTWF_PRINT ("\t\t(%s is stopped)\n", wdev_block->if_dev->name);
			}
			OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
		}
	}
	MTWF_PRINT ("\ttx_invalid_wdev = %d\n", tr_cnt->tx_invalid_wdev);
	MTWF_PRINT ("\ttx_sw_dataq_drop = %d\n", tr_cnt->tx_sw_dataq_drop);
	MTWF_PRINT ("\ttx_sw_mgmtq_drop = %d\n", tr_cnt->tx_sw_mgmtq_drop);
	MTWF_PRINT ("\ttx_sw_probe_rsp_drop = %d\n", tr_cnt->tx_sw_probe_rsp_drop);
	MTWF_PRINT ("\ttx_wcid_invalid = %d\n", tr_cnt->tx_wcid_invalid);
	MTWF_PRINT ("\twlan_state_non_valid_drop = %d\n", tr_cnt->wlan_state_non_valid_drop);
	MTWF_PRINT ("\ttx_not_allowed_drop = %d\n", tr_cnt->tx_not_allowed_drop);
	MTWF_PRINT ("\ttx_change_to_setup_link = %d\n", tr_cnt->tx_change_to_setup_link);
	MTWF_PRINT ("\tsys_not_ready_drop = %d\n", tr_cnt->sys_not_ready_drop);
	MTWF_PRINT ("\terr_recovery_drop = %d\n", tr_cnt->err_recovery_drop);
	MTWF_PRINT ("\ttx_forbid_drop = %d\n", tr_cnt->tx_forbid_drop);
	MTWF_PRINT ("\tigmp_clone_fail_drop = %d\n", tr_cnt->igmp_clone_fail_drop);
	MTWF_PRINT ("\tigmp_unknown_drop = %d\n", tr_cnt->igmp_unknown_drop);
	MTWF_PRINT ("\tps_max_drop = %d\n", tr_cnt->ps_max_drop);
	MTWF_PRINT ("\tfill_tx_blk_fail_drop = %d\n", tr_cnt->fill_tx_blk_fail_drop);
	MTWF_PRINT ("\tcarrier_detect_drop = %d\n", tr_cnt->carrier_detect_drop);
	MTWF_PRINT ("\tsilence_in_mlo_allow = %d\n", tr_cnt->silence_in_mlo_allow);
	MTWF_PRINT ("\ttx_unknow_type_drop = %d\n", tr_cnt->tx_unknow_type_drop);
	MTWF_PRINT ("\ttx_tcp_rack_drop = %d\n", tr_cnt->tx_tcp_rack_drop);
	MTWF_PRINT ("\ttx_invalid_mgmt_drop = %d\n", tr_cnt->tx_invalid_mgmt_drop);
	MTWF_PRINT ("\ttx_invalid_data_drop = %d\n", tr_cnt->tx_invalid_data_drop);
	MTWF_PRINT ("\ttx_hwifi_err_drop = %d\n", tr_cnt->tx_hwifi_err_drop);
	MTWF_PRINT ("\tba_tx_bar_cnt = %d\n", tr_cnt->ba_tx_bar_cnt);
#if defined(DOT11K_RRM_SUPPORT) && defined(QUIET_SUPPORT)
	MTWF_PRINT ("\ttx_rrm_quiet_drop = %d\n", tr_cnt->tx_rrm_quiet_drop);
#endif
#ifdef DOT11_EHT_BE
	MTWF_PRINT("\tapcli_bmc_loop_drop = %d\n", tr_cnt->apcli_bmc_loop_drop);
#endif
	MTWF_PRINT ("\tpkt_len_invalid = %d\n", tr_cnt->pkt_len_invalid);
	MTWF_PRINT ("\tpkt_invalid_wcid = %d\n", tr_cnt->pkt_invalid_wcid);
	for (i = 0; i < NUM_OF_TID; i++) {
		if (tr_cnt->me[i] > 0) {
			MTWF_PRINT ("\tme = %d\n", tr_cnt->me[i]);
		}
		if (tr_cnt->re[i] > 0) {
			MTWF_PRINT ("\tre = %d\n", tr_cnt->re[i]);
		}
		if (tr_cnt->le[i] > 0) {
			MTWF_PRINT ("\tle = %d\n", tr_cnt->le[i]);
		}
		if (tr_cnt->be[i] > 0) {
			MTWF_PRINT ("\tbe = %d\n", tr_cnt->be[i]);
		}
		if (tr_cnt->txop_limit_error[i] > 0) {
			MTWF_PRINT ("\ttxop_limit_error = %d\n",
				tr_cnt->txop_limit_error[i]);
		}
		if (tr_cnt->baf[i] > 0) {
			MTWF_PRINT ("\tbaf = %d\n", tr_cnt->baf[i]);
		}
	}
	for (i = 0; i < queue_deep_cnt_steps; i++)
		MTWF_PRINT ("\tqueue_deep_cnt[%u-%u] = %d\n",
			counting_range*i,
			(i == (queue_deep_cnt_steps-1))?qm_parm->max_data_que_num:counting_range*(i+1)-1,
			tr_cnt->queue_deep_cnt[i]);

#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	for (i = 0; i < sizeof(tr_cnt->ctxd_num) / sizeof(UINT32); i++)
		MTWF_PRINT ("\tctxd_cnt[%d] = %d\n", i, tr_cnt->ctxd_num[i]);
#endif

	for (j = 0; j < 4; j++)
		MTWF_PRINT ("\tCPU(%d)TX Enq(%u)/Deq(%u)\n", j, tr_cnt->tx_enq_cpu_stat[j], tr_cnt->tx_deq_cpu_stat[j]);

	/* RX */
	MTWF_PRINT ("\nRX Counter\n");
	MTWF_PRINT ("\tdamsdu type = %s\n", tr_ctl->amsdu_type ? "RX_HW_AMSDU" : "RX_SW_AMSDU");
	MTWF_PRINT ("\trx_icv_err_cnt = %d\n", tr_cnt->rx_icv_err_cnt);
	MTWF_PRINT ("\trx_sw_q_drop = %d\n", tr_cnt->rx_sw_q_drop);
	MTWF_PRINT ("\trx_invalid_pkt_drop = %d\n", tr_cnt->rx_invalid_pkt_drop);
	MTWF_PRINT ("\trx_invalid_wdev = %d\n", tr_cnt->rx_invalid_wdev);
	MTWF_PRINT ("\trx_rx_invalid_rxblk_drop = %d\n", tr_cnt->rx_invalid_rxblk_drop);
	MTWF_PRINT ("\trx_invalid_wcid_drop = %d\n", tr_cnt->rx_invalid_wcid_drop);
	MTWF_PRINT ("\trx_invalid_small_pkt_drop = %d\n", tr_cnt->rx_invalid_small_pkt_drop);
	MTWF_PRINT ("\trx_invalid_large_pkt_drop = %d\n", tr_cnt->rx_invalid_large_pkt_drop);
	MTWF_PRINT ("\trx_a4_conn_drop_a3_bmc = %d\n", tr_cnt->rx_a4_conn_drop_a3_bmc);
	MTWF_PRINT ("\trx_crc_err_drop = %d\n", tr_cnt->rx_crc_err_drop);
	MTWF_PRINT ("\trx_not_allowed_drop = %d\n", tr_cnt->rx_not_allowed_drop);
	MTWF_PRINT ("\trx_invalid_amsdu_drop = %d\n", tr_cnt->rx_invalid_amsdu_drop);
	MTWF_PRINT ("\trx_duplicate_drop = %d\n", tr_cnt->rx_duplicate_drop);
	MTWF_PRINT ("\trx_invalid_frag_drop = %d\n", tr_cnt->rx_invalid_frag_drop);
	MTWF_PRINT ("\trx_cipher_mismatch_drop = %d\n", tr_cnt->rx_cipher_mismatch_drop);
	MTWF_PRINT ("\trx_to_os_drop = %d\n", tr_cnt->rx_to_os_drop);
	MTWF_PRINT ("\trx_pn_mismatch = %d\n", tr_cnt->rx_pn_mismatch);
	MTWF_PRINT ("\tba_err_wcid_invalid = %d\n", tr_cnt->ba_err_wcid_invalid);
	MTWF_PRINT ("\tba_drop_unknown = %d\n", tr_cnt->ba_drop_unknown);
	MTWF_PRINT ("\tba_err_old = %d\n", tr_cnt->ba_err_old);
	MTWF_PRINT ("\tba_err_dup1 = %d\n", tr_cnt->ba_err_dup1);
	MTWF_PRINT ("\tba_err_dup2 = %d\n", tr_cnt->ba_err_dup2);
	MTWF_PRINT ("\tba_err_tear_down = %d\n", tr_cnt->ba_err_tear_down);
	MTWF_PRINT ("\tba_flush_one = %d\n", tr_cnt->ba_flush_one);
	MTWF_PRINT ("\tba_flush_all = %d\n", tr_cnt->ba_flush_all);
	MTWF_PRINT ("\tba_sn_large_win_end = %d\n", tr_cnt->ba_sn_large_win_end);
	MTWF_PRINT ("\tba_rx_bar_cnt = %d\n", tr_cnt->ba_rx_bar_cnt);
	MTWF_PRINT ("\tbar_large_win_start = %d\n", tr_cnt->bar_large_win_start);
	MTWF_PRINT ("\tba_amsdu_miss = %d\n", tr_cnt->ba_amsdu_miss);
#ifdef CONFIG_CSO_SUPPORT
	MTWF_PRINT ("\trx_cso_err_drop = %d\n", tr_cnt->rx_cso_err_drop);
#endif
	MTWF_PRINT ("\tB%u:RMAC_ppdu_drop = %u\n", i,
		pAd->mcli_ctl.RxPPDUDropCnt);
	MTWF_PRINT ("\tB%u:RMAC_RXFIFO_NOT_ENOUGH = %u\n", i,
		pAd->mcli_ctl.RxFIFONotEnoughCnt);
	pAd->mcli_ctl.RxPPDUDropCnt = pAd->mcli_ctl.RxFIFONotEnoughCnt = 0;

	if (pAd->tr_ctl.en_rx_profiling != en_rx_profiling && ops->ctrl_rxv_group) {
		if (en_rx_profiling > 0) {
			pAd->tr_ctl.en_rx_profiling = TRUE;
			MTWF_PRINT("\t===== Enable RX rate statistics report(%d) =====\n", en_rx_profiling);
			ops->ctrl_rxv_group(pAd, BAND0, 0x2, TRUE);
			ops->ctrl_rxv_group(pAd, BAND1, 0x2, TRUE);
			ops->ctrl_rxv_group(pAd, BAND0, 0x3, TRUE);
			ops->ctrl_rxv_group(pAd, BAND1, 0x3, TRUE);
		} else {
			pAd->tr_ctl.en_rx_profiling = FALSE;
			os_zero_mem(&pAd->tr_ctl.tr_cnt.rx_rate_rc, sizeof(struct _rx_profiling));
			MTWF_PRINT
				("\t===== Disable RX rate statistics report(%d) =====\n", en_rx_profiling);
			ops->ctrl_rxv_group(pAd, BAND0, 0x2, FALSE);
			ops->ctrl_rxv_group(pAd, BAND1, 0x2, FALSE);
			ops->ctrl_rxv_group(pAd, BAND0, 0x3, FALSE);
			ops->ctrl_rxv_group(pAd, BAND1, 0x3, FALSE);
		}
	} else if (pAd->tr_ctl.en_rx_profiling) {
		UINT8 band_idx = 0, bw_idx = 0;
		BOOLEAN rx_rate_data = FALSE;

		MTWF_PRINT ("\t===== RX rate statistics =====\n");
		band_idx = hc_get_hw_band_idx(pAd);
		rx_rate_rc = &pAd->tr_ctl.tr_cnt.rx_rate_rc;

			if (rx_rate_rc->total_mpdu_cnt != 0) {
				rx_rate_data = TRUE;
				MTWF_PRINT
					("\t[Band %d] Total MPDU:%u retry:%u(%d.%d%%)\n", band_idx,
					rx_rate_rc->total_mpdu_cnt, rx_rate_rc->total_retry_cnt,
					((rx_rate_rc->total_retry_cnt*1000)/rx_rate_rc->total_mpdu_cnt)/10,
					((rx_rate_rc->total_retry_cnt*1000)/rx_rate_rc->total_mpdu_cnt)%10);
			}
			if (rx_rate_data == FALSE)
				MTWF_PRINT ("\t[Band %d]\tN/A\n", band_idx);
			else {
				struct _rx_mod_cnt *mpdu_cnt = NULL;
				struct _rx_mod_cnt *retry_cnt = NULL;

				rx_rate_data = FALSE;
				for (bw_idx = 0 ; bw_idx < BW_160 ; bw_idx++) {
					mpdu_cnt = &pAd->tr_ctl.tr_cnt.rx_rate_rc.mpdu_cnt[bw_idx];
					retry_cnt = &pAd->tr_ctl.tr_cnt.rx_rate_rc.retry_cnt[bw_idx];

					if (bw_idx < 1) {
						/* CCK/OFDM only 20MHz bandwidth */
						for (i = 0; i < ARRAY_SIZE(mpdu_cnt->cck); i++) {
							if (mpdu_cnt->cck[i] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[CCK]\tMCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									 i, mpdu_cnt->cck[i],
									 ((mpdu_cnt->cck[i]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									 ((mpdu_cnt->cck[i]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									 retry_cnt->cck[i],
									 ((retry_cnt->cck[i]*1000)/mpdu_cnt->cck[i])/10,
									 ((retry_cnt->cck[i]*1000)/mpdu_cnt->cck[i])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
							"\t\t[CCK]\tN/A\n");
						else
							rx_rate_data = FALSE;

						for (i = 0; i < ARRAY_SIZE(mpdu_cnt->ofdm); i++) {
							if (mpdu_cnt->ofdm[i] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[OFDM]\tMCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									 i, mpdu_cnt->ofdm[i],
									 ((mpdu_cnt->ofdm[i]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									 ((mpdu_cnt->ofdm[i]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									 retry_cnt->ofdm[i],
									 ((retry_cnt->ofdm[i]*1000)/mpdu_cnt->ofdm[i])/10,
									 ((retry_cnt->ofdm[i]*1000)/mpdu_cnt->ofdm[i])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
							"\t\t[OFDM]\tN/A\n");
						else
							rx_rate_data = FALSE;
					}
					if (bw_idx < 2) {
						/* HT rates only 20/40Mhz bandwidth */
						for (i = 0; i < ARRAY_SIZE(mpdu_cnt->ht[0]); i++) {
							if (mpdu_cnt->ht[0][i] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[HT][%dMHz][LGI]\t", (INT32)(BIT(bw_idx)*20));
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									 i, mpdu_cnt->ht[0][i],
									 ((mpdu_cnt->ht[0][i]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									 ((mpdu_cnt->ht[0][i]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									 retry_cnt->ht[0][i],
									 ((retry_cnt->ht[0][i]*1000)/mpdu_cnt->ht[0][i])/10,
									 ((retry_cnt->ht[0][i]*1000)/mpdu_cnt->ht[0][i])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
							"\t\t[HT][%dMHz][LGI]\tN/A\n",
							(INT32)(BIT(bw_idx)*20));
						else
							rx_rate_data = FALSE;

						for (i = 0; i < ARRAY_SIZE(mpdu_cnt->ht[1]); i++) {
							if (mpdu_cnt->ht[1][i] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[HT][%dMHz][SGI]\t", (INT32)(BIT(bw_idx)*20));
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									 i, mpdu_cnt->ht[1][i],
									 ((mpdu_cnt->ht[1][i]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									 ((mpdu_cnt->ht[1][i]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									 retry_cnt->ht[1][i],
									 ((retry_cnt->ht[1][i]*1000)/mpdu_cnt->ht[1][i])/10,
									 ((retry_cnt->ht[1][i]*1000)/mpdu_cnt->ht[1][i])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								"\t\t[HT][%dMHz][SGI]\tN/A\n", (INT32)(BIT(bw_idx)*20));
						else
							rx_rate_data = FALSE;
					}
					for (i = 0; i < 4; i++) {
						UINT32 *rc = &mpdu_cnt->vht[0][i][0];
						UINT32 *retry_rc = &retry_cnt->vht[0][i][0];

						for (j = 0; j < ARRAY_SIZE(mpdu_cnt->vht[0][i]); j++) {
							if (rc[j] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[VHT][%dMHz][LGI]\t[NSS=%d]\t",
									  (INT32)(BIT(bw_idx)*20), i+1);
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u (%d.%d%%)\n",
									  j, rc[j],
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									  retry_rc[j],
									  ((retry_rc[j]*1000)/rc[j])/10,
									  ((retry_rc[j]*1000)/rc[j])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								 "\t\t[VHT][%dMHz][LGI]\t[NSS=%d]\tN/A\n",
								  (INT32)(BIT(bw_idx)*20), i+1);
						else
							rx_rate_data = FALSE;
					}
					for (i = 0; i < 4; i++) {
						UINT32 *rc = &mpdu_cnt->vht[1][i][0];
						UINT32 *retry_rc = &retry_cnt->vht[1][i][0];

						for (j = 0; j < ARRAY_SIZE(mpdu_cnt->vht[1][i]); j++) {
							if (rc[j] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[VHT][%dMHz][SGI]\t[NSS=%d]\t",
									  (INT32)(BIT(bw_idx)*20), i+1);
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									  j, rc[j],
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									  retry_rc[j],
									  ((retry_rc[j]*1000)/rc[j])/10,
									  ((retry_rc[j]*1000)/rc[j])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								 "\t\t[VHT][%dMHz][SGI]\t[NSS=%d]\tN/A\n",
								  (INT32)(BIT(bw_idx)*20), i+1);
						else
							rx_rate_data = FALSE;
					}
					for (i = 0; i < 4; i++) {
						UINT32 *rc = &mpdu_cnt->he[0][i][0];
						UINT32 *retry_rc = &retry_cnt->he[0][i][0];

						for (j = 0; j < ARRAY_SIZE(mpdu_cnt->he[0][i]); j++) {
							if (rc[j] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\n\t\t[HE][%dMHz][0.8us gi]\t[NSS=%d]\t",
									  (INT32)(BIT(bw_idx)*20), i+1);
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									  j, rc[j],
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									  retry_rc[j],
									  ((retry_rc[j]*1000)/rc[j])/10,
									  ((retry_rc[j]*1000)/rc[j])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								 "\t\t[HE][%dMHz][0.8us gi]\t[NSS=%d]\tN/A\n",
								  (INT32)(BIT(bw_idx)*20), i+1);
						else
							rx_rate_data = FALSE;
					}
					for (i = 0; i < 4; i++) {
						UINT32 *rc = &mpdu_cnt->he[1][i][0];
						UINT32 *retry_rc = &retry_cnt->he[1][i][0];

						for (j = 0; j < ARRAY_SIZE(mpdu_cnt->he[1][i]); j++) {
							if (rc[j] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[HE][%dMHz][1.6us gi]\t[NSS=%d]\t",
									  (INT32)(BIT(bw_idx)*20), i+1);
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									  j, rc[j],
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									  retry_rc[j],
									  ((retry_rc[j]*1000)/rc[j])/10,
									  ((retry_rc[j]*1000)/rc[j])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								 "\t\t[HE][%dMHz][1.6us gi]\t[NSS=%d]\tN/A\n",
								  (INT32)(BIT(bw_idx)*20), i+1);
						else
							rx_rate_data = FALSE;
					}
					for (i = 0; i < 4; i++) {
						UINT32 *rc = &mpdu_cnt->he[2][i][0];
						UINT32 *retry_rc = &retry_cnt->he[2][i][0];

						for (j = 0; j < ARRAY_SIZE(mpdu_cnt->he[2][i]); j++) {
							if (rc[j] != 0 && rx_rate_rc->total_mpdu_cnt != 0) {
								rx_rate_data = TRUE;
								MTWF_PRINT
									 ("\t\t[HE][%dMHz][3.2us gi]\t[NSS=%d]\t",
									  (INT32)(BIT(bw_idx)*20), i+1);
								MTWF_PRINT
									 ("MCS%d\t%u(%d.%d%%), retry:%u(%d.%d%%)\n",
									  j, rc[j],
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)/10,
									  ((rc[j]*1000)/rx_rate_rc->total_mpdu_cnt)%10,
									  retry_rc[j],
									  ((retry_rc[j]*1000)/rc[j])/10,
									  ((retry_rc[j]*1000)/rc[j])%10);
							}
						}
						if (rx_rate_data == FALSE)
							MTWF_DBG(NULL,
							DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
								 "\t\t[HE][%dMHz][3.2us gi]\t[NSS=%d]\tN/A\n",
								  (INT32)(BIT(bw_idx)*20), i+1);
						else
							rx_rate_data = FALSE;
					}
				}
				os_zero_mem(rx_rate_rc, sizeof(*rx_rate_rc));
			}
			MTWF_PRINT ("\n");
	}
	MTWF_PRINT ("\n\t==============================\n");
}

static VOID show_debug_info(RTMP_ADAPTER *pAd, UINT32 param0, UINT32 param1)
{
	MTWF_PRINT ("Current:\n");
	MTWF_PRINT ("\tDebug level: %d\n", debug_lvl);
	MTWF_PRINT ("\tDebug category: 0x%08x\n", debug_cat);
	debug_lvl = param0;
	debug_cat = param1;
	MTWF_PRINT ("After:\n");
	MTWF_PRINT ("\tDebug level: %d\n", debug_lvl);
	MTWF_PRINT ("\tDebug category: 0x%08x\n", debug_cat);
}

static VOID show_tpinfo_host_usage(RTMP_ADAPTER *pAd)
{
	MTWF_PRINT ("Host option usage:\n");
	MTWF_PRINT ("\t0: help\n");
	MTWF_PRINT ("\t1: debug info\n");
	MTWF_PRINT ("\t2: tx free notify host info\n");
	MTWF_PRINT ("\t3: wfdma info\n");
	MTWF_PRINT ("\t4: counter info\n");

	MTWF_PRINT ("Host debug info usage:\n");
	MTWF_PRINT ("\t0: DBG_LVL_OFF\n");
	MTWF_PRINT ("\t1: DBG_LVL_ERROR\n");
	MTWF_PRINT ("\t2: DBG_LVL_WARN\n");
	MTWF_PRINT ("\t3: DBG_LVL_INFO\n");
	MTWF_PRINT ("\t4: DBG_LVL_INFO\n");
}

static VOID show_pps_info(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

	MTWF_PRINT("\tif_idx = %d\n", get_dev_config_idx(pAd));
	MTWF_PRINT("\trx_to_os_cnt(read clear) = %u\n", tr_cnt->rx_to_os_cnt);
	tr_cnt->rx_to_os_cnt = 0;
}

VOID show_tpinfo_host(RTMP_ADAPTER *pAd, UINT32 option, UINT32 param0, UINT32 param1)
{
	switch (option) {
	case HOST_HELP:
		show_tpinfo_host_usage(pAd);
		break;
	case HOST_DBG_INFO:
		if (param0 == 0xFFFFFFFF && param1 == 0xFFFFFFFF) {
			MTWF_PRINT("%s: param0/param1 shall be set.\n", __func__);
			break;
		}
		show_debug_info(pAd, param0, param1);
		break;
	case COUNTER_INFO:
		show_counter_info(pAd, param0);
		break;
	case RX_PPS:
		show_pps_info(pAd);
		break;
	default:
		MTWF_PRINT("Not support(option=%d)\n", option);
		break;
	}
}

static VOID show_tpinfo_wacpu_usage(VOID)
{
	MTWF_PRINT ("Wacpu option usage:\n");
	MTWF_PRINT ("\t0: help\n");
	MTWF_PRINT ("\t1: debug info\n");
	MTWF_PRINT ("\t2: msdu drop info\n");
	MTWF_PRINT ("\t3: ac tail drop info\n");
	MTWF_PRINT ("\t4: bss table info\n");
	MTWF_PRINT ("\t5: sta record info\n");
	MTWF_PRINT ("\t6: tx free notify info\n");
	MTWF_PRINT ("\t7: ctxd info\n");
	MTWF_PRINT ("\t8: igmp info\n");
	MTWF_PRINT ("\t9: igmp white list info\n");
	MTWF_PRINT ("\t10: sdo info\n");
}

static VOID show_tpinfo_wacpu(RTMP_ADAPTER *pAd, UINT32 option)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support)
		MtUniCmdFwLog2Host(pAd, HOST2CR4, ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, 1, 2);

	switch (option) {
	case WACPU_HELP:
		show_tpinfo_wacpu_usage();
		break;
	case WACPU_DBG_INFO:
		MTWF_PRINT("%s: not support option = %d\n", __func__, option);
		break;
	case MSDU_DROP_INFO:
		MtCmdCr4Query(pAd, 0x16, 0, 0);
		break;
	case AC_TAIL_DROP_INFO:
		MtCmdCr4Query(pAd, 0x17, 0, 0);
		break;
	case BSS_TABLE_INFO:
		MtCmdCr4Query(pAd, 0x20, 0, 0);
		break;
	case STAREC_INFO:
		MtCmdCr4Query(pAd, 0x21, 0, 0);
		break;
	case TX_FREE_NOTIFY_WACPU_INFO:
		MtCmdCr4Query(pAd, 0x19, 0, 0);
		break;
	case CTXD_INFO:
		MtCmdCr4Query(pAd, 0x18, 0, 0);
		break;
	case IGMP_INFO:
		MtCmdCr4Query(pAd, 0x1a, 0, 0);
		break;
	case IGMP_WHITE_LIST_INFO:
		MtCmdCr4Query(pAd, 0x1b, 0, 0);
		break;
	case SDO_INFO:
		MtCmdCr4Query(pAd, 0x22, 0, 0);
		break;
	default:
		MTWF_PRINT("%s: unknown option = %d\n", __func__, option);
		break;
	}
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwLog2Host(pAd, HOST2CR4, ENUM_CMD_FW_LOG_2_HOST_CTRL_2_UART);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, 1, 0);
}

static VOID show_tpinfo_wocpu_usage(VOID)
{
	MTWF_PRINT ("Wocpu option usage:\n");
	MTWF_PRINT ("\t0: help\n");
	MTWF_PRINT ("\t1: debug info\n");
	MTWF_PRINT ("\t2: dev info\n");
	MTWF_PRINT ("\t3: bss info\n");
	MTWF_PRINT ("\t4: sta rec info\n");
	MTWF_PRINT ("\t5: ba info\n");
	MTWF_PRINT ("\t6: fbcmd ring info\n");
	MTWF_PRINT ("\t7: rx status\n");

	MTWF_PRINT ("Wocpu debug level usage:\n");
	MTWF_PRINT ("\t0: WO_DBG_ALERT\n");
	MTWF_PRINT ("\t1: WO_DBG_ERR\n");
	MTWF_PRINT ("\t2: WO_DBG_WARN\n");
	MTWF_PRINT ("\t3: WO_DBG_INFO\n");
	MTWF_PRINT ("\t4: WO_DBG_DEBUG\n");

	MTWF_PRINT ("Wocpu debug category usage:\n");
	MTWF_PRINT ("\t0: clear all category\n");
	MTWF_PRINT ("\t1: WO_DBGM_HAL\n");
	MTWF_PRINT ("\t2: WO_DBGM_QM\n");
	MTWF_PRINT ("\t4: WO_DBGM_RRO\n");
	MTWF_PRINT ("\t8: WO_DBGM_RXM\n");
	MTWF_PRINT ("\t16: WO_DBGM_INTR\n");
	MTWF_PRINT ("\t32: WO_DBGM_RING\n");
	MTWF_PRINT ("\t64: WO_DBGM_FB_CMD\n");
	MTWF_PRINT ("\t128: WO_DBGM_FW_CMD\n");
	MTWF_PRINT ("\t256: WO_DBGM_MIOD\n");
	MTWF_PRINT ("\t512: WO_DBGM_STA_INFO\n");
	MTWF_PRINT ("\t1024: WO_DBGM_BSS\n");
	MTWF_PRINT ("\t2048: WO_DBGM_DEV\n");
}

static VOID show_tpinfo_wocpu(RTMP_ADAPTER *pAd, UINT32 option, UINT32 param0, UINT32 param1)
{
	/* For wo level and category */
	debug_lvl = 0;
	debug_cat = 0xFFFFFFFF;
	switch (option) {
	case WOCPU_HELP:
		show_tpinfo_wocpu_usage();
		break;
	case WOCPU_DBG_INFO:
		if (param0 == 0xFFFFFFFF && param1 == 0xFFFFFFFF) {
			MTWF_PRINT("%s: param0/param1 shall be set.\n", __func__);
			break;
		}
		show_debug_info(pAd, param0, param1);
		mt_cmd_wo_query(pAd, WO_CMD_DBG_INFO, debug_lvl, debug_cat);
		break;
	case WOCPU_DEV_INFO:
		mt_cmd_wo_query(pAd, WO_CMD_DEV_INFO_DUMP, param0, 0xFFFFFFFF);
		break;
	case WOCPU_BSS_INFO:
		mt_cmd_wo_query(pAd, WO_CMD_BSS_INFO_DUMP, param0, 0xFFFFFFFF);
		break;
	case WOCPU_STA_REC:
		mt_cmd_wo_query(pAd, WO_CMD_STA_REC_DUMP, param0, 0xFFFFFFFF);
		break;
	case WOCPU_BA_INFO:
		mt_cmd_wo_query(pAd, WO_CMD_BA_INFO_DUMP, param0, 0xFFFFFFFF);
		break;
	case WOCPU_FBCMD_Q_INFO:
		mt_cmd_wo_query(pAd, WO_CMD_FBCMD_Q_DUMP, param0, 0xFFFFFFFF);
		break;
	case WOCPU_RX_STAT:
		mt_cmd_wo_query(pAd, WO_CMD_WED_RX_STAT, param0, 0xFFFFFFFF);
		break;
	default:
		MTWF_PRINT("unknown option = %d\n", option);
		break;
	}
}

INT show_tpinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cpu = 0, option = 0;
	UINT32 param0 = 0xFFFFFFFF, param1 = 0xFFFFFFFF;
	CHAR *param;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	param = rstrtok(arg, "-");

	if (param) {
		cpu = os_str_tol(param, 0, 10);
		param = rstrtok(NULL, "-");

		if (param) {
			option = os_str_toul(param, 0, 10);
			param = rstrtok(NULL, "-");

			if (param) {
				param0 = os_str_tol(param, 0, 10);
				param = rstrtok(NULL, "-");
				if (param)
					param1 = os_str_tol(param, 0, 10);
			}
		}
	}

	if (cpu == 0)
		show_tpinfo_host(pAd, option, param0, param1);
	else if (cpu == 1)
		show_tpinfo_wacpu(pAd, option);
	else if (cpu == 2)
		show_tpinfo_wocpu(pAd, option, param0, param1);
	else
		goto err;

	return TRUE;
err:
	MTWF_PRINT
		("\tiwpriv $(inf_name) show tpinfo=[cpu]-[option]-[param0]-[param1]\n");
	MTWF_PRINT
		("\t[cpu] 0: host, 1: wacpu, 2: wocpu\n");
	MTWF_PRINT
		("\t[param0] may be debug info or wlan_idx/bss_idx/dev_idx\n"
		 "\t\t- option1(debug info): debug level\n"
		 "\t\t- cpu2(wocpu): wlan_idx/bss_idx/dev_idx\n");
	MTWF_PRINT
		("\t[param1]\n"
		 "\t\t- option1(debug info): debug category\n");

	return TRUE;
}

INT show_mlmeinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef LINUX
	UCHAR que_idx = 0;
	ULONG tmp_idx;
	MLME_QUEUE *pQueue = NULL;
	MLME_QUEUE_ELEM *tmpElem = NULL;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	que_idx = (UCHAR)os_str_tol(arg, 0, 10);

	switch (que_idx) {
	case 0:
		pQueue = &pAd->Mlme.Queue;
		break;
#ifdef MLME_MULTI_QUEUE_SUPPORT
	case 1:
		pQueue = &pAd->Mlme.HPQueue;
		break;
	case 2:
		pQueue = &pAd->Mlme.LPQueue;
		break;
#endif
	default:
		MTWF_PRINT("No mlme queue matched!, que_idx = %d\n", que_idx);
		goto err;
	}

	NdisAcquireSpinLock(&(pQueue->Lock));
	if (pQueue->Num == 0 || list_empty(&pQueue->EntryInuseHead)) {
		NdisReleaseSpinLock(&(pQueue->Lock));
		MTWF_PRINT("MlmeQue(%d) is empty!\n", que_idx);
		return TRUE;
	}
#ifdef MLME_MULTI_QUEUE_SUPPORT
	MTWF_PRINT("MlmeQueRation-%d\n", pQueue->Ration);
#endif
	tmp_idx = 0;
	list_for_each_entry(tmpElem, &pQueue->EntryInuseHead, list) {
		MTWF_PRINT("IDX(%ld): Machine/MsgType = %ld/%ld, Wdev/Wcid(%d/%d)\n",
			tmp_idx, tmpElem->Machine, tmpElem->MsgType, tmpElem->wdev->wdev_idx, tmpElem->Wcid);
		tmp_idx++;
	}
	NdisReleaseSpinLock(&(pQueue->Lock));

	return TRUE;

err:
	MTWF_PRINT("\tiwpriv $(inf_name) show mlmeinfo=[que_idx]\n");
	MTWF_PRINT("\t[que_idx] 0: NormalQue, 1: HighQue, 2: LowQue\n");
#endif /* LINUX */
	return TRUE;
}

INT show_trinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);
	UINT8 num_of_rx_ring = hif_get_rx_res_num(pAd->hdev_ctrl);
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	struct hif_pci_tx_ring *tx_ring = NULL;

	MTWF_PRINT
			("=================================================\n");

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd) || IS_HWIFI_INF(pAd)) {
		UINT32 *tbase = NULL, *tcnt = NULL, *tcidx = NULL,
			*tdidx = NULL, *Buf_TXRingOps = NULL;
		UINT32 *rbase = NULL, *rcnt = NULL, *rcidx = NULL,
			*rdidx = NULL, *Buf_RXRingOps = NULL;
		INT idx;
		INT TxHwRingNum = num_of_tx_ring;
		INT RxHwRingNum = num_of_rx_ring;

#ifdef ERR_RECOVERY

		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			return TRUE;

#endif /* ERR_RECOVERY */
		os_alloc_mem(NULL, (UCHAR **)&Buf_TXRingOps, 4*num_of_tx_ring*sizeof(UINT32));
		if (Buf_TXRingOps == NULL)
			return FALSE;

		os_alloc_mem(NULL, (UCHAR **)&Buf_RXRingOps, 4*num_of_rx_ring*sizeof(UINT32));
		if (Buf_RXRingOps == NULL) {
			os_free_mem(Buf_TXRingOps);
			return FALSE;
		}

		os_zero_mem(Buf_TXRingOps, 4*num_of_tx_ring*sizeof(UINT32));
		os_zero_mem(Buf_RXRingOps, 4*num_of_rx_ring*sizeof(UINT32));
		tbase = &Buf_TXRingOps[0];
		tcnt = &Buf_TXRingOps[num_of_tx_ring];
		tcidx = &Buf_TXRingOps[num_of_tx_ring*2];

		tdidx = &Buf_TXRingOps[num_of_tx_ring*3];
		rbase = &Buf_RXRingOps[0];
		rcnt = &Buf_RXRingOps[num_of_rx_ring];
		rcidx = &Buf_RXRingOps[num_of_rx_ring*2];

		rdidx = &Buf_RXRingOps[num_of_rx_ring*3];

		for (idx = 0; idx < TxHwRingNum; idx++) {
			tx_ring = pci_get_tx_ring_by_ridx(hif, idx);
			HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_desc_base, &tbase[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_cnt_addr, &tcnt[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_didx_addr, &tdidx[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_cidx_addr, &tcidx[idx]);
		}

		for (idx = 0; idx < RxHwRingNum; idx++) {
			rx_ring = pci_get_rx_ring_by_ridx(hif, idx);
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_desc_base, &rbase[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_cnt_addr, &rcnt[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, &rcidx[idx]);
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rdidx[idx]);
		}

		MTWF_PRINT ("TxRing Configuration\n");
		MTWF_PRINT ("%4s %8s %8s %10s %6s %6s %6s %6s %6s\n",
				"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt", "FreeCnt");

		for (idx = 0; idx < TxHwRingNum; idx++) {
			UINT32 queue_cnt, free_cnt;

			tx_ring = pci_get_tx_ring_by_ridx(hif, idx);
			queue_cnt = (tcidx[idx] >= tdidx[idx]) ? (tcidx[idx] - tdidx[idx]) : (tcidx[idx] - tdidx[idx] + tcnt[idx]);
			free_cnt = hif_get_tx_resource_free_num(pAd->hdev_ctrl, idx);

			MTWF_PRINT
				("%4d %8s %8x %10x %6x %6x %6x %6x %6x\n",
				idx,
				(tx_ring->ring_attr == HIF_TX_DATA) ? "DATA" :
				(tx_ring->ring_attr == HIF_TX_CMD) ? "CMD" :
				(tx_ring->ring_attr == HIF_TX_CMD_WM) ? "CMD_WM" :
				(tx_ring->ring_attr == HIF_TX_FWDL) ? "FWDL" : "UN",
				tx_ring->hw_desc_base, tbase[idx],
				tcnt[idx], tcidx[idx], tdidx[idx], queue_cnt, free_cnt);
		}

		MTWF_PRINT ("RxRing Configuration\n");
		MTWF_PRINT ("%4s %8s %8s %10s %6s %6s %6s %6s\n",
				"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");

		for (idx = 0; idx < RxHwRingNum; idx++) {
			UINT32 queue_cnt;

			rx_ring = pci_get_rx_ring_by_ridx(hif, idx);
			queue_cnt = (rdidx[idx] > rcidx[idx]) ? (rdidx[idx] - rcidx[idx] - 1) : (rdidx[idx] - rcidx[idx] + rcnt[idx] - 1);

			MTWF_PRINT
				("%4d %8s %8x %10x %6x %6x %6x %6x\n",
				idx,
				(rx_ring->ring_attr == HIF_RX_DATA) ? "DATA" :
				(rx_ring->ring_attr == HIF_RX_EVENT) ? "EVENT" : "UN",
				rx_ring->hw_desc_base, rbase[idx],
				rcnt[idx], rcidx[idx], rdidx[idx], queue_cnt);
		}
		os_free_mem(Buf_TXRingOps);
		os_free_mem(Buf_RXRingOps);
	}

#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */

	if (chip_dbg->show_dma_info)
		chip_dbg->show_dma_info(pAd->hdev_ctrl);

	return TRUE;
}

INT show_swqinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

	if (qm_ops->dump_all_sw_queue) {
		MTWF_PRINT
			 ("%s: show_swqinfo\n", __func__);

		qm_ops->dump_all_sw_queue(pAd, arg);
	}

	return TRUE;
}

INT show_txqinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	CHAR *param;
	UINT16 wcid = 0;
	UCHAR q_idx = 0;
	enum PACKET_TYPE pkt_type = 0;

	MTWF_PRINT
			 ("%s::param = %s\n", __func__, arg);

	if (arg == NULL)
		goto error;

	param = rstrtok(arg, ":");

	if (param != NULL)
		wcid = os_str_tol(param, 0, 10);
	else
		goto error;

	param = rstrtok(NULL, ":");

	if (param != NULL)
		pkt_type = os_str_tol(param, 0, 10);
	else
		goto error;

	param = rstrtok(NULL, ":");

	if (param != NULL)
		q_idx = os_str_tol(param, 0, 10);
	else
		goto error;

	if (qm_ops->sta_dump_queue) {
		MTWF_PRINT
			 ("%s::wcid = %d, pkt_type = %d, q_idx = %d\n",
			  __func__, wcid, pkt_type, q_idx);

		qm_ops->sta_dump_queue(pAd, wcid, pkt_type, q_idx);
	}

	return TRUE;

error:

	return 0;
}

#ifdef DOT11_HE_AX
INT show_bsscolor_proc(RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	show_bss_color_info(ad);
	return TRUE;
}
#endif

#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT
INT	Show_WpsManufacturer_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	int ret;
	if (IfIdx >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			" invalid IfIdx=%d.\n", IfIdx);
		return 0;
	}

	pStaCfg = &pAd->StaCfg[IfIdx];

	ret = snprintf(pBuf, BufLen, "\tManufacturer = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.Manufacturer);
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return 1;
	}
	return 0;
}

INT	Show_WpsModelName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	int ret;
	if (IfIdx >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			" invalid IfIdx=%d.\n", IfIdx);
		return 0;
	}

	pStaCfg = &pAd->StaCfg[IfIdx];

	ret = snprintf(pBuf, BufLen, "\tModelName = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.ModelName);
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return 1;
	}
	return 0;
}

INT	Show_WpsDeviceName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	int ret;
	if (IfIdx >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			" invalid IfIdx=%d.\n", IfIdx);
		return 0;
	}

	pStaCfg = &pAd->StaCfg[IfIdx];

	ret = snprintf(pBuf, BufLen, "\tDeviceName = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.DeviceName);
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return 1;
	}
	return 0;
}

INT	Show_WpsModelNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	int ret;
	if (IfIdx >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			" invalid IfIdx=%d.\n", IfIdx);
		return 0;
	}

	pStaCfg = &pAd->StaCfg[IfIdx];

	ret = snprintf(pBuf, BufLen, "\tModelNumber = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.ModelNumber);
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return 1;
	}
	return 0;
}

INT	Show_WpsSerialNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32       IfIdx = pObj->ioctl_if;
	PSTA_ADMIN_CONFIG pStaCfg;
	int ret;
	if (IfIdx >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			" invalid IfIdx=%d.\n", IfIdx);
		return 0;
	}

	pStaCfg = &pAd->StaCfg[IfIdx];

	ret = snprintf(pBuf, BufLen, "\tSerialNumber = %s", pStaCfg->wdev.WscControl.RegData.SelfInfo.SerialNumber);
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		return 1;
	}
	return 0;
}
#endif /* WSC_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef SINGLE_SKU
INT	Show_ModuleTxpower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\tModuleTxpower = %d", pAd->CommonCfg.ModuleTxpower);
	return 0;
}
#endif /* SINGLE_SKU */

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN bConnect = FALSE;
	struct wifi_dev *wdev = NULL;
#ifdef MAC_REPEATER_SUPPORT
	MBSS_TO_CLI_LINK_MAP_T *pMbssToCliLinkMap = NULL;
	INT	MbssIdx;
#endif

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "==>\n");

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	wdev = &pAd->StaCfg[ifIndex].wdev;
	if (!wdev)
		return FALSE;
	MTWF_PRINT ("=============================================================\n");

	if (((GetAssociatedAPByWdev(pAd, wdev)) != NULL) && (pAd->StaCfg[ifIndex].SsidLen != 0)) {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);
			STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, i);

			if (IS_ENTRY_PEER_AP(pEntry)
				&& (pEntry->Sst == SST_ASSOC)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				if (pEntry->wdev == &pAd->StaCfg[ifIndex].wdev) {
					MTWF_PRINT
							 ("ApCli%d         Connected AP : "MACSTR"   SSID:%s\n",
							  ifIndex, MAC2STR(pEntry->Addr), pAd->StaCfg[ifIndex].Ssid);
					bConnect = TRUE;
#ifdef MWDS

					if (pAd->StaCfg[ifIndex].MlmeAux.bSupportMWDS)
						MTWF_PRINT ("MWDSCap : YES\n");
					else
						MTWF_PRINT ("MWDSCap : NO\n");

#endif /* MWDS */
				}
			}

#ifdef MAC_REPEATER_SUPPORT
			else if (IS_ENTRY_REPEATER(pEntry)
					 && (pEntry->Sst == SST_ASSOC)
					 && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				if (pEntry->wdev == &pAd->StaCfg[ifIndex].wdev) {
					MTWF_PRINT
							 ("Rept[wcid=%-3d] Connected AP : "MACSTR"   SSID:%s\n",
							  i, MAC2STR(pEntry->Addr), pAd->StaCfg[ifIndex].Ssid);
					bConnect = TRUE;
				}
			}

#endif
		}

		if (!bConnect)
			MTWF_PRINT ("ApCli%d Connected AP : Disconnect\n", ifIndex);
	} else
		MTWF_PRINT ("ApCli%d Connected AP : Disconnect\n", ifIndex);

#ifdef MAC_REPEATER_SUPPORT
	MTWF_PRINT ("ApCli%d CliLinkMap ra:", ifIndex);

	for (MbssIdx = 0; MbssIdx < pAd->ApCfg.BssidNum; MbssIdx++) {
		pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];

		if (pMbssToCliLinkMap->cli_link_wdev == &pAd->StaCfg[ifIndex].wdev)
			MTWF_PRINT ("%d ", MbssIdx);
	}

	MTWF_PRINT ("\n\r");
	MTWF_PRINT ("Ignore repeater MAC address\n\r");

	for (i = 0; i < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; i++) {
		INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[i];

		if (pEntry->bInsert)
			MTWF_PRINT ("[%d]"MACSTR"\n\r", i,
					 MAC2STR(pEntry->MacAddr));
	}

#endif
	MTWF_PRINT ("\n\r");
	MTWF_PRINT ("=============================================================\n");
	return TRUE;
}
#endif/*APCLI_SUPPORT*/

INT32 getLegacyOFDMMCSIndex(UINT8 MCS)
{
	INT32 mcs_index = MCS;

	if (MCS == 0xb)
		mcs_index = 0;
	else if (MCS == 0xf)
		mcs_index = 1;
	else if (MCS == 0xa)
		mcs_index = 2;
	else if (MCS == 0xe)
		mcs_index = 3;
	else if (MCS == 0x9)
		mcs_index = 4;
	else if (MCS == 0xd)
		mcs_index = 5;
	else if (MCS == 0x8)
		mcs_index = 6;
	else if (MCS == 0xc)
		mcs_index = 7;

	return mcs_index;
}

void  getRate(union _HTTRANSMIT_SETTING HTSetting, ULONG *fLastTxRxRate)

{
	UINT8					Antenna = 0;
	UINT8					MCS = HTSetting.field.MCS;
	int rate_count = sizeof(MCSMappingRateTable) / sizeof(int);
	int rate_index = 0;
	int value = 0;

#ifdef DOT11_VHT_AC
	if (HTSetting.field.MODE >= MODE_VHT) {
		MCS = HTSetting.field.MCS & 0xf;
		Antenna = (HTSetting.field.MCS >> 4) + 1;

		if (HTSetting.field.BW == BW_20) {
			rate_index = 112 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_40) {
			rate_index = 152 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_80) {
			rate_index = 192 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_160) {
			rate_index = 232 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		}
		if (MCS > MCS_9)
			rate_index = rate_index - 2;
	} else {
#endif /* DOT11_VHT_AC */
		if (HTSetting.field.MODE >= MODE_HTMIX) {
			MCS = HTSetting.field.MCS;

			if ((HTSetting.field.MODE == MODE_HTMIX)
				|| (HTSetting.field.MODE == MODE_HTGREENFIELD))
				Antenna = (MCS >> 3) + 1;

			/* map back to 1SS MCS , multiply by antenna numbers later */
			if (MCS > 7)
				MCS %= 8;

			rate_index = 16 + ((UCHAR)HTSetting.field.BW * 24) + ((UCHAR)HTSetting.field.ShortGI * 48) + ((UCHAR)MCS);
		} else {
			if (HTSetting.field.MODE == MODE_OFDM)
				rate_index = getLegacyOFDMMCSIndex(HTSetting.field.MCS) + 4;
			else if (HTSetting.field.MODE == MODE_CCK)
				rate_index = (UCHAR)(HTSetting.field.MCS);
		}
	}

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count - 1;

	if (HTSetting.field.MODE < MODE_VHT)
		value = (MCSMappingRateTable[rate_index] * 5) / 10;
	else
		value =  MCSMappingRateTable[rate_index];

	if (MCS > MCS_9)
		value = (value * 10) / 8;

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_VHT)
		value *= Antenna;

	*fLastTxRxRate = (ULONG)value;
	return;
}

#ifdef DOT11_HE_AX
void  get_rate_he(UINT8 mcs, UINT8 bw, UINT8 nss, UINT8 dcm, ULONG *last_tx_rate)

{
	ULONG value = 0;

	if (nss == 0) {
		nss = 1;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"NSS is invalid!\n");
	}

	if (mcs >= MAX_NUM_HE_MCS_ENTRIES)
		mcs = MAX_NUM_HE_MCS_ENTRIES - 1;

	if (nss > MAX_NUM_HE_SPATIAL_STREAMS)
		nss = MAX_NUM_HE_SPATIAL_STREAMS;

	if (bw >= MAX_NUM_HE_BANDWIDTHS)
		bw = MAX_NUM_HE_BANDWIDTHS - 1;

	nss--;

	value = he_mcs_phyrate_mapping_table[bw][nss][mcs];
	/*In spec data rate when DCM =1 is half of the data rate when DCM = 0*/
	if (dcm && value)
		value = value / 2 ;

	*last_tx_rate = (ULONG)value;

	return;
}

#endif

#ifdef DOT11_EHT_BE
void  get_rate_eht(UINT8 mcs, UINT8 bw, UINT8 nss, UINT8 dcm, ULONG *last_tx_rate)

{
	ULONG value = 0;

	if (nss == 0) {
		nss = 1;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"NSS is invalid!\n");
	}

	if (mcs >= MAX_NUM_EHT_MCS_ENTRIES)
		mcs = MAX_NUM_EHT_MCS_ENTRIES - 1;

	if (nss > MAX_NUM_EHT_SPATIAL_STREAMS)
		nss = MAX_NUM_EHT_SPATIAL_STREAMS;

	if (bw >= MAX_NUM_EHT_BANDWIDTHS)
		bw = MAX_NUM_EHT_BANDWIDTHS - 1;

	value = eht_mcs_phyrate_mapping_table[bw][mcs];

	*last_tx_rate = (ULONG)(value * nss);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				"BW: %d MCS: %d Antenna num: %d  Rate = %llu\n",
				bw, mcs, nss, *last_tx_rate);
}

#endif

#ifdef MT_MAC
INT show_txvinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_txv_info)
		return chip_dbg->show_txv_info(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}
#endif

#ifdef TXBF_SUPPORT

/*
	Set_InvTxBfTag_Proc - Invalidate BF Profile Tags
		usage: "iwpriv ra0 set InvTxBfTag=n"
		Reset Valid bit and zero out MAC address of each profile. The next profile will be stored in profile 0
*/
INT	Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}

#ifdef MT_MAC
/*
	Set_ETxBfCodebook_Proc - Set ETxBf Codebook
	usage: iwpriv ra0 set ETxBfCodebook=0 to 3
*/
INT Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_PRINT("%s(): Not support for HIF_MT yet!\n",
				 __func__);
		return FALSE;
	}

	if (t > 3) {
		MTWF_PRINT("%s: value > 3!\n", __func__);
		return FALSE;
	}

	return TRUE;
}

/*
	Set_ETxBfCoefficient_Proc - Set ETxBf Coefficient
		usage: iwpriv ra0 set ETxBfCoefficient=0 to 3
*/
INT Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_PRINT("%s(): Not support for HIF_MT yet!\n",
				 __func__);
		return FALSE;
	}

	if (t > 3) {
		MTWF_PRINT("%s: value > 3!\n", __func__);
		return FALSE;
	}

	return TRUE;
}

/*
	Set_ETxBfGrouping_Proc - Set ETxBf Grouping
		usage: iwpriv ra0 set ETxBfGrouping=0 to 2
*/
INT Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_PRINT("%s(): Not support for HIF_MT yet!\n",
				 __func__);
		return FALSE;
	}

	if (t > 2) {
		MTWF_PRINT("%s: value > 2!\n", __func__);
		return FALSE;
	}

	return TRUE;
}
#endif /* MT_MAC */

/*
	Set_ETxBfNoncompress_Proc - Set ETxBf Noncompress option
		usage: iwpriv ra0 set ETxBfNoncompress=0 or 1
*/
INT Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);

	if (t > 1) {
		MTWF_PRINT("%s: value > 1!\n", __func__);
		return FALSE;
	}

	pAd->CommonCfg.ETxBfNoncompress = t;
	return TRUE;
}

/*
	Set_ETxBfIncapable_Proc - Set ETxBf Incapable option
		usage: iwpriv ra0 set ETxBfIncapable=0 or 1
*/
INT Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return FALSE;

	if (t > 1)
		return FALSE;

	pAd->CommonCfg.ETxBfIncapable = t;
#ifdef MT_MAC
	mt_WrapSetETxBFCap(pAd, wdev, &pAd->CommonCfg.HtCapability.TxBFCap);
#endif /* MT_MAC */
	return TRUE;
}

/*
	Set_ITxBfDivCal_Proc - Calculate ITxBf Divider Calibration parameters
	usage: iwpriv ra0 set ITxBfDivCal=dd
			0=>display calibration parameters
			1=>update EEPROM values
			2=>update BBP R176
			10=>display calibration parameters and dump capture data
			11=>Skip divider calibration, just capture and dump capture data
*/
INT	Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int calFunction;
	UINT32 value, value1 = 0, restore_value = 0, loop = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	/* backup mac 1004 value */
	RTMP_IO_READ32(pAd->hdev_ctrl, 0x1004, &restore_value);
	/* Backup the original RTS retry count and then set to 0 */
	/* RTMP_IO_READ32(pAd->hdev_ctrl, 0x1344, &pAd->rts_tx_retry_num); */
	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1004, value);
	/* set RTS retry count = 0 */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1344, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x1200, &value);
		value &= 0x1;
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x2130, &value1);
		MTWF_PRINT("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n",
				 __func__);
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300) {
		MTWF_PRINT
				 ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __func__);
		return FALSE;
	}

	calFunction = os_str_tol(arg, 0, 10);
	ops->fITxBfDividerCalibration(pAd, calFunction, 0, NULL);
	/* enable TX/RX */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1004, restore_value);
	/* Restore RTS retry count */
	/* RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1344, pAd->rts_tx_retry_num); */
	return TRUE;
}


#ifdef MT_MAC
/*
	Set_ETxBfEnCond_Proc - enable/disable ETxBF
	usage: iwpriv ra0 set ETxBfEnCond=dd
		0=>disable, 1=>enable
	Note: After use this command, need to re-run apStartup()/LinkUp() operations to sync all status.
		  If ETxBfIncapable!=0 then we don't need to reassociate.
*/
INT	Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 i;
	UCHAR ucETxBfEn;
	UCHAR ucStatus = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;

	ucETxBfEn = os_str_tol(arg, 0, 10);

	if (ucETxBfEn > 1)
		return FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_NONE(pEntry)) {
			wdev = pEntry->wdev;
			TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
			TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;

#ifdef ANTENNA_CONTROL_SUPPORT
			{
				if (pAd->bAntennaSetAPEnable) {
					TxBfInfo.ucTxPathNum = pAd->TxStream;
					TxBfInfo.ucRxPathNum = pAd->RxStream;
				}
			}
#endif /* ANTENNA_CONTROL_SUPPORT */

			TxBfInfo.ucPhyMode   = wdev->PhyMode;
			TxBfInfo.u2Channel   = wdev->channel;
			TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
			TxBfInfo.cmmCfgETxBfIncapable = pAd->CommonCfg.ETxBfIncapable;
			TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
			TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
			TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] +  i * pAd->mac_ctrl.wtbl_entry_size[0];
			TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] +  i * pAd->mac_ctrl.wtbl_entry_size[1];
			TxBfInfo.ucETxBfTxEn = ucETxBfEn;
			TxBfInfo.ucITxBfTxEn  = FALSE;
			TxBfInfo.u2Wcid = i;
			TxBfInfo.ucBW = pEntry->HTPhyMode.field.BW;
			TxBfInfo.ucNDPARate = 2; /* MCS2 */
			ucStatus = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return ucStatus;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_etxbf_en_cond(RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 value)
{
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, inf_idx, inf_type);
	UINT16 i;
	UCHAR ucStatus = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;

	if (!wdev)
		return FALSE;

	if (value > 1)
		return FALSE;
	wlan_config_set_etxbf(wdev, value);
	pAd->CommonCfg.ETxBfEnCond = 0;
	pAd->CommonCfg.ETxBfEnCond |= value;
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_NONE(pEntry)) {
			wdev = pEntry->wdev;
			TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
			TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;

#ifdef ANTENNA_CONTROL_SUPPORT
			{
				if (pAd->bAntennaSetAPEnable) {
					TxBfInfo.ucTxPathNum = pAd->TxStream;
					TxBfInfo.ucRxPathNum = pAd->RxStream;
				}
			}
#endif /* ANTENNA_CONTROL_SUPPORT */

			TxBfInfo.ucPhyMode   = wdev->PhyMode;
			TxBfInfo.u2Channel   = wdev->channel;
			TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
			TxBfInfo.cmmCfgETxBfIncapable = pAd->CommonCfg.ETxBfIncapable;
			TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
			TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
			TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] +  i * pAd->mac_ctrl.wtbl_entry_size[0];
			TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] +  i * pAd->mac_ctrl.wtbl_entry_size[1];
			TxBfInfo.ucETxBfTxEn = value;
			TxBfInfo.ucITxBfTxEn  = FALSE;
			TxBfInfo.u2Wcid = i;
			TxBfInfo.ucBW = pEntry->HTPhyMode.field.BW;
			TxBfInfo.ucNDPARate = 2; /* MCS2 */
			ucStatus = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	MTWF_PRINT("mtk_cfg80211_set_etxbf_en_cond::(etxbf en=%d)\n", value);
	return ucStatus;
}
#endif

INT set_txbf_stop_report_poll_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	para[6];

	para[0] = os_str_tol(arg, 0, 10);

	if (para[0] != 0 && para[0] != 1) {
		MTWF_PRINT("Wrong format!\n");
		MTWF_PRINT("iwpirv ra0 set TxBfStopReportPoll=N\n");
		MTWF_PRINT("N=1: Stop Rpt Poll\nN=0: Re-enable Rpt Poll\n");
		return FALSE;
	}

	para[1] = '\0';

	return txbf_config(pAd, BF_CONFIG_TYPE_STOP_REPORT_POLL, &para[0]);
}

INT Set_TxBfTxApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	 *value;
	UCHAR	Input[5];
	INT		i;
	UINT16  u2WlanIdx;
	BOOLEAN fgETxBf, fgITxBf, fgMuTxBf, fgPhaseCali;
	BOOLEAN fgStatus = TRUE;

	if (strlen(arg) != 14)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	u2WlanIdx   = Input[0];
	fgETxBf     = Input[1];
	fgITxBf     = Input[2];
	fgMuTxBf    = Input[3];
	fgPhaseCali = Input[4];
	CmdTxBfTxApplyCtrl(pAd,
					   u2WlanIdx,
					   fgETxBf,
					   fgITxBf,
					   fgMuTxBf,
					   fgPhaseCali);
	return fgStatus;
}

INT	Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[7];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucSu_Mu, ucMuNum, ucWlanId[4];
	UINT32          u4SndInterval;

#ifdef CONFIG_WLAN_SERVICE
	UCHAR			control_band_idx;
	struct service_test *serv_test;
#endif /*CONFIG_WLAN_SERVICE*/

	if (strlen(arg) != 20)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu       = Input[0];
	ucMuNum       = Input[1];
	u4SndInterval = (UINT32) Input[2];
	u4SndInterval = u4SndInterval << 2;
	ucWlanId[0]   = Input[3];
	ucWlanId[1]   = Input[4];
	ucWlanId[2]   = Input[5];
	ucWlanId[3]   = Input[6];

	if (pAd->Antenna.field.TxPath <= 1)
		return FALSE;

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable &&
			(pAd->Antenna.field.TxPath <= 1)) {
			return FALSE;
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef CONFIG_WLAN_SERVICE
	serv_test = (struct service_test *)(pAd->serv.serv_handle);
	control_band_idx = serv_test->ctrl_band_idx;

	if (ATE_ON(pAd)) {
		/* Enable Tx MAC HW before trigger sounding */
		/* MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx); */
	}
#endif /*CONFIG_WLAN_SERVICE*/

	if (mt_Trigger_Sounding_Packet(pAd,
								   TRUE,
								   u4SndInterval,
								   ucSu_Mu,
								   ucMuNum,
								   ucWlanId) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_Stop_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;

	if (mt_Trigger_Sounding_Packet(pAd,
								   FALSE,
								   0,
								   0,
								   0,
								   NULL) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemAlloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[2];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucSu_Mu;
	UINT16          u2WlanIdx;


	if (strlen(arg) != 5)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu   = Input[0];
	u2WlanIdx = Input[1];

	if (CmdPfmuMemAlloc(pAd,
						ucSu_Mu,
						u2WlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemRelease(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;
	UINT16          u2WlanIdx;

	u2WlanIdx  = os_str_tol(arg, 0, 10);

	if (CmdPfmuMemRelease(pAd, u2WlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT	Set_TxBfPfmuMemAllocMapRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;

	if (CmdPfmuMemAllocMapRead(pAd) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT Set_StaRecBfUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[23];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	PMAC_TABLE_ENTRY pEntry;
	BOOLEAN          fgStatus = FALSE;
	struct txbf_starec_conf *man_bf_sta_rec = &pAd->manual_bf_sta_rec;

	if (strlen(arg) != 68)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	WlanIdx                          = Input[0];
	BssIdx                           = Input[1];
	pEntry                           = entry_get(pAd, WlanIdx);

	if (pEntry == NULL)
		return FALSE;

	pEntry->rStaRecBf.u2PfmuId       = Input[2];
	pEntry->rStaRecBf.fgSU_MU        = Input[3];
	pEntry->rStaRecBf.u1TxBfCap      = Input[4];
	pEntry->rStaRecBf.ucNdpaRate     = Input[5];
	pEntry->rStaRecBf.ucNdpRate      = Input[6];
	pEntry->rStaRecBf.ucReptPollRate = Input[7];
	pEntry->rStaRecBf.ucTxMode       = Input[8];
	pEntry->rStaRecBf.ucNc           = Input[9];
	pEntry->rStaRecBf.ucNr           = Input[10];
	pEntry->rStaRecBf.ucCBW          = Input[11];
	pEntry->rStaRecBf.ucSEIdx        = Input[12];
	pEntry->rStaRecBf.ucMemRequire20M = Input[14];
	pEntry->rStaRecBf.ucMemRow0      = Input[15];
	pEntry->rStaRecBf.ucMemCol0      = Input[16];
	pEntry->rStaRecBf.ucMemRow1      = Input[17];
	pEntry->rStaRecBf.ucMemCol1      = Input[18];
	pEntry->rStaRecBf.ucMemRow2      = Input[19];
	pEntry->rStaRecBf.ucMemCol2      = Input[20];
	pEntry->rStaRecBf.ucMemRow3      = Input[21];
	pEntry->rStaRecBf.ucMemCol3      = Input[22];
	/* Default setting */
	pEntry->rStaRecBf.u2SmartAnt     = 0;
	pEntry->rStaRecBf.ucSoundingPhy  = 1;
	pEntry->rStaRecBf.uciBfTimeOut   = 0xFF;
	pEntry->rStaRecBf.uciBfDBW       = 0;
	pEntry->rStaRecBf.uciBfNcol      = 0;
	pEntry->rStaRecBf.uciBfNrow      = 0;
	pEntry->rStaRecBf.nr_lt_bw80       = 0;
	pEntry->rStaRecBf.nc_lt_bw80       = 0;
	pEntry->rStaRecBf.ru_start_idx   = 0;
	pEntry->rStaRecBf.ru_end_idx     = 0;
	pEntry->rStaRecBf.trigger_su     = 0;
	pEntry->rStaRecBf.trigger_mu     = 0;
	pEntry->rStaRecBf.ng16_su        = 0;
	pEntry->rStaRecBf.ng16_mu        = 0;
	pEntry->rStaRecBf.codebook42_su  = 0;
	pEntry->rStaRecBf.codebook75_mu  = 0;
	pEntry->rStaRecBf.he_ltf         = 0;

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_SU_MU))
		pEntry->rStaRecBf.fgSU_MU = man_bf_sta_rec->conf_su_mu;

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_RU_RANGE)) {
		pEntry->rStaRecBf.ru_start_idx = man_bf_sta_rec->conf_ru_start_idx;
		pEntry->rStaRecBf.ru_end_idx = man_bf_sta_rec->conf_ru_end_idx;
	}

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_TRIGGER)) {
		pEntry->rStaRecBf.trigger_su = man_bf_sta_rec->conf_trigger_su;
		pEntry->rStaRecBf.trigger_mu = man_bf_sta_rec->conf_trigger_mu;
	}

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_NG16)) {
		pEntry->rStaRecBf.ng16_su = man_bf_sta_rec->conf_ng16_su;
		pEntry->rStaRecBf.ng16_mu = man_bf_sta_rec->conf_ng16_mu;
	}

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_CODEBOOK)) {
		pEntry->rStaRecBf.codebook42_su = man_bf_sta_rec->conf_codebook42_su;
		pEntry->rStaRecBf.codebook75_mu = man_bf_sta_rec->conf_codebook75_mu;
	}

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_LTF))
		pEntry->rStaRecBf.he_ltf = man_bf_sta_rec->conf_he_ltf;

	if (man_bf_sta_rec->conf & BIT(MANUAL_EHT_HE_IBF)) {
		pEntry->rStaRecBf.uciBfNcol = man_bf_sta_rec->conf_ibf_ncol;
		pEntry->rStaRecBf.uciBfNrow = man_bf_sta_rec->conf_ibf_nrow;
	}

	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
		StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = 0;
		StaCfg.u8EnableFeature = (1 << STA_REC_BF);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;

#ifdef WIFI_UNIFIED_COMMAND
		if (UniCmdStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE)
			fgStatus = TRUE;
#else
		if (CmdExtStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE)
			fgStatus = TRUE;
#endif
	}
	os_zero_mem(man_bf_sta_rec, sizeof(struct txbf_starec_conf));
	return fgStatus;
}

INT set_txbf_he_bf_starec(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct txbf_starec_conf *man_bf_sta_rec = &pAd->manual_bf_sta_rec;
	UINT32 input[14];
	CHAR *value;
	INT i;

	os_zero_mem(man_bf_sta_rec, sizeof(struct txbf_starec_conf));

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

	man_bf_sta_rec->conf = input[0];
	man_bf_sta_rec->conf_su_mu = input[1];
	man_bf_sta_rec->conf_ru_start_idx = input[2];
	man_bf_sta_rec->conf_ru_end_idx = input[3];
	man_bf_sta_rec->conf_trigger_su	= input[4];
	man_bf_sta_rec->conf_trigger_mu = input[5];
	man_bf_sta_rec->conf_ng16_su = input[6];
	man_bf_sta_rec->conf_ng16_mu = input[7];
	man_bf_sta_rec->conf_codebook42_su = input[8];
	man_bf_sta_rec->conf_codebook75_mu = input[9];
	man_bf_sta_rec->conf_he_ltf = input[10];
	man_bf_sta_rec->conf_ibf_ncol = input[11];
	man_bf_sta_rec->conf_ibf_nrow = input[12];

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf=%d\n", man_bf_sta_rec->conf);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_su_mu=%d\n", man_bf_sta_rec->conf_su_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ru_start_idx=%d\n", man_bf_sta_rec->conf_ru_start_idx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ru_end_idx=%d\n", man_bf_sta_rec->conf_ru_end_idx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_trigger_su=%d\n", man_bf_sta_rec->conf_trigger_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_trigger_mu=%d\n", man_bf_sta_rec->conf_trigger_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ng16_su=%d\n", man_bf_sta_rec->conf_ng16_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ng16_mu=%d\n", man_bf_sta_rec->conf_ng16_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_codebook42_su=%d\n", man_bf_sta_rec->conf_codebook42_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_codebook75_mu=%d\n", man_bf_sta_rec->conf_codebook75_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_he_ltf=%d\n", man_bf_sta_rec->conf_he_ltf);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ibf_ncol=%d\n", man_bf_sta_rec->conf_ibf_ncol);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO, "conf_ibf_nrow=%d\n", man_bf_sta_rec->conf_ibf_nrow);

	return TRUE;
}

INT Set_StaRecBfRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16           u2WlanIdx;
	BOOLEAN          fgStatus = FALSE;

	u2WlanIdx = os_str_tol(arg, 0, 10);

	if (CmdETxBfStaRecRead(pAd,
						   u2WlanIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT Set_TxBfAwareCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgBfAware, fgStatus = FALSE;

	fgBfAware = os_str_tol(arg, 0, 10);

	if (CmdTxBfAwareCtrl(pAd, fgBfAware) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT set_dynsnd_en_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN intr_en;
	INT status = 0;

	intr_en = os_str_tol(arg, 0, 10);

	if (cmd_txbf_en_dynsnd_intr(pAd, intr_en) == STATUS_TRUE)
		status = 1;

	return status;
}

INT Set_HostReportTxLatency(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
	BOOLEAN	fgStatus = FALSE;
	UCHAR	ucEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev is null.\n", __func__);
		return FALSE;
	}

	ucEnable = os_str_tol(arg, 0, 10);

	/* Set Host Report Tx Latency, Don't need input OwnMac Info */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		Ret = UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_HOSTREPORT_TXLATENCY_FEATURE, &ucEnable);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		Ret = CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_HOSTREPORT_TXLATENCY_FEATURE, &ucEnable);

	if (Ret == NDIS_STATUS_SUCCESS)
		fgStatus = TRUE;

	return fgStatus;
}

INT Set_RxFilterDropCtrlFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
	INT8 *value = NULL;
	UINT16 input[3] = {0};
	UINT8 i, argCnt = 0, ucAction = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	for (i = 0, value = rstrtok(arg, "-"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, "-"), i++) {
		input[i] = os_str_toul(value, 0, 10);
		argCnt++;
	}

	/* Check Item Count */
	if ((argCnt == 0) || (argCnt != 3))
		goto error;

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev is null.\n", __func__);
		return FALSE;
	}

	/* Drop RTS */
	if (input[0] == 1)
		ucAction |= CFGINFO_DROP_RTS_CTRL_FRAME;

	/* Drop CTS */
	if (input[1] == 1)
		ucAction |= CFGINFO_DROP_CTS_CTRL_FRAME;

	/* Drop unwanted Ctrl Frame */
	if (input[2] == 1)
		ucAction |= CFGINFO_DROP_UNWANTED_CTRL_FRAME;

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		Ret = UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE, &ucAction);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		Ret = CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE, &ucAction);

	if (Ret != STATUS_TRUE)
		goto error;

	return TRUE;

error:
	MTWF_PRINT ("Wrong Cmd Format. Plz input:\n");
	MTWF_PRINT ("iwpriv ra0 set rx_filter_ctrl=[0]-[1]-[2]\n");
	MTWF_PRINT ("  [0]=0: Don't Drop RTS CTRL Frame\n");
	MTWF_PRINT ("	   1: Drop RTS CTRL Frame)\n");
	MTWF_PRINT ("  [1]=0: Don't Drop CTS CTRL Frame\n");
	MTWF_PRINT ("	   1: Drop CTS CTRL Frame)\n");
	MTWF_PRINT ("  [2]=0: Don't Drop Unwanted CTRL Frame\n");
	MTWF_PRINT ("	   1: Drop Unwanted CTRL Frame)\n");

	return FALSE;
}

INT Set_CertCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
	BOOLEAN	fgStatus = FALSE;
	UCHAR	ucEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (wdev == NULL) {
		MTWF_PRINT("%s: wdev is null.\n", __func__);
		return FALSE;
	}

	ucEnable = os_str_tol(arg, 0, 10);

	/* Set Host Report Tx Latency, Don't need input OwnMac Info */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		Ret = UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &ucEnable);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		Ret = CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &ucEnable);

	if (Ret == NDIS_STATUS_SUCCESS)
		fgStatus = TRUE;

	return fgStatus;
}

#ifdef CFG_SUPPORT_MU_MIMO
INT set_dynsnd_cfg_dmcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tmp = NULL;
	INT status = 0;
	UINT8 mcs_index, mcs_th;

	tmp = strsep(&arg, ":");

	if (tmp != NULL)
		mcs_index = os_str_tol(tmp, 0, 10);
	else
		goto error;

	tmp = strsep(&arg, "");

	if (tmp != NULL)
		mcs_th = os_str_tol(tmp, 0, 10);
	else
		goto error;

	if (cmd_txbf_cfg_dynsnd_dmcsth(pAd, mcs_index, mcs_th) == STATUS_TRUE)
		status = 1;

error:
	MTWF_PRINT("%s: status = %d\n", __func__, status);
	return status;
}

INT set_dynsnd_en_mu_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *tmp = NULL;
	INT status = 0;
	BOOLEAN mu_intr_en;
	UINT8 pfid;

	tmp = strsep(&arg, ":");

	if (tmp != NULL)
		mu_intr_en = os_str_tol(tmp, 0, 10);
	else
		goto error;

	tmp = strsep(&arg, "");

	if (tmp != NULL)
		pfid = os_str_tol(tmp, 0, 10);
	else
		goto error;

	if (cmd_txbf_en_dynsnd_pfid_intr(pAd, mu_intr_en, pfid) == STATUS_TRUE)
		status = 1;

error:
	MTWF_PRINT("%s: status = %d\n", __func__, status);
	return status;
}
#endif /* CFG_SUPPORT_MU_MIMO */

#ifdef CONFIG_ATE
INT Set_StaRecCmmUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PMAC_TABLE_ENTRY pEntry;
	CHAR			 *value;
	UCHAR			 Input[9];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	BOOLEAN          fgStatus = FALSE;

	if (strlen(arg) != 26)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	WlanIdx       = Input[0];
	BssIdx        = Input[1];
	pEntry = entry_get(pAd, WlanIdx);

	if (pEntry == NULL)
		return FALSE;

	pEntry->Aid     = Input[2];
	pEntry->Addr[0] = Input[3];
	pEntry->Addr[1] = Input[4];
	pEntry->Addr[2] = Input[5];
	pEntry->Addr[3] = Input[6];
	pEntry->Addr[4] = Input[7];
	pEntry->Addr[5] = Input[8];
	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
		StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = CONNECTION_INFRA_AP;
		StaCfg.u8EnableFeature = (1 << STA_REC_BASIC_STA_RECORD);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.u2WlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = TRUE;

		if (CmdExtStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE)
			fgStatus = TRUE;
	}
	return fgStatus;
}

INT Set_BssInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[8];
	UCHAR            Bssid[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx, BssIdx;
	BOOLEAN          fgStatus = FALSE;
	BSS_INFO_ARGUMENT_T bss_info_argument;
	UCHAR control_band_idx;
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test;
	serv_test = (struct service_test *)(pAd->serv.serv_handle);
	control_band_idx = serv_test->ctrl_band_idx;
#else
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	control_band_idx = ATECtrl->control_band_idx;
#endif /*  CONFIG_WLAN_SERVICE */

	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	OwnMacIdx = Input[0];
	BssIdx    = Input[1];
	Bssid[0]  = Input[2];
	Bssid[1]  = Input[3];
	Bssid[2]  = Input[4];
	Bssid[3]  = Input[5];
	Bssid[4]  = Input[6];
	Bssid[5]  = Input[7];
	NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
	bss_info_argument.OwnMacIdx = OwnMacIdx;
	bss_info_argument.ucBssIndex = BssIdx;
	os_move_mem(bss_info_argument.Bssid, Bssid, MAC_ADDR_LEN);
	bss_info_argument.bmc_wlan_idx = 1; /* MCAST_WCID which needs to be modified by Patrick; */
	bss_info_argument.NetworkType = NETWORK_INFRA;
	bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
	bss_info_argument.CipherSuit = CIPHER_SUIT_NONE;
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.ucBandIdx = control_band_idx;
	bss_info_argument.u8BssInfoFeature = BSS_INFO_OWN_MAC_FEATURE | BSS_INFO_BASIC_FEATURE;

	if (AsicBssInfoUpdate(pAd, &bss_info_argument) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT Set_DevInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR			 *value;
	UCHAR			 Input[8];
	UCHAR            OwnMacAddr[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx;
	BOOLEAN          fgStatus = FALSE;
	UINT8		     BandIdx = 0;

	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}
	OwnMacIdx     = Input[0];
	OwnMacAddr[0] = Input[1];
	OwnMacAddr[1] = Input[2];
	OwnMacAddr[2] = Input[3];
	OwnMacAddr[3] = Input[4];
	OwnMacAddr[4] = Input[5];
	OwnMacAddr[5] = Input[6];
	BandIdx = Input[7];
	if (AsicDevInfoUpdate(
			pAd,
			OwnMacIdx,
			OwnMacAddr,
			BandIdx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /*CONFIG_ATE*/

#endif /* MT_MAC */

#if defined(MT_MAC)
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 i;
	UCHAR ucITxBfEn;
	INT   u4Status = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;

	ucITxBfEn = os_str_tol(arg, 0, 10);

	if (ucITxBfEn > 1)
		return FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);

		if (!IS_ENTRY_NONE(pEntry)) {
			wdev = pEntry->wdev;
			TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
			TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;

#ifdef ANTENNA_CONTROL_SUPPORT
			{
				if (pAd->bAntennaSetAPEnable) {
					TxBfInfo.ucTxPathNum = pAd->TxStream;
					TxBfInfo.ucRxPathNum = pAd->RxStream;
				}
			}
#endif /* ANTENNA_CONTROL_SUPPORT */

			TxBfInfo.ucPhyMode   = wdev->PhyMode;
			TxBfInfo.u2Channel   = wdev->channel;
			TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
			TxBfInfo.cmmCfgETxBfIncapable  = pAd->CommonCfg.ETxBfIncapable;
			TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
			TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
			TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] + i * pAd->mac_ctrl.wtbl_entry_size[0];
			TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] + i * pAd->mac_ctrl.wtbl_entry_size[1];
			TxBfInfo.ucETxBfTxEn = FALSE;
			TxBfInfo.ucITxBfTxEn = ucITxBfEn;
			TxBfInfo.u2Wcid      = i;
			TxBfInfo.ucBW        = pEntry->HTPhyMode.field.BW;
			TxBfInfo.ucNDPARate  = 2; /* MCS2 */
			u4Status = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return u4Status;
}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
/*
	The VhtNDPA sounding inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the MCS value.
*/
INT Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6];
	UINT mcs;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "\n%s\n", arg);

	/*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and MCS value in decimal format.*/
	if (strlen(arg) < 19)
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		mcs = (UINT)os_str_tol((token + 1), 0, 10);
		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token && (i < MAC_ADDR_LEN);
				token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&mac[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_PRINT ("\n"MACSTR"-%02x\n", MAC2STR(mac), mcs);
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);

		if (pEntry) {
#ifdef SOFT_SOUNDING
			pEntry->snd_rate.field.MODE = MODE_VHT;
			pEntry->snd_rate.field.BW = (mcs / 100) > BW_80 ? BW_80 : (mcs / 100);
			mcs %= 100;
			pEntry->snd_rate.field.MCS = ((mcs / 10) << 4 | (mcs % 10));
			MTWF_PRINT
					 ("%s():Trigger VHT NDPA Sounding="MACSTR", snding rate=VHT-%sHz, %dSS-MCS%d\n",
					  __func__, MAC2STR(mac),
					  get_bw_str(pEntry->snd_rate.field.BW, BW_FROM_OID),
					  (pEntry->snd_rate.field.MCS >> 4) + 1,
					  pEntry->snd_rate.field.MCS & 0xf);
#endif
			trigger_vht_ndpa(pAd, pEntry);
		}

		return TRUE;
	}

	return FALSE;
}
#endif /* VHT_TXBF_SUPPORT */

#if defined(MT_MAC)
#ifdef TXBF_SUPPORT

INT Set_TxBfProfileTag_Help(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT (
				 "========================================================================================================================\n"
				 "TxBfProfile Tag1 setting example :\n"
				 "iwpriv ra0 set TxBfProfileTagPfmuIdx  =xx\n"
				 "iwpriv ra0 set TxBfProfileTagBfType   =xx (0: iBF; 1: eBF)\n"
				 "iwpriv ra0 set TxBfProfileTagBw       =xx (0/1/2/3 : BW20/40/80/160NC)\n"
				 "iwpriv ra0 set TxBfProfileTagSuMu     =xx (0:SU, 1:MU)\n"
				 "iwpriv ra0 set TxBfProfileTagInvalid  =xx (0: valid, 1: invalid)\n"
				 "iwpriv ra0 set TxBfProfileTagMemAlloc =xx:xx:xx:xx:xx:xx:xx:xx (mem_row, mem_col), ..\n"
				 "iwpriv ra0 set TxBfProfileTagMatrix   =nrow:nol:ng:LM\n"
				 "iwpriv ra0 set TxBfProfileTagSnr      =SNR_STS0:SNR_STS1:SNR_STS2:SNR_STS3\n"
				 "\n\n"
				 "TxBfProfile Tag2 setting example :\n"
				 "iwpriv ra0 set TxBfProfileTagSmtAnt   =xx (11:0)\n"
				 "iwpriv ra0 set TxBfProfileTagSeIdx    =xx\n"
				 "iwpriv ra0 set TxBfProfileTagRmsdThrd =xx\n"
				 "iwpriv ra0 set TxBfProfileTagMcsThrd  =xx:xx:xx:xx:xx:xx (MCS TH L1SS:S1SS:L2SS:....)\n"
				 "iwpriv ra0 set TxBfProfileTagTimeOut  =xx\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredBw=xx (0/1/2/3 : BW20/40/80/160NC)\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredNc=xx\n"
				 "iwpriv ra0 set TxBfProfileTagDesiredNr=xx\n"
				 "\n\n"
				 "Read TxBf profile Tag :\n"
				 "iwpriv ra0 set TxBfProfileTagRead     =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile Tag :\n"
				 "iwpriv ra0 set TxBfProfileTagWrite    =xx (PFMU ID)\n"
				 "When you use one of relative CMD to update one of tag parameters, you should call TxBfProfileTagWrite to update Tag\n"
				 "\n\n"
				 "Read TxBf profile Data	:\n"
				 "iwpriv ra0 set TxBfProfileDataRead    =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile Data :\n"
				 "iwpriv ra0 set TxBfProfileDataWrite   =BW :subcarrier:phi11:psi2l:Phi21:Psi31:Phi31:Psi41:Phi22:Psi32:Phi32:Psi42:Phi33:Psi43\n"
				 "iwpriv ra0 set TxBfProfileDataWriteAll=Profile ID : BW (BW       : 0x00 (20M) , 0x01 (40M), 0x02 (80M), 0x3 (160M)\n"
				 "When you use CMD TxBfProfileDataWrite to update profile data per subcarrier, you should call TxBfProfileDataWriteAll to update all of\n"
				 "subcarrier's profile data.\n\n"
				 "Read TxBf profile PN	:\n"
				 "iwpriv ra0 set TxBfProfilePnRead      =xx (PFMU ID)\n"
				 "\n"
				 "Write TxBf profile PN :\n"
				 "iwpriv ra0 set TxBfProfilePnWrite     =Profile ID:BW:1STS_Tx0:1STS_Tx1:1STS_Tx2:1STS_Tx3:2STS_Tx0:2STS_Tx1:2STS_Tx2:2STS_Tx3:3STS_Tx1:3STS_Tx2:3STS_Tx3\n"
				 "========================================================================================================================\n");
	return TRUE;
}

INT Set_TxBfProfileTag_PfmuIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT16 profileIdx;

	profileIdx = (UINT16)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_PFMU_ID, profileIdx);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_BfType(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucBfType;

	ucBfType = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_IEBF, ucBfType);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_DBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucBw;

	ucBw = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_DBW, ucBw);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_SuMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucSuMu;

	ucSuMu = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SU_MU, ucSuMu);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_InValid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 InValid;

	InValid = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_INVALID, InValid);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_Mem(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR   Input[8];
	CHAR	 *value;
	INT	i;
	UINT8 aMemAddrColIdx[4], aMemAddrRowIdx[4];

	/* mem col0:row0:col1:row1:col2:row2:col3:row3 */
	if (strlen(arg) != 23)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	aMemAddrColIdx[0] = Input[0];
	aMemAddrRowIdx[0] = Input[1];
	aMemAddrColIdx[1] = Input[2];
	aMemAddrRowIdx[1] = Input[3];
	aMemAddrColIdx[2] = Input[4];
	aMemAddrRowIdx[2] = Input[5];
	aMemAddrColIdx[3] = Input[6];
	aMemAddrRowIdx[3] = Input[7];

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_ROW0, aMemAddrRowIdx[0]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_ROW1, aMemAddrRowIdx[1]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_ROW2, aMemAddrRowIdx[2]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_ROW3, aMemAddrRowIdx[3]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_COL0, aMemAddrColIdx[0]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_COL1, aMemAddrColIdx[1]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_COL2, aMemAddrColIdx[2]);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MEM_COL3, aMemAddrColIdx[3]);

		return TRUE;
	} else
		return FALSE;
}

INT Set_TxBfProfileTag_Matrix(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR   Input[6];
	CHAR	 *value;
	INT	i;
	UINT8   ucNrow, ucNcol, ucNgroup, ucLM, ucCodeBook, ucHtcExist;

	/* nrow:nol:ng:LM:CodeBook:HtcExist */
	if (strlen(arg) != 17)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucNrow     = Input[0];
	ucNcol     = Input[1];
	ucNgroup   = Input[2];
	ucLM       = Input[3];
	ucCodeBook = Input[4];
	ucHtcExist = Input[5];

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_NR, ucNrow);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_NC, ucNcol);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_NG, ucNgroup);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_LM, ucLM);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_CODEBOOK, ucCodeBook);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_HTC, ucHtcExist);

		return TRUE;
	} else
		return FALSE;

}

INT set_txbf_prof_tag_ru_range(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 input[2];
	UINT8 ru_start, ru_end;
	CHAR *value;
	UINT i;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = (UINT8)os_str_toul(value, 0, 10);

	if (i != 2) {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	ru_start = input[0];
	ru_end = input[1];

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_RU_START, ru_start);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_RU_END, ru_end);

		return TRUE;
	} else
		return FALSE;
}

INT set_txbf_prof_tag_partial_bw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 input[2];
	UINT8 u1Bitmap, u1Resolution;
	CHAR *value;
	INT i;
	UINT16 u2PartialBw;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = (UINT8)os_str_toul(value, 0, 10);

	if (i != 2) {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	u1Bitmap = input[0];
	u1Resolution = input[1];

	u2PartialBw = u1Bitmap << 1;
	u2PartialBw |= u1Resolution;
	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_PARTIAL_BW, u2PartialBw);

		return TRUE;
	} else
		return FALSE;
}

INT set_txbf_prof_tag_mob_cal_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 mob_cal_en;

	mob_cal_en = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_MOB_CAL_EN, mob_cal_en);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_SNR(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR   Input[8];
	CHAR	 *value;
	INT	i;
	UINT8 ucSNR_STS0, ucSNR_STS1, ucSNR_STS2, ucSNR_STS3;
	UINT8 ucSNR_STS4, ucSNR_STS5, ucSNR_STS6, ucSNR_STS7;

	if ((strlen(arg) != 11) && (strlen(arg) != 23))
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSNR_STS0 = Input[0];
	ucSNR_STS1 = Input[1];
	ucSNR_STS2 = Input[2];
	ucSNR_STS3 = Input[3];
	ucSNR_STS4 = Input[4];
	ucSNR_STS5 = Input[5];
	ucSNR_STS6 = Input[6];
	ucSNR_STS7 = Input[7];

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS0, ucSNR_STS0);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS1, ucSNR_STS1);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS2, ucSNR_STS2);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS3, ucSNR_STS3);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS4, ucSNR_STS4);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS5, ucSNR_STS5);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS6, ucSNR_STS6);
		ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG1_SNR_STS7, ucSNR_STS7);

		return TRUE;
	} else
		return FALSE;
}

INT Set_TxBfProfileTag_SmartAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 SmartAnt;

	SmartAnt = (UINT32)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_SMART_ANT, SmartAnt);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_SeIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucSeIdx;

	ucSeIdx = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_SE_ID, ucSeIdx);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_RmsdThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucRmsdThrd;

	ucRmsdThrd = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_RMSD_THRESHOLD, ucRmsdThrd);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_McsThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[6];
	CHAR	 *value;
	INT	i;
	UCHAR   ucMcsLss[3], ucMcsSss[3];

	if (strlen(arg) != 17)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucMcsLss[0] = Input[0];
	ucMcsSss[0] = Input[1];
	ucMcsLss[1] = Input[2];
	ucMcsSss[1] = Input[3];
	ucMcsLss[2] = Input[4];
	ucMcsSss[2] = Input[5];
#ifndef DOT11_HE_AX
	TxBfProfileTag_McsThd(&pAd->rPfmuTag2,
						  ucMcsLss,
						  ucMcsSss);
#endif
	return TRUE;
}

INT Set_TxBfProfileTag_TimeOut(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucTimeOut;

	ucTimeOut = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_TIMEOUT, ucTimeOut);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_DesiredBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucDesiredBW;

	ucDesiredBW = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_DBW, ucDesiredBW);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_DesiredNc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucDesiredNc;

	ucDesiredNc = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_NCOL, ucDesiredNc);
	else
		return FALSE;
}

INT Set_TxBfProfileTag_DesiredNr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 ucDesiredNr;

	ucDesiredNr = (UINT8)os_str_tol(arg, 0, 10);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_NROW, ucDesiredNr);
	else
		return FALSE;
}

INT set_txbf_prof_tag_ru_alloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#ifdef IBF_BITMAP_SUPPORT
	UINT16 bw_bitmap = (UINT16)os_str_tol(arg, 0, 10);

	MTWF_PRINT("bw_bitmap value = 0x%x\n", bw_bitmap);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_RU_ALLOC, bw_bitmap);
#else
	UINT8 ru_alloc = (UINT8)os_str_tol(arg, 0, 10);

	MTWF_PRINT("ru_alloc value = 0x%x\n", ru_alloc);
	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(pAd->hdev_ctrl, TAG2_IBF_RU_ALLOC, ru_alloc);
#endif
	else
		return FALSE;
}

INT Set_TxBfProfileTagRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	BOOLEAN fgBFer;
	UCHAR   Input[2];
	CHAR	 *value;
	INT	i;

	if (strlen(arg) != 5)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx = Input[0];
	fgBFer     = Input[1];

	return TxBfProfileTagRead(pAd, profileIdx, fgBFer);
}


INT Set_TxBfProfileTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR profileIdx;

	profileIdx = (UCHAR) os_str_tol(arg, 0, 10);

	if (ops->write_txbf_pfmu_tag)
		return ops->write_txbf_pfmu_tag(pAd->hdev_ctrl, profileIdx);
	else
		return FALSE;
}

INT Set_TxBfProfileDataRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	 *value;
	UCHAR   Input[4];
	INT	i;
	UCHAR   profileIdx, subcarrIdx_H, subcarrIdx_L;
	BOOLEAN fgBFer;
	USHORT  subcarrIdx;

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 11)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":")) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx   = Input[0];
	fgBFer       = Input[1];
	subcarrIdx_H = Input[2];
	subcarrIdx_L = Input[3];
	subcarrIdx = ((USHORT)(subcarrIdx_H << 8) | (USHORT)subcarrIdx_L);

	return TxBfProfileDataRead(pAd, profileIdx, fgBFer, subcarrIdx);
}

INT Set_TxBfProfileDataWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT Input[18];
	CHAR *value, value_T[12], onebyte;
	UCHAR strLen;
	INT i;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	os_zero_mem(Input, 36);

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 60)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":"), i++) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			strlcpy(value_T, "0", sizeof(value_T));
			strncat(value_T, value, strLen);
			AtoH(value_T, (PCHAR)(&Input[i]), 2);
			Input[i] = be2cpu16(Input[i]);
		} else if (strLen == 2) {
			AtoH(value, (PCHAR)(&onebyte), 1);
			Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
		} else
			MTWF_PRINT("%s: Error: Un-expected string len!\n", __func__);
	}

	if (ops->write_txbf_profile_data)
		return ops->write_txbf_profile_data(pAd, Input);
	else
		return FALSE;
}

INT set_txbf_angle_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 bfer, nc;
	UINT32 angle[14]; /* max angel pair - phi and psi */
	CHAR *tok;
	INT i;

	os_zero_mem(angle, sizeof(angle));

	tok = rstrtok(arg, ":");
	if (tok) {
		bfer = os_str_toul(tok, 0, 16);
	} else {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	tok = rstrtok(NULL, ":");
	if (tok) {
		nc = os_str_toul(tok, 0, 16);
	} else {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	tok = rstrtok(NULL, ":");
	for (i = 0; tok && (i < ARRAY_SIZE(angle)); i++) {
		angle[i] = os_str_toul(tok, 0, 16);
		tok = rstrtok(NULL, ":");
	}

	if (ops->set_txbf_angle)
		return ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);
	else
		return FALSE;
}

INT set_txbf_dsnr_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 bfer;
	UINT32 dsnr[8]; /* max dsnr quantity */
	CHAR *tok;
	INT i;

	os_zero_mem(dsnr, sizeof(dsnr));

	tok = rstrtok(arg, ":");
	if (tok) {
		bfer = os_str_toul(tok, 0, 16);
	} else {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	tok = rstrtok(NULL, ":");
	for (i = 0; tok && (i < ARRAY_SIZE(dsnr)); i++) {
		dsnr[i] = os_str_toul(tok, 0, 16);
		tok = rstrtok(NULL, ":");
	}

	if (ops->set_txbf_dsnr)
		return ops->set_txbf_dsnr(pAd->hdev_ctrl, bfer, dsnr);
	else
		return FALSE;
}

INT set_txbf_pfmu_data_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 input[3];
	UINT16 subc;
	UINT8 pfmu_id;
	BOOLEAN bfer;
	CHAR *value;
	INT i;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

	if (i != 3) {
		MTWF_PRINT("%s: Error: Un-expected format!\n", __func__);
		return FALSE;
	}

	pfmu_id = input[0];
	subc = input[1];
	bfer = input[2];

	if (ops->write_txbf_pfmu_data)
		return ops->write_txbf_pfmu_data(pAd->hdev_ctrl, pfmu_id, subc, bfer);
	else
		return FALSE;
}

#ifdef CONFIG_ATE
INT Set_TxBfProfileData20MAllWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* Input Cmd Argument Parsing */
	USHORT	*input;     /* Input[] array stores absolute data */
	CHAR	*value, value_t[12], one_byte; /* *value stores every input argument
between : */
	UCHAR	str_len;
	INT i;
	struct  _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR	control_band_idx;
	UINT8   tx_path = pAd->Antenna.field.TxPath;
	CHAR    sub_num, input_param_num;
	INT     sub_num_idx, arg_len;
	UCHAR	profile_idx;
	USHORT	sub_carr_id;
	UINT16  angle_ph11, angle_ph21, angle_ph31, angle_ph41, angle_ph51;
	INT16   phi11 = 0,  phi21 = 0,  phi31 = 0,  phi41 = 0;
	BOOLEAN fg_status = FALSE, fg_final_raw_data = FALSE;

	/* Init Array 720 bytes */
	os_alloc_mem(pAd, (UCHAR **)&input, sizeof(USHORT) * 360);
	if (!input)
		goto end;
	os_zero_mem(input, sizeof(USHORT) * 360);

	arg_len = strlen(arg);

	/* Absolute Phi Value Processing */
	if ((strlen(arg) != 183) && (strlen(arg) != 223) && (strlen(arg) != 5)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"False: Command inputs not meet the Command format Length!\n");

		fg_status = FALSE;
		goto end;
	}

	for (i = 0, value = rstrtok(arg, ":"); value && (i < 360);
			value = rstrtok(NULL, ":"), i++) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
					" False: Command input arguments aren't Hex format!\n");
			fg_status = FALSE;
			goto end;
		}

		str_len = strlen(value);

		if (str_len & 1) {
			strlcpy(value_t, "0", sizeof(value_t));
			strncat(value_t, value, str_len);
			AtoH(value_t, (PCHAR)(&input[i]), 2);
			input[i] = be2cpu16(input[i]);
		} else if (str_len == 2) {
			AtoH(value, (PCHAR)(&one_byte), 1);
			input[i] = ((USHORT)one_byte) & ((USHORT)0x00FF);
		} else if (str_len == 4) {
			AtoH(value, (PCHAR)(&input[i]), 2);
			input[i] = be2cpu16(input[i]);
		} else
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE,  DBG_LVL_ERROR,
					" Error: Un-expected Argument Length!\n");
	}

	/* Check if the input is the last raw data or not */
	fg_final_raw_data = FALSE;
	if (arg_len == 5) {
		if (input[1] == 0x00F0) {
			pAd->profile_data_cnt = 0;
			fg_status = TRUE;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,  DBG_LVL_INFO,
					"%s: Status: Start to Input Profile Data!\n", __func__);
			goto end;
		}

		if (input[1] == 0x00FF) {
			fg_final_raw_data = TRUE;
			profile_idx = input[0];
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE,  DBG_LVL_INFO,
					"%s: Status: End to Input Profile Data!\n", __func__);
		}
	}

	/* Relative Phi Value Processing */
	control_band_idx = ATECtrl->control_band_idx;
	input_param_num = 5;
	if (tx_path > 4)
		input_param_num = 6;

	if (fg_final_raw_data == FALSE) {
		/* Input Array Structure Assignment */
		for (sub_num = 0 ; sub_num < 8 ; sub_num++) {
			sub_num_idx = sub_num * input_param_num;
			sub_carr_id = input[sub_num_idx];

			if (sub_carr_id < 32)
				sub_carr_id += 224;
			else
				sub_carr_id -= 32;

			angle_ph11  = input[sub_num_idx+1];
			angle_ph21  = input[sub_num_idx+2];
			angle_ph31  = input[sub_num_idx+3];
			angle_ph41  = input[sub_num_idx+4];
			if (tx_path > NSTS_4)
				angle_ph51  = input[sub_num_idx+5];

			switch (tx_path) {
			case NSTS_2:
				phi11    = (INT16)(angle_ph21 - angle_ph11);
				phi21    = 0;
				phi31    = 0;

				MTWF_PRINT("%s:: SubCarrier ID=%d, angle_ph21 = %x, angle_ph11 = %x, phi11 = %x\n",
						__func__, sub_carr_id, angle_ph21, angle_ph11, phi11);
				break;

			case NSTS_3:
				phi11    = (INT16)(angle_ph31 - angle_ph11);
				phi21    = (INT16)(angle_ph31 - angle_ph21);
				phi31    = 0;

				MTWF_PRINT("%s:: SubCarrier ID=%d, angle_ph31 = %x, angle_ph21 = %x, angle_ph11 = %x, phi11 = %x, phi21 = %x\n",
						__func__, sub_carr_id, angle_ph31, angle_ph21, angle_ph11, phi11, phi21);
				break;

			case NSTS_4:
			default:
				phi11    = (INT16)(angle_ph41 - angle_ph11);
				phi21    = (INT16)(angle_ph41 - angle_ph21);
				phi31    = (INT16)(angle_ph41 - angle_ph31);
				break;

			case NSTS_5:
				phi11    = (INT16)(angle_ph51 - angle_ph11);
				phi21    = (INT16)(angle_ph51 - angle_ph21);
				phi31    = (INT16)(angle_ph51 - angle_ph31);
				phi41    = (INT16)(angle_ph51 - angle_ph41);
				break;
			}

			if (tx_path < NSTS_5) {
				pAd->profile_data[pAd->profile_data_cnt + sub_num].u2SubCarrIdx = sub_carr_id;
				pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi11 = phi11;
				pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi21 = phi21;
				pAd->profile_data[pAd->profile_data_cnt + sub_num].i2Phi31 = phi31;
			} else {
				pAd->profile_data_5x5[pAd->profile_data_cnt + sub_num].u2SubCarrIdx = sub_carr_id;
				pAd->profile_data_5x5[pAd->profile_data_cnt + sub_num].i2Phi11 = phi11;
				pAd->profile_data_5x5[pAd->profile_data_cnt + sub_num].i2Phi21 = phi21;
				pAd->profile_data_5x5[pAd->profile_data_cnt + sub_num].i2Phi31 = phi31;
				pAd->profile_data_5x5[pAd->profile_data_cnt + sub_num].i2Phi41 = phi41;
			}
		}

		pAd->profile_data_cnt += sub_num;
		fg_status = TRUE;

	} else {
		if (tx_path < NSTS_5) {
			if (CmdETxBfPfmuProfileDataWrite20MAll(pAd,
													profile_idx,
													(PUCHAR)&pAd->profile_data[0]) == STATUS_TRUE) {
				fg_status = TRUE;
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE,  DBG_LVL_INFO,
				"Status: Cmd Send to FW!\n");
			}
		} else {
			if (CmdETxBfPfmuProfileDataWrite20MAll_5x5(pAd,
													profile_idx,
													(PUCHAR)&pAd->profile_data_5x5[0]) == STATUS_TRUE) {
				fg_status = TRUE;
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE,  DBG_LVL_INFO,
				"Status: Cmd Send to FW!\n");
			}
		}
	}

end:
	if (input)
		os_free_mem(input);
	return fg_status;

}

#endif

INT Set_TxBfProfilePnRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx = os_str_tol(arg, 0, 10);
	return TxBfProfilePnRead(pAd, profileIdx);
}

INT Set_TxBfProfilePnWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	UCHAR   ucBw;
	CHAR    *value, value_T[12], onebyte;
	UCHAR   strLen;
	SHORT   Input[14] = {0};
	INT	status, i;
	PFMU_PN rPfmuPn;
	PFMU_PN_DBW80_80M rPfmuPn160M;
	PUCHAR  pPfmuPn = NULL;

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 55)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":"), i++) {
		if ((!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			strlcpy(value_T, "0", sizeof(value_T));
			strncat(value_T, value, strLen);
			AtoH(value_T, (PCHAR)(&Input[i]), 2);
			Input[i] = be2cpu16(Input[i]);
		} else if (strLen == 2) {
			AtoH(value, (PCHAR)(&onebyte), 1);
			Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
		} else
			MTWF_PRINT("%s: Error: Un-expected string len!!!!!\n", __func__);
	}

	profileIdx    = Input[0];
	ucBw          = Input[1];

	if (ucBw != P_DBW160M) {
		os_zero_mem(&rPfmuPn, sizeof(rPfmuPn));
		rPfmuPn.rField.u2CMM_1STS_Tx0    = Input[2];
		rPfmuPn.rField.u2CMM_1STS_Tx1    = Input[3];
		rPfmuPn.rField.u2CMM_1STS_Tx2    = Input[4] & 0x3FF;
		rPfmuPn.rField.u2CMM_1STS_Tx2Msb = Input[4] >> 11;
		rPfmuPn.rField.u2CMM_1STS_Tx3    = Input[5];
		rPfmuPn.rField.u2CMM_2STS_Tx0    = Input[6];
		rPfmuPn.rField.u2CMM_2STS_Tx1    = Input[7] & 0x1FF;
		rPfmuPn.rField.u2CMM_2STS_Tx1Msb = Input[7] >> 10;
		rPfmuPn.rField.u2CMM_2STS_Tx2    = Input[8];
		rPfmuPn.rField.u2CMM_2STS_Tx3    = Input[9];
		rPfmuPn.rField.u2CMM_3STS_Tx0    = Input[10] & 0x0FF;
		rPfmuPn.rField.u2CMM_3STS_Tx0Msb = Input[10] >> 9;
		rPfmuPn.rField.u2CMM_3STS_Tx1    = Input[11];
		rPfmuPn.rField.u2CMM_3STS_Tx2    = Input[12];
		rPfmuPn.rField.u2CMM_3STS_Tx3    = Input[13] & 0x07F;
		rPfmuPn.rField.u2CMM_3STS_Tx3Msb = Input[13] >> 8;
		pPfmuPn = (PUCHAR) (&rPfmuPn);
		status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	} else {
		os_zero_mem(&rPfmuPn160M, sizeof(rPfmuPn160M));
		rPfmuPn160M.rField.u2DBW160_1STS_Tx0    = Input[2];
		rPfmuPn160M.rField.u2DBW160_1STS_Tx1    = Input[3];
		rPfmuPn160M.rField.u2DBW160_2STS_Tx0    = Input[4] & 0x3FF;
		rPfmuPn160M.rField.u2DBW160_2STS_Tx0Msb = Input[4] >> 11;
		rPfmuPn160M.rField.u2DBW160_2STS_Tx1    = Input[5];
		pPfmuPn = (PUCHAR) (&rPfmuPn160M);
		status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	}

	return status;
}

INT Set_TxBfQdRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8	subcarrIdx;

	subcarrIdx = (INT8)simple_strtol(arg, 0, 10);
	TxBfQdRead(pAd, subcarrIdx);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	RF test switch mode.

	iwpriv ra0 set TxBfFbRptDbgInfo = ucAction : Input[1]

	ucAction
	0: BF_READ_AND_CLEAR_FBK_STAT_INFO
	1: BF_READ_FBK_STAT_INFO
	2: BF_SET_POLL_PFMU_INTR_STAT_TIMEOUT
	   Also set Input[1] as PollPFMUIntrStatTimeOut
	3: BF_SET_PFMU_DEQ_INTERVAL
	   Also set Input[1] as FbRptDeQInterval

	Return:
    ==========================================================================
*/
INT Set_TxBfFbRptDbgInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT8 i, Input[3] = {0};
#ifndef WIFI_UNIFIED_COMMAND
	EXT_CMD_TXBF_FBRPT_DBG_INFO_T ETxBfFbRptData;
#else
	struct UNI_CMD_TXBF_FBRPT_DBG_INFO ETxBfFbRptData;
#endif

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(Input));
			value = rstrtok(NULL, ":"), i++)
		Input[i] = (UINT8)simple_strtol(value, 0, 10);

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&ETxBfFbRptData, sizeof(EXT_CMD_TXBF_FBRPT_DBG_INFO_T));
#else
	os_zero_mem(&ETxBfFbRptData, sizeof(struct UNI_CMD_TXBF_FBRPT_DBG_INFO));
#endif

	ETxBfFbRptData.u1Action = Input[0];

	switch (ETxBfFbRptData.u1Action) {
	case BF_SET_POLL_PFMU_INTR_STAT_TIMEOUT:
		ETxBfFbRptData.u1PollPFMUIntrStatTimeOut = Input[1];
		break;
	case BF_SET_PFMU_DEQ_INTERVAL:
		ETxBfFbRptData.u1FbRptDeQInterval = Input[1];
		break;
	case BF_DYNAMIC_PFMU_UPDATE:
		ETxBfFbRptData.u2WlanIdx = Input[1];
		ETxBfFbRptData.u1PFMUUpdateEn = Input[2];
		break;
	default:
		break;
	}

	TxBfFbRptDbgInfo(pAd, (PUINT8)&ETxBfFbRptData);

	return TRUE;
}

INT Set_TxBfTxSndInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT16 input[5] = {0};
	UINT8 i;
#ifndef WIFI_UNIFIED_COMMAND
	EXT_CMD_TXBF_SND_CMD_T txbfSndCmd;
#else
	struct UNI_CMD_BF_SND_CMD UnitxbfSndCmd;
#endif

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&txbfSndCmd, sizeof(EXT_CMD_TXBF_SND_CMD_T));

	txbfSndCmd.ucAction = input[0];

	switch (txbfSndCmd.ucAction) {
	case BF_SND_READ_INFO:
		txbfSndCmd.ucReadClr = input[1];
		break;
	case BF_SND_CFG_OPT:
		txbfSndCmd.ucVhtOpt = input[1];
		txbfSndCmd.ucHeOpt = input[2];
		txbfSndCmd.ucGloOpt = input[3];
		break;
	case BF_SND_CFG_INTV:
		txbfSndCmd.u2WlanIdx = input[1];
		txbfSndCmd.ucSndIntv = input[2];
		break;
	case BF_SND_STA_STOP:
		txbfSndCmd.u2WlanIdx = input[1];
		txbfSndCmd.ucSndStop = input[2];
		break;
	case BF_SND_CFG_MAX_STA:
		txbfSndCmd.ucMaxSndStas = input[1];
		break;
	case BF_SND_CFG_BFRP:
		txbfSndCmd.fgMan = input[1];
		txbfSndCmd.ucTxTime = input[2];
		txbfSndCmd.ucMcs = input[3];
		txbfSndCmd.fgLDPC = input[4];
		break;
	case BF_SND_CFG_INF:
		txbfSndCmd.ucInf = input[1];
		break;
	case BF_SND_CFG_TXOP_SND:
		txbfSndCmd.fgMan = input[1];
		txbfSndCmd.ucAcQ = input[2];
		txbfSndCmd.fgSxnProtect = input[3];
		txbfSndCmd.fgDirectFBK = input[4];
		break;
	default:
		break;
	}

	TxBfTxSndInfo(pAd, (PUINT8)&txbfSndCmd);

#else
	os_zero_mem(&UnitxbfSndCmd, sizeof(struct UNI_CMD_BF_SND_CMD));
	UnitxbfSndCmd.u1Action = input[0];

	switch (UnitxbfSndCmd.u1Action) {
	case BF_SND_READ_INFO:
		UnitxbfSndCmd.u1ReadClr = input[1];
		break;
	case BF_SND_CFG_OPT:
		UnitxbfSndCmd.u1VhtOpt = input[1];
		UnitxbfSndCmd.u1HeOpt = input[2];
		UnitxbfSndCmd.u1GloOpt = input[3];
		break;
	case BF_SND_CFG_INTV:
		UnitxbfSndCmd.u2WlanIdx = input[1];
		UnitxbfSndCmd.u1SndIntv = input[2];
		break;
	case BF_SND_STA_STOP:
		UnitxbfSndCmd.u2WlanIdx = input[1];
		UnitxbfSndCmd.u1SndStop = input[2];
		break;
	case BF_SND_CFG_MAX_STA:
		UnitxbfSndCmd.u1MaxSndStas = input[1];
		break;
	case BF_SND_CFG_BFRP:
		UnitxbfSndCmd.fgMan = input[1];
		UnitxbfSndCmd.u1TxTime = input[2];
		UnitxbfSndCmd.u1Mcs = input[3];
		UnitxbfSndCmd.u1LDPC = input[4];
		break;
	case BF_SND_CFG_INF:
		UnitxbfSndCmd.u1Inf = input[1];
		break;
	case BF_SND_CFG_TXOP_SND:
		UnitxbfSndCmd.fgMan = input[1];
		UnitxbfSndCmd.ucAcQ = input[2];
		UnitxbfSndCmd.fgSxnProtect = input[3];
		UnitxbfSndCmd.fgDirectFBK = input[4];
		break;
	case BF_SND_CFG_IBF_ABNRML_CTRL:
		UnitxbfSndCmd.fgMan = input[1];
		UnitxbfSndCmd.i1Thr1 = (INT8)input[2];
		UnitxbfSndCmd.i1Thr2 = (INT8)input[3];
		break;
	default:
		break;
	}

	TxBfTxSndInfo(pAd, (UINT8 *)&UnitxbfSndCmd);

#endif

	return TRUE;
}

INT Set_TxBfPlyInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT16 input[4] = {0};
	UINT8 i;
#ifndef WIFI_UNIFIED_COMMAND
	EXT_CMD_TXBF_PLY_CMD_T txbfPlyCmd;
#else
	struct UNI_CMD_BF_PLY_CMD UnitxbfPlyCmd;
#endif

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&txbfPlyCmd, sizeof(EXT_CMD_TXBF_PLY_CMD_T));

	txbfPlyCmd.ucAction = input[0];

	switch (txbfPlyCmd.ucAction) {
	case BF_PLY_READ_INFO:
		break;
	case BF_PLY_CFG_OPT:
		txbfPlyCmd.ucGloOpt = input[1];
		txbfPlyCmd.ucGrpIBfOpt = input[2];
		txbfPlyCmd.ucGrpEBfOpt = input[3];
		break;
	case BF_PLY_CFG_STA_PLY:
		txbfPlyCmd.u2WlanIdx = input[1];
		txbfPlyCmd.ucNss = input[2];
		txbfPlyCmd.ucSSPly = input[3];
		break;
	default:
		break;
	}

	TxBfPlyInfo(pAd, (PUINT8)&txbfPlyCmd);
#else
	os_zero_mem(&UnitxbfPlyCmd, sizeof(struct UNI_CMD_BF_PLY_CMD));

	UnitxbfPlyCmd.u1Action = input[0];

	switch (UnitxbfPlyCmd.u1Action) {
	case BF_PLY_READ_INFO:
		break;
	case BF_PLY_CFG_OPT:
		UnitxbfPlyCmd.u1GloOpt = input[1];
		UnitxbfPlyCmd.u1GrpIBfOpt = input[2];
		UnitxbfPlyCmd.u1GrpEBfOpt = input[3];
		break;
	case BF_PLY_CFG_STA_PLY:
		UnitxbfPlyCmd.u2WlanIdx = input[1];
		UnitxbfPlyCmd.u1Nss = input[2];
		UnitxbfPlyCmd.u1SSPly = input[3];
		break;
	default:
		break;
	}

	TxBfPlyInfo(pAd, (UINT8 *)&UnitxbfPlyCmd);
#endif
	return TRUE;
}

INT Set_TxBfTxCmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT16 input[4] = {0};
	UINT8 i;
#ifndef WIFI_UNIFIED_COMMAND
	EXT_CMD_TXBF_TXCMD_CMD_T txbfTxCmdCmd;
#else
	struct UNI_CMD_BF_TXCMD UnitxbfTxCmd;
#endif

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&txbfTxCmdCmd, sizeof(EXT_CMD_TXBF_TXCMD_CMD_T));

	txbfTxCmdCmd.ucAction = input[0];

	switch (txbfTxCmdCmd.ucAction) {
	case BF_TXCMD_READ_INFO:
		break;
	case BF_TXCMD_BF_CFG:
		txbfTxCmdCmd.fgTxCmdBfManual = input[1];
		txbfTxCmdCmd.ucTxCmdBfBit = input[2];
		break;
	default:
		break;
	}

	TxBfTxCmd(pAd, (PUINT8)&txbfTxCmdCmd);
#else
	os_zero_mem(&UnitxbfTxCmd, sizeof(struct UNI_CMD_BF_TXCMD));

	UnitxbfTxCmd.u1Action = input[0];

	switch (UnitxbfTxCmd.u1Action) {
	case BF_TXCMD_READ_INFO:
		break;
	case BF_TXCMD_BF_CFG:
		UnitxbfTxCmd.fgTxCmdBfManual = input[1];
		UnitxbfTxCmd.u1TxCmdBfBit = input[2];
		break;
	default:
		break;
	}

	TxBfTxCmd(pAd, (UINT8 *)&UnitxbfTxCmd);
#endif
	return TRUE;
}


INT Set_TxBfSndCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT16 input[4] = {0};
	UINT8 i, u1ArgCnt = 0;
#ifndef WIFI_UNIFIED_COMMAND
	EXT_CMD_TXBF_SND_CNT_CMD_T rSndCntCmd;
#else
	struct UNI_CMD_BF_SND_CNT_CMD rUniSndCntCmd;
#endif

	for (i = 0, value = rstrtok(arg, "-"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, "-"), i++) {
		input[i] = os_str_toul(value, 0, 10);
		u1ArgCnt++;
	}

	if (u1ArgCnt == 0)
		goto error;

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&rSndCntCmd, sizeof(EXT_CMD_TXBF_SND_CNT_CMD_T));

	rSndCntCmd.u1Action = input[0];

	switch (rSndCntCmd.u1Action) {
	case BF_SND_CNT_READ:
		break;

	case BF_SND_CNT_SET_LMT_MAN:
		if (u1ArgCnt < 2)
			goto error;
		rSndCntCmd.u2SndCntLmtMan= input[1];
		break;

	default:
		break;
	}

	TxBfSndCnt(pAd, (PUINT8)&rSndCntCmd);
#else
	os_zero_mem(&rUniSndCntCmd, sizeof(struct UNI_CMD_BF_SND_CNT_CMD));

	rUniSndCntCmd.u2Action = input[0];

	switch (rUniSndCntCmd.u2Action) {
	case BF_SND_CNT_READ:
		break;

	case BF_SND_CNT_SET_LMT_MAN:
		if (u1ArgCnt < 2)
			goto error;
		rUniSndCntCmd.u2SndCntLmtMan = input[1];
		break;

	default:
		break;
	}

	TxBfSndCnt(pAd, (UINT8 *)&rUniSndCntCmd);
#endif
	return TRUE;

error:
	MTWF_PRINT ("Read Info:\n");
	MTWF_PRINT ("  iwpriv ra0 set TxBfSndCnt=0\n");
	MTWF_PRINT ("Set Snd Cnt Limit Manully:\n");
	MTWF_PRINT ("  iwpriv ra0 set TxBfSndCnt=1-[1]\n");
	MTWF_PRINT ("                              [1]: Cnt Limit (0: reset default, 1-65535: Cnt Limit)\n");

	return TRUE;
}

INT Set_HeRaMuMetricInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT16 input[19] = {0};
	UINT8 i;
#ifndef WIFI_UNIFIED_COMMAND
	HERA_MU_METRIC_CMD_T MuMetricCmd;
#else
	struct UNI_CMD_HERA_MU_METRIC UniMuMetricCmd;
#endif

	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, ":"), i++)
		input[i] = os_str_toul(value, 0, 16);

#ifndef WIFI_UNIFIED_COMMAND
	os_zero_mem(&MuMetricCmd, sizeof(HERA_MU_METRIC_CMD_T));

	MuMetricCmd.u1Action = input[0];

	switch (MuMetricCmd.u1Action) {
	case HERA_METRIC_READ_INFO:
		MuMetricCmd.u1ReadClr = input[1];
		break;
	case HERA_METRIC_START_CALC:
		MuMetricCmd.u1Band = input[1];
		MuMetricCmd.u1NUser = input[2];
		MuMetricCmd.u1DBW = input[3];
		MuMetricCmd.u1NTxer = input[4];
		MuMetricCmd.u1PFD = input[5];
		MuMetricCmd.u1RuSize = input[6];
		MuMetricCmd.u1RuIdx = input[7];
		MuMetricCmd.u1SpeIdx = input[8];
		MuMetricCmd.u1SpeedUp = input[9];
		MuMetricCmd.u1LDPC = input[10];
		MuMetricCmd.u1NStsUser[0] = input[11];
		MuMetricCmd.u1NStsUser[1] = input[12];
		MuMetricCmd.u1NStsUser[2] = input[13];
		MuMetricCmd.u1NStsUser[3] = input[14];
		MuMetricCmd.u2PfidUser[0] = input[15];
		MuMetricCmd.u2PfidUser[1] = input[16];
		MuMetricCmd.u2PfidUser[2] = input[17];
		MuMetricCmd.u2PfidUser[3] = input[18];
		break;
	case HERA_METRIC_CHANGE_POLLING_TIME:
		MuMetricCmd.u1PollingTime = input[1];
	default:
		break;
	}

	HeRaMuMetricInfo(pAd, (PUINT8)&MuMetricCmd);
#else
	os_zero_mem(&UniMuMetricCmd, sizeof(struct UNI_CMD_HERA_MU_METRIC));

	UniMuMetricCmd.u1Action = input[0];

	switch (UniMuMetricCmd.u1Action) {
	case HERA_METRIC_READ_INFO:
		UniMuMetricCmd.u1ReadClr = input[1];
		break;
	case HERA_METRIC_START_CALC:
		UniMuMetricCmd.u1Band = input[1];
		UniMuMetricCmd.u1NUser = input[2];
		UniMuMetricCmd.u1DBW = input[3];
		UniMuMetricCmd.u1NTxer = input[4];
		UniMuMetricCmd.u1PFD = input[5];
		UniMuMetricCmd.u1RuSize = input[6];
		UniMuMetricCmd.u1RuIdx = input[7];
		UniMuMetricCmd.u1SpeIdx = input[8];
		UniMuMetricCmd.u1SpeedUp = input[9];
		UniMuMetricCmd.u1LDPC = input[10];
		UniMuMetricCmd.u1NStsUser[0] = input[11];
		UniMuMetricCmd.u1NStsUser[1] = input[12];
		UniMuMetricCmd.u1NStsUser[2] = input[13];
		UniMuMetricCmd.u1NStsUser[3] = input[14];
		UniMuMetricCmd.u2PfidUser[0] = input[15];
		UniMuMetricCmd.u2PfidUser[1] = input[16];
		UniMuMetricCmd.u2PfidUser[2] = input[17];
		UniMuMetricCmd.u2PfidUser[3] = input[18];
		break;
	case HERA_METRIC_CHANGE_POLLING_TIME:
		UniMuMetricCmd.u1PollingTime = input[1];
		break;
	case HERA_METRIC_SET_ENABLE:
		UniMuMetricCmd.fgEnable = input[1];
		break;
	case HERA_METRIC_SET_MU_INIT_PLY:
		UniMuMetricCmd.u1MuInitRatePly = input[1];
		break;
	default:
		break;
	}

	HeRaMuMetricInfo(pAd, (UINT8 *)&UniMuMetricCmd);
#endif
	return TRUE;
}

INT Set_TxBfProfileSwTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT    status = FALSE, rv;
	UINT32 lm, nc, nr, bw, codebook, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d-%d-%d-%d", &lm, &nr, &nc, &bw, &codebook, &group);
		if (rv != 6) {
			MTWF_PRINT("%s: cmd params are invalid!!\n", __func__);

			return FALSE;
		} else {
			if ((lm > 0) && (group < 3) && (nr < 4) && (nc < 4) && (codebook < 4)) {
				MTWF_PRINT
					("%s: Lm=%d Nr=%d Nc=%d BW=%d CodeBook=%d Group=%d\n", __func__, lm, nr, nc, bw, codebook, group);

				status = TxBfPseudoTagUpdate(pAd, lm, nr, nc, bw, codebook, group);

				if (!status) {
					MTWF_PRINT("%s: set command failed.\n", __func__);
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

INT Set_TxBfAidUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT16 u2Aid = 0;
	INT8 *value = NULL;
	UINT16 input[3] = {0};
	UINT8 i, u1BandIdx, u1OwnMacIdx;

	if (arg != NULL) {
		for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(input));
				value = rstrtok(NULL, ":"), i++)
			input[i] = os_str_toul(value, 0, 10);
	} else {
		MTWF_PRINT("%s: Argument is NULL\n", __func__);
		Ret = FALSE;
		goto error;
	}

	u2Aid = input[0];
	u1BandIdx = input[1];
	u1OwnMacIdx = input[2];

	MTWF_PRINT("%s: Band:%u, OwnMac: %u, AID:%u\n", __func__, u1BandIdx, u1OwnMacIdx, u2Aid);

	if (CmdETxBfAidSetting(pAd, u2Aid, u1BandIdx, u1OwnMacIdx))
		Ret = FALSE;
error:
	MTWF_PRINT("CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

VOID ate_set_cmm_starec(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_cmm_starec)
		ops->set_cmm_starec(pAd, arg);
}

#endif  /* TXBF_SUPPORT */
#endif  /* MT_MAC */

INT set_mec_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *value = NULL;
	UINT32 input[5] = {0};
	UINT8 i, argCnt = 0;
	CMD_MEC_CTRL_CMD_T mecCtrlCmd;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hwifi_get_chip_cap(pAd);
	struct UNI_CMD_MEC_CTRL_PARAM_T UniCmdMecParam;
#endif /* WIFI_UNIFIED_COMMAND */

	for (i = 0, value = rstrtok(arg, "-"); value && (i < ARRAY_SIZE(input));
			value = rstrtok(NULL, "-"), i++) {
		input[i] = os_str_toul(value, 0, 10);
		argCnt++;
	}

	if (argCnt == 0)
		goto error;

	if (input[0] >= MEC_CTRL_ACTION_MAX)
		goto error;

#ifdef WIFI_UNIFIED_COMMAND
	if (cap == NULL) {
		goto error;
	}
	if (cap->uni_cmd_support) {
		UINT16 u2Action = input[0];
		os_zero_mem(&UniCmdMecParam, sizeof(UniCmdMecParam));

		MTWF_PRINT("%s: u2Action=%u\n",
				__func__, u2Action);

		switch (u2Action) {
		case MEC_CTRL_ACTION_READ_INFO:
			UniCmdMecParam.mec_read_info_t.u2Tag = cpu2le16(UNI_CMD_MEC_READ_INFO);
			UniCmdMecParam.mec_read_info_t.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_read_info_t));
			UniCmdMecParam.mec_read_info_t.u2ReadType = cpu2le16(input[1]);
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_READ_INFO] = TRUE;
			MTWF_PRINT("%s: u2ReadType=%u\n", __func__, input[1]);
			break;

		case MEC_CTRL_ACTION_AMSDU_ALGO_EN_STA:
			UniCmdMecParam.mec_algo_en_sta_t.u2Tag = cpu2le16(UNI_CMD_MEC_AMSDU_ALGO_EN_STA);
			UniCmdMecParam.mec_algo_en_sta_t.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_algo_en_sta_t));
			UniCmdMecParam.mec_algo_en_sta_t.u2WlanIdx = cpu2le16(input[1]);
			UniCmdMecParam.mec_algo_en_sta_t.u1AmsduAlgoEn = input[2];
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_AMSDU_ALGO_EN_STA] = TRUE;
			MTWF_PRINT("%s: u2WlanIdx=%u, u1AmsduAlgoEn=%u\n",
						__func__, input[1], input[2]);
			break;

		case MEC_CTRL_ACTION_AMSDU_PARA_STA:
			if (argCnt != 5)
				goto error;

			UniCmdMecParam.mec_amsdu_para_sta_t.u2Tag = cpu2le16(UNI_CMD_MEC_AMSDU_PARA_STA);
			UniCmdMecParam.mec_amsdu_para_sta_t.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_amsdu_para_sta_t));
			UniCmdMecParam.mec_amsdu_para_sta_t.u2WlanIdx = cpu2le16(input[1]);
			UniCmdMecParam.mec_amsdu_para_sta_t.u1AmsduEn = input[2];
			UniCmdMecParam.mec_amsdu_para_sta_t.u1AmsduNum = input[3];
			UniCmdMecParam.mec_amsdu_para_sta_t.u2AmsduLen = cpu2le16(input[4]);
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_AMSDU_PARA_STA] = TRUE;
			MTWF_PRINT("%s: u2WlanIdx=%u, u1AmsduEn=%u, u1AmsduNum=%u, u2AmsduLen=%u\n",
				__func__, input[1], input[2], input[3], input[4]);
			break;

		case MEC_CTRL_ACTION_AMSDU_ALGO_THRESHOLD:
			if (argCnt != 4)
				goto error;

			UniCmdMecParam.mec_amsdu_algo_thr.u2Tag = cpu2le16(UNI_CMD_MEC_AMSDU_ALGO_THRESHOLD);
			UniCmdMecParam.mec_amsdu_algo_thr.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_amsdu_algo_thr));
			UniCmdMecParam.mec_amsdu_algo_thr.u1BaNum = input[1];
			UniCmdMecParam.mec_amsdu_algo_thr.u1AmsduNum = input[2];
			UniCmdMecParam.mec_amsdu_algo_thr.u2AmsduRateThr = cpu2le16(input[3]);
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_AMSDU_ALGO_THRESHOLD] = TRUE;
			MTWF_PRINT("%s: u1BaNum=%u, u1AmsduNum=%u, u2AmsduRateThr=%u\n",
				__func__, input[1], input[2], input[3]);
			break;

		case MEC_CTRL_INTF_SPEED:
			if (argCnt != 2)
				goto error;

			UniCmdMecParam.mec_ifac_speed.u2Tag = cpu2le16(UNI_CMD_MEC_IFAC_SPEED);
			UniCmdMecParam.mec_ifac_speed.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_ifac_speed));
			UniCmdMecParam.mec_ifac_speed.u4IfacSpeed = cpu2le32(input[1]);
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_IFAC_SPEED] = TRUE;
			MTWF_PRINT("%s: u4InterfacSpeed=%u\n", __func__, input[1]);
			break;

		case MEC_CTRL_ACTION_AMSDU_MAX_LEN:
			if (argCnt != 2)
				goto error;

			UniCmdMecParam.mec_set_amsdu_max_size_t.u2Tag = cpu2le16(UNI_CMD_MEC_CTRL_ACTION_AMSDU_MAX_LEN);
			UniCmdMecParam.mec_set_amsdu_max_size_t.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_set_amsdu_max_size_t));
			UniCmdMecParam.mec_set_amsdu_max_size_t.u2WlanIdx = input[1];
			UniCmdMecParam.MecTagValid[UNI_CMD_MEC_CTRL_ACTION_AMSDU_MAX_LEN] = TRUE;
			MTWF_PRINT("%s: u2WlanIdx=%u\n",
				__func__, UniCmdMecParam.mec_set_amsdu_max_size_t.u2WlanIdx);
			break;

		default:
			goto error;
		}

		UniCmdMecCtrl(pAd, &UniCmdMecParam);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		os_zero_mem(&mecCtrlCmd, sizeof(CMD_MEC_CTRL_CMD_T));

		mecCtrlCmd.u2Action = input[0];
		MTWF_PRINT("%s: u2Action=%u\n",
				__func__, mecCtrlCmd.u2Action);

		switch (mecCtrlCmd.u2Action) {
		case MEC_CTRL_ACTION_READ_INFO:
			mecCtrlCmd.mecCmdPara.mec_read_info_t.u2ReadType = input[1];
			MTWF_PRINT("%s: u2ReadType=%u\n",
					__func__, mecCtrlCmd.mecCmdPara.mec_read_info_t.u2ReadType);

			break;

		case MEC_CTRL_ACTION_AMSDU_ALGO_EN_STA:
			if (argCnt != 3)
				goto error;
			mecCtrlCmd.mecCmdPara.mec_algo_en_sta_t.u2WlanIdx = input[1];
			mecCtrlCmd.mecCmdPara.mec_algo_en_sta_t.u1AmsduAlgoEn = input[2];
			MTWF_PRINT("%s: u2WlanIdx=%u, u1AmsduAlgoEn=%u\n",
					__func__,
					mecCtrlCmd.mecCmdPara.mec_algo_en_sta_t.u2WlanIdx,
					mecCtrlCmd.mecCmdPara.mec_algo_en_sta_t.u1AmsduAlgoEn);
			break;

		case MEC_CTRL_ACTION_AMSDU_PARA_STA:
			if (argCnt != 5)
				goto error;

			mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u2WlanIdx = input[1];
			mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u1AmsduEn = input[2];
			mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u1AmsduNum = input[3];
			mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u2AmsduLen = input[4];
			MTWF_PRINT
				    ("%s: u2WlanIdx=%u, u1AmsduEn=%u, u1AmsduNum=%u, u2AmsduLen=%u\n",
					__func__,
					mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u2WlanIdx,
					mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u1AmsduEn,
					mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u1AmsduNum,
					mecCtrlCmd.mecCmdPara.mec_amsdu_para_sta_t.u2AmsduLen);
			break;

		case MEC_CTRL_ACTION_AMSDU_ALGO_THRESHOLD:
			if (argCnt != 4)
				goto error;

			mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u1BaNum = input[1];
			mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u1AmsduNum = input[2];
			mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u2AmsduRateThr = input[3];
			MTWF_PRINT("%s: u1BaNum=%u, u1AmsduNum=%u, u2AmsduRateThr=%u\n",
					__func__,
					mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u1BaNum,
					mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u1AmsduNum,
					mecCtrlCmd.mecCmdPara.mec_amsdu_algo_thr.u2AmsduRateThr);
			break;

		case MEC_CTRL_INTF_SPEED:
			if (argCnt != 2)
				goto error;

			mecCtrlCmd.mecCmdPara.mec_ifac_speed.u4InterfacSpeed = input[1];
			MTWF_PRINT("%s: u4InterfacSpeed=%u\n",
					__func__, mecCtrlCmd.mecCmdPara.mec_ifac_speed.u4InterfacSpeed);
			break;

		case MEC_CTRL_ACTION_AMSDU_MAX_LEN:
			if (argCnt != 2)
				goto error;

			mecCtrlCmd.mecCmdPara.mec_set_amsdu_max_size_t.u2WlanIdx = input[1];
			MTWF_PRINT("%s: u2WlanIdx=%u\n",
				__func__, mecCtrlCmd.mecCmdPara.mec_set_amsdu_max_size_t.u2WlanIdx);
			break;

		default:
			goto error;
		}

		CmdMecCtrl(pAd, (PUINT8)&mecCtrlCmd);
	}

	return TRUE;

error:
	MTWF_PRINT ("Wrong Cmd Format. Plz input:\n");
	MTWF_PRINT ("iwpriv ra0 set mec_ctrl=[0]-[1]-[2]-[3]-[4]\n");
	MTWF_PRINT ("  [0]=0: Read Info: =0-[1]\n");
	MTWF_PRINT ("                 , [1]: Read Type (Optional, 0 for all, BIT(1) for Algo En, BIT(2) for Algo Thr)\n");
	MTWF_PRINT ("  [0]=1: Set AMSDU Algo Enable: =1-[1]-[2] \n");
	MTWF_PRINT ("                 , [1]: Wlan Idx (65535 for all)\n");
	MTWF_PRINT ("                 , [2]: Enable (0, 1 for Disable, Enable)\n");
	MTWF_PRINT ("  [0]=2: Set WTBL AMSDU Parameters: =2-[1]-[2]-[3]-[4]\n");
	MTWF_PRINT ("                 , [1]: Wlan Idx (65535 for all)\n");
	MTWF_PRINT ("                 , [2]: Enable (0, 1 for Disable, Enable)\n");
	MTWF_PRINT ("                 , [3]: Number (0-7 for AMSDU Num 1-8)\n");
	MTWF_PRINT ("                 , [4]: Length (The unit is Bytes)\n");
	MTWF_PRINT ("  [0]=3: Set PHY Rate Threshold of AMSDU Length: =3-[1]-[2]-[3]\n");
	MTWF_PRINT ("                 , [1]: BA Numer (0, 1 for BA 64, 256)\n");
	MTWF_PRINT ("                 , [2]: Amsdu Number (1-3 for AMSDU Num 2-4)\n");
	MTWF_PRINT ("                 , [3]: Threshold of PHY Rate (1-65535, unit: Mbps, 0 for reset default)\n");
	MTWF_PRINT ("                 e.g. =3-1-300 means if current PHY rate > 300Mbps, then set AMSDU Len of 2 num\n");
	MTWF_PRINT("  [0]=5: Set AMSDU len as STA's max MPDU size: =5-[1]\n");
	MTWF_PRINT("                 , [1]: Wlan Idx (65535 for all)\n");
	return FALSE;
}

#ifdef DOT11_N_SUPPORT
void assoc_ht_info_debugshow(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN HT_CAPABILITY_IE * pHTCapability)
{
	HT_CAP_INFO			*pHTCap;
	HT_CAP_PARM		*pHTCapParm;
	EXT_HT_CAP_INFO		*pExtHT;
#ifdef TXBF_SUPPORT
	HT_BF_CAP			*pBFCap;
#endif /* TXBF_SUPPORT */

	if (pHTCapability) {
		pHTCap = &pHTCapability->HtCapInfo;
		pHTCapParm = &pHTCapability->HtCapParm;
		pExtHT = &pHTCapability->ExtHtCapInfo;
#ifdef TXBF_SUPPORT
		pBFCap = &pHTCapability->TxBFCap;
#endif /* TXBF_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "Peer - 11n HT Info\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tHT Cap Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t HT_RX_LDPC(%d), BW(%d), MIMOPS(%d), GF(%d), ShortGI_20(%d), ShortGI_40(%d)\n",
				  pHTCap->ht_rx_ldpc, pHTCap->ChannelWidth, pHTCap->MimoPs, pHTCap->GF,
				  pHTCap->ShortGIfor20, pHTCap->ShortGIfor40);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t TxSTBC(%d), RxSTBC(%d), DelayedBA(%d), A-MSDU(%d), CCK_40(%d)\n",
				  pHTCap->TxSTBC, pHTCap->RxSTBC, pHTCap->DelayedBA, pHTCap->AMsduSize, pHTCap->CCKmodein40);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"\t\t PSMP(%d), Forty_Mhz_Intolerant(%d), L-SIG(%d)\n",
				 pHTCap->PSMP, pHTCap->Forty_Mhz_Intolerant, pHTCap->LSIGTxopProSup);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tHT Parm Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"\t\t MaxRx A-MPDU Factor(%d), MPDU Density(%d)\n",
				 pHTCapParm->MaxRAmpduFactor, pHTCapParm->MpduDensity);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tHT MCS set:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t RxMCS(%02x %02x %02x %02x %02x) MaxRxMbps(%d) TxMCSSetDef(%02x)\n",
				  pHTCapability->MCSSet[0], pHTCapability->MCSSet[1], pHTCapability->MCSSet[2],
				  pHTCapability->MCSSet[3], pHTCapability->MCSSet[4],
				  (pHTCapability->MCSSet[11] << 8) + pHTCapability->MCSSet[10],
				  pHTCapability->MCSSet[12]);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tExt HT Cap Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t PCO(%d), TransTime(%d), MCSFeedback(%d), +HTC(%d), RDG(%d)\n",
				  pExtHT->Pco, pExtHT->TranTime, pExtHT->MCSFeedback, pExtHT->PlusHTC, pExtHT->RDGSupport);
#ifdef TXBF_SUPPORT
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tTX BF Cap:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t ImpRxCap(%d), RXStagSnd(%d), TXStagSnd(%d), RxNDP(%d), TxNDP(%d) ImpTxCap(%d)\n",
				  pBFCap->TxBFRecCapable, pBFCap->RxSoundCapable, pBFCap->TxSoundCapable,
				  pBFCap->RxNDPCapable, pBFCap->TxNDPCapable, pBFCap->ImpTxBFCapable);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t Calibration(%d), ExpCSICapable(%d), ExpComSteerCapable(%d), ExpCSIFbk(%d), ExpNoComBF(%d) ExpComBF(%d)\n",
				  pBFCap->Calibration, pBFCap->ExpCSICapable, pBFCap->ExpComSteerCapable,
				  pBFCap->ExpCSIFbk, pBFCap->ExpNoComBF, pBFCap->ExpComBF);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\t\t MinGrouping(%d), CSIBFAntSup(%d), NoComSteerBFAntSup(%d), ComSteerBFAntSup(%d), CSIRowBFSup(%d) ChanEstimation(%d)\n",
				  pBFCap->MinGrouping, pBFCap->CSIBFAntSup, pBFCap->NoComSteerBFAntSup,
				  pBFCap->ComSteerBFAntSup, pBFCap->CSIRowBFSup, pBFCap->ChanEstimation);
#endif /* TXBF_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
				 "\nPeer - MODE=%d, BW=%d, MCS=%d, ShortGI=%d, MaxRxFactor=%d, MpduDensity=%d, MIMOPS=%d, AMSDU=%d\n",
				  pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.BW,
				  pEntry->HTPhyMode.field.MCS, pEntry->HTPhyMode.field.ShortGI,
				  pEntry->MaxRAmpduFactor, pEntry->MpduDensity,
				  pEntry->MmpsMode, pEntry->AMsduSize);
#ifdef DOT11N_DRAFT3
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO, "\tExt Cap Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
			"\t\tBss2040CoexistMgmt=%d\n",
			pEntry->BSS2040CoexistenceMgmtSupport);
#endif /* DOT11N_DRAFT3 */
	}
}

#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
VOID peer_assoc_vht_info_debugshow(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies)
{
	if ((pAd == NULL) || (peer == NULL) || (cmm_ies == NULL)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_ERROR,
			"!!ERROR!! NULL Pointer\n");
		return;
	}

	assoc_vht_info_debugshow(pAd, peer, &cmm_ies->vht_cap, &cmm_ies->vht_op);
}
VOID assoc_vht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN VHT_CAP_IE * vht_cap,
	IN VHT_OP_IE * vht_op)
{
	VHT_CAP_INFO *cap_info;
	VHT_MCS_SET *mcs_set;
	struct vht_opinfo *op_info;
	VHT_MCS_MAP *mcs_map;
	struct wifi_dev *wdev = pEntry->wdev;
	USHORT PhyMode = wdev->PhyMode;

	if (!WMODE_CAP_AC(PhyMode))
		return;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "Peer - 11AC VHT Info\n");

	if (vht_cap) {
		cap_info = &vht_cap->vht_cap;
		mcs_set = &vht_cap->mcs_set;
		hex_dump("peer vht_cap raw data", (UCHAR *)cap_info, sizeof(VHT_CAP_INFO));
		hex_dump("peer vht_mcs raw data", (UCHAR *)mcs_set, sizeof(VHT_MCS_SET));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\tVHT Cap Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO,
				 "\t\tMaxMpduLen(%d), BW(%d), SGI_80M(%d), RxLDPC(%d), TxSTBC(%d), RxSTBC(%d), +HTC-VHT(%d)\n",
				  cap_info->max_mpdu_len, cap_info->ch_width, cap_info->sgi_80M, cap_info->rx_ldpc, cap_info->tx_stbc,
				  cap_info->rx_stbc, cap_info->htc_vht_cap);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO,
				 "\t\tMaxAmpduExp(%d), VhtLinkAdapt(%d), RxAntConsist(%d), TxAntConsist(%d)\n",
				  cap_info->max_ampdu_exp, cap_info->vht_link_adapt, cap_info->rx_ant_consistency, cap_info->tx_ant_consistency);
		mcs_map = &mcs_set->rx_mcs_map;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\t\tRxMcsSet: HighRate(%d), RxMCSMap(%d,%d,%d,%d,%d,%d,%d)\n",
				 mcs_set->rx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				 mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7);
		mcs_map = &mcs_set->tx_mcs_map;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\t\tTxMcsSet: HighRate(%d), TxMcsMap(%d,%d,%d,%d,%d,%d,%d)\n",
				 mcs_set->tx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				 mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7);
#ifdef VHT_TXBF_SUPPORT
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\t\tETxBfCap: Bfer(%d), Bfee(%d), SndDim(%d)\n",
				 cap_info->bfer_cap_su, cap_info->bfee_cap_su, cap_info->num_snd_dimension);
#endif
	}

	if (vht_op) {
		op_info = &vht_op->vht_op_info;
		mcs_map = &vht_op->basic_mcs_set;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\tVHT OP Info:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\t\tChannel Width(%d), CenteralFreq1(%d), CenteralFreq2(%d)\n",
				 op_info->ch_width, op_info->ccfs_0, op_info->ccfs_1);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO,
				 "\t\tBasicMCSSet(SS1:%d, SS2:%d, SS3:%d, SS4:%d, SS5:%d, SS6:%d, SS7:%d)\n",
				  mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
				  mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6,
				  mcs_map->mcs_ss7);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_INFO, "\n");
}
#endif /* DOT11_VHT_AC */

INT Set_RateAdaptInterval(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_time, ra_qtime;
	RTMP_STRING *token;
	char sep = ':';
	ULONG irqFlags;
	/*
		The ra_interval inupt string format should be d:d, in units of ms.
			=>The first decimal number indicates the rate adaptation checking period,
			=>The second decimal number indicates the rate adaptation quick response checking period.
	*/
	MTWF_PRINT("%s():%s\n", __func__, arg);
	token = strchr(arg, sep);

	if (token != NULL) {
		*token = '\0';

		if (strlen(arg) && strlen(token + 1)) {
			ra_time = os_str_tol(arg, 0, 10);
			ra_qtime = os_str_tol(token + 1, 0, 10);
			MTWF_PRINT ("%s():Set RateAdaptation TimeInterval as(%d:%d) ms\n",
					 __func__, ra_time, ra_qtime);
			RTMP_IRQ_LOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), irqFlags);
			pAd->ra_interval = ra_time;
			pAd->ra_fast_interval = ra_qtime;
#ifdef CONFIG_AP_SUPPORT

			if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE) {
				BOOLEAN Cancelled;

				RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
			}

#endif /* CONFIG_AP_SUPPORT  */
			RTMP_IRQ_UNLOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), irqFlags);
			return TRUE;
		}
	}

	return FALSE;
}

#ifdef SNIFFER_RADIOTAP_SUPPORT
INT Set_SnifferBox_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	struct wifi_dev *wdev;
	UCHAR       apidx;
	INT enable = 0;
	UCHAR band_idx = BAND0;
#if defined(WHNAT_SUPPORT) || defined(WIFI_UNIFIED_COMMAND)
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	struct _EXT_CMD_SNIFFER_MODE_T SnifferFWCmd = {0};


	enable = os_str_tol(arg, 0, 10);
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	if (apidx < MAX_BEACON_NUM)
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	else {
			MTWF_PRINT("%s: invalid apidx=%d.\n", __func__, apidx);
			return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);

	if (enable == 1) {
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
		pAd->monitor_ctrl.bMonitorOn = TRUE;

#ifdef WHNAT_SUPPORT
		if (PD_GET_WHNAT_ENABLE(pAd->physical_dev) && (cap->tkn_info.feature & TOKEN_RX))
			mt_cmd_wo_query(pAd, WO_CMD_SET_CAP, 0x1, 0);
#endif

		SnifferFWCmd.ucDbdcIdx = band_idx;
		SnifferFWCmd.ucSnifferEn = 1;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSetSnifferMode(pAd, SnifferFWCmd);
#endif /* WIFI_UNIFIED_COMMAND */

		RTMP_OS_NETDEV_SET_TYPE(wdev->if_dev, ARPHRD_IEEE80211_RADIOTAP);

		MTWF_PRINT("Enable Sniffer Box~~\n");

	} else {
		SnifferFWCmd.ucDbdcIdx = band_idx;
		SnifferFWCmd.ucSnifferEn = 0;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSetSnifferMode(pAd, SnifferFWCmd);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WHNAT_SUPPORT
		if (PD_GET_WHNAT_ENABLE(pAd->physical_dev) && (cap->tkn_info.feature & TOKEN_RX))
			mt_cmd_wo_query(pAd, WO_CMD_SET_CAP, 0x0, 0);
#endif

		RTMP_OS_NETDEV_SET_TYPE(wdev->if_dev, ARPHRD_ETHER);
		pAd->monitor_ctrl.bMonitorOn = FALSE;
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));

		MTWF_PRINT("Disable Sniffer Box\n");
	}

return TRUE;

}
#endif

#ifdef SNIFFER_SUPPORT
INT Set_MonitorMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_SNIFFER_MODE_T SnifferFWCmd;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	POS_COOKIE pObj;
	struct wifi_dev *wdev;
	UINT32       apidx;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	pAd->monitor_ctrl.CurrentMonitorMode = os_str_tol(arg, 0, 10);
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	if (VALID_MBSS(pAd, apidx))
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	else {
		MTWF_PRINT("%s: invalid apidx = %d.\n", __func__, apidx);
		return FALSE;
	}

	SnifferFWCmd.ucDbdcIdx = 0;

	if (pAd->monitor_ctrl.CurrentMonitorMode > MONITOR_MODE_FULL || pAd->monitor_ctrl.CurrentMonitorMode < MONITOR_MODE_OFF)
		pAd->monitor_ctrl.CurrentMonitorMode = MONITOR_MODE_OFF;

	MTWF_PRINT("set Current Monitor Mode = %d , range(%d ~ %d)\n"
			  , pAd->monitor_ctrl.CurrentMonitorMode, MONITOR_MODE_OFF, MONITOR_MODE_FULL);

	switch (pAd->monitor_ctrl.CurrentMonitorMode) {
	case MONITOR_MODE_OFF:			/* reset to normal */
		pAd->monitor_ctrl.bMonitorOn = FALSE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 0;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSetSnifferMode(pAd, SnifferFWCmd);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;

	case MONITOR_MODE_REGULAR_RX:			/* report probe_request only , normal rx filter */
		pAd->monitor_ctrl.bMonitorOn = TRUE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 1;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSetSnifferMode(pAd, SnifferFWCmd);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;

	case MONITOR_MODE_FULL:			/* fully report, Enable Rx with promiscuous reception */
		pAd->monitor_ctrl.bMonitorOn = TRUE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucSnifferEn = 1;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSetSnifferMode(pAd, SnifferFWCmd);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		break;
	}

	return TRUE;
}

INT Set_MonitorFilterSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.FilterSize = os_str_tol(arg, 0, 10);
	if (pAd->monitor_ctrl.FilterSize < sizeof(struct mtk_radiotap_header))
		pAd->monitor_ctrl.FilterSize = RX_BUFFER_SIZE_MIN + sizeof(struct mtk_radiotap_header);
	return TRUE;
}

INT Set_MonitorFrameType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.FrameType = os_str_tol(arg, 0, 10);
	if (pAd->monitor_ctrl.FrameType > FC_TYPE_DATA)
		pAd->monitor_ctrl.FrameType = FC_TYPE_RSVED;
	return TRUE;
}

INT Set_MonitorMacFilter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ret = TRUE;
	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	INT idx = 0;

	MTWF_PRINT("--> %s()\n", __func__);

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_PRINT("An unnecessary delimiter entered!\n");
			continue;
		}
		/* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
		if (strlen(this_char) != 17) {
			MTWF_PRINT("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n");
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value && (idx < MAC_ADDR_LEN);
				value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_PRINT("illegal MAC address format or octet!\n");
				break;
			}

			AtoH(value, &pAd->monitor_ctrl.MacFilterAddr[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	MTWF_PRINT(MACSTR, MAC2STR(pAd->monitor_ctrl.MacFilterAddr));

	pAd->monitor_ctrl.MacFilterOn = TRUE;
	MTWF_PRINT("\n");
	MTWF_PRINT("<-- %s()\n", __func__);
	return ret;
}

INT Set_MonitorMacFilterOff_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->monitor_ctrl.MacFilterOn = FALSE;
	return TRUE;
}

#endif /* SNIFFER_SUPPORT */

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 Value;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_PRINT("Do NOT accept this command after interface is up.\n");
		return FALSE;
	}

	Value = (UINT16)os_str_tol(arg, 0, 10);
	pAd->CommonCfg.ModuleTxpower = Value;
	MTWF_PRINT("IF Set_ModuleTxpower_Proc::(ModuleTxpower=%d)\n",
			 pAd->CommonCfg.ModuleTxpower);
	return TRUE;
}
#endif /* SINGLE_SKU */


#ifdef DOT11_N_SUPPORT

#define MAX_AGG_CNT	8

/* DisplayTxAgg - display Aggregation statistics from MAC */
void DisplayTxAgg(RTMP_ADAPTER *pAd)
{
	ULONG totalCount;
	ULONG aggCnt[MAX_AGG_CNT + 2];
	int i;

	AsicReadAggCnt(pAd, aggCnt, sizeof(aggCnt) / sizeof(ULONG));
	totalCount = aggCnt[0] + aggCnt[1];

	if (totalCount > 0)
		for (i = 0; i < MAX_AGG_CNT; i++)
			MTWF_PRINT ("\t%d MPDU=%ld (%ld%%)\n", i + 1, aggCnt[i + 2],
					 aggCnt[i + 2] * 100 / totalCount);

	MTWF_PRINT("====================\n");
}
#endif /* DOT11_N_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
INT Set_ReduceAckEnable_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	if (pParam == NULL)
		return FALSE;

	ReduceAckSetEnable(pAdapter, os_str_tol(pParam, 0, 10));
	return TRUE;
}

INT Show_ReduceAckInfo_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	ReduceAckShow(pAdapter);
	return TRUE;
}

INT Set_ReduceAckProb_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam)
{
	if (pParam == NULL)
		return FALSE;

	ReduceAckSetProbability(pAdapter, os_str_tol(pParam, 0, 10));
	return TRUE;
}
#endif


static INT32 SetI2cEeprom(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef WIFI_UNIFIED_COMMAND
	UCHAR *param_ptr = NULL;
	UCHAR param_idx = 0, ret = 0;
	UINT32 rf_param[2] = {0}; /* rfidx, offset, value */
	UINT8 eeprom_value_set;
	UINT8 eeprom_value = 0;
	UINT isVaild = 0;

	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (Arg) {
		for (param_idx = 0, param_ptr = rstrtok(Arg, "-");
			param_ptr && (param_idx < ARRAY_SIZE(rf_param));
			param_ptr = rstrtok(NULL, "-"), param_idx++) {
			ret = sscanf(param_ptr, "%8x", &rf_param[param_idx]);

			if (ret == 0) {
				MTWF_PRINT(" invalid format(%s), ignored!\n", param_ptr);
				goto err_out;
			}
		}
		MTWF_PRINT("Offset = 0x%08x, Value = 0x%08x\n", rf_param[0], rf_param[1]);
	}

	if (param_idx < 3) {
		if (param_idx == 1) {
			if (cap->uni_cmd_support)
				UniCmdEepromAccessRead(pAd, rf_param[0], &eeprom_value, &isVaild, 1);
			MTWF_PRINT("%s: read[0x%08x]=0x%08x\n", __func__, rf_param[0], eeprom_value);
		} else if (param_idx == 2) {
			eeprom_value_set = (UINT8)rf_param[1];
			if (cap->uni_cmd_support)
				UniCmdEepromAccessWrite(pAd, rf_param[0], &eeprom_value_set, 1);
			if (cap->uni_cmd_support)
				UniCmdEepromAccessRead(pAd, rf_param[0], &eeprom_value, &isVaild, 1);
			MTWF_PRINT("%s: write[0x%08x] [0x%08x]=0x%08x\n", __func__, rf_param[0], eeprom_value_set, eeprom_value);
		}
	} else
		MTWF_PRINT(" incorrect format(%s)\n", Arg);
#endif /* WIFI_UNIFIED_COMMAND */

err_out:
	return 0;
}

#ifdef MT_MAC
static INT32 SetMTRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR *param_ptr = NULL;
	UCHAR param_idx = 0, ret = 0;
	UINT32 rf_param[3] = {0}; /* rfidx, offset, value */
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (Arg) {
		for (param_idx = 0, param_ptr = rstrtok(Arg, "-"); param_ptr && (param_idx < ARRAY_SIZE(rf_param));	 param_ptr = rstrtok(NULL, "-"), param_idx++) {
			ret = sscanf(param_ptr, "%8x", &rf_param[param_idx]);

			if (ret == 0) {
				MTWF_PRINT(" invalid format(%s), ignored!\n", param_ptr);
				goto err_out;
			}
		}
		MTWF_PRINT("RfIdx = %d, Offset = 0x%08x, Value = 0x%08x\n",
			rf_param[0], rf_param[1], rf_param[2]);
	}

	if (param_idx < 4) {
		if (param_idx == 2) {
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdRFRegAccessRead(pAd, rf_param[0], rf_param[1], &rf_param[2]);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdRFRegAccessRead(pAd, rf_param[0], rf_param[1], &rf_param[2]);
			MTWF_PRINT ("%s:%d read[0x%08x]=0x%08x\n", __func__, rf_param[0], rf_param[1], rf_param[2]);
		} else if (param_idx == 3) {
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdRFRegAccessWrite(pAd, rf_param[0], rf_param[1], rf_param[2]);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdRFRegAccessWrite(pAd, rf_param[0], rf_param[1], rf_param[2]);
			rf_param[2] = 0;
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdRFRegAccessRead(pAd, rf_param[0], rf_param[1], &rf_param[2]);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdRFRegAccessRead(pAd, rf_param[0], rf_param[1], &rf_param[2]);
			MTWF_PRINT("%s:%d write[0x%08x]=0x%08x\n", __func__,
				rf_param[0], rf_param[1], rf_param[2]);
		}
	} else
		MTWF_PRINT(" incorrect format(%s)\n", Arg);

err_out:
	return 0;
}
#endif

INT32 SetEeprom(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	Ret = SetI2cEeprom(pAd, Arg);
	return Ret;
}

INT32 SetRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
#ifdef MT_MAC
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->rf_type == RF_MT)
		Ret = SetMTRF(pAd, Arg);

#endif
	return Ret;
}

static struct {
	RTMP_STRING *name;
	INT (*show_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, ULONG BufLen);
} *PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC, RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC[] = {
#ifdef DBG
	{"SSID",					Show_SSID_Proc},
	{"WirelessMode",			Show_WirelessMode_Proc},
	{"TxBurst",					Show_TxBurst_Proc},
	{"TxPreamble",				Show_TxPreamble_Proc},
	{"TxPower",					Show_TxPower_Proc},
	{"Channel",					Show_Channel_Proc},
	{"BGProtection",			Show_BGProtection_Proc},
	{"RTSThreshold",			Show_RTSThreshold_Proc},
	{"FragThreshold",			Show_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"HtBw",					Show_HtBw_Proc},
	{"HtMcs",					Show_HtMcs_Proc},
	{"HtGi",					Show_HtGi_Proc},
	{"HtOpMode",				Show_HtOpMode_Proc},
	{"HtExtcha",				Show_HtExtcha_Proc},
	{"HtMpduDensity",			Show_HtMpduDensity_Proc},
	{"HtBaWinSize",		        Show_HtBaWinSize_Proc},
	{"HtRdg",			Show_HtRdg_Proc},
	{"HtAmsdu",			Show_HtAmsdu_Proc},
	{"HtAutoBa",		        Show_HtAutoBa_Proc},
#endif /* DOT11_N_SUPPORT */
	{"CountryRegion",			Show_CountryRegion_Proc},
	{"CountryRegionABand",		Show_CountryRegionABand_Proc},
	{"CountryCode",				Show_CountryCode_Proc},
	{"WmmCapable",				Show_WmmCapable_Proc},

	{"IEEE80211H",				Show_IEEE80211H_Proc},
#ifdef CONFIG_STA_SUPPORT
	{"NetworkType",				Show_NetworkType_Proc},
#ifdef WSC_STA_SUPPORT
	{"WpsApBand",				Show_WpsPbcBand_Proc},
	{"Manufacturer",			Show_WpsManufacturer_Proc},
	{"ModelName",				Show_WpsModelName_Proc},
	{"DeviceName",				Show_WpsDeviceName_Proc},
	{"ModelNumber",				Show_WpsModelNumber_Proc},
	{"SerialNumber",			Show_WpsSerialNumber_Proc},
#endif /* WSC_STA_SUPPORT */
	{"WPAPSK",					Show_WPAPSK_Proc},
	{"AutoReconnect",			Show_AutoReconnect_Proc},
	{"secinfo",				Show_STASecurityInfo_Proc},
#endif /* CONFIG_STA_SUPPORT */
#ifdef SINGLE_SKU
	{"ModuleTxpower",			Show_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */
#endif /* DBG */
	{"rainfo",					Show_STA_RAInfo_Proc},
	{NULL, NULL}
};

INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *pName,
	IN	RTMP_STRING *pBuf,
	IN	UINT32			MaxLen)
{
	INT	Status = 0;
	int ret, left_buf_size;

	for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC;
		 PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++) {
		if (!strcmp(pName, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name)) {
			if (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->show_proc(pAd, pBuf, MaxLen))
				Status = -EINVAL;

			break;  /*Exit for loop.*/
		}
	}

	if (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name == NULL) {
		ret = snprintf(pBuf, MaxLen, "\n");
		if (os_snprintf_error(MaxLen, ret)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			Status = -EINVAL;
		}

		for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC;
			 PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++) {
			if ((strlen(pBuf) + strlen(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name)) >= MaxLen)
				break;

			left_buf_size = MaxLen - strlen(pBuf);
			ret = snprintf(pBuf + strlen(pBuf), left_buf_size, "%s\n", PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
				Status = -EINVAL;
			}
		}
	}

	return Status;
}

#define WIFI_INTERRUPT_NUM_MAX  1

INT32 ShowWifiInterruptCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	const UCHAR WifiIntMaxNum = WIFI_INTERRUPT_NUM_MAX;
	const CHAR WifiIntDesc[WIFI_INTERRUPT_NUM_MAX][32] = {"Wifi Abnormal counter"};
	UINT32 WifiIntCnt[WIFI_INTERRUPT_NUM_MAX];
	UINT32 WifiIntMask = 0xF;
	UCHAR BandIdx;
	UINT32 WifiIntIdx;

	os_zero_mem(WifiIntCnt, sizeof(WifiIntCnt));

	for (BandIdx = 0; BandIdx < CFG_WIFI_RAM_BAND_NUM; BandIdx++) {
		MtCmdGetWifiInterruptCnt(pAd, BandIdx, WifiIntMaxNum, WifiIntMask, WifiIntCnt);

		for (WifiIntIdx = 0; WifiIntIdx < WifiIntMaxNum; WifiIntIdx++)
			MTWF_PRINT ("Band %u:%s = %u\n", BandIdx, WifiIntDesc[WifiIntIdx],
					 WifiIntCnt[WifiIntIdx]);
	}

	return TRUE;
}

#ifdef BACKGROUND_SCAN_SUPPORT
INT set_background_scan(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = BAND0;
	UINT8 BgndscanType = os_str_tol(arg, 0, 10);
	UINT8 bgnd_band_scan_info = 0;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	} else
		return FALSE;

	if (!wdev)
		return FALSE;

	band_idx = HcGetBandByWdev(wdev);

	/* Bit[7:4]: band_idx, Bit[3:0]: BgndscanType*/
	bgnd_band_scan_info |= BgndscanType;
	bgnd_band_scan_info |= (band_idx << BGND_BAND_IDX_SHFT);

	BackgroundScanStart(pAd, wdev, bgnd_band_scan_info);
	return TRUE;
}

#if (RDD_2_SUPPORTED == 1)
INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ipith = 0;
	INT32 Recv = 0;

	Recv = sscanf(arg, "%d", &(ipith));

	if (Recv != 1) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Format Error!\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				 "iwpriv ra0 set bgndscancfg=[IPI_TH]\n");
	} else {
		pAd->BgndScanCtrl.ipi_th = ipith;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ipith = %d\n", ipith);
	}

	return TRUE;
}

#else
INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32	bgndscanduration = 0; /* ms */
	UINT32	bgndscaninterval = 0; /* second */
	UINT32	bgndscannoisyth = 0;
	UINT32	bgndscanchbusyth = 0;
	UINT32	DriverTrigger = 0;
	UINT32	bgndscansupport = 0;
	UINT32	ipith = 0;
	INT32	Recv = 0;

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d", &(bgndscanduration), &(bgndscaninterval), &(bgndscannoisyth),
				  &(bgndscanchbusyth), &(ipith), &(DriverTrigger), &(bgndscansupport));

	if (Recv != 7) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Format Error!\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				 "iwpriv ra0 set bgndscancfg=[Scan_Duration]-[Partial_Scan_Interval]-[Noisy_TH]-[BusyTime_TH]-[IPI_TH]-[Driver_trigger_Support]-[BGND_Support]\n");
	} else {
		pAd->BgndScanCtrl.ScanDuration = bgndscanduration;
		pAd->BgndScanCtrl.PartialScanInterval = bgndscaninterval;
		pAd->BgndScanCtrl.NoisyTH = bgndscannoisyth;
		pAd->BgndScanCtrl.ChBusyTimeTH = bgndscanchbusyth;
		pAd->BgndScanCtrl.DriverTrigger = (BOOL)DriverTrigger;
		pAd->BgndScanCtrl.BgndScanSupport = (BOOL)bgndscansupport;
		pAd->BgndScanCtrl.IPIIdleTimeTH = (BOOL)ipith;
	}

	return TRUE;
}
#endif

INT set_background_scan_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	MT_BGND_SCAN_CFG BgndScanCfg;

	os_zero_mem(&BgndScanCfg, sizeof(MT_BGND_SCAN_CFG));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* ControlChannel */
			BgndScanCfg.ControlChannel = os_str_tol(value, 0, 10);
			break;

		case 1: /*  CentralChannel */
			BgndScanCfg.CentralChannel = os_str_tol(value, 0, 10);
			break;

		case 2: /* BW */
			BgndScanCfg.Bw = os_str_tol(value, 0, 10);
			break;

		case 3: /* TxStream */
			BgndScanCfg.TxStream = os_str_tol(value, 0, 10);
			break;

		case 4: /* RxPath */
			BgndScanCfg.RxPath = os_str_tol(value, 0, 16);
			break;

		case 5: /* Reason */
			BgndScanCfg.Reason = os_str_tol(value, 0, 10);
			break;

		case 6: /* BandIdx */
			BgndScanCfg.BandIdx = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			 "%s  Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			  __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			  BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath);
	BackgroundScanTest(pAd, BgndScanCfg);
	return TRUE;
}
INT set_background_scan_notify(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *value = 0;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	int i;

	os_zero_mem(&BgScNotify, sizeof(MT_BGND_SCAN_NOTIFY));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* Notify function */
			BgScNotify.NotifyFunc = os_str_tol(value, 0, 10);
			break;

		case 1: /*  Status */
			BgScNotify.BgndScanStatus = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%s  NotifyFunc=%d, BgndScanStatus=%d\n",
		__func__, BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus);
	MtCmdBgndScanNotify(pAd, BgScNotify);
	return TRUE;
}

INT show_background_scan_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT(" Background scan support = %d\n", pAd->BgndScanCtrl.BgndScanSupport);

	if (pAd->BgndScanCtrl.BgndScanSupport == TRUE) {
		MTWF_PRINT("===== Configuration =====\n");
		MTWF_PRINT(" Channel busy time Threshold = %d\n", pAd->BgndScanCtrl.ChBusyTimeTH);
		MTWF_PRINT(" Noisy Threshold = %d\n", pAd->BgndScanCtrl.NoisyTH);
		MTWF_PRINT(" IPI Idle Threshold (*8us) = %d\n", pAd->BgndScanCtrl.IPIIdleTimeTH);
		MTWF_PRINT(" Scan Duration = %d ms\n", pAd->BgndScanCtrl.ScanDuration);
		MTWF_PRINT(" Partial Scan Interval = %d second\n", pAd->BgndScanCtrl.PartialScanInterval);
		MTWF_PRINT(" DriverTrigger support= %d\n", pAd->BgndScanCtrl.DriverTrigger);
		MTWF_PRINT("===== Status / Statistic =====\n");
		MTWF_PRINT(" One sec channel busy time = %d\n", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx);
		MTWF_PRINT(" One sec primary channel busy time = %d\n", pAd->OneSecMibBucket.ChannelBusyTime);
		MTWF_PRINT(" One sec My Tx Airtime = %d\n", pAd->OneSecMibBucket.MyTxAirtime);
		MTWF_PRINT(" One sec My Rx Airtime = %d\n", pAd->OneSecMibBucket.MyRxAirtime);
		MTWF_PRINT(" IPI Idle time = %d\n", pAd->BgndScanCtrl.IPIIdleTime);
		MTWF_PRINT(" Noisy = %d\n", pAd->BgndScanCtrl.Noisy);
		MTWF_PRINT(" Current state = %ld\n", pAd->BgndScanCtrl.BgndScanStatMachine.CurrState);
		MTWF_PRINT(" Scan type = %d\n", pAd->BgndScanCtrl.ScanType);
		/* MTWF_PRINT (" Interval count = %d\n", pAd->BgndScanCtrl.BgndScanIntervalCount); */
		/* MTWF_PRINT (" Interval = %d\n", pAd->BgndScanCtrl.BgndScanInterval); */
	}

	return TRUE;
}
#endif /* BACKGROUND_SCAN_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
/*
    ==========================================================================
    Description:
	RF test switch mode.

	iwpriv ra0 set RBIST_SwitchMode = ModeEnable

	ModeEnable
	0: OPERATION_NORMAL_MODE
	1: OPERATION_RFTEST_MODE
	2: OPERATION_ICAP_MODE
	4: OPERATION_WIFI_SPECTRUM

    Return:
    ==========================================================================
*/
INT32 Set_RBIST_Switch_Mode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UINT8 ModeEnable = 0;
#ifdef CONFIG_ATE
	UINT32 op_mode = TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode);
#endif/* CONFIG_ATE */

	ModeEnable = simple_strtol(arg, 0, 10);
	if (ModeEnable > OPERATION_WIFI_SPECTRUM) {
		MTWF_PRINT("Invalid input mode are not supported.\n");
		return FALSE;
	}
	if (ModeEnable == OPERATION_NORMAL_MODE) {
#ifdef CONFIG_ATE
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, op_mode &= ~fATE_IN_RFTEST);
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_RFTEST_MODE) {
#ifdef CONFIG_ATE
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, op_mode |= fATE_IN_RFTEST);
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_ICAP_MODE) {
#ifdef CONFIG_ATE
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, op_mode |= fATE_IN_RFTEST);
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_ICAP_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else if (ModeEnable == OPERATION_WIFI_SPECTRUM) {
#ifdef CONFIG_ATE
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, op_mode &= ~fATE_IN_RFTEST);
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_WIFI_SPECTRUM, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	} else {
#ifdef CONFIG_ATE
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, op_mode &= ~fATE_IN_RFTEST);
#endif/* CONFIG_ATE */
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
							  RF_TEST_DEFAULT_RESP_LEN);
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set parameters when ICap/Wifi-spectrum is started or stopped.

	iwpriv ra0 set RBIST_CaptureStart
	= Mode : Trigger : RingCapEn : TriggerEvent : CaptureNode : CaptureLen :
	  CapStopCycle : BW : MACTriggerEvent : SourceAddr. : Band : PhyIdx : CapSrc

    Return:
    ==========================================================================
*/
INT32 Set_RBIST_Capture_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT32 i, j, retval;
	RTMP_STRING Temp1[2] = {0};
	INT8 *pTemp1 = Temp1;
	INT8 *value = NULL;
	UINT32 Temp2[6] = {0};
	UINT32 Mode = 0, Trig = 0, RingCapEn = 0, BBPTrigEvent = 0, CapNode = 0;
	UINT32 CapLen = 0, CapStopCycle = 0, MACTrigEvent = 0, PhyIdx = 0;
	UINT32 SrcAddrLSB = 0, SrcAddrMSB = 0, BandIdx = 0, BW = 0, CapSrc = 0;
	RBIST_CAP_START_T *prRBISTInfo = NULL;

	/* Dynamic allocate memory for prRBISTInfo */
	retval = os_alloc_mem(pAd, (UCHAR **)&prRBISTInfo, sizeof(RBIST_CAP_START_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT(" Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(prRBISTInfo, sizeof(RBIST_CAP_START_T));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case CAP_MODE:
			Mode = simple_strtol(value, 0, 16);
			break;

		case CAP_TRIGGER:
			Trig = simple_strtol(value, 0, 16);
			prRBISTInfo->fgTrigger = Trig;
			break;

		case CAP_RING_MODE:
			RingCapEn = simple_strtol(value, 0, 16);
			prRBISTInfo->fgRingCapEn = RingCapEn;
			break;

		case CAP_BBP_EVENT:
			BBPTrigEvent = simple_strtol(value, 0, 16);
			prRBISTInfo->u4TriggerEvent = BBPTrigEvent;
			break;

		case CAP_NODE:
			CapNode = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CaptureNode = CapNode;
			break;

		case CAP_LENGTH:
			CapLen = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CaptureLen = CapLen;
			break;

		case CAP_STOP_CYCLE:
			CapStopCycle = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CapStopCycle = CapStopCycle;
			break;

		case CAP_BW:
			BW = simple_strtol(value, 0, 16);
			prRBISTInfo->u4BW = BW;
			break;

		case CAP_MAC_EVENT:
			MACTrigEvent = simple_strtol(value, 0, 16);
			prRBISTInfo->u4MACTriggerEvent = MACTrigEvent;
			break;

		case CAP_SOURCE_ADDR:
			for (j = 0; j < 6; j++) {
				RTMPMoveMemory(pTemp1, value, 2);
				Temp2[j] = simple_strtol(pTemp1, 0, 16);
				value += 2;
			}

			SrcAddrLSB = (Temp2[0] | (Temp2[1] << 8) |
						  (Temp2[2] << 16) | (Temp2[3] << 24));
			SrcAddrMSB = (Temp2[4] | (Temp2[5] << 8) | (0x1 << 16));
			prRBISTInfo->u4SourceAddressLSB = SrcAddrLSB;
			prRBISTInfo->u4SourceAddressMSB = SrcAddrMSB;
			break;

		case CAP_BAND:
			BandIdx = simple_strtol(value, 0, 16);
			prRBISTInfo->u4BandIdx = BandIdx;
			break;

		case CAP_PHY:
			PhyIdx = simple_strtol(value, 0, 16);
			prRBISTInfo->u4PhyIdx = PhyIdx;
			break;

		case CAP_SOURCE:
			CapSrc = simple_strtol(value, 0, 16);
			prRBISTInfo->u4CapSource = CapSrc;
			break;

		default:
			break;
		}
	}

	MTWF_PRINT("%s :\n Mode = 0x%08x\n"
			" Trigger = 0x%08x\n RingCapEn = 0x%08x\n TriggerEvent = 0x%08x\n CaptureNode = 0x%08x\n"
			" CaptureLen = 0x%08x\n CapStopCycle = 0x%08x\n BW = 0x%08x\n MACTriggerEvent = 0x%08x\n"
			" SourceAddrLSB = 0x%08x\n SourceAddrMSB = 0x%08x\n Band = 0x%08x\n PhyIdx = 0x%08x\n"
			" CapSrc = 0x%08x\n", __func__, Mode, Trig, RingCapEn, BBPTrigEvent, CapNode, CapLen,
			CapStopCycle, BW, MACTrigEvent, SrcAddrLSB, SrcAddrMSB, BandIdx, PhyIdx, CapSrc);

	if (Mode == ICAP_MODE) {
#ifdef INTERNAL_CAPTURE_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapStart != NULL)
			ops->ICapStart(pAd, (UINT8 *)prRBISTInfo);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
#endif /* INTERNAL_CAPTURE_SUPPORT */
	} else if (Mode == WIFI_SPECTRUM_MODE) {
#ifdef WIFI_SPECTRUM_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStart != NULL)
			ops->SpectrumStart(pAd, (UINT8 *)prRBISTInfo);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
#endif /* WIFI_SPECTRUM_SUPPORT */
	}

error:
	if (prRBISTInfo != NULL)
		os_free_mem(prRBISTInfo);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Query ICap/Wifi-spectrum status.

	iwpriv ra0 set RBIST_CaptureStatus = Choice

	 Choice
	 0: ICAP_MODE
	 1: WIFI_SPECTRUM_MODE

    Return:
    ==========================================================================
*/
INT32 Get_RBIST_Capture_Status(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT32 Choice;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case ICAP_MODE:
#ifdef INTERNAL_CAPTURE_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapStatus != NULL)
			ops->ICapStatus(pAd);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */
		break;

	case WIFI_SPECTRUM_MODE:
#ifdef WIFI_SPECTRUM_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStatus != NULL)
			ops->SpectrumStatus(pAd);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
	}
#endif /* WIFI_SPECTRUM_SUPPORT */
		break;

	default:
		MTWF_PRINT("Not support for %d this selection !!\n", Choice);
		break;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap/Wifi-spectrum RBIST sysram raw data .

	 iwpriv ra0 set RBIST_RawDataProc = Choice

	 Choice
	 0: ICAP_MODE
	    a. Get ICap RBIST sysram raw data by unsolicited event.(on-the-fly)
	    b. Re-arrange ICap sysram buffer by wrapper.
	    c. Parsing ICap I/Q data.
	    d. Dump L32bit/M32bit/H32bit to file.
	 1: WIFI_SPECTRUM_MODE
	    a. Get Wifi-spectrum RBIST sysram raw data by unsolicited event.(on-the-fly)
	    b. Parsing Wifi-spectrum I/Q data.
	    c. Dump I/Q/LNA/LPF data to file.
    Return:
    ==========================================================================
*/
INT32 Get_RBIST_Raw_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UINT32 Choice;
	INT32 Status = CAP_FAIL;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case ICAP_MODE:
#ifdef INTERNAL_CAPTURE_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->ICapCmdUnSolicitRawDataProc != NULL)
			Status = ops->ICapCmdUnSolicitRawDataProc(pAd);
		else {
			MTWF_PRINT(" The function is not hooked !!\n");
		}
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */
		break;
	case WIFI_SPECTRUM_MODE:
#ifdef WIFI_SPECTRUM_SUPPORT
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumCmdRawDataProc != NULL)
			Status = ops->SpectrumCmdRawDataProc(pAd);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
	}
#endif /* WIFI_SPECTRUM_SUPPORT */
		break;

	default:
		MTWF_PRINT("Not support for %d this selection !!\n", Choice);
		break;
	}

	MTWF_PRINT("%s:(Status = %d)\n", __func__, Status);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap I/Q data which is stored in IQ_Array captured by
	 WF0/WF1/WF2/WF3.

    Return:
    ==========================================================================
*/
INT32 Get_RBIST_IQ_Data(
	IN RTMP_ADAPTER *pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num)
{
	UINT32 i, CapNode, TotalCnt, Len, CapSrc;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;

	/* Initialization of pData and pDataLen buffer */
	Len = ICAP_EVENT_DATA_SAMPLE * sizeof(INT32);
	os_zero_mem(pData, Len);
	os_zero_mem(pDataLen, sizeof(INT32));

	/* Query current capture node */
	CapNode = pAd->ICapCapNode;

	/* Query current capture source */
	CapSrc = pAd->ICapCapSrc;

	MTWF_PRINT("%s : CapNode = 0x%08x, CapSrc = 0x%08x\n", __func__, CapNode, CapSrc);

	/* Update total count of I or Q sample of each WF */
	if (CapNode == pChipCap->ICapPackedADC)
		TotalCnt = pChipCap->ICapADCIQCnt;
	else
		TotalCnt = pChipCap->ICapIQCIQCnt;

	/* Update initial value of ICapDataCnt if user want to display short length of data */
	if ((TotalCnt > pAd->ICapCapLen) && (pAd->ICapDataCnt == 0))
		pAd->ICapDataCnt = TotalCnt - pAd->ICapCapLen;

	/* Store I or Q data(1KBytes) to data buffer */
	for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
		UINT32 idx = pAd->ICapDataCnt;

		/* If it is the last one of I or Q data, just stop querying */
		if (pAd->ICapDataCnt == TotalCnt)
			break;

		/* Store I/Q data to data buffer */
		pData[i] = pIQ_Array[idx].IQ_Array[WF_Num][IQ_Type];
		/* Update data counter */
		pAd->ICapDataCnt++;
	}

	/* Update data length */
	*pDataLen = i;

	/* Reset data counter */
	if (*pDataLen == 0)
		pAd->ICapDataCnt = 0;

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get ICap I/Q data which is captured by WF0 or WF1 or WF2 or WF3.

	 iwpriv ra0 set RBIST_IQDataProc = IQ_Type : WF_Num : ICap_Len

	 IQ_Type
	 0: I_TYPE/1: Q_TYPE
	 WF_Num
	 0: WF0/1: WF1/2: WF2/3: WF3
	 ICap_Len(Unit: I or Q sample cnt)

	 a. Store I/Q data which is captured by WF0/WF1/WF2/WF3 to data buffer.
	 b. Dump I/Q data to file.

    Return:
    ==========================================================================
*/
#define CAP_IQ_Type			0
#define CAP_WF_Num			1
#define CAP_Len				2
INT32 Get_RBIST_IQ_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT i, retval;
	UINT32 IQ_Type = 0, WF_Num = 0, Len;
	PINT32 pData = NULL, pDataLen = NULL;
	RTMP_STRING *value = NULL;
	RTMP_STRING *pSrc_IQ = NULL;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case CAP_IQ_Type:
			IQ_Type = simple_strtol(value, 0, 10);
			break;

		case CAP_WF_Num:
			WF_Num = simple_strtol(value, 0, 10);
			break;
		case CAP_Len:
			pAd->ICapCapLen = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	/* Dynamic allocate memory for pSrc_IQ */
	retval = os_alloc_mem(pAd, (UCHAR **)&pSrc_IQ, ICAP_EVENT_DATA_SAMPLE);
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT(" Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pSrc_IQ, ICAP_EVENT_DATA_SAMPLE);

	/* Dynamic allocate memory for 1KByte data buffer */
	Len = ICAP_EVENT_DATA_SAMPLE * sizeof(INT32);
	retval = os_alloc_mem(pAd, (UCHAR **)&pData, Len);
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT(" Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pData, Len);

	/* Dynamic allocate memory for data length */
	retval = os_alloc_mem(pAd, (UCHAR **)&pDataLen, sizeof(INT32));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT(" Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pDataLen, sizeof(INT32));

	/* Fill in title for console log */
	if (IQ_Type == CAP_I_TYPE) {
		retval = snprintf(pSrc_IQ, ICAP_EVENT_DATA_SAMPLE, "Icap_%s%d", "I", WF_Num);
		if (os_snprintf_error(sizeof(RTMP_STRING), retval)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			goto error;
		}
	} else if (IQ_Type == CAP_Q_TYPE) {
		retval = snprintf(pSrc_IQ, ICAP_EVENT_DATA_SAMPLE, "Icap_%s%d", "Q", WF_Num);
		if (os_snprintf_error(sizeof(RTMP_STRING), retval)) {
			MTWF_PRINT("%s: final_name snprintf error!\n", __func__);
			goto error;
		}
	}
	MTWF_PRINT("%s\n", pSrc_IQ);

	/* Initialization of ICapDataCnt */
	pAd->ICapDataCnt = 0;

	while (1) {
#ifdef INTERNAL_CAPTURE_SUPPORT
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		/* Query I/Q data from buffer */
		if (ops->ICapGetIQData != NULL)
			ops->ICapGetIQData(pAd, pData, pDataLen, IQ_Type, WF_Num);
		else if (ops->ICapCmdSolicitRawDataProc != NULL)
			ops->ICapCmdSolicitRawDataProc(pAd, pData, pDataLen, IQ_Type, WF_Num);
		else {
			MTWF_PRINT("The function is not hooked !!\n");
		}
#endif /* INTERNAL_CAPTURE_SUPPORT */

		/* If data length is zero, it means the end of data querying */
		if (*pDataLen == 0)
			break;

		/* Print data log to console */
		for (i = 0; i < *pDataLen; i++)
			MTWF_PRINT("%d\n", pData[i]);
	}

error:
	if (pData != NULL)
		os_free_mem(pData);

	if (pDataLen != NULL)
		os_free_mem(pDataLen);

	if (pSrc_IQ != NULL)
		os_free_mem(pSrc_IQ);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Get Icap/Wifi-spectrum capture node information.

    Return: Value of capture node.
    ==========================================================================
*/
UINT32 Get_System_CapNode_Info(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 CapNode = 0;

	CapNode = pAd->ICapCapNode;
	MTWF_PRINT("%s : CaptureNode = 0x%08x\n", __func__, CapNode);

	return CapNode;
}

/*
    ==========================================================================
    Description:
	 Get current band central frequency information.

    Return: Value of central frequency(MHz).
    ==========================================================================
*/
UINT32 Get_System_CenFreq_Info(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ChIdx, CenFreq = 0;
	UINT8 CenCh = 0;
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(pAd);
	struct mtk_mac_phy *mac_phy = NULL;

	if (!mac_dev) {
		MTWF_PRINT("fail. expected mac_dev not found\n");
		return FALSE;
	}
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy) {
		MTWF_PRINT("fail. expected mac_phy not found\n");
		return FALSE;
	}
	CenCh = mac_phy->cen_chan;
	MTWF_PRINT("%s : CentralCh = %d\n", __func__, CenCh);

	for (ChIdx = 0; ChIdx < CH_HZ_ID_MAP_NUM; ChIdx++) {
		if (CenCh == CH_HZ_ID_MAP[ChIdx].channel) {
			CenFreq = CH_HZ_ID_MAP[ChIdx].freqKHz;
			break;
		}
	}

	MTWF_PRINT("%s : CentralFreq = %d\n", __func__, CenFreq);

	return CenFreq;
}

/*
    ==========================================================================
    Description:
	 Get current band bandwidth information.

    Return: Value of capture BW.
	    CAP_BW_20                            0
	    CAP_BW_40                            1
	    CAP_BW_80                            2
    ==========================================================================
*/
UINT8 Get_System_Bw_Info(
	IN PRTMP_ADAPTER pAd)
{
	INT8 Bw = 0, CapBw = 0;
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(pAd);
	struct mtk_mac_phy *mac_phy = NULL;

	if (!mac_dev) {
		MTWF_PRINT("fail. expected mac_dev not found\n");
		return FALSE;
	}
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy) {
		MTWF_PRINT("fail. expected mac_phy not found\n");
		return FALSE;
	}
	Bw = mac_phy->bw;
	MTWF_PRINT("%s : Bw = %d\n", __func__, Bw);

	switch (Bw) {
	case CMD_BW_20:
		CapBw = CAP_BW_20;
		break;

	case CMD_BW_40:
		CapBw = CAP_BW_40;
		break;

	case CMD_BW_80:
		CapBw = CAP_BW_80;
		break;

	case CMD_BW_160:
		CapBw = CAP_BW_80;
		break;

	case CMD_BW_8080:
		CapBw = CAP_BW_80;
		break;

	default:
		CapBw = CAP_BW_20;
		break;
	}

	MTWF_PRINT("%s : CaptureBw = %d\n", __func__, CapBw);

	return CapBw;
}

/*
    ==========================================================================
    Description:
	 Used for getting current band wireless information.

	 iwpriv ra0 set WirelessInfo = Choice

	 Choice
	 0: CentralFreq
	 1: Bw

    Return:
    ==========================================================================
*/
#define CEN_FREQ			0
#define SYS_BW				1
INT32 Get_System_Wireless_Info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UINT16 CenFreq = 0;
	UINT8 Bw = 0;
	INT32 Choice;

	Choice = simple_strtol(arg, 0, 10);
	switch (Choice) {
	case CEN_FREQ:
		CenFreq = Get_System_CenFreq_Info(pAd);
		break;

	case SYS_BW:
		Bw = Get_System_Bw_Info(pAd);
		break;

	default:
		break;
	}

	return TRUE;
}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(PHY_ICS_SUPPORT)
/*
    ==========================================================================
    Description:
	Set parameters when PHY ICS is started or stopped.

	iwpriv ra0 set PhyIcs_Start
	= Mode : Trigger : RingCapEn : BW : Band : PhyIdx : CapSrc
	  Partition : Event_Group : Event_ID_MSB : Event_ID_LSB : Timer(ms).

    Return:
    ==========================================================================
*/
INT32 Set_PhyIcs_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT32 i, retval;
	INT8 *value = NULL;
	UINT32 Trig = 0;
	ULONG PhyIcsTimer = 0;
	PHY_ICS_START_T *prPhyIcsInfo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Dynamic allocate memory for prPhyIcsInfo */
	retval = os_alloc_mem(pAd, (UCHAR **)&prPhyIcsInfo, sizeof(PHY_ICS_START_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("%s : Not enough memory for dynamic allocating !!\n", __func__);
		goto error;
	}
	os_zero_mem(prPhyIcsInfo, sizeof(PHY_ICS_START_T));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case ICS_TRIGGER: /* action =>  0:stop, 1:start */
			Trig = simple_strtol(value, 0, 16);
			prPhyIcsInfo->fgTrigger = Trig;
			if (Trig)
				pAd->PhyIcsFlag = TRUE;
			else
				pAd->PhyIcsFlag = FALSE;
			break;

		case ICS_TIMER:
			if (kstrtol(value, 10, &PhyIcsTimer))
				goto error;

			prPhyIcsInfo->u4PhyIcsTimer = PhyIcsTimer;
			break;

		default:
			break;
		}
	}

	MTWF_PRINT("Trigger=%d\n PhyIcsTimer=%ld\n", Trig, PhyIcsTimer);

	if (ops->PhyIcsStart != NULL) {
		ops->PhyIcsStart(pAd, (UINT8 *)prPhyIcsInfo);
		MTWF_PRINT("%s : PhyIcsStart is hooked success!!\n", __func__);
	} else {
		MTWF_PRINT("The function is not hooked !!\n");
	}

error:
	if (prPhyIcsInfo != NULL)
		os_free_mem(prPhyIcsInfo);

	return TRUE;
}

INT32 Set_PhyIcs_Event_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	INT32 i, retval;
	INT8 *value = NULL;
	UINT32 BandIdx = 0;
	UINT32 PhyIcsType = 0, PhyIcsEventGroup = 0;
	UINT32 PhyIcsEventID[2] = {0};
	PHY_ICS_START_T *prPhyIcsInfo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Dynamic allocate memory for prPhyIcsInfo */
	retval = os_alloc_mem(pAd, (UCHAR **)&prPhyIcsInfo, sizeof(PHY_ICS_START_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(prPhyIcsInfo, sizeof(PHY_ICS_START_T));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case ICS_BAND: /* phy ics 0: band0, 1:band1 */
			BandIdx = simple_strtol(value, 0, 16);
			prPhyIcsInfo->u4BandIdx = BandIdx;
			break;

		case ICS_PARTITION:
			PhyIcsType = simple_strtol(value, 0, 16);
			prPhyIcsInfo->u4PhyIcsType = PhyIcsType;
			break;

		case ICS_EVENT_GROUP:
			PhyIcsEventGroup = simple_strtol(value, 0, 16);
			prPhyIcsInfo->u4PhyIcsEventGroup = PhyIcsEventGroup;
			break;

		case ICS_EVENT_ID_MSB:
			PhyIcsEventID[0] = simple_strtol(value, 0, 16);
			prPhyIcsInfo->u4PhyIcsEventID[0] = PhyIcsEventID[0];
			break;

		case ICS_EVENT_ID_LSB:
			PhyIcsEventID[1] = simple_strtol(value, 0, 16);
			prPhyIcsInfo->u4PhyIcsEventID[1] = PhyIcsEventID[1];
			break;

		default:
			break;
		}
	}

	MTWF_PRINT("PhyIcs:Band=%d\n Type=0x%08x\n Group=0x%08x\n MSB_EID=0x%08x\n LSB_EID=0x%08x\n",
		BandIdx, PhyIcsType, PhyIcsEventGroup, PhyIcsEventID[0], PhyIcsEventID[1]);

	if (ops->PhyIcsEventEnable != NULL) {
		ops->PhyIcsEventEnable(pAd, (UINT8 *)prPhyIcsInfo);
		MTWF_PRINT("PhyIcsEventEnable is hooked success!!\n");
	} else {
		MTWF_PRINT("The function is not hooked !!\n");
	}

error:
	if (prPhyIcsInfo != NULL)
		os_free_mem(prPhyIcsInfo);

	return TRUE;
}

#endif /* defined(PHY_ICS_SUPPORT) */

/*
    ==========================================================================
    Description:
	 Set IRR ADC parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_ADC(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT8   AntIndex = 0;
	UINT8   BW = 0;
	UINT8   SX = 0;
	UINT8   DbdcIdx = 0;
	UINT8   RunType = 0;
	UINT8   FType = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		case 1:
			ChannelFreq = os_str_tol(value, 0, 10);
			break;

		case 2:
			BW = os_str_tol(value, 0, 10);
			break;

		case 3:
			SX = os_str_tol(value, 0, 10);
			break;

		case 4:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 5:
			RunType = os_str_tol(value, 0, 10);
			break;

		case 6:
			FType = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_PRINT("%s: <SetADC> Input Checking Log\n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					AntIndex = %d \n\
					BW = %d \n\
					SX= %d \n\
					DbdcIdx = %d \n\
					RunType = %d \n\
					FType = %d \n\n", __func__, \
			 ChannelFreq, \
			 AntIndex, \
			 BW, \
			 SX, \
			 DbdcIdx, \
			 RunType, \
			 FType);
	MtCmdRfTestSetADC(pAd, ChannelFreq, AntIndex, BW, SX, DbdcIdx, RunType, FType);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR Rx Gain parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_RxGain(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT8   LPFG = 0;
	UINT8   LNA = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			LPFG = os_str_tol(value, 0, 10);
			break;

		case 1:
			LNA = os_str_tol(value, 0, 10);
			break;

		case 2:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 3:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		default:
			break;
		}
	}

	MTWF_PRINT("%s: <SetRxGain> Input Checking Log\n\
					--------------------------------------------------------------\n\
					LPFG = %d \n\
					LNA = %d \n\
					DbdcIdx = %d \n\
					AntIndex= %d \n\n", __func__, \
			 LPFG, \
			 LNA, \
			 DbdcIdx, \
			 AntIndex);
	MtCmdRfTestSetRxGain(pAd, LPFG, LNA, DbdcIdx, AntIndex);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR TTG parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTG(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT32  ToneFreq = 0;
	UINT8   TTGPwrIdx = 0;
	UINT8	XtalFreq = 0;
	UINT8   DbdcIdx = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TTGPwrIdx = os_str_tol(value, 0, 10);
			break;

		case 1:
			ToneFreq = os_str_tol(value, 0, 10);
			break;

		case 2:
			ChannelFreq = os_str_tol(value, 0, 10);
			break;

		case 3:
			XtalFreq = os_str_tol(value, 0, 10);
			break;

		case 4:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		default:
			break;
		}
	}

	MTWF_PRINT("%s: <SetTTG> Input Checking Log\n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					ToneFreq = %d \n\
					TTGPwrIdx = %d \n\
					DbdcIdx= %d \n\n", __func__, \
			 ChannelFreq, \
			 ToneFreq, \
			 TTGPwrIdx, \
			 DbdcIdx);
	MtCmdRfTestSetTTG(pAd, ChannelFreq, ToneFreq, TTGPwrIdx, XtalFreq, DbdcIdx);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	 Set IRR TTGOnOff parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTGOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	CHAR    *value = 0;
	UINT8   TTGEnable = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TTGEnable = os_str_tol(value, 0, 10);
			break;

		case 1:
			DbdcIdx = os_str_tol(value, 0, 10);
			break;

		case 2:
			AntIndex = os_str_tol(value, 0, 10);

			switch (AntIndex) {
			case QA_IRR_WF0:
				AntIndex = WF0;
				break;

			case QA_IRR_WF1:
				AntIndex = WF1;
				break;

			case QA_IRR_WF2:
				AntIndex = WF2;
				break;

			case QA_IRR_WF3:
				AntIndex = WF3;
				break;
			}

			break;

		default:
			break;
		}
	}

	MTWF_PRINT("%s: <SetTTGOnOff> Input Checking Log\n\
					--------------------------------------------------------------\n\
					TTGEnable = %d \n\
					DbdcIdx = %d \n\
					AntIndex = %d \n\n", __func__, \
			 TTGEnable, \
			 DbdcIdx, \
			 AntIndex);
	MtCmdRfTestSetTTGOnOff(pAd, TTGEnable, DbdcIdx, AntIndex);
	return TRUE;
}

INT set_manual_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token;
	UINT32 wdev_idx = 0, mode = 0;
	struct wifi_dev *wdev;

	if (arg == NULL)
		goto err1;

	if (arg != NULL) {
		token = strsep(&arg, "-");
		wdev_idx = os_str_tol(token, 0, 10);

		if (pAd->wdev_list[wdev_idx] == NULL)
			goto err2;
	}

	wdev = pAd->wdev_list[wdev_idx];

	while (arg != NULL) {
		token = strsep(&arg, "+");

		if (!strcmp(token, "erp"))
			mode |= SET_PROTECT(ERP);
		else if (!strcmp(token, "no"))
			mode |= SET_PROTECT(NO_PROTECTION);
		else if (!strcmp(token, "non_member"))
			mode |= SET_PROTECT(NON_MEMBER_PROTECT);
		else if (!strcmp(token, "ht20"))
			mode |= SET_PROTECT(HT20_PROTECT);
		else if (!strcmp(token, "non_ht_mixmode"))
			mode |= SET_PROTECT(NON_HT_MIXMODE_PROTECT);
		else if (!strcmp(token, "longnav"))
			mode |= SET_PROTECT(LONG_NAV_PROTECT);
		else if (!strcmp(token, "gf"))
			mode |= SET_PROTECT(GREEN_FIELD_PROTECT);
		else if (!strcmp(token, "rifs"))
			mode |= SET_PROTECT(RIFS_PROTECT);
		else if (!strcmp(token, "rdg"))
			mode |= SET_PROTECT(RDG_PROTECT);
		else if (!strcmp(token, "force_rts"))
			mode |= SET_PROTECT(FORCE_RTS_PROTECT);
		else
			goto err3;
	}

	pAd->wdev_list[wdev_idx]->protection = mode;
	MTWF_PRINT
			 (" <<< manual trigger >>>\n HWFLAG_ID_UPDATE_PROTECT\n");
	MTWF_PRINT("   -- wdev_%d->protection: 0x%08x\n",
			  wdev_idx, pAd->wdev_list[wdev_idx]->protection);
	HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
	goto end;
err3:
	MTWF_PRINT(" -no mode [ERROR 3]\n");
	goto err1;
err2:
	MTWF_PRINT(" -no wdev_idx: 0x%x [ERROR 2]\n", wdev_idx);
err1:
	MTWF_PRINT("Usage:\niwpriv ra0 set protect=[wdev_idx]-[mode]+...\n");
	MTWF_PRINT("       mode: [erp|no|non_member|ht20|non_ht_mixmode|longnav|gf|rifs|rdg|force_rts]\n");
end:
	return TRUE;
}

INT set_cca_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->set_cca_en)
		return chip_dbg->set_cca_en(pAd->hdev_ctrl, arg);
	else
		return FALSE;
}

INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (pAd->physical_dev) {
		physical_device_show_timer_list(pAd->physical_dev);
		return TRUE;
	}
	return FALSE;
}

INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcWtblRecDump(pAd);
	return TRUE;
}

/*
*
*/
UINT VIRTUAL_IF_INC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags = 0;

	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
	cnt = pAd->VirtualIfCnt++;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
	return cnt;
}

/*
*
*/
UINT VIRTUAL_IF_DEC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags = 0;

	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
	cnt = pAd->VirtualIfCnt--;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
	return cnt;
}

/*
*
*/
UINT VIRTUAL_IF_NUM(RTMP_ADAPTER *pAd)
{
	UINT cnt;

	cnt = pAd->VirtualIfCnt;
	return cnt;
}

INT Set_Rx_Vector_Control(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	BOOLEAN Enable = 1;
	UCHAR ucBandIdx = 0;

	/* obtain Band index */
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
	MTWF_PRINT("%s: BandIdx = %d\n", __func__, ucBandIdx);
#endif /* CONFIG_AP_SUPPORT */

	MTWF_PRINT("%s----------------->\n", __func__);

	if (arg)
		Enable = os_str_tol(arg, 0, 10);

	Enable = (Enable == 0 ? 0 : 1);

	/* Turn off MibBucket */
	pAd->OneSecMibBucket.Enabled = !Enable;
	pAd->MsMibBucket.Enabled = !Enable;

	/* Mac Enable*/
	AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, Enable);

	/* Rxv Enable*/
	AsicSetRxvFilter(pAd, Enable, ucBandIdx);

	if (Enable)
		pAd->parse_rxv_stat_enable = 1;
	else
		pAd->parse_rxv_stat_enable = 0;

	MTWF_PRINT("%s<-----------------\n", __func__);
	return TRUE;
}

static VOID Parse_Rx_Rssi_CR(PRTMP_ADAPTER pAd, struct _RX_STATISTIC_CR *RxStat, INT type, UINT32 value)
{
	UINT32 IBRssi0 = 0, IBRssi1 = 0, WBRssi0 = 0, WBRssi1 = 0;

	MTWF_PRINT("%s: Value : %02x\n", __func__, value);

	IBRssi1 = (value & 0xFF000000) >> 24;
	if (IBRssi1 >= 128)
		IBRssi1 -= 256;
	WBRssi1 = (value & 0x00FF0000) >> 16;
	if (WBRssi1 >= 128)
		WBRssi1 -= 256;
	IBRssi0 = (value & 0x0000FF00) >> 8;
	if (IBRssi0 >= 128)
		IBRssi0 -= 256;
	WBRssi0 = (value & 0x000000FF);
	if (WBRssi0 >= 128)
		WBRssi0 -= 256;

	if (type == HQA_RX_STAT_RSSI || type == HQA_RX_STAT_RSSI_BAND1) {
		RxStat->Inst_IB_RSSSI[0] =  IBRssi0;
		RxStat->Inst_WB_RSSSI[0] =  WBRssi0;
		RxStat->Inst_IB_RSSSI[1] =  IBRssi1;
		RxStat->Inst_WB_RSSSI[1] =  WBRssi1;
	} else {
		RxStat->Inst_IB_RSSSI[2] =  IBRssi0;
		RxStat->Inst_WB_RSSSI[2] =  WBRssi0;
		RxStat->Inst_IB_RSSSI[3] =  IBRssi1;
		RxStat->Inst_WB_RSSSI[3] =  WBRssi1;
	}
}

INT Show_Rx_Statistic(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
#define MSG_LEN 2048
#define ENABLE 1
#define DISABLE 0
#define BAND0 0
#define BAND1 1
	RX_STATISTIC_RXV *rx_stat_rxv = &pAd->rx_stat_rxv;
	RX_STATISTIC_CR rx_stat_cr;
	UINT32 value = 0, i = 0, set = 1;
	UINT32 Status;
	UINT32 CurrBand0FCSErr, CurrBand0MDRDY;
	static UINT32 PreBand0FCSErr, PreBand0MDRDY;
	RTMP_STRING *msg;
	UCHAR ucBandIdx = 0;
	int ret, left_buf_size;

	ucBandIdx = hc_get_hw_band_idx(pAd);
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("%s: Invalid BandIdx = %d\n", __func__, ucBandIdx);
		return FALSE;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "\n");
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "BandIdx = %d\n", ucBandIdx);

	if (arg)
		set = os_str_tol(arg, 0, 10);

	set = (set == 0 ? 0 : 1);
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	if (msg == NULL) {
		MTWF_PRINT("os_alloc_mem failed\n");
		MTWF_PRINT("%s<-----------------\n", __func__);
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_PRINT("final_name snprintf error!\n");
		goto done;
	}

	switch (set) {
	case RESET_COUNTER:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Reset counter !!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
#ifdef CONFIG_HW_HAL_OFFLOAD
		/*Disable PHY Counter*/
		MtCmdSetPhyCounter(pAd, DISABLE, ucBandIdx);
#endif/*CONFIG_HW_HAL_OFFLOAD*/
		PreBand0FCSErr = 0;
		PreBand0MDRDY = 0;
		pAd->AccuOneSecRxBand0FcsErrCnt = 0;
		pAd->AccuOneSecRxBand0MdrdyCnt = 0;
		pAd->AccuOneSecRxBand1FcsErrCnt = 0;
		pAd->AccuOneSecRxBand1MdrdyCnt = 0;
		break;

	case SHOW_RX_STATISTIC:
#ifdef CONFIG_HW_HAL_OFFLOAD
		/*Enable PHY Counter*/
		MtCmdSetPhyCounter(pAd, ENABLE, ucBandIdx);
#endif/*CONFIG_HW_HAL_OFFLOAD*/

		/*Band0 PHY Counter */
		value = AsicGetRxStat(pAd, HQA_RX_STAT_PHY_FCSERRCNT);
		rx_stat_cr.FCSErr_OFDM = (value >> 16);
		rx_stat_cr.FCSErr_CCK = (value & 0xFFFF);
		value = AsicGetRxStat(pAd, HQA_RX_STAT_PD);
		rx_stat_cr.OFDM_PD = (value >> 16);
		rx_stat_cr.CCK_PD = (value & 0xFFFF);
		value = AsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD);
		rx_stat_cr.CCK_SIG_Err = (value >> 16);
		rx_stat_cr.CCK_SFD_Err = (value & 0xFFFF);
		value = AsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG);
		rx_stat_cr.OFDM_SIG_Err = (value >> 16);
		rx_stat_cr.OFDM_TAG_Err = (value & 0xFFFF);

		/*IBRSSI0 WBRSSI0 IBRSSI1 WBRSSI1*/
		value = AsicGetRxStat(pAd, HQA_RX_STAT_RSSI);
		Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI, value);

		/*IBRSSI2 WBRSSI2 IBRSSI3 WBRSSI3*/
		value = AsicGetRxStat(pAd, HQA_RX_STAT_RSSI_RX23);
		Parse_Rx_Rssi_CR(pAd, &rx_stat_cr, HQA_RX_STAT_RSSI_RX23, value);

		value = AsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITL);
		rx_stat_cr.ACIHitLow = ((value >> 18) & 0x1);
		value = AsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITH);
		rx_stat_cr.ACIHitHigh = ((value >> 18) & 0x1);

		value = AsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT);
		rx_stat_cr.PhyMdrdyOFDM = (value >> 16);
		rx_stat_cr.PhyMdrdyCCK =  (value & 0xFFFF);

		/*Band0 MAC Counter*/
		CurrBand0FCSErr = pAd->AccuOneSecRxBand0FcsErrCnt;
		rx_stat_cr.RxMacFCSErrCount = CurrBand0FCSErr - PreBand0FCSErr;
		PreBand0FCSErr = CurrBand0FCSErr;
		CurrBand0MDRDY = pAd->AccuOneSecRxBand0MdrdyCnt;
		rx_stat_cr.RxMacMdrdyCount = CurrBand0MDRDY - PreBand0MDRDY;
		PreBand0MDRDY = CurrBand0MDRDY;
		rx_stat_cr.RxMacFCSOKCount = rx_stat_cr.RxMacMdrdyCount - rx_stat_cr.RxMacFCSErrCount;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "\x1b[41m%s : \x1b[m\n", __func__);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "FreqOffsetFromRx   = %d\n", rx_stat_rxv->FreqOffsetFromRx[0]);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		for (i = 0; i < 4; i++) {
			left_buf_size = MSG_LEN - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "RCPI_%d             = %d\n", i, rx_stat_rxv->RCPI[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				goto done;
			}
		}
		for (i = 0; i < 4; i++) {
			left_buf_size = MSG_LEN - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "FAGC_RSSI_IB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_IB[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				goto done;
			}
		}
		for (i = 0; i < 4; i++) {
			left_buf_size = MSG_LEN - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "FAGC_RSSI_WB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_WB[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				goto done;
			}
		}
		for (i = 0; i < 4; i++) {
			left_buf_size = MSG_LEN - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "Inst_IB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_IB_RSSSI[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				goto done;
			}
		}
		for (i = 0; i < 4; i++) {
			left_buf_size = MSG_LEN - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "Inst_WB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_WB_RSSSI[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				goto done;
			}
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "SNR                = %d\n",  rx_stat_rxv->SNR[0]);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "ACIHitHigh         = %u\n",  rx_stat_cr.ACIHitHigh);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "ACIHitLow          = %u\n",  rx_stat_cr.ACIHitLow);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"\x1b[41mFor Band0Index : \x1b[m\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "MacMdrdyCount      = %u\n",  rx_stat_cr.RxMacMdrdyCount);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "MacFCSErrCount     = %u\n",  rx_stat_cr.RxMacFCSErrCount);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "MacFCSOKCount      = %u\n",  rx_stat_cr.RxMacFCSOKCount);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "CCK_PD             = %u\n",  rx_stat_cr.CCK_PD);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "CCK_SFD_Err        = %u\n",  rx_stat_cr.CCK_SFD_Err);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "CCK_SIG_Err        = %u\n",  rx_stat_cr.CCK_SIG_Err);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "CCK_FCS_Err        = %u\n",  rx_stat_cr.FCSErr_CCK);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "OFDM_PD            = %u\n",  rx_stat_cr.OFDM_PD);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "OFDM_SIG_Err       = %u\n",  rx_stat_cr.OFDM_SIG_Err);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "OFDM_FCS_Err       = %u\n",  rx_stat_cr.FCSErr_OFDM);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			goto done;
		}
		break;
	}

	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_PRINT("%s<-----------------\n", __func__);
	return TRUE;
done:
	os_free_mem(msg);
	MTWF_PRINT("%s<-----------------\n", __func__);
	return FALSE;
}

#ifdef LED_CONTROL_SUPPORT
INT	Set_Led_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR thisChar;
	long led_param[8];
	INT i = 0, j = 0;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT apidx = pObj->ioctl_if;
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;
	UCHAR led_idx = LED_IDX_0;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (wdev->if_up_down_state == FALSE) {
		MTWF_PRINT("bss[%d] is not ready!!!\n", apidx);
		return -1;
	}

	if (BandIdx == INVALID_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("hc_get_hw_band_idx(pAd) is wrong!\n");
		return -1;
	}

	MTWF_PRINT("\n %s ==> arg = %s\n", __func__, arg);
	memset(led_param, 0, sizeof(long) * 8);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		led_param[i] = os_str_tol(thisChar, 0, 10);
		i++;

		if (i >= 8)
			break;
	}

	MTWF_PRINT("\n%s\n", __func__);

	for (j = 0; j < i; j++)
		MTWF_PRINT("%02x\n", (UINT)led_param[j]);

	if (pled_table) {
		for (i = 0; i < LED_MAX_NUM; i++) {
			if (((pled_table[i].band_idx != LED_BAND_NONE) &&
				 (BandIdx == pled_table[i].band_idx))) {
				led_idx = pled_table[i].led_idx;
				break;
			}
		}
	}

	rtmp_control_led_cmd(pAd, led_idx, led_param[0], led_param[1], BandIdx,
		led_param[2], led_param[3], led_param[4], led_param[5]);

	return TRUE;
}
#endif

INT TpcManCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN	fgTpcManual
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCManCtrl(pAd, fgTpcManual) == NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcManCtrl(pAd, fgTpcManual) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT TpcEnableCfg(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN	fgTpcEnable
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCEnableCfg(pAd, fgTpcEnable) == NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcEnableCfg(pAd, fgTpcEnable) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT TpcWlanIdCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN	fgUplink,
	IN UINT8	u1EntryIdx,
	IN UINT16	u2WlanId,
	IN UINT8    u1DlTxType
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCWlanIdCtrl(pAd, fgUplink, u1EntryIdx, u2WlanId, u1DlTxType) == NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcWlanIdCtrl(pAd, fgUplink, u1EntryIdx, u2WlanId, u1DlTxType) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT TpcUlAlgoCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8	u1TpcCmd,
	IN UINT8	u1ApTxPwr,
	IN UINT8	u1EntryIdx,
	IN UINT8	u1TargetRssi,
	IN UINT8	u1UPH,
	IN BOOLEAN	fgMinPwrFlag
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCUlAlgoCtrl(pAd, u1TpcCmd, u1ApTxPwr, u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag)
			== NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcUlAlgoCtrl(pAd, u1TpcCmd, u1ApTxPwr, u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag)
			== RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}


INT TpcUlUtVarCfg(
	PRTMP_ADAPTER pAd,
	UINT8 u1EntryIdx,
	UINT8 u1VarType,
	INT16 i2Value
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCUlUtVarCfg(pAd, u1EntryIdx, u1VarType, i2Value)
			== NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcUlUtVarCfg(pAd, u1EntryIdx, u1VarType, i2Value)
			== RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT TpcAlgoUtGo(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTpcUtGo
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCUlUtGo(pAd, fgTpcUtGo) == NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcUlUtGo(pAd, fgTpcUtGo) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}


INT TpcDlAlgoCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8	u1TpcCmd,
	IN BOOLEAN	fgCmdCtrl,
	IN UINT8	u1DlTxType,
	IN CHAR		DlTxPwr,
	IN UINT8	u1EntryIdx,
	IN INT16	DlTxpwrAlpha
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCDlAlgoCtrl(pAd, u1TpcCmd, fgCmdCtrl, u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha)
			== NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcDlAlgoCtrl(pAd, u1TpcCmd, fgCmdCtrl, u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha)
			== RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT TpcManTblInfo(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN	fgUplink
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		if (UniCmdTPCManTblInfo(pAd, fgUplink) == NDIS_STATUS_SUCCESS)
			fgStatus = TRUE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdTpcManTblInfo(pAd, fgUplink) == RETURN_STATUS_TRUE)
			fgStatus = TRUE;
	}

	return fgStatus;
}

INT support_rate_table_ctrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 tx_mode,
	IN UINT8 tx_nss,
	IN UINT8 tx_bw,
	IN UINT16 *mcs_cap
)
{
	BOOLEAN status = FALSE;

	if (mt_cmd_support_rate_table_ctrl(pAd, tx_mode, tx_nss, tx_bw, mcs_cap, TRUE) == RETURN_STATUS_TRUE)
		status = TRUE;

	return status;
}

INT support_rate_table_info(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 tx_mode,
	IN UINT8 tx_nss,
	IN UINT8 tx_bw,
	IN UINT16 *mcs_cap
)
{
	BOOLEAN status = FALSE;

	if (mt_cmd_support_rate_table_ctrl(pAd, tx_mode, tx_nss, tx_bw, mcs_cap, FALSE) == RETURN_STATUS_TRUE)
		status = TRUE;

	return status;
}

INT ra_dbg_ctrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 param_num,
	IN UINT32 *param
)
{
	BOOLEAN status = FALSE;

	if (mt_cmd_ra_dbg_ctrl(pAd, param_num, param) == RETURN_STATUS_TRUE)
		status = TRUE;

	return status;
}

#ifdef SINGLE_SKU_V2
INT TxPowerSKUCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN tx_pwr_sku_en,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPowerSKUCtrl(pAd, tx_pwr_sku_en, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerBfBackoffCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTxBFBackoffEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxBfBackoffCtrl(pAd, fgTxBFBackoffEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* SINGLE_SKU_V2 */

INT TxPowerManualCtrl(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR		ucBandIdx,
	IN INT_8		cTxPower,
	IN UINT8		ucPhyMode,
	IN UINT8		ucTxRate,
	IN UINT8		ucBW
)
{

	BOOLEAN  fgStatus = FALSE;
#ifdef CONFIG_ATE
	if (MtCmdSetForceTxPowerCtrl(pAd, ucBandIdx, cTxPower, ucPhyMode, ucTxRate, ucBW) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;
#endif
	return fgStatus;
}

INT TxPowerPercentCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgTxPowerPercentEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;
#ifdef CONFIG_ATE
	if (MtCmdTxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;
#endif
	return fgStatus;
}

INT TxPowerDropCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 ucPowerDrop,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;
	INT8  cPowerDropLevel = 0;

	/* config Tx Power Drop value */
	if ((ucPowerDrop > 90) && (ucPowerDrop < 100))
		cPowerDropLevel = 0;
	else if ((ucPowerDrop > 60) && (ucPowerDrop <= 90))  /* reduce Pwr for 1 dB. */
		cPowerDropLevel = 2;
	else if ((ucPowerDrop > 30) && (ucPowerDrop <= 60))  /* reduce Pwr for 3 dB. */
		cPowerDropLevel = 6;
	else if ((ucPowerDrop > 15) && (ucPowerDrop <= 30))  /* reduce Pwr for 6 dB. */
		cPowerDropLevel = 12;
	else if ((ucPowerDrop > 9) && (ucPowerDrop <= 15))   /* reduce Pwr for 9 dB. */
		cPowerDropLevel = 18;
	else if ((ucPowerDrop > 0) && (ucPowerDrop <= 9))   /* reduce Pwr for 12 dB. */
		cPowerDropLevel = 24;

	if (MtCmdTxPowerDropCtrl(pAd, cPowerDropLevel, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxCCKStreamCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1CCKTxStream,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	MTWF_PRINT("u1CCKTxStream = %d \n", u1CCKTxStream);

	/* Work around - profile setting*/
	if (!u1CCKTxStream)
		u1CCKTxStream = 1;

	/* sanity check for input parameter range */
	if (u1CCKTxStream >= WF_NUM) {
		MTWF_PRINT("set wrong parameters\n");
		return FALSE;
	}

	return fgStatus;
}

INT ThermoCompCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgThermoCompEn,
	IN UCHAR ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdThermoCompCtrl(pAd, fgThermoCompEn, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerRfTxAnt(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 ucTxAntIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrRfTxAntCtrl(pAd, ucTxAntIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxPowerShowInfo(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR            ucTxPowerInfoCatg,
	IN UINT8            ucBandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

#ifdef WIFI_EAP_FEATURE
INT InitIPICtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdInitIPICtrl(pAd, BandIdx))
		status = 1;

	return status;
}

INT GetIPIValue(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdGetIPIValue(pAd, BandIdx))
		status = 1;

	return status;
}

INT SetDataTxPwrOffset(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 WlanIdx,
	IN INT8 TxPwr_Offset,
	IN UINT8 BandIdx
)
{
	INT status = 0;

	if (RETURN_STATUS_TRUE == MtCmdSetDataTxPwrOffset(pAd, WlanIdx, TxPwr_Offset, BandIdx))
		status = 1;

	return status;
}

INT SetFwRaTable(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx,
	IN UINT8 TblType,
	IN UINT8 TblIndex,
	IN UINT16 TblLength,
	PUCHAR Buffer
)
{
	INT status = 0;

	if (MtCmdSetRaTable(pAd, BandIdx, TblType, TblIndex, TblLength, Buffer) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}

INT GetRaTblInfo(
	PRTMP_ADAPTER pAd,
	UINT8 BandIdx,
	UINT8 TblType,
	UINT8 TblIndex,
	UINT8 ReadnWrite
)
{
	INT status = 0;

	if (MtCmdGetRaTblInfo(pAd, BandIdx, TblType, TblIndex, ReadnWrite) == RETURN_STATUS_TRUE)
		status = 1;

	return status;
}

#endif /* WIFI_EAP_FEATURE */

INT SetEDCCAThreshold(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1edcca_threshold[],
	IN UINT8 u1BandIdx
)
{
	INT status = FALSE;

	if (MtCmdSetEDCCAThreshold(pAd, (PUCHAR)u1edcca_threshold, u1BandIdx) == RETURN_STATUS_TRUE)
		status = TRUE;

	return status;
}


INT SetEDCCAEnable(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8            u1EDCCACtrl,
	IN UINT8            u1BandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdSetEDCCACEnable(pAd, u1BandIdx, u1EDCCACtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


NDIS_STATUS EDCCAInit(
	IN PRTMP_ADAPTER	pAd,
	UINT8 u1BandIdx
)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UINT8 u1EDCCAStd;
	USHORT radioPhy = HcGetRadioPhyModeByBandIdx(pAd);

	if (pAd->CommonCfg.u1EDCCACtrl) {
		UINT8 EDCCA_threshold[EDCCA_MAX_BW_NUM] = {0xcf, 0x7f, 0x7f, 0x7f};

		/* EDCCA mode check */
		if (pAd->CommonCfg.u1EDCCAMode == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"%s(): EDCCA mode is not set\n", __func__);
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support) {
						if (UniCmdSetEDCCAThreshold(pAd,
													(PUCHAR)pAd->CommonCfg.u1EDCCAThreshold,
													u1BandIdx, TRUE) != TRUE) {
							MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
									"EDCCA threshold set fail\n");
							return NDIS_STATUS_FAILURE;
						}
					} else
#endif /* WIFI_UNIFIED_COMMAND */
					{
						if (SetEDCCAThreshold(pAd,
												(PUCHAR)pAd->CommonCfg.u1EDCCAThreshold,
												u1BandIdx) != TRUE) {
							MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
									"EDCCA threshold set fail\n");
							return NDIS_STATUS_FAILURE;
						}
					}
		} else {
			os_move_mem(pAd->CommonCfg.u1EDCCAThreshold, EDCCA_threshold, EDCCA_MAX_BW_NUM);
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support) {
				if (UniCmdSetEDCCAThreshold(pAd, (PUCHAR)EDCCA_threshold, u1BandIdx, TRUE) != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
							"EDCCA threshold set fail\n");
					return NDIS_STATUS_FAILURE;
				}
			} else
#endif /* WIFI_UNIFIED_COMMAND */
			{
				if (SetEDCCAThreshold(pAd, (PUCHAR)EDCCA_threshold, u1BandIdx) != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					"EDCCA threshold set fail by mode\n");
					return NDIS_STATUS_FAILURE;
				}
			}
		}

		/*Get the default Threshold setting*/
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			if (UniCmdGetEDCCAThreshold(pAd, u1BandIdx, TRUE) != TRUE)
				return NDIS_STATUS_FAILURE;
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			if (MtCmdGetEDCCAThreshold(pAd, u1BandIdx, TRUE) != RETURN_STATUS_TRUE)
				return NDIS_STATUS_FAILURE;
		}
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"%s(): record edccathreshold default value\n", __func__);
	}

	u1EDCCAStd = GetEDCCAStd(pAd, pAd->CommonCfg.CountryCode, radioPhy);

	/*Set the default EDCCAEnable setting*/
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		if (UniCmdSetEDCCAEnable(pAd, pAd->CommonCfg.u1EDCCACtrl, u1BandIdx, u1EDCCAStd) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"EDCCA CTRL set fail\n");
			return NDIS_STATUS_FAILURE;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (SetEDCCAEnable(pAd, pAd->CommonCfg.u1EDCCACtrl, u1BandIdx) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"EDCCA CTRL set fail\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"%s(): EDCCA init success!!!!!!!!!\n", __func__);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WIFI_GPIO_CTRL
INT SetGpioCtrl(PRTMP_ADAPTER pAd, UINT8 GpioIdx, BOOLEAN GpioEn)
{
	INT status = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_GPIO_CFG_PARAM_T GpioCfgParam;

	os_zero_mem(&GpioCfgParam, sizeof(GpioCfgParam));
	GpioCfgParam->ucGpioIdx = GpioIdx;
	GpioCfgParam.GpioEnable.fgEnable = GpioEn;
	GpioCfgParam.GpioCfgValid[UNI_CMD_GPIO_ENABLE] = TRUE;

	if (cap->uni_cmd_support) {
		if (UniCmdGPIO(pAd, &GpioCfgParam) == NDIS_STATUS_SUCCESS)
			status = 1;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdSetGpioCtrl(pAd, GpioIdx, GpioEn) == RETURN_STATUS_TRUE)
			status = 1;
	}

	return status;
}

INT SetGpioValue(PRTMP_ADAPTER pAd, UINT8 GpioIdx, UINT8 GpioVal)
{
	INT status = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_GPIO_CFG_PARAM_T GpioCfgParam;

	os_zero_mem(&GpioCfgParam, sizeof(GpioCfgParam));
	GpioCfgParam->ucGpioIdx = GpioIdx;
	GpioCfgParam.GpioSetValue.ucGpioValue = GpioVal;
	GpioCfgParam.GpioCfgValid[UNI_CMD_GPIO_SET_VALUE] = TRUE;

	if (cap->uni_cmd_support) {
		if (UniCmdGPIO(pAd, &GpioCfgParam) == NDIS_STATUS_SUCCESS)
			status = 1;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdSetGpioVal(pAd, GpioIdx, GpioVal) == RETURN_STATUS_TRUE)
			status = 1;
	}

	return status;
}
#endif /* WIFI_GPIO_CTRL */

INT TOAECtrlCmd(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR                TOAECtrl
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTOAECalCtrl(pAd, TOAECtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT MuPwrCtrlCmd(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgMuTxPwrManEn,
	IN CHAR cMuTxPwr,
	IN UINT8 u1BandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdMuPwrCtrl(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

#ifdef DATA_TXPWR_CTRL
INT TxPwrDataFrameCtrlCmd(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT8 i1MaxBasePwr,
	IN UINT8 u1BandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrDataPktCtrl(pAd, pEntry, i1MaxBasePwr, u1BandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;

}
INT TxPwrMinLimitDataFrameCtrl(
	IN PRTMP_ADAPTER pAd,
	IN INT8 i1MinBasePwr,
	IN UINT8 u1BandIdx
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrMinDataPktCtrl(pAd, i1MinBasePwr, u1BandIdx) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

#endif

INT BFNDPATxDCtrlCmd(
	IN PRTMP_ADAPTER        pAd,
	IN BOOLEAN              fgNDPA_ManualMode,
	IN UINT8                ucNDPA_TxMode,
	IN UINT8                ucNDPA_Rate,
	IN UINT8                ucNDPA_BW,
	IN UINT8                ucNDPA_PowerOffset
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdBFNDPATxDCtrl(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW,
						   ucNDPA_PowerOffset) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT RxvEnCtrl(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN fgRxvEnCtrl
)
{
	BOOLEAN fgStatus = FALSE;

	if (mt_cmd_set_rxv_ctrl(pAd, fgRxvEnCtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT RxvRuCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1RxvRuCtrl
)
{
	BOOLEAN fgStatus = FALSE;

	if (mt_cmd_set_rxv_ru_ctrl(pAd, u1RxvRuCtrl) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT ThermalManCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1BandIdx,
	IN BOOLEAN fgManualMode,
	IN UINT8 u1ThermalAdc
)
{
	INT i4Status = FALSE;

	if (MtCmdThermalManCtrl(pAd, u1BandIdx, fgManualMode, u1ThermalAdc) == NDIS_STATUS_SUCCESS)
		i4Status = TRUE;

	return i4Status;
}

INT ThermalTaskCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1BandIdx,
	IN BOOLEAN fgTrigEn,
	IN UINT8 u1Thres
)
{
	INT i4Status = FALSE;
	UINT32 u4FuncPtr = 0;

	/* update function pointer */
	u4FuncPtr = *(UINT32 *)ThermalTaskAction;

	if (MtCmdThermalTaskCtrl(pAd, u1BandIdx, fgTrigEn, u1Thres, u4FuncPtr) == NDIS_STATUS_SUCCESS)
		i4Status = TRUE;

	return i4Status;
}

INT ThermalTaskAction(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u4PhyIdx,
	IN UINT32 u4ThermalTaskProp,
	IN UINT8 u1ThermalAdc
)
{
	MTWF_PRINT("%s(): u4PhyIdx: %d, u4ThermalTaskProp: %d, u1ThermalAdc: %d\n", __func__, u4PhyIdx, u4ThermalTaskProp, u1ThermalAdc);

	return TRUE;
}

INT ThermalBasicInfo(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1BandIdx
)
{
	INT8 i4Status = FALSE;

	if (MtCmdThermalBasicInfo(pAd, u1BandIdx) == NDIS_STATUS_SUCCESS)
		i4Status = TRUE;

	return i4Status;
}

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
INT TxPwrUpCtrl(
	PRTMP_ADAPTER   pAd,
	UINT8           ucBandIdx,
	CHAR            cPwrUpCat,
	signed char     *cPwrUpValue,
	UCHAR            cPwrUpValLen
)
{
	BOOLEAN  fgStatus = FALSE;

	if (MtCmdTxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue, cPwrUpValLen) ==
			RETURN_STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

/* [channel_band] 0: 2.4G, 1: 5G*/
UINT8 TxPowerGetChBand(
	IN UINT8				ucBandIdx,
	IN UINT8				CentralCh
)
{
	UINT8	ChannelBand = 0;

	if (CentralCh >= 14)
		ChannelBand = 1;
	else {
		if (ucBandIdx == 0)
			ChannelBand = 0;
		else
			ChannelBand = 1;
	}

	return ChannelBand;
}

INT SetCPSupportEnable(PRTMP_ADAPTER pAd, UINT8  cp_support)
{
	RTMP_CHIP_CAP *pChipCap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

	if ((cp_support >= 1) && (cp_support <= 3)) {
		if (pChipCap->asic_caps & fASIC_CAP_MCU_OFFLOAD) {
			MtCmdSetCPSEnable(pAd, HOST2CR4, cp_support);
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_INFO,
					 "set CR4 CP_SUPPORT to Mode %d.\n", cp_support);
		} else {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_INFO,
					 "set Driver CP_SUPPORT to Mode %d.\n", cp_support);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_ERROR,
				 "The CP Mode is invaild. Mode should be 1~3.\n");
		return FALSE;
	}

	return TRUE;
}

INT set_cp_support_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT32 rv, Mode;

	if (arg) {
		rv = sscanf(arg, "%d", &Mode);

		if ((rv > 0) && (Mode >= 1) && (Mode <= 3)) {
			pAd->cp_support = Mode;

			return SetCPSupportEnable(pAd, pAd->cp_support);
		} else {
			MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_TRCTRL, DBG_LVL_ERROR,
					 "The Mode is invaild. Mode should be 1~3.\n");
		}
	}

	return FALSE;
}

INT Show_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR band_idx = 0;

	band_idx = hc_get_hw_band_idx(pAd);

#ifdef ERR_RECOVERY
	if (ser_is_idle_state(pAd)) {
#endif
		MTWF_PRINT("====Band%d Enable = %d====\n", band_idx, pAd->OneSecMibBucket.Enabled);
		MTWF_PRINT("Channel Busy Time = %u\n", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx);
		MTWF_PRINT("Primary Channel Busy Time = %u\n", pAd->OneSecMibBucket.ChannelBusyTime);
		MTWF_PRINT("OBSS Air Time = %u\n", pAd->OneSecMibBucket.OBSSAirtime);
		MTWF_PRINT("My Tx Air Time = %u\n", pAd->OneSecMibBucket.MyTxAirtime);
		MTWF_PRINT("My Rx Air Time = %u\n", pAd->OneSecMibBucket.MyRxAirtime);
		MTWF_PRINT("EDCCA Time = %u\n", pAd->OneSecMibBucket.EDCCAtime);
		if (!IS_MT7915(pAd)) {
			MTWF_PRINT("PD count = %x\n", pAd->OneSecMibBucket.PdCount);
			MTWF_PRINT("MDRDY Count = %x\n", pAd->OneSecMibBucket.MdrdyCount);
		}

		MTWF_PRINT("Rx Cck Mdry Time = %u\n", pAd->OneSecMibBucket.RxCckMdrdyTime);
		MTWF_PRINT("Rx Ofdm Lg Mixed Mdry Time = %u\n", pAd->OneSecMibBucket.RxOfdmLgMixedMdrdyTime);
		MTWF_PRINT("Rx Ofdm Green Mdry Time = %u\n", pAd->OneSecMibBucket.RxOfdmGreenMdrdyTime);
		MTWF_PRINT("My MAC2PHY Tx Time = %u\n", pAd->OneSecMibBucket.MyMac2phyTxTime);

		MTWF_PRINT("after ajust, my tx airtime = %u\n", pAd->OneSecMibBucket.MyAjustTxAirTime);
		MTWF_PRINT("after ajust, my rx airtime = %u\n", pAd->OneSecMibBucket.MyAjustRxAirTime);
		MTWF_PRINT("after ajust, obss airtime = %u\n", pAd->OneSecMibBucket.AjustObssAirTime);

		MTWF_PRINT("after ajust, MibBss0CtrlFrameCntTime = %u\n", pAd->OneSecMibBucket.MibBss0CtrlFrameCntTime);
		MTWF_PRINT("after ajust, MibCntAmpduTime = %u\n", pAd->OneSecMibBucket.MibCntAmpduTime);
#ifdef ERR_RECOVERY
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"====Band%d Enable = %d====\n", band_idx, pAd->OneSecMibBucket.Enabled);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"Channel Busy Time = %u\n", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"Primary Channel Busy Time = %u\n", pAd->OneSecMibBucket.ChannelBusyTime);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"OBSS Air Time = %u\n", pAd->OneSecMibBucket.OBSSAirtime);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"My Tx Air Time = %u\n", pAd->OneSecMibBucket.MyTxAirtime);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"My Rx Air Time = %u\n", pAd->OneSecMibBucket.MyRxAirtime);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"EDCCA Time = %u\n", pAd->OneSecMibBucket.EDCCAtime);
		if (!IS_MT7915(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
					"PD count = %x\n", pAd->OneSecMibBucket.PdCount);
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
					"MDRDY Count = %x\n", pAd->OneSecMibBucket.MdrdyCount);
		}
	}
#endif

	return TRUE;
}

#ifdef GN_MIXMODE_SUPPORT
VOID gn_mixmode_is_enable(PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.GNMixMode) {
		MtCmdSetGNMixModeEnable(pAd, HOST2CR4, pAd->CommonCfg.GNMixMode);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HT, DBG_LVL_INFO,
		"set CR4/N9 GN MIXMODE Enable to %d.\n", pAd->CommonCfg.GNMixMode);
	}
}
#endif /* GN_MIXMODE_SUPPORT */

#ifdef DHCP_UC_SUPPORT
static UINT32  checksum(PUCHAR buf, INT32  nbytes, UINT32 sum)
{
	uint i;

	/* Checksum all the pairs of bytes first... */
	for (i = 0; i < (nbytes & ~1U); i += 2) {
		sum += (UINT16)ntohs(*((u_int16_t *)(buf + i)));

		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	/*
	 * If there's a single byte left over, checksum it, too.
	 * Network byte order is big-endian, so the remaining byte is
	 * the high byte.
	 */
	if (i < nbytes) {
		sum += buf[i] << 8;

		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}

	return sum;
}

static UINT32 wrapsum(UINT32 sum)
{
	UINT32 ret;

	sum = ~sum & 0xFFFF;
	ret = htons(sum);
	return ret;
}
UINT16 RTMP_UDP_Checksum(IN PNDIS_PACKET pSkb)
{
	PUCHAR pPktHdr, pLayerHdr;
	PUCHAR pPseudo_Hdr;
	PUCHAR pPayload_Hdr;
	PUCHAR pUdpHdr;
	UINT16 udp_chksum;
	UINT16 udp_len;
	UINT16 payload_len;

	pPktHdr = GET_OS_PKT_DATAPTR(pSkb);

	if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pSkb)))
		pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
	else
		pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

	pUdpHdr = pLayerHdr + 20;
	pPseudo_Hdr = pUdpHdr - 8;
	pPayload_Hdr = pUdpHdr + 8;
	udp_len = ntohs(*((UINT16 *) (pUdpHdr + 4)));
	payload_len = udp_len - 8;
	udp_chksum = wrapsum(
					 checksum(
						 pUdpHdr,
						 8,
						 checksum(
							 pPayload_Hdr,
							 payload_len,
							 checksum(
								 (unsigned char *)pPseudo_Hdr,
								 2 * 4,
								 17 + udp_len)))
				 );
	return udp_chksum;
}
#endif /* DHCP_UC_SUPPORT */

#ifdef ERR_RECOVERY
INT32 ShowSerProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef HWIFI_SUPPORT
	MTWF_PRINT("%s,::E R , stat=0x%08X\n",
			 __func__, ser_get_curr_state(pAd));
	/* Dump SER related CRs */
	ser_dump_ser_stat(pAd, TRUE);
	/* print out ser log timing */
	ser_dump_timelog(pAd, SER_LV_1_0);

#endif /* HWIFI_SUPPORT */

	return TRUE;
}

INT32 ShowSerProc2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	ShowSerProc(pAd, arg);

#ifdef DUMMY_N9_HEART_BEAT
	if (1) {
		UINT32 reg_tmp_val;

		MAC_IO_READ32(pAd->hdev_ctrl, DUMMY_N9_HEART_BEAT, &reg_tmp_val);

		MTWF_PRINT("HeartBeat 0x%x = %d\n",
			DUMMY_N9_HEART_BEAT, reg_tmp_val);
	}
#endif

	/* We will get more info from FW */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdSER(pAd, UNI_SER_ACTION_SET_QUERY, 0, DBDC_BAND0);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		CmdExtSER(pAd, SER_ACTION_QUERY, 0, DBDC_BAND0);

#ifdef WF_RESET_SUPPORT
	/* log wf reset status */
	MTWF_PRINT("WF_RESET WM %d WA %d WO %d\n",
		PD_GET_WF_RESET_WM_COUNT(pAd->physical_dev),
		PD_GET_WF_RESET_WA_COUNT(pAd->physical_dev),
		PD_GET_WF_RESET_WO_COUNT(pAd->physical_dev));
#endif

	return TRUE;
}

#endif

INT32 ShowBcnProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	UINT32 band_idx = 0;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
		if (arg != NULL && band_idx != os_str_toul(arg, 0, 10))
			continue;

		MTWF_PRINT("%s, Band %d\n", __func__, band_idx);
		MTWF_PRINT("===============================\n");
		if (chip_dbg->show_bcn_info)
			chip_dbg->show_bcn_info(pAd->hdev_ctrl, band_idx);
	}

	MTWF_PRINT("===============================\n");
#endif
	return TRUE;
}

#if (defined(CFG_SUPPORT_FALCON_MURU) || defined(CFG_SUPPORT_MU_MIMO))
INT ShowMuMimoGroupTblEntry(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_mumimo_group_entry_tbl)
		ops->show_mumimo_group_entry_tbl(pAd, arg);
	else
		return FALSE;

	return TRUE;
}
#endif /*CFG_SUPPORT_FALCON_MURU || CFG_SUPPORT_MU_MIMO*/

#if (defined(CFG_SUPPORT_MU_MIMO_RA) || defined(CFG_SUPPORT_FALCON_MURU))
INT ShowMuMimoAlgorithmMonitor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_mumimo_algorithm_monitor)
		ops->show_mumimo_algorithm_monitor(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT32 SetMuMimoFixedRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_mumimo_fixed_rate)
		ops->set_mumimo_fixed_rate(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT32 SetMuMiMoFixedGroupRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_mumimo_fixed_group_rate)
		ops->set_mumimo_fixed_group_rate(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT SetMuMimoForceMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = 0;
	BOOLEAN fgForceMU;

	if (arg != NULL)
		fgForceMU = os_str_tol(arg, 0, 10);
	else
		goto error;

	if (fgForceMU)
		fgForceMU = MUMIMO_FORCE_ENABLE_MU;

	if (SetMuMimoForceMUEnable(pAd, fgForceMU) == TRUE)
		status = 1;

error:
	MTWF_PRINT("%s:(status = %d\n", __func__, status);
	return status;
}

INT32 SetMuMimoForceMUEnable(RTMP_ADAPTER *pAd, BOOLEAN fgForceMu)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_mumimo_force_mu_enable)
		ops->set_mumimo_force_mu_enable(pAd, fgForceMu);
	else
		return FALSE;

	return TRUE;
}

#endif /*CFG_SUPPORT_MU_MIMO_RA || CFG_SUPPORT_FALCON_MURU*/

#ifdef TX_POWER_CONTROL_SUPPORT
static INT32 SetTxPowerBoostInfo_V0V1(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8 u1BandIdx = 0;
	CHAR	*value = 0;
	UINT8	u1Bw, u1PhyMode, u1PwrUpCat = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter band index\n");
		return -1;
	}
	u1BandIdx = simple_strtol(value, 0, 10);

	/* sanity check for Band index */
	if (u1BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!!\n");
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter PhyMode\n");
		return -1;
	}
	u1PhyMode = simple_strtol(value, 0, 10);

	value = strsep(&arg, "");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter bandwidth\n");
		return -1;
	}
	u1Bw = simple_strtol(value, 0, 10);

	if (arch_ops && arch_ops->arch_txpower_boost_power_cat_type) {
		if (!arch_ops->arch_txpower_boost_power_cat_type(pAd, u1PhyMode, u1Bw, &u1PwrUpCat))
			return FALSE;
	}

	MTWF_PRINT("=======================================================\n");
	MTWF_PRINT("Power Up Table (Band%d)\n", u1BandIdx);
	MTWF_PRINT("=======================================================\n");

	if (arch_ops && arch_ops->arch_txpower_boost_rate_type) {
		if (!arch_ops->arch_txpower_boost_rate_type(pAd, u1BandIdx, u1PwrUpCat))
			return FALSE;
	}

	return TRUE;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef TX_POWER_CONTROL_SUPPORT_V2
static INT32 SetTxPowerBoostInfo_V2(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8 u1BandIdx = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	u1BandIdx = hc_get_hw_band_idx(pAd);

	/* sanity check for Band index */
	if (u1BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!!\n");
		return FALSE;
	}

	MTWF_PRINT("\033[1;33mPower Up Table (Band%d)\x1b[m\n", u1BandIdx);

	if (arch_ops && arch_ops->arch_txpower_boost_info_V2) {
		if (!arch_ops->arch_txpower_boost_info_V2(pAd, 0))
			return FALSE;
	}

	return TRUE;
}
#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
INT SetTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef TX_POWER_CONTROL_SUPPORT_V2
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->tx_power_boost_ver == 2) {
		SetTxPowerBoostInfo_V2(pAd, arg);
		return TRUE;
	}
#endif

#ifdef TX_POWER_CONTROL_SUPPORT
	SetTxPowerBoostInfo_V0V1(pAd, arg);
#endif
	return TRUE;
}
#endif /* defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2) */

#ifdef ETSI_RX_BLOCKER_SUPPORT
INT32 ShowRssiThInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("--------------------------------------------------------------\n");
	MTWF_PRINT("%s: c1RWbRssiHTh: %d, c1RWbRssiLTh: %d, c1RIbRssiLTh: %d,c1WBRssiTh4R: %d\n", __FUNCTION__,
															pAd->c1RWbRssiHTh, pAd->c1RWbRssiLTh, pAd->c1RIbRssiLTh, pAd->c1WBRssiTh4R);
	MTWF_PRINT("--------------------------------------------------------------\n");
	return TRUE;
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#ifdef EEPROM_RETRIEVE_SUPPORT
static UINT8 g_dump_content[MAX_EEPROM_BUFFER_SIZE];
INT32 show_e2p_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int status = 0;
	PCHAR pch = NULL;
	UINT16 dump_offset = 0x0;
	UINT16 dump_size = 0x20;
	UINT8 *dump_content = &g_dump_content[0];
	int i;
	if (arg != NULL) {
		pch = strsep(&arg, "-");
		if (pch != NULL)
			dump_offset = os_str_tol(pch, 0, 10);

		pch = strsep(&arg, "-");
		if (pch != NULL)
			dump_size = os_str_tol(pch, 0, 10);
	}

	MTWF_PRINT("\x1b[1;33m %s: eeprom_type=%d, offset=%d, size=%d\x1b[m \n",
		__func__, PD_GET_EEPROM_TYPE(pAd->physical_dev), dump_offset, dump_size);

	MtCmdEfusBufferModeGet(pAd, EEPROM_EFUSE, dump_offset, dump_size, dump_content);

	MTWF_PRINT("Dump\n\r");
	for (i = 0; i < dump_size; i++) {
		if ((i%32) == 0)
			MTWF_PRINT("\n\r");
		MTWF_PRINT("%02x ", dump_content[i]);
	}
	MTWF_PRINT("\n\r");
	MTWF_PRINT("\x1b[1;33m %s: End \x1b[m \n", __func__);
	return TRUE;
}
#endif /* EEPROM_RETRIEVE_SUPPORT */

INT32 show_hwcfg_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_hwcfg_info)
		ops->show_hwcfg_info(pAd);
	else
		return FALSE;

	return TRUE;
}

#if defined(ANDLINK_FEATURE_SUPPORT) || defined(TR181_SUPPORT)
INT get_sta_rate_info(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	mtk_rate_info_t *tx_rate_info,
	mtk_rate_info_t *rx_rate_info) {

	u16 wcid = 0;
	struct wifi_dev *wdev = NULL;
	STA_ADMIN_CONFIG *pstacfg = NULL;
	ADD_HT_INFO_IE *addht= NULL;
	ULONG datarate_tx = 0;
	ULONG datarate_rx = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (!pEntry || !tx_rate_info || !rx_rate_info) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
		"ERROR!! pEntry=%p, tx_rate_info=%p, rx_rate_info=%p\n",
		pEntry, tx_rate_info, rx_rate_info);
		return FALSE;
	}

	wcid = pEntry->wcid;
	wdev = pEntry->wdev;
#ifdef CONFIG_STA_SUPPORT
	pstacfg = (STA_ADMIN_CONFIG *) GetStaCfgByWdev(pAd, wdev);
#endif /*CONFIG_STA_SUPPORT*/
	if (pEntry->Sst != SST_ASSOC) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"idx: %d, mac:"MACSTR" is disassociated\n",
			pEntry->wcid, MAC2STR(pEntry->Addr));
		return FALSE;
	}

	addht = wlan_operate_get_addht(pEntry->wdev);
	getRate(pEntry->HTPhyMode, &datarate_tx);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
			UCHAR nss;
			UCHAR nss_r;
			UINT32 RawData;
			UINT32 lastTxRate;
			UINT32 lastRxRate = pEntry->LastRxRate;
			UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);

			EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
			EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
			union _HTTRANSMIT_SETTING LastTxRate;
			union _HTTRANSMIT_SETTING LastRxRate;

			MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
			LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
			LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
			LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
			LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
			LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

			if (LastTxRate.field.MODE >= MODE_VHT)
				LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
			else if (LastTxRate.field.MODE == MODE_OFDM)
				LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
			else
				LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

			lastTxRate = (UINT32)(LastTxRate.word);
			LastRxRate.word = (USHORT)lastRxRate;
			RawData = lastTxRate;
			phy_mode = rTxStatResult.rEntryTxRate.MODE;
			rate = RawData & 0x3F;
			bw = (RawData >> 7) & 0x7;  // max bw enum is 7
			sgi = rTxStatResult.rEntryTxRate.ShortGI;
			stbc = ((RawData >> 10) & 0x1);
			nss = rTxStatResult.rEntryTxRate.VhtNss;

			MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, (UINT32 *)&rRxStatResult);
			LastRxRate.field.MODE = rRxStatResult.u1RxMode;
			LastRxRate.field.BW = rRxStatResult.u1BW;
			LastRxRate.field.ldpc = rRxStatResult.u1Coding;
			LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
			LastRxRate.field.STBC = rRxStatResult.u1Stbc;

			if (LastRxRate.field.MODE >= MODE_VHT)
				LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
			else if (LastRxRate.field.MODE == MODE_OFDM)
				LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate & 0xF);
			else
				LastRxRate.field.MCS = rRxStatResult.u1RxRate;

			phy_mode_r = rRxStatResult.u1RxMode;
			rate_r = rRxStatResult.u1RxRate & 0x3F;
			bw_r = rRxStatResult.u1BW;
			sgi_r = rRxStatResult.u1Gi;
			stbc_r = rRxStatResult.u1Stbc;
			nss_r = rRxStatResult.u1RxNsts + 1;
	/*TX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode >= MODE_VHT) {
				rate = rate & 0xF;
			}
#endif /* DOT11_VHT_AC */


	/*RX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode_r >= MODE_VHT) {
				nss_r = (rRxStatResult.u1RxNsts + 1) / (rRxStatResult.u1Stbc + 1);
				rate_r = rate_r & 0xF;
			} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
			if (phy_mode_r >= MODE_HTMIX) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"MODE_HTMIX(%d >= %d): MCS=%d\n",
					phy_mode_r, MODE_HTMIX, rate_r);
			} else
#endif
			if (phy_mode_r == MODE_OFDM) {
				rate_r = rate_r & 0xF;
				if (rate_r == TMI_TX_RATE_OFDM_6M)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_OFDM_9M)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_OFDM_12M)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_OFDM_18M)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_OFDM_24M)
					LastRxRate.field.MCS = 4;
				else if (rate_r == TMI_TX_RATE_OFDM_36M)
					LastRxRate.field.MCS = 5;
				else if (rate_r == TMI_TX_RATE_OFDM_48M)
					LastRxRate.field.MCS = 6;
				else if (rate_r == TMI_TX_RATE_OFDM_54M)
					LastRxRate.field.MCS = 7;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				rx_rate_info->mcs = rate_r;/*rx mcs: ofdm*/
			} else if (phy_mode_r == MODE_CCK) {
				rate_r = rate_r & 0xF;
				if (rate_r == TMI_TX_RATE_CCK_1M_LP)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
					LastRxRate.field.MCS = 3;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				rx_rate_info->mcs = LastRxRate.field.MCS;/*rx_mcs:cck*/
			}

			if (phy_mode >= MODE_HE) {
				/*ax tx */
#ifdef DOT11_EHT_BE
				if ((phy_mode == MODE_EHT) || (phy_mode == MODE_EHT_ER_SU) || (phy_mode == MODE_EHT_TB) || (phy_mode == MODE_EHT_MU))
					get_rate_eht((rate & 0xf), bw, nss, 0, &datarate_tx);
				else
#endif
					get_rate_he((rate & 0xf), bw, nss, 0, &datarate_tx);
				if (sgi == 1)
					datarate_tx = (datarate_tx * 967) >> 10;
				if (sgi == 2)
					datarate_tx = (datarate_tx * 870) >> 10;

				/*tx rate infos*/
				tx_rate_info->flags |= MTK_RATE_INFO_FLAGS_HE_MCS;
				tx_rate_info->mcs = rate;
				tx_rate_info->legacy = (UINT16)datarate_tx;
				tx_rate_info->nss = nss;
				tx_rate_info->bw = bw;
			} else {
				tx_rate_info->gi = sgi;/*tx_gi*/

				getRate(LastTxRate, &datarate_tx);
				/*tx rate infos*/
				if (phy_mode >= MODE_VHT) {
					tx_rate_info->flags |= MTK_RATE_INFO_FLAGS_VHT_MCS;
				} else if (phy_mode >= MODE_HTMIX) {
					tx_rate_info->flags |= MTK_RATE_INFO_FLAGS_MCS;
				} else {
					tx_rate_info->flags = 0;/*other as legacy*/
				}
				tx_rate_info->mcs = rate;
				tx_rate_info->legacy = (UINT16)datarate_tx;
				tx_rate_info->nss = nss;
				tx_rate_info->bw = bw;
			}

			if (phy_mode_r >= MODE_HE) {
				/*ax rx */
#ifdef DOT11_EHT_BE
				if ((phy_mode_r == MODE_EHT) || (phy_mode_r == MODE_EHT_ER_SU) || (phy_mode_r == MODE_EHT_TB) || (phy_mode_r == MODE_EHT_MU))
					get_rate_eht((rate_r & 0xf), bw_r, nss_r, 0, &datarate_rx);
				else
#endif
					get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &datarate_rx);
				if (sgi_r == 1)
					datarate_rx = (datarate_rx * 967) >> 10;
				if (sgi_r == 2)
					datarate_rx = (datarate_rx * 870) >> 10;

				/*rx rate infos*/
				rx_rate_info->flags |= MTK_RATE_INFO_FLAGS_HE_MCS;
				rx_rate_info->mcs = rate_r;
				rx_rate_info->legacy = (UINT16)datarate_rx;
				rx_rate_info->nss = nss_r;
				rx_rate_info->bw = bw_r;

			} else {
				rx_rate_info->gi = sgi_r;/*rx_gi*/
				getRate(LastRxRate, &datarate_rx);

				/*rx rate infos*/
				if (phy_mode_r >= MODE_VHT) {
					rx_rate_info->flags |= MTK_RATE_INFO_FLAGS_VHT_MCS;
				} else if (phy_mode_r >= MODE_HTMIX) {
					rx_rate_info->flags |= MTK_RATE_INFO_FLAGS_VHT_MCS;
				} else {
					/*other as legacy*/
					rx_rate_info->flags = 0;
				}
				rx_rate_info->mcs = rate_r;
				rx_rate_info->legacy = (UINT16)datarate_rx;
				rx_rate_info->nss = nss_r;
				rx_rate_info->bw = bw_r;
			}

			/*tx rate infos*/
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"\ntx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
				tx_rate_info->legacy,
				tx_rate_info->mcs, tx_rate_info->bw, tx_rate_info->nss);
			/*rx rate infos*/
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"\nrx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
				rx_rate_info->legacy,
				rx_rate_info->mcs, rx_rate_info->bw, rx_rate_info->nss);

		}
		return TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return FALSE;
}

#ifdef ANDLINK_V4_0
INT andlink_send_wifi_change_event(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct andlink_wifi_ch_info *wifi_ch_info)
{
	struct mtk_andlink_event event;
	struct andlink_wifi_ch_info *tmp_ch_info = NULL;

	if (!pAd || !wdev || !wifi_ch_info) {
		return -1;
	}

	if (wdev->if_dev) {
		NdisZeroMemory(&event, sizeof(event));
		event.event_id = ANDLINK_WIFI_CH_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);

		tmp_ch_info = &event.data.wifi_ch_info;
		tmp_ch_info->max_sta_num = wifi_ch_info->max_sta_num;
		tmp_ch_info->is_hidden = wifi_ch_info->is_hidden;
		snprintf(tmp_ch_info->pwd, LEN_PSK, "%s", wifi_ch_info->pwd);/*pwd*/
		snprintf(tmp_ch_info->sec_mode, ANDLINK_SEC_LEN, "%s", wifi_ch_info->sec_mode);/*security*/
		snprintf(tmp_ch_info->ssid, SSID_LEN, "%s", wifi_ch_info->ssid);/*ssid*/
		NdisCopyMemory(tmp_ch_info->ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);
		event.len = sizeof(struct mtk_andlink_event) - sizeof(event.len) - sizeof(event.event_id);

		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_INFO,
			"\nifname=%s ssid=%s sec_mode=%s max_sta_num=%d\n",
			tmp_ch_info->ifname, tmp_ch_info->ssid,
			tmp_ch_info->sec_mode, tmp_ch_info->max_sta_num);

#ifdef RT_CFG80211_SUPPORT
		if (pAd->pCfg80211_CB && wdev->if_up_down_state == TRUE)
			mtk_nl80211_andlink_wifi_change_event(pAd->pCfg80211_CB->pCfg80211_Wdev->wiphy,
				(void *)&event, sizeof(struct mtk_andlink_event));
#else
		/*send wireless event*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, OID_ANDLINK_EVENT_V40,
			NULL, (PUCHAR)&event, sizeof(struct mtk_andlink_event));
#endif
	}

	return 0;
}

#endif/*ANDLINK_V4_0*/


#ifdef ANDLINK_HOSTNAME_IP

NDIS_STATUS update_sta_ip(IN PRTMP_ADAPTER	pAd,
	                   IN PNDIS_PACKET  pPkt) {
    NET_PRO_ARP_HDR * arpHdr = NULL;
    BOOLEAN isUcastMac, isGoodIP;
    PUCHAR	pSMac = NULL;
    PUCHAR pSIP = NULL;
    PUCHAR  pLayerHdr = NULL;
    PUCHAR  pPktHdr = NULL;
    UINT16  protoType;

    if (pAd == NULL && pPkt == NULL) {
	    MTWF_PRINT("pAd or pPkt is NULL!\n");
	    return NDIS_STATUS_FAILURE;
    }

    pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return NDIS_STATUS_FAILURE;

    /* Get the upper layer protocol type of this 802.3 pkt and dispatch to specific handler */
    protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));
    /*ARP request check */
    if (ETH_P_ARP == protoType) {
#ifdef MAC_REPEATER_SUPPORT
        REPEATER_CLIENT_ENTRY *pRepEntry = NULL;
#endif

        /*arp ops check*/
        pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
        arpHdr = (NET_PRO_ARP_HDR *)pLayerHdr;
		/*
		*Check the arp header.
		*We just handle ether type hardware address and IPv4 internet
		*address type and opcode is  ARP reuqest/response.
		*/
		if ((arpHdr->ar_hrd != OS_HTONS(ARPHRD_ETHER)) || (arpHdr->ar_pro != OS_HTONS(ETH_P_IP)) ||
			(arpHdr->ar_op != OS_HTONS(ARPOP_REPLY) && arpHdr->ar_op != OS_HTONS(ARPOP_REQUEST))){
			MTWF_PRINT("is not ARP packet!\n");
			return NDIS_STATUS_FAILURE;
        }

        /*sender(Mac/IP address) fields*/
        pSMac = (pLayerHdr + 8);/*sender mac*/
        pSIP = (PUCHAR)(pSMac + MAC_ADDR_LEN);/*sender ip*/
        isUcastMac = IS_UCAST_MAC(pSMac);
        isGoodIP = IS_GOOD_IP(get_unaligned32((PUINT) pSIP));

#ifdef MAC_REPEATER_SUPPORT
        pRepEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pSMac, TRUE);
        if (pAd->MatCfg.bMACRepeaterEn && pRepEntry != NULL && isUcastMac && isGoodIP) {
            if(pRepEntry != NULL && isUcastMac && isGoodIP) {
                pRepEntry->ipaddr = ((UINT)pSIP[0]<<24) + (((UINT)pSIP[1]<<16)) + (((UINT)pSIP[2]<<8)) + (UINT)pSIP[3];
                MTWF_PRINT("%s(): Got the Mac("MACSTR") of IP(%d.%d.%d.%d)\n",
			__func__, MAC2STR(pRepEntry->OriginalAddress),
			(pRepEntry->ipaddr>>24) & 0xff,
			(pRepEntry->ipaddr>>16) & 0xff, (pRepEntry->ipaddr>>8) & 0xff,
			pRepEntry->ipaddr & 0xff);
                return NDIS_STATUS_SUCCESS;
            }
        }else
#endif
        {
            MAC_TABLE_ENTRY *pEntry = NULL;
            pEntry = MacTableLookup(pAd, pSMac);
            if (pEntry != NULL && isUcastMac && isGoodIP) {
                pEntry->ipaddr = ((UINT)pSIP[0]<<24) + (((UINT)pSIP[1]<<16)) + (((UINT)pSIP[2]<<8)) + (UINT)pSIP[3];

               MTWF_PRINT("%s(): Got the Mac("MACSTR") of IP(%d.%d.%d.%d)\n",
						__func__, MAC2STR(pEntry->Addr), (pEntry->ipaddr>>24) & 0xff, (pEntry->ipaddr>>16) & 0xff,
						(pEntry->ipaddr>>8) & 0xff, pEntry->ipaddr & 0xff);
                return NDIS_STATUS_SUCCESS;
            }else {
                MTWF_PRINT("[%s](%d) Can't find STA!\n",__func__, __LINE__);;
            }
        }
    }

    return NDIS_STATUS_FAILURE;
}

/*
*	========================================================================
*	Routine	Description:
*		andlink Sepc get each sta hostname in repeater mode.
*
*	Arguments:
*		pAd		=>Pointer to our adapter
*		pPkt 	=>pointer to the 802.3 header of outgoing packet
*
*	Return Value:
*		Success	=>
*			TRUE
*			find sta hostname in DHCP packet.
*		Error	=>
*			FALSE.
*
*	Note:
*		1.the pPktHdr must be a 802.3 packet.
*		2.We check every packet handle DHCP packet to get hostname.
*	========================================================================
 */
NDIS_STATUS update_sta_hostname(IN PRTMP_ADAPTER	pAd,
	                         IN PNDIS_PACKET  pPkt){
    PNDIS_PACKET newSkb = NULL;
    PUCHAR pSrcMac = NULL;
	PUCHAR pSrcIP = NULL;
    PUCHAR  pLayerHdr = NULL;
    PUCHAR  pPktHdr = NULL;
    UINT16  protoType;
    UCHAR pCliMacAdr[MAC_ADDR_LEN]={0};
	UCHAR  DHCP_MAGIC[] =  {0x63, 0x82, 0x53, 0x63};

    if (pAd == NULL && pPkt == NULL) {
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "pAd or pPkt is NULL!\n");
	return NDIS_STATUS_FAILURE;
    }

    pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return NDIS_STATUS_FAILURE;

    /* Get the upper layer protocol type of this 802.3 pkt and dispatch to specific handler */
    protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));
    /*DHCP Discover check*/
    if (ETH_P_IP == protoType) {
        pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
        pSrcMac = pPktHdr + 6;/*src mac*/
        pSrcIP = pLayerHdr + 12;/*src ip*/
        /*For UDP packet, we need to check about the DHCP packet, to modify the flag of DHCP discovey/request as broadcast. */
        if (*(pLayerHdr + 9) == 0x11) {
            PUCHAR udpHdr;
            UINT16 srcPort, dstPort;
            udpHdr = pLayerHdr + 20;
            srcPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr)));
            dstPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr + 2)));
            /*It's a DHCP packet */
            if (srcPort == 68 && dstPort == 67) {
                PUCHAR bootpHdr;
                UINT16 bootpFlag;
                PUCHAR dhcpHdr;
                PNDIS_PACKET pSkb;

                pSkb = RTPKT_TO_OSPKT(pPkt);
                if (OS_PKT_CLONED(pSkb)) {
				    OS_PKT_COPY(pSkb, newSkb);

                    if (newSkb) {
                        /* reassign packet header pointer for new skb*/
                        if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(newSkb))) {
                            pPktHdr = GET_OS_PKT_DATAPTR(newSkb);
                            pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
                            udpHdr = pLayerHdr + 20;
                        } else {
                            pPktHdr = GET_OS_PKT_DATAPTR(newSkb);
                            pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
                            udpHdr = pLayerHdr + 20;
                        }
                    }
                }
                bootpHdr = udpHdr + 8;
                bootpFlag = OS_NTOHS(get_unaligned((PUINT16)(bootpHdr + 10)));
                dhcpHdr = bootpHdr + 236;
                /*client hw address*/
                NdisMoveMemory(pCliMacAdr, (bootpHdr + 28), MAC_ADDR_LEN);
                /*dhcp magic check*/
                if (NdisEqualMemory(dhcpHdr, DHCP_MAGIC, 4)){
                    PUCHAR pOptCode, pOptLen;
                    UINT16 udpLen;
#ifdef MAC_REPEATER_SUPPORT
                    PREPEATER_CLIENT_ENTRY pRepEntry = NULL;
#endif


                    udpLen = OS_NTOHS(get_unaligned((PUINT16)(udpHdr + 4)));
                    pOptCode = (dhcpHdr + 4);

                    do {
                        pOptLen = pOptCode + 1;

                        if (*pOptCode == 12) {  /*hostname*/
                            /* update hostname */
#ifdef MAC_REPEATER_SUPPORT
                            pRepEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pCliMacAdr, TRUE);

                            if (pAd->MatCfg.bMACRepeaterEn && pRepEntry) {
                                if(pRepEntry != NULL) {
                                    NdisZeroMemory(pRepEntry->hostname, HOSTNAME_LEN);
                                    NdisMoveMemory(pRepEntry->hostname, pOptCode+2, *pOptLen > HOSTNAME_LEN ? HOSTNAME_LEN : *pOptLen);
                                    MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
						"HostName: %s\n", pRepEntry->hostname);
                                    return NDIS_STATUS_SUCCESS;
                                }
                            }else
#endif/*MAC_REPEATER_SUPPORT*/
                            {
                                MAC_TABLE_ENTRY *pEntry = NULL;
                                pEntry = MacTableLookup(pAd, pCliMacAdr);
                                if (NULL != pEntry) {
                                    NdisZeroMemory(pEntry->hostname, HOSTNAME_LEN);
                                    NdisMoveMemory(pEntry->hostname, pOptCode+2, *pOptLen);
                                    MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
						"HostName: %s\n", pEntry->hostname);
                                    return NDIS_STATUS_SUCCESS;
                                }else {
                                    MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
						"Can't find STA!\n");
                                }
                            }

                        }

                        pOptCode += (2 + *pOptLen);
                    } while ((*pOptCode != 0xFF) && ((pOptCode - udpHdr) <= udpLen));

                }
            }
        }
    }
    return NDIS_STATUS_FAILURE;
}
#endif/*ANDLINK_HOSTNAME_IP*/
#endif/*ANDLINK_FEATURE_SUPPORT*/


#ifdef ACK_CTS_TIMEOUT_SUPPORT
UINT32 get_ack_timeout_bycr(RTMP_ADAPTER *pAd, UINT32 type, UINT32 *ptimeout)
{
	POS_COOKIE pObj;
	struct wifi_dev *wdev;

	if (pAd == NULL || ptimeout == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			" invalid null input: pAd=%p, ptimeout=%p!!\n",
			pAd, ptimeout);
		return FALSE;
	}
	if (NULL == pAd->hdev_ctrl) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			" invalid hdev_ctrl: pAd->hdev_ctrl=%p;!!\n",
			pAd->hdev_ctrl);
		return FALSE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			"Incoorect BSS!!\n");
		return FALSE;
	}

	if (IS_MT7915(pAd)) {
		MAC_IO_READ32(pAd->hdev_ctrl, type, ptimeout);
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			"reg_addr(%x) = %x!!\n", type, *ptimeout);
	} else {
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (pChipCap->uni_cmd_support)
			UniCmdACKCTSTimeoutRead(pAd, wdev, (UINT8)type, ptimeout);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtCmdCfgRead(pAd, wdev, (UINT8)type, ptimeout);
	}
	return TRUE;
}

static INT32 get_ack_timeout_mode_byband(
	RTMP_ADAPTER *pAd,
	UINT32 *ptimeout,
	UINT32 bandidx,
	ACK_TIMEOUT_MODE_T ackmode)
{
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	INT32 ret = TRUE;

	if ((*ptimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range should between [0xFFFF:0]!!\n");
		return FALSE;
	}

	if (pAd->CommonCfg.ack_cts_enable == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			"ERROR! BAND%u, ack_cts_enable=%u, CTS/ACK FEATURE is not enable!!\n",
			 bandidx, pAd->CommonCfg.ack_cts_enable);
		return FALSE;
	}

	if (chip_ops->get_ack_timeout_mode_byband)
		ret = chip_ops->get_ack_timeout_mode_byband(pAd, ptimeout, bandidx, ackmode);
	return ret;
}


static INT32 get_cck_ofdm_ofdma_tout (RTMP_ADAPTER *pAd, UINT32 *ptimeout, ACK_TIMEOUT_MODE_T ack_mode)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = NULL;
	UINT32 apidx = 0;
	UCHAR band_idx = 0;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p;\n",
			pAd);
		return FALSE;
	}
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	if (NULL == pObj) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"Null pObj:pObj=%p!!\n",
			pObj);
		return FALSE;
	}
	apidx = pObj->ioctl_if;
	if (((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		&& (apidx < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			" invalid interafce or interface idx: %d!!\n",
			apidx);
		return FALSE;
	}

	/*GET BANDINDX*/
	band_idx = HcGetBandByWdev(wdev);


	/*get from chip*/
	if (FALSE == get_ack_timeout_mode_byband(pAd, ptimeout, band_idx, ack_mode)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}

	return TRUE;
}

INT show_distance_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;
	UINT32 distance = 0;

	/* Read CCK ACK Time CR*/
	if (TRUE == get_cck_ofdm_ofdma_tout(pAd, &mac_val, ACK_ALL_TIME_OUT)) {
		distance = ((mac_val & MAX_ACK_TIMEOUT)/2)*LIGHT_SPEED;
		MTWF_PRINT("Distance = %d m\n", distance);
		return TRUE;
	} else {
		MTWF_PRINT("SHOW CCK/OFDM/OFDMA CTS/ACK Timeout FAIL!\n");
		return FALSE;
	}
}

INT show_cck_ack_timeout_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;

	/* Read CCK ACK Time CR*/
	if (TRUE == get_cck_ofdm_ofdma_tout(pAd, &mac_val, CCK_TIME_OUT)) {
		MTWF_PRINT("CCK CTS/ACK Timeout = %d us\n",
		(mac_val & MAX_ACK_TIMEOUT));
		return TRUE;
	} else {
		MTWF_PRINT("SHOW OFDM CTS/ACK Timeout FAIL!\n");
		return FALSE;
	}
}

INT show_ofdm_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;

	/* Read OFDM ACK Time CR*/
	if (TRUE == get_cck_ofdm_ofdma_tout(pAd, &mac_val, OFDM_TIME_OUT)) {
		MTWF_PRINT("OFDM CTS/ACK Timeout = %d us\n",
		(mac_val & MAX_ACK_TIMEOUT));
		return TRUE;
	} else {
		MTWF_PRINT("SHOW OFDM CTS/ACK Timeout FAIL!\n");
		return FALSE;
	}
}

INT show_ofdma_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 mac_val = 0;

	/* Read OFDMA ACK Time CR*/
	if (TRUE == get_cck_ofdm_ofdma_tout(pAd, &mac_val, OFDMA_TIME_OUT)) {
		MTWF_PRINT("OFDMA CTS/ACK Timeout = %d us\n",
		(mac_val & MAX_ACK_TIMEOUT));
		return TRUE;
	} else {
		MTWF_PRINT("SHOW OFDMA CTS/ACK Timeout FAIL!\n");
		return FALSE;
	}
}

#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#ifdef CONFIG_MT7976_SUPPORT
EEPROM_PWR_ON_MODE_T get_power_on_cal_mode(
	IN	PRTMP_ADAPTER	pAd)
{
	EEPROM_PWR_ON_MODE_T mode = EEPROM_PWR_ON_CAL_2G_5G;
	struct wifi_dev *wdev;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	int i;
	/*do not change order*/

#ifdef CONFIG_AP_SUPPORT
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_6G)) {
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"MBSS[%d] 6G On!\n", i);
				return EEPROM_PWR_ON_CAL_2G_6G;
			}
		}

#ifdef WDS_SUPPORT
		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"WDS[%d] 6G On!\n", i);
				return EEPROM_PWR_ON_CAL_2G_6G;
			}
		}
#endif /*WDS_SUPPORT*/

#ifdef APCLI_SUPPORT
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &pAd->StaCfg[i].wdev;
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"APCLI[%d] 6G On!\n", i);
				return EEPROM_PWR_ON_CAL_2G_6G;
			}
		}
#endif /*APCLI_SUPPORT*/

#ifdef SNIFFER_SUPPORT
		for (i = 0; i < MONITOR_MAX_DEV_NUM; i++) {
			wdev = &pAd->monitor_ctrl.wdev;
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"SNIFFER 6G On!\n");
				return EEPROM_PWR_ON_CAL_2G_6G;
			}
		}
#endif
	}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_6G)) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &pAd->StaCfg[i].wdev;
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"STA[%d] 6G On!\n", i);
				return EEPROM_PWR_ON_CAL_2G_6G;
			}
		}
	}
#endif /*CONFIG_STA_SUPPORT*/

	return mode;
}
#endif  /* CONFIG_MT7976_SUPPORT */

#ifdef IXIA_C50_MODE
INT Set_pktlen_offset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING * arg)
{
	int offset ;

	offset = simple_strtol(arg, 0, 10);
	pAd->ixia_ctl.pkt_offset = offset;

	MTWF_PRINT("ixia: pkt_offset= %d\n", pAd->ixia_ctl.pkt_offset);

	return TRUE;
}

INT Set_ixia_debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING * arg)
{
	int dbg ;

	dbg = simple_strtol(arg, 0, 10);
	pAd->ixia_ctl.debug_lvl = dbg;

	MTWF_PRINT("ixia: debug_lvl = %d\n", pAd->ixia_ctl.debug_lvl);

	return TRUE;

}

INT Set_ForceIxia_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Ctrl;

	Ctrl = (UCHAR)simple_strtol(arg, 0, 10);

	if (Ctrl)
		pAd->ixia_ctl.iforceIxia = 1;
	else
		pAd->ixia_ctl.iforceIxia = 0;

	MTWF_PRINT("ixia: forceIxia = %d\n", pAd->ixia_ctl.iforceIxia);

	return TRUE;
}

INT Set_ixia_round_reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Ctrl;

	Ctrl = (UCHAR)simple_strtol(arg, 0, 10);

	if (Ctrl) {
		MTWF_PRINT("Ixia round(%d-%d) reset -> 0.\n",
		pAd->ixia_ctl.tx_test_round,
		pAd->ixia_ctl.rx_test_round);

		pAd->ixia_ctl.tx_test_round = 0;
		pAd->ixia_ctl.rx_test_round = 0;
	}

	return TRUE;
}
#endif

#ifdef WIFI_UNIFIED_COMMAND
INT Show_mldRec_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 MldRecIdx;
	INT Status = NDIS_STATUS_SUCCESS;

	MldRecIdx = os_str_tol(arg, 0, 10);

	if (MldRecIdx > MAX_MLO_MGMT_SUPPORT_MLD_NUM) {
		MTWF_PRINT("MldRecIdx should not be greater than %d\n",
			MAX_MLO_MGMT_SUPPORT_MLD_NUM);
		Status = NDIS_STATUS_FAILURE;
	}

	MTWF_PRINT("MldRecIdx=%d\n",
			MldRecIdx);

	if (UniCmdMldRec(pAd, MldRecIdx))
		Status = NDIS_STATUS_FAILURE;

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}
INT Set_MloAgcTx_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4MldRecIdx = 0, u4MldRecLinkIdx = 0, u4AcIdx = 0;
	UINT32 u4DispPolTx = 0, u4DispRatioTx = 0, u4DispOrderTx = 0;
	UINT32 u4DispMgfTx = 0, u4Recv = 0;
	INT Status = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TX_T rMloAgcTx;

	u4Recv = sscanf(arg, "%d:%d:%d:%d:%d:%d:%d", &(u4MldRecIdx), &(u4MldRecLinkIdx), &(u4AcIdx),
		&(u4DispPolTx), &(u4DispMgfTx), &(u4DispRatioTx), &(u4DispOrderTx));

	if (u4Recv != MLO_CMD_SET_MLOAGCTX_ARG_NUM) {
		MTWF_PRINT("Format Error! ArgNum = %d != %d\n",
			u4Recv, MLO_CMD_SET_MLOAGCTX_ARG_NUM);

		MTWF_PRINT("iwpriv ra0 set mloagctx= [MldRecIdx]:[Link]:[Ac]:[DispPol]:[MGF]:[Ratio]:[Order]\n");
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4MldRecIdx >= MAX_MLO_MGMT_SUPPORT_MLD_NUM) {
		MTWF_PRINT("u4MldRecIdx should be less than %d\n",
			MAX_MLO_MGMT_SUPPORT_MLD_NUM);
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4AcIdx >= MAX_MLO_MGMT_SUPPORT_AC_NUM) {
		MTWF_PRINT("u4AcIdx should be less than %d\n",
			MAX_MLO_MGMT_SUPPORT_AC_NUM);
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4MldRecLinkIdx >= MLD_LINK_MAX) {
		MTWF_PRINT("u4MldRecLinkIdx should be less than %d\n",
			MLD_LINK_MAX);
		Status = NDIS_STATUS_FAILURE;
	}

	rMloAgcTx.u1MldRecIdx = u4MldRecIdx;
	rMloAgcTx.u1MldRecLinkIdx = u4MldRecLinkIdx;
	rMloAgcTx.u1AcIdx = u4AcIdx;
	rMloAgcTx.u1DispPolTx = u4DispPolTx;
	rMloAgcTx.u2DispMgfTx = u4DispMgfTx;
	rMloAgcTx.u1DispRatioTx = u4DispRatioTx;
	rMloAgcTx.u1DispOrderTx = u4DispOrderTx;

	if (UniCmdMloAgcTx(pAd, &rMloAgcTx))
		Status = NDIS_STATUS_FAILURE;

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT Set_MloAgcTrig_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4MldRecIdx = 0, u4MldRecLinkIdx = 0, u4AcIdx = 0;
	UINT32 u4DispPolTrig = 0, u4DispRatioTrig = 0;
	UINT32 u4DispMgfTrig = 0, u4Recv = 0;
	INT Status = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MLO_MLD_REC_LINK_AGC_TRIG_T rMloAgcTrig;

	u4Recv = sscanf(arg, "%d:%d:%d:%d:%d:%d", &(u4MldRecIdx), &(u4MldRecLinkIdx), &(u4AcIdx),
		&(u4DispPolTrig), &(u4DispMgfTrig), &(u4DispRatioTrig));

	if (u4Recv != MLO_CMD_SET_MLOAGCTRIG_ARG_NUM) {
		MTWF_PRINT("Format Error! ArgNum = %d != %d\n",
			u4Recv, MLO_CMD_SET_MLOAGCTRIG_ARG_NUM);

		MTWF_PRINT("iwpriv ra0 set mloagctx= [MldRecIdx]:[Link]:[Ac]:[DispPol]:[MGF]:[Ratio]\n");
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4MldRecIdx >= MAX_MLO_MGMT_SUPPORT_MLD_NUM) {
		MTWF_PRINT("u4MldRecIdx should be less than %d\n",
			MAX_MLO_MGMT_SUPPORT_MLD_NUM);
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4AcIdx >= MAX_MLO_MGMT_SUPPORT_AC_NUM) {
		MTWF_PRINT("u4AcIdx should be less than %d\n",
			MAX_MLO_MGMT_SUPPORT_AC_NUM);
		Status = NDIS_STATUS_FAILURE;
	}
	if (u4MldRecLinkIdx >= MLD_LINK_MAX) {
		MTWF_PRINT("u4MldRecLinkIdx should be less than %d\n",
			MLD_LINK_MAX);
		Status = NDIS_STATUS_FAILURE;
	}

	rMloAgcTrig.u1MldRecIdx = u4MldRecIdx;
	rMloAgcTrig.u1MldRecLinkIdx = u4MldRecLinkIdx;
	rMloAgcTrig.u1AcIdx = u4AcIdx;
	rMloAgcTrig.u1DispPolTrig = u4DispPolTrig;
	rMloAgcTrig.u2DispMgfTrig = u4DispMgfTrig;
	rMloAgcTrig.u1DispRatioTrig = u4DispRatioTrig;

	if (UniCmdMloAgcTrig(pAd, &rMloAgcTrig))
		Status = NDIS_STATUS_FAILURE;

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

INT Set_MloOptionCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4OptionType = 0, u4Value = 0, u4Recv = 0;
	INT Status = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MLO_OPTION_CTRL_T rMloOptionCtrl;

	u4Recv = sscanf(arg, "%d:%d", &(u4OptionType), &(u4Value));

	if (u4Recv != MLO_CMD_SET_MLOOPTIONCTRL_ARG_NUM) {
		MTWF_PRINT("Format Error! ArgNum = %d != %d\n",
			u4Recv, MLO_CMD_SET_MLOOPTIONCTRL_ARG_NUM);

		MTWF_PRINT("iwpriv ra0 set mlooption= [OptionType]:[Value]\n");
		Status = NDIS_STATUS_FAILURE;
	}

	rMloOptionCtrl.u1OptionType = u4OptionType;
	rMloOptionCtrl.u1Value = u4Value;
	rMloOptionCtrl.u1Rsv[0] = 0;
	rMloOptionCtrl.u1Rsv[1] = 0;

	if (UniCmdMloOptionCtrl(pAd, &rMloOptionCtrl))
		Status = NDIS_STATUS_FAILURE;

	if (Status == NDIS_STATUS_SUCCESS)
		return TRUE;
	else
		return FALSE;
}
#endif /* WIFI_UNIFIED_COMMAND*/
#ifdef DOT11_EHT_BE
INT show_eht_config(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
			pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("No wpf_cfg for this wdev?!\n");
			return FALSE;
	}
	MTWF_PRINT("Show ETH Config:\n");
	MTWF_PRINT("\tnsep_priority_access_enable: %d\n",
		wlan_config_get_nsep_priority_access(wdev));
	MTWF_PRINT("\teht_om_ctrl_enable: %d\n",
		wlan_config_get_eht_om_ctrl(wdev));
	MTWF_PRINT("\ttxop_sharing_trigger_enable: %d\n",
		wlan_config_get_txop_sharing_trigger(wdev));
	MTWF_PRINT("\temlsr_mr: %d\n",
		wlan_config_get_emlsr_mr(wdev));
	MTWF_PRINT("\tbw: %d\n",
		wlan_config_get_eht_bw(wdev));
	MTWF_PRINT("\ttx_nss: %d\n",
		wlan_config_get_eht_tx_nss(wdev));
	MTWF_PRINT("\trx_nss: %d\n",
		wlan_config_get_eht_rx_nss(wdev));
	MTWF_PRINT("\tt2lm_nego: %d\n",
		wlan_config_get_t2lm_nego_support(wdev));
	return TRUE;
}
/*
 *  ==========================================================================
 *  Description:
 *	trigger_eml_oper_notification_proc.
 *  set eml_oper_notify = eml_mod
 *	eml_mod:
 *	0: eml disable,send emlsr disable
 *		to AP MLD
 *	1: emlsr,send emlsr enable and eml bitmap
 *		to AP MLD
 *	emlsrbitmap:
 * 0x3:link0 & link1 emlsr
 * 0x5:link0 & link2 emlsr
 * 0x6:link1 & link2 emlsr
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
INT trigger_eml_oper_notification_proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct eht_prot_action_frame eml_oper_mode_notify;
	INT status;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR eml_mode = 0;
	UINT32 eml_control_field = 0, ctrl_len = 0;
	u8 token = 1;

	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[0];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct _STA_REC_CTRL_T *strec = NULL;

	pEntry = entry_get(pAd, pStaCfg->MacTabWCID);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	strec = &tr_entry->StaRec;
	eml_mode = os_str_toul(arg, 0, 10);

	if (eml_mode >= EMLMR) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"not support eml mode\n");
		return TRUE;
	}
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"wdev not found\n");
		return FALSE;
	}
	if (NdisEqualMemory(ZERO_MAC_ADDR, pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"can not get apcli peer macaddr\n");
		return TRUE;
	}
	status = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Eml oper mode notify allocate memory failed\n");
		return FALSE;
	}
	if (eml_mode == EMLSR) {
		u16 emlsr_bitmap = wlan_config_get_emlsr_bitmap(wdev);

		if (!((emlsr_bitmap == 3) || (emlsr_bitmap == 5) || (emlsr_bitmap == 6))) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"invalid emlsr bitmap cfg\n");
			MlmeFreeMemory(pOutBuffer);
			return TRUE;
		}

		SET_DOT11BE_EML_CTRL_EMLSR_MODE(eml_control_field, eml_mode);
		SET_DOT11BE_EML_CTRL_EMLSR_LINK_BMAP(eml_control_field,	cpu_to_le16(emlsr_bitmap));
		ctrl_len = 3;

		pEntry->EmlsrBitmap = emlsr_bitmap;
		pEntry->eht_cfg.emlsr_antnum  = wlan_config_get_emlsr_antnum(wdev);
	} else if (eml_mode == EML_DISABLE) {
		ctrl_len = 1;
		pEntry->eht_cfg.emlsr_antnum = 0;
		pEntry->EmlsrBitmap = 0;
	}
		mtf_asic_sta_eml_op_update(pAd, strec);
		ActHeaderInit(pAd, &(eml_oper_mode_notify.Hdr), pStaCfg->MlmeAux.Bssid, wdev->if_addr, pStaCfg->MlmeAux.Bssid);
		eml_oper_mode_notify.category = CATEGORY_PROTECTED_EHT;
		eml_oper_mode_notify.prot_eht_action = EHT_PROT_ACT_EML_OP_NOTIF;

		MakeOutgoingFrame(pOutBuffer, &FrameLen,
			sizeof(struct eht_prot_action_frame), &eml_oper_mode_notify,
			sizeof(token), &token,
			ctrl_len, &eml_control_field, END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen, NULL);

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"send eml oper mode notification w/ ctrl_len(%d)\n", ctrl_len);

	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}

INT show_mld_info(PRTMP_ADAPTER pAd, char *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
			pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif
	struct query_mld_ap_basic bss_mld_info_basic = {0};

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"wdev not found\n");
		return FALSE;
	}

	bss_mngr_query_mld_ap_basic(wdev, &bss_mld_info_basic);

	MTWF_PRINT("Show MLD Info:\n");
	MTWF_PRINT("\tMldAddr: %pM\n", bss_mld_info_basic.addr);
	MTWF_PRINT("\tLinkID: %d, LinkCnt: %d\n",
		bss_mld_info_basic.link_id,
		bss_mld_info_basic.link_cnt);

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		MTWF_PRINT("\tGrpIdx: %d\n", pMbss->mld_grp_idx);
	}
#endif

	return TRUE;
}
#endif /* DOT11_EHT_BE */
//mtk patch:based on rssi to adjust the tx rate when in low t-put. hongchen
UINT32 RTMPGetLastTxRateTW(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
    union _HTTRANSMIT_SETTING lastTxRate;
    union _EHTTRANSMIT_SETTING lastTxRateEH;
    //UCHAR max_mode,max_bw , max_mcs,max_nss ,max_sgi,min_mode ,min_bw ,min_mcs,min_nss,min_sgi;
    UCHAR phy_mode,bw,mcs,nss,sgi,rate;
    CHAR avgRssi;
    
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
    EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
    MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
    if (rTxStatResult.rEntryTxRate.MODE >= MODE_HE)
    {
        lastTxRateEH.field.MODE = rTxStatResult.rEntryTxRate.MODE;
        lastTxRateEH.field.BW = rTxStatResult.rEntryTxRate.BW;
        lastTxRateEH.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
        lastTxRateEH.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI;
        lastTxRateEH.field.STBC = rTxStatResult.rEntryTxRate.STBC;
        lastTxRateEH.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;	
    }
    else
    {
        lastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
        lastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
        lastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
        lastTxRate.field.ShortGI = (rTxStatResult.rEntryTxRate.ShortGI == 1) ? 1 : 0;
        lastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

        if (lastTxRate.field.MODE >= MODE_VHT)
            lastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
        else if (lastTxRate.field.MODE == MODE_OFDM)
            lastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
        else
            lastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
    }

#else
    lastTxRate.word = pEntry->HTPhyMode.word;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#if 1	//mtk patch 20201211
    avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
    if (pEntry->AvgTxBytes < 500 && rTxStatResult.rEntryTxRate.MODE < MODE_HE) {  //Low T-Put 500Bps
        phy_mode = pEntry->MaxHTPhyMode.field.MODE;
        bw = pEntry->MaxHTPhyMode.field.BW;
        nss = ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
        sgi = pEntry->MaxHTPhyMode.field.ShortGI;
        mcs =  pEntry->MaxHTPhyMode.field.MCS;

        if (avgRssi > -65) {
            rate = mcs;
        } else if (avgRssi > -67) {
            rate = (mcs - 1 > 0) ? mcs - 1 : 0;
        } else if (avgRssi > -69) {
            rate = (mcs - 2 > 0) ? mcs - 2 : 0;
        } else if (avgRssi > -71) {
            rate = (mcs - 3 > 0) ? mcs - 3 : 0;
        } else if (avgRssi > -73) {
            rate = (mcs - 4 > 0) ? mcs - 4 : 0;
        } else if (avgRssi > -75) {
            rate = (mcs - 5 > 0) ? mcs - 5 : 0;
        } else if (avgRssi > -77) {
            rate = (mcs - 6 > 0) ? mcs - 6 : 0;
        } else if (avgRssi > -79) {
            rate = (mcs - 7 > 0) ? mcs - 7 : 0;
        } else if (avgRssi > -81) {
            rate = (mcs - 8 > 0) ? mcs - 8 : 0;
        } else if (avgRssi > -83) {
            rate = (mcs - 9 > 0) ? mcs - 9 : 0;
        } else if (avgRssi > -85) {
            rate = (mcs - 10 > 0) ? mcs - 10 : 0;
        } else if (avgRssi > -87) {
            rate = (mcs - 11 > 0) ? mcs - 11 : 0;
        } else {
            phy_mode = pEntry->MinHTPhyMode.field.MODE;
            bw = pEntry->MinHTPhyMode.field.BW;
            rate = 0;
            sgi = pEntry->MinHTPhyMode.field.ShortGI;
        }
        lastTxRate.field.BW = bw;
        lastTxRate.field.MODE = phy_mode;
        lastTxRate.field.MCS = rate;
        lastTxRate.field.ShortGI = sgi;        
        //MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("min_mode MODE [%d] max_mode[%d]\n", ((lastTxRate >> 15) & 0x7), LastTxRate.field.MODE));
    }   
#endif
    if (rTxStatResult.rEntryTxRate.MODE >= MODE_HE)
    {
        lastTxRate.word = lastTxRateEH.word;	 	
    }
    return lastTxRate.word;

}

VOID RTMPIoctlGetMacTableStaInfo(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	UINT8 u1Snr = 0;
    INT i;
    BOOLEAN need_send = FALSE;
    RT_802_11_MAC_TABLE *pMacTab = NULL;
    PRT_802_11_MAC_ENTRY pDst;
    MAC_TABLE_ENTRY *pEntry;
    /* allocate memory */
    os_alloc_mem(NULL, (UCHAR **)&pMacTab, sizeof(RT_802_11_MAC_TABLE)); 

    if (pMacTab == NULL) {
        MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
        return;
    }

    NdisZeroMemory(pMacTab, sizeof(RT_802_11_MAC_TABLE));

    for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
        pEntry = &(pAd->MacTab->Content[i]);

        if (pEntry->wdev != NULL && pEntry->wdev->if_dev != NULL) {
            /* As per new GUI design ifname with index as ra0/ra1/rai0/rai1/... (may not work with older GUI)*/
            if (!strcmp(wrq->ifr_ifrn.ifrn_name, pEntry->wdev->if_dev->name))
                need_send = TRUE;
            else
                need_send = FALSE;
        }
	if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry))
			&& (pEntry->Sst == SST_ASSOC)
			&& (need_send == TRUE)) {
            pDst = &pMacTab->Entry[pMacTab->Num];
            pDst->ApIdx = pEntry->func_tb_idx;
            COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
            pDst->Aid = (UCHAR)pEntry->Aid;
            pDst->Psm = pEntry->PsMode;
#ifdef DOT11_N_SUPPORT
            pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */
            /* Fill in RSSI per entry*/
            pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
            pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
            pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];
            pDst->AvgRssi3 = pEntry->RssiSample.AvgRssi[3];
			/* Fill in SNR per entry*/
			UniCmdPerStaGetSNR(pAd, pEntry->wcid, &u1Snr);
			pDst->AvgSnr = u1Snr;
            /* the connected time per entry*/
            pDst->ConnectedTime = pEntry->StaConnectTime;
            pDst->TxRate.word = RTMPGetLastTxRateTW(pAd, pEntry);
            pDst->EncryMode = pEntry->SecConfig.PairwiseCipher;
			pDst->AuthMode = pEntry->SecConfig.AKMMap;
	    pMacTab->Num += 1;
            /* Add to avoid Array cross board */
           	if (pMacTab->Num >= 544)
                break;

            //hongchen+ 20201205,add rx_rate
            if(pEntry->wdev != NULL)
            {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7990) || defined(MT7992)
                UINT32 rx_rate = 0;
                UCHAR ucBand = HcGetBandByWdev(pEntry->wdev);
                ShowLastRxPhyRate(pAd, ucBand, pEntry->wcid, &rx_rate);
                pDst->LastRxRate = rx_rate;
            
                MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
                "ucBand=0x%x,pDst->LastRxRate =0x%x,rx_rate=0x%x\n",ucBand,pDst->LastRxRate,rx_rate);
    
#endif
            }
            else
                pDst->LastRxRate = pEntry->LastRxRate;
        }
    }

    wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);

    if (copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length))
        MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "copy_to_user() fail\n");

    if (pMacTab != NULL)
        os_free_mem(pMacTab);
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_txpower(RTMP_ADAPTER *pAd, UCHAR TxPower)
{
	INT     status = FALSE;

	if (TxPower <= 100) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pAd->CommonCfg.ucTxPowerPercentage = TxPower;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pAd->CommonCfg.ucTxPowerDefault = TxPower;
			pAd->CommonCfg.ucTxPowerPercentage = pAd->CommonCfg.ucTxPowerDefault;
		}
#endif /* CONFIG_STA_SUPPORT */
		status = TRUE;
	} else
		status = FALSE;
	MTWF_PRINT("%s:(TxPowerPercentage=%d)\n", __func__, pAd->CommonCfg.ucTxPowerPercentage);
	return status;
}
INT mtk_cfg80211_set_maxtxpwr(RTMP_ADAPTER *pAd, UCHAR MaxTxPwr)
{
	UCHAR IfIdx;
	CHANNEL_CTRL *pChCtrl;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	IfIdx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
#endif
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[IfIdx].wdev;
	else if (pObj->ioctl_if_type == INT_MSTA)
		wdev = &pAd->StaCfg[IfIdx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("%s: pObj->ioctl_if_type = %d!!\n", __func__,
			pObj->ioctl_if_type);
		return FALSE;
	}
	if ((MaxTxPwr > 0) && (MaxTxPwr < 0xff)) {
		pAd->MaxTxPwr = MaxTxPwr;
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
		BuildChannelList(pAd, wdev);
#endif
		MTWF_PRINT("Set MaxTxPwr = %d\n", MaxTxPwr);
		return TRUE;
	}
	return FALSE;
}
INT	mtk_cfg80211_set_txburst_proc(RTMP_ADAPTER *pAd, UCHAR TxBurst)
{
	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  /*Invalid argument */
	MTWF_PRINT("%s::(TxBurst=%d)\n", __func__, pAd->CommonCfg.bEnableTxBurst);
	return TRUE;
}
#endif

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
int mtk_cfg80211_config_beacon_vie(PRTMP_ADAPTER pAd, UINT ifIndex, u8 *vie, u16 len)
{
	CHAR *vendor_ie_temp = NULL;
	struct wifi_dev *wdev = NULL;
	struct customer_vendor_ie *ap_vendor_ie = NULL;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error AP index\n");
		return -EINVAL;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	if (wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"interface is down, return!\n");
		return -EINVAL;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"wdev is null, return!\n");
		return -EINVAL;
	}

	if (!len) {	/*remvoe the vie*/

		ap_vendor_ie = &pAd->ApCfg.MBSSID[ifIndex].ap_vendor_ie;

		RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
		if (ap_vendor_ie->pointer != NULL) {
			os_free_mem(ap_vendor_ie->pointer);
			ap_vendor_ie->pointer = NULL;
		}
		ap_vendor_ie->length = 0;
		RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

		/* start sending BEACON out */
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	} else if ((len > 0) && (vie[0] == IE_VENDOR_SPECIFIC)) {		/*add vie*/

		os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, len);

		if (vendor_ie_temp == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
				"alloc memory fail\n");
			return -ENOMEM;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
			"=>cf80211 add beaocn vie\n");

		os_move_mem(vendor_ie_temp, vie, len);

		ap_vendor_ie = &pAd->ApCfg.MBSSID[ifIndex].ap_vendor_ie;
		RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);

		if (ap_vendor_ie->pointer != NULL)
			os_free_mem(ap_vendor_ie->pointer);

		ap_vendor_ie->pointer = vendor_ie_temp;
		ap_vendor_ie->length = len;

		RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

		/* start sending BEACON out */
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error input beaocn vie!!!\n");
		return -EINVAL;
	}

	return TRUE;
}

int mtk_cfg80211_config_probe_rsp_vie(PRTMP_ADAPTER pAd, UINT ifIndex, u8 *vie, u16 len)
{
	CHAR *vendor_ie_temp = NULL;
	UCHAR *vendor_ie_temp_header = NULL;
	struct wifi_dev *wdev;
	CUSTOMER_PROBE_RSP_VENDOR_IE *ap_probe_rsp_vendor_ie = NULL;
	BSS_STRUCT *mbss = NULL;
	PDL_LIST ap_probe_rsp_vendor_ie_list = NULL;
	BOOLEAN found = FALSE;
	UCHAR band;
	u32 ie_count;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error AP index\n");
		return -EINVAL;
	}

	mbss = &pAd->ApCfg.MBSSID[ifIndex];
	wdev = &mbss->wdev;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"wdev is null, return!\n");
		return -EINVAL;
	}

	if (wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"interface is down, return!\n");
		return -EINVAL;
	}

	if (len == 6) {		/*remove probe rsp vie*/

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
			"=>cf80211 add probe rsp vie\n");

		/* alloc mem for mac_addr and band*/
		os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp_header, 7);

		if (!vendor_ie_temp_header) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp_header!!!\n");
			return -ENOMEM;
		}

		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		ie_count = DlListLen(ap_probe_rsp_vendor_ie_list);
		if (ie_count) {
			DlListForEach(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_list,
				CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
				if (memcmp(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header, MAC_ADDR_LEN) == 0) {
					DlListDel(&ap_probe_rsp_vendor_ie->List);
					pAd->ApCfg.ap_probe_rsp_vendor_ie_count--;
					if (ap_probe_rsp_vendor_ie->pointer)
						os_free_mem(ap_probe_rsp_vendor_ie->pointer);
					os_free_mem(ap_probe_rsp_vendor_ie);
					found = TRUE;
					break;
				}
			}
		}
		if (found)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
				"Free STA MAC["MACSTR"]!\n", MAC2STR(ap_probe_rsp_vendor_ie->stamac));
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
			"Free STA MAC["MACSTR"] not found!\n", MAC2STR(vendor_ie_temp_header));

		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);
		os_free_mem(vendor_ie_temp_header);
	} else if ((len <= 264) && (len > 12)) {

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
			"=>cf80211 add probe rsp vie\n");

		/* alloc mem for mac_addr and band*/
		os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp_header, 7);

		if (!vendor_ie_temp_header) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp_header!!!\n");
			return -ENOMEM;
		}

		/* alloc mem for vie*/
		os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, len - 7);

		if (!vendor_ie_temp) {
			os_free_mem(vendor_ie_temp_header);
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp!!!\n");
			return -ENOMEM;
		}

		os_move_mem(vendor_ie_temp_header, vie, 7);
		os_move_mem(vendor_ie_temp, vie + 7, len - 7);

		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		DlListForEach(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_list,
			CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
			if (memcmp(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header, MAC_ADDR_LEN) == 0) {
				found = TRUE;
				break;
			}
		}

		band = vendor_ie_temp_header[6];
		if (found) {
			ap_probe_rsp_vendor_ie->band = band;

			if (ap_probe_rsp_vendor_ie->pointer != NULL)
				os_free_mem(ap_probe_rsp_vendor_ie->pointer);

			ap_probe_rsp_vendor_ie->pointer = vendor_ie_temp;
			ap_probe_rsp_vendor_ie->length = len - 7;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
					"found STA MAC["MACSTR"], replace probe response VIE!\n",
					MAC2STR(vendor_ie_temp_header));
		} else {
			if (pAd->ApCfg.ap_probe_rsp_vendor_ie_count < pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count) {
				os_alloc_mem(pAd, (UCHAR **)&ap_probe_rsp_vendor_ie, sizeof(CUSTOMER_PROBE_RSP_VENDOR_IE));

				if (!ap_probe_rsp_vendor_ie) {
					os_free_mem(vendor_ie_temp);
					os_free_mem(vendor_ie_temp_header);
					RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);

					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
						"Fail to alloc memory for ap_probe_rsp_vendor_ie!\n");
					return -ENOMEM;
				}

				NdisZeroMemory(ap_probe_rsp_vendor_ie, sizeof(CUSTOMER_PROBE_RSP_VENDOR_IE));
				ap_probe_rsp_vendor_ie->band = band;
				COPY_MAC_ADDR(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header);
				ap_probe_rsp_vendor_ie->pointer = vendor_ie_temp;
				ap_probe_rsp_vendor_ie->length = len - 7;

				DlListAddTail(&mbss->ap_probe_rsp_vendor_ie_list, &ap_probe_rsp_vendor_ie->List);
				pAd->ApCfg.ap_probe_rsp_vendor_ie_count++;

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
					"Add new STA MAC["MACSTR"]!!!\n", MAC2STR(ap_probe_rsp_vendor_ie->stamac));
				hex_dump_with_lvl("Probe rsp IE: ", ap_probe_rsp_vendor_ie->pointer,
					ap_probe_rsp_vendor_ie->length, DBG_LVL_DEBUG);
			} else {
				os_free_mem(vendor_ie_temp);
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
					"Fail to add ap_probe_rsp_vendor_ie, exceed List max count[%d]!\n",
					pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count);
			}
		}
		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);

		os_free_mem(vendor_ie_temp_header);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error input probe rsp vie!!!\n");
		return -EINVAL;
	}

	return TRUE;
}


int mtk_cfg80211_config_probe_req_vie(PRTMP_ADAPTER pAd, UINT ifIndex, u8 *vie, u16 len)
{
	CHAR *vendor_ie_temp = NULL;
	struct wifi_dev *wdev = NULL;
	struct customer_vendor_ie *apcli_vendor_ie = NULL;

	if (ifIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error station index\n");
		return -EINVAL;
	}

	wdev = &pAd->StaCfg[ifIndex].wdev;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"wdev is null, return!\n");
		return -EINVAL;
	}

	if (wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"interface is down, return!\n");
		return -EINVAL;
	}

	if (!len) {	/*remvoe the vie*/
		RTMP_SPIN_LOCK(&apcli_vendor_ie->vendor_ie_lock);

		if (apcli_vendor_ie->pointer != NULL) {
			os_free_mem(apcli_vendor_ie->pointer);
			apcli_vendor_ie->pointer = NULL;
		}

		apcli_vendor_ie->length = 0;
		RTMP_SPIN_UNLOCK(&apcli_vendor_ie->vendor_ie_lock);
	} else if ((len > 0) && (vie[0] == IE_VENDOR_SPECIFIC)) { /*add vie*/

		os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, len);

		if (!vendor_ie_temp) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
				"alloc memory fail\n");
			return -ENOMEM;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_INFO,
			"=>cf80211 add probe req vie\n");

		os_move_mem(vendor_ie_temp, vie, len);

		apcli_vendor_ie = &pAd->StaCfg[ifIndex].apcli_vendor_ie;
		RTMP_SPIN_LOCK(&apcli_vendor_ie->vendor_ie_lock);
		if (apcli_vendor_ie->pointer != NULL)
			os_free_mem(apcli_vendor_ie->pointer);

		apcli_vendor_ie->pointer = vendor_ie_temp;
		apcli_vendor_ie->length = len;
		RTMP_SPIN_UNLOCK(&apcli_vendor_ie->vendor_ie_lock);

	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error input probe req vie!!!\n");
		return -EINVAL;
	}

	return TRUE;
}

int mtk_cfg80211_config_oui_filter(PRTMP_ADAPTER pAd, UINT ifIndex, u8 *oui, u16 len)
{
	CHAR *customer_oui_temp;
	struct customer_oui_filter *ap_customer_oui;

	if (!len) {	/*remvoe the oui*/

		ap_customer_oui = &pAd->ApCfg.ap_customer_oui;
		RTMP_SPIN_LOCK(&ap_customer_oui->oui_filter_lock);

		if (ap_customer_oui->pointer != NULL) {
			os_free_mem(ap_customer_oui->pointer);
			ap_customer_oui->pointer = NULL;
		}

		ap_customer_oui->length = 0;
		RTMP_SPIN_UNLOCK(&ap_customer_oui->oui_filter_lock);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"OUI data length is 0, Delete Filter and return!!\n");
	} else if (len > 0) {	/*add oui*/

		os_alloc_mem(pAd, (UCHAR **)&customer_oui_temp, len);

		if (customer_oui_temp == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"alloc memory fail\n");
			return -ENOMEM;
		}

		os_move_mem(customer_oui_temp, oui, len);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"=>cf80211 add oui vie\n");

		ap_customer_oui = &pAd->ApCfg.ap_customer_oui;
		RTMP_SPIN_LOCK(&ap_customer_oui->oui_filter_lock);

		if (ap_customer_oui->pointer != NULL)
			os_free_mem(ap_customer_oui->pointer);

		ap_customer_oui->pointer = customer_oui_temp;
		ap_customer_oui->length = len;

		RTMP_SPIN_UNLOCK(&ap_customer_oui->oui_filter_lock);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_VENDOR, DBG_LVL_ERROR,
			"error input oui!!!\n");
		return -EINVAL;
	}

	return TRUE;
}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_EHT_BE
/* trigger dummy critical update for debug */
INT set_trigger_crit_upd_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *param;
	UCHAR *pBuf, *tmac_info;
	UINT8 i, cnt;
	UINT16 FrameLen = 0, rnr_pos = 0, mbssid_pos = 0;
	UINT16 rnr_cnt_pos, mbssid_cnt_pos;
	UINT32 mld_param = {0};
	UINT32 *pmld_param;
	UINT32 bss = 0, scope = 0, debug = FALSE;

	PNDIS_PACKET MgmtPkt = NULL;
	BSS_STRUCT *pMbss = NULL;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	struct wifi_dev *wdev;
	struct query_mld_info mld_query = {0};
	struct mbss_query_info mbss_info = {0};

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	param = rstrtok(arg, "-");

	if (param) {
		bss = os_str_tol(param, 0, 10);
		param = rstrtok(NULL, "-");

		if (param) {
			scope = os_str_toul(param, 0, 10);
			param = rstrtok(NULL, "-");

			if (param)
				debug = os_str_tol(param, 0, 10);
		}
	}

	MTWF_PRINT("band:%d, bss:%d, scope:%d, debug:%d\n",
		get_dev_config_idx(pAd), bss, scope, debug);

	if (bss >= pAd->ApCfg.BssidNum) {
		MTWF_PRINT("\tinvalid bss index\n");
		goto err;
	}
	if (scope >= BMGR_CRITICAL_UPDATE_MAX_NUM) {
		MTWF_PRINT("\tinvalid scope\n");
		goto err;
	}

	wdev = &pAd->ApCfg.MBSSID[bss].wdev;
	if (!wdev || (WDEV_BSS_STATE(wdev) < BSS_READY)) {
		MTWF_PRINT("\tBSS not ready\n");
		goto err;
	}

	/* trigger critical update */
	bss_mngr_mld_critical_update_trigger(wdev, scope, BCN_REASON(BCN_UPDATE_IE_CHG));

	if (debug) {
		/* query IEs by tx wdev */
#ifdef DOT11V_MBSSID_SUPPORT
		pMbss = (struct _BSS_STRUCT *)wdev->func_dev;
		if (pMbss->mbss_11v.mbss_11v_t_bss
			&& (pMbss->mbss_11v.mbss_11v_t_bss != pMbss)) {
			UCHAR OrigWdevIdx = wdev->wdev_idx;
			/* refine here for supporting MMBSSID (find ntx-BSS's tx-BSS) */
			wdev = &pMbss->mbss_11v.mbss_11v_t_bss->wdev;
			MTWF_PRINT("wdev(%d) is Nontransmitted Bssid, update to tx wdev(%d)\n",
					  OrigWdevIdx, wdev->wdev_idx);
		}
#endif /* DOT11V_MBSSID_SUPPORT */

		/* allcate temp buffer */
		Status = RTMPAllocateNdisPacket(pAd, &MgmtPkt, NULL, 0, NULL, MAX_BEACON_LENGTH);
		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("can not allocate buf\n");
			goto err;
		}
		tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(MgmtPkt);
		pBuf = (UCHAR *)tmac_info;

		/* set static parameters */
		mld_query.query_type = BMGR_QUERY_ML_IE_BCN;
		mld_query.ie.ml_ie = NULL;
		mbss_info.is_probe_rsp = FALSE; /* beacon */

		/* check bss_chg_cnt in MLD IE */
#define BCN_MLD_CNT_OFFSET 13
		mld_query.ie.f_buf = pBuf;
		FrameLen = 0;
		FrameLen += bss_mngr_query_mld_info(wdev, &mld_query);
		MTWF_PRINT("FrameLen after query ML IE: %d\n", FrameLen);
		if (FrameLen > 0) {
			/* MLD IE exists */
			cnt = *((u8 *)(pBuf + BCN_MLD_CNT_OFFSET));
			MTWF_PRINT("\tchg_cnt in ML IE(%d)\n", cnt);
		}

		/* check bss_chg_cnt in RNR IEs */
#define BCN_RNR_CNT_OFFSET 19
#define BCN_RNR_IE_LEN 23
		rnr_pos = FrameLen;
		FrameLen += bss_mngr_query_rnr_ie(wdev, (u8 *)(pBuf + FrameLen));
		MTWF_PRINT("FrameLen after query ML rnr: %d\n", FrameLen);
		mbssid_pos = FrameLen;
		if (FrameLen > rnr_pos) {
			/* rnr IEs exist */
			MTWF_PRINT("\tchg_cnt in RNR IE (");
			rnr_cnt_pos = rnr_pos + BCN_RNR_CNT_OFFSET;
			for (i = 0; i < 64; i++) {
				if (rnr_cnt_pos > mbssid_pos)
					break;
				mld_param = 0;	/* 3 octets */
				pmld_param = (u32 *)(pBuf + rnr_cnt_pos);
				mld_param = *pmld_param;
#ifdef LINUX
				mld_param = le32_to_cpu(mld_param);
#endif /* LINUX */
				cnt = GET_TBTT_INFO_MLD_PARAM_BSS_CHG_CNT(mld_param);
				MTWF_PRINT("%d,", cnt);
				rnr_cnt_pos += BCN_RNR_IE_LEN; // rnr leng=23
			}
			MTWF_PRINT(")\n");
		}

#ifdef DOT11V_MBSSID_SUPPORT
		/* check bss_chg_cnt in MBSSID IEs */
#define BCN_MBSS_CNT_OFFSET 38
#define BCN_MBSS_IE_LEN 41
		mbss_info.f_buf = (u8 *)(pBuf + FrameLen);
		FrameLen += bss_mngr_query_mbssid_ie(wdev, &mbss_info);
		MTWF_PRINT("FrameLen after query ML rnr MBSSID: %d\n", FrameLen);
		if (FrameLen > mbssid_pos) {
			/* MBSS IEs exist */
			MTWF_PRINT("\tchg_cnt in MBSSID IE (");
			mbssid_cnt_pos = mbssid_pos + BCN_MBSS_CNT_OFFSET;
			for (i = 0; i < 32; i++) {
				if (mbssid_cnt_pos > FrameLen)
					break;
				cnt = *((u8 *)(pBuf + mbssid_cnt_pos));
				MTWF_PRINT("%d,", cnt);
				mbssid_cnt_pos += BCN_MBSS_IE_LEN;
			}
			MTWF_PRINT(")\n");
		}
#endif /* DOT11V_MBSSID_SUPPORT */
	}

	return TRUE;
err:
	if (MgmtPkt)
		RTMPFreeNdisPacket(pAd, MgmtPkt);

	MTWF_PRINT
		("\tiwpriv $(inf_name) set crit_upd=[bss]-[scope]-[debug]\n");
	MTWF_PRINT
		("\t[bss] bss index (0~BssidNum-1)\n");
	MTWF_PRINT
		("\t[scope] 0: single, 1: all\n");
	MTWF_PRINT
		("\t[debug] 0: trigger cri-upd only (default), 1: debug mode (query IEs)\n");

	return FALSE;
}
#endif
#endif

INT set_testmdoe_en_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR value;
	struct physical_device *ph_dev = (struct physical_device *)pAd->physical_dev;

	value = (UCHAR)os_str_tol(arg, 0, 10);

	if (value <= 2) {
		ph_dev->TestModeEn = (u8)value;
		MTWF_PRINT("TestMode=%d\n", ph_dev->TestModeEn);
	}

	return TRUE;
}

INT set_tx_null_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR role = 0;
	UCHAR wcid;
	BOOLEAN bQosNull = FALSE;
	MAC_TABLE_ENTRY *pEntry;
	CHAR *param = NULL;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	param = rstrtok(arg, "-");

	if (param) {
		role = os_str_tol(param, 0, 10); /* 0:AP, 1:APC*/
		param = rstrtok(NULL, "-");

		if (param) {
			wcid = os_str_toul(param, 0, 10);
			if (wcid >= 255)
				goto err;
		} else {
			goto err;
		}
	} else
		goto err;

	if (role == 0) {
		pEntry = entry_get(pAd, wcid);
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
			bQosNull = TRUE;
		if (!pEntry->bTxPktChk) {
			pEntry->TotalTxSuccessCnt = 0;
			pEntry->TxStatRspCnt = 0;
			pEntry->bTxPktChk = TRUE;
		}
#ifdef CONFIG_AP_SUPPORT
		MTWF_PRINT("AP send null frame to wcid=%d(%d,%d,%d,%d)dev:"MACSTR" A1:"MACSTR" A2:"MACSTR" A3:"MACSTR"\n\r",
			pEntry->wcid,
#ifdef DOT11_EHT_BE
			IS_ENTRY_MLO(pEntry), pEntry->mlo.is_setup_link_entry,
#else
			0, 0,
#endif
			pEntry->func_tb_idx,
			pEntry->apidx, MAC2STR(pEntry->wdev->if_addr), MAC2STR(pEntry->Addr),
			MAC2STR(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid),
			MAC2STR(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr));
#endif
		RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
			pEntry->wcid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
		chip_do_extra_action(
		       pAd, NULL, pEntry->Addr,
		       CHIP_EXTRA_ACTION_IDLE_DETECT, NULL, NULL);
	} else if (role == 0) {
		/* Todo*/
		;
	}
	return TRUE;
err:
	MTWF_PRINT("iwpriv ra0 set tx_null=<AP|APC (0|1)>:<wcid(<255)>\n");
	return FALSE;
}


INT show_mac_cap_Proc(
	struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	if (chip_dbg->show_mac_cap)
		chip_dbg->show_mac_cap(ad);
	else
		MTWF_PRINT("chip_dbg->show_mac_cap is NULL.\n");

	return TRUE;
}

#define WF_TOP_CFG_ADDR			0x80020100
#define WF_CLKGEN_ON_TOP_ADDR	0x8002E01C
#define FREQ_METER_TIMEOUT	1000

static UINT32 freq_meter_get(
	struct _RTMP_ADAPTER *pAd, UINT8 l1_clk, UINT8 l2_clk)
{
	UINT32 value = 0;
	UINT32 measure_result = 0;
	UINT32 t = 0;
	//Set bit[23]=0x0 (Disable frequency detection)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value &= ~(BIT23);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	//Set bit[27]=0x0 (Disable clock gating)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value &= ~(BIT27);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	//Set bit[28:29]=l1_clk (Meter clock)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value &= ~(BIT29 | BIT28);
	value |= (l1_clk << 28);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	if (l1_clk == 1) {
		//Set bit[17:19]=l2_clk (Meter advance clock)
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_CLKGEN_ON_TOP_ADDR, &value);
		value &= ~(BIT19 | BIT18 | BIT17);
		value |= (l2_clk << 17);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_CLKGEN_ON_TOP_ADDR, value);

		//Set bit[6]=0x1 (Enable CG)
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_CLKGEN_ON_TOP_ADDR, &value);
		value |= (BIT6);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_CLKGEN_ON_TOP_ADDR, value);
	}

	//Set bit[20:22]=0x1 (Set div)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value &= ~(BIT22 | BIT21 | BIT20);
	value |= (0x1 << 20);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	//Set bit[27]=0x1 (Enable clock gating)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value |= (BIT27);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	//Set bit[23]=0x1 (Enable frequency detection gating)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value |= (BIT23);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	while (t <= FREQ_METER_TIMEOUT) {
		//Read bit[16] (Wait for detection complete)
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
		value = (value & BIT(16)) >> 16;
		if (value == 1) {
			//Read bit[0:15] (Get measurement counter)
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &measure_result);
			measure_result = (measure_result & BITS(0, 15));
			break;
		}
		RtmpusecDelay(1000);
		t++;
	}

	//Set bit[23]=0x0 (Disable frequency detection)
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, &value);
	value &= ~(BIT23);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_TOP_CFG_ADDR, value);

	return measure_result;
}

static UINT32 freq_meter_show(
	struct _RTMP_ADAPTER *pAd, UINT8 l1_clk, UINT8 l2_clk)
{
	UINT32 meas_result = 0;

	if (l1_clk == 0) {
		meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
		MTWF_PRINT("OSC_CLK:   %d\n", meas_result*32*1000);
	} else if (l1_clk == 1) {
		switch (l2_clk) {
		case 2:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("EXPLL_CLK: %d\n", meas_result*32*1000*8);
			break;
		case 3:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("WFDMA_CLK: %d\n", meas_result*32*1000*8);
			break;
		case 4:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("WMCPU_CLK: %d\n", meas_result*32*1000*8);
			break;
		case 5:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("WACPU_CLK: %d\n", meas_result*32*1000*8);
			break;
		case 6:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("LMAC_CLK:  %d\n", meas_result*32*1000*8);
			break;
		case 7:
			meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
			MTWF_PRINT("PHY_CLK:   %d\n", meas_result*32*1000*8);
			break;
		default:
			MTWF_PRINT("L2 clock type is error:%d\n", l2_clk);
			break;
		}
	} else if (l1_clk == 2) {
		meas_result = freq_meter_get(pAd, l1_clk, l2_clk);
		MTWF_PRINT("BUS_CLK:   %d\n", meas_result*32*1000);
	} else
		MTWF_PRINT("L1 clock type is error:%d\n", l1_clk);
	return TRUE;
}

INT SetFreqMeter(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i = 0;
	INT status = TRUE;
	INT sRet = 0;
	long tmp = 0;
	CHAR *value;
	UINT8 l1_clk = 0;
	UINT8 l2_clk = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	/* Parsing input parameter */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			l1_clk = (UCHAR)tmp;
			break;

		case 1:
			sRet = kstrtol(value, 10, &tmp); /* 2-bit format */
			l2_clk = (UCHAR)tmp;
			break;

		default:
			status = FALSE;
			MTWF_PRINT("%s: Set wrong parameters\n", __func__);
			break;
		}
		if (sRet) {
			MTWF_PRINT("%s, ret: 0x%x\n", __func__, sRet);
			return FALSE;
		}
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"L1 clock: %d, L2 clock: %d\n", l1_clk, l2_clk);

	/* frequency meter */
	freq_meter_show(pAd, l1_clk, l2_clk);
	return status;
}

struct PSDLimitFromCountryCode {
	UCHAR	CountryName[3];
	UCHAR	*pCountryName;
};

struct PSDLimitFromCountryCode PsdLimitCountryList[] = {
	{"US",	"UNITED STATES"},
	{"KR",	"KOREA REPUBLIC OF"},
	{"BR",	"BRAZIL"},
	{"CL",	"CHILE"},
	{"MY",	"MALAYSIA"},
	{"\0",	NULL},
};

enum TX_POWER_LIMIT_TYPE {
	POWER_LIMIT = 0,
	PSD_LIMIT = 1,
};

UINT8 GetPSDLimitFromCountryCode(
	IN UCHAR *country_code)
{
	UINT loop = 0;
	UINT8 psd_limit = POWER_LIMIT;

	while (PsdLimitCountryList[loop].pCountryName != NULL) {
		if (strncmp(PsdLimitCountryList[loop].CountryName, country_code, 2) == 0) {
			psd_limit = PSD_LIMIT;
			break;
		}
		loop++;
	}

	return psd_limit;
}

UINT8 CheckPSDLimitType(
	struct _RTMP_ADAPTER *ad)
{
	/* 0: Power limit, 1:PSD limit */
	return GetPSDLimitFromCountryCode(ad->CommonCfg.CountryCode);

}

void TxPowerLimitTypeInit(
	struct _RTMP_ADAPTER *pAd, UINT8 lpi_en)
{
	UINT8 PsdLimit = CheckPSDLimitType(pAd);
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	UINT8 BandIdx = hc_get_hw_band_idx(pAd);

	if (wdev && WMODE_CAP_6G(wdev->PhyMode)) {
		pAd->CommonCfg.LpiEn = lpi_en;
		if (lpi_en)
			MtAsicSetLpi(pAd, 0, PsdLimit, BandIdx);
		MTWF_DBG(pAd, DBG_CAT_AP, CATCFG_DBGLOG, DBG_LVL_WARN,
			"LPI en:%d, PSDLimit:%d, band_idx:%d\n", lpi_en, PsdLimit, BandIdx);
	}
}

INT show_assoc_pkt(PRTMP_ADAPTER pAd, char *arg)
{
	MTWF_PRINT("Show ASSOC Info:\n");

	if (pAd->last_assoc_req_len) {
		MTWF_PRINT("[assoc req - lens=%d]:\n", pAd->last_assoc_req_len);
		hex_dump_always("assoc req raw", pAd->last_assoc_req, pAd->last_assoc_req_len);
	}

	if (pAd->last_assoc_resp_len) {
		MTWF_PRINT("[assoc resp - lens=%d]:\n", pAd->last_assoc_resp_len);
		hex_dump_always("assoc resp raw", pAd->last_assoc_resp, pAd->last_assoc_resp_len);
	}

	return TRUE;
}
