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
    t2lm

    Abstract:
    t2lm

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _MLR_H_
#define _MLR_H_
#include "rtmp.h"
#include "rt_config.h"
#ifdef MLR_SUPPORT

#define MLR_MODE_NOT_SUPPORT	0x0     /* 0      --> Not support */
#define MLR_MODE_MLR_V1		0x1     /* BIT(0) --> MLR MODE V1 */
#define MLR_MODE_MLR_V2		0x3     /* BIT(0|1) --> MLR MODE V2 */
#define MLR_MODE_MLR_PLUS	0x4     /* BIT(2) --> MLRP MODE   */
#define MLR_MODE_ALR		0x8     /* BIT(3) --> ALR MODE    */
#define MLR_MODE_DUAL_CTS	0x10    /* BIT(4) --> Dual CTS    */
#define MLR_MODE_LOW_RCPI	0x20    /* BIT(5) --> RCPI IS LOW    */

INT set_mlr_ctl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_mlr_ver(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_mlr_force_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
UINT8 mlr_determine_version(RTMP_ADAPTER *pAd, struct _vendor_ie_cap *vendor_ie, UINT8 *state);

enum ENUM_MLR_STATE_T {
	MLR_STATE_IDLE  = 0,       /* MLR not start */
	MLR_STATE_START = 1,       /* MLR start     */
	MLR_STATE_NUM              /* Max num = 2   */
};

#endif /* MLR_SUPPORT */
#endif /*_MLR_H_*/
