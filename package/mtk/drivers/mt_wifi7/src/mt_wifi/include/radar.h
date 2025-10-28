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
    radar.h

    Abstract:
     CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#ifndef __RADAR_H__
#define __RADAR_H__

struct freq_cfg;

#define DEFAULT_CAL_BUF_TIME	60
#define DEFAULT_CAL_BUF_TIME_MAX	0x10000
#define RDD_CHECK_NOP_BY_WDEV 0
/* RESTRICTION_BAND_1: 5600MHz ~ 5650MHz */
#define RESTRICTION_BAND_1(_pAd, __Channel, _BW)												\
	(_BW >= BW_40 ?						\
	 ((__Channel >= 116) && (__Channel <= 128)) :	\
	 ((__Channel >= 120) && (__Channel <= 128)))

#define RESTRICTION_BAND_KOREA(_pAd, __Channel, _BW)												\
	(_BW >= BW_80 ? 				\
	((__Channel >= 116) && (__Channel <= 128)) :	\
	(_BW >= BW_40 ?						\
	((__Channel >= 124) && (__Channel <= 128)) :	\
	(__Channel == 128)))

#define IS_DOT11_H_RADAR_STATE(_pAd, _RadarState, __Channel, _pDot11h)		\
	((__Channel > 14)	\
	 && (_pAd->CommonCfg.bIEEE80211H == 1)	\
	 && RadarChannelCheck(_pAd, __Channel)	\
	 && _pDot11h->ChannelMode == _RadarState)

#ifdef MT_DFS_SUPPORT
#define IS_SUPPORT_MT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bDfsEnable == TRUE)

#define UPDATE_MT_ZEROWAIT_DFS_STATE(_pAd, _State) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState = _State; \
	}

#define UPDATE_MT_ZEROWAIT_DFS_Support(_pAd, _Enable) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.bZeroWaitSupport = _Enable; \
	}

#define IS_SUPPORT_MT_ZEROWAIT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bZeroWaitSupport == TRUE)

#define IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport == TRUE)

#define CHK_MT_ZEROWAIT_DFS_STATE(_pAd, __STATE) \
	((_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState == __STATE))

#define GET_MT_ZEROWAIT_DFS_STATE(_pAd) \
	((_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState))

#define UPDATE_MT_INIT_ZEROWAIT_MBSS(_pAd, _Enable) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.bInitMbssZeroWait = _Enable; \
	}

#define GET_MT_MT_INIT_ZEROWAIT_MBSS(_pAd) \
	((_pAd->CommonCfg.DfsParameter.bInitMbssZeroWait))

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define IS_SUPPORT_V10_DFS(_pAd) \
	((_pAd->CommonCfg.DfsParameter.bDFSV10Support == TRUE) \
	 && (_pAd->CommonCfg.DfsParameter.bDfsEnable == TRUE) \
	 && (_pAd->CommonCfg.bIEEE80211H == TRUE))

#define IS_DFS_V10_ACS_VALID(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bV10ChannelListValid == TRUE)
#define SET_DFS_V10_ACS_VALID(_pAd, valid) \
		(_pAd->CommonCfg.DfsParameter.bV10ChannelListValid = valid)
#endif

#endif /* MT_DFS_SUPPORT */

struct CSA_CHN_INFO {
	UINT8 new_ch;
	UINT8 new_bw;
	UINT8 new_cench1;
	UINT8 new_cench2;
};

/* 802.11H */
struct DOT11_H {
	/* 802.11H and DFS related params */
	UCHAR CSCount;  /*Channel switch counter */
	UCHAR CSPeriod;	/*Channel switch period (beacon count) */
	USHORT RDCount;	/*Radar detection counter, if RDCount >  cac_time, start to send beacons*/
	UCHAR ChannelMode;   /* Channel switch mode: Normal(0), Switch(1), Silence(2) */
	UCHAR org_ch;
	USHORT cac_time; /* CAC time */
	USHORT DfsZeroWaitChMovingTime;
	BOOLEAN bDFSIndoor;
	ULONG InServiceMonitorCount; /* unit: sec */
	ULONG CalBufTime;    /* A Timing buffer for befroe calibrations which generates Tx signals */
	UINT16 wdev_count;
	UINT32 csa_ap_bitmap;	/* csa ap index bitmap */
#ifdef CONFIG_MAP_SUPPORT
	BOOLEAN cac_not_required;
#endif
	BOOLEAN ChChangeCSA;
#ifdef DFS_ADJ_BW_ZERO_WAIT
	USHORT NOPCount; /* for BW160 zero wait, when NOP end with ch52~ch64, start CAC and prepare to enlarge BW to 160 */
#endif
	RALINK_TIMER_STRUCT CSAEventTimer;		/*FW can not send CSA event to Host*/
	RALINK_TIMER_STRUCT PPCSAEventTimer;		/*FW can not send CSA event to Host*/
	//TGX_TEST
	UINT16 MaxChannelSwitchTime;
	struct CSA_CHN_INFO csa_chn_info;
	struct MLO_CSA_INFO mlo_csa_info;        /* Receive from other bands*/
	BOOLEAN disconn_after_ch_switch;
#ifdef ZERO_PKT_LOSS_SUPPORT
	INT ChnlSwitchState;
	UINT8 ChannelSwitchTriggerCSACount;	/*User defined Channel Switch trigger count*/
	RALINK_TIMER_STRUCT CSALastBcnTxEventTimer;
	RALINK_TIMER_STRUCT ChnlSwitchStaNullDataTxTimer;
	RALINK_TIMER_STRUCT ChnlSwitchStaNullAckWaitTimer;
	UCHAR CSA0EventApidx;	/*save apidx rcvd in CSA event , to be used later in*/
#endif /*ZERO_PKT_LOSS_SUPPORT*/
};

#ifdef MT_DFS_SUPPORT
enum {
	DFS_DEDICATED_ZERO_WAIT_DISABLED = 0,
	DFS_DEDICATED_ZERO_WAIT_ENABLED,
	DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED,
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	DFS_DEDICATED_ZERO_WAIT_V10_SUPPORT,
#endif
	DFS_DEDICATED_ZERO_WAIT_DNMC,
	DFS_DEDICATED_ZERO_WAIT_BW80_ENABLE,
	DFS_SWBASED_ZERO_WAIT_BW160_ENABLE
};
#endif

#if defined(DFS_ADJ_BW_ZERO_WAIT)
#ifdef MT_DFS_SUPPORT
enum {
	DFS_NOT_ENABLE = 0,	/* AP channel not in ch36~ch128 */
	DFS_BW160_TX160RX160,	/* AP channel in ch36~ch64 BW160 */
	DFS_BW160_TX80RX160,	/* AP channel in BW160 but tx BW80 */
	DFS_BW160_TX80RX80,	/* AP channel in BW160 but hit radar condition*/
	DFS_BW160_TX0RX0,	/* AP channel in ch100~ch128 BW160 */
	DFS_BW80_TX80RX80,	/* AP channel in ch52~ch64 BW80 cac done*/
	DFS_BW80_TX80RX160,	/* AP channel in ch52~ch64 BW80 before cac*/
	DFS_BW80_TX0RX0,	/* AP channel in ch100~ch128 BW80 */
	DFS_RADAR_DETECTED,	/* AP channel hit radar condition */
	DFS_SET_CHANNEL
};
#endif
#endif

BOOLEAN dfs_get_outband_bw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN OUT PUCHAR phy_bw);

BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

BOOLEAN RadarChannelCheckWrapper160(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

BOOLEAN RadarChBwCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch,
	IN UCHAR			Bw);

BOOLEAN NeedDoDfsCac(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN DfsCacRestrictBandForCentralCh(/* Weather band channel: 5600 MHz - 5650 MHz */
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh);

VOID RadarStateCheck(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR ch);

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd);

UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode,
	IN struct wifi_dev *wdev);

#ifdef CONFIG_AP_SUPPORT
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev);

NTSTATUS DropRadarEventHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);
NTSTATUS Dot11HCntDownTimeoutAction(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
#ifdef ZERO_PKT_LOSS_SUPPORT
NTSTATUS LastBcnTxChannelSwitch(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);
NTSTATUS DisableZeroLossStaTraffic(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);
INT	Set_CHSWPeriod_Proc(IN	PRTMP_ADAPTER	pAd, IN	RTMP_STRING *arg);
INT	Set_ZeroLossStaAdd_Proc(IN	PRTMP_ADAPTER	pAd, IN	RTMP_STRING *arg);
INT	Set_ZeroLossStaRemove_Proc(IN	PRTMP_ADAPTER	pAd, IN	RTMP_STRING *arg);
INT	Set_STA_Tx_Unblock_Timeout_Proc(IN	PRTMP_ADAPTER	pAd, IN	RTMP_STRING *arg);
INT Show_ChannelSwitchTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_PerSTA_StopTx_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Show_ZeroLossStaList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */

VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR Channel);

INT Set_CSPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ChMovingTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BlockChReset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* wdev->pDot11H Initailization */
VOID UpdateDot11hForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN attach);
#ifdef MT_DFS_SUPPORT
INT set_radar_min_lpn_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_radar_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_radar_pls_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_radar_dbg_log_config_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_radar_test_pls_pattern_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif /* __RADAR_H__ */
