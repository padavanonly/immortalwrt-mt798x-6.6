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
 /***************************************************************************
 ***************************************************************************

    Module Name:
    mgmt_phy.c

    Abstract:
    Phy Elements build/process

*/

#include "rt_config.h"


/*
 * Defined in IEEE 802.11AC
 */
int build_txpwr_envelope_eirp(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie)
{
	INT len = 0, pwr_cnt;
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR he_bw = wlan_config_get_he_bw(wdev);
#ifdef DOT11_EHT_BE
	UCHAR eht_bw = wlan_config_get_eht_bw(wdev);
	UINT8 dis_subch_bitmap_enable = wlan_config_get_eht_dscb_enable(wdev);
#endif /*DOT11_EHT_BE*/
	struct txpwr_cfg *txpwr = &ad->physical_dev->txpwr_cfg;

#ifdef DOT11_EHT_BE
	if (dis_subch_bitmap_enable)
		he_bw = wlan_config_get_he_pp_bw(wdev);
#endif /*DOT11_EHT_BE*/

	NdisZeroMemory(tpe_ie, sizeof(struct _tx_pwr_env_ie));
	if ((he_bw == HE_BW_160)
		|| (he_bw == HE_BW_8080))
		pwr_cnt = 3;
	else if (he_bw == HE_BW_80)
		pwr_cnt = 2;
	else {
		if (ht_bw == HT_BW_40)
			pwr_cnt = 1;
		else
			pwr_cnt = 0;
	}

	tpe_ie->tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	tpe_ie->tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_EIRP;
	tpe_ie->tx_pwr_info.max_tx_pwr_category = TX_PWR_CATEGORY_DEFAULT;

#ifdef DOT11_EHT_BE
	if (eht_bw == EHT_BW_320)
		pwr_cnt = 4;
#endif /* DOT11_EHT_BE */

	switch (pwr_cnt) {
	case 4:
		tpe_ie->max_tx_pwr[4] = txpwr->local_max_txpwr_bw320;
		fallthrough;
	case 3:
		tpe_ie->max_tx_pwr[3] = txpwr->local_max_txpwr_bw160;
		fallthrough;
	case 2:
		tpe_ie->max_tx_pwr[2] = txpwr->local_max_txpwr_bw80;
		fallthrough;
	case 1:
		tpe_ie->max_tx_pwr[1] = txpwr->local_max_txpwr_bw40;
		fallthrough;
	case 0:
	default:
		tpe_ie->max_tx_pwr[0] = txpwr->local_max_txpwr_bw20;
		break;
	}
#ifdef CONFIG_6G_AFC_SUPPORT
	if (is_afc_in_run_state((struct _RTMP_ADAPTER *)wdev->sys_handle)) {
		tpe_ie->tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_REG_CLIENT_EIRP;
		afc_update_txpwr_envelope_params(wdev, &(tpe_ie->max_tx_pwr[0]), pwr_cnt, tpe_ie->tx_pwr_info.max_tx_pwr_interpretation);
	}
#endif
	len = 2 + pwr_cnt;
	return len;
}

/*
 * Defined in IEEE 802.11AX
 */
int build_txpwr_envelope_eirp_psd(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie,
	u8 u1TxPwrInterpretation, u8 u1TxpwrCategory)
{
	UINT8 len = 0, pwr_cnt, u1NValue = 0, u1KValue = 0;
	INT bw_diff = 0;
	UCHAR he_bw = wlan_config_get_he_bw(wdev);
#ifdef DOT11_EHT_BE
	UCHAR eht_bw = wlan_config_get_eht_bw(wdev);
	UINT8 dis_subch_bitmap_enable = wlan_config_get_eht_dscb_enable(wdev);
#endif /*DOT11_EHT_BE*/
	struct txpwr_cfg *txpwr = &ad->physical_dev->txpwr_cfg;
#ifdef DOT11_EHT_BE
	if (dis_subch_bitmap_enable)
		he_bw = wlan_config_get_he_pp_bw(wdev);
#endif /*DOT11_EHT_BE*/

	NdisZeroMemory(tpe_ie, sizeof(struct _tx_pwr_env_ie));
	if ((he_bw == HE_BW_160)
		|| (he_bw == HE_BW_8080))
		pwr_cnt = 4;
	else if (he_bw == HE_BW_80)
		pwr_cnt = 3;
	else if (he_bw == HE_BW_2040)
		pwr_cnt = 2;
	else
		pwr_cnt = 1;

	if (ad->CommonCfg.bTpePSDsingleBW20Flag && (eht_bw == EHT_BW_320))
		pwr_cnt = 0;

	/*
	 * The Maximum Transmit Power Count subfield determines the value of an integer N
	 * as defined in Table 9-277.
	 */
	switch (pwr_cnt) {
	case 1:
		u1NValue = 0;
		break;
	case 2:
		u1NValue = 1;
		break;
	case 3:
		u1NValue = 3;
		break;
	case 4:
		u1NValue = 7;
		break;
	default:
		u1NValue = 0;
	}

	tpe_ie->tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	tpe_ie->tx_pwr_info.max_tx_pwr_interpretation = u1TxPwrInterpretation;
	tpe_ie->tx_pwr_info.max_tx_pwr_category = u1TxpwrCategory;

	for (len = 0; len <= u1NValue; len++)
		tpe_ie->max_tx_pwr[len] = txpwr->max_txpwr_psd[len];
#ifdef DOT11_EHT_BE
	bw_diff = (INT) eht_bw - (INT) he_bw;
	if ((bw_diff > 0) && (pwr_cnt != 0)) {
		if (eht_bw == EHT_BW_320)
			u1KValue = TXPWR_PSD_N_K_NUM_BW320 - u1NValue - 1;
		else if (eht_bw == EHT_BW_160)
			u1KValue = TXPWR_PSD_N_K_NUM_BW160 - u1NValue - 1;
		else if (eht_bw == EHT_BW_80)
			u1KValue = TXPWR_PSD_N_K_NUM_BW80 - u1NValue - 1;
		else if (eht_bw == EHT_BW_2040)
			u1KValue = TXPWR_PSD_N_K_NUM_BW40 - u1NValue - 1;
		if ((u1KValue > 0) && ((u1NValue+1+u1KValue) < MAX_TX_POWER_COUNT)) {
			tpe_ie->max_tx_pwr[len] = u1KValue;
			for (len = u1NValue+2; len <= u1NValue+u1KValue+1; len++)
				tpe_ie->max_tx_pwr[len] = txpwr->max_txpwr_psd[len];
		}
	}
#endif /* DOT11_EHT_BE */

#ifdef CONFIG_6G_AFC_SUPPORT
	afc_update_txpwr_envelope_params(wdev, &(tpe_ie->max_tx_pwr[0]), u1NValue, u1TxPwrInterpretation);
	if (bw_diff > 0) {
		for (len = u1NValue+2; len <= u1NValue+u1KValue+1; len++)
			tpe_ie->max_tx_pwr[len] = tpe_ie->max_tx_pwr[0];
	}
#endif /*CONFIG_6G_AFC_SUPPORT*/

	len = 1 + len;
	return len;
}

/*
	Appeared in Beacon, ProbResp frames
*/
int build_txpwr_envelope(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _tx_pwr_env_ie *tpe_ie)
{
	UINT8 u1TxPwrInterpretation, u1TxpwrCategory;
	UCHAR ucBand = wlan_config_get_ch_band(wdev);

	if (ucBand != CMD_CH_BAND_6G) {
		u1TxpwrCategory = TX_PWR_CATEGORY_DEFAULT;
		u1TxPwrInterpretation = TX_PWR_INTERPRET_REG_CLIENT_EIRP;
	} else {
		u1TxpwrCategory = TX_PWR_CATEGORY_DEFAULT;
		u1TxPwrInterpretation = TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD;
	}

	if ((u1TxPwrInterpretation == TX_PWR_INTERPRET_EIRP)
		|| (u1TxPwrInterpretation == TX_PWR_INTERPRET_REG_CLIENT_EIRP))
		return build_txpwr_envelope_eirp(ad, wdev, tpe_ie);

	return build_txpwr_envelope_eirp_psd(ad, wdev, tpe_ie,
		u1TxPwrInterpretation, u1TxpwrCategory);
}

/*
	Appeared in Beacon, ProbResp frames
*/
int build_bw_indication_ie(
	struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _bw_ind_ie *bw_ind_ie, UINT8 bw)
{
	INT len = 0;
	UINT8 csa_dis_subch_bitmap_enable = (wlan_config_get_eht_csa_dis_subch_bitmap(wdev) != 0) ? 1 : 0;
	UCHAR eht_bw = rf_bw_2_eht_bw(bw);
	UCHAR eht_cen_ch = eht_cent_ch_freq(wdev, wdev->channel, eht_bw, wlan_config_get_ch_band(wdev));
	UINT8 prim_ch = wdev->channel;

	bw_ind_ie->bw_ind_param |= (csa_dis_subch_bitmap_enable << DOT11BE_BW_IND_PARAM_DSCB_PRESENT_OFFSET);
	len += 1;

	switch (bw) {
	case BW_320:
		bw_ind_ie->control = EHT_OP_CH_BW320;
		bw_ind_ie->ccfs0 = GET_BW320_PRIM160_CENT(prim_ch, eht_cen_ch);
		bw_ind_ie->ccfs1 = eht_cen_ch;
		break;
	case BW_160:
		bw_ind_ie->control = EHT_OP_CH_BW160;
		bw_ind_ie->ccfs0 = GET_BW160_PRIM80_CENT(prim_ch, eht_cen_ch);
		bw_ind_ie->ccfs1 = eht_cen_ch;
		break;
	case BW_80:
		bw_ind_ie->control = EHT_OP_CH_BW80;
		bw_ind_ie->ccfs0 = eht_cen_ch;
		bw_ind_ie->ccfs1 = 0;
		break;
	case BW_40:
		bw_ind_ie->control = EHT_OP_CH_BW40;
		bw_ind_ie->ccfs0 = eht_cen_ch;
		bw_ind_ie->ccfs1 = 0;
		break;
	case BW_20:
	default:
		bw_ind_ie->control = EHT_OP_CH_BW20;
		bw_ind_ie->ccfs0 = eht_cen_ch;
		bw_ind_ie->ccfs1 = 0;
	}
	len += 3;

	if (csa_dis_subch_bitmap_enable) {
		bw_ind_ie->dscb = cpu2le16(wlan_config_get_eht_csa_dis_subch_bitmap(wdev));
		len += 2;
	}

	return len;
}

