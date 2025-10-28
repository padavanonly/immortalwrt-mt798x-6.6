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
#include "mgmt/be_internal.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

static uint16_t eht_build_sta_multi_link_ie(uint8_t *pos,
	struct wifi_dev *wdev, uint16_t type)
{
	return sta_mld_build_multi_link_ie(pos, wdev, type);
}

uint8_t *eht_build_multi_link_ie(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint8_t *ml_ie,
	struct frag_ie_info *ml_frag_info,
	uint16_t ml_type,
	uint16_t mld_sta_idx)
{
	uint8_t *pos = f_buf;

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		switch (ml_type) {
		case BMGR_QUERY_ML_IE_PROBE_REQ:
			pos += eht_build_sta_multi_link_ie(pos, wdev, ML_CTRL_TYPE_PROBE_REQ);
			break;
		case BMGR_QUERY_ML_IE_AUTH_REQ:
			pos += eht_build_sta_multi_link_ie(pos, wdev, BMGR_QUERY_ML_IE_AUTH_REQ);
			break;
		case BMGR_QUERY_ML_IE_ASSOC_REQ:
			pos += eht_build_sta_multi_link_ie(pos, wdev, ML_CTRL_TYPE_BASIC);
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
				"Unsupported ml_type: %u\n",
				ml_type);
			break;
		}
	} else {
		struct query_mld_info mld_query = {0};
                if (wdev->ap_mlo_disable) {
                       return pos; 
                }

		mld_query.query_type = ml_type;
		mld_query.ie.mld_sta_idx = mld_sta_idx;
		mld_query.ie.ml_ie = ml_ie;
		mld_query.ie.f_buf = f_buf;
		mld_query.ie.ml_frag_info = ml_frag_info;

		pos += bss_mngr_query_mld_info(wdev, &mld_query);
		if (ml_type == BMGR_QUERY_ML_IE_BCN) {
			wdev->bcn_buf.csa.csa_ie_offset_in_ml = mld_query.ie.csa_ie.csa_offset_in_ml_ie;
			wdev->bcn_buf.csa.csa_bss_idx = mld_query.ie.csa_ie.csa_bss_idx;
		}
	}

	return pos;
}

void eht_parse_multi_link_ie(
	struct wifi_dev *wdev, uint8_t *ml_ie, struct frag_ie_info *ml_frag_info, struct ml_ie_info *ml_info)
{
	struct query_mld_info mld_query = {0};

	if (wdev && (wdev->wdev_type == WDEV_TYPE_AP)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"wdev(%d)\n", wdev->wdev_idx);

		mld_query.query_type			= BMGR_QUERY_ML_PARSING_IE;
		mld_query.parse_ie.ml_ie		= ml_ie;
		mld_query.parse_ie.ml_info		= ml_info;
		mld_query.parse_ie.ml_frag_info	= ml_frag_info;

		bss_mngr_query_mld_info(wdev, &mld_query);
	}
}

uint8_t *eht_build_reconfig_multi_link_ie(
	struct wifi_dev *wdev,
	uint8_t *local_fbuf,
	uint16_t f_len,
	uint16_t ml_type)
{
	uint8_t *pos = local_fbuf;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		BSS_STRUCT *pMbss = wdev->func_dev;
		struct mld_reconfig_ie_query_t reconf_query = {0};
		struct reconf_ie_info_t *reconfig_ie_info;
		uint8_t link, offset_idx;

		reconf_query.f_buf = local_fbuf;
		pos += bss_mngr_mld_reconfig_ie_query(pMbss->mld_grp_idx, &reconf_query);

		if ((ml_type == BMGR_QUERY_ML_IE_BCN) && reconf_query.f_len) {

			reconfig_ie_info = &wdev->bcn_buf.reconf_info;

			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				/* offset=0 means no Per-STA Profile */
				if (!reconf_query.tmr_offset[link])
					continue;

				offset_idx = reconfig_ie_info->num_offset++;
				reconfig_ie_info->sub_info[offset_idx].fw_bss_idx = reconf_query.fw_bss_idx[link];
				reconfig_ie_info->sub_info[offset_idx].tmr_offset =
					f_len + reconf_query.tmr_offset[link];

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
					"query: link=%d, off_idx=%d, fw_bss=%d, tmr_off=%d\n",
					link, offset_idx, reconfig_ie_info->sub_info[offset_idx].fw_bss_idx,
					reconfig_ie_info->sub_info[offset_idx].tmr_offset);
			}
		}
	}

	return pos;
}


static uint8_t *eht_build_tid_to_link_map_ie(
	struct wifi_dev *wdev, uint8_t *f_buf)
{
	uint8_t *pos = f_buf;

	pos += bss_mngr_query_tid_to_link_ie(wdev, pos);

	return pos;
}

static uint8_t *eht_build_multi_link_traffic_ie(
	struct wifi_dev *wdev, uint8_t *f_buf_hdr, uint8_t *local_fbuf)
{
	uint8_t *pos = local_fbuf;

	pos += bss_mngr_query_multi_link_traffic_ie(wdev, f_buf_hdr, pos);

	return pos;
}

static uint8_t *eht_build_at2lm_ie(
	struct wifi_dev *wdev, uint8_t *f_buf_hdr, uint8_t *local_fbuf, u8 frame_type)
{
	uint8_t *pos = local_fbuf;

	pos += at2lm_ie_query(wdev, f_buf_hdr, pos, frame_type);

	return pos;
}

static UINT8 *eht_build_mac_cap(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct eht_mac_capinfo eht_mac_cap;
	UINT16 cap = 0;
	UINT8 *pos = f_buf;

	if (wlan_config_get_nsep_priority_access(wdev))
		SET_DOT11BE_MAC_CAP_NSEP_PRI_ACCESS(cap, 1)
	if (wlan_config_get_eht_om_ctrl(wdev))
		SET_DOT11BE_MAC_CAP_OM_CTRL(cap, 1)
	if (wlan_config_get_txop_sharing_trigger(wdev))
		SET_DOT11BE_MAC_CAP_TXOP_SHARING_MODE1(cap, 1)
	if (wlan_config_get_eht_restricted_twt(wdev))
		SET_DOT11BE_MAC_CAP_RESTRICTED_TWT(cap, 1)
	if (wlan_config_get_scs_traffic(wdev))
		SET_DOT11BE_MAC_CAP_SCS_TRAFFIC(cap, 1)

	SET_DOT11BE_MAC_CAP_MAX_MPDU_LEN(cap, chip_cap->ppdu.max_mpdu_len)

	eht_mac_cap.mac_capinfo = cpu2le16(cap);
	NdisMoveMemory(f_buf,
		(UINT8 *)&eht_mac_cap, sizeof(eht_mac_cap));
	pos += sizeof(eht_mac_cap);

	return pos;
}

static UINT8 *eht_build_phy_cap(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct eht_phy_capinfo eht_phy_cap = {0};
	UINT32 phy_cap_1 = 0;
	UINT32 phy_cap_2 = 0;
	UINT8 *pos = f_buf;
	enum PHY_CAP phy_caps;
	UINT8 eht_bw = wlan_config_get_eht_bw(wdev);
	UINT8 eht_mcs15_mru = EHT_MCS15_MRU_106_or_52_w_26_tone;
	BOOLEAN has_mu_feature = FALSE;
	UINT8 eht_cmm_nominal_pad = wlan_config_get_eht_cmm_nominal_pad(wdev);

	phy_caps = wlan_config_get_phy_caps(wdev);

	if (wlan_config_get_mu_dl_mimo(wdev) || wlan_config_get_mu_ul_mimo(wdev) ||
		wlan_config_get_pp_dl_mu_mimo(wdev) || wlan_config_get_pp_ul_mu_mimo(wdev))
		has_mu_feature = TRUE;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_6G)
		&& WMODE_CAP_BE_6G(wdev->PhyMode)) {
		if (eht_bw == EHT_BW_320)
			phy_cap_1 |= DOT11BE_PHY_CAP_320M_6G;
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_EHT_DUP) && ((wlan_config_get_mcs14_disable(wdev) == FALSE)))
			phy_cap_2 |= DOT11BE_PHY_CAP_EHT_DUP_6G;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_EHT_PPE_EXIST))
			phy_cap_2 |= DOT11BE_PHY_CAP_PPE_THRLD_PRESENT;
	}

	if (eht_bw >= EHT_BW_80)
		eht_mcs15_mru |= EHT_MCS15_MRU_484_w_242_tone_80M;
	if (eht_bw >= EHT_BW_160)
		eht_mcs15_mru |= EHT_MCS15_MRU_996_to_242_tone_160M;
	if (eht_bw == EHT_BW_320)
		eht_mcs15_mru |= EHT_MCS15_MRU_3x996_tone_320M;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TXBF)) {
		struct eht_bf_info eht_bf_struct;

		NdisZeroMemory(&eht_bf_struct, sizeof(struct eht_bf_info));
		mt_wrap_get_eht_bf_cap(wdev, &eht_bf_struct);

		if (has_mu_feature == TRUE) {
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				phy_cap_2 |= DOT11BE_PHY_CAP_MU_BFER_LE_EQ_80M;
				if (eht_bw > EHT_BW_80)
					phy_cap_2 |= DOT11BE_PHY_CAP_MU_BFER_160M;

				if (WMODE_CAP_BE_6G(wdev->PhyMode) && (eht_bw > EHT_BW_160))
					phy_cap_2 |= DOT11BE_PHY_CAP_MU_BFER_320M;
			}
		}
#ifdef EHT_PARTIAL_BW_MU_MIMO_FEATURE
		if ((wdev->wdev_type == WDEV_TYPE_STA) && wlan_config_get_mu_dl_mimo(wdev))
			phy_cap_2 |= DOT11BE_PHY_CAP_PARTIAL_BW_DL_MU_MIMO;
#endif

		if (wlan_config_get_mu_ul_mimo(wdev)) {
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				phy_cap_2 |= DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_LE_EQ_BW80;
				if (eht_bw > EHT_BW_80)
					phy_cap_2 |= DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_BW160;

				if (WMODE_CAP_BE_6G(wdev->PhyMode) && (eht_bw == EHT_BW_320))
					phy_cap_2 |= DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_BW320;
			}
#ifdef EHT_PARTIAL_BW_MU_MIMO_FEATURE
			phy_cap_1 |= DOT11BE_PHY_CAP_PARTIAL_BW_UL_MU_MIMO;
#endif
		}

		if (eht_bf_struct.bf_cap & EHT_SU_BFER) {
			phy_cap_1 |= DOT11BE_PHY_CAP_SU_BFER;

			SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M(phy_cap_1, eht_bf_struct.snd_dim_le_eq_bw80);
			if (eht_bw > EHT_BW_80) {
				SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_160M(phy_cap_1, eht_bf_struct.snd_dim_bw160);
				if (eht_bw > EHT_BW_160)
					SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_320M(phy_cap_1, eht_bf_struct.snd_dim_bw320);
			}
		}

		if (eht_bf_struct.bf_cap & EHT_SU_BFEE) {
			phy_cap_1 |= DOT11BE_PHY_CAP_SU_BFEE;
			phy_cap_1 |= DOT11BE_PHY_CAP_NDP_4X_EHT_LTF_3DOT2US_GI;

			if (eht_bf_struct.bf_cap & EHT_BFEE_NG16_SU_FEEDBACK)
				phy_cap_1 |= DOT11BE_PHY_CAP_NG16_SU_FEEDBACK;

			if (eht_bf_struct.bf_cap & EHT_BFEE_NG16_MU_FEEDBACK)
				phy_cap_1 |= DOT11BE_PHY_CAP_NG16_MU_FEEDBACK;

			if (eht_bf_struct.bf_cap & EHT_BFEE_CODEBOOK_4_2_SU_FEEDBACK)
				phy_cap_1 |= DOT11BE_PHY_CAP_CODEBOOK_4_2_SU_FEEDBACK;

			if (eht_bf_struct.bf_cap & EHT_BFEE_CODEBOOK_7_5_SU_FEEDBACK)
				phy_cap_1 |= DOT11BE_PHY_CAP_CODEBOOK_7_5_SU_FEEDBACK;

			SET_DOT11BE_PHY_CAP_BFEE_SS_LE_EQ_80M(phy_cap_1, eht_bf_struct.bfee_ss_le_eq_bw80);
			if (eht_bw > EHT_BW_80) {
				SET_DOT11BE_PHY_CAP_BFEE_SS_160M(phy_cap_1, eht_bf_struct.bfee_ss_bw160);
				if (eht_bw > EHT_BW_160)
					SET_DOT11BE_PHY_CAP_BFEE_SS_320M(phy_cap_1, eht_bf_struct.bfee_ss_bw320);
			}

			if (wdev->wdev_type == WDEV_TYPE_STA) {
				if (eht_bw == EHT_BW_20)
					phy_cap_1 &= ~DOT11BE_PHY_CAP_242_TONE_RU_WT_20M;
			}
		}

		if (eht_bf_struct.bf_cap & EHT_TRIGED_SU_BF_FEEDBACK)
			phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_SU_BF_FEEDBACK;

		if ((eht_bf_struct.bf_cap & EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK) && (has_mu_feature == TRUE))
			phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_MU_BF_PARTIAL_BW_FEEDBACK;

		if (eht_bf_struct.bf_cap & EHT_TRIGED_CQI_FEEDBACK)
			phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_CQI_FEEDBACK;

		if (eht_bf_struct.bf_cap & EHT_NON_TRIGED_CQI_FEEDBACK)
			phy_cap_2 |= DOT11BE_PHY_CAP_NON_TRIGED_CQI_FEEDBACK;

		SET_DOT11BE_PHY_CAP_MAX_NC(phy_cap_2, eht_bf_struct.bfee_max_nc);

		if (wlan_config_get_extra_eht_ltf_disable(wdev) == FALSE)
			SET_DOT11BE_PHY_CAP_MAX_EHT_LTF_NUM(phy_cap_2, eht_bf_struct.max_eht_ltf_num);
	}

	phy_cap_2 |= DOT11BE_PHY_CAP_EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI;

	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_EHT_PPE_EXIST))
		SET_DOT11BE_PHY_CAP_COMMON_NOMINAL_PKT_PAD(phy_cap_2, eht_cmm_nominal_pad);

	if (wlan_config_get_mcs15_in_mru_disable(wdev) == FALSE)
		SET_DOT11BE_PHY_CAP_MCS_15(phy_cap_2, eht_mcs15_mru);

	eht_phy_cap.phy_capinfo_1 = cpu_to_le32(phy_cap_1);
	eht_phy_cap.phy_capinfo_2 = cpu_to_le32(phy_cap_2);
	NdisMoveMemory(f_buf,
		(UINT8 *)&eht_phy_cap, sizeof(eht_phy_cap));
	pos += sizeof(eht_phy_cap);
	return pos;
}

void eht_mcs_map_bw20(UINT8 tx_nss, UINT8 rx_nss, UINT8 max_mcs, struct eht_txrx_mcs_nss_20 *eht_mcs_bw20)
{
	eht_mcs_bw20->max_tx_rx_mcs0_7_nss |= rx_nss & DOT11BE_MCS_NSS_MASK;
	eht_mcs_bw20->max_tx_rx_mcs0_7_nss |=
		(tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	if (max_mcs > MCS_7) {
		eht_mcs_bw20->max_tx_rx_mcs8_9_nss |= rx_nss & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs8_9_nss |=
			(tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
	if (max_mcs > MCS_9) {
		eht_mcs_bw20->max_tx_rx_mcs10_11_nss |= rx_nss & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs10_11_nss |=
			(tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
	if (max_mcs > MCS_11) {
		eht_mcs_bw20->max_tx_rx_mcs12_13_nss |= rx_nss & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs12_13_nss |=
			(tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
}

void eht_mcs_map(UINT8 tx_nss, UINT8 rx_nss, UINT8 max_mcs, struct eht_txrx_mcs_nss *eht_mcs)
{

	eht_mcs->max_tx_rx_mcs0_9_nss = ((tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (rx_nss & DOT11BE_MCS_NSS_MASK);
	if (max_mcs > MCS_9)
		eht_mcs->max_tx_rx_mcs10_11_nss = ((tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (rx_nss & DOT11BE_MCS_NSS_MASK);
	if (max_mcs > MCS_11)
		eht_mcs->max_tx_rx_mcs12_13_nss = ((tx_nss & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (rx_nss & DOT11BE_MCS_NSS_MASK);
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
static void eht_mcs_eap_map_bw20(struct eht_nss_mcs eap_tx_nss, struct eht_nss_mcs eap_rx_nss, UINT8 max_mcs,
								struct eht_txrx_mcs_nss_20 *eht_mcs_bw20)
{
	eht_mcs_bw20->max_tx_rx_mcs0_7_nss |= eap_rx_nss.mcs0_7 & DOT11BE_MCS_NSS_MASK;
	eht_mcs_bw20->max_tx_rx_mcs0_7_nss |=
		(eap_tx_nss.mcs0_7 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	if (max_mcs > MCS_7) {
		eht_mcs_bw20->max_tx_rx_mcs8_9_nss |= eap_rx_nss.mcs8_9 & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs8_9_nss |=
			(eap_tx_nss.mcs8_9 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
	if (max_mcs > MCS_9) {
		eht_mcs_bw20->max_tx_rx_mcs10_11_nss |= eap_rx_nss.mcs10_11 & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs10_11_nss |=
			(eap_tx_nss.mcs10_11 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
	if (max_mcs > MCS_11) {
		eht_mcs_bw20->max_tx_rx_mcs12_13_nss |= eap_rx_nss.mcs12_13 & DOT11BE_MCS_NSS_MASK;
		eht_mcs_bw20->max_tx_rx_mcs12_13_nss |=
			(eap_tx_nss.mcs12_13 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT;
	}
}

static void eht_mcs_eap_map(struct eht_nss_mcs eap_tx_nss, struct eht_nss_mcs eap_rx_nss, UINT8 max_mcs, struct eht_txrx_mcs_nss *eht_mcs)
{
	eht_mcs->max_tx_rx_mcs0_9_nss = ((eap_tx_nss.mcs0_9 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (eap_rx_nss.mcs0_9 & DOT11BE_MCS_NSS_MASK);
	if (max_mcs > MCS_9)
		eht_mcs->max_tx_rx_mcs10_11_nss = ((eap_tx_nss.mcs10_11 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (eap_rx_nss.mcs10_11 & DOT11BE_MCS_NSS_MASK);
	if (max_mcs > MCS_11)
		eht_mcs->max_tx_rx_mcs12_13_nss = ((eap_tx_nss.mcs12_13 & DOT11BE_MCS_NSS_MASK) << DOT11BE_MCS_NSS_SHIFT) | (eap_rx_nss.mcs12_13 & DOT11BE_MCS_NSS_MASK);
}
#endif

static UINT8 *eht_build_mcs_nss(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct eht_txrx_mcs_nss eht_mcs_nss = {0};
	struct eht_txrx_mcs_nss_20 eht_mcs_nss_bw20 = {0};
	UINT8 eht_bw = wlan_config_get_eht_bw(wdev);
	UINT8 tx_nss = wlan_config_get_tx_stream(wdev);
	UINT8 rx_nss = wlan_config_get_rx_stream(wdev);
	UINT8 max_mcs = wlan_config_get_eht_max_mcs(wdev);
#ifdef MAP_R6
	struct _RTMP_ADAPTER *pAd = NULL;
#endif

	if (max_mcs == MCS_0)
		max_mcs = MCS_13;
#ifdef MAP_R6
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if ((pAd) && (IS_MAP_ENABLE(pAd)) && (IS_MAP_CERT_ENABLE(pAd)))
		max_mcs = MCS_9;
#endif

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
#define SET_EHT_MCS_NSS_BW20(eap_rx_nss, eap_tx_nss)							\
	do {																			\
		if (wdev->eap.eap_ehtsuprate_en) {											\
			eht_mcs_eap_map_bw20(eap_tx_nss, eap_rx_nss, max_mcs, &eht_mcs_nss_bw20);	\
		}																			\
		else {																		\
			eht_mcs_map_bw20(tx_nss, rx_nss, max_mcs, &eht_mcs_nss_bw20);				\
		}																			\
	} while (0)
#define SET_EHT_MCS_NSS(eap_rx_nss, eap_tx_nss)							\
	do {																	\
		if (wdev->eap.eap_ehtsuprate_en) {									\
			eht_mcs_eap_map(eap_tx_nss, eap_rx_nss, max_mcs, &eht_mcs_nss);	\
		}																	\
		else {																\
			eht_mcs_map(tx_nss, rx_nss, max_mcs, &eht_mcs_nss);				\
		}																	\
	} while (0)
#else
#define SET_EHT_MCS_NSS_BW20(eap_rx_nss, eap_tx_nss)			\
	eht_mcs_map_bw20(tx_nss, rx_nss, max_mcs, &eht_mcs_nss_bw20)
#define SET_EHT_MCS_NSS(eap_rx_nss, eap_tx_nss)					\
	eht_mcs_map(tx_nss, rx_nss, max_mcs, &eht_mcs_nss)
#endif

	if (eht_bw == EHT_BW_20) {
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			SET_EHT_MCS_NSS_BW20(wdev->eap.rate.rx_nss_mcs[EHT_NSS_MCS_20], wdev->eap.rate.tx_nss_mcs[EHT_NSS_MCS_20]);
			NdisMoveMemory(pos, (UINT8 *)&eht_mcs_nss_bw20, sizeof(eht_mcs_nss_bw20));
			pos += sizeof(struct eht_txrx_mcs_nss_20);/*20M -only */
		} else {
			SET_EHT_MCS_NSS(wdev->eap.rate.rx_nss_mcs[EHT_NSS_MCS_80], wdev->eap.rate.tx_nss_mcs[EHT_NSS_MCS_80]);
			NdisMoveMemory(pos, (UINT8 *)&eht_mcs_nss, sizeof(eht_mcs_nss));
			pos += sizeof(eht_mcs_nss);
		}
	} else if (eht_bw > EHT_BW_20) {
		SET_EHT_MCS_NSS(wdev->eap.rate.rx_nss_mcs[EHT_NSS_MCS_80], wdev->eap.rate.tx_nss_mcs[EHT_NSS_MCS_80]);
		NdisMoveMemory(pos, (UINT8 *)&eht_mcs_nss, sizeof(eht_mcs_nss));
		pos += sizeof(eht_mcs_nss);

		if (eht_bw > EHT_BW_80) {
			SET_EHT_MCS_NSS(wdev->eap.rate.rx_nss_mcs[EHT_NSS_MCS_160], wdev->eap.rate.tx_nss_mcs[EHT_NSS_MCS_160]);
			NdisMoveMemory(pos, (UINT8 *)&eht_mcs_nss, sizeof(eht_mcs_nss));
			pos += sizeof(eht_mcs_nss);

			if (eht_bw == EHT_BW_320) {
				SET_EHT_MCS_NSS(wdev->eap.rate.rx_nss_mcs[EHT_NSS_MCS_320], wdev->eap.rate.tx_nss_mcs[EHT_NSS_MCS_320]);
				NdisMoveMemory(pos, (UINT8 *)&eht_mcs_nss, sizeof(eht_mcs_nss));
				pos += sizeof(eht_mcs_nss);
			}
		}
	}
	return pos;
}

static UINT8 *eht_build_ppe(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	EHT_PORTING_TODO(Remember to add EHT PPE Setting);
	return pos;
}

static UINT8 *eht_build_op_ie(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	UINT8 ie_len = 0;
	UINT8 prim_ch = wdev->channel;
	UCHAR eht_cen_ch = 0;
	UINT8 eht_bw = wlan_operate_get_eht_bw(wdev);
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	struct eht_op_info op_info;
	struct eht_op_ie op_ie, *op_ie_ptr;
	UINT8 dis_subch_bitmap_enable = wlan_config_get_eht_dscb_enable(wdev);
	UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
	UINT8 bw_diff = TRUE;

	os_zero_mem(&op_ie, sizeof(op_ie));
	eid->Eid = IE_WLAN_EXTENSION;
	eid->Octet[0] = EID_EXT_EHT_OP;
	pos += sizeof(struct _EID_STRUCT);

	/* Basic EHT-MCS And Nss Set: minimum requirement for all sta to join the BSS */
	eht_mcs_map_bw20(1, 1, MCS_7, &op_ie.basic_eht_mcs_nss);

	op_ie_ptr = (struct eht_op_ie *)pos;
	NdisMoveMemory(pos, (u8 *)&op_ie, sizeof(u8) + sizeof(struct eht_txrx_mcs_nss_20));
	pos += sizeof(u8) + sizeof(struct eht_txrx_mcs_nss_20);

	switch (vht_bw) {
	case VHT_BW_2040:
		if (eht_bw == EHT_BW_2040)
			bw_diff = FALSE;
		break;
	case VHT_BW_80:
		if (eht_bw == EHT_BW_80)
			bw_diff = FALSE;
		break;
	case VHT_BW_160:
		if (eht_bw == EHT_BW_160)
			bw_diff = FALSE;
		break;
	case VHT_BW_8080:
		break;
	case VHT_BW_320:
		if (eht_bw == EHT_BW_320)
			bw_diff = FALSE;
		break;
	}

	if (bw_diff || dis_subch_bitmap_enable) {
		os_zero_mem(&op_info, sizeof(struct eht_op_info));
		/*
		 * The EHT Operation Information Present subfield is set to 1
		 * if the channel width indicated in an HT Operation, VHT Operation,
		 * or HE Operation element that is present in the same Managementframe
		 * is different from the Channel Width field indicated
		 * in the EHT Operation Information field.
		 */
		SET_DOT11BE_OP_PARAM_HAS_OP_INFO(op_ie_ptr->op_parameters, 1);
		eht_cen_ch = eht_cent_ch_freq(wdev, wdev->channel, eht_bw, ch_band);
		switch (eht_bw) {
		case EHT_BW_320:
			SET_DOT11BE_OP_CTRL_CH_BW(op_info.control, EHT_OP_CH_BW320);
			op_info.ccfs0 = GET_BW320_PRIM160_CENT(prim_ch, eht_cen_ch);
			op_info.ccfs1 = eht_cen_ch;
			break;
		case EHT_BW_160:
			SET_DOT11BE_OP_CTRL_CH_BW(op_info.control, EHT_OP_CH_BW160);
			op_info.ccfs0 = GET_BW160_PRIM80_CENT(prim_ch, eht_cen_ch);
			op_info.ccfs1 = eht_cen_ch;
			break;
		case EHT_BW_80:
			SET_DOT11BE_OP_CTRL_CH_BW(op_info.control, EHT_OP_CH_BW80);
			op_info.ccfs0 = eht_cen_ch;
			op_info.ccfs1 = 0;
			break;
		case EHT_BW_2040:
			SET_DOT11BE_OP_CTRL_CH_BW(op_info.control, EHT_OP_CH_BW40);
			op_info.ccfs0 = eht_cen_ch;
			op_info.ccfs1 = 0;
			break;
		default:
			SET_DOT11BE_OP_CTRL_CH_BW(op_info.control, EHT_OP_CH_BW20);
			op_info.ccfs0 = eht_cen_ch;
			op_info.ccfs1 = 0;
		}
		NdisMoveMemory(pos,
			(UINT8 *)&op_info, sizeof(struct eht_op_info));
		pos += sizeof(struct eht_op_info);
	}

	/* Disabled Subchannel Bitmap */
	if (dis_subch_bitmap_enable) {
		UINT16 dis_subch_bitmap = 0;

		SET_DOT11BE_OP_PARAM_HAS_DIS_SUB_CH_BITMAP(op_ie_ptr->op_parameters, 1);
		dis_subch_bitmap = cpu2le16(wlan_config_get_eht_dis_subch_bitmap(wdev));
		NdisMoveMemory(pos,
			(UINT8 *)&dis_subch_bitmap, sizeof(dis_subch_bitmap));
		pos += sizeof(dis_subch_bitmap);
	}

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

/*
 * EHT Capabilities element format
 *     EHT MAC Capabilities Information (TBD)
 *     EHT PHY Capabilities Information (TBD)
 *     Supported EHT-MCS And NSS Set (TBD)
 *     EHT PPE Thresholds (Optional)
 */
UINT8 *eht_build_cap_ie(
	struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	enum PHY_CAP phy_caps = wlan_config_get_phy_caps(wdev);
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	UINT8 ie_len = 0;

	eid->Eid = IE_WLAN_EXTENSION;
	eid->Octet[0] = EID_EXT_EHT_CAPS;
	pos += sizeof(struct _EID_STRUCT);

	/* EHT MAC Capablities Information */
	pos = eht_build_mac_cap(wdev, pos);

	/* EHT PHY Capablities Information */
	pos = eht_build_phy_cap(wdev, pos);

	/* Supported EHT-MCS And NSS Set */
	pos = eht_build_mcs_nss(wdev, pos);

	/* EHT PPE Thresholds (optional) */
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_EHT_PPE_EXIST))
		pos = eht_build_ppe(wdev, pos);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

int eht_get_multi_link_ie(
	uint8_t *ie_head, u8 *ml_ie, struct frag_ie_info *ml_frag_info, enum ie_exist *ie_exists, struct frag_ie_info *frag_info)
{
	uint8_t eie_len = ((struct _EID_STRUCT *)ie_head)->Len;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)ie_head;
	uint16_t ie_len_all;

	if (eie_len < ML_IE_MINIMUM_LEN) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"MLIE less than the minimum length!\n");
		return -1;
	}
	if (eie_len + 2 > MAX_LEN_OF_MLIE) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"MLIE less than the minimum length!\n");
		return -1;
	}

	ie_len_all = LEN_OF_IE_TAG_LENG + eid->Len;

	if (frag_info && eid->Len == MAX_LEN_OF_IE_DATA) {
		while ((ie_head + ie_len_all) < frag_info->frm_end) {
			eid = (struct _EID_STRUCT *)(ie_head + ie_len_all);
			if (eid->Eid == IE_FRAGMENT && eid->Len > 0)
				ie_len_all += (LEN_OF_IE_TAG_LENG + eid->Len);
			else
				break;
		}
	}

	/* copy whole multi-link ie include eid and eid_len */
	if (frag_info && ie_len_all > (LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA)) {
		/* IE defragmentation */
		if (defragment_information_element(ie_head, ie_len_all, ml_ie,
			&frag_info->ie_len_defrag) == NDIS_STATUS_FAILURE) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
				"failed to defrag MLIE!\n");
			frag_info->is_frag = FALSE;
			frag_info->ie_len_all = 0;
			return -1;
		}
		frag_info->ie_len_all = ie_len_all;
		frag_info->is_frag = TRUE;
		NdisMoveMemory(ml_frag_info, frag_info, sizeof(struct frag_ie_info));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"MLIE Frag: len_all=%d, len_defrag=%d\n",
			frag_info->ie_len_all, frag_info->ie_len_defrag);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"MLIE Frag: len_all=%d, len_defrag=%d\n",
			ml_frag_info->ie_len_all, ml_frag_info->ie_len_defrag);
	} else
		NdisMoveMemory(ml_ie, ie_head, eie_len + 2);

	SET_EHT_MLD_EXIST(*ie_exists);

	if (ml_reconfig_ie_check(ml_ie))
		SET_EHT_ML_RECONFIG_EXIST(*ie_exists);

	return 0;
}

int eht_get_caps_ie(
	UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
		UINT32 partial_ie_len = 0;
		UINT8 *ie_ptr = ie_ctx;
		UINT8 ch_width = HE_BW_80;
		UINT8 mcs_nss_ie_len = 0;

		if ((ie_len < sizeof(cmm_ies->eht_caps)) || (ie_len == 255)) /* overflow */
			return -1;
		SET_EHT_CAPS_EXIST(cmm_ies->ie_exists);
		partial_ie_len = sizeof(cmm_ies->eht_caps);
		mcs_nss_ie_len = ie_len - partial_ie_len;
		NdisMoveMemory((UINT8 *)&cmm_ies->eht_caps, ie_ptr, partial_ie_len);
#ifdef CFG_BIG_ENDIAN
		cmm_ies->eht_caps.mac_cap.mac_capinfo
			= le2cpu16(cmm_ies->eht_caps.mac_cap.mac_capinfo);
		cmm_ies->eht_caps.phy_cap.phy_capinfo_1
			= le2cpu32(cmm_ies->eht_caps.phy_cap.phy_capinfo_1);
		cmm_ies->eht_caps.phy_cap.phy_capinfo_2
			= le2cpu32(cmm_ies->eht_caps.phy_cap.phy_capinfo_2);
#endif /* CFG_BIG_ENDIAN */
#ifdef DOT11_HE_AX
		if (!mcs_nss_ie_len) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"[LOG] EHT_CAPS_IE don't have msc_nss_ie\n");
			return 0;
		}
		ie_ptr += partial_ie_len;
		if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists))
			ch_width = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(cmm_ies->he_caps.phy_cap.phy_capinfo_1));

		if (ch_width == HE_BW_20) {
			if (mcs_nss_ie_len >= sizeof(struct eht_txrx_mcs_nss_20))
				NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only, ie_ptr, sizeof(struct eht_txrx_mcs_nss_20));
			/*If STA config BW40, there will be no
			"eht_txrx_mcs_nss_20_only" information from STA;
			However, AP may fallback to BW20
			in some special cases (ex: BSSCoexistence)
			thus we need to create "20M-only nss/mcs"
			information from "rx_nss_mcs"(>20M) manually*/
			if (mcs_nss_ie_len != EHT_NSS_MCS_20M_ONLY && mcs_nss_ie_len >= sizeof(struct eht_txrx_mcs_nss)) {
				NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0], ie_ptr, sizeof(struct eht_txrx_mcs_nss));
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss = 0;
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss & DOT11BE_MCS_NSS_MASK);
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss
																							& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT));
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss = 0;
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss & DOT11BE_MCS_NSS_MASK);
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss
																							& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT));
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss = 0;
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs10_11_nss & DOT11BE_MCS_NSS_MASK);
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs10_11_nss
																							& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT));
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss = 0;
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs12_13_nss & DOT11BE_MCS_NSS_MASK);
				cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss |= (cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs12_13_nss
																							& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT));
			}
		} else {
			/* Need to notice that you may copy eht_txrx_mcs_nss_20_only into */
			/* cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0] */
			/* if (mcs_nss_ie_len == EHT_NSS_MCS_20M_ONLY). */
			/* We can not know the peer max bw at this stage. */
			if (mcs_nss_ie_len >= sizeof(struct eht_txrx_mcs_nss))
				NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[0], ie_ptr, sizeof(struct eht_txrx_mcs_nss));
			if (mcs_nss_ie_len == EHT_NSS_MCS_20M_ONLY)
				NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only, ie_ptr, sizeof(struct eht_txrx_mcs_nss_20));
			ie_ptr += sizeof(struct eht_txrx_mcs_nss);
			mcs_nss_ie_len -= sizeof(struct eht_txrx_mcs_nss);

			if (ch_width >= HE_BW_160 && mcs_nss_ie_len >= sizeof(struct eht_txrx_mcs_nss)) {
				NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[1], ie_ptr, sizeof(struct eht_txrx_mcs_nss));
				ie_ptr += sizeof(struct eht_txrx_mcs_nss);
				mcs_nss_ie_len -= sizeof(struct eht_txrx_mcs_nss);

				if (cmm_ies->eht_caps.phy_cap.phy_capinfo_1 & DOT11BE_PHY_CAP_320M_6G &&
					mcs_nss_ie_len >= sizeof(struct eht_txrx_mcs_nss))
					NdisMoveMemory(&cmm_ies->eht_support_mcs_nss.eht_txrx_mcs_nss[2], ie_ptr, sizeof(struct eht_txrx_mcs_nss));
			}
		}
#endif
	return 0;
}

int eht_get_op_ie(
	UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;
	UINT8 left_len = ie_len;

	if (ie_len < EHT_OP_IE_BASIC_LEN) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"[LOG] EHT OP IE less than the minimum length!\n");
		return -1;
	} else if (ie_len > EHT_OP_IE_MAX_LEN) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"[LOG] EHT OP IE exceed the maximum length!\n");
		return -1;
	}
	if (GET_DOT11BE_OP_PARAM_HAS_OP_INFO(ie_ctx[0])) {
		partial_ie_len = sizeof(struct eht_op_ie);
		if (partial_ie_len <= left_len)
			NdisMoveMemory((UINT8 *)&cmm_ies->eht_op, ie_ptr, partial_ie_len);
		else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"[LOG] the length of EHT OP IE is abnormal!\n");
			return -1;
		}
		left_len -= partial_ie_len;
		if (GET_DOT11BE_OP_PARAM_HAS_DIS_SUB_CH_BITMAP(ie_ctx[0])) {
			ie_ptr += partial_ie_len;
			partial_ie_len = sizeof(cmm_ies->dis_subch_bitmap);
			if (partial_ie_len <= left_len) {
				NdisMoveMemory((UINT8 *)&cmm_ies->dis_subch_bitmap, ie_ptr, partial_ie_len);
				cmm_ies->dis_subch_bitmap = le2cpu16(cmm_ies->dis_subch_bitmap);
				cmm_ies->dscb_enable = TRUE;
			} else {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
						"[LOG] the length of EHT OP IE is abnormal!\n");
				return -1;
			}
		} else {
			cmm_ies->dis_subch_bitmap = 0;
			cmm_ies->dscb_enable = FALSE;
		}
	} else {
		/* EHT Operation Parameters exists only */
		partial_ie_len = EHT_OP_IE_BASIC_LEN;
		if (partial_ie_len <= left_len)
			NdisMoveMemory((UINT8 *)&cmm_ies->eht_op, ie_ptr, partial_ie_len);
		else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"[LOG] the length of EHT OP IE is abnormal!\n");
			return -1;
		}
		left_len -= partial_ie_len;
		if (GET_DOT11BE_OP_PARAM_HAS_DIS_SUB_CH_BITMAP(ie_ctx[0])) {
			ie_ptr += partial_ie_len;
			partial_ie_len = sizeof(cmm_ies->dis_subch_bitmap);
			if (partial_ie_len <= left_len) {
				NdisMoveMemory((UINT8 *)&cmm_ies->dis_subch_bitmap, ie_ptr, partial_ie_len);
				cmm_ies->dis_subch_bitmap = le2cpu16(cmm_ies->dis_subch_bitmap);
				cmm_ies->dscb_enable = TRUE;
			} else {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"[LOG] the length of EHT OP IE is abnormal!\n");
				return -1;
			}
		} else {
			cmm_ies->dis_subch_bitmap = 0;
			cmm_ies->dscb_enable = FALSE;
		}
	}
	SET_EHT_OP_EXIST(cmm_ies->ie_exists);
	return 0;
}

static void eht_show_mac_caps(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *peer)
{
	if (wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "Self eht mac caps:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tnsep_priority_access = %d\n"
		, wlan_config_get_nsep_priority_access(wdev));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tom_ctrl = %d\n"
		, wlan_config_get_eht_om_ctrl(wdev));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\ttxop_sharing_trigger = %d\n"
		, wlan_config_get_txop_sharing_trigger(wdev));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\trestricted twt support = %d\n"
		, wlan_config_get_eht_restricted_twt(wdev));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tscs traffic support = %d\n"
		, wlan_config_get_scs_traffic(wdev));
	}
	if (peer) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "Peer eht mac caps:\n");
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tnsep_priority_access = %d\n"
		, peer->eht_cfg.nsep_priority_access_enable);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tom_ctrl = %d\n"
		, peer->eht_cfg.eht_om_ctrl_enable);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\ttxop_sharing_trigger = %d\n"
		, peer->eht_cfg.txop_sharing_trigger_enable);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\trestricted twt support = %d\n"
		, peer->eht_cfg.restricted_twt_support);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tscs traffic support = %d\n"
		, peer->eht_cfg.scs_traffic_support);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO
		, "\tpeer->AMsduSize = %d\n"
		, peer->AMsduSize);
	}
}

static void eht_update_peer_mac_caps(
	struct _MAC_TABLE_ENTRY *peer, struct eht_mac_capinfo *mac_cap)
{
	struct wifi_dev *wdev = peer->wdev;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_CAP *chip_cap = PD_GET_CHIP_CAP_PTR(ad->physical_dev);
	struct eht_mlo_t *peer_mlo = &peer->mlo;
	struct mld_entry_t mld_entry = {0};
	struct eht_cfg *peer_eht_cfg = &peer->eht_cfg;
	u8 amsdu_size = 0;

	peer->eht_mac_cap = mac_cap->mac_capinfo;
	if (peer_mlo->mlo_en) {
		if (GET_DOT11BE_MAC_CAP_NSEP_PRI_ACCESS(mac_cap->mac_capinfo)
			&& wlan_config_get_nsep_priority_access(wdev))
			mld_entry.nsep = TRUE;
		else
			mld_entry.nsep = FALSE;
		mld_entry_fill_fields(peer_mlo->mld_addr, &mld_entry,
								MLD_ENTRY_FIELD_FLAGS_NSEP);
	}

	if (GET_DOT11BE_MAC_CAP_OM_CTRL(mac_cap->mac_capinfo)
		&& wlan_config_get_eht_om_ctrl(wdev))
		peer_eht_cfg->eht_om_ctrl_enable = TRUE;
	else
		peer_eht_cfg->eht_om_ctrl_enable = FALSE;

	if (GET_DOT11BE_MAC_CAP_TXOP_SHARING_MODE1(mac_cap->mac_capinfo)
		&& wlan_config_get_txop_sharing_trigger(wdev))
		peer_eht_cfg->txop_sharing_trigger_enable = TRUE;
	else
		peer_eht_cfg->txop_sharing_trigger_enable = FALSE;

	if (GET_DOT11BE_MAC_CAP_RESTRICTED_TWT(mac_cap->mac_capinfo)
		&& wlan_config_get_eht_restricted_twt(wdev))
		peer_eht_cfg->restricted_twt_support = TRUE;
	else
		peer_eht_cfg->restricted_twt_support = FALSE;

	if (GET_DOT11BE_MAC_CAP_SCS_TRAFFIC(mac_cap->mac_capinfo)
		&& wlan_config_get_scs_traffic(wdev))
		peer_eht_cfg->scs_traffic_support = TRUE;
	else
		peer_eht_cfg->scs_traffic_support = FALSE;

	amsdu_size = GET_DOT11BE_MAC_CAP_MAX_MPDU_LEN(mac_cap->mac_capinfo);
	if (amsdu_size > chip_cap->ppdu.max_mpdu_len)
		amsdu_size = chip_cap->ppdu.max_mpdu_len;

	if (amsdu_size)
		peer->AMsduSize = amsdu_size;
	eht_show_mac_caps(wdev, peer);
}

static void eht_update_peer_phy_caps(
	struct _MAC_TABLE_ENTRY *peer, struct eht_phy_capinfo *phy_cap)
{
	/*init eht_phy_cap*/
	peer->cap.eht_phy_cap = 0;
	peer->cap.eht_gi = 0;
	peer->cap.eht_bf.bf_cap = 0;
	peer->eht_phy_cap = phy_cap->phy_capinfo_2;
	peer->eht_phy_cap = (peer->eht_phy_cap << 32) | phy_cap->phy_capinfo_1;
	peer->eht_phy_cap_ext |= phy_cap->phy_capinfo_3;

	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_320M_6G)
		peer->cap.eht_phy_cap |= EHT_320M_6G;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_242_TONE_RU_WT_20M)
		peer->cap.eht_phy_cap |= EHT_242_TONE_RU_WT_20M;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_NDP_4X_EHT_LTF_3DOT2US_GI)
		peer->cap.eht_gi |= EHT_NDP_4X_EHT_LTF_3DOT2US_GI;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_PARTIAL_BW_UL_MU_MIMO)
		peer->cap.eht_phy_cap |= EHT_PARTIAL_BW_UL_MI_MIMO;

	/* bf */
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_SU_BFER)
		peer->cap.eht_bf.bf_cap |= EHT_SU_BFER;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_SU_BFEE)
		peer->cap.eht_bf.bf_cap |= EHT_SU_BFEE;
	peer->cap.eht_bf.bfee_ss_le_eq_bw80 = GET_DOT11BE_PHY_CAP_BFEE_SS_LE_EQ_80M(phy_cap->phy_capinfo_1);
	peer->cap.eht_bf.bfee_ss_bw160 = GET_DOT11BE_PHY_CAP_BFEE_SS_160M(phy_cap->phy_capinfo_1);
	peer->cap.eht_bf.bfee_ss_bw320 = GET_DOT11BE_PHY_CAP_BFEE_SS_320M(phy_cap->phy_capinfo_1);
	peer->cap.eht_bf.snd_dim_le_eq_bw80 = GET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M(phy_cap->phy_capinfo_1);
	peer->cap.eht_bf.snd_dim_bw160 = GET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_160M(phy_cap->phy_capinfo_1);
	peer->cap.eht_bf.snd_dim_bw320 = GET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_320M(phy_cap->phy_capinfo_1);
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_NG16_SU_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_BFEE_NG16_SU_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_NG16_MU_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_BFEE_NG16_MU_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_CODEBOOK_4_2_SU_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_BFEE_CODEBOOK_4_2_SU_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_CODEBOOK_7_5_SU_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_BFEE_CODEBOOK_7_5_SU_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_TRIGED_SU_BF_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_TRIGED_SU_BF_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_TRIGED_MU_BF_PARTIAL_BW_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK;
	if (phy_cap->phy_capinfo_1 & DOT11BE_PHY_CAP_TRIGED_CQI_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_TRIGED_CQI_FEEDBACK;

	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_PARTIAL_BW_DL_MU_MIMO)
		peer->cap.eht_phy_cap |= EHT_PARTIAL_BW_DL_MU_MIMO;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_PSR_BASED_SR)
		peer->cap.eht_phy_cap |= EHT_PSR_BASED_SR;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_POWER_BOOST_FACTOR)
		peer->cap.eht_phy_cap |= EHT_POWER_BOOST_FACTOR;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI)
		peer->cap.eht_gi |= EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI;
	peer->cap.eht_bf.bfee_max_nc = GET_DOT11BE_PHY_CAP_MAX_NC(phy_cap->phy_capinfo_2);

	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_NON_TRIGED_CQI_FEEDBACK)
		peer->cap.eht_bf.bf_cap |= EHT_NON_TRIGED_CQI_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_TX_1024QAM_4096QAM_LE_242_TONE_RU)
		peer->cap.eht_phy_cap |= EHT_TX_1024QAM_4096QAM_LE_242_TONE_RU;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_RX_1024QAM_4096QAM_LE_242_TONE_RU)
		peer->cap.eht_phy_cap |= EHT_RX_1024QAM_4096QAM_LE_242_TONE_RU;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_PPE_THRLD_PRESENT)
		peer->cap.eht_phy_cap |= EHT_PPE_THRLD_PRESENT;
	peer->cap.eht_common_nominal_padd = GET_DOT11BE_PHY_CAP_COMMON_NOMINAL_PKT_PAD(phy_cap->phy_capinfo_2);
	peer->cap.eht_bf.max_eht_ltf_num = GET_DOT11BE_PHY_CAP_MAX_EHT_LTF_NUM(phy_cap->phy_capinfo_2);
	peer->cap.eht_mcs15_mru_set = GET_DOT11BE_PHY_CAP_MCS_15(phy_cap->phy_capinfo_2);
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_EHT_DUP_6G)
		peer->cap.eht_phy_cap |= EHT_DUP_6G;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_20M_RX_NDP_W_WIDER_BW)
		peer->cap.eht_phy_cap |= EHT_20M_RX_NDP_W_WIDER_BW;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_LE_EQ_BW80)
		peer->cap.eht_phy_cap |= EHT_NON_OFMDA_UL_MU_MIMO_LE_EQ_80M;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_BW160)
		peer->cap.eht_phy_cap |= EHT_NON_OFMDA_UL_MU_MIMO_160M;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_BW320)
		peer->cap.eht_phy_cap |= EHT_NON_OFMDA_UL_MU_MIMO_320M;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_MU_BFER_LE_EQ_80M)
		peer->cap.eht_phy_cap |= EHT_MU_BF_LE_EQ_80M;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_MU_BFER_160M)
		peer->cap.eht_phy_cap |= EHT_MU_BF_160M;
	if (phy_cap->phy_capinfo_2 & DOT11BE_PHY_CAP_MU_BFER_320M)
		peer->cap.eht_phy_cap |= EHT_MU_BF_320M;
}

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*
 * Build up EHT IEs for Beacon
 */
UINT32 eht_add_beacon_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/* Basic variant Multi-Link element */
	local_fbuf = eht_build_multi_link_ie(
			wdev, local_fbuf, NULL, NULL, BMGR_QUERY_ML_IE_BCN, BMGR_MAX_MLD_STA_CNT);
	/* Reconfiguration variant Multi-Link element */
	local_fbuf = eht_build_reconfig_multi_link_ie(
		wdev, local_fbuf, (UINT16)(local_fbuf - f_buf), BMGR_QUERY_ML_IE_BCN);
	/* EHT Cap. */
	local_fbuf = eht_build_cap_ie(wdev, local_fbuf);
	/* EHT Op. */
	local_fbuf = eht_build_op_ie(wdev, local_fbuf);

	/* Multi link traffic */
	local_fbuf = eht_build_multi_link_traffic_ie(wdev, f_buf/*bcn hdr*/, local_fbuf/*bcn last*/);

	/* TODO, AT2LM: at2lm offset is the same as MLTI IE offset */
	/* EHT AT2LM */
	local_fbuf = eht_build_at2lm_ie(wdev, f_buf, local_fbuf, AT2LM_BCN);

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Build up EHT IEs for Probe Response
 */
UINT32 eht_add_probe_rsp_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len,
	struct common_ies *cmm_ies)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/*
	 * Move the eht_build_multi_link_ie() outside this function, because of 802.11be
	 * inheritance rule of Per-STA Profile. In this case, we need to build EHT Cap /
	 * EHT Op (or all other IEs after ML IE) first, so we can perform inheritance rule
	 * correctly.
	 */
	/* if (cmm_ies) {
		// Probe Req variant Multi-Link element
		local_fbuf = eht_build_multi_link_ie(
				wdev, local_fbuf,
				HAS_EHT_MLD_EXIST(cmm_ies->ie_exists) ? cmm_ies->ml_ie : NULL,
				BMGR_QUERY_ML_IE_PROBE_RSP, BMGR_MAX_MLD_STA_CNT);
	}*/

	/* EHT Cap. */
	local_fbuf = eht_build_cap_ie(wdev, local_fbuf);
	/* EHT Op. */
	local_fbuf = eht_build_op_ie(wdev, local_fbuf);
	/* EHT AT2LM */
	local_fbuf = eht_build_at2lm_ie(wdev, f_buf, local_fbuf, AT2LM_PROBE_RSP);


	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Build up EHT(MLO) IEs for Auth Request
 */
uint32_t eht_add_auth_req_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len)
{
	uint8_t *local_fbuf = f_buf + f_len;
	uint32_t offset = 0;

	/* Basic variant Multi-Link element */
	local_fbuf = eht_build_multi_link_ie(wdev,
		local_fbuf, NULL, NULL, BMGR_QUERY_ML_IE_AUTH_REQ, BMGR_MAX_MLD_STA_CNT);

	offset = (uint32_t)(local_fbuf - f_buf - f_len);

	return offset;
}


/*
 * Build up EHT(MLO) IEs for Auth Response
 */
UINT32 eht_add_auth_rsp_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len,
	struct common_ies *cmm_ies)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/* Basic variant Multi-Link element */
	if (cmm_ies) {
		local_fbuf = eht_build_multi_link_ie(
						wdev, local_fbuf,
						HAS_EHT_MLD_EXIST(cmm_ies->ie_exists) ? cmm_ies->ml_ie : NULL,
						NULL, BMGR_QUERY_ML_IE_AUTH_RSP, BMGR_MAX_MLD_STA_CNT);
	}

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Build up EHT IEs for Assoc Response
 */
UINT32 eht_add_assoc_rsp_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len,
	struct common_ies *cmm_ies)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/*
	 * Move the eht_build_multi_link_ie() outside this function, because of 802.11be
	 * inheritance rule of Per-STA Profile. In this case, we need to build EHT Cap /
	 * EHT Op (or all other IEs after ML IE) first, so we can perform inheritance rule
	 * correctly.
	 */
	/* struct mtk_mac_bss *mac_bss;
	mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
	// Basic variant Multi-Link element
	if (HAS_EHT_MLD_EXIST(cmm_ies->ie_exists) &&
		(mac_bss->mld_group_idx != 0) &&
		(mac_bss->mld_group_idx != MLD_GROUP_NONE)) {
		local_fbuf = eht_build_multi_link_ie(
						wdev, local_fbuf,
						HAS_EHT_MLD_EXIST(cmm_ies->ie_exists) ? cmm_ies->ml_ie : NULL,
						BMGR_QUERY_ML_IE_ASSOC_RSP);
	}*/
	/* EHT Cap. */
	local_fbuf = eht_build_cap_ie(wdev, local_fbuf);
	/* EHT Op. */
	local_fbuf = eht_build_op_ie(wdev, local_fbuf);
	/* Append EHT AT2LM that is advertised */
	local_fbuf = eht_build_at2lm_ie(wdev, f_buf, local_fbuf, AT2LM_ASSOC);

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Build up multi-link IEs for Request
 */
UINT32 eht_add_assoc_req_ml_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;
	struct mld_dev *mld = &mld_device;

	/* Basic variant Multi-Link element */
	if (!IS_APCLI_DISABLE_MLO(wdev)) {
		local_fbuf = eht_build_multi_link_ie(wdev,
			local_fbuf, NULL, NULL, BMGR_QUERY_ML_IE_ASSOC_REQ, BMGR_MAX_MLD_STA_CNT);
		if (!mld->assoc_req_ml_ie.buf) {
			os_alloc_mem(NULL, (u8 **)&mld->assoc_req_ml_ie.buf, MAX_LEN_OF_MLIE);
			if (!mld->assoc_req_ml_ie.buf)
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
				"Error: fail to allocate memory for probe req ie\n");
		}
		if (mld->assoc_req_ml_ie.buf) {
			NdisZeroMemory(mld->assoc_req_ml_ie.buf, MAX_LEN_OF_MLIE);
			mld->assoc_req_ml_ie.len = local_fbuf - f_buf - f_len;
			NdisMoveMemory(mld->assoc_req_ml_ie.buf, f_buf + f_len, mld->assoc_req_ml_ie.len);
		}
	}

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

UINT32 eht_add_assoc_req_cap_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/* EHT Cap. */
	local_fbuf = eht_build_cap_ie(wdev, local_fbuf);
	/* EHT Op. */
	local_fbuf = eht_build_tid_to_link_map_ie(wdev, local_fbuf);

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Query probe response frame for a link
 * Note: the buffer be allocated in ap_probe_response_xmit() and return address and length,
 *      if done using the buffer, remember to free/release it!
 */
void eht_mlo_query_link_probe_rsp_frame(
	IN struct wifi_dev *wdev,
	OUT struct buffer_wrapper *rsp_buf)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MLME_QUEUE_ELEM *dummy_Elem = NULL;
	PEER_PROBE_REQ_PARAM *dummy_req_param = NULL;

	/* prepare input for ap_probe_rsponse_xmit() */
	os_alloc_mem(pAd, (UCHAR **)&dummy_Elem, sizeof(MLME_QUEUE_ELEM));
	if (!dummy_Elem) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Error: fail to allocate memory for response\n");
		goto err;
	}

	NdisZeroMemory(dummy_Elem, sizeof(MLME_QUEUE_ELEM));
	dummy_Elem->Others = rsp_buf;

	os_alloc_mem(pAd, (UCHAR **)&dummy_req_param, sizeof(PEER_PROBE_REQ_PARAM));
	if (!dummy_req_param) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"Error: fail to allocate memory for response\n");
		goto err;
	}

	NdisZeroMemory(dummy_req_param, sizeof(PEER_PROBE_REQ_PARAM));
	COPY_MAC_ADDR(dummy_req_param->Addr1, wdev->if_addr);

	/* call ap_probe_response_xmit() to obtain probe response frame */
	ap_probe_response_xmit(pAd, wdev, dummy_req_param, dummy_Elem);

err:
	if (dummy_Elem)
		os_free_mem(dummy_Elem);

	if (dummy_req_param)
		os_free_mem(dummy_req_param);

}

/*
 * Filter out the IEs (listed in id_buf & id_ext_buf) in source
 */
void eht_remove_ie_by_id_list(
	IN struct buffer_wrapper *source,
	OUT struct buffer_wrapper *target,
	IN struct buffer_wrapper *id_buf,
	IN struct buffer_wrapper *id_ext_buf)
{
	UINT8 i;
	UINT8 *extension_id;
	UINT16 ie_len;
	BOOLEAN removed, rm_frag = FALSE;
	struct _EID_STRUCT *source_eid = NULL;

	if (id_buf->buf_len == 0 && id_ext_buf->buf_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
			"id_buf & id_ext_buf are both 0, return.\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
	"[Start] Remove IE by id list, source len=%ld, target len=%ld, #id=%ld, #id_ext=%ld\n",
	source->buf_len, target->buf_len, id_buf->buf_len, id_ext_buf->buf_len);

	source_eid = (struct _EID_STRUCT *)source->buf;
	while ((UINT8 *)source_eid < (source->buf + source->buf_len)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				 "Source buffer, eid(%d), len(TLV:%d, body:%d)\n",
				 source_eid->Eid, source_eid->Len + 2, source_eid->Len);

		removed = FALSE;

		if (source_eid->Eid == IE_FRAGMENT) {
			if (rm_frag)
				removed = TRUE; /* remove Fragment IE */
		} else
			rm_frag = FALSE;

		if (source_eid->Eid != IE_WLAN_EXTENSION) {
			/* check ID list here */
			for (i = 0; i < id_buf->buf_len; i++) {
				if (source_eid->Eid == id_buf->buf[i]) {
					removed = TRUE;
					if (source_eid->Len == MAX_LEN_OF_IE_DATA)
						rm_frag = TRUE;
					break;
				}
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				 "%s eid(%d)\n",
				 removed ? "remove" : "keep", source_eid->Eid);
		} else {
			/* check ID_ext list here */
			extension_id = (UINT8 *)source_eid + 2;
			for (i = 0; i < id_ext_buf->buf_len; i++) {
				if ((*extension_id) == id_ext_buf->buf[i]) {
					removed = TRUE;
					if (source_eid->Len == MAX_LEN_OF_IE_DATA)
						rm_frag = TRUE;
					break;
				}
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				 "%s eid_ext(%d)\n",
				 removed ? "remove" : "keep", *extension_id);
		}

		if (!removed) {
			/* keep IE */
			ie_len = source_eid->Len + 2;
			NdisMoveMemory(target->buf + target->buf_len, source_eid, ie_len);
			target->buf_len += ie_len;
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
				 "Target buffer, current len(%ld)\n",
				 target->buf_len);

		source_eid = (PEID_STRUCT)((u8 *)source_eid + 2 + source_eid->Len);
	}


	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
	"[End] Remove IE by id list, source len=%ld, target len=%ld, #id=%ld, #id_ext=%ld\n",
	source->buf_len, target->buf_len, id_buf->buf_len, id_ext_buf->buf_len);
}

static BOOLEAN eht_ap_mlo_is_valid_sta_profile_ie(UINT8 id, UINT8 id_ext)
{
	BOOLEAN valid;

	switch (id) {
	case IE_SSID:
	case IE_TIM:
	case IE_BSS_MAX_IDLE:
	case IE_NEIGHBOR_REPORT:
	case IE_RNR:
	case IE_MULTIPLE_BSSID:
	case IE_MULTIPLE_BSSID_IDX:
#ifdef DOT11R_FT_SUPPORT
	case IE_FT_MDIE:
	case IE_FT_FTIE:
#endif /* DOT11R_FT_SUPPORT */
	/* temporarily skip the vendor IE */
	case IE_VENDOR_SPECIFIC:
		valid = FALSE;
		break;

	case IE_WLAN_EXTENSION:
		switch (id_ext) {
		case EID_EXT_MULTI_BSSID_CFG:
		case EID_EXT_EHT_MULTI_LINK:
		case EID_EXT_EHT_TID2LNK_MAP:
			valid = FALSE;
			break;
		default:
			valid = TRUE;
			break;
		}
		break;

	default:
		valid = TRUE;
		break;
	}

	return valid;
}

/*
 * Form STA Profile in Per-STA Profile sub-element in ML IE
 */
NDIS_STATUS eht_mlo_sta_profile_inheritance_rule(
	IN struct buffer_wrapper *repting_frame_ie,
	IN struct buffer_wrapper *repted_frame_ie,
	OUT struct buffer_wrapper *result)
{
#define ALL_IE_NUM 512
#define IE_NUM_PER_GROUP 64
#define NUM_IE_GROUP (ALL_IE_NUM / IE_NUM_PER_GROUP)

#define ALLOC_MEM_NUM_PER_TIME 32
#define NUM_MEM_GROUP (ALL_IE_NUM / ALLOC_MEM_NUM_PER_TIME)

/* These two macros based on IE_NUM_PER_GROUP */
#define get_pos(x) (x >> 6)			/* divided by 2^6 = 64 */
#define get_bit(x) (1ULL << (x & 63))	/* bitmask (2^n)-1, n=6 */

	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	UINT8 i, j;
	UINT16 current_id, hash;
	UINT8 *extension_id;
	UINT8 *pos;

	/* indicate the appearance of IEs in the reported frame:
	 * 64 bits * NUM_IE_GROUP = #all IEs = 512 */
	UINT64 repted_bitset[NUM_IE_GROUP] = {0};

	/* indicate the appearance of IEs and its virtual address in the reporting frame:
	 * 64 bits * NUM_IE_GROUP = #all IEs = 512 */
	struct MEM_NODE *repting_addr[NUM_IE_GROUP] = {0};	/* a hash table */
	/* management of dynamically allocated memory */
	UINT8 *MEM_MGMT[NUM_MEM_GROUP] = {0}, MEM_IDX = 0, MEM_AVAILABLE = 0;
	struct MEM_NODE *CUR_NODE = NULL, *TEMP_NODE;

	struct _EID_STRUCT *repting_eid = NULL;
	struct _EID_STRUCT *repted_eid = NULL;

	UINT8 non_inherit_ie_list[100] = {0}, non_inherit_ie_ext_list[100] = {0};
	UINT8 non_inherit_ie_list_len = 0, non_inherit_ie_ext_list_len = 0;
	ULONG tmp_len = 0;


	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
		"\tStart figuring out inheritance rule (reported len=%ld)\n",
		repted_frame_ie->buf_len);

	/* Step 1. trace the reported frame to set the bitset */
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
		"\t\t======== 1. Trace the repoted link IEs ========\n");
	repted_eid = (struct _EID_STRUCT *)repted_frame_ie->buf;
	while ((UINT8 *)repted_eid < (repted_frame_ie->buf + repted_frame_ie->buf_len)) {
		extension_id = (UINT8 *)repted_eid + 2;
		if (eht_ap_mlo_is_valid_sta_profile_ie(repted_eid->Eid, *extension_id)) {
			current_id = (repted_eid->Eid == IE_WLAN_EXTENSION) ?
				(*extension_id) + 256 : repted_eid->Eid;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
				"found %s[%d] in non-setup link rsp, set bitset\n",
				current_id >= 256 ? "ID_EXT" : "ID", current_id % 256);

			repted_bitset[get_pos(current_id)] |= get_bit(current_id);
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
				"found ID[%d]_LEN[%d] (if ID=255, ID_EXT[%d]) in non-setup link rsp, skip!\n",
				repted_eid->Eid, repted_eid->Len, *extension_id);

		repted_eid = (PEID_STRUCT)((u8 *)repted_eid + 2 + repted_eid->Len);
	}

	/* Step 2. trace the reporting frame to set the bitset */
	/*   + store address of IEs in reporting frame
	 *   + determine the IEs that are non-inherite by reported frame
	 */
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
		"\t\t ======== 2. Trace the setup link IEs ========\n");
	repting_eid = (struct _EID_STRUCT *)repting_frame_ie->buf;
	while ((UINT8 *)repting_eid < (repting_frame_ie->buf + repting_frame_ie->buf_len)) {

		/* allocate more linked-list nodes */
		if (MEM_AVAILABLE == 0) {
			if (MEM_IDX >= NUM_MEM_GROUP) {
				/* vendor ie or fragmentation? */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
					"Error: out of designed memory, the number of IE is larger than %d\n",
					ALL_IE_NUM);
				status = NDIS_STATUS_FAILURE;
				goto inheritance_rule_error;

			}

			os_alloc_mem(NULL, (UCHAR **)&MEM_MGMT[MEM_IDX],
				sizeof(struct MEM_NODE) * ALLOC_MEM_NUM_PER_TIME);
			if (!MEM_MGMT[MEM_IDX]) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
					"Error: Can not allocate memory\n");
				status = NDIS_STATUS_FAILURE;
				goto inheritance_rule_error;
			}

			MEM_AVAILABLE = ALLOC_MEM_NUM_PER_TIME;
			CUR_NODE = (struct MEM_NODE *)MEM_MGMT[MEM_IDX];
			MEM_IDX++;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
				"Allocate %d new node for setup link tracing, total allocated: %d\n",
				ALLOC_MEM_NUM_PER_TIME, MEM_IDX * ALLOC_MEM_NUM_PER_TIME);
		}

		extension_id = (UINT8 *)repting_eid + 2;
		if (eht_ap_mlo_is_valid_sta_profile_ie(repting_eid->Eid, *extension_id)) {
			current_id = (repting_eid->Eid == IE_WLAN_EXTENSION) ?
				(*extension_id) + 256 : repting_eid->Eid;

			if (!(repted_bitset[get_pos(current_id)] & get_bit(current_id))) {
				/* IE appears in reporting frame but not in reported frame
				 * -> non-inheritance */
				if (repting_eid->Eid == IE_WLAN_EXTENSION) {
					non_inherit_ie_ext_list[non_inherit_ie_ext_list_len] = current_id % 256;
					non_inherit_ie_ext_list_len++;
				} else {
					non_inherit_ie_list[non_inherit_ie_list_len] = current_id % 256;
					non_inherit_ie_list_len++;
				}

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
					"\tnon-inheritance %s[%d]\n",
					current_id >= 256 ? "ID_EXT" : "ID", current_id % 256);
			} else {
				/* IE appears in both reporting frame and reported frame */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
					"found %s[%d] in setup link rsp, set addr in node [available node: %d]\n",
					current_id >= 256 ? "ID_EXT" : "ID", current_id % 256, MEM_AVAILABLE);

				/* use a memory node */
				CUR_NODE->id = current_id; // include eid & eid_ext
				CUR_NODE->addr = (UINT8 *)repting_eid;
				CUR_NODE->next = NULL;

				/* add a memory node to hash table */
				hash = current_id % NUM_IE_GROUP;
				if (!repting_addr[hash]) {
					repting_addr[hash] = CUR_NODE;
				} else {
					TEMP_NODE = repting_addr[hash];

					while (TEMP_NODE->next != NULL)
						TEMP_NODE = TEMP_NODE->next;

					TEMP_NODE->next = CUR_NODE;
				}

				MEM_AVAILABLE -= 1;

				/* jump to next available MEM_NODE */
				CUR_NODE += 1;
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
				"found ID[%d]_LEN[%d] (if ID=255, ID_EXT[%d]) in non-setup link rsp, skip!\n",
				repting_eid->Eid, repting_eid->Len, *extension_id);

		repting_eid = (PEID_STRUCT)((u8 *)repting_eid + 2 + repting_eid->Len);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
		"After trace reporting frame => MEM_IDX:%d, MEM_AVAILABLE:%d\n", MEM_IDX, MEM_AVAILABLE);

	/* Step 3. trace the reported frame to determine
	 *   to inherite from reporting frame or put to STA Profile
	 */
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
		"\t\t======== 3. Perform inheritance rule! ========\n");
	repted_eid = (struct _EID_STRUCT *)repted_frame_ie->buf;
	while ((UINT8 *)repted_eid < (repted_frame_ie->buf + repted_frame_ie->buf_len)) {
		extension_id = (UINT8 *)repted_eid + 2;
		if (eht_ap_mlo_is_valid_sta_profile_ie(repted_eid->Eid, *extension_id)) {
			current_id = (repted_eid->Eid == IE_WLAN_EXTENSION) ?
				(*extension_id) + 256 : repted_eid->Eid;

			/* head of linked-list */
			hash = current_id % NUM_IE_GROUP;
			TEMP_NODE = repting_addr[hash];
			CUR_NODE = NULL;

			/* Is IE in reported frame in reporting frame? */
			while (TEMP_NODE) {
				if (TEMP_NODE->id == current_id) {
					CUR_NODE = TEMP_NODE;
					break;

				} else
					TEMP_NODE = TEMP_NODE->next;
			}

			if (CUR_NODE) {
				/* IE appears in both reporting & reported frame */
				repting_eid = (struct _EID_STRUCT *)CUR_NODE->addr;
				if ((repted_eid->Len != repting_eid->Len) ||
						memcmp(repted_eid, repting_eid, repted_eid->Len + 2)) {
					/* non-inheriteance case => move IE to STA Profile */
					NdisMoveMemory(result->buf + result->buf_len,
						repted_eid, repted_eid->Len + 2);
					result->buf_len += repted_eid->Len + 2;

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
						"IE %s[%d] for non-setup link is different from setup link frame body!\n",
						current_id >= 256 ? "ID_EXT" : "ID", current_id % 256);
				} else {
					/* inheritance case => skip */
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
						"inherit IE %s[%d] from setup link frame body!\n",
						current_id >= 256 ? "ID_EXT" : "ID", current_id % 256);
				}
			} else {
				/* IE appears in reported frame but not in reporting frame */
				NdisMoveMemory(result->buf + result->buf_len, repted_eid, repted_eid->Len + 2);
				result->buf_len += repted_eid->Len + 2;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
					"found link-specific IE %s[%d]!\n",
					current_id >= 256 ? "ID_EXT" : "ID", current_id % 256);
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
				"found ID[%d]_LEN[%d] (if ID=255, ID_EXT[%d]) in non-setup link rsp, skip!\n",
				repted_eid->Eid, repted_eid->Len, *extension_id);

		repted_eid = (PEID_STRUCT)((u8 *)repted_eid + 2 + repted_eid->Len);
	}

	/* Step 4. append non-inheritance element */
	if (non_inherit_ie_list_len || non_inherit_ie_ext_list_len) {
		/* bubble sort for IDs */
		for (i = 0; i < non_inherit_ie_list_len - 1; i++) {
			for (j = 0; j < non_inherit_ie_list_len - i - 1; j++) {
				if (non_inherit_ie_list[j] > non_inherit_ie_list[j + 1])
					NUM_SWAP(non_inherit_ie_list[j], non_inherit_ie_list[j + 1]);
			}
		}

		/* bubble sort for ID_EXTs */
		for (i = 0; i < non_inherit_ie_ext_list_len - 1; i++) {
			for (j = 0; j < non_inherit_ie_ext_list_len - i - 1; j++) {
				if (non_inherit_ie_ext_list[j] > non_inherit_ie_ext_list[j + 1])
					NUM_SWAP(non_inherit_ie_ext_list[j], non_inherit_ie_ext_list[j + 1]);
			}
		}

		pos = result->buf + result->buf_len;
		repted_eid = (struct _EID_STRUCT *)(result->buf + result->buf_len);
		repted_eid->Eid = IE_WLAN_EXTENSION;
		repted_eid->Octet[0] = IE_EXTENSION_ID_NON_INHERITANCE;
		pos += sizeof(struct _EID_STRUCT);

		MakeOutgoingFrame(pos, &tmp_len,
						  1,								&non_inherit_ie_list_len,
						  non_inherit_ie_list_len,		non_inherit_ie_list,
						  1,								&non_inherit_ie_ext_list_len,
						  non_inherit_ie_ext_list_len,	non_inherit_ie_ext_list,
						  END_OF_ARGS);
		pos += tmp_len;
		repted_eid->Len = pos - (UINT8 *)repted_eid - 2;

		// add non-inheritance IE
		result->buf_len += repted_eid->Len + 2;
	}

inheritance_rule_error:
	for (i = 0; i < MEM_IDX; i++) {
		if (MEM_MGMT[i])
			os_free_mem(MEM_MGMT[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
		"\tEnd of inheritance rule (result len=%ld)\n", result->buf_len);

	return status;
}

/*
 * Fragment the information element (SPEC 10.28.11)
 *
 * This API inserts Fragment element after fragmented element, e.g. ML IE in 11be;
 * therefore, the two input arguments will be modified.
 */
UINT8 *fragment_information_element(
	IN OUT UINT8 *ie_buf, IN OUT UINT16 *buf_len)
{
	UINT8 *pos;
	UINT16 todo_len = 0, result_len = 0;
	struct _EID_STRUCT *frag_eid;

	if (*buf_len <= (LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Notice: the length of buffer is not greater than %d (T+L+V), do not fragment\n",
			(LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA));
		return (ie_buf + *buf_len);
	}

	frag_eid = (struct _EID_STRUCT *)ie_buf;
	if (frag_eid->Eid == IE_FRAGMENT) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error: the Fragment IE shall not be fragmented\n");
		return (ie_buf + *buf_len);
	}

	/* keep leading element */
	frag_eid->Len = MAX_LEN_OF_IE_DATA;
	todo_len = *buf_len - (LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA);
	result_len = (LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
		"[LOG leading] buf_len=%d, eid->Len=%d, todo_len=%d, result_len=%d\n",
		*buf_len, frag_eid->Len, todo_len, result_len);

	/* insert Fragment elements */
	while (todo_len > 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"[LOG frag 1] buf_len=%d, todo_len=%d, result_len=%d\n",
			*buf_len, todo_len, result_len);
		pos = (UINT8 *)ie_buf + result_len;

		/* shift 2 bytes for accomdating tag and length fields of next Fragment IE */
		NdisMoveMemory(pos + LEN_OF_IE_TAG_LENG, pos, todo_len);
		result_len += LEN_OF_IE_TAG_LENG;

		frag_eid = (struct _EID_STRUCT *)pos;
		frag_eid->Eid = IE_FRAGMENT;
		frag_eid->Len = (todo_len > MAX_LEN_OF_IE_DATA) ? MAX_LEN_OF_IE_DATA : todo_len;
		result_len += frag_eid->Len;
		todo_len -= frag_eid->Len;

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"[LOG frag 2] buf_len=%d, eid->Len=%d, todo_len=%d, result_len=%d\n",
			*buf_len, frag_eid->Len, todo_len, result_len);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
		"[LOG end] buf_len=%d, todo_len=%d, result_len=%d\n",
		*buf_len, todo_len, result_len);
	/* update new length of buffer (leading IE + Fragment IEs) */
	*buf_len = result_len;

	pos = ie_buf + result_len;
	return pos;
}

/*
 * Defragment the information element (SPEC 10.28.12)
 *
 * This API removes Fragment element after fragmented element, e.g. ML IE in 11be.
 *
 * NOTE: After defragmentation, the second octet of "ie_buf", i.e. the length of IE,
 *	is incorrect, because one octet only can represent 0~255. You shall based on
 *	"defrag_len" to do any next action.
 */
NDIS_STATUS defragment_information_element(
	IN UINT8 *ie_buf, IN UINT16 buf_len, OUT UINT8 *defrag_buf, OUT UINT16 *defrag_len)
{
	UINT8 *pos;
	UINT8 tag, length;
	UINT16 done_len = 0, result_len = 0;
	struct _EID_STRUCT *frag_eid;

	if (buf_len < (LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Notice: the length of buffer is not greater than %d (T + L + V)\n",
			(LEN_OF_IE_TAG_LENG + MAX_LEN_OF_IE_DATA));
		return NDIS_STATUS_FAILURE;
	}

	frag_eid = (struct _EID_STRUCT *)ie_buf;
	if (frag_eid->Eid == IE_FRAGMENT) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error: the Fragment IE shall not be fragmented\n");
		return NDIS_STATUS_FAILURE;
	}

	/* leading element */
	result_len = done_len = LEN_OF_IE_TAG_LENG + frag_eid->Len;
	NdisMoveMemory(defrag_buf, ie_buf, result_len);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
		"buf_len=%d, eid->Len=%d, done_len=%d, result_len=%d\n",
		buf_len, frag_eid->Len, done_len, result_len);

	/* concatenate data filed of Fragment elements to leading element */
	while (done_len + LEN_OF_IE_TAG_LENG < buf_len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"[frag 1] buf_len=%d, done_len=%d, result_len=%d\n",
			buf_len, done_len, result_len);

		pos = (UINT8 *)ie_buf + done_len;
		frag_eid = (struct _EID_STRUCT *)pos;
		tag = frag_eid->Eid;
		length = frag_eid->Len;

		if (tag == IE_FRAGMENT && length) {
			if ((result_len + length) > MAX_LEN_OF_MLIE) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"Error: Len(=%d) > MAX_LEN_OF_MLIE(=%d)\n",
					(result_len + length), MAX_LEN_OF_MLIE);
				return NDIS_STATUS_FAILURE;
			}
			if ((done_len + LEN_OF_IE_TAG_LENG + length) > buf_len) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_DEBUG,
					"Error: Len(=%d) > buf_len(=%d)\n",
					(done_len + LEN_OF_IE_TAG_LENG + length), buf_len);
				return NDIS_STATUS_FAILURE;
			}
			/* shift 2 bytes for removal of tag and length fields of Fragment IE */
			NdisMoveMemory(defrag_buf + result_len,	pos + LEN_OF_IE_TAG_LENG, length);
			/* add length of fragmented portion (Data field) */
			result_len += length;

			/* goto next Fragment element */
			done_len += (LEN_OF_IE_TAG_LENG + length);
		} else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
				"Error: the next element is not Fragment element or len(%d) <= 0\n",
				length);
			break;
		}
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"[frag 2] buf_len=%d, eid->Len=%d, done_len=%d, result_len=%d, tag=%d, length=%d\n",
			buf_len, frag_eid->Len, done_len, result_len, tag, length);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
		"buf_len=%d, done_len=%d, result_len=%d\n",
		buf_len, done_len, result_len);
	/* update new length of buffer (only reconstructed leading IE) */
	if (defrag_len)
		*defrag_len = result_len;

	return NDIS_STATUS_SUCCESS;
}

/*
 * Get EHT IEs
 */
int eht_get_ies(
	UINT8 *ie_head, struct common_ies *cmm_ies, struct frag_ie_info *frag_info)
{
	struct _EID_STRUCT *ie_item = (struct _EID_STRUCT *)ie_head;
	UINT8 eid_ext;
	UINT8 ie_len;
	UINT8 *ie_ctx = ie_head + sizeof(struct _EID_STRUCT);

	if (ie_item) {
		if (ie_item->Len)
			ie_len = ie_item->Len - 1;
		else
			return -1;
		eid_ext = ie_item->Octet[0];
	} else
		return -1;

	switch (eid_ext) {
	case EID_EXT_EHT_MULTI_LINK:
		if (strlen(cmm_ies->ml_ie) && HAS_EHT_MLD_EXIST(cmm_ies->ie_exists)) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
				"11v mlo: find second ml ie!\n");

			/*11v_mlo case: peer ml probe rsp may take two ml ie
			 *the one is for requested mld info w/ per-sta profile, from non tx bss
			 *another one is for mld info w/o per-sta profile, from transmit bss*/;

			if (eht_get_multi_link_ie(ie_head, cmm_ies->ml_ie_2nd, &cmm_ies->ml_frag_info_2nd,
					&cmm_ies->ie_exists, frag_info) < 0) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
							"failed to get 2nd ML IE!\n");
				return -1;
			}

			return 0;
		}
		if (eht_get_multi_link_ie(ie_head, cmm_ies->ml_ie, &cmm_ies->ml_frag_info,
				&cmm_ies->ie_exists, frag_info) < 0) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
						"failed to get ML IE!\n");
			return -1;
		}
		break;
	case EID_EXT_EHT_CAPS:
		if (eht_get_caps_ie(ie_ctx, ie_len, cmm_ies) < 0) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
						"failed to get ETH CAPS IE!\n");
			return -1;
		}
		break;
	case EID_EXT_EHT_OP:
		if (eht_get_op_ie(ie_ctx, ie_len, cmm_ies) < 0) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
						"failed to get ETH OP IE\n");
			return -1;
		}
		break;
	case IE_EXTENSION_ID_NON_INHERITANCE:
		cmm_ies->no_inherit_ie = ie_head;
		break;
	default:
		/*not a HE IEs*/
		break;
	}
	return 0;
}

/*
 * sta record EHT feature decision
 */
UINT32 eht_starec_feature_decision(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	UINT64 *feature)
{
	UINT64 features = 0;

	if (IS_EHT_STA(entry->cap.modes))
		features |= STA_REC_EHT_BASIC_FEATURE;

	if (entry->mlo.mlo_en) {
		features |= STA_REC_MLD_SETUP_FEATURE;
		features |= STA_REC_EHT_MLD_FEATURE;
		features |= STA_REC_EHT_BASIC_FEATURE;
	}

	if (IS_EHT_6G_STA(entry->cap.modes))
		features |= STA_REC_BASIC_VHT_INFO_FEATURE;

	*feature |= features;

	return TRUE;
}

void eht_update_peer_dscb_info(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies)
{
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	UINT8  cur_is_dscb_enable = 0;
	UINT8  pre_is_dscb_enable = 0;
	UINT8  band_idx = 0;
	UINT8  omac_idx = 0;
	UINT8  ctrl = 0;
	UINT8  ccfs0 = 0;
	UINT8  ccfs1 = 0;
	UINT16 cur_dscb_bitmap = 0;
	UINT16 pre_dscb_bitmap = 0;
	BOOLEAN need_update = FALSE;

	if (!wdev || !peer || !cmm_ies)
		return;

	if (wdev->wdev_type != WDEV_TYPE_STA)
		return;

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	cur_dscb_bitmap = cmm_ies->dis_subch_bitmap;
	pre_dscb_bitmap = wlan_config_get_eht_dis_subch_bitmap(wdev);
	pre_is_dscb_enable = wlan_config_get_eht_dscb_enable(wdev);
	cur_is_dscb_enable = cmm_ies->dscb_enable;
	ctrl = cmm_ies->eht_op.op_info.control;
	ccfs0 = cmm_ies->eht_op.op_info.ccfs0;
	ccfs1 = cmm_ies->eht_op.op_info.ccfs1;
	omac_idx = HcGetOmacIdx(pAd, wdev);
	band_idx = HcGetBandByWdev(wdev);

	if (!pAd || !pStaCfg)
		return;
	/* Apcli is connected to rootAP */
	if (!(IS_ENTRY_PEER_AP(peer)) || (pStaCfg->ApcliInfStat.Valid == FALSE))
		return;

	if (cur_is_dscb_enable != pre_is_dscb_enable)
		need_update = TRUE;
	if (pre_dscb_bitmap != cur_dscb_bitmap)
		need_update = TRUE;

	if (need_update == TRUE) {
		HW_UPDATE_DSCBINFO(pAd, band_idx, omac_idx, cur_is_dscb_enable, cur_dscb_bitmap, ctrl, ccfs0, ccfs1);
		wlan_config_set_eht_dis_subch_bitmap(wdev, cur_dscb_bitmap);
		wlan_config_set_eht_dscb_enable(wdev, cur_is_dscb_enable);
	}
}

int eht_calculate_ch_band(struct common_ies *cmm_ies, UCHAR *ch_band)
{
	if (HAS_HE_OP_EXIST(cmm_ies->ie_exists) && (cmm_ies->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT))
		*ch_band = WIFI_CH_BAND_6G;
	else if (HAS_HT_OP_EXIST(cmm_ies->ie_exists) && (cmm_ies->ht_op.ControlChan != 0)) {
		if (cmm_ies->ht_op.ControlChan > 14)
			*ch_band = WIFI_CH_BAND_5G;
		else
			*ch_band = WIFI_CH_BAND_2G;
	} else {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"can't calcualte band info from he_ops ie or ht_op ie\n");
		return -1;
	}
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"the ch_band = %d\n", *ch_band);
	return 0;
}


int eht_calculate_channel(struct common_ies *cmm_ies, UCHAR *channel)
{
	if (HAS_HE_OP_EXIST(cmm_ies->ie_exists) && (cmm_ies->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT))
		*channel = cmm_ies->he6g_opinfo.prim_ch;
	else if (HAS_HT_OP_EXIST(cmm_ies->ie_exists) && (cmm_ies->ht_op.ControlChan != 0)) {
			*channel = cmm_ies->ht_op.ControlChan;
	} else {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"can't calcualte chanel from he_ops ie or ht_op ie\n");
		return -1;
	}
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"the channel = %d\n", *channel);
	return 0;
}

uint16_t eht_build_multi_link_conn_req(
	struct wifi_dev *wdev,
	uint8_t subtype,
	uint8_t *ml_ie,
	struct frag_ie_info *ml_frag_info,
	uint8_t *t2l_ie,
	uint8_t *link_addr,
	uint8_t *mld_addr,
	u8 ch_band)
{
	struct mld_conn_req mld_conn = {0};
	uint16_t mld_sta_idx = MLD_STA_NONE;
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		mld_conn.req_type = subtype;
		mld_conn.ml_ie = ml_ie;
		mld_conn.t2l_ie = t2l_ie;
		mld_conn.link_addr = link_addr;
		mld_conn.ml_frag_info = ml_frag_info;
		mld_conn.ch_band = ch_band;
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"caller:%pS  AP wdev(%d)\n", OS_TRACE, wdev->wdev_idx);
			ret = bss_mngr_mld_conn_req(wdev, &mld_conn);
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"STA wdev(%d)\n", wdev->wdev_idx);
			ret = sta_mld_conn_req(wdev, &mld_conn);
		}
	}

	if (ret == NDIS_STATUS_SUCCESS) {
		mld_sta_idx = mld_conn.mld_sta_idx;
		if (mld_addr)
			COPY_MAC_ADDR(mld_addr, mld_conn.mld_addr);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\tmld_sta_idx=%d\n", mld_sta_idx);

	return mld_sta_idx;
}
static VOID peer_eht_txrx_mcs_nss(struct _MAC_TABLE_ENTRY *peer,
		struct eht_support_mcs_nss *eht_mcs_nss)
{
	UINT8 i;
	struct wifi_dev *wdev = peer->wdev;
	UCHAR peer_max_nss = 0;
	UCHAR peer_max_mcs = 0;
	UCHAR peer_max_bw = BW_20;

	if (wdev->channel < 14 && WMODE_CAP_2G(wdev->PhyMode)) {
		/* 2G */
		if (peer->cap.ch_bw.he_ch_width & SUPP_40M_CW_IN_24G_BAND)
			peer_max_bw = BW_40;
	} else if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
		/* 5G & 6G*/
		if (peer->cap.ch_bw.he_ch_width & SUPP_40M_80M_CW_IN_5G_6G_BAND) {
			peer_max_bw = BW_80;
			if (peer->cap.ch_bw.he_ch_width & (SUPP_160M_CW_IN_5G_6G_BAND|SUPP_160M_8080M_CW_IN_5G_6G_BAND))
				peer_max_bw = BW_160;
		}
		if (WMODE_CAP_6G(wdev->PhyMode) && (peer_max_bw == BW_160) && (peer->cap.eht_phy_cap && EHT_320M_6G))
			peer_max_bw = BW_320;
	}

	if (peer_max_bw == BW_20) {
		/* Clear match nss when peer bw is 20 to fix potential issue at eht_get_caps_ie()*/
		if ((peer->EntryType & ENTRY_AP) == 0)
			NdisZeroMemory(&eht_mcs_nss->eht_txrx_mcs_nss, sizeof(eht_mcs_nss->eht_txrx_mcs_nss));
		else
			NdisZeroMemory(&eht_mcs_nss->eht_txrx_mcs_nss_20_only, sizeof(eht_mcs_nss->eht_txrx_mcs_nss_20_only));
	}


	for (i = 0; i < EHT_MCS_NSS_BW_NUM; i++) {
		if (i == EHT_NSS_MCS_20) {
			if (peer->EntryType == ENTRY_INFRA) {
				eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss;
				eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss;
				eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss;
			}
			peer->cap.rate.rx_nss_mcs[i].mcs0_7 = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs0_7 = (eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss
													& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			peer->cap.rate.rx_nss_mcs[i].mcs8_9 = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs8_9 = (eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss
														& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			peer->cap.rate.rx_nss_mcs[i].mcs10_11 = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs10_11 = (eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss
														& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			peer->cap.rate.rx_nss_mcs[i].mcs12_13 = eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs12_13 = (eht_mcs_nss->eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss
														& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			if (((peer->EntryType & ENTRY_AP) == 0) && (peer->MaxHTPhyMode.field.BW == BW_20)) {
				NdisMoveMemory((UINT8 *)&peer->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only, (UINT8 *)&eht_mcs_nss->eht_txrx_mcs_nss_20_only,
					sizeof(struct eht_txrx_mcs_nss_20));
			}

			if ((peer->cap.rate.rx_nss_mcs[i].mcs12_13 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs12_13 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs12_13;
				peer_max_mcs = BE_MCS_13;
			} else if ((peer->cap.rate.rx_nss_mcs[i].mcs10_11 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs10_11 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs10_11;
				peer_max_mcs = BE_MCS_11;
			} else if ((peer->cap.rate.rx_nss_mcs[i].mcs8_9 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs8_9 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs8_9;
				peer_max_mcs = BE_MCS_9;
			} else if ((peer->cap.rate.rx_nss_mcs[i].mcs0_7 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs0_7 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs0_7;
				peer_max_mcs = BE_MCS_7;
			} else
				;
		} else {
			peer->cap.rate.rx_nss_mcs[i].mcs0_9 = eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs0_9_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs0_9 = (eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs0_9_nss
													& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			peer->cap.rate.rx_nss_mcs[i].mcs10_11 = eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs10_11_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs10_11 = (eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs10_11_nss
													& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			peer->cap.rate.rx_nss_mcs[i].mcs12_13 = eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs12_13_nss & DOT11BE_MCS_NSS_MASK;
			peer->cap.rate.tx_nss_mcs[i].mcs12_13 = (eht_mcs_nss->eht_txrx_mcs_nss[i - 1].max_tx_rx_mcs12_13_nss
													& (DOT11BE_MCS_NSS_MASK << DOT11BE_MCS_NSS_SHIFT)) >> DOT11BE_MCS_NSS_SHIFT;
			NdisMoveMemory((UINT8 *)&peer->eht_support_mcs_nss.eht_txrx_mcs_nss[i-1], (UINT8 *)&eht_mcs_nss->eht_txrx_mcs_nss[i - 1],
				sizeof(struct eht_txrx_mcs_nss));
			if ((peer->cap.rate.rx_nss_mcs[i].mcs12_13 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs12_13 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs12_13;
				peer_max_mcs = BE_MCS_13;
			} else if ((peer->cap.rate.rx_nss_mcs[i].mcs10_11 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs10_11 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs10_11;
				peer_max_mcs = BE_MCS_11;
			} else if ((peer->cap.rate.rx_nss_mcs[i].mcs0_9 != 0) && (peer->cap.rate.rx_nss_mcs[i].mcs0_9 < DOT11BE_MAX_STREAM)) {
				peer_max_nss = peer->cap.rate.rx_nss_mcs[i].mcs0_9;
				peer_max_mcs = BE_MCS_9;
			} else
				;
		}
	}
	peer_max_nss = (peer_max_nss <= wlan_operate_get_tx_stream(wdev) ? peer_max_nss : wlan_operate_get_tx_stream(wdev));
	peer->MaxHTPhyMode.field.MCS = ((peer_max_nss-1) << 4);
	peer->MaxHTPhyMode.field.MCS &= 0x30;
	peer->MaxHTPhyMode.field.MCS |= peer_max_mcs;
}
void eht_update_peer_caps(
	struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies)
{
	struct wifi_dev *wdev = peer->wdev;

	/*mac caps*/
	eht_update_peer_mac_caps(peer, &cmm_ies->eht_caps.mac_cap);
	/*phy caps*/
	eht_update_peer_phy_caps(peer, &cmm_ies->eht_caps.phy_cap);
	/*txrx_mcs_nss_caps*/
	peer_eht_txrx_mcs_nss(peer, &cmm_ies->eht_support_mcs_nss);
	if (WMODE_CAP_BE_2G(wdev->PhyMode))
		peer->cap.modes |= EHT_24G_SUPPORT;
	if (WMODE_CAP_BE_5G(wdev->PhyMode))
		peer->cap.modes |= EHT_5G_SUPPORT;
	if (WMODE_CAP_BE_6G(wdev->PhyMode))
		peer->cap.modes |= EHT_6G_SUPPORT;
	peer->MaxHTPhyMode.field.MODE = MODE_EHT;
}

bool eht_operating_class_to_band(u8 operating_class,
		enum IEEE80211_BAND *band)
{
	switch (operating_class) {
	case 112:
	case 115 ... 127:
	case 128 ... 130:
		*band = IEEE80211_BAND_5G;
		return true;
	case 81:
	case 82:
	case 83:
	case 84:
		*band = IEEE80211_BAND_2G;
		return true;
	case 180:
		*band = IEEE80211_BAND_6G;
		return true;
	}

	return false;
}

/*
 * Store Basic Multi Link IE in Beacon
*/
void eht_store_basic_multi_link_info(
	UCHAR *basic_multi_link_ie,
	struct common_ies *cmm_ies)
{
	struct _EID_STRUCT *eid = NULL;

	eid = (struct _EID_STRUCT *)cmm_ies->ml_ie;

	if (eid != NULL) {
		hex_dump("Dump peer ML IE", (unsigned char *)eid, eid->Len + 2);
		os_move_mem(basic_multi_link_ie, eid, eid->Len + 2);
	}
}

/*
 * Build up EHT IEs for Probe Request
 */
UINT32 eht_add_probe_req_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;
	struct mld_dev *mld = &mld_device;

	if (!IS_APCLI_DISABLE_MLO(wdev)) {
		local_fbuf = eht_build_multi_link_ie(
			wdev, local_fbuf, NULL, NULL, BMGR_QUERY_ML_IE_PROBE_REQ, BMGR_MAX_MLD_STA_CNT);
		if (!mld->probe_req_ml_ie.buf) {
			os_alloc_mem(NULL, (u8 **)&mld->probe_req_ml_ie.buf, MAX_LEN_OF_MLIE);
			if (!mld->probe_req_ml_ie.buf)
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
				"Error: fail to allocate memory for assoc req ie\n");
		}
		if (mld->probe_req_ml_ie.buf) {
			NdisZeroMemory(mld->probe_req_ml_ie.buf, MAX_LEN_OF_MLIE);
			mld->probe_req_ml_ie.len = local_fbuf - f_buf;
			NdisCopyMemory(mld->probe_req_ml_ie.buf, f_buf, mld->probe_req_ml_ie.len);
		}
	}

	local_fbuf = eht_build_cap_ie(wdev, local_fbuf);

	offset = (UINT32)(local_fbuf - f_buf);
	return offset;
}

UCHAR rf_bw_2_eht_bw(UCHAR rf_bw)
{
	UCHAR eht_bw = EHT_BW_20;

	if (rf_bw == BW_320)
		eht_bw = EHT_BW_320;
	else if (rf_bw == BW_160)
		eht_bw = EHT_BW_160;
	else if (rf_bw == BW_80)
		eht_bw = EHT_BW_80;
	else if (rf_bw == BW_40)
		eht_bw = EHT_BW_2040;

	return eht_bw;
}

UCHAR eht_bw_2_rf_bw(UCHAR eht_bw)
{
	UCHAR rf_bw = BW_20;

	if (eht_bw == EHT_BW_320)
		rf_bw = BW_320;
	else if (eht_bw == EHT_BW_160)
		rf_bw = BW_160;
	else if (eht_bw == EHT_BW_80)
		rf_bw = BW_80;
	else if (eht_bw == EHT_BW_2040)
		rf_bw = BW_40;

	return rf_bw;
}

static struct eht_ch_layout eht_ch_40M_6G[] = {
	{1, 5, 3},
	{9, 13, 11},
	{17, 21, 19},
	{25, 29, 27},
	{33, 37, 35},
	{41, 45, 43},
	{49, 53, 51},
	{57, 61, 59},
	{65, 69, 67},
	{73, 77, 75},
	{81, 85, 83},
	{89, 93, 91},
	{97, 101, 99},
	{105, 109, 107},
	{113, 117, 115},
	{121, 125, 123},
	{129, 133, 131},
	{137, 141, 139},
	{145, 149, 147},
	{153, 157, 155},
	{161, 165, 163},
	{169, 173, 171},
	{177, 181, 179},
	{185, 189, 187},
	{193, 197, 195},
	{201, 205, 203},
	{209, 213, 211},
	{217, 221, 219},
	{225, 229, 227},
	{0, 0, 0},
};

static struct eht_ch_layout eht_ch_80M_6G[] = {
	{1, 13, 7},
	{17, 29, 23},
	{33, 45, 39},
	{49, 61, 55},
	{65, 77, 71},
	{81, 93, 87},
	{97, 109, 103},
	{113, 125, 119},
	{129, 141, 135},
	{145, 157, 151},
	{161, 173, 167},
	{177, 189, 183},
	{193, 205, 199},
	{209, 221, 215},
	{0, 0, 0},
};

static struct eht_ch_layout eht_ch_160M_6G[] = {
	{1, 29, 15},
	{33, 61, 47},
	{65, 93, 79},
	{97, 125, 111},
	{129, 157, 143},
	{161, 189, 175},
	{193, 221, 207},
	{0, 0, 0},
};

static struct  eht_ch_layout eht_ch_320M_6G[] = {
	{1, 61, 31},
	{33, 93, 63},
	{65, 125, 95},
	{97, 157, 127},
	{129, 189, 159},
	{161, 221, 191},
	{0, 0, 0},
};

static struct eht_ch_layout eht_ch_40M[] = {
	{36, 40, 38},
	{44, 48, 46},
	{52, 56, 54},
	{60, 64, 62},
	{100, 104, 102},
	{108, 112, 110},
	{116, 120, 118},
	{124, 128, 126},
	{132, 136, 134},
	{140, 144, 142},
	{149, 153, 151},
	{157, 161, 159},
	{0, 0, 0},
};

static struct eht_ch_layout eht_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{165, 177, 171},
	{0, 0, 0},
};

static struct eht_ch_layout eht_ch_160M[] = {
	{36, 64, 50},
	{100, 128, 114},
	{149, 177, 163},
	{0, 0, 0},
};

struct eht_ch_layout *get_eht_ch_array(UINT8 bw, UCHAR ch_band, UINT  *arr_size)
{
	switch (ch_band) {
	case CMD_CH_BAND_24G:
	case CMD_CH_BAND_5G:
		if (bw == BW_160) {
			*arr_size = ARRAY_SIZE(eht_ch_160M);
			return eht_ch_160M;
		} else if (bw == BW_80) {
			*arr_size = ARRAY_SIZE(eht_ch_80M);
			return eht_ch_80M;
		} else if (bw == BW_40) {
			*arr_size = ARRAY_SIZE(eht_ch_40M);
			return eht_ch_40M;
		} else
			return NULL;
		break;

	case CMD_CH_BAND_6G:
		if (bw == BW_320) {
			*arr_size = ARRAY_SIZE(eht_ch_320M_6G);
			return eht_ch_320M_6G;
		} else if (bw == BW_160) {
			*arr_size = ARRAY_SIZE(eht_ch_160M_6G);
			return eht_ch_160M_6G;
		} else if (bw == BW_80) {
			*arr_size = ARRAY_SIZE(eht_ch_80M_6G);
			return eht_ch_80M_6G;
		} else if (bw == BW_40) {
			*arr_size = ARRAY_SIZE(eht_ch_40M_6G);
			return eht_ch_40M_6G;
		} else
			return NULL;
		break;

	default:
		return NULL;
	}
}

UCHAR eht_cent_ch_freq(struct wifi_dev *wdev, UCHAR prim_ch, UCHAR eht_bw, UCHAR ch_band)
{
	INT idx = 0;
	UCHAR ext_ch = 0;
	UINT ch_layout_size[4] = {0};
	struct eht_ch_layout *ch_40M = get_eht_ch_array(BW_40, ch_band, &(ch_layout_size[0]));
	struct eht_ch_layout *ch_80M = get_eht_ch_array(BW_80, ch_band, &(ch_layout_size[1]));
	struct eht_ch_layout *ch_160M = get_eht_ch_array(BW_160, ch_band, &(ch_layout_size[2]));
	struct eht_ch_layout *ch_320M = get_eht_ch_array(BW_320, ch_band, &(ch_layout_size[3]));

	/* for BW320: */
	ext_ch = wlan_config_get_ext_cha(wdev);

	if ((eht_bw == EHT_BW_20) || (ch_band == CMD_CH_BAND_24G))
		return prim_ch;
	else if (eht_bw == EHT_BW_2040) {
		while (ch_40M && idx < ch_layout_size[0] &&
				ch_40M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_40M[idx].ch_low_bnd &&
				prim_ch <= ch_40M[idx].ch_up_bnd)
				return ch_40M[idx].cent_freq_idx;

			idx++;
		}
	} else if (eht_bw == EHT_BW_80) {
		while (ch_80M && idx < ch_layout_size[1] &&
				ch_80M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_80M[idx].ch_low_bnd &&
				prim_ch <= ch_80M[idx].ch_up_bnd)
				return ch_80M[idx].cent_freq_idx;

			idx++;
		}
	} else if (eht_bw == EHT_BW_160) {
		while (ch_160M && idx < ch_layout_size[2] &&
				ch_160M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_160M[idx].ch_low_bnd &&
				prim_ch <= ch_160M[idx].ch_up_bnd)
				return ch_160M[idx].cent_freq_idx;

			idx++;
		}
	} else if (eht_bw == EHT_BW_320) {
		while (ch_320M && idx < ch_layout_size[3] &&
				ch_320M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_320M[idx].ch_low_bnd &&
				prim_ch <= ch_320M[idx].ch_up_bnd) {
				/* if ext_ch == below:
				 * central ch < primary channel
				 * if ext_ch == above:
				 * central ch > primary channel
				 */
				if ((ext_ch == EXTCHA_BELOW) ||
					(prim_ch <= 29) || (prim_ch >= 193))
					return ch_320M[idx].cent_freq_idx;
				else
					return ch_320M[idx + 1].cent_freq_idx;
			}

			idx++;
		}
	}

	return prim_ch;
}

BOOLEAN eht320_channel_group(RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev)
{
	UINT old_idx = 0, new_idx = 0, cur_ch = 0;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	UINT ch_layout_size = 0;
	struct eht_ch_layout *old_ch_layout = get_eht_ch_array(BW_320, ch_band, &ch_layout_size);
	struct eht_ch_layout new_ch_layout[ARRAY_SIZE(eht_ch_320M_6G)] = {0};
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR groupch = 0;
	BOOLEAN is_valid = FALSE;

	/* build BW320 new_ch_layout with support_ch(pChCtrl) */
	while (old_idx < ch_layout_size && old_ch_layout[old_idx].ch_up_bnd != 0) {
		if (new_idx + 15 >= pChCtrl->ChListNum)
			break;

		if (pChCtrl->ChList[cur_ch].Channel == old_ch_layout[old_idx].ch_low_bnd) {
			if (pChCtrl->ChList[cur_ch+15].Channel == old_ch_layout[old_idx].ch_up_bnd) {
				new_ch_layout[new_idx].ch_low_bnd = old_ch_layout[old_idx].ch_low_bnd;
				new_ch_layout[new_idx].ch_up_bnd = old_ch_layout[old_idx].ch_up_bnd;
				new_ch_layout[new_idx++].cent_freq_idx = old_ch_layout[old_idx++].cent_freq_idx;
			}
			cur_ch++;
		} else if (pChCtrl->ChList[cur_ch].Channel < old_ch_layout[old_idx].ch_low_bnd) {
			cur_ch++;
		} else {
			old_idx++;
		}
	}

	/* judgment whether the channel is in the new_ch_layout */
	new_idx = 0;
	while (new_idx < ch_layout_size && new_ch_layout[new_idx].ch_up_bnd != 0) {
		if (channel >= new_ch_layout[new_idx].ch_low_bnd &&
			channel <= new_ch_layout[new_idx].ch_up_bnd) {
			/*check is 320M group channel valid*/
			is_valid = TRUE;
			for (groupch = new_ch_layout[new_idx].ch_low_bnd;
				groupch <= new_ch_layout[new_idx].ch_up_bnd; groupch += 4) {
				if (!IsValidChannel(pAd, groupch, wdev)) {
					is_valid = FALSE;
					break;
				}
			}
			if (is_valid)
				return TRUE;
		}

		new_idx++;
	}
	return FALSE;
}
VOID eht_mode_adjust(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies)
{
	UCHAR PeerMaxBw;
	BOOLEAN EnablePeerCoordinate = TRUE;

	if (!WMODE_CAP_BE(wdev->PhyMode) || wdev == NULL || peer == NULL)
		goto err;

	peer->cap.modes &= ~(EHT_24G_SUPPORT | EHT_5G_SUPPORT | EHT_6G_SUPPORT);

	PeerMaxBw = peer->MaxHTPhyMode.field.BW;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO, "Last Adjust PeerMaxBw result:%d\n", PeerMaxBw);
	if (WMODE_CAP_BE_2G(wdev->PhyMode))
		peer->cap.modes |= EHT_24G_SUPPORT;
	if (WMODE_CAP_BE_5G(wdev->PhyMode)) {
		peer->cap.modes |= EHT_5G_SUPPORT;
		PeerMaxBw = BW_160;
	}

	if (WMODE_CAP_BE_6G(wdev->PhyMode)) {
		peer->cap.modes |= EHT_6G_SUPPORT;
		PeerMaxBw = BW_160;
		if (peer->cap.eht_phy_cap & EHT_320M_6G) {
			if (wdev->wdev_type == WDEV_TYPE_STA)
				PeerMaxBw = BW_160;
			else
				PeerMaxBw = BW_320;

			if ((cmm_ies != NULL) && HAS_EHT_OP_EXIST(cmm_ies->ie_exists) &&
				GET_DOT11BE_OP_PARAM_HAS_OP_INFO(cmm_ies->eht_op.op_parameters)) {
				UCHAR op_bw = GET_DOT11BE_OP_CTRL_CH_BW(cmm_ies->eht_op.op_info.control);

				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO, "Got bw(%u) in EHT OP\n", op_bw);
				if (op_bw == EHT_OP_CH_BW320)
					PeerMaxBw = BW_320;
				else if (op_bw == EHT_OP_CH_BW160)
					PeerMaxBw = BW_160;
				else if (op_bw == EHT_OP_CH_BW80)
					PeerMaxBw = BW_80;
				else if (op_bw == EHT_OP_CH_BW40)
					PeerMaxBw = BW_40;
				else if (op_bw == EHT_OP_CH_BW20)
					PeerMaxBw = BW_20;
				else
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR, "Invalid bw(%u) in EHT OP\n", op_bw);
			}
		}
	}
	peer->MaxHTPhyMode.field.MODE = MODE_EHT;

	/* If HE BW is BW160, DO NOT cut EHT BW if it is BW320 */
	if ((PeerMaxBw > peer->MaxHTPhyMode.field.BW) && (peer->MaxHTPhyMode.field.BW < BW_160))
		PeerMaxBw = peer->MaxHTPhyMode.field.BW;
	peer->MaxHTPhyMode.field.BW = phy_bw_adjust_eht(PeerMaxBw, wlan_operate_get_eht_bw(wdev), EnablePeerCoordinate);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO, "peer->MaxHTPhyMode.field.BW:%d\n", peer->MaxHTPhyMode.field.BW);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO, "PeerMaxBw:%d\n", PeerMaxBw);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO, "wlan_operate_get_eht_bw(wdev):%d\n", wlan_operate_get_eht_bw(wdev));
err:
	return;
}
#endif /* DOT11_EHT_BE */
