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

*/

#include "rt_config.h"
#include "hdev/hdev.h"

/*Radio controller*/


/*
 *
*/

/*Local functions*/
static UCHAR rcGetRfByIdx(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
	return (RFIC_24GHZ | RFIC_5GHZ | RFIC_6GHZ);
}


/*Get RfIC Band from EEPORM content*/
static UINT8 rcGetBandSupport(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (BOARD_IS_5G_ONLY(pAd))
		return RFIC_5GHZ;
	else if (BOARD_IS_2G_ONLY(pAd))
		return RFIC_24GHZ;
	else if (RFIC_IS_5G_BAND(pAd))
		return rcGetRfByIdx(ctrl, DbdcMode, BandIdx);
	else
		return RFIC_24GHZ;
}


static USHORT rcGetDefaultPhyMode(UCHAR rf_band_cap)
{
	/*priority must the same as Default Channel*/
	if (rf_band_cap & RFIC_24GHZ)
		return WMODE_AX_24G;
	else
	if (rf_band_cap & RFIC_5GHZ)
		return WMODE_AX_5G;
	else
	if (rf_band_cap & RFIC_6GHZ)
		return WMODE_AX_6G;
	/*should return 0 since no rf_band_cap*/
	return 0;
}

static UCHAR get_cur_rfic_by_phymode(USHORT PhyMode)
{
	UCHAR rf_mode = 0;

	if (WMODE_CAP_6G(PhyMode))
		rf_mode = RFIC_6GHZ;
	else
	if (WMODE_CAP_5G(PhyMode))
		rf_mode = RFIC_5GHZ;
	else
	if (WMODE_CAP_2G(PhyMode))
		rf_mode = RFIC_24GHZ;

	return rf_mode;
}

static void rc_set_radio_default(struct radio_control *ctrl, UCHAR rf_band_cap)
{
	ctrl->PhyMode = rcGetDefaultPhyMode(rf_band_cap);
	ctrl->cur_rfic_type = get_cur_rfic_by_phymode(ctrl->PhyMode);
}



/*
*
*/
VOID rc_radio_init(struct hdev_ctrl *ctrl, UCHAR rfic, UCHAR dbdc_mode)
{
	RADIO_CTRL *radio_ctrl = NULL;
	RTMP_PHY_CTRL *phy_ctrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;
#ifdef DOT11_HE_AX
	UINT8 max_nss;
	UINT8 max_ru_num;
#endif
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	/*Allocate PhyCtrl for HwResource*/
	i = hc_get_hw_band_idx(pAd);
	phy_ctrl =  &pHwResource->PhyCtrl;
	radio_ctrl = &phy_ctrl->RadioCtrl;
	os_zero_mem(radio_ctrl, sizeof(*radio_ctrl));
	phy_ctrl->rf_band_cap = rcGetBandSupport(ctrl, dbdc_mode, i);
	rc_set_radio_default(radio_ctrl, phy_ctrl->rf_band_cap);
#ifdef DOT11_HE_AX
	max_nss = MCS_NSS_CAP(pAd)->max_nss;
	max_ru_num = MCS_NSS_CAP(pAd)->max_24g_ru_num;
	if (((phy_ctrl->rf_band_cap) & RFIC_5GHZ) || ((phy_ctrl->rf_band_cap) & RFIC_6GHZ))
		max_ru_num = MCS_NSS_CAP(pAd)->max_5g_ru_num;
	init_default_ppe(&phy_ctrl->pe_ctrl.pe_info, max_nss, max_ru_num);
#endif
	radio_ctrl->CurStat = PHY_IDLE;
#ifdef GREENAP_SUPPORT
	radio_ctrl->bGreenAPActive = FALSE;
#endif /* GREENAP_SUPPORT */
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"radio_ctrl=%p,Band=%d,rfcap=%d,PhyMode=%d\n",
			 radio_ctrl, i, phy_ctrl->rf_band_cap, radio_ctrl->PhyMode);
	HdevInit(ctrl, radio_ctrl);
}

VOID rc_radio_exit(struct hdev_ctrl *ctrl, UCHAR dbdc_mode)
{
	struct _HD_RESOURCE_CFG *hw_res = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *phy_ctrl = NULL;

	phy_ctrl =  &hw_res->PhyCtrl;
#ifdef DOT11_HE_AX
	os_free_mem(phy_ctrl->pe_ctrl.pe_info.pe_thld);
#endif
}

/*
*
*/
PHY_STATUS RcGetRadioCurStat(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->CurStat;
}

/*
*
*/
VOID RcSetRadioCurStat(struct radio_dev *rdev, PHY_STATUS CurStat)
{
	rdev->pRadioCtrl->CurStat = CurStat;
}


/*
*
*/
struct radio_dev *rc_init(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;

	pPhyCtrl = &pHwResourceCfg->PhyCtrl;
	os_zero_mem(pPhyCtrl, sizeof(RTMP_PHY_CTRL));

	return NULL;
}


/*
*
*/
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	MTWF_PRINT("rfic: %d, cur_rfic: %d\n",
	pHwResourceCfg->PhyCtrl.rf_band_cap, pHwResourceCfg->PhyCtrl.RadioCtrl.cur_rfic_type);
}
