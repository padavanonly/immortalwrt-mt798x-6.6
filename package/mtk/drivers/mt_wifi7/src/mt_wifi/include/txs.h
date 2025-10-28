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
	txs.h

	Abstract:
    This file contains the definitions for TxS handling

	Revision History:
	Who			    When			What
	--------------	----------		------------------------------------------
	Febrina Wijaya	2023-11-15		Created

*/

#ifndef __TXS_H__
#define __TXS_H__

#include "wlan_tr.h"

#define TXS2H_DISABLED 0
#define TXS2H_ENABLED 1

#define TXS_TMR_DISABLED -1
#define TXS_TMR_DEFAULT 10

#define MAX_TXS_CB_NUM (PID_TXS_MAX - PID_TXS_MIN + 1)
#define TXS_CB_IDX_TO_PID(_idx) (_idx + PID_TXS_MIN)
#define PID_TO_TXS_CB_IDX(_idx) (_idx - PID_TXS_MIN)
#define IS_TXS_CB_LIST_FULL(_pTxsCbList) (((_pTxsCbList)->EntryCnt < MAX_TXS_CB_NUM) ? FALSE : TRUE)

#define TR_DELAY_TO_JIFFIES(_tr_delay_usecs) (_tr_delay_usecs / jiffies_to_usecs(1))

struct txs_callback_info_t {
	INT (*callback_func)(VOID *arg, struct txs_info_t *txs_info);
	VOID *callback_arg;
	UINT32 callback_arg_len;
	/* Callback info will be removed after timeout_cnt * 1s */
	/* To disable timeout cnt, set timeout_cnt = TXS_TMR_DISABLED*/
	INT8 timeout_cnt;
	/* container for storing txs result from hwifi */
	struct txs_info_t *txs_info;
};

struct TXS_CB_LIST_ENTRY {
	UINT8 pid;
	struct _RTMP_ADAPTER *pAd;
	struct txs_callback_info_t *Callback;
	ULONG enq_timestamp;
	struct list_head list;
};

struct TXS_CB_LIST {
	BOOLEAN isPidOccupied[MAX_TXS_CB_NUM];
	INT16 LastAssignedIdx; /* For circular pid assignment */
	UINT8 EntryCnt;
	NDIS_SPIN_LOCK Lock;
	struct list_head EntryHead;
};

VOID TxsStateMachineInit(
	IN struct _RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

NDIS_STATUS TxsCbListInit(struct physical_device *physical_dev);

VOID TxsCbListDestroy(struct physical_device *physical_dev);

NDIS_STATUS TxsInitCallbackInfo(
	struct _RTMP_ADAPTER *pAd,
	struct txs_callback_info_t **cb,
	INT (*cb_func)(VOID *, struct txs_info_t *),
	VOID **cb_arg,
	INT cb_arg_size);

VOID TxsFreeCallbackInfo(struct txs_callback_info_t *cb);

NDIS_STATUS TxsRemoveCbEntry(struct physical_device *ph_dev, UINT8 pid);

NDIS_STATUS TxsRetrieveCbEntry(
	struct TXS_CB_LIST *pTxsCbList,
	UINT8 pid,
	ULONG tr_delay,
	struct txs_callback_info_t **Callback,
	struct _RTMP_ADAPTER **pAd);

NDIS_STATUS TxsInsertCbEntry(struct _RTMP_ADAPTER *pAd, struct txs_callback_info_t *Callback, UINT8 *pid);

VOID mlme_wait_txs_action(IN struct _RTMP_ADAPTER *pAd, IN struct _MLME_QUEUE_ELEM *elem);

VOID WaitTxsTOPeriodicHandler(IN struct physical_device *physical_dev);

VOID TxsReceiveHandler(struct physical_device *ph_dev, struct txs_info_t txs_info);

#endif	/* __TXS_H__ */
