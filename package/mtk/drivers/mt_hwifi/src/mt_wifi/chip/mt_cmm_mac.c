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

*/

#include "rt_config.h"

UCHAR tmi_rate_map_cck_lp[] = {
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_lp_size = ARRAY_SIZE(tmi_rate_map_cck_lp);

UCHAR tmi_rate_map_cck_sp[] = {
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_cck_sp_size = ARRAY_SIZE(tmi_rate_map_cck_sp);

UCHAR tmi_rate_map_ofdm[] = {
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};

UCHAR tmi_rate_map_ofdm_size = ARRAY_SIZE(tmi_rate_map_ofdm);

UCHAR cck_fr_rate_idx_map[] = {
/* 0-3: CCK Long Preamble */
	FR_CCK_SPE0x18_1M,
	FR_CCK_SPE0x18_2M,
	FR_CCK_SPE0x18_5_5M,
	FR_CCK_SPE0x18_11M,
	0,
/* 5-7: CCK Short Preamble */
	FR_CCKS_SPE0x18_2M,
	FR_CCKS_SPE0x18_5_5M,
	FR_CCKS_SPE0x18_11M,
};

UCHAR ofdm_fr_rate_idx_map[] = {
/* 0-7: OFDM */
	FR_OFDM_SPE0x18_6M,
	FR_OFDM_SPE0x18_9M,
	FR_OFDM_SPE0x18_12M,
	FR_OFDM_SPE0x18_18M,
	FR_OFDM_SPE0x18_24M,
	FR_OFDM_SPE0x18_36M,
	FR_OFDM_SPE0x18_48_5M,
	FR_OFDM_SPE0x18_54M
};

UCHAR transmit_to_fr_idx(UINT32 transmit)
{
	UCHAR phymode = 0, mcs = 0, tmpfrIdx = FR_OFDM_6M;
	union _HTTRANSMIT_SETTING tmpTransmit = {0};

	tmpTransmit.word = transmit;
	phymode = tmpTransmit.field.MODE;
	mcs = tmpTransmit.field.MCS;

	switch (phymode) {
	case MODE_CCK: /* CCK */
		tmpfrIdx = cck_fr_rate_idx_map[mcs];

		break;

	case MODE_OFDM:	/* OFDM */
		tmpfrIdx = ofdm_fr_rate_idx_map[mcs];

		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				 "unknown PhyMode/Mcs %d/%d.\n", phymode, mcs);
		break;
	}

	return tmpfrIdx;
}

const UCHAR altx_filter_list[] = {
	SUBTYPE_ASSOC_REQ,
	SUBTYPE_ASSOC_RSP,
	SUBTYPE_REASSOC_REQ,
	SUBTYPE_REASSOC_RSP,
	SUBTYPE_PROBE_REQ,
	SUBTYPE_PROBE_RSP,
	SUBTYPE_ATIM,
	SUBTYPE_DISASSOC,
	SUBTYPE_AUTH,
	SUBTYPE_DEAUTH,
};

char *pkt_ft_str[] = {"cut_through", "store_forward", "cmd", "PDA_FW_Download"};
char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};

char *rmac_info_type_str[] = {
	"TXS",
	"RXV",
	"RxNormal",
	"DupRFB",
	"TMR",
	"Unknown",
};

char *rxd_pkt_type_str(INT pkt_type)
{
	if (pkt_type >= 0 && pkt_type <= 0x04)
		return rmac_info_type_str[pkt_type];
	else
		return rmac_info_type_str[5];
}

BOOLEAN in_altx_filter_list(HEADER_802_11 *pHeader)
{
	UCHAR i;
	UCHAR sub_type = pHeader->FC.SubType;

	for (i = 0; i < (ARRAY_SIZE(altx_filter_list)); i++) {
		if (sub_type == altx_filter_list[i])
			return TRUE;
	}
	if (sub_type == SUBTYPE_ACTION) {
		UINT8 *ptr = (UINT8 *)pHeader;

		ptr += sizeof(HEADER_802_11);
		if (*ptr == CATEGORY_PUBLIC ||
			((*ptr == CATEGORY_BA) && (*(ptr+1) == ADDBA_RESP)) ||
			((*ptr == CATEGORY_SPECTRUM) && (*(ptr+1) == SPEC_CHANNEL_SWITCH)))

			return TRUE;
	}

	return FALSE;
}

BOOLEAN valid_broadcast_mgmt_in_acq(HEADER_802_11 *pHeader)
{
	UINT8 *ptr = (UINT8 *)pHeader;
	UCHAR sub_type = pHeader->FC.SubType;

	if (!MAC_ADDR_EQUAL(pHeader->Addr1, BROADCAST_ADDR))
		return FALSE;

	if (sub_type == SUBTYPE_ACTION_NO_ACK) {
		ptr += sizeof(HEADER_802_11);
		if ((*ptr == CATEGORY_PROTECTED_EHT)
			&& (*(ptr+1) == EHT_PROT_ACT_LINK_RECOMM))
			return TRUE;
	}

	return FALSE;
}

#ifdef CUT_THROUGH
INT mt_ct_get_hw_resource_free_num(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx, UINT32 *free_num, UINT32 *free_token)
{
	UINT free_ring_num, free_token_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	PKT_TOKEN_CB *cb = hif->PktTokenCb;
	UINT8 band_idx = 0;
	struct token_tx_pkt_queue *que = NULL;

	if (wdev)
		band_idx = HcGetBandByWdev(wdev);
	else
		band_idx = 0;

	que = token_tx_get_queue_by_band(cb, band_idx);

	free_ring_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);

	if (free_ring_num < tx_ring->tx_ring_low_water_mark) {
		pci_inc_resource_full_cnt(pAd, resource_idx);
		pci_set_resource_state(pAd, resource_idx, TX_RING_LOW);
		return NDIS_STATUS_RESOURCES;
	}
	free_ring_num = free_ring_num - tx_ring->tx_ring_low_water_mark + 1;
	free_token_num = token_tx_get_free_cnt(que);

	if (free_token_num < token_tx_get_lwmark(que)) {
		token_tx_inc_full_cnt(que);
		token_tx_set_state(que, TX_TOKEN_LOW);
		return NDIS_STATUS_FAILURE;
	}

	free_token_num = free_token_num - token_tx_get_lwmark(que) + 1;

	/* return free num which can be used */
	*free_num = (free_ring_num < free_token_num) ? free_ring_num:free_token_num;
	*free_token = free_token_num;
	return NDIS_STATUS_SUCCESS;
}
#endif

/*
*
*/
VOID ComposePsPoll(
	IN	RTMP_ADAPTER *pAd,
	IN	PPSPOLL_FRAME pPsPollFrame,
	IN	USHORT	Aid,
	IN	UCHAR *pBssid,
	IN	UCHAR *pTa)
{
	NdisZeroMemory(pPsPollFrame, sizeof(PSPOLL_FRAME));
	pPsPollFrame->FC.Type = FC_TYPE_CNTL;
	pPsPollFrame->FC.SubType = SUBTYPE_PS_POLL;
	pPsPollFrame->Aid = Aid | 0xC000;
	COPY_MAC_ADDR(pPsPollFrame->Bssid, pBssid);
	COPY_MAC_ADDR(pPsPollFrame->Ta, pTa);
}

VOID write_tmac_info_offload_pkt(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR type,
	UCHAR sub_type,
	UCHAR *tmac_buf,
	union _HTTRANSMIT_SETTING *TransmitSet,
	ULONG frmLen)
{
	MAC_TX_INFO mac_info;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	mac_info.Type = type;
	mac_info.SubType = sub_type;
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wdev->bss_info_argument.bmc_wlan_idx;
	/* SW_CONNECT_SUPPORT */
	/* asic_write_tmac_info_fixed_rate() will ref the pEntry/tr_entry again */
	mac_info.SW_WCID = wdev->tr_tb_idx;

	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = wdev->func_idx;
	mac_info.SpeEn = 1;
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;
	mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;

	if ((type == FC_TYPE_MGMT) && (sub_type == SUBTYPE_BEACON)) {
		mac_info.NSeq = TRUE;
		mac_info.q_idx = MAC_TXQ_IDX_BCN;
#ifdef BCN_PROTECTION_SUPPORT
		if (chip_cap->bcn_prot_sup == BCN_PROT_EN_HW_MODE) /* bcn prot using HW mode */
			mac_info.prot = 2;
#endif /* BCN_PROTECTION_SUPPORT */
	} else {
		/* HW SN and ALTX for non-beacon frame */
		mac_info.NSeq = FALSE;
		mac_info.q_idx = MAC_TXQ_IDX_ALTX0;
	}

	mac_info.IsOffloadPkt = TRUE;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	asic_write_tmac_info_fixed_rate(pAd, wdev, tmac_buf, &mac_info, TransmitSet);

#ifdef CFG_BIG_ENDIAN
	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_buf, TYPE_TXWI, sizeof(TMAC_TXD_L));
#endif
}

enum entry_wcid_type check_entry_wcid_type(
	RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry)
{
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	if (entry && IS_ENTRY_PEER_AP(entry)) {
#ifdef WARP_512_SUPPORT
		if (pAd->Warp512Support)
			return WCID_MATCH_SHIFT;
		else
#endif /* WARP_512_SUPPORT */
			return WCID_MATCH_NORMAL;
	}
#endif /* APCLI_SUPPORT || CONFIG_STA_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	if (entry && IS_ENTRY_REPEATER(entry))
		return WCID_MATCH_NORMAL;
#endif
#if  (defined(MWDS)) || (defined(A4_CONN))
	if (entry && IS_ENTRY_A4(entry)) {
#ifdef WARP_512_SUPPORT
		if (pAd->Warp512Support)
			return WCID_MATCH_SHIFT;
		else
#endif /* WARP_512_SUPPORT */
			return WCID_MATCH_NORMAL;
	}
#endif /* MWDS */
#ifdef MBSS_AS_WDS_AP_SUPPORT
#ifdef CLIENT_WDS
	if (entry && IS_ENTRY_CLIWDS(entry))
		return WCID_MATCH_NORMAL;
#endif
#endif
#ifdef WDS_SUPPORT
	if (entry && IS_ENTRY_WDS(entry))
		return WCID_MATCH_NORMAL;
#endif	/* WDS_SUPPORT */

	return WCID_NO_MATCH;
}

