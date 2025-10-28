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

    Module Name: cmm_peak_adjust.c

    Abstract: for enhance performance
*/

#ifdef PEAK_ENHANCE

#include "rt_config.h"
#include "bss_mngr.h"
#include "mgmt/be_internal.h"

extern struct bss_manager bss_mngr;

#ifdef WHNAT_SUPPORT
#ifdef CONFIG_CPE_SUPPORT
#define mtk_set_pse_drop(_config)
#else
extern void mtk_set_pse_drop(u32 config);
#endif
#endif /* WHNAT_SUPPORT */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */


/*******************************************************************************
 *                P R I V A T E    F U N C T I O N S
 *******************************************************************************
 */


/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void peak_adjust_init_ph_dev(
	void *dev_obj)
{
	struct physical_device *ph_dev = (struct physical_device *)dev_obj;
	struct peak_rx_enhance_ctrl *rx_ctrl = &ph_dev->peak_rx_ctrl;

	rx_ctrl->enable = FALSE;
#ifdef WHNAT_SUPPORT
	rx_ctrl->rx_drop = TRUE;
#endif /* WHNAT_SUPPORT */
}

void peak_adjust_init_mac_ad(
	struct _RTMP_ADAPTER *ad)
{
	struct peak_enhance_ctrl *ctrl = &ad->CommonCfg.peek_enhance;
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);

	os_zero_mem(ctrl, sizeof(struct peak_enhance_ctrl));
	ctrl->enable_adjust = (ENABLE_MLO_ENTRY_ADJUST_ONLY | ENABLE_TXOP_ADJUST);
	ctrl->aifsn = 0x5;
	if (ad->CommonCfg.bEnableTxBurst)
		ctrl->txop = chip_cap->peak_txop;
	else
		ctrl->txop = 0;
	ctrl->avg_rx_pkt_len = PEAK_TP_AVG_RX_PKT_LEN;
	ctrl->avg_tx_pkt_len = PEAK_TP_AVG_TX_PKT_LEN;
	ctrl->sim_avg_rx_pkt_len = 0;
	ctrl->rx_low_bound = 20;
	ctrl->rx_high_bound = 1450;
	ctrl->tx_count_th[BAND0] = PEAK_TP_TX_COUNT_TH;
	ctrl->tx_count_th[BAND1] = PEAK_TP_TX_COUNT_TH;
	ctrl->tx_count_th[BAND2] = PEAK_TP_TX_COUNT_TH;
}

void peak_adjust_txop(
	struct _RTMP_ADAPTER *pAd)
{
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	UINT32 avg_rx_msdu_len;
	struct txop_ctl *txop_ctl = &pAd->txop_ctl;
	struct mu_edca_cfg *mu_edca;
#ifndef ADJUST_TXOP_NEW_ARCH
	UCHAR wmm_idx = 0;
	struct wifi_dev *wdev;
	struct _MAC_TABLE_ENTRY *entry;
	ULONG avg_rx_pkt_len;
	UINT32 avg_tx_pkt_len_th = ctrl->avg_tx_pkt_len;
	UINT32 tx_count_th;
	UINT32 entry_tx_count;
	UINT32 entry_tx_mpdu_pkt_len;
	bool skip = FALSE;
#endif

	if (!ctrl->enable_adjust
		|| (band_idx >= CFG_WIFI_RAM_BAND_NUM))
		return;

#ifndef ADJUST_TXOP_NEW_ARCH
	if (txop_ctl->mlo_entry == TRUE) {
		if ((txop_ctl->cur_wdev == NULL)
			|| (pAd->peak_tp_ctl.main_entry == NULL))
			return;

		wdev = txop_ctl->cur_wdev;
		entry = pAd->peak_tp_ctl.main_entry;
		avg_rx_pkt_len = entry->avg_rx_pkt_len;
		wmm_idx = HcGetWmmIdx(pAd, wdev);
		tx_count_th = ctrl->tx_count_th[band_idx];
		if (ctrl->sim_tx_count[band_idx])
			entry_tx_count = ctrl->sim_tx_count[band_idx];
		else
			entry_tx_count = entry->avg_tx_mpdu_pkts;
		entry_tx_mpdu_pkt_len = (entry_tx_count) ? (UINT32)(entry->AvgTxBytes / entry_tx_count) : 0;

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			mu_edca = wlan_config_get_he_mu_edca(wdev);

			if (mu_edca == NULL)
				return;

			if (mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer != 0)
				skip = TRUE;
		}

		if (ctrl->txop && (ctrl->enable_adjust & ENABLE_TXOP_ADJUST)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
				"[band%d] tx_count_th = %d, avg_tx_pkt_len_th = %d\n",
				band_idx,
				tx_count_th,
				avg_tx_pkt_len_th);
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
				"[band%d] wcid[%d]: avg_tx_mpdu_pkts = %d, entry_tx_mpdu_pkt_len = %d\n",
				band_idx,
				entry->wcid,
				entry_tx_count,
				entry_tx_mpdu_pkt_len);
			if ((entry_tx_mpdu_pkt_len <= avg_tx_pkt_len_th) &&
				(skip == FALSE)) {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"band%d - Set wmm[%d]: TXOP = 0\n",
					band_idx, wmm_idx);
				HW_SET_TX_BURST(pAd, wdev, AC_BE, PRIO_DEFAULT, 0, 1);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"band%d - Set wmm[%d]: TXOP = 0x%x\n",
					band_idx, wmm_idx, ctrl->txop);
				HW_SET_TX_BURST(pAd, wdev, AC_BE, PRIO_DEFAULT, ctrl->txop, 1);
			}
		}
	} else {
		if (txop_ctl->spot_ap_wdev) {
			if (txop_ctl->all_rx_pkts_ap) {
				mu_edca = wlan_config_get_he_mu_edca(txop_ctl->spot_ap_wdev);
				avg_rx_msdu_len = (UINT32)(txop_ctl->all_rx_bytes_ap / txop_ctl->all_rx_pkts_ap);

				if (mu_edca == NULL)
					return;

				if ((avg_rx_msdu_len > UL_AVG_RX_PKT_LEN) &&
					(mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer == 0))
					HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 1);
				else
					HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 0);

			} else if (txop_ctl->all_rx_bytes_ap > txop_ctl->all_tx_bytes_ap * 10)
				HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 1);
			else
				HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 0);
		}

		if (txop_ctl->spot_sta_wdev) {
			if (txop_ctl->all_rx_pkts_sta) {
				avg_rx_msdu_len = (UINT32)(txop_ctl->all_rx_bytes_sta / txop_ctl->all_rx_pkts_sta);

				if (avg_rx_msdu_len > UL_AVG_RX_PKT_LEN)
					HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 1);
				else
					HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 0);
			} else
				HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 0);
		}
	}
#else
	/* To avoid many ADJUST_TXOP_NEW_ARCH, duplicate following code for readability */
	if (ctrl->txop && (ctrl->enable_adjust & ENABLE_TXOP_ADJUST)) {
		if (txop_ctl->spot_ap_wdev) {
			if (txop_ctl->all_rx_pkts_ap) {
				mu_edca = wlan_config_get_he_mu_edca(txop_ctl->spot_ap_wdev);
				avg_rx_msdu_len = (UINT32)(txop_ctl->all_rx_bytes_ap / txop_ctl->all_rx_pkts_ap);

				if (!mu_edca)
					return;

				if ((avg_rx_msdu_len > UL_AVG_RX_PKT_LEN) &&
					(mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer == 0))
					HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 1);
				else
					HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 0);

			} else if (txop_ctl->all_rx_bytes_ap > txop_ctl->all_tx_bytes_ap * 10)
				HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 1);
			else
				HW_SET_TX_BURST(pAd, txop_ctl->spot_ap_wdev, AC_BE, PRIO_UL, 0, 0);
		}

		if (txop_ctl->spot_sta_wdev) {
			if (txop_ctl->all_rx_pkts_sta) {
				avg_rx_msdu_len = (UINT32)(txop_ctl->all_rx_bytes_sta / txop_ctl->all_rx_pkts_sta);

				if (avg_rx_msdu_len > UL_AVG_RX_PKT_LEN)
					HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 1);
				else
					HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 0);
			} else
				HW_SET_TX_BURST(pAd, txop_ctl->spot_sta_wdev, AC_BE, PRIO_UL, 0, 0);
		}
	}
#endif /* ADJUST_TXOP_NEW_ARCH */
}

void reset_txop_all_txrx_counter(struct _RTMP_ADAPTER *ad)
{
	struct txop_ctl *ctl;

	if (ad == NULL)
		return;

	ctl = &ad->txop_ctl;
	if (ctl == NULL)
		return;

	ctl->spot_ap_wdev = NULL;
	ctl->spot_sta_wdev = NULL;
	ctl->all_tx_bytes_ap = 0;
	ctl->all_tx_pkts_ap = 0;
	ctl->all_rx_bytes_ap = 0;
	ctl->all_rx_pkts_ap = 0;
	ctl->all_tx_bytes_sta = 0;
	ctl->all_tx_pkts_sta = 0;
	ctl->all_rx_bytes_sta = 0;
	ctl->all_rx_pkts_sta = 0;
}

void _update_txop_stats(struct txop_ctl *ctl, struct _MAC_TABLE_ENTRY *entry)
{
	if (!ctl || !entry)
		return;

	if (IS_ENTRY_CLIENT(entry)) {
		ctl->all_tx_bytes_ap += entry->OneSecTxBytes;
		ctl->all_rx_bytes_ap += entry->OneSecRxBytes;
		ctl->all_tx_pkts_ap += entry->one_sec_tx_pkts;
		ctl->all_rx_pkts_ap += entry->one_sec_rx_pkts;
	}
#ifdef APCLI_SUPPORT
	if (IS_ENTRY_PEER_AP(entry)) {
		ctl->all_tx_bytes_sta += entry->OneSecTxBytes;
		ctl->all_rx_bytes_sta += entry->OneSecRxBytes;
		ctl->all_tx_pkts_sta += entry->one_sec_tx_pkts;
		ctl->all_rx_pkts_sta += entry->one_sec_rx_pkts;
	}
#endif
}

void update_txop_stats(struct txop_ctl *ctl, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef ADJUST_TXOP_NEW_ARCH
	int i;
	struct _MAC_TABLE_ENTRY *mac_entries[MLD_LINK_MAX];
	struct _MAC_TABLE_ENTRY *mac_entry;
	struct mld_entry_t *mld_entry;
#endif
	if (!entry)
		return;
#ifdef ADJUST_TXOP_NEW_ARCH
	if (entry->mlo.mlo_en) {
		mt_rcu_read_lock();
		mld_entry = rcu_dereference(entry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		for (i = 0; i < MLD_LINK_MAX; i++)
			mac_entries[i] = mld_entry->link_entry[i];
		mt_rcu_read_unlock();

		for (i = 0; i < MLD_LINK_MAX; i++) {
			mac_entry = mac_entries[i];
			if (!mac_entry || !mac_entry->mlo.mlo_en)
				continue;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
					"MLO Wcid[%d] tx_p(%lu), rx_p(%lu)\n",
					mac_entry->wcid,
					mac_entry->one_sec_tx_pkts,
					mac_entry->one_sec_rx_pkts);
			_update_txop_stats(ctl, mac_entry);
		}
	} else
		_update_txop_stats(ctl, entry);
#else
	_update_txop_stats(ctl, entry);
#endif /* ADJUST_TXOP_NEW_ARCH */
}

void sum_txop_all_txrx_counter(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	struct txop_ctl *ctl;

	if (ad == NULL)
		return;

	ctl = &ad->txop_ctl;
	if (ctl == NULL)
		return;

	if (IS_ENTRY_CLIENT(entry)) {
		if (ctl->spot_ap_wdev == NULL && entry->wdev != NULL)
			ctl->spot_ap_wdev = entry->wdev;

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
			"Wcid[%d] tx_b(%lu), rx_b(%lu), tx_p(%lu), rx_p(%lu), wdev(%pM)\n",
			entry->wcid, entry->OneSecTxBytes, entry->OneSecRxBytes,
			entry->one_sec_tx_pkts, entry->one_sec_rx_pkts,
			MAC2STR(ctl->spot_ap_wdev->if_addr));

		/* Skip STA that does not meet the criteria */
		if ((entry->AvgRxBytes + entry->AvgTxBytes) >> 17 < LEGACY_TPUT_THRESHOLD)
			return;
		update_txop_stats(ctl, entry);
	}

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_PEER_AP(entry)) {
		update_txop_stats(ctl, entry);
		if (ctl->spot_sta_wdev == NULL && entry->wdev != NULL) {
			if (WDEV_BSS_STATE(entry->wdev) == BSS_READY) {
				ctl->spot_sta_wdev = entry->wdev;

				if ((entry->AvgRxBytes + entry->AvgTxBytes) >> 17 < LEGACY_TPUT_THRESHOLD) {
					ctl->all_rx_bytes_sta = 0;
					ctl->all_rx_pkts_sta = 0;
				}
			}
		}
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_TXOP, DBG_LVL_INFO,
			"Wcid[%d] tx_b(%lu), rx_b(%lu), tx_p(%lu), rx_p(%lu), wdev(%pM)\n",
			entry->wcid, ctl->all_tx_bytes_sta, ctl->all_rx_bytes_sta,
			ctl->all_tx_pkts_sta, ctl->all_rx_pkts_sta,
			MAC2STR(ctl->spot_sta_wdev->if_addr));
	}
#endif
}

INT Set_peak_adjust_enable_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG Peak_tp_en;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	Peak_tp_en = os_str_tol(arg, 0, 10);
	ctrl->enable_adjust = Peak_tp_en;

	MTWF_PRINT("%s: enable_adjust = 0x%x\n", __func__, ctrl->enable_adjust);
	return TRUE;
}

INT Set_peak_adjust_aifsn_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG Peak_tp_be_aifsn;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	Peak_tp_be_aifsn = os_str_tol(arg, 0, 10);

	if (Peak_tp_be_aifsn <= 0 || Peak_tp_be_aifsn > 15) {
		MTWF_PRINT("invalid aifsn ,it should be (1-15)\n");
		return FALSE;
	}

	ctrl->aifsn = Peak_tp_be_aifsn;

	MTWF_PRINT("%s: aifsn = %d\n", __func__, ctrl->aifsn);
	return TRUE;
}

INT Set_peak_adjust_rx_avg_len_th_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 PeakTpAvgRxPktLen;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	PeakTpAvgRxPktLen = os_str_tol(arg, 0, 10);

	if (PeakTpAvgRxPktLen <= 0 || PeakTpAvgRxPktLen > 1600) {
		MTWF_PRINT("%s: invalid AvgRxPktLen(=%d)\n", __func__, PeakTpAvgRxPktLen);
		return FALSE;
	}

	ctrl->avg_rx_pkt_len = PeakTpAvgRxPktLen;

	MTWF_PRINT("%s: avg_rx_pkt_len = %d\n", __func__, ctrl->avg_rx_pkt_len);

	return TRUE;
}

INT Set_peak_adjust_rx_low_bound_th_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 PeakTpRxLowerBoundTput;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	PeakTpRxLowerBoundTput = os_str_tol(arg, 0, 10);

	if (PeakTpRxLowerBoundTput > 3000) {
		MTWF_PRINT("%s: invalid PeakTpRxLowerBoundTput(=%d)\n",
			__func__, PeakTpRxLowerBoundTput);
		return FALSE;
	}

	ctrl->rx_low_bound = PeakTpRxLowerBoundTput;

	MTWF_PRINT("%s: rx_low_bound = %d\n", __func__, ctrl->rx_low_bound);

	return TRUE;
}

INT Set_peak_adjust_rx_high_bound_th_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 PeakTpRxHigherBoundTput;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	PeakTpRxHigherBoundTput = os_str_tol(arg, 0, 10);

	if (PeakTpRxHigherBoundTput > 3000) {
		MTWF_PRINT("%s: invalid PeakTpTxTh(=%d)\n", __func__, PeakTpRxHigherBoundTput);
		return FALSE;
	}

	ctrl->rx_high_bound = PeakTpRxHigherBoundTput;

	MTWF_PRINT("%s: PeakTpRxTh = %d\n", __func__, ctrl->rx_high_bound);

	return TRUE;
}

INT Set_peak_adjust_sim_avg_rx_pkt_len(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	value = os_str_tol(arg, 0, 10);

	ctrl->sim_avg_rx_pkt_len = value;

	MTWF_PRINT("%s: sim_avg_rx_pkt_len = %d\n", __func__, ctrl->sim_avg_rx_pkt_len);

	return TRUE;
}

INT Set_peak_adjust_sim_avg_tx_pkt_len(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	value = os_str_tol(arg, 0, 10);

	ctrl->sim_avg_tx_pkt_len = value;

	MTWF_PRINT("%s: sim_avg_tx_pkt_len = %d\n", __func__, ctrl->sim_avg_tx_pkt_len);

	return TRUE;
}

INT Set_peak_adjust_sim_tx_count(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	value = os_str_tol(arg, 0, 10);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("%s: invalid band idx(=%d)\n", __func__, band_idx);
		return FALSE;
	}
	ctrl->sim_tx_count[band_idx] = value;

	MTWF_PRINT("%s: [band%d] set sim tx count = %d\n",
		__func__, band_idx, ctrl->sim_tx_count[band_idx]);

	return TRUE;
}

INT Set_peak_adjust_tx_pkt_len_th_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	value = os_str_tol(arg, 0, 10);

	ctrl->avg_tx_pkt_len = value;

	MTWF_PRINT("%s: avg_tx_pkt_len = %d\n", __func__, ctrl->avg_tx_pkt_len);

	return TRUE;
}

INT Set_peak_adjust_tx_count_th_Proc(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	value = os_str_tol(arg, 0, 10);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("%s: invalid band idx(=%d)\n", __func__, band_idx);
		return FALSE;
	}
	ctrl->tx_count_th[band_idx] = value;

	MTWF_PRINT("%s: [band%d] set tx count th = %d\n",
		__func__, band_idx, ctrl->tx_count_th[band_idx]);

	return TRUE;
}

INT show_peak_adjust_ctrl_info(
	struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 BandIdx = 0;
	struct peak_enhance_ctrl *ctrl = &pAd->CommonCfg.peek_enhance;

	BandIdx = hc_get_hw_band_idx(pAd);

	if (BandIdx == INVALID_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("invalid BandIdx:%d\n", BandIdx);
		return FALSE;
	}

	MTWF_PRINT("Band%d:\n", BandIdx);
	MTWF_PRINT("    enable_adjust=0x%x\n", ctrl->enable_adjust);
	MTWF_PRINT("    avg_tx_pkt_len_th=%d\n", ctrl->avg_tx_pkt_len);
	MTWF_PRINT("    tx_count_th=%d\n", ctrl->tx_count_th[BandIdx]);
	MTWF_PRINT("    sim_tx_count=%d\n", ctrl->sim_tx_count[BandIdx]);
	MTWF_PRINT("    sim_avg_tx_pkt_len=%d\n", ctrl->sim_avg_tx_pkt_len);
	return TRUE;
}

#ifdef WHNAT_SUPPORT
void peak_adjust_rx_drop_handle(
	void *dev_obj)
{
	int i;
	struct physical_device *ph_dev = (struct physical_device *)dev_obj;
	struct peak_rx_enhance_ctrl *rx_ctrl = &ph_dev->peak_rx_ctrl;
#ifdef APCLI_SUPPORT
	struct mld_dev *mld = &mld_device;
	struct peer_mld_entry *peer = NULL;
#endif /* APCLI_SUPPORT */
	bool has_mlo_sta = FALSE;

	if (rx_ctrl->enable == FALSE)
		return;

#ifdef DOT11_EHT_BE
	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		struct bmgr_mld_sta *mld_sta = GET_MLD_STA_BY_IDX(i);
		struct bmgr_mld_link *mld_link;

		if (mld_sta->valid) {
			int j;

			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				mld_link = &mld_sta->mld_link[j];
				if (mld_link->active) {
					has_mlo_sta = TRUE;
					goto check;
				}
			}
		}
	}
#ifdef APCLI_SUPPORT
	if (mld->valid_link_num) {
		peer = &mld->peer_mld;
		for (i = 0; i < MLD_LINK_MAX; i++) {
			if (peer->single_link[i].active) {
				has_mlo_sta = TRUE;
				goto check;
			}
		}
	}
#endif /* APCLI_SUPPORT */
#endif /* DOT11_EHT_BE */

check:
	if (has_mlo_sta) {
		if (rx_ctrl->rx_drop == TRUE) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_RX_DROP, DBG_LVL_INFO,
				"[MLO] disable rx drop\n");
			mtk_set_pse_drop(0x0); /* not drop */
			rx_ctrl->rx_drop = FALSE;
		} else
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_RX_DROP, DBG_LVL_INFO,
				"[MLO] rx drop is already disabled\n");
	} else {
		if (rx_ctrl->rx_drop == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_RX_DROP, DBG_LVL_INFO,
				"[MLO] enable rx drop\n");
			mtk_set_pse_drop(0x300); /* drop */
			rx_ctrl->rx_drop = TRUE;
		} else
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_RX_DROP, DBG_LVL_INFO,
				"[MLO] rx drop is already enabled\n");
	}
}
#endif /* WHNAT_SUPPORT */
#endif /* PEAK_ENHANCE */
