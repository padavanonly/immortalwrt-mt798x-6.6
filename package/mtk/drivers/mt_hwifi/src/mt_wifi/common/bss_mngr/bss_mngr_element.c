/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#include "bss_mngr.h"

extern struct bss_manager bss_mngr;

#ifdef DOT11V_MBSSID_SUPPORT

static bool has_rsn_ie(struct bmgr_entry *entry)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;

	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"11v tx entry null!!!\n");
		return FALSE;
	}

	wdev = RTMP_OS_NETDEV_GET_WDEV(entry->pNetDev);
	pSecConfig = &wdev->SecConfig;

	if (pSecConfig->RSNE_Type[1] == SEC_RSNIE_WPA2_IE)
		return TRUE;

	return FALSE;
}

#ifdef DOT11_SAE_SUPPORT
static bool has_rsnxe_ie(struct bmgr_entry *entry)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;

	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"11v tx entry null!!!\n");
		return FALSE;
	}

	wdev = RTMP_OS_NETDEV_GET_WDEV(entry->pNetDev);
	pSecConfig = &wdev->SecConfig;

	if (!IS_AKM_SAE(pSecConfig->AKMMap))
		return FALSE;

	if (IS_AKM_SAE(pSecConfig->AKMMap) &&
		pSecConfig->sae_cap.gen_pwe_method != PWE_LOOPING_ONLY)
		return TRUE;

	return FALSE;
}
#endif

/*add non-inherit ie*/
/*TBD: at now, we just take rsn/rsnxe as non-inherit ie
	other ies need refine later*/
static u16 build_non_tx_bssid_non_inherit_ie(
struct bmgr_entry *entry,
u8 *f_buf,
u8 rsn_non_inherit,
u8 rsnxe_non_inherit)
{
	u8 *pos = f_buf;
	ULONG tmp_len;
	u16 ie_len = 0;
	struct _EID_STRUCT *non_inherit_ie = (struct _EID_STRUCT *)f_buf;
	u8 non_inherit_ie_list[100] = {0};
	u8 non_inherit_ie_ext_list[100] = {0};
	u8 non_inherit_ie_ext_list_len = 0;
	u8 non_inherit_ie_list_len = 0;
	u8 ie_cnt = 0;

	if (rsn_non_inherit) {
		non_inherit_ie_list[ie_cnt] = IE_RSN;
		ie_cnt++;
	}

	if (rsnxe_non_inherit) {
		non_inherit_ie_list[ie_cnt] = IE_RSNXE;
		ie_cnt++;
	}
	non_inherit_ie_list_len = ie_cnt;

	/*make the ie*/
	non_inherit_ie->Eid = IE_WLAN_EXTENSION;
	non_inherit_ie->Octet[0] = IE_EXTENSION_ID_NON_INHERITANCE;

	pos += sizeof(struct _EID_STRUCT);
	MakeOutgoingFrame(pos, &tmp_len,
					  1,								&non_inherit_ie_list_len,
					  non_inherit_ie_list_len,		non_inherit_ie_list,
					  1,								&non_inherit_ie_ext_list_len,
					  non_inherit_ie_ext_list_len,	non_inherit_ie_ext_list,
					  END_OF_ARGS);

	non_inherit_ie->Len = tmp_len + 1;

	ie_len = non_inherit_ie->Len + 2;

	return ie_len;
}

/*add rsnxe ie for non-tx bss*/
static u16 build_non_tx_bssid_rsnxe_ie(struct bmgr_entry *entry, u8 *f_buf)
{
	u8 *pos = f_buf;
	int extend_length = 0;
	u8 ie = IE_RSNXE;
	u8 ie_len = 1;
	u8 cap = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	struct wifi_dev *wdev = NULL;

	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"11v non-t entry null!!!\n");
		return 0;
	}

	wdev = RTMP_OS_NETDEV_GET_WDEV(entry->pNetDev);
	sec_cfg = &wdev->SecConfig;

	/* remove it if any other authmode also use rsnxe */
	if (!IS_AKM_SAE(sec_cfg->AKMMap))
		return 0;

#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(sec_cfg->AKMMap) &&
		sec_cfg->sae_cap.gen_pwe_method != PWE_LOOPING_ONLY)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_H2E);

	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.sae_pk_en != SAE_PK_DISABLE)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_PK);
#endif

	if (cap == 0)
		return 0;

	NdisMoveMemory(pos + extend_length, &ie, sizeof(ie));
	extend_length += sizeof(ie);
	NdisMoveMemory(pos + extend_length, &ie_len, sizeof(ie_len));
	extend_length += sizeof(ie_len);
	NdisMoveMemory(pos + extend_length, &cap, sizeof(cap));
	extend_length += sizeof(cap);

	return extend_length;
}

/*add rsn ie for non-tx bss*/
static u16 build_non_tx_bssid_rsn_ie(struct bmgr_entry *entry, u8 *f_buf)
{
	u8 *pos = f_buf;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;
	CHAR rsne_idx = 0;
	u16 len = 0;

	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"11v non-t entry null!!!\n");
		return 0;
	}

	wdev = RTMP_OS_NETDEV_GET_WDEV(entry->pNetDev);
	pSecConfig = &wdev->SecConfig;

	for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
		if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
			continue;

		MAKE_IE_TO_BUF(pos, &pSecConfig->RSNE_EID[rsne_idx][0], 1, len);
		MAKE_IE_TO_BUF(pos, &pSecConfig->RSNE_Len[rsne_idx], 1, len);
		MAKE_IE_TO_BUF(pos, &pSecConfig->RSNE_Content[rsne_idx][0],
					   pSecConfig->RSNE_Len[rsne_idx], len);
	}

	return len;
}

/*add wmm ie for non-tx bss*/
/*TBD: need check the wmm inherit issue later*/
static u8 *build_non_tx_bssid_wmm_cap_ie(struct bmgr_entry *entry, u8 *f_buf)
{
	u32 len = 0;
	u8 i;
	u8 WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
	u8 AIFSN[4] = {0};
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	u8 *pos = f_buf;

	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"11v non-t entry null!!!\n");
		return pos;
	}

	wdev = RTMP_OS_NETDEV_GET_WDEV(entry->pNetDev);

	if (!wdev->bWmmCapable)
		return pos;

	pBssEdca = wlan_config_get_ht_edca(wdev);

	if (pBssEdca) {
		WmeParmIe[8] = pBssEdca->EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
		UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif
		NdisMoveMemory(AIFSN, pBssEdca->Aifsn, sizeof(AIFSN));

		for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
			WmeParmIe[10 + (i * 4)] = (i << 5) +
					((UCHAR)pBssEdca->bACM[i] << 4) +
					(AIFSN[i] & 0x0f);
			WmeParmIe[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4) +
					(pBssEdca->Cwmin[i] & 0x0f);
			WmeParmIe[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);
			WmeParmIe[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);
		}

		MAKE_IE_TO_BUF(pos, WmeParmIe, 26, len);
	}

	pos += len;

	return pos;
}

/*
 * 9.4.2.72 Nontransmitted BSSID Capability element, and SSID
 */
static u8 *build_non_tx_bssid_cap_ie(struct bmgr_entry *entry, u8 *f_buf, bool is_probe, u8 **cap_info_ptr)
{
	u8 *pos = f_buf;
	ULONG TempLen;
	u8 NontransmittedBssidCapIe = IE_NONTRANSMITTED_BSSID_CAP;
	u8 ie_len = 0;
	u8 *pSsid = NULL;
	u16 CapabilityInfo;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "%s\n", __func__);

	/* Capability element */
	ie_len = 2;
	CapabilityInfo = cpu2le16(entry->entry_info.cap_info);
	MakeOutgoingFrame(pos, &TempLen,
					1, &NontransmittedBssidCapIe,
					1, &ie_len,
					2, &CapabilityInfo,
					END_OF_ARGS);
	/* store cap_info offset for BSS Para. critical update */
	if (!is_probe)
		*cap_info_ptr = pos;

	pos += TempLen;

	/* SSID element */
	ie_len = (entry->entry_info.is_hide_ssid && !is_probe) ? 0 : entry->entry_info.ssid_len;
	pSsid = entry->entry_info.ssid;
	MakeOutgoingFrame(pos, &TempLen,
					1, &SsidIe,
					1, &ie_len,
					ie_len, pSsid,
					END_OF_ARGS);
	pos += TempLen;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"\tCap/SSID End Len = %d\n", (u32)(pos - f_buf));
	return pos;
}

/*
 * 9.4.2.74 Multiple BSSID-Index element
 */
static u8 *build_non_tx_bssid_idx_ie(struct bmgr_entry *entry, u8 *f_buf, bool is_probe, u8 **tim_ie_ptr)
{
	u8 *pos = f_buf;
	ULONG TempLen;
	u8 NontransmittedBssidIdxIe = IE_MULTIPLE_BSSID_IDX;
	u8 ie_len;
	u8 mbssid_idx = entry->entry_info.dot11v_mbssid_idx;
	u8 dtim_period = entry->entry_info.dtim_period;
	u8 dtim_count = dtim_period - 1;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"%s: 11v_mbssid_idx = %d\n", __func__, mbssid_idx);

	/* Len = 1 when IE is included in the Probe.Rsp, otherwise Len = 3 */
	if (is_probe) {
		ie_len = 1;
		MakeOutgoingFrame(pos, &TempLen,
						1, &NontransmittedBssidIdxIe,
						1, &ie_len,
						1, &mbssid_idx,
						END_OF_ARGS);
	} else {
		ie_len = 3;
		MakeOutgoingFrame(pos, &TempLen,
						1, &NontransmittedBssidIdxIe,
						1, &ie_len,
						1, &mbssid_idx,
						1, &dtim_period,
						1, &dtim_count,
						END_OF_ARGS);

		/* store Tim IE offset */
		*tim_ie_ptr = pos;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"tim_ie_ptr(%p)\n", *tim_ie_ptr);
	}
	pos += TempLen;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\tbssid_idx End Len = %d\n", (u32)(pos - f_buf));

	return pos;
}

/*
 * 9.4.2.45 Optional subelement IDs for Multiple BSSID
 */
static u8 *build_non_tx_bssid_sub_ie(
	struct bmgr_entry *entry,
	u8 *f_buf,
	bool is_probe,
	bool need_ml,
	u8 **tim_ie_ptr,
	u8 **mlt_ie_ptr,
	u8 **cap_info_ptr)
{
	u8 *pos = f_buf;
	struct multi_bssid_sub_ie *pmbss_sub_ie = (struct multi_bssid_sub_ie *)f_buf;
	u8 ie_len = 0;
	u8 sub_ie_len = 0;
	struct bmgr_entry *tx_entry = NULL;
	u8 rsn_non_inherit = FALSE;
	u8 rsnxe_non_inherit = FALSE;
	struct _RTMP_ADAPTER *ad = NULL;


	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "%s\n", __func__);

	ad = (struct _RTMP_ADAPTER *)entry->pwdev->sys_handle;

	/* Nontransmitted BSSID Profile */
	pmbss_sub_ie->sub_eid = SUB_IE_NON_TRANS_PROFILE;

	/* move pointer to NonTransProfiles */
	pos += sizeof(struct multi_bssid_sub_ie);

	/* make Nontransmitted BSSID Profile */
	pos = build_non_tx_bssid_cap_ie(entry, pos, is_probe, cap_info_ptr);
	pos = build_non_tx_bssid_idx_ie(entry, pos, is_probe, tim_ie_ptr);

#ifdef DOT11_EHT_BE
	/* include basic ml ie in MBSSID IE or not */
	if (need_ml) {
		pos = build_multi_link_ie(entry, pos, BMGR_MLD_BCN_COMMON, BMGR_MAX_MLD_STA_CNT);
		pos = build_multi_link_traffic_ie(entry, pos, mlt_ie_ptr);
	}
#endif

	if (ad->CommonCfg.wifi_cert)
		pos = build_non_tx_bssid_wmm_cap_ie(entry, pos);

	/* TBD - others subIEs for Nontransmitted BSSID Profile */
	if (BMGR_VALID_BSS_IDX(entry->bss_idx_tx)) {
		tx_entry = GET_BSS_ENTRY_BY_IDX(entry->bss_idx_tx);
	}


	/* check security is different from Tx-BSS, then add sub ie */
	if (tx_entry) {
		if (entry->entry_info.auth_mode != tx_entry->entry_info.auth_mode) {
			sub_ie_len = build_non_tx_bssid_rsn_ie(entry, pos);
			pos += sub_ie_len;

			/*do not inherit tx_entry's rsn ie*/
			if (!sub_ie_len && has_rsn_ie(tx_entry))
				rsn_non_inherit = TRUE;

			sub_ie_len = build_non_tx_bssid_rsnxe_ie(entry, pos);
			pos += sub_ie_len;

			/*do not inherit tx_entry's rsnxe ie*/
			if (!sub_ie_len && has_rsnxe_ie(tx_entry))
				rsnxe_non_inherit = TRUE;

			sub_ie_len = build_non_tx_bssid_non_inherit_ie(entry, pos, rsn_non_inherit, rsnxe_non_inherit);
			pos += sub_ie_len;
		}
	}

	/* TBD WPS*/
	/* TBD - Vendor Specific */

	/* eid_length */
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		pmbss_sub_ie->len = ie_len;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\tMbss Sub-IE Len = %d\n", pmbss_sub_ie->len);

	return pos;
}

/*
 * 9.4.2.45 Multiple BSSID element
 */
u8 *build_multi_bssid_ie(struct bmgr_entry *entry, u8 *f_buf, bool is_probe, bool need_ml, u8 **tim_ie_ptr, u8 **mlt_ie_ptr, u8 **cap_info_ptr)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf;
	u8 ie_len = 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"%s: type(%s) %s need_ml\n", __func__,
			is_probe ? "probe_rsp" : "bcn",
			need_ml ? "w/" : "w/o");

	if (eid) {
		eid->Eid = IE_MULTIPLE_BSSID;
		eid->Octet[0] = entry->entry_info.max_bssid_indicator;

		/* move pointer to mbss sub-ies */
		pos += sizeof(struct _EID_STRUCT);

		/* build optional sub-ies */
		pos = build_non_tx_bssid_sub_ie(entry, pos, is_probe, need_ml, tim_ie_ptr, mlt_ie_ptr, cap_info_ptr);

		/* eid_length */
		ie_len = pos - f_buf - 2;
		if (ie_len != 0)
			eid->Len = ie_len;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: Final mbss Len = %d\n", __func__, eid->Len + 2);
	}

	return pos;
}
#endif

u8 *build_rnr_ie(struct bmgr_entry *repting_entry, struct bmgr_entry *repted_entry, u8 *f_buf, u8 mld_id)
{
	u8 tbtt_info_len_sel;
	u8 tbtt_info_cnt = 1;
	struct rnr_tbtt_info_bmap {
		u8 tbtt_info_len;
		u8 nap_tbtt_offset:1;
		u8 bssid:1;
		u8 s_ssid:1;
		u8 bss_param:1;
		u8 psd_bw20:1;
		u8 mld_param:1;
		u8 dummy:1;
	} rnr_bmap[] = {
		/*len nap bss s_s b_p psd mld dmy */
		{  1,  1,  0,  0,  0,  0,  0,  0 },
		{  2,  1,  1,  0,  0,  0,  0,  0 },
		{  4,  1,  0,  0,  0,  0,  1,  0 },
		{  5,  1,  0,  1,  0,  0,  0,  0 },
		{  6,  1,  0,  1,  1,  0,  0,  0 },
		{  7,  1,  1,  0,  0,  0,  0,  0 },
		{  8,  1,  1,  0,  1,  0,  0,  0 },
		{  9,  1,  1,  0,  1,  1,  0,  0 },
		{  10, 1,  1,  0,  0,  0,  1,  0 },
		{  11, 1,  1,  1,  0,  0,  0,  0 },
		{  12, 1,  1,  1,  1,  0,  0,  0 },
		{  13, 1,  1,  1,  1,  1,  0,  0 },
		{  16, 1,  1,  1,  1,  1,  1,  0 },
		{  17, 1,  1,  1,  1,  1,  1,  1 },
		/* siz 1   6   4   1   1   3   1 */
	};
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	struct rnr_tbtt_info_bmap *bmap = NULL;
	struct neighbor_ap_info nap_info = {0};
	u16 tbtt_info_hdr = 0;
	u8 *pos = f_buf;
	u8 ie_len = 0;
	u8 i;
	u8 repted_band_idx = 0, repted_mbss_idx = 0;
	struct rnr_nap_info *rnr_nap = NULL;
	struct ie *rnr_tbtt = NULL;

	/*store reporting entry nap info*/
	if (repting_entry->entry_info.single_rnr) {
		repted_band_idx = HcGetBandByWdev(repted_entry->pwdev);
		repted_mbss_idx = repted_entry->pwdev->func_idx;
		rnr_nap = &repting_entry->rnr_nap[repted_band_idx];
		rnr_tbtt = &rnr_nap->tbtt[repted_mbss_idx];
	}

	/* TBTT Info Len decision */
#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(repted_entry->entry_info.phy_mode))
		tbtt_info_len_sel = 16;
	else
#endif
		tbtt_info_len_sel = 13;

	for (i = 0; i < ARRAY_SIZE(rnr_bmap); i++) {
		bmap = &rnr_bmap[i];
		if (bmap->tbtt_info_len == tbtt_info_len_sel)
			break;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"%s: [%d] tbtt_info count(%d) len(%d)\n", __func__, repted_entry->bss_idx,
			tbtt_info_cnt, bmap->tbtt_info_len);

	if (eid) {
		/* EID per-reported bss */
		eid->Eid = EID_REDUCED_NEIGHBOR_REPORT;
		pos = &eid->Octet[0];

		/* NAP Info: TBTT Information Header */
		SET_DOT11_RNR_TBTT_INFO_HDR_TYPE(tbtt_info_hdr, 0);
		SET_DOT11_RNR_TBTT_INFO_HDR_FLT_NAP(tbtt_info_hdr, TRUE);
		SET_DOT11_RNR_TBTT_INFO_HDR_CO_LOC_AP(tbtt_info_hdr, TRUE);
		SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT(tbtt_info_hdr,
			(tbtt_info_cnt - 1));	/* minus one */
		SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN(tbtt_info_hdr,
			bmap->tbtt_info_len);
		nap_info.tbtt_info_hdr = tbtt_info_hdr;

		/* NAP Info: Op Class and Ch Num */
		nap_info.op_class = repted_entry->entry_info.op_class;
		nap_info.ch_num = repted_entry->entry_info.channel;

		NdisMoveMemory(pos, (u8 *)&nap_info, sizeof(nap_info));
		pos += sizeof(nap_info);

		/* TBTT Information - Neighbor AP TBTT Offset */
		if (bmap->nap_tbtt_offset) {
			u8 offset = 0xff;

			/* TBD: The TBTT offset between two APs affiliated with the same AP
			 * MLD shall never be larger than 254 TUs. */
			if (repting_entry->mlo_en && repted_entry->mlo_en &&
				(repting_entry->entry_info.mld_grp == repted_entry->entry_info.mld_grp))
				offset = 100;

			NdisMoveMemory(pos, &offset, sizeof(offset));
			pos += sizeof(offset);
		}

		/* TBTT Information - BSSID */
		if (bmap->bssid) {
			COPY_MAC_ADDR(pos, repted_entry->entry_info.bssid);
			pos += MAC_ADDR_LEN;
		}

		/* TBTT Information - Short-SSID */
		if (bmap->s_ssid) {
			u32 s_ssid = Crcbitbybitfast(repted_entry->entry_info.ssid, repted_entry->entry_info.ssid_len);

			s_ssid = cpu_to_le32(s_ssid);
			NdisMoveMemory(pos, &s_ssid, sizeof(s_ssid));
			pos += sizeof(s_ssid);
		}

		/* TBTT Information - BSS parameters */
		if (bmap->bss_param) {
			u8 bss_param = TBTT_INFO_BSS_PARAM_CO_LOC_AP;
			struct bmgr_reg_info *repting_info = &repting_entry->entry_info;
			struct bmgr_reg_info *repted_info = &repted_entry->entry_info;

			if ((repting_info->ssid_len == repted_info->ssid_len) &&
				NdisEqualMemory(repting_info->ssid, repted_info->ssid, repting_info->ssid_len))
				bss_param |= TBTT_INFO_BSS_PARAM_SAME_SSID;

			switch (repted_info->dev_type) {
			case DEV_TYPE_AP_11VT:
			case DEV_TYPE_AP_COH:
				bss_param |= (TBTT_INFO_BSS_PARAM_MULTI_BSSID |
							  TBTT_INFO_BSS_PARAM_TX_BSSID);
				break;

			case DEV_TYPE_AP_11VNT:
				bss_param |= TBTT_INFO_BSS_PARAM_MULTI_BSSID;
				break;

			default:
				break;
			}

			NdisMoveMemory(pos, &bss_param, sizeof(bss_param));
			pos += sizeof(bss_param);
		}

		/* TBTT Information - 20 MHz PSD */
		if (bmap->psd_bw20) {
			u8 psd_bw20 = 1;	/* temp value */

			NdisMoveMemory(pos, &psd_bw20, sizeof(psd_bw20));
			pos += sizeof(psd_bw20);
		}

#ifdef DOT11_EHT_BE
		/* TBTT Information - MLD Parameters */
		if (bmap->mld_param) {
			u32 mld_param = {0};	/* 3 octors */
			u8 dli = 0;

			/* MLD Parameters */
			if (repted_entry->mlo_en) {
				SET_TBTT_INFO_MLD_PARAM_MLD_ID(mld_param, mld_id);
				SET_TBTT_INFO_MLD_PARAM_LINK_ID(mld_param, repted_entry->link_id);
				SET_TBTT_INFO_MLD_PARAM_BSS_CHG_CNT(mld_param, repted_entry->bss_chg_cnt);
				at2lm_rnr_dli_get(repted_entry, &dli);
			} else {
				/* not in MLD */
				SET_TBTT_INFO_MLD_PARAM_MLD_ID(mld_param, 0xff);
				SET_TBTT_INFO_MLD_PARAM_LINK_ID(mld_param, 0xf);
				SET_TBTT_INFO_MLD_PARAM_BSS_CHG_CNT(mld_param, 0xff);
			}
			SET_TBTT_INFO_MLD_PARAM_DLI(mld_param, dli);

			mld_param = cpu_to_le32(mld_param);
			NdisMoveMemory(pos, (u8 *)&mld_param, 3);
			pos += 3;
		}
#endif
		/* TBTT Information - dummy byte if tbtt_info_len = 17  */
		if (bmap->dummy) {
			u8 dummy = 0;

			for (i = 0; i < bmap->dummy; i++) {
				NdisMoveMemory(pos, &dummy, sizeof(dummy));
				pos += sizeof(dummy);
			}
		}

		/* eid_length */
		ie_len = pos - (u8 *)eid - 2;
		if (ie_len != 0)
			eid->Len = ie_len;

		if (repting_entry->entry_info.single_rnr) {
			if (ie_len > sizeof(nap_info)) {
				NdisMoveMemory(&rnr_nap->head[repted_mbss_idx], (u8 *)&nap_info, sizeof(nap_info));

				NdisZeroMemory(rnr_tbtt->buf, 32);
				NdisMoveMemory(rnr_tbtt->buf, eid->Octet + sizeof(nap_info), ie_len - sizeof(nap_info));

				rnr_tbtt->len = ie_len - sizeof(nap_info);

				if (!IS_TBTT_BITMAP_SET(rnr_nap->tbtt_bitmap, repted_mbss_idx)) {
					TBTT_BITMAP_SET(rnr_nap->tbtt_bitmap, repted_mbss_idx);
					rnr_nap->tbtt_cnt++;
				}
			}
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: Final rnr Len = %d\n", __func__, eid->Len + 2);
	}

	hex_dump_with_cat_and_lvl("RNR:", f_buf, ie_len + 2,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return pos;
}

#ifdef DOT11_EHT_BE
/*
 * 802.11be D1.0: 9.4.2.295b Multi-Link element
 */
u8 *build_multi_link_per_sta_profile(
	struct bmgr_entry *entry,
	u8 *f_buf,
	u8 frame_mld_type,
	u16 mld_sta_idx,
	u8 repting_link_id)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	struct ie *sta_profile = NULL;
	u8 *pos = f_buf;
	u16 ie_len = 0;
	u16 sta_ctrl = 0;
	u16 sta_ctrl_pres = 0;
#ifdef DOT11_BE_D12
	u8 *pos_info_len;
#endif
	u8 repted_link_id = entry->link_id;
	BOOLEAN b_tmp_buf = TRUE;

	/* Per-STA Profile subelement */
	eid->Eid = SUB_IE_MLD_PER_STA_PROFILE;
	pos += 2;

	/* Per-STA Control field */
	SET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(sta_ctrl, entry->link_id);
	if (frame_mld_type == BMGR_MLD_APPL_ELEM_STA_PROFILE) {
		SET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl, FALSE);
	} else {
		SET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl, TRUE);
	}

	/* Per-STA Control field - presence */
	if (GET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl)) {
		SET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl_pres, TRUE);
		SET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl_pres, TRUE);
		SET_DOT11BE_ML_PER_STA_CTRL_TSF_OFFSET_PRES(sta_ctrl_pres, TRUE);
		SET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl_pres, TRUE);
		/* meaningful if Maximum Number Of Simultaneous Links greater than 0 */
		SET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(sta_ctrl_pres, FALSE);
		if (frame_mld_type == BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE)
			SET_DOT11BE_ML_PER_STA_CTRL_BSS_CHG_CNT_PRES(sta_ctrl_pres, TRUE);
	} else {
		SET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_TSF_OFFSET_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_BSS_CHG_CNT_PRES(sta_ctrl_pres, FALSE);
	}

	/*
	 * meaningful if NSTR Link Pair Present is 1
	 *   Bitmap subfield: 0 = 1 octet, 1 = 2 octets
	 */
	/*
	SET_DOT11BE_ML_PER_STA_CTRL_NSTR_BMAP_SIZE(sta_ctrl_pres, 0);
	*/

	sta_ctrl = sta_ctrl | sta_ctrl_pres;
	sta_ctrl = cpu_to_le16(sta_ctrl);
	NdisMoveMemory(pos, (u8 *)&sta_ctrl, sizeof(sta_ctrl));
	pos += sizeof(sta_ctrl);

#ifdef DOT11_BE_D12
	/* STA Info Length */
	pos_info_len = pos;
	pos++;
#endif

	/* MAC Address Present */
	if (GET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl_pres)) {
		COPY_MAC_ADDR(pos, entry->entry_info.bssid);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\t    link_addr(%pM)\n", entry->entry_info.bssid);
		pos += MAC_ADDR_LEN;
	}

	/* Beacon Interval Present */
	if (GET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl_pres)) {
		u16 bcn_interval = entry->entry_info.bcn_interval;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\t    bcn_interval(%d)\n", bcn_interval);

		bcn_interval = cpu_to_le16(bcn_interval);
		NdisMoveMemory(pos, (u8 *)&bcn_interval, sizeof(bcn_interval));
		pos += sizeof(bcn_interval);
	}

	/* TSF Offset Present */
	if (GET_DOT11BE_ML_PER_STA_CTRL_TSF_OFFSET_PRES(sta_ctrl_pres)) {
		/* repted - repting */
		INT64 tsf_offset = entry->tsf_offset[repting_link_id];

		NdisMoveMemory(pos, &tsf_offset, sizeof(tsf_offset));
		pos += sizeof(tsf_offset);
	}

	/* DTIM Info Present */
	if (GET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl_pres)) {
		u8 dtim_count = 0;
		u8 dtim_period = entry->entry_info.dtim_period;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\t    dtim_count(%d) period(%d)\n", dtim_count, dtim_period);

		NdisMoveMemory(pos, &dtim_count, sizeof(dtim_count));
		pos += sizeof(dtim_count);
		NdisMoveMemory(pos, &dtim_period, sizeof(dtim_period));
		pos += sizeof(dtim_period);
	}

	/* AP MLD is in STR mode */

	/* BSS Parameters Change Count */
	if (GET_DOT11BE_ML_PER_STA_CTRL_BSS_CHG_CNT_PRES(sta_ctrl_pres)) {
		NdisMoveMemory(pos, &entry->bss_chg_cnt, sizeof(entry->bss_chg_cnt));
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\t	bss_chg_cnt(%d)\n", entry->bss_chg_cnt);
		pos++;
	}

#ifdef DOT11_BE_D12
	/* STA Info Length */
	*pos_info_len = (pos - pos_info_len);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			 "\tsta_info_len(%d)\n", *pos_info_len);
#endif

	/* STA Profile */
	if (frame_mld_type == BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE) {
		/* The STA Profile for Assoc Rsp. are put in bmgr_sta */
		struct bmgr_mld_sta *bmgr_sta = NULL;
		struct bmgr_mld_link *sta_mld_link = NULL;

		bmgr_sta = BMGR_VALID_MLD_STA(mld_sta_idx) ? GET_MLD_STA_BY_IDX(mld_sta_idx) : NULL;
		if (bmgr_sta && bmgr_sta->valid) {
			sta_mld_link = &bmgr_sta->mld_link[entry->link_id];
			if (sta_mld_link->requested) {
				sta_profile = &sta_mld_link->assoc_rsp_sta_profile;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\twdev=%s, assoc_rsp sta_profile=%p, len=%d\n",
					entry->pNetDev->name, sta_profile->buf, sta_profile->len);
			}
		}
	} else if (frame_mld_type == BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE) {
		/*
		 *  if peer transmits ML Probe Request by link 1:
		 *     - [link 0 entry] deliver ml_probe_rsp_sta_pf[1] to [link 1].
		 *     - [link 1 entry] skip.
		 *     - [link 2 entry] deliver ml_probe_rsp_sta_pf[1] to [link 1].
		 */
		sta_profile = &entry->ml_probe_rsp_sta_pf[repting_link_id];
		b_tmp_buf = FALSE;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_INFO,
			"\twdev=%s, probe_rsp sta_profile=%p, len=%d\n",
			entry->pNetDev->name, sta_profile->buf, sta_profile->len);
	} else if (frame_mld_type == BMGR_MLD_APPL_ELEM_STA_PROFILE) {
		/* copy reported buf to reporting link buf. */
		b_tmp_buf = FALSE; /* use reported link buf, not tmp buf */
		sta_profile = &entry->bcn_appl_elem_per_sta_pf[repted_link_id];
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"\twdev=%s, len=%d\n", entry->pNetDev->name, sta_profile->len);
	}

	if (sta_profile && sta_profile->buf && sta_profile->len) {
		if ((pos - f_buf + sta_profile->len) < BSS_MNGR_MAX_PER_STA_PROFILE_LEN) {
			if (frame_mld_type == BMGR_MLD_APPL_ELEM_STA_PROFILE) {
				entry->csa.csa_offset_in_per_sta_pf[repting_link_id] =
					pos - f_buf + PER_STA_PROILE_CAP_INFO_SIZE;
				}
			NdisMoveMemory(pos, sta_profile->buf, sta_profile->len);
			pos += sta_profile->len;
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error: (%ld) over the size (%d)\n",
				pos - f_buf + sta_profile->len, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
		}

		if (b_tmp_buf) {
			/* sta profile buffer only used once */
			os_free_mem(sta_profile->buf);
			sta_profile->buf = NULL;
			sta_profile->len = 0;
		}
	}

	/* eid_length */
	ie_len = pos - (u8 *)eid;
	if (ie_len != 0) {
		if (ie_len > (2 + MAX_LEN_OF_IE_DATA))
			pos = frag_multi_link_per_sta_profile((u8 *)eid, &ie_len);
		else
			eid->Len = ie_len - 2;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"Final Per-STA Profile Len = %d\n", ie_len);

	return pos;
}

u8 *build_multi_link_ie(struct bmgr_entry *entry, u8 *f_buf, u8 frame_mld_type, u16 mld_sta_idx)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf;
	u16 ie_len = 0;
	u16 mld_ctrl = 0;
	struct bmgr_entry *mld_entry;
	struct bmgr_mlo_dev *mld = entry->mld_ptr;
	u8 link_id, ml_ctrl_type;
	u16 present_bmap = 0;
	bool is_repted;
#ifdef DOT11_BE_D12
	u8 *ptr_info_len;
	u8 cmm_info_len = 0;
#endif

	if (!entry->mlo_en || !BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\t[%d] ml ie not required\n", entry->bss_idx);
		goto end;
	}

	if (eid) {
		eid->Eid = IE_WLAN_EXTENSION;
		eid->Octet[0] = EID_EXT_EHT_MULTI_LINK;
		pos += sizeof(struct _EID_STRUCT);

		switch (frame_mld_type) {
		case BMGR_MLD_BCN_COMMON:
		case BMGR_MLD_PROBE_RSP_PARTIAL_PROFILE: /* TODO */
		case BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE:
		case BMGR_MLD_AUTH_COMMON:
		case BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE:
		case BMGR_MLD_COMMON_LINK_INFO:
			ml_ctrl_type = ML_CTRL_TYPE_BASIC;
			break;

		case BMGR_MLD_PROBE_REQ:
			ml_ctrl_type = ML_CTRL_TYPE_PROBE_REQ;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				 "[%d] unknown frm_mld_type(%d) w/ mld_sta_idx(%d)\n",
				 entry->bss_idx, frame_mld_type, mld_sta_idx);
			ml_ctrl_type = ML_CTRL_TYPE_BASIC;
			break;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "%s: [%d] ml_ctrl_type(%s) frm_mld_type(%d) mld_sta_idx(%d)\n",
				 __func__, entry->bss_idx, ml_ctrl_type ? "probe" : "basic",
				 frame_mld_type, mld_sta_idx);

		/* Multi-Link Control */
		SET_DOT11BE_ML_CTRL_TYPE(mld_ctrl, ml_ctrl_type);
		if (frame_mld_type == BMGR_MLD_AUTH_COMMON) {
			/* Authentication frame w/ Basic variant */
			present_bmap = 0;
		} else if (ml_ctrl_type == ML_CTRL_TYPE_PROBE_REQ) {
			/* Probe Request variant */
			present_bmap =
				DOT11BE_ML_BASIC_PRESENCE_MLD_ID;
		} else {
			/* Basic variant except in Authentication frame */
			present_bmap =
				DOT11BE_ML_BASIC_PRESENCE_LINK_ID |
				DOT11BE_ML_BASIC_PRESENCE_BSS_CHG_CNT;

			if (mld->attr.med_sync.en)
				present_bmap |= DOT11BE_ML_BASIC_PRESENCE_MD_SYNC_DELAY;
			if (mld->attr.eml_caps.en && (mld->mld_link_cnt > 1))
				present_bmap |= DOT11BE_ML_BASIC_PRESENCE_EML_CAPS;
			if (mld->attr.mld_caps.en)
				present_bmap |= DOT11BE_ML_BASIC_PRESENCE_MLD_CAPS_N_OP;
			if (IS_DEV_TYPE_11V_NT(entry->entry_info.dev_type))
				present_bmap |= DOT11BE_ML_BASIC_PRESENCE_MLD_ID;
		}
		SET_DOT11BE_ML_CTRL_PRESENCE_BMAP(mld_ctrl, present_bmap);

		mld_ctrl = cpu_to_le16(mld_ctrl);
		NdisMoveMemory(pos, (u8 *)&mld_ctrl, sizeof(mld_ctrl));
		pos += sizeof(mld_ctrl);

#ifdef DOT11_BE_D12
		/* Common Info - Common Info Length */
		ptr_info_len = pos;
		NdisMoveMemory(pos, &cmm_info_len, sizeof(cmm_info_len));
		pos += sizeof(cmm_info_len);
#endif

		/* Common Info - MLD MAC Address */
		COPY_MAC_ADDR(pos, mld->mld_addr);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\tmld_addr(%pM)\n", mld->mld_addr);
		pos += MAC_ADDR_LEN;

		/* Common Info - Link ID Info */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_LINK_ID) {
			u8 link_id = 0;

			SET_DOT11BE_ML_BASIC_CMM_LINK_ID(link_id, entry->link_id);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tlink_id(%d)\n", link_id);
			NdisMoveMemory(pos, &link_id, sizeof(link_id));
			pos += sizeof(link_id);
		}

		/* Common Info - BSS Parameters Change Count */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_BSS_CHG_CNT) {
			u8 bss_chg_cnt = 0;

			bss_chg_cnt = entry->bss_chg_cnt;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tbss_chg_cnt(%d)\n", bss_chg_cnt);
			NdisMoveMemory(pos, &bss_chg_cnt, sizeof(bss_chg_cnt));
			pos += sizeof(bss_chg_cnt);
		}

		/* Common Info - Medium Synchronization Delay Information */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MD_SYNC_DELAY) {
			u16 med_sync = 0;

			SET_DOT11BE_ML_BASIC_CMM_MD_SYNC_DELAY_DUR(med_sync, mld->attr.med_sync.dur);
			SET_DOT11BE_ML_BASIC_CMM_MD_SYNC_OFDM_ED_THR(med_sync, mld->attr.med_sync.ofdm_ed_thr);
			SET_DOT11BE_ML_BASIC_CMM_MD_SYNC_MAX_TXOP(med_sync, mld->attr.med_sync.max_txop);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tmed_sync(0x%x)\n", med_sync);
			med_sync = cpu_to_le16(med_sync);
			NdisMoveMemory(pos, &med_sync, sizeof(med_sync));
			pos += sizeof(med_sync);
		}

		/* Common Info - EML Capabilities */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_EML_CAPS) {
			u16 eml_caps = 0;

			/* D4.0 */
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_SUP(eml_caps, mld->attr.eml_caps.emlsr_supp);
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_PADDING_DELAY(eml_caps, mld->attr.eml_caps.eml_padding_delay);
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_TRANS_DELAY(eml_caps, mld->attr.eml_caps.eml_trans_delay);
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLMR_SUP(eml_caps, mld->attr.eml_caps.emlmr_supp);
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_TRANS_TO(eml_caps, mld->attr.eml_caps.trans_to);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\teml_caps(0x%x)\n", eml_caps);
			eml_caps = cpu_to_le16(eml_caps);
			NdisMoveMemory(pos, &eml_caps, sizeof(eml_caps));
			pos += sizeof(eml_caps);
		}

		/* Common Info - MLD Capabilities and Operations */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MLD_CAPS_N_OP) {
			u16 mld_caps = 0;

			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_MAX_LINKS(mld_caps, (mld->mld_link_cnt - 1));
			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_SRS_SUP(mld_caps, mld->attr.mld_caps.srs_supp);
			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_TID2LNK_NEGO_SUP(mld_caps, mld->attr.mld_caps.t2l_nego_supp);
			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_FREQ_SEP_STR(mld_caps, mld->attr.mld_caps.freq_sep_str);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tmld_caps(0x%x)\n", mld_caps);
			mld_caps = cpu_to_le16(mld_caps);
			NdisMoveMemory(pos, &mld_caps, sizeof(mld_caps));
			pos += sizeof(mld_caps);
		}

		/* Common Info - MLD ID */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MLD_ID) {
			u8 mld_id = (ml_ctrl_type == ML_CTRL_TYPE_BASIC) ?
				entry->entry_info.dot11v_mbssid_idx : entry->dot11be_mld_id;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tmld_id(%d)\n", mld_id);
			NdisMoveMemory(pos, &mld_id, sizeof(mld_id));
			pos += sizeof(mld_id);
		}

#ifdef DOT11_BE_D12
		/* Common Info - fill Common Info Length */
		cmm_info_len = (u8)(pos - ptr_info_len);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\tcmm_info_len(%d)\n", cmm_info_len);
		*ptr_info_len = cmm_info_len;
#endif

		/* Link Info */
		if (frame_mld_type == BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE &&
			BMGR_VALID_MLD_STA(mld_sta_idx)) {
			/* Assoc Rsp. */
			struct bmgr_mld_sta *bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
			struct bmgr_mld_link *sta_mld_link = NULL;

			if (bmgr_sta && bmgr_sta->valid) {
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
					u8 bss_idx_mld = mld->bss_idx_mld[link_id];

					if (!BMGR_VALID_BSS_IDX(bss_idx_mld))
						continue;

					/* skip formation of Per-STA Profile for reporting link */
					is_repted = (link_id != entry->link_id) ? TRUE : FALSE;
					mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
					/* check this link is requested by peer */
					sta_mld_link = &bmgr_sta->mld_link[link_id];
					if (is_repted && BMGR_VALID_MLO_BSS_ENTRY(mld_entry) && sta_mld_link->requested) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
								"\t  Add link(%d):\n", link_id);
						pos = build_multi_link_per_sta_profile(
								mld_entry, pos, frame_mld_type,
								mld_sta_idx, entry->link_id);
					}
				}
			}
		} else if (frame_mld_type == BMGR_MLD_COMMON_LINK_INFO) {
			/* for original entry->mld buffer */
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				u8 bss_idx_mld = mld->bss_idx_mld[link_id];

				/* skip formation of Per-STA Profile for reporting link */
				is_repted = (link_id != entry->link_id) ? TRUE : FALSE;
				if (BMGR_VALID_BSS_IDX(bss_idx_mld)) {
					mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
					if (is_repted && BMGR_VALID_MLO_BSS_ENTRY(mld_entry)) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						   "\t	Add link(%d):\n", link_id);
						pos = build_multi_link_per_sta_profile(
								mld_entry, pos, frame_mld_type,
								BMGR_MAX_MLD_STA_CNT,
								entry->link_id);
					}
				}
			}
		}

		/* eid_length */
		ie_len = pos - (u8 *)eid;
		if (ie_len != 0) {
			if (ie_len > (2 + MAX_LEN_OF_IE_DATA)) {
				/* do IE fragmentation */
				struct bss_mngr_event_ops *event_ops = NULL;

				event_ops = entry->entry_info.event_ops;
				if (event_ops)
					pos = event_ops->bmgr_event_ie_fragment((u8 *)eid, &ie_len);
				else
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"Error: event_ops is not inited, can't frag the ML IE!");
			} else
				eid->Len = (ie_len - 2);
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: Final ml Len = %d\n", __func__, ie_len);
	}

end:
	return pos;
}

u8 *build_multi_link_probe_rsp_complete_per_sta_pf(
	struct bmgr_entry *entry,
	struct ml_ie_info *ml_info,
	u8 *f_buf)
{
	u8 *pos = f_buf;
	u8 repted_link, mld_bss_idx;
	bool requested_ap, request_partial_ap = FALSE;

	struct bmgr_entry *mld_entry;
	struct bmgr_mlo_dev *mld = entry->mld_ptr;
	struct ie per_sta_pf;

	/* check peer requesting all of AP or part of AP in AP MLD */
	for (repted_link = 0; repted_link < BSS_MNGR_MAX_BAND_NUM; repted_link++)
		request_partial_ap |= ml_info->link[repted_link].active;

	for (repted_link = 0; repted_link < BSS_MNGR_MAX_BAND_NUM; repted_link++) {
		/* the reporting link do not need Per-STA Profile */
		if (repted_link == entry->link_id)
			continue;

		/* request all || request this reported link */
		requested_ap = (!request_partial_ap) || (ml_info->link[repted_link].active);

		if (requested_ap) {
			mld_bss_idx = BMGR_VALID_MLO_DEV(mld) ?
				mld->bss_idx_mld[repted_link] : BMGR_INVALID_BSS_IDX;
			if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
				if (mld_entry && mld_entry->valid && mld_entry->mlo_en) {
					/*
					 * if peer transmits ML Probe Request by link 1:
					 *  - [link 0 entry] deliver per_sta_pf[1] to [link 1].
					 *  - [link 1 entry] skip.
					 *  - [link 2 entry] deliver per_sta_pf[1] to [link 1].
					 */
					per_sta_pf = mld_entry->ml_probe_rsp_per_sta_pf[entry->link_id];
					if (per_sta_pf.buf && per_sta_pf.len) {
						NdisMoveMemory(pos, per_sta_pf.buf, per_sta_pf.len);
						pos += per_sta_pf.len;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"Append repted link[%d] Per-STA Pf to repting link[%d] w/ len=%d\n",
							repted_link, entry->link_id, per_sta_pf.len);
					}
				}
			}
		}
	}
	return pos;
}

/* copy reported info from reported entry to buf for all reported links */
u8 *build_multi_link_appl_elem_per_sta_pf(
	struct bmgr_entry *repting_entry,
	u8 query_type,
	u8 *f_buf)
{
	u8 *pos = f_buf;
	u8 repted_link, mld_bss_idx;
	struct bmgr_entry *repted_entry;
	struct bmgr_mlo_dev *mld = repting_entry->mld_ptr;
	struct ie per_sta_pf;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"repting_entry->link_id=%d, query_type=%d\n",
		repting_entry->link_id, query_type);

	repting_entry->csa.csa_offset_in_per_sta_pf[repting_entry->link_id] = 0;
	for (repted_link = 0; repted_link < BSS_MNGR_MAX_BAND_NUM; repted_link++) {

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\trepted_link=%d\n", repted_link);

		/* the reporting link do not need Per-STA Profile */
		if (repted_link == repting_entry->link_id)
			continue;

		mld_bss_idx = BMGR_VALID_MLO_DEV(mld) ?
			mld->bss_idx_mld[repted_link] : BMGR_INVALID_BSS_IDX;
		if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\trepted_link=%d\n", repted_link);
			repted_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
			if (repted_entry && repted_entry->valid && repted_entry->mlo_en) {
				/*
				 * if peer transmits ML Probe Request by link 1:
				 *  - [link 0 entry] deliver per_sta_pf[1] to [link 1].
				 *  - [link 1 entry] skip.
				 *  - [link 2 entry] deliver per_sta_pf[1] to [link 1].
				 */
				NdisMoveMemory(&per_sta_pf,
					&repted_entry->bcn_appl_elem_per_sta_pf[repting_entry->link_id],
					sizeof(per_sta_pf));

				if (per_sta_pf.buf && per_sta_pf.len) {
					if (query_type == BMGR_QUERY_ML_IE_BCN) {
						/* calculate csa offset */
						repting_entry->csa.csa_offset_in_per_sta_pf[repting_entry->link_id] =
							repted_entry->csa.csa_offset_in_per_sta_pf[repting_entry->link_id]
							+ (pos - f_buf);
						repting_entry->csa.csa_bss_idx = repted_entry->entry_info.hw_bss_index;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
							"csa_offset_in_per_sta_pf[%d]=%d, repting_entry->csa_bss_idx=%d\n",
							repting_entry->link_id,
							repting_entry->csa.csa_offset_in_per_sta_pf[repting_entry->link_id],
							repting_entry->csa.csa_bss_idx);
					}
					NdisMoveMemory(pos, per_sta_pf.buf, per_sta_pf.len);
					pos += per_sta_pf.len;
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"Append repted link[%d] Per-STA Pf to repting link[%d] w/ len=%d\n",
						repted_link, repting_entry->link_id, per_sta_pf.len);
				}
			}
		}
	}

	return pos;
}

static u8 *build_reconfig_multi_link_per_sta_profile(
	IN struct bmgr_entry *entry,
	OUT u8 *f_buf,
	IN u16 ap_rm_tmr,
	OUT u16 *tmr_offset)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf;
	u16 ie_len = 0;
	u16 sta_ctrl = 0;
	u8 *pos_info_len;

	/* Per-STA Profile subelement */
	eid->Eid = SUB_IE_MLD_PER_STA_PROFILE;
	pos += 2;

	/* Per-STA Control field */
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_LINK_ID(sta_ctrl, entry->link_id);
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_COMP_PROF(sta_ctrl, FALSE);
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl, FALSE);
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_AP_RM_TMR_PRES(sta_ctrl, TRUE);
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_OP_UPD_TYPE(sta_ctrl, 0);
	SET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_OP_PARAM_PRES(sta_ctrl, FALSE);

	sta_ctrl = cpu_to_le16(sta_ctrl);
	NdisMoveMemory(pos, (u8 *)&sta_ctrl, sizeof(sta_ctrl));
	pos += sizeof(sta_ctrl);

	/* STA Info Length */
	pos_info_len = pos;
	pos++;

	/* MAC Address Present */
	if (GET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl)) {
		COPY_MAC_ADDR(pos, entry->entry_info.bssid);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
				 "link_addr(%pM)\n", entry->entry_info.bssid);
		pos += MAC_ADDR_LEN;
	}

	/* AP Removal Timer Present */
	if (GET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_AP_RM_TMR_PRES(sta_ctrl)) {
		/* converts from seconds to TBTT */
		ap_rm_tmr = (ap_rm_tmr * 1000) / entry->entry_info.bcn_interval;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
				 "ap_removal_timer(%d)\n", ap_rm_tmr);

		*tmr_offset = (u16)(pos - f_buf);
		ap_rm_tmr = cpu_to_le16(ap_rm_tmr);
		NdisMoveMemory(pos, (u8 *)&ap_rm_tmr, sizeof(ap_rm_tmr));
		pos += sizeof(ap_rm_tmr);
	}

	/* Operation Parameters Present */
	if (GET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_OP_PARAM_PRES(sta_ctrl)) {
		u16 op_param = 0;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
				 "op_param(%d)\n", op_param);

		op_param = cpu_to_le16(op_param);
		NdisMoveMemory(pos, &op_param, sizeof(op_param));
		pos += sizeof(op_param);
	}

	/* STA Info Length */
	*pos_info_len = (pos - pos_info_len);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
			 "sta_info_len(%d)\n", *pos_info_len);

	/* eid_length */
	ie_len = pos - (u8 *)eid;
	if (ie_len != 0)
		eid->Len = ie_len - 2;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
		"Final Per-STA Profile Len = %d\n", ie_len);

	return pos;
}

bool build_reconfig_multi_link_ie(struct bmgr_mlo_dev *mld, struct mld_reconfig_ie_build_t *reconfig_info)
{
	struct reconfig_ml_ie_t *reconf_ie;
	struct _EID_STRUCT *eid;
	struct bmgr_entry *entry;
	u8 *pos, *ptr_info_len;
	u16 mld_ctrl = 0, present_bmap = 0;
	u8 link_id, ml_ctrl_type, cmm_info_len = 0;

	if (!BMGR_VALID_MLO_DEV(mld) || (mld->reconf_ie.ie_status < BUF_STAT_ALLOCED)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"[%d] mld or buf(%d) not valid\n", mld ? mld->mld_grp : 0xff,
			mld ? mld->reconf_ie.ie_status : 0xff);
		return FALSE;
	}

	reconf_ie = &mld->reconf_ie;
	eid = (struct _EID_STRUCT *)reconf_ie->common_info.buf;
	pos = reconf_ie->common_info.buf;

	if (eid) {
		eid->Eid = IE_WLAN_EXTENSION;
		eid->Octet[0] = EID_EXT_EHT_MULTI_LINK;
		pos += sizeof(struct _EID_STRUCT);

		ml_ctrl_type = ML_CTRL_TYPE_RECONFIG;

		/* Multi-Link Control */
		SET_DOT11BE_ML_CTRL_TYPE(mld_ctrl, ml_ctrl_type);

		SET_DOT11BE_ML_CTRL_PRESENCE_BMAP(mld_ctrl, present_bmap);

		mld_ctrl = cpu_to_le16(mld_ctrl);
		NdisMoveMemory(pos, (u8 *)&mld_ctrl, sizeof(mld_ctrl));
		pos += sizeof(mld_ctrl);

		/* Common Info - Common Info Length */
		ptr_info_len = pos;
		NdisMoveMemory(pos, &cmm_info_len, sizeof(cmm_info_len));
		pos += sizeof(cmm_info_len);

		/* Common Info - MLD MAC Address */
		if (present_bmap & DOT11BE_ML_RECONFIG_PRESENCE_MLD_MAC_ADDR) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
					 "mld_addr(%pM)\n", mld->mld_addr);
			COPY_MAC_ADDR(pos, mld->mld_addr);
			pos += MAC_ADDR_LEN;
		}

		/* Common Info - fill Common Info Length */
		cmm_info_len = (u8)(pos - ptr_info_len);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
				 "cmm_info_len(%d)\n", cmm_info_len);
		*ptr_info_len = cmm_info_len;

		/* Common Info buffer length (includes TLV) */
		reconf_ie->common_info.len = (u16)(pos - reconf_ie->common_info.buf);

		/* Link Info */
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			u8 bss_idx_mld = mld->bss_idx_mld[link_id];

			/* handle some links are counting down */
			if (!reconfig_info->rm_links[link_id] || reconf_ie->link_info[link_id].len)
				continue;

			if (BMGR_VALID_BSS_IDX(bss_idx_mld)) {
				entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
				if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
					   "Add link(%d) bss(%d)\n", link_id, entry->bss_idx);
					pos = build_reconfig_multi_link_per_sta_profile(
							entry, reconf_ie->link_info[link_id].buf,
							reconfig_info->tmr[link_id],
							&reconf_ie->tmr_offset[link_id]);

					reconf_ie->link_info[link_id].len =
						(u16)(pos - reconf_ie->link_info[link_id].buf);
					reconf_ie->fw_bss_idx[link_id] = entry->entry_info.hw_bss_index;
				}
			}
		}

		/* eid_length */

		mld->reconf_ie.ie_status = BUF_STAT_IE_READY;

		return TRUE;
	}

	return FALSE;
}


u8 *build_multi_link_traffic_ie(struct bmgr_entry *entry, u8 *f_buf, u8 **mlt_ie_ptr)
{
	u8 *pos = f_buf;

	if (!entry->mlo_en) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\t[%d] mlt ie not required\n", entry->bss_idx);
		goto end;
	}

	/* store multi link traffic ie offset */
	if (f_buf && mlt_ie_ptr) {
		*mlt_ie_ptr = pos;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"mlt_ie_ptr(%p)\n", *mlt_ie_ptr);
	}

end:
	return pos;
}

u8 *frag_multi_link_per_sta_profile(IN OUT u8 *per_sta_pf, IN OUT u16 *pf_len)
{
	UINT8 *pos;
	UINT16 todo_len = 0, result_len = 0;
	struct _EID_STRUCT *frag_eid;

	if (*pf_len <= (2 + MAX_LEN_OF_IE_DATA)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Notice: the length of buffer is not greater than %d (T+L+V), do not fragment\n",
			(2 + MAX_LEN_OF_IE_DATA));
		return (per_sta_pf + *pf_len);
	}

	frag_eid = (struct _EID_STRUCT *)per_sta_pf;

	/* keep leading element */
	frag_eid->Len = MAX_LEN_OF_IE_DATA;
	todo_len = *pf_len - (2 + MAX_LEN_OF_IE_DATA);
	result_len = (2 + MAX_LEN_OF_IE_DATA);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"[leading] buf_len=%d, eid->Len=%d, todo_len=%d, result_len=%d\n",
		*pf_len, frag_eid->Len, todo_len, result_len);

	/* insert Fragment sub-elements */
	while (todo_len > 0) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[frag begin] buf_len=%d, todo_len=%d, result_len=%d\n",
			*pf_len, todo_len, result_len);
		pos = (UINT8 *)per_sta_pf + result_len;

		/* shift 2 bytes for accomdating tag and length fields of next Fragment sub-IE */
		NdisMoveMemory(pos + 2, pos, todo_len);
		result_len += 2;

		frag_eid = (struct _EID_STRUCT *)pos;
		frag_eid->Eid = SUB_IE_MLD_FRAGMENT;
		frag_eid->Len = (todo_len > MAX_LEN_OF_IE_DATA) ? MAX_LEN_OF_IE_DATA : todo_len;
		result_len += frag_eid->Len;
		todo_len -= frag_eid->Len;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[frag end] buf_len=%d, eid->Len=%d, todo_len=%d, result_len=%d\n",
			*pf_len, frag_eid->Len, todo_len, result_len);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"[end] buf_len=%d, todo_len=%d, result_len=%d\n",
		*pf_len, todo_len, result_len);
	/* update new length of buffer (leading IE + Fragment IEs) */
	*pf_len = result_len;

	pos = per_sta_pf + result_len;
	return pos;
}

u16 defrag_multi_link_per_sta_profile(u8 *per_sta_pf, u8 *ml_ie_end, u8 *pf_buf, u16 *pf_len)
{
	u8 *pos;
	u8 *output_buf = pf_buf;
	u8 tag, length;
	u16 done_len = 0, result_len = 0;
	u16 ie_len_all = 2 + MAX_LEN_OF_IE_DATA;
	struct _EID_STRUCT *sub_eid = (struct _EID_STRUCT *)per_sta_pf;

	while ((per_sta_pf + ie_len_all) < ml_ie_end) {
		sub_eid = (struct _EID_STRUCT *)(per_sta_pf + ie_len_all);
		if (((UCHAR *)sub_eid + 2) > ml_ie_end)
			break;
		if (sub_eid->Eid == SUB_IE_MLD_FRAGMENT && sub_eid->Len > 0)
			ie_len_all += (2 + sub_eid->Len);
		else
			break;
	}

	sub_eid = (struct _EID_STRUCT *)per_sta_pf;

	/* leading subelement */
	result_len = done_len = 2 + sub_eid->Len;
	NdisMoveMemory(output_buf, per_sta_pf, result_len);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"[leading] buf_len=%d, eid->Len=%d, done_len=%d, result_len=%d\n",
		ie_len_all, sub_eid->Len, done_len, result_len);

	/* concatenate data field of Fragment subelements to leading subelement */
	while (done_len < ie_len_all) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[LOG frag begin] buf_len=%d, done_len=%d, result_len=%d\n",
			ie_len_all, done_len, result_len);

		pos = (UINT8 *)per_sta_pf + done_len;
		sub_eid = (struct _EID_STRUCT *)pos;
		if (((UCHAR *)sub_eid + 2) > ml_ie_end)
			break;
		tag = sub_eid->Eid;
		length = sub_eid->Len;

		if (tag == SUB_IE_MLD_FRAGMENT && length > 0) {
			if (((UCHAR *)sub_eid + 2 + length) > ml_ie_end)
				break;
			/* shift 2 bytes for removal of tag and length fields of Fragment subIE */
			NdisMoveMemory(output_buf + result_len,	pos + 2, length);
			/* add length of fragmented portion (Data field) */
			result_len += length;

			/* goto next Fragment subelement */
			done_len += (2 + length);
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error: the next element is not Fragment subelement or len(%d) <= 0\n",
				length);
			break;
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[LOG frag end] buf_len=%d, eid->Len=%d, done_len=%d, result_len=%d, tag=%d, length=%d\n",
			ie_len_all, sub_eid->Len, done_len, result_len, tag, length);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"[end] buf_len=%d, done_len=%d, result_len=%d\n",
		ie_len_all, done_len, result_len);
	/* update new length of buffer (only reconstructed leading IE) */
	*pf_len = result_len;
	done_len = done_len - 2;
	return done_len;

}

int parse_multi_link_per_sta_profile(u8 *per_sta_pf, u16 pf_len, struct ml_ie_info *ml_info)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)per_sta_pf;
	u8 link_id;
	struct ml_ie_link *ml_link;
	u8 *pos;
	u16 sta_ctrl;
	u16 total_len = 0;

	pos = per_sta_pf + 2;	/* Subelement ID and Length */

	hex_dump_with_cat_and_lvl("Per-STA:", (u8 *)eid, pf_len + 2,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	if (pf_len >= sizeof(sta_ctrl)) {
		/* STA Control */
		total_len += sizeof(sta_ctrl);
		NdisMoveMemory((u8 *)&sta_ctrl, pos, sizeof(sta_ctrl));
		sta_ctrl = cpu_to_le16(sta_ctrl);
		pos += sizeof(sta_ctrl);

		link_id = GET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(sta_ctrl);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\t sta_ctrl(0x%x), link_id(%d)\n", sta_ctrl, link_id);

		if (link_id < BSS_MNGR_MAX_BAND_NUM) {
			ml_link = &ml_info->link[link_id];

			ml_link->active = TRUE;
			ml_link->sta_ctrl = sta_ctrl;

			if (ml_info->type == ML_CTRL_TYPE_BASIC) {
				/* STA Info Length */
				total_len += sizeof(ml_link->sta_info_len);
				if (total_len > pf_len)
					return -1;
				NdisMoveMemory(&ml_link->sta_info_len, pos, sizeof(ml_link->sta_info_len));
				pos += sizeof(ml_link->sta_info_len);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						 "\t sta_info_len(%d)\n", ml_link->sta_info_len);

				/* STA Info - MAC Address Present */
				if (GET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl)) {
					total_len += MAC_ADDR_LEN;
					if (total_len > pf_len)
						return -1;
					COPY_MAC_ADDR(ml_link->link_addr, pos);
					pos += MAC_ADDR_LEN;

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\t link_addr(%pM)\n", ml_link->link_addr);
				}

				/* STA Info - Beacon Interval Present */
				if (GET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl)) {
					u16 bcn_interval = 0;

					total_len += sizeof(bcn_interval);
					if (total_len > pf_len)
						return -1;
					NdisMoveMemory((u8 *)&bcn_interval, pos, sizeof(bcn_interval));
					ml_link->bcn_interval = cpu_to_le16(bcn_interval);
					pos += sizeof(bcn_interval);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\t bcn_interval(%d)\n", bcn_interval);
				}


				/* STA Info - TSF Offset */
				if (GET_DOT11BE_ML_PER_STA_CTRL_TSF_OFFSET_PRES(sta_ctrl)) {
					u32 tsf_offset[2] = {0};

					total_len += sizeof(tsf_offset);
					if (total_len > pf_len)
						return -1;
					NdisMoveMemory((u8 *)&tsf_offset, pos, sizeof(tsf_offset));
					pos += sizeof(tsf_offset);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\t tsf_offset(%08x %08x)\n", tsf_offset[0], tsf_offset[1]);
				}

				/* STA Info - DTIM Info Present */
				if (GET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl)) {
					total_len += (sizeof(ml_link->dtim_count) + sizeof(ml_link->dtim_period));
					if (total_len > pf_len)
						return -1;
					NdisMoveMemory(&ml_link->dtim_count, pos, sizeof(ml_link->dtim_count));
					pos += sizeof(ml_link->dtim_count);
					NdisMoveMemory(&ml_link->dtim_period, pos, sizeof(ml_link->dtim_period));
					pos += sizeof(ml_link->dtim_period);
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\t dtim_count(%d) period(%d)\n",
							 ml_link->dtim_count, ml_link->dtim_period);
				}

				/* STA Info - NSTR Link Pair Present */
				if (GET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl) &&
					GET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(sta_ctrl)) {
					if (GET_DOT11BE_ML_PER_STA_CTRL_NSTR_BMAP_SIZE(sta_ctrl) == 0) {
						u8 nstr_bmap0 = 0;

						total_len += sizeof(nstr_bmap0);
						if (total_len > pf_len)
							return -1;
						NdisMoveMemory(&nstr_bmap0, pos, sizeof(nstr_bmap0));
						ml_link->nstr_bmap = nstr_bmap0;
						pos += sizeof(nstr_bmap0);

						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
								 "\t NSTR en, bmap0(0x%x)\n", ml_link->nstr_bmap);
					} else {
						u16 nstr_bmap1 = 0;

						total_len += sizeof(nstr_bmap1);
						if (total_len > pf_len)
							return -1;
						NdisMoveMemory(&nstr_bmap1, pos, sizeof(nstr_bmap1));
						ml_link->nstr_bmap = cpu_to_le16(nstr_bmap1);
						pos += sizeof(nstr_bmap1);

						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
								 "\t NSTR en, bmap1(0x%x)\n", ml_link->nstr_bmap);
					}
				}

				/* STA Info - BSS Parameters Change Count */
				if (GET_DOT11BE_ML_PER_STA_CTRL_BSS_CHG_CNT_PRES(sta_ctrl)) {
					u8 bss_chg_cnt = 0;

					total_len += sizeof(bss_chg_cnt);
					if (total_len > pf_len)
						return -1;
					NdisMoveMemory(&bss_chg_cnt, pos, sizeof(bss_chg_cnt));
					pos++;

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\t bss_chg_cnt(%d)\n", bss_chg_cnt);
				}
			} else if (ml_info->type == ML_CTRL_TYPE_RECONFIG) {
				/* STA Info Length */
				total_len += sizeof(ml_link->sta_info_len);
				if (total_len > pf_len)
					return -1;
				NdisMoveMemory(&ml_link->sta_info_len, pos, sizeof(ml_link->sta_info_len));
				pos += sizeof(ml_link->sta_info_len);
				ml_link->link_id = link_id;
				ml_link->reconfig_type = GET_DOT11BE_ML_RECONFIG_PER_STA_CTRL_OP_UPD_TYPE(sta_ctrl);

				/*D3.0 reconfig type have no sta profile*/
				return 0;
			}

			/* STA Profile */
			ml_link->sta_profile_len = pf_len - sizeof(ml_link->sta_ctrl) - ml_link->sta_info_len;
			total_len += ml_link->sta_profile_len;
			if (total_len > pf_len)
				return -1;
			NdisMoveMemory(ml_link->sta_profile, pos, ml_link->sta_profile_len);
			// ml_link->sta_profile = pos;
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN, "\t Link(%d) not supported\n", link_id);
		}
	}

	return 0;
}

/**
 * @ml_reconfig_ie_check check whether ml_ie type is reconfig type;
 *
 * @param ml_ie multi link element
 */
BOOLEAN ml_reconfig_ie_check(u8 *ml_ie)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)ml_ie;
	u8 *pos = ml_ie;
	u16 mld_ctrl = 0;
	BOOLEAN ret = FALSE;

	if (!ml_ie)
		return ret;
	if (eid->Eid != IE_WLAN_EXTENSION || eid->Octet[0] != EID_EXT_EHT_MULTI_LINK)
		return ret;

	pos += sizeof(struct _EID_STRUCT);
	NdisMoveMemory(&mld_ctrl, (u8 *)pos, sizeof(mld_ctrl));
	mld_ctrl = cpu_to_le16(mld_ctrl);
	if (GET_DOT11BE_ML_CTRL_TYPE(mld_ctrl) == ML_CTRL_TYPE_RECONFIG)
		ret = TRUE;
	return ret;
}

int parse_multi_link_ie(u8 *ml_ie, struct frag_ie_info *frag_info, struct ml_ie_info *ml_info)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)ml_ie;
	u8 *pos = ml_ie;
	u8 *ml_ie_end = NULL, *pf_pos = NULL, *sub_ie_buf = NULL;
	u16 ml_ie_len = 0, sub_ie_len = 0;
	u16 sub_ie_total_len = 0;
	u16 mld_ctrl = 0;
	struct _EID_STRUCT *sub_eid = NULL;
	u16 present_bmap = 0;
	int total_len = 0;
	struct _EID_STRUCT *tmp_sub_eid = NULL;

	if (eid->Eid == IE_WLAN_EXTENSION) {
		ml_ie_len = (frag_info && frag_info->is_frag) ? (frag_info->ie_len_defrag - 2) : eid->Len;
		ml_ie_end = ml_ie + 2 + ml_ie_len;

		if (ml_ie_len && (eid->Octet[0] == EID_EXT_EHT_MULTI_LINK)) {
			if (pos + sizeof(struct _EID_STRUCT) > ml_ie_end) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal\n");
				return -1;
			}
			pos += sizeof(struct _EID_STRUCT);

			/* Multi-Link Control */
			total_len += sizeof(mld_ctrl);
			if (total_len > ml_ie_len) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
				return -1;
			}
			NdisMoveMemory(&mld_ctrl, (u8 *)pos, sizeof(mld_ctrl));
			mld_ctrl = cpu_to_le16(mld_ctrl);
			pos += sizeof(mld_ctrl);

			ml_info->type = GET_DOT11BE_ML_CTRL_TYPE(mld_ctrl);
			ml_info->pres_bmap = present_bmap = GET_DOT11BE_ML_CTRL_PRESENCE_BMAP(mld_ctrl);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"%s: type(%d), pres_bmap(0x%x), ml_ie_len(%d)\n",
					__func__, ml_info->type, ml_info->pres_bmap, ml_ie_len);

#ifdef DOT11_BE_D12
			/* Common Info - Common Info Length */
			total_len += sizeof(ml_info->cmm_info_len);
			if (total_len > ml_ie_len) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
				NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
				return -1;
			}
			NdisMoveMemory(&ml_info->cmm_info_len, pos, sizeof(ml_info->cmm_info_len));
			pos += sizeof(ml_info->cmm_info_len);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tcmm_info_len(%d)\n", ml_info->cmm_info_len);
#endif
			switch (ml_info->type) {
			case ML_CTRL_TYPE_BASIC:
				/* Common Info - MLD MAC Address */
				total_len += MAC_ADDR_LEN;
				if (total_len > ml_ie_len) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
					NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
					return -1;
				}
				COPY_MAC_ADDR(ml_info->mld_addr, pos);
				pos += MAC_ADDR_LEN;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						 "\tmld_addr(%pM)\n", ml_info->mld_addr);

				/* Common Info - Link ID Info */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_LINK_ID) {
					total_len += sizeof(ml_info->link_id);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&ml_info->link_id, pos, sizeof(ml_info->link_id));
					pos += sizeof(ml_info->link_id);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tlink_id(%d)\n", ml_info->link_id);
				}

				/* Common Info - BSS Parameters Change Count */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_BSS_CHG_CNT) {
					total_len += sizeof(ml_info->bss_chg_cnt);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&ml_info->bss_chg_cnt, pos, sizeof(ml_info->bss_chg_cnt));
					pos += sizeof(ml_info->bss_chg_cnt);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tbss_chg_cnt(%d)\n", ml_info->bss_chg_cnt);
				}

				/* Common Info - Medium Synchronization Delay Info */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MD_SYNC_DELAY) {
					u16 md_sync_delay = 0;

					total_len += sizeof(md_sync_delay);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&md_sync_delay, pos, sizeof(md_sync_delay));
					ml_info->md_sync_delay = cpu_to_le16(md_sync_delay);
					pos += sizeof(ml_info->md_sync_delay);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tmd_sync_delay(0x%x)\n", ml_info->md_sync_delay);
				}

				/* Common Info - EML Capabilities */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_EML_CAPS) {
					u16 eml_caps = 0;

					total_len += sizeof(eml_caps);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&eml_caps, pos, sizeof(eml_caps));
					ml_info->eml_caps = cpu_to_le16(eml_caps);
					ml_info->emlsr_sup = GET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_SUP(ml_info->eml_caps);
					pos += sizeof(eml_caps);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\teml_caps(0x%x) \n", ml_info->eml_caps);
				}

				/* Common Info - MLD Capabilities and Operations */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MLD_CAPS_N_OP) {
					u16 mld_caps = 0;

					total_len += sizeof(mld_caps);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&mld_caps, pos, sizeof(mld_caps));
					ml_info->mld_caps = cpu_to_le16(mld_caps);
					pos += sizeof(mld_caps);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tmld_caps(0x%x)\n", ml_info->mld_caps);
				}

				/* Common Info - MLD ID */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MLD_ID) {
					total_len += sizeof(ml_info->mld_id);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&ml_info->mld_id, pos, sizeof(ml_info->mld_id));
					pos += sizeof(ml_info->mld_id);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tmld_id(%d)\n", ml_info->mld_id);
				}

				/* Common Info - Extended MLD Capabilities and Operations */
				if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_EXT_MLD_CAPS_N_OP) {
					u16 ext_mld_caps = 0;

					NdisMoveMemory(&ext_mld_caps, pos, sizeof(ext_mld_caps));
					ml_info->ext_mld_caps = cpu_to_le16(ext_mld_caps);
					pos += sizeof(ext_mld_caps);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\text_mld_caps(0x%x)\n", ml_info->ext_mld_caps);
				}
				break;

			case ML_CTRL_TYPE_PROBE_REQ:
				/* Common Info - MLD ID */
				if (present_bmap & DOT11BE_ML_PROBE_REQ_PRESENCE_MLD_ID) {
					total_len += sizeof(ml_info->mld_id);
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					NdisMoveMemory(&ml_info->mld_id, pos, sizeof(ml_info->mld_id));
					pos += sizeof(ml_info->mld_id);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							 "\tmld_id(%d)\n", ml_info->mld_id);
				}
				break;

			case ML_CTRL_TYPE_RECONFIG:
				if (present_bmap & DOT11BE_ML_RECONFIG_PRESENCE_MLD_MAC_ADDR) {
					total_len += MAC_ADDR_LEN;
					if (total_len > ml_ie_len) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
									"ML IE is abnormal total_len= %d, ml_ie_len =%d\n", total_len, ml_ie_len);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					COPY_MAC_ADDR(ml_info->mld_addr, pos);
					pos += MAC_ADDR_LEN;
				}
				break;

			default:
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: unknown ML IE type (%u)\n", ml_info->type);
			}

			hex_dump_with_cat_and_lvl("ML IE:", (u8 *)ml_ie, (pos - ml_ie),
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

			/* Link Info */
			sub_eid = (struct _EID_STRUCT *)pos;

			os_alloc_mem(NULL, (UCHAR **)&sub_ie_buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
			if (!sub_ie_buf) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"failed to allocate memory for parsing ML IE\n");
				NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
				return -1;
			}
			while (((u8 *)sub_eid) < ml_ie_end) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"  Per-STA, eid(%d) len=(%d)\n", sub_eid->Eid, sub_eid->Len);
				sub_ie_len = sub_eid->Len;
				sub_ie_total_len = sub_ie_len;

				switch (sub_eid->Eid) {
				case SUB_IE_MLD_PER_STA_PROFILE:
					if (sub_eid->Len == MAX_LEN_OF_IE_DATA) {
						/* Per-STA Profile defragmentation */
						NdisZeroMemory(sub_ie_buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
						total_len = 0;
						tmp_sub_eid = sub_eid;

						while (((u8 *)tmp_sub_eid + total_len) < ml_ie_end) {
							tmp_sub_eid = sub_eid + total_len;
							if (tmp_sub_eid->Eid == SUB_IE_MLD_FRAGMENT && tmp_sub_eid->Len > 0)
								total_len += (2 + tmp_sub_eid->Len);
							else
								break;
						}
						if (total_len > BSS_MNGR_MAX_PER_STA_PROFILE_LEN) {
							MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"  Per-STA-PROFILE exceed the maxmum length:%d\n", total_len);
							if (sub_ie_buf)
								os_free_mem(sub_ie_buf);
							NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
							return -1;
						}

						sub_ie_total_len = defrag_multi_link_per_sta_profile(
							(u8 *)sub_eid, ml_ie_end, sub_ie_buf, &sub_ie_len);

						pf_pos = sub_ie_buf;
						sub_ie_len -= 2;
					} else {
						pf_pos = (u8 *)sub_eid;
						sub_ie_len = sub_eid->Len;
						sub_ie_total_len = sub_ie_len;
					}

					if (parse_multi_link_per_sta_profile(pf_pos, sub_ie_len, ml_info) < 0) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"prase multi link per sta profile fail\n");
						if (sub_ie_buf)
							os_free_mem(sub_ie_buf);
						NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
						return -1;
					}
					break;
//				case SUB_IE_MLD_VENDOR_SPECIFIC:
//					break;
				default:
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
							"  unsupport sub_eid(%d)\n", sub_eid->Eid);
					break;
				}
				sub_eid = (PEID_STRUCT)((u8 *)sub_eid + 2 + sub_ie_total_len);
			}

			if (sub_ie_buf)
				os_free_mem(sub_ie_buf);

		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					 "%s: incorrect eid_ext(%d)\n", __func__, eid->Octet[0]);
			return -1;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				 "%s: incorrect eid(%d)\n", __func__, eid->Eid);
		return -1;
	}

	return 0; //pos;
}

u8 *build_tid_to_link_map_ie(struct bmgr_entry *entry, u8 *f_buf)
{
	u8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 ie_len = 0;
	u16 tid2lnk_ctrl = 0;
	u8 link_id;
	struct bmgr_entry *mld_entry;
	struct bmgr_mlo_dev *mld = NULL;
	u8 link_map_present = 0xff;	/* TID[7:0] enabled */
	u16 link_map;
	u8 tid;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			 "%s: [%d]\n", __func__, entry->bss_idx);

	if (eid) {
		eid->Eid = IE_WLAN_EXTENSION;
		eid->Octet[0] = EID_EXT_EHT_TID2LNK_MAP;
		pos += sizeof(struct _EID_STRUCT);

		/* TID-To-Link Mapping Control */
		SET_DOT11BE_TID2LNK_MAP_CTRL_DIR(tid2lnk_ctrl, TID2LNK_MAP_CTRL_DIR_BI);
		SET_DOT11BE_TID2LNK_MAP_CTRL_DEFAULT_LNK_MAP(tid2lnk_ctrl, TRUE);
		SET_DOT11BE_TID2LNK_MAP_CTRL_LNK_MAP_PRES(tid2lnk_ctrl, link_map_present);
		tid2lnk_ctrl = cpu_to_le16(tid2lnk_ctrl);
		NdisMoveMemory((u8 *)pos, &tid2lnk_ctrl, sizeof(tid2lnk_ctrl));
		pos += sizeof(tid2lnk_ctrl);

		/* Link Mapping Of TID 0 ~ 7 */
		for (tid = 0; tid < MAX_TID_MAPPING_NUM; tid++) {
			link_map = 0;
			if (link_map_present & BIT(tid)) {
				mld = entry->mld_ptr;
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
					u8 bss_idx_mld = BMGR_VALID_MLO_DEV(mld) ?
						mld->bss_idx_mld[link_id] : BMGR_INVALID_BSS_IDX;

					/* get mld per-link's entry */
					if (BMGR_VALID_BSS_IDX(bss_idx_mld))
						mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
					else
						continue;

					/* check per-link's tid mapping */
					if (mld_entry && (mld_entry->tid_map & BIT(tid)))
						link_map |= BIT(link_id);
				}
				/* append link mapping of [tid] */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						 "\t TID%d link mapping (0x%x)\n", tid, link_map);
				link_map = cpu_to_le16(link_map);
				NdisMoveMemory((u8 *)pos, &link_map, sizeof(link_map));
				pos += sizeof(link_map);
			}
		}

		/* eid_length */
		ie_len = pos - (u8 *)eid - 2;
		if (ie_len != 0)
			eid->Len = ie_len;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: Final t2l Len = %d\n", __func__, eid->Len + 2);
	}

	return pos;
}

u32 parse_tid_to_link_map_ie(u8 *t2l_ie, struct tid2lnk_ie_info *t2l_info)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)t2l_ie;
	u8 *pos = t2l_ie;
	u8 t2l_ie_len = 0;
	u16 t2l_ctrl = 0;
	u16 link_map = 0;
	u8 tid;
	u8 link_id;

	if (eid->Eid != IE_WLAN_EXTENSION) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "%s: incorrect eid(%d)\n", __func__, eid->Eid);
		return 0;
	}

	t2l_ie_len = eid->Len;
	if ((t2l_ie_len == 0) || (eid->Octet[0] != EID_EXT_EHT_TID2LNK_MAP)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				 "%s: incorrect eid_ext(%d)\n", __func__, eid->Octet[0]);
		return 0;
	}

	pos += sizeof(struct _EID_STRUCT);
	/* TID-To-Link Control */
	NdisMoveMemory(&t2l_ctrl, (u8 *)pos, sizeof(t2l_ctrl));
	t2l_ctrl = cpu_to_le16(t2l_ctrl);
	pos += sizeof(t2l_ctrl);

	t2l_info->ctrl_dir = GET_DOT11BE_TID2LNK_MAP_CTRL_DIR(t2l_ctrl);
	t2l_info->ctrl_default = GET_DOT11BE_TID2LNK_MAP_CTRL_DEFAULT_LNK_MAP(t2l_ctrl);
	t2l_info->ctrl_map_pres = GET_DOT11BE_TID2LNK_MAP_CTRL_LNK_MAP_PRES(t2l_ctrl);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"%s: dir(%d), default(%d), map_pres(0x%x)\n", __func__,
			t2l_info->ctrl_dir,
			t2l_info->ctrl_default,
			t2l_info->ctrl_map_pres);

	/* Link Mapping Of TID 0 ~ 7 */
	for (tid = 0; tid < MAX_TID_MAPPING_NUM; tid++) {
		/* check link mapping present */
		if (t2l_info->ctrl_map_pres & BIT(tid)) {
			/* get link mapping of tid */
			NdisMoveMemory(&link_map, (u8 *)pos, sizeof(link_map));
			link_map = cpu_to_le16(link_map);
			pos += sizeof(link_map);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\ttid(%d) link_map(0x%04x)\n", tid, link_map);

			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				if (link_map & BIT(link_id))
					t2l_info->tid_map[link_id] |= BIT(tid);
			}

			if (pos >= (t2l_ie + 2 + t2l_ie_len))
				break;
		}
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				 "\tlink(%d) tid_map(0x%02x)\n", link_id, t2l_info->tid_map[link_id]);
	}
	hex_dump_with_cat_and_lvl("T2L IE:", (u8 *)t2l_ie, (pos - t2l_ie),
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return 0; //pos;
}

u8 *build_aid_bitmap_ie(struct mld_link_recomm *link_recomm, u8 *f_buf)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf;
	u16 i, ie_len;
	u8 partial_aid_bmap_len;
	u8 bmap_ctrl = 0;
	unsigned long aid_bmap[BITS_TO_LONGS(INVALID_AID)] = {0};
	int first_bit, last_bit, first_octet, last_octet;
	struct mld_recomm_sta_info *recomm_sta_info;

	if (!link_recomm || !f_buf) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"input error: link_recomm(%p), f_buf(%p)\n",
			link_recomm, f_buf);
		return f_buf;
	}

	/* without MLD STA */
	if (!link_recomm->mld_sta_num) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Without any AID!\n");
		return f_buf;
	}

	if (eid) {
		recomm_sta_info = (struct mld_recomm_sta_info *)link_recomm->mld_sta_info;
		for (i = 0; i < link_recomm->mld_sta_num; i++, recomm_sta_info++) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"set AID %d to bmap (valid %d)\n",
				recomm_sta_info->aid, recomm_sta_info->valid);
			if (!recomm_sta_info->valid)
				continue;

			set_bit(recomm_sta_info->aid, aid_bmap);
		}

		first_bit = find_first_bit(aid_bmap, INVALID_AID);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"first bit %d\n", first_bit);

		if (first_bit < INVALID_AID) {
			eid->Eid = IE_WLAN_EXTENSION;
			eid->Octet[0] = EID_EXT_EHT_AID_BITMAP;
			pos += sizeof(struct _EID_STRUCT);

			first_octet = first_bit / 8;
			if (first_octet & 0x01) /* even */
				first_octet -= 1;
			bmap_ctrl = first_octet;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"first octet %d, bmap_ctrl 0x%x\n", first_octet, bmap_ctrl);

			last_bit = find_last_bit(aid_bmap, INVALID_AID);
			last_octet = last_bit / 8;


			/* Partial AID Bitmap Length */
			partial_aid_bmap_len = last_octet - first_octet + 1;
			NdisMoveMemory(pos, &partial_aid_bmap_len, sizeof(partial_aid_bmap_len));
			pos += sizeof(partial_aid_bmap_len);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"last octet %d, PAB length 0x%x\n", last_octet, partial_aid_bmap_len);

			/* Bitmap Control */
			NdisMoveMemory(pos, &bmap_ctrl, sizeof(bmap_ctrl));
			pos += sizeof(bmap_ctrl);

			/* Partial AID Bitmap */
			NdisCopyMemory(pos, ((u8 *)aid_bmap) + first_octet, partial_aid_bmap_len);
			pos += partial_aid_bmap_len;
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"(ERROR) first AID not found\n");
			return f_buf;
		}

		/* eid_length */
		ie_len = pos - f_buf;
		if (ie_len != 0)
			eid->Len = (ie_len - 2);

		/* hex_dump_always("AID Bitmap IE", f_buf, eid->Len + 2); */
	}

	return pos;
}

u8 *build_link_recomm_multi_link_traffic_ie(struct mld_link_recomm *link_recomm, u8 *f_buf)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf, *per_link_bmap = NULL;
	u16 i, j, ie_len, mem_size;
	u16 mlt_ctrl = 0, aid_offset = INVALID_AID;
	u16 valid_aid_num = 0, bit_index;
	u8 BITS_PER_AID = BSS_MNGR_MAX_LINK_ID + 1;
	struct mld_recomm_sta_info *recomm_sta_info;

	if (!link_recomm || !f_buf) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"input error: link_recomm(%p), f_buf(%p)\n",
			link_recomm, f_buf);
		return f_buf;
	}

	/* without MLD STA */
	if (!link_recomm->mld_sta_num) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Without any AID!\n");
		return f_buf;
	}

	if (eid) {
		/* search first valid AID and count number of valid AID */
		recomm_sta_info = (struct mld_recomm_sta_info *)link_recomm->mld_sta_info;
		for (i = 0; i < link_recomm->mld_sta_num; i++) {
			if (recomm_sta_info->valid) {
				valid_aid_num++;

				/* NOTE: link_recomm->mld_sta_info must be arranged
				 * in ascending order of AID */
				if (aid_offset == INVALID_AID)
					aid_offset = recomm_sta_info->aid;
			}
			recomm_sta_info++;
		}

		if (!valid_aid_num) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"(ERROR) No valid AID\n");
			goto err;
		}

		/* allocate Per-Link Traffic Indication List */
		mem_size = valid_aid_num * (BSS_MNGR_MAX_LINK_ID + 1); /* number of bit */
		/* octet alignment */
		mem_size = (mem_size & 0x7) ? ((mem_size | 0x7) + 1) : mem_size;
		mem_size = mem_size / 8; /* number of octet */
		os_alloc_mem(NULL, (u8 **)&per_link_bmap, mem_size);
		if (!per_link_bmap)
			goto err;
		NdisZeroMemory(per_link_bmap, mem_size);

		eid->Eid = IE_WLAN_EXTENSION;
		eid->Octet[0] = EID_EXT_EHT_ML_TRAFFIC;
		pos += sizeof(struct _EID_STRUCT);

		/* Multi-Link Traffic Indication Control - Bitmap Size */
		SET_DOT11BE_MLT_CTRL_BITMAP_SIZE(mlt_ctrl, BSS_MNGR_MAX_LINK_ID);

		/* Multi-Link Traffic Indication Control - AID Offset */
		SET_DOT11BE_MLT_CTRL_AID_OFFSET(mlt_ctrl, aid_offset);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"aid_offset %d, mlt_ctrl 0x%x\n", aid_offset, mlt_ctrl);

		/* Multi-Link Traffic Indication Control */
		mlt_ctrl = cpu_to_le16(mlt_ctrl);
		NdisMoveMemory(pos, &mlt_ctrl, sizeof(mlt_ctrl));
		pos += sizeof(mlt_ctrl);

		/* Per-Link Traffic Indication List */
		bit_index = 0;
		recomm_sta_info = (struct mld_recomm_sta_info *)link_recomm->mld_sta_info;
		for (i = 0; i < link_recomm->mld_sta_num; i++, recomm_sta_info++) {
			if (!recomm_sta_info->valid)
				continue;

			for (j = 0; j < BITS_PER_AID; j++) {
				if (recomm_sta_info->recomm_links & BIT(j))
					per_link_bmap[(bit_index >> 3)] |= BIT(bit_index & 0x7);

				bit_index++;
			}
		}
		NdisMoveMemory(pos, per_link_bmap, mem_size);
		pos += mem_size;

		/* eid_length */
		ie_len = pos - f_buf;
		if (ie_len != 0)
			eid->Len = (ie_len - 2);

		/* hex_dump_always("MLT IE", f_buf, eid->Len + 2); */

		if (per_link_bmap)
			os_free_mem(per_link_bmap);
	}

	return pos;

err:
	return f_buf;
}


#endif


