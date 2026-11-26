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

	All CFG80211 Function Prototype.

***************************************************************************/


#ifndef __CFG80211_VNDR_H__
#define __CFG80211_VNDR_H__

#ifdef RT_CFG80211_SUPPORT

#ifdef CONFIG_AP_SUPPORT
INT mtk_cfg80211_set_beacon_period(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u16 bcn_int);
INT mtk_cfg80211_set_dtim_period(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 dtim_int);
INT mtk_cfg80211_set_hide_ssid(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 hide_en);
INT mtk_cfg80211_set_mgmt_rx(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 mgmt_rx_action);
INT mtk_cfg80211_set_no_bcn(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 no_bcn);
INT mtk_cfg80211_set_11axonly(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable);
INT mtk_cfg80211_set_11beonly(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable);
INT mtk_cfg80211_set_no_agmode(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable);
INT mtk_cfg80211_set_no_nmode(
	struct _RTMP_ADAPTER *pAd, u32 inf_idx, INT inf_type, u8 enable);
#endif /* CONFIG_AP_SUPPORT */

#ifdef FTM_SUPPORT
extern struct
nla_policy SUBCMD_FTM_POLICY[];

int mtk_cfg80211_vndr_set_ftm_handler(
	struct wiphy *wiphy, struct wireless_dev *wl_dev, const void *data, int len);
VOID mtk_cfg80211_dump_ftm_parm(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID mtk_cfg80211_set_ftm_test_mode(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT enable);
VOID mtk_cfg80211_set_ftm_enable(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT ftm_enable);
VOID mtk_cfg80211_set_ftm_role(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT role);
VOID mtk_cfg80211_set_ftm_burst_exp(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT burst_exp);
VOID mtk_cfg80211_set_ftm_burst_dur(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT burst_dur);
VOID mtk_cfg80211_update_ftm_min_delta(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID mtk_cfg80211_set_ftm_min_delta(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT min_delta);
VOID mtk_cfg80211_set_ftm_ptsf(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT ptsf);
VOID mtk_cfg80211_set_ftm_ptsf_no_perfer(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT ptsf_no_perfer);
VOID mtk_cfg80211_set_ftm_asap(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT asap);
VOID mtk_cfg80211_set_ftm_num(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT ftms_per_burst);
INT mtk_cfg80211_set_fmt_and_bw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT fmt_and_bw);
VOID mtk_cfg80211_set_ftm_burst_period(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT burst_period);
VOID mtk_cfg80211_set_ftm_target_mac(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 *macAddr);
VOID mtk_cfg80211_set_ftm_debug(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *macAddr);

VOID mtk_cfg80211_set_ftm_toae_cfg(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *toae_cfg);
VOID mtk_cfg80211_range_req_mc_after_scan(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID mtk_cfg80211_range_req_mc(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT enable, UINT8 peer_num);

extern struct
nla_policy SUBCMD_FTM_STAT_POLICY[];

int mtk_cfg80211_vndr_get_ftm_stat_handler(
	struct wiphy *wiphy, struct wireless_dev *wdev, const void *data, int len);
int mtk_cfg80211_vndr_get_ftm_stat_iSTA(
	struct _RTMP_ADAPTER *pAd, POS_COOKIE pObj, RTMP_STRING *msg, int msg_buf_len, int value);
#endif /* FTM_SUPPORT */

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211_VNDR_H__ */


