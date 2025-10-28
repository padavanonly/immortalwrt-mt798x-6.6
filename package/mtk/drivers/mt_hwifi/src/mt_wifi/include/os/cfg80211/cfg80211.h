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

	All MAC80211/CFG80211 Related Structure & Definition.

***************************************************************************/
#ifndef __CFG80211_H__
#define __CFG80211_H__

#ifdef RT_CFG80211_SUPPORT

#include <linux/ieee80211.h>
#ifdef WAPP_SUPPORT
#include "wapp/wapp_cmm_type.h"
#endif /* WAPP_SUPPORT */

#if (KERNEL_VERSION(4, 7, 0) <= LINUX_VERSION_CODE)
#define IEEE80211_NUM_BANDS NUM_NL80211_BANDS
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#define IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#define IEEE80211_BAND_6GHZ NL80211_BAND_6GHZ
#endif

#define COOkIE_VALUE 5678

typedef enum _NDIS_HOSTAPD_STATUS {
	Hostapd_Disable = 0,
	Hostapd_EXT,
	Hostapd_CFG
} NDIS_HOSTAPD_STATUS, *PNDIS_HOSTAPD_STATUS;

#ifdef SUPP_SAE_SUPPORT
typedef struct __MTK_PMKSA_EVENT {
	UINT8 pmkid[16];
	UINT8 pmk[64];
	UINT32 pmk_len;
	UINT32 akmp;
	UINT8 aa[ETH_ALEN];
} MTK_PMKSA_EVENT;
#endif

#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
enum PARAM_MLO_EVENT_SUBID {
	MTK_GRID_MLO_EXTERNAL_AUTH = 1,/* 1 */
};

struct  GNU_PACKED PARAM_EXTERNAL_AUTH_INFO {
	UINT8 id;
	UINT8 len;
	UINT8 ssid[32+1];
	UINT8 ssid_len;
	UINT8 bssid[ETH_ALEN];
	UINT32 key_mgmt_suite;
	UINT32 action;
	UINT8 da[ETH_ALEN];
	UINT8 ext_ie[0];
};

enum CFG80211_ML_EVENT {
	CFG80211_ML_EVENT_ADDLINK = 0,
	CFG80211_ML_EVENT_DELLINK,
};

int mtk_cfg80211_send_bss_ml_event(struct wifi_dev *wdev, enum CFG80211_ML_EVENT event_id);
#endif

typedef struct __CFG80211_CB {

	/* we can change channel/rate information on the fly so we backup them */
	struct ieee80211_supported_band Cfg80211_bands[IEEE80211_NUM_BANDS];
	struct ieee80211_channel *pCfg80211_Channels;
	struct ieee80211_rate *pCfg80211_Rates;

	/* used in wiphy_unregister */
	struct wireless_dev *pCfg80211_Wdev;

	/* used in scan end */
	struct cfg80211_scan_request *pCfg80211_ScanReq;

	/* monitor filter */
	UINT32 MonFilterFlag;

	/* channel information */
	struct ieee80211_channel ChanInfo[MAX_NUM_OF_CHS];

	/* to protect scan status */
	spinlock_t scan_notify_lock;

} CFG80211_CB;

/*
========================================================================
Routine Description:
	Register MAC80211 Module.

Arguments:
	pAd				- WLAN control block pointer
	pDev			- Generic device interface
	pNetDev			- Network device

Return Value:
	NONE

Note:
	pDev != pNetDev
	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))

	Can not use pNetDev to replace pDev; Or kernel panic.
========================================================================
*/
BOOLEAN CFG80211_Register(
	struct _RTMP_ADAPTER *pAd,
	struct device *pDev,
	struct net_device *pNetDev);
UINT32 bssinfo_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	UINT16 wcid,
	UINT64 *feature);
int nl80211_send_event_ACS_per_channel_info(
	struct wiphy *wiphy,
	void *data, int len);
int nl80211_send_event_offchannel_info(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	void *data, int len);
int nl80211_send_event_stop_disassoc_timer(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	void *data, int len);
int nl80211_send_event_sta_link_mac(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	void *data, int len);
int mtk_cfg80211_vndr_cmd_reply_msg(
	struct _RTMP_ADAPTER *pAd,
	struct wiphy *wiphy,
	unsigned int attr_cmd,
	void *data,
	unsigned int data_len);
#ifdef WAPP_SUPPORT
int nl80211_send_wapp_qry_rsp(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	struct wapp_event *event);
#endif /* WAPP_SUPPORT */

#ifdef DFS_SLAVE_SUPPORT
int mtk_nl80211_bhstatus_event(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	void *data, int len);
#endif /* DFS_SLAVE_SUPPORT */
#ifdef ANDLINK_V4_0
INT mtk_nl80211_andlink_wifi_change_event(struct wiphy *wiphy,
						void *data, int len);
INT	mtk_cfg80211_andlink_get_uplink_info(
	struct _RTMP_ADAPTER *pAd, INT apidx, void *data);
INT	mtk_cfg80211_andlink_get_stainfo_result(
	struct _RTMP_ADAPTER *pAd, INT apidx, void *data, INT offset);
INT	mtk_cfg80211_andlink_get_scan_result(
	struct _RTMP_ADAPTER *pAd, INT apidx, void *data, INT offset);
#endif /* ANDLINK_V4_0 */



struct os_cookie *cfg80211_vndr_cmd_cookie_prepare(
	struct _RTMP_ADAPTER *pAd, ULONG priv_flags,
	struct net_device *net_dev);

int nl80211_send_acs_complete_event(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
struct wifi_dev *mtk_cfg80211_vndr_get_wdev_by_wireless_dev(
	struct _RTMP_ADAPTER *pAd,
	struct wireless_dev *wl_dev);

VOID *CFG80211_FindChan(struct _RTMP_ADAPTER *pAd, UCHAR chan_id);

enum nl80211_band CFG80211_Get_BandId(struct _RTMP_ADAPTER *pAd);

/* cfg80211 ops may sync some
 *parameters to other driver modules */

/* cfg80211 state */
enum CFG80211_STATE {
	CFG80211_STATE_BASE,
	WAIT_FOR_OPS_SYNC,
	MAX_CFG80211_STATE,
};

/* cfg80211 events */
enum CFG80211_EVENT {
	CFG80211_EVENT_BASE,
	UPDATE_BSS_MGMT,
	UPDATE_BSS_MNGR,
	MAX_CFG80211_MSG,
};

int nl80211_afc_daemon_channel_info(struct _RTMP_ADAPTER *pAd, struct wiphy *wiphy);
int mtk_cfg80211_reply_wifi_tempreture(struct _RTMP_ADAPTER *pAd,
					struct wiphy *wiphy, struct wireless_dev *wl_dev);
int mtk_cfg80211_reply_radio_rssi(struct _RTMP_ADAPTER *pAd,
					struct wiphy *wiphy, struct wireless_dev *wl_dev);
#define CFG80211_FUNC_SIZE (MAX_CFG80211_STATE * MAX_CFG80211_MSG)

#endif /* RT_CFG80211_SUPPORT */

#ifndef AP_SETTINGS_DOT11VMBSSID_SUPPORT
#define AP_SETTINGS_DOT11VMBSSID_SUPPORT   (0x1 << 16)
#endif

#endif /* __CFG80211_H__ */

/* End of cfg80211.h */
