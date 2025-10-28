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

#ifndef __CONFIG_EHT_H__
#define __CONFIG_EHT_H__

struct eht_cfg {
	u8 nsep_priority_access_enable;
	u8 eht_om_ctrl_enable;
	/*
	 * For an EHT AP:
	 *   Set to 1:
	 *     to indicate that the AP is capable of transmitting
	 *     a modified MU-RTS frame that allocates time to a STA
	 *     to transmit non-TB PPDUs.
	 *   Set to 0 otherwise.
	 * For an non-AP EHT STA:
	 *   Set to 1:
	 *     to indicate that the non-AP STA is capable of responding to
	 *     a modified MU-RTS frame that allocates time to a STA
	 *     to transmit non-TB PPDUs
	 *  Set to 0 otherwise.
	 */
	u8 txop_sharing_trigger_enable;
	u8 restricted_twt_support;
	/*
	 * Indicates support for transmission and reception of
	 * SCS Descriptor elements containing a TSPEC subelement.
	 */
	u8 scs_traffic_support;
	u8 bw;
	u8 tx_nss;
	u8 rx_nss;
	/*
	 * 9:  0~9
	 * 11: 0~11
	 * 13: 0~13
	 */
	u8 max_mcs;
	/*
	 * 0: not support EMLSR or EMLMR
	 * 1: support EMLSR
	 * 2: support EMLMR
	 */
	u8 emlsr_mr;
	u8 eml_omn_enable;
	u8 emlsr_padding;
	u8 emlsr_trans_delay;
	u8 nstr_bitmap;
	u16 emlsr_bitmap;
	u8 dscb_enable;
	u8 csa_dscb_enable;
	u16 dis_subch_bitmap;
	u16 csa_dis_subch_bitmap;
	u8 emlmr_delay;
	u8 trans_to;
	u8 emlsr_antnum;
	u8 med_sync_pres;
	u8 med_sync_dur;
	u8 med_sync_ofdm_ed_thr;
	u8 med_sync_max_txop;
	u8 mcs14_disable;
	u8 mcs15_in_mru_disable;
	u8 extra_eht_ltf_disable;
	u8 eht_cmm_nominal_pad;
	u8 t2lm_nego_support;
};

/* SET */
void wlan_config_set_med_sync_pres(
	struct wifi_dev *wdev,
	u8 med_sync_pres);
void wlan_config_set_med_sync_dur(
	struct wifi_dev *wdev,
	u8 dur);
void wlan_config_set_med_sync_ofdm_ed_thr(
	struct wifi_dev *wdev,
	u8 ofdm_ed_thr);
void wlan_config_set_med_sync_max_txop(
	struct wifi_dev *wdev,
	u8 max_txop);
void wlan_config_set_emlsr_mr(
	struct wifi_dev *wdev,
	u8 emlsr_mr);
void wlan_config_set_emlsr_padding(
	struct wifi_dev *wdev,
	u8 padding);
void wlan_config_set_emlsr_trans_delay(
	struct wifi_dev *wdev,
	u8 emlsr_trans_delay);
void wlan_config_set_nstr_bitmap(
	struct wifi_dev *wdev,
	u8 nstr_bitmap);
void wlan_config_set_emlsr_bitmap(
	struct wifi_dev *wdev,
	u16 emlsr_bitmap);
void wlan_config_set_emlsr_antnum(
	struct wifi_dev *wdev,
	u8 emlsr_antnum);
void wlan_config_set_emlmr_delay(
	struct wifi_dev *wdev,
	u8 emlmr_delay);
void wlan_config_set_trans_to(
	struct wifi_dev *wdev,
	u8 trans_to);
void wlan_config_set_eml_omn_en(
	struct wifi_dev *wdev,
	u8 enable);
void wlan_config_set_eht_om_ctrl(
	struct wifi_dev *wdev,
	u8 enable);
void wlan_config_set_eht_txop_sharing_trigger(
	struct wifi_dev *wdev,
	u8 txop_sharing_trigger);
void wlan_config_set_eht_restricted_twt(
	struct wifi_dev *wdev,
	u8 restricted_twt);
void wlan_config_set_eht_scs_traffic(
	struct wifi_dev *wdev,
	u8 scs_traffic);
void wlan_config_set_nsep_priority_access(
	struct wifi_dev *wdev,
	u8 enable);
void wlan_config_set_mcs14_disable(
	struct wifi_dev *wdev,
	u8 disable);
void wlan_config_set_mcs15_in_mru_disable(
	struct wifi_dev *wdev,
	u8 disable);
void wlan_config_set_extra_eht_ltf_disable(
	struct wifi_dev *wdev,
	u8 disable);
void wlan_config_set_eht_cmm_nominal_pad(
	struct wifi_dev *wdev,
	u8 eht_cmm_nominal_pad);


/* GET */
u8 wlan_config_get_nsep_priority_access(
	struct wifi_dev *wdev);
u8 wlan_config_get_eht_om_ctrl(
	struct wifi_dev *wdev);
u8 wlan_config_get_txop_sharing_trigger(
	struct wifi_dev *wdev);
u8 wlan_config_get_eht_restricted_twt(
	struct wifi_dev *wdev);
u8 wlan_config_get_scs_traffic(
	struct wifi_dev *wdev);
u8 wlan_config_get_eht_cmm_nominal_pad(
	struct wifi_dev *wdev);

void wlan_config_set_eht_bw(struct wifi_dev *wdev, u8 eht_bw);
void wlan_config_set_eht_tx_nss(struct wifi_dev *wdev, u8 tx_nss);
void wlan_config_set_eht_rx_nss(struct wifi_dev *wdev, u8 rx_nss);
void wlan_config_set_eht_max_mcs(struct wifi_dev *wdev, u8 max_mcs);
void wlan_config_set_eht_dis_subch_bitmap(
	struct wifi_dev *wdev, u16 bit_map);
void wlan_config_set_eht_dscb_enable(
	struct wifi_dev *wdev, u8 enable);
void wlan_config_set_eht_csa_dis_subch_bitmap(
	struct wifi_dev *wdev, u16 bit_map);
void wlan_config_set_eht_csa_dscb_enable(
	struct wifi_dev *wdev, u8 enable);
void wlan_config_set_t2lm_nego_support(
	struct wifi_dev *wdev,
	u8 t2lm_nego_support);

u8 wlan_config_get_eht_bw(struct wifi_dev *wdev);
u8 wlan_config_get_eht_tx_nss(struct wifi_dev *wdev);
u8 wlan_config_get_eht_rx_nss(struct wifi_dev *wdev);
u8 wlan_config_get_eht_max_mcs(struct wifi_dev *wdev);
u8 wlan_config_get_med_sync_pres(struct wifi_dev *wdev);
u8 wlan_config_get_med_sync_dur(struct wifi_dev *wdev);
u8 wlan_config_get_med_sync_ofdm_ed_thr(struct wifi_dev *wdev);
u8 wlan_config_get_med_sync_max_txop(struct wifi_dev *wdev);
u8 wlan_config_get_emlsr_mr(
	struct wifi_dev *wdev);
u8 wlan_config_get_emlsr_padding(
	struct wifi_dev *wdev);
u8 wlan_config_get_emlsr_trans_delay(
	struct wifi_dev *wdev);
u8 wlan_config_get_nstr_bitmap(
	struct wifi_dev *wdev);
u16 wlan_config_get_emlsr_bitmap(
	struct wifi_dev *wdev);
u8 wlan_config_get_emlmr_delay(
	struct wifi_dev *wdev);
u8 wlan_config_get_trans_to(
	struct wifi_dev *wdev);
u8 wlan_config_get_eml_omn_en(
	struct wifi_dev *wdev);
u16 wlan_config_get_eht_dis_subch_bitmap(
	struct wifi_dev *wdev);
u8 wlan_config_get_eht_dscb_enable(
	struct wifi_dev *wdev);
u16 wlan_config_get_eht_csa_dis_subch_bitmap(
	struct wifi_dev *wdev);
u8 wlan_config_get_eht_csa_dscb_enable(
	struct wifi_dev *wdev);
u8 wlan_config_get_emlsr_antnum(
	struct wifi_dev *wdev);
u8 wlan_config_get_mcs14_disable(
	struct wifi_dev *wdev);
u8 wlan_config_get_mcs15_in_mru_disable(
	struct wifi_dev *wdev);
u8 wlan_config_get_extra_eht_ltf_disable(
	struct wifi_dev *wdev);
u8 wlan_config_get_t2lm_nego_support(
	struct wifi_dev *wdev);

#endif /* __CONFIG_EHT_H__ */
