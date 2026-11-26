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
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *		1. 2009/09/17	Sample Lin
 *			(1) Init version.
 *		2. 2009/10/27	Sample Lin
 *			(1) Do not use ieee80211_register_hw() to create virtual interface.
 *				Use wiphy_register() to register nl80211 command handlers.
 *			(2) Support iw utility.
 *		3. 2009/11/03	Sample Lin
 *			(1) Change name MAC80211 to CFG80211.
 *			(2) Modify CFG80211_OpsChannelSet().
 *			(3) Move CFG80211_Register()/CFG80211_UnRegister() to open/close.
 *		4. 2009/12/16	Sample Lin
 *			(1) Patch for Linux 2.6.32.
 *			(2) Add more supported functions in CFG80211_Ops.
 *		5. 2010/12/10	Sample Lin
 *			(1) Modify for OS_ABL.
 *		6. 2011/04/19	Sample Lin
 *			(1) Add more supported functions in CFG80211_Ops v33 ~ 38.
 *
 *	Note:
 *		The feature is supported only in "LINUX" 2.6.28 ~ 2.6.38.
 *
 ***************************************************************************/


#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"
#include "rt_linux.h"
#include "bss_mngr.h"
#include "cfg80211/cfg80211.h"
#include <net/netlink.h>
#ifdef RT_CFG80211_SUPPORT
#include "mtk_vendor_nl80211.h"
#endif
#include "ap_cfg.h"
#ifdef VOW_SUPPORT
#include "ap_vow.h"
#endif
#include "hdev/hdev_basic.h"

#if (KERNEL_VERSION(2, 6, 28) <= LINUX_VERSION_CODE)
#ifdef RT_CFG80211_SUPPORT

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#ifdef DOT11_EHT_BE
extern struct bss_manager bss_mngr;
#endif

static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC,
	WLAN_CIPHER_SUITE_BIP_CMAC_256,
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
#endif /* HOSTAPD_SUITEB_SUPPORT */
#endif /*DOT11W_PMF_SUPPORT*/
	WLAN_CIPHER_SUITE_GCMP,
	WLAN_CIPHER_SUITE_CCMP_256,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_GCMP_256,
#endif /* HOSTAPD_SUITEB_SUPPORT */

};

#ifdef HOSTAPD_11R_SUPPORT
/* ft policy, used by hostapd */
static struct nla_policy mtk_nl80211_vendor_attr_ft_policy[MTK_NL80211_VENDOR_ATTR_FT_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_FTIE] = NLA_POLICY_EXACT_LEN(sizeof(struct ap_11r_params)),
};
#endif

#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
/* mlo policy, used by hostapd */
static struct nla_policy mtk_nl80211_vendor_attr_mlo_policy[MTK_NL80211_VENDOR_ATTR_MLO_IE_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_AUTH_MLO_IE] = NLA_POLICY_EXACT_LEN(sizeof(struct wpa_mlo_ie_parse)),
};

static struct nla_policy mtk_nl80211_vendor_attr_mlo_ft_policy[MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY] = { .type = NLA_BINARY, .len = 200 },
};
#endif

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
/* pmkid policy, used by hostapd */
static struct nla_policy mtk_nl80211_vendor_attr_pmkid_policy[MTK_NL80211_VENDOR_ATTR_PMKID_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_PMKSA_CACHE] = NLA_POLICY_EXACT_LEN(sizeof(struct __RT_CMD_AP_IOCTL_UPDATE_PMKID)),
};
#endif

#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
/* mac address policy, used by wpa_supplicant */
static struct nla_policy mtk_nl80211_vendor_attr_set_mac_policy[MTK_NL80211_VENDOR_ATTR_SET_MAC_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_STA_MAC] = NLA_POLICY_ETH_ADDR,
};
static struct nla_policy mtk_nl80211_vendor_attr_set_seq_num_policy[MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM] = { .type = NLA_U8, .len = sizeof(u8) },
};
#endif

static struct nla_policy mtk_nl80211_vendor_attr_sta_clean_policy[MTK_NL80211_VENDOR_STA_CLEAN_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_STA_CLEAN_MAC] = NLA_POLICY_ETH_ADDR,
};
/* The driver's regulatory notification callback */
static void CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN struct regulatory_request	*pRequest);

/* get RALINK pAd control block in 80211 Ops */
#define MAC80211_PAD_GET(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (struct _RTMP_ADAPTER *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return -EINVAL;											\
		}															\
	}

#define MAC80211_PAD_GET_NO_RV(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return;											\
		}															\
	}

#define MAC80211_PAD_GET_RETURN_NULL(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return NULL;											\
		}															\
	}

#define MSEC_TO_SEC(_msec)		((_msec) / MSEC_PER_SEC)

POS_COOKIE cfg80211_vndr_cmd_cookie_prepare(struct _RTMP_ADAPTER *pAd, ULONG priv_flags,
	struct net_device *net_dev)
{
	POS_COOKIE pObj;
	int index;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pObj->pSecConfig = NULL;

	/* determine this ioctl command is coming from which interface. */
	if (priv_flags == INT_MAIN) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;
	} else if (priv_flags == INT_MBSSID) {
		pObj->ioctl_if_type = INT_MBSSID;

		for (index = 1; index < pAd->ApCfg.BssidNum; index++) {
			if (pAd->ApCfg.MBSSID[index].wdev.if_dev == net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->ApCfg.MBSSID[index].wdev.SecConfig;
				break;
			}
		}

		/* Interface not found! */
		if (index == pAd->ApCfg.BssidNum)
			return NULL;

		MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
	}

#ifdef WDS_SUPPORT
	else if (priv_flags == INT_WDS) {
		pObj->ioctl_if_type = INT_WDS;

		for (index = 0; index < MAX_WDS_ENTRY; index++) {
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->WdsTab.WdsEntry[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_WDS_ENTRY) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
					"can not find wds I/F\n");
				return NULL;
			}
		}
	}

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;

		for (index = 0; index < MAX_APCLI_NUM; index++) {
			if (pAd->StaCfg[index].wdev.if_dev == net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->StaCfg[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_APCLI_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
					"can not find Apcli I/F\n");
				return NULL;
			}
		}

		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}

#endif /* APCLI_SUPPORT */
	else {
		return NULL;
	}

	return pObj;
}


/*
 * ========================================================================
 * Routine Description:
 *	Change type/configuration of virtual interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	IfIndex			- Interface index
 *	Type			- Interface type, managed/adhoc/ap/station, etc.
 *	pFlags			- Monitor flags
 *	pParams			- Mesh parameters
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: set type, set monitor
 * ========================================================================
 */
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNetDevIn,
	IN enum nl80211_iftype			Type,
	struct vif_params				*pParams)
{
	UINT oldType = pNetDevIn->ieee80211_ptr->iftype;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_WARN, "80211>==>\n");
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_WARN,
		"80211> IfTypeChange %d ==> %d\n", oldType, Type);
	return -EOPNOTSUPP;
}

#if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT)
extern int rt_ioctl_siwscan(struct net_device *dev,
							struct iw_request_info *info,
							union iwreq_data *wreq, char *extra);
#endif /* LINUX_VERSION_CODE: 2.6.30 */
/*
 * ========================================================================
 * Routine Description:
 *	Request to do a scan. If returning zero, the scan request is given
 *	the driver, and will be valid until passed to cfg80211_scan_done().
 *	For scan results, call cfg80211_inform_bss(); you can call this outside
 *	the scan/scan_done bracket too.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pRequest		- Scan request
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: scan
 *
 *	struct cfg80211_scan_request {
 *		struct cfg80211_ssid *ssids;
 *		int n_ssids;
 *		struct ieee80211_channel **channels;
 *		UINT32 n_channels;
 *		const u8 *ie;
 *		size_t ie_len;
 *
 *	 * @ssids: SSIDs to scan for (active scan only)
 *	 * @n_ssids: number of SSIDs
 *	 * @channels: channels to scan on.
 *	 * @n_channels: number of channels for each band
 *	 * @ie: optional information element(s) to add into Probe Request or %NULL
 *	 * @ie_len: length of ie in octets
 * ========================================================================
 */
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct cfg80211_scan_request *pRequest)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	CFG80211_CB *pCfg80211_CB;
	RT_CMD_STA_IOCTL_SCAN scan_cmd;
	CHAR staIndex, apIndex;
	UCHAR ssid[32] = {0};
	struct net_device *pNdev = NULL;
	struct wireless_dev *pWdev = NULL;
	/*request dev infos*/
	struct wifi_dev *preq_wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "80211> ==>\n");

	if (pRequest == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"pRequest is NULL!!\n");
		return -EOPNOTSUPP;
	}

	if (pRequest->ssids)
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> ==>ssid:%s, pRequest->bssid:%pM\n",
			pRequest->ssids->ssid, pRequest->bssid);

	MAC80211_PAD_GET(pAd, pWiphy);
	pCfg80211_CB = pAd->pCfg80211_CB;

	pWdev = pRequest->wdev;
	if (pWdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"pWdev is NULL!!\n");
		return -EOPNOTSUPP;
	}

	pNdev = pWdev->netdev;
	if (pNdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"pNdev is NULL!!\n");
		return -EOPNOTSUPP;
	}
	apIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNdev);
	staIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNdev);
	if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) {
		if (staIndex != WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Scan Request for Apcli i/f proceed for scanning\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Invaliad APCLI index i/f end scan\n");
			pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */
			CFG80211OS_ScanEnd(pAd, TRUE);
			return 0;
		}
	} else if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
		if (apIndex != WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Scan Request for AP i/f proceed for scanning\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Invaliad Ap index, i/f end Scan.\n");
			pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */
			CFG80211OS_ScanEnd(pAd, TRUE);
			return 0;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Invaliad interface type, i/f end Scan, interface type=%d.\n",
			pNdev->ieee80211_ptr->iftype);
		pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */
		CFG80211OS_ScanEnd(pAd, TRUE);
		return 0;
	}

	preq_wdev = RTMP_OS_NETDEV_GET_WDEV(pNdev);
	if (preq_wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"preq_wdev is NULL!!\n");
		return -EOPNOTSUPP;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"========================================================================\n");
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"80211> ==> %s(%d)\n", pNdev->name, pNdev->ieee80211_ptr->iftype);
	/* YF_TODO: record the scan_req per netdevice */
	pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */

	/* Iftype Check
	if (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_STATION) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"80211> DeviceType Not Support Scan ==> %d\n",
			pNdev->ieee80211_ptr->iftype);
		CFG80211OS_ScanEnd(pAd, TRUE);
		return -EOPNOTSUPP;
	} */

	/* Driver ICOTL Sanity Check */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "80211> Network is down!\n");
		CFG80211OS_ScanEnd(pAd, TRUE);
		return -ENETDOWN;
	}

	/*do dfs cac check for 5G band*/
#ifdef MT_DFS_SUPPORT
	if (wlan_config_get_ch_band(preq_wdev) == CMD_CH_BAND_5G) {
		struct DOT11_H *pDot11h = &pAd->Dot11_H;

		if (pDot11h && (pDot11h->ChannelMode == CHAN_SILENCE_MODE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"still in silence mode, can not trigger scan\n");
			CFG80211OS_ScanEnd(pAd, TRUE);
			return -EBUSY;
		}
	}
#endif

	if (!TakeChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_SCAN, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"TakeChannelOpCharge fail for SCAN!!\n");
		CFG80211OS_ScanEnd(pAd, TRUE);
		return -EBUSY;
	}

	if (RTMP_DRIVER_80211_SCAN(pAd, pNdev->ieee80211_ptr->iftype) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"\n\n\n\n\n80211> BUSY - SCANNING\n\n\n\n\n");
		CFG80211OS_ScanEnd(pAd, TRUE);
		ReleaseChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_SCAN);
		return -EBUSY;
	}

	if (pRequest->ie_len != 0) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> ExtraIEs Not Null in ProbeRequest from upper layer...\n");
		/* YF@20120321: Using Cfg80211_CB carry on pAd struct to overwirte the pWpsProbeReqIe. */
		RTMP_DRIVER_80211_SCAN_EXTRA_IE_SET(pAd);
	} else
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"80211> ExtraIEs Null in ProbeRequest from upper layer...\n");
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
		"80211> Num %d of SSID from upper layer...\n",	pRequest->n_ssids);

	/* Set Channel List for this Scan Action */
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
		"80211> [%d] Channels In ProbeRequest.\n",  pRequest->n_channels);

	if (pRequest->n_channels > 0) {
		INT32 *pChanList;
		UINT32  idx, ch_cnt = 0;
		UINT8 band;

		os_alloc_mem(NULL, (UCHAR **)&pChanList, sizeof(UINT32) * pRequest->n_channels);

		if (pChanList == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Alloc memory fail\n");
			CFG80211OS_ScanEnd(pAd, TRUE);
			ReleaseChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_SCAN);
			return -ENOMEM;
		}
		os_zero_mem((PVOID) pChanList, sizeof(UINT32) * (pRequest->n_channels));
		for (idx = 0; idx < pRequest->n_channels; idx++) {
			band = pRequest->channels[idx]->band;
			if ((WMODE_CAP_2G(preq_wdev->PhyMode) && band == NL80211_BAND_2GHZ) ||
				(WMODE_CAP_5G(preq_wdev->PhyMode) && band == NL80211_BAND_5GHZ) ||
				(WMODE_CAP_6G(preq_wdev->PhyMode) && band == NL80211_BAND_6GHZ)) {
				pChanList[ch_cnt++] =
					ieee80211_frequency_to_channel(pRequest->channels[idx]->center_freq);
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
					"%d,", pChanList[ch_cnt-1]);
			}
		}

		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG, "\n");
		RTMP_DRIVER_80211_SCAN_CHANNEL_LIST_SET(pAd, pChanList, ch_cnt);

		if (pChanList)
			os_free_mem(pChanList);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"no channel list for SCAN!!\n");
		CFG80211OS_ScanEnd(pAd, TRUE);
		ReleaseChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_SCAN);
		return -EINVAL;
	}

	memset(&scan_cmd, 0, sizeof(RT_CMD_STA_IOCTL_SCAN));
	memset(ssid, 0, sizeof(ssid));

	if (pRequest->n_ssids && pRequest->ssids) {
		scan_cmd.SsidLen = pRequest->ssids->ssid_len;
		if ((scan_cmd.SsidLen > 0) && (scan_cmd.SsidLen <= sizeof(ssid)))
			memcpy(ssid, pRequest->ssids->ssid, scan_cmd.SsidLen);
		scan_cmd.pSsid = ssid;
	}

	scan_cmd.duration = pRequest->duration;
	scan_cmd.ScanType = SCAN_ACTIVE;
	if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) {
		scan_cmd.StaIndex = staIndex;
		RTMP_DRIVER_80211_APCLI_SCAN(pAd, &scan_cmd);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
			"Into RTMP_DRIVER_80211_APCLI_SCAN\n");
	} else {
		scan_cmd.ApIndex = apIndex;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
			"Into RTMP_DRIVER_80211_AP_SCAN\n");
		RTMP_DRIVER_80211_AP_SCAN(pAd, &scan_cmd);
	}
	return 0;
}

static void CFG80211_OpsAbortScan(struct wiphy *pWiphy, struct wireless_dev *wdev)
{
	SCAN_CTRL *ScanCtrl = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct wifi_dev *preq_wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "80211> ==>\n");

	if (!pWiphy || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR, Invalid pointer: pWiphy = %p, wdev = %p\n", pWiphy, wdev);
		return;
	}

	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
	preq_wdev = RTMP_OS_NETDEV_GET_WDEV(wdev->netdev);
	if (preq_wdev && CFG80211DRV_OpsScanRunning(pAd)) {
		ScanCtrl = get_scan_ctrl_by_wdev(pAd, preq_wdev);
		if (ScanCtrl) {
			ScanCtrl->opAbortScan = TRUE;
			if (TakeChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_ABORT_SCAN, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
					"scan is aborted with success\n");
				ReleaseChannelOpChargeByBand(pAd, 0, CH_OP_OWNER_ABORT_SCAN);
			} else
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
					"fail to abort scan in time\n");
		}
	}
}

#ifdef CFG80211_FULL_OPS_SUPPORT
static int CFG80211_OpsChanSwitch(struct wiphy *wiphy, struct net_device *dev,
			     struct cfg80211_csa_settings *params)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_CHAN chaninfo;
	struct wifi_dev *pwifi_dev = NULL;
	struct cfg80211_chan_def *chandef;

	if (NULL == wiphy || NULL == dev || NULL == params) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR, Invalid parameter: pWiphy = %p;net_dev = %p; params = %p\n",
		wiphy, dev, params);
		return -EINVAL;
	}

	/*interface infos*/
	pwifi_dev = RTMP_OS_NETDEV_GET_WDEV(dev);
	if (pwifi_dev == NULL)
		return -ENODEV;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "[%s](%d):request infname: %s, mac:"MACSTR";\n",
		__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(dev),
		MAC2STR(pwifi_dev->if_addr));

	/*pad infos*/
	MAC80211_PAD_GET(pAd, wiphy);

	/*init data*/
	NdisZeroMemory(&chaninfo, sizeof(CMD_RTPRIV_IOCTL_80211_CHAN));

	/*get wireless device*/
	chaninfo.pWdev = dev->ieee80211_ptr;/*wireless device kernel struct*/

	/*channel*/
	chandef = &params->chandef;
	chaninfo.ChanId = ieee80211_frequency_to_channel(chandef->chan->center_freq);
	chaninfo.CenterChanId = ieee80211_frequency_to_channel(chandef->center_freq1);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"[%s](%d): Channel = %d, CenterChanId = %d\n",
		__func__, __LINE__, chaninfo.ChanId, chaninfo.CenterChanId);

	/*bw*/
	/*bw infos*/
	if (chandef->width == NL80211_CHAN_WIDTH_20_NOHT)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20_NOHT;
	else if (chandef->width == NL80211_CHAN_WIDTH_20)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20;
	else if (chandef->width == NL80211_CHAN_WIDTH_40)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_40;
#ifdef DOT11_VHT_AC
	else if (chandef->width == NL80211_CHAN_WIDTH_80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80;
	else if (chandef->width == NL80211_CHAN_WIDTH_80P80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80P80;
	else if (chandef->width == NL80211_CHAN_WIDTH_160)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_160;
#endif /*DOT11_VHT_AC*/
	else {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[%s](%d): ERROR! channel bandwidth: %d not support.\n",
			__func__, __LINE__, chandef->width);

		return -EOPNOTSUPP;
	};

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "[%s](%d): ChanType = %d\n",
		__func__, __LINE__, chaninfo.ChanType);

	if (RTMP_DRIVER_AP_80211_CHAN_SET(pAd, &chaninfo) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[%s](%d):ERROR! RTMP_DRIVER_AP_80211_CHAN_SET: FAIL\n", __func__, __LINE__);
		return -EOPNOTSUPP;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "80211> %s end.\n", __func__);

	return 0;
}

#endif

#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE

INT CFG80211_OpsChanWithSet(
	struct wiphy *wiphy,
	struct net_device *dev,
	struct cfg80211_chan_def *chandef) {
	struct _RTMP_ADAPTER *pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_CHAN chaninfo;
	struct wifi_dev *pwifi_dev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "80211> ==>\n");

	if (NULL == wiphy || NULL == dev || NULL == chandef) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR, Invalid parameter: pWiphy = %p;net_dev = %p; chandef = %p\n",
		wiphy, dev, chandef);
		return -EINVAL;
	}

	/*interface infos*/
	pwifi_dev = RTMP_OS_NETDEV_GET_WDEV(dev);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"request infname: %s, mac:"MACSTR";\n",
		RTMP_OS_NETDEV_GET_DEVNAME(dev), MAC2STR(pwifi_dev->if_addr));

	/*pad infos*/
	MAC80211_PAD_GET(pAd, wiphy);

	/*init data*/
	NdisZeroMemory(&chaninfo, sizeof(CMD_RTPRIV_IOCTL_80211_CHAN));

	/*get wireless device*/
	chaninfo.pWdev = dev->ieee80211_ptr;/*wireless device kernel struct*/

	/*channel*/
	chaninfo.ChanId = ieee80211_frequency_to_channel(chandef->chan->center_freq);
	chaninfo.CenterChanId = ieee80211_frequency_to_channel(chandef->center_freq1);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"Channel = %d, CenterChanId = %d\n", chaninfo.ChanId, chaninfo.CenterChanId);

	/*bw*/
	/*bw infos*/
	if (chandef->width == NL80211_CHAN_WIDTH_20_NOHT)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20_NOHT;
	else if (chandef->width == NL80211_CHAN_WIDTH_20)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20;
	else if (chandef->width == NL80211_CHAN_WIDTH_40)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_40;
#ifdef DOT11_VHT_AC
	else if (chandef->width == NL80211_CHAN_WIDTH_80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80;
	else if (chandef->width == NL80211_CHAN_WIDTH_80P80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80P80;
	else if (chandef->width == NL80211_CHAN_WIDTH_160)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_160;
#endif /*DOT11_VHT_AC*/
	else {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"ERROR! channel bandwidth: %d not support.\n", chandef->width);
		return -EOPNOTSUPP;
	};

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"ChanType = %d\n", chaninfo.ChanType);

	if (NDIS_STATUS_SUCCESS != RTMP_DRIVER_AP_80211_CHAN_SET(pAd, &chaninfo)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"ERROR! RTMP_DRIVER_AP_80211_CHAN_SET: FAIL\n");
		return -EOPNOTSUPP;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "80211> end.\n");
	return 0;
}

#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/

int CFG80211_OpsMcastRateSet(
	IN struct wiphy *wiphy,
	IN struct net_device *dev,
	IN int rate[NUM_NL80211_BANDS])
{
	struct _RTMP_ADAPTER *pAd;
	INT apidx, ret;
	UCHAR band, PhyMode, Mcs;
	union _HTTRANSMIT_SETTING transmit = {0};

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE, "80211> ==>\n");

	MAC80211_PAD_GET(pAd, wiphy);

	if (!dev) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"[ERROR]can't find wdev in MBSS.\n");
		return -EFAULT;
	}

	for (band = 0; band < NUM_NL80211_BANDS; band++) {
		if (rate[band] > 0)//find mcast rate
			break;
	}

	switch (band) {
	case NL80211_BAND_2GHZ:
		if (rate[band] <= 4) {
			PhyMode = MODE_CCK;
			Mcs = rate[band] - 1;//Mcs0/1/2/3 -> CCK_1M/2M/5.5M/11M
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)) {
				if (Mcs == 0)//Short Preamble Not support CCK_1M
					Mcs = Mcs + 1;
				Mcs |= 0x4;
			}
		} else {
			PhyMode = MODE_OFDM;
			Mcs = (rate[band] - 4) - 1;//Mcs0~7 -> OFDM_6M~54M
		}

		break;

	case NL80211_BAND_5GHZ:
	case NL80211_BAND_6GHZ:
		PhyMode = MODE_OFDM;
		Mcs = rate[band] - 1;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Error Mcast Rate input.\n");
		return -EINVAL;
	}

	transmit.field.MODE = PhyMode;
	transmit.field.MCS = Mcs;
	transmit.field.BW = BW_20;

	ret = RTMP_DRIVER_80211_MCAST_RATE_SET(pAd, (VOID *)&transmit, apidx);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "Not Support\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

int CFG80211_OpsColorChange(
	IN struct wiphy *wiphy,
	IN struct net_device *dev,
	IN struct cfg80211_color_change_settings *params)
{
	struct _RTMP_ADAPTER *pAd;
	INT apidx, ret;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_NOTICE, "80211> ==>\n");

	MAC80211_PAD_GET(pAd, wiphy);

	if (!dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
			"[ERROR]can't find wdev in MBSS.\n");
		return -EFAULT;
	}

	ret = RTMP_DRIVER_80211_BSS_COLOR_CHANGE(pAd, (VOID *)params, apidx);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "Not Support\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

int CFG80211_OpsFtIesUpate(
	IN struct wiphy *wiphy,
	IN struct net_device *dev,
	IN struct cfg80211_update_ft_ies_params *ftie)
{
	struct _RTMP_ADAPTER *pAd;
	INT apidx, ret;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "80211> ==>\n");

	MAC80211_PAD_GET(pAd, wiphy);

	if (!dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"[ERROR]can't find wdev in MBSS.\n");
		return -EFAULT;
	}

	/*
	* struct cfg80211_update_ft_ies_params {
	*	u16 md;			[MDID]
	*	const u8 *ie;	[RSN IE + MDIE + FTIE(with MIC field set to 0)
	*					+ RIC-Request (if present) + RSNXE (if present)]
	*	size_t ie_len;	[LEN]
	* };
	*/

	if (!ftie || !ftie->ie)
		return -EINVAL;

	ret = RTMP_DRIVER_80211_FT_IES_UPDATE(pAd, (VOID *)ftie, apidx);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Not Support\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)

/*
 * ========================================================================
 * Routine Description:
 *	Get information for a specific station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */

static BOOLEAN CFG80211_FILL_STA_FLAGS(
	struct mtk_nl80211_sta_flag drv_sta_flags,
	struct station_info *pstainfo)
{
	struct nl80211_sta_flag_update *sta_flags;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");

	if (NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
			"ERROR, Invalid data bw = NULL;\n");
		return FALSE;
	}

	pstainfo->filled |= BIT(NL80211_STA_INFO_STA_FLAGS);
	sta_flags = &pstainfo->sta_flags;

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_AUTHORIZED));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_SHORT_PREAMBLE));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_WME)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_WME);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_WME)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_WME);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_WME));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_MFP)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_MFP);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_MFP)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_MFP);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_MFP));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_AUTHENTICATED));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_TDLS_PEER);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_TDLS_PEER);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_TDLS_PEER));
		}
	}
	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_ASSOCIATED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_ASSOCIATED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_ASSOCIATED));
		}
	}

	return TRUE;
}

static BOOLEAN CFG80211_FILL_BSS_PARAM(
	pmtk_cfg_sta_bss_para pbss_info,
	struct station_info *pstainfo)
{

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");


	if (NULL == pbss_info || NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			"ERROR, pbss_info = %p, pstainfo = %p;\n",
		pbss_info, pstainfo);
		return FALSE;
	}


	pstainfo->filled |= BIT(NL80211_STA_INFO_BSS_PARAM);
	/*dtim and beacon interval*/
	pstainfo->bss_param.dtim_period = pbss_info->dtim_period;
	pstainfo->bss_param.beacon_interval = pbss_info->beacon_interval;

	if (pbss_info->flags & BSS_PARAM_FLAGS_CTS_PROT) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
					"\nBSS_PARAM_FLAGS_CTS_PROT: TRUE\n");
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_CTS_PROT;
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
					"\nBSS_PARAM_FLAGS_CTS_PROT: FALSE\n");
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_CTS_PROT;
	}

	if (pbss_info->flags & BSS_PARAM_FLAGS_SHORT_PREAMBLE) {
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	} else {
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	}

	if (pbss_info->flags & BSS_PARAM_FLAGS_SHORT_SLOT_TIME) {
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	} else {
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	}

	return TRUE;
}
#ifdef WIFI_IAP_BCN_STAT_FEATURE
static BOOLEAN CFG80211_FILL_BCN_PARAM(
	CMD_RTPRIV_IOCTL_80211_STA * pdrv_stainfo,
	struct station_info *pstainfo)
{

	if (NULL == pdrv_stainfo || NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"ERROR, pdrv_stainfo = %p, pstainfo = %p;\n",
		pdrv_stainfo, pstainfo);
		return FALSE;
	}

	/*beacon loss*/
	if (pdrv_stainfo->beacon_mask & BIT(NL80211_STA_INFO_BEACON_LOSS)) {
		pstainfo->filled |= BIT(NL80211_STA_INFO_BEACON_LOSS);
		pstainfo->beacon_loss_count = pdrv_stainfo->beacon_loss_count;
	} else {
		pstainfo->filled &= ~(BIT(NL80211_STA_INFO_BEACON_LOSS));
	}
	/*beacon rx*/
	if (pdrv_stainfo->beacon_mask & BIT(NL80211_STA_INFO_BEACON_RX)) {
		pstainfo->filled |= BIT(NL80211_STA_INFO_BEACON_RX);
		pstainfo->rx_beacon = (UINT64)pdrv_stainfo->rx_beacon;
		pstainfo->filled |= BIT(NL80211_STA_INFO_BEACON_SIGNAL_AVG);
		pstainfo->rx_beacon_signal_avg = pdrv_stainfo->rx_beacon_signal_avg;
	} else {
		pstainfo->filled &= ~(BIT(NL80211_STA_INFO_BEACON_RX));
		pstainfo->filled &= ~(BIT(NL80211_STA_INFO_BEACON_SIGNAL_AVG));
	}
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"rx_beacon = %llu, rx_loss = %u\n",
			pstainfo->rx_beacon, pstainfo->beacon_loss_count);

	return TRUE;
}
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

static INT CFG80211_FILL_BW(CHAR *bw)
{

	if (NULL == bw) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"ERROR, Invalid data bw = NULL;\n");
		return EINVAL;
	}

	switch (*bw) {
	case BW_5:
		*bw = RATE_INFO_BW_5;
		break;
	case BW_10:
		*bw = RATE_INFO_BW_10;
		break;
	case BW_20:
		*bw = RATE_INFO_BW_20;
		break;
	case BW_40:
		*bw = RATE_INFO_BW_40;
		break;
	case BW_80:
		*bw = RATE_INFO_BW_80;
		break;
	case BW_160:
	case BW_8080:
		*bw = RATE_INFO_BW_160;
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"default ERROR, Invalid data bw = %d;\n", *bw);
		break;
	}

	return 0;
}
static int CFG80211_STA_SET_SINFO(struct station_info *pSinfo, CMD_RTPRIV_IOCTL_80211_STA *pStaInfo)
{
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	UCHAR i;
#endif
	/* fill tx rx rate */
	pSinfo->txrate.flags = pStaInfo->txrate.flags;
	pSinfo->txrate.mcs = pStaInfo->txrate.mcs;
	pSinfo->txrate.legacy = pStaInfo->txrate.legacy;
	pSinfo->txrate.nss = pStaInfo->txrate.nss;
	pSinfo->txrate.bw = pStaInfo->txrate.bw;
	pSinfo->rxrate.flags = pStaInfo->rxrate.flags;
	pSinfo->rxrate.mcs = pStaInfo->rxrate.mcs;
	pSinfo->rxrate.legacy = pStaInfo->rxrate.legacy;
	pSinfo->rxrate.nss = pStaInfo->rxrate.nss;
	pSinfo->rxrate.bw = pStaInfo->rxrate.bw;
	CFG80211_FILL_BW(&pSinfo->txrate.bw);
	CFG80211_FILL_BW(&pSinfo->rxrate.bw);

	/* phymode bw mcs sanity check */
	if (pSinfo->txrate.flags & RATE_INFO_FLAGS_VHT_MCS)
		if (pSinfo->txrate.mcs > 9 || pSinfo->txrate.bw > RATE_INFO_BW_160)
			goto ERROR;
	if (pSinfo->rxrate.flags & RATE_INFO_FLAGS_VHT_MCS)
		if (pSinfo->rxrate.mcs > 9 || pSinfo->rxrate.bw > RATE_INFO_BW_160)
			goto ERROR;

	if (pSinfo->txrate.flags & RATE_INFO_FLAGS_HE_MCS)
		if (pSinfo->txrate.mcs > 11 || pSinfo->txrate.bw > RATE_INFO_BW_160)
			goto ERROR;
	if (pSinfo->rxrate.flags & RATE_INFO_FLAGS_HE_MCS)
		if (pSinfo->rxrate.mcs > 11 || pSinfo->rxrate.bw > RATE_INFO_BW_160)
			goto ERROR;

#ifdef IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ
	if (pSinfo->txrate.flags & RATE_INFO_FLAGS_EHT_MCS)
		if (pSinfo->txrate.mcs > 13 || pSinfo->txrate.bw > RATE_INFO_BW_320)
			goto ERROR;
	if (pSinfo->rxrate.flags & RATE_INFO_FLAGS_EHT_MCS)
		if (pSinfo->rxrate.mcs > 13 || pSinfo->rxrate.bw > RATE_INFO_BW_320)
			goto ERROR;
#endif /* IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ */


	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BITRATE);
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_BITRATE);

	/* fill signal */
	pSinfo->signal = pStaInfo->Signal;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_SIGNAL);

	/* fill ack signal */
	pSinfo->ack_signal = pStaInfo->ack_signal;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_ACK_SIGNAL);

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	/* fill chain signal */
	pSinfo->chains = pStaInfo->chains;
	for (i = 0; i < IEEE80211_MAX_CHAINS; i++)
		pSinfo->chain_signal[i] = pStaInfo->chain_signal[i];
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_CHAIN_SIGNAL);
#endif
	/*tx retries*/
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_RETRIES);
	pSinfo->tx_retries = (UINT32)pStaInfo->tx_retries;

	/*tx_failed*/
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_FAILED);
	pSinfo->tx_failed = (UINT32)pStaInfo->tx_failed;

	/* fill tx/rx count */

	pSinfo->rx_bytes = pStaInfo->rx_bytes;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_BYTES);

	pSinfo->tx_bytes = pStaInfo->tx_bytes;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BYTES);

	pSinfo->rx_packets = pStaInfo->rx_packets;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_PACKETS);

	pSinfo->tx_packets = pStaInfo->tx_packets;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_PACKETS);

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	/* fill tx duration */
	pSinfo->tx_duration = pStaInfo->tx_duration;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_DURATION);

	/* fill rx duration*/
	pSinfo->rx_duration = pStaInfo->rx_duration;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_DURATION);
#endif
	/* fill inactive time */
	pSinfo->inactive_time = pStaInfo->InactiveTime;
	pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_INACTIVE_TIME);

	/*fill stainfos*/
	CFG80211_FILL_STA_FLAGS(pStaInfo->sta_flags, pSinfo);

	if (pStaInfo->sta_flags.mask & BIT_ULL(NL80211_STA_FLAG_ASSOCIATED)) {

		/* fill connected time */
		pSinfo->connected_time = pStaInfo->connected_time;
		pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_CONNECTED_TIME);

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
		/* fill assoc at boottime */
		pSinfo->assoc_at = pStaInfo->assoc_at;
		pSinfo->filled |= BIT_ULL(NL80211_STA_INFO_ASSOC_AT_BOOTTIME);
#endif
		/*fill bss parameter*/
		CFG80211_FILL_BSS_PARAM(&pStaInfo->bss_param, pSinfo);
	}

	/*fill beacon statistics*/
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	CFG80211_FILL_BCN_PARAM(&StaInfo, pSinfo);
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
	return 0;

ERROR:
	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
		"Tx: pSinfo->flag=%d, pSinfo->mcs=%d, pSinfo->legacy=%d, pSinfo->nss=%d, pSinfo->bw=%d\n",
		pSinfo->txrate.flags, pSinfo->txrate.mcs, pSinfo->txrate.legacy, pSinfo->txrate.nss, pSinfo->txrate.bw);
	MTWF_DBG_NP(DBG_CAT_CFG, CATCFG_CMD, DBG_LVL_ERROR,
		"Rx: pSinfo->flag=%d, pSinfo->mcs=%d, pSinfo->legacy=%d, pSinfo->nss=%d, pSinfo->bw=%d\n",
		pSinfo->rxrate.flags, pSinfo->rxrate.mcs, pSinfo->rxrate.legacy, pSinfo->rxrate.nss, pSinfo->rxrate.bw);

	return -EINVAL;

}

/*
 * ========================================================================
 * Routine Description:
 *	Get information for a specific station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN const UINT8						*pMac,
	IN struct station_info				*pSinfo)
{
	struct _RTMP_ADAPTER *pAd;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;

	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");

	/* init */
	memset(pSinfo, 0, sizeof(*pSinfo));
	memset(&StaInfo, 0, sizeof(StaInfo));
	memcpy(StaInfo.MAC, pMac, 6);

	/* get sta information */
	if (RTMP_DRIVER_80211_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

	if (CFG80211_STA_SET_SINFO(pSinfo, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -EINVAL;

	return 0;
}

#define CFG_IS_VALID_MAC(addr) \
	((addr[0])|(addr[1])|(addr[2])|(addr[3])|(addr[4])|(addr[5]))

#ifdef CFG80211_FULL_OPS_SUPPORT
static int CFG80211_OpsSurveyGet(
	IN struct wiphy *pWiphy,
	IN struct net_device *dev,
	IN int idx,
	OUT struct survey_info *survey)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *chan;
	struct _RTMP_ADAPTER *pAd;
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_phy *mac_phy = NULL;
	CFG80211_BAND BandInfo;
	int ret = 0;

	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);

	sband = pWiphy->bands[NL80211_BAND_2GHZ];
	if (sband && idx >= sband->n_channels) {
		idx -= sband->n_channels;
		sband = NULL;
	}

	if (!sband)
		sband = pWiphy->bands[NL80211_BAND_5GHZ];

	if (sband && idx >= sband->n_channels)  {
		idx -= sband->n_channels;
		sband = NULL;
	}
	if (!sband)
		sband = pWiphy->bands[NL80211_BAND_6GHZ];

	if (!sband || idx >= sband->n_channels)
		return -ENOENT;

	chan = &sband->channels[idx];

	memset(survey, 0, sizeof(*survey));
	mac_dev = hc_get_mac_dev(pAd);
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy)
		return -ENETDOWN;

	OS_SEM_LOCK(&mac_phy->lock);
	if (!mac_phy->state) {
		OS_SEM_UNLOCK(&mac_phy->lock);
		return -ENETDOWN;
	}
	if (idx == 0)
		RTMP_UPDATE_CHANNEL_INFO(pAd, Channel2Index(pAd, mac_phy->chan));

	if (ieee80211_frequency_to_channel(chan->center_freq) == mac_phy->chan)
		survey->filled |= SURVEY_INFO_IN_USE;
	OS_SEM_UNLOCK(&mac_phy->lock);

	survey->filled |= SURVEY_INFO_TIME_BUSY;
	survey->filled |= SURVEY_INFO_NOISE_DBM;
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	survey->filled |= SURVEY_INFO_TIME_BSS_RX;
#endif
	survey->filled |=  SURVEY_INFO_TIME_TX | SURVEY_INFO_TIME_RX | SURVEY_INFO_TIME;

	survey->channel = chan;
	survey->time_busy = div_u64(pAd->ScanCtrl.busytime[idx], 1000);
	survey->time_rx = div_u64(pAd->ScanCtrl.rx_time[idx],  1000);
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	survey->time_bss_rx = div_u64(pAd->ScanCtrl.bss_rx_time[idx], 1000);
#endif
	survey->time_tx = div_u64(pAd->ScanCtrl.tx_time[idx], 1000);
	survey->noise = pAd->ScanCtrl.ch_AvgNF[idx];
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	survey->time = div_u64(pAd->ScanCtrl.active_time[idx], 1000);
#endif

	return ret;
}
#endif/*CFG80211_FULL_OPS_SUPPORT */
/*
 * ========================================================================
 * Routine Description:
 *	Get information for ap station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */

INT CFG80211_OpsAp_StaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN INT 								idx,
	IN UINT8							*pMac,
	IN struct station_info				*pSinfo)
{
	RTMP_ADAPTER  *pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;
	INT i = 0;
	UINT assoc_idx = 0;
	UCHAR is_find = FALSE;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct wifi_dev *inf_wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");

	if (NULL == pWiphy || NULL == pNdev || NULL == pSinfo) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
			"ERROR, Invalid data: pWiphy = %p; pNdev = %p; pSinfo = %p\n",
		pWiphy, pNdev, pSinfo);
		return -ENOENT;
	}

	MAC80211_PAD_GET(pAd, pWiphy);

	/*wifi dev interface*/
	inf_wdev = RTMP_OS_NETDEV_GET_WDEV(pNdev);
	if (!inf_wdev) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
			"ERROR, Invalid data: inf_wdev = %p\n",
		inf_wdev);
		return -ENOENT;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"request infname: %s, mac:"MACSTR";\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pNdev), MAC2STR(inf_wdev->if_addr));

	/*init data*/
	NdisZeroMemory(pMac, MAC_ADDR_LEN);
	NdisZeroMemory(pSinfo, sizeof(*pSinfo));
	NdisZeroMemory(&StaInfo, sizeof(StaInfo));

	/*from idx find the assoc_idx sta_mac*/
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = entry_get(pAd, i);
		if (pEntry && inf_wdev == pEntry->wdev && (pEntry->Sst == SST_ASSOC) &&
			CFG_IS_VALID_MAC(pEntry->Addr) && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
			if (assoc_idx == idx) {
				NdisMoveMemory(pMac, pEntry->Addr, MAC_ADDR_LEN);
				NdisMoveMemory(StaInfo.MAC, pMac, MAC_ADDR_LEN);
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO,
				"idx: %d, mac: "MACSTR";(TYPE:%x)\n",
				idx, MAC2STR(pMac), pEntry->EntryType);
				is_find = TRUE;
				break;
			}

			assoc_idx++;
		}
	}

	/*find check*/
	if (FALSE == is_find) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_WARN,
			"cann't find sta idx(%d) in sta table!\n", idx);
		return -ENOENT;
	}

	/*get ap stainfo*/
	if (RTMP_DRIVER_80211_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

	if (CFG80211_STA_SET_SINFO(pSinfo, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -EINVAL;
	return 0;
}

/*
 * ========================================================================
 * Routine Description:
 *	List all stations known, e.g. the AP on managed interfaces.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	Idx				-
 *	pMac			-
 *	pSinfo			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaDump(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN int								Idx,
	IN UINT8 * pMac,
	IN struct station_info				*pSinfo)
{
	struct _RTMP_ADAPTER *pAd;

	if (Idx != 0)
		return -ENOENT;

	if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"80211> ==> AP dump station\n");
		MAC80211_PAD_GET(pAd, pWiphy);

		if (CFG80211_OpsAp_StaGet(pWiphy, pNdev, Idx, pMac, pSinfo) == 0)
			return 0;
		else
			return -EINVAL;
	}

	else if (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"80211> ==> STA dump station\n");
		MAC80211_PAD_GET(pAd, pWiphy);

		if (CFG80211_OpsAp_StaGet(pWiphy, pNdev, Idx, pMac, pSinfo) == 0)
			return 0;
		else
			return -EINVAL;
	}

	return -EOPNOTSUPP;
} /* End of CFG80211_OpsStaDump */


/*
 * ========================================================================
 * Routine Description:
 *	Notify that wiphy parameters have changed.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	Changed			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsWiphyParamsSet(
	IN struct wiphy						*pWiphy,
	IN UINT32							Changed)
{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	if (Changed & WIPHY_PARAM_RTS_THRESHOLD) {
		RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(pAd, pWiphy->rts_threshold);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"80211> ==> rts_threshold(%d)\n", pWiphy->rts_threshold);
		return 0;
	} else if (Changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(pAd, pWiphy->frag_threshold);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"80211> ==> frag_threshold(%d)\n", pWiphy->frag_threshold);
		return 0;
	}
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	else if (Changed & WIPHY_PARAM_COVERAGE_CLASS) {
	UINT ack_time = 0;

	if (pWiphy->coverage_class > 255) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> ==>COVERAGE_threshold(%d) is invalid!\n",
			pWiphy->coverage_class);
		return -EOPNOTSUPP;
	}

	ack_time = (UINT) (pWiphy->coverage_class * 3);

	/* IEEE 802.11-2007 table 7-27*/
	if (0 == ack_time)
		ack_time = 1;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"80211> ==>distance or COVERAGE_threshold(%d), ack_time=%d us\n",
		pWiphy->coverage_class, ack_time);
	if (NDIS_STATUS_SUCCESS != RTMP_DRIVER_80211_ACK_THRESHOLD_ADD(pAd, ack_time)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"ERROR, SET ACK TIMEOUT FAIL!\n");
		return -EOPNOTSUPP;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
		"SET ACK TIMEOUT SUCCESS!\n");
	return 0;
}
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsWiphyParamsSet */


/*
 * ========================================================================
 * Routine Description:
 *	Add a key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	link_id			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pParams			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	pMacAddr will be NULL when adding a group key.
 * ========================================================================
 */
static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8 * pMacAddr,
	IN struct key_params				*pParams)
{
	struct _RTMP_ADAPTER *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;

	p80211CB = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> KeyIdx = %d\n", KeyIdx);

	if (pParams->key_len >= sizeof(KeyInfo.KeyBuf) || pParams->key_len < 0)
		return -EINVAL;

	/* End of if */
	/* init */
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	memcpy(KeyInfo.KeyBuf, pParams->key, pParams->key_len);
	KeyInfo.KeyBuf[pParams->key_len] = 0x00;
	KeyInfo.KeyId = KeyIdx;
	KeyInfo.bPairwise = Pairwise;
	KeyInfo.KeyLen = pParams->key_len;
#ifdef DOT11W_PMF_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION) &&
		(pParams->cipher == WLAN_CIPHER_SUITE_AES_CMAC)) {
		PRTMP_ADAPTER pad = (PRTMP_ADAPTER)pAd;
		struct wifi_dev *wdev = &pad->StaCfg[0].wdev;
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = GetAssociatedAPByWdev(pad, wdev);
		memcpy(KeyInfo.KeyBuf, pEntry->SecConfig.GTK, LEN_TK);
		memcpy(KeyInfo.KeyBuf + LEN_TK, pParams->key, pParams->key_len);
		KeyInfo.KeyLen = LEN_TK + pParams->key_len;
		KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
		KeyInfo.cipher = Ndis802_11AESEnable;
		KeyInfo.KeyId = pEntry->SecConfig.GroupKeyId;
	} else
#endif /* CONFIG_STA_SUPPORT */
#endif /*APCLI_CFG80211_SUPPORT */
#endif	/* DOT11W_PMF_SUPPORT */
		if (pParams->cipher == WLAN_CIPHER_SUITE_WEP40)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP40;
		else if (pParams->cipher == WLAN_CIPHER_SUITE_WEP104)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP104;
		else if ((pParams->cipher == WLAN_CIPHER_SUITE_TKIP) ||
			 (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;

			if (pParams->cipher == WLAN_CIPHER_SUITE_TKIP)
				KeyInfo.cipher = Ndis802_11TKIPEnable;
			else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)
				KeyInfo.cipher = Ndis802_11AESEnable;
#ifdef HOSTAPD_SUITEB_SUPPORT
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_GCMP_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11GCMP256Enable;
#endif
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_GCMP) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11GCMP128Enable;
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11CCMP256Enable;
		}

#ifdef DOT11W_PMF_SUPPORT
		else if (pParams->cipher == WLAN_CIPHER_SUITE_AES_CMAC) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_CMAC;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		}
#ifdef HOSTAPD_SUITEB_SUPPORT
		else if (pParams->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_GMAC256;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		}
#endif
		else if (pParams->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_128) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_GMAC128;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_BIP_CMAC_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_CMAC256;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		}
#endif /* DOT11W_PMF_SUPPORT */
		else
			return -ENOTSUPP;

	KeyInfo.pNetDev = pNdev;
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP_VLAN)) {
		if (pMacAddr) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"80211> ifname=%s KeyAdd STA("MACSTR") ==>\n",
				pNdev->name, MAC2STR(pMacAddr));
			NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
		} else if (KeyInfo.bPairwise == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"80211> ifname=%s KeyAdd GroupKey\n", pNdev->name);
		}

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> AP Key Add\n");
		RTMP_DRIVER_80211_AP_KEY_ADD(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> STA Key Add\n");
	RTMP_DRIVER_80211_STA_KEY_ADD(pAd, &KeyInfo);
#endif	/* CONFIG_STA_SUPPORT */
	}

	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Get information about the key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	link_id			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pCookie			-
 *	pCallback		-
 *
 * Return Value:
 *	0			- success
 *	-x			- fail
 *
 * Note:
 *	pMacAddr will be NULL when requesting information for a group key.
 *
 *	All pointers given to the pCallback function need not be valid after
 *	it returns.
 *
 *	This function should return an error if it is not possible to
 *	retrieve the key, -ENOENT if it doesn't exist.
 * ========================================================================
 */
static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *pCookie,
	struct key_params *key_params))
{
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev = NULL;
	struct  key_params key_params;
	UCHAR tx_tsc[LEN_WPA_TSC] = {0};
	UINT32 pn_type_mask;
	UINT32 apidx;

	if (!pNdev)
		return -ENODEV;
	pAd = RTMP_OS_NETDEV_GET_PRIV(pNdev);
	memset(&key_params, 0, sizeof(key_params));

	NdisZeroMemory(&tx_tsc, sizeof(tx_tsc));
	pn_type_mask = TSC_TYPE_BIGTK_PN_MASK;
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNdev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS\n");
		return -ENODEV;
	}
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	AsicGetTxTsc(pAd, wdev, pn_type_mask, tx_tsc);
#ifdef BCN_PROTECTION_SUPPORT
	wdev->SecConfig.Bigtk_Reinstall = 1;
#endif
	if (!Pairwise && (KeyIdx == 6 || KeyIdx == 7)) {
		key_params.seq = tx_tsc;
		key_params.seq_len = LEN_WPA_TSC;
		pCallback(pCookie, &key_params);
		return 0;
	} else {
		return -ENOENT;
	}
}


/*
 * ========================================================================
 * Routine Description:
 *	Remove a key given the pMacAddr (NULL for a group key) and KeyIdx.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	link_id			-
 *	KeyIdx			-
 *	pMacAddr		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	return -ENOENT if the key doesn't exist.
 * ========================================================================
 */
static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr)
{
	struct _RTMP_ADAPTER *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;
	struct wifi_dev *wdev = NULL;

	p80211CB = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");

	MAC80211_PAD_GET(pAd, pWiphy);
	p80211CB = pAd->pCfg80211_CB;
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	if (pMacAddr) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"80211> KeyDel STA("MACSTR") ==>\n", MAC2STR(pMacAddr));
		NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
	}
	wdev = &pAd->StaCfg[0].wdev;
	KeyInfo.KeyId = KeyIdx;
	KeyInfo.pNetDev = pNdev;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"80211> KeyDel isPairwise %d\n", Pairwise);
	KeyInfo.bPairwise = Pairwise;
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> AP Key Del\n");
		RTMP_DRIVER_80211_AP_KEY_DEL(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> STA Key Del\n");

		if (pMacAddr && wdev) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"80211> STA Key Del -- DISCONNECT\n");
			RTMPWPARemoveAllKeys(pAd, wdev);
		}
	}

	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Set the default key on an interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	link_id			-
 *	KeyIdx			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx,
	IN bool								Unicast,
	IN bool								Multicast)
{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"80211> Default KeyIdx = %d\n", KeyIdx);
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(pAd, pNdev, KeyIdx);
	else
#endif /* CONFIG_AP_SUPPORT */
		RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(pAd, KeyIdx);

	return 0;
} /* End of CFG80211_OpsKeyDefaultSet */
#ifdef BCN_PROTECTION_SUPPORT
static int CFG80211_OpsDefaultBeaconKeySet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx)
{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"80211> Default KeyIdx = %d\n", KeyIdx);
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_BEACON_KEY_DEFAULT_SET(pAd, KeyIdx);
#endif /* CONFIG_AP_SUPPORT */

	return 0;
} /* End of CFG80211_OpsKeyDefaultSet */
#endif /*BCN_PROTECTION_SUPPORT*/
#ifdef DOT11W_PMF_SUPPORT
/*
 *========================================================================
 *Routine Description:
 *	Set the default management key on an interface.

 *Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *
 *Return Value:
 *	0				- success
 *	-x				- fail
 *
 *Note:
========================================================================
*/
static int CFG80211_OpsMgmtKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	IN int						link_id,
#endif /* CFG_CFG80211_VERSION */
	IN UINT8							KeyIdx)

{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
		"80211> Default Mgmt KeyIdx = %d\n", KeyIdx);

#ifdef CONFIG_AP_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_MGMT_SET(pAd, pNdev, KeyIdx);
#endif

	return 0;
} /* End of CFG80211_OpsMgmtKeyDefaultSet */
#endif /*DOT11W_PMF_SUPPORT*/



/*
 * ========================================================================
 * Routine Description:
 *	Connect to the ESS with the specified parameters. When connected,
 *	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
 *	If the connection fails for some reason, call cfg80211_connect_result()
 *	with the status from the AP.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pSme			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *
 *	You must use "iw ra0 connect xxx", then "iw ra0 disconnect";
 *	You can not use "iw ra0 connect xxx" twice without disconnect;
 *	Or you will suffer "command failed: Operation already in progress (-114)".
 *
 *	You must support add_key and set_default_key function;
 *	Or kernel will crash without any error message in linux 2.6.32.
 *
 *
 * struct cfg80211_connect_params - Connection parameters
 *
 * This structure provides information needed to complete IEEE 802.11
 * authentication and association.
 *
 *  @channel: The channel to use or %NULL if not specified (auto-select based
 *	on scan results)
 *  @bssid: The AP BSSID or %NULL if not specified (auto-select based on scan
 *	results)
 * @ssid: SSID
 *  @ssid_len: Length of ssid in octets
 *  @auth_type: Authentication type (algorithm)
 *
 * @ie: IEs for association request
 * @ie_len: Length of assoc_ie in octets
 *
 * @privacy: indicates whether privacy-enabled APs should be used
 * @crypto: crypto settings
 * @key_len: length of WEP key for shared key authentication
 * @key_idx: index of WEP key for shared key authentication
 * @key: WEP key for shared key authentication
 * ========================================================================
 */
static int CFG80211_OpsConnect(
	IN struct wiphy				*pWiphy,
	IN struct net_device			*pNdev,
	IN struct cfg80211_connect_params	*pSme)
{
#ifdef CONFIG_STA_SUPPORT
	struct _RTMP_ADAPTER *pAd;
	CMD_RTPRIV_IOCTL_80211_CONNECT ConnInfo;
	CMD_RTPRIV_IOCTL_80211_ASSOC_IE AssocIe;
	PEID_STRUCT wsc_ie = NULL;
	UINT wps_oui = 0x0050F2;
	UCHAR wps_oui_type = 4;

	struct cfg80211_crypto_settings *crypto = &pSme->crypto;
	// INT32 Chan = -1;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	// if (pSme->Channel != NULL)
	//	Chan = ieee80211_frequency_to_channel(pChannel->center_freq);

	memset(&ConnInfo, 0, sizeof(ConnInfo));

	if (crypto->wpa_versions & NL80211_WPA_VERSION_1)
		ConnInfo.WpaVer = 1;
	if (crypto->wpa_versions & NL80211_WPA_VERSION_2)
		ConnInfo.WpaVer = 2;
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"WpaVer: %02x\n", ConnInfo.WpaVer);

	switch (pSme->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		ConnInfo.AuthType = Ndis802_11AuthModeOpen;
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
		ConnInfo.AuthType = Ndis802_11AuthModeShared;
		break;
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	case NL80211_AUTHTYPE_SAE:
		ConnInfo.AuthType = Ndis802_11AuthModeSAE;
		break;
#endif /* DOT11_SAE_SUPPORT && SUPP_SAE_SUPPORT */
	default:
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
			"Unknown auth_type: %d\n", pSme->auth_type);
		ConnInfo.AuthType = Ndis802_11AuthModeAutoSwitch;
	}
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"AuthType: %d\n", ConnInfo.AuthType);

	// wpa_supplicant sets 1 akm-suite and pairwise-cipher
	ConnInfo.AkmSuite = crypto->akm_suites[0];
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"AkmSuite: %08x\n", ConnInfo.AkmSuite);

	ConnInfo.Pairwise = crypto->ciphers_pairwise[0];
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"Pairwise: %08x\n", ConnInfo.Pairwise);

	ConnInfo.Group = crypto->cipher_group;
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"Group: %08x\n", ConnInfo.Group);

	ConnInfo.pKey = (UINT8 *) pSme->key;
	ConnInfo.KeyLen = pSme->key_len;
	ConnInfo.KeyIdx = pSme->key_idx;
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"KeyIdx=%hu, KeyLen=%hu\n", ConnInfo.KeyIdx, ConnInfo.KeyLen);

	ConnInfo.pSsid = (UINT8 *) pSme->ssid;
	ConnInfo.SsidLen = pSme->ssid_len;

	if (pSme->bssid != NULL) {
		ConnInfo.pBssid = (UINT8 *) pSme->bssid;
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
			"Bssid: " MACSTR "\n", MAC2STR(ConnInfo.pBssid));
	}

#ifdef DOT11W_PMF_SUPPORT
	ConnInfo.mfpc = pSme->mfp != NL80211_MFP_NO;
	ConnInfo.mfpr = pSme->mfp == NL80211_MFP_REQUIRED;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
		"mfpc=%d, mfpr=%d\n", ConnInfo.mfpc, ConnInfo.mfpr);
#endif /* DOT11W_PMF_SUPPORT */

	/* YF@20120328: Reset to default */
	ConnInfo.bWpsConnection = FALSE;
	ConnInfo.pNetDev = pNdev;

	/* hex_dump("AssocInfo:", pSme->ie, pSme->ie_len); */
	/* YF@20120328: Use SIOCSIWGENIE to make out the WPA/WPS IEs in AssocReq. */
	memset(&AssocIe, 0, sizeof(AssocIe));
	AssocIe.pNetDev = pNdev;
	AssocIe.ie = (UINT8 *)pSme->ie;
	AssocIe.ie_len = pSme->ie_len;
	RTMP_DRIVER_80211_STA_ASSSOC_IE_SET(pAd, &AssocIe, pNdev->ieee80211_ptr->iftype);

	wsc_ie = (PEID_STRUCT)cfg80211_find_vendor_ie(wps_oui, wps_oui_type, AssocIe.ie, AssocIe.ie_len);
	if (wsc_ie) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_IE_INFO, DBG_LVL_INFO,
			"80211> find the wsc ie.\n");
		ConnInfo.bWpsConnection = TRUE;
	}
	RTMP_DRIVER_80211_CONNECT(pAd, &ConnInfo, pNdev->ieee80211_ptr->iftype);
#endif/*CONFIG_STA_SUPPORT*/
	return 0;
} /* End of CFG80211_OpsConnect */


/*
 * ========================================================================
 * Routine Description:
 *	Disconnect from the BSS/ESS.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	ReasonCode		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *========================================================================
 */
static int CFG80211_OpsDisconnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT16								ReasonCode)
{
#ifdef CONFIG_STA_SUPPORT
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO, "80211> ==>\n");
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"80211> ReasonCode = %d\n", ReasonCode);
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev);

	cfg80211_disconnected(pNdev, ReasonCode, NULL, 0, TRUE, GFP_KERNEL);

#endif /*CONFIG_STA_SUPPORT*/
	return 0;
}

#ifdef APCLI_CFG80211_SUPPORT
static int CFG80211_ExternalAuth(IN struct wiphy *pWiphy, IN struct net_device *pNdev,
				IN struct cfg80211_external_auth_params *params)
{
	INT staidx;
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *pwdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_NOTICE, "80211> ==>\n");
	if (!pWiphy || !pNdev || !params) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
			"pWiphy:%p, pNdev:%p, params:%p\n", pWiphy, pNdev, params);
		return 0;
	}

	if (pNdev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_STATION) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
			"iftype:%d\n", pNdev->ieee80211_ptr->iftype);
		return 0;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"action:%d, bssid:%pM, ssid:%s, ssid_len:%d\n",
		params->action, params->bssid, params->ssid.ssid, params->ssid.ssid_len);
#if defined(BACKPORT_NOSTDINC)
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_INFO,
		"key_mgmt_suite:0x%x, status:0x%x, pmkid:%p\n",
		params->key_mgmt_suite, params->status, params->pmkid);
#endif /* BACKPORT_NOSTDINC */
	MAC80211_PAD_GET(pAd, pWiphy);

	staidx = CFG80211_FindStaIdxByNetDevice(pAd, pNdev);
	if (staidx == WDEV_NOT_FOUND) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_STA, DBG_LVL_ERROR,
			"Sta Interface not found\n");
		return 0;
	}
	pwdev = &pAd->StaCfg[staidx].wdev;

	if (pwdev) {
		auth_fsm_state_transition(pwdev, AUTH_FSM_IDLE, __func__);
		cntl_auth_assoc_conf(pwdev, CNTL_MLME_AUTH_CONF, params->status);
	}
	return 0;
}
#endif

#endif /* LINUX_VERSION_CODE */


#ifdef RFKILL_HW_SUPPORT
static int CFG80211_OpsRFKill(
	IN struct wiphy						*pWiphy)
{
	struct _RTMP_ADAPTER		*pAd;
	BOOLEAN		active;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_RFKILL(pAd, &active);
	wiphy_rfkill_set_hw_state(pWiphy, !active);
	return active;
}


VOID CFG80211_RFKillStatusUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN active)
{
	struct wiphy *pWiphy;
	CFG80211_CB *pCfg80211_CB;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");
	pCfg80211_CB = pAd->pCfg80211_CB;
	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	wiphy_rfkill_set_hw_state(pWiphy, !active);
}
#endif /* RFKILL_HW_SUPPORT */

#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)
/*
 * ========================================================================
 * Routine Description:
 *	Cache a PMKID for a BSSID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	This is mostly useful for fullmac devices running firmwares capable of
 *	generating the (re) association RSN IE.
 *	It allows for faster roaming between WPA2 BSSIDs.
 * ========================================================================
 */
static int CFG80211_OpsPmksaSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;

#ifdef CONFIG_AP_SUPPORT
	if (pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) {
		/*  NL80211_EXT_FEATURE_AP_PMKSA_CACHING */
		/* no support */
		return 0;
	}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	if (pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION) {
		RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;

		pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_ADD;
		pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
		pIoctlPmaSa->pPmkid = (UCHAR *)pPmksa->pmkid;
		pIoctlPmaSa->pNetDev = pNdev;
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
			"80211> STA PMK Cache Set!\n");
		RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
	}
#endif

	return 0;
} /* End of CFG80211_OpsPmksaSet */


/*
 * ========================================================================
 * Routine Description:
 *	Delete a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
/*
 * ========================================================================
 * Routine Description:
 *	Flush a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPmksaFlush(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev)
{
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	struct _RTMP_ADAPTER *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_FLUSH;
	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */
#endif
	return 0;
} /* End of CFG80211_OpsPmksaFlush */
#endif /* LINUX_VERSION_CODE */

static int CFG80211_OpsRemainOnChannel(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *pWdev,
	IN struct ieee80211_channel *pChan,
	IN unsigned int duration,
	OUT u64 *cookie)
{
	struct _RTMP_ADAPTER *pAd;
	UINT32 ChanId;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	u32 rndCookie;
	INT ChannelType = RT_CMD_80211_CHANTYPE_HT20;
	struct net_device *dev = NULL;

	dev = pWdev->netdev;
	rndCookie = MtRandom32() | 1;
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	/*CFG_TODO: Shall check channel type*/
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"CH = %d, Type = %d, duration = %d, cookie=%d\n",
		ChanId, ChannelType, duration, rndCookie);
	/* init */
	*cookie = rndCookie;
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	ChanInfo.IfType = dev->ieee80211_ptr->iftype;
	ChanInfo.ChanType = ChannelType;
	ChanInfo.chan = pChan;
	ChanInfo.cookie = rndCookie;
	ChanInfo.pWdev = pWdev;
	/* set channel */
	RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(pAd, &ChanInfo, duration);
	return 0;
}


#ifdef BACKPORT_NOSTDINC /* OpenWRT Backport Wireless */
static void
CFG80211_OpsUpdate_mgmt_frame_registrations(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     struct mgmt_frame_regs *upd)
{
	u16 new_mask = upd->interface_stypes;
	u16 old_mask = 0;
	struct _RTMP_ADAPTER *pAd;
	static const struct {
		u16 mask;
	} updates[] = {
		{
			.mask = BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
				BIT(IEEE80211_STYPE_ASSOC_REQ >> 4),
		},
		{
			.mask = BIT(IEEE80211_STYPE_AUTH >> 4),
		},
		{
			.mask = BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
		},
		{
			.mask = BIT(IEEE80211_STYPE_ACTION >> 4),
		},
	};
	unsigned int i;
	struct net_device *dev = wdev->netdev;
#if defined(APCLI_CFG80211_SUPPORT)
	BOOLEAN isAp = TRUE;

	if (wdev->iftype == NL80211_IFTYPE_STATION)
		isAp = FALSE;
#endif /* APCLI_CFG80211_SUPPORT */
	MAC80211_PAD_GET_NO_RV(pAd, wdev->wiphy);


#if defined(APCLI_CFG80211_SUPPORT)
	old_mask = isAp ? (pAd->cfg80211_ctrl.mgmt_bit_mask) : (pAd->cfg80211_ctrl.mgmt_bit_mask_sta);
#else
	old_mask = pAd->cfg80211_ctrl.mgmt_bit_mask;
#endif /* APCLI_CFG80211_SUPPORT */

	if (new_mask == old_mask) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
			"mgmt reg mask does not change, no need to update\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(updates); i++) {
		u16 mask = updates[i].mask;
		bool reg;

		if (!(new_mask & mask) == !(old_mask & mask)) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
				"mgmt frame: 0x%x REG state does not change\n", mask);
			continue;
		}

		reg = new_mask & mask;

		if (mask == BIT(IEEE80211_STYPE_PROBE_REQ >> 4)) {
			RTMP_DRIVER_80211_MGMT_FRAME_REG(pAd, dev, reg);
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
				"PROBE frame was %sregistered for intf %s\n",
				(reg ? ("") : ("un")), dev->name);
		} else if (mask == BIT(IEEE80211_STYPE_ACTION >> 4)) {
			RTMP_DRIVER_80211_ACTION_FRAME_REG(pAd, dev, reg);
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_NOTICE,
				"ACTION frame was %sregistered for intf %s\n",
				(reg ? ("") : ("un")), dev->name);
		} else {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
				"UNSUPPORT mgmt frame type mask: 0x%x for intf %s\n",
				mask, dev->name);
		}
	}
#if defined(APCLI_CFG80211_SUPPORT)
		if (isAp)
			pAd->cfg80211_ctrl.mgmt_bit_mask = new_mask;
		else
			pAd->cfg80211_ctrl.mgmt_bit_mask_sta = new_mask;
#else
		pAd->cfg80211_ctrl.mgmt_bit_mask = new_mask;
#endif /* APCLI_CFG80211_SUPPORT */
}
#else /* BACKPORT_NOSTDINC */
static void CFG80211_OpsMgmtFrameRegister(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev,
	UINT16 frame_type, bool reg)
{
	VOID *pAd;
	struct net_device *dev = NULL;
	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG, "80211> ==>\n");
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_DEBUG,
		"frame_type = %x, req = %d , (%d)\n", frame_type, reg,  dev->ieee80211_ptr->iftype);

	if (frame_type == IEEE80211_STYPE_PROBE_REQ)
		RTMP_DRIVER_80211_MGMT_FRAME_REG(pAd, dev, reg);
	else if (frame_type == IEEE80211_STYPE_ACTION)
		RTMP_DRIVER_80211_ACTION_FRAME_REG(pAd, dev, reg);
	else
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"Unknown frame_type = %x, req = %d\n", frame_type, reg);
}
#endif /* !BACKPORT_NOSTDINC */

static int CFG80211_OpsMgmtTx(
    IN struct wiphy *pWiphy,
    IN struct wireless_dev *wdev,
	IN struct cfg80211_mgmt_tx_params *params,
    IN u64 *pCookie)
{
    struct _RTMP_ADAPTER *pAd;
    UINT32 ChanId;
	struct net_device *dev = NULL;
#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
	struct wifi_dev *Wdev = NULL;
	UCHAR supp_seq_num;
	UCHAR ifIndex;
#endif
	struct ieee80211_channel *pChan = params->chan;
	/*	bool Offchan = params->offchan; */
	/*      unsigned int Wait = params->wait; */
	const u8 *pBuf = params->buf;
	size_t Len = params->len;
	bool no_cck = params->no_cck;
   /* 	bool done_wait_for_ack = params->dont_wait_for_ack; */

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_DEBUG, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
	if (wdev == NULL)
		return 0;
	Wdev = RTMP_OS_NETDEV_GET_WDEV(wdev->netdev);
	if (Wdev == NULL)
		return 0;
	ifIndex = Wdev->func_idx;
	supp_seq_num = Wdev->SecConfig.wpa_supp_seq_num;
#endif
    /* get channel number */
	if (pChan == NULL)
		ChanId = 0;
	else
		ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_DEBUG, "80211> Mgmt Channel = %d\n", ChanId);

	/* Send the Frame with basic rate 6 */
    if (no_cck)
		; /*pAd->isCfgDeviceInP2p = TRUE; */

	*pCookie = COOkIE_VALUE;
#ifndef HOSTAPD_AUTO_CH_SUPPORT
    if (ChanId != 0)
	RTMP_DRIVER_80211_CHANNEL_LOCK(pAd, ChanId);
#endif

#ifdef APCLI_CFG80211_SUPPORT
	if (wdev && wdev->iftype == NL80211_IFTYPE_STATION) {
		UINT frmLen = Len;
		UCHAR	*pFrmBuf = NULL;
		struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)pBuf;
		POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
		PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

		if (mgmt == NULL)
			return 0;
		if (mgmt->u.action.category == CATEGORY_WNM) {
			os_alloc_mem(NULL, (UCHAR **)&pFrmBuf, frmLen);
			if (pFrmBuf == NULL) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"allocate memory failed.\n");
				return 0;
			}

			MTWF_DBG_NP(DBG_CAT_TX, CATTX_MGMT, DBG_LVL_DEBUG,
					"[cfg80211] send btm rsp to "MACSTR".\n", MAC2STR(mgmt->da));
			Set_AutoRoaming_Proc(pAd, "1");
			pStaCfg->isReassoc = TRUE;
			NdisCopyMemory(pFrmBuf, pBuf, frmLen);
			MiniportMMRequest(pAd, 0, pFrmBuf, frmLen, NULL);
			os_free_mem(pFrmBuf);
			return 0;
		}

		if (mgmt && ((ieee80211_is_auth(mgmt->frame_control) && mgmt->u.auth.auth_alg == AUTH_MODE_SAE)
			|| (ieee80211_is_action(mgmt->frame_control)))) {
			/*add it to support DPP R1 action frame*/
			os_alloc_mem(NULL, (UCHAR **)&pFrmBuf, frmLen);
			if (pFrmBuf == NULL) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"allocate memory failed.\n");
				return 0;
			}

			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_NOTICE,
				"[cfg80211] send auth req to "MACSTR
				" (alg=%d, transaction=%d, status=%d)...\n",
				MAC2STR(mgmt->da), mgmt->u.auth.auth_alg,
				mgmt->u.auth.auth_transaction, mgmt->u.auth.status_code);

#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
		if (pAd->StaCfg[ifIndex].apcli_random_mac_support)
			UniCmdStaSNSet(pAd, 0, supp_seq_num);
#endif

			NdisCopyMemory(pFrmBuf, pBuf, frmLen);
			MiniportMMRequest(pAd, 0, pFrmBuf, frmLen, NULL);
			/*dpp action frame need driver indicate TX status event*/
			if (ieee80211_is_action(mgmt->frame_control)) {
				int idx = 0;

				for (idx = 0; idx < MAX_MULTI_STA; idx++) {
					struct wifi_dev *wdev = &pAd->StaCfg[idx].wdev;

					if (wdev->if_dev != NULL) {
						if (RTMPEqualMemory(mgmt->sa, wdev->if_dev->dev_addr, MAC_ADDR_LEN)) {
							CFG80211OS_TxStatus(wdev->if_dev, COOkIE_VALUE, pFrmBuf, frmLen, 1);
							break;
						}
					}
				}
			}
			os_free_mem(pFrmBuf);
		}
	} else
#endif
		RTMP_DRIVER_80211_MGMT_FRAME_SEND(pAd, (VOID *)pBuf, Len);

	/* Mark it for using Supplicant-Based off-channel wait
		if (Offchan)
			RTMP_DRIVER_80211_CHANNEL_RESTORE(pAd);
	 */

    return 0;
}

static int CFG80211_OpsTxCancelWait(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *wdev,
	u64 cookie)
{
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR, "80211> ==>\n");
	return 0;
}

static int CFG80211_OpsCancelRemainOnChannel(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev,
	u64 cookie)
{
	struct _RTMP_ADAPTER *pAd;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);
	/* It cause the Supplicant-based OffChannel Hang */
	RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(pAd, cookie);
	return 0;
}

#ifdef CONFIG_AP_SUPPORT
extern const struct ieee80211_rate Cfg80211_SupRate[12];
#define OFDM_RATE_IDX_OFFSET 4

static int CFG80211_OpsStartAp(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_ap_settings *settings)
{
	struct _RTMP_ADAPTER *pAd;
	struct CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	struct CMD_RTPRIV_IOCTL_80211_BEACON_CFG cfg;
	UCHAR *beacon_head_buf = NULL, *beacon_tail_buf = NULL;
	INT apidx;
	struct wifi_dev *pWdev = NULL;
	int ret = 0;
	UCHAR rate_idx = 0;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE, "80211> ==>\n");

	MAC80211_PAD_GET(pAd, pWiphy);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		ret = -EINVAL;
		goto end;
	}

	if (settings->chandef.chan == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"failed - [ERROR]chandef.chan NULL.\n");
		ret = -EINVAL;
		goto end;
	}

	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	NdisZeroMemory(&bcn, sizeof(bcn));
	NdisZeroMemory(&cfg, sizeof(cfg));
	/* update info into bcn structure */
	bcn.apidx = apidx;
	bcn.pNetDev = netdev;

#ifdef DOT11_EHT_BE
	MBSS_Reconfig_Init(pAd, &pAd->ApCfg.MBSSID[apidx]);
#endif

#if KERNEL_VERSION(5, 1, 0) <= LINUX_VERSION_CODE
	if (settings->flags & AP_SETTINGS_DOT11VMBSSID_SUPPORT) {
		/* Currently should be only for AX Cert. Case */
		pAd->ApCfg.dot11v_mbssid_bitmap |= 1 << apidx;

		if (apidx == 0) {
			pAd->ApCfg.MBSSID[apidx].mbss_11v.mbss_11v_enable = MBSS_11V_T;
			pAd->ApCfg.dot11v_BssidNum[apidx] = pAd->ApCfg.BssidNum;
			pAd->ApCfg.dot11v_max_indicator[apidx] =
					mbss_11v_bssid_num_to_max_indicator(pAd->ApCfg.BssidNum);
		} else {
			pAd->ApCfg.MBSSID[apidx].mbss_11v.mbss_11v_enable = MBSS_11V_NT;
		}
		pAd->ApCfg.MBSSID[apidx].mbss_11v.mbss_11v_t_bss_idx = 0;
		entrytb_aid_bitmap_reserve(pAd, &pAd->aid_info);
	}
#endif
	if (settings->beacon.head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, settings->beacon.head_len);
		if (beacon_head_buf == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"memory alloc failed\n");
			ret = -ENOMEM;
			goto end;
		}
		NdisCopyMemory(beacon_head_buf, settings->beacon.head, settings->beacon.head_len);
	}

	if (settings->beacon.tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, settings->beacon.tail_len);
		if (beacon_tail_buf == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"memory alloc failed\n");
			ret = -ENOMEM;
			goto end;
		}
		NdisCopyMemory(beacon_tail_buf, settings->beacon.tail, settings->beacon.tail_len);
	}

	bcn.beacon_head_len = settings->beacon.head_len;
	bcn.beacon_tail_len = settings->beacon.tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	cfg.dtim_period = settings->dtim_period;
	cfg.interval = settings->beacon_interval;
	cfg.ssid_len = settings->ssid_len;
	cfg.privacy = settings->privacy;
	cfg.band = settings->chandef.chan->band;
	if (cfg.band < 0 || cfg.band >= NUM_NL80211_BANDS) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"invalid band:%d\n", cfg.band);
		ret = -EINVAL;
		goto end;
	}

	if (settings->beacon_rate.control[cfg.band].legacy > 0) {
		UINT32 arr_size = ARRAY_SIZE(Cfg80211_SupRate);

		rate_idx = ffs(settings->beacon_rate.control[cfg.band].legacy) - 1;
		if (rate_idx >= arr_size) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"invalid rate_idx:%d\n", rate_idx);
			ret = -EINVAL;
			goto end;
		}

		if (cfg.band == NL80211_BAND_2GHZ) {
			cfg.bitrate = Cfg80211_SupRate[rate_idx].bitrate;
		} else {
			if (rate_idx + OFDM_RATE_IDX_OFFSET >= arr_size) {
				MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"invalid rate_idx:%d\n", rate_idx);
				ret = -EINVAL;
				goto end;
			}

			cfg.bitrate = Cfg80211_SupRate[rate_idx + OFDM_RATE_IDX_OFFSET].bitrate;
		}
	}

	if (settings->crypto.akm_suites[0] == WLAN_AKM_SUITE_8021X) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
			"80211> This is a 1X wdev\n");
		pWdev->IsCFG1xWdev = TRUE;
	} else {
		pWdev->IsCFG1xWdev = FALSE;
	}

	NdisZeroMemory(cfg.ssid, MAX_LEN_OF_SSID);
	if (settings->ssid_len <= 32)
		NdisCopyMemory(cfg.ssid, settings->ssid, settings->ssid_len);
	cfg.auth_type = settings->auth_type;

	cfg.hidden_ssid = settings->hidden_ssid;

#ifdef CONFIG_6G_SUPPORT
#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)
	if ((cfg.band == NL80211_BAND_6GHZ) && (settings->unsol_bcast_probe_resp.interval > 0))
		in_band_discovery_update_conf(pWdev, UNSOLICIT_TX_PROBE_RSP,
			settings->unsol_bcast_probe_resp.interval, UNSOLICIT_TXMODE_NON_HT, TRUE);
#endif
#endif
	if (pAd->CommonCfg.wifi_cert
		&& pAd->ApCfg.AutoChannelAlg == 0) {
		/* set channel callback has been replaced by chandef of cfg80211_ap_settings */
		CFG80211_CB *p80211CB;
		CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;

		/* init */
		memset(&ChanInfo, 0, sizeof(ChanInfo));

		p80211CB = pAd->pCfg80211_CB;

		if (p80211CB == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"80211> p80211CB == NULL!\n");
			goto end;
		}

		/* get channel number */

		ChanInfo.ChanId = ieee80211_frequency_to_channel(
			settings->chandef.chan->center_freq);
		ChanInfo.CenterChanId = ieee80211_frequency_to_channel(
			settings->chandef.center_freq1);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
			"80211> Channel = %d, CenterChanId = %d\n",
			ChanInfo.ChanId, ChanInfo.CenterChanId);

		ChanInfo.IfType = RT_CMD_80211_IFTYPE_P2P_GO;

		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
			"80211> ChanInfo.IfType == %d!\n", ChanInfo.IfType);

		switch (settings->chandef.width) {
		case NL80211_CHAN_WIDTH_20_NOHT:
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;

		case NL80211_CHAN_WIDTH_20:
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
			break;

		case NL80211_CHAN_WIDTH_40:
			if (settings->chandef.center_freq1 > settings->chandef.chan->center_freq)
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;
			else
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
			break;

#ifdef DOT11_VHT_AC
		case NL80211_CHAN_WIDTH_80:
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
				"80211> NL80211_CHAN_WIDTH_80 CtrlCh: %d, CentCh: %d\n",
				ChanInfo.ChanId, ChanInfo.CenterChanId);
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_VHT80;
			break;

			/* Separated BW 80 and BW 160 is not supported yet */
		case NL80211_CHAN_WIDTH_80P80:
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_VHT80P80;
			break;
		case NL80211_CHAN_WIDTH_160:
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_VHT160;
			break;
#endif /* DOT11_VHT_AC */

		default:
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
				"80211> Unsupported Chan Width: %d\n",
				settings->chandef.width);
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;
		}

		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
			"80211> ChanInfo.ChanType == %d!\n", ChanInfo.ChanType);
		ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;

		/* set channel */
		RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	}

	/*ht green filed*/
#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.wifi_cert) {
		if (settings->ht_cap) {
			if (settings->ht_cap->cap_info & IEEE80211_HT_CAP_GRN_FLD)
				pAd->CommonCfg.RegTransmitSetting.field.HTMODE = HTMODE_GF;
			else
				pAd->CommonCfg.RegTransmitSetting.field.HTMODE = HTMODE_MM;

			wlan_config_set_ht_mode(pWdev, pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
			wlan_operate_loader_greenfield(pWdev, pAd->CommonCfg.RegTransmitSetting.field.HTMODE);

			SetCommonHtVht(pAd, pWdev);

			MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
			"CFG80211:Setting HTMODE = %s!\n",
			pAd->CommonCfg.RegTransmitSetting.field.HTMODE ? "HT_GF" : "HT_MM");
		}
	}
#endif

	CFG80211DRV_OpsBeaconAdd(pAd, &bcn, &cfg);
#ifdef DOT11_EHT_BE
	/* sync ML Probe.rsp Per-STA Profile buffer */
	bss_mngr_mld_sync_ml_probe_rsp(pWdev);
#endif/*DOT11_EHT_BE*/

end:
	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);
	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return ret;
}

VOID CFG80211_UpdateAssocRespExtraIe(
	VOID *pAdOrg,
	UINT32 apidx,
	UCHAR *assocresp_ies,
	UINT32 assocresp_ies_len)
{
	UINT32 len = 0;
	PEID_STRUCT eid_ptr = (PEID_STRUCT)assocresp_ies;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PUCHAR pAssocRespBuf = (PUCHAR)pAd->ApCfg.MBSSID[apidx].AssocRespExtraIe;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "80211> ==>\n");
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
		"IE len = %d\n", assocresp_ies_len);
	if (assocresp_ies_len > sizeof(pAd->ApCfg.MBSSID[apidx].AssocRespExtraIe)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
			"AssocResp buf size not enough\n");
		return;
	}

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < (assocresp_ies + assocresp_ies_len)) {
		switch (eid_ptr->Eid) {
		case IE_EXT_CAPABILITY:
			break;
		default:
			NdisCopyMemory(pAssocRespBuf+len, (UCHAR *)eid_ptr, (eid_ptr->Len + 2));
			len += eid_ptr->Len + 2;
			break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
		if (((UCHAR *)eid_ptr >= (assocresp_ies + assocresp_ies_len))
			|| ((UCHAR *)(eid_ptr+2) > (assocresp_ies + assocresp_ies_len)))
			break;
	}

	pAd->ApCfg.MBSSID[apidx].AssocRespExtraIeLen = len;
}

static int CFG80211_OpsChangeBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_beacon_data *info)
{
	struct _RTMP_ADAPTER *pAd;
	struct CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf = NULL;
	UCHAR *beacon_tail_buf = NULL;
	int ret = 0;
	INT apidx;
	struct wifi_dev *pWdev = NULL;
	memset(&bcn, 0, sizeof(bcn));

	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "80211> ==>\n");
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		ret = -EINVAL;
		goto end;
	}
	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (info->head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
		if (beacon_head_buf == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"memory alloc failed\n");
			ret = -ENOMEM;
			goto end;
		}
		NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	} else {
		ret = -EINVAL;
		goto end;
	}

	if (info->tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
		if (beacon_tail_buf == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
				"memory alloc failed\n");
			ret = -ENOMEM;
			goto end;
		}
		NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	} else {
		ret = -EINVAL;
		goto end;
	}

	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;

	if (bcn.beacon_head) {
		bcn.apidx = get_apidx_by_addr(pAd, bcn.beacon_head+10);
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
			"apidx %d\n", bcn.apidx);

		/* Update assoc resp extra ie */
		if (info->assocresp_ies_len && info->assocresp_ies)
			CFG80211_UpdateAssocRespExtraIe(pAd, bcn.apidx, (UCHAR *)info->assocresp_ies, info->assocresp_ies_len);
	}

	/* update beacon ies */
	CFG80211DRV_OpsBeaconSet(pAd, &bcn, NULL);
#ifdef DOT11_EHT_BE
	/* sync ML Probe.rsp Per-STA Profile buffer */
	bss_mngr_mld_sync_ml_probe_rsp(pWdev);
#endif/*DOT11_EHT_BE*/

end:
	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);
	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return ret;

}

static int CFG80211_OpsStopAp(
	struct wiphy *pWiphy,
	struct net_device *netdev
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	, unsigned int link_id
#endif /* CFG_CFG80211_VERSION */
	)
{
	struct _RTMP_ADAPTER *pAd;
	INT apidx, i;
	struct wifi_dev *pWdev = NULL;
	struct _NDIS_AP_802_11_PMKID *pmkid_cache;
#ifdef DOT11_EHT_BE
	UCHAR *own_mac_mld = NULL;
#endif /*DOT11_EHT_BE*/

	MAC80211_PAD_GET(pAd, pWiphy);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR, "80211> ==>\n");
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);
	pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);

	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_ERROR,
			"80211> wdev not found<==\n");
		return -ENODEV;
	}
#ifdef DOT11_EHT_BE
	MBSS_Reconfig_Deinit(pAd, &pAd->ApCfg.MBSSID[apidx]);
#endif
	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	RTMP_DRIVER_80211_BEACON_DEL(pAd, apidx);
#ifdef DOT11_EHT_BE
	/* sync ML Probe.rsp Per-STA Profile buffer */
	bss_mngr_mld_sync_ml_probe_rsp(pWdev);
	own_mac_mld = pWdev->bss_info_argument.mld_info.mld_addr;
#endif /* DOT11_EHT_BE */
	/* flush all pmk cache while ap stop*/
	OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_NOTICE,
		"80211> flush pmkid cache of bss ("MACSTR")<==\n",
		MAC2STR(pWdev->if_addr));
	for (i = 0; i < MAX_PMKID_COUNT; i++) {
		RTMPDeletePMKIDCache(pmkid_cache, pWdev->if_addr, i);
#ifdef DOT11_EHT_BE
		if (!MAC_ADDR_EQUAL(own_mac_mld, pWdev->if_addr) && !MAC_ADDR_EQUAL(own_mac_mld, ZERO_MAC_ADDR)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO,
				"clear PMKID cache for MLO, own mac mld="MACSTR"\n", own_mac_mld);
			RTMPDeletePMKIDCache(pmkid_cache, own_mac_mld, i);
		}
#endif /* DOT11_EHT_BE */
	}
	OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
	return 0;

}
#endif /* CONFIG_AP_SUPPORT */

enum nl80211_band CFG80211_Get_BandId(struct _RTMP_ADAPTER *pAd)
{
	struct wifi_dev *main_bss_wdev = NULL;

	main_bss_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

	if (WMODE_CAP_6G(main_bss_wdev->PhyMode))
		return NL80211_BAND_6GHZ;
	else if (WMODE_CAP_2G(main_bss_wdev->PhyMode))
		return NL80211_BAND_2GHZ;
	else if (WMODE_CAP_5G(main_bss_wdev->PhyMode))
		return NL80211_BAND_5GHZ;

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
		"PhyMode(%d) invalid, return NL80211_BAND_5GHZ!\n", main_bss_wdev->PhyMode);
	return NL80211_BAND_5GHZ;
}

VOID *CFG80211_FindChan(struct _RTMP_ADAPTER *pAd, UCHAR chan_id)
{
	CFG80211_CB *pCfg80211_CB = NULL;
	UINT32 IdChan, ChanNum, ChanFreq;
	enum nl80211_band band_idx;
	struct cfg80211_chan_def *chandef;
	struct ieee80211_supported_band *pBand;

	pCfg80211_CB = pAd->pCfg80211_CB;

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
	chandef = wdev_chandef(pCfg80211_CB->pCfg80211_Wdev, 0);
#else
	chandef = &pCfg80211_CB->pCfg80211_Wdev->chandef;
#endif /* CFG_CFG80211_VERSION */
	if (!chandef) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"chandef is NULL\n");
		return NULL;
	}

	band_idx = CFG80211_Get_BandId(pAd);
	pBand = &pCfg80211_CB->Cfg80211_bands[band_idx];
	ChanFreq = ieee80211_channel_to_frequency(chan_id, band_idx);
	ChanNum = pCfg80211_CB->Cfg80211_bands[band_idx].n_channels;

	for (IdChan = 0; IdChan < ChanNum; IdChan++) {
		if (pBand->channels[IdChan].center_freq == ChanFreq)
			return &pBand->channels[IdChan];
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
		"Find fail for chan %d.\n", chan_id);
	return NULL;
}

static int CFG80211_OpsGetChannel(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
				      unsigned int link_id,
#endif /* CFG_CFG80211_VERSION */
				      struct cfg80211_chan_def *chandef)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	CFG80211_BAND BandInfo = {0};
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_phy *mac_phy = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "\n");
	if (!wiphy) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "wiphy NULL!\n");
		return -ENODEV;
	}

	/*pad infos*/
	MAC80211_PAD_GET(pAd, wiphy);
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);

	mac_dev = hc_get_mac_dev(pAd);
	mac_phy = &mac_dev->mac_phy;
	if (!mac_phy)
		return -ENETDOWN;

	OS_SEM_LOCK(&mac_phy->lock);
	if (!mac_phy->state) {
		OS_SEM_UNLOCK(&mac_phy->lock);
		return -ENETDOWN;
	}

	chandef->chan = (struct ieee80211_channel *)CFG80211_FindChan(pAd, mac_phy->chan);
	if (!chandef->chan) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"CFG80211_FindChan failed!\n");
		OS_SEM_UNLOCK(&mac_phy->lock);
		return -EFAULT;
	}

#ifndef IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ
	if (mac_phy->bw > BW_160) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"320M not support in cfg80211!\n");
		OS_SEM_UNLOCK(&mac_phy->lock);
		return -EOPNOTSUPP;
	}
#endif /* IEEE80211_EHT_OPER_CHAN_WIDTH_320MHZ */

	chandef->width = phy_bw_2_nl_bw(mac_phy->bw);
	chandef->center_freq1 = ieee80211_channel_to_frequency(mac_phy->cen_chan, chandef->chan->band);
	chandef->center_freq2 = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"chan=0x%p, width=%d, center_freq1=%d, center_freq2=%d, mac_phy->chan=%d\n",
		chandef->chan, chandef->width, chandef->center_freq1, chandef->center_freq2,
		mac_phy->chan);
	OS_SEM_UNLOCK(&mac_phy->lock);

	return 0;
}

#ifdef CFG80211_FULL_OPS_SUPPORT
static int CFG80211_OpsStartRadarDetect(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_chan_def *chandef,
	u32 cac_time_ms)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	struct DOT11_H *pDot11h = NULL;
	PDFS_PARAM pDfsParam = NULL;
	UCHAR BandIdx;
	UCHAR rd_region = 0;
	INT ret = FALSE;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "80211> ==>\n");

	if (!pWiphy || !netdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR, Invalid parameter: pWiphy = %p, netdev = %p\n",
		pWiphy, netdev);
		return -EINVAL;
	}

	/*Get pAd*/
	MAC80211_PAD_GET(pAd, pWiphy);
	BandIdx = CFG80211_Get_BandId(pAd);

	pDfsParam = &pAd->CommonCfg.DfsParameter;
	rd_region = pAd->CommonCfg.RDDurRegion;

	if (BandIdx > NUM_NL80211_BANDS)
		return -EFAULT;

	pDot11h = &pAd->Dot11_H;
	if (pDot11h == NULL)
		return -EINVAL;

	/* DfsCacNormalStart(pAd, wdev, pDot11h->ChannelMode); */
	pDot11h->RDCount = 0;
	pDot11h->cac_time = (UINT32)MSEC_TO_SEC(cac_time_ms);

	switch (BandIdx) {
	case NL80211_BAND_5GHZ:
		ret = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		ret = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
		ret = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;

		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "StartRadarDetect Fail, (ret = %d)\n", ret);
			return -EINVAL;
		}

		break;

	case NL80211_BAND_2GHZ:
	case NL80211_BAND_6GHZ:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Error DFS Not Support 2GHZ or 6GHZ.\n");
		return -EINVAL;

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Error band input.\n");
		return -EINVAL;
	}


	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"[%s](%d): BandIdx = %d, cac_time = %d (secs), ChannelMode = %d\n",
		__func__, __LINE__, BandIdx, (UINT32)MSEC_TO_SEC(cac_time_ms), pDot11h->ChannelMode);


	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE, "80211> %s end.\n", __func__);

	return 0;
}


static void CFG80211_OpsEndCac(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	struct DOT11_H *pDot11h = NULL;
	PDFS_PARAM pDfsParam = NULL;
	UCHAR BandIdx;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "80211> ==>\n");

	if (!pWiphy || !netdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"ERROR, Invalid parameter: pWiphy = %p, netdev = %p\n",
		pWiphy, netdev);
		return;
	}

	/*Get pAd*/
	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
	BandIdx = CFG80211_Get_BandId(pAd);

	if (BandIdx > NUM_NL80211_BANDS)
		return;

	pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDot11h = &pAd->Dot11_H;
	if (pDot11h == NULL)
		return;


	MTWF_PRINT("%s(): set CAC value to 0, CAC End, origin is %d.\n", __func__, pDot11h->RDCount);


	if ((pDot11h->ChannelMode == CHAN_SILENCE_MODE)
		|| (pDfsParam->BW160ZeroWaitSupport == TRUE))
		pDot11h->RDCount = pDot11h->cac_time;

	pDfsParam->DedicatedOutBandCacCount = pDfsParam->DedicatedOutBandCacTime;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "80211> %s end.\n", __func__);
}

#endif

static int CFG80211_OpsChangeBss(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct bss_parameters *params)
{
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev = NULL;

	wdev = RTMP_OS_NETDEV_GET_WDEV(netdev);
	if (!wdev)
		return -ENODEV;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, pWiphy);

	if (wdev->func_idx < 0 || wdev->func_idx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"invalid wdev->func_idx:%d\n", wdev->func_idx);
		return -EINVAL;
	}

	/* ap isolate */
	if (params->ap_isolate == 1)
		pAd->ApCfg.MBSSID[wdev->func_idx].IsolateInterStaTraffic = TRUE;
	else if (params->ap_isolate == 0)
		pAd->ApCfg.MBSSID[wdev->func_idx].IsolateInterStaTraffic = FALSE;


	return 0;
}

static int CFG80211_OpsStaDel(
	struct wiphy *pWiphy,
	struct net_device *dev,
	struct station_del_parameters *params)
{
	struct _RTMP_ADAPTER *pAd;
	const UINT8 *pMacAddr = params->mac;
	CMD_RTPRIV_IOCTL_AP_STA_DEL rApStaDel = {.pSta_MAC = NULL, .pWdev = NULL, .subtype = params->subtype};

	MAC80211_PAD_GET(pAd, pWiphy);

	if (dev) {
		rApStaDel.pWdev = RTMP_OS_NETDEV_GET_WDEV(dev);
		if (!rApStaDel.pWdev)
			return -ENODEV;
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
			"80211> ==> for bssid ("MACSTR")\n",
			MAC2STR(rApStaDel.pWdev->bssid));
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "80211> ==>");

	if (pMacAddr) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
			"80211> Delete STA("MACSTR") ==>\n", MAC2STR(pMacAddr));
		rApStaDel.pSta_MAC = (UINT8 *)pMacAddr;
	}
	RTMP_DRIVER_80211_AP_STA_DEL(pAd, (VOID *)&rApStaDel, params->reason_code);
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "80211> <==\n");

	return 0;
}

#if defined(CONFIG_NL80211_TESTMODE) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
static int CFG80211_OpsTestModeCmd(
	struct wiphy *pWiphy,
	IN struct wireless_dev *wdev,
	void *data,
	INT datalen)
{
	struct _RTMP_ADAPTER *pAd;
	RT_CMD_AP_IOCTL_UPDATE_PMKID pmkid_entry = {0};

	/*copy pentry details from data variable*/
	NdisCopyMemory(&pmkid_entry, data, datalen);

	MAC80211_PAD_GET(pAd, pWiphy);

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO, "80211> datalen:%d==>\n", datalen);

	hex_dump_with_lvl("testmode_buffer", (unsigned char *) data, datalen, DBG_LVL_INFO);

	RTMP_DRIVER_80211_AP_UPDATE_STA_PMKID(pAd, (VOID *)&pmkid_entry);

	return 0;
}
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
static int mtk_cfg80211_set_pmkid_cache(PRTMP_ADAPTER pAd,
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	struct nlattr *tb)
{
	char *data = nla_data(tb);
	RT_CMD_AP_IOCTL_UPDATE_PMKID pmkid_entry = {0};

	if (nla_len(tb) < sizeof(pmkid_entry))
		return -1;
	/*copy pentry details from data variable*/
	NdisCopyMemory(&pmkid_entry, data, sizeof(pmkid_entry));

	MAC80211_PAD_GET(pAd, wiphy);


	hex_dump_with_lvl("pmkid_cache", (unsigned char *) data, sizeof(pmkid_entry), DBG_LVL_INFO);

	RTMP_DRIVER_80211_AP_UPDATE_STA_PMKID(pAd, (VOID *)&pmkid_entry);

	return 0;
}

static struct nla_policy mtk_nl80211_vendor_attr_pmkid_policy[];

int mtk_cfg80211_set_pmkid(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx;
	struct net_device *pNetDev = wdev->netdev;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_PMKID_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_NOTICE,
		"ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_PMKID_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_pmkid_policy, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_PMKSA_CACHE]) {
		ret = mtk_cfg80211_set_pmkid_cache(pAd, wiphy, wdev, tb[MTK_NL80211_VENDOR_ATTR_SET_PMKSA_CACHE]);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_ERROR,
				"mtk_cfg80211_command failed.\n");
			return -EINVAL;
		}
	}

	return 0;
}
#endif

#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
static int mtk_cfg80211_set_sta_mac(PRTMP_ADAPTER pAd,
	struct wiphy *wiphy,
	struct wifi_dev *wdev,
	struct nlattr *tb)
{
	char *data = nla_data(tb);
	UCHAR ifIndex = wdev->func_idx;

	MAC80211_PAD_GET(pAd, wiphy);
	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"PE: wifi_sys_close fail!!!\n");
		return FALSE;
	}
	if (ifIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"apcli interface out of range!!!\n");
		return FALSE;
	}

	NdisMoveMemory(pAd->ApcliAddr[ifIndex], data, MAC_ADDR_LEN);

	MTWF_DBG_NP(DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"%s: "MACSTR".\n", __func__, MAC2STR(data));

	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"PE: wifi_sys_open fail!!!\n");
		return FALSE;
	}


	return 0;
}

static struct nla_policy mtk_nl80211_vendor_attr_set_mac_policy[];

int mtk_cfg80211_set_mac(struct wiphy *wiphy,
	struct wireless_dev *wdev,
	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct wifi_dev *Wdev = NULL;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SET_MAC_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	Wdev = RTMP_OS_NETDEV_GET_WDEV(wdev->netdev);
	if (!Wdev)
		return -ENODEV;

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SET_MAC_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_set_mac_policy, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_STA_MAC]) {
		ret = mtk_cfg80211_set_sta_mac(pAd, wiphy, Wdev, tb[MTK_NL80211_VENDOR_ATTR_SET_STA_MAC]);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"mtk_cfg80211_command failed.\n");
		return -EINVAL;
		}
	}
return 0;
}

static struct nla_policy mtk_nl80211_vendor_attr_set_seq_num_policy[];

int mtk_cfg80211_set_seq_num(struct wiphy *wiphy,
	struct wireless_dev *wdev,
	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct wifi_dev *Wdev = NULL;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	Wdev = RTMP_OS_NETDEV_GET_WDEV(wdev->netdev);
	if (!Wdev)
		return -ENODEV;

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_set_seq_num_policy, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM]) {
		unsigned char seq_num = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM]);

		NdisMoveMemory(&Wdev->SecConfig.wpa_supp_seq_num, &seq_num, sizeof(u8));
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"wpa_supp_seq_num[%d]\n", Wdev->SecConfig.wpa_supp_seq_num);
	}
return 0;
}
#endif

static struct nla_policy mtk_nl80211_vendor_attr_sta_clean_policy[];

int mtk_cfg80211_clean_sta_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_STA_CLEAN_ATTR_MAX + 1];
	MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_STA_CLEAN_ATTR_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_sta_clean_policy, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_STA_CLEAN_MAC]) {
		UCHAR *mac_addr = nla_data(tb[MTK_NL80211_VENDOR_ATTR_STA_CLEAN_MAC]);

		entry = MacTableLookup(pAd, mac_addr);
		if (entry != NULL)
			mac_entry_delete(pAd, entry, TRUE);
	}
	return 0;
}

static int CFG80211_OpsStaAdd(
	struct wiphy *wiphy,
	struct net_device *dev,
	const UINT8 *mac,

	struct station_parameters *params)
{
	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO, "80211> ==>\n");
	return 0;
}


static int CFG80211_OpsStaChg(
	struct wiphy *pWiphy,
	struct net_device *dev,
	const UINT8 *pMacAddr,
	struct station_parameters *params)
{
	struct _RTMP_ADAPTER *pAd;
	CFG80211_CB *p80211CB;
	MAC_TABLE_ENTRY *pEntry;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
		"80211> Change STA("MACSTR") ==>\n", MAC2STR(pMacAddr));
	MAC80211_PAD_GET(pAd, pWiphy);
	p80211CB = pAd->pCfg80211_CB;

	pEntry = MacTableLookup(pAd, (UCHAR *)pMacAddr);

	if ((dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP) &&
		(dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_P2P_GO) &&
		(dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP_VLAN))
		return -EOPNOTSUPP;
/*  vikas: used for VLAN, along with auth flag
	if(!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)))
	{
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %x ==>\n", params->sta_flags_mask));
		return -EOPNOTSUPP;
	}
*/

	if (pEntry && IS_AKM_WPA2(pEntry->SecConfig.AKMMap)) {
		if (params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			if ((params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) &&
				(pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE)) {
				MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
					"80211> STA("MACSTR") ==> PortSecured\n",
					MAC2STR(pMacAddr));
				RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 1);
			} else {
				MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
					"80211> STA("MACSTR") ==> PortNotSecured\n",
					MAC2STR(pMacAddr));
				RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 0);
			}
		}
	} else {
		if (params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
				MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
					"80211> STA("MACSTR") ==> PortSecured\n",
					MAC2STR(pMacAddr));
				RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 1);
			} else {
				MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
					"80211> STA("MACSTR") ==> PortNotSecured\n",
					MAC2STR(pMacAddr));
				RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 0);
			}
		}
	}
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (params->vlan) {
		struct wifi_dev *wdev;
		struct vlan_gtk_info *vg_info;
		MAC_TABLE_ENTRY *pEntry;

		wdev = CFG80211_GetWdevByVlandev((PRTMP_ADAPTER)pAd, params->vlan);
		if (!wdev) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
				"%s() invalid params->vlan vlan_dev=%p\n", __func__,
				params->vlan);
			return -EINVAL;
		}

		pEntry = MacTableLookup(pAd, (UCHAR *)pMacAddr);
		if (!pEntry) {
			MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_AP, DBG_LVL_INFO,
				"%s()  can't find STA %02X:%02X:%02X:%02X:%02X:%02X, set VLAN failed\n",
				__func__, PRINT_MAC(pMacAddr));
			return -EINVAL;
		}

		vg_info = CFG80211_GetVlanInfoByVlandev(wdev, params->vlan);
		pEntry->vlan_id = (vg_info) ? vg_info->vlan_id : 0;
		MTWF_PRINT("%s set STA %02X:%02X:%02X:%02X:%02X:%02X to iface %s with vlan_id %d\n",
			__func__, PRINT_MAC(pMacAddr), params->vlan->name, pEntry->vlan_id);

	}
#endif
	return 0;
}

#ifdef HOSTAPD_HS_R2_SUPPORT
static int CFG80211_OpsSetQosMap(
		IN struct wiphy *wiphy,
		IN struct net_device *dev,
		IN struct cfg80211_qos_map *qos_map)
{
	PRTMP_ADAPTER pAd;
	INT apidx;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, wiphy);
	/*get mbss from net device and for all the clients associated with that mbss set the given qosmap */

	if (qos_map == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO, "Qos Map NULL\n");
		return 0;
	}

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev);
	if (apidx != WDEV_NOT_FOUND) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
			"Setting Qos Param for apidx %d\n", apidx);
		RTMP_DRIVER_80211_QOS_PARAM_SET(pAd, qos_map, apidx);
	} else
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HS_R2, DBG_LVL_INFO,
			"AP Index for setting qos map not found\n");

	return 0;
}
#endif

static int CFG80211_OpsSetAntenna(
	struct wiphy *wiphy,
	u32 tx_ant,
	u32 rx_ant)
{

	int ret = 0;
	PRTMP_ADAPTER pAd = NULL;
	u8 tx_ant_count = 0;
	u8 rx_ant_count = 0;
	u32 ant_bitmap = 0;
	CMD_RTPRIV_IOCTL_80211_ANTENNA ant_cnt;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "80211> ==>\n");

	memset(&ant_cnt, 0, sizeof(CMD_RTPRIV_IOCTL_80211_ANTENNA));
	MAC80211_PAD_GET(pAd, wiphy);

	ant_bitmap = tx_ant;
	while (ant_bitmap & 0x1) {
		tx_ant_count++;
		ant_bitmap >>= 1;
	}

	ant_bitmap = rx_ant;
	while (ant_bitmap & 0x1) {
		rx_ant_count++;
		ant_bitmap >>= 1;
	}

	if (tx_ant_count == 0 || rx_ant_count == 0) {
		ret = -1;
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"80211> Wrong input parameter!\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"80211> TX ant count = %x, RX ant count = %x\n",
			tx_ant_count, rx_ant_count);
		ant_cnt.tx_ant = tx_ant_count;
		ant_cnt.rx_ant = rx_ant_count;
		ret = RTMP_DRIVER_80211_SET_ANTENNA(pAd, &ant_cnt);
	}

	return ret;
}

static int CFG80211_OpsGetAntenna(
	struct wiphy *wiphy,
	u32 *tx_ant,
	u32 *rx_ant)
{
	int ret = 0;
	PRTMP_ADAPTER pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_ANTENNA ant_cnt;

	memset(&ant_cnt, 0, sizeof(CMD_RTPRIV_IOCTL_80211_ANTENNA));
	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "80211> ==>\n");

	ret = RTMP_DRIVER_80211_GET_ANTENNA(pAd, &ant_cnt);
	if (ret == 0) {
		*tx_ant = BIT(ant_cnt.tx_ant) - 1;
		*rx_ant = BIT(ant_cnt.rx_ant) - 1;
	}
	return ret;
}

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
static struct wireless_dev *CFG80211_OpsVirtualInfAdd(
	IN struct wiphy *pWiphy,
	IN const char *name,
	IN unsigned char name_assign_type,
	IN enum nl80211_iftype Type,
	struct vif_params *pParams)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
	PWIRELESS_DEV pDev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_NOTICE, "80211> ==>\n");

	MAC80211_PAD_GET_RETURN_NULL(pAd, pWiphy);
	vifInfo.vifType = Type;
	vifInfo.vifNameLen = strlen(name);
	vifInfo.pWdev = NULL;

	if (vifInfo.vifNameLen < IFNAMSIZ) {
		memset(vifInfo.vifName, 0, sizeof(vifInfo.vifName));
		NdisCopyMemory(vifInfo.vifName, name, vifInfo.vifNameLen);
	} else {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
			"Too long interface name!!!\n");
		return ERR_PTR(-EINVAL);
	}

	pDev = RTMP_CFG80211_FindVifEntryWdev_ByName(pAd, vifInfo.vifName);
	if (pDev) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
			"pDev %s existed\n", vifInfo.vifName);
		return ERR_PTR(-ENFILE);
	}

	if ((pWiphy->flags) & (WIPHY_FLAG_4ADDR_STATION))
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_NOTICE,
			"Support WIPHY_FLAG_4ADDR_STATION\n");

	vifInfo.flags = pWiphy->flags;

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_ERROR,
			"pDev %s add failed\n", vifInfo.vifName);
		return ERR_PTR(-EINVAL);
	}

	return vifInfo.pWdev;
}

static int CFG80211_OpsVirtualInfDel(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *pwdev
)
{
		struct _RTMP_ADAPTER *pAd;
		struct net_device *dev = NULL;

		dev = pwdev->netdev;
		if (!dev)
			return 0;
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_INTF, DBG_LVL_NOTICE,
			"80211> %s [%d]==>\n", dev->name, dev->ieee80211_ptr->iftype);
		MAC80211_PAD_GET(pAd, pWiphy);
		RTMP_DRIVER_80211_VIF_DEL(pAd, dev, dev->ieee80211_ptr->iftype);

		return 0;
}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

static const struct ieee80211_txrx_stypes
	ralink_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) | BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) | BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},

};

#ifdef SUPP_SAE_SUPPORT
int mtk_cfg80211_event_connect_params(
	struct _RTMP_ADAPTER *pAd, UCHAR *pmk, int pmk_len)
{
	struct sk_buff *skb;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");

	pCfg80211_CB = pAd->pCfg80211_CB;
	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
							pmk_len, 0, GFP_KERNEL);
	if (!skb)
		return -1;

	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, pmk_len, pmk)) {
		kfree_skb(skb);
		return -1;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}


int mtk_cfg80211_event_pmksa(struct _RTMP_ADAPTER *pAd,
		UCHAR *pmk, int pmk_len, UCHAR *pmkid, UINT32 akmp, UINT8 *aa)
{
	struct sk_buff *skb;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;
	MTK_PMKSA_EVENT *pmksa_event = NULL;

	pCfg80211_CB = pAd->pCfg80211_CB;
	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_KEY, DBG_LVL_INFO, "80211> ==>\n");

	os_alloc_mem(pAd, (UCHAR **)&pmksa_event, sizeof(MTK_PMKSA_EVENT));
	if (!pmksa_event)
		return 0;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
						sizeof(MTK_PMKSA_EVENT), MTK_NL80211_VENDOR_EVENT_RSP_WAPP_EVENT, GFP_KERNEL);

	if (!skb) {
		os_free_mem(pmksa_event);
		return -1;
	}

	os_zero_mem(pmksa_event, sizeof(MTK_PMKSA_EVENT));
	os_move_mem(pmksa_event->pmk, pmk, pmk_len);
	pmksa_event->pmk_len = pmk_len;
	os_move_mem(pmksa_event->pmkid, pmkid, 16);
	pmksa_event->akmp = akmp;
	os_move_mem(pmksa_event->aa, aa, 6);
	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, sizeof(MTK_PMKSA_EVENT), pmksa_event)) {
		kfree_skb(skb);
		os_free_mem(pmksa_event);
		return -1;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);

	os_free_mem(pmksa_event);
	return 0;
}

#endif

static int CFG80211_SetBitRate(struct wiphy *wiphy,
				      struct net_device *netdev,
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 1, 0) <= CFG_CFG80211_VERSION)
				      unsigned int link_id,
#endif /* CFG_CFG80211_VERSION */
				      const u8 *addr,
				      const struct cfg80211_bitrate_mask *mask)
{
	PRTMP_ADAPTER pAd = NULL;
	INT apidx;

	if (!wiphy || !netdev || !mask) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"failed - [ERROR]input NULL pointer.\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "80211> ==>\n");
	MAC80211_PAD_GET(pAd, wiphy);
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);

	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return FALSE;
	}

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pAd.\n");
		return FALSE;
	}

	RTMP_DRIVER_80211_BITRATE_SET(pAd, (VOID *)mask, apidx);

	return 0;
}

struct wifi_dev *mtk_cfg80211_vndr_get_wdev_by_wireless_dev(struct _RTMP_ADAPTER *pAd,
									struct wireless_dev *wl_dev)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;

	if (wl_dev->iftype == NL80211_IFTYPE_AP) {
		ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				    "error AP index\n");
			return NULL;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_CFG80211_SUPPORT
	else if (wl_dev->iftype == NL80211_IFTYPE_STATION) {
		ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				    "error station index\n");
			return NULL;
		}
		wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif /* APCLI_CFG80211_SUPPORT */
	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				    "Can't find a valid wdev\n");
		return NULL;
	}

	return wdev;
}

int mtk_cfg80211_vndr_cmd_reply_msg(struct _RTMP_ADAPTER *pAd, struct wiphy *wiphy,
					     unsigned int attr_cmd, void *data, unsigned int data_len)
{
	struct sk_buff *skb;
	int ret;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "(caller:%pS)\n",
		OS_TRACE);

	if (!data) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"data pointer is NULL\n");
		return -ENOMEM;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, data_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -ENOMEM;
	}

	if (nla_put(skb, attr_cmd, data_len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}

int mtk_cfg80211_vndr_mac_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	const struct nlattr *attr, *head = data;
	int rem, ret;
	UINT32 IdMac, map_addr, mac_s = 0, mac_e = 0;
	UINT32 macVal = 0, max_len = 4096;
	BOOLEAN IsFound;
	RTMP_STRING *mpool, *msg;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct sk_buff *skb;
	struct mac_param *param;
	int msg_len = 0;
	int free_len = max_len;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (4096 + 256 + 12));

	if (!mpool)
		return -ENOMEM;

	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	memset(msg, 0x00, max_len);

	nla_for_each_attr(attr, head, len, rem) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "%d nla_type(attr)=%d\n", __LINE__, nla_type(attr));
		if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct mac_param *)nla_data(attr);
			mac_s = param->start;
			macVal = param->value;
#ifdef WIFI_UNIFIED_COMMAND
			/* WF_PHY_DSP CR via WM */
			if ((mac_s & 0x86000000) == 0x86000000) {
				RTMP_REG_PAIR dsp_reg;

				dsp_reg.Register = mac_s;
				dsp_reg.Value = macVal;
				UniCmdMultipleMacRegAccessWrite(pAd, &dsp_reg, 1);
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
					"write: dsp_addr=0x%08x, val=0x%08x\n",
					mac_s, macVal);
			} else
#endif
				RTMP_IO_WRITE32(pAd->hdev_ctrl, mac_s, macVal);
			map_addr = mac_s;
			IsFound = mt_mac_cr_range_mapping(pAd->physical_dev, &map_addr);
			ret = snprintf(msg + msg_len, free_len, "[0x%04x]:%08x  ",
				map_addr, macVal);
			if (os_snprintf_error(free_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					"msg snprintf error!\n");
				goto error;
			}
			msg_len += ret;
			free_len -= ret;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
				"MacAddr=0x%x, MacValue=0x%x, IsRemap=%d\n", map_addr, macVal, !IsFound);
		} else if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct mac_param *)nla_data(attr);
			mac_s = param->start;
			mac_e = param->end;
			for (IdMac = mac_s; IdMac <= mac_e; IdMac += 4) {
#ifdef WIFI_UNIFIED_COMMAND
				/* WF_PHY_DSP CR via WM */
				if ((mac_s & 0x86000000) == 0x86000000) {
					RTMP_REG_PAIR dsp_reg;

					dsp_reg.Register = IdMac;
					dsp_reg.Value = 0;
					UniCmdMultipleMacRegAccessRead(pAd, &dsp_reg, 1);
					macVal = dsp_reg.Value;
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
							"read: dsp_addr=0x%08x, val=0x%08x\n",
							IdMac, macVal);
				} else
#endif
					RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);

				map_addr = IdMac;
				IsFound = mt_mac_cr_range_mapping(pAd->physical_dev, &map_addr);
				ret = snprintf(msg + msg_len, free_len, "[0x%04x]:%08x  ", map_addr, macVal);
				if (os_snprintf_error(free_len, ret)) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"msg snprintf error!\n");
					goto error;
				}
				msg_len += ret;
				free_len -= ret;
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
					"MacAddr=0x%x, MacValue=0x%x, IsRemap=%d\n", map_addr, macVal, !IsFound);
			}
		}
	}

	if (msg_len == 0) {
		ret = snprintf(msg, free_len, "===>Error command format!");
		if (os_snprintf_error(free_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"msg snprintf error!\n");
			goto error;
		}
		msg_len += ret;
		free_len -= ret;
	}


	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_len + 1);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"fail to allocate reply msg\n");
		goto error;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR, msg_len + 1, msg)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		goto error;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"reply msg failed\n");
		goto error;
	}

	os_free_mem(mpool);
	return 0;
error:
	os_free_mem(mpool);
	return -EFAULT;
}

int mtk_cfg80211_vndr_e2p_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	const struct nlattr *attr, *head = data;
	int rem, ret, i = 1;
	UINT32 eepAddr, e2p_s = 0, e2p_e = 0;
	UINT32 max_len = EEPROM_SIZE;
	USHORT e2pVal = 0;
	RTMP_STRING *mpool, *msg;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct sk_buff *skb;
	struct e2p_param *param;
	int msg_len = 0;
	int free_len = max_len;
	struct _RTMP_CHIP_CAP *chip_cap;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	max_len = chip_cap->EEPROM_DEFAULT_BIN_SIZE;

	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (max_len + 256 + 12));

	if (!mpool)
		return -ENOMEM;

	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	memset(msg, 0x00, max_len);

	nla_for_each_attr(attr, head, len, rem) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "%d nla_type(attr)=%d\n", __LINE__, nla_type(attr));
		if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct e2p_param *)nla_data(attr);
			eepAddr = param->start;
			e2pVal = param->value;
			RT28xx_EEPROM_WRITE16(pAd, eepAddr, e2pVal);
			ret = snprintf(msg + msg_len, free_len, "[0x%04X]:0x%04X  ", eepAddr, e2pVal);
			if (os_snprintf_error(free_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"msg snprintf error!\n");
				goto error;
			}

			msg_len += ret;
			free_len -= ret;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"E2p=0x%x, e2pVal=0x%x\n", eepAddr, e2pVal);
		} else if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct e2p_param *)nla_data(attr);
			e2p_s = param->start;
			e2p_e = param->end;
			for (eepAddr = e2p_s; eepAddr <= e2p_e; eepAddr += 2) {
				RT28xx_EEPROM_READ16(pAd, eepAddr, e2pVal);
				ret = snprintf(msg + msg_len, free_len, "[0x%04X]:0x%04X  %s", eepAddr, e2pVal, ((i % 4 == 0)?"\n" : ""));
				if (os_snprintf_error(free_len, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"msg snprintf error!\n");
					goto error;
				}
				i += 1;
				msg_len += ret;
				free_len -= ret;
			}
		} else if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_E2P_DUMP_ALL_PARAM) {
			e2p_s = 0;
			e2p_e = chip_cap->EEPROM_DEFAULT_BIN_SIZE;
			/* UINT_32 e2p_print_lmt = EEPROM_SIZE; */

			if (e2p_e >= EEPROM_SIZE)
				e2p_e = EEPROM_SIZE;

			for (eepAddr = e2p_s; eepAddr <= e2p_e; eepAddr += 2) {
				RT28xx_EEPROM_READ16(pAd, eepAddr, e2pVal);
				MTWF_PRINT("[0x%04X]:%04X  %s", eepAddr, e2pVal, ((eepAddr & 0x6) == 0x06) ? "\n" : "");
			}

			ret = snprintf(msg, free_len, "dump done");
			if (os_snprintf_error(free_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"msg snprintf error!\n");
				goto error;
			}
			msg_len += ret;
			free_len -= ret;
		}
	}

	if (msg_len == 0) {
		ret = snprintf(msg, free_len, "===>Error command format!");
		if (os_snprintf_error(free_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"msg snprintf error!\n");
			goto error;
		}
		msg_len += ret;
		free_len -= ret;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_len + 1);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"fail to allocate reply msg\n");
		goto error;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_E2P_RSP_STR, msg_len + 1, msg)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		goto error;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"reply msg failed\n");
		goto error;
	}

	os_free_mem(mpool);
	return 0;
error:
	os_free_mem(mpool);
	return -EFAULT;
}

#ifdef CONFIG_WLAN_SERVICE
int mtk_cfg80211_vndr_testengine_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	const struct nlattr *attr, *head = data;
	int rem, ret;
	UINT32 rsp_data = 0;
	UINT32 max_len = 4096;
	RTMP_STRING *mpool, *msg;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct sk_buff *skb;
	struct testengine_param *param;
	int msg_len = 0;
	int free_len = max_len;
	struct _RTMP_CHIP_CAP *chip_cap;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (max_len + 256 + 12));
	if (!mpool)
		return -ENOMEM;

	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	memset(msg, 0x00, max_len);

	nla_for_each_attr(attr, head, len, rem) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO, "%d nla_type(attr)=%d\n", __LINE__, nla_type(attr));
		if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_TESTENGINE_SET &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct testengine_param *)nla_data(attr);

			ret = MtCmdSetTestEngine(pAd, param->atidx, param->value);
			if (ret)
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"Set ret: 0x%x, atidx: %d  Data: 0x%x\n", ret, param->atidx, param->value);

			ret = snprintf(msg + msg_len, free_len, "set=> atidx:%u   value:%u\n", param->atidx, param->value);
			if (os_snprintf_error(free_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
					"msg snprintf error!\n");
				goto error;
			}

			msg_len += ret;
			free_len -= ret;
		} else if (nla_type(attr) == MTK_NL80211_VENDOR_ATTR_TESTENGINE_GET &&
			nla_len(attr) >= sizeof(*param)) {
			param = (struct testengine_param *)nla_data(attr);

			ret = MtCmdGetTestEngine(pAd, param->atidx, param->value, &rsp_data);
			if (ret)
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
				"Get ret: 0x%x, atidx: %d  Data: 0x%x\n", ret, param->atidx, param->value);

			ret = snprintf(msg + msg_len, free_len, "get=> atidx:%u   value:%u\n", param->atidx, rsp_data);
			if (os_snprintf_error(free_len, ret)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
					"msg snprintf error!\n");
				goto error;
			}
			msg_len += ret;
			free_len -= ret;
		}
	}

	if (msg_len == 0) {
		ret = snprintf(msg, free_len, "===>Error command format!");
		if (os_snprintf_error(free_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"msg snprintf error!\n");
			goto error;
		}
		msg_len += ret;
		free_len -= ret;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_len + 1);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"fail to allocate reply msg\n");
		goto error;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_TESTENGINE_RSP_STR, msg_len + 1, msg)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		goto error;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
			"reply msg failed\n");
		goto error;
	}

	os_free_mem(mpool);
	return 0;
error:
	os_free_mem(mpool);
	return -EFAULT;
}
#endif /* CONFIG_WLAN_SERVICE */

int RTMPGetStatisticsStr(RTMP_ADAPTER *pAd, POS_COOKIE pObj, RTMP_STRING *msg, int msg_buf_len);

static const struct
nla_policy SUBCMD_VENDOR_SET_POLICY[];

int mtk_cfg80211_vndr_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_ATTR_MAX + 1];
	char *cmd_string;
	int cmd_len;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	ULONG priv_flags;
	POS_COOKIE pObj;
	RTMP_STRING *this_char, *value;
	UCHAR *tmp = NULL;
	INT Status = NDIS_STATUS_SUCCESS;
	int ret;
	struct PRIV_SET_PROC *SET_PROC;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_VENDOR_SET_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_VENDOR_SET_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (!tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR]) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"no MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR attr\n");
		return -EINVAL;
	}

	cmd_string = nla_data(tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR]);
	cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR]);
	if (strlen(cmd_string) > cmd_len)
		return -EINVAL;

	net_dev = wdev->netdev;

	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	tmp = cmd_string;
	while ((this_char = strsep((char **)&tmp, "\0")) != NULL) {
		if (!*this_char)
			continue;
#ifdef DBG
#ifdef DBG_ENHANCE
		{
			struct wifi_dev *wdev = NULL;
			INT ifIndex, ifType;
			struct net_device *netDev = NULL;

			ifIndex = pObj->ioctl_if;
			ifType = pObj->ioctl_if_type;

			if (ifIndex < 0) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"invalid ifIndex:%d\n", ifIndex);
				return -EFAULT;
			}

			if (ifType == INT_MAIN || ifType == INT_MBSSID) {
				if (VALID_MBSS(pAd, ifIndex))
					wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			} else if (ifType == INT_APCLI) {
				if (ifIndex < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[ifIndex].wdev;
			} else if (ifType == INT_WDS) {
#ifdef WDS_SUPPORT
				if (ifIndex < MAX_WDS_ENTRY)
					wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
#endif /* WDS_SUPPORT */
			} else {
			}

			if (wdev) {
				netDev = (struct net_device *) wdev->if_dev;

				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"CFG: mwctl set %s %s\n",
					(netDev && netDev->name)?netDev->name:"N/A", this_char);
			}
		}
#endif /* DBG_ENHANCE */
#endif /* DBG */
		value = strchr(this_char, '=');

		if (value != NULL)
			*value++ = 0;

		if (!value
#ifdef WSC_AP_SUPPORT
			&& (
				(strcmp(this_char, "WscStop") != 0) &&
				(strcmp(this_char, "ser") != 0) &&
				(strcmp(this_char, "WscGenPinCode") != 0)
			)
#endif /* WSC_AP_SUPPORT */
		   )
			continue;

		for (SET_PROC = RTMP_PRIVATE_AP_SUPPORT_PROC; SET_PROC->name; SET_PROC++) {
			if (rtstrcasecmp(this_char, SET_PROC->name) == TRUE) {
				if (!SET_PROC->set_proc(pAd, value)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}

				break;  /*Exit for loop. */
			}
		}

		if (SET_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
				"nl80211::(mwctl) Command not Support [%s=%s]\n", this_char,
				 value);
			break;
		}
	}

	return 0;
}

int mtk_cfg80211_vndr_show_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX + 1];
	char *cmd_string;
	int cmd_len;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	static struct nla_policy policy[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX + 1] = {
		[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR] = { .type = NLA_STRING },
	};
	ULONG priv_flags;
	POS_COOKIE pObj;
	UCHAR *tmp = NULL;
	INT Status = NDIS_STATUS_SUCCESS;
	int ret;
	RTMP_STRING *this_char, *value = NULL;
	struct sk_buff *skb = NULL;
	int msg_len = 0;
	RTMP_STRING *msg = NULL;
	UINT32 max_len = 4096;
	int free_len = max_len;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX, (struct nlattr *)data,
		len, policy, NULL);

	if (ret)
		return -EINVAL;

	if (!tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR]) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"no MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR attr\n");
		return -EINVAL;
	}

	cmd_string = nla_data(tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR]);
	cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR]);

	if (strlen(cmd_string) > cmd_len)
		return -EINVAL;
	net_dev = wdev->netdev;

	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR) * max_len);
	if (msg == NULL)
		return -EFAULT;
	memset(msg, 0x00, max_len);

	tmp = cmd_string;
	while ((this_char = strsep((char **)&tmp, ",")) != NULL) {
		if (!*this_char)
			continue;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Before check, this_char=%s\n", this_char);
		value = strchr(this_char, '=');

		if (value) {
			if (strlen(value) > 1) {
				*value = 0;
				value++;
			} else
				value = NULL;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"After check, this_char=%s, value=%s\n",
			this_char, (value == NULL ? "" : value));

		for (PRTMP_PRIVATE_AP_SHOW_PROC = RTMP_PRIVATE_AP_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_AP_SHOW_PROC->name;
		PRTMP_PRIVATE_AP_SHOW_PROC++) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"PRTMP_PRIVATE_AP_SHOW_PROC->name=%s\n",
				PRTMP_PRIVATE_AP_SHOW_PROC->name);
			if (rtstrcasecmp(this_char, PRTMP_PRIVATE_AP_SHOW_PROC->name) == TRUE) {
				if (value) {
					ret = snprintf(msg + msg_len, free_len, value);
					if (os_snprintf_error(free_len, ret)) {
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
						"msg snprintf error!\n");
					}
					msg_len += ret;
					free_len -= ret;
				}

				if (!PRTMP_PRIVATE_AP_SHOW_PROC->set_proc(pAd, msg)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}
				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_AP_SHOW_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"nl80211::(mwctl) Command not Support [%s=%s]\n", this_char, value);
			break;
		}

		if (value != NULL && rtstrcasecmp(value, "stdout") == true && rtstrcasecmp(msg, "stdout") == false) {
			msg_len = strlen(msg);
			if (msg_len == 0) {
				ret = snprintf(msg, free_len, "===>Error command format!");
				if (os_snprintf_error(free_len, ret)) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
						"msg snprintf error!\n");
					goto error;
				}
				msg_len += ret;
				free_len -= ret;
			}

			skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_len + 1);
			if (!skb) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"fail to allocate reply msg\n");
				goto error;
			}

			if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_RSP_STR, msg_len + 1, msg)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"fail to put nla to skb\n");
				kfree_skb(skb);
				goto error;
			}

			ret = mt_cfg80211_vendor_cmd_reply(skb);
			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_NOTICE,
					"reply msg failed\n");
				goto error;
			}
		}
	}
	if (msg)
		os_free_mem(msg);
	return 0;
error:
	if (msg)
		os_free_mem(msg);
	return -EFAULT;
}

#ifdef DOT11_EHT_BE

static const struct
nla_policy SUBCMD_VENDOR_MLO_INFO_POLICY[];

int mtk_cfg80211_vndr_mlo_info_handler(struct wiphy *wiphy,
			  struct wireless_dev *wdev,
			  const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_ATTR_MAX + 1];
	UCHAR value;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"pPriv is NULL\n");
		return -EFAULT;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
					"pObj is NULL\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_ATTR_MAX, (struct nlattr *)data,
					len, SUBCMD_VENDOR_MLO_INFO_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR]) {
		value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR]);

		if (value == 1)
			show_sta_mlo_link_info(pAd, NULL);
		else {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
					"error value=%d\n", value);
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_DUMP_APCLI_MLD]) {
		ret = mtk_cfg80211_reply_apcli_mld(wiphy);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

static const struct
nla_policy SUBCMD_VENDOR_MLO_SWITH_POLICY[];

int mkt_cfg80211_vndr_mlo_switch_handler(struct wiphy *wiphy,
			  struct wireless_dev *wdev,
			  const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[_MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_ATTR_MAX + 1];
	UCHAR value;
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"pPriv is NULL\n");
		return -EFAULT;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"pObj is NULL\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, _MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_ATTR_MAX, (struct nlattr *)data,
					len, SUBCMD_VENDOR_MLO_SWITH_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (!tb[MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET]) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"no MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR attr\n");
		return -EINVAL;
	}

	value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET]);

	mtk_cfg80211_vndr_cmd_set_mlo_switch_attributes(pAd, value);

	return 0;

}
#endif

int nl80211_send_acs_complete_event(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wiphy *wiphy = pAd->pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct sk_buff *skb;
struct acs_req_params {
	u32 pri_frequency;
	u32 sec_frequency;
	u8 hw_mode;
	u8 edmg_channel;
	u8 seg0_center_channel;
	u8 seg1_center_channel;
	u16 ch_bw;
} params = {0};
	UCHAR band_idx = CFG80211_Get_BandId(pAd);
	INT len = sizeof(struct acs_req_params);


/* hostapd just need the pri_frequency, so other params are zero.
 * hw_mode is set to MTK_ACS_MODE_IEEE80211A for upping hostapd`s interface.
 */
	params.pri_frequency = ieee80211_channel_to_frequency(wdev->channel, band_idx);
	params.hw_mode = MTK_ACS_MODE_IEEE80211A;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE, "\n");

	skb = cfg80211_vendor_event_alloc(wiphy, NULL, len, MTK_NL80211_VENDOR_EVENT_ACS_COMPLETE_EVENT, GFP_ATOMIC);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_PRIMARY_FREQUENCY, params.pri_frequency) ||
	nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_SECONDARY_FREQUENCY, params.sec_frequency) ||
	nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_HW_MODE, params.hw_mode) ||
	nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_EDMG_CHANNEL, params.edmg_channel) ||
	nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_VHT_SEG0_CENTER_CHANNEL, params.seg0_center_channel) ||
	nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_VHT_SEG1_CENTER_CHANNEL, params.seg1_center_channel) ||
	nla_put_u16(skb, MTK_NL80211_VENDOR_ATTR_EVENT_ACS_COMPLETE_CHWIDTH, params.ch_bw)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_ATOMIC);
	return 0;
}

static int mkt_cfg80211_vndr_offload_acs_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *tdev;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	tdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wdev);
	if (tdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"tdev is NULL\n");
		return -EFAULT;
	}

	nl80211_send_acs_complete_event(pAd, tdev);
	return 0;
}
int mtk_cfg80211_vndr_statistic_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	POS_COOKIE pObj;
	ULONG priv_flags;
	RTMP_STRING *msg;
	UINT max_len = 2048;
	INT32 msg_len;
	struct wifi_dev *pwdev;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	if (!wdev->netdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev->netdev is NULL\n");
		return -EFAULT;
	}

	priv_flags = RT_DEV_PRIV_FLAGS_GET(wdev->netdev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, wdev->netdev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (!pwdev)
		return -EINVAL;

	os_alloc_mem(pAd, (UCHAR **)&msg, max_len);

	if (msg == NULL)
		return -ENOMEM;

	memset(msg, 0x00, max_len);

	msg_len = RTMPGetStatisticsStr(pAd, pObj, msg, max_len);
	if (msg_len < 0) {
		ret = -EFAULT;
		goto error;
	} else if (msg_len == 0) {
		ret = snprintf(msg, max_len, "===>Error Statictics string!");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"msg snprintf error!\n");
			ret = -EINVAL;
			goto error;
		}
		msg_len += ret;
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_STATISTICS_STR,
						msg, msg_len + 1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

error:
	os_free_mem(msg);
	return ret;
}

static const struct
nla_policy SUBCMD_AP_RFEATURE_POLICY[];

int mtk_cfg80211_vndr_set_ap_rfeature_handler(struct wiphy *wiphy,
					  struct wireless_dev *wl_dev,
					  const void *data, int len)
{
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_RFEATURE_ATTR_MAX + 1];
	unsigned char gi = 0, ltf = 0, ack_plcy = 0, ppdu_type = 0;
	char temp_buf[64] = "\0";

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"pPriv is NULL\n");
		return -EFAULT;
	}

	if (!wl_dev) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	net_dev = wl_dev->netdev;
	if (!net_dev) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_RFEATURE_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_AP_RFEATURE_POLICY, NULL);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"nla_parse error\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI]) {
		gi = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI]);
		ap_set_rfeature_he_gi(pAd, (VOID *)&gi);
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"gi %d\n", gi);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF]) {
		ltf = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF]);
		ap_set_rfeature_he_ltf(pAd, (VOID *)&ltf);
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"ltf %d\n", ltf);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE]) {
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
		nla_strlcpy(temp_buf, tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE],
					ARRAY_SIZE(temp_buf));
#else
		nla_strscpy(temp_buf, tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE],
					ARRAY_SIZE(temp_buf));
#endif
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"trig_cfg %s\n", temp_buf);

		if (strlen(temp_buf) > 0 &&
			strlen(temp_buf) < nla_len(tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE])) {
			unsigned int recv_cnt = 0;
			unsigned int enable = 0;
			unsigned int trig_type = 0;
			char trig_type_buf[64] = "\0";

			recv_cnt = sscanf(temp_buf, "%d,%d", &enable, &trig_type);
			if (recv_cnt == 2) {
				if (enable == 0) {
					ap_set_rfeature_dis_trig_type(pAd, (VOID *)&trig_type);
				} else {
					ret = snprintf(trig_type_buf, sizeof(trig_type_buf), "%d", trig_type);
					if (os_snprintf_error(sizeof(trig_type_buf), ret)) {
						MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
							"snprintf error!\n");
						return -EFAULT;
					}
					ap_set_rfeature_trig_type(pAd, (VOID *)trig_type_buf);
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
						"recv_cnt(%d) doesn't match with number of rule!\n",
						recv_cnt);
			}
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY]) {
		ack_plcy = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY]);
		ap_set_rfeature_ack_policy(pAd, (VOID *)&ack_plcy);
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"ack_plcy %d\n", ack_plcy);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE]) {
		ppdu_type = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE]);
		ap_set_rfeature_ppdu_tx_type(pAd, (VOID *)&ppdu_type);
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"ppdu_type %d\n", ppdu_type);
	}

	return 0;
}


static const struct
nla_policy SUBCMD_AP_WIRELESS_POLICY[];

int mtk_cfg80211_vndr_set_ap_wireless_handler(struct wiphy *wiphy,
	struct wireless_dev *wl_dev,
	const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MAX + 1];
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	struct _RTMP_CHIP_CAP *chip_cap = NULL;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	unsigned char mcs = 0, ofdma = 0, ppdu_type = 0;
	unsigned char nuser_ofdma = 0, mimo = 0;
	unsigned char ampdu = 0, amsdu = 0;
	unsigned short ba_buff_size = 0, he_txop_rts_thld = 0;
	UINT16 max_ba_wsize = 0;
	char temp_buf[64] = "\0";

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"__pPriv is NULL\n");
		return -EFAULT;
	}

	if (!wl_dev) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	net_dev = wl_dev->netdev;
	if (!net_dev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"net_dev is NULL\n");
		return -EFAULT;
	}

	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pObj is NULL\n");
		return -EFAULT;
	}
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MAX, (struct nlattr *)data,
		len, SUBCMD_AP_WIRELESS_POLICY, NULL);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse error\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS]) {
		mcs = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS]);
		wlan_config_set_fixed_mcs(wdev, mcs);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mcs %d\n", mcs);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA]) {
		ofdma = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA]);
		if ((ofdma == CAPI_OFDMA_DL) ||
			(ofdma == CAPI_OFDMA_DL_20n80)) {
			wlan_config_set_ofdma_direction(wdev, ofdma);
			wlan_config_set_mu_dl_ofdma(wdev, 1);
			if (wlan_config_get_etxbf(wdev) != SUBF_ALL) {
				wlan_config_set_etxbf(wdev, SUBF_ALL);
				bf_type_ctrl(pAd);
			}
		} else if (ofdma == CAPI_OFDMA_UL) {
			wlan_config_set_ofdma_direction(wdev, ofdma);
			wlan_config_set_mu_ul_ofdma(wdev, 1);
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"ofdma %d\n", ofdma);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE]) {
		ppdu_type = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE]);
		wlan_config_set_ppdu_tx_type(wdev, ppdu_type);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"ppdu_type %d\n", ppdu_type);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA]) {
		nuser_ofdma = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA]);
		wlan_config_set_ofdma_user_cnt(wdev, nuser_ofdma);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nuser_ofdma %d\n", nuser_ofdma);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE]) {
		ba_buff_size = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE]);
		max_ba_wsize = ba_get_default_max_ba_wsize(wdev, pAd);
		wlan_config_set_ba_txrx_wsize(wdev, ba_buff_size, ba_buff_size, max_ba_wsize);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"ba_buff_size %d\n", ba_buff_size);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO]) {
		mimo = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO]);
		if (mimo == CAPI_MIMO_DL) {
			wlan_config_set_mu_dl_mimo(wdev, 1);
		} else if (mimo == CAPI_MIMO_UL) {
			wlan_config_set_mu_ul_mimo(wdev, 1);
		} else if (mimo == CAPI_MIMO_DL_UL) {
			wlan_config_set_mu_dl_mimo(wdev, 1);
			wlan_config_set_mu_ul_mimo(wdev, 1);
		} else {
			/* by default */
			wlan_config_set_mu_dl_mimo(wdev, 1);
		}
		/* ignore it, get from ap_set_wireless_bss_configs by profile */
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mimo %d\n", mimo);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU]) {
		ampdu = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU]);
		wlan_config_set_ba_enable(wdev, ampdu);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"ampdu %d\n", ampdu);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU]) {
		amsdu = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU]);
		wlan_config_set_amsdu_en(wdev, amsdu);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"amsdu %d\n", amsdu);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT]) {
		unsigned int recv_cnt = 0;
		unsigned char drv_cert_en = 0, fw_cert_en = 0;

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
		nla_strlcpy(temp_buf, tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT],
					ARRAY_SIZE(temp_buf));
#else
		nla_strscpy(temp_buf, tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT],
					ARRAY_SIZE(temp_buf));
#endif
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"cert %s\n", temp_buf);

		recv_cnt = sscanf(temp_buf, "%hhu-%hhu", &drv_cert_en, &fw_cert_en);
		if (recv_cnt == 2) {
			pAd->CommonCfg.wifi_cert = drv_cert_en;
#ifdef WIFI_UNIFIED_COMMAND
			if (chip_cap->uni_cmd_support)
				UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &fw_cert_en);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &fw_cert_en);

			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"drv_cert_en = %u, fw_cert_en = %u\n", drv_cert_en, fw_cert_en);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"recv_cnt(%d) doesn't match with number of rule!\n", recv_cnt);
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD]) {
		he_txop_rts_thld = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD]);
		if (he_txop_rts_thld > MAX_TXOP_DURATION_RTS_THRESHOLD) {
			MTWF_DBG(pAd, DBG_CAT_CFG,
				 CATCFG_DBGLOG,
				 DBG_LVL_ERROR,
				 "incorrect value:%d\n", he_txop_rts_thld);
			return -EINVAL;
		}
		set_txop_dur_rts_thld(wdev, he_txop_rts_thld);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"he_txop_rts_thld = %d\n", he_txop_rts_thld);
	}

	return 0;
}

static const struct
nla_policy SUBCMD_SET_COUNTRY_POLICY[];

int mtk_cfg80211_vndr_country_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_MAX + 1];

	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd = NULL;

	int ret = 0;

	UCHAR *p_country_code = NULL;
	UCHAR *p_country_string = NULL;
	UINT32 region;
	BOOLEAN b_set_code = FALSE, b_set_region = FALSE, b_set_name = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE, "\n");

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "__pPriv is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "====>\n");

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_COUNTRY_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_COUNTRY_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_parse failed.\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE]) {
		p_country_code = nla_data(tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE]);
		if (strlen(p_country_code) <= nla_len(tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE]))
			b_set_code = TRUE;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION]) {
		region = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION]);
		b_set_region = TRUE;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME]) {
		p_country_string = nla_data(tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME]);
		if (strlen(p_country_string) <= nla_len(tb[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME]))
			b_set_name = TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		 "b_set_code=%d, b_set_region=%d, b_set_name=%d\n", b_set_code, b_set_region,
		 b_set_name);

	if (b_set_code) {
		ret = mtk_cfg80211_vndr_cmd_set_country_code(pAd, p_country_code);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Set country code failed! ret=%x\n", ret);
			return ret;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Set country code to %s succeed.\n", p_country_code);
	}

	if (b_set_region) {
		ret = mtk_cfg80211_vndr_cmd_set_country_region(pAd, region);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Set region failed! ret=%x\n", ret);
			return ret;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Set region to %d succeed.\n", region);
	}

	if (b_set_name) {
		ret = mtk_cfg80211_vndr_cmd_set_country_string(pAd, p_country_string);

		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "Set country string failed! ret=%x\n", ret);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Set country string to %s succeed.\n", p_country_string);
	}

	RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT, &(pAd->CommonCfg.CountryCode),
						sizeof(pAd->CommonCfg.CountryCode));
	return ret;
}

static const struct
nla_policy SUBCMD_SET_ACTION_POLICY[];

int mtk_cfg80211_vndr_set_action(struct wiphy *wiphy,
						struct wireless_dev *wl_dev,
						const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_MAX + 1];
	int ret;
	u16 punctured_bitmap = 0;
	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"ifname: %s, iftype: %d\n", pNetDev->name, wl_dev->iftype);

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SET_ACTION_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_ACTION_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]nla_parse fail .\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME]) {
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
		struct action_frm_data *off_ch_action_data;
		int len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME]);

//TODO: Add if needed in future
		if (len < sizeof(struct action_frm_data)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid data length %d\n", len);
			return -EINVAL;
		}
		off_ch_action_data = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME]);

		if (len < (sizeof(struct action_frm_data) + off_ch_action_data->frm_len)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid frm length %u\n", off_ch_action_data->frm_len);
			return -EINVAL;
		}
		mtk_send_offchannel_action_frame(pAd, wdev, off_ch_action_data);
#endif /* CHANNEL_SWITCH_MONITOR_CONFIG */
	}


	if (tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME]) {
#ifdef QOS_R1
		struct action_frm_data *qos_action_data;
		int data_len;

		data_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME]);
		if ((data_len < sizeof(*qos_action_data)) ||
		    (data_len > MAX_MGMT_PKT_LEN)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid data length %d\n", data_len);
			return -EINVAL;
		}

		qos_action_data = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME]);
		if (qos_action_data->frm_len != (data_len - sizeof(*qos_action_data))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid frm length %u\n", qos_action_data->frm_len);
			return -EINVAL;
		}

		QoS_send_action_frame(pAd, wdev, qos_action_data);
#endif /* QOS_R1 */
	}
#ifdef CONFIG_MAP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_STATIC_PUNCTURE_BITMAP]) {
		punctured_bitmap = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_STATIC_PUNCTURE_BITMAP]);
		MTWF_PRINT("%s static punc bitmap value from wapp:%d\n", __func__, punctured_bitmap);
		ret = mtk_nl80211_set_punc_bitmap(pAd, wiphy, wl_dev, punctured_bitmap);
		if (!ret) {
			MTWF_PRINT("%s failed - [ERROR]mtk_nl80211_set_punc_bitmap fail\n", __func__);
			return -EINVAL;
		}
	}
#endif /* CONFIG_MAP_SUPPORT */
	if (tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST]) {
		struct nl_80211_t2lm_cmd *t2lm_cmd;
		int data_len;

		data_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST]);
		if (data_len != sizeof(*t2lm_cmd)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid t2lm_cmd length %d\n", data_len);
			return -EINVAL;
		}

		t2lm_cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST]);
		mtk_cfg80211_nt2lm_request(pAd, wdev, t2lm_cmd);

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"%s, t2lm_cmd->ta_addr:%pM, t2lm_cmd->ra_addr: %pM\n",
				wdev->if_dev->name, t2lm_cmd->ta_addr, t2lm_cmd->ra_addr);
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING]) {
		void *data = NULL;
		int data_len;

		data_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING]);
		if (data_len <= 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR] invalid t2lm_cmd t2lmmap %d\n", data_len);
			return -EINVAL;
		}

		data = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING]);
		set_ap_at2lm_setting(pAd, data);
	}
	return 0;
}

static const struct
nla_policy VENDOR_SUBCMD_QOS_POLICY[];

int mtk_cfg80211_vndr_qos(struct wiphy *wiphy,
						struct wireless_dev *wl_dev,
						const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_QOS_MAX + 1];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"ifname: %s, iftype: %d\n", pNetDev->name, wl_dev->iftype);

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_QOS_MAX, (struct nlattr *)data,
		len, VENDOR_SUBCMD_QOS_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]nla_parse fail .\n");
		return -EINVAL;
	}

#ifdef QOS_R1
	if (tb[MTK_NL80211_VENDOR_ATTR_QOS_UP_TUPLE_EXPIRED_NOTIFY]) {
#ifdef MSCS_PROPRIETARY
		PMAC_TABLE_ENTRY pEntry;
		struct wapp_vend_spec_classifier_para_report *classifier_para = NULL;

		classifier_para = nla_data(tb[MTK_NL80211_VENDOR_ATTR_QOS_UP_TUPLE_EXPIRED_NOTIFY]);

		if (classifier_para->id > MAX_QOS_PARAM_TBL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"para id is out of range.\n");
		} else {
			pEntry = MacTableLookup(pAd, classifier_para->sta_mac);
			if (pEntry) {
				delete_qos_param(pAd, classifier_para->id);
				QoS_send_mscs_rsp(pAd, pEntry->wcid, 0, TCLAS_PROCESSING_TERMINATED, classifier_para);
			}
		}
#endif /* MSCS_PROPRIETARY */
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC]) {
		PUCHAR pStaMacAddr = NULL;
		unsigned int macAddrLen;
		PMAC_TABLE_ENTRY  pEntry = NULL;

		pStaMacAddr = nla_data(tb[MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC]);
		macAddrLen = nla_len(tb[MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC]);

		if (macAddrLen > 6u) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
							"macAddrLen length check fail!\n");
			return -EINVAL;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"get pmk of STA:"MACSTR".\n", MAC2STR(pStaMacAddr));

		pEntry = MacTableLookup(pAd, pStaMacAddr);

		ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC,
						pEntry->SecConfig.PMK, LEN_PMK);
		if (ret != 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
			return ret;
		}
	}
#ifdef QOS_R3
	if (tb[MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE]) {
		struct qos_characteristics *qosdata = NULL;
		unsigned int datalen;

		qosdata = nla_data(tb[MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE]);
		datalen = nla_len(tb[MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE]);

		if (datalen != sizeof(*qosdata)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"qosdata length=%d check fail!\n", datalen);
			return -EINVAL;
		}
		QoS_set_qos_characteristics_ie(pAd, qosdata, wdev);
	}
#endif
#endif /* QOS_R1 */
#ifdef EPCS_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC]) {
		struct epcs_sta *sta = NULL;
		unsigned int datalen = 0;

		sta = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC]);
		datalen = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC]);

		if (datalen != sizeof(*sta)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"qosdata length=%d check fail!\n", datalen);
			return -EINVAL;
		}
		epcs_sta_state_sync(pAd, wdev, sta);
	}
#endif

	return 0;
}

#ifdef OFFCHANNEL_SCAN_FEATURE
static const struct
nla_policy SUBCMD_OFFCHANNEL_INFO_POLICY[];

int mtk_cfg80211_vndr_subcmd_offchannel_info(struct wiphy *wiphy,
							struct wireless_dev *wl_dev,
							const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_MAX + 1];
	int ret, i = 0;
	OFFCHANNEL_SCAN_MSG msg;
	int offchannel_len;
	SCAN_CTRL *ScanCtrl = NULL;
	int Status = CHANNEL_MONITOR_STRG_SUCCESS;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"ifname: %s, iftype: %d\n", pNetDev->name, wl_dev->iftype);

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) {
		struct DOT11_H *pDot11h = &pAd->Dot11_H;

		if (pDot11h && (pDot11h->ChannelMode == CHAN_SILENCE_MODE || pDot11h->ChannelMode == CHAN_NOP_MODE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"radio is down, can not trigger scan!!!\n");
			return -EINVAL;
		}
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_MAX,
		    (struct nlattr *)data, len, SUBCMD_OFFCHANNEL_INFO_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"failed - [ERROR]nla_parse fail .\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO]) {
		offchannel_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO]);
		if (offchannel_len != sizeof(OFFCHANNEL_SCAN_MSG)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"offchannel_len error\n");
			return -EINVAL;
		}

		memcpy(&msg,
			(OFFCHANNEL_SCAN_MSG *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO]),
			offchannel_len);

		ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
		if (!ScanCtrl)
			return -EOPNOTSUPP;
		memcpy(ScanCtrl->if_name, msg.ifrn_name, IFNAMSIZ);
		switch (msg.Action) {
		case GET_OFFCHANNEL_INFO:
		{
			/*To do OffChannelScan, need TakeChannelOpCharge first*/
			if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"TakeChannelOpCharge fail for Off-Channel SCAN!!\n");
				return CHANNEL_MONITOR_STRG_FAILURE;
			}

			if (scan_in_run_state(pAd, wdev) == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					 "Scan in running State\n");
				ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
				return CHANNEL_MONITOR_STRG_FAILURE;
			}

			if (ScanCtrl->state != OFFCHANNEL_SCAN_INVALID) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"failed because offchannel scan is still ongoing\n");
				ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
				return CHANNEL_MONITOR_STRG_FAILURE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						"num of away channel to scan = %d\n",
						 msg.data.offchannel_param.Num_of_Away_Channel);

				if (msg.data.offchannel_param.Num_of_Away_Channel > MAX_AWAY_CHANNEL) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						 "Invalid Argument\n");
					ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
					return CHANNEL_MONITOR_STRG_INVALID_ARG;
				}

				ScanCtrl->Num_Of_Channels = msg.data.offchannel_param.Num_of_Away_Channel;
				ScanCtrl->Off_Ch_Scan_BW = msg.data.offchannel_param.bw;
				/* Fillup the parameters received for all channels */
				for (i = 0; i < msg.data.offchannel_param.Num_of_Away_Channel; i++) {
					ScanCtrl->ScanGivenChannel[i] = msg.data.offchannel_param.channel[i];
					/* TODO: Raghav: update on the first ch scan also */
					ScanCtrl->Offchan_Scan_Type[i] = msg.data.offchannel_param.scan_type[i];
					ScanCtrl->ScanTime[i] = msg.data.offchannel_param.scan_time[i];
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
							"channel = %d:scan type = %d:scan time = %d\n",
							 ScanCtrl->ScanGivenChannel[i],
							 ScanCtrl->Offchan_Scan_Type[i], ScanCtrl->ScanTime[i]);
				}
				Status = ApSiteSurveyNew_by_wdev(pAd, msg.data.offchannel_param.channel[0],
							msg.data.offchannel_param.scan_time[0],
							msg.data.offchannel_param.scan_type[0], FALSE, wdev);
			}
		}
			break;
		case TRIGGER_DRIVER_CHANNEL_SWITCH:
			/*To do set channel, need TakeChannelOpCharge first*/
			if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"TakeChannelOpCharge fail for SET channel!!\n");
				return FALSE;
			}
			pAd->ApCfg.iwpriv_event_flag = TRUE;

			RTMP_OS_REINIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);
			Status = rtmp_set_channel(pAd, wdev, msg.data.channel_data.channel);

			if (pAd->ApCfg.set_ch_async_flag == TRUE) {
				ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
				if (ret)
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"wait channel setting success.\n");
				else {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"wait channel setting timeout.\n");
					pAd->ApCfg.set_ch_async_flag = FALSE;
				}
			}
			pAd->ApCfg.iwpriv_event_flag = FALSE;

			/*if channel setting is DONE, release ChannelOpCharge here*/
			ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);
			break;
		case UPDATE_DRIVER_SORTED_CHANNEL_LIST:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				 "num of channels in sorted channel list received from App =%d\n",
				  msg.data.sorted_channel_list.size);
			if (msg.data.sorted_channel_list.size > 0) {
				pAd->sorted_list.size = msg.data.sorted_channel_list.size;
				for (i = 0; i < msg.data.sorted_channel_list.size; i++) {
					pAd->sorted_list.SortedMaxChannelBusyTimeList[i] = msg.data.sorted_channel_list.SortedMaxChannelBusyTimeList[i];
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
							"channel[%d] = %d\n", i, pAd->sorted_list.SortedMaxChannelBusyTimeList[i]);
					pAd->sorted_list.SortedMinChannelBusyTimeList[i] = msg.data.sorted_channel_list.SortedMinChannelBusyTimeList[i];
				}
				Status = CHANNEL_MONITOR_STRG_SUCCESS;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"num of channels in sorted channel list received is invalid\n");
				Status = CHANNEL_MONITOR_STRG_FAILURE;
			}
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
					"unknown action code. (%d)\n", msg.Action);
			break;
		}

		if (Status != CHANNEL_MONITOR_STRG_SUCCESS)
			return -EINVAL;
	}

	return 0;
}
#endif /* OFFCHANNEL_SCAN_FEATURE */

static const struct
nla_policy SUBCMD_DFS_POLICY[];

int mtk_cfg80211_vndr_subcmd_dfs(struct wiphy *wiphy,
						struct wireless_dev *wl_dev,
						const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_DFS_MAX + 1];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_NOTICE,
			"ifname: %s, iftype: %d\n", pNetDev->name, wl_dev->iftype);

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_DFS_MAX, (struct nlattr *)data,
		len, SUBCMD_DFS_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
			"failed - [ERROR]nla_parse fail .\n");
		return -EINVAL;
	}

#ifdef MT_DFS_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_DFS_SET_ZERO_WAIT])
		ZeroWaitDfsMsgHandle(pAd, nla_data(tb[MTK_NL80211_VENDOR_ATTR_DFS_SET_ZERO_WAIT]));

	if (tb[MTK_NL80211_VENDOR_ATTR_DFS_GET_ZERO_WAIT]) {
		union dfs_zero_wait_msg msg;

		os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));
#ifdef MAP_R2
		msg.aval_channel_list_msg.Action = QUERY_AVAL_CH_LIST;
#endif /* MAP_R2 */
		ZeroWaitDfsQueryMsgHandle(pAd, (char *)&msg);

		ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_DFS_GET_ZERO_WAIT,
						&msg, sizeof(union dfs_zero_wait_msg));
		if (ret != 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
			return ret;
		}
	}
#endif /* MT_DFS_SUPPORT */

	if (tb[MTK_NL80211_VENDOR_ATTR_DFS_CAC_STOP]) {
		int i = 0;
		UCHAR BandIdx = 0;
		BOOLEAN bSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);

		BandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_INFO,
			 "OID_802_11_CAC_STOP!!\n");

#ifdef A_BAND_SUPPORT
		if (bSupport5G && (pAd->CommonCfg.bIEEE80211H == 1)) {
			BOOLEAN BandInCac[CFG_WIFI_RAM_BAND_NUM];
			struct DOT11_H *pDot11hTest = NULL;

			for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++)
				BandInCac[i] = FALSE;

			pDot11hTest = &pAd->Dot11_H;
			if (pDot11hTest == NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
					"pDot11hTest == NULL!\n");
				return -EINVAL;
			}

#ifdef MT_DFS_SUPPORT
			if (pDot11hTest->ChannelMode == CHAN_SILENCE_MODE) {
#ifdef MAP_R2
				BandInCac[BandIdx] = TRUE;
#endif /* MAP_R2 */
				pDot11hTest->RDCount = 0;
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
				pDot11hTest->ChannelMode = CHAN_NORMAL_MODE;
			}
		}
#endif /* MT_DFS_SUPPORT */
#endif /* A_BAND_SUPPORT */
	}
	return 0;
}

static const struct
nla_policy SUBCMD_EASYMESH_POLICY[];

static int mtk_cfg80211_reply_ap_mld(struct wiphy *wiphy, struct bmgr_mlo_dev *mld);

int mtk_cfg80211_vndr_subcmd_easymesh(struct wiphy *wiphy,
						struct wireless_dev *wl_dev,
						const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_MAX + 1];
	ULONG priv_flags;
	POS_COOKIE pObj;
	char *cmd;
	int ret, cmd_len;
	UCHAR map_en = 0;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"ifname: %s, iftype: %d\n", pNetDev->name, wl_dev->iftype);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pObj in driver .\n");
		return -EINVAL;
	}

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_EASYMESH_MAX, (struct nlattr *)data,
		len, SUBCMD_EASYMESH_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]nla_parse fail .\n");
		return -EINVAL;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID]);

		ret = mtk_nl80211_easymesh_set_ssid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ssid fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE]);

		ret = mtk_nl80211_easymesh_set_wireless_mode(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_wireless_mode fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL]);

		ret = mtk_nl80211_easymesh_set_channel(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_channel fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW]);

		ret = mtk_nl80211_easymesh_set_htbw(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_htbw fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW]);

		ret = mtk_nl80211_easymesh_set_vhtbw(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_vhtbw fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW]);

		ret = mtk_nl80211_easymesh_set_ehtbw(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ehtbw fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_CREATE]) {
		struct mld_group *req_mld;
		struct bmgr_mlo_dev *mld;

		req_mld = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_CREATE]);

		ret = mtk_nl80211_set_mld_group_create(pAd, wiphy, wl_dev, req_mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_set_mld_group_create fail .\n");
			return -EINVAL;
		}

		mld = GET_MLD_BY_GRP_IDX(req_mld->mld_group_idx);
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]invalid mld group.\n");
			return -EINVAL;
		}

		ret = mtk_cfg80211_reply_ap_mld(wiphy, mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_ap_mld failed. ret=%x.\n", ret);
			return -EINVAL;
		}
	}
#ifdef MAP_R6
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_RECONF]) {
		struct mld_reconf *req_mld;

		req_mld = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_RECONF]);

		ret = mtk_nl80211_set_mld_link_reconf(pAd, wiphy, wl_dev, req_mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_set_map_channel fail .\n");
			return -EINVAL;
		}
	}
#endif /* MAP_R6 */

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY]);

		ret = mtk_nl80211_easymesh_set_mld_group_delete(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_mld_group_del fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_ADD]) {
		struct mld_add_link *req_mld;

		req_mld = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_ADD]);

		ret = mtk_nl80211_easymesh_set_mld_link_add(pAd, wl_dev, req_mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh__add_mld_link fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL]);

		ret = mtk_nl80211_easymesh_set_mld_del_link(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_del_link fail .\n");
			return -EINVAL;
		}
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER]);

		ret = mtk_nl80211_easymesh_set_mld_link_transfer(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_tranfer_mld_link fail .\n");
			return -EINVAL;
		}
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SR_ENABLE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SR_ENABLE]);

		ret = mtk_nl80211_easymesh_sr_enable(pAd, wl_dev, cmd);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"failed - [ERROR]nl80211_easymesh_sr_enable fail.\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_SR_BITMAP]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_SR_BITMAP]);

		ret = mtk_nl80211_easymesh_set_bh_sr_bitmap(pAd, wl_dev, cmd);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"failed - [ERROR]nl80211_easymesh_sr_enable fail.\n");
			return -EINVAL;
		}
	}



	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID]);

		ret = mtk_nl80211_easymesh_set_hidden_ssid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_hidden_ssid fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_auth_mode(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_auth_mode fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_encrypt_type(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_encrypt_type fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID]);

		ret = mtk_nl80211_easymesh_set_default_key_id(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_default_key_id fail .\n");
			return -EINVAL;
		}
	}


	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_key1(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_key1 fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_wpa_psk(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_wpa_psk fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON]);

		ret = mtk_nl80211_easymesh_set_radio_on(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_radio_on fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA]);

		ret = mtk_nl80211_easymesh_set_disconnect_sta(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_disconnect_sta fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID]);

		ret = mtk_nl80211_easymesh_set_ts_bh_primary_vid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ts_bh_primary_vid fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP]);

		ret = mtk_nl80211_easymesh_set_ts_bh_primary_pcp(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ts_bh_primary_pcp fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_ts_bh_vid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ts_bh_vid fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID]);

		ret = mtk_nl80211_easymesh_set_ts_fh_vid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_ts_fh_vid fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}
		ret = mtk_nl80211_easymesh_set_transparent_vid(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_transparent_vid fail .\n");
			return -EINVAL;
		}
	}

#ifdef DOT11K_RRM_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ]);

		if (strlen(cmd) > cmd_len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid nla length%lu, %d .\n", strlen(cmd), cmd_len);
			return -EINVAL;
		}

		ret = mtk_nl80211_easymesh_set_bcn_req(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_bcn_req fail .\n");
			return -EINVAL;
		}
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX]);

		ret = mtk_nl80211_easymesh_set_HtBssCoex(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_HtBssCoex fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC]);

		ret = mtk_nl80211_easymesh_set_PMFMFPC(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_PMFMFPC fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING]);

		ret = mtk_nl80211_easymesh_set_AutoRoaming(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_AutoRoaming fail .\n");
			return -EINVAL;
		}
	}

#ifdef QOS_R2
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE]);

		ret = mtk_nl80211_easymesh_set_DSCPPolicyEnable(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_QOS_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_DSCPPolicyEnable fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA]);

		ret = mtk_nl80211_easymesh_set_QosMapCapaEnable(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_QOS_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]nl80211_easymesh_set_QosMapCapa fail .\n");
			return -EINVAL;
		}
	}
#ifdef MAP_R5
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_RESET_DSCP2UP_TABLE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_RESET_DSCP2UP_TABLE]);
		if (cmd && *cmd)
			QoS_Init_DSCP2UP_Mapping(pAd);
	}
#endif
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK]);

		ret = mtk_nl80211_easymesh_set_psk(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_easymesh_set_psk fail .\n");
			return -EINVAL;
		}
	}

#ifdef DPP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK]);

		ret = mtk_nl80211_easymesh_set_pmk(pAd, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DPP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_easymesh_set_pmk fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME]);

		ret = mtk_nl80211_send_dpp_frame(pAd, wiphy, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DPP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_send_dpp_frame fail .\n");
			return -EINVAL;
		}
	}
#endif /* DPP_SUPPORT */

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME]);

		ret = mtk_nl80211_get_assoc_req_frame(pAd, wiphy, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_send_dpp_frame fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ]);

		ret = mtk_nl80211_get_spt_reuse_req(pAd, wiphy, wl_dev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_send_spt_reuse_req_frame fail .\n");
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE]) {
		char *ie = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE]);
		unsigned int ie_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE]);
		INT ifIndex;

		if (wl_dev->iftype == NL80211_IFTYPE_AP)
			ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
		else
			ifIndex = WDEV_NOT_FOUND;
		if (ifIndex == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]can't find ifIndex in driver .\n");
			return -EINVAL;
		}

		if (!ie || !ie_len || ie_len > len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
				"failed - invalid data.\n");
			return -EINVAL;
		}
		/* default use ap ifIndex here */
		wapp_set_ap_ie(pAd, ie, ie_len, ifIndex);
	}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_CANCEL_ROC])
		mtk_cancel_roc(pAd, wdev);
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_START_ROC]) {
		UCHAR *Buf = NULL;
		Buf = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_START_ROC]);
		mtk_start_roc(pAd, wdev, (struct roc_req *)Buf);
	}
#endif /* CHANNEL_SWITCH_MONITOR_CONFIG */

#ifdef MAP_R3
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_DEL_CCE_IE]) {
		if (!pAd->bDppEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
			return -EINVAL;
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"Del::OID_CCe_IE_DEL\n");
		if (wdev->DPPCfg.cce_ie_len) {
			os_zero_mem(wdev->DPPCfg.cce_ie_buf, 6);
			wdev->DPPCfg.cce_ie_len = 0;
			if (wdev->bAllowBeaconing)
				UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"CCE IE not enabled for %s wdev\n", RtmpOsGetNetDevName(wdev->if_dev));
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI]);
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI]);

		if (!pAd->bDppEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
			return -EINVAL;
		}

		/* If len coming from wapp is zero then clear the    *
		 * existing allocated buffer so legacy WPS can work  *
		 */
		if (cmd_len == 0) {
			if (pAd->dpp_uri_ptr) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"dpp uri ptr is not NULL and len is zero!\n");
				os_free_mem(pAd->dpp_uri_ptr);
				pAd->dpp_uri_ptr = NULL;
			} else
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"dpp uri ptr is already NULL and len is zero!\n");
			return 0;
		}

		if (pAd->dpp_uri_ptr) {
			if (cmd_len < pAd->dpp_uri_len) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"invalid nla length %d\n", cmd_len);
				return -EINVAL;
			}
			/* URI already present now need to replace it if not same*/
			if (memcmp(pAd->dpp_uri_ptr, cmd, pAd->dpp_uri_len) != 0) {
				os_free_mem(pAd->dpp_uri_ptr);
				pAd->dpp_uri_ptr = NULL;
			}
		}

		if (pAd->dpp_uri_ptr == NULL) {
			os_alloc_mem(pAd->dpp_uri_ptr, (UCHAR **)&pAd->dpp_uri_ptr, cmd_len);

			if (pAd->dpp_uri_ptr == NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"dpp uri ptr is null, return!\n");
				return -EINVAL;
			}
			pAd->dpp_uri_len = (UCHAR)cmd_len;
			memcpy(pAd->dpp_uri_ptr, cmd, cmd_len);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"URI copied for wdev:%s URI:%s len:%u\n",
				RtmpOsGetNetDevName(wdev->if_dev), pAd->dpp_uri_ptr, pAd->dpp_uri_len);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"URI already present for wdev:%s URI:%s len:%u\n",
				RtmpOsGetNetDevName(wdev->if_dev), pAd->dpp_uri_ptr, pAd->dpp_uri_len);
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_1905_SEC_ENABLED]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_1905_SEC_ENABLED]);

		if (!pAd->bDppEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
			return -EINVAL;
		}

		pAd->map_sec_enable = *cmd;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"setting the security flag %u for wdev:%s\n",
				 pAd->map_sec_enable, RtmpOsGetNetDevName(wdev->if_dev));
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_ONBOARDING_TYPE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_ONBOARDING_TYPE]);

		if (!pAd->bDppEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
			return -EINVAL;
		}

		pAd->map_onboard_type = *cmd;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"setting the onboarding type %u for wdev:%s\n",
				 pAd->map_onboard_type, RtmpOsGetNetDevName(wdev->if_dev));
	}
#endif /* MAP_R3 */

#ifdef CONFIG_MAP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS]);

		/* only do this for AP MBSS, ignore other inf type */
		if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
			UINT8 IfIdx = pObj->ioctl_if;

			wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		} else
			return -EINVAL;

		if (*cmd) {
			wdev->MAPCfg.DevOwnRole |= BIT(MAP_ROLE_BACKHAUL_BSS);
			pAd->bh_bss_wdev = wdev;
		} else {
			wdev->MAPCfg.DevOwnRole &= ~BIT(MAP_ROLE_BACKHAUL_BSS);
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"%s bandIdx = %d ,DevOwnRole 0x%x\n",
				wdev->if_dev->name, HcGetBandByWdev(wdev), wdev->MAPCfg.DevOwnRole);
	}
#ifdef WSC_INCLUDED
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS]) {
		PWSC_CTRL pWscControl = NULL;
		unsigned int i;

		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS]);

		/* only do this for AP MBSS, ignore other inf type */
		if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
			UINT8 IfIdx = pObj->ioctl_if;

			wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
			pWscControl = &pAd->ApCfg.MBSSID[IfIdx].wdev.WscControl;
		} else
			return -EINVAL;

		if (*cmd) {
			wdev->MAPCfg.DevOwnRole |= BIT(MAP_ROLE_FRONTHAUL_BSS);
		} else {
			wdev->MAPCfg.DevOwnRole &= ~BIT(MAP_ROLE_FRONTHAUL_BSS);
			/* reset wsc backhaul profiles */
			for (i = 0; i < pWscControl->WscBhProfiles.ProfileCnt; i++)
				NdisZeroMemory(&pWscControl->WscBhProfiles.Profile[i], sizeof(WSC_CREDENTIAL));
			pWscControl->WscBhProfiles.ProfileCnt = 0;
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"%s bandIdx = %d ,DevOwnRole 0x%x\n",
				wdev->if_dev->name, HcGetBandByWdev(wdev), wdev->MAPCfg.DevOwnRole);
	}
#endif /* WSC_INCLUDED */
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_CH]) {
		struct map_ch *wapp_map_ch;

		wapp_map_ch = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_CH]);

		ret = mtk_nl80211_set_map_channel(pAd, wiphy, wl_dev, wapp_map_ch);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_set_map_channel fail .\n");
			return -EINVAL;
		}
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_ENABLE]) {
		cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_ENABLE]);

		map_en = *cmd;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"MAP En value from wapp:%d\n", map_en);

		ret = mtk_nl80211_set_map_enable(pAd, wiphy, wl_dev, map_en);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_set_map_enable fail\n");
			return -EINVAL;
		}

	}
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_ASSOC_CTRL]) {
		UCHAR apIdx = pObj->ioctl_if;
		struct bh_assoc_disallow_info *bh_disallow_info = &pAd->ApCfg.MBSSID[apIdx].bh_disallow_info;
		struct bh_assoc_disallow_info *info = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_ASSOC_CTRL]);

		RTMPZeroMemory(bh_disallow_info, sizeof(struct bh_assoc_disallow_info));
		NdisMoveMemory(bh_disallow_info, info, sizeof(struct bh_assoc_disallow_info));
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"BH BSSID ["MACSTR"], disallow profile P1 %d, P2 %d!!!\n",
			MAC2STR(pAd->ApCfg.MBSSID[apIdx].bh_disallow_info.bssid),
			pAd->ApCfg.MBSSID[apIdx].bh_disallow_info.profile1_bh_assoc_disallow,
			pAd->ApCfg.MBSSID[apIdx].bh_disallow_info.profile2_bh_assoc_disallow);
	}
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DPP_STAMAC]) {
		char *peer_mac = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DPP_STAMAC]);
		struct dpp_gas_frame_list *gas_frame = NULL;
		struct dpp_gas_frame_list *gas_frame_tmp = NULL;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"Peer mac addr from wapp:"MACSTR"\n", MAC2STR(peer_mac));

		OS_SEM_LOCK(&pAd->gas_frame_list_lock);
		/* If any gas statme machime is ongoing then clear it */
		if (!DlListEmpty(&pAd->dpp_gas_event_list) && pAd->is_dpp_gas_list_init == TRUE) {
			DlListForEachSafe(gas_frame, gas_frame_tmp, &pAd->dpp_gas_event_list,
					struct dpp_gas_frame_list, List) {
				if (NdisEqualMemory(gas_frame->sta_mac, peer_mac, MAC_ADDR_LEN)) {
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"Free gas list with "MACSTR"\n", MAC2STR(gas_frame->sta_mac));
					DlListDel(&gas_frame->List);
					os_free_mem(gas_frame);
					break;
				}
			}
		}
		OS_SEM_UNLOCK(&pAd->gas_frame_list_lock);
	}
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MAP_VENDOR_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_NOP_STATE]) {
		struct cont_nop_info *wapp_nop_info;

		wapp_nop_info = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_NOP_STATE]);

		ret = mtk_nl80211_set_map_vendor_nop_info(pAd, wiphy, wl_dev, wapp_nop_info);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]mtk_nl80211_set_wapp_vendor_nop_info fail .\n");
			return -EINVAL;
		}
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_CH_PREF_STATE]) {
		struct cont_ch_info *wapp_cont_ch_pref_info;

		wapp_cont_ch_pref_info = nla_data(tb[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_CH_PREF_STATE]);

		ret = mtk_nl80211_set_map_vendor_ch_info(pAd, wiphy, wl_dev, wapp_cont_ch_pref_info);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR] mtk_nl80211_set_map_vendor_ch_info .\n");
			return -EINVAL;
		}
	}
#endif /* MAP_VENDOR_SUPPORT */

	return 0;
}
int nl80211_send_event_ACS_per_channel_info(struct wiphy *wiphy,
						void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "\n");

	skb = cfg80211_vendor_event_alloc(wiphy, NULL, len, MTK_NL80211_VENDOR_EVENT_ACS_PER_CH_INFO, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_EVENT_ACS_PER_CH_INFO, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

int nl80211_send_event_sta_link_mac(struct wiphy *wiphy, struct wireless_dev *wdev, void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, len, MTK_NL80211_VENDOR_EVENT_SEND_MLO_STA_LINK_MAC, GFP_KERNEL);

	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

int nl80211_send_event_offchannel_info(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	skb = cfg80211_vendor_event_alloc(wiphy, wdev, len, MTK_NL80211_VENDOR_EVENT_OFFCHANNEL_INFO, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_EVENT_OFF_CHANNEL_INFO, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

int nl80211_send_event_stop_disassoc_timer(struct wiphy *wiphy, struct wireless_dev *wdev, void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"calling event to stop disassoc immninet timer\n");

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, len, MTK_NL80211_VENDOR_EVENT_RX_T2LM_STOP_DISASSOC_TIMER, GFP_KERNEL);

	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

static const struct
nla_policy SUBCMD_SET_CHANNEL_POLICY[];
int mtk_cfg80211_vndr_channel_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_CHAN_MAX + 1];

	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd = NULL;

	int ret = 0;
	UCHAR chan = 0, ht_coex = 0, ext_chan = 0;
	UINT32 set_flag = 0;
	UINT32 freq = 0, bw = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	if (!__pPriv) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "__pPriv is NULL\n");
		return -EFAULT;
	}

	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_CHAN_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_CHANNEL_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "nla_parse failed.\n");
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM]) {
		chan = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM]);
		set_flag |= MTK_CFG80211_CHAN_SET_FLAG_CHAN;
	} else if (tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ]) {
		freq = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ]);
		chan = ieee80211_frequency_to_channel(freq);
		set_flag |= MTK_CFG80211_CHAN_SET_FLAG_CHAN;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW]) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
		bw = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW]);
		set_flag |= MTK_CFG80211_CHAN_SET_FLAG_BW;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN]) {
		ext_chan = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN]);
		set_flag |= MTK_CFG80211_CHAN_SET_FLAG_EXT_CHAN;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX]) {
		ht_coex = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX]);
		set_flag |= MTK_CFG80211_CHAN_SET_FLAG_COEX;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "set_flag=0x%x\n", set_flag);
	if (set_flag != 0) {
		ret = mtk_cfg80211_vndr_cmd_set_channel_attributes(
			pAd, set_flag, chan, bw, ext_chan, ht_coex);

		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"Update channel attribute failed! ret=%x\n", ret);
			return ret;
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Update channel attribute succeed.\n");
	}

	return ret;
}

static const struct
nla_policy SUBCMD_SET_ACL_POLICY[];

int mtk_cfg80211_vndr_set_acl_handler(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX + 1];

	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;

	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	INT ifIndex = 0;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	//pObj = (POS_COOKIE) (pAd->OS_Cookie);
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (pObj == NULL)
		return FALSE;
	ifIndex = pObj->ioctl_if;

	if (VALID_MBSS(pAd, ifIndex) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
		"invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX, (struct nlattr *)data,
	len, SUBCMD_SET_ACL_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_ACL_POLICY]) {
		u8 policy_mode = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_ACL_POLICY]);

		ret = cfg80211_set_acl_policy(pAd, ifIndex, policy_mode);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC]) {
		int len;
		u8 *mac_list, mac_cnt;

		len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC]);
		mac_list = nla_data(tb[MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC]);

		if (len % MAC_ADDR_LEN == 0)
			mac_cnt = len / MAC_ADDR_LEN;
		else
			return -EINVAL;

		ret = cfg80211_add_acl_entry(pAd, ifIndex, mac_list, mac_cnt);

		if (ret < 0)
			return ret;
	}


	if (tb[MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC]) {
		int len;
		u8 *mac_list, mac_cnt;

		len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC]);
		mac_list = nla_data(tb[MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC]);

		if (len % MAC_ADDR_LEN == 0)
			mac_cnt = len / MAC_ADDR_LEN;
		else
			return -EINVAL;

		ret = cfg80211_del_acl_entry(pAd, ifIndex, mac_list, mac_cnt);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_ACL_SHOW_ALL]) {

		ret = cfg80211_get_acl_list(wiphy, pAd, ifIndex);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_ACL_CLEAR_ALL]) {

		ret = cfg80211_clear_acl_list(pAd, ifIndex);

		if (ret < 0)
			return ret;
	}

	return 0;
}

static const struct
nla_policy SUBCMD_SET_MULTICAST_SNOOPING_POLICY[];

int mtk_cfg80211_vndr_set_mcast_snoop_handler(struct wiphy *wiphy,
																			struct wireless_dev *wdev,
																			const void *data,
																			int len)
{
#if defined(IGMP_SNOOP_SUPPORT)
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX + 1];

	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;

	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_MULTICAST_SNOOPING_POLICY, NULL);
	if (ret)
		return -EINVAL;

#if defined(RT_CFG80211_SUPPORT)
	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]) {
		UINT8 mcsnoop_enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]);
		if (mcsnoop_enable == 0xf)
			return cfg80211_get_igmp_snoop_status(pAd, wiphy);
		ret = Set_IgmpSn_Enable(pAd, mcsnoop_enable);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY]) {
		UINT8 mcsnoop_policy = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY]);

		ret = Set_IgmpSn_Allow_Non_Memb_Enable(pAd, mcsnoop_policy);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD]) {
		char group_entry[60] = {0};
		int grp_entry_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD]);
		char *p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD]);

		if (grp_entry_len >= 60)
			return -EINVAL;

		memcpy(group_entry, p, grp_entry_len);

		ret = Set_IgmpSn_AddEntry(pAd, group_entry);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL]) {
		char group_entry[60] = {0};
		int grp_entry_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL]);
		char *p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL]);

		if (grp_entry_len >= 60)
			return -EINVAL;

		memcpy(group_entry, p, grp_entry_len);

		ret = Set_IgmpSn_DelEntry(pAd, group_entry);
		if (ret < 0)
			return ret;
	}
#endif

#ifdef IGMP_SNOOPING_DENY_LIST
	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST]) {
		char deny_entry[64] = {0};
		int deny_entry_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST]);
		char *p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST]);

		if (deny_entry_len >= 64)
			return -EINVAL;

		memcpy(deny_entry, p, deny_entry_len);

		ret = Set_IgmpSn_Deny(pAd, deny_entry);
		if (ret < 0)
			return ret;
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR]) {
		char white_list[21] = {0};
		int white_list_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR]);
		char *p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR]);

		if (white_list_len >= 21)
			return -EINVAL;

		memcpy(white_list, p, white_list_len);

		ret = Set_Igmp_Flooding_CIDR_Proc(pAd, white_list);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_CFGPERBANDSIZE]) {
		char per_band_list[16] = {0};
		int per_band_list_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_CFGPERBANDSIZE]);
		char *p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_CFGPERBANDSIZE]);

		if (per_band_list_len >= 16)
			return -EINVAL;

		memcpy(per_band_list, p, per_band_list_len);

		ret = Set_IgmpSn_CfgPerBandSize_Proc(pAd, per_band_list);
		if (ret < 0)
			return ret;
	}

	return 0;
#else
	return -EINVAL;
#endif /* IGMP_SNOOP_SUPPORT */
}

#ifdef WAPP_SUPPORT
int nl80211_send_wapp_qry_rsp(
	struct wiphy *wiphy,
	struct wireless_dev *wdev,
	struct wapp_event *event)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	UINT buflen = sizeof(struct wapp_event);

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "\n");

	event->len = buflen - sizeof(event->len) - sizeof(event->event_id);

	skb = cfg80211_vendor_event_alloc(wiphy, wdev, buflen, MTK_NL80211_VENDOR_EVENT_RSP_WAPP_EVENT, GFP_ATOMIC);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_EVENT_RSP_WAPP_EVENT, buflen, event)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_ATOMIC);
	return 0;
}
#ifdef RT_CFG80211_SUPPORT
#endif
#endif /* WAPP_SUPPORT */

int mtk_cfg80211_event_bss_ml_info(
	IN PNET_DEV pNetDev,
	IN struct ml_info_event ml_event)
{
	struct wiphy *wiphy = pNetDev->ieee80211_ptr->wiphy;
	struct wireless_dev *wdev = pNetDev->ieee80211_ptr;
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	UINT buflen;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO, "\n");

	buflen = sizeof(struct ml_info_event);

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
				"buflen=%d\n", buflen);
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, buflen, MTK_NL80211_VENDOR_EVENT_SEND_ML_INFO, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_EVENT_SEND_ML_INFO, buflen, &ml_event)) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

int mtk_cfg80211_event_reconf_sm(
	IN PNET_DEV pNetDev,
	IN UINT32 reconfig_state)
{
	struct wiphy *wiphy = pNetDev->ieee80211_ptr->wiphy;
	struct wireless_dev *wdev = pNetDev->ieee80211_ptr;
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	UINT buflen;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO, "\n");

	buflen = sizeof(UINT32);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_INFO,
				"buflen=%d, reconfig_state=%d\n", buflen, reconfig_state);
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, buflen, MTK_NL80211_VENDOR_EVENT_MLO_RECONF, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_EVENT_MLO_RECONF_SM, buflen, &reconfig_state)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_RECONF_FLOW, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

#ifdef WAPP_SUPPORT
static int mtk_cfg80211_reply_wapp_max_num_of_sta(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT16 MaxStaNum = 0;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_STA\n");

	MaxStaNum = GET_MAX_UCAST_NUM(pAd);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "Driver MaxStaNum = %d\n", MaxStaNum);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_STA,
					&MaxStaNum, sizeof(UINT16));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_wapp_chan_list(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int i = 0, ret;
	CHANNEL_CTRL *pChCtrl = NULL;
	struct wifi_dev *wdev;
	wdev_chn_info *chn_list;
	UINT ifIndex;
	POS_COOKIE pObj;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHAN_LIST\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	os_alloc_mem(pAd, (UCHAR **)&chn_list, sizeof(wdev_chn_info));
	if (chn_list == NULL)
		return -EINVAL;
	NdisZeroMemory(chn_list, sizeof(wdev_chn_info));

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "wdev is not an AP\n");
		os_free_mem(chn_list);
		return -EINVAL;
	}
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "(sync) Msg received!\n");
	pAd->ChannelListNum = pChCtrl->ChListNum;
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		pAd->ChannelList[i].Channel = pChCtrl->ChList[i].Channel;
		pAd->ChannelList[i].DfsReq = pChCtrl->ChList[i].DfsReq;
	}
	chn_list->band = wdev->PhyMode;
	chn_list->op_ch = wlan_operate_get_prim_ch(wdev);
	chn_list->op_class = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
	chn_list->ch_list_num = pAd->ChannelListNum;
	chn_list->dl_mcs = wdev->HTPhyMode.field.MCS;
	setChannelList(pAd, wdev, chn_list);
#ifdef CONFIG_MAP_SUPPORT
	chn_list->non_op_chn_num = getNonOpChnNum(pAd, wdev, chn_list->op_class);
	setNonOpChnList(pAd,
					wdev,
					chn_list->non_op_ch_list,
					chn_list->op_class,
					chn_list->non_op_chn_num);

	setAutoChannelSkipList(pAd, wdev, chn_list);
#endif /* CONFIG_MAP_SUPPORT */

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHAN_LIST,
						chn_list, sizeof(wdev_chn_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(chn_list);
	return ret;
}

static int mtk_cfg80211_reply_wapp_nop_chan_list(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct nop_channel_list_s *nop_channels;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_NOP_CHANNEL_LIST\n");

	os_alloc_mem(pAd, (UCHAR **)&nop_channels, sizeof(struct nop_channel_list_s));
	if (nop_channels == NULL)
		return -EINVAL;
	NdisZeroMemory(nop_channels, sizeof(struct nop_channel_list_s));

	wapp_prepare_nop_channel_list(pAd, nop_channels);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_NOP_CHANNEL_LIST,
					nop_channels, sizeof(struct nop_channel_list_s));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(nop_channels);
	return ret;
}

static int mtk_cfg80211_reply_wapp_op_class(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
#ifndef MAP_6E_SUPPORT
	wdev_op_class_info *op_class;
#else
	struct _wdev_op_class_info_ext *op_class;
#endif

	struct wifi_dev *wdev = NULL;
	UINT ifIndex;
	POS_COOKIE pObj;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

#ifndef MAP_6E_SUPPORT
	os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info));
#else
	os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(struct _wdev_op_class_info_ext));
#endif

	if (op_class == NULL)
		return -EINVAL;

#ifndef MAP_6E_SUPPORT
	NdisZeroMemory(op_class, sizeof(wdev_op_class_info));
#else
	NdisZeroMemory(op_class, sizeof(struct _wdev_op_class_info_ext));
#endif
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "wdev is not an AP\n");
		os_free_mem(op_class);
		return -EINVAL;
	}
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_6E_SUPPORT
	if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
		op_class->num_of_op_class = map_set_op_class_info_6g(pAd, wdev, op_class);
	else
#endif
		op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);

#endif /* CONFIG_MAP_SUPPORT */
#ifndef MAP_6E_SUPPORT
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS,
					op_class, sizeof(wdev_op_class_info));
#else
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS,
					op_class, sizeof(struct _wdev_op_class_info_ext));
#endif

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(op_class);
	return ret;
}

static int mtk_cfg80211_reply_wapp_bss_info(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex;
	POS_COOKIE pObj;
	wdev_bss_info *bss_info;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSS_INFO\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	os_alloc_mem(pAd, (UCHAR **)&bss_info, sizeof(wdev_bss_info));
	if (bss_info == NULL)
		return -EINVAL;
	NdisZeroMemory(bss_info, sizeof(wdev_bss_info));

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR, "wdev is not an AP\n");
		os_free_mem(bss_info);
		return -EINVAL;
	}
	bss_info->SsidLen = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
	NdisMoveMemory(bss_info->ssid,
			pAd->ApCfg.MBSSID[ifIndex].Ssid,
			(MAX_LEN_OF_SSID+1));
	NdisMoveMemory(bss_info->bssid, wdev->bssid, MAC_ADDR_LEN);
	NdisMoveMemory(bss_info->if_addr, wdev->if_addr, MAC_ADDR_LEN);
#ifdef CONFIG_MAP_SUPPORT
	bss_info->map_role = wdev->MAPCfg.DevOwnRole;
#endif
	bss_info->auth_mode = pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap;
	bss_info->enc_type = pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.PairwiseCipher;
#ifdef WSC_AP_SUPPORT
	bss_info->key_len = strlen(pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WpaPsk);
	NdisMoveMemory(bss_info->key,
		pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WpaPsk, bss_info->key_len);
#else
	bss_info->key_len = strlen(pAd->ApCfg.MBSSID[ifIndex].PSK);
	NdisMoveMemory(bss_info->key, pAd->ApCfg.MBSSID[ifIndex].PSK, bss_info->key_len);
#endif
	bss_info->hidden_ssid = pAd->ApCfg.MBSSID[ifIndex].bHideSsid;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSS_INFO,
					bss_info, sizeof(wdev_bss_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(bss_info);
	return ret;
}
#endif /* WAPP_SUPPORT */

static int mtk_cfg80211_reply_acs_refresh_period(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_ACS_REFRESH_PERIOD,
					&pAd->ApCfg.AcsCfg.ACSCheckTime, sizeof(UINT32));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_channel_last_change(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	UINT32 Time, ChannelLastChange;
	ULONG TNow;
	struct hdev_ctrl *ctrl = NULL;

	MAC80211_PAD_GET(pAd, wiphy);

	ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	if (!ctrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "hdev_ctrl is invalid.\n");
		return -EINVAL;
	}

	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
	ChannelLastChange = Time - ctrl->rdev.pRadioCtrl->CurChannelUpTime;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANNEL_LAST_CHANGE,
					&ChannelLastChange, sizeof(UINT32));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_max_num_of_ssids(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	UCHAR MaxSupportedSSIDs;

	MAC80211_PAD_GET(pAd, wiphy);

	MaxSupportedSSIDs = MAX_MBSSID_NUM(pAd);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_SSIDS,
					&MaxSupportedSSIDs, sizeof(UCHAR));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_extension_channel(struct wiphy *wiphy,
	 struct wireless_dev *wdev, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret, apidx;
	struct net_device *pNetDev = wdev->netdev;
	UCHAR extensionChannel;

	MAC80211_PAD_GET(pAd, wiphy);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"apidx is invalid.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"apidx is %d.\n", apidx);

	extensionChannel = wlan_config_get_ext_cha(&pAd->ApCfg.MBSSID[apidx].wdev);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_EXTENSION_CHANNEL,
					&extensionChannel, sizeof(UCHAR));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_transmit_power(struct wiphy *wiphy,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_TRANSMITPOWER,
					&pAd->CommonCfg.ucTxPowerPercentage, sizeof(UCHAR));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_80211h(struct wiphy *wiphy,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_80211H,
					&pAd->CommonCfg.bIEEE80211H, sizeof(UCHAR));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_basic_rate(struct wiphy *wiphy,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BASIC_RATE,
					&pAd->CommonCfg.BasicRateBitmap, sizeof(ULONG));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}

static int mtk_cfg80211_reply_supp_rate(struct wiphy *wiphy,
	struct wifi_dev *wdev, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SUPP_RATE,
					&wdev->rate.legacy_rate.sup_rate, wdev->rate.legacy_rate.sup_rate_len);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}

#ifdef WAPP_SUPPORT
#ifdef WSC_INCLUDED
static int mtk_cfg80211_reply_wapp_wsc_profiles(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT ifIndex = 0;
	POS_COOKIE pObj;
	int TotalLen = 0;
	wsc_apcli_config_msg *wsc_config = NULL;
	int ret = 0;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WAPP_WSC_PROFILES\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	TotalLen = Fill_OID_WSC_PROFILE(pAd, ifIndex, &wsc_config);
	if (wsc_config == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"wsc_config is NULL return FALSE\n");
		return -EINVAL;
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WAPP_WSC_PROFILES,
					wsc_config, (unsigned int)TotalLen);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(wsc_config);
	return ret;
}
#endif /* WSC_INCLUDED */
#endif /* WAPP_SUPPORT */
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
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
static int  mtk_cfg80211_reply_mlo_mld_info(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct _wdev_mlo_mld_info *mlo_info;

	struct wifi_dev *wdev = NULL;
	UINT ifIndex;
	POS_COOKIE pObj;
	int ret;
	struct bmgr_entry *entry = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_WARN,
		"get MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	os_alloc_mem(pAd, (UCHAR **)&mlo_info, sizeof(struct _wdev_mlo_mld_info));

	if (mlo_info == NULL)
		return -EINVAL;

	NdisZeroMemory(mlo_info, sizeof(struct _wdev_mlo_mld_info));

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR, "wdev is not an AP\n");
		ret = -1;
		goto out;
	}

	entry = get_bss_entry_by_netdev(wdev->if_dev);
	if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		if (!BMGR_VALID_MLO_DEV(entry->mld_ptr)) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"Entry not affiliated to MLO GRP\n");
			ret = -1;
			goto out;
		}
			mlo_info->mld_idx = entry->mld_ptr->mld_grp;
			NdisMoveMemory(mlo_info->mld_addr, entry->mld_ptr->mld_addr, MAC_ADDR_LEN);
#ifdef MAP_R6
			mlo_info->link_id = entry->link_id;
			mlo_info->ap_str_support = 1;
			mlo_info->ap_nstr_support = 0;
			mlo_info->ap_emlsr_support = entry->mld_ptr->attr.eml_caps.emlsr_supp;
			mlo_info->ap_emlmr_support = entry->mld_ptr->attr.eml_caps.emlmr_supp;
#endif
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MLO_MLD_INFO,
					 mlo_info, sizeof(struct _wdev_mlo_mld_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
out:
	os_free_mem(mlo_info);
	return ret;
}
#endif

static int mtk_cfg80211_reply_wmode_info(struct wiphy *wiphy,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	struct wifi_dev *wdev;

	MAC80211_PAD_GET(pAd, wiphy);
	wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

	if (!wdev)
		return -EINVAL;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE,
					&wdev->PhyMode, sizeof(USHORT));

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}
#ifdef WAPP_SUPPORT
static int mtk_cfg80211_reply_wapp_tx_pwr(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex;
	POS_COOKIE pObj;
	wdev_tx_power tx_pwr;
#ifndef MAP_6E_SUPPORT
	wdev_op_class_info *op_class;
#else
	struct _wdev_op_class_info_ext *op_class;
#endif

	UINT8 op = 0, ch = 0, pwr_num = 0;
	UINT8 channel_set_num = 0, MaxTxPwr = 0, ChSetMinLimPwr;
	PUCHAR channel_set = NULL;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_TX_PWR\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
#ifndef MAP_6E_SUPPORT
	os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info));
#else
	os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(struct _wdev_op_class_info_ext));
#endif

	if (op_class == NULL)
		return -EINVAL;
#ifndef MAP_6E_SUPPORT
	NdisZeroMemory(op_class, sizeof(wdev_op_class_info));
#else
	NdisZeroMemory(op_class, sizeof(struct _wdev_op_class_info_ext));
#endif

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "wdev is not an AP\n");
		os_free_mem(op_class);
		return -EINVAL;
	}
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_6E_SUPPORT
	if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
		op_class->num_of_op_class = map_set_op_class_info_6g(pAd, wdev, op_class);
	else
#endif
		op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);

#endif
	tx_pwr.num_of_op_class = op_class->num_of_op_class;
	for (op = 0; op < op_class->num_of_op_class; op++) {
#ifndef MAP_6E_SUPPORT
		if (op_class->opClassInfo[op].op_class == 0)
#else
		if (op_class->opClassInfoExt[op].op_class == 0)
#endif
			continue;
#ifndef MAP_6E_SUPPORT
		tx_pwr.tx_pwr_limit[pwr_num].op_class = op_class->opClassInfo[op].op_class;
		channel_set = get_channelset_by_reg_class(pAd, op_class->opClassInfo[op].op_class, wdev->PhyMode);
#else
		tx_pwr.tx_pwr_limit[pwr_num].op_class = op_class->opClassInfoExt[op].op_class;
		channel_set = get_channelset_by_reg_class(pAd, op_class->opClassInfoExt[op].op_class, wdev->PhyMode);
#endif
		channel_set_num = get_channel_set_num(channel_set);
		/* no match channel set. */
		if (channel_set == NULL)
			continue;
		/* empty channel set. */
		if (channel_set_num == 0)
			continue;
		/*
			There is many channel which have different limit tx power
			we choose the minimum
		*/
		ChSetMinLimPwr = 0xFF;
		for (ch = 0; ch < channel_set_num; ch++) {
			MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, channel_set[ch], wdev);
			if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
				MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
			if (MaxTxPwr < ChSetMinLimPwr)
				ChSetMinLimPwr = MaxTxPwr;
			if (ChSetMinLimPwr == 0xff)
				ChSetMinLimPwr = pAd->MaxTxPwr;
		}
		tx_pwr.tx_pwr_limit[pwr_num].max_pwr = ChSetMinLimPwr;
		pwr_num++;
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_TX_PWR,
					&tx_pwr, sizeof(wdev_tx_power));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	if (op_class)
		os_free_mem(op_class);
	return ret;
}

static int mtk_cfg80211_reply_wapp_ap_metrics(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex;
	POS_COOKIE pObj;
	wdev_ap_metric *ap_metric;
	BSS_STRUCT *mbss;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_AP_METRICS\n");

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	os_alloc_mem(pAd, (UCHAR **)&ap_metric, sizeof(wdev_ap_metric));
	if (ap_metric == NULL)
		return -EINVAL;
	NdisZeroMemory(ap_metric, sizeof(wdev_ap_metric));

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "wdev is not an AP\n");
		os_free_mem(ap_metric);
		return -EINVAL;
	}
	mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	NdisMoveMemory(ap_metric->bssid, wdev->bssid, MAC_ADDR_LEN);
	ap_metric->cu = get_channel_utilization(pAd, ifIndex);
	NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BE], mbss->ESPI_AC_BE, sizeof(mbss->ESPI_AC_BE));
	NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BK], mbss->ESPI_AC_BK, sizeof(mbss->ESPI_AC_BK));
	NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VO], mbss->ESPI_AC_VO, sizeof(mbss->ESPI_AC_VO));
	NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VI], mbss->ESPI_AC_VI, sizeof(mbss->ESPI_AC_VI));

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_AP_METRICS,
					ap_metric, sizeof(wdev_ap_metric));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	os_free_mem(ap_metric);
	return ret;
}

static int mtk_cfg80211_reply_wapp_support_ver(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	RTMP_STRING wapp_support_ver[16];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WAPP_SUPPORT_VER\n");

	ret = snprintf(&wapp_support_ver[0], sizeof(wapp_support_ver), "%s", WAPP_SUPPORT_VERSION);
	if (os_snprintf_error(sizeof(wapp_support_ver), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"wapp_support_ver snprintf error!\n");
		return -EINVAL;
	}
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WAPP_SUPPORT_VER,
				wapp_support_ver, strlen(WAPP_SUPPORT_VERSION) + 1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif /* WAPP_SUPPORT */
static int mtk_cfg80211_reply_wifi_ver(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	RTMP_STRING driver_ver[16];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_VER\n");

	ret = snprintf(&driver_ver[0], sizeof(driver_ver), "%s", AP_DRIVER_VERSION);
	if (os_snprintf_error(sizeof(driver_ver), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"driver_ver snprintf error!\n");
		return -EINVAL;
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_VER,
				driver_ver, strlen(AP_DRIVER_VERSION) + 1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_band_info_band(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	char *band_str[BAND_SELECT_BAND_MAX] = {"N/A", "2G", "5G", "6G", "5G-Low", "5G-High", "6G-Low", "6G-High"};

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND %d\n", pAd->CommonCfg.BandSelBand);

	if (pAd->CommonCfg.BandSelBand <= BAND_SELECT_BAND_NONE ||
		pAd->CommonCfg.BandSelBand >= BAND_SELECT_BAND_MAX)
		return -EINVAL;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND,
					band_str[pAd->CommonCfg.BandSelBand],
					strlen(band_str[pAd->CommonCfg.BandSelBand])+1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

#define FREQ_STR_LEN 8
static int mtk_cfg80211_reply_band_info_freqlist(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT Status = 0, ret = 0, i = 0;
	CHANNEL_CTRL *pChCtrl;
	RTMP_STRING *msg;
	UINT32 TotalLen, LeftBufSize;

	MAC80211_PAD_GET(pAd, wiphy);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	TotalLen = FREQ_STR_LEN * (pChCtrl->FreqListNum + 1);

	os_alloc_mem(NULL, (UCHAR **)&msg, TotalLen);
	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Alloc memory fail\n");
		return -ENOMEM;
	}

	os_zero_mem((PVOID) msg, TotalLen);

	for (i = 0; (i < pChCtrl->FreqListNum) && (pChCtrl->FreqListNum <= MAX_NUM_OF_CHANNELS); i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"Freq = %d, channel = %d\n", pChCtrl->FreqList[i],
				FreqToChannel(pChCtrl->FreqList[i]));

		LeftBufSize = TotalLen - strlen(msg);
		if (i == 0)
			Status = snprintf(msg + strlen(msg), LeftBufSize, "%d", FreqToChannel(pChCtrl->FreqList[i]));
		else
			Status = snprintf(msg + strlen(msg), LeftBufSize, ", %d", FreqToChannel(pChCtrl->FreqList[i]));

		if (os_snprintf_error(LeftBufSize, Status)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Snprintf failed!\n");
			goto ERROR;
		}
	}

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
			MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST,
			msg,
			strlen(msg)+1);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
ERROR:
	if (msg)
		os_free_mem(msg);

	return ret;
}

static int mtk_cfg80211_reply_band_info_bandwidth(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_phy *mac_phy = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	mac_dev = hc_get_mac_dev(pAd);
	mac_phy = &mac_dev->mac_phy;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BANDWIDTH,
					&mac_phy->bw, sizeof(u8));

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}


static int mtk_cfg80211_reply_band_info_channel(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	struct mtk_mac_dev *mac_dev = NULL;
	struct mtk_mac_phy *mac_phy = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	mac_dev = hc_get_mac_dev(pAd);
	mac_phy = &mac_dev->mac_phy;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_CHANNEL,
					&mac_phy->chan, sizeof(u8));

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}
#ifdef MGMT_TXPWR_CTRL
static int mtk_cfg80211_reply_mgmt_real_pwr(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;
	UINT ifIndex;
	POS_COOKIE pObj;
	unsigned char mgmt_real_pwr = 0;
	struct wifi_dev *wdev;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	mgmt_real_pwr = pAd->ApCfg.MgmtTxPwr + pAd->ApCfg.EpaFeGain + (wdev->TxPwrDelta);
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_TXPWR_GET_MGMT,
					&mgmt_real_pwr, sizeof(u8));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}
#endif
#ifdef CONFIG_6G_AFC_SUPPORT
static int mtk_cfg80211_reply_afc_data(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret = 0;

	MAC80211_PAD_GET(pAd, wiphy);
	ret = nl80211_afc_daemon_channel_info(pAd, wiphy);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_AFC, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif
#ifdef WAPP_SUPPORT
static int mtk_cfg80211_reply_wapp_chip_id(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID,
					&pAd->ChipID, sizeof(pAd->ChipID));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#if (defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT))
static int mtk_cfg80211_reply_coexistence(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_COEXISTENCE,
					&pAd->CommonCfg.bBssCoexEnable,
					sizeof(pAd->CommonCfg.bBssCoexEnable));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif /* defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT) */
static int mtk_cfg80211_reply_wdev(struct wiphy *wiphy, struct wireless_dev *wl_dev, const void *data, int len)
{
	INT i;
	struct wifi_dev *wdev;
	struct wifi_dev *wdev_in;
	wapp_dev_info dev_info;
	PRTMP_ADAPTER pAd;
	int payload_len;
	int ret = 0;

	MAC80211_PAD_GET(pAd, wiphy);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			wdev_in = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);

			if (!wdev_in) {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
						"failed - [ERROR]can't find wdev in driver .\n");
				return -EINVAL;
			}

			if (wdev->if_dev && wdev_in->if_dev &&
					RtmpOsGetNetIfIndex(wdev->if_dev) == RtmpOsGetNetIfIndex(wdev_in->if_dev)) {
				dev_info.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
				dev_info.dev_type = wdev->wdev_type;
				COPY_MAC_ADDR(dev_info.mac_addr, wdev->if_addr);
				NdisCopyMemory(dev_info.ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);
				dev_info.radio_id = HcGetBandByWdev(wdev);
				dev_info.adpt_id = (uintptr_t) pAd;
				dev_info.wireless_mode = wdev->PhyMode;
				dev_info.dev_active = HcIsRadioAcq(wdev);
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
					"incoming index %d - %d driver index name %s\n",
					RtmpOsGetNetIfIndex(wdev_in->if_dev),
					(RtmpOsGetNetIfIndex(wdev->if_dev)),
					RtmpOsGetNetDevName(wdev->if_dev));
				payload_len = sizeof(wapp_dev_info);
				ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
						MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WDEV,
						&dev_info, payload_len);
				if (IS_MAP_TURNKEY_ENABLE(pAd) &&
						wdev->wdev_type == WDEV_TYPE_STA) {
					if (wdev->func_idx >= MAX_APCLI_NUM)
						continue;
					SetApCliEnableByWdev(pAd, wdev, FALSE);
				}
			}
		}
	}
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
				"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_cac_cap(struct wiphy *wiphy, struct wireless_dev *wl_dev, const void *data, int len)
{
#ifdef DFS_CAC_R2
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *pDot11h = NULL;
	PDFS_PARAM pDfsParam = NULL;
	int payload_len;
	UCHAR bandIdx;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
			"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_CAC_CAP\n");

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	pDfsParam = &pAd->CommonCfg.DfsParameter;
	NdisZeroMemory(&wdev->cac_capability, sizeof(wdev->cac_capability));
	wapp_get_cac_cap(pAd, wdev, &wdev->cac_capability);
	NdisMoveMemory(&wdev->cac_capability.country_code[0],
		pAd->CommonCfg.CountryCode, 2);
	wdev->cac_capability.rdd_region = pAd->CommonCfg.RDDurRegion;

	/* CAC Ongoing Update */
	bandIdx = HcGetBandByWdev(wdev);
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return -EINVAL;

	wdev->cac_capability.active_cac = FALSE;
	if (pDot11h->ChannelMode == CHAN_SILENCE_MODE) {
		wdev->cac_capability.active_cac = TRUE;
		if (pDfsParam->band_bw == BW_80 ||
		pDfsParam->band_bw == BW_160)
			wdev->cac_capability.ch_num =
				DfsPrimToCent(pDfsParam->PrimCh, pDfsParam->band_bw);
		else
			wdev->cac_capability.ch_num = pDfsParam->PrimCh;
		if (pDot11h->cac_time > pDot11h->RDCount)
			wdev->cac_capability.remain_time =
				(pDot11h->cac_time - pDot11h->RDCount);
		else
			wdev->cac_capability.remain_time = 0;
	}
	wdev->cac_capability.cac_mode = get_cac_mode(pAd, pDfsParam, wdev);

	payload_len = sizeof(wdev->cac_capability);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_CAC_CAP,
							&wdev->cac_capability, payload_len);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
#else /* DFS_CAC_R2 */
	return -EOPNOTSUPP;
#endif /* DFS_CAC_R2 */
}

static int mtk_cfg80211_reply_misc_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	wdev_misc_cap misc_cap;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	NdisZeroMemory(&misc_cap, sizeof(wdev_misc_cap));
	misc_cap.max_num_of_cli = 64;
	misc_cap.max_num_of_bss = 32;
	misc_cap.num_of_bss = pAd->ApCfg.BssidNum;
	misc_cap.max_num_of_block_cli = BLOCK_LIST_NUM;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MISC_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MISC_CAP,
					&misc_cap, sizeof(wdev_misc_cap));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_ht_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	wdev_ht_cap ht_cap;
	UINT ifIndex;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	NdisZeroMemory(&ht_cap, sizeof(wdev_ht_cap));
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	ht_cap.tx_stream = wlan_config_get_tx_stream(wdev);
	ht_cap.rx_stream = wlan_config_get_rx_stream(wdev);
	ht_cap.sgi_20 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
	ht_cap.sgi_40 = (wlan_config_get_ht_gi(wdev) == GI_400) ?  1:0;
	ht_cap.ht_40 = (wlan_operate_get_ht_bw(wdev) == BW_40) ? 1:0;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HT_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HT_CAP,
					&ht_cap, sizeof(wdev_ht_cap));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

static int mtk_cfg80211_reply_vht_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT ifIndex, sup_tx_mcs_size = 0, sup_rx_mcs_size = 0;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	wdev_vht_cap vht_cap;
	VHT_CAP_INFO drv_vht_cap;
	VHT_OP_IE drv_vht_op;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	NdisZeroMemory(&vht_cap, sizeof(wdev_vht_cap));
	NdisZeroMemory(&drv_vht_cap, sizeof(VHT_CAP_INFO));
	NdisZeroMemory(&drv_vht_op, sizeof(VHT_OP_IE));
	NdisCopyMemory(&drv_vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
	if (wdev->wdev_type != WDEV_TYPE_AP) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "wdev is not an AP\n");
		return -EINVAL;
	}
	mt_WrapSetVHTETxBFCap(pAd, wdev, &drv_vht_cap);
	drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
	drv_vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;
	switch	(wlan_operate_get_rx_stream(wdev)) {
	case 4:
		drv_vht_op.basic_mcs_set.mcs_ss4 = MCS_NSS_CAP(pAd)->max_vht_mcs;
		fallthrough;
	case 3:
		drv_vht_op.basic_mcs_set.mcs_ss3 = MCS_NSS_CAP(pAd)->max_vht_mcs;
		fallthrough;
	case 2:
		drv_vht_op.basic_mcs_set.mcs_ss2 = MCS_NSS_CAP(pAd)->max_vht_mcs;
		fallthrough;
	case 1:
		drv_vht_op.basic_mcs_set.mcs_ss1 = MCS_NSS_CAP(pAd)->max_vht_mcs;
		break;
	}
	sup_tx_mcs_size = sizeof(vht_cap.sup_tx_mcs) < sizeof(drv_vht_op.basic_mcs_set)
						? sizeof(vht_cap.sup_tx_mcs) : sizeof(drv_vht_op.basic_mcs_set);
	sup_rx_mcs_size = sizeof(vht_cap.sup_rx_mcs) < sizeof(drv_vht_op.basic_mcs_set)
						? sizeof(vht_cap.sup_tx_mcs) : sizeof(drv_vht_op.basic_mcs_set);

	NdisMoveMemory(vht_cap.sup_tx_mcs,
					&drv_vht_op.basic_mcs_set,
					sup_tx_mcs_size);
	NdisMoveMemory(vht_cap.sup_rx_mcs,
					&drv_vht_op.basic_mcs_set,
					sup_rx_mcs_size);
	vht_cap.tx_stream = wlan_config_get_tx_stream(wdev);
	vht_cap.rx_stream = wlan_config_get_tx_stream(wdev);
	vht_cap.sgi_80 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
	vht_cap.sgi_160 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
	vht_cap.vht_160 = (wlan_operate_get_vht_bw(wdev) == BW_160) ? 1:0;
	vht_cap.vht_8080 = (wlan_operate_get_vht_bw(wdev) == BW_8080) ? 1:0;
	vht_cap.su_bf = (drv_vht_cap.bfer_cap_su) ? 1:0;
	vht_cap.mu_bf = (drv_vht_cap.bfer_cap_mu) ? 1:0;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_VHT_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_VHT_CAP,
				&vht_cap, sizeof(wdev_vht_cap));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

#ifdef CONFIG_MAP_SUPPORT
static int mtk_cfg80211_reply_eht_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT ifIndex;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	struct wdev_eht_cap eht_cap;
	struct _RTMP_CHIP_CAP *cap;
	int ret;
	UINT8 eht_bw, prim_ch;
	UCHAR ch_band, eht_cen_ch;

#ifdef MAP_R6
	UINT8 eht_tx_nss, eht_rx_nss, eht_max_mcs;
	struct eht_txrx_mcs_nss eht_mcs_nss = {0};
	struct eht_txrx_mcs_nss_20 eht_mcs_nss_bw20 = {0};
#endif

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	NdisZeroMemory(&eht_cap, sizeof(struct wdev_eht_cap));

	prim_ch = wdev->channel;
	eht_bw = wlan_config_get_eht_bw(wdev);
	eht_cap.eht_ch_width = eht_bw;
	ch_band = wlan_config_get_ch_band(wdev);
	eht_cen_ch = eht_cent_ch_freq(wdev, wdev->channel, eht_bw, ch_band);
#ifdef MAP_R6
	eht_cap.interface_index = wdev->wdev_idx;
	eht_cap.eht_operation_information_valid = 1;
	eht_cap.eht_default_pe_duration = 1;
	eht_cap.group_addressed_BU_indication_limit = 1;
	eht_cap.group_addressed_BU_indication_exponent = 1;
	eht_tx_nss = wlan_config_get_eht_tx_nss(wdev);
	eht_rx_nss = wlan_config_get_eht_rx_nss(wdev);
	eht_max_mcs = wlan_config_get_eht_max_mcs(wdev);
	eht_cap.control = eht_bw;

	if (eht_max_mcs == MCS_0)
		eht_max_mcs = MCS_13;

	if (eht_bw == EHT_BW_20) {
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			eht_mcs_map_bw20(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss_bw20);
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss_bw20.max_tx_rx_mcs0_7_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss_bw20.max_tx_rx_mcs8_9_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss_bw20.max_tx_rx_mcs10_11_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss_bw20.max_tx_rx_mcs12_13_nss;
		} else {
			eht_mcs_map(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss);
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss.max_tx_rx_mcs10_11_nss;
			eht_cap.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss.max_tx_rx_mcs12_13_nss;
		}
	} else {
		eht_mcs_map(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss);
		eht_cap.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
		eht_cap.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
		eht_cap.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss.max_tx_rx_mcs10_11_nss;
		eht_cap.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss.max_tx_rx_mcs12_13_nss;
	}
	eht_cap.dscb_bitmap = 0;
#endif
	switch (eht_bw) {
	case EHT_BW_320:
		eht_cap.ccfs0 = GET_BW320_PRIM160_CENT(prim_ch, eht_cen_ch);
		eht_cap.ccfs1 = eht_cen_ch;
		break;
	case EHT_BW_160:
		eht_cap.ccfs0 = GET_BW160_PRIM80_CENT(prim_ch, eht_cen_ch);
		eht_cap.ccfs1 = eht_cen_ch;
		break;
	case EHT_BW_80:
		eht_cap.ccfs0 = eht_cen_ch;
		eht_cap.ccfs1 = 0;
		break;
	case EHT_BW_2040:
		eht_cap.ccfs0 = eht_cen_ch;
		eht_cap.ccfs1 = 0;
		break;
	default:
		eht_cap.ccfs0 = eht_cen_ch;
		eht_cap.ccfs1 = 0;
	}
	eht_cap.tx_stream = wlan_config_get_tx_stream(wdev);
	eht_cap.rx_stream = wlan_config_get_tx_stream(wdev);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_EHT_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_EHT_CAP,
				&eht_cap, sizeof(struct wdev_eht_cap));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif
#endif /* WAPP_SUPPORT */
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
static int mtk_cfg80211_reply_mlo_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT ifIndex;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	struct wdev_mlo_caps mlo_cap;
	struct _RTMP_CHIP_CAP *cap;
	int ret, i;
	struct bmgr_entry *entry = NULL;
	u8 mld_grp, mld_type;
	struct bmgr_reg_info *reg_info = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	if (ifIndex == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATAP_BCN, DBG_LVL_ERROR,
			"failed - [ERROR]can't ifIndex in driver MBSS.\n");
		return -EINVAL;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Driver: NL80211_VENDOR_ATTR_GET_CAP_MLO_CAPABILITY is called from WAPP in driver\n");
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	NdisZeroMemory(&mlo_cap, sizeof(struct wdev_mlo_caps));
	if (IF_COMBO_HAVE_AP_STA(pAd))
		mlo_cap.RoleNum = 2;
	else
		mlo_cap.RoleNum = 1;
#define MLO_AP_INFO 0
#define MLO_APCLI_INFO 1

	for (i = 0; i < mlo_cap.RoleNum; i++) {
		if (i == 0) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (!wdev) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						"wdev is NULL\n");
				continue;
			}
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						"wdev is not an AP\n");
				continue;
			}
			mlo_cap.MloCapPerRole[i].DeviceRole = MLO_AP_INFO;
		}
#ifdef APCLI_SUPPORT
		else if (i == 1) {
			UCHAR b_idx = 0;

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			b_idx = HcGetBandByWdev(wdev);
			if (pAd->ApCfg.ApCliInfRunned) {
				if (MAX_MULTI_STA >= 1)
					wdev = &pAd->StaCfg[0].wdev;
			}
			if (!wdev) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						"wdev is NULL\n");
				continue;
			}
			mlo_cap.MloCapPerRole[i].DeviceRole = MLO_APCLI_INFO;
			mlo_cap.MloCapPerRole[i].EmlCap.emlsr_supp = wlan_config_get_emlsr_mr(wdev);

			if (pAd->map_apcli_mlo_disable == 0)
				mlo_cap.MloCapPerRole[i].MloEnable = 1;

		}
#endif
		entry = get_bss_entry_by_netdev(wdev->if_dev);
		if (BMGR_VALID_BSS_ENTRY(entry)) {
			if (!BMGR_VALID_MLO_DEV(entry->mld_ptr)) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"Entry not affiliated to MLO GRP\n");
				ret = -1;
				return ret;
			}

			reg_info = &entry->entry_info;
			if (reg_info)
				mld_grp = reg_info->mld_grp;
			mld_type = BMGR_IS_ML_MLD_GRP_IDX(mld_grp) ? BMGR_MLD_TYPE_MULTI : BMGR_MLD_TYPE_SINGLE;

			mlo_cap.MloCapPerRole[i].StatPunctureSupport = 1;
#ifndef MAP_R6
			if ((i == 0) && mld_type == BMGR_MLD_TYPE_MULTI)
				mlo_cap.MloCapPerRole[i].MloEnable = 1;
#else
			if ((i == 0) && ((mld_type == BMGR_MLD_TYPE_MULTI
					|| mld_type == BMGR_MLD_TYPE_SINGLE)
					&& mld_grp < BMGR_INVALID_MLD_GRP
					&& mld_grp != 0)) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"mld_type = %d mld_grp %d\n", mld_type, mld_grp);
				mlo_cap.MloCapPerRole[i].MloEnable = 1;
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"non-mld type mld_type = %d, mld_grp %d or sta %d\n", mld_type, mld_grp, i);
			}
#endif
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"MLO CAP :device role %d StatPunc Support %d, MloEnable %d\n",
				mlo_cap.MloCapPerRole[i].DeviceRole,
				mlo_cap.MloCapPerRole[i].StatPunctureSupport,
				mlo_cap.MloCapPerRole[i].MloEnable);
			if (i != 1)
				mlo_cap.MloCapPerRole[i].EmlCap.emlsr_supp = entry->mld_ptr->attr.eml_caps.emlsr_supp;

			mlo_cap.MloCapPerRole[i].EmlCap.emlsr_padding = entry->mld_ptr->attr.eml_caps.eml_padding_delay;
			mlo_cap.MloCapPerRole[i].EmlCap.emlsr_trans_delay = entry->mld_ptr->attr.eml_caps.eml_trans_delay;
			mlo_cap.MloCapPerRole[i].EmlCap.emlmr_supp = entry->mld_ptr->attr.eml_caps.emlmr_supp;
			mlo_cap.MloCapPerRole[i].EmlCap.emlmr_delay = 0;
			mlo_cap.MloCapPerRole[i].EmlCap.trans_to = entry->mld_ptr->attr.eml_caps.trans_to;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"MLO EML CAP :EMLSR Support %d padding %d Delay %d\n",
				mlo_cap.MloCapPerRole[i].EmlCap.emlsr_supp,
				mlo_cap.MloCapPerRole[i].EmlCap.emlsr_padding,
				mlo_cap.MloCapPerRole[i].EmlCap.emlsr_trans_delay);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"MLO EML CAP : EMLMR Support %d Delay %d trans timeout %d\n",
				mlo_cap.MloCapPerRole[i].EmlCap.emlmr_supp,
				mlo_cap.MloCapPerRole[i].EmlCap.emlmr_delay,
				mlo_cap.MloCapPerRole[i].EmlCap.trans_to);
			mlo_cap.MloCapPerRole[i].MldCap.max_simul_link = (entry->mld_ptr->mld_link_cnt - 1);
			mlo_cap.MloCapPerRole[i].MldCap.srs_supp = entry->mld_ptr->attr.mld_caps.srs_supp;
			mlo_cap.MloCapPerRole[i].MldCap.t2l_nego_supp = entry->mld_ptr->attr.mld_caps.t2l_nego_supp;
			mlo_cap.MloCapPerRole[i].MldCap.freq_sep_str = entry->mld_ptr->attr.mld_caps.freq_sep_str;
			mlo_cap.MloCapPerRole[i].MldCap.aar_supp = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"MLO MLD CAP : max_simul_link %d srs supp %d t2lnego %d\n",
				mlo_cap.MloCapPerRole[i].MldCap.max_simul_link,
				mlo_cap.MloCapPerRole[i].MldCap.srs_supp,
				mlo_cap.MloCapPerRole[i].MldCap.t2l_nego_supp);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"MLO MLD CAP : Freq Sep Support %d AAR Supp %d\n",
				mlo_cap.MloCapPerRole[i].MldCap.freq_sep_str,
				mlo_cap.MloCapPerRole[i].MldCap.aar_supp);
		}
	}
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MLO_CAP\n");
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MLO_CAP,
				&mlo_cap, sizeof(struct wdev_mlo_caps));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif

#ifdef WAPP_SUPPORT
extern UINT16 he_mcs_map(UINT8 nss, UINT8 he_mcs);
static int mtk_cfg80211_reply_he_cap(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	UINT ifIndex;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	wdev_he_cap he_cap;
	UINT16 he_max_mcs_nss, *he_mcs_pos, he_mcs_len = 0;
	UINT8 he_bw;
	struct he_bf_info he_bf_struct;
	struct mcs_nss_caps *mcs_nss;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		if (ifIndex >= pAd->ApCfg.BssidNum)
			return -EFAULT;
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATAP_BCN, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	he_bw = wlan_config_get_he_bw(wdev);
	NdisZeroMemory(&he_cap, sizeof(wdev_he_cap));
	he_cap.tx_stream = wlan_config_get_tx_stream(wdev);
	he_cap.rx_stream = wlan_config_get_rx_stream(wdev);
	he_mcs_pos = (UINT16 *)he_cap.he_mcs;
	he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
	NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
	he_mcs_pos++;
	he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
	NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
	he_mcs_pos++;
	he_mcs_len += 4;
	if (he_bw > HE_BW_80) {
		if (he_cap.tx_stream > mcs_nss->bw160_max_nss)
			he_cap.tx_stream = mcs_nss->bw160_max_nss;
		if (he_cap.rx_stream > mcs_nss->bw160_max_nss)
			he_cap.rx_stream = mcs_nss->bw160_max_nss;
		he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
		NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
		he_mcs_pos++;
		he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
		NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
		he_mcs_pos++;
		he_mcs_len += 4;
		he_cap.he_160 = 1;
		if (he_bw > HE_BW_160) {
			he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
			NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
			he_mcs_pos++;
			he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
			NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
			he_mcs_pos++;
			he_mcs_len += 4;
			he_cap.he_8080 = 1;
		}
	}
	he_cap.he_mcs_len = he_mcs_len;

	NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
	mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);

	if (he_bf_struct.bf_cap & HE_SU_BFER)
		he_cap.su_bf_cap = 1;
	if (he_bf_struct.bf_cap & HE_MU_BFER)
		he_cap.mu_bf_cap = 1;
	if (wlan_config_get_mu_dl_mimo(wdev)) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			he_cap.dl_mu_mimo_ofdma_cap = 1;
		he_cap.dl_ofdma_cap = 1; /*To see*/
	}
	if (wlan_config_get_mu_ul_mimo(wdev)) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			he_cap.ul_mu_mimo_ofdma_cap = 1;
			he_cap.ul_mu_mimo_cap = 1;
			he_cap.ul_ofdma_cap = 1; /*To see*/
		}
	}
	he_cap.gi = wlan_config_get_he_gi(wdev);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP,
					&he_cap, sizeof(wdev_he_cap));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif /* WAPP_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef MAP_R3
static int mtk_cfg80211_reply_wf6_cap(struct wiphy *wiphy, struct wireless_dev *wl_dev, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int payload_len;
	INT ifIndex;
	struct wifi_dev *wdev = NULL;
	struct net_device *pNetDev = wl_dev->netdev;
	wdev_wf6_cap_roles wf6_cap;
	UINT16 i = 0;
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (ifIndex == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"failed - [ERROR]can't ifIndex in driver MBSS.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"WF6 Driver: NL80211_VENDOR_ATTR_GET_CAP_INFO_WF6_CAPABILITY is called from WAPP in driver\n");
	NdisZeroMemory(&wf6_cap, sizeof(wdev_wf6_cap_roles));
	if (IF_COMBO_HAVE_AP_STA(pAd))
		wf6_cap.role_supp = 2;
	else
		wf6_cap.role_supp = 1;
#define WF6_AP_INFO 0
#define WF6_APCLI_INFO 1
	/* Role supported as 1 when previously used*/

	for (i = 0; i < wf6_cap.role_supp; i++) {
		UINT16 he_max_mcs_nss = 0, *he_mcs_pos = NULL, he_mcs_len = 0;
		struct mcs_nss_caps *mcs_nss = NULL;
		UINT8 he_bw = 0;
		struct he_bf_info he_bf_struct;
#ifdef APCLI_SUPPORT
		UCHAR b_idx = 0;
#endif
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		if (!wdev) {
			wf6_cap.role_supp = 1;
			continue;
		}
		if (!wdev->pHObj) {
			wf6_cap.role_supp = 1;
			continue;
		}
#ifdef APCLI_SUPPORT
		b_idx = HcGetBandByWdev(wdev);
		if ((i == 1) && (pAd->ApCfg.ApCliInfRunned)) {
			if (MAX_MULTI_STA >= 1)
				wdev = &pAd->StaCfg[0].wdev;
		} else if (i == 1) {
			wf6_cap.role_supp = 1;
			continue;
		}
		if (!wdev) {
			wf6_cap.role_supp = 1;
			continue;
		}
		if (!wdev->pHObj) {
			wf6_cap.role_supp = 1;
			continue;
		}
#endif
		mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
		he_bw = wlan_config_get_he_bw(wdev);

		wf6_cap.wf6_role[i].tx_stream = wlan_config_get_tx_stream(wdev);
		wf6_cap.wf6_role[i].rx_stream = wlan_config_get_rx_stream(wdev);

		he_mcs_pos = (UINT16 *)wf6_cap.wf6_role[i].he_mcs;

		he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
		NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
		he_mcs_pos++;

		he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
		NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
		he_mcs_pos++;

		he_mcs_len += 4;

		if (he_bw > HE_BW_80
#ifdef MAP_R3
			|| ((IS_MAP_ENABLE(pAd)
				&& IS_MAP_CERT_ENABLE(pAd)
				&& IS_MAP_R3_ENABLE(pAd)))
#endif

		) {
			if (wf6_cap.wf6_role[i].tx_stream > mcs_nss->bw160_max_nss)
				wf6_cap.wf6_role[i].tx_stream = mcs_nss->bw160_max_nss;
			if (wf6_cap.wf6_role[i].rx_stream > mcs_nss->bw160_max_nss)
				wf6_cap.wf6_role[i].rx_stream = mcs_nss->bw160_max_nss;

			he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
			NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
			he_mcs_pos++;

			he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
			NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
			he_mcs_pos++;

			he_mcs_len += 4;
			wf6_cap.wf6_role[i].he_8080 = 1;

			if (he_bw > HE_BW_160
#ifdef MAP_R3
				|| ((IS_MAP_ENABLE(pAd)
				&& IS_MAP_CERT_ENABLE(pAd)
				&& IS_MAP_R3_ENABLE(pAd)))
#endif
			) {
				he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;

				he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;

				he_mcs_len += 4;
				wf6_cap.wf6_role[i].he_160 = 1;
			}
		}

		wf6_cap.wf6_role[i].he_mcs_len = he_mcs_len;
		he_mcs_len = 0;

		NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
		mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);

		if (he_bf_struct.bf_cap & HE_SU_BFER)
			wf6_cap.wf6_role[i].su_bf_cap = 1;
		if (he_bf_struct.bf_cap & HE_MU_BFER)
			wf6_cap.wf6_role[i].mu_bf_cap = 1;
		if (wlan_config_get_mu_dl_mimo(wdev)) {
			if (wdev->wdev_type == WDEV_TYPE_AP)
				wf6_cap.wf6_role[i].dl_mu_mimo_ofdma_cap = 1;
		}
		if (wlan_config_get_mu_ul_mimo(wdev)) {
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				wf6_cap.wf6_role[i].ul_mu_mimo_ofdma_cap = 1;
				wf6_cap.wf6_role[i].ul_mu_mimo_cap = 1;
			}
		}
		/* agent_role = 0 for AP, agent_role = 1 for APCLI */
		wf6_cap.wf6_role[i].agent_role = i;
		wf6_cap.wf6_role[i].su_beamformee_status = 0;
		wf6_cap.wf6_role[i].beamformee_sts_less80 = 0;
		wf6_cap.wf6_role[i].beamformee_sts_more80 = 0;
		wf6_cap.wf6_role[i].max_user_dl_tx_mu_mimo = 0;
		wf6_cap.wf6_role[i].max_user_ul_rx_mu_mimo = 0;
		wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = 0;
		wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = 0;
		wf6_cap.wf6_role[i].rts_status = 0;
		wf6_cap.wf6_role[i].mu_rts_status = 0; /* Not Supported */
		wf6_cap.wf6_role[i].m_bssid_status = 0;
		wf6_cap.wf6_role[i].mu_edca_status = 0;
		wf6_cap.wf6_role[i].twt_requester_status = 0;
		wf6_cap.wf6_role[i].twt_responder_status = 0;
		wf6_cap.wf6_role[i].ul_ofdma_cap = 0;
		wf6_cap.wf6_role[i].dl_ofdma_cap = 0;
#ifdef HE_TXBF_SUPPORT
		if (he_bf_struct.bf_cap & HE_SU_BFEE)
			wf6_cap.wf6_role[i].su_beamformee_status = 1;

		if (he_bf_struct.bfee_sts_le_eq_bw80)
			wf6_cap.wf6_role[i].beamformee_sts_less80 = 1;

		if (he_bf_struct.bfee_sts_gt_bw80)
			wf6_cap.wf6_role[i].beamformee_sts_more80 = 1;
#endif /*HE_TXBF_SUPPORT*/
#ifdef WIFI_TWT_SUPPORT
		if (wlan_config_get_asic_twt_caps(wdev) &&
				(TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))) {
			if (i == WF6_AP_INFO) {
				wf6_cap.wf6_role[i].twt_requester_status = 0;
				wf6_cap.wf6_role[i].twt_responder_status = 1;
			} else {
				wf6_cap.wf6_role[i].twt_requester_status = 1;
				wf6_cap.wf6_role[i].twt_responder_status = 0;
			}
		}
#endif /*WIFI_TWT_SUPPORT*/

		if (wlan_config_get_he_mu_edca(wdev))
			wf6_cap.wf6_role[i].mu_edca_status = 1;

		if (wlan_config_get_mu_dl_ofdma(wdev) &&
			wdev->wdev_type == WDEV_TYPE_AP) {
			wf6_cap.wf6_role[i].dl_ofdma_cap = 1;
			wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
			if (!wf6_cap.wf6_role[i].max_user_dl_tx_ofdma) {
			/*iwpriv ra0 set set_muru_manual_config=ul_comm_user_cnt:*/
				wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
			}
#endif
		}

		if (wlan_config_get_mu_ul_ofdma(wdev) &&
				wdev->wdev_type == WDEV_TYPE_AP) {
			wf6_cap.wf6_role[i].ul_ofdma_cap = 1;
			wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
			if (!wf6_cap.wf6_role[i].max_user_ul_rx_ofdma)
				wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
#endif
		}

#ifdef MBSS_SUPPORT
		if (MAX_MBSSID_NUM(pAd) > 1)
			wf6_cap.wf6_role[i].m_bssid_status = 1;
#endif /*MBSS_SUPPORT*/

		if (pAd->mcli_ctl.c2s_only == TRUE)
			wf6_cap.wf6_role[i].rts_status = 1;

		if (wlan_config_get_mu_dl_mimo(wdev)) {
			if (wdev->wdev_type == WDEV_TYPE_AP)
				wf6_cap.wf6_role[i].max_user_dl_tx_mu_mimo = wf6_cap.wf6_role[i].max_user_dl_tx_ofdma;
		}
		if (wlan_config_get_mu_ul_mimo(wdev)) {
			if (wdev->wdev_type == WDEV_TYPE_AP)
				wf6_cap.wf6_role[i].max_user_ul_rx_mu_mimo = wf6_cap.wf6_role[i].max_user_ul_rx_ofdma;
		}
	}

	payload_len = sizeof(wf6_cap);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
		"get MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP\n");

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WF6_CAPABILITY,
					&wf6_cap, payload_len);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}
#endif /* MAP_R3 */
#endif /* DOT11_HE_AX */

static const struct
nla_policy SUBCMD_TEST_POLICY[];

int mtk_nl80211_vendor_subcmd_test(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_TEST_MAX + 1];
	int err;
	u32 val;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"\n");

	err = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_TEST_MAX, (struct nlattr *)data, len, SUBCMD_TEST_POLICY, NULL);
	if (err)
		return err;

	if (!tb[MTK_NL80211_VENDOR_ATTR_TEST]) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nl80211 test fail!!!\n");
		return -EINVAL;
	}

	val = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_TEST]);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
				"nl80211 test success, val = %d\n", val);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
				"nl80211 event test !!!\n");
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, 20, MTK_NL80211_VENDOR_EVENT_TEST, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"cfg80211_vendor_event_alloc fail!!\n");
		return -ENOMEM;
	}

	/* nla_put() will fill up data within
	 * NL80211_ATTR_VENDOR_DATA.
	 */
	if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_EVENT_TEST, val + 1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_put_u32 fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	/* Send the event - this will call nla_nest_end() */
	cfg80211_vendor_event(skb, GFP_KERNEL);

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
				"nl80211 test response !!!\n");

	/* Send a response to the command */
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 20);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"cfg80211_vendor_cmd_alloc_reply_skb fail!!\n");
		return -ENOMEM;
	}

	/* nla_put() will fill up data within
	 * NL80211_ATTR_VENDOR_DATA.
	 */
	if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_TEST, val + 2)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_put_u32 fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	return mt_cfg80211_vendor_cmd_reply(skb);
}

int mtk_cfg80211_get_runtime_info(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	ULONG priv_flags;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];
	int ret;
	struct wifi_dev *preq_wdev = NULL;


	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"ifname: %s\n", pNetDev->name);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pObj in driver .\n");
		return -EINVAL;
	}

	preq_wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wdev);
	if (!preq_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	if (preq_wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR] Interface is down .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;

#ifdef WAPP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_STA]) {
		ret = mtk_cfg80211_reply_wapp_max_num_of_sta(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_max_num_of_sta failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHAN_LIST]) {
		ret = mtk_cfg80211_reply_wapp_chan_list(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_chan_list failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_NOP_CHANNEL_LIST]) {
		ret = mtk_cfg80211_reply_wapp_nop_chan_list(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_nop_chan_list failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS]) {
		ret = mtk_cfg80211_reply_wapp_op_class(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_op_class failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSS_INFO]) {
		ret = mtk_cfg80211_reply_wapp_bss_info(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_bss_info failed.\n");
	}
#ifdef WSC_INCLUDED
	else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WAPP_WSC_PROFILES]) {
		ret = mtk_cfg80211_reply_wapp_wsc_profiles(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_wsc_profiles failed.\n");
	}
#endif /* WSC_INCLUDED */
#endif /* WAPP_SUPPORT */

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANNEL_LAST_CHANGE]) {
		ret = mtk_cfg80211_reply_channel_last_change(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_channel_last_change failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_SSIDS]) {
		ret = mtk_cfg80211_reply_max_num_of_ssids(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_max_num_of_ssids failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_EXTENSION_CHANNEL]) {
		ret = mtk_cfg80211_reply_extension_channel(wiphy, wdev, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_extension_channel failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_TRANSMITPOWER]) {
		ret = mtk_cfg80211_reply_transmit_power(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_transmit_power failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_80211H]) {
		ret = mtk_cfg80211_reply_80211h(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_80211h failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BASIC_RATE]) {
		ret = mtk_cfg80211_reply_basic_rate(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_basic_rate failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SUPP_RATE]) {
		ret = mtk_cfg80211_reply_supp_rate(wiphy, preq_wdev, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_supp_rate failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_ACS_REFRESH_PERIOD]) {
		ret = mtk_cfg80211_reply_acs_refresh_period(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_acs_refresh_period failed.\n");
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MLO_MLD_INFO]) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
		ret = mtk_cfg80211_reply_mlo_mld_info(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_mld_mlo_info failed.\n");
#endif
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE]) {
		ret = mtk_cfg80211_reply_wmode_info(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wmode failed.\n");
	} else
		return -EINVAL;

	return 0;
}

int mtk_cfg80211_get_static_info(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT ifIndex;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_MAX + 1];
	int ret;
	struct wifi_dev *preq_wdev = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ifname: %s, iftype: %d\n", pNetDev->name, wdev->iftype);

	preq_wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wdev);
	if (!preq_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	if (preq_wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR] Interface is down .\n");
		return -EINVAL;
	}

	if (wdev->iftype == NL80211_IFTYPE_AP)
		ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
#ifdef APCLI_CFG80211_SUPPORT
	else if (wdev->iftype == NL80211_IFTYPE_STATION)
		ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
#endif
	else
		return -EINVAL;

	if (ifIndex == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find ifIndex in driver.\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;

#ifdef WAPP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WAPP_SUPPORT_VER]) {
		ret = mtk_cfg80211_reply_wapp_support_ver(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_support_ver failed.\n");
		else
			return 0;
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_VER]) {
		ret = mtk_cfg80211_reply_wifi_ver(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wifi_ver failed.\n");
		else
			return 0;
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID]) {
		ret = mtk_cfg80211_reply_wapp_chip_id(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_chip_id failed.\n");
		else
			return 0;
	}
#endif /* WAPP_SUPPORT */

#ifdef CONFIG_6G_AFC_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_AFC_DATA]) {
		ret = mtk_cfg80211_reply_afc_data(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_AFC_DATA failed.\n");
		else
			return 0;
	}
#endif

#if (defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT))
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_COEXISTENCE]) {
		ret = mtk_cfg80211_reply_coexistence(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_coexistence failed.\n");
		else
			return 0;
	}
#endif
	return -EINVAL;

}

#ifdef ANDLINK_V4_0
INT	mtk_cfg80211_andlink_get_uplink_info(
	RTMP_ADAPTER *pAd, INT apidx, void *data)
{
	struct wifi_dev *wdev =  NULL;
	struct mtk_wifi_uplink_info *andlink_uplink_info = (struct mtk_wifi_uplink_info *)data;
	INT idx = 0, if_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR, "==>\n");
	/*init andlink_uplink_info*/
	wdev = &pAd->StaCfg[apidx].wdev;
	if (!wdev || !(HcGetBandByWdev(wdev) < CFG_WIFI_RAM_BAND_NUM) ||
		pAd->CommonCfg.andlink_enable != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"wdev is null or andlink enbable(%d) in band(%d).\n",
					pAd->CommonCfg.andlink_enable,
					HcGetBandByWdev(wdev));
		return -EFAULT;
	}

	/*find apcli entry*/
	if (((GetAssociatedAPByWdev(pAd, wdev)) != NULL) && (pAd->StaCfg[apidx].SsidLen != 0)) {
		for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab->Content[idx];

			if (pEntry->wdev != &pAd->StaCfg[apidx].wdev)
				continue;

			if (IS_ENTRY_PEER_AP(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				PMAC_TABLE_ENTRY apcli_entry = NULL;
				mtk_rate_info_t tx_rate;
				mtk_rate_info_t rx_rate;

				if (pAd->StaCfg[apidx].SsidLen > 0)
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR, "ssid %s\n", pAd->StaCfg[apidx].Ssid);

				NdisMoveMemory(&andlink_uplink_info->ssid, pAd->StaCfg[apidx].Ssid,
				pAd->StaCfg[apidx].SsidLen > MAX_LEN_OF_SSID ? MAX_LEN_OF_SSID : pAd->StaCfg[apidx].SsidLen);
				/*channel*/
				andlink_uplink_info->channel = pAd->StaCfg[apidx].wdev.channel;
				if ((apidx >= 0) && IS_WCID_VALID(pAd, pAd->StaCfg[apidx].MacTabWCID))
					apcli_entry = &pAd->MacTab->Content[pAd->StaCfg[apidx].MacTabWCID];
				if (apcli_entry) {
					/*rssi*/
					andlink_uplink_info->rssi = RTMPMaxRssi(pAd, apcli_entry->RssiSample.AvgRssi[0],
												apcli_entry->RssiSample.AvgRssi[1],
												apcli_entry->RssiSample.AvgRssi[2]);
					/*noise*/
					andlink_uplink_info->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
						pAd->ApCfg.RssiSample.AvgRssi[1], pAd->ApCfg.RssiSample.AvgRssi[2]) -
						RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0], pAd->ApCfg.RssiSample.AvgSnr[1]);

					/*snr*/
					andlink_uplink_info->snr = andlink_uplink_info->rssi - andlink_uplink_info->noise;
					/*broadcast and multicast*/
					if (apcli_entry->pMbss) {
						andlink_uplink_info->mcBytesTx = MBSS_GET(pEntry->pMbss)->mcBytesTx;
						andlink_uplink_info->bcBytesTx = MBSS_GET(pEntry->pMbss)->bcBytesTx;
					}
				} else {
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR, "%s apcli_entry null\n", __func__);
				}
				if (get_sta_rate_info(pAd, pEntry, &tx_rate, &rx_rate) == TRUE) {
					/*tx/rx_rate*/
					andlink_uplink_info->tx_rate = (UINT)tx_rate.legacy;
					andlink_uplink_info->rx_rate = (UINT)rx_rate.legacy;
				}

				/*tx/rx_rate_rt Mbps*/
				andlink_uplink_info->tx_rate_rt = (UINT32)pEntry->OneSecTxBytes;
				andlink_uplink_info->rx_rate_rt = (UINT32)pEntry->OneSecRxBytes;
				for (if_idx = 0; if_idx < ANDLINK_IF_MAX; if_idx++) {
					/*avg_tx/rx_rt*/
					andlink_uplink_info->avg_tx_rate[if_idx] = pEntry->andlink_avg_tx_rate[if_idx] / 128;
					andlink_uplink_info->avg_rx_rate[if_idx] = pEntry->andlink_avg_rx_rate[if_idx] / 128;
					/*max_tx/rx_rt*/
					andlink_uplink_info->max_tx_rate[if_idx] = pEntry->andlink_max_tx_rate[if_idx] / 128;
					andlink_uplink_info->max_rx_rate[if_idx] = pEntry->andlink_max_rx_rate[if_idx] / 128;
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_INFO,
						"if_idx=%d avg_tx_rate=%llu avg_rx_rate=%llu max_tx_rate=%llu max_rx_rate=%llu\n",
						if_idx, pEntry->andlink_avg_tx_rate[if_idx], pEntry->andlink_avg_rx_rate[if_idx],
						pEntry->andlink_max_tx_rate[if_idx], pEntry->andlink_max_rx_rate[if_idx]);
				}
			}
			break;
		}
	}

	return 0;
}

INT mtk_andlink_get_mld_sta_idx(UCHAR *mac)
{
	struct bmgr_mld_sta *mld_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	u16 i, j;

	if (!mac) {
		MTWF_DBG(NULL, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"error:mac is NULL\n");
		return -1;
	}

	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);
		if (mld_sta->valid) {
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				mld_link = &mld_sta->mld_link[j];
				if ((mld_link->active || mld_link->requested) && strcmp(mld_link->link_addr, mac) == 0)
					return i;
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
		"error:get mld sta idx fail\n");
	return -1;
}

INT	mtk_cfg80211_andlink_get_stainfo_result(
	RTMP_ADAPTER *pAd, INT apidx, void *data, INT offset)
{
	MAC_TABLE_ENTRY *pEnt = NULL;
	INT idx = 0, if_idx = 0;
	UCHAR *dst_ptr = NULL;
	UCHAR *src_ptr = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY * pRepEnt = NULL;
#endif/*MAC_REPEATER_SUPPORT*/
	INT sta_idx = 0, max_num_of_mac = 33;
	struct mtk_rate_info tx_rate_info;
	struct mtk_rate_info rx_rate_info;
	struct mtk_andlink_wifi_sta_info *sta_info = (struct mtk_andlink_wifi_sta_info *)data;

	for (idx = 0; idx < max_num_of_mac; idx++) {
		pEnt = &pAd->MacTab->Content[idx];
		if (!pEnt || !(IS_ENTRY_CLIENT(pEnt) || IS_ENTRY_PEER_AP(pEnt) || IS_ENTRY_REPEATER(pEnt)))
			continue;
		if (pEnt->wdev && pAd->wdev_list && (pEnt->wdev->channel != pAd->wdev_list[0]->channel))
			continue;
		if (pEnt->Sst == SST_ASSOC) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"\nsta_idx=%d offset=%d\n", sta_idx, offset);
			sta_idx++;
			if (sta_idx <= offset)
				continue;
#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn) && IS_REPT_LINK_UP(pEnt->pReptCli)) {
				if ((sta_info->sta_cnt >= ANDLINK_MAX_ASSOC_NUM) ||
					(sta_info->sta_cnt >= ARRAY_SIZE(sta_info->sta_entry))) {
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"rept_sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
					sta_info->sta_cnt, ANDLINK_MAX_ASSOC_NUM);
					break;
				}
				/*rept OMAC*/
				dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].mac_addr;
				src_ptr = pEnt->pReptCli->OriginalAddress;
				NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

				/*rept VMAC*/
				dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
				src_ptr = pEnt->pReptCli->CurrentAddress;
				NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
			}
#endif/*MAC_REPEATER_SUPPORT*/
			if (pEnt->Sst == SST_ASSOC && pEnt->EntryType == ENTRY_CLIENT) {
				if ((sta_info->sta_cnt >= ANDLINK_MAX_ASSOC_NUM) ||
					(sta_info->sta_cnt >= ARRAY_SIZE(sta_info->sta_entry))) {
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
					sta_info->sta_cnt, ANDLINK_MAX_ASSOC_NUM);
					break;
				}
				/*sta OMAC*/
				dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].mac_addr;
				src_ptr = pEnt->Addr;
				NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

				MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"\nRmac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(dst_ptr));
#ifdef MAC_REPEATER_SUPPORT
				pRepEnt = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEnt->Addr, TRUE);
				if (pRepEnt != NULL && pAd->ApCfg.bMACRepeaterEn) {
					/*sta vamc*/
					dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
					src_ptr = pRepEnt->CurrentAddress;
					NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
				} else
#endif/*MAC_REPEATER_SUPPORT*/
				{
					dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
					src_ptr = pEnt->Addr;
					NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
				}
				MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"\nVmac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(dst_ptr));
				/*mlo_en*/
				if (pEnt->mlo.mlo_en) {
					sta_info->sta_entry[sta_info->sta_cnt].mlo_en = pEnt->mlo.mlo_en;
					sta_info->sta_entry[sta_info->sta_cnt].mlo_idx = (unsigned char)mtk_andlink_get_mld_sta_idx(dst_ptr);
					sta_info->sta_entry[sta_info->sta_cnt].setup_link = pEnt->mlo.is_setup_link_entry;
				} else {
					sta_info->sta_entry[sta_info->sta_cnt].mlo_en = 0;
					sta_info->sta_entry[sta_info->sta_cnt].mlo_idx = 0;
					sta_info->sta_entry[sta_info->sta_cnt].setup_link = 0;
				}
				/* RSSI */
				sta_info->sta_entry[sta_info->sta_cnt].rssi = RTMPMaxRssi(pAd, pEnt->RssiSample.AvgRssi[0],
					pEnt->RssiSample.AvgRssi[1], pEnt->RssiSample.AvgRssi[2]);
				sta_info->sta_entry[sta_info->sta_cnt].uptime = (ULONGLONG)pEnt->StaConnectTime;
				/*Rate*/
				get_sta_rate_info(pAd, pEnt, &tx_rate_info, &rx_rate_info);
				sta_info->sta_entry[sta_info->sta_cnt].tx_rate = tx_rate_info.legacy;/*tx_rate*/
				sta_info->sta_entry[sta_info->sta_cnt].rx_rate = rx_rate_info.legacy;
				/*bw*/
				sta_info->sta_entry[sta_info->sta_cnt].bw = rx_rate_info.bw;
				/*mode*/
				sta_info->sta_entry[sta_info->sta_cnt].mode = pEnt->HTPhyMode.field.MODE;
				/*Rate_rt*/
				sta_info->sta_entry[sta_info->sta_cnt].tx_rate_rt = (pEnt->AvgTxBytes >> 17);
				sta_info->sta_entry[sta_info->sta_cnt].rx_rate_rt = (pEnt->AvgRxBytes >> 17);
				MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"\ntx_rate_rt=%lu rx_rate_rt=%lu\n",
					sta_info->sta_entry[sta_info->sta_cnt].tx_rate_rt,
					sta_info->sta_entry[sta_info->sta_cnt].rx_rate_rt);
				for (if_idx = 0; if_idx < ANDLINK_IF_MAX; if_idx++) {
					sta_info->sta_entry[sta_info->sta_cnt].avg_tx_rate[if_idx] = pEnt->andlink_avg_tx_rate[if_idx];
					sta_info->sta_entry[sta_info->sta_cnt].avg_rx_rate[if_idx] = pEnt->andlink_avg_rx_rate[if_idx];
					sta_info->sta_entry[sta_info->sta_cnt].max_tx_rate[if_idx] = pEnt->andlink_max_tx_rate[if_idx];
					sta_info->sta_entry[sta_info->sta_cnt].max_rx_rate[if_idx] = pEnt->andlink_max_rx_rate[if_idx];
					MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"\navg_tx_rate=%llu max_tx_rate=%llu avg_rx_rate=%llu max_rx_rate=%llu avg_tx_rate =%p\n",
					pEnt->andlink_avg_tx_rate[if_idx],
					pEnt->andlink_max_tx_rate[if_idx],
					pEnt->andlink_avg_rx_rate[if_idx],
					pEnt->andlink_max_rx_rate[if_idx],
					pEnt->andlink_avg_tx_rate[if_idx]);
				}
				sta_info->sta_entry[sta_info->sta_cnt].rx_pkts = (ULONGLONG)pEnt->TxPackets.QuadPart;
				sta_info->sta_entry[sta_info->sta_cnt].rx_bytes = pEnt->TxBytes;
				sta_info->sta_entry[sta_info->sta_cnt].tx_bytes = pEnt->RxBytes;
				sta_info->sta_entry[sta_info->sta_cnt].tx_pkts = (ULONGLONG)pEnt->RxPackets.QuadPart;
				MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"\nsta_cnt =%d rx_pkts=%llu tx_pkts=%llu rx_bytes=%llu tx_bytes=%llu tx_rate_rt=%ld rx_rate_rt=%ld\n",
					sta_info->sta_cnt,
					sta_info->sta_entry[sta_info->sta_cnt].rx_pkts,
					sta_info->sta_entry[sta_info->sta_cnt].tx_pkts,
					sta_info->sta_entry[sta_info->sta_cnt].rx_bytes,
					sta_info->sta_entry[sta_info->sta_cnt].tx_bytes,
					sta_info->sta_entry[sta_info->sta_cnt].tx_rate_rt,
					sta_info->sta_entry[sta_info->sta_cnt].rx_rate_rt);
				sta_info->sta_cnt++;
			}
		}
	}
	return 0;
}
INT	mtk_cfg80211_andlink_get_scan_result(
	RTMP_ADAPTER *pAd, INT apidx, void *data, INT offset)
{
	struct wifi_dev *wdev =  NULL;
	BSS_TABLE *scan_tab = NULL;
	BSS_ENTRY *bss = NULL;
	INT bss_idx = 0;
	INT idx = 0;
	struct mtk_wifi_scan_info *andlink_scan_info = (struct mtk_wifi_scan_info *)data;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	int bw20 = 1, bw40 = 2, bw80 = 3, bw160 = 4, bw80_80 = 5;
	int wifi_mode[] = {Ndis802_11A_B, Ndis802_11A_B, Ndis802_11A_B, Ndis802_11B_G, Ndis802_11A_B, Ndis802_11A_N, Ndis802_11B_G_N, Ndis802_11A_N_AC, Ndis802_11B_G_N_AX, Ndis802_11A_N_AC_AX};

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	scan_tab = get_scan_tab_by_wdev(pAd, wdev);
	if (scan_tab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"scan_tab is null(%p).\n", scan_tab);
		return -EFAULT;
	}

	for (idx = 0; idx < scan_tab->BssNr; idx++) {
		if (idx < offset)
			continue;
		bss = &scan_tab->BssEntry[idx];
		if (bss == NULL || bss->SsidLen <= 0 || bss->Channel == 0) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"bss(%p)  SSID(%s) or channel(%d) is invalid.\n",
					bss, bss->Ssid, bss->Channel);
			continue;
		}
		bss_idx = idx - offset;
		if (bss_idx >= ANDLINK_MAX_WLAN_NEIGHBOR) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"bss(%p) is null or bss_idx >= %d\n", bss, bss_idx);
			break;
		}
		/*bss*/
		NdisMoveMemory(andlink_scan_info->scan_entry[bss_idx].ssid, bss->Ssid, SSID_LEN-1);
		/*mac_addr*/
		COPY_MAC_ADDR(andlink_scan_info->scan_entry[bss_idx].mac_addr, bss->Bssid);
		andlink_scan_info->scan_entry[bss_idx].channel = bss->Channel;/*channel*/
		andlink_scan_info->scan_entry[bss_idx].rssi = bss->Rssi;/*rssi*/
		/* bandwidth*/
		if (WMODE_CAP_6G(wdev->PhyMode))
			andlink_scan_info->scan_entry[bss_idx].bandwidth = 0;
		else if (WMODE_CAP_5G(wdev->PhyMode)) {
			/*80+80MHz*/
			if (bss->vht_op_ie.vht_op_info.ch_width == 3)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80_80;
			else if (bss->vht_op_ie.vht_op_info.ch_width == 2)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw160; /*160MHz*/
			else if (bss->vht_op_ie.vht_op_info.ch_width == 1) {
				/*80MHz*/
				if (bss->vht_op_ie.vht_op_info.ccfs_1 == 0)
					andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80;
				else {
					if (((bss->vht_op_ie.vht_op_info.ccfs_1) - (bss->vht_op_ie.vht_op_info.ccfs_0)) == 8
							|| ((bss->vht_op_ie.vht_op_info.ccfs_1) - (bss->vht_op_ie.vht_op_info.ccfs_0)) == -8)
						andlink_scan_info->scan_entry[bss_idx].bandwidth = bw160; /*160MHz*/
					else
						andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80_80; /*80+80MHz*/
				}
			} else if (bss->vht_op_ie.vht_op_info.ch_width == 0 && bss->vht_op_ie.vht_op_info.ccfs_0 == 1)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw40; /*40MHz*/
			else
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20; /*20MHz*/
		} else if (bss->HtCapability.HtCapInfo.ChannelWidth == HT_BW_20)
			andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20;/*20*/
		else
			andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20;/*40MHz*/
		/*loadRate*/
		andlink_scan_info->scan_entry[bss_idx].load_rate = bss->QbssLoad.ChannelUtilization * 100 / 255;
		/* wifistandard*/
		wireless_mode = NetworkTypeInUseSanity(bss);
		if (wireless_mode < (ARRAY_SIZE(wifi_mode)))
			andlink_scan_info->scan_entry[bss_idx].wifistandard = wifi_mode[wireless_mode];
		else
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 0;
		/*authmode*/
		snprintf(andlink_scan_info->scan_entry[bss_idx].authmode, ANDLINK_SEC_LEN,
			"%s", GetAuthModeStr(bss->AKMMap));
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
					"load_rate=%d,wifistandard=%d,authmode=%s\n",
					andlink_scan_info->scan_entry[bss_idx].load_rate,
					andlink_scan_info->scan_entry[bss_idx].wifistandard,
					andlink_scan_info->scan_entry[bss_idx].authmode);
	}
	andlink_scan_info->num = bss_idx;

	MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"bss_idx(%d), offset(%d), ssid(%s).\n",
					bss_idx, offset, bss->Ssid);
	return 0;
}

INT	mtk_cfg80211_reply_andlink_uplink_info(
	struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx = 0;
	int ret;
	POS_COOKIE pObj;
	struct mtk_wifi_uplink_info andlink_uplink_info = {0};

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

	mtk_cfg80211_andlink_get_uplink_info(pAd, apidx, &andlink_uplink_info);
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_UPLINK_STATISTICS_ANDLINK_FORMAT,
					&andlink_uplink_info, sizeof(struct mtk_wifi_uplink_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"error:reply andlink uplink info fail!\n");
	return ret;
}

INT	mtk_cfg80211_reply_andlink_sta_info(
	struct wiphy *wiphy, const void *data, int len)
{
	INT apidx = 0;
	POS_COOKIE pObj;
	PRTMP_ADAPTER pAd;
	INT	Status = NDIS_STATUS_SUCCESS;
	INT ret = 0;
	INT offset = 0;
	struct mtk_andlink_wifi_sta_info *sta_info = NULL;
	struct mtk_andlink_wifi_sta_info *sta_info_recv = NULL;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"VALID_MBSS\n");
		return -EFAULT;
	}

	Status = os_alloc_mem(pAd, (UCHAR **)&sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	if (Status != NDIS_STATUS_SUCCESS)
		return -ENOMEM;
	NdisZeroMemory(sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	/*get offset*/
	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;
	sta_info_recv = (struct mtk_andlink_wifi_sta_info *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT]);
	offset = sta_info_recv->offset;
	NdisZeroMemory(sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	sta_info->sta_cnt = 0;

	mtk_cfg80211_andlink_get_stainfo_result(pAd, apidx, sta_info, offset);
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT,
					sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"error:reply station info fail!\n");
	os_free_mem(sta_info);
	return Status;
}

INT	mtk_cfg80211_reply_andlink_bss_info(
	struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT	status = NDIS_STATUS_SUCCESS;
	struct wifi_dev *wdev =  NULL;
	struct mtk_andlink_radio_info radio_info = {0};
	RX_STATISTIC_RXV *rx_stat_rxv = NULL;
	POS_COOKIE pObj;
	INT apidx = 0;
	INT ret = 0;

	MAC_TABLE_ENTRY *pEnt = NULL;
	INT idx = 0, max_num_of_mac = 33, sta_cnt = 0;
	UINT8 u1Snr[4] = {0}, snr = 0;
	signed char rssi = 0;

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"VALID_MBSS\n");
		return -EFAULT;
	}
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	rx_stat_rxv = &pAd->rx_stat_rxv;

	/*band snr/rssi/noise infos*/
	radio_info.rssi = RTMPMaxRssi(pAd, rx_stat_rxv->RSSI[0], rx_stat_rxv->RSSI[1], rx_stat_rxv->RSSI[2]);
	radio_info.snr = rx_stat_rxv->SNR[0];
	radio_info.noise = radio_info.rssi - radio_info.snr;

	for (idx = 0; idx < max_num_of_mac; idx++) {
		pEnt = &pAd->MacTab->Content[idx];
		if (!pEnt || !(IS_ENTRY_CLIENT(pEnt) || IS_ENTRY_PEER_AP(pEnt) || IS_ENTRY_REPEATER(pEnt)))
			continue;
		if (pEnt->Sst == SST_ASSOC) {
			/* RSSI */
			rssi += RTMPMaxRssi(pAd, pEnt->RssiSample.AvgRssi[0],
					pEnt->RssiSample.AvgRssi[1], pEnt->RssiSample.AvgRssi[2]);

			HW_GET_STA_SNR(pAd, pEnt->wcid, u1Snr);
			snr += u1Snr[0];
			sta_cnt++;
		}
	}

	radio_info.rssi = (signed char)(rssi / sta_cnt);
	radio_info.snr = (signed char)(snr / sta_cnt);
	radio_info.noise = radio_info.rssi - radio_info.snr;
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_RADIO_BSS_INFO_ANDLINK_FORMAT,
					&radio_info, sizeof(struct mtk_andlink_radio_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"error:reply andlink bss info fail!\n");

	return status;
}

INT	mtk_cfg80211_reply_andlink_scan_result_info(
	struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	INT apidx = 0, ret = 0, offset = 0;
	INT status = NDIS_STATUS_SUCCESS;
	struct mtk_wifi_scan_info *andlink_scan_info_recev = NULL;
	struct mtk_wifi_scan_info *andlink_scan_info = NULL;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SCAN_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
					"VALID_MBSS\n");
		return -EFAULT;
	}
	/*get offset*/
	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SCAN_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;
	status = os_alloc_mem(pAd, (UCHAR **)&andlink_scan_info, sizeof(struct mtk_wifi_scan_info));
	if (status != NDIS_STATUS_SUCCESS)
		return -ENOMEM;
	andlink_scan_info_recev = (struct mtk_wifi_scan_info *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT]);
	offset = andlink_scan_info_recev->offset;
	MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_WARN,
					"offset(%d).\n", offset);
	mtk_cfg80211_andlink_get_scan_result(pAd, apidx, andlink_scan_info, offset);
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT,
					andlink_scan_info, sizeof(struct mtk_wifi_scan_info));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"error:reply scan result fail!\n");
	os_free_mem(andlink_scan_info);
	return ret;
}
#endif /*ANDLINK_V4_0*/
int mtk_cfg80211_get_statistic(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	ULONG priv_flags;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_MAX + 1];
	int ret;
	struct wifi_dev *preq_wdev = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ifname: %s\n", pNetDev->name);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pObj in driver .\n");
		return -EINVAL;
	}

	preq_wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wdev);
	if (!preq_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	if (preq_wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR] Interface is down .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;
#ifdef ANDLINK_V4_0
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_UPLINK_STATISTICS_ANDLINK_FORMAT]) {
		ret = mtk_cfg80211_reply_andlink_uplink_info(wiphy, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_andlink_uplink_info failed.\n");
			return -EINVAL;
		} else
			return 0;
	}
#endif/*ANDLINK_V4_0*/
#ifdef WAPP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_TX_PWR]) {
		ret = mtk_cfg80211_reply_wapp_tx_pwr(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_tx_pwr failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_AP_METRICS]) {
		ret = mtk_cfg80211_reply_wapp_ap_metrics(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wapp_ap_metrics failed.\n");
	} else
		return -EINVAL;
#endif /* WAPP_SUPPORT */

	return 0;
}

int mtk_cfg80211_get_band_info(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX + 1];
	int ret;
	PRTMP_ADAPTER pAd;
	UCHAR band_idx = 0;

	MAC80211_PAD_GET(pAd, wiphy);
	band_idx = hc_get_hw_band_idx(pAd);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX,
		(struct nlattr *)data, len, NULL, NULL);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND]) {

		ret = mtk_cfg80211_reply_band_info_band(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_band_info_band failed.\n");
		else
			return 0;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST]) {
		ret = mtk_cfg80211_reply_band_info_freqlist(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_band_info_freqlist failed.\n");
		else
			return 0;
		return 0;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BANDWIDTH]) {
		ret = mtk_cfg80211_reply_band_info_bandwidth(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_band_info_bandwidth failed.\n");
		else
			return 0;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_CHANNEL]) {
		ret = mtk_cfg80211_reply_band_info_channel(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_band_info_channel failed.\n");
		else
			return 0;
	}


	return 0;

}

#ifdef WAPP_SUPPORT
static int parse_wapp_req(PRTMP_ADAPTER pAd,
				struct wiphy *wiphy,
				struct wireless_dev *wdev,
				struct nlattr *tb, int len)
{
	struct wapp_req *req;
	req = (struct wapp_req *)nla_data(tb);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		   "req_id %d\n", req->req_id);

	if (nla_len(tb) < sizeof(struct wapp_req))
		return -EINVAL;

	wapp_event_handle(pAd, req, wdev);

	return 0;
}
#ifdef RT_CFG80211_SUPPORT
#endif
#endif /* WAPP_SUPPORT */

static UCHAR update_auto_ch_skip_list(PRTMP_ADAPTER pAd, struct ch_list_info *skip_list)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR i;
	UCHAR ChIdx;
	UCHAR valid = FALSE;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define MAX_SKIP_CH_NUM 20
#else
#define MAX_SKIP_CH_NUM MAX_NUM_OF_CHANNELS
#endif
	UCHAR skip_ch[MAX_SKIP_CH_NUM] = {0};
	UCHAR skip_count = 0;

	if (skip_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"skip_list is NULL!\n");
		return FALSE;
	}

	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pChCtrl is NULL!\n");
		return FALSE;
	}

	/*if skip_list->num_of_ch == 0 originally, means need reset auto ch skip list*/
	if (skip_list->num_of_ch == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"reset auto ch skip list!\n");
		os_zero_mem(pAd->ApCfg.AutoChannelSkipList, MAX_SKIP_CH_NUM);
		pAd->ApCfg.AutoChannelSkipListNum = 0;
		return TRUE;
	}

	/*skip_list->num_of_ch valid check*/
	if (skip_list->num_of_ch > MAX_SKIP_CH_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"invalid skip_list ch num!\n");
		return FALSE;
	}

	/*at least one channel is valid, the skip list will be effective*/
	for (i = 0; i < skip_list->num_of_ch; i++) {
		UCHAR ch;

		ch = skip_list->ch_list[i];
		for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
			if (ch == pChCtrl->ChList[ChIdx].Channel) {
				valid = TRUE;
				break;
			}
		}

		if (valid)
			break;
	}

	if (!valid) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"no valid channel, set skip list fail!\n");
		return FALSE;
	}

	/*when skip list is valid, needs reset the skip list first*/
	os_zero_mem(pAd->ApCfg.AutoChannelSkipList, MAX_SKIP_CH_NUM);
	pAd->ApCfg.AutoChannelSkipListNum = 0;

	/*the channels which are in skip list, should also be in ChCtrl list*/
	for (i = 0; i < skip_list->num_of_ch; i++) {
		UCHAR ch;
		UCHAR find = FALSE;

		ch = skip_list->ch_list[i];
		for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
			if (ch == pChCtrl->ChList[ChIdx].Channel) {
				find = TRUE;
				break;
			}
		}

		if (find) {
			skip_ch[skip_count] = ch;
			skip_count++;
		}
	}

	memcpy(&pAd->ApCfg.AutoChannelSkipList[0], &skip_ch[0], skip_count);
	pAd->ApCfg.AutoChannelSkipListNum = skip_count;

	for (ChIdx = 0; ChIdx < pAd->ApCfg.AutoChannelSkipListNum; ChIdx++)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"get skip list, ch = %d\n", pAd->ApCfg.AutoChannelSkipList[ChIdx]);

	return TRUE;
}

static UCHAR update_auto_ch_priority_from_prefer_list(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct ch_list_info *prefer_list)
{

	if (prefer_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"prefer_list is NULL!\n");
		return FALSE;
	}

	if ((prefer_list->num_of_ch == 0) || (prefer_list->num_of_ch > MAX_NUM_OF_CHANNELS)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"invalid prefer_list ch num!\n");
		return FALSE;
	}

	if (!UpdatePreferChannel(pAd, wdev, prefer_list->ch_list, prefer_list->num_of_ch,
		CH_PRIO_OP_OWNER_RUNTIME_CFG)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"UpdateChannelPriority failed!\n");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN mtk_cfg80211_auto_ch_sel(PRTMP_ADAPTER pAd,
										struct wifi_dev *wifi_wdev)
{
	INT32 ret = 0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if (!wifi_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "NULL wdev!\n");
		return FALSE;
	}

	/*if scan is running, ACS will not be triggered*/
#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, wifi_wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return FALSE;
	}
#endif

#ifdef TR181_SUPPORT
	{
		struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

		/*set ACS trigger flag to Manual trigger*/
		if (ctrl)
			ctrl->rdev.pRadioCtrl->ACSTriggerFlag = 2;
	}
#endif /*TR181_SUPPORT*/
	/*To do ACS, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wifi_wdev, CH_OP_OWNER_ACS, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for ACS!!\n");
		return FALSE;
	}

	pAd->ApCfg.iwpriv_event_flag = TRUE;
	AutoChSelScanStart(pAd, wifi_wdev);

	ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((600*100*OS_HZ)/1000));/* Wait 60s.*/
	if (ret)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"wait channel setting success.\n");
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				"wait channel setting timeout.\n");
		pAd->ApCfg.set_ch_async_flag = FALSE;
		pAutoChCtrl = HcGetAutoChCtrl(pAd);
		if (pAutoChCtrl)
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	/*When ACS is done, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wifi_wdev, CH_OP_OWNER_ACS);

	return TRUE;
}

static const struct
nla_policy SUBCMD_SET_AUTO_CH_SEL_POLICY[];

int mtk_cfg80211_vndr_set_auto_ch_sel_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *pNetDev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_MAX + 1];
	ULONG *__pPriv;
	ULONG priv_flags;
	PRTMP_ADAPTER pAd = NULL;
	POS_COOKIE pObj;
	struct wifi_dev *wifi_wdev = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	int ret;
	BOOLEAN need_trigger_acs = FALSE;

	if (!wiphy) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]wiphy is NULL\n");
		return -EFAULT;
	}
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]wdev is NULL\n");
		return -EFAULT;
	}

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]pAd is NULL\n");
		return -EFAULT;
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	pNetDev = wdev->netdev;
	if (!pNetDev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]pNetDev is NULL\n");
		return -EFAULT;
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
		"ifname: %s\n", pNetDev->name);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]pObj is NULL\n");
		return -EFAULT;
	}

	if (pObj->ioctl_if_type != INT_MBSSID && pObj->ioctl_if_type != INT_MAIN) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"not ap wdev, return");
		return -EINVAL;
	}

	wifi_wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (!wifi_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"[ERROR]wifi_wdev is NULL\n");
		return -EFAULT;
	}
	pAutoChCtrl = HcGetAutoChCtrl(pAd);
	if (pAutoChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR,
			"pAutoChCtrl is NULL, return\n");
		return -EFAULT;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_AUTO_CH_SEL_POLICY, NULL);
	if (ret)
		return -EINVAL;

	/*if ACS is running, the ACS parameter should not be changed*/
	if (GetCurrentChannelOpOwner(pAd, wifi_wdev) == CH_OP_OWNER_ACS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"Failed!!!ACS is running, please try again after ACS done!\n");
		return -EFAULT;
	}

#ifdef CONFIG_6G_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC]) {
		u8 psc_enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC]);

		if (wlan_config_get_ch_band(wifi_wdev) != CMD_CH_BAND_6G) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
				"not 6G BAND, no need configure PSC ACS.\n");
		} else {
			if (psc_enable > 0)
				pAutoChCtrl->AutoChSelCtrl.PSC_ACS = TRUE;
			else
				pAutoChCtrl->AutoChSelCtrl.PSC_ACS = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
					"6G PSC_ACS is %d\n", pAutoChCtrl->AutoChSelCtrl.PSC_ACS);
		}
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST]) {
		struct ch_list_info *skip_list;

		skip_list = (struct ch_list_info *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST]);
		if (update_auto_ch_skip_list(pAd, skip_list)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"skip_list is set!\n");
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST]) {
		struct ch_list_info *prefer_list;

		prefer_list = (struct ch_list_info *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST]);
		if (update_auto_ch_priority_from_prefer_list(pAd, wifi_wdev, prefer_list)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
			"prefer_list is set!\n");
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL]) {
		u8 Partial_Scan_ACS = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL]);

		if (Partial_Scan_ACS > 0)
			pAutoChCtrl->AutoChSelCtrl.PartialScanACS = TRUE;
		else
			pAutoChCtrl->AutoChSelCtrl.PartialScanACS = FALSE;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"PartialScanACS is %d\n", pAutoChCtrl->AutoChSelCtrl.PartialScanACS);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL]) {
		unsigned short ScanningDwell = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Set Scanning Dwell=%d\n", ScanningDwell);
		pAutoChCtrl->AutoChSelCtrl.ScanningDwell = ScanningDwell;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL]) {
		unsigned short RestoreDwell = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Set Restoring Dwell=%d\n", RestoreDwell);
		pAutoChCtrl->AutoChSelCtrl.RestoreDwell = RestoreDwell;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM]) {
		u8 ScanningChNum = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_INFO,
				"Set Partial Scan ACS ScanningChNum=%d\n", ScanningChNum);
		pAutoChCtrl->AutoChSelCtrl.ScanningChNum = ScanningChNum;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL]) {
		unsigned char sel = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL]);

		switch (sel) {
		case 7:
			pAd->ApCfg.auto_ch_score_flag = TRUE;
			fallthrough;
		case 3:
			pAd->ApCfg.AutoChannelAlg = ChannelAlgBusyTime;
			need_trigger_acs = TRUE;
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
					"Algo(%d) error!\n", sel);
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME]) {
		u32 check_time = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"check time=%d\n", check_time);
		pAd->ApCfg.AcsCfg.ACSCheckTime = check_time;
		pAutoChCtrl->AutoChSelCtrl.ACSCheckCount = 0;
		if (pAd->ApCfg.AcsCfg.ACSCheckTime > 0
			&& pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime)
			need_trigger_acs = TRUE;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR]) {
		u8 ch_utl_thr = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Set Periodic ACS ChUtilThr=%d\n", ch_utl_thr);
		pAd->ApCfg.AcsCfg.ChUtilThr = ch_utl_thr;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR]) {
		u16 sta_num_thr = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR]);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Set Periodic ACS StaNumThr=%d\n", sta_num_thr);
		pAd->ApCfg.AcsCfg.StaNumThr = sta_num_thr;
	}

	if (need_trigger_acs) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_NOTICE,
				"Start to trigger acs scan.\n");
		if (!mtk_cfg80211_auto_ch_sel(pAd, wifi_wdev)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_WARN,
					"Trigger acs scan failed.\n");
			return -EINVAL;
		}
	}

	return 0;
}

#ifdef SCAN_SUPPORT
static const struct
nla_policy SUBCMD_SET_SCAN_POLICY[];

int mtk_cfg80211_vndr_set_scan_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *pNetDev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_SCAN_MAX + 1];
	ULONG *__pPriv;
	PRTMP_ADAPTER pAd = NULL;
	ULONG priv_flags;
	POS_COOKIE pObj;
	struct wifi_dev *wifi_wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	int ret;
	UCHAR ap_scan = TRUE;
	UCHAR scan_type;
#ifdef OFFCHANNEL_SCAN_FEATURE
	UINT32 target_ch;
	UCHAR active;
	UINT32 duration;
#endif
	NDIS_802_11_SSID Ssid;
	int ssid_len = 0;
	char *p = NULL;

	if (!wiphy) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]wiphy is NULL\n");
		return -EFAULT;
	}
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]wdev is NULL\n");
		return -EFAULT;
	}

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]pAd is NULL\n");
		return -EFAULT;
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EFAULT;
	}

	pNetDev = wdev->netdev;
	if (!pNetDev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]pNetDev is NULL\n");
		return -EFAULT;
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"ifname: %s\n", pNetDev->name);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);

	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]pObj is NULL\n");
		return -EFAULT;
	}

	wifi_wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wifi_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"[ERROR]wifi_wdev is NULL\n");
		return -EFAULT;
	}

	if (pObj->ioctl_if_type == INT_APCLI) {
		ap_scan = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
			"not ap_scan\n");
	}

	ScanCtrl = &pAd->ScanCtrl;
	if (ScanCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"failed - [ERROR]ScanCtrl is NULL\n");
		return -EFAULT;
	}

	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_SCAN_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_SCAN_POLICY, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX]) {
		UINT32 bss_start_idx = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX]);

		/* No need to dump scan result if we are doing scanning */
		if (scan_in_run_state(pAd, wifi_wdev) || (ScanCtrl->PartialScan.bScanning == TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"scan is ongoing, no need to dump scan table\n");
			return -EBUSY;
		}

		if (!mtk_cfg80211_get_scan_result(wiphy, pAd, ScanCtrl, bss_start_idx)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
					"fail to get scan result\n");
			return -EFAULT;
		}

		return 0;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR]) {
		/* Don't clear the scan table if we are doing scanning */
		if (scan_in_run_state(pAd, wifi_wdev) || (ScanCtrl->PartialScan.bScanning == TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"scan is ongoing, can not clear scan table\n");
			return -EBUSY;
		}

		BssTableInit(&ScanCtrl->ScanTab);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"clear scan table!\n");
		return 0;
	}
#ifdef CONFIG_6G_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN]) {
		UCHAR psc_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN]);

		if (psc_en > 0)
			ScanCtrl->psc_scan_en = TRUE;
		else
			ScanCtrl->psc_scan_en = FALSE;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"6G PSC scan is %d\n", ScanCtrl->psc_scan_en);
		return 0;
	}
#endif
#ifdef ANDLINK_V4_0
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT]) {
		ret = mtk_cfg80211_reply_andlink_scan_result_info(wiphy, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"failed - [ERROR]mtk_cfg80211_reply_andlink_scan_result_info fail.\n");
			return -EINVAL;
		}
		return 0;
	}
#endif/*ANDLINK_V4_0*/
	if (tb[MTK_NL80211_VENDOR_ATTR_SCAN_TYPE]) {
		scan_type = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_SCAN_TYPE]);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"scan_type = %d\n", scan_type);
	} else
		return -EINVAL;

	/*do dfs cac check for 5G band*/
#ifdef MT_DFS_SUPPORT
	if (wlan_config_get_ch_band(wifi_wdev) == CMD_CH_BAND_5G) {
		struct DOT11_H *pDot11h = &pAd->Dot11_H;

		if (pDot11h && (pDot11h->ChannelMode == CHAN_SILENCE_MODE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"still in silence mode, can not trigger scan\n");
			return -EFAULT;
		}
	}
#endif

	switch (scan_type) {
	case NL80211_FULL_SCAN:
		if (tb[MTK_NL80211_VENDOR_ATTR_SCAN_SSID]) {
			ssid_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SCAN_SSID]);
			p = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SCAN_SSID]);

			if (ssid_len > 32 || ssid_len < 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"invalid ssid_len=%d, force to do passive scan\n", ssid_len);
				ssid_len = 0;
			}
			if (ssid_len == 0)
				Ssid.SsidLength = 0;
			else {
				Ssid.SsidLength = ssid_len;
				memcpy(Ssid.Ssid, p, ssid_len);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"SsidLength = %d, ssid : %s\n", Ssid.SsidLength, Ssid.Ssid);
			}
		}

		if (ap_scan) {
			if (ssid_len == 0)
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_PASSIVE, FALSE, wifi_wdev);
			else
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wifi_wdev);
		} else {
			/*To do SiteSurvey, need TakeChannelOpCharge first*/
			if (!TakeChannelOpCharge(pAd, wifi_wdev, CH_OP_OWNER_SCAN, FALSE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"TakeChannelOpCharge fail for SCAN!!\n");
				return -EBUSY;
			}
			if (pObj->ioctl_if >= 0)
				pAd->StaCfg[pObj->ioctl_if].bSkipAutoScanConn = TRUE;
			else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
					"invalid ioctl if\n");
				return -EBUSY;
			}
			StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, wifi_wdev);
		}

		break;

	case NL80211_PARTIAL_SCAN:
		if (tb[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_INTERVAL]) {
			UINT32 TimerInterval = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_INTERVAL]);

			ScanCtrl->PartialScan.TimerInterval = TimerInterval;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"set partial scan timer interval = %d\n", TimerInterval);
		}

		if (tb[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH]) {
			unsigned char num_of_ch = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH]);

			ScanCtrl->PartialScan.NumOfChannels = num_of_ch;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"set partial scan Number of channels = %d\n", num_of_ch);
		}

		if (ScanCtrl->PartialScan.bScanning) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"partial scan is ongoing, cannot trigger again\n");
			return -EBUSY;
		}
		/*To set ParitalScan flag TRUE, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wifi_wdev, CH_OP_OWNER_PARTIAL_SCAN, TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"TakeChannelOpCharge fail for PARTIAL SCAN!!\n");
			return -EBUSY;
		}

		ScanCtrl->PartialScan.pwdev = wifi_wdev;
		wifi_wdev->ScanInfo.LastScanChannel = 0;
		ScanCtrl->PartialScan.bScanning = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"set partial scan bScanning = %d\n", ScanCtrl->PartialScan.bScanning);
		break;

#ifdef OFFCHANNEL_SCAN_FEATURE
	case NL80211_OFF_CH_SCAN:
		if (tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH]) {
			target_ch = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH]);

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"channel number to trigger off-ch scan:%d\n", target_ch);
			if (!IsValidChannel(pAd, target_ch, NULL)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
						"wrong channel number to trigger off-ch scan:%d\n", target_ch);
				return -EINVAL;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"no target ch to trigger off-ch scan, return\n");
			return -EINVAL;
		}

		if (tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE]) {
			active = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE]);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"active to trigger off-ch scan:%d\n", active);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"not set active=1/0 to trigger off-ch scan, return\n");
			return -EINVAL;
		}

		if (tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION]) {
			duration = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION]);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"duration to trigger off-ch scan:%d\n", duration);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"no duration to trigger off-ch scan, return\n");
			return -EINVAL;
		}

		/*To do OffChannelScan, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wifi_wdev, CH_OP_OWNER_SCAN, TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"TakeChannelOpCharge fail for Off-Channel SCAN!!\n");
			return -EBUSY;
		}

		ScanCtrl->Num_Of_Channels = 1;
		ScanCtrl->ScanTime[0] = 0;
		ScanCtrl->CurrentGivenChan_Index = 0;
		ScanCtrl->state = OFFCHANNEL_SCAN_START;
		if (active)
			ApSiteSurveyNew_by_wdev(pAd, target_ch, duration, SCAN_ACTIVE, FALSE, wifi_wdev);
		else
			ApSiteSurveyNew_by_wdev(pAd, target_ch, duration, SCAN_PASSIVE, FALSE, wifi_wdev);

		break;
#endif
	case NL80211_2040_OVERLAP_SCAN:
		if (!mtk_cfg80211_trigger_2040_overlap_scan(pAd, wifi_wdev)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
					"fail to trigger 2040 overlap scan, return\n");
			return -EFAULT;
		}

		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#endif
#ifdef WAPP_SUPPORT
static const struct
nla_policy SUBCMD_WAPP_REQ_POLICY[];

int mtk_cfg80211_wapp_req(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_WAPP_REQ_MAX + 1];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"ifname: %s\n", pNetDev->name);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_WAPP_REQ_MAX, (struct nlattr *)data, len, SUBCMD_WAPP_REQ_POLICY, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_WAPP_REQ]) {
		ret = parse_wapp_req(pAd, wiphy, wdev, tb[MTK_NL80211_VENDOR_ATTR_WAPP_REQ], len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_wapp_event failed.\n");
	} else
		return -EINVAL;

	return 0;
}
#endif /* WAPP_SUPPORT */
#ifdef VOW_SUPPORT
static int mtk_cfg80211_reply_vow_info(PRTMP_ADAPTER pAd, struct wiphy *wiphy, UINT32 group)
{
	int ret;
	struct vow_info vow_info;

	if (group >= VOW_MAX_GROUP_NUM) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_VOW, DBG_LVL_ERROR,
			"%s: set command failed. group(=%d)\n", __func__, group);
		return -EFAULT;
	}
	vow_info.atf_en = pAd->vow_cfg.en_airtime_fairness;
	vow_info.bw_en = pAd->vow_cfg.en_bw_ctrl;
	vow_info.ratio = pAd->vow_bss_cfg[group].max_airtime_ratio;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO,
					&vow_info, sizeof(struct vow_info));

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}

static const struct
nla_policy SUBCMD_SET_AP_VOW_POLICY[];

int mtk_cfg80211_vow_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_VOW_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_VOW_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_AP_VOW_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO]) {

		UCHAR vow_atf_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO]);

		ret = mtk_cfg80211_set_vow_atf_en(pAd, vow_atf_en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO]) {
		UCHAR vow_bw_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO]);

		ret = mtk_cfg80211_set_vow_bw_en(pAd, vow_bw_en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO]) {
		struct vow_group_en_param *param;

		param = (struct vow_group_en_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO]);
		ret = mtk_cfg80211_set_vow_atc_en(pAd, param->group, param->en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO]) {
		struct vow_group_en_param *param;

		param = (struct vow_group_en_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO]);
		ret = mtk_cfg80211_set_vow_bw_ctrl_en(pAd, param->group, param->en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO]) {
		struct vow_rate_param *param;

		param = (struct vow_rate_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO]);
		ret = mtk_cfg80211_set_vow_min_rate(pAd, param->group, param->rate);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO]) {
		struct vow_rate_param *param;

		param = (struct vow_rate_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO]);
		ret = mtk_cfg80211_set_vow_max_rate(pAd, param->group, param->rate);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO]) {
		struct vow_ratio_param *param;

		param = (struct vow_ratio_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO]);
		ret = mtk_cfg80211_set_vow_min_ratio(pAd, param->group, param->ratio);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO]) {
		struct vow_ratio_param *param;

		param = (struct vow_ratio_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO]);
		ret = mtk_cfg80211_set_vow_max_ratio(pAd, param->group, param->ratio);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO]) {
		struct vow_info *param;

		param = (struct vow_info *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO]);
		ret = mtk_cfg80211_reply_vow_info(pAd, wiphy, param->group);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_vow_info failed.\n");
	}
	return 0;
}
#endif

#ifdef MWDS
static const struct
nla_policy SUBCMD_SET_MWDS_POLICY[];

int mtk_cfg80211_set_mwds(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx, apcli_idx;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_MWDS_ATTR_MAX + 1];
	int ret;
	BOOLEAN isAp;
	UCHAR if_idx;

	MAC80211_PAD_GET(pAd, wiphy);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	apcli_idx = CFG80211_FindApcliIdxByNetDevice(pAd, pNetDev);
	if (apidx != WDEV_NOT_FOUND)
		isAp = TRUE;
	else if (apcli_idx != WDEV_NOT_FOUND)
		isAp = FALSE;
	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev if_idx\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, 2, (struct nlattr *)data, len, SUBCMD_SET_MWDS_POLICY, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE]) {
		char para = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE]);

		if (isAp)
			if_idx = apidx;
		else
			if_idx = apcli_idx;
		mtk_cfg80211_set_mwds_cap(pAd, if_idx, isAp, para);
	} else if (tb[MTK_NL80211_VENDOR_ATTR_MWDS_INFO]) {   /*for MWDS autotest*/
		char para = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MWDS_INFO]);
		INT i;
		PMAC_TABLE_ENTRY pEntry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = entry_get(pAd, i);

			if (IS_ENTRY_A4(pEntry)) {
				if (para == 0) {
					pEntry->MWDSInfo.Addr4PktNum = 0;
					pEntry->MWDSInfo.Addr3PktNum = 0;
					pEntry->MWDSInfo.NullPktNum = 0;
					pEntry->MWDSInfo.bcPktNum = 0;
				} else
					MTWF_PRINT(
				"MAC: "MACSTR" A4: %d, A3: %d, NULL: %d, bc/mc: %d, total: %d\n",
				MAC2STR(pEntry->Addr), pEntry->MWDSInfo.Addr4PktNum,
				pEntry->MWDSInfo.Addr3PktNum,
				pEntry->MWDSInfo.NullPktNum, pEntry->MWDSInfo.bcPktNum,
				(pEntry->MWDSInfo.NullPktNum + pEntry->MWDSInfo.bcPktNum));
			}
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			   "invalid mwds cmd\n");
	return 0;
}
#endif

static const struct
nla_policy SUBCMD_SET_AP_BSS_POLICY[];

int mtk_cfg80211_bss_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_BSS_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	u32 ifIndex = 0;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj)
		return -EINVAL;
	ifIndex = pObj->ioctl_if;

	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_BSS_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_AP_BSS_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE]) {
		unsigned char phymode = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE]);

		ret = mtk_cfg80211_set_wireless_mode(pAd, ifIndex, phymode);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO]) {
		ULONG value = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO]);

		ret = mtk_cfg80211_Set_HtTxStream_Proc(pAd, value);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO]) {
		CHAR rssi = nla_get_s8(tb[MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO]);

		ret = mtk_cfg80211_Set_AP_ASSOC_REQ_RSSI_THRESHOLD(pAd, rssi);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY]);

		ret = mtk_cfg80211_set_mpdu_density(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN]);
		if (value == 0xf)
			return mtk_cfg80211_get_amsdu_status(pAd, ifIndex, pObj->ioctl_if_type, wiphy);

		ret = mtk_cfg80211_set_amsdu_en(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}

#ifdef DOT11N_DRAFT3
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_CNT_THR]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_CNT_THR]);

		ret = mtk_cfg80211_set_coex_cnt_thr(pAd, value);

		if (ret < 0)
			return ret;
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA]);

		ret = mtk_cfg80211_set_ht_extcha(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT]);

		ret = mtk_cfg80211_set_ht_protect(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}

#ifdef DOT11_VHT_AC
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT]);

		ret = mtk_cfg80211_set_disallow_vht(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}
#endif

#ifdef MT_MAC
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND]);

		ret = mtk_cfg80211_set_etxbf_en_cond(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}
#endif

#ifdef DOT11W_PMF_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256]);

		ret = mtk_cfg80211_set_pmf_sha256(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BCN_INT]) {
		unsigned short bcn_int = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AP_BCN_INT]);

		ret = mtk_cfg80211_set_beacon_period(pAd, ifIndex, pObj->ioctl_if_type, bcn_int);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT]) {
		unsigned char dtim_int = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT]);

		ret = mtk_cfg80211_set_dtim_period(pAd, ifIndex, pObj->ioctl_if_type, dtim_int);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID]) {
		unsigned char hide_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID]);

		ret = mtk_cfg80211_set_hide_ssid(pAd, ifIndex, pObj->ioctl_if_type, hide_en);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE]) {
		unsigned char ht_op_mode = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE]);

		ret = mtk_cfg80211_set_ht_op_mode(pAd, ifIndex, pObj->ioctl_if_type, ht_op_mode);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE]) {
		unsigned int idle_period = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE]);

		ret = mtk_cfg80211_set_bss_max_idle_period_proc(pAd, idle_period);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN]) {
		UCHAR enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN]);

		ret = mtk_cfg80211_Set_MuOfdma_Dl_Enable(pAd, (UINT_8)enable);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN]) {
		UCHAR enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN]);

		ret = mtk_cfg80211_Set_MuOfdma_Ul_Enable(pAd, (UINT_8)enable);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN]) {
		UCHAR enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN]);

		ret = mtk_cfg80211_Set_MuMimo_Dl_Enable(pAd, (UINT_8)enable);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN]) {
		UCHAR enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN]);

		ret = mtk_cfg80211_Set_MuMimo_Ul_Enable(pAd, (UINT_8)enable);
		if (ret < 0)
			return ret;
	}

#ifdef DOT11_VHT_AC
	if (tb[MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING]) {
		UCHAR value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING]);

		ret = mtk_cfg80211_set_vht_bw_signaling(pAd, ifIndex, pObj->ioctl_if_type, value);
		if (ret < 0)
			return ret;
	}
#endif

#ifdef WAPP_SUPPORT
#ifdef DOT11_HE_AX
	if (tb[MTK_NL80211_VENDOR_ATTR_SET_AP_BSS_COLOR]) {
		struct bss_color *ap_bss_color;
		UINT8 wdev_idx, action, value;

		ap_bss_color = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_AP_BSS_COLOR]);

		wdev_idx = ap_bss_color->wdev_id;
		action = ap_bss_color->action;
		value = ap_bss_color->bss_color_val;

		if (wdev_idx >= WDEV_NUM_MAX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
					"wdev_index is out of range\n");
			return -EINVAL;
		}
		if (action > BSS_COLOR_DBG_MAX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
					"action is out of range\n");
			return -EINVAL;
		}

		if (value < BSS_COLOR_VALUE_MIN || value > BSS_COLOR_VALUE_MAX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
					"bss color value is out of range\n");
			return -EINVAL;
		}

		set_bss_color_dbg(pAd, (UINT8)wdev_idx, (UINT8)action, (UINT8)value);
	}
#endif /* DOT11_HE_AX */
#endif /* WAPP_SUPPORT */


#ifdef DOT11_HE_AX
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY]) {
		unsigned char enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY]);

		ret = mtk_cfg80211_set_11axonly(pAd, ifIndex, pObj->ioctl_if_type, enable);

		if (ret < 0)
			return ret;
	}
#endif /*DOT11_HE_AX*/

#ifdef DOT11_EHT_BE
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY]) {
		unsigned char enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY]);

		ret = mtk_cfg80211_set_11beonly(pAd, ifIndex, pObj->ioctl_if_type, enable);

		if (ret < 0)
			return ret;
	}
#endif /* DOT11_EHT_BE */

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE]) {
		unsigned char enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE]);

		ret = mtk_cfg80211_set_no_agmode(pAd, ifIndex, pObj->ioctl_if_type, enable);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE]) {
		unsigned char enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE]);

		ret = mtk_cfg80211_set_no_nmode(pAd, ifIndex, pObj->ioctl_if_type, enable);

		if (ret < 0)
			return ret;
	}

#ifdef CONFIG_6G_AFC_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_SET_AFC_PARAM]) {
		struct afc_response *afc_resp;
		UINT32 len = 0;

		afc_resp = (struct afc_response *) nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_AFC_PARAM]);
		len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_AFC_PARAM]);

		return nl80211_afc_daemon_response(pAd, afc_resp, len);
	}
#endif /*CONFIG_6G_AFC_SUPPORT*/

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MGMT_RX]) {
		unsigned char mgmt_rx = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MGMT_RX]);

		ret = mtk_cfg80211_set_mgmt_rx(pAd, ifIndex, pObj->ioctl_if_type, mgmt_rx);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_NO_BCN]) {
		unsigned char no_bcn = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_NO_BCN]);

		ret = mtk_cfg80211_set_no_bcn(pAd, ifIndex, pObj->ioctl_if_type, no_bcn);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_ISOLATION]) {
		unsigned char value = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_ISOLATION]);

		if (value == 0xf)
			return mtk_cfg80211_get_bss_isolation_status(pAd, wiphy);

		ret = mtk_cfg80211_set_bss_isolate_en(pAd, ifIndex, pObj->ioctl_if_type, value);

		if (ret < 0)
			return ret;
	}

	return 0;
}

#ifdef AIR_MONITOR

static const struct
nla_policy SUBCMD_SET_AP_MONITOR_POLICY[];

int mtk_cfg80211_monitor_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_MONITOR_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_MONITOR_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_AP_MONITOR_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE]) {
		UCHAR monitor_enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE]);

		mtk_cfg80211_set_monitor_enable(pAd, monitor_enable);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE]) {
		UCHAR *monitor_rule = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE]);

		mtk_cfg80211_set_monitor_rule(pAd, monitor_rule);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA]) {
		UCHAR *monitor_mac_addr = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA]);

		mtk_cfg80211_set_monitor_target(pAd, monitor_mac_addr);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX]) {
		UCHAR monitor_idx = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX]);

		mtk_cfg80211_set_monitor_index(pAd, monitor_idx);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_CLR])
		mtk_cfg80211_set_monitor_clear(pAd);

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0]) {
		UCHAR *monitor_mac_addr0 = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0]);

		mtk_cfg80211_set_monitor_target(pAd, monitor_mac_addr0);
		mtk_cfg80211_set_monitor_index(pAd, 0);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA_ID]) {
		struct mnt_sta *req_sta;

		req_sta = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA_ID]);

		ret = mtk_cfg80211_set_monitor_sta_index(pAd, req_sta);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"failed - [ERROR]mtk_cfg80211_set_monitor_sta_index fail\n");
			return ret;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_MAX_PKT]) {
		struct mnt_max_pkt *mnt_pkt = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_MAX_PKT]);

		UINT32 max_pkt = mnt_pkt->pkt_number;

		mtk_cfg80211_set_monitor_max_pkt_cnt(pAd, max_pkt);
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_RESULT]) {
		char *cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_RESULT]);
		int cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_RESULT]);

		ret = mtk_cfg80211_get_air_mnt_result(pAd, wiphy, wdev, cmd, cmd_len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"failed - [ERROR]mtk_cfg80211_get_air_mnt_result fail\n");
			return ret;
		}
	}
	return 0;
}
#endif

#ifdef CFG_SUPPORT_FALCON_PP
static int mtk_cfg80211_reply_pp_bitmap(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	int ret;

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
					MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO,
					&pAd->CommonCfg.punctured_bitmap, sizeof(USHORT));

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
}
#endif

static const struct
nla_policy SUBCMD_SET_AP_RADIO_POLICY[];

int mtk_cfg80211_radio_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_RADIO_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_RADIO_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_AP_RADIO_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO]) {
		UCHAR ieee80211 = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO]);

		ret = mtk_cfg80211_set_ieee80211h(pAd, ieee80211);
		if (ret < 0)
			return ret;
	}

#ifdef ACK_CTS_TIMEOUT_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO]) {
		UCHAR ackcts_tout_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO]);

		ret = mtk_cfg80211_set_ackcts_tout_en(pAd, ackcts_tout_en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO]) {
		UINT distance = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO]);

		ret = mtk_cfg80211_set_dst2acktimeout(pAd, distance);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO]) {
		UINT cck_ack_tout = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO]);

		ret = mtk_cfg80211_set_cck_ack_tout(pAd, cck_ack_tout);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO]) {
		UINT ofdm_ack_tout = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO]);

		ret = mtk_cfg80211_set_ofdm_ack_tout(pAd, ofdm_ack_tout);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO]) {
		UINT ofdma_ack_tout = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO]);

		ret = mtk_cfg80211_set_ofdma_ack_tout(pAd, ofdma_ack_tout);
		if (ret < 0)
			return ret;
	}
#endif

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT]) {
		UCHAR csa_2g = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT]);

		ret = mtk_cfg80211_set_csa_2g(pAd, csa_2g);
		if (ret < 0)
			return ret;
	}

#ifdef CFG_SUPPORT_FALCON_PP
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO]) {
		USHORT punctured_bitmap = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO]);

		if (punctured_bitmap == 0xffff)
			ret = mtk_cfg80211_reply_pp_bitmap(pAd, wiphy);
		else
			ret = mtk_cfg80211_set_pp_bitmap(pAd, punctured_bitmap);

		if (ret < 0)
			return ret;
	}
#endif
#ifdef ANDLINK_V4_0
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_STA_NUM]) {
		POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
		INT		apidx = pObj->ioctl_if;
		UINT max_sta_num = nla_get_u32(tb[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_STA_NUM]);

		if (max_sta_num > 0) {
			pAd->ApCfg.MBSSID[apidx].MaxStaNum = max_sta_num;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"IF(rai%d) (MaxStaNum=%d) max=%p\n", apidx,
				pAd->ApCfg.MBSSID[apidx].MaxStaNum, pAd->ApCfg.MBSSID[apidx].MaxStaNum);
			return 0;
		}
	}
#endif/*ANDLINK_V4_0*/
	return 0;
}

static const struct
nla_policy VENDOR_SUBCMD_SET_WMM_POLICY[];

int mtk_cfg80211_wmm_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_WMM_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_WMM_ATTR_MAX, (struct nlattr *)data,
		len, VENDOR_SUBCMD_SET_WMM_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO]) {
		UCHAR aifsn_value[WMM_NUM_OF_AC] = {0};
		int aifsn_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO]);
		UCHAR *aifsn_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO]);

		if (aifsn_len > WMM_NUM_OF_AC)
			aifsn_len = WMM_NUM_OF_AC;

		memcpy(aifsn_value, aifsn_temp, aifsn_len);

		ret = mtk_cfg80211_Set_APAifsn_Proc(pAd, aifsn_value);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO]) {
		UCHAR cwmin_value[WMM_NUM_OF_AC] = {0};
		int cwmin_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO]);
		UCHAR *cwmin_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO]);

		if (cwmin_len > WMM_NUM_OF_AC)
			cwmin_len = WMM_NUM_OF_AC;

		memcpy(cwmin_value, cwmin_temp, cwmin_len);

		ret = mtk_cfg80211_Set_APCwmin_Proc(pAd, cwmin_value);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO]) {
		UCHAR cwmax_value[WMM_NUM_OF_AC] = {0};
		int cwmax_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO]);
		UCHAR *cwmax_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO]);

		if (cwmax_len > WMM_NUM_OF_AC)
			cwmax_len = WMM_NUM_OF_AC;

		memcpy(cwmax_value, cwmax_temp, cwmax_len);

		ret = mtk_cfg80211_Set_APCwmax_Proc(pAd, cwmax_value);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO]) {
		UCHAR txop_value[WMM_NUM_OF_AC] = {0};
		int txop_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO]);
		UCHAR *txop_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO]);

		if (txop_len > WMM_NUM_OF_AC)
			txop_len = WMM_NUM_OF_AC;

		memcpy(txop_value, txop_temp, txop_len);

		ret = mtk_cfg80211_Set_APTxop_Proc(pAd, txop_value);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO]) {
		UCHAR wmm_cap = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO]);

		switch (wmm_cap) {
		case 0:
		case 1:
			ret = mtk_cfg80211_set_wmm_cap(pAd, wmm_cap);
			break;
		case 0xf:
			ret = mtk_cfg80211_get_wmm_cap_status(pAd, wiphy);
			break;
		default:
			return -EINVAL;
		}

		if (ret < 0)
			return ret;
	}

	return 0;
}

#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)

static struct nla_policy mtk_nl80211_vendor_attr_mlo_preset_linkid_policy[MTK_NL80211_VENDOR_MLO_PRESET_LINKID_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_INFO] = { .type = NLA_U8, .len = sizeof(u8) },
};


int mtk_cfg80211_mlo_preset_linkid_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_WMM_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_MLO_PRESET_LINKID_ATTR_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_mlo_preset_linkid_policy, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_INFO]) {
		unsigned char link_id = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_MLO_PRESET_LINKID_INFO]);

		ret = mtk_cfg80211_mlo_preset_link_id(pAd, link_id);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif

#ifdef MBO_SUPPORT
static const struct
nla_policy SUBCMD_SET_MBO_POLICY[];

int mtk_cfg80211_mbo_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_MBO_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_MBO_MAX, (struct nlattr *)data,
			len, SUBCMD_SET_MBO_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC]) {
		UCHAR npc_value[MBO_NPC_LEN] = {0};
		int npc_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC]);
		UCHAR *npc_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC]);

		memcpy(npc_value, npc_temp, npc_len);

		ret = mtk_cfg80211_Set_mbo_npc_Proc(pAd, npc_value);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif

#ifdef DFS_SLAVE_SUPPORT
static const struct
nla_policy SUBCMD_BH_STATUS_POLICY[];

int mtk_cfg80211_bh_status_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	int cmd_len = 0;
	UCHAR *cmd_data = NULL;
	struct BH_STATUS_MSG *pMsg = NULL;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
			"ifname: %s\n", net_dev->name);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS, (struct nlattr *)data,
			len, SUBCMD_BH_STATUS_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS]) {
		cmd_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS]);
		cmd_data = nla_data(tb[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS]);

		pMsg = (struct BH_STATUS_MSG *)cmd_data;

		if (pMsg->fBhStatus)
			slave_bcn_ctrl(pAd, TRUE);
		else
			slave_bcn_ctrl(pAd, FALSE);
	}

	return 0;
}
#endif /* DFS_SLAVE_SUPPORT */


#ifdef VLAN_SUPPORT
static const struct
nla_policy SUBCMD_SET_VLAN_POLICY[];

int mtk_cfg80211_vlan_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_VLAN_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_VLAN_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_VLAN_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO]) {
		UCHAR vlan_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO]);

		ret = mtk_cfg80211_Set_VLANEn_Proc(pAd, vlan_en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO]) {
		UINT16 vlan_id = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO]);

		ret = mtk_cfg80211_Set_VLANID_Proc(pAd, vlan_id);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO]) {
		UCHAR vlan_priority = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO]);

		ret = mtk_cfg80211_Set_VLANPriority_Proc(pAd, vlan_priority);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO]) {
		UCHAR vlan_tag = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO]);

		ret = mtk_cfg80211_Set_VLAN_TAG_Proc(pAd, vlan_tag);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO]) {
		struct vlan_policy_param *param;

		param = (struct vlan_policy_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO]);

		ret = mtk_cfg80211_Set_VLAN_Policy_Proc(pAd, param->direction, param->policy);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif

static const struct
nla_policy SUBCMD_SET_BA_POLICY[];

int mtk_cfg80211_ba_set_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_AP_BA_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_AP_BA_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_BA_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO]) {
		UCHAR ba_decline = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO]);

		if (ba_decline == 0xf)
			return cfg80211_badecline_status(pAd, wiphy);

		ret = mtk_cfg80211_set_ba_decline(pAd, ba_decline);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO]) {
		USHORT ba_wsize = nla_get_u16(tb[MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO]);

		ret = mtk_cfg80211_set_ht_ba_wsize(pAd, ba_wsize);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO]) {
		UCHAR ba_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO]);

		if (ba_en == 0xf)
			return cfg80211_ba_auto_status(pAd, wiphy);

		ret = mtk_cfg80211_set_ht_auto_ba(pAd, ba_en);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO]) {
		struct ba_mactid_param *ba_setup = (struct ba_mactid_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO]);

		ret = mtk_cfg80211_set_ba_setup(pAd, ba_setup->mac_addr, ba_setup->tid);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO]) {
		struct ba_mactid_param *ba_ori_td = (struct ba_mactid_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO]);

		ret = mtk_cfg80211_set_ba_ori_teardown(pAd, ba_ori_td->mac_addr, ba_ori_td->tid);
		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO]) {
		struct ba_mactid_param *ba_rec_td = (struct ba_mactid_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO]);

		ret = mtk_cfg80211_set_ba_rec_teardown(pAd, ba_rec_td->mac_addr, ba_rec_td->tid);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int mtk_cfg80211_get_cap(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	ULONG priv_flags;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MAX + 1];
	int ret;
	struct wifi_dev *preq_wdev = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ifname: %s\n", pNetDev->name);

	priv_flags = RT_DEV_PRIV_FLAGS_GET(pNetDev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, pNetDev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pObj in driver .\n");
		return -EINVAL;
	}

	preq_wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wdev);
	if (!preq_wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver .\n");
		return -EINVAL;
	}

	if (preq_wdev->if_up_down_state == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR] Interface is down .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;

#ifdef WAPP_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_CAC_CAP]) {
		ret = mtk_cfg80211_reply_cac_cap(wiphy, wdev, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_cac_cap failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MISC_CAP]) {
		ret = mtk_cfg80211_reply_misc_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_misc_cap failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HT_CAP]) {
		ret = mtk_cfg80211_reply_ht_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_ht_cap failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_VHT_CAP]) {
		ret = mtk_cfg80211_reply_vht_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_vht_cap failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP]) {
		ret = mtk_cfg80211_reply_he_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_he_cap failed.\n");
	} else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WDEV]) {
		ret = mtk_cfg80211_reply_wdev(wiphy, wdev, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wdev failed.\n");
	}
#ifdef CONFIG_MAP_SUPPORT
	else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_EHT_CAP]) {
		ret = mtk_cfg80211_reply_eht_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_eht_cap failed.\n");
	}
#endif
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
	else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MLO_CAP]) {
		ret = mtk_cfg80211_reply_mlo_cap(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_mlo_cap failed.\n");
	}
#endif
#ifdef DOT11_HE_AX
#ifdef MAP_R3
	else if (tb[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WF6_CAPABILITY]) {
		ret = mtk_cfg80211_reply_wf6_cap(wiphy, wdev, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_wf6_cap failed.\n");

	}
#endif
#endif
	else
		return -EINVAL;
#endif /* WAPP_SUPPORT */

	return 0;
}

#ifdef HOSTAPD_11R_SUPPORT
static int parse_hostapd_ftie(PRTMP_ADAPTER pAd,
				struct wiphy *wiphy,
				struct wireless_dev *wdev,
				struct nlattr *tb)
{
	struct ap_11r_params *ap_ft_params;
	PFT_CFG pFtCfg;
	char *data = nla_data(tb);
	UINT apidx = 0, size_tmp = 0;

	ap_ft_params = (struct ap_11r_params *)data;

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"OwnMac: %02x%02x%02x%02x%02x%02x\n",
			PRINT_MAC(ap_ft_params->own_mac));

	apidx = get_apidx_by_addr(pAd, ap_ft_params->own_mac);
	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"apidx invalid");
		return -1;
	}

	pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;

	if (pFtCfg->FtCapFlag.Dot11rFtEnable) {
		NdisZeroMemory(pFtCfg->FtR0khId, sizeof(pFtCfg->FtR0khId));
		size_tmp = ARRAY_SIZE(pFtCfg->FtR0khId) < ARRAY_SIZE(ap_ft_params->nas_identifier)
					? ARRAY_SIZE(pFtCfg->FtR0khId) : ARRAY_SIZE(ap_ft_params->nas_identifier);
		size_tmp = ap_ft_params->nas_id_len < size_tmp ? ap_ft_params->nas_id_len : size_tmp;
		NdisMoveMemory(pFtCfg->FtR0khId, ap_ft_params->nas_identifier, size_tmp);
		pFtCfg->FtR0khIdLen = size_tmp;

		NdisZeroMemory(pFtCfg->FtR1khId, MAC_ADDR_LEN);
		NdisMoveMemory(pFtCfg->FtR1khId, ap_ft_params->r1_key_holder, MAC_ADDR_LEN);

		pFtCfg->AssocDeadLine = ap_ft_params->reassociation_deadline;

		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
					"R1KH: %02x%02x%02x%02x%02x%02x\n", PRINT_MAC(ap_ft_params->r1_key_holder));
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
						"NAS-ID: %s\n", ap_ft_params->nas_identifier);
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
						"AssocDeadline: %d\n", ap_ft_params->reassociation_deadline);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"11R disabled in driver\n");

	return 0;
}

static struct nla_policy mtk_nl80211_vendor_attr_ft_policy[];

int mtk_cfg80211_ft_ie(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx;
	struct net_device *pNetDev = wdev->netdev;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_FT_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_FT_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_ft_policy, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_FTIE]) {
		ret = parse_hostapd_ftie(pAd, wiphy, wdev, tb[MTK_NL80211_VENDOR_ATTR_SET_FTIE]);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_command failed.\n");
			return -EINVAL;
		}
	}

	return 0;
}
#endif

#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
static int parse_hostapd_mlo_ie(PRTMP_ADAPTER pAd,
				struct wiphy *wiphy,
				struct wireless_dev *wdev,
				struct nlattr *tb,
				INT apidx)
{
	struct wpa_mlo_ie_parse *peer_mlo_ie;
	UINT8 *construct_peer_mlo_ie;
	char *data = nla_data(tb);

	peer_mlo_ie = (struct wpa_mlo_ie_parse *)data;

	construct_peer_mlo_ie = pAd->ApCfg.MBSSID[0].wdev.hostapd_peer_ml_ie;
	construct_peer_mlo_ie[0] = IE_WLAN_EXTENSION;
	construct_peer_mlo_ie[1] = 10;
	construct_peer_mlo_ie[2] = EID_EXT_EHT_MULTI_LINK;
	construct_peer_mlo_ie[3] = 0; /* ml common control */
	construct_peer_mlo_ie[4] = 0;
	construct_peer_mlo_ie[5] = 7; /* len = 7 */
	NdisMoveMemory(&construct_peer_mlo_ie[6], peer_mlo_ie->ml_addr, MAC_ADDR_LEN);

	hex_dump_with_lvl("mlo_ie", (unsigned char *) construct_peer_mlo_ie, 12, DBG_LVL_INFO);
	return 0;
}

static int hostapd_ft_action_req(RTMP_ADAPTER *pAd, struct nlattr *tb, int MsgLen, int apidx)
{
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *pMbss;
	PEID_STRUCT eid_ptr;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *mlo_ie = NULL;
	uint16_t mld_sta_idx = MLD_STA_NONE;
	UINT8 mld_addr[MAC_ADDR_LEN] = {0};
	UINT8 sta_addr[MAC_ADDR_LEN] = {0};
	UINT8 sta_mld_addr[MAC_ADDR_LEN] = {0};
	UCHAR *data = nla_data(tb);
	UINT8 mlo_ie_flag = FALSE;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
#endif /* DOT11R_FT_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_HW, CATMLME_AUTH, DBG_LVL_NOTICE, "FT ACTION - %s, data len %d\n", __func__, MsgLen);
	if (!data) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATMLME_AUTH, DBG_LVL_ERROR, "FT ACTION - %s, data is null\n", __func__);
		return 1;
	}

	NdisCopyMemory(sta_addr, data, MAC_ADDR_LEN);
	NdisCopyMemory(sta_mld_addr, data + 16, MAC_ADDR_LEN);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR, "%s(%d), wdev is null\n", __func__, __LINE__);
		return 1;
	}

#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &(wdev->FtCfg);
	if (pFtCfg)
		pFtCfg->update_sta_mac = FALSE;
#endif /* DOT11R_FT_SUPPORT */

	for (eid_ptr = (PEID_STRUCT) (data + 28);
			eid_ptr->Len + 2 + (UCHAR *) eid_ptr - data < MsgLen;
			eid_ptr = (PEID_STRUCT) ((UCHAR *) eid_ptr + eid_ptr->Len + 2)) {

		if (eid_ptr->Eid == IE_WLAN_EXTENSION &&
				eid_ptr->Len && eid_ptr->Octet[0] == EID_EXT_EHT_MULTI_LINK) {

			mlo_ie = (UCHAR *)eid_ptr;

			if (mlo_ie) {
				mlo_ie_flag = TRUE;
				wdev->do_not_send_sta_del_event_flag = 1;
				mld_sta_idx = eht_build_multi_link_conn_req(wdev,
						SUBTYPE_AUTH, mlo_ie, NULL, NULL, sta_addr, mld_addr, 0);
				wdev->do_not_send_sta_del_event_flag = 0;

				if (mld_sta_idx == MLD_STA_NONE) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_ERROR,
						"Fail: mld conn request (bss:%s, setup addr:%pM)\n",
						RtmpOsGetNetDevName(wdev->if_dev), sta_addr);
					return 1;
				}
#ifdef DOT11R_FT_SUPPORT
				if (pFtCfg) {
					NdisZeroMemory(pFtCfg->sta_mld_addr, MAC_ADDR_LEN);
					COPY_MAC_ADDR(pFtCfg->sta_mld_addr, sta_addr);
				}
#endif /* DOT11R_FT_SUPPORT */
			}

			pEntry = MacTableLookup(pAd, sta_addr);

			if (!pEntry) {
				pEntry = MacTableInsertEntry(
						pAd, sta_addr,
						wdev, ENTRY_CLIENT, OPMODE_AP,
						TRUE, mld_sta_idx, mld_addr);
			}
			break;
		}
	}

	if (!mlo_ie_flag) {
		pEntry = MacTableLookup(pAd, sta_mld_addr);
#ifdef DOT11R_FT_SUPPORT
		if (pFtCfg) {
			NdisZeroMemory(pFtCfg->sta_mld_addr, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pFtCfg->sta_mld_addr, sta_mld_addr);
		}
#endif /* DOT11R_FT_SUPPORT */
		if (!pEntry) {
			pEntry = MacTableInsertEntry(
					pAd, sta_mld_addr,
					wdev, ENTRY_CLIENT, OPMODE_AP,
					TRUE, MLD_STA_NONE, NULL);
		}
	}

	return 0;
}


static int mtk_cfg80211_del_mlo_sta_entry(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx;
	struct net_device *pNetDev = wdev->netdev;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
			"ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX]) {
		ret = hostapd_ft_action_req(pAd, tb[MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY_MAX], len, apidx);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"mtk_cfg80211_command failed.\n");
			return -EINVAL;
		}
	}

	return 0;
}

static struct nla_policy mtk_nl80211_vendor_attr_mlo_policy[];
int mtk_cfg80211_mlo_ie(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	INT apidx;
	struct net_device *pNetDev = wdev->netdev;
	int ret;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_MLO_IE_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_MLO_IE_MAX, (struct nlattr *)data,
		len, mtk_nl80211_vendor_attr_mlo_policy, NULL);
	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_AUTH_MLO_IE]) {
		ret = parse_hostapd_mlo_ie(pAd, wiphy, wdev, tb[MTK_NL80211_VENDOR_ATTR_SET_AUTH_MLO_IE], apidx);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_command failed.\n");
			return -EINVAL;
		}
	}

	return 0;
}
#endif
#ifdef RT_CFG80211_SUPPORT
#endif

static const struct
nla_policy SUBCMD_SET_TXPOWER_POLICY[];

int mtk_cfg80211_tx_power_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_TXPWR_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_TXPWR_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_TXPOWER_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE]) {
		unsigned char pwr_percentage = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE]);

		ret = mtk_cfg80211_set_txpower(pAd, pwr_percentage);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MAX]) {
		unsigned char pwr_max = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MAX]);

		ret = mtk_cfg80211_set_maxtxpwr(pAd, pwr_max);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_INFO]) {
		unsigned char pwr_info = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_INFO]);

		ret = mtk_cfg80211_set_txpwr_info(pAd, pwr_info);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN]) {
		unsigned char pwr_drop_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN]);

		ret = mtk_cfg80211_set_percentage_ctrl(pAd, pwr_drop_en);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL]) {
		unsigned char pwr_drop = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL]);

		ret = mtk_cfg80211_set_pwr_drop_ctrl(pAd, pwr_drop);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE]) {
		unsigned char pwr_dec = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE]);

		ret = mtk_cfg80211_set_decrease_pwrctrl(pAd, pwr_dec);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL]) {
		unsigned char sku_en = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL]);

		ret = mtk_cfg80211_set_sku_ctrl(pAd, sku_en);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_INFO]) {
		ret = mtk_cfg80211_set_sku_info(pAd);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MU]) {
		struct mu_power_param *ppwr;

		ppwr = (struct mu_power_param *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MU]);
		ret = mtk_cfg80211_set_mutxpwr(pAd, ppwr->en, ppwr->value);
		if (ret < 0)
			return ret;
	}
#ifdef MGMT_TXPWR_CTRL
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT]) {
		CHAR *pMgmtPwr;
		int len;

		pMgmtPwr = nla_data(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT]);
		len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT]);

		if (strlen(pMgmtPwr) > len) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"failed - [ERROR]invalid pMgmtPwr length%lu,%d\n", strlen(pMgmtPwr), len);
			return -EINVAL;
		}

		ret = mtk_cfg80211_set_mgmt_frame_power(pAd, pMgmtPwr);
		if (ret < 0)
			return ret;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_TXPWR_GET_MGMT]) {
		ret = mtk_cfg80211_reply_mgmt_real_pwr(wiphy, data, len);
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_mgmt_real_pwr failed.\n");
	}
#endif
	return 0;
}

static const struct
nla_policy SUBCMD_SET_EDCA_POLICY[];

int mtk_cfg80211_edca_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_EDCA_ATTR_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	ret = nla_parse(tb, MTK_NL80211_VENDOR_EDCA_ATTR_MAX, (struct nlattr *)data,
		len, SUBCMD_SET_EDCA_POLICY, NULL);
	if (ret)
		return -EINVAL;
	if (tb[MTK_NL80211_VENDOR_ATTR_TX_BURST]) {
		unsigned char txburst = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_TX_BURST]);

		ret = mtk_cfg80211_set_txburst_proc(pAd, txburst);
		if (ret < 0)
			return ret;
	}
	return 0;
}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
int mtk_cfg80211_vndr_set_vie_handler(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_VIE_ATTR_MAX + 1];
	ULONG *__pPriv;
	struct _RTMP_ADAPTER *pAd;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;
	INT ifIndex = 0;
	u16 vie_len;
	u8 *vie_temp = NULL;
	u16 oui_len;
	u8 *oui_temp = NULL;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);

	if (pObj == NULL)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_VIE_ATTR_MAX, (struct nlattr *)data,
		len, NULL, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_BEACON_VIE]) {

		if ((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"error: only ap ineterface can config beacon vie!\n");
			return -EINVAL;
		}

		vie_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_BEACON_VIE]);
		vie_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_BEACON_VIE]);

		ret = mtk_cfg80211_config_beacon_vie(pAd, ifIndex, vie_temp, vie_len);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_PROBE_RSP_VIE]) {

		if ((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"error: only ap ineterface can config beacon vie!\n");
			return -EINVAL;
		}

		vie_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_PROBE_RSP_VIE]);
		vie_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_PROBE_RSP_VIE]);

		ret = mtk_cfg80211_config_probe_rsp_vie(pAd, ifIndex, vie_temp, vie_len);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_PROBE_REQ_VIE]) {

		if (pObj->ioctl_if_type != INT_APCLI) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"error: only apcli ineterface can config beacon vie!\n");
			return -EINVAL;
		}

		vie_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_PROBE_REQ_VIE]);
		vie_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_PROBE_REQ_VIE]);

		ret = mtk_cfg80211_config_probe_req_vie(pAd, ifIndex, vie_temp, vie_len);

		if (ret < 0)
			return ret;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_OUI_FILTER]) {

		if ((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
				"error: only ap ineterface can config OUI!\n");
			return -EINVAL;
		}

		oui_len = nla_len(tb[MTK_NL80211_VENDOR_ATTR_OUI_FILTER]);
		oui_temp = nla_data(tb[MTK_NL80211_VENDOR_ATTR_OUI_FILTER]);

		ret = mtk_cfg80211_config_oui_filter(pAd, ifIndex, oui_temp, oui_len);

		if (ret < 0)
			return ret;
	}

	return 0;

}
#endif

#ifdef CONFIG_WLAN_SERVICE
static int parse_hqa_cmd_frame(PRTMP_ADAPTER pAd,
				struct wiphy *wiphy,
				struct wireless_dev *wdev,
				struct nlattr *tb, int len)
{
	struct hqa_frame *hqa_frame;
	struct hqa_frame_ctrl local_hqa;
	unsigned int rsp_date_len = 0;
	int ret;
	char *data = nla_data(tb);

	os_alloc_mem_suspend(pAd, (UCHAR **)&hqa_frame, sizeof(*hqa_frame));

	if (!hqa_frame) {
		ret = -ENOMEM;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"hqa_frame fail\n");
		goto err;
	}

	if (sizeof(struct hqa_frame) < len) {
		ret = -EINVAL;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nl data len too long: %d, %lu\n", len, sizeof(struct hqa_frame));
		goto err;
	}

	NdisZeroMemory(hqa_frame, sizeof(*hqa_frame));
	NdisCopyMemory(hqa_frame, data, len);

	/* 0 means eth type hqa frame */
	local_hqa.type = 0;
	local_hqa.hqa_frame_eth = hqa_frame;
	ret = mt_agent_hqa_cmd_handler(&pAd->serv, (struct hqa_frame_ctrl *)&local_hqa);
	if (ret)	{
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"hqa cmd handler fail, err:0x%08x\n", ret);
		ret = -EFAULT;
		goto err;
	}

	/* Update cmd_frame length */
	if (OS_NTOHS((hqa_frame)->length) > SERV_IOCTLBUFF) {
		ret = -EFAULT;
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"hqa fram length too long: %d\n", OS_NTOHS((hqa_frame)->length));
		goto err;
	}

	rsp_date_len = sizeof((hqa_frame)->magic_no) + sizeof((hqa_frame)->type)
				+ sizeof((hqa_frame)->id) + sizeof((hqa_frame)->length)
				+ sizeof((hqa_frame)->sequence) + OS_NTOHS((hqa_frame)->length);

	/* hex_dump("hqa_cmd_frame", (UCHAR *)local_hqa.hqa_frame_eth, rsp_date_len); */

	/* Feedback as soon as we can to avoid QA timeout */
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
			MTK_NL80211_VENDOR_ATTR_HQA,
			local_hqa.hqa_frame_eth, rsp_date_len);

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail\n");

err:
	os_free_mem(hqa_frame);

	return 0;
}

int mtk_cfg80211_hqa_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_HQA_MAX + 1];
	int ret;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
		"ifname:%s, len:%d\n", pNetDev->name, len);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_HQA_MAX, (struct nlattr *)data, len, NULL, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"EINVAL\n");
		return -EINVAL;
	}

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "No intf up in band\n");
		return -ENETDOWN;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_HQA]) {
		ret = parse_hqa_cmd_frame(pAd, wiphy, wdev, tb[MTK_NL80211_VENDOR_ATTR_HQA], nla_len(tb[MTK_NL80211_VENDOR_ATTR_HQA]));
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_wapp_event failed.\n");
	} else
		return -EINVAL;

	return 0;
}
#endif /* CONFIG_WLAN_SERVICE */


static const struct
nla_policy SUBCMD_SET_RADIO_STATS_POLICY[];

int mtk_cfg80211_vndr_set_radio_stats(struct wiphy *wiphy,
			 struct wireless_dev *wdev,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int apidx = 0, ret = 0;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_ATTR_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "ifname: %s\n",
		pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"apidx is invalid\n");
		return -EINVAL;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"apidx is %d.\n", apidx);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_ATTR_MAX,
		(struct nlattr *)data, len, SUBCMD_SET_RADIO_STATS_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_MEASURING_METHOD]) {
		struct wifi_radio_stats_measure *stats_measure;

		stats_measure =
			(struct wifi_radio_stats_measure *)nla_data(tb[MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_MEASURING_METHOD]);
		pAd->radio_measure_rate = stats_measure->radio_stats_measuring_rate;
		pAd->radio_measure_interval = stats_measure->radio_stats_measuring_interval;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_RADIO_SET_MEASURE_ENABEL]) {
		BOOLEAN enable = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_RADIO_SET_MEASURE_ENABEL]);

		pAd->radio_measure_enable = enable;
	}

	return ret;
}


static int mtk_cfg80211_reply_radio_stats(struct wiphy *wiphy, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb = NULL;
	struct wifi_radio_stats radio_stats;
	int ret = 0, payload_len = 0;

	MAC80211_PAD_GET(pAd, wiphy);

	NdisZeroMemory(&radio_stats, sizeof(struct wifi_radio_stats));
	ret = Get_Radio_Statistics(pAd, &radio_stats);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to Get_Radio_Statistics.\n");
		return -EINVAL;
	}

	payload_len = sizeof(struct wifi_radio_stats);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_RADIO_STATS, payload_len, &radio_stats)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	return ret;
}


int mtk_cfg80211_vndr_get_radio_stats(struct wiphy *wiphy,
			 struct wireless_dev *wdev,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int apidx = 0, ret = 0;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "ifname: %s\n",
		pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"apidx is invalid\n");
		return -EINVAL;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"apidx is %d.\n", apidx);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX,
		(struct nlattr *)data, len, NULL, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}
	if (tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS]) {
		ret = mtk_cfg80211_reply_radio_stats(wiphy, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_radio_stats failed. ret=%x\n", ret);
			return -EINVAL;
		}
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RADIO_CPU_TEMPRETURE]) {
		ret = mtk_cfg80211_reply_wifi_tempreture(pAd, wiphy, wdev);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"failed - [ERROR]nl80211_easymesh_sr_enable fail.\n");
			return -EINVAL;
		}
	}
#ifdef ANDLINK_V4_0
	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RADIO_BSS_INFO_ANDLINK_FORMAT]) {
		ret = mtk_cfg80211_reply_andlink_bss_info(wiphy, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"failed - [ERROR]mtk_cfg80211_reply_andlink_bss_info fail.\n");
			return -EINVAL;
		}
	}
#endif/*ANDLINK_V4_0*/

	if (tb[MTK_NL80211_VENDOR_ATTR_GET_RADIO_RSSI]) {
		ret = mtk_cfg80211_reply_radio_rssi(pAd, wiphy, wdev);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_radio_rssi failed. ret=%x\n", ret);
			return -EINVAL;
		}
	}

	return ret;
}

static int mtk_cfg80211_reply_bss_stats(struct wiphy *wiphy,
	int apidx, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb = NULL;
	int ret = 0, payload_len = 0, k = 0;
	struct wifi_bss_stats stats;

	if ((apidx < 0) || (apidx >= MAX_BEACON_NUM))
		return -EINVAL;

	MAC80211_PAD_GET(pAd, wiphy);
	NdisZeroMemory(&stats, sizeof(stats));

	stats.PacketsSent = pAd->ApCfg.MBSSID[apidx].TxCount;
	stats.PacketsReceived = pAd->ApCfg.MBSSID[apidx].RxCount;
	stats.BytesSent = pAd->ApCfg.MBSSID[apidx].TransmittedByteCount;
	stats.BytesReceived = pAd->ApCfg.MBSSID[apidx].ReceivedByteCount;
	stats.ErrorsReceived = pAd->ApCfg.MBSSID[apidx].RxErrorCount;
	stats.DiscardPacketsReceived = pAd->ApCfg.MBSSID[apidx].RxDropCount;
	stats.ErrorsSent = pAd->ApCfg.MBSSID[apidx].TxErrorCount;
	stats.DiscardPacketsSent = pAd->ApCfg.MBSSID[apidx].TxDropCount;
	stats.UnicastPacketsSent = pAd->ApCfg.MBSSID[apidx].ucPktsTx;
	stats.UnicastPacketsReceived = pAd->ApCfg.MBSSID[apidx].ucPktsRx;
	stats.MulticastPacketsSent = pAd->ApCfg.MBSSID[apidx].mcPktsTx;
	stats.MulticastPacketsReceived = pAd->ApCfg.MBSSID[apidx].mcPktsRx;
	stats.BroadcastPacketsSent = pAd->ApCfg.MBSSID[apidx].bcPktsTx;
	stats.BroadcastPacketsReceived = pAd->ApCfg.MBSSID[apidx].bcPktsRx;
	for (k = 0; k < MAX_ACCESS_CAT_NUM; k++) {
		stats.WmmBytesSent[k] = pAd->ApCfg.MBSSID[apidx].TxBytesAdm[k];
		stats.WmmBytesReceived[k] = pAd->ApCfg.MBSSID[apidx].RxBytesAdm[k];
	}

	payload_len = sizeof(struct wifi_bss_stats);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_BSS_STATS, payload_len, &stats)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	return ret;
}



int mtk_cfg80211_vndr_get_bss_stats(struct wiphy *wiphy,
	 struct wireless_dev *wdev, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int apidx = 0, ret = 0;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX + 1];

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "apidx is invalid.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "apidx is %d.\n", apidx);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX,
		(struct nlattr *)data, len, NULL, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS]) {
		ret = mtk_cfg80211_reply_bss_stats(wiphy, apidx, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_ssid_stats failed. ret=%x.\n", ret);
			return -EINVAL;
		}
	}

	return ret;
}
#ifdef TR181_SUPPORT
static int mtk_cfg80211_reply_sta(struct wiphy *wiphy, const UCHAR *staMac)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	int ret = 0;
	struct wifi_station sta;

	MAC80211_PAD_GET(pAd, wiphy);

	NdisZeroMemory(&sta, sizeof(sta));
	NdisCopyMemory(&sta.MacAddr, staMac, MAC_ADDR_LEN);
	Get_Sta_Info(pAd, &sta);


	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(struct wifi_station));
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_STA, sizeof(struct wifi_station), &sta)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	return ret;
}
#endif

extern INT get_station_info_from_mac_entry(struct wifi_dev *wdev, UINT32 ent_type, const UCHAR *mac_addr,
	struct station_information *sta);

static int mtk_cfg80211_get_sta(struct wiphy *wiphy, struct wireless_dev *wld, const UCHAR *staMac)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;
	int ret = 0;
	struct station_information sta;
	struct wifi_dev *wdev;

	MAC80211_PAD_GET(pAd, wiphy);

	NdisZeroMemory(&sta, sizeof(sta));
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wld);
	if (!wdev)
		return -EINVAL;

	if (get_station_info_from_mac_entry(wdev, ENTRY_CLIENT, staMac, &sta))
		return -EINVAL;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(struct station_information));
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_STA_INFO, sizeof(struct station_information), &sta)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	return ret;
}


static const struct
nla_policy SUBCMD_GET_STA_POLICY[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];

int mtk_cfg80211_vndr_get_sta(struct wiphy *wiphy,
							 struct wireless_dev *wdev,
							 const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];
	int ret = 0;
	UCHAR *StaMac = NULL;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"ifname: %s\n", pNetDev->name);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX,
		(struct nlattr *)data, len, SUBCMD_GET_STA_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}
#ifdef TR181_SUPPORT
	if (tb[MTK_NL80211_VENDOR_ATTR_STA]) {
		StaMac = nla_data(tb[MTK_NL80211_VENDOR_ATTR_STA]);
		ret = mtk_cfg80211_reply_sta(wiphy, StaMac);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_sta failed.ret=%x\n", ret);
			return -EINVAL;
		}
	}
#endif
	if (tb[MTK_NL80211_VENDOR_ATTR_STA_MAC]) {
		StaMac = nla_data(tb[MTK_NL80211_VENDOR_ATTR_STA_MAC]);
		ret = mtk_cfg80211_get_sta(wiphy, wdev, StaMac);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_sta failed.ret=%x\n", ret);
			return -EINVAL;
		}
	}

#ifdef ANDLINK_V4_0
	if (tb[MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT]) {
		ret = mtk_cfg80211_reply_andlink_sta_info(wiphy, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_andlink_sta_info failed.\n");
			return -EINVAL;
		}
	}
#endif/*ANDLINK_V4_0*/

	return ret;
}


#ifdef CONVERTER_MODE_SWITCH_SUPPORT
static const struct
nla_policy SUBCMD_V10CONVERTER_POLICY[];

int mtk_cfg80211_vndr_set_v10converter(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct net_device *net_dev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_V10CONVERTER_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	ULONG priv_flags;
	POS_COOKIE pObj;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	net_dev = wdev->netdev;
	priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pObj = cfg80211_vndr_cmd_cookie_prepare(pAd, priv_flags, net_dev);
	if (!pObj) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find pObj in driver .\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_V10CONVERTER_MAX, (struct nlattr *)data,
		len, SUBCMD_V10CONVERTER_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_V10CONVERTER]) {
		char *cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_V10CONVERTER]);
		UCHAR en = *cmd;

		ret = mtk_cfg80211_Set_V10ConverterMode(pAd, en);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/

#ifdef A4_CONN
static const struct
nla_policy SUBCMD_APPROXYREFRESH_POLICY[];

int mtk_cfg80211_vndr_set_apropxyrefresh(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_MAX + 1];
	struct _RTMP_ADAPTER *pAd;
	ULONG *__pPriv;
	int ret;

	__pPriv = (ULONG *)(wiphy_priv(wiphy));
	pAd = (struct _RTMP_ADAPTER *)(*__pPriv);
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"pAd not start up. return!!\n");
		return -EINVAL;
	}

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_MAX, (struct nlattr *)data,
		len, SUBCMD_APPROXYREFRESH_POLICY, NULL);

	if (ret)
		return -EINVAL;

	if (tb[MTK_NL80211_VENDOR_ATTR_SET_APPROXYREFRESH]) {
		char *cmd = nla_data(tb[MTK_NL80211_VENDOR_ATTR_SET_APPROXYREFRESH]);
		UCHAR en = *cmd;

		/* Enable a4 need refresh based on input from application */
		pAd->a4_need_refresh = en;
	}

	return 0;
}
#endif /* A4_CONN */

#ifdef DOT11_EHT_BE
static int mtk_cfg80211_reply_bss_mlo_info(struct wiphy *wiphy,
	int apidx, const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb = NULL;
	int ret = 0, payload_len = 0;
	struct bss_mlo_info ml_info = {0};

	MAC80211_PAD_GET(pAd, wiphy);

	if (apidx < 0 || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"invalid apidx:%d\n", apidx);
		return -EINVAL;
	}

	payload_len = sizeof(struct bss_mlo_info);
	if (pAd->ApCfg.MBSSID[apidx].mld_grp_idx != 0
		&& pAd->ApCfg.MBSSID[apidx].mld_grp_idx != MLD_GROUP_NONE) {
		ml_info.link_cnt = pAd->ApCfg.MBSSID[apidx].wdev.ml_event.event_id;
		ml_info.mld_grp_idx = pAd->ApCfg.MBSSID[apidx].wdev.ml_event.mld_grp_idx;
		ml_info.link_id = pAd->ApCfg.MBSSID[apidx].wdev.ml_event.link_id;
		NdisCopyMemory(ml_info.addr, pAd->ApCfg.MBSSID[apidx].wdev.ml_event.addr, MAC_ADDR_LEN);
	}
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO, payload_len, &ml_info)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "mld_grp_idx=%d, link_id=%d\n",
			ml_info.mld_grp_idx, ml_info.link_id);

	return ret;
}

extern INT get_mac_entry_trx_statistics(PMAC_TABLE_ENTRY pEntry, UCHAR *tx_phymode,
	UCHAR *rx_phymode, UCHAR *tx_bw, UCHAR *rx_bw, UCHAR *tx_nss, UCHAR *rx_nss,
	UCHAR *tx_mcs, UCHAR *rx_mcs, ULONG *tx_rate, ULONG *rx_rate, UCHAR *tx_sgi,
	UCHAR *rx_sgi, UCHAR *snr);

static int mtk_cfg80211_fill_connected_sta_mld(PRTMP_ADAPTER pAd, struct wiphy *wiphy, struct bmgr_mld_sta *mld_sta)
{
	int i, j;
	struct nlattr *tmp_affiliated_sta_attr, *tmp_attr2;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct station_information sta;
	UINT64 DataRate;
	UINT32 rate;
	int rssi = 0, nss = 0;
	struct sk_buff *skb;
	int payload_len = 0;
	PRTMP_ADAPTER ad;
	signed char sta_rssi[4];
	UCHAR tx_sgi, rx_sgi, snr;
#ifdef VOW_SUPPORT
	UINT64 tx_time, rx_time;
	int ac;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#endif

	/*64 * sizeof(struct nlaattr) + 337*/
	payload_len =  64 * sizeof(struct nlattr) + 512;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		goto fail;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP, mld_sta->mld_grp_idx)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC, MAC_ADDR_LEN, mld_sta->mld_addr)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	if (GET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLSR_SUP(mld_sta->eml_caps)) {
		if (nla_put_flag(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLSR)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
	}

	if (GET_DOT11BE_ML_BASIC_CMM_EML_CAP_EMLMR_SUP(mld_sta->eml_caps)) {
		if (nla_put_flag(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLMR)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
	}

	if (GET_DOT11BE_ML_BASIC_CMM_MLD_CAP_FREQ_SEP_STR(mld_sta->mld_caps)) {
		if (nla_put_flag(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_STR)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
	}
	pEntry = (MAC_TABLE_ENTRY *)mld_sta->mld_link[0].priv_ptr;
	if (pEntry && pEntry->wdev) {
		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME, pEntry->StaConnectTime)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
	}

	tmp_affiliated_sta_attr = nla_nest_start(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA);
	if (!tmp_affiliated_sta_attr) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG,
			DBG_LVL_ERROR, "nla_put_nest fail.\n");
		goto fail;
	}
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		if (!mld_sta->mld_link[i].active)
			continue;

		pEntry = (MAC_TABLE_ENTRY *)mld_sta->mld_link[i].priv_ptr;

		if (!pEntry || !pEntry->wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"pEntry NULL. i = %d link_addr "MACSTR"\n", i,
				MAC2STR(mld_sta->mld_link[i].link_addr));
			continue;
		}
		if (get_mac_entry_trx_statistics(pEntry, &(sta.tx_phymode), &(sta.rx_phymode), &(sta.tx_bw),  &(sta.rx_bw),
					&(sta.tx_nss), &(sta.rx_nss), &(sta.tx_mcs), &(sta.rx_mcs), &(sta.tx_rate), &(sta.rx_rate),
					&tx_sgi, &rx_sgi, &snr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"get_mac_entry_trx_statistics fail!! i = %d link_addr "MACSTR"\n",
				i, MAC2STR(mld_sta->mld_link[i].link_addr));
			continue;
		}

		ad = (PRTMP_ADAPTER)(pEntry->pAd);
		if (!ad) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"pAd is NULL!!. i = %d link_addr "MACSTR"\n", i,
				MAC2STR(mld_sta->mld_link[i].link_addr));
			continue;
		}
		for (j = 0; j < 4; j++)
			sta_rssi[j] = -127;
		rtmp_get_rssi(ad, pEntry->wcid, sta_rssi, MCS_NSS_CAP(ad)->max_path[MAX_PATH_RX]);

		tmp_attr2 = nla_nest_start(skb, i);
		if (!tmp_attr2) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG,
				DBG_LVL_ERROR, "nla_put_nest fail.\n");
			goto fail;
		}
		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC,
			MAC_ADDR_LEN, mld_sta->mld_link[i].link_addr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID,
			MAC_ADDR_LEN, pEntry->wdev->if_addr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID,
			pEntry->mlo.link_info.link_id)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL,
			pEntry->mlo.link_info.tid_map_ul)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL,
			pEntry->mlo.link_info.tid_map_dl)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT,
			sizeof(pEntry->TxBytes), &(pEntry->TxBytes))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED,
			sizeof(pEntry->RxBytes), &(pEntry->RxBytes))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED,
			pEntry->RxPackets.u.LowPart)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT,
			pEntry->TxPackets.u.LowPart)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT,
			sizeof(pEntry->TxFailCount), &(pEntry->TxFailCount))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES,
			pEntry->txMsduRetryCnt)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		nss = 0;
		rssi = 0;

		for (j = 0; j < 4; j++) {
			if (sta_rssi[j] != -127) {
				rssi += (int)(sta_rssi[j]);
				nss++;
			}
		}
		if (nss > 0) {
			rssi = rssi/nss;
			if (nla_put_s32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH,
				rssi)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}

		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SNR,
			snr)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_s32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE,
			sta.tx_rate)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_s32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE,
			sta.rx_rate)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		RtmpDrvMaxRateGet(pAd, pEntry->MaxHTPhyMode.field.MODE, pEntry->HTPhyMode.field.ShortGI,
									  pEntry->MaxHTPhyMode.field.BW, (pEntry->MaxHTPhyMode.field.MCS & 0xf),
									  (pEntry->MaxHTPhyMode.field.MCS >> 4) + 1, &DataRate);
		DataRate /= 1000000;
		rate = (unsigned int)DataRate;
		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE,
			rate)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u32(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE,
			rate)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

#ifdef VOW_SUPPORT
		tx_time = 0;
		rx_time = 0;
		if (chip_dbg && chip_dbg->get_sta_airtime) {
			for (ac = 0; ac < WMM_NUM; ac++) {
				tx_time += chip_dbg->get_sta_airtime(pAd, pEntry->wcid, ac, TRUE);
				rx_time += chip_dbg->get_sta_airtime(pAd, pEntry->wcid, ac, FALSE);
			}

			if (nla_put(skb,
				MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT,
				sizeof(tx_time), &tx_time)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}

			if (nla_put(skb,
				MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE,
				sizeof(rx_time), &rx_time)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}
#endif
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_PHY_MODE,
			sta.rx_phymode)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_PHY_MODE,
			sta.tx_phymode)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_BW,
			sta.rx_bw)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_BW,
			sta.tx_bw)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_MCS,
			sta.rx_mcs)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_MCS,
			sta.tx_mcs)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_NSS,
			sta.rx_nss)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_NSS,
			sta.tx_nss)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_UL_SGI,
			rx_sgi)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_DL_SGI,
			tx_sgi)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}
		nla_nest_end(skb, tmp_attr2);
	}
	nla_nest_end(skb, tmp_affiliated_sta_attr);
	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EFAULT;
	}

	return 0;
fail:
	if (skb)
		kfree_skb(skb);
	return -EFAULT;
}

static int mtk_cfg80211_reply_ap_mld(struct wiphy *wiphy,
	struct bmgr_mlo_dev *mld)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb = NULL;
	int ret = 0, payload_len = 0, i = 0;
	struct bmgr_entry *bss;
	struct nlattr *tmp_attr, *affiliated_ap_attr;
	UCHAR emlsr_mr;

	MAC80211_PAD_GET(pAd, wiphy);

	payload_len =  21 * sizeof(struct nlattr) + 64;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload_len);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"fail to allocate reply msg.\n");
		goto fail;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX, mld->mld_grp)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS, MAC_ADDR_LEN, mld->mld_addr)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_TYPE, mld->mld_type)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
		goto fail;
	}

	tmp_attr = nla_nest_start(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS);
	if (!tmp_attr) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_nest_start fail.\n");
		goto fail;
	}
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		if (!BMGR_VALID_BSS_IDX(mld->bss_idx_mld[i]))
			continue;

		bss = GET_BSS_ENTRY_BY_IDX(mld->bss_idx_mld[i]);
		if (!BMGR_VALID_MLO_BSS_ENTRY(bss) || !bss->pwdev)
			continue;

		affiliated_ap_attr = nla_nest_start(skb, i);
		if (!affiliated_ap_attr) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"nla_nest_start fail.\n");
			goto fail;
		}

		if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID, MAC_ADDR_LEN, bss->pwdev->bssid)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID, bss->link_id)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		if (wlan_config_get_eht_dscb_enable(bss->pwdev)) {
			if (nla_put_u16(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN,
				wlan_config_get_eht_dis_subch_bitmap(bss->pwdev))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}

		emlsr_mr = wlan_config_get_emlsr_mr(bss->pwdev);
		if (emlsr_mr) {
			if (nla_put_flag(skb, emlsr_mr == 1 ? MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR : MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"nla_put fail.\n");
				goto fail;
			}
		}

		/*currenttly STR is always enabled*/
		if (nla_put_flag(skb, MTK_NL80211_VENDOR_ATTR_AP_MLD_STR)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "nla_put fail.\n");
			goto fail;
		}

		nla_nest_end(skb, affiliated_ap_attr);
	}

	nla_nest_end(skb, tmp_attr);

	ret = mt_cfg80211_vendor_cmd_reply(skb);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "reply msg failed.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "reply mld mld_grp_idx=%u\n",
			mld->mld_grp);

	return ret;
fail:
	if (skb)
		kfree_skb(skb);
	return -EINVAL;
}

static int mtk_cfg80211_reply_connected_sta_mld(struct wiphy *wiphy,
	struct bmgr_mlo_dev *mld)
{
	PRTMP_ADAPTER pAd;
	int ret = 0, i, j;
	struct query_mld_conn *mld_conn = NULL;
	u32 mem_size;
	struct query_mld_sta *mld_sta_info;
	struct bmgr_mld_sta *mld_sta;

	MAC80211_PAD_GET(pAd, wiphy);

	mem_size = sizeof(struct query_mld_conn);
	mem_size += BMGR_MAX_MLD_STA_CNT * sizeof(struct query_mld_sta);

	os_alloc_mem(NULL, (UCHAR **)&mld_conn, mem_size);
	if (!mld_conn)
		return -ENOMEM;

	NdisZeroMemory(mld_conn, mem_size);

	mld_conn->mld_group_idx = mld->mld_grp;
	ret = bss_mngr_query_mld_conn(mld_conn);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"MLD Conn Query failed: mld_group_idx(%d)\n",
			pAd->ApCfg.link_recom.recom_mld_grp);
		goto fail;
	}

	mld_sta_info = (struct query_mld_sta *)(mld_conn->mld_sta_info);
	for (i = 0; i < mld_conn->mld_sta_num; i++) {
		for (j = 0; BMGR_VALID_MLD_STA(j); j++) {
			mld_sta = GET_MLD_STA_BY_IDX(j);
			if (mld_sta->valid && mld_sta_info->mld_sta_idx == mld_sta->idx) {
				if (mtk_cfg80211_fill_connected_sta_mld(pAd, wiphy, mld_sta)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"failed to reply sta mld(%u).\n", mld_sta_info->mld_sta_idx);
				}
				break;
			}
		}
		mld_sta_info++;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
		"reply mld mld_grp_idx connected sta mlds=%u\n", mld->mld_grp);

fail:
	if (mld_conn)
		os_free_mem(mld_conn);
	return ret;
}

static const struct
nla_policy SUBCMD_GET_BSS_MLO_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX + 1];
struct bmgr_mlo_dev *bss_mngr_con_mld_lookup(u8 *mld_addr);

int mtk_cfg80211_vndr_get_bss_mlo_info(struct wiphy *wiphy,
			 struct wireless_dev *wdev,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int apidx = 0, ret = 0, i;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX + 1];
	UCHAR mld_index = 0, *mld_mac;
	struct bmgr_mlo_dev *mld;
	struct query_mld_basic mld_basic = {0};

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "apidx is invalid.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, CATCFG_DBGLOG, DBG_LVL_INFO, "apidx is %d.\n", apidx);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX,
		(struct nlattr *)data, len, SUBCMD_GET_BSS_MLO_INFO_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO]) {
		ret = mtk_cfg80211_reply_bss_mlo_info(wiphy, apidx, data, len);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_bss_mlo_info failed. ret=%x.\n", ret);
			return -EINVAL;
		}
	}

	/*get ap mld information by mld group index*/
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX])
		mld_index = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX]);

	/*get ap mld information by mld mac*/
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS]) {
		mld_mac = nla_data(tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS]);
		COPY_MAC_ADDR(mld_basic.addr, mld_mac);
		if (bss_mngr_query_mld_basic(&mld_basic) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"MLD Lookup failed: MLD MAC (%pM)\n", mld_mac);
			return -EINVAL;
		}

		mld_index = mld_basic.mld_grp_idx;
	}

	if (mld_index) {
		if (!BMGR_VALID_MLD_GRP_IDX(mld_index)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid mld index=%u\n", mld_index);
			return -EINVAL;
		}
		mld = GET_MLD_BY_GRP_IDX(mld_index);
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid mld\n");
			return -EINVAL;
		}

		ret = mtk_cfg80211_reply_ap_mld(wiphy, mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"mtk_cfg80211_reply_bss_mlo_info failed. ret=%x.\n", ret);
			return -EINVAL;
		}
	}

	/*dump all ap mld information*/
	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_DUMP]) {
		for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
			mld = GET_MLD_BY_GRP_IDX(i);
			if (!BMGR_VALID_MLO_DEV(mld)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"invalid mld\n");
				continue;
			}
			if (mld->mld_type != BMGR_MLD_TYPE_MULTI)
				continue;
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"reply mld[%u] %02x:%02x:%02x:%02x:%02x:%02x\n",
					mld->mld_grp, PRINT_MAC(mld->mld_addr));
			ret = mtk_cfg80211_reply_ap_mld(wiphy, mld);
			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"mtk_cfg80211_reply_bss_mlo_info failed. ret=%x.\n", ret);
				continue;
			}
		}
	}

	return ret;
}

static const struct
nla_policy SUBCMD_GET_CONNECTED_STA_MLD_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1];

int mtk_cfg80211_vndr_get_connected_sta_mld(struct wiphy *wiphy,
			 struct wireless_dev *wdev,	const void *data, int len)
{
	PRTMP_ADAPTER pAd;
	int apidx = 0, ret = 0, i;
	struct net_device *pNetDev = wdev->netdev;
	struct nlattr *tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1];
	UCHAR mld_index;
	struct bmgr_mlo_dev *mld;

	MAC80211_PAD_GET(pAd, wiphy);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO, "ifname: %s\n", pNetDev->name);

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return -EINVAL;
	} else if (apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "apidx is invalid.\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, CATCFG_DBGLOG, DBG_LVL_INFO, "apidx is %d.\n", apidx);

	ret = nla_parse(tb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX,
		(struct nlattr *)data, len, SUBCMD_GET_CONNECTED_STA_MLD_INFO_POLICY, NULL);
	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"nla_parse failed. ret=%x.\n", ret);
		return -EINVAL;
	}

	if (tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP]) {
		mld_index = nla_get_u8(tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP]);
		if (!BMGR_VALID_MLD_GRP_IDX(mld_index)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid mld index=%u\n", mld_index);
			return -EINVAL;
		}
		mld = GET_MLD_BY_GRP_IDX(mld_index);
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid mld\n");
			return -EINVAL;
		}

		ret = mtk_cfg80211_reply_connected_sta_mld(wiphy, mld);
		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"reply_connected_sta_mld failed. ret=%x.\n", ret);
			return -EINVAL;
		}
	} else {
		for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
			mld = GET_MLD_BY_GRP_IDX(i);
			if (!BMGR_VALID_MLO_DEV(mld)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
					"invalid mld\n");
				continue;
			}
			if (mld->mld_type != BMGR_MLD_TYPE_MULTI)
				continue;

			ret = mtk_cfg80211_reply_connected_sta_mld(wiphy, mld);
			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"reply_connected_sta_mld failed. ret=%x.\n", ret);
				continue;
			}
		}
	}

	return ret;
}

/* wrapper function */
int mtk_cfg80211_send_bss_ml_event(struct wifi_dev *wdev, enum CFG80211_ML_EVENT event_id)
{
	mtk_fill_bss_ml_info_event(wdev,
		&wdev->bss_info_argument.mld_info, event_id);
	MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_NOTICE,
		"ml_event ===> event(%d) grp(%d) link(%d) addr(%pM)\n",
		wdev->ml_event.event_id,
		wdev->ml_event.mld_grp_idx,
		wdev->ml_event.link_id,
		wdev->ml_event.addr);
	return mtk_cfg80211_event_bss_ml_info(wdev->if_dev, wdev->ml_event);
}
#endif

#ifdef DFS_SLAVE_SUPPORT
int mtk_nl80211_bhstatus_event(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;

	MAC80211_PAD_GET(pAd, wiphy);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
		"[DFS-SLAVE][%s] len:%d wdev:%p\n", __func__, len, wdev);

	skb = cfg80211_vendor_event_alloc(wiphy, wdev, len, MTK_NL80211_VENDOR_EVENT_BH_STATUS, GFP_KERNEL);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}
#endif /* DFS_SLAVE_SUPPORT */

#ifdef ANDLINK_V4_0
int mtk_nl80211_andlink_wifi_change_event(struct wiphy *wiphy,
						void *data, int len)
{
	PRTMP_ADAPTER pAd;
	struct sk_buff *skb;

	if (!wiphy)
		return -EINVAL;
	MAC80211_PAD_GET(pAd, wiphy);
	if (pAd == NULL)
		return -EINVAL;
	skb = cfg80211_vendor_event_alloc(wiphy, NULL, len, MTK_NL80211_VENDOR_EVENT_ANDLINK, GFP_KERNEL);

	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"cfg80211_vendor_event_alloc fail!!\n");
		return -EINVAL;
	}

	if (nla_put(skb, MTK_NL80211_VENDOR_ANDLINK_WIFI_CHANGE_EVENT, len, data)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"nla_put fail!!\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;
}

#endif /*ANDLINK_V4_0*/

int mtk_cfg80211_reply_wifi_tempreture(struct _RTMP_ADAPTER *pAd,
			struct wiphy *wiphy, struct wireless_dev *wl_dev)
{
	int ret;
	UCHAR band = 0;
	struct wifi_dev *wdev;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	band = HcGetBandByWdev(wdev);

	RTMP_GET_TEMPERATURE(pAd, band, &pAd->temperature);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				 "CurrentTemperature = %d\n", pAd->temperature);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_RADIO_CPU_TEMPRETURE,
				&pAd->temperature, sizeof(INT));
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

int mtk_cfg80211_reply_radio_rssi(struct _RTMP_ADAPTER *pAd,
			struct wiphy *wiphy, struct wireless_dev *wl_dev)
{
	int ret;
	UCHAR band = 0;
	struct wifi_dev *wdev;
	CHAR rssi[MAX_RSSI_LEN] = {0};
	UINT8 len = MAX_RSSI_LEN;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	band = HcGetBandByWdev(wdev);

	PhyStatGetRssi(pAd, band, rssi, &len);

	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy,
				MTK_NL80211_VENDOR_ATTR_GET_RADIO_RSSI,
				rssi, MAX_RSSI_LEN);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");
	return ret;
}

#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
static const struct ieee80211_iface_limit ra_p2p_sta_go_limits[] = {
	{
		.max = 16,
		.types = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP),
	},
};

#define MTK_OUI 0x000ce7

#ifdef SUPP_SAE_SUPPORT
#define MTK_OUI_SAE 0x000c43
#endif

#define MTK_MAX_VENDOR_MSG_LEN 4095

static const struct
nla_policy common_policy[2] = {
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	[1] = { .type = NLA_MIN_LEN, .len = 0 },
#else
	[1] = NLA_POLICY_MIN_LEN(0),
#endif
};

static const struct
nla_policy SUBCMD_TEST_POLICY[MTK_NL80211_VENDOR_ATTR_TEST_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_TEST] = { .type = NLA_U32 },
};

static const struct
nla_policy SUBCMD_GET_CAP_POLICY[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MAX + 1] = {
#ifdef WAPP_SUPPORT
#ifdef DFS_CAC_R2
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_CAC_CAP] = { .type = NLA_BINARY,
		.len = sizeof(struct cac_capability_lib)},
#endif /* DFS_CAC_R2 */
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MISC_CAP] = { .type = NLA_BINARY,
		.len = sizeof(wdev_misc_cap)},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HT_CAP] = { .type = NLA_BINARY,
		.len = sizeof(wdev_ht_cap)},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_VHT_CAP] = { .type = NLA_BINARY,
		.len = sizeof(wdev_vht_cap)},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_HE_CAP] = { .type = NLA_BINARY,
		.len = sizeof(wdev_he_cap)},
#endif /* WAPP_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_EHT_CAP] = { .type = NLA_BINARY,
		.len = sizeof(struct wdev_eht_cap)},
#endif
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MLO_CAP] = { .type = NLA_BINARY,
		.len = sizeof(struct wdev_mlo_caps)},
#endif
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WF6_CAPABILITY] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_11H_CAPABILITY] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_RRM_CAPABILITY] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_QUERY_KVR_CAPABILITY] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_WDEV] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};

static const struct
nla_policy SUBCMD_GET_RUNTIME_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_STA] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHAN_LIST] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_OP_CLASS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSS_INFO] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_NOP_CHANNEL_LIST] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WAPP_WSC_PROFILES] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_PMK_BY_PEER_MAC] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_AUTHENTICATION_MODE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_MAC_ADDRESS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_802_11_CURRENTCHANNEL] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_ACS_REFRESH_PERIOD] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANNEL_LAST_CHANGE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MAX_NUM_OF_SSIDS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_EXTENSION_CHANNEL] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_TRANSMITPOWER] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_80211H] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_MLO_MLD_INFO] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BASIC_RATE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SUPP_RATE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};

static const struct
nla_policy SUBCMD_GET_STATISTIC_POLICY[MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_GET_802_11_STATISTICS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_TX_PWR] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_AP_METRICS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_802_11_PER_BSS_STATISTICS] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_CPU_TEMPERATURE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_UPLINK_STATISTICS_ANDLINK_FORMAT] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};

static const struct
nla_policy SUBCMD_GET_STATIC_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_CHIP_ID] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_DRIVER_VER] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_COEXISTENCE] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_VER] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WAPP_SUPPORT_VER] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
	[MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_WIFI_AFC_DATA] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};
#ifdef RT_CFG80211_SUPPORT
#endif

#ifdef WAPP_SUPPORT
static const struct
nla_policy SUBCMD_WAPP_REQ_POLICY[MTK_NL80211_VENDOR_ATTR_WAPP_REQ_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_WAPP_REQ] = NLA_POLICY_EXACT_LEN(sizeof(struct wapp_req)),
};
#endif /* WAPP_SUPPORT */

static const struct
nla_policy SUBCMD_SET_MULTICAST_SNOOPING_POLICY[MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_CFGPERBANDSIZE] = { .type = NLA_STRING },
};

#ifdef FTM_SUPPORT
struct
nla_policy SUBCMD_FTM_POLICY[MTK_NL80211_VENDOR_ATTR_FTM_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_ENABLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_ASAP] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_NUM] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_FTM_DEBUG] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_FTM_ROLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE] = { .type = NLA_U8 },
};

struct
nla_policy SUBCMD_FTM_STAT_POLICY[MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR] = { .type = NLA_U8 },
};
#endif /* FTM_SUPPORT */

static const struct
nla_policy SUBCMD_AP_WIRELESS_POLICY[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD] = { .type = NLA_U16 },
};

static const struct
nla_policy SUBCMD_AP_RFEATURE_POLICY[MTK_NL80211_VENDOR_AP_RFEATURE_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_COUNTRY_POLICY[MTK_NL80211_VENDOR_ATTR_COUNTRY_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME] = { .type = NLA_STRING },
};

static const struct
nla_policy SUBCMD_SET_CHANNEL_POLICY[MTK_NL80211_VENDOR_ATTR_CHAN_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_ACL_POLICY[MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_ACL_POLICY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC] = { .type = NLA_BINARY,
		.len = MAX_NUM_OF_ACL_LIST * MAC_ADDR_LEN},
	[MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC] = { .type = NLA_BINARY,
		.len = MAX_NUM_OF_ACL_LIST * MAC_ADDR_LEN},
	[MTK_NL80211_VENDOR_ATTR_ACL_SHOW_ALL] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_ACL_CLEAR_ALL] = { .type = NLA_FLAG },
};

static const struct
nla_policy SUBCMD_VENDOR_SET_POLICY[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR] = { .type = NLA_STRING },
};

static const struct
nla_policy SUBCMD_VENDOR_SHOW_POLICY[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR] = { .type = NLA_STRING },
};

static const struct
nla_policy VENDOR_SUBCMD_MAC_POLICY[MTK_NL80211_VENDOR_ATTR_MAC_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM] = { .type = NLA_BINARY,
		.len = sizeof(struct mac_param) },
	[MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM] = { .type = NLA_BINARY,
		.len = sizeof(struct mac_param) },
	[MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR] = { .type = NLA_STRING },
};

static const struct
nla_policy SUBCMD_SET_AP_VOW_POLICY[MTK_NL80211_VENDOR_AP_VOW_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_group_en_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_group_en_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_rate_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_rate_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_ratio_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_ratio_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_VOW_GET_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vow_info)),
};

static const struct
nla_policy SUBCMD_SET_MWDS_POLICY[MTK_NL80211_VENDOR_MWDS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MWDS_INFO] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_AUTO_CH_SEL_POLICY[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST] =
		NLA_POLICY_EXACT_LEN(sizeof(struct ch_list_info)),
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST] =
		NLA_POLICY_EXACT_LEN(sizeof(struct ch_list_info)),
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR] = { .type = NLA_U16 },
};

static const struct
nla_policy SUBCMD_SET_SCAN_POLICY[MTK_NL80211_VENDOR_ATTR_SCAN_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SCAN_TYPE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_SCAN_SSID] = { .type = NLA_STRING, .len = SSID_LEN},
	[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_INTERVAL] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT] = { .type = NLA_REJECT },
	[MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT_ANDLINK_FORMAT] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};

static const struct
nla_policy SUBCMD_SET_AP_BSS_POLICY[MTK_NL80211_VENDOR_AP_BSS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO] = { .type = NLA_S8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_CNT_THR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_BCN_INT] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING] = { .type = NLA_U8 },
#ifdef WAPP_SUPPORT
	[MTK_NL80211_VENDOR_ATTR_SET_AP_BSS_COLOR] =
		NLA_POLICY_EXACT_LEN(sizeof(struct bss_color)),
#endif /* WAPP_SUPPORT */
	[MTK_NL80211_VENDOR_ATTR_SET_AFC_PARAM] = { .type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN },
	[MTK_NL80211_VENDOR_ATTR_AP_MGMT_RX] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_NO_BCN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_ISOLATION] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_AP_RADIO_POLICY[MTK_NL80211_VENDOR_AP_RADIO_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MRU_INFO] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_STA_NUM] = { .type = NLA_U32 },
};

static const struct
nla_policy VENDOR_SUBCMD_SET_WMM_POLICY[MTK_NL80211_VENDOR_WMM_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO] = { .type = NLA_BINARY, .len = WMM_NUM_OF_AC },
	[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO] = { .type = NLA_BINARY, .len = WMM_NUM_OF_AC },
	[MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO] = { .type = NLA_BINARY, .len = WMM_NUM_OF_AC },
	[MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO] = { .type = NLA_BINARY, .len = WMM_NUM_OF_AC },
	[MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_AP_MONITOR_POLICY[MTK_NL80211_VENDOR_AP_MONITOR_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE] = NLA_POLICY_EXACT_LEN(3),
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA] = NLA_POLICY_ETH_ADDR,
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_CLR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0] = NLA_POLICY_ETH_ADDR,
#if defined(WAPP_SUPPORT) && defined(AIR_MONITOR)
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA_ID] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mnt_sta)),
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_MAX_PKT] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mnt_max_pkt)),
	[MTK_NL80211_VENDOR_ATTR_AP_MONITOR_GET_RESULT] = { .type = NLA_BINARY,
		.len = (MAX_NUM_OF_MONITOR_STA * sizeof(wapp_mnt_info)) },
#endif /* WAPP_SUPPORT && AIR_MONITOR */
};

static const struct
nla_policy SUBCMD_SET_BA_POLICY[MTK_NL80211_VENDOR_AP_BA_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO] = { .type = NLA_U16},
	[MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct ba_mactid_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct ba_mactid_param)),
	[MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct ba_mactid_param)),
};

static const struct
nla_policy SUBCMD_SET_ACTION_POLICY[MTK_NL80211_VENDOR_ATTR_SET_ACTION_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_OFFCHAN_ACTION_FRAME] = { .type = NLA_BINARY,
			.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_QOS_ACTION_FRAME] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_COSR_ACTION_FRAME] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_STATIC_PUNCTURE_BITMAP] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SEND_T2LM_REQUEST] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_SET_ACTION_SET_T2LM_MAPPING] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
};

static const struct
nla_policy VENDOR_SUBCMD_QOS_POLICY[MTK_NL80211_VENDOR_ATTR_QOS_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_QOS_UP_TUPLE_EXPIRED_NOTIFY] =
#ifdef MSCS_PROPRIETARY
		NLA_POLICY_EXACT_LEN(sizeof(struct wapp_vend_spec_classifier_para_report)),
#else
		{ .type = NLA_FLAG },
#endif
	[MTK_NL80211_VENDOR_ATTR_GET_PMK_BY_PEER_MAC] = NLA_POLICY_ETH_ADDR,
#ifdef QOS_R3
	[MTK_NL80211_VENDOR_ATTR_QOS_CHARATERISTICS_IE] =
		NLA_POLICY_EXACT_LEN(sizeof(struct qos_characteristics)),
#endif
#ifdef EPCS_SUPPORT
	[MTK_NL80211_VENDOR_ATTR_EPCS_STATE_SYNC] =
		NLA_POLICY_EXACT_LEN(sizeof(struct epcs_sta)),
#endif
};

#ifdef OFFCHANNEL_SCAN_FEATURE
static const struct
nla_policy SUBCMD_OFFCHANNEL_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(OFFCHANNEL_SCAN_MSG)),
};
#endif /* OFFCHANNEL_SCAN_FEATURE */

static const struct
nla_policy SUBCMD_DFS_POLICY[MTK_NL80211_VENDOR_ATTR_DFS_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_DFS_SET_ZERO_WAIT] =
		NLA_POLICY_EXACT_LEN(sizeof(union dfs_zero_wait_msg)),
	[MTK_NL80211_VENDOR_ATTR_DFS_GET_ZERO_WAIT] = { .type = NLA_BINARY,
		.len = sizeof(union dfs_zero_wait_msg) },
	[MTK_NL80211_VENDOR_ATTR_DFS_CAC_STOP] = { .type = NLA_FLAG },
};

#ifdef CONFIG_MAP_SUPPORT
static const struct
nla_policy SUBCMD_EASYMESH_POLICY[MTK_NL80211_VENDOR_ATTR_EASYMESH_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_SSID] = { .type = NLA_STRING },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PSK] = { .type = NLA_BINARY, .len = 64 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WIRELESS_MODE] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_CHANNEL] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBW] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VHTBW] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_EHTBW] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HIDDEN_SSID] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTH_MODE] = { .type = NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_ENCRYP_TYPE] = { .type = NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DEFAULT_KEY_ID] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_KEY1] = { .type =  NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_WPA_PSK] = { .type = NLA_BINARY, .len = 64 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_RADIO_ON] = { .type =  NLA_U32},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DISCONNECT_STA] = NLA_POLICY_ETH_ADDR,
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_VID] = { .type =  NLA_U16},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_PRIMARY_PCP] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_BH_VID] = { .type =  NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TS_FH_VID] = { .type =  NLA_U16},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_TRANSPARENT_VID] = { .type =  NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BCN_REQ] = { .type =  NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_HTBSSCOEX] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMFMFPC] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_AUTO_ROAMING] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DSCP_POLICY_ENABLE] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_QOS_MAP_CAPA] = { .type =  NLA_U8},
#ifdef DPP_SUPPORT
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_PMK] =
		NLA_POLICY_EXACT_LEN(sizeof(struct pmk_req)),
#endif
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_WAPP_IE] = { .type = NLA_BINARY, .len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_CANCEL_ROC] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_DEL_CCE_IE] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_DPP_URI] = { .type = NLA_BINARY,
		.len = MAX_MGMT_PKT_LEN },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_1905_SEC_ENABLED] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_R3_ONBOARDING_TYPE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS] = { .type = NLA_S8 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS] = { .type = NLA_S8 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_START_ROC] =
		NLA_POLICY_EXACT_LEN(sizeof(struct roc_req)),
	[MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ] =
		NLA_POLICY_EXACT_LEN(sizeof(struct wapp_mesh_sr_info)),
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_CH] =
		NLA_POLICY_EXACT_LEN(sizeof(struct map_ch)),
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MAP_ENABLE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_DPP_STAMAC] = NLA_POLICY_ETH_ADDR,
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_ASSOC_CTRL] =
		NLA_POLICY_EXACT_LEN(sizeof(struct bh_assoc_disallow_info)),
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_CREATE] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mld_group)),
#ifdef MAP_R6
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_RECONF] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mld_reconf)),
#endif /* MAP_R6 */
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_DESTROY] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_ADD] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mld_add_link)),
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_DEL] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_MLD_LINK_TRANSFER] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SR_ENABLE] = { .type =  NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BH_SR_BITMAP] =
		NLA_POLICY_EXACT_LEN(sizeof(struct wapp_srg_bitmap)),
#ifdef MAP_VENDOR_SUPPORT
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_NOP_STATE] = { .type = NLA_BINARY,
		.len = sizeof(struct cont_nop_info) },
	[MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_VENDOR_CH_PREF_STATE] = { .type = NLA_BINARY,
		.len = sizeof(struct cont_ch_info) },
#endif /* MAP_VENDOR_SUPPORT */
};
#endif /* CONFIG_MAP_SUPPORT */

static const struct
nla_policy SUBCMD_SET_VLAN_POLICY[MTK_NL80211_VENDOR_VLAN_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO] =
		NLA_POLICY_EXACT_LEN(sizeof(struct vlan_policy_param)),
};

static const struct
nla_policy SUBCMD_SET_MBO_POLICY[MTK_NL80211_VENDOR_ATTR_MBO_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC] = NLA_POLICY_EXACT_LEN(5),
};

static const struct
nla_policy SUBCMD_SET_TXPOWER_POLICY[MTK_NL80211_VENDOR_TXPWR_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_MAX] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_INFO] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_INFO] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_TXPWR_MU] =
		NLA_POLICY_EXACT_LEN(sizeof(struct mu_power_param)),
	[MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT] = { .type = NLA_STRING},
	[MTK_NL80211_VENDOR_ATTR_TXPWR_GET_MGMT] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_SET_EDCA_POLICY[MTK_NL80211_VENDOR_EDCA_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_TX_BURST] = { .type = NLA_U8 },
};

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
static const struct
nla_policy SUBCMD_SET_VENDOR_IE_POLICY[MTK_NL80211_VENDOR_ATTR_VIE_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_BEACON_VIE] = { .type = NLA_BINARY },
	[MTK_NL80211_VENDOR_ATTR_PROBE_RSP_VIE] = { .type = NLA_BINARY },
	[MTK_NL80211_VENDOR_ATTR_PROBE_REQ_VIE] = { .type = NLA_BINARY },
	[MTK_NL80211_VENDOR_ATTR_OUI_FILTER] = { .type = NLA_BINARY },
};
#endif

static const struct
nla_policy SUBCMD_SET_RADIO_STATS_POLICY[MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_MEASURING_METHOD] =
		NLA_POLICY_EXACT_LEN(sizeof(struct wifi_radio_stats_measure)),
	[MTK_NL80211_VENDOR_ATTR_RADIO_SET_MEASURE_ENABEL] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_GET_RADIO_STATS_POLICY[MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_RADIO_STATS] = { .type = NLA_BINARY,
		.len = sizeof(struct wifi_radio_stats)},
	[MTK_NL80211_VENDOR_ATTR_GET_RADIO_CPU_TEMPRETURE] = {	.type = NLA_U32},
	[MTK_NL80211_VENDOR_ATTR_GET_RADIO_BSS_INFO_ANDLINK_FORMAT] = {.type = NLA_BINARY,
		.len = sizeof(struct GNU_PACKED mtk_andlink_radio_info)},
	[MTK_NL80211_VENDOR_ATTR_GET_RADIO_RSSI] = {	.type = NLA_BINARY, .len = MAX_RSSI_LEN},
};

static const struct
nla_policy SUBCMD_GET_SSID_STATS_POLICY[MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_BSS_STATS] = { .type = NLA_BINARY,
		.len = sizeof(struct wifi_bss_stats)},
};

static const struct
nla_policy SUBCMD_GET_STA_POLICY[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_STA] = NLA_POLICY_ETH_ADDR,
	[MTK_NL80211_VENDOR_ATTR_STA_MAC] = NLA_POLICY_ETH_ADDR,
	[MTK_NL80211_VENDOR_ATTR_STA_INFO_ANDLINK_FORMAT] = {.type = NLA_BINARY,
		.len = MTK_MAX_VENDOR_MSG_LEN},
};

#ifdef CONFIG_WLAN_SERVICE
static const struct
nla_policy SUBCMD_HQA_POLICY[MTK_NL80211_VENDOR_ATTR_HQA_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_HQA] = { .type = NLA_BINARY, .len = sizeof(struct hqa_frame) },
};
#endif

static const struct
nla_policy SUBCMD_V10CONVERTER_POLICY[MTK_NL80211_VENDOR_ATTR_V10CONVERTER_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_V10CONVERTER] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_APPROXYREFRESH_POLICY[MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_SET_APPROXYREFRESH] = { .type = NLA_U8 },
};

static const struct
nla_policy VENDOR_SUBCMD_E2P_POLICY[MTK_NL80211_VENDOR_ATTR_E2P_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM] = { .type = NLA_BINARY,
		.len = sizeof(struct e2p_param) },
	[MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM] = { .type = NLA_BINARY,
		.len = sizeof(struct e2p_param) },
	[MTK_NL80211_VENDOR_ATTR_E2P_DUMP_ALL_PARAM] = { .type = NLA_BINARY,
		.len = sizeof(struct e2p_param) },
};

#ifdef CONFIG_WLAN_SERVICE
static const struct
nla_policy VENDOR_SUBCMD_TESTENGINE_POLICY[MTK_NL80211_VENDOR_ATTR_TESTENGINE_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_TESTENGINE_SET] = { .type = NLA_BINARY,
		.len = sizeof(struct testengine_param) },
	[MTK_NL80211_VENDOR_ATTR_TESTENGINE_GET] = { .type = NLA_BINARY,
		.len = sizeof(struct testengine_param) },
};
#endif /* CONFIG_WLAN_SERVICE */

static const struct
nla_policy SUBCMD_VENDOR_MLO_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR] = { .type = NLA_U8  },
	[MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_RSP_STR] = { .type = NLA_U8  },
	[MTK_NL80211_VENDOR_ATTR_DUMP_APCLI_MLD] = { .type = NLA_FLAG  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC] = { .type = NLA_BINARY, .len = ETH_ALEN },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC] = { .type = NLA_BINARY, .len = ETH_ALEN  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS] = { .type = NLA_BINARY  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC] = { .type = NLA_BINARY, .len = ETH_ALEN  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLMR_ENABLE] = { .type = NLA_FLAG  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLSR_ENABLE] = { .type = NLA_FLAG  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_STR_ENABLE] = { .type = NLA_FLAG  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_NSTR_ENABLE] = { .type = NLA_FLAG  },
	[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC] = { .type = NLA_BINARY, .len = ETH_ALEN},
};

static const struct
nla_policy SUBCMD_VENDOR_MLO_SWITH_POLICY[_MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET] = { .type = NLA_U8	},
};

static const struct
nla_policy SUBCMD_GET_BSS_MLO_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO] = { .type = NLA_BINARY,
		.len = sizeof(struct bss_mlo_info)},
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS] = { .type = NLA_BINARY, .len = 6 * sizeof(char) },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_DUMP] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS] = { .type = NLA_BINARY },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID] = { .type = NLA_BINARY, .len = 6 * sizeof(char) },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN] = { .type = NLA_U16 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_STR] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_NSTR] = { .type = NLA_U8 },
};

static const struct
nla_policy SUBCMD_GET_CONNECTED_STA_MLD_INFO_AFFILIATED_STA_POLICY[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC] = { .type = NLA_BINARY, .len = 6 * sizeof(char) },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID] = { .type = NLA_BINARY, .len = 6 * sizeof(char) },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID] = { .type = NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL] = { .type = NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL] = { .type = NLA_U8},
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT] = { .type = NLA_U64 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED] = { .type = NLA_U64 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT] = { .type = NLA_U64 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH] = { .type = NLA_S32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE] = { .type = NLA_U64 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT] = { .type = NLA_U64 },
};

static const struct
nla_policy SUBCMD_GET_CONNECTED_STA_MLD_INFO_POLICY[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP] = { .type = NLA_U8 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC] = { .type = NLA_BINARY, .len = 6 * sizeof(char) },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLMR] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLSR] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_STR] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_NSTR] = { .type = NLA_FLAG },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME] = { .type = NLA_U32 },
	[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA] = { .type = NLA_NESTED,
#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
		.validation_data = (void *)SUBCMD_GET_CONNECTED_STA_MLD_INFO_AFFILIATED_STA_POLICY},
#else
		.nested_policy = SUBCMD_GET_CONNECTED_STA_MLD_INFO_AFFILIATED_STA_POLICY},
#endif
};

#ifdef DFS_SLAVE_SUPPORT
static const struct
nla_policy SUBCMD_BH_STATUS_POLICY[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_BH_STATUS] =
		NLA_POLICY_EXACT_LEN(sizeof(struct BH_STATUS_MSG)),
};
#endif /* DFS_SLAVE_SUPPORT.*/

static const struct
nla_policy SUBCMD_OFFLOAD_ACS_POLICY[MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_ATTR_MAX + 1] = {
	[MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_SET] = {.type = NLA_U8},
};


static const struct wiphy_vendor_command mtk_vendor_commands[] = {
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_TEST
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_nl80211_vendor_subcmd_test,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_TEST_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_TEST_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_CAP,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_get_cap,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_CAP_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_GET_CAP_INFO_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_RUNTIME_INFO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_get_runtime_info,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_RUNTIME_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_STATISTIC,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_get_statistic,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_STATISTIC_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_GET_STATISTIC_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_STATIC_INFO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_get_static_info,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_STATIC_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_GET_STATIC_INFO_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_BAND_INFO,
		},
		.flags = 0,
		.doit = mtk_cfg80211_get_band_info,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_STATIC_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX,
#endif
	},
#ifdef WAPP_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_WAPP_REQ,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_wapp_req,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_WAPP_REQ_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_WAPP_REQ_MAX,
#endif
	},
#endif /* WAPP_SUPPORT */
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MULTICAST_SNOOPING,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_mcast_snoop_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_MULTICAST_SNOOPING_POLICY,
		.maxattr = MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX,
#endif
	},
#ifdef FTM_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_FTM,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_ftm_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_FTM_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_FTM_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_FTM_STAT,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_ftm_stat_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_FTM_STAT_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX,
#endif
	},
#endif /* FTM_SUPPORT */
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_AP_WIRELESS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_ap_wireless_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_AP_WIRELESS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_AP_RFEATURE,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_ap_rfeature_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_AP_RFEATURE_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_RFEATURE_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_COUNTRY,
		},
		.doit = mtk_cfg80211_vndr_country_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_COUNTRY_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_COUNTRY_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_CHANNEL,
		},
		.doit = mtk_cfg80211_vndr_channel_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_CHANNEL_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_CHAN_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_ACL,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_acl_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_ACL_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_VENDOR_SHOW,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_show_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_VENDOR_SHOW_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_VENDOR_SET_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_VENDOR_SET_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_MAC,
		},
		.doit = mtk_cfg80211_vndr_mac_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = VENDOR_SUBCMD_MAC_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_MAC_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_STATISTICS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_statistic_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = common_policy,
#endif
	},
#ifdef VOW_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_AP_VOW,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vow_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_AP_VOW_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_VOW_ATTR_MAX,
#endif
	},
#endif
#ifdef MWDS
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MWDS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_set_mwds,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_MWDS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_MWDS_ATTR_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_AUTO_CH_SEL,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_auto_ch_sel_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_AUTO_CH_SEL_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL_MAX,
#endif
	},
#ifdef SCAN_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_SCAN,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_scan_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_SCAN_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_SCAN_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_bss_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_AP_BSS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_BSS_ATTR_MAX,
#endif
	},

	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO,
		},
		.flags = 0,
		.doit = mtk_cfg80211_radio_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_AP_RADIO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_RADIO_ATTR_MAX,
#endif
	},
#ifdef HOSTAPD_11R_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_FT,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_ft_ie,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_ft_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_FT_MAX,
#endif
	},
#endif
#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MLO_IE,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_mlo_ie,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_mlo_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_MLO_IE_MAX,
#endif
	},
#endif
#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_DEL_MLO_STA_ENTRY,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_del_mlo_sta_entry,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_mlo_ft_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_DEL_MLO_STA_ENTRY,
#endif
#endif
	},
#ifdef RT_CFG80211_SUPPORT
#endif /* RT_CFG80211_SUPPORT */

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_PMKID,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_set_pmkid,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_pmkid_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_PMKID_MAX,
#endif
	},
#endif

#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MAC,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_set_mac,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_set_mac_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_SET_MAC_MAX,
#endif
	},

	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SEQ_NUM,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_set_seq_num,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_set_seq_num_policy,
		.maxattr = MTK_NL80211_VENDOR_ATTR_SET_SEQ_NUM_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_WMM,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_wmm_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = VENDOR_SUBCMD_SET_WMM_POLICY,
		.maxattr = MTK_NL80211_VENDOR_WMM_ATTR_MAX,
#endif
	},
#ifdef AIR_MONITOR
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_monitor_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_AP_MONITOR_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_MONITOR_ATTR_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_CLEAN_STATION,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_clean_sta_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = mtk_nl80211_vendor_attr_sta_clean_policy,
		.maxattr = MTK_NL80211_VENDOR_STA_CLEAN_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_BA,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_ba_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_BA_POLICY,
		.maxattr = MTK_NL80211_VENDOR_AP_BA_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_ACTION,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_action,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_ACTION_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_SET_ACTION_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_QOS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_qos,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = VENDOR_SUBCMD_QOS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_QOS_MAX,
#endif
	},
#ifdef OFFCHANNEL_SCAN_FEATURE
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_OFFCHANNEL_INFO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_subcmd_offchannel_info,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_OFFCHANNEL_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_SUBCMD_OFF_CHANNEL_INFO_MAX,
#endif
	},
#endif /* OFFCHANNEL_SCAN_FEATURE */
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_DFS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_subcmd_dfs,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_DFS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_DFS_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_EASYMESH,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_subcmd_easymesh,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_EASYMESH_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_EASYMESH_MAX,
#endif
	},
#ifdef VLAN_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_VLAN,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vlan_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_VLAN_POLICY,
		.maxattr = MTK_NL80211_VENDOR_VLAN_ATTR_MAX,
#endif
	},
#endif
#ifdef MBO_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MBO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_mbo_set_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_MBO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_MBO_MAX,
#endif
	},
#endif
#ifdef DOT11_EHT_BE
	{
			.info = {
				.vendor_id = MTK_OUI,
				.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MLO_PRESET_LINK,
			},
			.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
					WIPHY_VENDOR_CMD_NEED_NETDEV,
			.doit = mtk_cfg80211_mlo_preset_linkid_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
			.policy = mtk_nl80211_vendor_attr_mlo_preset_linkid_policy,
			.maxattr = MTK_NL80211_VENDOR_MLO_PRESET_LINKID_ATTR_MAX,
#endif
	},
#endif /*DOT11_EHT_BE*/
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_TXPOWER,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_tx_power_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_TXPOWER_POLICY,
		.maxattr = MTK_NL80211_VENDOR_TXPWR_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_EDCA,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_edca_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_EDCA_POLICY,
		.maxattr = MTK_NL80211_VENDOR_EDCA_ATTR_MAX,
#endif
	},

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_VIE,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_vie_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_VENDOR_IE_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_VIE_ATTR_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_RADIO_STATS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_radio_stats,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_SET_RADIO_STATS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_RADIO_SET_STATS_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_radio_stats,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_RADIO_STATS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_BSS_STATS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_bss_stats,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_SSID_STATS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_STA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_sta,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_STA_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX,
#endif
	},
#ifdef CONFIG_WLAN_SERVICE
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_HQA,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_hqa_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_HQA_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_HQA_MAX,
#endif
	},
#endif /* CONFIG_WLAN_SERVICE */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_V10CONVERTER,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_v10converter,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_V10CONVERTER_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_V10CONVERTER_MAX,
#endif
	},
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/
#ifdef A4_CONN
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_APPROXYREFRESH,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_set_apropxyrefresh,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_APPROXYREFRESH_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_APPROXYREFRESH_MAX,
#endif
	},
#endif /* A4_CONN */
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_E2P,
		},
		.doit = mtk_cfg80211_vndr_e2p_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = VENDOR_SUBCMD_E2P_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_E2P_MAX,
#endif
	},
#ifdef CONFIG_WLAN_SERVICE
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_TESTENGINE,
		},
		.doit = mtk_cfg80211_vndr_testengine_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = VENDOR_SUBCMD_TESTENGINE_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_TESTENGINE_MAX,
#endif
	},
#endif /* CONFIG_WLAN_SERVICE */
#ifdef DOT11_EHT_BE
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SHOW_MLO_INFO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_mlo_info_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_VENDOR_MLO_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_SET_MLO_SWITCH,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mkt_cfg80211_vndr_mlo_switch_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_VENDOR_MLO_SWITH_POLICY,
		.maxattr = _MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_ATTR_MAX,
#endif
	},
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_BSS_MLO_INFO,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_bss_mlo_info,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_BSS_MLO_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_BSS_MLO_INFO_ATTR_MAX,
#endif
	},
#endif
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_OFFLOAD_ACS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mkt_cfg80211_vndr_offload_acs_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_OFFLOAD_ACS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_OFFLOAD_ACS_ATTR_MAX,
#endif
	},
#ifdef DFS_SLAVE_SUPPORT
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_BH_STATUS,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_bh_status_handler,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_BH_STATUS_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_DFS_SLAVE_MAX,
#endif
	},
#endif /* DFS_SLAVE_SUPPORT.*/
	{
		.info = {
			.vendor_id = MTK_OUI,
			.subcmd = MTK_NL80211_VENDOR_SUBCMD_GET_CONNECTED_STA_MLD,
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vndr_get_connected_sta_mld,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = SUBCMD_GET_CONNECTED_STA_MLD_INFO_POLICY,
		.maxattr = MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX,
#endif
	},
};

/*netlink vendor cmds <==*/

static const struct nl80211_vendor_cmd_info mtk_vendor_events[] = {
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_UNSPEC,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_TEST,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_RSP_WAPP_EVENT,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_OFFCHANNEL_INFO,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_SEND_ML_INFO,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_MLO_EVENT,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_STA_PROFILE_EVENT,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_DISC_STA,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_COSR,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_BH_STATUS,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_ACS_COMPLETE_EVENT,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_RX_T2LM_STOP_DISASSOC_TIMER,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_ACS_PER_CH_INFO,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_SEND_MLO_STA_LINK_MAC,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_MLO_RECONF,
	},
	{
		.vendor_id = MTK_OUI,
		.subcmd = MTK_NL80211_VENDOR_EVENT_ANDLINK,
	},
	/* add new mtk vendor events above here */

};

static const struct ieee80211_iface_combination
	ra_iface_combinations_p2p[] = {
	{
		.num_different_channels = 2,
		.max_interfaces = 8,
		/* CFG TODO*/
		/* .beacon_int_infra_match = true, */
		.limits = ra_p2p_sta_go_limits,
		.n_limits = ARRAY_SIZE(ra_p2p_sta_go_limits),
	},
};
#endif /* LINUX_VERSION_CODE: 3.8.0 */

struct cfg80211_ops CFG80211_Ops = {
#ifdef CONFIG_AP_SUPPORT
	.set_bitrate_mask = CFG80211_SetBitRate,

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
	.add_virtual_intf = CFG80211_OpsVirtualInfAdd,
	.del_virtual_intf = CFG80211_OpsVirtualInfDel,
#endif /* IWCOMMAND_CFG80211_SUPPORT */

	.start_ap	    = CFG80211_OpsStartAp,
	.change_beacon	= CFG80211_OpsChangeBeacon,
	.stop_ap	    = CFG80211_OpsStopAp,
#endif /* CONFIG_AP_SUPPORT */
	/* set channel for a given wireless interface */
	/* CFG_TODO */
	/* .set_monitor_channel = CFG80211_OpsMonitorChannelSet, */
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
	.set_ap_chanwidth 	= CFG80211_OpsChanWithSet,
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/

#ifdef CFG80211_FULL_OPS_SUPPORT
	.channel_switch		= CFG80211_OpsChanSwitch,
#endif
	.get_channel		= CFG80211_OpsGetChannel,

#ifdef CFG80211_FULL_OPS_SUPPORT
	.start_radar_detection		= CFG80211_OpsStartRadarDetect,
	.end_cac			= CFG80211_OpsEndCac,
#endif

	/* change type/configuration of virtual interface */
	.change_virtual_intf		= CFG80211_OpsVirtualInfChg,

	/* request to do a scan */
	/*
	 *	Note: must exist whatever AP or STA mode; Or your kernel will crash
	 *	in v2.6.38.
	 */
	.scan						= CFG80211_OpsScan,
	.abort_scan		= CFG80211_OpsAbortScan,

	/* configure WLAN power management */
	.set_power_mgmt				= NULL,
	/* get station information for the station identified by @mac */
	.get_station				= CFG80211_OpsStaGet,
	/* dump station callback */
	.dump_station				= CFG80211_OpsStaDump,
	/* notify that wiphy parameters have changed */
	.set_wiphy_params			= CFG80211_OpsWiphyParamsSet,
	/* add a key with the given parameters */
	.add_key					= CFG80211_OpsKeyAdd,
	/* get information about the key with the given parameters */
	.get_key					= CFG80211_OpsKeyGet,
	/* remove a key given the @mac_addr */
	.del_key					= CFG80211_OpsKeyDel,
	/* set the default key on an interface */
	.set_default_key			= CFG80211_OpsKeyDefaultSet,
#ifdef BCN_PROTECTION_SUPPORT
#if (KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE) || defined(BACKPORT_NOSTDINC)
	/*set default beacon key */
	.set_default_beacon_key                 = CFG80211_OpsDefaultBeaconKeySet,
#endif
#endif /*BCN_PROTECTION_SUPPORT*/
#ifdef DOT11W_PMF_SUPPORT
	/* set the default mgmt key on an interface */
	.set_default_mgmt_key		= CFG80211_OpsMgmtKeyDefaultSet,
#endif /*DOT11W_PMF_SUPPORT*/
	/* connect to the ESS with the specified parameters */
	.connect					= CFG80211_OpsConnect,
	/* disconnect from the BSS/ESS */
	.disconnect					= CFG80211_OpsDisconnect,
#ifdef APCLI_CFG80211_SUPPORT
	.external_auth			= CFG80211_ExternalAuth,
#endif

#ifdef RFKILL_HW_SUPPORT
	/* polls the hw rfkill line */
	.rfkill_poll				= CFG80211_OpsRFKill,
#endif /* RFKILL_HW_SUPPORT */

	/* get site survey information */
#ifdef CFG80211_FULL_OPS_SUPPORT
	.dump_survey				= CFG80211_OpsSurveyGet,
#endif
	/* cache a PMKID for a BSSID */
	.set_pmksa					= CFG80211_OpsPmksaSet,
	/* flush all cached PMKIDs */
	.flush_pmksa				= CFG80211_OpsPmksaFlush,

	/*
	 *	Request the driver to remain awake on the specified
	 *	channel for the specified duration to complete an off-channel
	 *	operation (e.g., public action frame exchange).
	 */
	.remain_on_channel			= CFG80211_OpsRemainOnChannel,
	/* cancel an on-going remain-on-channel operation */
	.cancel_remain_on_channel	=  CFG80211_OpsCancelRemainOnChannel,
	.mgmt_tx                    = CFG80211_OpsMgmtTx,

	.mgmt_tx_cancel_wait       = CFG80211_OpsTxCancelWait,


	/* configure connection quality monitor RSSI threshold */
	.set_cqm_rssi_config		= NULL,

	/* notify driver that a management frame type was registered */
#ifdef BACKPORT_NOSTDINC
	.update_mgmt_frame_registrations =
		CFG80211_OpsUpdate_mgmt_frame_registrations,
#else
	.mgmt_frame_register		= CFG80211_OpsMgmtFrameRegister,
#endif

#ifdef HOSTAPD_HS_R2_SUPPORT
	.set_qos_map				= CFG80211_OpsSetQosMap,
#endif
	/* set antenna configuration (tx_ant, rx_ant) on the device */
	.set_antenna				= CFG80211_OpsSetAntenna,
	/* get current antenna configuration from device (tx_ant, rx_ant) */
	.get_antenna				= CFG80211_OpsGetAntenna,
	.change_bss                             = CFG80211_OpsChangeBss,
	.del_station                            = CFG80211_OpsStaDel,
	.add_station                            = CFG80211_OpsStaAdd,
	.change_station                         = CFG80211_OpsStaChg,
	/* .set_bitrate_mask                       = CFG80211_OpsBitrateSet, */
#if defined(CONFIG_NL80211_TESTMODE) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
	.testmode_cmd			= CFG80211_OpsTestModeCmd,
#endif /*CONFIG_NL80211_TESTMODE && HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/
};

/* =========================== Global Function ============================== */

static INT CFG80211NetdevNotifierEvent(
	struct notifier_block *nb, ULONG state, VOID *ndev)
{
	struct _RTMP_ADAPTER *pAd;
	struct net_device *pNev;
	struct wireless_dev *pWdev;

	if (!ndev)
		return NOTIFY_DONE;
	pNev = ndev;
	pWdev = pNev->ieee80211_ptr;

	if (!pWdev || !pWdev->wiphy)
		return NOTIFY_DONE;

	MAC80211_PAD_GET(pAd, pWdev->wiphy);

	switch (state) {
	case NETDEV_UNREGISTER:
		break;

	case NETDEV_GOING_DOWN:
		RTMP_DRIVER_80211_NETDEV_EVENT(pAd, pNev, state);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block cfg80211_netdev_notifier = {
	.notifier_call = CFG80211NetdevNotifierEvent,
};


static void CFG80211_WiphyFeatureSet(
	struct wiphy *prWiphy)
{
	/* @NL80211_FEATURE_INACTIVITY_TIMER:
	 * This driver takes care of freeingup
	 * the connected inactive stations in AP mode.
	 */
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* http://www.permalink.gmane.org/gmane.linux.kernel.wireless.general/86454 */
	prWiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;

#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
	prWiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif /*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/

#ifdef SUPP_SAE_SUPPORT
	prWiphy->features |= NL80211_FEATURE_SAE;
#endif /* SUPP_SAE_SUPPORT */

	/* Enabling OCV to indicate Hostapd to support OCV Feature*/
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)
	prWiphy->ext_features[NL80211_EXT_FEATURE_OPERATING_CHANNEL_VALIDATION/8] |=  BIT(NL80211_EXT_FEATURE_OPERATING_CHANNEL_VALIDATION%8);
#endif
#if defined(APCLI_CFG80211_SUPPORT) && defined(SUPP_RANDOM_MAC_IN_DRIVER_SUPPORT)
	prWiphy->ext_features[NL80211_EXT_FEATURE_MGMT_TX_RANDOM_TA/8] |=  BIT(NL80211_EXT_FEATURE_MGMT_TX_RANDOM_TA%8);
#endif

#ifdef BCN_PROTECTION_SUPPORT
#if KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)
	prWiphy->ext_features[NL80211_EXT_FEATURE_BEACON_PROTECTION/8] |=  BIT(NL80211_EXT_FEATURE_BEACON_PROTECTION%8);
#endif
#endif /*BCN_PROTECTION_SUPPORT*/
#ifdef CONFIG_6G_SUPPORT
#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE || defined(BACKPORT_NOSTDINC)
	prWiphy->ext_features[NL80211_EXT_FEATURE_UNSOL_BCAST_PROBE_RESP/8] |=  BIT(NL80211_EXT_FEATURE_UNSOL_BCAST_PROBE_RESP%8);
#endif
#endif
}

/*
 * ========================================================================
 * Routine Description:
 *	Allocate a wireless device.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pDev			- Generic device interface
 *
 * Return Value:
 *	wireless device
 *
 * Note:
 * ========================================================================
 */
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
static const struct wiphy_iftype_ext_capab add_iftypes_ext_capa[] = {
	{
		.iftype = NL80211_IFTYPE_AP,
		.eml_capabilities = IEEE80211_EML_CAP_EMLSR_SUPP,
	},
	{
		.iftype = NL80211_IFTYPE_STATION,
		.eml_capabilities = IEEE80211_EML_CAP_EMLSR_SUPP,
	},
};
#endif /* defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION) */



static struct wireless_dev *CFG80211_WdevAlloc(
	IN CFG80211_CB					*pCfg80211_CB,
	IN CFG80211_BAND * pBandInfo,
	IN VOID						*pAd,
	IN struct device				*pDev)
{
	struct wireless_dev *pWdev;
	struct wiphy *prWiphy = NULL;
	ULONG *pPriv;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)pAd;
#ifdef SEL_DUAL_BAND
	char phy_name[10];
	int ret = 0;
#endif

	/*
	 * We're trying to have the following memory layout:
	 *
	 * +------------------------+
	 * | struct wiphy			|
	 * +------------------------+
	 * | pAd pointer			|
	 * +------------------------+
	 */
	os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
	if (pWdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"80211> Wireless device allocation fail!\n");
		return NULL;
	} /* End of if */
	os_zero_mem((uint8_t *)pWdev, sizeof(*pWdev));

#ifdef SEL_DUAL_BAND
	ret = sprintf(phy_name, "phy%d", ad->dev_idx);
	if (ret < 0) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
		"80211> Sprintf error!\n");
	}
	pWdev->wiphy = wiphy_new_nm(&CFG80211_Ops, sizeof(ULONG *), phy_name);
#else
	pWdev->wiphy = wiphy_new(&CFG80211_Ops, sizeof(ULONG *));
#endif

	if (pWdev->wiphy == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"80211> Wiphy device allocation fail!\n");
		goto LabelErrWiphyNew;
	} /* End of if */
	prWiphy = pWdev->wiphy;

	/*netlink vendor cmds and events*/
	prWiphy->vendor_commands = mtk_vendor_commands;
	prWiphy->n_vendor_commands = ARRAY_SIZE(mtk_vendor_commands);

	prWiphy->vendor_events = mtk_vendor_events;
	prWiphy->n_vendor_events = ARRAY_SIZE(mtk_vendor_events);

	if (NdisEqualMemory(ZERO_MAC_ADDR, ad->PermanentAddress, MAC_ADDR_LEN) == FALSE)
		COPY_MAC_ADDR(prWiphy->perm_addr, ad->PermanentAddress);

	MTWF_DBG(ad, DBG_CAT_HW, CATINIT_INTF, DBG_LVL_INFO, "PermanentAddress MAC: ="MACSTR"\n",
			 MAC2STR(ad->PermanentAddress));
	/* keep pAd pointer */
	pPriv = (ULONG *)(wiphy_priv(prWiphy));
	*pPriv = (ULONG)pAd;
	set_wiphy_dev(prWiphy, pDev);
	/* max_scan_ssids means in each scan request, how many ssids can driver handle to send probe-req.
	 *  In current design, we only support 1 ssid at a time. So we should set to 1.
	*/
	/* pWdev->wiphy->max_scan_ssids = pBandInfo->MaxBssTable; */
	prWiphy->max_scan_ssids = 1;

	CFG80211_WiphyFeatureSet(prWiphy);

	prWiphy->interface_modes = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_STATION);
#ifdef CONFIG_VLAN_GTK_SUPPORT
	pWdev->wiphy->interface_modes |= BIT(NL80211_IFTYPE_AP_VLAN);
#endif
#ifdef CONFIG_STA_SUPPORT
	prWiphy->interface_modes |= BIT(NL80211_IFTYPE_ADHOC);
#endif /* CONFIG_STA_SUPPORT */
	 pWdev->wiphy->reg_notifier = CFG80211_RegNotifier;
	/* init channel information */
	CFG80211_SupBandInit(pAd, pCfg80211_CB, pBandInfo, prWiphy, NULL, NULL, FALSE);

	/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
	prWiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	prWiphy->max_scan_ie_len = IEEE80211_MAX_DATA_LEN;
	prWiphy->max_num_pmkids = 4;
	prWiphy->max_remain_on_channel_duration = 5000;
	prWiphy->mgmt_stypes = ralink_mgmt_stypes;
	prWiphy->cipher_suites = CipherSuites;
	prWiphy->n_cipher_suites = ARRAY_SIZE(CipherSuites);
	prWiphy->flags |= WIPHY_FLAG_AP_UAPSD;
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 018-cfg80211-internal-ap-mlme.patch */
	prWiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 008-cfg80211-offchan-flags.patch */
	prWiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_STRICT_REGULATORY; */

	prWiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
	prWiphy->flags |= WIPHY_FLAG_SUPPORTS_MLO;
	prWiphy->iftype_ext_capab = add_iftypes_ext_capa;
	prWiphy->num_iftype_ext_capab = ARRAY_SIZE(add_iftypes_ext_capa);
#endif /* defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION) */
	prWiphy->max_num_csa_counters = 10;

#ifdef DOT11V_MBSSID_SUPPORT
	prWiphy->support_mbssid = 1;
#endif

	/* Driver Report Support TDLS to supplicant */
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_IBSS_RSN; */
	prWiphy->iface_combinations = ra_iface_combinations_p2p;
	prWiphy->n_iface_combinations = ARRAY_SIZE(ra_iface_combinations_p2p);

	{
		struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(ad);
		prWiphy->available_antennas_tx = BIT(nss_cap->max_nss) - 1;
		prWiphy->available_antennas_rx = BIT(nss_cap->max_nss) - 1;
	}

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
	prWiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#if defined(CFG_CFG80211_VERSION) && (KERNEL_VERSION(6, 0, 0) <= CFG_CFG80211_VERSION)
	prWiphy->max_num_akm_suites = 10;
#endif
	if (wiphy_register(prWiphy) < 0) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"80211> Register wiphy device fail!\n");
		goto LabelErrReg;
	}

	/*add to support DPP action frame Tx*/
	prWiphy->flags |= WIPHY_FLAG_OFFCHAN_TX;

	return pWdev;
LabelErrReg:
	wiphy_free(prWiphy);
	pWdev->wiphy = NULL;
LabelErrWiphyNew:
	os_free_mem(pWdev);
	return NULL;
} /* End of CFG80211_WdevAlloc */


/*
 * ========================================================================
 * Routine Description:
 *	Register MAC80211 Module.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pDev			- Generic device interface
 *	pNetDev			- Network device
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	pDev != pNetDev
 *	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))
 *
 *	Can not use pNetDev to replace pDev; Or kernel panic.
 * ========================================================================
 */
BOOLEAN CFG80211_Register(
	IN struct _RTMP_ADAPTER	*pAd,
	IN struct device *pDev,
	IN struct net_device *pNetDev)
{
	CFG80211_CB *pCfg80211_CB = NULL;
	CFG80211_BAND BandInfo;
	INT err = 0;
	UINT32 OpMode;
	/* allocate Main Device Info structure */
	os_alloc_mem(NULL, (UCHAR **)&pCfg80211_CB, sizeof(CFG80211_CB));

	if (pCfg80211_CB == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"80211> Allocate MAC80211 CB fail!\n");
		return FALSE;
	}

	os_zero_mem(pCfg80211_CB, sizeof(CFG80211_CB));
	/* allocate wireless device */
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);
	pCfg80211_CB->pCfg80211_Wdev = CFG80211_WdevAlloc(pCfg80211_CB, &BandInfo, pAd, pDev);

	if (pCfg80211_CB->pCfg80211_Wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"80211> Allocate Wdev fail!\n");
		os_free_mem(pCfg80211_CB);
		return FALSE;
	}

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);

	/* bind wireless device with net device */
#ifdef CONFIG_AP_SUPPORT
	/* default we are AP mode */
	if (OpMode == OPMODE_AP)
		pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_AP;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	/* default we are station mode */
	if (OpMode == OPMODE_STA)
		pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_STATION;
#endif /* CONFIG_STA_SUPPORT */
	pNetDev->ieee80211_ptr = pCfg80211_CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(pCfg80211_CB->pCfg80211_Wdev->wiphy));
	pCfg80211_CB->pCfg80211_Wdev->netdev = pNetDev;
#ifdef RFKILL_HW_SUPPORT
	wiphy_rfkill_start_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */
	pAd->pCfg80211_CB = pCfg80211_CB;
	RTMP_DRIVER_80211_RESET(pAd);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, TRUE);

	wiphy_ext_feature_set(pCfg80211_CB->pCfg80211_Wdev->wiphy, NL80211_EXT_FEATURE_DFS_OFFLOAD);
	wiphy_ext_feature_set(pCfg80211_CB->pCfg80211_Wdev->wiphy, NL80211_EXT_FEATURE_BEACON_RATE_LEGACY);
	wiphy_ext_feature_set(pCfg80211_CB->pCfg80211_Wdev->wiphy, NL80211_EXT_FEATURE_ACK_SIGNAL_SUPPORT);
	/* TODO */
	/* err = register_netdevice_notifier(&cfg80211_netdev_notifier);	//CFG TODO */
	if (err)
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"80211> Failed to register notifierl %d\n", err);

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR, "80211> end\n");
	return TRUE;
} /* End of CFG80211_Register */




/* =========================== Local Function =============================== */

/*
 * ========================================================================
 * Routine Description:
 *	The driver's regulatory notification callback.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pRequest		- Regulatory request
 *
 * Return Value:
 *	0
 *
 * Note:
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
static void CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN struct regulatory_request	*pRequest)
{
	VOID *pAd;
	ULONG *pPriv;
	/* sanity check */
	pPriv = (ULONG *)(wiphy_priv(pWiphy));
	pAd = (VOID *)(*pPriv);

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
			"crda> reg notify but pAd = NULL!");
		return;
	} /* End of if */

	/*
	 *	Change the band settings (PASS scan, IBSS allow, or DFS) in mac80211
	 *	based on EEPROM.
	 *
	 *	IEEE80211_CHAN_DISABLED: This channel is disabled.
	 *	IEEE80211_CHAN_PASSIVE_SCAN: Only passive scanning is permitted
	 *				on this channel.
	 *	IEEE80211_CHAN_NO_IBSS: IBSS is not allowed on this channel.
	 *	IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
	 *	IEEE80211_CHAN_NO_FAT_ABOVE: extension channel above this channel
	 *				is not permitted.
	 *	IEEE80211_CHAN_NO_FAT_BELOW: extension channel below this channel
	 *				is not permitted.
	 */

	switch (pRequest->initiator) {
	case NL80211_REGDOM_SET_BY_CORE:
		/* Core queried CRDA for a dynamic world regulatory domain. */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by core: ");
		break;

	case NL80211_REGDOM_SET_BY_USER:
		/*
		 *	User asked the wireless core to set the regulatory domain.
		 *	(when iw, network manager, wpa supplicant, etc.)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by user: ");
		break;

	case NL80211_REGDOM_SET_BY_DRIVER:
		/*
		 *	A wireless drivers has hinted to the wireless core it thinks
		 *	its knows the regulatory domain we should be in.
		 *	(when driver initialization, calling regulatory_hint)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by driver: ");
		break;

	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		/*
		 *	The wireless core has received an 802.11 country information
		 *	element with regulatory information it thinks we should consider.
		 *	(when beacon receive, calling regulatory_hint_11d)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by country IE: ");
		break;
	} /* End of switch */

	MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"%c%c\n", pRequest->alpha2[0], pRequest->alpha2[1]);

	/* only follow rules from user */
	if (pRequest->initiator == NL80211_REGDOM_SET_BY_USER) {
		/* keep Alpha2 and we can re-call the function when interface is up */
		CMD_RTPRIV_IOCTL_80211_REG_NOTIFY RegInfo;

		RegInfo.Alpha2[0] = pRequest->alpha2[0];
		RegInfo.Alpha2[1] = pRequest->alpha2[1];
		RegInfo.pWiphy = pWiphy;
		RTMP_DRIVER_80211_REG_NOTIFY(pAd, &RegInfo);
	} /* End of if */

	return;
} /* End of CFG80211_RegNotifier */

#else

static void CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN enum reg_set_by				Request)
{
	struct device *pDev = pWiphy->dev.parent;
	struct net_device *pNetDev = dev_get_drvdata(pDev);
	VOID *pAd = (VOID *)RTMP_OS_NETDEV_GET_PRIV(pNetDev);
	UINT32 ReqType = Request;

	/* sanity check */
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO,
			"crda> reg notify but pAd = NULL!");
		return;
	} /* End of if */

	/*
	 *	Change the band settings (PASS scan, IBSS allow, or DFS) in mac80211
	 *	based on EEPROM.
	 *
	 *	IEEE80211_CHAN_DISABLED: This channel is disabled.
	 *	IEEE80211_CHAN_PASSIVE_SCAN: Only passive scanning is permitted
	 *				on this channel.
	 *	IEEE80211_CHAN_NO_IBSS: IBSS is not allowed on this channel.
	 *	IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
	 *	IEEE80211_CHAN_NO_FAT_ABOVE: extension channel above this channel
	 *				is not permitted.
	 *	IEEE80211_CHAN_NO_FAT_BELOW: extension channel below this channel
	 *				is not permitted.
	 */

	switch (ReqType) {
	case REGDOM_SET_BY_CORE:
		/* Core queried CRDA for a dynamic world regulatory domain. */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by core: ");
		break;

	case REGDOM_SET_BY_USER:
		/*
		 *	User asked the wireless core to set the regulatory domain.
		 *	(when iw, network manager, wpa supplicant, etc.)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by user: ");
		break;

	case REGDOM_SET_BY_DRIVER:
		/*
		 *	A wireless drivers has hinted to the wireless core it thinks
		 *	its knows the regulatory domain we should be in.
		 *	(when driver initialization, calling regulatory_hint)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by driver: ");
		break;

	case REGDOM_SET_BY_COUNTRY_IE:
		/*
		 *	The wireless core has received an 802.11 country information
		 *	element with regulatory information it thinks we should consider.
		 *	(when beacon receive, calling regulatory_hint_11d)
		 */
		MTWF_DBG(NULL, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_ERROR,
			"crda> requlation requestion by country IE: ");
		break;
	} /* End of switch */

	MTWF_DBG(pAd, DBG_CAT_CFG80211, CATCFG80211_CMM, DBG_LVL_INFO, "00\n");

	/* only follow rules from user */
	if (ReqType == REGDOM_SET_BY_USER) {
		/* keep Alpha2 and we can re-call the function when interface is up */
		CMD_RTPRIV_IOCTL_80211_REG_NOTIFY RegInfo;

		RegInfo.Alpha2[0] = '0';
		RegInfo.Alpha2[1] = '0';
		RegInfo.pWiphy = pWiphy;
		RTMP_DRIVER_80211_REG_NOTIFY(pAd, &RegInfo);
	} /* End of if */

	return;
} /* End of CFG80211_RegNotifier */
#endif /* LINUX_VERSION_CODE */


#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE */

/* End of crda.c */
