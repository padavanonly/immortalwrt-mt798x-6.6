#ifdef MTK_LICENSE
/****************************************************************************
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
 ***************************************************************************/
#endif /* MTK_LICENSE */
/****************************************************************************

	Abstract:

	All related CFG80211 AP Vendor Command Function Body.

	History:

***************************************************************************/

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#include "rt_config.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */


/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
INT mtk_cfg80211_set_beacon_period(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u16 bcn_int)
{
	struct wifi_dev *wdev = NULL;

	/* only do this for AP MBSS, ignore other inf type */
	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else
		return FALSE;

	if ((bcn_int >= 20) && (bcn_int < 1024) && wdev) {
		pAd->CommonCfg.BeaconPeriod = bcn_int;

#ifdef AP_QLOAD_SUPPORT
		/* re-calculate QloadBusyTimeThreshold */
		QBSS_LoadAlarmReset(pAd);
#endif /* AP_QLOAD_SUPPORT */
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

		wdev->bss_info_argument.bcn_period = bcn_int;
		wdev->bss_info_argument.u8BssInfoFeature = BSS_INFO_BASIC_FEATURE;
		if (AsicBssInfoUpdate(pAd, &wdev->bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
				"Fail to apply the bssinfo\n");

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
			"mwctl:set bss(%d) BeaconPeriod=%d\n", wdev->func_idx, bcn_int);
	}

	return TRUE;
}

INT mtk_cfg80211_set_dtim_period(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 dtim_int)
{
	struct wifi_dev *wdev = NULL;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else
		return FALSE;

	if ((dtim_int >= 1) && (dtim_int <= 255)) {

		pAd->ApCfg.MBSSID[inf_idx].DtimPeriod = dtim_int;

		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

		wdev->bss_info_argument.dtim_period = dtim_int;
		wdev->bss_info_argument.u8BssInfoFeature = BSS_INFO_BASIC_FEATURE;
		if (AsicBssInfoUpdate(pAd, &wdev->bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
				"Fail to apply the bssinfo\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"mwctl:set bss(%d) Dtim Period=%d\n", wdev->func_idx, dtim_int);

	return true;
}

INT mtk_cfg80211_set_hide_ssid(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 hide_en)
{
	struct wifi_dev *wdev = NULL;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else
		return FALSE;

	if (pAd->ApCfg.MBSSID[inf_idx].bHideSsid != hide_en)
		pAd->ApCfg.MBSSID[inf_idx].bHideSsid = hide_en;


	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"mwctl:set bss(%d) HideSSID=%d\n", inf_idx,
		pAd->ApCfg.MBSSID[inf_idx].bHideSsid);

	return TRUE;

}

INT mtk_cfg80211_set_mgmt_rx(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 mgmt_rx_action)
{
	if (mgmt_rx_action >= MGMT_RX_ACTION_MAX) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"mwctl:invalid mgmt_rx = %d\n", mgmt_rx_action);
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"mwctl:set mgmt_rx = %d\n", mgmt_rx_action);
	pAd->ApCfg.reject_mgmt_rx = mgmt_rx_action;
	return TRUE;
}

INT mtk_cfg80211_set_no_bcn(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 no_bcn)
{
	struct wifi_dev *wdev = NULL;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"no this device.(inf_type=0x%x, inf_idx=%d)\n", inf_type, inf_idx);
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"mwctl:set no_bcn = %d\n", no_bcn);

	if (wdev) {
		if (no_bcn)
			BcnStopHandle(pAd, wdev, TRUE);
		else
			BcnStopHandle(pAd, wdev, FALSE);
	}
	return TRUE;
}

INT mtk_cfg80211_set_no_agmode(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable)
{
	struct wifi_dev *wdev = NULL;
	USHORT wmode;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"no this device.(inf_type=0x%x, inf_idx=%d)\n", inf_type, inf_idx);
		return -EINVAL;
	}

	wmode = wdev->PhyMode;
	wmode = wmode_2_cfgmode(wmode);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"wirelessmode %d\n", wmode);

	if ((wmode < 12) || (wmode > 27))
		wdev->IsNoAGMode = 0;
	else
		wdev->IsNoAGMode = enable;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
	"mwctl:set no_agmode  enable = %d\n Note: wirelessmode must 12~27\n", wdev->IsNoAGMode);

	return 0;
}

INT mtk_cfg80211_set_no_nmode(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable)
{
	struct wifi_dev *wdev = NULL;
	USHORT wmode;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"no this device.(inf_type=0x%x, inf_idx=%d)\n", inf_type, inf_idx);
		return -EINVAL;
	}

	wmode = wdev->PhyMode;
	wmode = wmode_2_cfgmode(wmode);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"wirelessmode %d\n", wmode);

	if ((wmode < 12) || (wmode > 27))
		wdev->IsNoNMode = 0;
	else
		wdev->IsNoNMode = enable;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
	"mwctl:set no_nmode enable = %d\n Note: wirelessmode must 12~27\n", wdev->IsNoNMode);

	return 0;
}

INT mtk_cfg80211_set_11axonly(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable)
{
	struct wifi_dev *wdev = NULL;
	USHORT wmode;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"no this device.(inf_type=0x%x, inf_idx=%d)\n", inf_type, inf_idx);
		return -EINVAL;
	}

	wmode = wdev->PhyMode;
	wmode = wmode_2_cfgmode(wmode);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"wirelessmode %d\n", wmode);

	if ((wmode < 16) || (wmode > 21))
		wdev->IsAXOnly = 0;
	else
		wdev->IsAXOnly = enable;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
	"mwctl:set 11axonly enable = %d\n Note: wirelessmode must 16~21\n", wdev->IsAXOnly);

	return 0;
}

INT mtk_cfg80211_set_11beonly(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable)
{
	struct wifi_dev *wdev = NULL;
	USHORT wmode;

	if ((inf_type == INT_MBSSID) || (inf_type == INT_MAIN)) {
		if (inf_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
				"invalid ioctl_if %d\n", inf_idx);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_WARN,
			"no this device.(inf_type=0x%x, inf_idx=%d)\n", inf_type, inf_idx);
		return -EINVAL;
	}

	wmode = wdev->PhyMode;
	wmode = wmode_2_cfgmode(wmode);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
		"wirelessmode %d\n", wmode);

	if ((wmode < 22) || (wmode > 27))
		wdev->IsBEOnly = 0;
	else
		wdev->IsBEOnly = enable;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_NOTICE,
	"mwctl:set 11beonly enable = %d\n Note: wirelessmode must 22~27\n", wdev->IsBEOnly);

	return 0;
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

