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
	rtmp_timer.h

    Abstract:
	Ralink Wireless Driver timer related data structures and delcarations

    Revision History:
	Who           When                What
	--------    ----------      ----------------------------------------------
	Name          Date                 Modification logs
	Shiang Tu    Aug-28-2008	init version

*/

#ifndef __RTMP_TIMER_H__
#define  __RTMP_TIMER_H__

#include "rtmp_os.h"

struct _RTMP_ADAPTER;

#ifdef LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(struct timer_list *_timer)
#else
#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)
#endif
#else /* LINUX */
#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)
#endif /* NONE */

#define GET_TIMER_FUNCTION(_func)				\
	(PVOID)rtmp_timer_##_func

/* ----------------- Timer Related MARCO ---------------*/
/* In some os or chipset, we have a lot of timer functions and will read/write register, */
/*   it's not allowed in Linux USB sub-system to do it ( because of sleep issue when */
/*  submit to ctrl pipe). So we need a wrapper function to take care it. */

#ifdef RTMP_TIMER_TASK_SUPPORT
typedef VOID(
	*RTMP_TIMER_TASK_HANDLE) (
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3);
#endif /* RTMP_TIMER_TASK_SUPPORT */

typedef struct _RALINK_TIMER_STRUCT {
	RTMP_OS_TIMER TimerObj;	/* Ndis Timer object */
	BOOLEAN Valid;		/* Set to True when call RTMPInitTimer */
	BOOLEAN State;		/* True if timer cancelled */
	BOOLEAN PeriodicType;	/* True if timer is periodic timer */
	BOOLEAN Repeat;		/* True if periodic timer */
	ULONG TimerValue;	/* Timer value in milliseconds */
	ULONG cookie;		/* os specific object */
	void *pAd;
	void *ph_dev;
	NDIS_SPIN_LOCK *timer_lock;
#ifdef RTMP_TIMER_TASK_SUPPORT
	RTMP_TIMER_TASK_HANDLE handle;
#endif				/* RTMP_TIMER_TASK_SUPPORT */
	VOID *pCaller;
} RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;

typedef struct _TIMER_FUNC_CONTEXT {
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	UCHAR BandIdx;
} TIMER_FUNC_CONTEXT, *PTIMER_FUNC_CONTEXT;


#ifdef RTMP_TIMER_TASK_SUPPORT
typedef struct _RTMP_TIMER_TASK_ENTRY_ {
	RALINK_TIMER_STRUCT *pRaTimer;
	struct _RTMP_TIMER_TASK_ENTRY_ *pNext;
} RTMP_TIMER_TASK_ENTRY;

#define TIMER_QUEUE_SIZE_MAX	128
typedef struct _RTMP_TIMER_TASK_QUEUE_ {
	unsigned int status;
	unsigned char *pTimerQPoll;
	RTMP_TIMER_TASK_ENTRY *pQPollFreeList;
	RTMP_TIMER_TASK_ENTRY *pQHead;
	RTMP_TIMER_TASK_ENTRY *pQTail;
} RTMP_TIMER_TASK_QUEUE;


INT RtmpTimerQThread(ULONG Context);


RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN struct _RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(
	IN struct _RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(struct _RTMP_ADAPTER *pAd);
void RtmpTimerQInit(struct _RTMP_ADAPTER *pAd);

#ifdef LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#define BUILD_TIMER_FUNCTION(_func) \
	void rtmp_timer_##_func(struct timer_list *_timer) \
	{ \
		PRALINK_TIMER_STRUCT _pTimer = from_timer(_pTimer, _timer, _timer); \
		RTMP_TIMER_TASK_ENTRY *_pQNode; \
		RTMP_ADAPTER *_pAd; \
		\
		_pTimer->handle = _func; \
		_pAd = (RTMP_ADAPTER *)_pTimer->pAd; \
		if (_pAd) {\
			_pQNode = RtmpTimerQInsert(_pAd, _pTimer); \
			if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT)) \
				RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ); \
		} else \
			RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ); \
	}
#else
#define BUILD_TIMER_FUNCTION(_func) \
	void rtmp_timer_##_func(unsigned long data) \
	{ \
		PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data; \
		RTMP_TIMER_TASK_ENTRY	*_pQNode; \
		RTMP_ADAPTER *_pAd; \
		\
		_pAd = (RTMP_ADAPTER *)_pTimer->pAd; \
		if (_pAd && IS_ASIC_CAP(_pAd, fASIC_CAP_MGMT_TIMER_TASK)) { \
			_pTimer->handle = _func; \
			_pQNode = RtmpTimerQInsert(_pAd, _pTimer); \
			if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT)) \
				RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ); \
		} else { \
			_func(NULL, (PVOID) _pTimer->cookie, NULL, _pTimer); \
			if (_pTimer->Repeat) \
				RTMP_OS_Add_Timer(&_pTimer->TimerObj, _pTimer->TimerValue); \
		}\
	}
#endif
#else /* LINUX */
#define BUILD_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)	\
{																			\
		PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data;\
		RTMP_TIMER_TASK_ENTRY	*_pQNode;			\
		RTMP_ADAPTER			*_pAd;			\
									\
		_pAd = (RTMP_ADAPTER *)_pTimer->pAd;			\
		if (IS_ASIC_CAP(_pAd, fASIC_CAP_MGMT_TIMER_TASK)) {	\
			_pTimer->handle = _func;			\
			_pQNode = RtmpTimerQInsert(_pAd, _pTimer);	\
			if ((_pQNode == NULL) &&			\
			(_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))\
				RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);\
		} else {\
			_func(NULL, (PVOID) _pTimer->cookie, NULL, _pTimer);\
			if (_pTimer->Repeat)				\
				RTMP_OS_Add_Timer(&_pTimer->TimerObj,	\
					_pTimer->TimerValue);		\
		} \
}
#endif /* NONE */
#else /* !RTMP_TIMER_TASK_SUPPORT */
#ifdef LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(struct timer_list *_timer)										\
{																			\
	PRALINK_TIMER_STRUCT	pTimer = from_timer(pTimer,_timer,TimerObj);				\
																			\
	_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer); 							\
	if (pTimer->Repeat)														\
		RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
}
#else
#define BUILD_TIMER_FUNCTION(_func)										\
	void rtmp_timer_##_func(unsigned long data)										\
	{																			\
		PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;				\
		\
		_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer);							\
		if (pTimer->Repeat)														\
			RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
	}
#endif
#else /* LINUX */
#define BUILD_TIMER_FUNCTION(_func)					\
	void rtmp_timer_##_func(unsigned long data)			\
	{								\
		PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;\
		\
		_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer);	\
		if (pTimer->Repeat)					\
			RTMP_OS_Add_Timer(&pTimer->TimerObj,		\
				pTimer->TimerValue);			\
	}
#endif /* NONE */
#endif /* RTMP_TIMER_TASK_SUPPORT */

DECLARE_TIMER_FUNCTION(MlmePeriodicExecTimer);
DECLARE_TIMER_FUNCTION(MlmeRssiReportExec);
DECLARE_TIMER_FUNCTION(AsicRxAntEvalTimeout);

#ifdef DOT11W_PMF_SUPPORT
DECLARE_TIMER_FUNCTION(PMF_SAQueryTimeOut);
DECLARE_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
DECLARE_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif /* DOT11N_DRAFT3 */

DECLARE_TIMER_FUNCTION(CMTimerExec);
DECLARE_TIMER_FUNCTION(APQuickResponeForRateUpExec);

#ifdef IDS_SUPPORT
DECLARE_TIMER_FUNCTION(RTMPIdsPeriodicExec);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
DECLARE_TIMER_FUNCTION(FT_KDP_InfoBroadcast);
#endif /* DOT11R_FT_SUPPORT */
#ifdef ZERO_PKT_LOSS_SUPPORT
DECLARE_TIMER_FUNCTION(CSALastBcnTxEventTimeout);
DECLARE_TIMER_FUNCTION(ChnlSwitchStaNullAckWaitTimeout);
DECLARE_TIMER_FUNCTION(ChnlSwitchStaNullDataTxTimeout);
#endif /*ZERO_PKT_LOSS_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
DECLARE_TIMER_FUNCTION(CSAEventTimeout);
DECLARE_TIMER_FUNCTION(PPCSAEventTimeout);

#ifdef CONFIG_STA_SUPPORT
DECLARE_TIMER_FUNCTION(StaQuickResponeForRateUpExec);
DECLARE_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc);

#ifdef RTMP_PCI_SUPPORT
DECLARE_TIMER_FUNCTION(PsPollWakeExec);
DECLARE_TIMER_FUNCTION(RadioOnExec);
#endif /* RTMP_PCI_SUPPORT */

#ifdef SWACI_MECHANISM
DECLARE_TIMER_FUNCTION(SetRcpiTestMode);
#endif

#ifdef DOT11R_FT_SUPPORT
DECLARE_TIMER_FUNCTION(FT_OTA_AuthTimeout);
DECLARE_TIMER_FUNCTION(FT_OTD_TimeoutAction);
#endif /* DOT11R_FT_SUPPORT */


#endif /* CONFIG_STA_SUPPORT */

#ifdef WSC_INCLUDED
DECLARE_TIMER_FUNCTION(WscEAPOLTimeOutAction);
DECLARE_TIMER_FUNCTION(Wsc2MinsTimeOutAction);
DECLARE_TIMER_FUNCTION(WscUPnPMsgTimeOutAction);
DECLARE_TIMER_FUNCTION(WscM2DTimeOutAction);
DECLARE_TIMER_FUNCTION(WscPBCTimeOutAction);
#ifdef CON_WPS
DECLARE_TIMER_FUNCTION(WscScanDoneCheckTimeOutAction);
#endif /*CON_WPS*/
#ifdef WSC_STA_SUPPORT
DECLARE_TIMER_FUNCTION(WscPINTimeOutAction);
#endif
DECLARE_TIMER_FUNCTION(WscScanTimeOutAction);
DECLARE_TIMER_FUNCTION(WscProfileRetryTimeout);
#ifdef WSC_LED_SUPPORT
DECLARE_TIMER_FUNCTION(WscLEDTimer);
DECLARE_TIMER_FUNCTION(WscSkipTurnOffLEDTimer);
#endif /* WSC_LED_SUPPORT */
#ifdef LED_CONTROL_SUPPORT
DECLARE_TIMER_FUNCTION(LEDControlTimer);
#endif
#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(WscUpdatePortCfgTimeout);
#ifdef WSC_V2_SUPPORT
DECLARE_TIMER_FUNCTION(WscSetupLockTimeout);
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef CONFIG_HOTSPOT
#ifdef CONFIG_STA_SUPPORT
DECLARE_TIMER_FUNCTION(GASResponseTimeout);
DECLARE_TIMER_FUNCTION(GASCBDelayTimeout);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(PostReplyTimeout);
#endif /* CONFIG_AP_SUPPORT */
#endif /* CONFIG_HOTSPOT */


#ifdef CONFIG_ATE
DECLARE_TIMER_FUNCTION(ATEPeriodicExec);
#endif /* CONFIG_ATE */

#ifdef BACKGROUND_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(BackgroundScanTimeout);
DECLARE_TIMER_FUNCTION(dedicated_rx_hist_scan_timeout);
#ifdef IPI_MAINRADIO_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(mainRadio_rx_hist_scan_timeout);
#endif
/*DECLARE_TIMER_FUNCTION(DfsZeroWaitTimeout);*/
DECLARE_TIMER_FUNCTION(dfs_zero_wait_ch_init_timeout);
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(AutoChSelScanTimeout);
DECLARE_TIMER_FUNCTION(AutoChSelRestoreTimeout);
#endif/* CONFIG_AP_SUPPORT */

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
DECLARE_TIMER_FUNCTION(ch_switch_monitor_timeout);
#endif
DECLARE_TIMER_FUNCTION(ChOpTimeout);

#ifdef DOT11_HE_AX
#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(trigger_timer_callback);
#endif
#ifdef CONFIG_STA_SUPPORT
DECLARE_TIMER_FUNCTION(notify_timer_callback);
#endif

#ifdef ERR_RECOVERY
DECLARE_TIMER_FUNCTION(ser_l1_timeout);
#endif

#endif

#ifdef CFG_SUPPORT_CSI
DECLARE_TIMER_FUNCTION(csi_report_timer_handle);
#endif

#endif /* __RTMP_TIMER_H__ */

