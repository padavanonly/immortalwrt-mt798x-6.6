/*
 * Copyright (c) [2024], MediaTek Inc. All rights reserved.
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
 ****************************************************************************

	Module Name:
	ap_ch_prio.c

	Abstract:
	Handle operations related to channel priority

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Yancy Leng  02-02-2024    created

 */


#ifndef __CH_PRIO_H__
#define __CH_PRIO_H__

#include "rt_config.h"

enum ENUM_CHAN_RELATIVE_PRIORITY {
	CH_PRIO_LOW = 1,
	CH_PRIO_MEDIUM = 2,
	CH_PRIO_HIGH = 3,
};

enum ENUM_CH_PRIO_OP_OWNER {
	CH_PRIO_OP_OWNER_NONE = 0,
	CH_PRIO_OP_OWNER_UNSAFE_CHN = 1,
	CH_PRIO_OP_OWNER_STATIC_CFG = 2,
	CH_PRIO_OP_OWNER_RUNTIME_CFG = 3
};

struct _CH_PRIO_LIST {
	UCHAR channel;
	UCHAR priority;
};
struct _CH_PRIO_INFO {
	struct _CH_PRIO_LIST ch_list[MAX_NUM_OF_CHANNELS + 1];	//update channel list
	UCHAR ch_num;//number of channels in the list
};

BOOLEAN UpdateChannelPriority(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct _CH_PRIO_INFO *pChPrioUpdateInfo,
	UCHAR owner);

VOID ChannelPriorityOpCtrlInit(IN struct _RTMP_ADAPTER	*pAd);
VOID ChannelPriorityOpCtrlDeinit(IN struct _RTMP_ADAPTER	*pAd);
BOOLEAN ResetChPrio(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	UCHAR owner);
BOOLEAN LoadCfgChprio(CHANNEL_CTRL *pChCtrl);
BOOLEAN UpdatePreferChannel(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	UCHAR ch_list[],
	UCHAR ch_num,
	UCHAR owner);
INT32 ShowChPrioInfo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
UCHAR GetMaxChannelPriority(
	IN struct _RTMP_ADAPTER *pAd
);

#endif /* __CH_PRIO_H__ */
