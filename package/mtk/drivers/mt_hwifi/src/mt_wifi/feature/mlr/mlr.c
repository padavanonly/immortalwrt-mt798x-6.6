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
    mlr

    Abstract:
    mlr

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#include "rt_config.h"

#ifdef MLR_SUPPORT
UINT8 mlr_determine_version(RTMP_ADAPTER *pAd, struct _vendor_ie_cap *vendor_ie, UINT8 *state)
{
	UINT8 value;

	if (pAd->CommonCfg.bMLREnable) {
		if (pAd->CommonCfg.MLRVersion == MLR_MODE_MLR_V2)
			value = (vendor_ie->mtk_mlr_cap & (MEDIATEK_TLV1_TYPE1_MLR1 | MEDIATEK_TLV1_TYPE1_MLR2));
		else
			value = (vendor_ie->mtk_mlr_cap & (MEDIATEK_TLV1_TYPE1_MLR1));

		if (vendor_ie->mtk_mlr_cap & (MEDIATEK_TLV1_TYPE1_LOWRCPI))
			*state = 1;
	} else
		value = 0;

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_DEBUG,
		"AP MLR status [%d][%d], STA MLR %d => %d\n",
		pAd->CommonCfg.bMLREnable, pAd->CommonCfg.MLRVersion, vendor_ie->mtk_mlr_cap, value);

	return value;
}

INT set_mlr_ctl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value;

	if (arg == NULL)
		return FALSE;

	value = (UINT16)os_str_tol(arg, 0, 10);

	if (value == 0 || value == 1)
		pAd->CommonCfg.bMLREnable = value ? TRUE : FALSE;

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_WARN,
		"MLREnable : %d\n", pAd->CommonCfg.bMLREnable);

	return TRUE;
}

INT set_mlr_ver(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 value;

	if (arg == NULL)
		return FALSE;

	value = (UINT8)os_str_tol(arg, 0, 10);

	if (value == 2)
		pAd->CommonCfg.MLRVersion = MLR_MODE_MLR_V2;
	else
		pAd->CommonCfg.MLRVersion = MLR_MODE_MLR_V1;

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_WARN,
		"MLR Version : 0x%x\n", pAd->CommonCfg.MLRVersion);

	return TRUE;
}

INT set_mlr_force_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 wcid = 0, state = 0;
	RTMP_STRING *pWcidDest = NULL;
	MAC_TABLE_ENTRY	*pEntry = NULL;

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_DEBUG,
			"%s\n", arg);

	pWcidDest = strsep(&arg, ":");

	if (pWcidDest == NULL || arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_ERROR, "Invalid parameters\n");
		return FALSE;
	}

	wcid = os_str_toul(pWcidDest, 0, 10);
	state = os_str_toul(arg, 0, 10);

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_DEBUG,
			"wcid: %d, state: %d\n", wcid, state);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid) || (wcid >= WTBL_MAX_NUM(pAd))) {
		MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_ERROR, "Wcid not found\n");
		return FALSE;
	}

	pEntry = entry_get(pAd, wcid);
	if (pEntry->Sst == SST_ASSOC && (pEntry->MlrMode == MLR_MODE_MLR_V2)) {
		UniCmdStaRecForceMlrState(pAd, wcid, state);
		MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_CFG, DBG_LVL_NOTICE,
			"(Set %02X:%02X:%02X:%02X:%02X:%02X MLR Force State %d)\n",
			PRINT_MAC(pEntry->Addr), state);
	}
	return TRUE;
}
#endif /* MLR_SUPPORT */
