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
 ***************************************************************************/

/****************************************************************************
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/
#ifdef RT_CFG80211_SUPPORT

#include "rtmp_comm.h"
#include "rt_os_util.h"

#include "rt_config.h"
#include "chlist.h"
/* all available channels */

static const UCHAR Cfg80211_Chan[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	/* 802.11 UNI / HyperLan 2 */
	36, 40, 44, 48, 52, 56, 60, 64,

	/* 802.11 HyperLan 2 */
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136,

	/* 802.11 UNII */
	140, 144, 149, 153, 157, 161, 165, 169, 173, 177,

#ifdef CFG80211_FULL_OPS_SUPPORT
	// /* Japan */
	// 184, 188, 192, 196, 208, 212, 216,
#else
	/* Japan */
	184, 188, 192, 196, 208, 212, 216,
#endif
	1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41,

	45, 49, 53, 57, 61, 65,	69, 73, 77, 81,

	85, 89, 93, 97, 101, 105, 109, 113, 117,

	121, 125, 129, 133, 137, 141, 145, 149,

	153, 157, 161, 165, 169, 173, 177, 181,

	185, 189, 193, 197, 201, 205, 209, 213,

	217, 221, 225, 229, 233,
};

UCHAR Cfg80211_RadarChan[] = {
#ifdef CFG80211_FULL_OPS_SUPPORT
	52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144,
#else
	52, 54, 56, 60, 62, 64, 100, 104, 144,
#endif
};

/*
 *	Array of bitrates the hardware can operate with
 *	in this band. Must be sorted to give a valid "supported
 *	rates" IE, i.e. CCK rates first, then OFDM.
 *
 *	For HT, assign MCS in another structure, ieee80211_sta_ht_cap.
 */
const struct ieee80211_rate Cfg80211_SupRate[12] = {
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 10,
		.hw_value = 0,
		.hw_value_short = 0,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.hw_value = 1,
		.hw_value_short = 1,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.hw_value = 2,
		.hw_value_short = 2,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.hw_value = 3,
		.hw_value_short = 3,
	},
	{
		.flags = 0,
		.bitrate = 60,
		.hw_value = 4,
		.hw_value_short = 4,
	},
	{
		.flags = 0,
		.bitrate = 90,
		.hw_value = 5,
		.hw_value_short = 5,
	},
	{
		.flags = 0,
		.bitrate = 120,
		.hw_value = 6,
		.hw_value_short = 6,
	},
	{
		.flags = 0,
		.bitrate = 180,
		.hw_value = 7,
		.hw_value_short = 7,
	},
	{
		.flags = 0,
		.bitrate = 240,
		.hw_value = 8,
		.hw_value_short = 8,
	},
	{
		.flags = 0,
		.bitrate = 360,
		.hw_value = 9,
		.hw_value_short = 9,
	},
	{
		.flags = 0,
		.bitrate = 480,
		.hw_value = 10,
		.hw_value_short = 10,
	},
	{
		.flags = 0,
		.bitrate = 540,
		.hw_value = 11,
		.hw_value_short = 11,
	},
};

static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC,
	WLAN_CIPHER_SUITE_BIP_CMAC_256,
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
#endif /* HOSTAPD_SUITEB_SUPPORT */
#endif /*DOT11W_PMF_SUPPORT*/
	WLAN_CIPHER_SUITE_GCMP,
	WLAN_CIPHER_SUITE_CCMP_256,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_GCMP_256,
#endif /* HOSTAPD_SUITEB_SUPPORT */
};

static BOOLEAN IsRadarChannel(UCHAR ch)
{
	UINT idx = 0;

	for (idx = 0; idx < sizeof(Cfg80211_RadarChan); idx++) {
		if (Cfg80211_RadarChan[idx] == ch)
			return TRUE;
	}

	return FALSE;
}

UINT16 get_he_mcs_map(unsigned char nss, enum ieee80211_he_mcs_support sup)
{
	UINT16 he_mcs_map;
	int i = 0;

	for (i = 0, he_mcs_map = 0xFFFF; i < nss; i++)
		he_mcs_map = (he_mcs_map << 2) | sup;

	return cpu_to_le16(he_mcs_map);
}

static void CFG80211_sband_data_he_cap_init(
	enum PHY_CAP phy_caps,
	enum nl80211_band band_idx,
	struct ieee80211_sband_iftype_data *data,
	struct __CFG80211_BAND *pDriverBandInfo)
{
	struct ieee80211_sta_he_cap *he_cap = &data->he_cap;
	struct ieee80211_he_cap_elem *he_cap_elem;
	struct ieee80211_he_mcs_nss_supp *he_mcs_nss_supp;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE)) {
		he_cap->has_he = true;
		he_cap_elem = &he_cap->he_cap_elem;
		he_mcs_nss_supp = &he_cap->he_mcs_nss_supp;
		he_cap_elem->mac_cap_info[0] = IEEE80211_HE_MAC_CAP0_HTC_HE |
				IEEE80211_HE_MAC_CAP0_TWT_RES;
		he_cap_elem->mac_cap_info[1] =
				IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US |
				IEEE80211_HE_MAC_CAP1_MULTI_TID_AGG_RX_QOS_8;
		he_cap_elem->mac_cap_info[2] =
				IEEE80211_HE_MAC_CAP2_32BIT_BA_BITMAP |
				IEEE80211_HE_MAC_CAP2_ACK_EN |
				IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
				IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO;
#ifdef BACKPORT_NOSTDINC
		he_cap_elem->mac_cap_info[3] = IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_3;
		he_cap_elem->mac_cap_info[4] = IEEE80211_HE_MAC_CAP4_AMSDU_IN_AMPDU;
#else /* BACKPORT_NOSTDINC */
		he_cap_elem->mac_cap_info[3] = IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_VHT_2;
		he_cap_elem->mac_cap_info[4] = IEEE80211_HE_MAC_CAP4_AMDSU_IN_AMPDU;
#endif /* !BACKPORT_NOSTDINC */
		if (band_idx == IEEE80211_BAND_2GHZ) {
			he_cap_elem->phy_cap_info[0] =
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
		} else {
			he_cap_elem->phy_cap_info[0] =
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G |
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
		}
		he_cap_elem->phy_cap_info[1] =
				IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A |
				IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD |
				IEEE80211_HE_PHY_CAP1_MIDAMBLE_RX_TX_MAX_NSTS;
		he_cap_elem->phy_cap_info[2] =
				IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
				IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ |
				IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ |
				IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
				IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO;
		he_cap_elem->phy_cap_info[3] =
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
				IEEE80211_HE_PHY_CAP3_SU_BEAMFORMER;
		he_cap_elem->phy_cap_info[4] =
				IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |
				IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER |
				IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_8 |
				IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_8;
		he_cap_elem->phy_cap_info[5] =
				IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_2 |
				IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_2;
		he_cap_elem->phy_cap_info[6] =
				IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO;
		he_cap_elem->phy_cap_info[7] =
#ifdef BACKPORT_NOSTDINC
				IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP |
#else
				IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_AR |
#endif
				IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI |
				IEEE80211_HE_PHY_CAP7_MAX_NC_7;
		he_cap_elem->phy_cap_info[8] =
				IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
				IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G |
				IEEE80211_HE_PHY_CAP8_20MHZ_IN_160MHZ_HE_PPDU |
				IEEE80211_HE_PHY_CAP8_80MHZ_IN_160MHZ_HE_PPDU;
		he_mcs_nss_supp->rx_mcs_80 =
			get_he_mcs_map(pDriverBandInfo->RxStream, IEEE80211_HE_MCS_SUPPORT_0_11);
		he_mcs_nss_supp->tx_mcs_80 =
			get_he_mcs_map(pDriverBandInfo->TxStream, IEEE80211_HE_MCS_SUPPORT_0_11);

		if (band_idx != IEEE80211_BAND_2GHZ) {
			he_mcs_nss_supp->rx_mcs_160 = he_mcs_nss_supp->rx_mcs_80;
			he_mcs_nss_supp->rx_mcs_80p80 = cpu_to_le16(0xffff);
			he_mcs_nss_supp->tx_mcs_160 = he_mcs_nss_supp->tx_mcs_80;
			he_mcs_nss_supp->tx_mcs_80p80 = cpu_to_le16(0xffff);
		}
		/*
		 * Set default PPE thresholds, with PPET16 set to 0, PPET8 set
		 * to 7
		 */
		he_cap->ppe_thres[0] = 0x61;
		he_cap->ppe_thres[1] = 0x1c;
		he_cap->ppe_thres[2] = 0xc7;
		he_cap->ppe_thres[3] = 0x71;
	}
#if (KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE) || defined(BACKPORT_NOSTDINC)
	if ((band_idx == IEEE80211_BAND_6GHZ)
		&& IS_PHY_CAPS(phy_caps, fPHY_CAP_6G)) {
		struct ieee80211_he_6ghz_capa *he_6ghz_capa = &data->he_6ghz_capa;

		he_6ghz_capa->capa = IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN |
				IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP |
				IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS |
				IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS;
	}
#endif
}

static void CFG80211_sband_data_eht_cap_init(
	struct _RTMP_ADAPTER *pAd,
	enum PHY_CAP phy_caps,
	struct mcs_nss_caps *nss_caps,
	enum nl80211_band band_idx,
	struct ieee80211_sband_iftype_data *data)
{
#if (KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE) || defined(BACKPORT_NOSTDINC)
	struct ieee80211_sta_eht_cap *eht_cap;
	struct ieee80211_eht_cap_elem_fixed *eht_cap_elem;
	struct ieee80211_eht_mcs_nss_supp *eht_mcs_nss_supp;
	u8 val;
	int nss = nss_caps->max_nss;
	int sts = nss_caps->max_nss;

#define SET_EHT_MAX_NSS(_bw, _val) do {			 \
		eht_mcs_nss_supp->bw._##_bw.rx_tx_mcs9_max_nss = _val;   \
		eht_mcs_nss_supp->bw._##_bw.rx_tx_mcs11_max_nss = _val;  \
		eht_mcs_nss_supp->bw._##_bw.rx_tx_mcs13_max_nss = _val;  \
	} while (0)

	eht_cap = &data->eht_cap;
	if (PD_GET_GEN_DOWN(pAd->physical_dev) == true)
		phy_caps &= ~fPHY_CAP_EHT;
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_EHT)) {
		eht_cap->has_eht = true;
		eht_cap_elem = &eht_cap->eht_cap_elem;
		eht_mcs_nss_supp = &eht_cap->eht_mcs_nss_supp;

		os_zero_mem(eht_cap_elem, sizeof(*eht_cap_elem));
		os_zero_mem(eht_mcs_nss_supp, sizeof(*eht_mcs_nss_supp));
		if ((band_idx == IEEE80211_BAND_2GHZ) &&
		    IS_PHY_CAPS(phy_caps, fPHY_CAP_24G)) {
			eht_cap_elem->mac_cap_info[0] =
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
				IEEE80211_EHT_MAC_CAP0_EPCS_PRIO_ACCESS |
#endif /* defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION) */
				IEEE80211_EHT_MAC_CAP0_OM_CONTROL;
			eht_cap_elem->phy_cap_info[0] =
				IEEE80211_EHT_PHY_CAP0_NDP_4_EHT_LFT_32_GI |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMER |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMEE |
				u8_encode_bits(u8_get_bits(sts - 1, BIT(0)),
					IEEE80211_EHT_PHY_CAP0_BEAMFORMEE_SS_80MHZ_MASK);
			eht_cap_elem->phy_cap_info[1] =
				u8_encode_bits(u8_get_bits(sts - 1, GENMASK(2, 1)),
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_80MHZ_MASK);
			eht_cap_elem->phy_cap_info[2] =
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_80MHZ_MASK);
			eht_cap_elem->phy_cap_info[3] =
				IEEE80211_EHT_PHY_CAP3_NG_16_SU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_NG_16_MU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_4_2_SU_FDBK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_7_5_MU_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_SU_BF_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_MU_BF_PART_BW_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_CQI_FDBK;
			eht_cap_elem->phy_cap_info[4] =
				u8_encode_bits(min_t(int, sts - 1, 2),
					IEEE80211_EHT_PHY_CAP4_MAX_NC_MASK);
			eht_cap_elem->phy_cap_info[5] =
				IEEE80211_EHT_PHY_CAP5_NON_TRIG_CQI_FEEDBACK |
				u8_encode_bits(IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_16US,
					IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_MASK) |
				u8_encode_bits(u8_get_bits(0x11, GENMASK(1, 0)),
					IEEE80211_EHT_PHY_CAP5_MAX_NUM_SUPP_EHT_LTF_MASK);

			val = 0x1;
			eht_cap_elem->phy_cap_info[6] =
				u8_encode_bits(u8_get_bits(0x11, GENMASK(4, 2)),
					       IEEE80211_EHT_PHY_CAP6_MAX_NUM_SUPP_EHT_LTF_MASK) |
				u8_encode_bits(val, IEEE80211_EHT_PHY_CAP6_MCS15_SUPP_MASK);

			eht_cap_elem->phy_cap_info[7] = 0;

			val = u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_RX) |
			      u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_TX);
			SET_EHT_MAX_NSS(80, val);
		} else if ((band_idx == IEEE80211_BAND_5GHZ) &&
			   IS_PHY_CAPS(phy_caps, fPHY_CAP_5G)) {
			eht_cap_elem->mac_cap_info[0] =
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
				IEEE80211_EHT_MAC_CAP0_EPCS_PRIO_ACCESS |
#endif /* defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION) */
				IEEE80211_EHT_MAC_CAP0_OM_CONTROL;
			eht_cap_elem->phy_cap_info[0] =
				IEEE80211_EHT_PHY_CAP0_NDP_4_EHT_LFT_32_GI |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMER |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMEE |
				u8_encode_bits(u8_get_bits(sts - 1, BIT(0)),
					IEEE80211_EHT_PHY_CAP0_BEAMFORMEE_SS_80MHZ_MASK);
			eht_cap_elem->phy_cap_info[1] =
				u8_encode_bits(u8_get_bits(sts - 1, GENMASK(2, 1)),
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_80MHZ_MASK) |
				u8_encode_bits(sts - 1,
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_160MHZ_MASK);
			eht_cap_elem->phy_cap_info[2] =
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_80MHZ_MASK) |
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_160MHZ_MASK);
			eht_cap_elem->phy_cap_info[3] =
				IEEE80211_EHT_PHY_CAP3_NG_16_SU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_NG_16_MU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_4_2_SU_FDBK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_7_5_MU_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_SU_BF_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_MU_BF_PART_BW_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_CQI_FDBK;
			eht_cap_elem->phy_cap_info[4] =
				u8_encode_bits(min_t(int, sts - 1, 2),
					IEEE80211_EHT_PHY_CAP4_MAX_NC_MASK);
			eht_cap_elem->phy_cap_info[5] =
				IEEE80211_EHT_PHY_CAP5_NON_TRIG_CQI_FEEDBACK |
				u8_encode_bits(IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_16US,
					IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_MASK) |
				u8_encode_bits(u8_get_bits(0x11, GENMASK(1, 0)),
					IEEE80211_EHT_PHY_CAP5_MAX_NUM_SUPP_EHT_LTF_MASK);
			if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160C_STD) ||
				 IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160NC) ||
				 IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160C))
				val = 0x7;
			else if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW80))
				val = 0x3;
			else
				val = 0x1;
			eht_cap_elem->phy_cap_info[6] =
				u8_encode_bits(u8_get_bits(0x11, GENMASK(4, 2)),
					       IEEE80211_EHT_PHY_CAP6_MAX_NUM_SUPP_EHT_LTF_MASK) |
				u8_encode_bits(val, IEEE80211_EHT_PHY_CAP6_MCS15_SUPP_MASK);

			eht_cap_elem->phy_cap_info[7] =
				IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_80MHZ |
				IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_160MHZ |
				IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_80MHZ |
				IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_160MHZ;

			val = u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_RX) |
			      u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_TX);
			SET_EHT_MAX_NSS(80, val);
			SET_EHT_MAX_NSS(160, val);
		} else if ((band_idx == IEEE80211_BAND_6GHZ) &&
			   IS_PHY_CAPS(phy_caps, fPHY_CAP_6G)) {
			eht_cap_elem->mac_cap_info[0] =
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
				IEEE80211_EHT_MAC_CAP0_EPCS_PRIO_ACCESS |
#endif /* defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION) */
				IEEE80211_EHT_MAC_CAP0_OM_CONTROL;
			eht_cap_elem->phy_cap_info[0] =
				IEEE80211_EHT_PHY_CAP0_NDP_4_EHT_LFT_32_GI |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMER |
				IEEE80211_EHT_PHY_CAP0_SU_BEAMFORMEE |
				u8_encode_bits(u8_get_bits(sts - 1, BIT(0)),
					IEEE80211_EHT_PHY_CAP0_BEAMFORMEE_SS_80MHZ_MASK);
			eht_cap_elem->phy_cap_info[1] =
				u8_encode_bits(u8_get_bits(sts - 1, GENMASK(2, 1)),
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_80MHZ_MASK) |
				u8_encode_bits(sts - 1,
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_160MHZ_MASK) |
				u8_encode_bits(sts - 1,
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_320MHZ_MASK);
			eht_cap_elem->phy_cap_info[2] =
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_80MHZ_MASK) |
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_160MHZ_MASK) |
				u8_encode_bits(sts - 1, IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_320MHZ_MASK);
			eht_cap_elem->phy_cap_info[3] =
				IEEE80211_EHT_PHY_CAP3_NG_16_SU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_NG_16_MU_FEEDBACK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_4_2_SU_FDBK |
				IEEE80211_EHT_PHY_CAP3_CODEBOOK_7_5_MU_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_SU_BF_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_MU_BF_PART_BW_FDBK |
				IEEE80211_EHT_PHY_CAP3_TRIG_CQI_FDBK;
			eht_cap_elem->phy_cap_info[4] =
				u8_encode_bits(min_t(int, sts - 1, 2),
					IEEE80211_EHT_PHY_CAP4_MAX_NC_MASK);
			eht_cap_elem->phy_cap_info[5] =
				IEEE80211_EHT_PHY_CAP5_NON_TRIG_CQI_FEEDBACK |
				u8_encode_bits(IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_16US,
					IEEE80211_EHT_PHY_CAP5_COMMON_NOMINAL_PKT_PAD_MASK) |
				u8_encode_bits(u8_get_bits(0x11, GENMASK(1, 0)),
					IEEE80211_EHT_PHY_CAP5_MAX_NUM_SUPP_EHT_LTF_MASK);
			if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW320))
				val = 0xf;
			else if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160C_STD) ||
				 IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160NC) ||
				 IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160C))
				val = 0x7;
			else if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW80))
				val = 0x3;
			else
				val = 0x1;

			eht_cap_elem->phy_cap_info[6] =
				u8_encode_bits(u8_get_bits(0x11, GENMASK(4, 2)),
					       IEEE80211_EHT_PHY_CAP6_MAX_NUM_SUPP_EHT_LTF_MASK) |
				u8_encode_bits(val, IEEE80211_EHT_PHY_CAP6_MCS15_SUPP_MASK);

			eht_cap_elem->phy_cap_info[7] =
				IEEE80211_EHT_PHY_CAP7_20MHZ_STA_RX_NDP_WIDER_BW |
				IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_80MHZ |
				IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_160MHZ |
				IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_80MHZ |
				IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_160MHZ;

			val = u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_RX) |
			      u8_encode_bits(nss, IEEE80211_EHT_MCS_NSS_TX);

			SET_EHT_MAX_NSS(80, val);
			SET_EHT_MAX_NSS(160, val);

			if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW320)) {
				eht_cap_elem->phy_cap_info[0] |=
					IEEE80211_EHT_PHY_CAP0_320MHZ_IN_6GHZ;
				eht_cap_elem->phy_cap_info[1] |=
					IEEE80211_EHT_PHY_CAP1_BEAMFORMEE_SS_320MHZ_MASK;
				eht_cap_elem->phy_cap_info[2] |=
					IEEE80211_EHT_PHY_CAP2_SOUNDING_DIM_320MHZ_MASK;
				eht_cap_elem->phy_cap_info[7] |=
					IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_320MHZ |
					IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_320MHZ;
				SET_EHT_MAX_NSS(320, val);
			}
		}
	}
#undef SET_EHT_MAX_NSS
#endif /* (KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE) || defined(BACKPORT_NOSTDINC) */
}

static void CFG80211_sband_data_init(
	struct _RTMP_ADAPTER *pAd,
	struct _RTMP_ADAPTER *mac_ad,
	struct __CFG80211_BAND *pDriverBandInfo,
	enum nl80211_band band_idx,
	struct ieee80211_supported_band *band_info,
	UINT8 re_init)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	struct ieee80211_sband_iftype_data *data;
	enum PHY_CAP phy_caps = 0;
	struct mcs_nss_caps *nss_cap;

	if (!pDriverBandInfo) {
		MTWF_PRINT("%s: pDriverBandInfo is null.\n", __func__);
		return;
	}
	if (!band_info) {
		MTWF_PRINT("%s: band_info is null.\n", __func__);
		return;
	}

	if (!mac_ad
		|| (band_idx >= NUM_NL80211_BANDS)) {
		band_info->n_iftype_data = 0;
		MTWF_PRINT("%s: band_idx = %d (NUM_NL80211_BANDS = %d)\n",
			__func__, band_idx, NUM_NL80211_BANDS);
		return;
	}
	chip_cap = PD_GET_CHIP_CAP_PTR(mac_ad->physical_dev);
	phy_caps = pDriverBandInfo->phy_caps;
	nss_cap = MCS_NSS_CAP(mac_ad);
	data = &chip_cap->sband_data[band_idx];
	os_zero_mem(data, sizeof(struct ieee80211_sband_iftype_data));
	data->types_mask = BIT(NL80211_IFTYPE_AP);
	CFG80211_sband_data_he_cap_init(phy_caps, band_idx, data, pDriverBandInfo);
	CFG80211_sband_data_eht_cap_init(pAd, phy_caps, nss_cap, band_idx, data);
	band_info->n_iftype_data = 1;
	band_info->iftype_data = &chip_cap->sband_data[band_idx];

	/*
	 * ATE would call CFG80211OS_SupBandReInit directly
	 * (i.e. w/o calling CFG80211_SupBandInit)
	 */
	if (re_init) {
		data = (struct ieee80211_sband_iftype_data *)band_info->iftype_data;
		data->types_mask |= BIT(NL80211_IFTYPE_STATION);
	}
}

VOID CFG80211OS_PutBss(
	IN VOID *pWiphyOrg,
	IN VOID *pCfg80211Bss)
{
	struct cfg80211_bss *bss = (struct cfg80211_bss *)pCfg80211Bss;
	struct wiphy *pWiphy = (struct wiphy *) pWiphyOrg;

	cfg80211_put_bss(pWiphy, bss);
}

/*
 * ========================================================================
 * Routine Description:
 *	UnRegister MAC80211 Module.
 *
 * Arguments:
 *	pCB				- CFG80211 control block pointer
 *	pNetDev			- Network device
 *
 * Return Value:
 *	NONE
 *
 * ========================================================================
 */
VOID CFG80211OS_UnRegister(VOID *pCB, VOID *pNetDevOrg)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct net_device *pNetDev = (struct net_device *)pNetDevOrg;

	/* unregister */
	if (pCfg80211_CB->pCfg80211_Wdev != NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
			"80211> unregister/free wireless device\n");
		/*
		 *	Must unregister, or you will suffer problem when you change
		 *	regulatory domain by using iw.
		 */
#ifdef RFKILL_HW_SUPPORT
		wiphy_rfkill_stop_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */
		wiphy_unregister(pCfg80211_CB->pCfg80211_Wdev->wiphy);
		wiphy_free(pCfg80211_CB->pCfg80211_Wdev->wiphy);
		os_free_mem(pCfg80211_CB->pCfg80211_Wdev);

		if (pCfg80211_CB->pCfg80211_Channels != NULL)
			os_free_mem(pCfg80211_CB->pCfg80211_Channels);

		if (pCfg80211_CB->pCfg80211_Rates != NULL)
			os_free_mem(pCfg80211_CB->pCfg80211_Rates);

		pCfg80211_CB->pCfg80211_Wdev = NULL;
		pCfg80211_CB->pCfg80211_Channels = NULL;
		pCfg80211_CB->pCfg80211_Rates = NULL;
		/* must reset to NULL; or kernel will panic in unregister_netdev */
		pNetDev->ieee80211_ptr = NULL;
		SET_NETDEV_DEV(pNetDev, NULL);
	}

	os_free_mem(pCfg80211_CB);
}


/*
 * ========================================================================
 * Routine Description:
 *	Initialize wireless channel in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pWiphy			- WLAN PHY interface
 *	pChannels		- Current channel info
 *	pRates			- Current rate info
 *
 * Return Value:
 *	TRUE			- init successfully
 *	FALSE			- init fail
 *
 * Note:
 *	TX Power related:
 *
 *	1. Suppose we can send power to 15dBm in the board.
 *	2. A value 0x0 ~ 0x1F for a channel. We will adjust it based on 15dBm/
 *		54Mbps. So if value == 0x07, the TX power of 54Mbps is 15dBm and
 *		the value is 0x07 in the EEPROM.
 *	3. Based on TX power value of 54Mbps/channel, adjust another value
 *		0x0 ~ 0xF for other data rate. (-6dBm ~ +6dBm)
 *
 *	Other related factors:
 *	1. TX power percentage from UI/users;
 *	2. Maximum TX power limitation in the regulatory domain.
 * ========================================================================
 */

static UINT16 VHT_HIGH_RATE_BW80[3][4] = {
	{292, 585, 877, 1170},
	{351, 702, 1053, 1404},
	{390, 780, 1170, 1560},
};

UINT16 get_vht_mcs_map(unsigned char nss, enum ieee80211_vht_mcs_support sup)
{
	UINT16 vht_mcs_map;
	int i = 0;

	for (i = 0, vht_mcs_map = 0xFFFF; i < nss; i++)
		vht_mcs_map = (vht_mcs_map << 2) | sup;

	return cpu_to_le16(vht_mcs_map);
}
BOOLEAN CFG80211_SupBandInit(
	struct _RTMP_ADAPTER *pAd,
	VOID *pCB,
	struct __CFG80211_BAND *pDriverBandInfo,
	VOID *pWiphyOrg,
	VOID *pChannelsOrg,
	VOID *pRatesOrg,
	UINT8 ReInit)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;
	struct ieee80211_channel *pChannels = (struct ieee80211_channel *)pChannelsOrg;
	struct ieee80211_rate *pRates = (struct ieee80211_rate *)pRatesOrg;
	struct ieee80211_supported_band *pBand;
	UINT32 NumOfChan, NumOfRate;
	UINT32 IdLoop;
	UINT32 CurTxPower;
	ULONG *priv;
	UINT16 vht_mcs;
	struct _RTMP_ADAPTER *mac_ad = NULL;

	priv = (ULONG *)(wiphy_priv(pWiphy));
	if (priv)
		mac_ad = (struct _RTMP_ADAPTER *)(*priv);
	/* sanity check */
	if (pDriverBandInfo->RFICType == 0)
		pDriverBandInfo->RFICType = RFIC_24GHZ | RFIC_5GHZ | RFIC_6GHZ;

	/* 1. Calcute the Channel Number */
	NumOfChan = CFG80211_NUM_OF_CHAN_2GHZ;

	if (pDriverBandInfo->RFICType & RFIC_5GHZ)
		NumOfChan += CFG80211_NUM_OF_CHAN_5GHZ;

	if (pDriverBandInfo->RFICType & RFIC_6GHZ)
		NumOfChan += CFG80211_NUM_OF_CHAN_6GHZ;

	/*need to check why FlgIsBMode set to TRUE*/
	NumOfRate = 4 + 8;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
		"80211> RFICType= %d, NumOfChan= %d, ReInit = %d\n",
		pDriverBandInfo->RFICType, NumOfChan, ReInit);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
		"80211> Number of rate = %d\n", NumOfRate);

	/* 3. Allocate the Channel instance */
	if (pChannels == NULL && NumOfChan) {
		pChannels = kcalloc(NumOfChan, sizeof(*pChannels), GFP_KERNEL);

		if (!pChannels) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"80211> ieee80211_channel allocation fail!\n");
			return FALSE;
		}
	}

	/* 4. Allocate the Rate instance */
	if (pRates == NULL && NumOfRate) {
		pRates = kcalloc(NumOfRate, sizeof(*pRates), GFP_KERNEL);

		if (!pRates) {
			os_free_mem(pChannels);
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"80211> ieee80211_rate allocation fail!\n");
			return FALSE;
		}
	}

	/* get TX power */
#ifdef SINGLE_SKU
	CurTxPower = pDriverBandInfo->DefineMaxTxPwr; /* dBm */
#else
	CurTxPower = 0; /* unknown */
#endif /* SINGLE_SKU */
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
		"80211> CurTxPower = %d dBm\n", CurTxPower);

	/* 5. init channel */
	for (IdLoop = 0; IdLoop < NumOfChan; IdLoop++) {
		if (IdLoop >= (CFG80211_NUM_OF_CHAN_5GHZ + CFG80211_NUM_OF_CHAN_2GHZ)) {
			pChannels[IdLoop].band = IEEE80211_BAND_6GHZ;
			pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop], IEEE80211_BAND_6GHZ);
		} else if (IdLoop >= CFG80211_NUM_OF_CHAN_2GHZ) {
			pChannels[IdLoop].band = IEEE80211_BAND_5GHZ;
			pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop], IEEE80211_BAND_5GHZ);
		} else {
			pChannels[IdLoop].band = IEEE80211_BAND_2GHZ;
			pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop], IEEE80211_BAND_2GHZ);
		}

		pChannels[IdLoop].hw_value = IdLoop;
		pChannels[IdLoop].max_power = CurTxPower;

		pChannels[IdLoop].max_antenna_gain = 0xff;
		pChannels[IdLoop].flags = 0;

		/* if (RadarChannelCheck(pAd, Cfg80211_Chan[IdLoop])) */
		if (IsRadarChannel(Cfg80211_Chan[IdLoop])) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"====> Radar Channel %d\n", Cfg80211_Chan[IdLoop]);
#ifdef CFG80211_FULL_OPS_SUPPORT
			pChannels[IdLoop].flags |= IEEE80211_CHAN_RADAR;
#else
			pChannels[IdLoop].flags |= (IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_IR);
#endif
			pChannels[IdLoop].dfs_state = NL80211_DFS_AVAILABLE;
		}

		/*		CFG_TODO:
		 *		pChannels[IdLoop].flags
		 *		enum ieee80211_channel_flags {
		 *			IEEE80211_CHAN_DISABLED		= 1<<0,
		 *			IEEE80211_CHAN_PASSIVE_SCAN	= 1<<1,
		 *			IEEE80211_CHAN_NO_IBSS		= 1<<2,
		 *			IEEE80211_CHAN_RADAR		= 1<<3,
		 *			IEEE80211_CHAN_NO_HT40PLUS	= 1<<4,
		 *			IEEE80211_CHAN_NO_HT40MINUS	= 1<<5,
		 *		};
		 */
	}

	/* 6. init rate */
	for (IdLoop = 0; IdLoop < NumOfRate; IdLoop++)
		memcpy(&pRates[IdLoop], &Cfg80211_SupRate[IdLoop], sizeof(*pRates));

	/*		CFG_TODO:
	 *		enum ieee80211_rate_flags {
	 *			IEEE80211_RATE_SHORT_PREAMBLE	= 1<<0,
	 *			IEEE80211_RATE_MANDATORY_A	= 1<<1,
	 *			IEEE80211_RATE_MANDATORY_B	= 1<<2,
	 *			IEEE80211_RATE_MANDATORY_G	= 1<<3,
	 *			IEEE80211_RATE_ERP_G		= 1<<4,
	 *		};
	 */
	/* 7. Fill the Band 2.4GHz */
	pBand = &pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_2GHZ];

	if (pDriverBandInfo->RFICType & RFIC_24GHZ) {
		pBand->n_channels = CFG80211_NUM_OF_CHAN_2GHZ;
		if (pDriverBandInfo->FlgIsBMode == TRUE)
			pBand->n_bitrates = 4;
		else
			pBand->n_bitrates = NumOfRate;

		pBand->channels = pChannels;
		pBand->bitrates = pRates;
#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
							IEEE80211_HT_CAP_SM_PS |
							IEEE80211_HT_CAP_SGI_40 |
							IEEE80211_HT_CAP_SGI_20 |
							IEEE80211_HT_CAP_TX_STBC |
						    IEEE80211_HT_CAP_RX_STBC |
							IEEE80211_HT_CAP_DSSSCCK40 |
							IEEE80211_HT_CAP_LDPC_CODING |
							IEEE80211_HT_CAP_MAX_AMSDU |
							IEEE80211_HT_CAP_GRN_FLD;
		pBand->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = pDriverBandInfo->MpduDensity; /* YF_TODO */
		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));

		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
			"80211> TxStream=%d, 80211> RxStream=%d\n",
			pDriverBandInfo->TxStream, pDriverBandInfo->RxStream);

		switch (pDriverBandInfo->RxStream) {
		case 1:
		default:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			break;

		case 2:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(300);
			break;

		case 3:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			break;

		case 4:
		    pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_mask[3] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(600);
			break;
		}

		pBand->ht_cap.mcs.rx_mask[4] = 0x01; /* 40MHz*/
		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
		if (pDriverBandInfo->TxStream != pDriverBandInfo->RxStream) {
			pBand->ht_cap.mcs.tx_params |= IEEE80211_HT_MCS_TX_RX_DIFF;
			pBand->ht_cap.mcs.tx_params |= (((pDriverBandInfo->TxStream - 1) & 0x03) <<
				IEEE80211_HT_MCS_TX_MAX_STREAMS_SHIFT);
		}
#endif /* DOT11_N_SUPPORT */
		CFG80211_sband_data_init(
			pAd, mac_ad, pDriverBandInfo, IEEE80211_BAND_2GHZ, pBand, ReInit);
		pWiphy->bands[IEEE80211_BAND_2GHZ] = pBand;
	} else {
		pWiphy->bands[IEEE80211_BAND_2GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
		pBand->n_iftype_data = 0;
	}

	/* 8. Fill the Band 5GHz */
	pBand = &pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_5GHZ];

	if (pDriverBandInfo->RFICType & RFIC_5GHZ) {
		pBand->n_channels = CFG80211_NUM_OF_CHAN_5GHZ;
		pBand->n_bitrates = NumOfRate - 4;	/*Disable 11B rate*/
		pBand->channels = &pChannels[CFG80211_NUM_OF_CHAN_2GHZ];
		pBand->bitrates = &pRates[4];
#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
							IEEE80211_HT_CAP_SM_PS |
							IEEE80211_HT_CAP_SGI_40 |
							IEEE80211_HT_CAP_SGI_20 |
							IEEE80211_HT_CAP_TX_STBC |
						    IEEE80211_HT_CAP_RX_STBC |
							IEEE80211_HT_CAP_DSSSCCK40 |
							IEEE80211_HT_CAP_LDPC_CODING |
							IEEE80211_HT_CAP_MAX_AMSDU |
							IEEE80211_HT_CAP_GRN_FLD;
		pBand->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = pDriverBandInfo->MpduDensity; /* YF_TODO */
		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
					"80211> TxStream=%d, 80211> RxStream=%d\n",
					pDriverBandInfo->TxStream, pDriverBandInfo->RxStream);

		switch (pDriverBandInfo->RxStream) {
		case 1:
		default:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			break;

		case 2:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(300);
			break;

		case 3:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			break;

		case 4:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_mask[3] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(600);
			break;
		}

		pBand->ht_cap.mcs.rx_mask[4] = 0x01; /* 40MHz*/
		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
		if (pDriverBandInfo->TxStream != pDriverBandInfo->RxStream) {
			pBand->ht_cap.mcs.tx_params |= IEEE80211_HT_MCS_TX_RX_DIFF;
			pBand->ht_cap.mcs.tx_params |= (((pDriverBandInfo->TxStream - 1) & 0x03) <<
				IEEE80211_HT_MCS_TX_MAX_STREAMS_SHIFT);
		}
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		pBand->vht_cap.vht_supported = true;
		pBand->vht_cap.cap = IEEE80211_VHT_CAP_RXLDPC  |
					IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ |
					IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
							 IEEE80211_VHT_CAP_SHORT_GI_80  |
							 IEEE80211_VHT_CAP_SHORT_GI_160 |
							 IEEE80211_VHT_CAP_TXSTBC |
							 IEEE80211_STA_RX_BW_80 |
							 IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |
					IEEE80211_VHT_CAP_RXSTBC_MASK |
					IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE |
					IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
					IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE |
					IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE |
					IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN |
					IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN;
		vht_mcs = get_vht_mcs_map(pDriverBandInfo->RxStream, IEEE80211_VHT_MCS_SUPPORT_0_9);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
					"80211 rx_vht_mcs=%04x\n", vht_mcs);

		pBand->vht_cap.vht_mcs.rx_mcs_map = vht_mcs;
		pBand->vht_cap.vht_mcs.rx_highest = cpu_to_le16(VHT_HIGH_RATE_BW80[2][pDriverBandInfo->RxStream & 0x03]);

		vht_mcs = get_vht_mcs_map(pDriverBandInfo->TxStream, IEEE80211_VHT_MCS_SUPPORT_0_9);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
					"80211 tx_vht_mcs=%04x\n", vht_mcs);

		pBand->vht_cap.vht_mcs.tx_mcs_map = vht_mcs;
		pBand->vht_cap.vht_mcs.tx_highest = cpu_to_le16(VHT_HIGH_RATE_BW80[2][pDriverBandInfo->TxStream & 0x03]);
#endif /* DOT11_VHT_AC */
		CFG80211_sband_data_init(
			pAd, mac_ad, pDriverBandInfo, IEEE80211_BAND_5GHZ, pBand, ReInit);
		pWiphy->bands[IEEE80211_BAND_5GHZ] = pBand;
	} else {
		pWiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
		pBand->n_iftype_data = 0;
	}

	/* 9. Fill the Band 6GHz */
	pBand = &pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_6GHZ];

	if (pDriverBandInfo->RFICType & RFIC_6GHZ) {
		pBand->n_channels = CFG80211_NUM_OF_CHAN_6GHZ;
		pBand->n_bitrates = NumOfRate - 4;	/*Disable 11B rate*/
		pBand->channels =
			&pChannels[CFG80211_NUM_OF_CHAN_2GHZ + CFG80211_NUM_OF_CHAN_5GHZ];
		pBand->bitrates = &pRates[4];
		CFG80211_sband_data_init(
			pAd, mac_ad, pDriverBandInfo, IEEE80211_BAND_6GHZ, pBand, ReInit);
		pWiphy->bands[IEEE80211_BAND_6GHZ] = pBand;
	} else {
		pWiphy->bands[IEEE80211_BAND_6GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
		pBand->n_iftype_data = 0;
	}

//	pWiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;

	/* 10. re-assign to mainDevice info */
	pCfg80211_CB->pCfg80211_Channels = pChannels;
	pCfg80211_CB->pCfg80211_Rates = pRates;
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pCB				- CFG80211 control block pointer
 *	pBandInfo		- Band information
 *
 * Return Value:
 *	TRUE			- re-init successfully
 *	FALSE			- re-init fail
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 *	But we do not have complete chip information in xx_probe() so we
 *	need to re-init bands in xx_open().
 * ========================================================================
 */
BOOLEAN CFG80211OS_SupBandReInit(
	IN struct _RTMP_ADAPTER *pAd,
	IN VOID *pCB,
	IN CFG80211_BAND * pBandInfo)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy;
	ULONG *priv;
	struct _RTMP_ADAPTER *mac_ad = NULL;
	struct _RTMP_CHIP_CAP *cap;

	if ((pCfg80211_CB == NULL) || (pCfg80211_CB->pCfg80211_Wdev == NULL))
		return FALSE;

	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	priv = (ULONG *)(wiphy_priv(pWiphy));
	if (priv)
		mac_ad = (struct _RTMP_ADAPTER *)(*priv);
	if (pWiphy && mac_ad) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"80211> re-init bands...\n");
		/* re-init bands */
		cap = hc_get_chip_cap(mac_ad->hdev_ctrl);
		pBandInfo->phy_caps = cap->phy_caps;
		CFG80211_SupBandInit(pAd, pCfg80211_CB, pBandInfo, pWiphy,
				pCfg80211_CB->pCfg80211_Channels,
				pCfg80211_CB->pCfg80211_Rates,
				TRUE);
		/* re-init PHY */
		pWiphy->rts_threshold = pBandInfo->RtsThreshold;
		pWiphy->frag_threshold = pBandInfo->FragmentThreshold;
		pWiphy->retry_short = pBandInfo->RetryMaxCnt & 0xff;
		pWiphy->retry_long = (pBandInfo->RetryMaxCnt & 0xff00) >> 8;
		regulatory_hint(pWiphy, (const char *)(pBandInfo->CountryCode));
		return TRUE;
	}

	return FALSE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from driver.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211OS_RegHint(
	IN VOID *pCB,
	IN UCHAR *pCountryIe,
	IN ULONG CountryIeLen)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
				"crda> regulatory domain hint: %c%c\n",
				 pCountryIe[0], pCountryIe[1]);

	if ((pCfg80211_CB->pCfg80211_Wdev == NULL) || (pCountryIe == NULL)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> regulatory domain hint not support!\n");
		return;
	}

	/* hints a country IE as a regulatory domain "without" channel/power info. */
	regulatory_hint(pCfg80211_CB->pCfg80211_Wdev->wiphy, (const char *)pCountryIe);
}


/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from country element.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211OS_RegHint11D(
	IN VOID *pCB,
	IN UCHAR *pCountryIe,
	IN ULONG CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
}


BOOLEAN CFG80211OS_BandInfoGet(
	IN VOID *pCB,
	IN VOID *pWiphyOrg,
	OUT VOID **ppBand24,
	OUT VOID **ppBand5,
	OUT VOID **ppBand6)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL)
		return FALSE;

	*ppBand24 = pWiphy->bands[IEEE80211_BAND_2GHZ];
	*ppBand5 = pWiphy->bands[IEEE80211_BAND_5GHZ];
	*ppBand6 = pWiphy->bands[IEEE80211_BAND_6GHZ];
	return TRUE;
}


UINT32 CFG80211OS_ChanNumGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN UINT32					IdBand)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL)
		return 0;

	if (pWiphy->bands[IdBand] != NULL)
		return pWiphy->bands[IdBand]->n_channels;

	return 0;
}

BOOLEAN CFG80211OS_ChanInfoGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN UINT32					IdBand,
	IN UINT32					IdChan,
	OUT UINT32					*pChanId,
	OUT UINT32					*pPower,
	OUT BOOLEAN					*pFlgIsRadar)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;
	struct ieee80211_supported_band *pSband;
	struct ieee80211_channel *pChan;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR, "pWiphy NULL!\n");
		return FALSE;
	}

	pSband = pWiphy->bands[IdBand];
	if (pSband == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR, "pSband NULL!\n");
		return FALSE;
	}

	pChan = &pSband->channels[IdChan];
	if (pChan == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR, "pChan NULL!\n");
		return FALSE;
	}

	*pChanId = ieee80211_frequency_to_channel(pChan->center_freq);

	if (pChan->flags & IEEE80211_CHAN_DISABLED) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"Chan %03d (frq %d):\tnot allowed!\n",
			(*pChanId), pChan->center_freq);
		return FALSE;
	}

	*pPower = pChan->max_power;

	if (pChan->flags & IEEE80211_CHAN_RADAR)
		*pFlgIsRadar = TRUE;
	else
		*pFlgIsRadar = FALSE;

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Initialize a channel information used in scan inform.
 *
 * Arguments:
 *
 * Return Value:
 *	TRUE		- Successful
 *	FALSE		- Fail
 *
 * ========================================================================
 */
BOOLEAN CFG80211OS_ChanInfoInit(
	IN VOID						*pCB,
	IN UINT32					InfoIndex,
	IN UCHAR					ChanId,
	IN UCHAR					MaxTxPwr,
	IN BOOLEAN					FlgIsNMode,
	IN BOOLEAN					FlgIsBW20M)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct ieee80211_channel *pChan;

	if (InfoIndex >= MAX_NUM_OF_CHANNELS)
		return FALSE;

	pChan = (struct ieee80211_channel *) &(pCfg80211_CB->ChanInfo[InfoIndex]);
	memset(pChan, 0, sizeof(*pChan));

	if (ChanId > 14)
		pChan->band = IEEE80211_BAND_5GHZ;
	else
		pChan->band = IEEE80211_BAND_2GHZ;

	pChan->center_freq = ieee80211_channel_to_frequency(ChanId, pChan->band);
	/* no use currently in 2.6.30 */
	/*	if (ieee80211_is_beacon(((struct ieee80211_mgmt *)pFrame)->frame_control)) */
	/*		pChan->beacon_found = 1; */
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Inform us that a scan is got.
 *
 * Arguments:
 *	pAdCB				- WLAN control block pointer
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Call RT_CFG80211_SCANNING_INFORM, not CFG80211_Scaning
 * ========================================================================
 */
VOID CFG80211OS_Scaning(
	IN VOID						*pCB,
	IN UINT32					ChanId,
	IN UCHAR					*pFrame,
	IN UINT32					FrameLen,
	IN INT32					RSSI,
	IN BOOLEAN					FlgIsNMode,
	IN UINT16					RawChannel,
	IN UINT8					BW)
{
#ifdef CONFIG_STA_SUPPORT
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct ieee80211_supported_band *pBand;
	UINT32 IdChan;
	UINT32 CenFreq;
	UINT CurBand;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct cfg80211_bss *bss = NULL;
	struct ieee80211_mgmt *mgmt;

	mgmt = (struct ieee80211_mgmt *) pFrame;

	if (ChanId == 0)
		ChanId = 1;

	/* get channel information */

	if (ChanId > 14)
		CenFreq = ieee80211_channel_to_frequency(ChanId, IEEE80211_BAND_5GHZ);
	else
		CenFreq = ieee80211_channel_to_frequency(ChanId, IEEE80211_BAND_2GHZ);

	if (RawChannel >= CHANNEL_6G_BASE)
		CenFreq = ieee80211_channel_to_frequency(ChanId, IEEE80211_BAND_6GHZ);

	if (ChanId > 14)
		CurBand = IEEE80211_BAND_5GHZ;
	else
		CurBand = IEEE80211_BAND_2GHZ;

	if (RawChannel >= CHANNEL_6G_BASE)
		CurBand = IEEE80211_BAND_6GHZ;

	pBand = &pCfg80211_CB->Cfg80211_bands[CurBand];

	for (IdChan = 0; IdChan < pBand->n_channels; IdChan++) {
		if (pBand->channels[IdChan].center_freq == CenFreq)
			break;
	}

	if (IdChan >= pBand->n_channels) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> Can not find any chan info! ==> %d[%d],[%d]\n",
				 ChanId, CenFreq, pBand->n_channels);
		return;
	}

	if (pWiphy->signal_type == CFG80211_SIGNAL_TYPE_MBM) {
		/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
		RSSI = RSSI * 100;
	}

	if (!mgmt->u.probe_resp.timestamp) {
#if (KERNEL_VERSION(5, 0, 0) < LINUX_VERSION_CODE)
		struct timespec64 tv;

		ktime_get_real_ts64(&tv);
		mgmt->u.probe_resp.timestamp = (((UINT64) tv.tv_sec * 1000000000) + tv.tv_nsec)/1000;
#else
		struct timeval tv;

		do_gettimeofday(&tv);
		mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_usec;
#endif
	}

	/* inform 80211 a scan is got */
	/* we can use cfg80211_inform_bss in 2.6.31, it is easy more than the one */
	/* in cfg80211_inform_bss_frame(), it will memcpy pFrame but pChan */
	bss = cfg80211_inform_bss_frame(pWiphy, &pBand->channels[IdChan],
									mgmt,	FrameLen,
									RSSI,	GFP_ATOMIC);

	if (unlikely(!bss)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> bss inform fail ==> %d\n", IdChan);
		return;
	}

	CFG80211OS_PutBss(pWiphy, bss);
#endif /* CONFIG_STA_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 *	Inform us that scan ends.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	FlgIsAborted	- 1: scan is aborted
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211OS_ScanEnd(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN FlgIsAborted)
{
	CFG80211_CB *pCfg80211_CB = pAd->pCfg80211_CB;

	NdisAcquireSpinLock(&pCfg80211_CB->scan_notify_lock);

	if (pCfg80211_CB->pCfg80211_ScanReq) {
		struct cfg80211_scan_info info = {
			.aborted = FlgIsAborted,
		};
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> cfg80211_scan_done\n");
		cfg80211_scan_done(pCfg80211_CB->pCfg80211_ScanReq, &info);

		pCfg80211_CB->pCfg80211_ScanReq = NULL;
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> cfg80211_scan_done ==> NULL\n");

	NdisReleaseSpinLock(&pCfg80211_CB->scan_notify_lock);

	pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Inform CFG80211 about association status.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pBSSID			- the BSSID of the AP
 *	pReqIe			- the element list in the association request frame
 *	ReqIeLen		- the request element length
 *	pRspIe			- the element list in the association response frame
 *	RspIeLen		- the response element length
 *	FlgIsSuccess	- 1: success; otherwise: fail
 *
 * Return Value:
 *	None
 * ========================================================================
 */
void CFG80211OS_ConnectResultInform(
	IN PNET_DEV pNetDev, IN UCHAR * pBSSID, IN UCHAR * pReqIe, IN UINT32 ReqIeLen,
	IN UCHAR * pRspIe, IN UINT32 RspIeLen, IN UCHAR FlgIsSuccess)
{
	UCHAR *tmpReqIe = ReqIeLen ? pReqIe : NULL;
	UCHAR *tmpRsqIe = RspIeLen ? pRspIe : NULL;

	if ((pNetDev == NULL) || (pBSSID == NULL))
		return;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
		"ReqIeLen:%d, RspIeLen:%d, FlgIsSuccess:%d\n", ReqIeLen, RspIeLen, FlgIsSuccess);

	cfg80211_connect_result(pNetDev, pBSSID, tmpReqIe, ReqIeLen,
			tmpRsqIe, RspIeLen, FlgIsSuccess, GFP_KERNEL);
}

BOOLEAN CFG80211OS_RxMgmt(IN PNET_DEV pNetDev, IN INT32 freq, IN PUCHAR frame, IN UINT32 len)
{
	struct wireless_dev *wdev = NULL;

	/* Sanity Check */
	if (pNetDev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"pNetDev == NULL\n");
		return FALSE;
	}

	wdev = pNetDev->ieee80211_ptr;
	/* Sanity Check */
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
		"pwireless dev == NULL\n");
		return FALSE;
	}

	/* we may check if netdev is running.*/
	if (!netif_running(pNetDev)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
		"net dev is not running.\n");
		return FALSE;
	}


	return cfg80211_rx_mgmt(pNetDev->ieee80211_ptr,
							freq,
							0,       /* CFG_TODO return 0 in dbm */
							frame,
							len,
							GFP_ATOMIC);
}

VOID CFG80211OS_TxStatus(IN PNET_DEV pNetDev, IN INT32 cookie, IN PUCHAR frame, IN UINT32 len, IN BOOLEAN ack)
{
	return cfg80211_mgmt_tx_status(pNetDev->ieee80211_ptr, cookie, frame, len, ack, GFP_ATOMIC);
}

VOID CFG80211OS_NewSta(IN PNET_DEV pNetDev,
			IN const PUCHAR mac_addr,
			IN const PUCHAR assoc_frame,
			IN UINT32 assoc_len,
			IN BOOLEAN isReassoc,
			IN const PUCHAR assoc_resp_frame,
			IN UINT32 assoc_resp_len,
			IN BOOLEAN mlo_params_valid,
			IN UINT8 mlo_link_id,
			IN const PUCHAR mld_addr)
{
	struct station_info *sinfo;
	struct ieee80211_mgmt *mgmt;

	os_alloc_mem(NULL, (UCHAR **)&sinfo, sizeof(struct station_info));

	if (!sinfo)
		return;

	os_zero_mem(sinfo, sizeof(struct station_info));

/* If get error here, be sure patch the cfg80211_new_sta.patch into kernel. */
	if (pNetDev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_ADHOC) {
		mgmt = (struct ieee80211_mgmt *) assoc_frame;
		if (isReassoc) {
			sinfo->assoc_req_ies_len = assoc_len - 24 - 10;
			sinfo->assoc_req_ies = mgmt->u.reassoc_req.variable;
		} else {
#ifdef PWHM_SUPPORT
			sinfo->assoc_req_ies_len = assoc_len;
			sinfo->assoc_req_ies = assoc_frame; /*Handled PWHM Case*/
#else
			sinfo->assoc_req_ies_len = assoc_len - 24 - 4;
			sinfo->assoc_req_ies = mgmt->u.assoc_req.variable;
#endif
		}

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
		if (assoc_resp_frame && (assoc_resp_len > 0)) {
			mgmt = (struct ieee80211_mgmt *) assoc_resp_frame;
			sinfo->assoc_resp_ies = mgmt->u.assoc_resp.variable;
			sinfo->assoc_resp_ies_len = assoc_resp_len - 24 - 6;
		}

		if (mlo_params_valid && mld_addr) {
			sinfo->assoc_link_id = mlo_link_id;
			sinfo->mlo_params_valid = TRUE;
			memcpy(sinfo->mld_addr, mld_addr, ETH_ALEN);
		}
#endif
	}

	cfg80211_new_sta(pNetDev, mac_addr, sinfo, GFP_ATOMIC);
	os_free_mem(sinfo);
}

#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
VOID CFG80211OS_Conn_Failed(IN PNET_DEV pNetDev, IN const PUCHAR mac_addr, IN UCHAR fail_reason)
{
	return cfg80211_conn_failed(pNetDev, mac_addr, fail_reason, GFP_ATOMIC);
}
#endif

VOID CFG80211OS_DelSta(IN PNET_DEV pNetDev, IN const PUCHAR mac_addr)
{
	return cfg80211_del_sta(pNetDev, mac_addr, GFP_ATOMIC);
}


VOID CFG80211OS_MICFailReport(PNET_DEV pNetDev, const PUCHAR src_addr, BOOLEAN unicast, INT key_id, const PUCHAR tsc)
{
	cfg80211_michael_mic_failure(pNetDev, src_addr,
								 (unicast ? NL80211_KEYTYPE_PAIRWISE : NL80211_KEYTYPE_GROUP),
								 key_id, tsc, GFP_ATOMIC);
}

VOID CFG80211OS_Roamed(
	PNET_DEV pNetDev, IN UCHAR *pBSSID,
	IN UCHAR *pReqIe, IN UINT32 ReqIeLen,
	IN UCHAR *pRspIe, IN UINT32 RspIeLen)
{
	struct cfg80211_roam_info roam_info = {
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
		.links[0].channel = NULL,
		.links[0].bssid = pBSSID,
#else
		.channel = NULL,
		.bssid = pBSSID,
#endif /* CFG_CFG80211_VERSION */
		.req_ie = pReqIe,
		.req_ie_len = ReqIeLen,
		.resp_ie = pRspIe,
		.resp_ie_len = RspIeLen,
	};
	cfg80211_roamed(pNetDev,
			&roam_info, GFP_KERNEL
	);
}

VOID CFG80211OS_EnableChanFlagsByBand(IN struct ieee80211_channel *pChannels,
				      IN UINT32 n_channels,
				      IN UINT32 freq_start_mhz,
				      IN UINT32 freq_end_mhz, IN UINT32 flags)
{
	INT32 idx = 0;

	if (!pChannels)
		return;

	for (idx = 0; idx < n_channels; idx++) {
		if ((pChannels[idx].center_freq >= (UINT16) freq_start_mhz) &&
		    (pChannels[idx].center_freq <= (UINT16) freq_end_mhz)) {
			/* If this is not disabled channel, we clear the flag of IEEE80211_CHAN_DISABLED */
				pChannels[idx].flags &= ~IEEE80211_CHAN_DISABLED;
		}
	}

}

VOID CFG80211OS_ForceUpdateChanFlagsByBand(IN struct ieee80211_supported_band *pBand,
					   IN struct ieee80211_channel *pChannelUpdate)
{
	struct ieee80211_channel *pChannels;
	INT32 idx = 0;

	if (!pBand || !pChannelUpdate)
		return;

	pChannels = pBand->channels;

	for (idx = 0; idx < pBand->n_channels; idx++) {
		if (pChannelUpdate[idx].flags & IEEE80211_CHAN_DISABLED)
			pChannels[idx].flags |= IEEE80211_CHAN_DISABLED;
		else
			pChannels[idx].flags &= ~IEEE80211_CHAN_DISABLED;
	}
}

static VOID cfg80211_clear_band_info(IN VOID *pCB, UCHAR band_idx)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *) pCB;
	struct ieee80211_supported_band tmpBand;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
		"band_idx=%d\n", band_idx);
	memcpy(&tmpBand, &pCfg80211_CB->Cfg80211_bands[band_idx], sizeof(tmpBand));
	memset(&pCfg80211_CB->Cfg80211_bands[band_idx], 0, sizeof(pCfg80211_CB->Cfg80211_bands[band_idx]));
	pCfg80211_CB->pCfg80211_Wdev->wiphy->bands[band_idx] = NULL;

	tmpBand.channels = NULL;
	tmpBand.bitrates = NULL;
}

INT32 CFG80211OS_UpdateRegRuleByRegionIdx(
IN VOID * pCB, IN VOID * pChDesc2G, IN VOID * pChDesc5G, IN VOID * pChDesc6G)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *) pCB;
	struct wiphy *pWiphy = NULL;
	UINT32 freq_start_mhz = 0, freq_end_mhz = 0;
#ifdef EXT_BUILD_CHANNEL_LIST
	PCH_DESP pChDesc = NULL;
#else
	PCH_DESC pChDesc = NULL;
#endif
	INT32 n_channels = 0;
	INT32 ii = 0;
	struct ieee80211_supported_band *pSband;
	UCHAR band_idx = IEEE80211_BAND_2GHZ;
	UCHAR chn_idx_step = 1;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
		"pChDesc2G=%p, pChDesc5G=%p, pChDesc6G=%p\n",
		pChDesc2G, pChDesc5G, pChDesc6G);

	if (!pCB || (!pChDesc2G && !pChDesc5G && !pChDesc6G)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> invalid input!!\n");
		return -EINVAL;
	}

	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	if (!pWiphy) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> invalid pWiphy!!\n");
		return -EFAULT;
	}

	if (pChDesc2G) {
#ifdef EXT_BUILD_CHANNEL_LIST
		pChDesc = (PCH_DESP) pChDesc2G;
#else
		pChDesc = (PCH_DESC) pChDesc2G;
#endif

		cfg80211_clear_band_info(pCB, IEEE80211_BAND_5GHZ);
		cfg80211_clear_band_info(pCB, IEEE80211_BAND_6GHZ);

	} else if (pChDesc5G) {
		band_idx = IEEE80211_BAND_5GHZ;
		chn_idx_step = 4;

#ifdef EXT_BUILD_CHANNEL_LIST
		pChDesc = (PCH_DESP) pChDesc5G;
#else
		pChDesc = (PCH_DESC) pChDesc5G;
#endif

		cfg80211_clear_band_info(pCB, IEEE80211_BAND_2GHZ);
		cfg80211_clear_band_info(pCB, IEEE80211_BAND_6GHZ);
	} else if (pChDesc6G) {
		band_idx = IEEE80211_BAND_6GHZ;
		chn_idx_step = 4;

#ifdef EXT_BUILD_CHANNEL_LIST
		pChDesc = (PCH_DESP) pChDesc6G;
#else
		pChDesc = (PCH_DESC) pChDesc6G;
#endif

		cfg80211_clear_band_info(pCB, IEEE80211_BAND_2GHZ);
		cfg80211_clear_band_info(pCB, IEEE80211_BAND_5GHZ);
	}

	n_channels = pCfg80211_CB->Cfg80211_bands[band_idx].n_channels;
	if (pChDesc && n_channels > 0) {
		struct ieee80211_channel *pTmpCh = NULL;

		os_alloc_mem(NULL, (UCHAR **)&pTmpCh,
			sizeof(struct ieee80211_channel) * n_channels);
		if(pTmpCh == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"alloc memory for pTmpCh fail\n");
			return -ENOMEM;
		}

		memset(pTmpCh, 0, sizeof(struct ieee80211_channel) * n_channels);
		/* init all channels to be disabled */
		for (ii = 0; ii < n_channels; ii++) {
			pTmpCh[ii].flags |= IEEE80211_CHAN_DISABLED;
			pTmpCh[ii].center_freq =
			    pCfg80211_CB->Cfg80211_bands[band_idx].channels[ii].center_freq;
		}
		while (pChDesc && pChDesc->FirstChannel) {
			freq_start_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel,
							   band_idx);
			freq_end_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel +
							   (pChDesc->NumOfCh - 1) * chn_idx_step,
							   band_idx);
#ifdef EXT_BUILD_CHANNEL_LIST
			CFG80211OS_EnableChanFlagsByBand(pTmpCh, n_channels, freq_start_mhz,
							 freq_end_mhz,
							 0);
#else
			CFG80211OS_EnableChanFlagsByBand(pTmpCh, n_channels, freq_start_mhz,
							 freq_end_mhz,
							 (UINT32) pChDesc->ChannelProp);
#endif
			pChDesc++;
		}
		pSband = pWiphy->bands[band_idx];
		CFG80211OS_ForceUpdateChanFlagsByBand(pSband, pTmpCh);
		os_free_mem(pTmpCh);
	}

	return 0;
}

#ifdef APCLI_CFG80211_SUPPORT
#define AKM_SUITE_SAE 0x08ac0f00
int CFG80211OS_ExternalAuthRequest(PSTA_ADMIN_CONFIG pStaCfg)
{
	int ret, len;
	struct cfg80211_external_auth_params auth = {0};

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR, "80211> ==>\n");

	if (!pStaCfg)
		return -1;

	if (pStaCfg->CfgSsidLen) {
		len = clamp_val(pStaCfg->CfgSsidLen, 0, MAX_LEN_OF_SSID);
		memcpy(auth.ssid.ssid, pStaCfg->CfgSsid, len);
		auth.ssid.ssid_len = len;
	}

	auth.key_mgmt_suite = AKM_SUITE_SAE;
	auth.action = NL80211_EXTERNAL_AUTH_START;
	NdisCopyMemory(auth.bssid, pStaCfg->CfgApCliBssid, MAC_ADDR_LEN);

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_DEBUG,
		"%s, external SAE processing: bssid=%pM action=%u akm=0x%x\n",
		pStaCfg->wdev.if_dev->name, auth.bssid, auth.action, auth.key_mgmt_suite);

	ret = cfg80211_external_auth_request(pStaCfg->wdev.if_dev, &auth, GFP_KERNEL);
	if (ret)
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
				"failed to offload external auth request\n");
	return ret;
}
#endif

#ifdef DOT11_EHT_BE
int CFG80211OS_VendorEventMloResponse(
	struct _RTMP_ADAPTER *pAd, uint8_t *data, uint32_t len)
{
	struct sk_buff *skb;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;

	pCfg80211_CB = pAd->pCfg80211_CB;
	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
			len, MTK_NL80211_VENDOR_EVENT_MLO_EVENT, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"allocate skb failed\n");
		return -ENOMEM;
	}

	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, len, data)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -1;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"send vendor response success.\n");
	return 0;
}

int CFG80211OS_MloExternalAuthRequest(struct _RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	struct wiphy *wiphy = NULL;
	struct wireless_dev *wdev = NULL;
	struct PARAM_EXTERNAL_AUTH_INFO *info = NULL;
	MAC_TABLE_ENTRY *entry = NULL;
	ULONG size = 0, len = 0;
	CFG80211_CB *pCfg80211_CB = NULL;

	pCfg80211_CB = pAd->pCfg80211_CB;
	wdev = pCfg80211_CB->pCfg80211_Wdev;
	wiphy = wdev->wiphy;

	size = sizeof(struct PARAM_EXTERNAL_AUTH_INFO) + MAX_LEN_OF_MLIE;
	if (os_alloc_mem(pAd, (UCHAR **)&info, size) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"alloc vendor external auth event fail\n");
		return -1;
	}
	NdisZeroMemory(info, size);
	info->id = MTK_GRID_MLO_EXTERNAL_AUTH;
	info->len = sizeof(*info) - 2;
	info->action = NL80211_EXTERNAL_AUTH_START;

	entry = MacTableLookup(pAd, pStaCfg->CfgApCliBssid);
	if (!entry) {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"can't find the CfgApCliBssid("MACSTR") ENTRY. FAIL\n",
				MAC2STR(pStaCfg->CfgApCliBssid));
		os_free_mem(info);
		return -1;
	}

	NdisCopyMemory(info->bssid, pStaCfg->CfgApCliBssid, MAC_ADDR_LEN);
	if (entry->mlo.mlo_en) {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_WARN,
				"CfgApCliBssid("MACSTR") ENTRY. MLD PEER MAC("MACSTR").\n",
				MAC2STR(pStaCfg->CfgApCliBssid), MAC2STR(entry->mlo.mld_addr));
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"CfgApCliBssid("MACSTR") ENTRY.\n",
				MAC2STR(pStaCfg->CfgApCliBssid));
	}
	len = clamp_val(pStaCfg->CfgSsidLen, 0, MAX_LEN_OF_SSID);
	info->ssid_len = len;
	NdisCopyMemory(info->ssid, pStaCfg->CfgSsid, len);
	info->key_mgmt_suite = AKM_SUITE_SAE;
	COPY_MAC_ADDR(info->da, entry->Addr);
	/*add eht ies*/
	len = eht_add_auth_req_ies(&pStaCfg->wdev, info->ext_ie, 0);
	info->len += len;
	hex_dump("\n sae external eht_add_auth_req_ies:", info->ext_ie, len);

	CFG80211OS_VendorEventMloResponse(pAd, (uint8_t *)info, (uint32_t)(sizeof(*info) + len));

	os_free_mem(info);

	return 0;
}
#endif /* DOT11_EHT_BE */

#ifdef CFG_RSNO_SUPPORT
BOOLEAN CFG80211_handle_rsne_override(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	unsigned char *ie,
	unsigned int ie_len)
{
	const UCHAR *rsne_override_ie = NULL;
	struct _SECURITY_CONFIG *sec_config;

	rsne_override_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, OUI_WFA_RSNE_OVERRIDE,
			ie, ie_len);
	sec_config = &wdev->SecConfig;
	sec_config->rsneo_len = 0;
	NdisZeroMemory(sec_config->rsneo_content, MAX_LEN_OF_RSNIE);
	if (rsne_override_ie != NULL) {
		EID_STRUCT *eid;

		MTWF_DBG(mac_ad, DBG_CAT_SEC, CATSEC_RSNO, DBG_LVL_INFO,
				"RSNE Override IE(=%d)\n", ie_len);
		eid = (EID_STRUCT *)rsne_override_ie;
		if (eid->Len + 2 <= MAX_LEN_OF_RSNIE) {
			NdisCopyMemory(sec_config->rsneo_content, rsne_override_ie, eid->Len+2);
			sec_config->rsneo_len = eid->Len + 2;
			return TRUE;
		}
	}
	return FALSE;
}

void CFG80211_handle_rsnxe_override(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	unsigned char *ie,
	unsigned int ie_len)
{
	const UCHAR *rsnxe_override_ie = NULL;
	struct _SECURITY_CONFIG *sec_config;

	rsnxe_override_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, OUI_WFA_RSNXE_OVERRIDE,
			ie, ie_len);
	sec_config = &wdev->SecConfig;
	NdisZeroMemory(sec_config->rsnxeo_content, MAX_LEN_OF_RSNXEIE);
	sec_config->rsnxeo_len = 0;
	if (rsnxe_override_ie != NULL) {
		EID_STRUCT *eid;

		MTWF_DBG(mac_ad, DBG_CAT_SEC, CATSEC_RSNO, DBG_LVL_INFO,
				"RSNXE Override IE(=%d)\n", ie_len);
		eid = (EID_STRUCT *)rsnxe_override_ie;
		if (eid->Len + 2 <= MAX_LEN_OF_RSNXEIE) {
			NdisCopyMemory(sec_config->rsnxeo_content, rsnxe_override_ie, eid->Len+2);
			sec_config->rsnxeo_len = eid->Len + 2;
		}
	}
}
#endif /* CFG_RSNO_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
