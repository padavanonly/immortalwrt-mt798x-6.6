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
#include "rt_config.h"

#ifdef DOT11_EHT_BE
struct mld_dev mld_device;

struct mld_link_entry *ger_unused_link(struct mld_dev *mld)
{
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (!mld->mld_own_links[i].used)
			return &mld->mld_own_links[i];
	}

	return NULL;
}

int sta_mld_link_mgr_reg_dev(struct wifi_dev *wdev)
{
	struct mld_link_entry *link = NULL;
	struct mld_dev *mld = &mld_device;
	int i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (link->wdev == wdev && link->used == 1) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: already register wdev as mld link\n",
				__func__);
			return 0;
		}
	}

	link = ger_unused_link(mld);
	if (!link) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"%s: ERROR!!! own mld link number %d\n",
				__func__, mld->valid_link_num);
		return -1;
	}
	os_zero_mem(link, sizeof(struct mld_link_entry));

	link->wdev = wdev;
	link->used = 1;

	if (mld->valid_link_num == 0) {
		mld->master_link = link;
		os_move_mem(mld->mld_addr, wdev->if_addr, MAC_ADDR_LEN);
	}
	mld->valid_link_num++;
	wdev->mld_dev = mld;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"%s: mld_dev: 0x%p, wdev(%d)(%pM): 0x%p, link: 0x%p\n",
			__func__, mld, wdev->func_idx, wdev->if_addr, wdev, link);

	return 0;
}

void sta_mld_link_mgr_dereg_dev(struct wifi_dev *wdev)
{
	struct mld_dev *mld = &mld_device;
	struct mld_link_entry *link = NULL;
	uint8_t i = 0, j = 0;
	struct wifi_dev *pCurwdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"%s: mld_dev: 0x%p, wdev(%d)(%pM): 0x%p\n",
			__func__, mld, wdev->func_idx, wdev->if_addr, wdev);

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];

		if (link->wdev == wdev) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"link: 0x%p\n", link);
			mld->valid_link_num--;
			if (!link->is_setup_link && link->used) {
				pCurwdev = mld->mld_own_links[i].wdev;
				if (pCurwdev && pCurwdev->wdev_ops && pCurwdev->wdev_ops->linkdown) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"(%02x:%02x:%02x:%02x:%02x:%02x) to do link down\n",
						PRINT_MAC(pCurwdev->if_addr));
					pCurwdev->wdev_ops->linkdown(pCurwdev);
				}
			}
			link->used = 0;

			/* change master link*/
			if (mld->master_link == link) {
				for (j = 0; j < MLD_LINK_MAX; j++)
					if (mld->mld_own_links[j].used && mld->mld_own_links[j].wdev) {
						mld->master_link = &mld->mld_own_links[j];
						os_move_mem(mld->mld_addr, mld->master_link->wdev->if_addr, MAC_ADDR_LEN);
						break;
					}
			}
			if (j == MLD_LINK_MAX)
				mld->master_link = NULL;

			if (mld->peer_mld.single_link[i].is_peer_need_clear == 1)
				os_zero_mem(&mld->peer_mld, sizeof(mld->peer_mld));
			break;
		}
	}
}

struct wifi_dev *get_sta_mld_setup_wdev(struct mld_dev *mld)
{
	if (mld && mld->peer_mld.valid && mld->peer_mld.set_up_link
		&& mld->peer_mld.set_up_link->is_setup_link)
		return mld->peer_mld.set_up_link->wdev;

	return NULL;
}


struct mld_link_entry *get_sta_mld_link_by_idx(struct mld_dev *mld, uint8_t link_idx)
{
	/* sanity check*/
	if (mld != &mld_device)
		return NULL;

	if (link_idx >= MLD_LINK_MAX)
		return NULL;

	return mld->mld_own_links[link_idx].used == 0 ? NULL : &mld->mld_own_links[link_idx];
}

struct mld_link_entry *get_sta_mld_link_by_wdev(struct mld_dev *mld, struct wifi_dev *wdev)
{
	struct mld_link_entry *link = NULL;
	uint8_t i = 0;

	if (mld != &mld_device)
		goto end;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];

		if (link->used && link->wdev == wdev)
			return link;
	}

end:
	return NULL;
}

int sta_mld_ra_init(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	uint8_t i = 0;
	struct mld_dev *mld = wdev->mld_dev;
	MAC_TABLE_ENTRY *pCurEntry = NULL;

	chip_ra_init(pAd, pEntry);

	if (mld != &mld_device)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry || pCurEntry == NULL)
				continue;
			chip_ra_init(pAd, pCurEntry);
		}
	}

	return 0;
}

int sta_mld_disconn_req(struct wifi_dev *wdev)
{
	struct mld_dev *mld = wdev->mld_dev;
	struct peer_mld_entry *peer = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	struct wifi_dev *tmp_wdev = NULL;
	UCHAR *link_addr = NULL;
	uint8_t i = 0;

	if (mld != &mld_device)
		return -1;

	peer = &mld->peer_mld;

	if (!peer->valid)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (peer->single_link[i].active) {
			tmp_wdev = mld->mld_own_links[i].wdev;
			ad = (struct _RTMP_ADAPTER *)tmp_wdev->sys_handle;
			link_addr = peer->single_link[i].link_addr;
			pEntry = MacTableLookup(ad, link_addr);
			if (pEntry) {
				mld->mld_own_links[i].is_setup_link = 0;
				MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
			"own_idx(%d) WDEV("MACSTR") link_addr("MACSTR")entry del it!\n",
					i, MAC2STR(tmp_wdev->if_addr), MAC2STR(link_addr));
				_MacTableDeleteEntry(ad, pEntry->wcid, pEntry->Addr);
				if (IS_AKM_WPA_CAPABILITY_Entry(tmp_wdev))
					/* Remove all apcli wdev WPA keys */
					RTMPWPARemoveAllKeys(ad, tmp_wdev);
				/* init mlo_en */
				pEntry->mlo.mlo_en = 0;
			} else {
				MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
			"own_idx(%d) WDEV("MACSTR") link_addr("MACSTR")entry not exist!\n",
					i, MAC2STR(tmp_wdev->if_addr), MAC2STR(link_addr));
			}
			if (peer->single_link[i].active && peer->valid == 1)
				peer->single_link[i].is_peer_need_clear = 1;

			peer->single_link[i].priv_ptr = NULL;
			peer->single_link[i].active = 0;
		}
	}
	return 0;
}

INT sta_sync_no_inheritance_cap(struct common_ies *cmm_ies)
{
	UINT8 ie_len = ((struct _EID_STRUCT *)cmm_ies->no_inherit_ie)->Len - 1;
	UINT8 *ie_ctx = cmm_ies->no_inherit_ie + sizeof(struct _EID_STRUCT);
	UINT8 elem_id = 0, idx = 0;
	UINT8 ie_list_len = 0, *ie_list_ies = NULL, ext_ie_list_len = 0, *ext_ie_list_ies = NULL;
	PEDCA_PARM pEdcaParm = &cmm_ies->vendor_ie.EdcaParm;

	if (!ie_len)
		return -1;

	/*ie list*/
	ie_list_len = *ie_ctx;
	ie_list_ies = ie_ctx+1;
	if (ie_list_len + 3 < ie_len) { /* 3: Element ID + Length + Element ID extension */
		/*ext_ie_list*/
		ext_ie_list_len = *(ie_ctx + 1 + ie_list_len);
		ext_ie_list_ies = ie_ctx + 1 + ie_list_len;
	}
	/*sanity check*/
	if (ie_len != (ie_list_len + 1 + ext_ie_list_len + 1)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"(%s)[%d] length not match, ie_len(%d)!= ie_list_len(%d)+1+ext_ie_list_len(%d)+1\n",
			__func__, __LINE__, ie_len, ie_list_len, ext_ie_list_len);
		return -1;
	}
	/*ie list check*/
	for (idx = 0; idx < ie_list_len; idx++) {
		elem_id = *(ie_list_ies + idx);
		switch (elem_id) {
		case IE_HT_CAP:
			if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
				CLR_HT_CAPS_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->ht_cap, sizeof(cmm_ies->ht_cap));
			}
			break;
		case IE_ADD_HT:
			if (HAS_HT_OP_EXIST(cmm_ies->ie_exists)) {
				CLR_HT_OP_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->ht_op, sizeof(cmm_ies->ht_op));
			}
			break;
		case IE_VHT_CAP:
			if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
				CLR_VHT_CAPS_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->vht_cap, sizeof(cmm_ies->vht_cap));
			}
			break;
		case IE_VHT_OP:
			if (HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
				CLR_VHT_OP_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->vht_op, sizeof(cmm_ies->vht_op));
			}
			break;
		case IE_VENDOR_SPECIFIC:
			if (pEdcaParm->bValid == TRUE)
				os_zero_mem(pEdcaParm, sizeof(EDCA_PARM));
			break;
		default:
			break;
		}
	}
	/*ext ie list check*/
	for (idx = 0; idx < ext_ie_list_len; idx++) {
		elem_id = *(ext_ie_list_ies + idx);
		switch (elem_id) {
		case EID_EXT_HE_CAPS:
			if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
				CLR_HE_CAPS_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->he_caps, sizeof(cmm_ies->he_caps));
			}
			break;
		case EID_EXT_HE_6G_CAPS:
			if (HAS_HE_6G_CAP_EXIST(cmm_ies->ie_exists)) {
				CLR_HE_6G_CAP_EXIST(cmm_ies->ie_exists);
				os_zero_mem(&cmm_ies->he6g_caps, sizeof(cmm_ies->he6g_caps));
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

int fill_peer_mlo_info(struct peer_mld_entry *peer, uint8_t link_num,
	uint8_t is_setup_link, uint16_t setup_wcid, OUT struct eht_mlo_t *mlo)
{
	struct eht_link_t *link_info = NULL;
	uint8_t link_id = 0;

	mlo->mlo_en = TRUE;
	mlo->is_setup_link_entry = is_setup_link;

	link_info = &mlo->link_info;
	link_id = link_info->link_id;

	os_zero_mem(link_info, sizeof(struct eht_link_t));
	link_info->link_id = link_id;
	link_info->tid_map_dl = 0xff;	/*default to 0xff*/
	link_info->tid_map_ul = 0xff;	/*default to 0xff*/
	link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);

	return 0;
}

/* Parse per sta profile and store into ie_list */
VOID sta_parse_sta_profile_cap(UINT8 *sta_profile, UINT16 sta_profile_len, IE_LISTS *ie_list, int flag)
{
	USHORT status_code, peer_cap_info;
	struct _EID_STRUCT *sta_pf_eid = NULL;
	UINT16 Length = 0;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;
	PEDCA_PARM pEdcaParm = &cmm_ies->vendor_ie.EdcaParm;

	if (!sta_profile) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "sta profile is NULL!\n");
		return;
	}

	NdisMoveMemory(&peer_cap_info, sta_profile, 2);
	if (flag == 1) {
		NdisMoveMemory(&status_code, sta_profile + 2, 2);
		sta_pf_eid = (struct _EID_STRUCT *)(sta_profile + 4);
		Length = 4;
	} else {
		sta_pf_eid = (struct _EID_STRUCT *)(sta_profile + 2);
		Length = 2;
	}

	if (sta_profile_len - Length < 2) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"sta_profile_len is invalid!\n");
		return;
	}
	while ((Length + 2 + sta_pf_eid->Len) <= sta_profile_len) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"per link[Eid=%d]\n", sta_pf_eid->Eid);
		switch (sta_pf_eid->Eid) {
		case IE_SUPP_RATES:
			parse_support_rate_ie(rate, sta_pf_eid);
			break;
		case IE_EXT_SUPP_RATES:
			parse_support_ext_rate_ie(rate, sta_pf_eid);
			break;
#ifdef DOT11_N_SUPPORT
		case IE_HT_CAP:
			if (sta_pf_eid->Len >= SIZE_HT_CAP_IE) {
				NdisMoveMemory(&cmm_ies->ht_cap, sta_pf_eid->Octet, sizeof(HT_CAPABILITY_IE));
				SET_HT_CAPS_EXIST(cmm_ies->ie_exists);
				*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo) = cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.HtCapInfo));
				*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&cmm_ies->ht_cap.ExtHtCapInfo));
				*(USHORT *)(&cmm_ies->ht_cap.TxBFCap) = le2cpu32(*(USHORT *)(&cmm_ies->ht_cap.TxBFCap));
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"wrong IE_HT_CAP\n");

			break;
		case IE_ADD_HT:
			if (sta_pf_eid->Len >= sizeof(ADD_HT_INFO_IE)) {
				/*
				 *	 This IE allows extension,
				 *	 but we can ignore extra bytes beyond our knowledge,
				 *	 so only copy first sizeof(ADD_HT_INFO_IE)
				 */
				NdisMoveMemory(&cmm_ies->ht_op, sta_pf_eid->Octet, sizeof(ADD_HT_INFO_IE));
				SET_HT_OP_EXIST(cmm_ies->ie_exists);
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2) = cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo2));
				*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3) = cpu2le16(*(USHORT *)(&cmm_ies->ht_op.AddHtInfo3));
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"wrong IE_ADD_HT\n");

			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (sta_pf_eid->Len == sizeof(VHT_CAP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT32 tmp_1;
				UINT64 tmp_2;
#endif

				NdisMoveMemory(&cmm_ies->vht_cap, sta_pf_eid->Octet, sizeof(VHT_CAP_IE));
				SET_VHT_CAPS_EXIST(cmm_ies->ie_exists);

#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &cmm_ies->vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&cmm_ies->vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(cmm_ies->vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(cmm_ies->vht_cap.mcs_set), &tmp_2, 8);
#endif
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"wrong IE_VHT_CAP\n");

			break;

		case IE_VHT_OP:
			if (sta_pf_eid->Len == sizeof(VHT_OP_IE)) {
#ifdef CFG_BIG_ENDIAN
				UINT16 tmp;
#endif
				NdisMoveMemory(&cmm_ies->vht_op, sta_pf_eid->Octet, sizeof(VHT_OP_IE));
				SET_VHT_OP_EXIST(cmm_ies->ie_exists);
#ifdef CFG_BIG_ENDIAN
				NdisCopyMemory(&tmp, &cmm_ies->vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&cmm_ies->vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"wrong IE_VHT_OP\n");

			break;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		case IE_WLAN_EXTENSION:
				/*parse EXTENSION EID*/
				if (sta_profile_len - Length < sizeof(struct _EID_STRUCT)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
						"wrong IE length (at least eid + length + extension eid)\n");
					return;
				}
				parse_he_assoc_rsp_ies((UINT8 *)sta_pf_eid, ie_list);
				if (eht_get_ies((UINT8 *)sta_pf_eid, cmm_ies, NULL) < 0)
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
						"wrong IE of ETH\n");
				break;
		case IE_VENDOR_SPECIFIC:
			if ((sta_pf_eid->Len == 24) && NdisEqualMemory(sta_pf_eid->Octet, WME_PARM_ELEM, 6)) {
				PUCHAR ptr;
				int i;
				/* parsing EDCA parameters */
				pEdcaParm->bValid = TRUE;
				pEdcaParm->bQAck = FALSE;	/* pEid->Octet[0] & 0x10; */
				pEdcaParm->bQueueRequest = FALSE;	/* pEid->Octet[0] & 0x20; */
				pEdcaParm->bTxopRequest = FALSE;	/* pEid->Octet[0] & 0x40; */
				pEdcaParm->EdcaUpdateCount =
					sta_pf_eid->Octet[6] & 0x0f;
				pEdcaParm->bAPSDCapable =
					(sta_pf_eid->Octet[6] & 0x80) ? 1 : 0;
				ptr = (PUCHAR) &sta_pf_eid->Octet[8];

				for (i = 0; i < 4; i++) {
					UCHAR aci = (*ptr & 0x60) >> 5; /* b5~6 is AC INDEX */

					pEdcaParm->bACM[aci] = (((*ptr) & 0x10) == 0x10);
					/* b5 is ACM */
					pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;
					/* b0~3 is AIFSN */
					pEdcaParm->Cwmin[aci] = *(ptr + 1) & 0x0f;
					/* b0~4 is Cwmin */
					pEdcaParm->Cwmax[aci] = *(ptr + 1) >> 4;
					/* b5~8 is Cwmax */
					pEdcaParm->Txop[aci] = *(ptr + 2) + 256 * (*(ptr + 3));
					/* in unit of 32-us */
					ptr += 4;
					/* point to next AC */
				}
			}
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"ignore unrecognized EID = %d\n", sta_pf_eid->Eid);
				break;
		}
		Length = Length + 2 + sta_pf_eid->Len;
		sta_pf_eid = (PEID_STRUCT)((UCHAR *)sta_pf_eid + 2 + sta_pf_eid->Len);
		if (sta_profile_len - Length < 2)
			break;
	}
	/*no-inheritance ie list sync*/
	if (cmm_ies->no_inherit_ie)
		sta_sync_no_inheritance_cap(cmm_ies);
}

static VOID sta_sync_sta_profile_mlo_csa(RTMP_ADAPTER *pAd, UINT8 *sta_profile, UINT16 sta_profile_len, int flag)
{
	struct _EID_STRUCT *sta_pf_eid = NULL;
	USHORT status_code, peer_cap_info;
	UINT16 Length = 0;
	struct MLO_CSA_INFO *mlo_csa_info = &pAd->Dot11_H.mlo_csa_info;

	if (!sta_profile) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "sta_profile is NULL!\n");
		return;
	}

	NdisMoveMemory(&peer_cap_info, sta_profile, 2);
	if (flag == 1) {
		NdisMoveMemory(&status_code, sta_profile + 2, 2);
		sta_pf_eid = (struct _EID_STRUCT *)(sta_profile + 4);
	} else
		sta_pf_eid = (struct _EID_STRUCT *)(sta_profile + 2);
	while ((Length + 2 + sta_pf_eid->Len) <= sta_profile_len) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"per link[Eid=%d]\n", sta_pf_eid->Eid);
		switch (sta_pf_eid->Eid) {
		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ch_switch_announcement_ie(sta_pf_eid)) {
				NdisMoveMemory(&mlo_csa_info->ChSwAnnIE, &sta_pf_eid->Octet[0], sta_pf_eid->Len);
				mlo_csa_info->new_channel = mlo_csa_info->ChSwAnnIE.Channel;
			} else
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "%s() - wrong IE_CHANNEL_SWITCH_ANNOUNCEMENT in STA profile\n", __func__);
			break;

		case IE_WLAN_EXTENSION: {
			INT8 max_ch_swich_len = sta_pf_eid->Len;
			UCHAR *subelement = &sta_pf_eid->Octet[0];

			if (max_ch_swich_len > 0) {

				if (*subelement == IE_EXTENSION_ID_MAX_CH_SWITCH_TIME) {
					subelement += 1;
					NdisMoveMemory(&mlo_csa_info->MCST_info.SwitchTime, &subelement, 2); /*extract the maxchannel switch time*/
					mlo_csa_info->MCST_info.SwitchTime = le2cpu16(mlo_csa_info->MCST_info.SwitchTime);
				} else {
					subelement  += max_ch_swich_len;
					max_ch_swich_len -= max_ch_swich_len;
					MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "%s() - wrong IE_EXTENSION_ID_MAX_CH_SWITCH_TIME in STA profile\n", __func__);
				}
			}
			break;
		}
		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ext_ch_switch_announcement_ie(sta_pf_eid))
				NdisMoveMemory(&mlo_csa_info->ExtChSwAnnIE, &sta_pf_eid->Octet[0], sta_pf_eid->Len);
			else {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "%s() - wrong IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n", __func__);
				return;
			}
			break;
		case IE_CH_SWITCH_WRAPPER: {
			INT8 ch_sw_wrp_len = sta_pf_eid->Len;
			UCHAR *subelement = &sta_pf_eid->Octet[0];
			INT8		len_subelement = 0;

			if (ch_sw_wrp_len > 0) {
				len_subelement = *(subelement + 1);

				if (*subelement == IE_WIDE_BW_CH_SWITCH) {
					subelement += 2;
					NdisMoveMemory(&mlo_csa_info->wb_info, subelement, len_subelement);
				} else {
					subelement  += ch_sw_wrp_len;
					ch_sw_wrp_len -= ch_sw_wrp_len;
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLO_BMGR, DBG_LVL_NOTICE, "%s() - wrong IE_WIDE_BW_CH_SWITCH\n", __func__);
				}
			}
		}
			break;
		default:
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "%s():ignore unrecognized EID = %d\n", __func__, sta_pf_eid->Eid);
				break;
		}
		Length = Length + 2 + sta_pf_eid->Len;
		sta_pf_eid = (PEID_STRUCT)((UCHAR *)sta_pf_eid + 2 + sta_pf_eid->Len);
	}
}
int sta_mld_insert_link(struct mld_dev *mld, struct peer_mld_entry *peer, struct ml_ie_info *ml_info,
	int own_idx, int peer_idx, uint16_t *setup_wcid, struct mld_link_entry *set_up_link, uint8_t *link_num)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct wifi_dev *tmp_wdev = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if ((own_idx < 0) || (own_idx >= MLD_LINK_MAX) ||
		(peer_idx < 0) || (peer_idx >= BSS_MNGR_MAX_BAND_NUM)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"own links idx(%d) or peer idx(%d) error!!!\n", own_idx, peer_idx);
		return -1;
	}

	if (mld->mld_own_links[own_idx].used == 0) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"own links idx(%d) is not up, not insert this mld link.\n", own_idx);
		return -1;
	}

	peer->single_link[own_idx].active = 1;
	COPY_MAC_ADDR(peer->single_link[own_idx].link_addr, ml_info->link[peer_idx].link_addr);
	peer->single_link[own_idx].sta_ctrl = ml_info->link[peer_idx].sta_ctrl;
	peer->single_link[own_idx].bcn_interval = ml_info->link[peer_idx].bcn_interval;
	peer->single_link[own_idx].dtim_count = ml_info->link[peer_idx].dtim_count;
	peer->single_link[own_idx].dtim_period = ml_info->link[peer_idx].dtim_period;
	peer->single_link[own_idx].nstr_bmap = ml_info->link[peer_idx].nstr_bmap;
	peer->single_link[own_idx].link_id = ml_info->link[peer_idx].link_id;

	/*
	 * why we assume link match to each other
	 * of own links and links in ie
	 */
	tmp_wdev = mld->mld_own_links[own_idx].wdev;
	ad = (struct _RTMP_ADAPTER *)tmp_wdev->sys_handle;
	if (ad->MacTab->Size >= GET_MAX_UCAST_NUM(ad)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tpEntry: MAC table full!!\n");
		return -1;
	}
	/* 5. insert mac table entry for each link*/
	pEntry = MacTableLookup(ad, peer->single_link[own_idx].link_addr);
	if (pEntry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"ad=%p, wdev(%pM) link_addr=%pM mld_addr(%pM)\n",
			ad, tmp_wdev->if_addr, peer->single_link[own_idx].link_addr,
			peer->mld_addr);
		/* allocate one MAC entry */
		pEntry = MacTableInsertEntry(ad, peer->single_link[own_idx].link_addr, tmp_wdev,
			ENTRY_INFRA, OPMODE_STA, TRUE, peer->idx, peer->mld_addr);
		if (pEntry) {
			peer->single_link[own_idx].priv_ptr = (void *)pEntry;
			if (tmp_wdev == set_up_link->wdev)
				*setup_wcid = pEntry->wcid;
			pEntry->mlo.link_info.link_id =  ml_info->link[peer_idx].link_id;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\twdev(%02x:%02x:%02x:%02x:%02x:%02x) pEntry(%p): wcid(%d) setup_wcid(%d) acquired!!\n",
				PRINT_MAC(tmp_wdev->if_addr), pEntry, pEntry->wcid, *setup_wcid);
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\tpEntry: acquire failed!!\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tpEntry(%p): wcid(%d) allocated!!\n", pEntry, pEntry->wcid);
	}
	(*link_num)++;
	return 0;
}

int sta_parse_multi_link_per_sta_profile(u8 *per_sta_pf, u16 pf_len, struct ml_ie_info *ml_info, int *link_num)
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

		if (*link_num < BSS_MNGR_MAX_BAND_NUM) {
			ml_link = &ml_info->link[*link_num];

			ml_link->link_id = link_id;
			ml_link->active = TRUE;
			ml_link->sta_ctrl = sta_ctrl;
			(*link_num)++;

			if (ml_info->type == ML_CTRL_TYPE_BASIC) {
				/* STA Info Length */
				total_len += sizeof(ml_link->sta_info_len);
				if (total_len > pf_len)
					return -1;
				NdisMoveMemory(&ml_link->sta_info_len, pos, sizeof(ml_link->sta_info_len));
				pos += sizeof(ml_link->sta_info_len);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						 "\t sta_info_len(%d)\n", ml_link->sta_info_len);

				/* STA Info - MAC Address Present */
				if (GET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl)) {
					total_len += MAC_ADDR_LEN;
					if (total_len > pf_len)
						return -1;
					COPY_MAC_ADDR(ml_link->link_addr, pos);
					pos += MAC_ADDR_LEN;

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
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

		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN, "\t link_num(%d) not supported\n", *link_num);
		}
	}

	return 0;
}

int sta_parse_multi_link_ie(u8 *ml_ie, struct frag_ie_info *frag_info, struct ml_ie_info *ml_info)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)ml_ie;
	u8 *pos = ml_ie;
	u8 *ml_ie_end = NULL, *pf_pos = NULL, *sub_ie_buf;
	u16 ml_ie_len = 0, sub_ie_len = 0;
	u16 sub_ie_total_len = 0;
	u16 mld_ctrl = 0;
	struct _EID_STRUCT *sub_eid = NULL;
	u16 present_bmap = 0;
	int link_num = 0;
	int total_len = 0;
	struct _EID_STRUCT *tmp_sub_eid = NULL;

	if (eid->Eid == IE_WLAN_EXTENSION) {
		ml_ie_len = (frag_info && frag_info->is_frag) ? (frag_info->ie_len_defrag - 2) : eid->Len;
		ml_ie_end = ml_ie + 2 + ml_ie_len;

		if (ml_ie_len && (eid->Octet[0] == EID_EXT_EHT_MULTI_LINK)) {
			if (pos + sizeof(struct _EID_STRUCT) > ml_ie_end) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"ML IE is abnormal!\n");
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
							 "\teml_caps(0x%x)\n", ml_info->eml_caps);
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
					"Error: unknown ML IE type(%d)\n", ml_info->type);
			}

			hex_dump_with_cat_and_lvl("ML IE:", (u8 *)ml_ie, (pos - ml_ie),
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

			sub_eid = (struct _EID_STRUCT *)pos;
			/* Link Info */
			os_alloc_mem(NULL, (UCHAR **)&sub_ie_buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
			if (!sub_ie_buf) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"failed to allocate memory for parsing ML IE\n");
				NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
				return -1;
			}
			while (((u8 *)sub_eid) < ml_ie_end) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"  Per-STA, eid(%d) len=(%d)\n", sub_eid->Eid, sub_eid->Len);
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
					if (sta_parse_multi_link_per_sta_profile(pf_pos, sub_ie_len, ml_info, &link_num) < 0) {
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

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"total link num = %d in sta profile\n", link_num);
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
int sta_mld_conn_req(struct wifi_dev *wdev, struct mld_conn_req *mld_conn)
{
	struct mld_dev *mld = wdev->mld_dev;
	struct mld_link_entry *set_up_link = NULL;
	struct peer_mld_entry *peer = NULL;
	struct ml_ie_info *ml_info = NULL;
	struct tid2lnk_ie_info t2l_info = {0};
	PRTMP_ADAPTER pad = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	uint8_t i = 0, j = 0, link_num = 0, flag_once = 0;
	uint16_t setup_wcid = 0;
	UINT8 *sta_profile = NULL;
	UINT16 sta_profile_len = 0;
	IE_LISTS *ie_list = NULL;
	int with_setup_link_info = 0;

	if (mld != &mld_device)
		return -1;

	set_up_link = get_sta_mld_link_by_wdev(mld, wdev);

	if (!set_up_link)
		return -1;

	/* 1. disconnect current connection*/
	if (mld->peer_mld.valid)
		sta_mld_disconn_req(wdev);

	/* 2. parse t2l ie*/
	if (mld_conn->t2l_ie) {
		/*need to check return value later*/
		parse_tid_to_link_map_ie(mld_conn->t2l_ie, &t2l_info);
	}

	if (mld_conn->ml_ie) {
		/* 3. parse mld ie*/
		/*need to check return value later*/
		os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
		if (!ml_info) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"failed to allocate memory for parsing ML IE\n");
			return -1;
		}
		NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
		if (sta_parse_multi_link_ie(mld_conn->ml_ie, mld_conn->ml_frag_info, ml_info) < 0) {
			os_free_mem(ml_info);
			return -1;
		}

		set_up_link->is_setup_link = 1;

		peer = &mld->peer_mld;
		os_zero_mem(peer, sizeof(struct peer_mld_entry));
		/* 4. fill peer mld info*/
		COPY_MAC_ADDR(peer->mld_addr, ml_info->mld_addr);
		peer->valid = 1;
		peer->idx = PEER_AP_MLD_IDX;
		peer->aid = peer->idx + START_MLD_STA_AID;
		peer->set_up_link = set_up_link;
		peer->BAOriTID = 0;
		peer->BARecTID = 0;

		/*5. parse per sta profile to get ch_band*/
		os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));
		if (ie_list == NULL) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "alloc fail\n");
			os_free_mem(ml_info);
			return -1;
		}

		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			struct ml_ie_link *ml_link = &ml_info->link[i];
			UCHAR ch_band = 0;
			UCHAR channel = 0;

			if (ml_link->active == FALSE)
				break;

			if (mld_conn->link_addr && !memcmp(mld_conn->link_addr, ml_link->link_addr, MAC_ADDR_LEN)) {
				with_setup_link_info = 1;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "find setup add("MACSTR") in mld info\n",
				MAC2STR(ml_link->link_addr));
			}

			sta_profile = ml_link->sta_profile;
			sta_profile_len = ml_link->sta_profile_len;

			if (sta_profile_len <= 4) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"[link id=%d] no per link ie\n", ml_link->link_id);
				os_free_mem(ml_info);
				os_free_mem(ie_list);
				return -1;
			}

			NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));
			sta_parse_sta_profile_cap(sta_profile, sta_profile_len, ie_list, 0);
			if (eht_calculate_ch_band(&ie_list->cmm_ies, &ch_band) < 0) {
				os_free_mem(ml_info);
				os_free_mem(ie_list);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Failed to calcualte band info!\n");
				return -1;
			}
			ml_link->ch_band = ch_band;

			if (eht_calculate_channel(&ie_list->cmm_ies, &channel) < 0) {
				os_free_mem(ml_info);
				os_free_mem(ie_list);
				return -1;
			}
			ml_link->channel = channel;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"peer link_id(%d) band(%d) channel(%d)\n", ml_link->link_id,
				ml_link->ch_band, ml_link->channel);
		}

		if (!with_setup_link_info && mld_conn->link_addr && i < BSS_MNGR_MAX_BAND_NUM) {
			struct ml_ie_link *ml_link = &ml_info->link[i];

			ml_link->active = TRUE;
			ml_link->link_id = ml_info->link_id;
			COPY_MAC_ADDR(ml_link->link_addr, mld_conn->link_addr);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"set peer setup link(%d) active\n", ml_info->link_id);
			ml_link->ch_band = mld_conn->ch_band;
			ml_link->channel = wdev->channel;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"peer setup link ch_band = %d, channel=%d\n",
						ml_link->ch_band, ml_link->channel);
		}

		/*first insert set_up_link.*/
		for (i = 0; i < MLD_LINK_MAX; i++) {
			UCHAR own_ch_band = 0;
			/*1. find setup link*/
			if (set_up_link != &mld->mld_own_links[i])
				continue;
			/*own link is down or invalid*/
			if (mld->mld_own_links[i].used == 0 || !mld->mld_own_links[i].wdev)
				continue;
			own_ch_band = wlan_config_get_ch_band(mld->mld_own_links[i].wdev);
			pad = (PRTMP_ADAPTER)mld->mld_own_links[i].wdev->sys_handle;
			/*2. match  peer link band and channel is in own wdev channel list*/
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				if (ml_info->link[j].active && ml_info->link[j].ch_band == own_ch_band
					&& IsValidChannel(pad, ml_info->link[j].channel, wdev)) {
					MTWF_DBG(pad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"channel(%d) is supported by own wdev("MACSTR")\n", ml_info->link[j].channel,
					MAC2STR(mld->mld_own_links[i].wdev->if_addr));
					if (!MacTableLookup(pad, ml_info->link[j].link_addr)) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"%s, own link[%d] insert setup entry: band=%d, channel = %d, peer_link_id=%d\n",
						__func__, i, ml_info->link[j].ch_band, ml_info->link[j].channel, ml_info->link[j].link_id);
						sta_mld_insert_link(mld, peer, ml_info, i, j, &setup_wcid, set_up_link, &link_num);
						break;
					}
				}
			}
		}

		/*insert other links*/
		for (i = 0; i < MLD_LINK_MAX; i++) {
			UCHAR own_ch_band = 0;
			if (set_up_link == &mld->mld_own_links[i])
				continue;
			/*own link is down or the peer link is invalid*/
			if (mld->mld_own_links[i].used == 0 || !mld->mld_own_links[i].wdev)
				continue;
			own_ch_band = wlan_config_get_ch_band(mld->mld_own_links[i].wdev);
			pad = (PRTMP_ADAPTER)mld->mld_own_links[i].wdev->sys_handle;
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				/*check the peer band and link channel is in own link channel list*/
				if (ml_info->link[j].active && ml_info->link[j].ch_band == own_ch_band
					&& IsValidChannel(pad, ml_info->link[j].channel, mld->mld_own_links[i].wdev)) {
					MTWF_DBG(pad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"channel(%d) is supported by own wdev("MACSTR")\n", ml_info->link[j].channel,
					MAC2STR(mld->mld_own_links[i].wdev->if_addr));
					if (!MacTableLookup(pad, ml_info->link[j].link_addr)) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"%s, own link[%d] insert non-setup entry:band=%d, channel = %d, peer_link_id=%d\n",
						__func__, i, ml_info->link[j].ch_band, ml_info->link[j].channel, ml_info->link[j].link_id);
						sta_mld_insert_link(mld, peer, ml_info, i, j, &setup_wcid, set_up_link, &link_num);
						break;
					}
				}
			}
		}

		/* 6. fill mlo info for each mac table entry*/
		for (i = 0; i < MLD_LINK_MAX; i++) {
			pEntry = (MAC_TABLE_ENTRY *)peer->single_link[i].priv_ptr;
			if (pEntry) {
				struct mld_entry_t *mld_entry = NULL, *new_mld_entry = NULL;
				struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
				struct hlist_head *head = NULL;
				u8 hash_idx = MAC_ADDR_HASH_INDEX(peer->mld_addr);

				spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
				mld_entry = get_mld_entry_by_mac(peer->mld_addr);
				if (mld_entry) {
					new_mld_entry = create_mld_entry();
					if (new_mld_entry) {
						update_mld_entry_apcli(mld_entry, new_mld_entry, wdev, i, link_num, &flag_once, setup_wcid);
						hlist_replace_rcu(&mld_entry->hlist, &new_mld_entry->hlist);
						update_link_mld_entry(new_mld_entry);
						mt_call_rcu(&mld_entry->rcu, free_mld_entry);
					} else
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
							" create mld_entry Fail!\n");
				} else {
					mld_entry = create_mld_entry();
					if (mld_entry) {
						update_mld_entry_apcli(NULL, mld_entry, wdev, i, link_num, &flag_once, setup_wcid);
						head = &mld_entry_ctrl->mld_hash[hash_idx];
						hlist_add_head_rcu(&mld_entry->hlist, head);
						update_link_mld_entry(mld_entry);
					} else
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
							" create mld_entry Fail!\n");
				}
				spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);
				fill_peer_mlo_info(peer, link_num, pEntry->wdev == set_up_link->wdev ? 1 : 0,
					setup_wcid, &pEntry->mlo);

				/*dump mlo information in mac table entry*/
				bss_mngr_peer_integrity_check(pEntry);
			}
		}
	}


	/* fill mld sta index */
	if (peer)
		mld_conn->mld_sta_idx = peer->idx;
	else
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN, "peer is null!\n");

	if (ml_info)
		os_free_mem(ml_info);
	if (ie_list)
		os_free_mem(ie_list);

	return 0;
}

bool sta_mld_set_up_wdev_check(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	struct mld_dev *mld = wdev->mld_dev;
	struct mld_link_entry *link = NULL;
	bool ret = TRUE;

	if (mld != &mld_device)
		return ret;

	if (pEntry && pEntry->mlo.mlo_en) {
		link = get_sta_mld_link_by_wdev(mld, wdev);
		if (link) {
			if (link->is_setup_link)
				ret = TRUE;
			else
				ret = FALSE;
		}
	}
	return ret;
}
static uint16_t sta_build_multi_link_per_sta_profile(struct wifi_dev *tdev, uint8_t *pos)
{
	uint16_t sub_ie_len = 0;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)tdev->sys_handle;/*not set up link*/
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[0];
	//struct wifi_dev *main_wdev = &pMbss->wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	struct _build_ie_info ie_info = {0};
	struct legacy_rate *rate = &tdev->rate.legacy_rate;

	ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
	ie_info.channel = tdev->channel;
	ie_info.phy_mode = tdev->PhyMode;
	ie_info.wdev = tdev;

	NdisMoveMemory(pos, &pMbss->CapabilityInfo, sizeof(USHORT));
	sub_ie_len += sizeof(USHORT);
	sub_ie_len += build_support_rate_ie(tdev, rate->sup_rate, rate->sup_rate_len, pos + sub_ie_len);
	sub_ie_len += build_support_ext_rate_ie(tdev, rate->sup_rate_len,
					rate->ext_rate, rate->ext_rate_len, pos + sub_ie_len);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"sub_ie_len=%d\n", sub_ie_len);
#ifdef DOT11_N_SUPPORT
			/* HT */
			if (WMODE_CAP_N(tdev->PhyMode)) {
				ie_info.frame_buf = (UCHAR *)(pos + sub_ie_len);
				sub_ie_len += build_ht_ies(pAd, &ie_info);
			}
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"ht:sub_ie_len=%d\n", sub_ie_len);
			/* VHT/HE */
			if (WMODE_CAP_N(tdev->PhyMode)) {
#ifdef DOT11_VHT_AC
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
				ucETxBfCap = wlan_config_get_etxbf(tdev);

				if (bf_is_support(tdev) == FALSE)
					wlan_config_set_etxbf(tdev, SUBF_OFF);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
				ie_info.frame_buf = (UCHAR *)(pos + sub_ie_len);
				sub_ie_len += build_vht_ies(pAd, &ie_info);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"vht:sub_ie_len=%d\n", sub_ie_len);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
				wlan_config_set_etxbf(tdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */


#ifdef DOT11_HE_AX
			if (WMODE_CAP_AX(tdev->PhyMode)) {
				UINT8 *local_fbuf = NULL;

				local_fbuf = build_he_cap_ie(tdev, pos + sub_ie_len);
				sub_ie_len = local_fbuf - pos;
#ifdef CONFIG_6G_SUPPORT
					/*HE 6 GHz Band Capabilities*/
				local_fbuf = build_he_6g_cap_ie(tdev, pos + sub_ie_len);
				sub_ie_len = local_fbuf - pos;
#ifdef MBO_SUPPORT
				sub_ie_len += build_supp_op_class_ie(pAd, tdev, pos + sub_ie_len);
#endif
#endif
				}
#endif /*DOT11_HE_AX*/
			}
			/* HE_6G */
			else if (WMODE_CAP_6G(tdev->PhyMode)) {
				UINT8 *local_fbuf = NULL;

				local_fbuf = build_he_cap_ie(tdev, pos + sub_ie_len);
				sub_ie_len = local_fbuf - pos;
#ifdef CONFIG_6G_SUPPORT
					/*HE 6 GHz Band Capabilities*/
				local_fbuf = build_he_6g_cap_ie(tdev, pos + sub_ie_len);
				sub_ie_len = local_fbuf - pos;
#ifdef MBO_SUPPORT
				sub_ie_len += build_supp_op_class_ie(pAd, tdev, pos + sub_ie_len);
#endif
#endif
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"he:sub_ie_len=%d\n", sub_ie_len);
#ifdef DOT11_EHT_BE
			if (WMODE_CAP_BE(tdev->PhyMode)) {
				UINT8 *local_fbuf = NULL;
				/* EHT Cap. */
				local_fbuf = eht_build_cap_ie(tdev, pos + sub_ie_len);
				sub_ie_len = (local_fbuf - pos);
			}
#endif/*DOT11_EHT_BE*/
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"eht:sub_ie_len=%d\n", sub_ie_len);

#endif /* DOT11_N_SUPPORT */
	return sub_ie_len;
}
uint16_t sta_mld_build_multi_link_ie(uint8_t *pos,
	struct wifi_dev *wdev, uint16_t type)
{
	struct mld_dev *mld = wdev->mld_dev;
	struct ie_multi_link_info ml_info;
	struct _EID_STRUCT *eid;
	struct rnr_ml_param ml_parms[BSS_MNGR_MAX_BAND_NUM-1] = {0};
	PRTMP_ADAPTER pad =  (PRTMP_ADAPTER)wdev->sys_handle;
	uint8_t i = 0, count = 0, per_sta_pf_cnt = 0;
#ifdef DOT11_BE_D12
	u8 *ptr_info_len;
	u8 comm_info_len = 0;
	u8 *ptr_sta_info_len;
#endif
	u16 present_bmap = 0;
	u16 sta_prof_len = 0;
	u16 profile_ie_len = 0;
	u16 total_ie_len = 0;
#ifdef DOT11V_MBSSID_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(NULL, wdev);
	struct _MLME_AUX *mlmeAux = &pStaCfg->MlmeAux;
#endif

	if (mld != &mld_device)
		return 0;

	if (type != ML_CTRL_TYPE_BASIC && type != ML_CTRL_TYPE_PROBE_REQ &&
		type != BMGR_QUERY_ML_IE_AUTH_REQ)
		return 0;

	eid = (struct _EID_STRUCT *)pos;
	eid->Eid = IE_WLAN_EXTENSION;
	eid->Len = 1;
	eid->Octet[0] = EID_EXT_EHT_MULTI_LINK;

	pos += eid->Len + 2;
	os_zero_mem(&ml_info, sizeof(struct ie_multi_link_info));

	if (type == BMGR_QUERY_ML_IE_AUTH_REQ) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"%s: mlo auth request for basic mlo ie\n", __func__);
		SET_DOT11BE_ML_CTRL_TYPE(ml_info.ml_ctrl, ML_CTRL_TYPE_BASIC);
	} else
		SET_DOT11BE_ML_CTRL_TYPE(ml_info.ml_ctrl, type);
#ifdef DOT11_BE_D10
	/*include mld mac address bit only lin draft 1.0*/
	SET_DOT11BE_ML_CTRL_MLD_MAC_PRESENT(ml_info.ml_ctrl, TRUE);
#endif
	if (wlan_config_get_emlsr_mr(wdev) == EMLSR)
		present_bmap |= DOT11BE_ML_BASIC_PRESENCE_EML_CAPS;
	if (type == ML_CTRL_TYPE_BASIC)
		present_bmap |= DOT11BE_ML_BASIC_PRESENCE_MLD_CAPS_N_OP;
	SET_DOT11BE_ML_CTRL_PRESENCE_BMAP(ml_info.ml_ctrl, present_bmap);

	ml_info.ml_ctrl = cpu_to_le16(ml_info.ml_ctrl);

	COPY_MAC_ADDR(ml_info.mld_addr, mld->mld_addr);
	if (type != ML_CTRL_TYPE_PROBE_REQ) {
		NdisMoveMemory(pos, &ml_info, sizeof(struct ie_multi_link_info));
		eid->Len += sizeof(struct ie_multi_link_info);
#ifdef DOT11_BE_D12
		ptr_info_len = pos + 2;
#endif

		pos += sizeof(struct ie_multi_link_info);
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_EML_CAPS) {
			u8 eml_caps[2] = {0};

			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_SUP(*eml_caps, (wlan_config_get_emlsr_mr(wdev) == 1) ? 1:0);
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_PADDING_DELAY(*eml_caps, wlan_config_get_emlsr_padding(wdev));
			SET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_TRANS_DELAY(*eml_caps, wlan_config_get_emlsr_trans_delay(wdev));
			/*not support emlmr yet*/

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\teml_caps(0x%x)\n", *eml_caps);

			NdisMoveMemory(pos, eml_caps, 2);
			eid->Len += 2;
			pos += 2;
		}
		/* Common Info - MLD Capabilities */
		if (present_bmap & DOT11BE_ML_BASIC_PRESENCE_MLD_CAPS_N_OP) {
			u16 mld_caps = 0;
			u8 str_link_num = 0;
			struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
			u8 str_bitmap = 0;
			u8 nt2lm_para = 0;

			if (PD_GET_MLO_CONFIG_OP_SET(pAd->physical_dev))
				str_bitmap = PD_GET_MLO_STR_BITMAP(pAd->physical_dev);
			else
				str_bitmap = 7;
			if ((str_bitmap & 1) && (mld->mld_own_links[0].used) && (mld->mld_own_links[1].used))
				str_link_num++;
			if (((str_bitmap >> 1) & 1) && (mld->mld_own_links[0].used) && (mld->mld_own_links[2].used))
				str_link_num++;
			if (((str_bitmap >> 2) & 1) && (mld->mld_own_links[1].used) && (mld->mld_own_links[2].used))
				str_link_num++;
			if (str_link_num > 1)
				str_link_num = str_link_num - 1;

			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_MAX_LINKS(mld_caps, str_link_num);
			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_SRS_SUP(mld_caps, FALSE);

			nt2lm_para = wlan_config_get_t2lm_nego_support(wdev);
			if (nt2lm_para <= 3 && nt2lm_para > 0)
				SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_TID2LNK_NEGO_SUP(mld_caps, nt2lm_para)
			else
				SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_TID2LNK_NEGO_SUP(mld_caps, 0)

			SET_DOT11BE_ML_BASIC_CMM_MLD_CAP_FREQ_SEP_STR(mld_caps, FALSE);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\tmld_caps(0x%x)\n", mld_caps);
			mld_caps = cpu_to_le16(mld_caps);
			NdisMoveMemory(pos, &mld_caps, sizeof(mld_caps));
			eid->Len += sizeof(mld_caps);
			pos += sizeof(mld_caps);
		}

#ifdef DOT11_BE_D12
		comm_info_len = (u8)(pos - ptr_info_len);
		*ptr_info_len = comm_info_len;
#endif
	} else {
		/*ml ctrl
		spec 11be3.0: 9.4.2.312.3 Probe Request Multi-Link element
			B0			B1- B11
		MLD ID present | Reserved
		*/
		eid->Len += sizeof(ml_info.ml_ctrl);
		present_bmap |= BIT(0);
		SET_DOT11BE_ML_CTRL_PRESENCE_BMAP(ml_info.ml_ctrl, present_bmap);
		NdisMoveMemory(pos, &ml_info.ml_ctrl, sizeof(ml_info.ml_ctrl));
		pos += sizeof(ml_info.ml_ctrl);

		/*mlti-link common info MLD-ID
		spec 11be3.0: 9.4.2.312.3 Probe Request Multi-Link element
		Octets:|	1 -------| --0 or 1
		|Common Info Length | AP MLD ID|
		*/
		*pos++ = 2;/*length*/
#ifdef DOT11V_MBSSID_SUPPORT
		*pos++ = (mlmeAux->mld_id) ? (mlmeAux->mld_id) : 0; /*ap mld id*/
#else
		*pos++ = 0;/*ap mld id*/
#endif
		eid->Len += 2;
	}

	if (type == ML_CTRL_TYPE_BASIC || type == ML_CTRL_TYPE_PROBE_REQ) {
		/*prepare the ap ml rnr ml param infos*/
		if (type == ML_CTRL_TYPE_PROBE_REQ) {
			PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pad, wdev);

			if (pStaCfg) {
				int rnr_idx = 0;
				MLME_AUX *mlme_aux = &pStaCfg->MlmeAux;
				BSS_TABLE *scan_tab = get_scan_tab_by_wdev(pad, wdev);
				ULONG bss_idx = BssSsidTableSearchByBSSID(scan_tab, mlme_aux->Bssid);

				if (bss_idx < MAX_LEN_OF_BSS_TABLE) {
					struct rnr_ml_param *rnr_ml_parms = scan_tab->BssEntry[bss_idx].rnr_ml_parms;
					/*find the all mld id :0 */
					for (rnr_idx = 0; rnr_idx < MAX_MLD_GROUP_NUM; rnr_idx++) {
						if (rnr_ml_parms[rnr_idx].is_valid == 0)
							continue;
						/*spec 11be d3.0:9.4.2.170.2
						Neighbor AP Information field:
						Figure 9-709c MLD Parameters subfield format
						If the reported AP is affiliated with the
						same MLD as the reporting
						AP sending the frame carrying this element,
						the AP MLD ID subfield is set to 0
						*/
						if (rnr_ml_parms[rnr_idx].mld_id < 255) {
							if (per_sta_pf_cnt >= BSS_MNGR_MAX_BAND_NUM-1) {

								MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
									"already find the %d rnr_ml_parm entries.\n",
									per_sta_pf_cnt);
								break;
							}
							if (rnr_ml_parms[rnr_idx].is_valid == 0
								|| rnr_ml_parms[rnr_idx].mld_id != mlmeAux->mld_id) {
								MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
									"error: rnr_idx %d, mld_id: %d, is_valid:%d.\n", rnr_idx,
									rnr_ml_parms[rnr_idx].mld_id, rnr_ml_parms[rnr_idx].is_valid);
								continue;
							}
							os_move_mem(&ml_parms[per_sta_pf_cnt], &rnr_ml_parms[rnr_idx],
								sizeof(rnr_ml_parms[rnr_idx]));
							MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
									"rnr_idx %d, mld_id: %d, link_id: %d,is_valid:%d.\n",
									rnr_idx, ml_parms[per_sta_pf_cnt].mld_id,
									ml_parms[per_sta_pf_cnt].link_id,
									ml_parms[per_sta_pf_cnt].is_valid);
							per_sta_pf_cnt++;
						}
					}
				}
			}
		}
		count = 0;
		for (i = 0; i < MLD_LINK_MAX; i++) {
			uint16_t ctrl = 0;
			struct wifi_dev *tdev = mld->mld_own_links[i].wdev;
			struct _EID_STRUCT *sub_eid;
			struct peer_mld_single_link peer = mld->peer_mld.single_link[i];

			if (type == ML_CTRL_TYPE_BASIC) {
				if (!mld->mld_own_links[i].used ||
					!mld->peer_mld.single_link[i].active || wdev == tdev)
					continue;

				if (mld->mld_own_links[i].is_setup_link) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"link id = [%d] is setup link: skip to add Per STA-Profile\n", i);
					continue;
				}
			}

			sub_eid = (struct _EID_STRUCT *)pos;
			sub_eid->Eid = SUB_IE_MLD_PER_STA_PROFILE;
			sub_eid->Len = 0;
			pos += 2;

			if (type == ML_CTRL_TYPE_BASIC) {
				SET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(ctrl, peer.link_id);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"insert peer link id [%d] to add Per STA-Profile\n", peer.link_id);
				SET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(ctrl, 1);
				SET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(ctrl, 1);
				if (wlan_config_get_nstr_bitmap(tdev)) {
					SET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(ctrl, 1);
					SET_DOT11BE_ML_PER_STA_CTRL_NSTR_BMAP_SIZE(ctrl, 0);
				}
			} else {
				/*ML_CTRL_TYPE_PROBE_REQ*/
				if (count < per_sta_pf_cnt && ml_parms[count].is_valid) {
					SET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(ctrl, ml_parms[count].link_id);
					SET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(ctrl, 1);
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"succss: mld_parms[%d]: is_valid: %d, mld_id:%d, link_id:%d\n",
						count, ml_parms[count].is_valid, ml_parms[count].mld_id,
						ml_parms[count].link_id);
				} else {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
						"error: mld_parms[%d]: is_valid: %d, mld_id:%d, link_id:%d\n",
						count, ml_parms[count].is_valid, ml_parms[count].mld_id,
						ml_parms[count].link_id);
				}
				count++;
			}

			NdisMoveMemory(pos, &ctrl, sizeof(uint16_t));
			sub_eid->Len += sizeof(uint16_t);
			pos += sizeof(uint16_t);
			/*probe req only take partial per-sta-pf sta, control feilds*/
			if (type == ML_CTRL_TYPE_PROBE_REQ) {
				if (count >= BSS_MNGR_MAX_BAND_NUM-1 || count >= per_sta_pf_cnt)
					break;
				continue;
			}

#ifdef DOT11_BE_D12
			sub_eid->Len++;
			ptr_sta_info_len = pos;
			pos++;
#endif
			COPY_MAC_ADDR(pos, tdev->if_addr);
			sub_eid->Len += MAC_ADDR_LEN;
			pos += MAC_ADDR_LEN;

			if (wlan_config_get_nstr_bitmap(tdev)) {
				u8 nstr_bitmap = wlan_config_get_nstr_bitmap(tdev);

				NdisMoveMemory(pos, &nstr_bitmap, sizeof(uint8_t));
				sub_eid->Len += sizeof(uint8_t);
				pos += sizeof(uint8_t);
			}
#ifdef DOT11_BE_D12
			*ptr_sta_info_len = (pos - ptr_sta_info_len);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"sta_info_len(%d)\n", *ptr_sta_info_len);
#endif
			/*build per sta profile sub ie*/
			sta_prof_len = sta_build_multi_link_per_sta_profile(tdev, pos);
			/*++per sta profile fragment++*/
			if ((sta_prof_len + sub_eid->Len) > MAX_LEN_OF_IE_DATA) {
				profile_ie_len = sta_prof_len + sub_eid->Len + 2;
				pos = frag_multi_link_per_sta_profile((u8 *)sub_eid, &profile_ie_len);
			} else {
				sub_eid->Len += sta_prof_len;
				pos += sta_prof_len;
			}
		}

		total_ie_len = pos - (u8 *)eid;
		if (total_ie_len != 0) {
			if (total_ie_len > (2 + MAX_LEN_OF_IE_DATA)) {
				/* do IE fragmentation */
				pos = fragment_information_element((u8 *)eid, &total_ie_len);
			} else
				eid->Len = total_ie_len - 2;
		}
	}
	total_ie_len = pos - (u8 *)eid;
	hex_dump("Dump Multi link IE", (unsigned char *)eid, total_ie_len);
	return total_ie_len;
}


int sta_mld_link_sync(struct wifi_dev *wdev_src, MAC_TABLE_ENTRY *pEntry_src)
{
	struct mld_dev *mld = wdev_src->mld_dev;
	RTMP_ADAPTER *pAd_src = NULL, *pAd_dest = NULL;
	STA_TR_ENTRY *tr_src = NULL, *tr_dest = NULL;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pDestEntry = NULL;
	struct wifi_dev *pDestwdev = NULL;
	PSTA_ADMIN_CONFIG pStaCfg_src = NULL, pStaCfg_dst = NULL;

	if (!pEntry_src)
		return -1;

	if (mld != &mld_device)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pDestEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pDestEntry == pEntry_src)
				break;
		}
	}

	if (i == MLD_LINK_MAX)
		return -1;

	pAd_src = (RTMP_ADAPTER *)wdev_src->sys_handle;

	if (!pAd_src) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"error!!! pAd_src is NULL!!\n");
		return -1;
	}

	tr_src = tr_entry_get(pAd_src, pEntry_src->wcid);
	pStaCfg_src = GetStaCfgByWdev(pAd_src, wdev_src);

	if (!pStaCfg_src) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"error!!! pStaCfg_src is NULL!!\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pDestEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;

			/* skip source device*/
			if (!pDestEntry || pDestEntry == pEntry_src)
				continue;

			pDestwdev = mld->mld_own_links[i].wdev;
			if (!pDestwdev)
				continue;

			pAd_dest = (RTMP_ADAPTER *)pDestwdev->sys_handle;
			if (!pAd_dest)
				continue;

			tr_dest = tr_entry_get(pAd_src, pDestEntry->wcid);
			pStaCfg_dst = GetStaCfgByWdev(pAd_dest, pDestwdev);
			if (!pStaCfg_dst)
				continue;

			pDestwdev->PortSecured = wdev_src->PortSecured;
			pDestwdev->SecConfig.AKMMap = wdev_src->SecConfig.AKMMap;
			pDestwdev->SecConfig.PairwiseCipher = wdev_src->SecConfig.PairwiseCipher;
			tr_dest->PortSecured = tr_src->PortSecured;
			pStaCfg_dst->ApcliInfStat.Enable = pStaCfg_src->ApcliInfStat.Enable;
			pStaCfg_dst->StaActive.Aid = pStaCfg_src->StaActive.Aid;
			if (pStaCfg_dst->ApcliInfStat.Enable == TRUE) {
				NdisMoveMemory(pStaCfg_dst->Bssid, pDestEntry->Addr, MAC_ADDR_LEN);
				NdisMoveMemory(pStaCfg_dst->MlmeAux.Bssid, pDestEntry->Addr, MAC_ADDR_LEN);
			} else {
				NdisZeroMemory(pStaCfg_dst->Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pStaCfg_dst->MlmeAux.Bssid, MAC_ADDR_LEN);
			}
			if (INFRA_ON(pStaCfg_src))
				STA_STATUS_SET_FLAG(pStaCfg_dst, fSTA_STATUS_INFRA_ON);
			else
				STA_STATUS_CLEAR_FLAG(pStaCfg_dst, fSTA_STATUS_INFRA_ON);
			if (pStaCfg_dst->SsidLen <= MAX_LEN_OF_SSID + 1) {
				NdisMoveMemory(pStaCfg_dst->Ssid, pStaCfg_src->Ssid, pStaCfg_dst->SsidLen);
				pStaCfg_dst->SsidLen =  pStaCfg_src->SsidLen;
			}
			/*security info sec*/
			pDestEntry->SecConfig.AKMMap = pEntry_src->SecConfig.AKMMap;
			pDestEntry->SecConfig.PairwiseCipher = pEntry_src->SecConfig.PairwiseCipher;
			pDestEntry->Sst = pEntry_src->Sst;
#ifdef DOT11W_PMF_SUPPORT
			pDestEntry->SecConfig.PmfCfg.UsePMFConnect = pEntry_src->SecConfig.PmfCfg.UsePMFConnect;
#endif
			/* sync vendor ie info */
			NdisCopyMemory(&pDestEntry->vendor_ie, &pEntry_src->vendor_ie, sizeof(struct _vendor_ie_cap));

			MTWF_DBG(pDestEntry->pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"%s:wdev(%02x:%02x:%02x:%02x:%02x:%02x)INFRA_ON(%d)BSSID("MACSTR") to wdev(%02x:%02x:%02x:%02x:%02x:%02x) INFRA_ON(%d)BSSID("MACSTR")!!\n",
				__func__, PRINT_MAC(wdev_src->if_addr), INFRA_ON(pStaCfg_src), MAC2STR(pStaCfg_src->Bssid),
				PRINT_MAC(pDestwdev->if_addr), INFRA_ON(pStaCfg_dst), MAC2STR(pStaCfg_dst->Bssid));
		}
	}

	return 0;
}


/**
* sta_mld_update_rx_bcn_time() - update apcli
* rx bcn/probe_rsp/data time value.
* @pwdev:  wifi dev
* @rx_bcn_type: update beacon type value;
*	enum ENUM_ML_BCN_RX_UPDATE_NUM {
*	ML_APC_BCN_RX_TIME = 0,
*	ML_APC_BCN_RX_ENQ_TIME = 1,
*	ML_APC_BCN_RX_TIME_MAX_NUM
* };
*
* when link_up and rx_beacon/rx_probe_rsp/rx_data
* should update the rx beacon time.
*
* return:
*		0 : success, other : fail
*/
int sta_mld_update_rx_bcn_time(struct wifi_dev *pwdev, uint8_t rx_bcn_type)
{
	RTMP_ADAPTER *pad = NULL;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
	struct wifi_dev *setup_wdev = NULL;

	if (!pwdev || rx_bcn_type >=  ML_APC_BCN_RX_TIME_MAX_NUM) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"(%s)[%d] input invalid wdev(%p), rx_bcn_type(%d)",
			__func__, __LINE__, pwdev, rx_bcn_type);
		return -1;
	}

	setup_wdev = get_sta_mld_setup_wdev(pwdev->mld_dev);
	if (setup_wdev && setup_wdev->sys_handle) {
		pad = (RTMP_ADAPTER *) setup_wdev->sys_handle;
		sta_cfg =  GetStaCfgByWdev(pad, setup_wdev);
		if (sta_cfg) {
			if (rx_bcn_type == ML_APC_BCN_RX_TIME)
				NdisGetSystemUpTime(&sta_cfg->ApcliInfStat.ApCliRcvBeaconTime);
			else if (rx_bcn_type == ML_APC_BCN_RX_ENQ_TIME)
				NdisGetSystemUpTime(&sta_cfg->ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv);
		} else {
			MTWF_DBG(pad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"error: can't get the setup link stacfg(NULL).\n");
			return -1;
		}
	} else
		MTWF_DBG(pad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "success: none-mlo case.\n");

	return 0;
}


int sta_mld_link_up(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
	STA_TR_ENTRY * tr_entry = NULL;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	RTMP_ADAPTER *pAd = NULL;
#endif


	if (mld != &mld_device)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			pCurwdev = mld->mld_own_links[i].wdev;
			if (pCurEntry && pCurwdev->wdev_ops && pCurwdev->wdev_ops->linkup) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"%s (%02x:%02x:%02x:%02x:%02x:%02x) pEntry(%p): wcid(%d)!!\n",
				__func__, PRINT_MAC(pCurwdev->if_addr), pCurEntry, pCurEntry->wcid);
				pCurwdev->wdev_ops->linkup(pCurwdev, pCurEntry);
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
				//Send AP join event
				pAd = (RTMP_ADAPTER *)pCurwdev->sys_handle;
				tr_entry = tr_entry_get(pAd, pCurEntry->wcid);
				pApCliEntry = &pAd->StaCfg[tr_entry->func_tb_idx];
				wapp_send_ap_join_event(pAd, pApCliEntry);
#endif
			}
		}
	}

	return 0;
}

int sta_mld_link_down(struct wifi_dev *wdev)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	struct wifi_dev *pCurwdev = NULL;

	if (mld != &mld_device || !mld->peer_mld.valid)
		return 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->mld_own_links[i].used) {
			pCurwdev = mld->mld_own_links[i].wdev;
			if (pCurwdev && pCurwdev->wdev_ops && pCurwdev->wdev_ops->linkdown &&
				wdev != pCurwdev) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"%s (%02x:%02x:%02x:%02x:%02x:%02x)!!\n",
					__func__, PRINT_MAC(pCurwdev->if_addr));
				pCurwdev->wdev_ops->linkdown(pCurwdev);
			}
		}
	}
	if (mld->probe_req_ml_ie.buf) {
		os_free_mem(mld->probe_req_ml_ie.buf);
		mld->probe_req_ml_ie.buf = NULL;
		mld->probe_req_ml_ie.len = 0;
	}
	if (mld->assoc_req_ml_ie.buf) {
		os_free_mem(mld->assoc_req_ml_ie.buf);
		mld->assoc_req_ml_ie.buf = NULL;
		mld->assoc_req_ml_ie.len = 0;
	}
	/*clear peer partial infos*/
	if (mld->peer_mld.valid) {
		mld->peer_mld.valid = 0;
		mld->peer_mld.idx = 0;
		mld->peer_mld.aid = 0;
		mld->peer_mld.emlsr = 0;
		os_zero_mem(&mld->peer_mld.mld_addr, MAC_ADDR_LEN);
		mld->peer_mld.BAOriTID = 0;
		mld->peer_mld.BARecTID = 0;
	}

	return 0;
}

int sta_mld_connect_act(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;

	if (mld != &mld_device)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			pCurwdev = mld->mld_own_links[i].wdev;
			if (pCurEntry && pCurwdev->wdev_ops && pCurwdev->wdev_ops->conn_act) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"%s (%02x:%02x:%02x:%02x:%02x:%02x) pEntry(%p): wcid(%d)!!\n",
				__func__, PRINT_MAC(pCurwdev->if_addr), pCurEntry, pCurEntry->wcid);
				pCurwdev->wdev_ops->conn_act(pCurwdev, pCurEntry);
			}
		}
	}

	return 0;
}


int sta_mld_disconnect_act(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;

	if (mld != &mld_device)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX)
		return -1;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			pCurwdev = mld->mld_own_links[i].wdev;
			if (pCurEntry && pCurwdev->wdev_ops && pCurwdev->wdev_ops->disconn_act) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"%s (%02x:%02x:%02x:%02x:%02x:%02x) pEntry(%p): wcid(%d)!!\n",
					__func__, PRINT_MAC(pCurwdev->if_addr), pCurEntry, pCurEntry->wcid);
					pCurwdev->wdev_ops->disconn_act(pCurwdev, pCurEntry);
#ifdef CONFIG_MAP_SUPPORT
				pCurwdev->bh_ready_sent = FALSE;
#endif
			}
		}
	}

	return 0;
}
int sta_mld_ba_ori_setup(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u16 TimeOut
)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *mld_master_entry, *pCurEntry = NULL;
	struct wifi_dev *mld_master_wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct mld_link_entry *link = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! can not find mld_device\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! can not find pEntry\n");
		return -1;
	}
	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (mld->master_link == link) {
			mld_master_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			mld_master_wdev = mld->mld_own_links[i].wdev;
			break;
		}
	}

	if (!mld_master_wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! wdev is NULL!!\n");
		return -1;
	}

	pAd = (struct _RTMP_ADAPTER *)mld_master_wdev->sys_handle;
	if (pAd && mld_master_entry)
		ba_ori_session_start(pAd, mld_master_entry->wcid, TID);
	else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pAd or  mld_master_entry is NULL!!\n");
		return -1;
	}
	return 0;

}
int sta_mld_ba_resrc_ori_add(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u16 ori_ba_wsize,
	u8 amsdu_en,
	u16 TimeOut
)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *mld_master_entry, *pCurEntry = NULL;
	struct wifi_dev *mld_master_wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct mld_link_entry *link = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (mld->master_link == link) {
			mld_master_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			mld_master_wdev = mld->mld_own_links[i].wdev;
			break;
		}
	}
	if (!mld_master_wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! wdev is NULL!!\n");
		return -1;
	}
	pAd = (struct _RTMP_ADAPTER *)mld_master_wdev->sys_handle;
	if (pAd && mld_master_entry)
		ba_resrc_ori_add(pAd, mld_master_entry->wcid, TID, ori_ba_wsize, amsdu_en, TimeOut);
	else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pAd or  mld_master_entry is NULL!!\n");
		return -1;
	}
	return 0;
}

int sta_mld_ba_resrc_rec_add(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u16 TimeOut,
	u16 StartSeq,
	u16 rec_ba_wsize)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *mld_master_entry, *pCurEntry = NULL;
	struct wifi_dev *mld_master_wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct mld_link_entry *link = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}
	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}


	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (mld->master_link == link) {
			mld_master_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			mld_master_wdev = mld->mld_own_links[i].wdev;
			break;
		}
	}
	if (!mld_master_wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! wdev is NULL!!\n");
		return -1;
	}
	pAd = (struct _RTMP_ADAPTER *)mld_master_wdev->sys_handle;
	if (pAd && mld_master_entry)
		ba_resrc_rec_add(pAd, mld_master_entry->wcid, TID, TimeOut, StartSeq, rec_ba_wsize);
	else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pAd or  mld_master_entry is NULL!!\n");
		return -1;
	}
	return 0;
}

int sta_mld_ba_resrc_ori_del(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u8 bPassive
)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *mld_master_entry, *pCurEntry = NULL;
	struct wifi_dev *mld_master_wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct mld_link_entry *link = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}
	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}


	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (mld->master_link == link) {
			mld_master_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			mld_master_wdev = mld->mld_own_links[i].wdev;
			break;
		}
	}

	if (!mld_master_wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! wdev is NULL!!\n");
		return -1;
	}
	pAd = (struct _RTMP_ADAPTER *)mld_master_wdev->sys_handle;
	if (mld_master_entry && pAd) {
		if (!ba_resrc_ori_del(pAd, mld_master_entry->wcid, TID))
			return -1;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"ERROR!!! pAd or  mld_master_entry is NULL!!\n");
		return -1;
	}
	return 0;
}

int sta_mld_ba_resrc_rec_del(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u8 bPassive)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *mld_master_entry, *pCurEntry = NULL;
	struct wifi_dev *mld_master_wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct mld_link_entry *link = NULL;


	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}
	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}


	for (i = 0; i < MLD_LINK_MAX; i++) {
		link = &mld->mld_own_links[i];
		if (mld->master_link == link) {
			mld_master_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			mld_master_wdev = mld->mld_own_links[i].wdev;
			break;
		}
	}

	if (!mld_master_wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! wdev is NULL!!\n");
		return -1;
	}
	pAd = (struct _RTMP_ADAPTER *)mld_master_wdev->sys_handle;
	if (mld_master_entry && pAd) {
		if (!ba_resrc_rec_del(pAd, mld_master_entry->wcid, TID))
			return -1;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! pAd or  mld_master_entry is NULL!!\n");
		return -1;
	}
	return 0;

}

int sta_mld_ba_add_to_asic(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	u16 Seq,
	u16 BAWinSize,
	int type,
	u8 amsdu_en)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}


	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			pCurwdev = mld->mld_own_links[i].wdev;
			if (!pCurwdev) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pCurwdev is NULL!!\n");
				return -1;
			}
			pAd = (struct _RTMP_ADAPTER *)pCurwdev->sys_handle;
			if (pCurEntry && pAd) {
				RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pCurEntry->wcid, TID,
					Seq, BAWinSize, type, amsdu_en);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pAd or pCurEntry is NULL!!\n");
				return -1;
			}
		}
	}
	return 0;
}

int sta_mld_ba_del_from_asic(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	u8 TID,
	int type,
	u8 amsdu_en)
{
	struct mld_dev *mld = wdev->mld_dev;
	uint8_t i = 0;
	MAC_TABLE_ENTRY *pCurEntry = NULL;
	struct wifi_dev *pCurwdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;

	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find mld_device\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (pCurEntry == pEntry)
				break;
		}
	}

	if (i == MLD_LINK_MAX) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"ERROR!!! can not find pEntry\n");
		return -1;
	}

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->peer_mld.single_link[i].active && mld->mld_own_links[i].used) {
			pCurEntry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			pCurwdev = mld->mld_own_links[i].wdev;
			if (!pCurwdev) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pCurwdev is NULL!!\n");
				return -1;
			}
			pAd = (struct _RTMP_ADAPTER *)pCurwdev->sys_handle;
			if (pCurEntry && pAd) {
				RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, pCurEntry->wcid, TID, type, amsdu_en);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! pAd or pCurEntry is NULL!!\n");
				return -1;
			}
		}
	}
	return 0;
}

int sta_update_mld_group_id(u8 group_id)
{

	int sta_grp_idx, idx;

	if (group_id >= MAX_MLD_GROUP_NUM) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! INVALID group_id: %d, only can be 0--63, used_mld_grp_idx(%d).\n",
				group_id, mld_device.used_mld_grp_idx);
		return -1;
	}

	/*find the group id, if it's used*/
	for (idx = 0; idx < MAX_MLD_GROUP_NUM; idx++) {
		if (group_id == mld_device.used_mld_grp_ids[idx])
			break;
	}
	/*if the group id not used. update to the idxs array*/
	if (idx == MAX_MLD_GROUP_NUM) {
		idx = mld_device.used_mld_grp_idx;
		mld_device.used_mld_grp_ids[idx] = group_id;
		mld_device.used_mld_grp_idx++;

		if (mld_device.used_mld_grp_idx >= MAX_MLD_GROUP_NUM) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! INVALID used_mld_grp_idx: %d\n",
				mld_device.used_mld_grp_idx);
			return -1;
		}

		/*find the not used group id in the used_mld_grp_idxs*/
		for (sta_grp_idx = 1; sta_grp_idx < MAX_MLD_GROUP_NUM; sta_grp_idx++) {
			for (idx = 0; idx < mld_device.used_mld_grp_idx; idx++) {
				if (sta_grp_idx == mld_device.used_mld_grp_ids[idx])
					break;
			}
			if (idx == mld_device.used_mld_grp_idx) {
				mld_device.mld_grp_idx = sta_grp_idx;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"update the new sta group idx: %d\n", mld_device.mld_grp_idx);
				return 0;
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"mld_device.mld_grp_idx: %d\n", mld_device.mld_grp_idx);
	return 0;
}

static VOID mlo_sta_sync_channel(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR channel)
{
	struct wifi_dev *wdev = pEntry->wdev;
	UCHAR i = 0;
	struct wifi_dev *tdev = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"wdev is NULL!\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"ie channel=%d, wdev channel=%d!\n", channel, wdev->channel);

	if (channel != wdev->channel) {
		if (IsValidChannel(pAd, channel, wdev)) {
			for (i = 0; i < WDEV_NUM_MAX; i++) {
				tdev = pAd->wdev_list[i];
				/*No CSA flow, need delete the STA of MBSS in APCLI side */
				if (tdev && HcIsRadioAcq(tdev) && (tdev->wdev_type == WDEV_TYPE_AP))
					MacTableResetNonMapWdev(pAd, tdev);
			}
			wdev->channel = channel;
			wdev_sync_prim_ch(pAd, wdev);
			wlan_operate_set_prim_ch(wdev, wdev->channel);
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd) || IS_MAP_BS_ENABLE(pAd)) {
				if (!wdev->map_indicate_channel_change)
					wdev->map_indicate_channel_change = 1;

				for (i = 0; i < WDEV_NUM_MAX; i++) {
					tdev = pAd->wdev_list[i];
					if (tdev == wdev)
						continue;

					if (tdev && HcIsRadioAcq(tdev) && !tdev->map_indicate_channel_change)
						tdev->map_indicate_channel_change = 1;
				}
			}
#endif
		} else
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"channel(%d) SwitchChSanityCheck failed!\n", channel);
	}
}
static VOID sta_sync_ie_to_no_setup_entry(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct common_ies *cmm_ies)
{
	BOOLEAN is_cap_ht = FALSE, is_cap_vht = FALSE;
	UCHAR channel = 0;
	UCHAR extChan = EXTCHA_NONE;
	UCHAR extChan_6g = EXTCHA_NONE;
	UCHAR cenChan_6g = 0;
	BOOLEAN b_extchan_change = FALSE;
	BOOLEAN b_extchan_change_6g = FALSE;

	/*sync channel && extChan first*/
	if (WMODE_CAP_N(pEntry->wdev->PhyMode) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		extChan = cmm_ies->ht_op.AddHtInfo.ExtChanOffset;
		channel = cmm_ies->ht_op.ControlChan;
	}

	if (HAS_EHT_OP_EXIST(cmm_ies->ie_exists) &&
		GET_DOT11BE_OP_PARAM_HAS_OP_INFO(cmm_ies->eht_op.op_parameters) &&
		GET_DOT11BE_OP_CTRL_CH_BW(cmm_ies->eht_op.op_info.control) == EHT_OP_CH_BW320)
		cenChan_6g = cmm_ies->eht_op.op_info.ccfs1;

	if (WMODE_CAP_BE(pEntry->wdev->PhyMode)
			&& HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
		if ((channel == 0) && (cmm_ies->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT))
			channel = cmm_ies->he6g_opinfo.prim_ch;
	}

	if (channel == 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Parse ie channel failed!\n");
		return;
	}

	if (cenChan_6g != 0) {
		if (channel > cenChan_6g)
			extChan_6g = EXTCHA_BELOW;
		else if (channel < cenChan_6g)
			extChan_6g = EXTCHA_ABOVE;
	}

	if (wlan_config_get_ch_band(pEntry->wdev) == CMD_CH_BAND_24G &&
		wlan_operate_get_ht_bw(pEntry->wdev) == HT_BW_40 && cmm_ies->ht_cap.HtCapInfo.ChannelWidth == 1) {
		if (extChan != EXTCHA_NONE) {
			update_ht_extchan_cfg(pAd, extChan, &b_extchan_change);
			if (b_extchan_change)
				apply_ht_extchan_cfg(pAd);
		}
	}

	if (wlan_config_get_ch_band(pEntry->wdev) == CMD_CH_BAND_6G &&
		wlan_operate_get_bw(pEntry->wdev) == BW_320) {
		if (extChan_6g != EXTCHA_NONE) {
			update_ht_extchan_cfg(pAd, extChan_6g, &b_extchan_change_6g);
			if (b_extchan_change_6g)
				apply_ht_extchan_cfg(pAd);
		}
	}

	mlo_sta_sync_channel(pAd, pEntry, channel);

	if (WMODE_CAP_N(pEntry->wdev->PhyMode) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		is_cap_ht = TRUE;
		NdisMoveMemory(&pEntry->HTCapability, &cmm_ies->ht_cap, sizeof(HT_CAPABILITY_IE));
		ht_mode_adjust(pAd, pEntry, &pEntry->HTCapability);
		set_sta_ht_cap(pAd, pEntry, &cmm_ies->ht_cap);
	}

	if (WMODE_CAP_AC(pEntry->wdev->PhyMode) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
		is_cap_vht = TRUE;
		sta_vht_mode_adjust(pAd, pEntry, cmm_ies);
		update_peer_vht_caps(pAd, pEntry, cmm_ies);
		peer_assoc_vht_info_debugshow(pAd, pEntry, cmm_ies);
	}

	update_peer_support_mcs(pAd, OPMODE_AP, pEntry, cmm_ies);
	if (WMODE_CAP_AX(pEntry->wdev->PhyMode)) {
		if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists))
			update_peer_he_caps(pEntry, cmm_ies);
		else if (HAS_HE_6G_CAP_EXIST(cmm_ies->ie_exists))
			update_peer_he_6g_caps(pEntry, cmm_ies);
		if (HAS_HE_OP_EXIST(cmm_ies->ie_exists))
			update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(pEntry->wdev, pEntry, NULL, (is_cap_ht || is_cap_vht));
	}

	if (WMODE_CAP_BE(pEntry->wdev->PhyMode)
			&& HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
		eht_update_peer_caps(pEntry, cmm_ies);
		eht_mode_adjust(pEntry->wdev, pEntry, cmm_ies);
	}
	if (cmm_ies->vendor_ie.EdcaParm.bValid) {
		PSTA_ADMIN_CONFIG pApCliEntry = GetStaCfgByWdev(pAd, pEntry->wdev);

		NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, &cmm_ies->vendor_ie.EdcaParm, sizeof(EDCA_PARM));
	}
}

INT sta_sync_mlo_comm_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct common_ies *cmm_ies, PEDCA_PARM pEdcaParm)
{
	INT idx = 0;
	BOOLEAN is_cap_ht = FALSE, is_cap_vht = FALSE;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *mld_link_entry[MLD_LINK_MAX];
	MAC_TABLE_ENTRY *dst_entry = NULL;
	RTMP_ADAPTER *ad = NULL;
	PSTA_ADMIN_CONFIG dst_pApCliEntry = NULL;

	if (!pEntry || !cmm_ies) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"pEntry or cmm_ies is NULL!\n");
		return -1;
	}
	mt_rcu_read_lock();
	if (pEntry->mlo.mlo_en)
		mld_entry = rcu_dereference(pEntry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld_entry is NULL, failed!\n");
		return -1;
	}
	for (idx = 0; idx < MLD_LINK_MAX; idx++)
		mld_link_entry[idx] = mld_entry->link_entry[idx];
	mt_rcu_read_unlock();
	/*sync CAP to other entry */
	for (idx = 0; idx < MLD_LINK_MAX; idx++) {
		dst_entry = mld_link_entry[idx];
		if (dst_entry == NULL || dst_entry == pEntry)
			continue;
		ad = (RTMP_ADAPTER *)dst_entry->pAd;

		if (WMODE_CAP_N(dst_entry->wdev->PhyMode) && !WMODE_CAP_6G(dst_entry->wdev->PhyMode) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
			is_cap_ht = TRUE;
			NdisMoveMemory(&dst_entry->HTCapability, &cmm_ies->ht_cap, sizeof(HT_CAPABILITY_IE));
			ht_mode_adjust(ad, dst_entry, &dst_entry->HTCapability);
			set_sta_ht_cap(ad, dst_entry, &cmm_ies->ht_cap);
		}
		if (WMODE_CAP_AC(dst_entry->wdev->PhyMode) && !WMODE_CAP_6G(dst_entry->wdev->PhyMode) &&
				HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			is_cap_vht = TRUE;
			sta_vht_mode_adjust(ad, dst_entry, cmm_ies);
			update_peer_vht_caps(ad, dst_entry, cmm_ies);
			peer_assoc_vht_info_debugshow(ad, dst_entry, cmm_ies);
		}
		update_peer_support_mcs(ad, OPMODE_AP, dst_entry, cmm_ies);
		if (WMODE_CAP_AX(dst_entry->wdev->PhyMode)) {
			if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists))
				update_peer_he_caps(dst_entry, cmm_ies);
			else if (HAS_HE_6G_CAP_EXIST(cmm_ies->ie_exists))
				update_peer_he_6g_caps(dst_entry, cmm_ies);
			if (HAS_HE_OP_EXIST(cmm_ies->ie_exists))
				update_peer_he_operation(dst_entry, cmm_ies);
			he_mode_adjust(dst_entry->wdev, dst_entry, NULL, (is_cap_ht || is_cap_vht));
		}
		if (WMODE_CAP_BE(dst_entry->wdev->PhyMode)
				&& HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			eht_update_peer_caps(dst_entry, cmm_ies);
			eht_mode_adjust(dst_entry->wdev, dst_entry, cmm_ies);
		}
		if (pEdcaParm) {
			dst_pApCliEntry = GetStaCfgByWdev(pAd, dst_entry->wdev);
			NdisMoveMemory(&dst_pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		}
	}

	return 0;
}

VOID sta_sync_mld_per_link_cap(struct _MAC_TABLE_ENTRY *pEntry, struct ml_ie_info *ml_info, struct common_ies *comm_ies)
{
	UINT8 i, j;
	MAC_TABLE_ENTRY *non_setup_link_pEntry = NULL;
	struct ml_ie_link *ml_link = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *mld_link_entry[MLD_LINK_MAX];
	UINT8 *sta_profile = NULL;
	UINT16 sta_profile_len = 0;
	PRTMP_ADAPTER pAd = NULL;
	IE_LISTS *ie_list = NULL;

	if (!pEntry || !ml_info) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"pEntry or ml_info is NULL!\n");
		return;
	}

	pAd = pEntry->wdev->sys_handle;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "pAd is NULL!\n");
		return;
	}

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));
	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "mem alloc failed!\n");
		return;
	}

	mt_rcu_read_lock();
	if (pEntry->mlo.mlo_en)
		mld_entry = rcu_dereference(pEntry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld_entry is NULL, failed!\n");
		os_free_mem(ie_list);
		return;
	}
	for (i = 0; i < MLD_LINK_MAX; i++)
		mld_link_entry[i] = mld_entry->link_entry[i];
	mt_rcu_read_unlock();

	for (i = 0; i < MLD_LINK_MAX; i++) {

		non_setup_link_pEntry = mld_link_entry[i];

		if (non_setup_link_pEntry == pEntry)
			continue;

		if (!non_setup_link_pEntry || !non_setup_link_pEntry->mlo.mlo_en)
			continue;

		for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
			ml_link = &ml_info->link[j];
			if (!ml_link->active)
				continue;
			/*check mac addr is match peer link addr*/
			if (!MAC_ADDR_EQUAL(non_setup_link_pEntry->Addr, ml_link->link_addr))
				continue;

			sta_profile = ml_link->sta_profile;
			sta_profile_len = ml_link->sta_profile_len;

			if (sta_profile_len <= 4) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"[link id=%d] no per link ie\n", j);
			} else {
				NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));
				/*update HE cap from common ies*/
				if (HAS_HE_CAPS_EXIST(comm_ies->ie_exists) &&
					WMODE_CAP_AX(non_setup_link_pEntry->wdev->PhyMode)) {
					SET_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists);
					NdisMoveMemory(&ie_list->cmm_ies.he_caps, &comm_ies->he_caps, sizeof(comm_ies->he_caps));
				}
				sta_parse_sta_profile_cap(sta_profile, sta_profile_len, ie_list, 1);

				pAd = non_setup_link_pEntry->wdev->sys_handle;
				sta_sync_ie_to_no_setup_entry(pAd, non_setup_link_pEntry, &ie_list->cmm_ies);
			}
		}
	}
	if (ie_list)
		os_free_mem(ie_list);
}
VOID sta_sync_mlo_csa_channel(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	UCHAR apidx;

	if (pAd->Dot11_H.mlo_csa_info.new_channel != 0 &&
		pEntry->wdev->channel != pAd->Dot11_H.mlo_csa_info.new_channel &&
		pAd->Dot11_H.mlo_csa_info.Ch_Switching != TRUE) {
		pAd->Dot11_H.mlo_csa_info.apidx = pEntry->wdev->func_idx;
		apidx = pAd->Dot11_H.mlo_csa_info.apidx;
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"Sync CSA_Info to BandSelBand(%d) apidx=%d channel=%d max_ch_sw_time=%d!\n",
			pAd->CommonCfg.BandSelBand, apidx,
			pAd->Dot11_H.mlo_csa_info.new_channel,
			pAd->Dot11_H.mlo_csa_info.MCST_info.SwitchTime);
		if (RTEnqueueInternalCmd(pAd, CMDTHREAD_APCLI_MLO_CSA_SWITCH_CHANNEL, &apidx, sizeof(UCHAR)) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Enq CMDTHREAD_APCLI_MLO_CSA_SWITCH_CHANNEL failed!\n");
		else
			pAd->Dot11_H.mlo_csa_info.Ch_Switching = TRUE;
	}
}

VOID sta_sync_mld_per_link_mlo_csa(struct _MAC_TABLE_ENTRY *pEntry, struct ml_ie_info *ml_info)
{
	UINT8 band_id, i, link_id;
	MAC_TABLE_ENTRY *non_setup_link_pEntry = NULL;
	struct ml_ie_link *ml_link = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *mld_link_entry[MLD_LINK_MAX];
	UINT8 *sta_profile = NULL;
	UINT16 sta_profile_len = 0;
	PRTMP_ADAPTER pSelfAd = NULL;
	PRTMP_ADAPTER pDestAd = NULL;

	if (!pEntry || !ml_info) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"pEntry or ml_info is NULL!\n");
		return;
	}

	pSelfAd = pEntry->wdev->sys_handle;

	if (!pSelfAd) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "pAd is NULL!\n");
		return;
	}

	mt_rcu_read_lock();
	mld_entry = rcu_dereference(pEntry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		MTWF_DBG(pSelfAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld_entry is NULL, failed!\n");
		return;
	}
	for (i = 0; i < MLD_LINK_MAX; i++)
		mld_link_entry[i] = mld_entry->link_entry[i];
	mt_rcu_read_unlock();

	for (i = 0; i < MLD_LINK_MAX; i++) {

		non_setup_link_pEntry = mld_link_entry[i];

		if (non_setup_link_pEntry == pEntry || !non_setup_link_pEntry)
			continue;

		for (band_id = 0; band_id < BSS_MNGR_MAX_BAND_NUM; band_id++) {
			ml_link = &ml_info->link[band_id];
			if (!ml_link->active)
				continue;
			sta_profile = ml_link->sta_profile;
			sta_profile_len = ml_link->sta_profile_len;
			link_id = GET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(ml_link->sta_ctrl);
			if (sta_profile_len <= 4) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"[link id=%d] no per link ie\n", link_id);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"[link id=%d]\n", link_id);
				if (i == link_id) {
					pDestAd = non_setup_link_pEntry->wdev->sys_handle;
					if (pDestAd->CommonCfg.bIEEE80211H == FALSE) {
						MTWF_DBG(pDestAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "bIEEE80211H is FALSE!\n");
						continue;
					}
					sta_sync_sta_profile_mlo_csa(pDestAd, sta_profile, sta_profile_len, 0);
					sta_sync_mlo_csa_channel(pDestAd, non_setup_link_pEntry);
				}
			}
		}
	}
}

/* parse mlo per_sta_profile IE, and update to ml_info->sta_profiles[]. */
VOID mld_parse_per_link_cap(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list,
	struct multi_link_info *ml_info)
{
	UINT8 link_id, link_num = 0;
	struct ml_ie_link *ml_link = NULL;
	UINT8 *sta_profile_ie = NULL;
	UINT16 sta_profile_len = 0;
	IE_LISTS *link_ie_list = NULL;
	UCHAR channel, ch_band = 0;
	struct common_ies *cmm_ies = NULL;
	struct sta_profile *sta_profile = NULL;
	struct ml_ie_info *ml_ie_info;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "Start.\n");
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "pAd is NULL!\n");
		return;
	}

	if (!ml_info) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "ml_info is NULL!\n");
		return;
	}

	os_alloc_mem(pAd, (UCHAR **)&ml_ie_info, sizeof(struct ml_ie_info));
	if (!ml_ie_info) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"failed to allocate memory for parsing ML IE\n");
		goto end;
	}

	os_alloc_mem(pAd, (UCHAR **)&link_ie_list, sizeof(IE_LISTS));
	if (link_ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "mem alloc failed!\n");
		goto end;
	}

	NdisZeroMemory(link_ie_list, sizeof(IE_LISTS));
	NdisZeroMemory(ml_ie_info, sizeof(struct ml_ie_info));
	if (parse_multi_link_ie(ie_list->cmm_ies.ml_ie, &ie_list->cmm_ies.ml_frag_info,
		ml_ie_info) < 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"failed to parse ML IE!\n");
		goto end;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "\n");

	COPY_MAC_ADDR(ml_info->mld_addr, ml_ie_info->mld_addr);

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		ml_link = &ml_ie_info->link[link_id];
		if (!ml_link->active)
			continue;
		sta_profile_ie = ml_link->sta_profile;
		sta_profile_len = ml_link->sta_profile_len;

		if (sta_profile_len <= 4) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[link id=%d] no per link ie\n", link_id);
			continue;
		}

		NdisZeroMemory((UCHAR *)link_ie_list, sizeof(IE_LISTS));
		channel = 0;

		/* parse per sta profile ie and store into ie_list */
		sta_parse_sta_profile_cap(sta_profile_ie, sta_profile_len, link_ie_list, 0);

		/* Fill into mlo sta_profile info */

		cmm_ies = &link_ie_list->cmm_ies;
		if (eht_calculate_ch_band(cmm_ies, &ch_band) < 0)
			continue;

		sta_profile = &ml_info->sta_profiles[link_id];
		sta_profile->rfband = ch_band;
		sta_profile->link_id = ml_link->link_id;
		COPY_MAC_ADDR(sta_profile->link_addr, ml_link->link_addr);
		link_num++;

		if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
			SET_HT_CAPS_EXIST(sta_profile->ie_exists);
			NdisMoveMemory(&sta_profile->HtCapability, &cmm_ies->ht_cap, SIZE_HT_CAP_IE);
			if (HAS_HT_OP_EXIST(cmm_ies->ie_exists)) {
				SET_HT_OP_EXIST(sta_profile->ie_exists);
				NdisMoveMemory(&sta_profile->AddHtInfo, &cmm_ies->ht_op, SIZE_ADD_HT_INFO_IE);
				channel = cmm_ies->ht_op.ControlChan;
				sta_profile->CentralChannel = get_cent_ch_by_htinfo(pAd, &cmm_ies->ht_op,
									   &cmm_ies->ht_cap);
			}

			if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
				NdisMoveMemory(&sta_profile->vht_cap_ie, &cmm_ies->vht_cap, SIZE_OF_VHT_CAP_IE);
				SET_VHT_CAPS_EXIST(sta_profile->ie_exists);
			}

			if (HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
				VHT_OP_IE *vht_op;

				NdisMoveMemory(&sta_profile->vht_op_ie, &cmm_ies->vht_op, SIZE_OF_VHT_OP_IE);
				SET_VHT_OP_EXIST(sta_profile->ie_exists);
				vht_op = &cmm_ies->vht_op;

				if ((vht_op->vht_op_info.ch_width > 0) &&
					(cmm_ies->ht_op.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
					(cmm_ies->ht_cap.HtCapInfo.ChannelWidth == BW_40) &&
					(sta_profile->CentralChannel != cmm_ies->ht_op.ControlChan)) {
					sta_profile->CentralChannel = vht_op->vht_op_info.ccfs_0;
				}
			}

			if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists))
				SET_HE_CAPS_EXIST(sta_profile->ie_exists);

			if (HAS_HE_OP_EXIST(cmm_ies->ie_exists))
				SET_HE_OP_EXIST(sta_profile->ie_exists);
		}

		if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
			NdisMoveMemory(&sta_profile->he_caps, &cmm_ies->he_caps, sizeof(struct he_cap_ie));
			NdisMoveMemory(&sta_profile->he_mcs_nss_160, &cmm_ies->mcs_nss_160,
				sizeof(struct he_txrx_mcs_nss));
			NdisMoveMemory(&sta_profile->he_mcs_nss_8080, &cmm_ies->mcs_nss_8080,
				sizeof(struct he_txrx_mcs_nss));
			SET_HE_CAPS_EXIST(sta_profile->ie_exists);
		}
		if (HAS_HE_OP_EXIST(cmm_ies->ie_exists)) {
			NdisMoveMemory(&sta_profile->he_ops, &cmm_ies->he_ops, sizeof(struct he_op_ie));
			SET_HE_OP_EXIST(sta_profile->ie_exists);
			if ((channel == 0)
				&& (cmm_ies->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT))
				channel = cmm_ies->he6g_opinfo.prim_ch;
		}

		if (HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			SET_EHT_CAPS_EXIST(sta_profile->ie_exists);
			NdisMoveMemory(&sta_profile->eht_support_mcs_nss, &cmm_ies->eht_support_mcs_nss,
				sizeof(struct eht_support_mcs_nss));
		}

		if (HAS_EHT_OP_EXIST(cmm_ies->ie_exists)) {
			SET_EHT_OP_EXIST(sta_profile->ie_exists);
			sta_profile->eht_cap.eht_ch_width = cmm_ies->eht_op.op_info.control;
			sta_profile->eht_cap.ccfs0 = cmm_ies->eht_op.op_info.ccfs0;
			sta_profile->eht_cap.ccfs1 = cmm_ies->eht_op.op_info.ccfs1;
		}

		sta_profile->channel = channel;

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"(%p)sta_profile[%d].channel: %d, band: %d\n",
			sta_profile, link_id, sta_profile->channel, sta_profile->rfband);
	}

	if (link_num)
		ml_info->link_num = link_num;

end:
	if (ml_ie_info)
		os_free_mem(ml_ie_info);
	if (link_ie_list)
		os_free_mem(link_ie_list);
	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"End, link_num=%d.\n", ml_info->link_num);
}

int init_sta_mld_link_mgr(void)
{
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "--->\n");

	os_zero_mem(&mld_device, sizeof(struct mld_dev));

	mld_device.mld_grp_idx = (uint32_t)(STA_MLD_GROUP_IDX);

	return 0;
}

void uninit_sta_mld_link_mgr(void)
{
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO, "--->\n");
}

VOID show_sta_mlo_link_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct mld_dev *mld = &mld_device;
	struct wifi_dev *pCurwdev = NULL;
	struct peer_mld_entry *peer = NULL;
	struct peer_mld_single_link *single_link = NULL;
	int i = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
			pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev->wdev_type != WDEV_TYPE_STA) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"wdev type error\n");
		return;
	}

	MTWF_PRINT("Show Sta Mlo Link Info:\n");

	if (mld->valid_link_num == 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"sta:mld_device not found\n");
		return;
	}

	pCurwdev = mld->master_link->wdev;
	if (pCurwdev == NULL)
		return;
	MTWF_PRINT("\tmaster link: (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		PRINT_MAC(pCurwdev->if_addr));
	MTWF_PRINT("\tMLD addr: (%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(mld->mld_addr));
	MTWF_PRINT("\tmld_gropu_idx: %d\n", mld->mld_grp_idx);
	MTWF_PRINT("\ttotal valid link num: %d\n", mld->valid_link_num);
	MTWF_PRINT("Apcli own links:\n");
	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (mld->mld_own_links[i].used == 1) {
			pCurwdev = mld->mld_own_links[i].wdev;
			if (pCurwdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"wdev not found\n");
				return;
			}
			MTWF_PRINT("\t[idx=%d band%d]:\n", i, wlan_config_get_ch_band(pCurwdev));
			MTWF_PRINT("\t\tlink_addr:(%02x:%02x:%02x:%02x:%02x:%02x)\n",
				PRINT_MAC(pCurwdev->if_addr));
			MTWF_PRINT("\t\tsetup link:%d\n", mld->mld_own_links[i].is_setup_link);
		}
	}
	if (mld->probe_req_ml_ie.buf && mld->probe_req_ml_ie.len) {
		MTWF_PRINT("ML IE in Probe Req: len = %d\n", mld->probe_req_ml_ie.len);
		hex_dump_with_cat_and_lvl("ML_IE:", mld->probe_req_ml_ie.buf, mld->probe_req_ml_ie.len,
			DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE);
	}
	if (mld->assoc_req_ml_ie.buf && mld->assoc_req_ml_ie.len) {
		MTWF_PRINT("ML IE in Assoc Req: len = %d\n", mld->assoc_req_ml_ie.len);
		hex_dump_with_cat_and_lvl("ML_IE:", mld->assoc_req_ml_ie.buf, mld->assoc_req_ml_ie.len,
			DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE);
	}
	if (mld->peer_mld.valid) {
		peer = &mld->peer_mld;
		MTWF_PRINT("Peer links:\n");
		MTWF_PRINT("\tMLD addr:(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(peer->mld_addr));
		MTWF_PRINT("\tmld idx:%d\n", peer->idx);
		MTWF_PRINT("\taid:%d\t\temlser:%d\n", peer->aid, peer->emlsr);
		MTWF_PRINT("\tBAOriTID:%d\t\tBARecTID:%d\n", peer->BAOriTID, peer->BARecTID);
		for (i = 0; i < MLD_LINK_MAX; i++) {
			if (peer->single_link[i].active) {
				single_link = &peer->single_link[i];
				MTWF_PRINT("\t[link id:%d]", single_link->link_id);
				MTWF_PRINT("\tlink_addr:(%02x:%02x:%02x:%02x:%02x:%02x)\n",
					PRINT_MAC(single_link->link_addr));
				MTWF_PRINT("\t\tsta_ctrl:%d\t\tbcn_interval:%d\n", single_link->sta_ctrl, single_link->bcn_interval);
				MTWF_PRINT("\t\tdtim_count:%d\t\tdtim_period:%d\n", single_link->dtim_count, single_link->dtim_period);
				MTWF_PRINT("\t\tnstr_bmap:%d\n", single_link->nstr_bmap);
			}
		}
	}
}

VOID mtk_cfg80211_vndr_cmd_set_mlo_switch_attributes(
	struct _RTMP_ADAPTER *pAd, UCHAR flag)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
			pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev->wdev_type != WDEV_TYPE_STA) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"wdev type error\n");
		return;
	}

#ifdef MAP_R6
	if (!IS_MAP_ENABLE(pAd)) {
#endif /* MAP_R6 */
		if (WDEV_BSS_STATE(wdev) >= BSS_INITED) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"wdev isn't in disconnected status\n");
			return;
		}
#ifdef MAP_R6
	}
#endif /* MAP_R6 */

	if (!IS_APCLI_DISABLE_MLO(wdev) && flag == 1)
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"apcli already enable MLO\n");
	else if (IS_APCLI_DISABLE_MLO(wdev) && flag == 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"apcli already disable MLO\n");
	} else if (IS_APCLI_DISABLE_MLO(wdev) && flag == 1) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"apcli need to enable MLO\n");
		if (sta_mld_link_mgr_reg_dev(wdev) < 0)
			return;
		wdev->apcli_mlo_diable = 0;
#if defined(CONFIG_MAP_SUPPORT) && defined(MTK_HOSTAPD_SUPPORT)
		pAd->map_apcli_mlo_disable = 0;
#endif
	} else if (!IS_APCLI_DISABLE_MLO(wdev) && flag == 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"apcli already disable MLO\n");
		sta_mld_link_mgr_dereg_dev(wdev);
		wdev->apcli_mlo_diable = 1;
#if defined(CONFIG_MAP_SUPPORT) && defined(MTK_HOSTAPD_SUPPORT)
		pAd->map_apcli_mlo_disable = 1;
#endif
	}

}

struct rnr_ml_param *get_unused_rnr_ml_param_entry(
	struct rnr_ml_param *rnr_ml_params, int rnr_len)
{
	int idx = 0;

	for (idx = 0; idx < rnr_len; idx++) {
		if (rnr_ml_params[idx].is_valid == 0)
			return &rnr_ml_params[idx];
	}
	return NULL;
}

struct rnr_ml_param *search_rnr_ml_param_entry(
struct rnr_ml_param *rnr_ml_params, int rnr_len, struct rnr_ml_param *ml_param)
{
	int idx = 0;

	for (idx = 0; idx < rnr_len; idx++) {
		if (os_cmp_mem(&rnr_ml_params[idx], ml_param, sizeof(*ml_param)) == 0)
			return &rnr_ml_params[idx];
	}
	return NULL;
}

/**
 * @sta_mld_get_add_link_rnr  get rnr_param of link need add
 *
 * @param *wdev wifi_dev
 * @bcn_ie_list
 */
struct rnr_ml_param *sta_mld_get_add_link_rnr(struct wifi_dev *wdev,
	BCN_IE_LIST *bcn_ie_list)
{
	struct mld_dev *mld;
	u8 link;
	MAC_TABLE_ENTRY *tmp_entry = NULL;
	UINT8 mld_link_id, rnr_link_id;
	UINT8 rnr_idx;
	struct rnr_ml_param *rnr_ml = NULL;

	if (!wdev || !bcn_ie_list)
		return NULL;

	mld = wdev->mld_dev;
	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! can not find mld_device\n");
		return NULL;
	}

	for (rnr_idx = 0; rnr_idx < MAX_MLD_GROUP_NUM; rnr_idx++) {
		if (!bcn_ie_list->rnr_ml_parms[rnr_idx].is_valid)
			continue;
		rnr_link_id = bcn_ie_list->rnr_ml_parms[rnr_idx].link_id;
		for (link = 0; link < MLD_LINK_MAX; link++) {
			if (mld->peer_mld.single_link[link].active &&
				!mld->peer_mld.single_link[link].reconfig_rm &&
				mld->mld_own_links[link].used) {
				tmp_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[link].priv_ptr;
				mld_link_id = mld->peer_mld.single_link[link].link_id;
			} else
				tmp_entry = NULL;
			if (!tmp_entry)
				continue;
			if (rnr_link_id == mld_link_id)
				break;
		}
		if (link == MLD_LINK_MAX) {
			rnr_ml = &bcn_ie_list->rnr_ml_parms[rnr_idx];
			break;
		}
	}
	return rnr_ml;
}

/**
 * @sta_mld_link_change_handle handle add/remove link
 *
 * @param pAd PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid
 * @bcn_ie_list
 */
void sta_mld_link_change_handle(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, u16 wcid, BCN_IE_LIST *bcn_ie_list)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct mld_dev *mld;
	UINT8 link;
	UINT8 valid_link_num = 0;
	MAC_TABLE_ENTRY *tmp_entry = NULL;

	if (!pAd || !VALID_UCAST_ENTRY_WCID(pAd, wcid) ||
			!wdev || !bcn_ie_list)
		return;

	pEntry = entry_get(pAd, wcid);
	if (IS_APCLI_DISABLE_MLO(wdev)
			|| !pEntry->mlo.mlo_en
			|| !HAS_EHT_MLD_EXIST(bcn_ie_list->cmm_ies.ie_exists))
		return;

	if (sta_parse_multi_link_ie(bcn_ie_list->cmm_ies.ml_ie,
		&bcn_ie_list->cmm_ies.ml_frag_info, &bcn_ie_list->mlo_ie_info) < 0) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"Parse multi_link_ie failed\n");
		return;
	}

	mld = wdev->mld_dev;
	if (mld != &mld_device) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ERROR!!! can not find mld_device\n");
		return;
	}

	/*++++add link handle++++*/
	for (link = 0; link < MLD_LINK_MAX; link++) {
		if (mld->peer_mld.single_link[link].active &&
			!mld->peer_mld.single_link[link].reconfig_rm &&
			mld->mld_own_links[link].used) {
			tmp_entry = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[link].priv_ptr;
			if (!tmp_entry)
				continue;
			valid_link_num++;
		}
	}
	if (((GET_DOT11BE_ML_BASIC_CMM_MLD_CAP_MAX_LINKS(bcn_ie_list->mlo_ie_info.mld_caps) + 1)
		> valid_link_num) &&
		!HAS_EHT_ML_RECONFIG_EXIST(bcn_ie_list->cmm_ies.ie_exists)) {
		struct rnr_ml_param *add_link_rnr_info = NULL;
		struct wifi_dev *tmp_wdev;
		struct peer_mld_single_link *single_link = NULL;
		BOOLEAN add_link = FALSE;

		add_link_rnr_info = sta_mld_get_add_link_rnr(wdev, bcn_ie_list);
		if (add_link_rnr_info) {
			UCHAR channel_band = WIFI_CH_BAND_NUM;

			/*added link is non setup link*/
			for (link = 0; link < MLD_LINK_MAX; link++) {
				tmp_wdev = mld->mld_own_links[link].wdev;
				if (!mld->mld_own_links[link].used || !tmp_wdev)
					continue;
				if (mld->peer_mld.single_link[link].active &&
					!mld->peer_mld.single_link[link].reconfig_rm)
					continue;

				channel_band = wlan_config_get_ch_band(tmp_wdev);
				if (add_link_rnr_info->channel_band == channel_band)
					break;
			}
			if (link < MLD_LINK_MAX)
				add_link = TRUE;
		} else {
			/*added link is setup link*/
			for (link = 0; link < MLD_LINK_MAX; link++) {
				single_link = &mld->peer_mld.single_link[link];
				if (!single_link->active)
					continue;
				tmp_entry = single_link->priv_ptr;
				if (!tmp_entry)
					continue;

				if (tmp_entry->mlo.is_setup_link_entry &&
					single_link->reconfig_rm) {
					add_link = TRUE;
					break;
				}
			}
		}
		if (add_link) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Apcli add link, disassociate firstly\n");
			LinkDown(pAd, 0, wdev, NULL);
		}
	}
	/*----add link handl----*/

	/*++++remove link handle++++*/
	if (bcn_ie_list->mlo_ie_reconfig.type == ML_CTRL_TYPE_RECONFIG) {
		struct ml_ie_info *ml_info = &bcn_ie_list->mlo_ie_reconfig;
		u8 link_id;
		struct ml_ie_link *ml_link;
		struct reconfig_rm_link_req_t req = {0};
		u8 i;
		struct peer_mld_single_link *peer_link = NULL;
		struct wifi_dev *tmp_wdev = NULL;
		u8 link_id_bss_info_idx[MLD_LINK_MAX] = {0};
		u16 rm_link_id_bitmap = 0;
		BOOLEAN remove_link = FALSE;

		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			ml_link = &ml_info->link[link_id];
			if (ml_link->active) {
				if (ml_link->link_id >= BSS_MNGR_MAX_BAND_NUM)
					continue;

				for (i = 0; i < MLD_LINK_MAX; i++) {
					peer_link = &mld->peer_mld.single_link[i];
					tmp_wdev = mld->mld_own_links[i].wdev;
					if (peer_link->link_id == ml_link->link_id)
						break;
				}
				if (i == MLD_LINK_MAX || !tmp_wdev
					|| !peer_link)
					continue;
				if (peer_link->reconfig_rm)
					continue;
				peer_link->reconfig_rm = TRUE;
				link_id_bss_info_idx[i] = tmp_wdev->bss_info_argument.ucBssIndex;
				rm_link_id_bitmap |= 1 << (ml_link->link_id);
				remove_link = TRUE;
			}
		}
		/*sync to WM fw*/
		if (remove_link) {
			COPY_MAC_ADDR(req.mld_addr, mld->mld_addr);
			req.fw_mld_idx = 0xff;
			req.flag = 0;
			req.rm_link_id_bitmap = rm_link_id_bitmap;
			for (i = 0; i < MLD_LINK_MAX; i++)
				req.link_id_bss_info_idx[i] = link_id_bss_info_idx[i];
			HW_REQ_RECONFIG_RM_LINK(wdev, &req);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"remove link_id = %d, rm_link_bitmap = %x\n", ml_link->link_id, req.rm_link_id_bitmap);
		}
	}
	/*----remove link handle----*/
}

#ifdef RT_CFG80211_SUPPORT
int mtk_cfg80211_reply_apcli_mld(struct wiphy *wiphy)
{
	struct mld_dev *mld = &mld_device;
	struct wifi_dev *pCurwdev = NULL;
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb = NULL;
	int ret = 0, payload_len = 0, i = 0;
	struct nlattr *tmp_attr, *affiliated_sta_attr;
	UCHAR emlsr_mr;
	ULONG *__pPriv;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"pPriv is NULL\n");
		return -EFAULT;
	}

	if (mld->valid_link_num == 0)
		return -EFAULT;

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);

	/*3 * sizeof(struct nlattr) + 1 + ETH_ALEN + 1 + 3 *(4 * sizeof(struct nlattr) + 8)*/
	payload_len =  15 * sizeof(struct nlattr) + 64;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		goto fail;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC, MAC_ADDR_LEN, mld->mld_addr)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	if (mld->peer_mld.valid) {
		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC, MAC_ADDR_LEN, mld->peer_mld.mld_addr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
	}

	tmp_attr = nla_nest_start(skb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS);
	if (!tmp_attr) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_nest_start fail.\n");
		goto fail;
	}
	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (!mld->mld_own_links[i].used)
			continue;
		pCurwdev = mld->mld_own_links[i].wdev;
		if (pCurwdev == NULL)
			continue;
		affiliated_sta_attr = nla_nest_start(skb, i);
		if (!affiliated_sta_attr) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"nla_nest_start fail.\n");
			goto fail;
		}
		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC, MAC_ADDR_LEN, pCurwdev->if_addr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_flag(skb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_STR_ENABLE)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		emlsr_mr = wlan_config_get_emlsr_mr(pCurwdev);
		if (emlsr_mr) {
			if (nla_put_flag(skb, emlsr_mr == 1 ?
				MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLSR_ENABLE :
				MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLMR_ENABLE)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}

		if (mld->peer_mld.single_link[i].active) {
			if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC, MAC_ADDR_LEN,
				mld->peer_mld.single_link[i].link_addr)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}

		nla_nest_end(skb, affiliated_sta_attr);
	}

	nla_nest_end(skb, tmp_attr);

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "reply apcli mld\n");

	return ret;
fail:
	if (skb)
		kfree_skb(skb);
	return -EINVAL;
}
#endif
#endif
