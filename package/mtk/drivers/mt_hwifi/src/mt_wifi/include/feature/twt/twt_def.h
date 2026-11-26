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
    twt_core.h

    Abstract:
    Support twt mlme

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _TWT_DEF_H_
#define _TWT_DEF_H_

#ifdef MT_MAC

//#define TWT_AGRT_ALLOC_SIM			1
#define TWT_TX_ACTION_BY_TXS			1
#define TWT_WAIT_ACT_TIME                       5000 /* 5sec */

/* resource control */
#define TWT_STATE_SW_OCCUPIED                   1

/* TWT definitions for hw */
#define TWT_HW_AGRT_MAX_NUM                     16
#define TWT_HW_BTWT_MAX_MEMBER_CNT              8
#define TWT_CMD_MAX_NUM                         2
#define TWT_MIN_SP_DURATION					64 /* 16ms */

#define TWT_TYPE_INDIVIDUAL                     0
#define TWT_TYPE_BTWT                           1

/* 16TU = 16*1024usec*/
#define TWT_TSF_ALIGNMNET_UINT                  (16 * 1024)

#define SCH_LINK                                0
#define USCH_LINK                               1
#define SCH_LINK_NUM                            2

#define NTBBT_BEFORE_REJECT                     100
#define ALL_BTWT_ID                             255


#endif /* MT_MAC */

#endif /* _TWT_DEF_H_ */
