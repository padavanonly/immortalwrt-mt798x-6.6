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
    cmm_info_element.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"
#include "mgmt/be_internal.h"

void ie_filter_init(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev)
{
	wdev->ap_ie_filter.filter_mode = IE_FILTER_DISABLE;
	initList(&wdev->ap_ie_filter.ie_filter_list);
	OS_NdisAllocateSpinLock(&wdev->ap_ie_filter.ie_filter_lock);
}

static struct ie_filter_node *ie_filter_get_rule_by_eid(
	LIST_HEADER * list,
	UCHAR frm,
	UINT16 search_eid)
{
	struct ie_filter_node *node = NULL;

	if (list)
		node = (struct ie_filter_node *)list->pHead;

	while (node) {
		if ((node->eid == search_eid) && (node->frm == frm))
			break;
		node = (struct ie_filter_node *)node->pNext;
	}

	return node;
}

static INT ie_filter_show(
	struct wifi_dev *wdev)
{
	LIST_HEADER *list;
	struct ie_filter_node *node;
	int x = 0;

	if (wdev == NULL)
		return -1;

	OS_SEM_LOCK(&wdev->ap_ie_filter.ie_filter_lock);
	list = &wdev->ap_ie_filter.ie_filter_list;
	node = (struct ie_filter_node *)list->pHead;

	MTWF_PRINT("Mode [%d] Rule [%d]\nAct\t Frm\t Eid\t Len\t Content\n",
		wdev->ap_ie_filter.filter_mode, list->size);

	while (node) {
		MTWF_PRINT("[%c]\t %d\t %x\t %d\t",
			node->action?'M':'D', node->frm, node->eid, node->ie_len);

		for (x = 0; x < node->ie_len; x++)
			MTWF_PRINT("%02x ", ((unsigned char)node->ie_content[x]));

		MTWF_PRINT("\n");
		node = (struct ie_filter_node *)node->pNext;
	}
	OS_SEM_UNLOCK(&wdev->ap_ie_filter.ie_filter_lock);

	return 0;
}

static void ie_filter_update_rule(
	struct ie_filter_node *node,
	UINT32 frm,
	UINT32 eid,
	UINT32 act,
	UCHAR *ie_ctnt,
	UINT32 ie_len)
{
	node->frm = (UCHAR)frm;
	node->eid = (UINT16)eid;
	node->action = (UCHAR)act;

	if (node->ie_content)
		os_free_mem(node->ie_content);

	node->ie_len = 0;
	node->ie_content = NULL;

	if ((act == IE_FILTER_RULE_MODIFY || act == IE_FILTER_RULE_APPEND) &&
		(ie_len > 0) &&
		(ie_len <= MAX_LEN_OF_IE_DATA) &&
		(ie_ctnt != NULL)) {
		os_alloc_mem(NULL, (UCHAR **) &(node->ie_content), ie_len);
		if (node->ie_content) {
			os_move_mem(node->ie_content, ie_ctnt, ie_len);
			node->ie_len = ie_len;
		}
	}

}
static INT ie_filter_add_rule(
	struct wifi_dev *wdev,
	UINT32 frm,
	UINT32 eid,
	UINT32 act,
	UCHAR *ie_ctnt,
	UINT32 ie_len)
{
	LIST_HEADER *list;
	struct ie_filter_node *new_rule, *old_rule = NULL;

	if (wdev == NULL)
		return -1;

	OS_SEM_LOCK(&wdev->ap_ie_filter.ie_filter_lock);
	list = &wdev->ap_ie_filter.ie_filter_list;
	old_rule = ie_filter_get_rule_by_eid(list, (UCHAR)frm, (UINT16)eid);
	/* update rule */
	if (old_rule) {
		ie_filter_update_rule(old_rule, frm, eid, act, ie_ctnt, ie_len);
		goto end;
	}

	/* add new rule */
	os_alloc_mem(NULL, (UCHAR **) &(new_rule), sizeof(struct ie_filter_node));
	if (new_rule == NULL)
		goto end;

	new_rule->ie_len = 0;
	new_rule->ie_content = NULL;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_NOTICE,
		"[+] frm:%d, eid:0x%x, action:%d, ie_len:%d\n",
		frm, eid, act, ie_len);

	ie_filter_update_rule(new_rule, frm, eid, act, ie_ctnt, ie_len);
	insertTailList(list, (RT_LIST_ENTRY *)new_rule);
end:
	OS_SEM_UNLOCK(&wdev->ap_ie_filter.ie_filter_lock);
	return 0;
}

static BOOLEAN ie_filter_sanity(
	UINT32 frm,
	UINT32 eid,
	UINT32 act)
{
	if (frm != SUBTYPE_BEACON)
		return FALSE;

	if (eid > 0xffff)
		return FALSE;

	if (act >= IE_FILTER_RULE_INVALID)
		return FALSE;

	return TRUE;
}

static INT ie_filter_del_rule(
	struct wifi_dev *wdev,
	UINT32 frm,
	UINT32 rm_eid)
{
	LIST_HEADER *list;
	struct ie_filter_node *node;

	if (wdev == NULL)
		return -1;

	OS_SEM_LOCK(&wdev->ap_ie_filter.ie_filter_lock);
	list = &wdev->ap_ie_filter.ie_filter_list;
	node = ie_filter_get_rule_by_eid(list, (UCHAR)frm, (UINT16)rm_eid);
	if (node) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_NOTICE,
			"[-] frm:%d, eid:0x%x\n", frm, rm_eid);
		delEntryList(list, (RT_LIST_ENTRY *)node);
		if (node->ie_content)
			os_free_mem(node->ie_content);
		os_free_mem(node);
	} else
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_NOTICE,
			"[Not Found] frm:%d, eid:0x%x\n", frm, rm_eid);

	OS_SEM_UNLOCK(&wdev->ap_ie_filter.ie_filter_lock);
	return 0;
}


INT ie_filter_oper_proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	UINT8 value;
	struct wifi_dev *wdev = NULL;
	struct os_cookie *os_obj;

	if (arg == NULL)
		return FALSE;

	os_obj = (struct os_cookie *)pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		return FALSE;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if ((os_obj->ioctl_if < 0) || (os_obj->ioctl_if >= MAX_BEACON_NUM))
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[os_obj->ioctl_if].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */

	value = (UINT8)os_str_tol(arg, 0, 10);

	if (wdev)
		wdev->ap_ie_filter.filter_mode = value;

	return TRUE;
}

INT ie_filter_rule_proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct os_cookie *os_obj;
	UINT32 frm = 0, oper = 0, eid = 0, act = 0;
	UINT32 input_argument = 0;
	UCHAR *ctnt;
	UCHAR *ie_hex_ctnt;

	if (arg == NULL)
		return FALSE;

	os_obj = (struct os_cookie *)pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		return FALSE;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if ((os_obj->ioctl_if < 0) || (os_obj->ioctl_if >= MAX_BEACON_NUM))
			return FALSE;
		wdev = &pAd->ApCfg.MBSSID[os_obj->ioctl_if].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */

	os_alloc_mem(pAd, (UCHAR **)&ctnt, (MAX_LEN_OF_IE_DATA + 1) * 2);
	os_alloc_mem(pAd, (UCHAR **)&ie_hex_ctnt, MAX_LEN_OF_IE_DATA);

	if (!ctnt || !ie_hex_ctnt)
		goto end;

	os_zero_mem(ctnt, (MAX_VENDOR_IE_LEN + 1) * 2);
	os_zero_mem(ie_hex_ctnt, MAX_VENDOR_IE_LEN);

	input_argument = sscanf(arg,
				"%d-frm:%d-eid:%x-act:%d-ctnt:%511s",
				&oper, &frm, &eid, &act, ctnt);
	if (input_argument <= 1)
		goto end;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_DEBUG,
		"oper:%d, frm:%d, eid:0x%x, action:%d, len:%lu, ctnt:%s\n",
		oper, frm, eid, act, strlen(ctnt), ctnt);

	if (ie_filter_sanity(frm, eid, act) == FALSE)
		goto end;

	switch (oper) {
	case 1:
		AtoH(ctnt, ie_hex_ctnt, strlen(ctnt));
		ie_filter_add_rule(wdev, frm, eid, act, ie_hex_ctnt, (strlen(ctnt)/2));
		break;
	case 2:
		ie_filter_del_rule(wdev, frm, eid);
		break;
	default:
		ie_filter_show(wdev);
		break;

	}

end:
	if (ctnt)
		os_free_mem(ctnt);
	if (ie_hex_ctnt)
		os_free_mem(ie_hex_ctnt);

	return TRUE;
}


BOOLEAN makeup_ie(
	UINT16 TagNumber,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pFrame)
{
	LIST_HEADER *list;
	struct ie_filter_node *node;
	ULONG TmpLen;
	BOOLEAN ret = FALSE;

	if (!wdev)
		return FALSE;

	/* Disable ie filter*/
	if (wdev->ap_ie_filter.filter_mode == IE_FILTER_DISABLE)
		return FALSE;

	/* Enable and Force remove all*/
	if (wdev->ap_ie_filter.filter_mode == IE_FILTER_ENABLE_ALL)
		return TRUE;

	/* Enable and apply user define rule*/
	if (wdev->ap_ie_filter.filter_mode == IE_FILTER_ENABLE_RULE) {
		OS_SEM_LOCK(&wdev->ap_ie_filter.ie_filter_lock);
		list = &wdev->ap_ie_filter.ie_filter_list;
		node = ie_filter_get_rule_by_eid(list, SUBTYPE_BEACON, (UINT16)TagNumber);
		if (node) {
			if (node->action == IE_FILTER_RULE_MODIFY && (node->ie_len > 0)) {
				MakeOutgoingFrame(pFrame + (*pFrameLen), &TmpLen,
						  node->ie_len,
						  node->ie_content, END_OF_ARGS);
				*pFrameLen += TmpLen;
			}
			ret = TRUE;
		}
		OS_SEM_UNLOCK(&wdev->ap_ie_filter.ie_filter_lock);
	}

	return ret;
}

VOID SupportRate(
	IN struct legacy_rate *rate,
	OUT PUCHAR *ppRates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate)
{
	INT i;
	UCHAR sup_rate_len, ext_rate_len, *sup_rate, *ext_rate;

	sup_rate_len = rate->sup_rate_len;
	ext_rate_len = rate->ext_rate_len;
	sup_rate = rate->sup_rate;
	ext_rate = rate->ext_rate;

	*pMaxSupportRate = 0;
	if ((sup_rate_len <= MAX_LEN_OF_SUPPORTED_RATES) && (sup_rate_len > 0)) {
		NdisMoveMemory(*ppRates, sup_rate, sup_rate_len);
		*RatesLen = sup_rate_len;
	} else {
		*RatesLen = 8;
		*(*ppRates + 0) = 0x82;
		*(*ppRates + 1) = 0x84;
		*(*ppRates + 2) = 0x8b;
		*(*ppRates + 3) = 0x96;
		*(*ppRates + 4) = 0x12;
		*(*ppRates + 5) = 0x24;
		*(*ppRates + 6) = 0x48;
		*(*ppRates + 7) = 0x6c;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_DEBUG,
			"SUPP_RATES., Len=%d\n", sup_rate_len);
	}

	if (ext_rate_len + *RatesLen <= MAX_LEN_OF_SUPPORTED_RATES) {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ext_rate, ext_rate_len);
		*RatesLen = (*RatesLen) + ext_rate_len;
	} else {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ext_rate, MAX_LEN_OF_SUPPORTED_RATES - (*RatesLen));
		*RatesLen = MAX_LEN_OF_SUPPORTED_RATES;
	}


	for (i = 0; i < *RatesLen; i++) {
		if (*pMaxSupportRate < (*(*ppRates + i) & 0x7f))
			*pMaxSupportRate = (*(*ppRates + i) & 0x7f);
	}
}

#ifdef DOT11_N_SUPPORT
void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE * pIE,
	IN UCHAR Channel,
	IN USHORT PhyMode,
	IN struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
#ifdef ZERO_PKT_LOSS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	pIE->ID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	pIE->Length = 4;
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*if radar not detected on old channel, allow frames*/
	if ((pAd->Zero_Loss_Enable == 1) && (pDfsParam->ZeroLossRadarDetect == FALSE))
		pIE->ChannelSwitchMode = 0;/*frames allowed*/
	else {
#endif /*ZERO_PKT_LOSS_SUPPORT*/
		pIE->ChannelSwitchMode = 1;	/*no further frames */
#ifdef DOT11_EHT_BE
	/* Fill the channel_switch_mode = 0 when PP+CSA flow*/
	pIE->ChannelSwitchMode = (wlan_config_get_eht_csa_dscb_enable(wdev)) ? 0 : 1;
#endif
#ifdef ZERO_PKT_LOSS_SUPPORT
	}
#endif
	pIE->NewRegClass = get_regulatory_class_for_newCh(pAd, Channel, PhyMode, wdev);
	pIE->NewChannelNum = Channel;
	pIE->ChannelSwitchCount = (pDot11h->CSPeriod - pDot11h->CSCount);
}
#endif /* DOT11_N_SUPPORT */

VOID build_max_channel_switch_time_ie(
	PRTMP_ADAPTER pAd,
	struct MAX_CHANNEL_SWITCH_TIME_IE *pIE)
{
	NdisZeroMemory(pIE, sizeof(struct MAX_CHANNEL_SWITCH_TIME_IE));

	pIE->ID = IE_WLAN_EXTENSION;
	pIE->Length = 4;
	pIE->ID_EXTENTION = IE_EXTENSION_ID_MAX_CH_SWITCH_TIME;
	pIE->SwitchTime = cpu2le16(pAd->Dot11_H.MaxChannelSwitchTime);
}

VOID build_wide_bandwidth_ie(
	UINT8 channel,
	UINT8 bw,
	UINT8 cen_ch_1,
	UINT8 cen_ch_2,
	struct WIDE_BW_CH_SWITCH_IE *pIE)
{
	UCHAR bw_value = 0;
	UINT8 ccfs0 = cen_ch_1, ccfs1 = 0;

	NdisZeroMemory(pIE, sizeof(struct WIDE_BW_CH_SWITCH_IE));

	pIE->id = IE_WIDE_BW_CH_SWITCH;
	pIE->len = WIDE_BW_CH_SWITCH_ELEMENT_LEN;

	switch (bw) {
	case BW_80:
		bw_value = 1;
		break;

	case BW_160:
		bw_value = 1;
		ccfs0 = GET_BW160_PRIM80_CENT(channel, cen_ch_1);
		ccfs1 = cen_ch_1;
		break;

	case BW_8080:
		bw_value = 1;
		ccfs1 = cen_ch_2;
		break;

	case BW_320:
		bw_value = 4;
		ccfs0 = GET_BW320_PRIM160_CENT(channel, cen_ch_1);
		ccfs1 = cen_ch_1;
		break;
	}

	pIE->ccfs0 = ccfs0;
	pIE->ccfs1 = ccfs1;
	pIE->bw = bw_value;
}

VOID MakeCSAIe(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN bcn,
	ULONG *pFrameLen,
	UCHAR *pFrame)
{
	ULONG FrameLen = *pFrameLen;
#if defined(A_BAND_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	struct DOT11_H *pDot11h = wdev->pDot11_H;
#endif

	/* fill Channel switch related elements:  */
	if (wpa3_test_ctrl == 6 || wpa3_test_ctrl == 7
#if defined(A_BAND_SUPPORT) && defined(CONFIG_AP_SUPPORT)
		|| (pComCfg->bIEEE80211H && (pDot11h->ChannelMode == CHAN_SWITCHING_MODE))
#endif
#ifdef DOT11_EHT_BE
		|| wlan_config_get_eht_csa_dscb_enable(wdev)
#endif
		) {
		UINT16 ie_size = 0;

		build_channel_switch_relatd_ie(pAd, wdev, pFrame + FrameLen, &ie_size);
		if (bcn)
			wdev->bcn_buf.CsaIELocationInBeacon = FrameLen;

		FrameLen += ie_size;
	}

	*pFrameLen = FrameLen;
}

VOID build_channel_switch_relatd_ie(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *buf,
	UINT16 *buf_len)
{
	UINT8 *ptr = NULL;
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	struct DOT11_H *pDot11h = NULL;
	UINT8 channel;
	USHORT PhyMode;
	UINT8 bw = BW_20, cen_ch_1, cen_ch_2;
#ifdef ZERO_PKT_LOSS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif
#ifdef DOT11_EHT_BE
	UINT8 bw_ind_ie_enable = 0;
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"wdev is NULL, return\n");
		return;
	}

#ifdef DOT11_EHT_BE
	bw_ind_ie_enable = wlan_config_get_eht_csa_dscb_enable(wdev);
#endif

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pDot11h is NULL, return\n");
		return;
	}

	channel = pDot11h->csa_chn_info.new_ch;
	bw = pDot11h->csa_chn_info.new_bw;
	PhyMode = wdev->PhyMode;
	cen_ch_1 = pDot11h->csa_chn_info.new_cench1;
	cen_ch_2 = pDot11h->csa_chn_info.new_cench2;

	/* Channel Switch Announcement Element */
	*buf_len = 0;
	ptr = buf;
	*ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	*(ptr + 1) = 3; /* Len = 3 */
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*if radar not detected on old channel, allow frames*/
	if ((pAd->Zero_Loss_Enable == 1) && (pDfsParam->ZeroLossRadarDetect == FALSE))
		*(ptr + 2) = 0;/*frames allowed*/
	else
#endif /*ZERO_PKT_LOSS_SUPPORT*/
		*(ptr + 2) = 1;	/* Switch_mode = 1, disable TX */
#ifdef DOT11_EHT_BE
	/* Fill the channel_switch_mode = 0 when PP+CSA flow*/
	if (bw_ind_ie_enable)
		*(ptr + 2) = 0;
#endif
	*(ptr + 3) = channel;
	*(ptr + 4) = (pDot11h->CSPeriod - pDot11h->CSCount);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Channel=%d, bw=%d, cs_count=%d\n",
			channel, bw, (pDot11h->CSPeriod - pDot11h->CSCount));

	ptr += 5;
	*buf_len += 5;
#ifdef DOT11_N_SUPPORT
	/* Extended Channel Switch Announcement Element */
	if (pComCfg->bExtChannelSwitchAnnouncement) {
		HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

		build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe,
									channel,
									wdev->PhyMode,
									wdev
								   );
		NdisMoveMemory(ptr, &HtExtChannelSwitchIe, sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE));
		ptr += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
		*buf_len += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
	}
#ifdef ZERO_PKT_LOSS_SUPPORT
	/*add secondary channel offset IE
	* for 5Ghz, find ext chn offset using new channel
	*/
	if (pAd->Zero_Loss_Enable) {
		struct GNU_PACKED SecondaryChannelOffsetIe {
			UCHAR		ID;
			UCHAR		Length;
			UCHAR		SecondaryChannelOffset;
		};
		struct SecondaryChannelOffsetIe SecChanOffsetIe;
			UCHAR ext_cha = 0, op_ht_bw = 0;
			int idx;
			UCHAR wfa_ht_ch_ext[] = {
			36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
			44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
			52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
			60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
			100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
			108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
			116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
			124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
			132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
			140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
			149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
			157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
			0, 0};

		op_ht_bw = wlan_operate_get_ht_bw(wdev);
		if (op_ht_bw == BW_40) {
			if (wdev->channel > 14) {
				idx = 0;
				while (wfa_ht_ch_ext[idx] != 0) {
					if (wfa_ht_ch_ext[idx] == wdev->channel) {
						ext_cha = wfa_ht_ch_ext[idx + 1];
						break;
					}
					idx += 2;
				};
				if (wfa_ht_ch_ext[idx] == 0)
					ext_cha = EXTCHA_NONE;
			} else {
				/*2G band case*/
				ext_cha = wlan_operate_get_ext_cha(wdev);
			}
		}
		SecChanOffsetIe.ID = 0x3e;
		SecChanOffsetIe.Length = 0x01;
		SecChanOffsetIe.SecondaryChannelOffset = ext_cha;
		NdisMoveMemory(ptr, &SecChanOffsetIe, sizeof(struct SecondaryChannelOffsetIe));
		ptr += sizeof(struct SecondaryChannelOffsetIe);
		*buf_len += sizeof(struct SecondaryChannelOffsetIe);
	}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		INT tp_len = 0, wb_len = 0, bw_ind_len = 0;
		UCHAR *ch_sw_wrapper;

		*ptr = IE_CH_SWITCH_WRAPPER;
		ch_sw_wrapper = (UCHAR *)(ptr + 1); /* reserve for length */
		ptr += 2; /* skip len */

		if (bw >= BW_40) {
			struct WIDE_BW_CH_SWITCH_IE wb_ie;

			build_wide_bandwidth_ie(channel, bw, cen_ch_1, cen_ch_2, &wb_ie);
			NdisMoveMemory(ptr, &wb_ie, sizeof(struct WIDE_BW_CH_SWITCH_IE));
			ptr += sizeof(struct WIDE_BW_CH_SWITCH_IE);
			wb_len = sizeof(struct WIDE_BW_CH_SWITCH_IE);
		}

		if (WMODE_CAP_AX(PhyMode)
			|| WMODE_CAP_AC(PhyMode)
			|| WMODE_CAP_BE(PhyMode)) {
			struct _tx_pwr_env_ie tpe_ie;

			tp_len = build_txpwr_envelope(pAd, wdev, &tpe_ie);
			*ptr = IE_TPE;
			*(ptr + 1) = tp_len;
			ptr += 2;
			NdisMoveMemory(ptr, &tpe_ie, sizeof(struct _tx_pwr_env_ie));
			ptr += tp_len;
			tp_len += 2;
		}
#ifdef DOT11_EHT_BE
		/*Bandwidth Indication subelement*/
		if (WMODE_CAP_BE(PhyMode)) {
			if ((wlan_config_get_eht_csa_dis_subch_bitmap(wdev) != 0) || (bw == BW_320)) {
				struct _bw_ind_ie bw_ind_ie = {0};

				bw_ind_len = build_bw_indication_ie(pAd, wdev, &bw_ind_ie, bw);
				*ptr = IE_WLAN_EXTENSION;
				/*Add one more byte in length field for EXTENSION ID*/
				*(ptr + 1) = bw_ind_len + 1;
				*(ptr + 2) = IE_EXTENSION_ID_BW_IND;
				ptr += 3;
				NdisMoveMemory(ptr, &bw_ind_ie, bw_ind_len);
				ptr += bw_ind_len;
				bw_ind_len += 3;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
					"Added BW IND IE, len=%d,(caller:%pS)\n", bw_ind_len, OS_TRACE);
			}
		}
#endif
		*ch_sw_wrapper = wb_len + tp_len + bw_ind_len;
		*buf_len += (2 + wb_len + tp_len + bw_ind_len);
	}

#endif /* DOT11_VHT_AC */

	/* Extended Channel Switch Announcement Element */
	if (pComCfg->bMaxChannelSwitchTime) {
		struct MAX_CHANNEL_SWITCH_TIME_IE MaxChannelSwitchTimeIe;
		UINT16 ie_len = sizeof(struct MAX_CHANNEL_SWITCH_TIME_IE);

		build_max_channel_switch_time_ie(pAd, &MaxChannelSwitchTimeIe);
		NdisMoveMemory(ptr, &MaxChannelSwitchTimeIe, ie_len);
		ptr += ie_len;
		*buf_len += ie_len;
	}

#endif /* DOT11_N_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "buf_len=%d\n", *buf_len);

}

static INT build_wsc_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_INCLUDED
	BOOLEAN bHasWscIe = FALSE;
#ifdef CONFIG_MAP_SUPPORT

/* To prevent WSC overlap in dbdc case ,disable wsc IE in probe request during all channel scan */
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		if ((pAd->ScanCtrl.Channel != 0) &&
			(pAd->ScanCtrl.ScanReqwdev) &&
			(pAd->ScanCtrl.ScanReqwdev->wdev_type == WDEV_TYPE_STA))
			return len;
	}

#endif

	if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
#ifdef APCLI_SUPPORT
			/*
				Append WSC information in probe request if WSC state is running
			*/
			if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				(wdev->WscControl.bWscTrigger))
				bHasWscIe = TRUE;

#if defined(WSC_V2_SUPPORT) && !defined(CONFIG_MAP_SUPPORT) && !defined(CON_WPS)
			/* need to check if !defined(CONFIG_MAP_SUPPORT) is necessary */
			else if (wdev->WscControl.WscV2Info.bEnableWpsV2)
				bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		if (bHasWscIe) {
			PWSC_CTRL pWscControl = NULL;

			/* Do not include wsc ie in case concurrent WPS is running */
			bHasWscIe = FALSE;
			pWscControl = &wdev->WscControl;

			if ((pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED) ||
			    (pAd->ApCfg.ConWpsApCliMode != CON_WPS_APCLI_BAND_AUTO))
				bHasWscIe = TRUE;

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
				"[scan_active] ConWpsApCliMode=%d conWscStatus=%d bHasWscIe=%d\n",
				pAd->ApCfg.ConWpsApCliMode,
				pWscControl->conWscStatus, bHasWscIe);
		}
#endif /*CON_WPS*/
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef CONFIG_STA_SUPPORT
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

		ASSERT(pStaCfg);
#ifdef WSC_STA_SUPPORT

		/*
			Append WSC information in probe request if WSC state is running
		*/
		if ((wdev->WscControl.WscEnProbeReqIE) &&
			(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
			(wdev->WscControl.bWscTrigger == TRUE))
			bHasWscIe = TRUE;

#ifdef WSC_V2_SUPPORT
		else if ((wdev->WscControl.WscEnProbeReqIE) &&
				 (wdev->WscControl.WscV2Info.bEnableWpsV2))
			bHasWscIe = TRUE;

#endif /* WSC_V2_SUPPORT */

#endif /* WSC_STA_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/
	}

	if (bHasWscIe) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildProbeReqIE(pAd, wdev, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
				"WscBuf Allocate failed!\n");
	}

#endif /* WSC_INCLUDED */
	return len;
}

static INT build_wsc_probe_rsp_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;

	switch (wdev->wdev_type) {
	case WDEV_TYPE_AP: {
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		if ((wdev->WscIEProbeResp.ValueLen)
#else
		/* for windows 7 logo test */
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE)
#endif
#ifdef DOT1X_SUPPORT
			&& (!IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
			&& (IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher))) {
			/*
				Non-WPS Windows XP and Vista PCs are unable to determine if a WEP enalbed network is static key based
				or 802.1X based. If the legacy station gets an EAP-Rquest/Identity from the AP, it assume the WEP
				network is 802.1X enabled & will prompt the user for 802.1X credentials. If the legacy station doesn't
				receive anything after sending an EAPOL-Start, it will assume the WEP network is static key based and
				prompt user for the WEP key. <<from "WPS and Static Key WEP Networks">>
				A WPS enabled AP should include this IE in the beacon when the AP is hosting a static WEP key network.
				The IE would be 7 bytes long with the Extended Capability field set to 0 (all bits zero)
				http://msdn.microsoft.com/library/default.asp?url=/library/en-us/randz/protocol/securing_public_wi-fi_hotspots.asp
			*/
			const UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

			MAKE_IE_TO_BUF(buf, PROVISION_SERVICE_IE, 7, len);
		}

		/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		if (wdev->WscIEProbeResp.ValueLen)
#else
		if ((wdev->WscControl.WscConfMode > WSC_DISABLE) && (wdev->WscIEProbeResp.ValueLen))
#endif
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#ifdef WSC_STA_SUPPORT

	case WDEV_TYPE_ADHOC: {
		/* PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd,wdev); */
		/* add Simple Config Information Element */
		if (wdev->WscIEProbeResp.ValueLen != 0)
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#endif /* WSC_STA_SUPPORT */
	}

	return len;
}

static INT build_wsc_assoc_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_STA_SUPPORT

	/* Add WSC IE if we are connecting to WSC AP */
	if ((wdev->WscControl.WscEnAssociateIE) &&
		(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger)) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(pAd, (UCHAR **) &pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocReqIE(&wdev->WscControl, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
				"WscBuf Allocate failed!\n");
	}

#endif /* WSC_STA_SUPPORT */
	return len;
}

INT build_wsc_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_wsc_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_PROBE_RSP)
		len += build_wsc_probe_rsp_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_ASSOC_REQ)
		len += build_wsc_assoc_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

/* rsp */
INT build_rsn_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	pSecConfig = &wdev->SecConfig;

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		BSS_STRUCT *mbss;
		CHAR rsne_idx = 0;

		mbss = wdev->func_dev;
#ifdef CONFIG_HOTSPOT_R2

		if ((mbss->HotSpotCtrl.HotSpotEnable == 0) &&
			(mbss->HotSpotCtrl.bASANEnable == 1) &&
			(IS_AKM_WPA2_Entry(wdev))) {
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
			extern UCHAR OSEN_IE[];
			extern UCHAR OSEN_IELEN;
			UCHAR RSNIe = IE_WPA;

			MAKE_IE_TO_BUF(buf, &RSNIe, 1, len);
			MAKE_IE_TO_BUF(buf, &OSEN_IELEN,  1, len);
			MAKE_IE_TO_BUF(buf, OSEN_IE, OSEN_IELEN, len);
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_EID[rsne_idx][0], 1, len);
				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_Len[rsne_idx], 1, len);
				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_Content[rsne_idx][0],
							   pSecConfig->RSNE_Len[rsne_idx], len);
			}
		}
		break;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	case WDEV_TYPE_ADHOC: {
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
		UCHAR RSNIe = IE_WPA;

		/* Modify by Eddy, support WPA2PSK in Adhoc mode */
		if (IS_AKM_WPANONE(wdev->SecConfig.AKMMap)) {
			MAKE_IE_TO_BUF(buf, &RSNIe, 1, len);
			MAKE_IE_TO_BUF(buf, &pStaCfg->RSNIE_Len, 1, len);
			MAKE_IE_TO_BUF(buf, pStaCfg->RSN_IE, pStaCfg->RSNIE_Len, len);
		}

		break;
	}

#endif /* CONFIG_STA_SUPPORT */
	}

	return len;
}

#ifdef CFG_RSNO_SUPPORT
INT build_rsn_override_ie(
	struct _RTMP_ADAPTER *mac_ad,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	UCHAR sub_type,
	UCHAR *ie_buf)
{
	INT len = 0;
	struct _SECURITY_CONFIG *sec_config = NULL;

	switch (sub_type) {
	case SUBTYPE_PROBE_RSP:
	case SUBTYPE_BEACON:
	case SUBTYPE_ASSOC_REQ:
		if (wdev)
			sec_config = &wdev->SecConfig;
		break;
	default:
		break;
	}

	if (sec_config) {
		if (sec_config->rsneo_len) {
			MAKE_IE_TO_BUF(
				ie_buf,
				sec_config->rsneo_content,
				sec_config->rsneo_len,
				len);
		}
		if (sec_config->rsnxeo_len) {
			MAKE_IE_TO_BUF(
				ie_buf,
				sec_config->rsnxeo_content,
				sec_config->rsnxeo_len,
				len);
		}
	}
	return len;
}
#endif /* CFG_RSNO_SUPPORT */

static INT build_extra_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (wdev->wdev_type == WDEV_TYPE_STA)
		ASSERT(pStaCfg);

#if defined(APCLI_CFG80211_SUPPORT)
	if (pStaCfg &&
		(pAd->cfg80211_ctrl.ExtraIeLen > 0)) {
		MAKE_IE_TO_BUF(buf, pAd->cfg80211_ctrl.pExtraIe,
					   pAd->cfg80211_ctrl.ExtraIeLen, len);
	}
#endif

#endif /* RT_CFG80211_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/

	if (ScanInfo->ExtraIeLen && ScanInfo->ExtraIe) {
		MAKE_IE_TO_BUF(buf, ScanInfo->ExtraIe,
					   ScanInfo->ExtraIeLen, len);
	}

	return len;
}

INT build_extra_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_extra_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

#ifdef CONFIG_AP_SUPPORT
/* Extended Capabilities IE */
INT build_ap_extended_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	ULONG infoPos;
	PUCHAR pInfo;
	BOOLEAN bNeedAppendExtIE = FALSE;
#ifdef BCN_EXTCAP_VAR_LEN
	UCHAR extInfoLen = wdev->BcnExtCapLen;
#else
	UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
#endif /* CN_EXTCAP_VAR_LEN */
	BSS_STRUCT *mbss;
	EXT_CAP_INFO_ELEMENT extCapInfo = { 0 };

	/* NdisZeroMemory(&extCapInfo, extInfoLen); */
	mbss = wdev->func_dev;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	/* P802.11n_D1.10, HT Information Exchange Support */
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
		WMODE_CAP_N(wdev->PhyMode) && (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) &&
		(wdev->DesiredHtPhyInfo.bHtEnable))
		extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)

	if (mbss->WNMCtrl.ProxyARPEnable)
		extCapInfo.proxy_arp = 1;
#endif
#ifdef CONFIG_DOT11V_WNM
	if (mbss->WNMCtrl.WNMBTMEnable)
		extCapInfo.BssTransitionManmt = 1;

#ifdef CONFIG_HOTSPOT_R2

	if (mbss->WNMCtrl.WNMNotifyEnable)
		extCapInfo.wnm_notification = 1;

	if (mbss->HotSpotCtrl.QosMapEnable && mbss->HotSpotCtrl.HotSpotEnable)
		extCapInfo.qosmap = 1;

#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

#ifdef QOS_R1
	if (IS_QOS_ENABLE(pAd)) {
		extCapInfo.qosmap = mbss->QosMapSupport ? 1 : 0;
		extCapInfo.dot11MSCSActivated = 1;
#ifdef QOS_R2
		extCapInfo.dot11SCSActivated = 1;
#endif
	}
#endif

#ifdef WAPP_SUPPORT
#ifdef CONFIG_DOT11U_INTERWORKING
	if (mbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */
#endif /* WAPP_SUPPORT */
#ifdef DOT11V_WNM_SUPPORT

	if (IS_BSS_TRANSIT_MANMT_SUPPORT(pAd, wdev->func_idx))
		extCapInfo.BssTransitionManmt = 1;

	if (IS_WNMDMS_SUPPORT(pAd, wdev->func_idx))
		extCapInfo.DMSSupport = 1;

#endif /* DOT11V_WNM_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		WMODE_CAP_5G(wdev->PhyMode))
		extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
#ifdef FTM_SUPPORT

	/* 802.11mc D3.0: 10.24.6.2 (p.1717):
		"A STA in which dot11FineTimingMsmtRespActivated is true shall set the Fine Timing Measurement
		Responder field of the Extended Capabilities element to 1."
	*/
	/* 8.4.2.26 Extended Capabilities element (p.817):
		Capabilities field= 70: Fine Timing Measurement Responder (p.823)
	*/
	if (wdev->FtmCtrl.FTMResp)
		extCapInfo.ftm_resp = 1;

	if (wdev->FtmCtrl.FTMInit)
		extCapInfo.ftm_init = 1;

#endif /* FTM_SUPPORT */

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS(wdev->SecConfig.AKMMap))
		extCapInfo.FILSCap = 1;
#endif /* OCE_FILS_SUPPORT */


#ifdef DOT11V_MBSSID_SUPPORT
	if (IS_MBSSID_IE_NEEDED(&mbss->mbss_11v))
		extCapInfo.mbssid = 1;
	else
		extCapInfo.mbssid = 0;

	if (IS_BSSID_11V_ENABLED(pAd))
		extCapInfo.cmpl_non_txbssid = 1;
	else
		extCapInfo.cmpl_non_txbssid = 0;
#endif


#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {
#ifdef WIFI_TWT_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_responder_support = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_requester_support = 1;
		}
#endif /* WIFI_TWT_SUPPORT */
    }
#endif /* DOT11_HE_AX */

#ifdef DOT11_SAE_SUPPORT
	{
		struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->pwd_id_cnt != 0)
			extCapInfo.sae_pwd_id_in_use = 1;
		else
			extCapInfo.sae_pwd_id_in_use = 0;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.pwd_id_only)
			extCapInfo.sae_pwd_id_used_exclusively = 1;
		else
			extCapInfo.sae_pwd_id_used_exclusively = 0;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.sae_pk_en)
			extCapInfo.sae_pk_pwd_used_exclusively = 1;
		else
			extCapInfo.sae_pk_pwd_used_exclusively = 0;
	}
#endif

#ifdef BCN_PROTECTION_SUPPORT
	{
		struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

		if (sec_cfg->bcn_prot_cfg.bcn_prot_en)
			extCapInfo.bcn_prot_en = 1;
		else
			extCapInfo.bcn_prot_en = 0;
	}
#endif

#ifdef CFG_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

	pInfo = (PUCHAR)(&extCapInfo);

	for (infoPos = 0; infoPos < extInfoLen; infoPos++) {
		if (pInfo[infoPos] != 0) {
			bNeedAppendExtIE = TRUE;
			break;
		}
	}

	if (bNeedAppendExtIE == TRUE) {
		for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--) {
			if (pInfo[infoPos] == 0)
				extInfoLen--;
			else
				break;
		}

		MAKE_IE_TO_BUF(buf, &ExtCapIe,   1, len);
		MAKE_IE_TO_BUF(buf, &extInfoLen, 1, len);
		MAKE_IE_TO_BUF(buf, &extCapInfo, extInfoLen, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
INT build_sta_extended_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#if defined(DOT11N_DRAFT3)
	{
		EXT_CAP_INFO_ELEMENT extCapInfo;
		UCHAR extInfoLen;
		ULONG infoPos;
		PUCHAR pInfo;
		BOOLEAN bNeedAppendExtIE = FALSE;

		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef DOT11N_DRAFT3

		if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
			WMODE_CAP_N(wdev->PhyMode)
			&& (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G))
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#ifdef MBO_SUPPORT
		if (IS_MBO_ENABLE(wdev))
			extCapInfo.BssTransitionManmt = 1;
#endif /* MBO_SUPPORT */
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(wdev->PhyMode) &&
			WMODE_CAP_5G(wdev->PhyMode))
			extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
#ifdef FTM_SUPPORT
		if (wdev->FtmCtrl.FTMResp)
			extCapInfo.ftm_resp = 1;

		if (wdev->FtmCtrl.FTMInit)
			extCapInfo.ftm_init = 1;
#endif /* FTM_SUPPORT */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {
#ifdef DOT11V_MBSSID_SUPPORT
		extCapInfo.mbssid = 1;
#endif /* DOT11V_MBSSID_SUPPORT */
#ifdef WIFI_TWT_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_responder_support = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_requester_support = 1;
		}
#endif /* WIFI_TWT_SUPPORT */
    }
#endif /* DOT11_HE_AX */

#ifdef DOT11_SAE_SUPPORT
		{
			struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

			if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->pwd_id_cnt != 0)
				extCapInfo.sae_pwd_id_in_use = 1;
			else
				extCapInfo.sae_pwd_id_in_use = 0;

			if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.pwd_id_only)
				extCapInfo.sae_pwd_id_used_exclusively = 1;
			else
				extCapInfo.sae_pwd_id_used_exclusively = 0;
		}
#endif

		pInfo = (PUCHAR)(&extCapInfo);

		for (infoPos = 0; infoPos < extInfoLen; infoPos++) {
			if (pInfo[infoPos] != 0) {
				bNeedAppendExtIE = TRUE;
				break;
			}
		}

		if (bNeedAppendExtIE == TRUE) {
			for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--) {
				if (pInfo[infoPos] == 0)
					extInfoLen--;
				else
					break;
			}
#ifdef CFG_BIG_ENDIAN
			RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

			MAKE_IE_TO_BUF(buf, &ExtCapIe,   1, len);
			MAKE_IE_TO_BUF(buf, &extInfoLen, 1, len);
			MAKE_IE_TO_BUF(buf, &extCapInfo, extInfoLen, len);
		}
	}
#endif /* defined(DOT11N_DRAFT3) */
	return len;
}
#endif /* CONFIG_STA_SUPPORT */

INT build_extended_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	switch (wdev->wdev_type) {
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_MESH:
		len += build_sta_extended_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		break;
#endif /* CONFIG_STA_SUPPORT */
	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_extended_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}

#ifdef CONFIG_AP_SUPPORT
static INT build_ap_wmm_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR i;
	UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
	UINT8 AIFSN[4];
	struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

	if (pBssEdca) {
		WmeParmIe[8] = pBssEdca->EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
		UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif /* UAPSD_SUPPORT */
		NdisMoveMemory(AIFSN, pBssEdca->Aifsn, sizeof(AIFSN));

		for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
			WmeParmIe[10 + (i * 4)] = (i << 5)							  +	/* b5-6 is ACI */
									  ((UCHAR)pBssEdca->bACM[i] << 4)	  +	/* b4 is ACM */
									  (AIFSN[i] & 0x0f);						/* b0-3 is AIFSN */
			WmeParmIe[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4)		  +	/* b5-8 is CWMAX */
									  (pBssEdca->Cwmin[i] & 0x0f);				/* b0-3 is CWMIN */
			WmeParmIe[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);		/* low byte of TXOP */
			WmeParmIe[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);			/* high byte of TXOP */
		}

		MAKE_IE_TO_BUF(buf, WmeParmIe, 26, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
static INT build_sta_wmm_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (pStaCfg->MlmeAux.APEdcaParm.bValid) {
		if (pStaCfg->wdev.UapsdInfo.bAPSDCapable
			&& pStaCfg->MlmeAux.APEdcaParm.bAPSDCapable) {
			QBSS_STA_INFO_PARM QosInfo;

			NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
			QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
			QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
			QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
			QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
			QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
			WmeIe[8] |= *(PUCHAR)&QosInfo;
		} else {
			/* The Parameter Set Count is set to 0 in the association request frames */
			/* WmeIe[8] |= (pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
		}

		MAKE_IE_TO_BUF(buf, WmeIe, 9, len);
	}

	return len;
}
#endif /* CONFIG_STA_SUPPORT */

INT build_wmm_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	if (!wdev->bWmmCapable)
		return len;

	switch (wdev->wdev_type) {
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_MESH:
	case WDEV_TYPE_REPEATER:
		len += build_sta_wmm_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		break;
#endif /* CONFIG_STA_SUPPORT */

	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_wmm_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}


#ifdef MBO_SUPPORT
INT build_supp_op_class_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	ULONG TmpLen;
	UCHAR OperatingIe = IE_SUPP_REG_CLASS;
	UCHAR OperClassList = 0;
	UCHAR SuppClasslist[50];
	INT len = 0;
	UCHAR RegulatoryClass = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);

	OperClassList = get_operating_class_list(pAd, wdev->channel, wdev->PhyMode, wdev, SuppClasslist, &len);
	SuppClasslist[len] = 130;
	len += 2; /* to include Regulatory Class and Delimiter 0x82 */
	if (OperClassList == TRUE) {
		MakeOutgoingFrame(buf,    &TmpLen,
				1, &OperatingIe,
				1, &len,
				1, &RegulatoryClass,
				len-1, &SuppClasslist,
				END_OF_ARGS);
	}
	return TmpLen;
}
#endif /* MBO_SUPPORT */


ULONG build_support_rate_ie(struct wifi_dev *wdev, UCHAR *sup_rate, UCHAR sup_rate_len, UCHAR *buf)
{
	ULONG frame_len = 0;
	ULONG bss_mem_selector_len = 0;
	UCHAR bss_mem_selector_code = BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR real_sup_rate_len = sup_rate_len;
	UCHAR total_len;

	if (sup_rate_len == 0)
		return frame_len;

	if (PhyMode == WMODE_B)
		real_sup_rate_len = 4;

#ifdef DOT11_SAE_SUPPORT
	if (wdev->SecConfig.sae_cap.gen_pwe_method == PWE_HASH_ONLY && real_sup_rate_len < 8)
		bss_mem_selector_len++;
#endif
	total_len = real_sup_rate_len + bss_mem_selector_len;

	MakeOutgoingFrame(buf, &frame_len, 1, &SupRateIe,
				1, &total_len,
				real_sup_rate_len, sup_rate,
				bss_mem_selector_len, &bss_mem_selector_code,
				END_OF_ARGS);

	return frame_len;
}


ULONG build_support_ext_rate_ie(struct wifi_dev *wdev, UCHAR sup_rate_len,
	UCHAR *ext_sup_rate, UCHAR ext_sup_rate_len, UCHAR *buf)
{
	ULONG frame_len = 0;
	ULONG bss_mem_selector_len = 0;
	UCHAR bss_mem_selector_code = BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR total_len;

	if (PhyMode == WMODE_B)
		return frame_len;

#ifdef DOT11_SAE_SUPPORT
	if (sup_rate_len >= 8 && wdev->SecConfig.sae_cap.gen_pwe_method == PWE_HASH_ONLY)
		bss_mem_selector_len++;
#endif
	if (ext_sup_rate_len == 0 && bss_mem_selector_len == 0)
		return frame_len;

	total_len = ext_sup_rate_len + bss_mem_selector_len;

	MakeOutgoingFrame(buf, &frame_len, 1, &ExtRateIe,
				1, &total_len,
				ext_sup_rate_len, ext_sup_rate,
				bss_mem_selector_len, &bss_mem_selector_code,
				END_OF_ARGS);

	return frame_len;
}

/* 9.4.2.79 BSS Max Idle Period element */
ULONG build_bss_max_idle_ie(
	struct wifi_dev *wdev,
	UCHAR *buf,
	UINT16 max_idle_period,
	UINT8 option)
{
	UCHAR ie = IE_BSS_MAX_IDLE;
	UINT8 idle_opt = 0;
	ULONG total_len = 0;
	INT ie_len = 3;

	if (option)
		idle_opt |= 0x1 << 0;	/* B0: Protected Keep-Alive Required */

	MakeOutgoingFrame(buf, &total_len,
			1, &ie,
			1, &ie_len,
			2, &max_idle_period,
			1, &idle_opt,
			END_OF_ARGS);

	return total_len;
}

INT parse_ssid_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len <= MAX_LEN_OF_SSID;
}

INT parse_ds_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_cf_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 6;
}

INT parse_ibss_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 2;
}

INT parse_qbss_load_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_tpc_report_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 2;
}

INT parse_ch_switch_announcement_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 3;
}

INT parse_ext_ch_switch_announcement_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 4;
}

INT parse_measurement_ie(UCHAR Len)
{
	return Len >= 3 && Len < 255;
}

INT parse_erp_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_overlapbss_scan_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 14;
}

INT parse_rm_enable_cap_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_wapi_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 4;
}

INT parse_ht_info_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == SIZE_ADD_HT_INFO_IE;
}

INT parse_sec_ch_offset_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_ft_timeout_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_ft_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= 82;
}

INT parse_md_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 3;
}

INT parse_country_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= 3;
}

INT parse_rsn_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= MIN_LEN_OF_RSNIE;
}

INT parse_qos_cap_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_ht_cap_ie(UCHAR Len)
{
	return Len >= SIZE_HT_CAP_IE;
}

INT parse_support_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr)
{
	UCHAR i = 0;

	if (rate == NULL) {
		if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0))
			return TRUE;
		else
			return FALSE;
	}

	if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0)) {
		rate->sup_rate_len = 0;
		for (i = 0; i < eid_ptr->Len; i++)
			if (eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HT_PHY) &&
			    eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_VHT_PHY) &&
			    eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY) &&
			    eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HE_PHY))
				rate->sup_rate[rate->sup_rate_len++] = eid_ptr->Octet[i];
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_DEBUG,
				"IE_SUPP_RATES., Len=%d. Rates[0]=%x\n",
				eid_ptr->Len, rate->sup_rate[0]);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_DEBUG,
				"Rates[1]=%x %x %x %x %x %x %x\n",
				rate->sup_rate[1], rate->sup_rate[2],
				rate->sup_rate[3], rate->sup_rate[4],
				rate->sup_rate[5], rate->sup_rate[6],
				rate->sup_rate[7]);
	}
	return TRUE;
}

INT parse_support_ext_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr)
{
	UINT16 i = 0;

	if (rate == NULL && eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES)
		return TRUE;

	if (eid_ptr->Len > MAX_LEN_OF_SUPPORTED_RATES) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_ERROR,
			"ext support rate ie size(%d) is large than MAX_LEN_OF_SUPPORTED_RATE(%d))\n",
			eid_ptr->Len, MAX_LEN_OF_SUPPORTED_RATES);
		return FALSE;
	}
	if (!rate->sup_rate_len && !eid_ptr->Len) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_ERROR,
			"Ext support rate ie size is (%d) and Supported rate Len (%d))\n",
			eid_ptr->Len, rate->sup_rate_len);
		return FALSE;
	}

	rate->ext_rate_len = 0;
	for (i = 0; i < eid_ptr->Len; i++)
		if (eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HT_PHY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_VHT_PHY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HE_PHY))
			rate->ext_rate[rate->ext_rate_len++] = eid_ptr->Octet[i];
	return TRUE;
}
