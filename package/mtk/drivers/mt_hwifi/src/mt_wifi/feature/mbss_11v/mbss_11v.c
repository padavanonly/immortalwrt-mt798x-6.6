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

    Abstract:

    Support 11v multi-BSS function.

***************************************************************************/

#ifdef DOT11V_MBSSID_SUPPORT

#include "rt_config.h"

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                P R I V A T E    F U N C T I O N S
 *******************************************************************************
 */
static void
mbss_11v_free_nt_list(LIST_HEADER *list)
{
	RT_LIST_ENTRY *entry = NULL;

	entry = list->pHead;
	while (entry) {
		removeHeadList(list);
		os_free_mem(entry);
		entry = list->pHead;
	}
}

static u8
mbss_11v_find_bss(struct _BSS_STRUCT *t_bss, struct _BSS_STRUCT *role_bss)
{
	struct mbss_11v_member_node *bss_node;
	struct mbss_11v_ctrl *t_mbss_11v = &t_bss->mbss_11v;
	u8 found = FALSE;

	OS_SEM_LOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
	bss_node = (struct mbss_11v_member_node *)t_mbss_11v->mbss_11v_member.pHead;

	while (bss_node) {
		if (bss_node->role_bss == role_bss) {
			found = TRUE;
			break;
		}
		bss_node = (struct mbss_11v_member_node *)bss_node->pNext;
	}
	OS_SEM_UNLOCK(&t_mbss_11v->mbss_11v_ctrl_lock);

	return found;
}

static void
mbss_11v_show_bss_information(
	struct wifi_dev *wdev, struct _BSS_STRUCT *mbss, u8 band_idx)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	pSecConfig = &wdev->SecConfig;
	MTWF_PRINT("\t\t\t - SSID: %s (H=%d), Short: %08x, Idx: %2d\n",
		 mbss->Ssid, mbss->bHideSsid,
		 mbss->ShortSSID, mbss->mbss_idx);

	MTWF_PRINT("\t\t\t - Band:%d (OM:0x%02x), Ch:%3d, Dtim:%2d, StaCount:%3d\n",
		band_idx, wdev->OmacIdx,
		wdev->channel, mbss->DtimPeriod, mbss->StaCount);
	MTWF_PRINT("\t\t\t - AuthMode: %s, Cipher(P:%s/G:%s), BmcIdx=0x%x\n",
		GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
		GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
		GetEncryModeStr(GET_GROUP_CIPHER(pSecConfig)),
		wdev->bss_info_argument.bmc_wlan_idx);
	MTWF_PRINT("\t\t\t - MaxIdle:%5ds\n",
		mbss->max_idle_period);
}

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

u8 mbss_11v_bssid_num_to_max_indicator(UCHAR bssid_num)
{
	UCHAR max_bssid_num = 1;
	UCHAR dot11v_max_bssid_indicator = 0;

	if (bssid_num > 0) {
		while (max_bssid_num < bssid_num) {
			dot11v_max_bssid_indicator++;
			max_bssid_num = 1 << dot11v_max_bssid_indicator;
		}
	}

	return dot11v_max_bssid_indicator;
}

void mbss_11v_init(struct _RTMP_ADAPTER *ad, struct _BSS_STRUCT *mbss)
{
	struct mbss_11v_ctrl *mbss_11v = &mbss->mbss_11v;

	initList(&mbss_11v->mbss_11v_member);
	OS_NdisAllocateSpinLock(&mbss_11v->mbss_11v_ctrl_lock);
	mbss_11v->mbss_11v_enable = MBSS_11V_NONE;
	mbss_11v->mbss_11v_t_bss_idx = 0;
	mbss_11v->mbss_11v_grp_idx = 0;
}

void mbss_11v_exit(struct _RTMP_ADAPTER *ad, struct _BSS_STRUCT *mbss)
{
	struct mbss_11v_ctrl *mbss_11v = &mbss->mbss_11v;

	mbss_11v_free_nt_list(&mbss_11v->mbss_11v_member);
	NdisFreeSpinLock(&mbss_11v->mbss_11v_ctrl_lock);
}

u8 mbss_11v_add_member_bss(struct _BSS_STRUCT *t_bss, struct _BSS_STRUCT *role_bss, enum mbss_11v_mode role)
{
	LIST_HEADER *bss_list;
	struct mbss_11v_member_node *bss_node;
	struct mbss_11v_ctrl *t_mbss_11v = &t_bss->mbss_11v;

	OS_SEM_LOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
	bss_list = &t_mbss_11v->mbss_11v_member;
	/* allocate a entry */
	os_alloc_mem(NULL, (UCHAR **) &(bss_node), sizeof(struct mbss_11v_member_node));
	if (bss_node) {
		bss_node->role = role;

		if (role == MBSS_11V_NT)
			t_mbss_11v->mbss_11v_nt_bss_num++;
		else if (role == MBSS_11V_CH)
			t_mbss_11v->mbss_11v_ch_bss_num++;

		bss_node->role_bss = role_bss;
		insertTailList(bss_list, (RT_LIST_ENTRY *)bss_node);
	} else {
		OS_SEM_UNLOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
		return FALSE;
	}
	OS_SEM_UNLOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
	return TRUE;
}

void mbss_11v_remove_member_bss(struct _BSS_STRUCT *t_bss, struct _BSS_STRUCT *role_bss)
{
	LIST_HEADER *bss_list;
	struct mbss_11v_member_node *bss_node;
	struct mbss_11v_ctrl *t_mbss_11v = &t_bss->mbss_11v;

	OS_SEM_LOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
	bss_list = &t_mbss_11v->mbss_11v_member;
	bss_node = (struct mbss_11v_member_node *)bss_list->pHead;
	while (bss_node) {
		if (bss_node->role_bss == role_bss) {

			if (bss_node->role == MBSS_11V_NT) {
				if (t_mbss_11v->mbss_11v_nt_bss_num)
					t_mbss_11v->mbss_11v_nt_bss_num--;
				else
					MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
						"Impossible, please check NT.!!\n");
			} else if (bss_node->role == MBSS_11V_CH) {
				if (t_mbss_11v->mbss_11v_ch_bss_num)
					t_mbss_11v->mbss_11v_ch_bss_num--;
				else
					MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
						"Impossible, please check CH.!!\n");
			} else
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
					"Impossible, please check role=%d.!!\n", bss_node->role);

			delEntryList(bss_list, (RT_LIST_ENTRY *)bss_node);
			os_free_mem(bss_node);
			break;
		}

		bss_node = (struct mbss_11v_member_node *)bss_node->pNext;
	}
	OS_SEM_UNLOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
}

/* return per tx group's mbss idx */
u8 mbss_11v_group_add(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _BSS_STRUCT *mbss = (struct _BSS_STRUCT *)wdev->func_dev;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	u8 bss_trans_idx = 0;
	u8 dot11v_mbssid_idx = 0;

	if (mbss) {
		struct _BSS_STRUCT *t_bss;
		struct mbss_11v_ctrl *mbss_11v_ctrl = &mbss->mbss_11v;

		MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_INFO,
				"Add [%s-bss(=%d)]\n",
				RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev),
				mbss->mbss_idx);

		bss_trans_idx = mbss_11v_ctrl->mbss_11v_t_bss_idx;
		if ((bss_trans_idx >= MAX_BEACON_NUM)
			|| !(BIT(bss_trans_idx) & cap->transmitted_bss_bitmap)) {
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_WARN,
				"[%s]incorrect bss_trans_idx(=%d)\n",
				RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev),
				bss_trans_idx);
			/* incorrect setting, this mbss cannot be 11v mbss. */
			mbss_11v_ctrl->mbss_11v_enable = MBSS_11V_NONE;
			return dot11v_mbssid_idx;
		}
		if ((mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_NT) || (mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_CH)) {
			t_bss = &ad->ApCfg.MBSSID[bss_trans_idx];
			if (mbss_11v_find_bss(t_bss, mbss) == FALSE) {
				if (mbss_11v_add_member_bss(t_bss, mbss, mbss_11v_ctrl->mbss_11v_enable) == FALSE)
					MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
						"[%s]Add bss failed.\n",
						RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
				else {
					mbss_11v_ctrl->mbss_11v_t_bss = t_bss;

					if ((mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_NT)
							&& (mbss->mbss_idx >= bss_trans_idx))
						dot11v_mbssid_idx = (mbss->mbss_idx - bss_trans_idx);

					mbss_11v_ctrl->mbss_11v_nt_idx = dot11v_mbssid_idx;
					OS_SEM_LOCK(&t_bss->mbss_11v.mbss_11v_ctrl_lock);
					t_bss->mbss_11v.mbss_11v_max_bssid_indicator =
						mbss_11v_bssid_num_to_max_indicator(
							t_bss->mbss_11v.mbss_11v_nt_bss_num +
							t_bss->mbss_11v.mbss_11v_ch_bss_num + 1);
					mbss_11v_ctrl->mbss_11v_max_bssid_indicator =
						t_bss->mbss_11v.mbss_11v_max_bssid_indicator;
					OS_SEM_UNLOCK(&t_bss->mbss_11v.mbss_11v_ctrl_lock);
				}
			} else {
				/* Found , return the dot11v_mbssid_idx */
				if (mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_NT)
					dot11v_mbssid_idx = mbss_11v_ctrl->mbss_11v_nt_idx;
			}
		} else if (mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_T)
			mbss_11v_ctrl->mbss_11v_t_bss = mbss;
	} else
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR, "[%s]mbss is NULL.\n",
			RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));

	return dot11v_mbssid_idx;
}

void mbss_11v_group_remove(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _BSS_STRUCT *mbss = (struct _BSS_STRUCT *)wdev->func_dev;

	if (mbss) {
		struct _BSS_STRUCT *t_bss;
		struct mbss_11v_ctrl *mbss_11v_ctrl = &mbss->mbss_11v;

		if (((mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_NT) || (mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_CH))
			&& mbss_11v_ctrl->mbss_11v_t_bss) {
			t_bss = mbss_11v_ctrl->mbss_11v_t_bss;

			mbss_11v_remove_member_bss(t_bss, mbss);
/*
			mbss_11v_ctrl->mbss_11v_enable = MBSS_11V_NONE;
			mbss_11v_ctrl->mbss_11v_t_bss = NULL;
*/
			mbss_11v_ctrl->mbss_11v_nt_idx = 0;

			MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_INFO,
				"[%s]Remove from 11v mbss list.\n",
				RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
		} else {
			if (mbss_11v_ctrl->mbss_11v_nt_bss_num)
				MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_ERROR,
					"[%s]This 11v T_BSS still has NT_BSS.\n",
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
		}
	}
}

void mbss_11v_group_information(struct _RTMP_ADAPTER *ad, u8 dump_level)
{
	UINT32 i;
	BSS_STRUCT *mbss, *role_bss = NULL;
	INT ret;
	CHAR type[16] = "";
	struct wifi_dev *wdev;
	struct mbss_11v_ctrl *mbss_11v_ctrl;
	struct mbss_11v_member_node *bss_node;
	CHAR *mod_str = NULL;
	u8 band_idx = hc_get_hw_band_idx(ad);

	MTWF_PRINT("\n\t11v MBSS Information:\n");
	if (band_idx == INVALID_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("\tInvalid band_idx(=%d)\n", band_idx);
		return;
	}

	for (i = 0; i < MAX_TX_BSS_CNT; i++)
		MTWF_PRINT("\tdot11v_BssidNum[%d]=%u\n", i, ad->ApCfg.dot11v_BssidNum[i]);

	for (i = 0; i < ad->ApCfg.BssidNum; i++) {
		mbss = &ad->ApCfg.MBSSID[i];
		mbss_11v_ctrl = &mbss->mbss_11v;
		wdev = &mbss->wdev;

		if ((mbss_11v_ctrl->mbss_11v_enable == MBSS_11V_T)
			&& (mbss_11v_ctrl->mbss_11v_nt_bss_num < MAX_BEACON_NUM)
			&& wdev->if_dev) {
			struct _BSS_STRUCT *t_bss = mbss_11v_ctrl->mbss_11v_t_bss;

			ret = snprintf(type, sizeof(type), "%s", "(11vT)");
			if (os_snprintf_error(sizeof(type), ret)) {
				MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_WARN,
					"snprintf error!\n");
			}
			mod_str = wmode_2_str(wdev->PhyMode);

			OS_SEM_LOCK(&t_bss->mbss_11v.mbss_11v_ctrl_lock);
			MTWF_PRINT("\tmax_bssid_indicator=%d, bitmap=0x%08x\n", mbss_11v_ctrl->mbss_11v_max_bssid_indicator, ad->ApCfg.dot11v_mbssid_bitmap);
			OS_SEM_UNLOCK(&t_bss->mbss_11v.mbss_11v_ctrl_lock);

			MTWF_PRINT("\tT_BSS Idx\t\tPhy Mode\tIF_addr\t\t\tbss_idx\tmbss_idx\n");
			MTWF_PRINT("\t%9d\t%16s\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t%d\t(%s)\t%s\n",
				i, mod_str, PRINT_MAC(wdev->if_addr),
				mbss->mbss_idx,
				0,
				RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), type);
			/* dump mbss advanced info */
			if (dump_level > 0)
				mbss_11v_show_bss_information(wdev, mbss, band_idx);
			if (mod_str)
				os_free_mem(mod_str);

			OS_SEM_LOCK(&mbss_11v_ctrl->mbss_11v_ctrl_lock);
			bss_node =
				(struct mbss_11v_member_node *)mbss_11v_ctrl->mbss_11v_member.pHead;
			while (bss_node) {
				struct mbss_11v_ctrl *role_11v_ctrl = NULL;
				role_bss = bss_node->role_bss;
				wdev = &role_bss->wdev;
				role_11v_ctrl = &role_bss->mbss_11v;
				mod_str = wmode_2_str(wdev->PhyMode);
				if (bss_node->role == MBSS_11V_NT)
					ret = snprintf(type, sizeof(type), "%s", "(11vNT)");
				else
					ret = snprintf(type, sizeof(type), "%s", "(11vCoH)");

				if (os_snprintf_error(sizeof(type), ret)) {
					MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS,
						DBG_LVL_WARN, "snprintf error!\n");
				}
				MTWF_PRINT(
					"\t%9u\t%16s\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t%d\t(%s)\t%s\n",
					role_11v_ctrl->mbss_11v_t_bss_idx, mod_str, PRINT_MAC(wdev->if_addr),
					role_bss->mbss_idx,
					role_11v_ctrl->mbss_11v_nt_idx,
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), type);
				/* dump mbss advanced info */
				if (dump_level > 0)
					mbss_11v_show_bss_information(wdev, role_bss, band_idx);
				bss_node = (struct mbss_11v_member_node *)bss_node->pNext;
				if (mod_str)
					os_free_mem(mod_str);
			}
			OS_SEM_UNLOCK(&mbss_11v_ctrl->mbss_11v_ctrl_lock);
			MTWF_PRINT("\n");
		}
	}
}

struct wifi_dev *mbss_11v_get_tx_wdev(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_STRUCT *mbss,
	BOOLEAN *bMakeBeacon)
{
	struct mbss_11v_ctrl *bss_11v_ctrl = &mbss->mbss_11v;
	struct wifi_dev *t_wdev = wdev;

	/* if BSSID is non-transmitted, must do update by transmitted BSSID */
	if (bss_11v_ctrl->mbss_11v_t_bss
		&& bss_11v_ctrl->mbss_11v_t_bss != mbss) {
		UCHAR OrigWdevIdx = wdev->wdev_idx;

		if (bss_11v_ctrl->mbss_11v_enable == MBSS_11V_NT) {
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_INFO,
				"wdev(%d) is Nontransmitted Bssid, update to BssIdx %d wdev(%d)\n",
				OrigWdevIdx, mbss->mbss_11v.mbss_11v_t_bss_idx, wdev->wdev_idx);

			/* make new beacon with this MBSSID's IE add/removed */
			*bMakeBeacon = TRUE;
			t_wdev = &bss_11v_ctrl->mbss_11v_t_bss->wdev;
		}
	}
	return t_wdev;
}

void mbss_11v_tim_ie_handle(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_STRUCT *mbss,
	UCHAR *pBeaconFrame,
	ULONG *FrameLen)
{
	ULONG frame_len = *FrameLen;
	LIST_HEADER *bss_list;
	struct mbss_11v_member_node *bss_node;
	struct mbss_11v_ctrl *t_mbss_11v = &mbss->mbss_11v;

	if (IS_MBSSID_IE_NEEDED(t_mbss_11v)) {
		ULONG mbssie_offset = frame_len;
		struct mbss_query_info mbss_info = {0};
		INT32 id_bss = 1;
		BCN_BUF_STRUCT *pbcn_buf;
		struct _BSS_STRUCT *nt_bss = NULL;

		mbss_info.is_probe_rsp = FALSE;	/* beacon */
		mbss_info.f_buf = (u8 *)(pBeaconFrame + frame_len);

		frame_len += bss_mngr_query_mbssid_ie(wdev, &mbss_info);
		OS_SEM_LOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
		bss_list = &t_mbss_11v->mbss_11v_member;
		bss_node = (struct mbss_11v_member_node *)bss_list->pHead;

		/* update tim ie offset & cap info offset & multi link traffic ie offset */
		while (bss_node) {
			nt_bss = bss_node->role_bss;
			if ((bss_node->role == MBSS_11V_NT) && nt_bss) {
				pbcn_buf = &nt_bss->wdev.bcn_buf;
				pbcn_buf->tim_ie_offset =
					mbssie_offset + mbss_info.tim_ie_offset[id_bss];
				pbcn_buf->cap_ie_pos =
					mbssie_offset + mbss_info.cap_info_offset[id_bss];

				MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_INFO,
					"\tIdBss%d tim offset: %d -> %d\n",
					id_bss,
					mbss_info.tim_ie_offset[id_bss],
					pbcn_buf->tim_ie_offset);
#ifdef DOT11_EHT_BE
				pbcn_buf->mlt_ie_offset =
					mbssie_offset + mbss_info.mlt_ie_offset[id_bss];

				MTWF_DBG(ad, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_INFO,
					"\tIdBss%d mlt offset: %d -> %d\n",
					id_bss,
					mbss_info.mlt_ie_offset[id_bss],
					pbcn_buf->mlt_ie_offset);
#endif /* DOT11_EHT_BE */
				id_bss++;
			}
			bss_node = (struct mbss_11v_member_node *)bss_node->pNext;
		}
		OS_SEM_UNLOCK(&t_mbss_11v->mbss_11v_ctrl_lock);
	}
	*FrameLen = frame_len;
}

/*
 * this function is for MT7915 and afterward chips
 * (per-band) 11v address assignment
 */
VOID mbss_11v_set_if_addr_gen3(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	UCHAR *if_addr = (UCHAR *)wdev->if_addr;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	INT mbss_idx = wdev->func_idx;
	struct _BSS_STRUCT *mbss = (struct _BSS_STRUCT *)wdev->func_dev;
	struct mbss_11v_ctrl *mbss_11v_ctrl = &mbss->mbss_11v;
	INT mbss_trans_bss_idx = mbss_11v_ctrl->mbss_11v_t_bss_idx;
	UCHAR tx_grp_idx = mbss_11v_ctrl->mbss_11v_grp_idx;
	UCHAR max_indicator = pAd->ApCfg.dot11v_max_indicator[tx_grp_idx];
	UCHAR MacMask = BITS(0, (max_indicator - 1));

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
			"%s: tx_grp_idx(%u), mbss_idx(%d), band(%d), trans idx(%d), bLocalAdminMAC=%d\n",
			__func__, tx_grp_idx, mbss_idx, band_idx, mbss_trans_bss_idx, pAd->bLocalAdminMAC);


#ifdef MAC_ADDR_ADJACENT_CHK
	if (PD_GET_MAC_ADDR_ADJ(pAd->physical_dev)) {
		if ((band_idx == BAND1 || band_idx == BAND2) &&
			(pAd->bLocalAdminMAC == FALSE)) {
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			if_addr[0] &= 0xe7;
			if_addr[0] |= 0x2;
			if_addr[0] |= (band_idx << 3);
			COPY_MAC_ADDR(pAd->CurrentAddress, if_addr);
		}
	}
#endif /* MAC_ADDR_ADJACENT_CHK */

	if (mbss_idx == 0) {
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
	} else {
		if (mbss_idx &&
			NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[mbss_idx - 1], MAC_ADDR_LEN)) {
			UCHAR byte5 = 0;
			/* mac addr not assigned */
			if (mbss_trans_bss_idx > 0) {
				/* use gen2's for non first group Tx-BSS */
				MtAsicSetMbssWdevIfAddrGen2(pAd, wdev);
				COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[mbss_trans_bss_idx - 1]);
#ifdef MAC_ADDR_ADJACENT_CHK
				if (PD_GET_MAC_ADDR_ADJ(pAd->physical_dev)) {
					if_addr[0] &= 0xe7;
					if_addr[0] |= 0x2;
					if_addr[0] |= (band_idx << 3);
				}
#endif /* MAC_ADDR_ADJACENT_CHK */
				/* leverage MtAsicSetMbssWdevIfAddrGen2() :
				 * reverse bit[24] to prevent 2nd(3rd/4th)
				 * Tx BSS MAC conflit with main mac
				 */

				if (NdisEqualMemory(if_addr, pAd->CurrentAddress, MAC_ADDR_LEN)) {
					if_addr[3] ^= 0x1;
					/* write back to table */
					 pAd->ExtendMBssAddr[mbss_trans_bss_idx - 1][3] = if_addr[3];
				}

			} else
				COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			byte5 = if_addr[5];
			/* clear n LSB bits */
			if_addr[5] &= ~MacMask;
			/* assign n LSB bits to [REF_BSSID + idx] mod 2^n */
			if_addr[5] |= ((byte5 + mbss_idx) & MacMask);
			/* write back to table */
			COPY_MAC_ADDR(pAd->ExtendMBssAddr[mbss_idx - 1], if_addr);
		} else if (mbss_idx) {
			/* mac addr assigned */
			COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[mbss_idx - 1]);
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_11V_MBSS, DBG_LVL_NOTICE,
			 "\tif_addr = "MACSTR"\n", MAC2STR(if_addr));
}


#ifdef APCLI_CFG80211_SUPPORT
static inline void gen_new_bssid(const u8 *bssid, u8 max_bssid,
					  u8 mbssid_index, u8 *new_bssid)
{
	u64 bssid_u64 = ether_addr_to_u64(bssid);
	u64 mask = GENMASK_ULL(max_bssid - 1, 0);
	u64 new_bssid_u64;

	new_bssid_u64 = bssid_u64 & ~mask;

	new_bssid_u64 |= ((bssid_u64 & mask) + mbssid_index) & mask;

	u64_to_ether_addr(new_bssid_u64, new_bssid);
}

size_t gen_new_ie(const u8 *ie, size_t ielen,
				  const u8 *subelement, size_t subie_len, u8 *new_ie)
{
	u8 *pos, *tmp;
	const u8 *tmp_old, *tmp_new;
	const struct element *non_inherit_elem;
	u8 *sub_copy;

	/* copy subelement as we need to change its content to
	 * mark an ie after it is processed.
	 */
	sub_copy = kmemdup(subelement, subie_len, /*GFP_KERNEL*/GFP_ATOMIC);
	if (!sub_copy)
		return 0;

	pos = &new_ie[0];

	/* set new ssid */
	tmp_new = cfg80211_find_ie(WLAN_EID_SSID, sub_copy, subie_len);
	if (tmp_new) {
		memcpy(pos, tmp_new, tmp_new[1] + 2);
		pos += (tmp_new[1] + 2);
	}

	/* get non inheritance list if exists */
	non_inherit_elem =
		cfg80211_find_ext_elem(WLAN_EID_EXT_NON_INHERITANCE,
				       sub_copy, subie_len);

	/* go through IEs in ie (skip SSID) and subelement,
	 * merge them into new_ie
	 */
	tmp_old = cfg80211_find_ie(WLAN_EID_SSID, ie, ielen);
	tmp_old = (tmp_old) ? tmp_old + tmp_old[1] + 2 : ie;

	while (tmp_old + 2 - ie <= ielen &&
	       tmp_old + tmp_old[1] + 2 - ie <= ielen) {
		if (tmp_old[0] == 0) {
			tmp_old++;
			continue;
		}

		if (tmp_old[0] == WLAN_EID_EXTENSION)
			tmp = (u8 *)cfg80211_find_ext_ie(tmp_old[2], sub_copy,
							 subie_len);
		else
			tmp = (u8 *)cfg80211_find_ie(tmp_old[0], sub_copy,
						     subie_len);

		if (!tmp) {
			const struct element *old_elem = (void *)tmp_old;

			/* ie in old ie but not in subelement */
			if (cfg80211_is_element_inherited(old_elem,
							  non_inherit_elem)) {
				memcpy(pos, tmp_old, tmp_old[1] + 2);
				pos += tmp_old[1] + 2;
			}
		} else {
			/* ie in transmitting ie also in subelement,
			 * copy from subelement and flag the ie in subelement
			 * as copied (by setting eid field to WLAN_EID_SSID,
			 * which is skipped anyway).
			 * For vendor ie, compare OUI + type + subType to
			 * determine if they are the same ie.
			 */
			if (tmp_old[0] == WLAN_EID_VENDOR_SPECIFIC) {
				if (tmp_old[1] >= 5 && tmp[1] >= 5 &&
				    !memcmp(tmp_old + 2, tmp + 2, 5)) {
					/* same vendor ie, copy from
					 * subelement
					 */
					memcpy(pos, tmp, tmp[1] + 2);
					pos += tmp[1] + 2;
					tmp[0] = WLAN_EID_SSID;
				} else {
					memcpy(pos, tmp_old, tmp_old[1] + 2);
					pos += tmp_old[1] + 2;
				}
			} else {
				/* copy ie from subelement into new ie */
				memcpy(pos, tmp, tmp[1] + 2);
				pos += tmp[1] + 2;
				tmp[0] = WLAN_EID_SSID;
			}
		}

		if (tmp_old + tmp_old[1] + 2 - ie == ielen)
			break;

		tmp_old += tmp_old[1] + 2;
	}

	/* go through subelement again to check if there is any ie not
	 * copied to new ie, skip ssid, capability, bssid-index ie
	 */
	tmp_new = sub_copy;
	while (tmp_new + 2 - sub_copy <= subie_len &&
	       tmp_new + tmp_new[1] + 2 - sub_copy <= subie_len) {
		if (!(tmp_new[0] == WLAN_EID_NON_TX_BSSID_CAP ||
		      tmp_new[0] == WLAN_EID_SSID)) {
			memcpy(pos, tmp_new, tmp_new[1] + 2);
			pos += tmp_new[1] + 2;
		}
		if (tmp_new + tmp_new[1] + 2 - sub_copy == subie_len)
			break;
		tmp_new += tmp_new[1] + 2;
	}

	kfree(sub_copy);
	return pos - new_ie;
}

static inline const u8 *
find_ie_match(u8 eid, const u8 *ies, unsigned int len,
		       const u8 *match, unsigned int match_len,
		       unsigned int match_offset)
{
	/* match_offset can't be smaller than 2, unless match_len is
	 * zero, in which case match_offset must be zero as well.
	 */
	if (WARN_ON((match_len && match_offset < 2) ||
		    (!match_len && match_offset)))
		return NULL;

	return (const void *)cfg80211_find_elem_match(eid, ies, len,
						      match, match_len,
						      match_offset ?
							match_offset - 2 : 0);
}

static inline const u8 *find_ie(u8 eid, const u8 *ies, int len)
{
	return find_ie_match(eid, ies, len, NULL, 0, 0);
}

/* cfg80211_get_profile_continuation */
static const struct element
*get_profile_continuation(const u8 *ie, size_t ielen,
				   const struct element *mbssid_elem,
				   const struct element *sub_elem)
{
	const u8 *mbssid_end = mbssid_elem->data + mbssid_elem->datalen;
	const struct element *next_mbssid;
	const struct element *next_sub;

	next_mbssid = cfg80211_find_elem(WLAN_EID_MULTIPLE_BSSID,
					 mbssid_end,
					 ielen - (mbssid_end - ie));

	/*
	 * If it is not the last subelement in current MBSSID IE or there isn't
	 * a next MBSSID IE - profile is complete.
	*/
	if ((sub_elem->data + sub_elem->datalen < mbssid_end - 1) ||
	    !next_mbssid)
		return NULL;

	/* For any length error, just return NULL */

	if (next_mbssid->datalen < 4)
		return NULL;

	next_sub = (void *)&next_mbssid->data[1];

	if (next_mbssid->data + next_mbssid->datalen <
	    next_sub->data + next_sub->datalen)
		return NULL;

	if (next_sub->id != 0 || next_sub->datalen < 2)
		return NULL;

	/*
	 * Check if the first element in the next sub element is a start
	 * of a new profile
	 */
	return next_sub->data[0] == WLAN_EID_NON_TX_BSSID_CAP ?
	       NULL : next_mbssid;
}

/* cfg80211_merge_profile */
size_t merge_profile(const u8 *ie, size_t ielen,
			      const struct element *mbssid_elem,
			      const struct element *sub_elem,
			      u8 *merged_ie, size_t max_copy_len)
{
	size_t copied_len = sub_elem->datalen;
	const struct element *next_mbssid;


	const struct element *tmp_mbssid_elem = mbssid_elem;
	const struct element *tmp_sub_elem = sub_elem;

	if (sub_elem->datalen > max_copy_len)
		return 0;

	memcpy(merged_ie, sub_elem->data, sub_elem->datalen);

	while ((next_mbssid = get_profile_continuation(ie, ielen,
								/*mbssid_elem*/tmp_mbssid_elem,
								/* sub_elem*/ tmp_sub_elem))) {
		const struct element *next_sub = (void *)&next_mbssid->data[1];

		if (copied_len + next_sub->datalen > max_copy_len)
			break;
		memcpy(merged_ie + copied_len, next_sub->data,
		       next_sub->datalen);
		copied_len += next_sub->datalen;

		tmp_mbssid_elem = next_mbssid;
		tmp_sub_elem = next_sub;
	}

	return copied_len;
}

/* cfg80211_parse_mbssid_data */
void parse_mbssid_data(struct _RTMP_ADAPTER *pAd,
				       enum cfg80211_bss_frame_type ftype,
				       const u8 *bssid, u64 tsf,
				       u16 beacon_interval, struct ieee80211_mgmt *mgmt,
				       size_t len,
				       struct wifi_dev *wdev,  struct _RX_BLK *pRxBlk)
{
	const u8 *mbssid_index_ie;
	const struct element *elem, *sub;
	size_t new_ie_len, dup_frm_len;
	u8 new_bssid[ETH_ALEN];
	u8 *new_ie = NULL, *profile = NULL;
	u64 seen_indices = 0;
	u16 capability;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 wcid = pRxBlk->wcid;

	/* record the original whole frame */
	struct ieee80211_mgmt *ori_frm  = mgmt;
	u8 *ie = mgmt->u.probe_resp.variable;
	size_t ielen = len - offsetof(struct ieee80211_mgmt, u.probe_resp.variable);

	/* dup frm */
	struct ieee80211_mgmt *dup_frm = NULL;

	if (!cfg80211_find_elem(WLAN_EID_MULTIPLE_BSSID, ie, ielen))
		return;

	os_alloc_mem(NULL, (PUCHAR *)&dup_frm, IEEE80211_MAX_DATA_LEN);
	if (!dup_frm)
		return;

	os_alloc_mem(NULL, (PUCHAR *)&profile, ielen);
	if (!profile)
		goto out;

	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, ie, ielen) {
		if (elem->datalen < 4)
			continue;
		if (elem->data[0] < 1 || (int)elem->data[0] > 8)
			continue;
		for_each_element(sub, elem->data + 1, elem->datalen - 1) {
			size_t profile_len; /* cfg api have the bug that merge over u8 */

			if (sub->id != 0 || sub->datalen < 4) {
				/* not a valid BSS profile */
				continue;
			}

			if (sub->data[0] != WLAN_EID_NON_TX_BSSID_CAP ||
			    sub->data[1] != 2) {
				/* The first element within the Nontransmitted
				 * BSSID Profile is not the Nontransmitted
				 * BSSID Capability element.
				 */
				continue;
			}

			memset(profile, 0, ielen);

			/* Fix cfg80211_merge_profile() merge too big profile_len */
			profile_len = merge_profile(ie, ielen,
							     elem,
							     sub,
							     profile,
							     ielen);

			/* found a Nontransmitted BSSID Profile */
			mbssid_index_ie = find_ie
				(WLAN_EID_MULTI_BSSID_IDX,
				 profile, profile_len);
			if (!mbssid_index_ie || mbssid_index_ie[1] < 1 ||
			    mbssid_index_ie[2] == 0 ||
			    mbssid_index_ie[2] > 46) {
				/* No valid Multiple BSSID-Index element */
				continue;
			}

			if (seen_indices & BIT_ULL(mbssid_index_ie[2]))
				/* We don't support legacy split of a profile */
				MTWF_PRINT("Partial info for BSSID index %d\n",
						    mbssid_index_ie[2]);

			seen_indices |= BIT_ULL(mbssid_index_ie[2]);

			gen_new_bssid(bssid, elem->data[0], mbssid_index_ie[2], new_bssid);

			memset(dup_frm, 0, IEEE80211_MAX_DATA_LEN);

			new_ie =  ((u8 *)dup_frm + offsetof(struct ieee80211_mgmt, u.probe_resp.variable));

			new_ie_len = gen_new_ie(ie, ielen,
							 profile,
							 profile_len, new_ie);

			if (!new_ie_len)
				continue;

			capability = get_unaligned_le16(profile + 2);

			/* re-gen header : include the header + fix part */
			dup_frm_len = (new_ie_len + offsetof(struct ieee80211_mgmt, u.probe_resp.variable));
			memcpy((u8 *)dup_frm, (u8 *)ori_frm, offsetof(struct ieee80211_mgmt, u.probe_resp.variable));
			// fake A2/BSSID/cap
			memcpy((u8 *)(dup_frm->sa), (u8 *)new_bssid, ETH_ALEN); //A2
			memcpy((u8 *)(dup_frm->bssid), (u8 *)new_bssid, ETH_ALEN); // BSSID
			dup_frm->u.probe_resp.capab_info = capability; //cap

			/* re-search wcid for beacon rx time update in
			*  sta_rx_peer_response_updated()
			*/
			pEntry = MacTableLookup(pAd, new_bssid);

			if (pEntry)
				wcid = pEntry->wcid;

			REPORT_MGMT_FRAME_TO_MLME(pAd, wcid,
				dup_frm,
				dup_frm_len,
				pRxBlk->rx_signal.raw_rssi[0],
				pRxBlk->rx_signal.raw_rssi[1],
				pRxBlk->rx_signal.raw_rssi[2],
				pRxBlk->rx_signal.raw_rssi[3],
				min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
				pRxBlk->channel_freq,
				OPMODE_AP,
				wdev,
				pRxBlk->rx_rate.field.MODE,
				0,
				pRxBlk->raw_channel);
		}
	}

out:
	if (dup_frm)
		os_free_mem(dup_frm);

	if (profile)
		os_free_mem(profile);
}

/* porting from cfg80211 func : cfg80211_parse_mbssid_frame_data() */
BOOLEAN ap_ieee_802_11_rx_bcn_dup(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,  struct _RX_BLK *rx_blk)
{
	enum cfg80211_bss_frame_type ftype;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)(rx_blk->FC);
	size_t len = (size_t)(rx_blk->DataSize);

	ftype = ieee80211_is_beacon(mgmt->frame_control) ?
		CFG80211_BSS_FTYPE_BEACON : CFG80211_BSS_FTYPE_PRESP;

	parse_mbssid_data(pAd, ftype, mgmt->bssid,
			   le64_to_cpu(mgmt->u.probe_resp.timestamp),
			   le16_to_cpu(mgmt->u.probe_resp.beacon_int),
			   mgmt, len, wdev,  rx_blk);

	return TRUE;

}

#endif /* APCLI_CFG80211_SUPPORT */

#endif /* DOT11V_MBSSID_SUPPORT */

