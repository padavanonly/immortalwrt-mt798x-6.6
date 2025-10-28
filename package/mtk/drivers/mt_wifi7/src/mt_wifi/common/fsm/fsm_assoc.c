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
	assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John		2004-9-3		porting from RT2500
*/
#include "rt_config.h"

#ifdef DOT11R_FT_SUPPORT
#include "ft/ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef GREENAP_SUPPORT
BOOLEAN greenap_get_allow_status(RTMP_ADAPTER *ad);
#endif
#endif

#if defined(DOT11_EHT_BE) && defined(RT_CFG80211_SUPPORT)
extern struct bss_manager bss_mngr;

#ifdef DOT11R_FT_SUPPORT
int mlo_ft_set_entry(IN struct _MAC_TABLE_ENTRY *entry)
{
	struct _MAC_TABLE_ENTRY *entry_ptr;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];
	u8 i = 0, link_index = 0;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
		"(%s)[%d]: ERROR, mld_entry=NULL\n", __func__, __LINE__);
		mt_rcu_read_unlock();
		return -1;
	}
	for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
		tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];
	mt_rcu_read_unlock();
	do {
		entry_ptr = tmp_entry_ptr[i++];
		if (!entry_ptr)
			continue;
		entry_ptr->MdIeInfo.Len = entry->MdIeInfo.Len;
		if (!IS_FT_STA(entry_ptr))
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
				"%s: %02x:%02x:%02x:%02x:%02x:%02x is not a FT STA\n",
				__func__, PRINT_MAC(entry_ptr->Addr));
		else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
				"%s: %02x:%02x:%02x:%02x:%02x:%02x is a FT STA\n",
				__func__, PRINT_MAC(entry_ptr->Addr));
	} while (i < MLD_LINK_MAX);
	return 0;
}
#endif /* DOT11R_FT_SUPPORT */

int mtk_cfg80211_event_bss_ml_sta_profile_info(
	struct _RTMP_ADAPTER *pAd, uint8_t *data, uint32_t len)
{
	struct sk_buff *skb;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;

	pCfg80211_CB = pAd->pCfg80211_CB;
	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
			len, MTK_NL80211_VENDOR_EVENT_STA_PROFILE_EVENT, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR, "allocate skb failed\n");
		return -ENOMEM;
	}

	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -1;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO, "send vendor response success.\n");
	return 0;
}

static struct bmgr_entry *get_bss_entry_by_netdev(
	IN PNET_DEV pNetDev)
{
	struct bmgr_entry *entry = NULL;
	int i;

	for (i = 0; BMGR_VALID_BSS_IDX(i); i++) {
		entry = bss_mngr.entry[i];
		if (entry && (entry->pNetDev == pNetDev))
			return entry;
	}

	return NULL;
}

static u8 *get_ap_per_sta_profile_info(struct bmgr_entry *entry, u8 *f_buf)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	u8 *pos = f_buf;
	u8 ie_len = 0;
	u16 sta_ctrl = 0;
	u16 sta_ctrl_pres = 0;
#ifdef DOT11_BE_D12
	u8 *pos_info_len;
#endif

	/* Per-STA Profile subelement */
	eid->Eid = SUB_IE_MLD_PER_STA_PROFILE;
	pos += 2;

	/* Per-STA Control field */
	SET_DOT11BE_ML_PER_STA_CTRL_LINK_ID(sta_ctrl, entry->link_id);
	SET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl, TRUE);

	/* Per-STA Control field - presence */
	if (GET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl)) {
		SET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl_pres, TRUE);
		SET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl_pres, TRUE);
		SET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl_pres, TRUE);
	} else {
		SET_DOT11BE_ML_PER_STA_CTRL_MAC_ADDR_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_BCN_INT_PRES(sta_ctrl_pres, FALSE);
		SET_DOT11BE_ML_PER_STA_CTRL_DTIM_PRES(sta_ctrl_pres, FALSE);
	}

	/* meaningful if Maximum Number Of Simultaneous Links greater than 0 */
	SET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(sta_ctrl_pres, TRUE);
	/*
	 * meaningful if NSTR Link Pair Present is 1
	 *   Bitmap subfield: 0 = 1 octet, 1 = 2 octets
	 */
	SET_DOT11BE_ML_PER_STA_CTRL_NSTR_BMAP_SIZE(sta_ctrl_pres, 0);

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
				 "\t    link_addr(%pM), %02x:%02x:%02x:%02x:%02x:%02x\n",
				 entry->entry_info.bssid, PRINT_MAC(entry->entry_info.bssid));
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

	/* NSTR Link Pair Present */
	if (GET_DOT11BE_ML_PER_STA_CTRL_COMP_PROF(sta_ctrl) &&
		GET_DOT11BE_ML_PER_STA_CTRL_NSTR_LPR_PRES(sta_ctrl_pres)) {
			u8 nstr_bmap0 = 0;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					 "\t	NSTR en, bmap0(0x%x)\n", nstr_bmap0);
			NdisMoveMemory(pos, &nstr_bmap0, sizeof(nstr_bmap0));
			pos += sizeof(nstr_bmap0);
	}

#ifdef DOT11_BE_D12
	/* STA Info Length */
	*pos_info_len = (pos - pos_info_len);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			 "\tsta_info_len(%d)\n", *pos_info_len);
#endif

	/* eid_length */
	ie_len = pos - (u8 *)eid - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}
#endif

const CHAR *ASSOC_FSM_STATE_STR[MAX_ASSOC_STATE] = {
	"ASSOC_IDLE",
	"ASSOC_WAIT_RSP",
	"REASSOC_WAIT_RSP",
	"DISASSOC_WAIT_RSP"
};

const CHAR *ASSOC_FSM_MSG_STR[MAX_ASSOC_MSG] = {
	"MLME_ASSOC_REQ",
	"MLME_REASSOC_REQ",
	"MLME_DISASSOC_REQ",
	"PEER_DISASSOC_REQ",
	"PEER_ASSOC_REQ",
	"PEER_ASSOC_RSP",
	"PEER_REASSOC_REQ",
	"PEER_REASSOC_RSP",
	"DISASSOC_TIMEOUT",
	"ASSOC_TIMEOUT",
	"REASSOC_TIMEOUT"
};

/*
	==========================================================================
	Description:
		mlme assoc req handling procedure
	Parameters:
		Adapter - Adapter pointer
		Elem - MLME Queue Element
	Pre:
		the station has been authenticated and the following information is stored in the config
			-# SSID
			-# supported rates and their length
			-# listen interval (Adapter->StaCfg[0].default_listen_count)
			-# Transmit power  (Adapter->StaCfg[0].tx_power)
	Post  :
		-# An association request frame is generated and sent to the air
		-# Association timer starts
		-# Association state -> ASSOC_WAIT_RSP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_assoc_req_action)
			assoc_api->mlme_assoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_assoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_assoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "assoc_req", DBG_LVL_INFO, &tl);
}

/*
	==========================================================================
	Description:
		mlme reassoc req handling procedure
	Parameters:
		Elem -
	Pre:
		-# SSID  (Adapter->StaCfg[0].ssid[])
		-# BSSID (AP address, Adapter->StaCfg[0].bssid)
		-# Supported rates (Adapter->StaCfg[0].supported_rates[])
		-# Supported rates length (Adapter->StaCfg[0].supported_rates_len)
		-# Tx power (Adapter->StaCfg[0].tx_power)

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_reassoc_req_action)
			assoc_api->mlme_reassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_reassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_reassoc_req_action ? "HOOKED" : "NULL");
	}

	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		Upper layer issues disassoc request
	Parameters:
		Elem -

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_disassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_disassoc_req_action)
			assoc_api->mlme_disassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_disassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_disassoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		peer sends assoc rsp back
	Parameters:
		Elme - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_assoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_assoc_rsp_action)
			assoc_api->peer_assoc_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_assoc_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_assoc_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_assoc_rsp", DBG_LVL_INFO, &tl);
}


/*
	==========================================================================
	Description:
		peer sends reassoc rsp
	Parametrs:
		Elem - MLME message cntaining the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_reassoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_reassoc_rsp_action)
			assoc_api->peer_reassoc_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_reassoc_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_reassoc_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}


/*
	==========================================================================
	Description:
		left part of IEEE 802.11/1999 p.374
	Parameters:
		Elem - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_disassoc_action)
			assoc_api->peer_disassoc_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_disassoc_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_disassoc_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after assoc timeout
	Parameters:
		Elme -

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_assoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_assoc_req_timeout_action)
			assoc_api->mlme_assoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_assoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_assoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after reassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_reassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_reassoc_req_timeout_action)
			assoc_api->mlme_reassoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_reassoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_reassoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after disassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_disassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_disassoc_req_timeout_action)
			assoc_api->mlme_disassoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_disassoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_disassoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID assoc_fsm_peer_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_assoc_req_action)
			assoc_api->peer_assoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_assoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_assoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_assoc_req", DBG_LVL_INFO, &tl);
}

static VOID assoc_fsm_peer_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_reassoc_req_action)
			assoc_api->peer_reassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_reassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_reassoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_reassoc_req", DBG_LVL_INFO, &tl);
}

#ifdef RT_CFG80211_SUPPORT
static BOOLEAN assoc_fsm_go_to_driver_handle(
	struct _RTMP_ADAPTER *mac_ad,
	struct _MLME_QUEUE_ELEM *elem,
	struct _IE_lists *ie_list)
{
	struct wifi_dev *wdev = elem->wdev;
#ifdef CFG_RSNO_SUPPORT
	struct _MAC_TABLE_ENTRY *entry;
	unsigned short wcid = (unsigned short)elem->Priv;
#endif /* CFG_RSNO_SUPPORT */

	if (!wdev)
		return FALSE;

	if (IS_AKM_OWE(wdev->SecConfig.AKMMap) || IS_AKM_DPP_ONLY(wdev->SecConfig.AKMMap))
		return FALSE;

#ifdef CFG_RSNO_SUPPORT
	/* Need to check entry security first when ap has rsn override setting. */
	if (VALID_UCAST_ENTRY_WCID(mac_ad, wcid)) {
		entry = entry_get(mac_ad, wcid);
		sec_update_entry_rsn_ie(entry, ie_list);
		if (WPAValidateRSNIE(wdev, &entry->SecConfig, &entry->RSN_IE[0], entry->RSNIE_Len)
			== MLME_SUCCESS) {
			if ((!IS_AKM_OWE(entry->SecConfig.AKMMap))
				&& (!IS_AKM_DPP(entry->SecConfig.AKMMap))) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_RSNO, DBG_LVL_INFO,
				 "Not owe or dpp, forward to driver handle function.\n");
				assoc_fsm_peer_assoc_req_action(mac_ad, elem);
				return TRUE;
			}
		}
	}
#endif /* CFG_RSNO_SUPPORT */
	return FALSE;
}

static VOID assoc_fsm_hostapd_assoc_req_handler(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocReqHdr;
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate = 0;
	UCHAR PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
/*	UCHAR ucETxBfCap; */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UCHAR isReassoc = 0;
	BOOLEAN bACLReject = FALSE;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
#ifdef CFG_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*CFG_BIG_ENDIAN*/
	ADD_HT_INFO_IE *addht;
#ifdef GREENAP_SUPPORT
/*    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap; */
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	BOOLEAN bMboReject = FALSE;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */
	unsigned char *FC = NULL;
	unsigned short DataSize = 0;

	if (!Elem) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "Elem is null.\n");
		return;
	}

	FC = (unsigned char *)Elem->Msg;
	DataSize = Elem->MsgLen;

	if (!FC) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "FC is null.\n");
		return;
	}

	NdisZeroMemory(&AssocReqHdr, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *)&(AssocReqHdr.FC), FC, sizeof(FRAME_CONTROL));

	/* its reassoc frame */
	if (AssocReqHdr.FC.SubType == SUBTYPE_REASSOC_REQ)
		isReassoc = 1;

	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "Disallow new Association\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	/* fill ie_list */
	if (!PeerAssocReqCmmSanity(pAd, isReassoc, (VOID *)FC, DataSize, ie_list))
		goto LabelOK;

	if (assoc_fsm_go_to_driver_handle(pAd, Elem, ie_list) == TRUE)
		goto LabelOK;

#ifdef DOT11_EHT_BE
	if (!PeerAssocReqMloCapSanity(pAd, isReassoc, (VOID *)FC, DataSize, ie_list, TRUE))
		goto LabelOK;

	/* build multi-link connect request */
	if (HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists)) {
		wdev = wdev_search_by_address(pAd, ie_list->Addr1);

		if (wdev) {
			uint16_t mld_sta_idx;

			mld_sta_idx = eht_build_multi_link_conn_req(wdev, SUBTYPE_ASSOC_REQ,
				ie_list->cmm_ies.ml_ie, &ie_list->cmm_ies.ml_frag_info, ie_list->cmm_ies.t2l_ie, ie_list->Addr2, NULL, 0);

			if (mld_sta_idx == MLD_STA_NONE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
					RtmpOsGetNetDevName(wdev->if_dev), ie_list->Addr2);
				goto LabelOK;
			}
		}
	}
#endif /* DOT11_EHT_BE */

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "NoAuth MAC - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));
		goto LabelOK;
	}
	/* record owe Elem to pEntry, and use it in build sta profile */
	NdisMoveMemory(&pEntry->Elem_for_hostapd_owe, Elem, sizeof(MLME_QUEUE_ELEM));

	/* attach ie_list to pEntry */
	pEntry->ie_list = ie_list;

	if (pEntry->ie_list && pEntry->ie_list->ecdh_ie.length) {
		UINT32 freq = 0;
		PNET_DEV pNetDev;
		struct wifi_dev *pWdev = wdev_search_by_address(pAd, ie_list->Addr1);

		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;
		if (pWdev) {
			pNetDev = pWdev->if_dev;
		} else {
			pNetDev = CFG80211_GetEventDevice(pAd);
		}
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
			"ASSOC Req sending to CFG layer for OWE mode\n");
		CFG80211OS_RxMgmt(pNetDev, freq,
			FC, DataSize);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_WARN,
			"ASSOC Req OWE ie not found.\n");
		if (pEntry->ie_list != NULL)
			goto LabelOK;
	}

	if (pEntry->func_tb_idx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "pEntry bounding invalid wdev(apidx=%d)\n",
				  pEntry->func_tb_idx);
		goto LabelOK;
	}

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			 "%s():pEntry->func_tb_idx=%d\n",
			  __func__, pEntry->func_tb_idx);
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if ((wdev == NULL) || (wdev->func_idx < 0)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "Wrong Addr1 - "MACSTR"\n",
				  MAC2STR(ie_list->Addr1));
		goto LabelOK;
	}

	/* WPS_BandSteering Support */
#if defined(BAND_STEERING) && defined(WSC_INCLUDED)
	if (pAd->ApCfg.BandSteering) {
		PWSC_CTRL pWscControl = NULL;
		PBND_STRG_CLI_ENTRY cli_entry = NULL;
		PBND_STRG_CLI_TABLE table = NULL;
		PWPS_WHITELIST_ENTRY wps_entry = NULL;
		PBS_LIST_ENTRY bs_whitelist_entry = NULL;

		pWscControl = &pAd->ApCfg.MBSSID[wdev->func_idx].wdev.WscControl;
		table = &pAd->ApCfg.BndStrgTable;
		if (table->bEnabled) {

			cli_entry = BndStrg_TableLookup(table, pEntry->Addr);
			wps_entry = FindWpsWhiteListEntry(&table->WpsWhiteList, pEntry->Addr);

			/* WPS: special WIN7 case: no wps/rsn ie in assoc */
			/* and send eapol start, consider it as wps station */
			if ((ie_list->RSNIE_Len == 0) && (IS_AKM_WPA_CAPABILITY_Entry(wdev))
				&& (pWscControl->WscConfMode != WSC_DISABLE))
				ie_list->bWscCapable = TRUE;

			/* in case probe did not have wps ie, */
			/* but assoc has, create wps whitelist entry here */
			if (!wps_entry && ie_list->bWscCapable && pWscControl->bWscTrigger) {

				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				AddWpsWhiteList(&table->WpsWhiteList, pEntry->Addr);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					"channel %u: WPS Assoc req: STA "MACSTR" wps whitelisted\n",
					table->Channel, MAC2STR(pEntry->Addr));
				BND_STRG_PRINTQAMSG(table, pEntry->Addr,
				"ASSOC STA "MACSTR" channel %u  added in WPS Whitelist\n",
				MAC2STR(pEntry->Addr), table->Channel);
			}

			bs_whitelist_entry = FindBsListEntry(&table->WhiteList, pEntry->Addr);

			/* handle case: where a client has wps ie in probe, not have bndstrg entry/bndstrg whitelist, */
			/* but doing normal assoc: dont allow */
			if ((pWscControl->bWscTrigger) && (!cli_entry) && (!ie_list->bWscCapable) && (!bs_whitelist_entry)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"\nReject assoc:bWscTrigger:%d,entry:%p,bWscCapable:%d,bs_whitelist_entry:%p\n",
				pWscControl->bWscTrigger, cli_entry,
				ie_list->bWscCapable, bs_whitelist_entry);
				BND_STRG_PRINTQAMSG(table, pEntry->Addr,
				"STA "MACSTR" Normal Assoc Rejected for BS unauthorized client\n",
				MAC2STR(pEntry->Addr));
				goto LabelOK;
			}
		}
	}
#endif
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "AP is not ready, disallow new Association\n");
		goto LabelOK;
	}

		if (pAd->FragFrame.wcid == pEntry->wcid) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATMLME_ASSOC, DBG_LVL_WARN,
				"%s: Clear Wcid = %d FragBuffer !!!!!\n", __func__, pEntry->wcid);
			RESET_FRAGFRAME(pAd->FragFrame);
		}

		FlgIs11bSta = 1;

		for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
			if (((ie_list->rate.sup_rate[i] & 0x7F) != 2) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 4) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 11) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 22)) {
				FlgIs11bSta = 0;
				break;
			}
		}
#ifdef CONFIG_MAP_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
				"%s():IS_MAP_ENABLE=%d\n", __func__, IS_MAP_ENABLE(pAd));
	if (IS_MAP_ENABLE(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			"Elem->MsgLen=%d, ASSOC_REQ_LEN = %d\n",
			DataSize, (INT)ASSOC_REQ_LEN);
		NdisMoveMemory(pEntry->assoc_req_frame, (VOID *)FC, ASSOC_REQ_LEN);
	}
#endif

#ifdef GN_MIXMODE_SUPPORT
	pEntry->FlgIs11bSta = FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef MBO_SUPPORT
	if (!MBO_AP_ALLOW_ASSOC(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_WARN,
				"%s():MBO not allowed assoc\n", __func__);
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		bMboReject = TRUE;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
		/* goto SendAssocResponse; */
		return;
	}
#endif /* MBO_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
	)
#ifdef DOT11W_PMF_SUPPORT
	&& (!((AssocReqHdr.FC.Wep == 0) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)))
#endif /* DOT11W_PMF_SUPPORT */
	) {
		struct _ASIC_SEC_INFO *info = NULL;

		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */
		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
		if (info == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Fail to allocate memory!\n");
			return;
		} else {
			os_zero_mem(info, sizeof(ASIC_SEC_INFO));
			info->Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
			info->Wcid = pEntry->wcid;
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, info);
			os_free_mem(info);
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

			/* Notify 802.1x daemon to clear this sta info */
			if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
				|| IS_IEEE8021X(&pEntry->SecConfig))
				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		}
	}

#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0) &&
			(IS_AKM_WPA_CAPABILITY_Entry(wdev)) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}

#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0))
			goto LabelOK;
#ifdef WSC_V2_SUPPORT
			/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
			if ((pEntry->bWscCapable) &&
				(wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				(wsc_ctrl->WscV2Info.bEnableWpsV2) &&
				(wsc_ctrl->WscV2Info.bWpsEnable) &&
				(MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)))
				;
			else
#endif /* WSC_V2_SUPPORT */
				/* set a flag for sending Assoc-Fail response to unwanted STA later. */
				if (!ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->func_tb_idx))
					bACLReject = TRUE;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "assoc - MBSS(%d), receive reassoc request from "MACSTR"\n",
				  pEntry->func_tb_idx, MAC2STR(ie_list->Addr2));
		/* supported rates array may not be sorted. sort it and find the maximum rate */
		for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
			if (MaxSupportedRate < (ie_list->rate.sup_rate[i] & 0x7f))
				MaxSupportedRate = ie_list->rate.sup_rate[i] & 0x7f;
		}
		/*
		 *	Assign RateLen here or we will select wrong rate table in
		 *	APBuildAssociation() when 11N compile option is disabled.
		*/
		pEntry->RateLen = ie_list->rate.sup_rate_len;
		pEntry->MaxSupportedRate = MaxSupportedRate;
		RTMPSetSupportMCS(pAd,
						  OPMODE_AP,
						  pEntry,
						  &ie_list->rate,
#ifdef DOT11_VHT_AC
						  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
						  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
						  &ie_list->cmm_ies.ht_cap,
						  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));
#ifdef GN_MIXMODE_SUPPORT
		if (pAd->CommonCfg.GNMixMode
			&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
				|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
				|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
			pEntry->SupportRateMode &= ~SUPPORT_CCK_MODE;
			pEntry->SupportCCKMCS &= ~(1 << MCS_0 | 1 << MCS_1 | 1 << MCS_2 | 1 << MCS_3);
		}
#endif /* GN_MIXMODE_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE, "%s <<<<<<\n", __func__);
	return;

LabelOK:
#ifdef RT_CFG80211_SUPPORT
	if (StatusCode != MLME_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "%s StaDel event\n", __func__);
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);
	}
#endif /* RT_CFG80211_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL) {
		os_free_mem(ie_list);
		if (pEntry)
			pEntry->ie_list = NULL;
	}

	return;
}

static VOID assoc_fsm_hostapd_reassoc_req_handler(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocReqHdr;
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate = 0;
	UCHAR PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
/*	UCHAR ucETxBfCap; */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UCHAR isReassoc = 0;
	BOOLEAN bACLReject = FALSE;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
#ifdef CFG_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*CFG_BIG_ENDIAN*/
	ADD_HT_INFO_IE *addht;
#ifdef GREENAP_SUPPORT
/*	  struct greenap_ctrl *greenap = &pAd->ApCfg.greenap; */
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	BOOLEAN bMboReject = FALSE;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */
	unsigned char *FC = NULL;
	unsigned short DataSize = 0;

	if (!Elem) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "Elem is null - %s\n", __func__);
		return;
	}

	FC = (unsigned char *)Elem->Msg;
	DataSize = Elem->MsgLen;

	if (!FC) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE, "FC is null - %s\n", __func__);
		return;
	}

	NdisZeroMemory(&AssocReqHdr, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *)&(AssocReqHdr.FC), FC, sizeof(FRAME_CONTROL));

	/* its reassoc frame */
	if (AssocReqHdr.FC.SubType == SUBTYPE_REASSOC_REQ)
		isReassoc = 1;

	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "Disallow new Association\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	/* fill ie_list */
	if (!PeerAssocReqCmmSanity(pAd, isReassoc, (VOID *)FC, DataSize, ie_list))
		goto LabelOK;

	if (assoc_fsm_go_to_driver_handle(pAd, Elem, ie_list) == TRUE)
		goto LabelOK;

#ifdef DOT11_EHT_BE
	if (!PeerAssocReqMloCapSanity(pAd, isReassoc, (VOID *)FC, DataSize, ie_list, TRUE))
		goto LabelOK;

	/* build multi-link connect request */
	if (HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists)) {
		wdev = wdev_search_by_address(pAd, ie_list->Addr1);

		if (wdev) {
			uint16_t mld_sta_idx;

			mld_sta_idx = eht_build_multi_link_conn_req(wdev, SUBTYPE_ASSOC_REQ,
				ie_list->cmm_ies.ml_ie, &ie_list->cmm_ies.ml_frag_info, ie_list->cmm_ies.t2l_ie, ie_list->Addr2, NULL, 0);

			if (mld_sta_idx == MLD_STA_NONE) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
					RtmpOsGetNetDevName(wdev->if_dev), ie_list->Addr2);
				goto LabelOK;
			}
		}
	}
#endif /* DOT11_EHT_BE */

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "NoAuth MAC - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));
		goto LabelOK;
	}

	/* record owe Elem to pEntry, and use it in build sta profile */
	NdisMoveMemory(&pEntry->Elem_for_hostapd_owe, Elem, sizeof(MLME_QUEUE_ELEM));

	/* attach ie_list to pEntry */
	pEntry->ie_list = ie_list;

#ifdef RT_CFG80211_SUPPORT
	if (1) {
		UINT32 freq = 0;
		PNET_DEV pNetDev;
		struct wifi_dev *pWdev = wdev_search_by_address(pAd, ie_list->Addr1);
#if defined(DOT11_EHT_BE)
		u8 link_id;
		uint8_t *pos = NULL, *start = NULL;
		uint8_t len = 0;
		struct bmgr_entry *bmgr_entry = NULL, *mld_entry = NULL;
		struct bmgr_mlo_dev *mld = NULL;
#endif

		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;
		if (pWdev) {
			pNetDev = pWdev->if_dev;
		} else {
			pNetDev = CFG80211_GetEventDevice(pAd);
		}

	if (0
#ifdef HOSTAPD_11R_SUPPORT
		|| (pEntry->ie_list && pEntry->ie_list->FtInfo.MdIeInfo.Len)
#endif
		) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				"calling CFG80211OS_RxMgmt:FT reassoc req frame\n");

#ifdef DOT11R_FT_SUPPORT
			pEntry->MdIeInfo.Len = pEntry->ie_list->FtInfo.MdIeInfo.Len;
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
				"pEntry->MdIeInfo.Len = %d\n", pEntry->MdIeInfo.Len);
			if (!IS_FT_STA(pEntry))
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
					"%02x:%02x:%02x:%02x:%02x:%02x is not a FT STA\n",
					PRINT_MAC(pEntry->Addr));
			else
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
					"%02x:%02x:%02x:%02x:%02x:%02x is a FT STA\n",
					PRINT_MAC(pEntry->Addr));
#endif /* DOT11R_FT_SUPPORT */
			CFG80211OS_RxMgmt(pNetDev, freq, FC, DataSize);

#if defined(DOT11_EHT_BE) && defined(RT_CFG80211_SUPPORT)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
				"query per-sta profile info\n");
			os_alloc_mem(NULL, (UCHAR **)&pos, MAX_PER_STA_PROFILE_LEN);
#ifdef DOT11R_FT_SUPPORT
			if (pEntry && pEntry->mlo.mlo_en) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_NOTICE,
					"set mlo ft entry\n");
				mlo_ft_set_entry(pEntry);
			}
#endif /* DOT11R_FT_SUPPORT */
		if (pos == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
				"allocate pos memory fail\n");
			return;
		}

		start = pos;
		bmgr_entry = get_bss_entry_by_netdev(pNetDev);

		/* query per-sta profile info*/
		if (!BMGR_VALID_MLO_BSS_ENTRY(bmgr_entry)) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
				"bmgr_entry invalid\n");
			os_free_mem(pos);
			return;
		}

		mld = bmgr_entry->mld_ptr;
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			u8 bss_idx_mld = BMGR_VALID_MLO_DEV(mld) ?
				mld->bss_idx_mld[link_id] : BMGR_INVALID_BSS_IDX;
			if (!BMGR_VALID_BSS_IDX(bss_idx_mld))
				continue;

			mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
			if (mld_entry && mld_entry->valid && mld_entry->mlo_en) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\t  Get per-sta profile for link(%d):\n", link_id);
				pos = get_ap_per_sta_profile_info(mld_entry, pos);
			}
		}

		len = pos - start;
		hex_dump_with_lvl("Add per-sta profile Sub-IE", start, len, DBG_LVL_INFO);
		mtk_cfg80211_event_bss_ml_sta_profile_info(pAd, start, len);

		if (pos != NULL && start != NULL)
			os_free_mem(start);
#endif
	} else if (0
#ifdef HOSTAPD_OWE_SUPPORT
		|| (pEntry->ie_list && pEntry->ie_list->ecdh_ie.length)
#endif
		) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
			"OWE Reassoc Req Handle in CFG Layer\n");
		CFG80211OS_RxMgmt(pNetDev, freq, FC, DataSize);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			"no FT or OWE ie\n");
		if (pEntry->ie_list != NULL)
			goto LabelOK;
	}
}
#endif /*RT_CFG80211_SUPPORT*/

	if (pEntry->func_tb_idx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "pEntry bounding invalid wdev(apidx=%d)\n",
				  pEntry->func_tb_idx);
		goto LabelOK;
	}

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
			 "%s():pEntry->func_tb_idx=%d\n",
			  __func__, pEntry->func_tb_idx);
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if ((wdev == NULL) || (wdev->func_idx < 0)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "Wrong Addr1 - "MACSTR"\n",
				  MAC2STR(ie_list->Addr1));
		goto LabelOK;
	}

	/* WPS_BandSteering Support */
#if defined(BAND_STEERING) && defined(WSC_INCLUDED)
	if (pAd->ApCfg.BandSteering) {
		PWSC_CTRL pWscControl = NULL;
		PBND_STRG_CLI_ENTRY cli_entry = NULL;
		PBND_STRG_CLI_TABLE table = NULL;
		PWPS_WHITELIST_ENTRY wps_entry = NULL;
		PBS_LIST_ENTRY bs_whitelist_entry = NULL;

		pWscControl = &pAd->ApCfg.MBSSID[wdev->func_idx].wdev.WscControl;
		table = &pAd->ApCfg.BndStrgTable;
		if (table->bEnabled) {

			cli_entry = BndStrg_TableLookup(table, pEntry->Addr);
			wps_entry = FindWpsWhiteListEntry(&table->WpsWhiteList, pEntry->Addr);

			/* WPS: special WIN7 case: no wps/rsn ie in assoc */
			/* and send eapol start, consider it as wps station */
			if ((ie_list->RSNIE_Len == 0) && (IS_AKM_WPA_CAPABILITY_Entry(wdev))
				&& (pWscControl->WscConfMode != WSC_DISABLE))
				ie_list->bWscCapable = TRUE;

			/* in case probe did not have wps ie, */
			/* but assoc has, create wps whitelist entry here */
			if (!wps_entry && ie_list->bWscCapable && pWscControl->bWscTrigger) {

				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				AddWpsWhiteList(&table->WpsWhiteList, pEntry->Addr);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					"channel %u: WPS Assoc req: STA "MACSTR" wps whitelisted\n",
					table->Channel, MAC2STR(pEntry->Addr));
				BND_STRG_PRINTQAMSG(table, pEntry->Addr,
				"ASSOC STA "MACSTR" channel %u  added in WPS Whitelist\n",
				MAC2STR(pEntry->Addr), table->Channel);
			}

			bs_whitelist_entry = FindBsListEntry(&table->WhiteList, pEntry->Addr);

			/* handle case: where a client has wps ie in probe, not have bndstrg entry/bndstrg whitelist, */
			/* but doing normal assoc: dont allow */
			if ((pWscControl->bWscTrigger) && (!cli_entry) && (!ie_list->bWscCapable) && (!bs_whitelist_entry)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				"\nRejectAssoc:bWscTrigger:%d,entry:%p,bWscCapable:%d,whitelist:%p\n",
				pWscControl->bWscTrigger, cli_entry,
				ie_list->bWscCapable, bs_whitelist_entry);
				BND_STRG_PRINTQAMSG(table, pEntry->Addr,
				"STA "MACSTR" Normal Assoc Rejected for BS unauthorized client\n",
				MAC2STR(pEntry->Addr));
				goto LabelOK;
			}
		}
	}
#endif
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				 "AP is not ready, disallow new Association\n");
		goto LabelOK;
	}

		if (pAd->FragFrame.wcid == pEntry->wcid) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"%s: Clear Wcid = %d FragBuffer !!!!!\n", __func__, pEntry->wcid);
			RESET_FRAGFRAME(pAd->FragFrame);
		}

		FlgIs11bSta = 1;

		for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
			if (((ie_list->rate.sup_rate[i] & 0x7F) != 2) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 4) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 11) &&
				((ie_list->rate.sup_rate[i] & 0x7F) != 22)) {
				FlgIs11bSta = 0;
				break;
			}
		}
#ifdef CONFIG_MAP_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_DEBUG,
		"IS_MAP_ENABLE=%d\n", IS_MAP_ENABLE(pAd));
	if (IS_MAP_ENABLE(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_DEBUG,
			"Elem->MsgLen=%d, ASSOC_REQ_LEN = %d\n",
			DataSize, (INT)ASSOC_REQ_LEN);
		NdisMoveMemory(pEntry->assoc_req_frame, (VOID *)FC, ASSOC_REQ_LEN);
	}
#endif

#ifdef GN_MIXMODE_SUPPORT
	pEntry->FlgIs11bSta = FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef MBO_SUPPORT
	if (!MBO_AP_ALLOW_ASSOC(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_ERROR,
				"MBO not allowed assoc\n");
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		bMboReject = TRUE;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
		/* goto SendAssocResponse; */
		return;
	}
#endif /* MBO_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
	)
#ifdef DOT11W_PMF_SUPPORT
	&& (!((AssocReqHdr.FC.Wep == 0) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)))
#endif /* DOT11W_PMF_SUPPORT */
	) {
		struct _ASIC_SEC_INFO *info = NULL;

		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */
		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
		if (info == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Fail to allocate memory!\n");
			return;
		} else {
			os_zero_mem(info, sizeof(ASIC_SEC_INFO));
			info->Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
			info->Wcid = pEntry->wcid;
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, info);
			os_free_mem(info);
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

			/* Notify 802.1x daemon to clear this sta info */
			if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
				|| IS_IEEE8021X(&pEntry->SecConfig))
				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		}
	}

#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0) &&
			(IS_AKM_WPA_CAPABILITY_Entry(wdev)) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}

#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0))
			goto LabelOK;
#ifdef WSC_V2_SUPPORT
			/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
			if ((pEntry->bWscCapable) &&
				(wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				(wsc_ctrl->WscV2Info.bEnableWpsV2) &&
				(wsc_ctrl->WscV2Info.bWpsEnable) &&
				(MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)))
				;
			else
#endif /* WSC_V2_SUPPORT */
				/* set a flag for sending Assoc-Fail response to unwanted STA later. */
				if (!ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->func_tb_idx))
					bACLReject = TRUE;
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"assoc - MBSS(%d), receive reassoc request from "MACSTR"\n",
			pEntry->func_tb_idx, MAC2STR(ie_list->Addr2));
		/* supported rates array may not be sorted. sort it and find the maximum rate */
		for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
			if (MaxSupportedRate < (ie_list->rate.sup_rate[i] & 0x7f))
				MaxSupportedRate = ie_list->rate.sup_rate[i] & 0x7f;
		}
		/*
		 *	Assign RateLen here or we will select wrong rate table in
		 *	APBuildAssociation() when 11N compile option is disabled.
		*/
		pEntry->RateLen = ie_list->rate.sup_rate_len;
		pEntry->MaxSupportedRate = MaxSupportedRate;
		RTMPSetSupportMCS(pAd,
						  OPMODE_AP,
						  pEntry,
						  &ie_list->rate,
#ifdef DOT11_VHT_AC
						  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
						  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
						  &ie_list->cmm_ies.ht_cap,
						  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));
#ifdef GN_MIXMODE_SUPPORT
		if (pAd->CommonCfg.GNMixMode
			&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
				|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
				|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
			pEntry->SupportRateMode &= ~SUPPORT_CCK_MODE;
			pEntry->SupportCCKMCS &= ~(1 << MCS_0 | 1 << MCS_1 | 1 << MCS_2 | 1 << MCS_3);
		}
#endif /* GN_MIXMODE_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE, "<<<<<<\n");
	return;

LabelOK:
#ifdef RT_CFG80211_SUPPORT
	if (StatusCode != MLME_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "StaDel event\n");
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);
	}
#endif /* RT_CFG80211_SUPPORT */


#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL) {
		os_free_mem(ie_list);
		if (pEntry)
			pEntry->ie_list = NULL;
	}

	return;
}

VOID assoc_fsm_hostapd_assoc_resp_handler(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	/* BOOLEAN bAssocSkip = FALSE; */
	/* CHAR rssi; */
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	/* HEADER_802_11 AssocReqHdr; */
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid = 0;
	PUCHAR pOutBuffer = NULL;
	PUCHAR pRSNXE_Hapd = NULL;
	UINT8 RSNXE_Hapd_len = 0;
	PUCHAR pExtendedIECap_Hapd = NULL;
	UINT8 ExtendedIECap_Hapd_len = 0;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR SupRateLen, PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct BA_INFO *ba_info;
	UCHAR isReassoc;
	UCHAR SubType;
	/* BOOLEAN bACLReject = FALSE; */
#ifdef DOT1X_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /* DOT1X_SUPPORT */
#ifdef MBO_SUPPORT
	P_MBO_CTRL pMboCtrl = NULL;
#endif /* MBO_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	PFT_INFO pFtInfoBuf = NULL; 	/*Wframe-larger-than=1024 warning  removal*/
	PEID_STRUCT pFtIe = NULL;
	PEID_STRUCT pFtFragIe = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
	PEID_STRUCT pEcdhIe = NULL;
	PEID_STRUCT pRsnIe = NULL;
#endif
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	ADD_HT_INFO_IE *addht;
	struct _build_ie_info ie_info;
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */
	struct ieee80211_mgmt *mgmt = NULL;
	ULONG Data = 0;
	UINT8 apidx = 0;
#ifdef DOT11_EHT_BE
	SST setup_link_Sst;
	ULONG offset_ml_ie = 0;
	UINT8 *ml_ie_buf = NULL;
	BOOLEAN setup_link_success = TRUE;
	struct eht_assoc_req_priv *assoc_info = NULL;
#endif /* DOT11_EHT_BE */

	if (!Elem) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR, "Elem is NULL\n");
		return;
	}

	mgmt = (struct ieee80211_mgmt *)(Elem->Msg);
	Data = Elem->MsgLen;

	apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE, "ASSOC\n");

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"Wrong Addr1 - "MACSTR"\n",
					MAC2STR(mgmt->sa));
		goto LabelOK;
	}

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &pMbss->wdev.WscControl;
#endif /* WSC_AP_SUPPORT */

	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);
	FlgIs11bSta = 1;

#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &(wdev->FtCfg);

	if (pFtCfg && pFtCfg->update_sta_mac && pFtCfg->FtCapFlag.FtOverDs)
		COPY_MAC_ADDR(mgmt->da, &pFtCfg->sta_link_addr[0]);
#endif /* DOT11R_FT_SUPPORT */

	pEntry = MacTableLookup(pAd, mgmt->da);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"NoAuth MAC - "MACSTR"\n",
					MAC2STR(mgmt->da));
		goto LabelOK;
	}


#ifdef DOT11_EHT_BE
	/* Sst of setup link for sync non-setup link association */
	setup_link_Sst = pEntry->Sst;
#endif

	ie_list = pEntry->ie_list;
	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			"ie_list is NULL\n");
		goto LabelOK;
	}

#ifdef DOT11R_FT_SUPPORT
	if (pFtCfg && pFtCfg->update_sta_mac && pFtCfg->FtCapFlag.FtOverDs)
		COPY_MAC_ADDR(ie_list->Addr2, &pFtCfg->sta_link_addr[0]);

	os_alloc_mem(NULL, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));
	if (pFtInfoBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
			"pFtInfoBuf mem alloc failed\n");
		goto LabelOK;
	}
	NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));
#endif /* DOT11R_FT_SUPPORT */

	for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
		if (((ie_list->rate.sup_rate[i] & 0x7F) != 2) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 4) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 11) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 22)) {
			FlgIs11bSta = 0;
			break;
		}
	}

	if (pEntry->func_tb_idx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"pEntry bounding invalid wdev(apidx=%d)\n",
					pEntry->func_tb_idx);
		goto LabelOK;
	}

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"pEntry->func_tb_idx=%d\n",
				pEntry->func_tb_idx);

	tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);

	isReassoc = ieee80211_is_reassoc_resp(mgmt->frame_control);

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, pEntry->ie_list, pEntry->MaxSupportedRate, &Aid, isReassoc);
	if (mgmt->u.reassoc_resp.status_code != MLME_SUCCESS)
		StatusCode = mgmt->u.reassoc_resp.status_code;

#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		update_sta_conn_state(pEntry->wdev, pEntry);
	}
	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
		BOOLEAN  bExtractIe = FALSE;
#ifdef DOT11R_FT_SUPPORT
		pFtCfg = &(wdev->FtCfg);
		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			bExtractIe = TRUE;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && (IS_AKM_OWE(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
				|| IS_AKM_OWE(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
				))
			bExtractIe = TRUE;
#endif
		if (wdev && (IS_AKM_DPP(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
				|| IS_AKM_DPP(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
				))
			bExtractIe = TRUE;

		if (bExtractIe) {
			PEID_STRUCT eid_ptr;
			UCHAR *Ptr;
			UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
			/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

#ifdef DOT11R_FT_SUPPORT
			NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));
#endif /* DOT11R_FT_SUPPORT */
			/* Ptr = &Fr->Octet[6]; */
			Ptr = mgmt->u.reassoc_resp.variable;
			eid_ptr = (PEID_STRUCT) Ptr;

			/* get variable fields from payload and advance the pointer */
			while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
				switch (eid_ptr->Eid) {
#ifdef DOT11R_FT_SUPPORT
				case IE_FT_MDIE:
					FT_FillMdIeInfo(eid_ptr, &pFtInfoBuf->MdIeInfo);
					break;

				case IE_FT_FTIE:
					pFtIe = eid_ptr;
					FT_FillFtIeInfo(eid_ptr, &pFtInfoBuf->FtIeInfo);
					break;

				case IE_FT_FTIE_FRAGMENT:
					pFtFragIe = eid_ptr;
					break;

				case IE_FT_RIC_DATA:
					/* record the pointer of first RDIE. */
					if (pFtInfoBuf->RicInfo.pRicInfo == NULL) {
						pFtInfoBuf->RicInfo.pRicInfo = &eid_ptr->Eid;
						pFtInfoBuf->RicInfo.Len = ((UCHAR *)mgmt + Data)
												- (UCHAR *)eid_ptr + 1;
					}

					if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;

				case IE_FT_RIC_DESCRIPTOR:
					if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;
#endif
#if defined(DOT11R_FT_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)

				case IE_RSN:
					if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
#ifdef DOT11R_FT_SUPPORT
						NdisMoveMemory(pFtInfoBuf->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfoBuf->RSNIE_Len = eid_ptr->Len + 2;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
						pRsnIe = eid_ptr;
#endif
					}
					break;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
				case IE_WLAN_EXTENSION:
				{
					/*parse EXTENSION EID*/
					UCHAR *extension_id = (UCHAR *)eid_ptr + 2;

					switch (*extension_id) {
					case IE_EXTENSION_ID_ECDH:
						pEcdhIe = eid_ptr;
					}
				}
					break;
#endif
				case IE_EXT_CAPABILITY:
					pExtendedIECap_Hapd = (UCHAR *)eid_ptr;
					ExtendedIECap_Hapd_len = eid_ptr->Len + 2;
					break;

				case IE_RSNXE:
					pRSNXE_Hapd = (UCHAR *)eid_ptr;
					RSNXE_Hapd_len = eid_ptr->Len + 2;
					break;

				default:
					break;
				}
				eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
			}
		}

		if (mgmt->u.reassoc_resp.status_code == MLME_SUCCESS) {
/*				NdisMoveMemory(&pEntry->MdIeInfo, &auth_info.FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));
*
*				pEntry->AuthState = AS_AUTH_OPEN;
*				pEntry->Sst = SST_AUTH;
*/
		}
	}
		/* just silencely discard this frame */
		/*if (StatusCode == 0xFFFF)
		*	goto LabelOK;
		*/

#ifdef DOT11K_RRM_SUPPORT
	if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_ENABLE(wdev))
		pEntry->RrmEnCap.word = ie_list->RrmEnCap.word;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		/* +++Add by shiang for debug */
		if (WMODE_CAP_AC(wdev->PhyMode)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					 "Peer is VHT capable device!\n");
			NdisMoveMemory(&pEntry->ext_cap, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_DEBUG,
					 "\tOperatingModeNotification=%d\n",
					  pEntry->ext_cap.operating_mode_notification);
			/* dump_vht_cap(pAd, &ie_list->vht_cap); */
		}

		/* ---Add by shiang for debug */
	}
#endif /* DOT11_VHT_AC */

	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
		RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, wdev->wdev_idx, 0);

#ifdef WH_EVENT_NOTIFIER
	if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);

		if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry, Elem);
	}
#endif /* WH_EVENT_NOTIFIER */

#ifdef DOT11W_PMF_SUPPORT
	/* SendAssocResponse: */
#endif /* DOT11W_PMF_SUPPORT */
	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "Reassoc - Send reassoc response (Status=%d)...\n",
			  StatusCode);
	Aid |= 0xc000; /* 2 most significant bits should be ON */
	SubType = isReassoc ? SUBTYPE_REASSOC_RSP : SUBTYPE_ASSOC_RSP;
	CapabilityInfoForAssocResp = pMbss->CapabilityInfo; /*use AP's cability */
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if ((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
			(ie_list->CapabilityInfo & 0x0010))
			CapabilityInfoForAssocResp |= 0x0010;
	}

#endif /* WSC_AP_SUPPORT */
		/* fail in ACL checking => send an Assoc-Fail resp. */
	SupRateLen = rate->legacy_rate.sup_rate_len;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == 1)
		SupRateLen = 4;

	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,						&CapabilityInfoForAssocResp,
					  2,						&StatusCode,
					  2,						&Aid,
					  1,						&SupRateIe,
					  1,						&SupRateLen,
					  SupRateLen,				rate->legacy_rate.sup_rate,
					  END_OF_ARGS);

	if ((rate->legacy_rate.ext_rate_len) && (PhyMode != WMODE_B) && (FlgIs11bSta == 0)) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,
						&TmpLen,				1,
						&ExtRateIe,				1,
						&rate->legacy_rate.ext_rate_len,
						rate->legacy_rate.ext_rate_len,
						rate->legacy_rate.ext_rate,
						END_OF_ARGS);
		FrameLen += TmpLen;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		pEntry->DevPeerRole = ie_list->MAP_AttriValue;
		MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
	}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

	ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (ie_info.wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
					"wdev is NULL\n");
		MlmeFreeMemory((PVOID) pOutBuffer);
		pOutBuffer = NULL;
		goto LabelOK;
	}

	/* add WMM IE here */
	ie_info.is_draft_n_type = FALSE;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;
#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY) {
		ULONG TmpLen;
		UCHAR IEType = IE_TIMEOUT_INTERVAL; /* IE:0x15 */
		UCHAR IELen = 5;
		UCHAR TIType = 3;
		UINT32 units = 1 << 10; /* 1 seconds, should be 0x3E8 */

		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  1, &IEType,
						  1, &IELen,
						  1, &TIType,
						  4, &units,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef DOT11_N_SUPPORT

		/* HT capability in AssocRsp frame. */
	if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) && WMODE_CAP_N(wdev->PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
#ifdef DOT11_VHT_AC
		struct _build_ie_info vht_ie_info;
#endif /* DOT11_VHT_AC */

		ie_info.is_draft_n_type = FALSE;
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);

		if ((ie_list->cmm_ies.vendor_ie.ra_cap) == 0 || (pAd->bBroadComHT == TRUE)) {
			ie_info.is_draft_n_type = TRUE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

		}
#ifdef DOT11_VHT_AC
		vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		vht_ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
		vht_ie_info.channel = wdev->channel;
		vht_ie_info.phy_mode = wdev->PhyMode;
		vht_ie_info.wdev = wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		ucETxBfCap = wlan_config_get_etxbf(wdev);

		if (bf_is_support(wdev) == FALSE)
			wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
			|| HAS_VHT_OP_EXIST(ie_list->cmm_ies.ie_exists)
			|| !WMODE_CAP_2G(wdev->PhyMode))
			FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
	}

#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
		&& IS_HE_STA(pEntry->cap.modes) && WMODE_CAP_AX(wdev->PhyMode)
			&& wdev->DesiredHtPhyInfo.bHtEnable) {
		UINT32 offset = 0;

		offset = add_assoc_rsp_he_ies(wdev, (UINT8 *)pOutBuffer, FrameLen);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_INFO,
				"add he assoc_rsp, len=%d\n", offset);

		FrameLen += offset;
	}
#endif /*DOT11_HE_AX*/

#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
	/* qosmap IE */
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_NOTICE,
		"entry wcid %d QosMapSupport=%d\n", pEntry->wcid, pEntry->QosMapSupport);

	if (pEntry->QosMapSupport) {
		ULONG	TmpLen;
		UCHAR	QosMapIE, ielen = 0, explen = 0;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable) {
			QosMapIE = IE_QOS_MAP_SET;

			/* Fixed field Dscp range:16, len:1 IE_ID:1*/
			if (pHSCtrl->QosMapSetIELen > 18)
				explen = pHSCtrl->QosMapSetIELen - 18;

			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pHSCtrl->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pHSCtrl->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#endif /* CONFIG_HOTSPOT_R2 */

		/* 7.3.2.27 Extended Capabilities IE */
	{
		ULONG TmpLen, infoPos;
		PUCHAR pInfo;
		UCHAR extInfoLen;
		BOOLEAN bNeedAppendExtIE = FALSE;
		EXT_CAP_INFO_ELEMENT extCapInfo;
#ifdef CFG_BIG_ENDIAN
		UCHAR *pextCapInfo;
#endif

		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		/* P802.11n_D1.10, HT Information Exchange Support */
		if (WMODE_CAP_N(wdev->PhyMode)
			&& (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance */

		if (ie_list->ExtCapInfo.BssTransitionManmt == 1) {
			pEntry->bBSSMantSTASupport = TRUE;
			if (pMbss->WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
		}
		/* #endif CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

#ifdef CONFIG_DOT11U_INTERWORKING
		if (pMbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */
#ifdef BCN_PROTECTION_SUPPORT
		{
			struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

			if (sec_cfg->bcn_prot_cfg.bcn_prot_en)
				extCapInfo.bcn_prot_en = 1;
			else
				extCapInfo.bcn_prot_en = 0;
		}
#endif
#ifdef DOT11V_WNM_SUPPORT

		if (IS_BSS_TRANSIT_MANMT_SUPPORT(pAd, pEntry->func_tb_idx)) {
			if (ie_list->ExtCapInfo.BssTransitionManmt == 1) {
				extCapInfo.BssTransitionManmt = 1;
				pEntry->bBSSMantSTASupport = TRUE;
			}
		}

		if (IS_WNMDMS_SUPPORT(pAd, pEntry->func_tb_idx)) {
			if (ie_list->ExtCapInfo.DMSSupport == 1) {
				extCapInfo.DMSSupport = 1;
				pEntry->bDMSSTASupport = TRUE;
			}
		}

#endif /* DOT11V_WNM_SUPPORT */
#ifdef DOT11_VHT_AC

		if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G)
			extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
#ifdef FTM_SUPPORT
		/* 802.11mc D3.0: 10.24.6.2 (p.1717):
		 *	"A STA in which dot11FineTimingMsmtRespActivated is true shall set the Fine Timing Measurement
		 *	Responder field of the Extended Capabilities element to 1."
		*/
		/* 8.4.2.26 Extended Capabilities element (p.817):
		 *	Capabilities field= 70: Fine Timing Measurement Responder (p.823)
		*/
		extCapInfo.ftm_resp = 1;
#endif /* FTM_SUPPORT */
#ifdef CFG_BIG_ENDIAN
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif

		pInfo = (UCHAR *)(&extCapInfo);

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

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&ExtCapIe,
							  1,			&extInfoLen,
							  extInfoLen,	&extCapInfo,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
#ifdef RT_CFG80211_SUPPORT
		pMboCtrl = &wdev->MboCtrl;
		if (pMboCtrl->MboIELen > 0 && pMboCtrl->MboIELen <= MBO_IE_MAX_LEN) {
			ULONG TmpLen;
			/* MBO element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					pMboCtrl->MboIELen,
					pMboCtrl->MBOIE, END_OF_ARGS);
			FrameLen += TmpLen;
		} else
#endif /* RT_CFG80211_SUPPORT */
			MakeMboOceIE(pAd, wdev, pEntry, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_RSP);
	}
#endif /* MBO_SUPPORT */
#ifdef WSC_AP_SUPPORT

	if (pEntry->bWscCapable) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;
		ULONG WscTmpLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocRespIE(pAd, pEntry->func_tb_idx, 0, 512, pWscBuf, &WscIeLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
							  WscIeLen, pWscBuf,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
			os_free_mem(pWscBuf);
		}
	}

#endif /* WSC_AP_SUPPORT */

#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && (IS_AKM_OWE(wdev->SecConfig.AKMMap)
				|| IS_AKM_DPP(wdev->SecConfig.AKMMap)
#ifdef CFG_RSNO_SUPPORT
				|| IS_AKM_OWE(wdev->SecConfig_ext.AKMMap)
				|| IS_AKM_DPP(wdev->SecConfig_ext.AKMMap)
#endif /* CFG_RSNO_SUPPORT */
				)) {
			if (pRsnIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pRsnIe->Len + 2,
								  pRsnIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			if (pEcdhIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pEcdhIe->Len + 2,
								  pEcdhIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
#endif
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		/*PUINT8	ftie_ptr = NULL;*/
		/*UINT8	ftie_len = 0;*/
		/*PUINT8  ricie_ptr = NULL;*/
		/*UINT8   ricie_len = 0;*/
		/* struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig; */

		/* Insert RSNIE if necessary */
		if (pFtInfoBuf->RSNIE_Len != 0) {
			ULONG TmpLen;

			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
				pFtInfoBuf->RSNIE_Len, pFtInfoBuf->RSN_IE,
				END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer+FrameLen;
		mdie_len = 5;
		FT_InsertMdIE(pOutBuffer+FrameLen,
				&FrameLen,
			pFtInfoBuf->MdIeInfo.MdId,
			pFtInfoBuf->MdIeInfo.FtCapPlc);

		/* Insert FTIE. */
		if (pFtIe) {
			ULONG TmpLen = 0;

			MakeOutgoingFrame(pOutBuffer+FrameLen,
							  &TmpLen,
							  pFtIe->Len + 2,
							  pFtIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;

			/* Insert FTIE Frag part. */
			if (pFtFragIe) {
				ULONG TmpFragLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
							  &TmpFragLen,
							  pFtFragIe->Len + 2,
							  pFtFragIe,
							  END_OF_ARGS);
				FrameLen += TmpFragLen;
			}
		}
	}
#endif /* DOT11R_FT_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	if (ExtendedIECap_Hapd_len > 0) {
		ULONG TmpLen = 0;

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"Re-AssocResp ExtendedIE Cap Added (Len=%d)\n", ExtendedIECap_Hapd_len);
		MakeOutgoingFrame(pOutBuffer + FrameLen,
				&TmpLen,
				ExtendedIECap_Hapd_len,
				pExtendedIECap_Hapd,
				END_OF_ARGS);
		FrameLen += TmpLen;
	}

	if (RSNXE_Hapd_len > 0) {
		ULONG TmpLen = 0;

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				"Re-AssocResp RSNXE IE Added (Len=%d)\n", RSNXE_Hapd_len);
		MakeOutgoingFrame(pOutBuffer + FrameLen,
				&TmpLen,
				RSNXE_Hapd_len,
				pRSNXE_Hapd,
				END_OF_ARGS);
		FrameLen += TmpLen;
	}
#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11_EHT_BE
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"(be)TODO: cap.mode=0x%x, bHtEn=%d\n",
			pEntry->cap.modes, wdev->DesiredHtPhyInfo.bHtEnable);
	if (IS_EHT_STA(pEntry->cap.modes)
		&& WMODE_CAP_BE(wdev->PhyMode)
		&& wdev->DesiredHtPhyInfo.bHtEnable) {

		/* this offset should be set to the position of EHT Cap IE */
		/* and be used later for ML IE reorder */
		offset_ml_ie = FrameLen;
		FrameLen += eht_add_assoc_rsp_ies(
				wdev, pOutBuffer, FrameLen, &ie_list->cmm_ies);
	}

	/* EHT MLO non-setup link association */
	if (Elem->Others) {
		assoc_info = (struct eht_assoc_req_priv *)Elem->Others;
		assoc_info->buf = pOutBuffer;
		assoc_info->buf_len = FrameLen;
		setup_link_success = assoc_info->setup_link_success;
		/* In this case, pOutBuffer will be freed */
		/* in eht_ap_mlo_peer_non_setup_links_assoc_req() */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
			"non-setup link assoc rsp: %p, len: %ld\n",
			assoc_info->buf, assoc_info->buf_len);
	} else
#endif /* DOT11_EHT_BE */
	{
		/* legacy & EHT MLO setup link association */
#ifdef DOT11_EHT_BE
		/*
		 * In 802.11be, we shall deduce the inheritance of IEs in STA Profile
		 * in the Basic Multi-link IE in management frames. First, we shall form
		 * the "complete set" of IEs (except ML IE) in the association response
		 * frame of the setup link, and therefore we can do non-setup link association
		 * and apply the inheritance rule to association response frames belonging
		 * to setup link and non-setup links. Consequently, we must put the query
		 * of ML IE at the last of the setup link association, and perform reorder
		 * of ML IE to form the correct association response frame of the setup link.
		 */
		if (IS_EHT_STA(pEntry->cap.modes)
			&& WMODE_CAP_BE(wdev->PhyMode)
			&& wdev->DesiredHtPhyInfo.bHtEnable) {
			ULONG ml_ie_len = 0;
			UINT8 *ml_ie_end_pos = NULL;
			struct mtk_mac_bss *mac_bss;

			/* MLO: non-setup links association */
			if (HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists) && pEntry->mlo.mlo_en) {
				struct buffer_wrapper setup_link_rsp;
				uint16_t mld_sta_idx;
				struct mld_entry_t *mld_entry;

				mt_rcu_read_lock();
				mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
				if (!mld_entry) {
					mt_rcu_read_unlock();
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
						"Error: mld_entry is NULL\n");
					goto LabelOK;
				}

				mld_sta_idx = mld_entry->mld_sta_idx;
				mt_rcu_read_unlock();

				/* Basic variant Multi-Link element */
				mac_bss = (struct mtk_mac_bss *)wdev->pHObj;
				if ((mac_bss->if_cfg.mld_group_idx != 0) &&
					(mac_bss->if_cfg.mld_group_idx != MLD_GROUP_NONE)) {

					setup_link_rsp.buf = pOutBuffer;
					setup_link_rsp.buf_len = FrameLen;

					eht_ap_mlo_peer_non_setup_links_assoc_req(&pEntry->Elem_for_hostapd_owe, pEntry,
						setup_link_Sst, StatusCode, &setup_link_rsp, &ie_list->cmm_ies, isReassoc);

					os_alloc_mem(pAd, (UCHAR **)&ml_ie_buf, MAX_LEN_OF_MLIE);
					if (!ml_ie_buf) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"Error: can not allocate memory for ml ie\n");
						goto LabelOK;
					}

					ml_ie_end_pos = eht_build_multi_link_ie(
						wdev, ml_ie_buf,
						HAS_EHT_MLD_EXIST(ie_list->cmm_ies.ie_exists) ?
							ie_list->cmm_ies.ml_ie : NULL,
						&ie_list->cmm_ies.ml_frag_info,
						BMGR_QUERY_ML_IE_ASSOC_RSP,
						mld_sta_idx);
					ml_ie_len = ml_ie_end_pos - ml_ie_buf;

					/* reorder Basic Variance Multi-Link element */
					if (ml_ie_len != 0) {
						NdisMoveMemory(pOutBuffer + offset_ml_ie + ml_ie_len,
							pOutBuffer + offset_ml_ie, FrameLen - offset_ml_ie);

						NdisMoveMemory(pOutBuffer + offset_ml_ie, ml_ie_buf, ml_ie_len);
						FrameLen += ml_ie_len;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_ERROR,
							"After Reorder ML IE, buf:%p, buf_len:%ld\n",
							pOutBuffer, FrameLen);
					}

					if (ml_ie_buf)
						os_free_mem(ml_ie_buf);
					ml_ie_buf = NULL;
				}
			}
		}
#endif /* DOT11_EHT_BE */

		/* add Ralink-specific IE here -
		 * Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_RESP);
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_wmm_cap_ie(pAd, &ie_info);
		}

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
			"send assoc resp to "MACSTR".\n", MAC2STR(ie_list->Addr2));

		/* transmit the association response frame */
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	}

/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)
#ifdef DOT11_EHT_BE
		/* In MLO, the condition of successful association of non-setup link
		 * is also based on result of setup link association */
		&& (!pEntry->mlo.mlo_en || setup_link_success)
#endif /* DOT11_EHT_BE */
	) {

		/* legacy link or mlo setup link */
		if (wdev_do_conn_act(pEntry->wdev, pEntry) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
				"%s(): connect action fail!!\n", __func__);
			goto assoc_check;
		}

#ifdef WTBL_TDD_SUPPORT
		if (IS_WTBL_TDD_ENABLED(pAd)) {
			pEntry->wtblTddCtrl.state = WTBL_TDD_STA_IDLE;
			pEntry->wtblTddCtrl.guardTime = 0;
		}
#endif /* WTBL_TDD_SUPPORT */
	}

		/* set up BA session */
	if (StatusCode == MLME_SUCCESS
#ifdef DOT11_EHT_BE
		/* In MLO, the condition of successful association of non-setup link
		 * is also based on result of setup link association */
		&& (!pEntry->mlo.mlo_en || setup_link_success)
#endif /* DOT11_EHT_BE */
	) {
		struct _STA_TR_ENTRY *peer_tr_entry;
	pEntry->PsMode = PWR_ACTIVE;
	peer_tr_entry = tr_entry_get(pAd, pEntry->wcid);
	peer_tr_entry->PsMode = PWR_ACTIVE;
	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
#ifdef IAPP_SUPPORT
	{
#ifndef RT_CFG80211_SUPPORT
		IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, pEntry->wdev->wdev_idx);
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "####### Send L2 Frame Mac="MACSTR"\n",
				  MAC2STR(pEntry->Addr));
#endif
	}
#endif /* IAPP_SUPPORT */
	/* ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list); */
	/* send wireless event - for association */
#ifdef VENDOR_FEATURE7_SUPPORT
	/* Passed in the pEntry->apindx argument */
	RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, pEntry->func_tb_idx, 0);
#else
	RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
#endif
	/* This is a reassociation procedure */
	pEntry->IsReassocSta = isReassoc;

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry) && pEntry->mlo.is_setup_link_entry) {
		struct mld_entry_t *mld_entry;

		mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
		if (mld_entry)
			ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	/* clear txBA bitmap */
	ba_info->TxBitmap = 0;

#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif

	if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX) {
	CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		if ((wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) && addht->AddHtInfo.ExtChanOffset &&
				(ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth == BW_40))
			SendBeaconRequest(pAd, pEntry->wcid);

		ba_ori_session_start(pAd, pEntry->wcid, 5);
	}


#ifdef RT_CFG80211_SUPPORT
	if (TRUE) { /*CFG_TODO*/
		/* need to update pEntry to inform later flow */
		/* to keep ConnectionState in connected */
		pEntry->bWscCapable = ie_list->bWscCapable;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			"SINGLE CFG: NOITFY ASSOCIATED, pEntry->bWscCapable:%d\n",
			pEntry->bWscCapable);
#ifdef RT_CFG80211_SUPPORT
	/*	CFG80211OS_NewSta(pEntry->wdev->if_dev, ie_list->Addr2,
			(PUCHAR)Elem->Msg, Elem->MsgLen, isReassoc); */
#endif

		if (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)) {
			struct _ASIC_SEC_INFO *info = NULL;

			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			/* Set key material to Asic */
			if (info) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				info->Direction = SEC_ASIC_KEY_BOTH;
				info->Wcid = pEntry->wcid;
				info->BssIndex = pEntry->func_tb_idx;
				info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
				info->Cipher = pEntry->SecConfig.PairwiseCipher;
				info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
				os_move_mem(&info->Key,
					&pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId],
					sizeof(SEC_KEY_INFO));
				os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		}

		/* hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen); */
	} else
#endif
	/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
	if (IS_AKM_PSK_Entry(pEntry)) {
		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
						   ie_list->RSNIE_Len,
						   PMKID_LIST,
						   &pmkid_count);

		if (pPmkid != NULL) {
			INT CacheIdx;

			CacheIdx = RTMPValidatePMKIDCache(PD_GET_PMKID_PTR(pAd->physical_dev),
							  pEntry->wdev->bssid,
							  pEntry->Addr,
							  pPmkid,
							  FALSE);

			store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx);

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
				 "ASSOC - CacheIdx = %d\n",
				  CacheIdx);

			if (IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
			   !is_pmkid_cache_in_sec_config(&pEntry->SecConfig)) {
				MTWF_DBG(pAd, DBG_CAT_SEC,
					 CATSEC_SAE,
					 DBG_LVL_ERROR,
					 "ASSOC - SAE - verify pmkid fail\n");
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				goto LabelOK;
			}
		}
#ifdef WSC_AP_SUPPORT
		/*
		 * In WPA-PSK mode,
		 * If Association Request of station has RSN/SSN,
		 * WPS AP Must Not send EAP-Request/Identity to station
		 * no matter WPS AP does receive EAPoL-Start from STA or not.
		 * Marvell WPS test bed(v2.1.1.5) will send AssocReq with WPS IE and RSN/SSN IE.
		 */
		if (pEntry->bWscCapable || (ie_list->RSNIE_Len == 0)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "ASSOC - IF(ra%d) This is a WPS Client.\n\n",
					  pEntry->func_tb_idx);
			goto LabelOK;
		} else {
			pEntry->bWscCapable = FALSE;
			pEntry->Receive_EapolStart_EapRspId = (WSC_ENTRY_GET_EAPOL_START |
								   WSC_ENTRY_GET_EAP_RSP_ID);
			/* This STA is not a WPS STA */
			NdisZeroMemory(wsc_ctrl->EntryAddr, 6);
		}

#endif /* WSC_AP_SUPPORT */
		/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
		if (1
#ifdef WSC_AP_SUPPORT
			&& !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
		   ) {
			/* Enqueue a EAPOL-start message with the pEntry */
			os_move_mem(&pEntry->SecConfig.Handshake.AAddr, wdev->bssid, MAC_ADDR_LEN);
			os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr, MAC_ADDR_LEN);

			if (!IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
				!(IS_AKM_OWE(pEntry->SecConfig.AKMMap)))
				os_move_mem(&pEntry->SecConfig.PMK, &wdev->SecConfig.PMK, LEN_PMK);

#ifndef RT_CFG80211_SUPPORT
			RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
#endif /*RT_CFG80211_SUPPORT*/
		}
	}

#ifdef DOT1X_SUPPORT
	else if (IS_AKM_WPA2_Entry(pEntry) ||
		 IS_AKM_WPA3_192BIT_Entry(pEntry)) {
		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
						   ie_list->RSNIE_Len,
						   PMKID_LIST,
						   &pmkid_count);

		if (pPmkid != NULL) {
			/* Key cache */
			INT CacheIdx;

			CacheIdx = RTMPValidatePMKIDCache(PD_GET_PMKID_PTR(pAd->physical_dev),
							  pEntry->wdev->bssid,
							  pEntry->Addr,
							  pPmkid,
							  FALSE);

			process_pmkid(pAd, wdev, pEntry, CacheIdx);
		}
	} else if (IS_AKM_1X_Entry(pEntry) ||
		   (IS_IEEE8021X(&pEntry->SecConfig)
#ifdef WSC_AP_SUPPORT
		   && (!pEntry->bWscCapable)
#endif /* WSC_AP_SUPPORT */
		   )) {
		/* Enqueue a EAPOL-start message to trigger EAP SM */
		if (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
		) {
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
#ifndef RT_CFG80211_SUPPORT
			RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
#endif /*#ifndef RT_CFG80211_SUPPORT*/
		}
	}

#endif /* DOT1X_SUPPORT */

#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
	if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
		MWDSAPPeerEnable(pAd, pEntry);
#endif
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
		if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
			if (pEntry->mlo.mlo_en)
				map_a4_mlo_peer_enable(pAd, NULL, pEntry, TRUE);
			else
#endif
				map_a4_peer_enable(pAd, pEntry, TRUE);
		}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
		wapp_send_cli_join_event(pAd, pEntry);
#endif
	}
#endif
#ifdef GREENAP_SUPPORT

	if (greenap_get_capability(pAd) && greenap_get_allow_status(pAd)) {
		if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
			greenap_check_peer_connection_at_link_up_down(pAd, wdev);
		}

#endif /* GREENAP_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2

	/* add to cr4 pool */
	if (pEntry->QosMapSupport) {
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable) {
			if (!pHSCtrl->QosMapAddToPool) {
				pHSCtrl->QosMapAddToPool = TRUE;
				pHSCtrl->QosMapPoolID = hotspot_qosmap_add_pool(pAd, pEntry);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
					"add current MBSS qosmap to CR4\n");
			}

			hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pHSCtrl->QosMapPoolID);
		}
	}

#endif /* CONFIG_HOTSPOT_R2 */
#ifdef DSCP_QOS_MAP_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
		if (pMbss->DscpQosMapEnable) {
			pEntry->PoolId = pMbss->DscpQosPoolId;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_NOTICE,
				"[DSCP-QOS-MAP] update sta mapping to CR4 for Pool %d wcid %d\n",
				pEntry->PoolId, pEntry->wcid);
			dscp_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pEntry->PoolId);
		}
	}
#endif
	}

#ifdef FAST_EAPOL_WAR
	/*
	*	Moved from WifiSysApPeerLinkUp() in open security mode.
	*	to make sure the STATE_PORT_SECURE flag can be polled by MU N9 module.
	*	then MU Action Frame sent out after Asso Resp.
	*/
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
		if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
			|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
			|| pEntry->bWscCapable)) {
			WifiSysUpdatePortSecur(pAd, pEntry, NULL);
		}
	}
#endif /* FAST_EAPOL_WAR */
#ifdef BAND_STEERING
	if ((pAd->ApCfg.BandSteering))
		BndStrg_UpdateEntry(pAd, pEntry, ie_list, TRUE);
#endif

#ifdef VENDOR_FEATURE7_SUPPORT
	if (arris_event_send_hook && pEntry && (StatusCode == MLME_SUCCESS)) {
		UCHAR assoc_event_msg[32] = {0};
		UINT32 count = 0;
		UCHAR *assoc_sta_info = NULL;
		HT_CAP_INFO  *pHTCap = &(ie_list->HTCapability).HtCapInfo;
		HT_CAP_PARM  *pHTCapParm = &(ie_list->HTCapability).HtCapParm;
		/* Send a WLAN_EVENT to ATOM which in turns sends an RPC
		*	to update our client table on the ARM.
		*/
		NdisZeroMemory(assoc_event_msg, sizeof(assoc_event_msg));
		if (WMODE_CAP_5G(PhyMode))
			count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
					""MACSTR" BSS(%d)",
					MAC2STR(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_50_RADIO);
		else
			count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
					""MACSTR" BSS(%d)",
					MAC2STR(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_24_RADIO);

		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_ASSOC,
			assoc_event_msg, count);

		/* Log this cleint's capabilities in our nvram */
		/* assoc_sta_info = kmalloc(1300, GFP_ATOMIC); */
		os_alloc_mem(NULL, (UCHAR **)&assoc_sta_info, 1300);
		if (assoc_sta_info) {
			NdisZeroMemory(assoc_sta_info, 1300);
			count = 0;
			count += snprintf((assoc_sta_info+count), (1300-count),
			"Association: ("MACSTR") --> %s%d (%s)\n",
			MAC2STR(pEntry->Addr), INF_MAIN_DEV_NAME, pEntry->func_tb_idx,
			MBSS_GET(pEntry->pMbss)->Ssid);
			if (pHTCap && pHTCapParm && ie_list->ht_cap_len && WMODE_CAP_N(wdev->PhyMode)) {
				count += snprintf((assoc_sta_info+count), (1300-count), "  Station Info:\n");
				count += snprintf((assoc_sta_info+count), (1300-count),
				"\tRSSI0(%d), RSSI1(%d), Mode(%s), BW(%s), MCS(%d), SGI(%d)\n",
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
				get_phymode_str(pEntry->HTPhyMode.field.MODE),
				get_bw_str(pEntry->HTPhyMode.field.BW, BW_FROM_OID),
				pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.ShortGI);
			} else {
				count += snprintf((assoc_sta_info+count), (1300-count),
					"	 Station Info (Legacy):\n");
				count += snprintf((assoc_sta_info+count), (1300-count),
					"\tRSSI0(%d), RSSI1(%d), Mode(%s), MCS(%d)\n",
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					get_phymode_str(pEntry->HTPhyMode.field.MODE),
					pEntry->HTPhyMode.field.MCS);
			}
			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST,
			WLAN_LOG_SAVE, 0, assoc_sta_info, count);
			os_free_mem(assoc_sta_info);
		}
	}
#endif
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable) && isReassoc
		 && IS_MBO_ENABLE(wdev)) {
		struct _ASIC_SEC_INFO *info = NULL;
		PPMF_CFG pPmfCfg = &wdev->SecConfig.PmfCfg;
		USHORT Wcid;

		/* Get a specific WCID to record this MBSS key attribute */
		GET_GroupKey_WCID(wdev, Wcid);

		/* Set key material to Asic */
		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));

		if (info) {
			if (pEntry->SecConfig.ptk_Reinstall) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				info->Direction = SEC_ASIC_KEY_BOTH;
				info->Wcid = pEntry->wcid;
				info->BssIndex = pEntry->func_tb_idx;
				info->Cipher = pEntry->SecConfig.PairwiseCipher;
				info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
				os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(info->Key.Key, &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
				hex_dump_with_cat_and_lvl("ApKeyAdd PTK", (UCHAR *)info->Key.Key,
						LEN_TK + LEN_TK2, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO);
				WPAInstallKey(pAd, info, TRUE, TRUE);
#ifdef DOT11_EHT_BE
				if (pEntry && pEntry->mlo.mlo_en) {
					mlo_install_key(pEntry, info, TRUE, TRUE);
					mlo_update_port_secure(pEntry, NULL);
				} else
#endif
					pEntry->SecConfig.ptk_Reinstall = 0;
			}
			if (pEntry->SecConfig.gtk_Reinstall) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_TX;
				info->Wcid = Wcid;
				info->BssIndex = pEntry->apidx;
				info->Cipher = wdev->SecConfig.GroupCipher;
				info->KeyIdx = wdev->SecConfig.GroupKeyId;
				os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
#ifdef RT_CFG80211_SUPPORT
				wdev->Is_hostapd_gtk = 1;
				os_move_mem(wdev->Hostapd_GTK, wdev->SecConfig.GTK, LEN_MAX_GTK);
#endif
				/* Install Shared key */
				os_move_mem(info->Key.Key, wdev->SecConfig.GTK, LEN_MAX_GTK);
				hex_dump_with_cat_and_lvl("ApKeyAdd GTK", (UCHAR *)info->Key.Key,
						LEN_MAX_GTK, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO);
#ifdef DOT11_EHT_BE
				if (pEntry && pEntry->mlo.mlo_en) {
					mlo_install_key(pEntry, info, TRUE, TRUE);
				} else
#endif
					WPAInstallKey(pAd, info, TRUE, TRUE);
				wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
				pEntry->SecConfig.gtk_Reinstall = 0;
			}

#ifdef DOT11W_PMF_SUPPORT
			/* Set key material to Asic */
			if (pEntry->SecConfig.gtk_Reinstall) {
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_TX;
				info->Wcid = Wcid;
				info->BssIndex = pEntry->apidx;
				info->Cipher = wdev->SecConfig.PmfCfg.igtk_cipher;
				info->igtk_key_idx = pPmfCfg->IGTK_KeyIdx;
				info->IGTKKeyLen = LEN_BIP128_IGTK;
				if (IS_CIPHER_BIP_CMAC256(wdev->SecConfig.PmfCfg.igtk_cipher)
						|| IS_CIPHER_BIP_GMAC256(wdev->SecConfig.PmfCfg.igtk_cipher))
					info->IGTKKeyLen = LEN_BIP256_IGTK;
				os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				/* Install Shared key */
				hex_dump_with_cat_and_lvl("ApKeyAdd IGTK", (UCHAR *)pPmfCfg->IGTK[pPmfCfg->IGTK_KeyIdx - 4],
						LEN_MAX_IGTK, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO);
				os_move_mem(info->IGTK, &pPmfCfg->IGTK[pPmfCfg->IGTK_KeyIdx - 4][0], info->IGTKKeyLen);
#ifdef DOT11_EHT_BE
				if (pEntry && pEntry->mlo.mlo_en) {
					mlo_install_key(pEntry, info, TRUE, TRUE);
					mlo_update_port_secure(pEntry, NULL);
				} else
#endif
					WPAInstallKey(pAd, info, TRUE, TRUE);
				wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
			}
#endif /* DOT11W_PMF_SUPPORT */
			os_free_mem(info);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT,
					DBG_LVL_ERROR, "struct alloc fail\n");
		}
	}
#endif /* DOT11R_FT_SUPPORT */

LabelOK:
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

assoc_check:
	if (ie_list != NULL) {
		os_free_mem(ie_list);
		if (pEntry)
			pEntry->ie_list = NULL;
	}
#ifdef RT_CFG80211_SUPPORT

	if ((StatusCode != MLME_SUCCESS) && (StatusCode != MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED)) {
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
			"assoc resp (status:%d) is not success,\n", StatusCode);
	}

#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	if (pFtInfoBuf != NULL)
		os_free_mem(pFtInfoBuf);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11_EHT_BE
		if (ml_ie_buf != NULL)
			os_free_mem(ml_ie_buf);
#endif /* DOT11_EHT_BE */

	if (pOutBuffer != NULL)
		MlmeFreeMemory((PVOID) pOutBuffer);

	return;
}


#endif /*RT_CFG80211_SUPPORT*/


static VOID assoc_fsm_msg_invalid_state(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	ULONG curr_state;
	BOOLEAN isErrHandle = TRUE;
	USHORT Status = MLME_STATE_MACHINE_REJECT;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			 "wdev(type = %d,func_idx = %d\n", wdev->wdev_type, wdev->func_idx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			 "wdev = NULL\n");
		return;
	}
#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	if (!pStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_ERROR,
			 "pStaCfg = NULL\n");
		return;
	}
#endif


	switch (Elem->MsgType) {
	case ASSOC_FSM_MLME_ASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_ASSOC_CONF, Status);
		break;

	case ASSOC_FSM_MLME_REASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status);
		break;

	case ASSOC_FSM_MLME_DISASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DISASSOC_CONF, Status);
		break;

	default:
		isErrHandle = FALSE;
	}

	curr_state = wdev->assoc_machine.CurrState;

	if (isErrHandle == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
			"[%s]: [%s][%s] ====================> state Recovery for CNTL\n",
			wdev->if_dev->name,
			ASSOC_FSM_STATE_STR[curr_state],
			ASSOC_FSM_MSG_STR[Elem->MsgType]);
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_WARN,
			"[%s]: [%s][%s] ====================> FSM MSG DROP\n",
			wdev->if_dev->name,
			ASSOC_FSM_STATE_STR[curr_state],
			ASSOC_FSM_MSG_STR[Elem->MsgType]);
	}
}

static BOOLEAN assoc_fsm_msg_checker(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isMsgDrop = FALSE;
	struct wifi_dev *wdev = Elem->wdev;

	if (!wdev)
		return isMsgDrop;

	if (!wdev->DevInfo.WdevActive)
		isMsgDrop = TRUE;

#ifdef APCLI_SUPPORT

	if (IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) {
		if (isValidApCliIf(wdev->func_idx) == FALSE)
			isMsgDrop = TRUE;
	}

#endif /* APCLI_SUPPORT */

	return isMsgDrop;
}

/* --> PUBLIC Function Start */
BOOLEAN assoc_fsm_state_transition(struct wifi_dev *wdev, ULONG NextState)
{
	ULONG OldState = 0;

	OldState = wdev->assoc_machine.CurrState;
	wdev->assoc_machine.CurrState = NextState;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			 "ASSOC[%s, TYPE:%d %s]: [%s] \t==============================================> [%s]\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  ASSOC_FSM_STATE_STR[OldState],
			  ASSOC_FSM_STATE_STR[NextState]);
	return TRUE;
}


VOID assoc_fsm_reset(struct wifi_dev *wdev)
{
	assoc_fsm_state_transition(wdev, ASSOC_IDLE);
}


/*
	==========================================================================
	Description:
		association state machine init, including state transition and timer init
	Parameters:
		S - pointer to the association state machine

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID assoc_fsm_init(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, Trans, MAX_ASSOC_STATE, MAX_ASSOC_MSG,
					 (STATE_MACHINE_FUNC) assoc_fsm_msg_invalid_state, ASSOC_IDLE,
					 ASSOC_FSM_BASE);
	StateMachineSetMsgChecker(S, (STATE_MACHINE_MSG_CHECKER)assoc_fsm_msg_checker);
	/* first column */
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_ASSOC_REQ,    (STATE_MACHINE_FUNC) assoc_fsm_mlme_assoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_REASSOC_REQ,  (STATE_MACHINE_FUNC) assoc_fsm_mlme_reassoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_mlme_disassoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_ASSOC_REQ,    (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_REASSOC_REQ,  (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_req_action);
	/* second column */
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_ASSOC_RSP,    (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_rsp_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_REASSOC_RSP,  (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_rsp_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_ASSOC_TIMEOUT,     (STATE_MACHINE_FUNC) assoc_fsm_mlme_assoc_req_timeout_action);
	/* third column */
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_REASSOC_RSP,  (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_rsp_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_ASSOC_RSP,    (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_rsp_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_REASSOC_TIMEOUT,   (STATE_MACHINE_FUNC) assoc_fsm_mlme_reassoc_req_timeout_action);
	/* fourth column */
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, ASSOC_FSM_DISASSOC_TIMEOUT,  (STATE_MACHINE_FUNC) assoc_fsm_mlme_disassoc_req_timeout_action);

#ifdef RT_CFG80211_SUPPORT
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_HOSTAPD_ASSOC_REQ_HANDLER,  (STATE_MACHINE_FUNC) assoc_fsm_hostapd_assoc_req_handler);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_HOSTAPD_REASSOC_REQ_HANDLER,  (STATE_MACHINE_FUNC) assoc_fsm_hostapd_reassoc_req_handler);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_HOSTAPD_ASSOC_RESP_HANDLER,	(STATE_MACHINE_FUNC) assoc_fsm_hostapd_assoc_resp_handler);
#endif /*RT_CFG80211_SUPPORT*/

	wdev->assoc_machine.CurrState = ASSOC_IDLE;
}
