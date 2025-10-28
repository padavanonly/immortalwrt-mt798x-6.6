/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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

#ifdef DOT11_EHT_BE

#include "rt_config.h"
#include "config_internal.h"

#define DEFAULT_STR_BITMAP 0x7
#define DEFAULT_LINK_ANT_NUM 1

void eht_cfg_init(struct eht_cfg *obj)
{
	obj->nsep_priority_access_enable = FALSE;
	obj->eht_om_ctrl_enable = TRUE;
	obj->txop_sharing_trigger_enable = TRUE;
	obj->bw = EHT_BW_80;
	obj->tx_nss = 4;
	obj->rx_nss = 4;
	obj->dis_subch_bitmap = 0x0;
	obj->mcs14_disable = FALSE;
	obj->mcs15_in_mru_disable = FALSE;
	obj->extra_eht_ltf_disable = FALSE;
	obj->eht_cmm_nominal_pad = COMMON_NOMINAL_PAD_16_US;
}

void eht_cfg_exit(struct eht_cfg *obj)
{
	os_zero_mem(obj, sizeof(struct eht_cfg));
}

void eht_global_cfg_init(void *ph_dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)ph_dev_obj;
	int i;

	ph_dev->mlo.mlo_config_op_set = FALSE;
	ph_dev->mlo.str_bitmap = DEFAULT_STR_BITMAP;
	ph_dev->mlo.sync_tx_enable = FALSE;
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++)
		ph_dev->mlo.link_ant_num[i] = DEFAULT_LINK_ANT_NUM;
	PD_MTK_MLO_V1_DISABLE(ph_dev);
}

/*
 * SET function
 */
void wlan_config_set_eht_cmm_nominal_pad(
	struct wifi_dev *wdev,
	u8 eht_cmm_nominal_pad)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	MTWF_PRINT("%s: [Before set] cfg eht_cmm_nominal_pad = %d\n",
		__func__,
		cfg->eht_conf.eht_cmm_nominal_pad);
	cfg->eht_conf.eht_cmm_nominal_pad = eht_cmm_nominal_pad;
	MTWF_PRINT("%s: [After set] cfg eht_cmm_nominal_pad = %d\n",
		__func__,
		cfg->eht_conf.eht_cmm_nominal_pad);
}
void wlan_config_set_nsep_priority_access(
	struct wifi_dev *wdev,
	u8 nsep_pri_access)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.nsep_priority_access_enable = nsep_pri_access;
	MTWF_PRINT("%s: cfg nsep_priority_access_enable = %d\n",
		__func__,
		cfg->eht_conf.nsep_priority_access_enable);
}

void wlan_config_set_eht_om_ctrl(
	struct wifi_dev *wdev,
	u8 om_ctrl)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.eht_om_ctrl_enable = om_ctrl;
	MTWF_PRINT("%s: cfg eht_om_ctrl_enable = %d\n",
		__func__,
		cfg->eht_conf.eht_om_ctrl_enable);
}

void wlan_config_set_eht_txop_sharing_trigger(
	struct wifi_dev *wdev,
	u8 txop_sharing_trigger)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.txop_sharing_trigger_enable = txop_sharing_trigger;
	MTWF_PRINT("%s: cfg txop_sharing_trigger_enable = %d\n",
		__func__,
		cfg->eht_conf.txop_sharing_trigger_enable);
}

void wlan_config_set_eht_restricted_twt(
	struct wifi_dev *wdev,
	u8 restricted_twt)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.restricted_twt_support = restricted_twt;
	MTWF_PRINT("%s: cfg restricted_twt_support = %d\n",
		__func__,
		cfg->eht_conf.restricted_twt_support);
}

void wlan_config_set_eht_scs_traffic(
	struct wifi_dev *wdev,
	u8 scs_traffic)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.scs_traffic_support = scs_traffic;
	MTWF_PRINT("%s: cfg restricted_twt_support = %d\n",
		__func__,
		cfg->eht_conf.scs_traffic_support);
}

void wlan_config_set_med_sync_pres(
	struct wifi_dev *wdev,
	u8 med_sync_pres)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.med_sync_pres = med_sync_pres;
	MTWF_PRINT("%s: cfg med_sync_pres = %d\n",
		__func__,
		cfg->eht_conf.med_sync_pres);
}

void wlan_config_set_med_sync_dur(
	struct wifi_dev *wdev,
	u8 dur)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.med_sync_dur = dur;
	MTWF_PRINT("%s: cfg med_sync_dur = %d\n",
		__func__,
		cfg->eht_conf.med_sync_dur);
}

void wlan_config_set_med_sync_ofdm_ed_thr(
	struct wifi_dev *wdev,
	u8 ofdm_ed_thr)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.med_sync_ofdm_ed_thr = ofdm_ed_thr;
	MTWF_PRINT("%s: cfg med_sync_ofdm_ed_thr = %d\n",
		__func__,
		cfg->eht_conf.med_sync_ofdm_ed_thr);
}

void wlan_config_set_med_sync_max_txop(
	struct wifi_dev *wdev,
	u8 max_txop)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.med_sync_max_txop = max_txop;
	MTWF_PRINT("%s: cfg med_sync_max_txop = %d\n",
		__func__,
		cfg->eht_conf.med_sync_max_txop);
}

void wlan_config_set_emlsr_mr(
	struct wifi_dev *wdev,
	u8 emlsr_mr)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlsr_mr = emlsr_mr;
	MTWF_PRINT("%s: cfg emlsr_mr = %d\n",
		__func__,
		cfg->eht_conf.emlsr_mr);
}
void wlan_config_set_emlsr_padding(
	struct wifi_dev *wdev,
	u8 padding)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlsr_padding = padding;
	MTWF_PRINT("%s: cfg emlsr_padding = %d\n",
		__func__,
		cfg->eht_conf.emlsr_padding);
}
void wlan_config_set_emlsr_trans_delay(
	struct wifi_dev *wdev,
	u8 emlsr_trans_delay)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlsr_trans_delay = emlsr_trans_delay;
	MTWF_PRINT("%s: cfg emlsr_trans_delay = %d\n",
		__func__,
		cfg->eht_conf.emlsr_trans_delay);
}


void wlan_config_set_nstr_bitmap(
	struct wifi_dev *wdev,
	u8 nstr_bitmap)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.nstr_bitmap = nstr_bitmap;
	MTWF_PRINT("%s: cfg nstr_bitmap = %d\n",
		__func__,
		cfg->eht_conf.nstr_bitmap);
}
void wlan_config_set_emlsr_bitmap(
	struct wifi_dev *wdev,
	u16 emlsr_bitmap)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlsr_bitmap = emlsr_bitmap;
	MTWF_PRINT("%s: cfg emlsr_bitmap = %d\n",
		__func__,
		cfg->eht_conf.emlsr_bitmap);
}
void wlan_config_set_emlsr_antnum(
	struct wifi_dev *wdev,
	u8 emlsr_antnum)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlsr_antnum = emlsr_antnum;
	MTWF_PRINT("%s: cfg emlsr_bitmap = %d\n",
		__func__,
		cfg->eht_conf.emlsr_antnum);
}

void wlan_config_set_emlmr_delay(
	struct wifi_dev *wdev,
	u8 emlmr_delay)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.emlmr_delay = emlmr_delay;
	MTWF_PRINT("%s: cfg emlmr_delay = %d\n",
		__func__,
		cfg->eht_conf.emlmr_delay);
}

void wlan_config_set_trans_to(
	struct wifi_dev *wdev,
	u8 trans_to)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.trans_to = trans_to;
	MTWF_PRINT("%s: cfg trans_to = %d\n",
		__func__,
		cfg->eht_conf.trans_to);
}

void wlan_config_set_eml_omn_en(
	struct wifi_dev *wdev,
	u8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.eml_omn_enable = enable;
	MTWF_PRINT("%s: cfg EML OMN enable = %d\n",
		__func__,
		cfg->eht_conf.eml_omn_enable);
}

void wlan_config_set_eht_bw(struct wifi_dev *wdev, u8 eht_bw)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	if (eht_bw >= EHT_BW_MAX) {
		MTWF_PRINT("%s: invalid input(=%d), set eht_bw to be EHT_BW_2040.\n",
			__func__, eht_bw);
		eht_bw = EHT_BW_2040;
	}

	if ((eht_bw == EHT_BW_320) && !WMODE_CAP_BE_6G(wdev->PhyMode)) {
		eht_bw = EHT_BW_160;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_WARN,
			"EHT BW 320 is for EHT 6G only\n");
	}
	cfg->eht_conf.bw = eht_bw;
	MTWF_PRINT("%s: cfg bw = %d\n",
		__func__,
		cfg->eht_conf.bw);
}

void wlan_config_set_eht_tx_nss(struct wifi_dev *wdev, u8 tx_nss)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.tx_nss = tx_nss;
	MTWF_PRINT("%s: cfg tx_nss = %d\n",
		__func__,
		cfg->eht_conf.tx_nss);
}

void wlan_config_set_eht_rx_nss(struct wifi_dev *wdev, u8 rx_nss)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.rx_nss = rx_nss;
	MTWF_PRINT("%s: cfg rx_nss = %d\n",
		__func__,
		cfg->eht_conf.rx_nss);
}

void wlan_config_set_eht_max_mcs(struct wifi_dev *wdev, u8 max_mcs)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.max_mcs = max_mcs;
	MTWF_PRINT("%s: cfg max_mcs = %d\n",
		__func__,
		cfg->eht_conf.max_mcs);
}

void wlan_config_set_eht_dis_subch_bitmap(
	struct wifi_dev *wdev, u16 bit_map)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.dis_subch_bitmap = bit_map;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"cfg dis_subch_bitmap = 0x%x\n",
		cfg->eht_conf.dis_subch_bitmap);
}

void wlan_config_set_eht_dscb_enable(
	struct wifi_dev *wdev, u8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.dscb_enable = enable;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
		"cfg dscb_enable = %d\n",
		cfg->eht_conf.dscb_enable);
}

void wlan_config_set_mcs14_disable(
	struct wifi_dev *wdev, u8 disable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.mcs14_disable = disable;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"%s: cfg mcs14_disable = %d\n", __func__,
		cfg->eht_conf.mcs14_disable);
}

void wlan_config_set_mcs15_in_mru_disable(
	struct wifi_dev *wdev, u8 disable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.mcs15_in_mru_disable = disable;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"%s: cfg mcs15_in_mru_disable = %d\n", __func__,
		cfg->eht_conf.mcs15_in_mru_disable);
}

void wlan_config_set_extra_eht_ltf_disable(
	struct wifi_dev *wdev, u8 disable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.extra_eht_ltf_disable = disable;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"%s: cfg extra_eht_ltf_disable = %d\n", __func__,
		cfg->eht_conf.extra_eht_ltf_disable);
}

void wlan_config_set_eht_csa_dis_subch_bitmap(
	struct wifi_dev *wdev, u16 bit_map)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.csa_dis_subch_bitmap = bit_map;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"%s: cfg csa_dis_subch_bitmap = 0x%x\n", __func__,
		cfg->eht_conf.csa_dis_subch_bitmap);
}

void wlan_config_set_eht_csa_dscb_enable(
	struct wifi_dev *wdev, u8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.csa_dscb_enable = enable;
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_INFO,
		"%s: cfg csa_dscb_enable = %d\n", __func__,
		cfg->eht_conf.csa_dscb_enable);
}

void wlan_config_set_t2lm_nego_support(
	struct wifi_dev *wdev,
	u8 t2lm_nego_support)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return;
	}

	cfg->eht_conf.t2lm_nego_support = t2lm_nego_support;
	MTWF_PRINT("%s: cfg t2lm_nego_support = %d\n",
		__func__,
		cfg->eht_conf.t2lm_nego_support);
}

/*
 * GET function
 */
u8 wlan_config_get_eht_cmm_nominal_pad(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.eht_cmm_nominal_pad;
}
u8 wlan_config_get_nsep_priority_access(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.nsep_priority_access_enable;
}

u8 wlan_config_get_eht_om_ctrl(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.eht_om_ctrl_enable;
}

u8 wlan_config_get_txop_sharing_trigger(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.txop_sharing_trigger_enable;
}

u8 wlan_config_get_eht_restricted_twt(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.restricted_twt_support;
}

u8 wlan_config_get_scs_traffic(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.scs_traffic_support;
}

u8 wlan_config_get_med_sync_pres(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.med_sync_pres;
}

u8 wlan_config_get_med_sync_dur(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.med_sync_dur;
}

u8 wlan_config_get_med_sync_ofdm_ed_thr(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.med_sync_ofdm_ed_thr;
}

u8 wlan_config_get_med_sync_max_txop(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.med_sync_max_txop;
}

u8 wlan_config_get_emlsr_mr(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlsr_mr;
}
u8 wlan_config_get_emlsr_padding(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlsr_padding;
}
u8 wlan_config_get_emlsr_trans_delay(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlsr_trans_delay;
}


u8 wlan_config_get_eht_bw(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.bw;
}

u8 wlan_config_get_eht_max_mcs(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.max_mcs;
}

u8 wlan_config_get_nstr_bitmap(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.nstr_bitmap;
}
u16 wlan_config_get_emlsr_bitmap(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlsr_bitmap;
}

u8 wlan_config_get_emlsr_antnum(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlsr_antnum;
}
u8 wlan_config_get_emlmr_delay(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.emlmr_delay;
}

u8 wlan_config_get_trans_to(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.trans_to;
}

u8 wlan_config_get_eml_omn_en(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.eml_omn_enable;
}

u8 wlan_config_get_eht_tx_nss(struct wifi_dev *wdev)
{
	u8 tx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	tx_nss = cfg->eht_conf.tx_nss;

	return tx_nss;
}

u8 wlan_config_get_eht_rx_nss(struct wifi_dev *wdev)
{
	u8 rx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	rx_nss = cfg->eht_conf.rx_nss;

	return rx_nss;
}

u16 wlan_config_get_eht_dis_subch_bitmap(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.dis_subch_bitmap;
}

u8 wlan_config_get_eht_dscb_enable(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.dscb_enable;
}

u16 wlan_config_get_eht_csa_dis_subch_bitmap(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.csa_dis_subch_bitmap;
}

u8 wlan_config_get_eht_csa_dscb_enable(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.csa_dscb_enable;
}

u8 wlan_config_get_t2lm_nego_support(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_PRINT("%s: cfg NULL\n", __func__);
		return 0;
	}

	return cfg->eht_conf.t2lm_nego_support;
}

VOID wlan_config_set_pp_dl_ofdma(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.pp_dl_ofdma = enable;
	cfg->phy_conf.pp_mu_ctrl_valid[PP_DL_OFDMA] = TRUE;
}

VOID wlan_config_set_pp_ul_ofdma(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.pp_ul_ofdma = enable;
	cfg->phy_conf.pp_mu_ctrl_valid[PP_UL_OFDMA] = TRUE;
}

VOID wlan_config_set_pp_dl_mu_mimo(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.pp_dl_mu_mimo = enable;
	cfg->phy_conf.pp_mu_ctrl_valid[PP_DL_MUMIMO] = TRUE;
}

VOID wlan_config_set_pp_ul_mu_mimo(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.pp_ul_mu_mimo = enable;
	cfg->phy_conf.pp_mu_ctrl_valid[PP_UL_MUMIMO] = TRUE;
}

UINT8 wlan_config_get_pp_dl_ofdma(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.pp_dl_ofdma;
}

UINT8 wlan_config_get_pp_ul_ofdma(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.pp_ul_ofdma;
}

UINT8 wlan_config_get_pp_dl_mu_mimo(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.pp_dl_mu_mimo;
}

UINT8 wlan_config_get_pp_ul_mu_mimo(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.pp_ul_mu_mimo;
}

UINT8 wlan_config_get_pp_mu_ctrl_valid(struct wifi_dev *wdev, UINT8 mode)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}
	if (mode >= PP_MU_OP_MAX_NUM) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "invalid mdoe(%d)\n", mode);
		return 0;
	}

	return cfg->phy_conf.pp_mu_ctrl_valid[mode];
}

u8 wlan_config_get_mcs14_disable(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->eht_conf.mcs14_disable;
}

u8 wlan_config_get_mcs15_in_mru_disable(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->eht_conf.mcs15_in_mru_disable;
}

u8 wlan_config_get_extra_eht_ltf_disable(
	struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->eht_conf.extra_eht_ltf_disable;
}

#endif /* DOT11_EHT_BE */

