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

	Module Name:
	sanity.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang  2004-09-01      add WMM support
*/
#include "rt_config.h"
#include "action.h"
#ifdef DOT11R_FT_SUPPORT
#include "ft/ft.h"
#include "ft/ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
#if defined(MWDS)
extern UCHAR	MTK_OUI[];
#endif
extern UCHAR	BROADCOM_OUI[];
extern UCHAR	MARVELL_OUI[];
extern UCHAR	METALINK_OUI[];

typedef struct wsc_ie_probreq_data {
	UCHAR	ssid[32];
	UCHAR	macAddr[6];
	UCHAR	data[2];
} WSC_IE_PROBREQ_DATA;

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAddBAReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen)
{
	PMLME_ADDBA_REQ_STRUCT   pInfo;

	pInfo = (MLME_ADDBA_REQ_STRUCT *)Msg;

	if ((MsgLen != sizeof(MLME_ADDBA_REQ_STRUCT))) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - message length not correct.\n");
		return FALSE;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pInfo->Wcid)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - The peer Mac is not associated yet.\n");
		return FALSE;
	}

	/*
	The IEEE has specified that the most significant bit of the most significant byte be used for this purpose.
	If its a 1, that means multicast, 0 means unicast. The most significant byte is the left most byte in the
	address, and the most significant bit is the right most bit of the byte
	*/
	if ((pInfo->pAddr[0] & 0x01) == 0x01) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - multicast address not support BA\n");
		return FALSE;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeDelBAReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen)
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	struct _MAC_TABLE_ENTRY *entry;

	pInfo = (MLME_DELBA_REQ_STRUCT *)Msg;

	if (!IS_WCID_VALID(pAd, pInfo->Wcid))
		return FALSE;

	if ((MsgLen != sizeof(MLME_DELBA_REQ_STRUCT))) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"sanity fail - message length not correct.\n");
		return FALSE;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pInfo->Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"sanity fail - The peer Mac is not associated yet.\n");
		return FALSE;
	}

	if (pInfo->TID >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"sanity fail - The peer TID is incorrect.\n");
		return FALSE;
	}

	entry = entry_get(pAd, pInfo->Wcid);
	if (NdisEqualMemory(entry->Addr, pInfo->Addr, MAC_ADDR_LEN) == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"sanity fail - the peer addr dosen't exist.\n");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN mlme_addba_resp_sanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen)
{
	MLME_ADDBA_RESP_STRUCT *pInfo;

	pInfo = (MLME_ADDBA_RESP_STRUCT *)Msg;

	if ((MsgLen != sizeof(MLME_ADDBA_RESP_STRUCT))) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"message length not correct\n");
		return FALSE;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pInfo->wcid)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"The peer Mac is not associated yet\n");
		return FALSE;
	}

	if (pInfo->tid >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"The peer TID is incorrect\n");
		return FALSE;
	}

	return TRUE;
}


BOOLEAN PeerAddBAReqActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen)
{
	PFRAME_ADDBA_REQ pAddFrame;

	pAddFrame = (PFRAME_ADDBA_REQ)(pMsg);

	if (MsgLen < (sizeof(FRAME_ADDBA_REQ))) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"ADDBA Request frame length size = %ld incorrect\n",
			MsgLen);
		return FALSE;
	}

#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);
	pAddFrame->BaStartSeq.word = cpu2le16(pAddFrame->BaStartSeq.word);

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"ADDBA Request Ba Policy[%d] not support\n",
			pAddFrame->BaParm.BAPolicy);
		return FALSE;
	}

	if (pAddFrame->BaParm.TID >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"Wrong TID %d!\n", pAddFrame->BaParm.TID);
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"ADDBA Request: Start Seq = %08x, wsize = %d, TID = %d, AMSDU = %x\n",
		pAddFrame->BaStartSeq.field.StartSeq, pAddFrame->BaParm.BufSize,
		pAddFrame->BaParm.TID, pAddFrame->BaParm.AMSDUSupported);

	return TRUE;
}

BOOLEAN PeerAddBARspActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen,
	IN UINT16 Wcid,
	OUT VOID *addba_option_info)
{
	PFRAME_ADDBA_RSP pAddFrame;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;

	struct addba_option_info *option_info;

	option_info = (struct addba_option_info *)addba_option_info;
	pAddFrame = (PFRAME_ADDBA_RSP)(pMsg);
	pEntry = entry_get(pAd, Wcid);
	ba_info = &pEntry->ba_info;

	if (MsgLen < (sizeof(FRAME_ADDBA_RSP))) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"ADDBA Resp frame length incorrect(len=%ld)\n", MsgLen);
		return FALSE;
	}

	RTMPZeroMemory(option_info,
		sizeof(struct addba_option_info));
	if (MsgLen > sizeof(struct _FRAME_ADDBA_RSP)) {
		UINT8 *option_ie;
		UINT32 option_ie_total_len = 0;

		option_ie = (UINT8 *)pMsg+sizeof(struct _FRAME_ADDBA_RSP);
		option_ie_total_len =
			MsgLen - sizeof(struct _FRAME_ADDBA_RSP);
		parse_addba_option_element(
			option_ie,
			option_ie_total_len,
			option_info);
	}

	/* we support immediate BA.*/
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->StatusCode = cpu2le16(pAddFrame->StatusCode);
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);

	if (pAddFrame->BaParm.TID >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"Wrong TID %d!\n", pAddFrame->BaParm.TID);
		return FALSE;
	}

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"ADDBA Resp Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy);
#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();

		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				return FALSE;
			}
			ba_info = &mld_entry->ba_info;
		}
#endif /* DOT11_EHT_BE */

		ba_info->PolicyNotSupBitmap |= 1 << pAddFrame->BaParm.TID;
#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif
		return FALSE;
	}

	/* SPEC define the buffer size of add ba resp should be at least 1 */
	if ((pAddFrame->BaParm.BufSize == 0)
		&& (option_info->addba_ext_bufsize == 0)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"illegal BA buffer size = %d && addba_ext_bufsize = %d\n",
			pAddFrame->BaParm.BufSize, option_info->addba_ext_bufsize);
		return FALSE;
	}

	return TRUE;
}

BOOLEAN PeerDelBAActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN VOID * pMsg,
	IN ULONG MsgLen)
{
	PFRAME_DELBA_REQ  pDelFrame;

	if (MsgLen != (sizeof(FRAME_DELBA_REQ)))
		return FALSE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	pDelFrame = (PFRAME_DELBA_REQ)(pMsg);
	*(USHORT *)(&pDelFrame->DelbaParm) = cpu2le16(*(USHORT *)(&pDelFrame->DelbaParm));
	pDelFrame->ReasonCode = cpu2le16(pDelFrame->ReasonCode);
	return TRUE;
}


#ifdef DOT11V_MBSSID_SUPPORT
BOOLEAN parse_mbssid_subelement(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 * ie_head,
	IN VOID * ie_list)
{
	UINT16 sub_len, offset, profile_len, profile_offset;
	PEID_STRUCT sub_ie;
	PEID_STRUCT profile_ie;
	PEID_STRUCT mbssid_idx_ie = NULL;
	PEID_STRUCT non_tx_ssid_ie = NULL;
	PEID_STRUCT mbssid_ie = (struct _EID_STRUCT *)ie_head;
	struct _bcn_ie_list *bcn_ie = (struct _bcn_ie_list *)ie_list;
	UINT8 bssid[MAC_ADDR_LEN];
	UINT8 bssid_lsb;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	ULONG idx;
	int ret = TRUE;
#ifdef DOT11_EHT_BE
	u8 *ml_ie = NULL;
	struct ml_ie_info *ml_info = NULL;
#endif

	if (mbssid_ie->Octet[0] <= 0 ||
	    mbssid_ie->Octet[0] > 8) {
		/* invalid Multiple BSSID ie
		 * (max MBSSID indicator must in range 1~8)
		 */
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
			"illegal mbssid ie, indicator(%d).\n", mbssid_ie->Octet[0]);
		ret = FALSE;
		goto out;
	}

#ifdef DOT11_EHT_BE
	os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));

	if (!ml_info) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
			"failed to allocate memory for parsing ML IE\n");
		ret = FALSE;
		goto out;
	}

	NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
#endif

	if (mbssid_ie->Len == 0) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
			"illegal mbssid ie len.\n");
		ret = FALSE;
		goto out;
	}

	sub_ie = (struct _EID_STRUCT *)&mbssid_ie->Octet[1];
	sub_len = mbssid_ie->Len - 1; /* minus max MBSSID indicator */

	offset = 0;
	while ((sub_len - offset) >= 2 && (offset + 2 + sub_ie->Len) <= sub_len) {

		profile_ie = NULL;
		/*search for each nontransmitted bssid profile*/
		if (sub_ie->Eid == NON_TX_BSSID_PROFILE) {
			profile_ie = (struct _EID_STRUCT *)&sub_ie->Octet[0];
			profile_len = sub_ie->Len;
			profile_offset = 0;
		}

		if (profile_ie == NULL)
			goto PARSE_END;

		if (profile_len - profile_offset < 2) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
				"illegal mbssid profile len.\n");
			ret = FALSE;
			goto out;
		}

		/*search for mbssid index ie in each profile*/
		mbssid_idx_ie = NULL;
		while ((profile_offset + 2 + profile_ie->Len) <= profile_len) {

			if (profile_ie->Eid == IE_MULTIPLE_BSSID_IDX)
				mbssid_idx_ie = profile_ie;

			if (profile_ie->Eid == IE_SSID)
				non_tx_ssid_ie = profile_ie;

#ifdef DOT11_EHT_BE
			if (profile_ie->Eid == IE_WLAN_EXTENSION &&
				profile_ie->Octet[0] == EID_EXT_EHT_MULTI_LINK)
				ml_ie = (u8 *)profile_ie;
#endif

			profile_offset = profile_offset + 2 + profile_ie->Len;

			if (profile_offset == profile_len)
				break;
			else if (profile_offset + 2 > profile_len)
				goto PARSE_END;

			profile_ie = (PEID_STRUCT)
				((UCHAR *)profile_ie + 2 + profile_ie->Len);
		}

		if (mbssid_idx_ie == NULL ||
			mbssid_idx_ie->Octet[0] == 0) /* check BSSID index */
			goto PARSE_END;

		if (non_tx_ssid_ie == NULL ||
			non_tx_ssid_ie->Octet[0] == 0) /* check non-tx ssid */
			goto PARSE_END;

		/*calculate BSSID of this profile*/
		NdisMoveMemory(bssid, &bcn_ie->Bssid[0], MAC_ADDR_LEN);
		bssid_lsb = bssid[5] & ((1 << mbssid_ie->Octet[0]) - 1);
		bssid[5] &= ~((1 << mbssid_ie->Octet[0]) - 1);
		bssid[5] |= (bssid_lsb + mbssid_idx_ie->Octet[0]) %
				  (1 << mbssid_ie->Octet[0]);

#ifdef DOT11_EHT_BE
		/*parse ml ie */
		if (ml_ie)
			if (parse_multi_link_ie(ml_ie, NULL, ml_info) < 0) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
					"ml ie parse fail.\n");
				ret = FALSE;
				goto out;
			}
#endif

		/*sync non-tx bss info to ie info*/
		NdisMoveMemory(bcn_ie->mbss_info.nontx_bssid[bcn_ie->mbss_info.nontx_bss_num],
			bssid, MAC_ADDR_LEN);

		/*Store nontx_ssid length*/
		NdisMoveMemory(bcn_ie->mbss_info.nontx_ssid[bcn_ie->mbss_info.nontx_bss_num],
			&non_tx_ssid_ie->Octet[0], non_tx_ssid_ie->Len);

		bcn_ie->mbss_info.nontx_ssid_Len[bcn_ie->mbss_info.nontx_bss_num] =
			non_tx_ssid_ie->Len;

		bcn_ie->mbss_info.nontx_bss_num++;

		/*sync non-tx bss info to scan tab*/
		idx = BssTableSearch(ScanTab, bssid, bcn_ie->Channel);

		if ((idx != BSS_NOT_FOUND) && (idx < MAX_LEN_OF_BSS_TABLE)) {
			ScanTab->BssEntry[idx].max_bssid_indicator =
						mbssid_ie->Octet[0];
			ScanTab->BssEntry[idx].mbssid_index =
						mbssid_idx_ie->Octet[0];

			NdisMoveMemory(ScanTab->BssEntry[idx].trans_bssid, &bcn_ie->Bssid[0], MAC_ADDR_LEN);
			NdisMoveMemory(ScanTab->BssEntry[idx].trans_ssid, &bcn_ie->Ssid[0], MAX_LEN_OF_SSID);
#ifdef DOT11_EHT_BE
			if (ml_ie) {
				ScanTab->BssEntry[idx].ml_capable = TRUE;
				/*mld id == mbss idx, someone may not show the mld id*/
				if (ml_info->mld_id)
					ScanTab->BssEntry[idx].ml_info.mld_id = ml_info->mld_id;
				else
					ScanTab->BssEntry[idx].ml_info.mld_id = mbssid_idx_ie->Octet[0];
			}
#endif

		}
PARSE_END:
		offset = offset + 2 + sub_ie->Len;
		sub_ie = (PEID_STRUCT)((UCHAR *)sub_ie + 2 + sub_ie->Len);
	}

out:
#ifdef DOT11_EHT_BE
	if (ml_info)
		os_free_mem(ml_info);
#endif

	return ret;
}
#endif
#if defined(MAP_6E_SUPPORT) || defined(CONFIG_6G_SUPPORT) || defined(DOT11_EHT_BE)

void parse_rnr_subelement(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 * ie_head,
	IN VOID * ie_list)
{
	PEID_STRUCT rnr_ie = (struct _EID_STRUCT *)ie_head;
	struct rnr_ml_param *ml_param = NULL, tmp_ml_param;
	struct neighbor_ap_info *nbr_ap_info = NULL;
	struct _bcn_ie_list *bcn_ie = (struct _bcn_ie_list *)ie_list;
	UCHAR *pos = NULL, *pos_tmp = NULL;
	int tbtt_inf_cnt = 0, tbtt_inf_len = 0, idx = 0, mld_param_offset = 0;
	struct rnr_tbtt_mld_param tmp_val = {0};
	UINT32 remain_len;

	if (rnr_ie->Len >= 4) {

		pos = ie_head + 2; /*point to the neighbir info*/
		pos_tmp = pos;
		remain_len = rnr_ie->Len;
		do {
			nbr_ap_info = (struct neighbor_ap_info *) pos;

			/* Update RNR info only for 6E channels */
			if ((nbr_ap_info->op_class > 130 && nbr_ap_info->op_class <= 137)) {
#ifdef MAP_6E_SUPPORT
				bcn_ie->rnr_info.op = nbr_ap_info->op_class;
#endif
#ifdef MAP_6E_SUPPORT
				bcn_ie->rnr_info.channel = nbr_ap_info->ch_num;
#endif
#ifdef CONFIG_6G_SUPPORT
				bcn_ie->rnr_channel = nbr_ap_info->ch_num;
#endif
			}

			if (nbr_ap_info->op_class > 130 && nbr_ap_info->op_class <= 137)
				tmp_ml_param.channel_band = WIFI_CH_BAND_6G;
			else if (nbr_ap_info->ch_num >= 36)
				tmp_ml_param.channel_band = WIFI_CH_BAND_5G;
			else
				tmp_ml_param.channel_band = WIFI_CH_BAND_2G;

			/*parse the tbtt inf cout */
			tbtt_inf_cnt = ((nbr_ap_info->tbtt_info_hdr & DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT_MASK)
				>> DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT_SHIFT) + 1;
			tbtt_inf_len = (nbr_ap_info->tbtt_info_hdr & DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_MASK)
				>> DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_SHIFT;

			if (remain_len > sizeof(struct neighbor_ap_info))
				remain_len -= sizeof(struct neighbor_ap_info);
			else {
				remain_len = 0;
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_DEBUG,
					"no more data.\n");
				break;
			}
			for (idx = 0; idx < tbtt_inf_cnt; idx++) {
				ml_param = NULL;
				/*1:len check*/
				pos_tmp += tbtt_inf_len;
				if (pos_tmp > (UCHAR *)(ie_head + sizeof(EID_STRUCT)-1 + rnr_ie->Len)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
						"OOB check: the read buffer len (%lu) > rnr ie len(%lu).\n", pos_tmp - ie_head,
						sizeof(EID_STRUCT)-1 + rnr_ie->Len);
					return;
				}

		/*spec 11be.d3.0: Table 9-321 TBTT Information field
		contents if the TBTT Information;
		Figure 9-709 TBTT Information field format
		Neighbor AP TBTT|Offset BSSID|Short SSID|BSS parameters|20 MHz PSD|MLD Parameters
						(optional)   |(optional) |
		Octets:    1    |   0 or 6   |   0 or 4   | 0 or 1         |     0 or 1 | 0 or 3
		*/
				if (tbtt_inf_len == 10) {
					/*Neighbor AP TBTT + Offset BSSID + MLD Parameters*/
					mld_param_offset = 7;
				} else if (tbtt_inf_len >= 16 &&
						  tbtt_inf_len <= 255) {
					/*all tbtt subfile include*/
					mld_param_offset = 13;
				} else {
					/*others: can't judge whether it include mld params*/
					if (remain_len > tbtt_inf_len)
						remain_len -= tbtt_inf_len;
					else
						remain_len = 0;
					continue;
				}
				if (remain_len < tbtt_inf_len) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_DEBUG,
						"OOB check: remain_len(=%d) < tbtt_inf_len(=%d)\n",
						remain_len, tbtt_inf_len);
					return;
				}
				remain_len -= tbtt_inf_len;
				/*get the ml parm info ie*/
				os_zero_mem(&tmp_val, sizeof(tmp_val));
				os_move_mem(&tmp_val, &nbr_ap_info->tbtt[idx * tbtt_inf_len + mld_param_offset], sizeof(tmp_val));
				tmp_ml_param.is_valid = 1;
				tmp_ml_param.mld_id = tmp_val.ap_mld_id;
				tmp_ml_param.link_id = tmp_val.link_id;
				/*not insert the same ml_param_entry*/
				if (search_rnr_ml_param_entry(bcn_ie->rnr_ml_parms, MAX_MLD_GROUP_NUM, &tmp_ml_param)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_INFO,
						"mld:%d, link_id: %d is exist.\n", tmp_ml_param.mld_id, tmp_ml_param.link_id);
					continue;
				}
				/*find unused entry to store the mld param*/
				ml_param = get_unused_rnr_ml_param_entry(bcn_ie->rnr_ml_parms, MAX_MLD_GROUP_NUM);
				if (!ml_param) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_CAT_AP, DBG_LVL_ERROR,
						"can't find unused ml params in bcn list.\n");
					continue;
				}
				os_move_mem(ml_param, &tmp_ml_param, sizeof(tmp_ml_param));
			}
			pos = pos+sizeof(*nbr_ap_info)+(tbtt_inf_cnt * tbtt_inf_len);
		} while ((pos + sizeof(struct neighbor_ap_info)) < (UCHAR *)(ie_head + sizeof(EID_STRUCT)-1 + rnr_ie->Len));
	}
	return;
}
#endif
static inline void copy_to_vie(UCHAR *ptr, USHORT *len_vie, UCHAR *ptr_eid, EID_STRUCT *eid)
{
	NdisMoveMemory(ptr + *len_vie, ptr_eid, eid->Len + 2);
	*len_vie += (eid->Len + 2);
}

#ifndef WSC_INCLUDED
#define WSC_ID_SEL_REGISTRAR       0x1041
#define WSC_OUI_LEN 4
#define WSC_TAG_LEN_SIZE 4
#endif

void PeerBeaconWscSelReg(PEID_STRUCT pEid, BCN_IE_LIST *ie_list)
{
	PUCHAR		pData;
	INT		Len = 0;
	USHORT		DataLen = 0;
	PWSC_IE		pWscIE;

	pData = (PUCHAR) pEid->Octet + WSC_OUI_LEN;
	Len = (SHORT)(pEid->Len - WSC_OUI_LEN);

	if (Len <= 0 || Len > MAX_LEN_OF_WSC_IE)
		return;

	while (Len >= WSC_TAG_LEN_SIZE) {
		WSC_IE	WscIE;
		INT	WscIELen;

		NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
		/* Check for WSC IEs */
		pWscIE = &WscIE;

		if (pWscIE->Length == 0)
			return;
		WscIELen = (INT)(be2cpu16(pWscIE->Length)) + WSC_TAG_LEN_SIZE;
		if (WscIELen > Len)
			return;

		if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REGISTRAR) {
			DataLen = be2cpu16(pWscIE->Length);
			NdisMoveMemory(&ie_list->selReg,
				pData + WSC_TAG_LEN_SIZE, sizeof(ie_list->selReg));
			break;
		}
		/* Set the offset and look for next WSC Tag information */
		/* Since Type and Length are both short type,
			we need to offset 4, not 2
		*/
		Len   -= WscIELen;
		if (Len >= WSC_TAG_LEN_SIZE)
			pData += WscIELen;
	}
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerBeaconAndProbeRspSanity(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN VOID * Msg,
	IN ULONG MsgLen,
	IN UCHAR  MsgChannel,
	OUT BCN_IE_LIST * ie_list,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE,
	IN BOOLEAN bGetDtim,
	IN BOOLEAN bFromBeaconReport)
{
	UCHAR *Ptr;
#ifdef CONFIG_STA_SUPPORT
	UCHAR TimLen;
#endif /* CONFIG_STA_SUPPORT */
	PFRAME_802_11 pFrame;
	PEID_STRUCT pEid;
	UCHAR SubType = SUBTYPE_ASSOC_REQ;
	UCHAR Sanity;
	ULONG Length = 0;
	UCHAR *pPeerWscIe = NULL;
	INT PeerWscIeLen = 0;
	BOOLEAN bWscCheck = TRUE;
	UCHAR LatchRfChannel = 0;
	UCHAR *ptr_eid = NULL;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;
	struct frag_ie_info frag_info = {0};
	CSA_IE_INFO *CsaInfo = &ie_list->CsaInfo;
#ifdef MBO_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bMboAPAssocDisallow = FALSE;
#endif /* CONFIG_STA_SUPPORT */
#endif /* MBO_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = (wdev->wdev_type == WDEV_TYPE_STA) ? \
								GetStaCfgByWdev(pAd, wdev) : NULL;
#endif
	/*
		For some 11a AP which didn't have DS_IE, we use two conditions to decide the channel
		1. If the AP is 11n enabled, then check the control channel.
		2. If the AP didn't have any info about channel, use the channel we received this
			frame as the channel. (May inaccuracy!!)
	*/
	UCHAR CtrlChannel = 0;
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	P_OCE_CTRL	pOceCtrl = &wdev->OceCtrl;
	BOOLEAN bHasOceIE = FALSE;
	INT OceNonOcePresentOldValue;
#endif /* CONFIG_AP_SUPPORT */
#endif /* OCE_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#endif

	os_alloc_mem(NULL, &pPeerWscIe, 512);
	if (pPeerWscIe == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"allocate memory for pPeerWscIe failed!\n");
		return FALSE;
	}
	Sanity = 0;		/* Add for 3 necessary EID field check*/
	ie_list->AironetCellPowerLimit = 0xFF;  /* Default of AironetCellPowerLimit is 0xFF*/
	ie_list->NewExtChannelOffset = 0xff;	/*Default 0xff means no such IE*/
	*LengthVIE = 0; /* Set the length of VIE to init value 0*/

	/* record end of frame for IE defragmentation */
	frag_info.frm_end = (UINT8 *)Msg + MsgLen;

	if (bFromBeaconReport == FALSE) {
		pFrame = (PFRAME_802_11)Msg;
		/* get subtype from header*/
		SubType = (UCHAR)pFrame->Hdr.FC.SubType;
		/* get Addr2 and BSSID from header*/
		COPY_MAC_ADDR(&ie_list->Addr2[0], pFrame->Hdr.Addr2);
		COPY_MAC_ADDR(&ie_list->Bssid[0], pFrame->Hdr.Addr3);
		Ptr = pFrame->Octet;
		Length += LENGTH_802_11;
	} else {
		/* beacon report response's body have no 802.11 header part! */
		SubType = 255; /* beacon report can't get SubType init 255 */
		Ptr = (UINT8 *)Msg;
		pFrame = NULL; /* init. */
	}

	/* get timestamp from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->TimeStamp, Ptr, TIMESTAMP_LEN);
	ie_list->TimeStamp.u.LowPart = cpu2le32(ie_list->TimeStamp.u.LowPart);
	ie_list->TimeStamp.u.HighPart = cpu2le32(ie_list->TimeStamp.u.HighPart);
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;
	/* get beacon interval from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->BeaconPeriod, Ptr, 2);
	Ptr += 2;
	Length += 2;
	/* get capability info from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->CapabilityInfo, Ptr, 2);
	Ptr += 2;
	Length += 2;

	if (CAP_IS_ESS_ON(ie_list->CapabilityInfo))
		ie_list->BssType = BSS_INFRA;
	else
		ie_list->BssType = BSS_ADHOC;

	pEid = (PEID_STRUCT) Ptr;

#ifdef TXBF_SUPPORT
	/* Reset OUI when recieving a new Probe Response */
	if (SubType == SUBTYPE_PROBE_RSP) {
		txbf_clear_oui();
	}
#endif

	/* get variable fields from payload and advance the pointer*/
	while ((Length + 2 + pEid->Len) <= MsgLen) {
		/* Secure copy VIE to VarIE[MAX_VIE_LEN] didn't overflow.*/
		if ((*LengthVIE + pEid->Len + 2) >= MAX_VIE_LEN) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
				"Variable IEs out of resource [len(=%d) > MAX_VIE_LEN(=%d)]\n",
				(*LengthVIE + pEid->Len + 2), MAX_VIE_LEN);
			break;
		}

		ptr_eid = (UCHAR *)pEid;

		switch (pEid->Eid) {
		case IE_SSID:

			/* Already has one SSID EID in this beacon, ignore the second one*/
			if (Sanity & 0x1)
				break;

			if (parse_ssid_ie(pEid)) {
				NdisMoveMemory(&ie_list->Ssid[0], pEid->Octet, pEid->Len);
				ie_list->SsidLen = pEid->Len;
				Sanity |= 0x1;
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_SSID (len=%d)\n", pEid->Len);
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}

			break;

		case IE_SUPP_RATES:
			if (parse_support_rate_ie(rate, pEid)) {
				Sanity |= 0x2;
				parse_support_rate_ie(rate, pEid);
				/*
				TODO: 2004-09-14 not a good design here, cause it exclude extra
				rates from ScanTab. We should report as is. And filter out
				unsupported rates in MlmeAux
				*/
				/* Check against the supported rates*/
				/* RTMPCheckRates(pAd, SupRate, pSupRateLen,wdev->PhyMode);*/
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_SUPP_RATES (len=%d)\n", pEid->Len);
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}

			break;

		case IE_HT_CAP:
			if (parse_ht_cap_ie(pEid->Len)) {
				NdisMoveMemory(&cmm_ies->ht_cap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
				SET_HT_CAPS_EXIST(cmm_ies->ie_exists);
				*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo) =
					cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo),
							(PUCHAR)(&cmm_ies->ht_cap.ExtHtCapInfo),
							sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&cmm_ies->ht_cap.ExtHtCapInfo),
							(PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo) =
					cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#ifdef CFG_BIG_ENDIAN
				*(USHORT *)(&cmm_ies->ht_cap.TxBFCap) =
					le2cpu32(*(USHORT *)(&cmm_ies->ht_cap.TxBFCap));
#endif

#ifdef CONFIG_STA_SUPPORT
				if (pStaCfg) {
					CLR_PREN_CAPS_EXIST(ie_list->cmm_ies.ie_exists);
					copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
				}

#endif /* CONFIG_STA_SUPPORT */
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
					"wrong IE_HT_CAP. pEid->Len = %d\n", pEid->Len);
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}

			break;

		case IE_ADD_HT:
			if (parse_ht_info_ie(pEid)) {
				/* This IE allows extension, but we ignore extra bytes beyond sizeof(ADD_HT_INFO_IE) */
				NdisMoveMemory(&cmm_ies->ht_op, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				SET_HT_OP_EXIST(cmm_ies->ie_exists);
				CtrlChannel = cmm_ies->ht_op.ControlChan;
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2) =
					cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2));
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3) =
					cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3));
#ifdef CONFIG_STA_SUPPORT
				if (pStaCfg) {
					copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
				}

#endif /* CONFIG_STA_SUPPORT */
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
					"wrong IE_ADD_HT.\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_SECONDARY_CH_OFFSET:
			if (parse_sec_ch_offset_ie(pEid)) {
				ie_list->NewExtChannelOffset = pEid->Octet[0];
#ifdef CONFIG_RCSA_SUPPORT
				CsaInfo->SChOffIE.SecondaryChannelOffset = pEid->Octet[0];
#endif
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
					"wrong IE_SECONDARY_CH_OFFSET.\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_FH_PARM:
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
				"(IE_FH_PARM)\n");
			break;

		case IE_DS_PARM:
			if (parse_ds_parm_ie(pEid)) {
				ie_list->Channel = *pEid->Octet;
#ifdef CONFIG_STA_SUPPORT

				/* snowpin for ap/sta IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
				if (pStaCfg) {
					if (IsValidChannel(pAd, ie_list->Channel, wdev) == 0) {
						goto SanityCheck;
					}
				}

#endif /* CONFIG_STA_SUPPORT */
				Sanity |= 0x4;
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_DS_PARM (len=%d)\n", pEid->Len);
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}

			break;

		case IE_CF_PARM:
			if (parse_cf_parm_ie(pEid)) {
				ie_list->CfParm.bValid = TRUE;
				ie_list->CfParm.CfpCount = pEid->Octet[0];
				ie_list->CfParm.CfpPeriod = pEid->Octet[1];
				ie_list->CfParm.CfpMaxDuration = pEid->Octet[2] + 256 * pEid->Octet[3];
				ie_list->CfParm.CfpDurRemaining = pEid->Octet[4] + 256 * pEid->Octet[5];
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_CF_PARM\n");

				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);

				return FALSE;
			}

			break;

		case IE_IBSS_PARM:
			if (parse_ibss_parm_ie(pEid))
				NdisMoveMemory(&ie_list->AtimWin, pEid->Octet, pEid->Len);
			else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_IBSS_PARM\n");

				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);

				return FALSE;
			}

			break;
#ifdef CONFIG_STA_SUPPORT

		case IE_TIM:
			if (SubType == SUBTYPE_BEACON) {
				if (pStaCfg) { /* snowpin for ap/sta ++ */
					if ((INFRA_ON(pStaCfg) && NdisEqualMemory(&ie_list->Bssid[0], pStaCfg->Bssid, MAC_ADDR_LEN)) || bGetDtim) {
						GetTimBit((PCHAR)pEid, pStaCfg->StaActive.Aid, &TimLen, &ie_list->BcastFlag,
								  &ie_list->DtimCount, &ie_list->DtimPeriod, &ie_list->MessageToMe);
					}
				} /* snowpin for ap/sta -- */
			}

			break;
#endif /* CONFIG_STA_SUPPORT */

		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ch_switch_announcement_ie(pEid)) {
				ie_list->NewChannel = pEid->Octet[1];	/*extract new channel number*/
#ifdef CONFIG_RCSA_SUPPORT
				NdisMoveMemory(&CsaInfo->ChSwAnnIE, &pEid->Octet[0], pEid->Len);
#endif
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_CHANNEL_SWITCH_ANNOUNCEMENT\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

#ifdef CONFIG_RCSA_SUPPORT
		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ext_ch_switch_announcement_ie(pEid)) {
				if (ie_list->NewChannel == 0)
					ie_list->NewChannel = pEid->Octet[2];	/*extract new channel number*/
				NdisMoveMemory(&CsaInfo->ExtChSwAnnIE, &pEid->Octet[0], pEid->Len);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;
#endif
		case IE_WIDE_BW_CH_SWITCH:
			if (pEid->Len == 3)
				NdisMoveMemory(&CsaInfo->wb_info, &pEid->Octet[0], pEid->Len);
			else {
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		/*
		New for WPA
		CCX v2 has the same IE, we need to parse that too
		Wifi WMM use the same IE vale, need to parse that too
		*/
		/* case IE_WPA:*/
		case IE_VENDOR_SPECIFIC:
#ifdef CFG_RSNO_SUPPORT
			/*
			 * Find WFA OUI IE
			 * Len of oui(OUI_WFA_LEN) + type(1)
			 */
			if ((pEid->Len >= (OUI_WFA_LEN+1))
				&& NdisEqualMemory(pEid->Octet, wfa_oui, OUI_WFA_LEN)) {
				copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			}
#endif /* CFG_RSNO_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			if (scan_in_run_state(pAd, wdev))
				customer_check_vendor_ie(pAd, (UCHAR *)pEid,
					&(ie_list->CustomerVendorIE), ie_list);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef TXBF_SUPPORT
			/* Record Netgear RAX40's OUI in Probe Response for BF IOT */
			if (SubType == SUBTYPE_PROBE_RSP) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_DEBUG,
					"pEid->Octet[0-2] = %x, %x, %x\n",
					pEid->Octet[0], pEid->Octet[1], pEid->Octet[2]);
				if (NdisEqualMemory(pEid->Octet, METALINK_OUI, 3)) {
					txbf_set_oui(ENUM_BF_OUI_METALINK);
				}
			}
#endif
			if (NdisEqualMemory(pEid->Octet, MARVELL_OUI, 3))
				ie_list->is_marvell_ap = TRUE;

			/* Check the OUI version, filter out non-standard usage*/
			check_vendor_ie(pAd, (UCHAR *)pEid, &(cmm_ies->vendor_ie));
			NdisCopyMemory(&ie_list->vendor_ie, &(cmm_ies->vendor_ie), sizeof(struct _vendor_ie_cap));
#ifdef MWDS
			if (cmm_ies->vendor_ie.mtk_cap_found == TRUE)
				ie_list->vendor_ie.mtk_cap_found = TRUE;
			if (cmm_ies->vendor_ie.support_mwds == TRUE)
				ie_list->vendor_ie.support_mwds = TRUE;
#endif /* MWDS */
#ifdef RT_CFG80211_SUPPORT
#endif
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT

			/* This HT IE is before IEEE draft set HT IE value.2006-09-28 by Jan.*/

			/* Other vendors had production before IE_HT_CAP value is assigned. To backward support those old-firmware AP,*/
			/* Check broadcom-defiend pre-802.11nD1.0 OUI for HT related IE, including HT Capatilities IE and HT Information IE*/
			if ((!HAS_HT_CAPS_EXIST(cmm_ies->ie_exists))
					&& NdisEqualMemory(pEid->Octet, BROADCOM_OUI, 3)
					&& (pEid->Len >= 4) && (pAd->OpMode == OPMODE_STA)) {
				if ((pEid->Octet[3] == OUI_BROADCOM_HT) && (pEid->Len >= 30)) {
					NdisMoveMemory(&cmm_ies->ht_cap, &pEid->Octet[4], sizeof(HT_CAPABILITY_IE));
					SET_PREN_CAPS_EXIST(cmm_ies->ie_exists);
				}

				if ((pEid->Octet[3] == OUI_PREN_ADD_HT) && (pEid->Len >= 26)) {
					NdisMoveMemory(&cmm_ies->ht_op, &pEid->Octet[4], sizeof(ADD_HT_INFO_IE));
					SET_HT_OP_EXIST(cmm_ies->ie_exists);
				}
			}

#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_OWE_SUPPORT
			if (NdisEqualMemory(pEid->Octet, OWE_TRANS_OUI, 4)) {
				/* Copy to pVIE which will report to bssid list.*/
				copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			} else
#endif /*CONFIG_OWE_SUPPORT*/
			if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) {
				/* Copy to pVIE which will report to bssid list.*/
				copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			} else if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24)) {
				PUCHAR ptr;
				int i;
				/* parsing EDCA parameters*/
				SET_WMM_CAPS_EXIST(cmm_ies->ie_exists);
				ie_list->EdcaParm.bValid          = TRUE;
				ie_list->EdcaParm.bQAck           = FALSE; /* pEid->Octet[0] & 0x10;*/
				ie_list->EdcaParm.bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20;*/
				ie_list->EdcaParm.bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40;*/
				ie_list->EdcaParm.EdcaUpdateCount = pEid->Octet[6] & 0x0f;
				ie_list->EdcaParm.bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
				ptr = &pEid->Octet[8];

				for (i = 0; i < 4; i++) {
					UCHAR aci = (*ptr & 0x60) >> 5; /* b5~6 is AC INDEX*/

					ie_list->EdcaParm.bACM[aci]  = (((*ptr) & 0x10) == 0x10);   /* b5 is ACM*/
					ie_list->EdcaParm.Aifsn[aci] = (*ptr) & 0x0f;               /* b0~3 is AIFSN*/
					ie_list->EdcaParm.Cwmin[aci] = *(ptr + 1) & 0x0f;           /* b0~4 is Cwmin*/
					ie_list->EdcaParm.Cwmax[aci] = *(ptr + 1) >> 4;             /* b5~8 is Cwmax*/
					ie_list->EdcaParm.Txop[aci]  = *(ptr + 2) + 256 * (*(ptr + 3)); /* in unit of 32-us*/
					ptr += 4; /* point to next AC*/
				}
			} else if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7)) {
				/* parsing EDCA parameters*/
				SET_WMM_CAPS_EXIST(cmm_ies->ie_exists);
				ie_list->EdcaParm.bValid          = TRUE;
				ie_list->EdcaParm.bQAck           = FALSE; /* pEid->Octet[0] & 0x10;*/
				ie_list->EdcaParm.bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20;*/
				ie_list->EdcaParm.bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40;*/
				ie_list->EdcaParm.EdcaUpdateCount = pEid->Octet[6] & 0x0f;
				ie_list->EdcaParm.bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
				/* use default EDCA parameter*/
				ie_list->EdcaParm.bACM[QID_AC_BE]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_BE] = 3;
				ie_list->EdcaParm.Cwmin[QID_AC_BE] = pAd->wmm_cw_min;
				ie_list->EdcaParm.Cwmax[QID_AC_BE] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_BE]  = 0;
				ie_list->EdcaParm.bACM[QID_AC_BK]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_BK] = 7;
				ie_list->EdcaParm.Cwmin[QID_AC_BK] = pAd->wmm_cw_min;
				ie_list->EdcaParm.Cwmax[QID_AC_BK] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_BK]  = 0;
				ie_list->EdcaParm.bACM[QID_AC_VI]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_VI] = 2;
				ie_list->EdcaParm.Cwmin[QID_AC_VI] = pAd->wmm_cw_min - 1;
				ie_list->EdcaParm.Cwmax[QID_AC_VI] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_VI]  = 96;   /* AC_VI: 96*32us ~= 3ms*/
				ie_list->EdcaParm.bACM[QID_AC_VO]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_VO] = 2;
				ie_list->EdcaParm.Cwmin[QID_AC_VO] = pAd->wmm_cw_min - 2;
				ie_list->EdcaParm.Cwmax[QID_AC_VO] = pAd->wmm_cw_max - 1;
				ie_list->EdcaParm.Txop[QID_AC_VO]  = 48;   /* AC_VO: 48*32us ~= 1.5ms*/
			} else if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)) {
				/*
					1. WSC 2.0 IE also has 0x104a000110. (WSC 1.0 version)

					2. Some developing devices would broadcast incorrect IE content.
					   To prevent system crashed by those developing devices, we shall check length.

					@20140123
				*/
				if (pPeerWscIe && (pEid->Len > 4)) {
					if ((PeerWscIeLen + (pEid->Len - 4)) <= 512) {
						NdisMoveMemory(pPeerWscIe + PeerWscIeLen, pEid->Octet + 4, pEid->Len - 4);
						PeerWscIeLen += (pEid->Len - 4);
					} else { /* ((PeerWscIeLen +(pEid->Len - 4)) > 512) */
						bWscCheck = FALSE;
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
							"Error!!! Sum of All PeerWscIeLen = %d (> 512)\n",
							(PeerWscIeLen + (pEid->Len - 4)));
					}
				} else {
					bWscCheck = FALSE;

					if (pEid->Len <= 4)
						MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
							"Error!!! Incorrect WPS IE!\n");

					if (pPeerWscIe == NULL)
						MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
							"Error!!! pPeerWscIe is null!\n");
				}
			}
#ifdef MBO_SUPPORT
#ifdef CONFIG_STA_SUPPORT
			if (IS_MBO_ENABLE(wdev)) {
				if (pFrame && (pEid->Octet[0] == MBO_OCE_OUI_0) &&
						(pEid->Octet[1] == MBO_OCE_OUI_1) &&
						(pEid->Octet[2] == MBO_OCE_OUI_2) &&
						(pEid->Octet[3] == MBO_OCE_OUI_TYPE))
					bMboAPAssocDisallow =
						MboParseApMboIE(pAd, pFrame->Hdr.Addr2, pEid->Octet, pEid->Len);
			}
#endif /* CONFIG_STA_SUPPORT */
#endif /* MBO_SUPPORT */

#ifdef MBO_SUPPORT
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			if (IS_OCE_ENABLE(wdev) && !pOceCtrl->Scan11bOceAPTimerRunning) {
				if ((pEid->Octet[0] == MBO_OCE_OUI_0) &&
						(pEid->Octet[1] == MBO_OCE_OUI_1) &&
						(pEid->Octet[2] == MBO_OCE_OUI_2) &&
						(pEid->Octet[3] == MBO_OCE_OUI_TYPE) &&
						(pEid->Len >= 5))
					bHasOceIE = OceCheckOceCap(pAd, wdev, pEid->Octet, pEid->Len);
			}
#endif /* CONFIG_AP_SUPPORT */
#endif /* OCE_SUPPORT */
#endif /* MBO_SUPPORT */
			break;


		case IE_EXT_SUPP_RATES:
			if (parse_support_ext_rate_ie(rate, pEid) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_EXT_SUPP_RATES\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_ERP:
			if (parse_erp_ie(pEid)) {
				ie_list->Erp = (UCHAR)pEid->Octet[0];
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_ERP\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_AIRONET_CKIP:

			/*
			0. Check Aironet IE length, it must be larger or equal to 28
			Cisco AP350 used length as 28
			Cisco AP12XX used length as 30
			*/
			if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
				break;

			/* 1. Copy CKIP flag byte to buffer for process*/
			ie_list->CkipFlag = *(pEid->Octet + 8);
			break;

		case IE_AP_TX_POWER:

			/* AP Control of Client Transmit Power*/
			/*0. Check Aironet IE length, it must be 6*/
			if (pEid->Len != 0x06)
				break;

			/* Get cell power limit in dBm*/
			if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
				ie_list->AironetCellPowerLimit = *(pEid->Octet + 4);

			break;

		/* WPA2 & 802.11i RSN*/
		case IE_RSN:

			/* There is no OUI for version anymore, check the group cipher OUI before copying*/
			if (parse_rsn_ie(pEid)) {
				if (RTMPEqualMemory(pEid->Octet + 2, RSN_OUI, 3)) {
					/* Copy to pVIE which will report to microsoft bssid list.*/
					copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_RSN\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_RSNXE:
			/* Copy to pVIE which will report to microsoft bssid list.*/
			copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			break;

#ifdef CONFIG_STA_SUPPORT
#if defined(EXT_BUILD_CHANNEL_LIST) || defined(RT_CFG80211_SUPPORT)

		case IE_COUNTRY:
			copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			break;
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */

		case IE_QBSS_LOAD:
			if (parse_qbss_load_ie(pEid)) {
				ie_list->QbssLoad.bValid = TRUE;
				ie_list->QbssLoad.StaNum = pEid->Octet[0] + pEid->Octet[1] * 256;
				ie_list->QbssLoad.ChannelUtilization = pEid->Octet[2];
				ie_list->QbssLoad.RemainingAdmissionControl = pEid->Octet[3] + pEid->Octet[4] * 256;
				/* Copy to pVIE*/
				copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"wrong IE_QBSS_LOAD\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			copy_to_vie((UCHAR *)pVIE, LengthVIE, ptr_eid, pEid);
			break;
#endif /* DOT11R_FT_SUPPORT */

		case IE_EXT_CAPABILITY:
			if (pEid->Len >= 1) {
				UCHAR cp_len, buf_space = sizeof(EXT_CAP_INFO_ELEMENT);

				cp_len = min(pEid->Len, buf_space);
				NdisMoveMemory(&ie_list->ExtCapInfo, &pEid->Octet[0], cp_len);
#ifdef CFG_BIG_ENDIAN
				(*(UINT32 *)(&(ie_list->ExtCapInfo))) = le2cpu32(*(UINT32 *)(&(ie_list->ExtCapInfo)));
				(*(UINT32 *)((UCHAR *)&(ie_list->ExtCapInfo)+4)) =
					le2cpu32(*(UINT32 *)((UCHAR *)&(ie_list->ExtCapInfo)+4));
#endif
			}

			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (pEid->Len == sizeof(VHT_CAP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT32 tmp_1;
				UINT64 tmp_2;
#endif
				NdisMoveMemory(&cmm_ies->vht_cap, &pEid->Octet[0], sizeof(VHT_CAP_IE));
				SET_VHT_CAPS_EXIST(cmm_ies->ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &cmm_ies->vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&cmm_ies->vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(cmm_ies->vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(cmm_ies->vht_cap.mcs_set), &tmp_2, 8);
#endif
			} else {
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_VHT_OP:
			if (pEid->Len == sizeof(VHT_OP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT16 tmp;
#endif
				NdisMoveMemory(&cmm_ies->vht_op, &pEid->Octet[0], sizeof(VHT_OP_IE));
				/* hex_dump ("recv. vht op", (UCHAR *)&ie_list->cmm_ies.vht_op, sizeof(VHT_OP_IE)); */
				SET_VHT_OP_EXIST(cmm_ies->ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp, &cmm_ies->vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&cmm_ies->vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
			} else {
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_OPERATING_MODE_NOTIFY:
			if (pEid->Len == sizeof(OPERATING_MODE) && (bFromBeaconReport == FALSE)) {
				OPERATING_MODE op_mode;
				MAC_TABLE_ENTRY *pEntry;
				struct wifi_dev_ops *ops = wdev->wdev_ops;

				if (!WMODE_CAP_AX_6G(wdev->PhyMode)) {
					ops->mac_entry_lookup(pAd, pFrame->Hdr.Addr2, wdev, &pEntry);
					NdisMoveMemory(&op_mode, &pEid->Octet[0], sizeof(OPERATING_MODE));

					if ((pEntry) && (op_mode.rx_nss_type == 0)) {
						pEntry->force_op_mode = TRUE;
						NdisMoveMemory(&pEntry->operating_mode, &op_mode, 1);
					}
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_DEBUG, "IE_OPERATING_MODE_NOTIFY(=%d)\n", pEid->Eid);
				} else {
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_VHT, DBG_LVL_WARN,
						"IE_OPERATING_MODE_NOTIFY at invalid band\n");
				}
			} else {
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;

		case IE_CH_SWITCH_WRAPPER: {
			INT8 ch_sw_wrp_len = pEid->Len;
			UCHAR *subelement = &pEid->Octet[0];
			INT8		len_subelement = 0;

			while (ch_sw_wrp_len > 0) {
				len_subelement = *(subelement + 1);

				if (*subelement == IE_WIDE_BW_CH_SWITCH) {
					if (len_subelement != sizeof(ie_list->cmm_ies.wb_info)) {
						MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_MISC, DBG_LVL_ERROR,
							"%s:%d: invalid subelement length:%u\n",
							__func__, __LINE__, len_subelement);
						break;
					}
					subelement += 2;
					NdisMoveMemory(&ie_list->cmm_ies.wb_info, subelement, len_subelement);
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_DEBUG,
						"%d Rx IE_WIDE_BW_CH_SWITCH, width:%u, CCFS0:%u, CCFS1%u\n",
						__LINE__, ie_list->cmm_ies.wb_info.new_ch_width, ie_list->cmm_ies.wb_info.center_freq_1,
						ie_list->cmm_ies.wb_info.center_freq_2);
					break;
				} else {
					subelement  += ch_sw_wrp_len;
					ch_sw_wrp_len -= ch_sw_wrp_len;
				}
			}
		}
		break;
#endif /* DOT11_VHT_AC */
		case IE_WLAN_EXTENSION:
#ifdef DOT11_HE_AX
			parse_he_beacon_probe_rsp_ies((UINT8 *)pEid, ie_list);
#endif /*DOT11_HE_AX*/
#ifdef DOT11_EHT_BE
			if (eht_get_ies((UINT8 *)pEid, &ie_list->cmm_ies, &frag_info) < 0) {
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}

			/*parse reconfiguration ml ie for remove links*/
			if (HAS_EHT_ML_RECONFIG_EXIST(ie_list->cmm_ies.ie_exists)) {
				if (parse_multi_link_ie(ie_list->cmm_ies.ml_ie_2nd,
					&ie_list->cmm_ies.ml_frag_info_2nd,
					&ie_list->mlo_ie_reconfig) < 0) {
					if (pPeerWscIe)
						os_free_mem(pPeerWscIe);
					return FALSE;
				}
			}
#endif /*DOT11_EHT_BE*/
			break;
#ifdef DOT11V_MBSSID_SUPPORT

		case IE_MULTIPLE_BSSID:
			if (parse_mbssid_subelement(pAd, wdev, (UINT8 *)pEid, ie_list) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_MULTIPLE_BSSID\n");
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
				return FALSE;
			}
			break;
#endif
#ifdef DOT11_EHT_BE
		case EID_REDUCED_NEIGHBOR_REPORT:
#if defined(MAP_6E_SUPPORT) || defined(CONFIG_6G_SUPPORT) || defined(DOT11_EHT_BE)
			parse_rnr_subelement(pAd, wdev, (UINT8 *)pEid,
			ie_list);
#endif

			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_DEBUG,
				"cache rnr ie\n");
			NdisMoveMemory(ie_list->cmm_ies.rnr_ie, pEid, pEid->Len + 2);
			SET_RNR_EXIST(cmm_ies->ie_exists);
			break;
#endif
		default:
			break;
		}

		if (frag_info.is_frag) {
			Length = Length + frag_info.ie_len_all;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR *)pEid + frag_info.ie_len_all);
			frag_info.is_frag = FALSE;
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
				"[IE frag] Jump to next IE: %d, (defrag=%d)\n",
				frag_info.ie_len_all, frag_info.ie_len_defrag);
		} else {
			Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
		}
	}

	LatchRfChannel = MsgChannel;
	/* this will lead Infra can't get beacon, when Infra on 5G then p2p in 2G. */
	if ((WMODE_CAP_AX_6G(wdev->PhyMode) || (LatchRfChannel > 14)) &&
			((Sanity & 0x4) == 0))
	{
		struct freq_oper oper;
		UCHAR bw = BW_20;

		if (hc_radio_query(pAd, &oper) == HC_STATUS_OK)
			bw = oper.bw;

		if (CtrlChannel != 0)
			ie_list->Channel = CtrlChannel;
		else {
			if (bw == BW_40
#ifdef DOT11_VHT_AC
				|| bw == BW_80
#endif /* DOT11_VHT_AC */
			   ) {
#ifdef CONFIG_STA_SUPPORT

				/* TODO: shiang-usw, fix me for this check!! */
				if (pStaCfg && pStaCfg->MlmeAux.Channel)
					ie_list->Channel = pStaCfg->MlmeAux.Channel;
				else
#endif /* CONFIG_STA_SUPPORT */
				{
					ie_list->Channel = LatchRfChannel;
				}
			} else
				ie_list->Channel = LatchRfChannel;
		}

		Sanity |= 0x4;
	}

	if (pPeerWscIe && (PeerWscIeLen > 0) && (PeerWscIeLen <= 512) && (bWscCheck == TRUE)) {
		UCHAR WscIe[] = {0xdd, 0x00, 0x00, 0x50, 0xF2, 0x04};

		Ptr = (PUCHAR) pVIE;
		WscIe[1] = PeerWscIeLen + 4;
		NdisMoveMemory(Ptr + *LengthVIE, WscIe, 6);
		NdisMoveMemory(Ptr + *LengthVIE + 6, pPeerWscIe, PeerWscIeLen);

		if (SubType == SUBTYPE_BEACON || SubType == SUBTYPE_PROBE_RSP)
			PeerBeaconWscSelReg((PEID_STRUCT)(Ptr + *LengthVIE), ie_list);
		*LengthVIE += (PeerWscIeLen + 6);
	}
SanityCheck:

	if (pPeerWscIe)
		os_free_mem(pPeerWscIe);

	if ((Sanity != 0x7) || (bWscCheck == FALSE)) {
		if (((bFromBeaconReport == FALSE) && (Sanity != 0x7)) /* case 1: */
			|| ((bFromBeaconReport == TRUE) && (!(Sanity & 0x1))) /* case 2: */
			|| (bWscCheck == FALSE) /* case 3 */
		   ) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_DEBUG, "missing field, Sanity=0x%02x, bWscCheck=%d\n", Sanity, bWscCheck);
			return FALSE;
		}
	}
#ifdef MBO_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	/*
	 * The presence of the MBO-OCE IE with the Association Disallowed attribute
	 * in any of the Beacon, Probe Response or (Re)Association Response frames
	 * shall be interpreted as an indication that the Wi-Fi Agile Multiband AP
	 * is currently not accepting associations.
	 */
	else if (bMboAPAssocDisallow == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"Assoc Disallow set by AP\n");
		return FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */
#endif /* MBO_SUPPORT */

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && !pOceCtrl->Scan11bOceAPTimerRunning) {
		if (!bHasOceIE) {
			OceNonOcePresentOldValue = OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
			OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET);
			OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				1, OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET);

			if (OceNonOcePresentOldValue != OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET))
				OceSendFilsDiscoveryAction(pAd, wdev);
			ie_list->is_oce_ap = FALSE;
		} else {
			ie_list->is_oce_ap = TRUE;
		}
	}
#endif /* OCE_SUPPORT */

	return TRUE;
}

#ifdef DOT11N_DRAFT3
/*
	==========================================================================
	Description:
		MLME message sanity check for some IE addressed  in 802.11n d3.03.
	Return:
		TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	IN OVERLAP_BSS_SCAN_IE * BssScan,
	OUT BCN_IE_LIST * ie_list,
	OUT UCHAR	* RegClass)
{
	CHAR				*Ptr;
	PFRAME_802_11		pFrame;
	PEID_STRUCT			pEid;
	ULONG				Length = 0;
	BOOLEAN				brc;
	CSA_IE_INFO *CsaInfo = &ie_list->CsaInfo;

	if (MsgLen <= LENGTH_802_11 + TIMESTAMP_LEN + 2 + 2)
		return FALSE;

	pFrame = (PFRAME_802_11)Msg;
	*RegClass = 0;
	Ptr = pFrame->Octet;
	Length += LENGTH_802_11;
	/* get timestamp from payload and advance the pointer*/
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;
	/* get beacon interval from payload and advance the pointer*/
	Ptr += 2;
	Length += 2;
	/* get capability info from payload and advance the pointer*/
	Ptr += 2;
	Length += 2;
	pEid = (PEID_STRUCT) Ptr;
	brc = FALSE;
	RTMPZeroMemory(BssScan, sizeof(OVERLAP_BSS_SCAN_IE));

	if (MsgLen - Length < 2)
		return FALSE;
	/* get variable fields from payload and advance the pointer*/
	while ((Length + 2 + pEid->Len) <= MsgLen) {
		switch (pEid->Eid) {
		case IE_SUPP_REG_CLASS:
			if (pEid->Len > 0) {
				brc = TRUE;
				*RegClass = *pEid->Octet;
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"PeerBeaconAndProbeRspSanity - wrong IE_SUPP_REG_CLASS (len=%d)\n", pEid->Len);
				return FALSE;
			}
			break;

		case IE_OVERLAPBSS_SCAN_PARM:
			if (parse_overlapbss_scan_parm_ie(pEid)) {
				brc = TRUE;
				RTMPMoveMemory(BssScan, pEid->Octet, sizeof(OVERLAP_BSS_SCAN_IE));
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
					"PeerBeaconAndProbeRspSanity - wrong IE_OVERLAPBSS_SCAN_PARM (len=%d)\n",
					pEid->Len);
				return FALSE;
			}
			break;

		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ext_ch_switch_announcement_ie(pEid)) {
				brc = TRUE;
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"PeerBeaconAndProbeRspSanity - IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n");
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_NOTICE,
					"wrong IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n");
				return FALSE;
			}
			break;

		case IE_WIDE_BW_CH_SWITCH:
			if (pEid->Len == 3)
				NdisMoveMemory(&CsaInfo->wb_info, &pEid->Octet[0], pEid->Len);
			 MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
				"Rx IE_WIDE_BW_CH_SWITCH, width:%u, CCFS0:%u, CCFS1%u\n",
				CsaInfo->wb_info.new_ch_width, CsaInfo->wb_info.center_freq_1,
				CsaInfo->wb_info.center_freq_2);
			break;
		}

		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len]	*/
		if (MsgLen - Length < 2)
			return FALSE;

		pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
	}

	return brc;
}
#endif /* DOT11N_DRAFT3 */

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeScanReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT UCHAR *pBssType,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pScanType)
{
	MLME_SCAN_REQ_STRUCT *Info;

	Info = (MLME_SCAN_REQ_STRUCT *)(Msg);
	*pBssType = Info->BssType;
	*pSsidLen = Info->SsidLen;
	NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);
	*pScanType = Info->ScanType;

	if ((*pBssType == BSS_INFRA || *pBssType == BSS_ADHOC || *pBssType == BSS_ANY)
		&& (SCAN_MODE_VALID(*pScanType))
	   )
		return TRUE;
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - wrong BssType or ScanType\n");
		return FALSE;
	}
}
#endif

UCHAR PSC_Ch_Check(
	IN UCHAR Ch)
{
	UCHAR isPSC = FALSE;
	UCHAR PSCList[CHANNEL_6G_PSC_NUM] = {5, 21, 37, 53, 69, 85, 101, 117, 133, 149, 165, 181, 197, 213, 229};
	UCHAR i;

	if (Ch == 0)
		return isPSC;

	for (i = 0; i < CHANNEL_6G_PSC_NUM; i++) {
		if (Ch == PSCList[i]) {
			isPSC = TRUE;
			break;
		}
	}

	return isPSC;
}

/* IRQL = DISPATCH_LEVEL*/
/*
	==========================================================================
	Description:
	Switch channel sanity check
	1. Use original channel to find out correct Band Index
	2. Compare new channel with current Band Index channel list

	Return:
	0 : new channel not exist in current Band Index channel list
	1 : new channel exist in current Band Index channel list

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
UCHAR SwitchChSanityCheck(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oriCh,
	IN UCHAR newCh)
{
	int i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (newCh == pChCtrl->ChList[i].Channel)
			return 1;
	}

	return 0;
}

UCHAR SwitchChSanityCheckByWdev(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR newCh)
{
	int i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (newCh == pChCtrl->ChList[i].Channel)
			return 1;
	}

	return 0;
}

UCHAR ChannelSanity(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	int i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (channel == pChCtrl->ChList[i].Channel)
			return 1;
	}

	return 0;
}

int FreqToChannel(IN USHORT freq)
{
	/* see 802.11 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq < 5925)
		return (freq - 5000) / 5;
	else if (freq == 5935)
		return 2;
	else if (freq <= 45000)
		/* see 802.11ax D6.1 27.3.22.2 */
		return (freq - 5950) / 5;
	else
		return 0;
}

BOOLEAN ChannelValidForFreqList(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR channel)
{
	UCHAR i;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	if (pChCtrl->FreqListNum == 0)
		return TRUE;

	for (i = 0; (i < pChCtrl->FreqListNum) && (pChCtrl->FreqListNum <= MAX_NUM_OF_CHANNELS); i++) {
		if (channel == FreqToChannel(pChCtrl->FreqList[i]))
			return TRUE;
	}

	return FALSE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDeauthSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pAddr3,
	OUT USHORT *pReason)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr1, pFrame->Hdr.Addr1);
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	COPY_MAC_ADDR(pAddr3, pFrame->Hdr.Addr3);
	NdisMoveMemory(pReason, &pFrame->Octet[0], 2);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAuthSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *pAlg,
	OUT USHORT *pSeq,
	OUT USHORT *pStatus,
	CHAR *pChlgText)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr,   pFrame->Hdr.Addr2);
	NdisMoveMemory(pAlg,    &pFrame->Octet[0], 2);
	NdisMoveMemory(pSeq,    &pFrame->Octet[2], 2);
	NdisMoveMemory(pStatus, &pFrame->Octet[4], 2);

	if (*pAlg == AUTH_MODE_OPEN) {
		if (*pSeq == 1 || *pSeq == 2)
			return TRUE;
		else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
				"sanity fail - wrong Seg#\n");
			return FALSE;
		}
	} else if (*pAlg == AUTH_MODE_KEY) {
		if (*pSeq == 1 || *pSeq == 4)
			return TRUE;
		else if (*pSeq == 2 || *pSeq == 3) {
			NdisMoveMemory(pChlgText, &pFrame->Octet[8], CIPHER_TEXT_LEN);
			return TRUE;
		} else {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
				"sanity fail - wrong Seg#\n");
			return FALSE;
		}
	}

#ifdef DOT11R_FT_SUPPORT
	else if (*pAlg == AUTH_MODE_FT)
		return TRUE;

#endif /* DOT11R_FT_SUPPORT */
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - wrong algorithm\n");
		return FALSE;
	}
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeAuthReqSanity(
	IN PRTMP_ADAPTER pAd,
#ifdef CONFIG_STA_SUPPORT
	IN struct wifi_dev *wdev,
#endif
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *pTimeout,
	OUT USHORT *pAlg)
{
	MLME_AUTH_REQ_STRUCT *pInfo;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	pInfo  = (MLME_AUTH_REQ_STRUCT *)Msg;
	COPY_MAC_ADDR(pAddr, pInfo->Addr);
	*pTimeout = pInfo->Timeout;
	*pAlg = pInfo->Alg;

	if (((*pAlg == AUTH_MODE_KEY) || (*pAlg == AUTH_MODE_OPEN)
#ifdef DOT11R_FT_SUPPORT
		 || (*pAlg == AUTH_MODE_FT)
#endif /* DOT11R_FT_SUPPORT */
		) &&
		((*pAddr & 0x01) == 0)) {
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_INCLUDED

		if (pStaCfg->wdev.WscControl.bWscTrigger && (pStaCfg->wdev.WscControl.WscConfMode != WSC_DISABLE))
			*pAlg = AUTH_MODE_OPEN;

#endif /* WSC_INCLUDED */
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO, "STA: %p\n", pStaCfg);
#endif /* CONFIG_STA_SUPPORT */
		return TRUE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - wrong algorithm\n");
		return FALSE;
	}
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAssocReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *pCapabilityInfo,
	OUT ULONG *pTimeout,
	OUT USHORT *pListenIntv)
{
	MLME_ASSOC_REQ_STRUCT *pInfo;

	pInfo = (MLME_ASSOC_REQ_STRUCT *)Msg;
	*pTimeout = pInfo->Timeout;                             /* timeout*/
	COPY_MAC_ADDR(pApAddr, pInfo->Addr);                   /* AP address*/
	*pCapabilityInfo = pInfo->CapabilityInfo;               /* capability info*/
	*pListenIntv = pInfo->ListenIntv;
	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDisassocSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pReason)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	NdisMoveMemory(pReason, &pFrame->Octet[0], 2);
	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Sanity check NetworkType (11b, 11g or 11a)

	Arguments:
		pBss - Pointer to BSS table.

	Return Value:
	Ndis802_11DS .......(11b)
	Ndis802_11OFDM24....(11g)
	Ndis802_11OFDM5.....(11a)

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(BSS_ENTRY *pBss)
{
	NDIS_802_11_NETWORK_TYPE	NetWorkType;
	UCHAR						rate, i;

	NetWorkType = Ndis802_11DS;

	if (pBss->Channel <= 14) {
		/* First check support Rate.*/
		for (i = 0; i < pBss->SupRateLen; i++) {
			rate = pBss->SupRate[i] & 0x7f; /* Mask out basic rate set bit*/

			if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
				continue;
			else {
				/* Otherwise (even rate > 108) means Ndis802_11OFDM24*/
				NetWorkType = Ndis802_11OFDM24;
				break;
			}
		}

		/* Second check Extend Rate.*/
		if (NetWorkType != Ndis802_11OFDM24) {
			for (i = 0; i < pBss->ExtRateLen; i++) {
				rate = pBss->SupRate[i] & 0x7f; /* Mask out basic rate set bit*/

				if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
					continue;
				else {
					/* Otherwise (even rate > 108) means Ndis802_11OFDM24*/
					NetWorkType = Ndis802_11OFDM24;
					break;
				}
			}
		}
	} else
		NetWorkType = Ndis802_11OFDM5;

	if (HAS_HT_CAPS_EXIST(pBss->ie_exists)) {
		if (NetWorkType == Ndis802_11OFDM5) {
#ifdef DOT11_VHT_AC
			if (HAS_VHT_CAPS_EXIST(pBss->ie_exists))
				NetWorkType = Ndis802_11OFDM5_AC;
			else
#endif /* DOT11_VHT_AC */
				NetWorkType = Ndis802_11OFDM5_N;
		} else
			NetWorkType = Ndis802_11OFDM24_N;
	}

	if (HAS_HE_CAPS_EXIST(pBss->ie_exists)) {
		if ((NetWorkType == Ndis802_11OFDM5) || (NetWorkType == Ndis802_11OFDM5_AC) || (NetWorkType == Ndis802_11OFDM5_N))
			NetWorkType = Ndis802_11OFDM5_HE;
		else
			NetWorkType = Ndis802_11OFDM24_HE;
	}

	if (HAS_EHT_CAPS_EXIST(pBss->ie_exists)) {
		if ((NetWorkType == Ndis802_11OFDM5) || (NetWorkType == Ndis802_11OFDM5_AC) || (NetWorkType == Ndis802_11OFDM5_N) || (NetWorkType == Ndis802_11OFDM5_HE))
			NetWorkType = Ndis802_11OFDM5_EHT;
		else
			NetWorkType = Ndis802_11OFDM24_EHT;
	}
	return NetWorkType;
}


/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerProbeReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PEER_PROBE_REQ_PARAM * ProbeReqParam)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;
	UCHAR		*Ptr;
	UCHAR		eid = 0, eid_len = 0, *eid_data;
#ifdef CONFIG_AP_SUPPORT
#if defined(WSC_INCLUDED) || defined(EASY_CONFIG_SETUP)
	UCHAR       apidx = MAIN_MBSSID;
#endif /* defined(WSC_INCLUDED) || defined(EASY_CONFIG_SETUP) */
	UCHAR       Addr1[MAC_ADDR_LEN];
#ifdef WSC_INCLUDED
	UCHAR		*pPeerWscIe = NULL;
	UINT		PeerWscIeLen = 0;
	BOOLEAN		bWscCheck = TRUE;
#endif /* WSC_INCLUDED */
#endif /* CONFIG_AP_SUPPORT */
	UINT		total_ie_len = 0;
#ifdef WH_EVENT_NOTIFIER
	IE_LISTS *ie_lists = &ProbeReqParam->ie_list;
#endif /* WH_EVENT_NOTIFIER */
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	UINT32 MaxChannelTime;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	P_OCE_CTRL pOceCtrl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.OceCtrl;
#endif
#endif /* OCE_SUPPORT */

#ifdef WSC_INCLUDED
	MLME_QUEUE_ELEM *Elem = NULL;
	UCHAR current_band = 0;
#endif /*WSC_INCLUDED*/

#ifdef DOT11_EHT_BE
	UINT8 aucMtkOui[] = VENDOR_OUI_MTK;
#endif
#ifdef EM_PLUS_SUPPORT
	UCHAR mbss_idx = MAIN_MBSSID;
#endif /*EM_PLUS_SUPPORT */

	if (MsgLen < LENGTH_802_11 + 2)
		return FALSE;

	/* NdisZeroMemory(ProbeReqParam, sizeof(*ProbeReqParam)); */
	COPY_MAC_ADDR(ProbeReqParam->Addr2, &Fr->Hdr.Addr2);
	COPY_MAC_ADDR(ProbeReqParam->Addr3, &Fr->Hdr.Addr3);
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(ProbeReqParam->Addr1, &Fr->Hdr.Addr1);
#endif

	if (Fr->Octet[0] != IE_SSID || Fr->Octet[1] > MAX_LEN_OF_SSID || Fr->Octet[1] > (MsgLen - LENGTH_802_11 - 2)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
			"sanity fail - wrong SSID IE\n");
		return FALSE;
	}
#ifdef EM_PLUS_SUPPORT
	if (IS_BROADCAST_MAC_ADDR(ProbeReqParam->Addr1)) {
		int apidx;
		/*deliver broadcast frame to all virtual interface */
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[apidx];
			struct wifi_dev *wdev_acl = &mbss->wdev;

			if (wdev_acl->if_dev == NULL)
				continue;

			/* other virtual bss may be not ready for processing this frame*/
			if (!netif_running(wdev_acl->if_dev))
				continue;

			/* fail in ACL checking =>  drop probe req. */
			if (!ApCheckAccessControlList(pAd, ProbeReqParam->Addr2, apidx)) {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
						"Device "MACSTR" ACL for wdev %s, block probe!!\n",
						MAC2STR(ProbeReqParam->Addr2),
						RtmpOsGetNetDevName(wdev_acl->if_dev));
				return FALSE;
			}
		}
	} else {
		mbss_idx = get_apidx_by_addr(pAd, ProbeReqParam->Addr1);

		/* fail in ACL checking =>  drop probe req. */
		if (!ApCheckAccessControlList(pAd, ProbeReqParam->Addr2, mbss_idx)) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
					"Device "MACSTR" ACL for wdev %s, block probe!!\n",
					MAC2STR(ProbeReqParam->Addr2),
					RtmpOsGetNetDevName(pAd->ApCfg.MBSSID[mbss_idx].wdev.if_dev));
			return FALSE;
		}
	}
#endif /* EM_PLUS_SUPPORT */


	/*
	 * MAP-R1 4.8.5 workaround
	 * Test case expect auth/assoc reject after probe response therefore
	 * dropping it here will cause test case to fail.
	 * Impact on band steering: Client will be able to see blocked bss as
	 * well, however connection will be rejected in auth phase.
	 * TODO: Define a seperate ioctl for probe withholding and do not
	 * call that in certification case. Correct apidex passed in below API.
	 */

	ProbeReqParam->SsidLen = Fr->Octet[1];
	NdisMoveMemory(ProbeReqParam->Ssid, &Fr->Octet[2], ProbeReqParam->SsidLen);
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(Addr1, &Fr->Hdr.Addr1);
#ifdef WSC_AP_SUPPORT
	os_alloc_mem(NULL, &pPeerWscIe, 512);
	if (pPeerWscIe == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
			"allocate memory for pPeerWscIe failed!\n");
		return FALSE;
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	Ptr = Fr->Octet;
	eid = Ptr[0];
	eid_len = Ptr[1];
	total_ie_len = eid_len + 2;
	eid_data = Ptr + 2;

	/* get variable fields from payload and advance the pointer*/
	while ((eid_data + eid_len) <= ((UCHAR *)Fr + MsgLen)) {
		switch (eid) {
		case IE_VENDOR_SPECIFIC:
			if (eid_len <= 4)
				break;

#ifdef RSSI_FEEDBACK

			if (ProbeReqParam->bRssiRequested &&
				NdisEqualMemory(eid_data, RALINK_OUI, 3) && (eid_len == 7)) {
				if (*(eid_data + 3/* skip RALINK_OUI */) & 0x8)
					ProbeReqParam->bRssiRequested = TRUE;

				break;
			}

#endif /* RSSI_FEEDBACK */
#ifdef WH_EVENT_NOTIFIER

			if (pAd->ApCfg.EventNotifyCfg.CustomOUILen &&
				(eid_len >= pAd->ApCfg.EventNotifyCfg.CustomOUILen) &&
				NdisEqualMemory(eid_data, pAd->ApCfg.EventNotifyCfg.CustomOUI, pAd->ApCfg.EventNotifyCfg.CustomOUILen)) {
				ie_lists->vendor_ie.custom_ie_len = eid_len;
				NdisMoveMemory(ie_lists->vendor_ie.custom_ie, eid_data, eid_len);
				break;
			}

#endif /* WH_EVENT_NOTIFIER */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			if ((pAd->ApCfg.ap_customer_oui.pointer != NULL) && (eid_len >= pAd->ApCfg.ap_customer_oui.length) &&
				NdisEqualMemory(eid_data, pAd->ApCfg.ap_customer_oui.pointer, pAd->ApCfg.ap_customer_oui.length)) {
				ProbeReqParam->report_param.vendor_ie.element_id = IE_VENDOR_SPECIFIC;
				ProbeReqParam->report_param.vendor_ie.len = eid_len;
				NdisMoveMemory(ProbeReqParam->report_param.vendor_ie.custom_ie, eid_data, eid_len);
				break;
			}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

			if (NdisEqualMemory(eid_data, WPS_OUI, 4)) {
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED

				Elem = CONTAINER_OF(Msg, MLME_QUEUE_ELEM, Msg[0]);
				current_band = HcGetBandByChannel(pAd, Elem->Channel);
				WscCheckPeerDPID(pAd, Fr, eid_data, eid_len, current_band);
#ifdef CONFIG_AP_SUPPORT
				/* Ignore old WPS IE fragments, if we get the version 0x10 */
				/* First WPS IE will have version 0x10 */
				if (*(eid_data + 4) == 0x10) {
					NdisMoveMemory(pPeerWscIe, eid_data + 4, eid_len - 4);
					PeerWscIeLen = (eid_len - 4);
				} else { /* reassembly remanning, */
					/* other IE fragmentations will not have version 0x10 */
					if ((PeerWscIeLen + (eid_len - 4)) <= 512) {
						NdisMoveMemory(pPeerWscIe + PeerWscIeLen, eid_data + 4, eid_len - 4);
						PeerWscIeLen += (eid_len - 4);
					} else { /* ((PeerWscIeLen +(eid_len-4)) > 512) */
						bWscCheck = FALSE;
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_ERROR,
							"Error!Sum of All PeerWscIeLen = %d (> 512)\n",
							(PeerWscIeLen + (eid_len - 4)));
					}
				}
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
			}
#ifdef MBO_SUPPORT
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			if (IS_OCE_ENABLE(wdev)) {
				if ((eid_data[0] == MBO_OCE_OUI_0) &&
						(eid_data[1] == MBO_OCE_OUI_1) &&
						(eid_data[2] == MBO_OCE_OUI_2) &&
						(eid_data[3] == MBO_OCE_OUI_TYPE) &&
						(eid_len >= 5))
					OceParseStaOceIE(pAd, eid_data, eid_len, ProbeReqParam);
			}
#endif /* CONFIG_AP_SUPPORT */
#endif /* OCE_SUPPORT */
#endif /* MBO_SUPPORT */
#ifdef DOT11_EHT_BE
			/* Parse MTK OUI */
			if (eid_len > 7 && NdisEqualMemory(eid_data, aucMtkOui, 3))
				parse_mtk_tlv1_vendor_ie(pAd, eid_data-2, &ProbeReqParam->cmm_ies.vendor_ie);
#endif
			break;

#ifdef CONFIG_HOTSPOT

		case IE_INTERWORKING:
			ProbeReqParam->AccessNetWorkType = (*eid_data) & 0x0F;

			if (eid_len > 3) {
				if (eid_len == 7)
					NdisMoveMemory(ProbeReqParam->Hessid, eid_data + 1, MAC_ADDR_LEN);
				else
					NdisMoveMemory(ProbeReqParam->Hessid, eid_data + 3, MAC_ADDR_LEN);

				ProbeReqParam->IsHessid = TRUE;
			}

			ProbeReqParam->IsIWIE = TRUE;
			break;
#endif

		case IE_EXT_CAPABILITY:
#ifdef CONFIG_HOTSPOT
			if (eid_len >= 4) {
				if (((*(eid_data + 3)) & 0x80) == 0x80)
					ProbeReqParam->IsIWCapability = TRUE;
			}

#endif
			break;
#if (defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING) || defined(WH_EVENT_NOTIFIER))

		case IE_HT_CAP:
			if (parse_ht_cap_ie(eid_len)) {
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
#ifdef CCN67_BS_SUPPORT
				if (PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
				if (pAd->ApCfg.BandSteering)
#endif
				{
					ProbeReqParam->IsHtSupport = TRUE;
					ProbeReqParam->RxMCSBitmask = *(UINT32 *)(eid_data + 3);
				}

#endif
#ifdef WH_EVENT_NOTIFIER
				NdisMoveMemory(&ie_lists->HTCapability, eid_data, SIZE_HT_CAP_IE);
				*(USHORT *)(&ie_lists->HTCapability.HtCapInfo) = cpu2le16(*(USHORT *)(&ie_lists->HTCapability.HtCapInfo));
				*(USHORT *)(&ie_lists->HTCapability.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&ie_lists->HTCapability.ExtHtCapInfo));
				SET_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
#endif /* WH_EVENT_NOTIFIER */
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
					"wrong IE_HT_CAP. eid_len = %d\n", eid_len);
#ifdef WSC_AP_SUPPORT
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
				return FALSE;
			}
			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (eid_len >= SIZE_OF_VHT_CAP_IE) {
#if defined(CCN67_BS_SUPPORT) || defined(BAND_STEERING)
#ifdef CCN67_BS_SUPPORT
				if (PD_GET_MESH_ENABLED(pAd->physical_dev))
#else
				if (pAd->ApCfg.BandSteering)
#endif
					ProbeReqParam->IsVhtSupport = TRUE;
#endif

#ifdef WH_EVENT_NOTIFIER
				NdisMoveMemory(&ie_lists->vht_cap, eid_data, eid_len);
				SET_VHT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
#endif /* WH_EVENT_NOTIFIER */
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
				ProbeReqParam->bfer_cap_su = ((VHT_CAP_IE *)eid_data)->vht_cap.bfer_cap_su;
				ProbeReqParam->num_snd_dimension = ((VHT_CAP_IE *)eid_data)->vht_cap.num_snd_dimension;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
					"wrong IE_VHT_CAP. eid_len = %d\n", eid_len);
#ifdef WSC_AP_SUPPORT
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
				return FALSE;
			}
			break;
#endif /* DOT11_VHT_AC */
#endif

		case IE_WLAN_EXTENSION:
			if (eid_len < 1) {
#ifdef WSC_AP_SUPPORT
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
				return FALSE;
			}
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT

			if (IS_OCE_ENABLE(wdev) && ProbeReqParam->IsOceCapability &&
				*(eid_data) == FILS_REQ_ID_EXTENSION &&
				MAC_ADDR_EQUAL(ProbeReqParam->Addr1, BROADCAST_ADDR)) {
				if (eid_len - 2 < sizeof(MaxChannelTime)) {
#ifdef WSC_AP_SUPPORT
					if (pPeerWscIe)
						os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
					return FALSE;
				}
				MaxChannelTime = *(eid_data + 2);
				ProbeReqParam->MaxChannelTime = MaxChannelTime;
				if (!pOceCtrl->MaxChannelTimerRunning) {
					RTMPSetTimer(&pOceCtrl->MaxChannelTimer, MaxChannelTime);
					pOceCtrl->MaxChannelTimesUp = FALSE;
					pOceCtrl->MaxChannelTimerRunning = TRUE;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#endif /* OCE_SUPPORT */
#ifdef DOT11_HE_AX
			/*parse_he_probe_req_ies(Ptr, ie_lists);*/

			if (*(eid_data) == EID_EXT_SHORT_SSID_LIST) {
				if (eid_len <= 1) /* bypass for length is zero */
					;
				else if (eid_len - 1 < SHORT_SSID_LEN) {
#ifdef WSC_AP_SUPPORT
					if (pPeerWscIe)
						os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_WARN,
						"wrong Short SSID length. eid_len = %d\n", eid_len);
					return FALSE;
				} else {
					NdisMoveMemory(&ProbeReqParam->ShortSSID, eid_data + 1, SHORT_SSID_LEN);
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SANITY, DBG_LVL_INFO,
						"short ssid = %08x\n", ProbeReqParam->ShortSSID);
				}
			}
#endif /*DOT11_HE_AX*/
#ifdef DOT11_EHT_BE
		if (*(eid_data) == EID_EXT_EHT_MULTI_LINK
				|| *(eid_data) == EID_EXT_EHT_CAPS
				|| *(eid_data) == EID_EXT_EHT_OP)
			if (eht_get_ies((UINT8 *)(eid_data - 2), &ProbeReqParam->cmm_ies, NULL) < 0) {
#ifdef WSC_AP_SUPPORT
				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);
#endif /* WSC_AP_SUPPORT */
				return FALSE;
			}
#endif /* DOT11_EHT_BE */
			break;

		default:
			break;
		}

		if (MsgLen - LENGTH_802_11 - total_ie_len < 2)
			break;
		eid = Ptr[total_ie_len];
		eid_len = Ptr[total_ie_len + 1];
		eid_data = Ptr + total_ie_len + 2;
		total_ie_len += (eid_len + 2);
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_INCLUDED

	if (pPeerWscIe && (PeerWscIeLen > 0) && (bWscCheck == TRUE)) {
/* WPS_BandSteering Support */
#ifdef BAND_STEERING
		ProbeReqParam->bWpsCapable = TRUE;
#endif
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if (NdisEqualMemory(Addr1, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
				break;
		}

		/*
			Due to Addr1 in Probe Request may be FF:FF:FF:FF:FF:FF
			and we need to send out this information to external registrar.
			Therefore we choose ra0 to send this probe req when we couldn't find apidx by Addr1.
		*/
		if (apidx >= pAd->ApCfg.BssidNum)
			apidx = MAIN_MBSSID;

		if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode & WSC_PROXY) != WSC_DISABLE) {
			int bufLen = 0;
			PUCHAR pBuf = NULL;
			WSC_IE_PROBREQ_DATA	*pprobreq = NULL;
			/*
				PeerWscIeLen: Len of WSC IE without WSC OUI
			*/
			bufLen = sizeof(WSC_IE_PROBREQ_DATA) + PeerWscIeLen;
			os_alloc_mem(NULL, &pBuf, bufLen);

			if (pBuf) {
				/*Send WSC probe req to UPnP*/
				NdisZeroMemory(pBuf, bufLen);
				pprobreq = (WSC_IE_PROBREQ_DATA *)pBuf;

				if (ProbeReqParam->SsidLen <= 32) {	/*Well, I think that it must be TRUE!*/
					NdisMoveMemory(pprobreq->ssid, ProbeReqParam->Ssid, ProbeReqParam->SsidLen);			/* SSID*/
					NdisMoveMemory(pprobreq->macAddr, Fr->Hdr.Addr2, 6);	/* Mac address*/
					pprobreq->data[0] = PeerWscIeLen >> 8;									/* element ID*/
					pprobreq->data[1] = PeerWscIeLen & 0xff;							/* element Length					*/
					NdisMoveMemory((pBuf + sizeof(WSC_IE_PROBREQ_DATA)), pPeerWscIe, PeerWscIeLen);	/* (WscProbeReqData)*/
					WscSendUPnPMessage(pAd, apidx,
									   WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_PROBE_REQ,
									   pBuf, bufLen, 0, 0, &Fr->Hdr.Addr2[0], AP_MODE);
				}

				os_free_mem(pBuf);
			}
		}
	}

	if (pPeerWscIe)
		os_free_mem(pPeerWscIe);

#endif /* WSC_INCLUDED */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

/*
========================================================================
Routine Description:
	Check a packet is Action frame or not

Arguments:
	pAd			- WLAN control block pointer
	pbuf			- packet buffer

Return Value:
	TRUE		- yes
	FALSE		- no

========================================================================
*/
BOOLEAN
IsPublicActionFrame(
	IN PRTMP_ADAPTER	pAd,
	IN VOID * pbuf
)
{
	HEADER_802_11 *pHeader = pbuf;
	UINT8 *ptr = pbuf;

	if (pHeader->FC.Type != FC_TYPE_MGMT)
		return FALSE;

	if (pHeader->FC.SubType != SUBTYPE_ACTION)
		return FALSE;

	ptr += sizeof(HEADER_802_11);

	if (*ptr == CATEGORY_PUBLIC
#ifdef DOT11W_PMF_SUPPORT
		|| (*ptr == CATEGORY_PD)
#endif /* DOT11W_PMF_SUPPORT */
	)
		return TRUE;
	else
		return FALSE;
}
