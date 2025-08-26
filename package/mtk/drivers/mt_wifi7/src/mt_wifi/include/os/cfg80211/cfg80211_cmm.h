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


#ifndef __CFG80211CMM_H__
#define __CFG80211CMM_H__

#ifdef RT_CFG80211_SUPPORT

#define RTMP_CFG80211_HOSTAPD_ON(__pAd) (__pAd->net_dev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP)


#define CFG80211_GetEventDevice(__pAd) __pAd->net_dev

#define CFG_GO_BSSID_IDX (MAIN_MBSSID + 1)

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211CMM_H__ */


