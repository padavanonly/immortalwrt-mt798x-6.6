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
	greenap.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/


#ifndef __GREENAP_H__
#define __GREENAP_H__


#ifdef GREENAP_SUPPORT
struct greenap_ctrl;
struct greenap_on_off_ctrl;
VOID greenap_init(
	struct _RTMP_ADAPTER *ad);
VOID greenap_show(
	struct _RTMP_ADAPTER *ad);
BOOLEAN greenap_check_when_if_down_up(
	struct _RTMP_ADAPTER *ad);
BOOLEAN greenap_check_when_ap_bss_change(
	struct _RTMP_ADAPTER *ad);
VOID greenap_check_peer_connection_at_link_up_down(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev);
VOID greenap_suspend(
	struct _RTMP_ADAPTER *ad,
	UINT32 reason);
VOID greenap_resume(
	struct _RTMP_ADAPTER *ad,
	UINT32 reason);
VOID greenap_set_capability(
	struct _RTMP_ADAPTER *ad,
	BOOLEAN greenap_cap);
BOOLEAN greenap_get_capability(
	struct _RTMP_ADAPTER *ad);
VOID greenap_proc(
	struct _RTMP_ADAPTER *ad,
	BOOLEAN greenap_cap_on);

VOID enable_greenap(
	struct _RTMP_ADAPTER *ad,
	struct greenap_on_off_ctrl *greenap_on_off);
VOID disable_greenap(
	struct _RTMP_ADAPTER *ad,
	struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv2(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv2(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv1(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv1(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */


#endif /* __GREENAP_H__ */
