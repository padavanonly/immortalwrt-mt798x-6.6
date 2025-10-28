/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2018, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice doeas not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ser_mngr.h

    Abstract:
    ser manager

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __SER_MNGR_H__
#define __SER_MNGR_H__

#include "rt_config.h"
#include "ser_cmm.h"

#define all_pAd(i, pAd_list, pAd_num, pAd)		\
		for (i = 0, pAd = pAd_list[i];		\
		     i < pAd_num && pAd;		\
		     i += 1, pAd = pAd_list[i])

#define SER_LV_1_0_TIMEOUT                     5000 /* 5sec */

enum SER_LEVEL {
	SER_LV_0_0 = 0,
	SER_LV_0_5 = 5,
	SER_LV_1_0 = 10,
#ifdef MTK_FE_RESET_RECOVER
	FE_RESET = 50,
	FE_WDMA_RESET = 51,
#endif
	SER_LV_10_0 = 100,
	SER_LV_END,
};

enum SER_STATE_1_0 {
	SER_LV_1_0_STATE_STOP_IDLE	= ERR_RECOV_STAGE_STOP_IDLE,
	SER_LV_1_0_STATE_STOP_PDMA0	= ERR_RECOV_STAGE_STOP_PDMA0,
	SER_LV_1_0_STATE_RESET_PDMA0	= ERR_RECOV_STAGE_RESET_PDMA0,
	SER_LV_1_0_STATE_STOP_IDLE_DONE	= ERR_RECOV_STAGE_STOP_IDLE_DONE,
	SER_LV_1_0_STATE_WAIT_N9_NORMAL	= ERR_RECOV_STAGE_WAIT_N9_NORMAL,
	SER_LV_1_0_STATE_EVENT_REENTRY	= ERR_RECOV_STAGE_EVENT_REENTRY,
#ifdef MTK_FE_RESET_RECOVER
	SER_LV_1_0_STATE_STOP_PDMA0_PRE = ERR_RECOV_STAGE_STOP_PDMA0_FE_PRE,
#endif
	SER_LV_1_0_STATE_STATE_NUM	= ERR_RECOV_STAGE_STATE_NUM
};

enum SER_LV_1_0_EVT {
	SER_LV_1_0_EVT_STOP_PDMA	= ERROR_DETECT_STOP_PDMA,
	SER_LV_1_0_EVT_RESET_DONE	= ERROR_DETECT_RESET_DONE,
	SER_LV_1_0_EVT_RECOVERY_DONE	= ERROR_DETECT_RECOVERY_DONE,
	SER_LV_1_0_EVT_N9_NORMAL_STATE	= ERROR_DETECT_N9_NORMAL_STATE,
#ifdef MTK_FE_RESET_RECOVER
	SER_LV_1_0_EVT_STOP_PDMA_PRE	= ERROR_DETECT_STOP_PDMA_PRE,
#endif
};

#ifdef MTK_FE_RESET_RECOVER
#define HAS_SER_LV_1_0_EVT(event) ((event) & (SER_LV_1_0_EVT_STOP_PDMA |	\
										SER_LV_1_0_EVT_RESET_DONE |			\
										SER_LV_1_0_EVT_RECOVERY_DONE |		\
										SER_LV_1_0_EVT_N9_NORMAL_STATE |	\
										SER_LV_1_0_EVT_STOP_PDMA_PRE))
#else
#define HAS_SER_LV_1_0_EVT(event) ((event) & (SER_LV_1_0_EVT_STOP_PDMA |	\
										SER_LV_1_0_EVT_RESET_DONE |			\
										SER_LV_1_0_EVT_RECOVERY_DONE |		\
										SER_LV_1_0_EVT_N9_NORMAL_STATE))
#endif

enum SER_LV_1_0_INT {
	SER_LV_1_0_INT_PDMA0_STOP_DONE	= MCU_INT_PDMA0_STOP_DONE,
	SER_LV_1_0_INT_PDMA0_INIT_DONE	= MCU_INT_PDMA0_INIT_DONE,
	SER_LV_1_0_INT_PDMA0_RECV_DONE	= MCU_INT_PDMA0_RECOVERY_DONE,
#ifdef MTK_FE_RESET_RECOVER
	SER_LV_1_0_FE_WIFI_RESET_DONE	= FE_WIFI_RESET_DONE,
#endif
};

struct ser_event_cb_t {
	DL_LIST list;
	UINT32 chip_id;
	UINT8 ser_level;
	UINT32 ser_event;
	UINT32 hw_id;
};


struct ser_cb_t {
	DL_LIST list;
	UINT32 chip_id;
	UINT8 ser_level;
	UINT8 ser_state;
	UINT32 ser_event;
	UINT32 hw_id;
#ifdef MTK_FE_RESET_RECOVER
	BOOLEAN need_reset;
#endif
	UINT32 ser_times[SER_TIME_ID_END];
	RALINK_TIMER_STRUCT ser_timer;
};

typedef void (*ser_handler_t)(struct ser_cb_t *ser);
typedef void (*ser_dump_ser_stat_t)(struct _RTMP_ADAPTER *ad, UINT8 dump_all);


struct ser_tbl_t {
	UINT32 chip_id;
	void (*ser_1_0)(struct ser_cb_t *ser);
	void (*ser_0_5)(struct ser_cb_t *ser);
	void (*ser_0_0)(struct ser_cb_t *ser);
	void (*ser_dump_ser_stat)(struct _RTMP_ADAPTER *ad, UINT8 dump_all);
#ifdef MTK_FE_RESET_RECOVER
	void (*fe_fe_reset)(struct ser_cb_t *ser);
	void (*fe_fe_wdma_reset)(struct ser_cb_t *ser);
#endif
	void (*ser_10_0)(struct ser_cb_t *ser);
};

struct ser_ctrl_t {
	/* ser thread */
	NDIS_SPIN_LOCK ser_lock;
	RTMP_OS_TASK ser_task;
	UINT32 ser_func_state;
	/* ser event queue */
	NDIS_SPIN_LOCK event_q_lock;
	DL_LIST event_q;
	/* ser_cb list */
	NDIS_SPIN_LOCK cb_lsit_lock;
	DL_LIST ser_cb_list;
	UINT32 event_times;
};

int ser_event_enq(
	UINT32 chip_id,
	UINT32 ser_level,
	UINT32 ser_event,
	UINT32 hw_id);
UINT8 ser_get_ser_state(
	struct ser_cb_t *ser_cb);
void ser_tans_ser_state(
	struct ser_cb_t *ser_cb,
	UINT8 ser_state);
void ser_ack_event(
	struct ser_cb_t *ser_cb,
	UINT32 ack_event);
UINT8 ser_get_curr_state(
	struct _RTMP_ADAPTER *ad);
UINT8 ser_is_idle_state(
	struct _RTMP_ADAPTER *ad);
UINT32 ser_get_time(
	void);
UINT8 _ser_dump_timelog(
	struct ser_cb_t *ser_cb);
UINT8 ser_get_hwid_by_pAd(
	struct _RTMP_ADAPTER *ad,
	UINT32 *hw_id);
int ser_get_all_pAd_by_chipid_hwid(
	struct _RTMP_ADAPTER **ret_pAd_list,
	UINT32 chip_id,
	UINT32 hw_id);
#ifdef MTK_FE_RESET_RECOVER
struct _RTMP_ADAPTER *ser_get_fe_pAd_by_mainpAd(
	struct _RTMP_ADAPTER *ad);
#endif

void ser_dump_ser_stat(
	struct _RTMP_ADAPTER *ad,
	UINT8 dump_all);
void ser_dump_timelog(
	struct _RTMP_ADAPTER *ad,
	UINT8 ser_lv);

VOID ser_l1_timeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);

UINT32 ser_get_t0(
	struct _RTMP_ADAPTER *ad);
INT ser_mngr_init(
	void);

#ifdef WF_RESET_SUPPORT
INT ser_mngr_free_event_cb(
	void);
#endif
INT ser_mngr_exit(
	void);

#endif /*__SER_BMG_H__*/
