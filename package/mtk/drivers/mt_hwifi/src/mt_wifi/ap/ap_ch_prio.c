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
	Yancy Leng  02-02-2024    created for channel priority feature
	Yacny Leng  02-18-2024    merge with Logan trunk
*/


#include "rt_config.h"

#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT*/


/**
* ChannelPriorityOpCtrlInit - Init Channel Priority Operation Control DB.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID ChannelPriorityOpCtrlInit(IN struct _RTMP_ADAPTER *pAd)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/*Legality check*/
	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pChCtrl is NULL\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "pAd=%p\n", pAd);

	/*init spinlock and set owner to idle*/
	NdisAllocateSpinLock(pAd, &pChCtrl->ChPrioOpLock);
	pChCtrl->CurChPrioOpOwner = CH_PRIO_OP_OWNER_NONE;

}

/**
* ChannelPriorityOpCtrlDeinit - Deinit Channel Priority Operation Control DB.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID ChannelPriorityOpCtrlDeinit(IN struct _RTMP_ADAPTER *pAd)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/*Legality check*/
	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pChCtrl is NULL\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "pAd=%p\n", pAd);

	/*release the channel priority operation lock
		and set the owner to idle*/
	NdisFreeSpinLock(&pChCtrl->ChPrioOpLock);
	pChCtrl->CurChPrioOpOwner = CH_PRIO_OP_OWNER_NONE;

}

/**
* ResetChPrio - set channel priority to default value.
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
* @owner: owner of this channel priority operation
**/
BOOLEAN ResetChPrio(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	UCHAR owner)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	/*1.Legality check*/
	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pChCtrl is NULL\n");
		return FALSE;
	}

	/*2.set all channel priority parameters to default*/
	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);
	/*owner' priority check*/
	if (pChCtrl->CurChPrioOpOwner > owner) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"request ch prio owner:%d, current owner:%d, ignore this request\n",
			owner, pChCtrl->CurChPrioOpOwner);
		NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);
		return FALSE;
	}

	/*Setting the flag indicates that
	ch prio needs to be set to cfg*/
	pChCtrl->NeedResetToCfgChPrio = TRUE;

	/*3.refresh channel list and capability*/
	BuildChannelList(pAd, wdev);

	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	return TRUE;
}

/**
* LoadCfgChprio - update channel priority from cfg file
* @pChCtrl: pointer of the CHANNEL_CTRL
**/
BOOLEAN LoadCfgChprio(CHANNEL_CTRL *pChCtrl)
{
	struct Chan_Config *pChCfg = NULL;
	UCHAR num, chListIdx, chPriIdx;
	BOOLEAN bFound;

	/*1.Legality check*/
	if (!pChCtrl) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"ERROR: %s's pChCtrl is NULL !\n", __func__);
		return FALSE;
	}

	num = pChCtrl->ChListNum;
	pChCfg = &pChCtrl->ch_cfg;

	/*2.set priority by config*/

	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);
	/*find out whether priority is set*/
	if (pChCfg->chan_prio_valid == FALSE) {
		/*no priority set,set each channel's priority to CHAN_PRIO_UNSPECIFIED(0xFF)*/
		pChCtrl->MaxChPrio = CHAN_PRIO_UNSPECIFIED;
		for (chListIdx = 0; chListIdx < num; chListIdx++)
			pChCtrl->ChList[chListIdx].Priority = CHAN_PRIO_UNSPECIFIED;

		pChCtrl->CurChPrioOpOwner = CH_PRIO_OP_OWNER_NONE;

		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"set channel priority to default(0XFF).\n");

	} else {
		/*priority set,set each channel's priority according to priority table*/
		/*reset max channel priority to 0*/
		pChCtrl->MaxChPrio = 0;

		for (chListIdx = 0; chListIdx < num; chListIdx++) {

			bFound = FALSE;

			for (chPriIdx = 0; chPriIdx < MAX_NUM_OF_CHANNELS; chPriIdx++) {
				if (pChCfg->ch_prio[chPriIdx].start_ch == 0 && pChCfg->ch_prio[chPriIdx].end_ch == 0)
					break;

				else if (pChCtrl->ChList[chListIdx].Channel >= pChCfg->ch_prio[chPriIdx].start_ch &&
						pChCtrl->ChList[chListIdx].Channel <= pChCfg->ch_prio[chPriIdx].end_ch) {

					bFound = TRUE;
					pChCtrl->ChList[chListIdx].Priority = pChCfg->ch_prio[chPriIdx].prio;
					pChCtrl->MaxChPrio = pChCtrl->MaxChPrio < pChCtrl->ChList[chListIdx].Priority ?
										pChCtrl->ChList[chListIdx].Priority : pChCtrl->MaxChPrio;

					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
								"ch %d Priority set to %u\n",
								pChCtrl->ChList[chListIdx].Channel, pChCtrl->ChList[chListIdx].Priority);
					break;
				}
			}

			/*if channel not in the priority list then set is as 0*/
			if (bFound == FALSE) {

				pChCtrl->ChList[chListIdx].Priority = CHAN_PRIO_UNAVAILABLE;

				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"ch %d priority set to unuseable %u\n",
					pChCtrl->ChList[chListIdx].Channel, pChCtrl->ChList[chListIdx].Priority);
			}
		}

		/*set current channel priority operation owner as config*/
		pChCtrl->CurChPrioOpOwner = CH_PRIO_OP_OWNER_STATIC_CFG;
	}

	/*3.Finishing work,
	setting the values ​​of various flags and unlocking them*/

	/*reset the flag,avoid update channel list again*/
	pChCtrl->NeedResetToCfgChPrio = FALSE;
	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	return TRUE;
}

/**
* UpdateChannelPriority - Update runtime channel priority
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
* @pChPrioUpdateInfo: pointer of the channel priority update information
**/
BOOLEAN UpdateChannelPriority(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct _CH_PRIO_INFO *pChPrioUpdateInfo,
	UCHAR owner)
{
	UCHAR chListIdx, chListnum, updateChListIdx, updateChListNum;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	BOOLEAN bFound;

	/*1.Legality check*/
	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
		return FALSE;
	}

	if (pChPrioUpdateInfo->ch_num > MAX_NUM_OF_CHANNELS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"ch prio update channel number is out of range\n");
		return FALSE;
	}

	for (chListIdx = 0; chListIdx < pChPrioUpdateInfo->ch_num; chListIdx++) {
		if (pChPrioUpdateInfo->ch_list[chListIdx].priority > CHAN_PRIO_MAXIMUM
			&& pChPrioUpdateInfo->ch_list[chListIdx].priority != CHAN_PRIO_UNSPECIFIED) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"channel priority %d is out of range [%d - %d] and is not unspecified %d\n",
				pChPrioUpdateInfo->ch_list[chListIdx].priority, CHAN_PRIO_UNAVAILABLE, CHAN_PRIO_MAXIMUM, CHAN_PRIO_UNSPECIFIED);
			return FALSE;
		}
	}

	/*show channel priority update log*/
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"Channel priority is applied for update, applicant: %d update channels:\n",
	owner);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"====================================================================\n");
	for (chListIdx = 0; chListIdx < pChPrioUpdateInfo->ch_num; chListIdx++)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"Channel %3d : Priority = %d\n",
			pChPrioUpdateInfo->ch_list[chListIdx].channel, pChPrioUpdateInfo->ch_list[chListIdx].priority);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"====================================================================\n");

	/*2.update channel priority*/
	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);

	/*owner' priority check*/
	if (pChCtrl->CurChPrioOpOwner > owner) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"request ch prio owner:%d, current owner:%d, ignore this request\n",
			owner, pChCtrl->CurChPrioOpOwner);

		NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);
		return FALSE;
	}

	/*update channel priority, max channel priority and owner*/
	pChCtrl->CurChPrioOpOwner = owner;
	chListnum = pChCtrl->ChListNum;
	updateChListNum = pChPrioUpdateInfo->ch_num;
	if (pChCtrl->MaxChPrio == CHAN_PRIO_UNSPECIFIED) {
		pChCtrl->MaxChPrio = 0;
		for (chListIdx = 0; chListIdx < chListnum; chListIdx++) {
			bFound = FALSE;

			for (updateChListIdx = 0; updateChListIdx < updateChListNum; updateChListIdx++) {
				if (pChCtrl->ChList[chListIdx].Channel == pChPrioUpdateInfo->ch_list[updateChListIdx].channel) {
					pChCtrl->ChList[chListIdx].Priority = pChPrioUpdateInfo->ch_list[updateChListIdx].priority;
					bFound = TRUE;
					pChCtrl->MaxChPrio = pChCtrl->MaxChPrio < pChCtrl->ChList[chListIdx].Priority ?
										pChCtrl->ChList[chListIdx].Priority : pChCtrl->MaxChPrio;
					break;
				}
			}

			if (bFound == FALSE)
				pChCtrl->ChList[chListIdx].Priority = CHAN_PRIO_UNAVAILABLE;
		}

	} else {

		for (chListIdx = 0; chListIdx < chListnum; chListIdx++) {
			for (updateChListIdx = 0; updateChListIdx < updateChListNum; updateChListIdx++) {
				if (pChCtrl->ChList[chListIdx].Channel == pChPrioUpdateInfo->ch_list[updateChListIdx].channel) {
					pChCtrl->ChList[chListIdx].Priority = pChPrioUpdateInfo->ch_list[updateChListIdx].priority;
					pChCtrl->MaxChPrio = pChCtrl->MaxChPrio < pChCtrl->ChList[chListIdx].Priority ?
										pChCtrl->ChList[chListIdx].Priority : pChCtrl->MaxChPrio;
					break;
				}
			}
		}
	}

	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	/*3.refresh channel list and capability*/
	BuildChannelList(pAd, wdev);

	return TRUE;
}

/**
* ShowChPrioInfo - show runtime channel priority info (current owner)
* @pAd: pointer of the RTMP_ADAPTER
* @arg: Not currently in use
**/
INT32 ShowChPrioInfo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev == NULL)
		MTWF_PRINT("Get Wdev Fail!");
	else {
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

		/*Legality check*/
		if (pChCtrl == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
			return FALSE;
		}

		/*get lock,to avoid being interrupted by other processes*/
		NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);

		if (pChCtrl->ChListNum == 0) {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d\n ",
				BandIdx,
				pChCtrl->ChListNum);
		} else {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m\n", BandIdx);
			MTWF_PRINT("ChannelListNum = %d\n", pChCtrl->ChListNum);
			MTWF_PRINT("ChGrpABandEn = %d\n Channel list information:\n",
				pChCtrl->ChGrpABandEn);
			MTWF_PRINT("CurChPrioOpOwner = %d\n", pChCtrl->CurChPrioOpOwner);
			MTWF_PRINT("CurMaxChPrio = %d\n", pChCtrl->MaxChPrio);
		}

		/*unlock channel priority operation lock*/
		NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);
	}
	return TRUE;
}

/**
* GetMaxChannelPriority - get the maximum channel priority
* @pAd: pointer of the RTMP_ADAPTER
**/
UCHAR GetMaxChannelPriority(
	IN struct _RTMP_ADAPTER *pAd
)
{
	UCHAR MaxChPrio = 0;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	/*1.Legality check*/
	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
		return FALSE;
	}

	/*get lock,to avoid being interrupted by other processes*/
	NdisAcquireSpinLock(&pChCtrl->ChPrioOpLock);

	MaxChPrio = pChCtrl->MaxChPrio;

	/*unlock channel priority operation lock*/
	NdisReleaseSpinLock(&pChCtrl->ChPrioOpLock);

	return MaxChPrio;
}

/**
* UpdatePreferChannel - Update prefer channel in runtime channel priority
* @pAd: pointer of the RTMP_ADAPTER
* @wdev: pointer of the wifi_dev
* @ch_list: channel list
* @ch_num: channel number of channel list
* @owner: owner of this channel priority update request
**/
BOOLEAN UpdatePreferChannel(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	UCHAR ch_list[],
	UCHAR ch_num,
	UCHAR owner)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	UCHAR chListIdx, chListnum, updateChListIdx, updateChListNum;
	struct _CH_PRIO_INFO ChPrioUpdateInfo = {0};
	BOOLEAN find_ch;

	/*Legality check*/
	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
		return FALSE;
	}

	if (ch_num > MAX_NUM_OF_CHANNELS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
			"ch prio update channel number is out of range\n");
		return FALSE;
	}

	/*Convert to absolute priority*/
	chListnum = pChCtrl->ChListNum;
	updateChListNum = ch_num;
	ChPrioUpdateInfo.ch_num = chListnum;
	for (chListIdx = 0; chListIdx < chListnum; chListIdx++) {
		find_ch = FALSE;

		for (updateChListIdx = 0; updateChListIdx < updateChListNum; updateChListIdx++) {
			if (pChCtrl->ChList[chListIdx].Channel == ch_list[updateChListIdx]) {
				/*for a preferred channel that is unavailable,
				notify and ignore the preference setting for this channel.*/
				if (pChCtrl->ChList[chListIdx].Priority == CHAN_PRIO_UNAVAILABLE) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
						"channel %d, is unavailable, can't set as prefer channel.\n",
						pChCtrl->ChList[chListIdx].Channel);
						break;

				} else {
					/*for a preferred channel that is available,
					set the priority to medium*/
					ChPrioUpdateInfo.ch_list[chListIdx].channel = pChCtrl->ChList[chListIdx].Channel;
					ChPrioUpdateInfo.ch_list[chListIdx].priority = CH_PRIO_MEDIUM;

					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						"Channel %d, set Prio%d.\n",
						ChPrioUpdateInfo.ch_list[chListIdx].channel,
						ChPrioUpdateInfo.ch_list[chListIdx].priority);
					find_ch = TRUE;
					break;
				}
			}
		}

		if (!find_ch) {
			/*for an unavailable channel,
			the priority should remain as unavailable*/
			if (pChCtrl->ChList[chListIdx].Priority == CHAN_PRIO_UNAVAILABLE) {
				ChPrioUpdateInfo.ch_list[chListIdx].channel = pChCtrl->ChList[chListIdx].Channel;
				ChPrioUpdateInfo.ch_list[chListIdx].priority = CHAN_PRIO_UNAVAILABLE;
			} else {
				/*for an available channel,
				set the priority to low*/
				ChPrioUpdateInfo.ch_list[chListIdx].channel = pChCtrl->ChList[chListIdx].Channel;
				ChPrioUpdateInfo.ch_list[chListIdx].priority = CH_PRIO_LOW;
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"Channel %d, set Prio%d.\n",
					ChPrioUpdateInfo.ch_list[chListIdx].channel,
					ChPrioUpdateInfo.ch_list[chListIdx].priority);
			}
		}
	}

	/*Update absolute priority
	to runtime channel priority*/
	if (!UpdateChannelPriority(pAd, wdev, &ChPrioUpdateInfo, owner)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"update abs prio to runtime prio failed!\n");
		return FALSE;
	}

	return TRUE;
}
