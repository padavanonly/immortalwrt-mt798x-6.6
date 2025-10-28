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
    ap_mbss.h

    Abstract:
    Support multi-BSS function.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Sample Lin  01-02-2007      created
*/


#define MBSS_GET(__mbss) ((struct _BSS_STRUCT *)__mbss)

/* Public function list */
INT	Show_MbssInfo_Display_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID MBSS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);

VOID MBSS_Remove(RTMP_ADAPTER *pAd);

INT32 RT28xx_MBSS_IdxGet(
	IN PRTMP_ADAPTER	pAd,
	IN PNET_DEV			pDev);

#ifdef MT_MAC
INT ext_mbss_hw_cr_enable(PNET_DEV pDev);
INT ext_mbss_hw_cr_disable(PNET_DEV pDev);
#endif

VOID MBSS_Reconfig_Init(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID MBSS_Reconfig_Deinit(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
BOOLEAN MBSS_Reconfig_Is_Idle(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID MBSS_Reconfig_State_transition(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, enum bss_reconfig_state next_state, UINT timeout);
INT MBSS_Reconfig_flow_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MBSS_Reconfig_flow_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID MBSS_Reconfig_SM(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 reconfig_state);
NTSTATUS MBSS_Reconfig_SM_Handler(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
