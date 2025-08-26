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
 ***************************************************************************

	Module Name:
	bf.c

	Abstract:
	beamform related setting and interface

*/
#ifdef TXBF_SUPPORT

#include "rt_config.h"

struct bf_mode_en_ctrl bf_modeEnCtrl;
u8 ebf_bitmap = 0, ibf_bitmap = 0;

/*******************************************************************************
 *                         L O C A L  F U N C T I O N S
 *******************************************************************************
 */
u8 bf_get_hw_num(u8 flag_hw_bf_cap)
{
	u8 ret;
	if (flag_hw_bf_cap & TXBF_HW_3BF)
		ret = 3;
	else if (flag_hw_bf_cap & TXBF_HW_2BF)
		ret = 2;
	else
		ret = 1;
	return ret;
}

/*******************************************************************************
 *                        G L O B A L  F U N C T I O N S
 *******************************************************************************
 */
bool bf_is_support(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	bool ret = TRUE;

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"wdev is NULL (ret:%d)\n", ret);
		ret = FALSE;
		goto err;
	}
	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"pAd is NULL (ret:%d)\n", ret);
		ret = FALSE;
		goto err;
	} else
		ret = pAd->Bfctrl.is_bf_enable;

err:
	return ret;
}

int bf_type_ctrl(struct _RTMP_ADAPTER *pAd)
{
	int status = NDIS_STATUS_SUCCESS;
	bool ebf = FALSE, ibf = FALSE;
	int band_idx;

	if (pAd == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"pAd is NULL (status:0x%02x)\n", status);
		goto err;
	}

	band_idx = hc_get_hw_band_idx(pAd);
	if (pAd->CommonCfg.ETxBfEnCond == 0)
		ebf_bitmap &= ~(1 << band_idx);
	else
		ebf_bitmap |= 1 << band_idx;
	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == 0)
		ibf_bitmap &= ~(1 << band_idx);
	else
		ibf_bitmap |= 1 << band_idx;

	if (ebf_bitmap > 0)
		ebf = TRUE;
	if (ibf_bitmap > 0)
		ibf = TRUE;

	status = AsicTxBfHwEnStatusUpdate(pAd, ebf, ibf);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"AsicTxBfHwEnStatusUpdate not success (status:0x%02x)\n", status);
	}
	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
		"ETxBfEnCond:%lu, ITxBfEn:%d\n", pAd->CommonCfg.ETxBfEnCond,
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
err:
	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
		"ebf:%d, ibf:%d\n", ebf, ibf);
	return status;
}

int bf_phy_cfg_ctrl(struct _RTMP_ADAPTER *pAd, u8 band_idx)
{
	int status = NDIS_STATUS_SUCCESS;
	EXT_CMD_TXBF_CFG_BF_PHY_T rTxBfCfgBfPhy;
	struct bf_ctrl *Bfctrl = NULL;

	if (pAd == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"pAd is NULL (status:0x%02x)\n", status);
		goto err;
	}
	Bfctrl = &pAd->Bfctrl;
	if (Bfctrl == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"Bfctrl is NULL (status:0x%02x)\n", status);
		goto err;
	}

	memset(&rTxBfCfgBfPhy, 0, sizeof(EXT_CMD_TXBF_CFG_BF_PHY_T));
	if (Bfctrl->is_bf_enable && pAd->CommonCfg.BfSmthIntlBypass) {
		rTxBfCfgBfPhy.ucAction = BF_PHY_SMTH_INTL_BYPASS;
		rTxBfCfgBfPhy.ucBandIdx = band_idx;
		rTxBfCfgBfPhy.ucSmthIntlBypass = pAd->CommonCfg.BfSmthIntlBypass;

		status = AsicTxBfCfgBfPhy(pAd, (u8 *)&rTxBfCfgBfPhy);
		if (status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"AsicTxBfCfgBfPhy not success (status:0x%02x)\n", status);
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"ucAction = %d, ucBandIdx = %d, ucSmthIntlBypass = %d\n",
				rTxBfCfgBfPhy.ucAction, rTxBfCfgBfPhy.ucBandIdx,
				rTxBfCfgBfPhy.ucSmthIntlBypass);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
			"is_bf_enable:%d, BfSmthIntlBypass:%d, band_idx:%d\n",
			Bfctrl->is_bf_enable, pAd->CommonCfg.BfSmthIntlBypass, band_idx);
	}

err:
	return status;
}

int bf_enable_ctrl(struct _RTMP_ADAPTER *pAd, u8 band_idx)
{
	struct _RTMP_CHIP_CAP *cap = NULL;
	int status = NDIS_STATUS_SUCCESS;

	if (pAd == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"pAd is NULL (status:0x%02x)\n", status);
		goto err;
	}
	cap = hwifi_get_chip_cap(pAd);
	if (cap == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"cap is NULL (status:0x%02x)\n", status);
		goto err;
	}

	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
		"FlgHwTxBfCap:0x%02x\n", cap->FlgHwTxBfCap);

	if (pAd->Bfctrl.is_bf_enable) {
		bf_modeEnCtrl.u1BfNum = bf_get_hw_num(cap->FlgHwTxBfCap);

		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
			"[band:%d] u1BfNum:%d\n", band_idx, bf_modeEnCtrl.u1BfNum);

		if (bf_modeEnCtrl.u1BfNum > 1) {
			bf_modeEnCtrl.u1BfBitmap |= 1 << band_idx;
			bf_modeEnCtrl.u1BfSelBand[band_idx] = 0;
		} else {
			bf_modeEnCtrl.u1BfBitmap |= 1;
			bf_modeEnCtrl.u1BfSelBand[0] = band_idx;
		}

		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_INFO,
			"u1BfBitmap = 0x%x, u1BfSelBand[%d] = %d\n",
			bf_modeEnCtrl.u1BfBitmap, band_idx, bf_modeEnCtrl.u1BfSelBand[band_idx]);

		status = AsicTxBfModuleEnCtrl(pAd, bf_modeEnCtrl.u1BfNum, bf_modeEnCtrl.u1BfBitmap,
			&bf_modeEnCtrl.u1BfSelBand[0]);
		if (status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"AsicTxBfModuleEnCtrl not success (status:0x%02x)\n", status);
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
				"u1BfNum = %d, u1BfBitmap = %d, u1BfSelBand[0] = %d\n",
				bf_modeEnCtrl.u1BfNum, bf_modeEnCtrl.u1BfBitmap, bf_modeEnCtrl.u1BfSelBand[0]);
		}
	} else {
		status = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"is_bf_enable:%d\n", pAd->Bfctrl.is_bf_enable);
	}

err:
	return status;
}

int bf_init(struct _RTMP_ADAPTER *pAd)
{
	struct bf_ctrl *Bfctrl = NULL;
	int status = NDIS_STATUS_SUCCESS;
	u8 band_idx = 0;

	if (pAd == NULL) {
		status = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"pAd is NULL (status:0x%02x)\n", status);
		goto err;
	}

	Bfctrl =  &pAd->Bfctrl;
	Bfctrl->is_bf_enable = TRUE;
	band_idx = hc_get_hw_band_idx(pAd);

	status = bf_enable_ctrl(pAd, band_idx);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"bf_enable_ctrl not success in band %d, status: 0x%02x\n", band_idx, status);
	}
	status = bf_phy_cfg_ctrl(pAd, band_idx);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"bf_phy_cfg_ctrl not success in band %d, status: 0x%02x\n", band_idx, status);
	}
	status = mt_Trigger_Sounding_Packet(pAd, TRUE, 0, BF_PROCESSING, 0, NULL);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"mt_Trigger_Sounding_Packet not success in band %d, status: 0x%02x\n", band_idx, status);
	}
	status = bf_type_ctrl(pAd);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"bf_type_ctrl not success in band %d, status: 0x%02x\n", band_idx, status);
	}

err:
	return status;
}

#endif /* TXBF_SUPPORT */
