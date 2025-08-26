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
    mbss_11v.h

    Abstract:
    Support 11v multi-BSS function.

*/

/* for 4 Tx bss per band */
#define MAX_TX_BSS_CNT 4

#ifdef DOT11V_MBSSID_SUPPORT
enum mbss_11v_mode {
	MBSS_11V_NONE = 0,
	MBSS_11V_T    = 1,
	MBSS_11V_NT   = 2,
	MBSS_11V_CH   = 3,
};

struct mbss_11v_member_node {
	struct mbss_11v_member_node *pNext;
	struct _BSS_STRUCT *role_bss;
	enum mbss_11v_mode role;
};

struct mbss_11v_ctrl {
	 /* 0: legacy, 1: 11vT, 2: 11vNT, 3: Co-Host --> change by profile/cmd */
	enum mbss_11v_mode mbss_11v_enable;
	u8 mbss_11v_t_bss_idx; /* change by profile/cmd */
	u8 mbss_11v_grp_idx; /* change by profile  range tx grp : 0~3 */
	/* Bss number of this 11v group. Valid when mbss_11v_enable is 11vT. */
	u8 mbss_11v_nt_bss_num;
	u8 mbss_11v_ch_bss_num;
	/* 11vNT bss idx. valid when mbss_11v_enable is 11vNT. */
	u8 mbss_11v_nt_idx;
	/* mac adapter list, per pad per band. */
	struct _LIST_HEADER mbss_11v_member; /* 1vNT & CoHost */
	NDIS_SPIN_LOCK mbss_11v_ctrl_lock;
	struct _BSS_STRUCT *mbss_11v_t_bss;
	/* = n, where 2^n is the max number of BSSIDs in Multiple-BSSID set */
	u8 mbss_11v_max_bssid_indicator;
};

#define IS_BSSID_11V_ENABLED(_pAd) \
	((_pAd)->ApCfg.dot11v_mbssid_bitmap != 0)

#define IS_BSSID_11V_ROLE_NONE(_bss_11v_ctrl) \
	(((struct mbss_11v_ctrl *)_bss_11v_ctrl)->mbss_11v_enable == MBSS_11V_NONE)

#define IS_BSSID_11V_TRANSMITTED(_bss_11v_ctrl) \
	(((struct mbss_11v_ctrl *)_bss_11v_ctrl)->mbss_11v_enable == MBSS_11V_T)

#define IS_BSSID_11V_NON_TRANS(_bss_11v_ctrl) \
	(((struct mbss_11v_ctrl *)_bss_11v_ctrl)->mbss_11v_enable == MBSS_11V_NT)

#define IS_BSSID_11V_CO_HOSTED(_bss_11v_ctrl) \
	(((struct mbss_11v_ctrl *)_bss_11v_ctrl)->mbss_11v_enable == MBSS_11V_CH)

/*
 * transmitted-BSSID is responsible for (Multiple BSSID) IEs creating to Nontransmitted-BSSID
 * check if any Nontransmitted-BSSID IE needed
 */
#define IS_MBSSID_IE_NEEDED(_bss_11v_ctrl) \
	(IS_BSSID_11V_TRANSMITTED(_bss_11v_ctrl) && \
	(((struct mbss_11v_ctrl *)_bss_11v_ctrl)->mbss_11v_nt_bss_num))

u8 mbss_11v_bssid_num_to_max_indicator(UCHAR bssid_num);
void mbss_11v_init(struct _RTMP_ADAPTER *ad, struct _BSS_STRUCT *mbss);
void mbss_11v_exit(struct _RTMP_ADAPTER *ad, struct _BSS_STRUCT *mbss);
/* return per tx group's mbss idx */
u8 mbss_11v_group_add(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
void mbss_11v_group_remove(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
void mbss_11v_group_information(struct _RTMP_ADAPTER *ad, u8 dump_level);
struct wifi_dev *mbss_11v_get_tx_wdev(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
	struct _BSS_STRUCT *mbss, BOOLEAN *bMakeBeacon);
void mbss_11v_tim_ie_handle(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
	struct _BSS_STRUCT *mbss, UCHAR *pBeaconFrame, ULONG *FrameLen);
VOID mbss_11v_set_if_addr_gen3(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

/*
* dup bcn of NT-BSS : ref & copy from cfg80211 func : cfg80211_parse_mbssid_frame_data() kernel 6.x
* 1. merge the profile
* 2. handle NON-INHERITANCE of
* 3. en-queue the NT-BSS bcn into mlme. after 1, 2
*/
BOOLEAN ap_ieee_802_11_rx_bcn_dup(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,  struct _RX_BLK *rx_blk);
#endif

