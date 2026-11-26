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
	txs.c
*/

/**
 * @addtogroup tx_rx_path Wi-Fi
 * @{
 * @name TxS Control API
 * @{
 */

#include "rt_config.h"
#include "txs.h"

VOID TxsStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, TXS_FSM_MAX_STATE, TXS_FSM_MAX_MSG,
		(STATE_MACHINE_FUNC)Drop, TXS_FSM_IDLE, TXS_MACHINE_BASE);
	StateMachineSetAction(S, TXS_FSM_IDLE, TXS_FSM_WAIT_TXS_RES, (STATE_MACHINE_FUNC)mlme_wait_txs_action);
}

NDIS_STATUS TxsCbListInit(struct physical_device *physical_dev)
{
	struct TXS_CB_LIST *pTxsCbList = &physical_dev->TxsCbList;

	NdisAllocateSpinLock(NULL, &pTxsCbList->Lock);
	NdisAcquireSpinLock(&(pTxsCbList->Lock));
	INIT_LIST_HEAD(&pTxsCbList->EntryHead);
	pTxsCbList->EntryCnt = 0;
	pTxsCbList->LastAssignedIdx = -1;
	NdisReleaseSpinLock(&(pTxsCbList->Lock));
	return NDIS_STATUS_SUCCESS;
}

VOID TxsCbListDestroy(struct physical_device *physical_dev)
{
	struct TXS_CB_LIST *pTxsCbList = &physical_dev->TxsCbList;
	struct TXS_CB_LIST_ENTRY *elem = NULL;
	struct TXS_CB_LIST_ENTRY *temp_elem = NULL;
	struct txs_callback_info_t *Callback;

	NdisAcquireSpinLock(&(pTxsCbList->Lock));

	/* Check if callback list destroyed before initialized */
	if (pTxsCbList->EntryHead.next == NULL) {
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		return;
	}

	list_for_each_entry_safe(elem, temp_elem, &pTxsCbList->EntryHead, list) {
		Callback = elem->Callback;
		os_free_mem(Callback->callback_arg);
		os_free_mem(Callback);
		list_del(&elem->list);
		os_free_mem(elem);
	}
	pTxsCbList->EntryCnt = 0;
	INIT_LIST_HEAD(&pTxsCbList->EntryHead);
	NdisReleaseSpinLock(&(pTxsCbList->Lock));
	NdisFreeSpinLock(&(pTxsCbList->Lock));
}

NDIS_STATUS TxsInitCallbackInfo(
	RTMP_ADAPTER *pAd,
	struct txs_callback_info_t **cb,
	INT (*cb_func)(VOID *, struct txs_info_t *),
	VOID **cb_arg,
	INT cb_arg_size)
{
	os_alloc_mem(pAd,  (UCHAR **)cb, sizeof(struct txs_callback_info_t));
	if (*cb == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Fail to allocate mem for callback\n");
		return NDIS_STATUS_FAILURE;
	}

	os_alloc_mem(pAd,  (UCHAR **)cb_arg, cb_arg_size);
	if (*cb_arg == NULL) {
		os_free_mem(*cb);
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Fail to allocate mem for callback arg\n");
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(*cb, sizeof(struct txs_callback_info_t));
	os_zero_mem(*cb_arg, cb_arg_size);

	(*cb)->timeout_cnt = TXS_TMR_DEFAULT;
	(*cb)->callback_func = cb_func;
	(*cb)->callback_arg_len = cb_arg_size;
	(*cb)->callback_arg = *cb_arg;
	(*cb)->txs_info = NULL;

	return NDIS_STATUS_SUCCESS;
}

VOID TxsFreeCallbackInfo(struct txs_callback_info_t *cb)
{
	if (cb) {
		if (cb->callback_arg)
			os_free_mem(cb->callback_arg);
		os_free_mem(cb);
	}
}

NDIS_STATUS AcquirePid(struct TXS_CB_LIST *pTxsCbList, UINT8 *pid)
{
	UINT8 i = 0;

	if (!pTxsCbList) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"pTxsCbList NULL\n");
		return NDIS_STATUS_FAILURE;
	}
	for (i = pTxsCbList->LastAssignedIdx+1; i < MAX_TXS_CB_NUM; ++i) {
		if (!pTxsCbList->isPidOccupied[i]) {
			pTxsCbList->isPidOccupied[i] = TRUE;
			*pid = TXS_CB_IDX_TO_PID(i);
			pTxsCbList->LastAssignedIdx = i;
			return NDIS_STATUS_SUCCESS;
		}
	}
	for (i = 0; i <= pTxsCbList->LastAssignedIdx; ++i) {
		if (!pTxsCbList->isPidOccupied[i]) {
			pTxsCbList->isPidOccupied[i] = TRUE;
			*pid = TXS_CB_IDX_TO_PID(i);
			pTxsCbList->LastAssignedIdx = i;
			return NDIS_STATUS_SUCCESS;
		}
	}
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS ReleasePid(struct TXS_CB_LIST *pTxsCbList, UINT8 pid)
{
	UINT8 idx = PID_TO_TXS_CB_IDX(pid);

	if (idx >= MAX_TXS_CB_NUM) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"TxsCbList idx invalid: pid = %d, idx = %d\n", pid, idx);
		return NDIS_STATUS_FAILURE;
	}
	if (!pTxsCbList) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"pTxsCbList NULL\n");
		return NDIS_STATUS_FAILURE;
	}
	if (!pTxsCbList->isPidOccupied[idx]) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Attempt to release unoccupied PID: %d\n", pid);
		return NDIS_STATUS_FAILURE;
	}
	pTxsCbList->isPidOccupied[idx] = FALSE;
	return NDIS_STATUS_SUCCESS;
}

/*Assigned PID will be returned to caller*/
NDIS_STATUS TxsInsertCbEntry(RTMP_ADAPTER *pAd, struct txs_callback_info_t *Callback, UINT8 *pid)
{
	struct TXS_CB_LIST *pTxsCbList = &pAd->physical_dev->TxsCbList;
	struct TXS_CB_LIST_ENTRY *pElem;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;

	NdisAcquireSpinLock(&(pTxsCbList->Lock));
	if (IS_TXS_CB_LIST_FULL(pTxsCbList)) {
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Can't insert entry: TxsCbList is full\n");
		return NDIS_STATUS_FAILURE;
	}

	status = AcquirePid(pTxsCbList, pid);
	if (status == NDIS_STATUS_FAILURE) {
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Fail to acquire PID\n");
		return NDIS_STATUS_FAILURE;
	}
	os_alloc_mem(NULL, (UCHAR **)&pElem, sizeof(struct TXS_CB_LIST_ENTRY));
	if (pElem == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"Mlme Queue Entry buffer alloc fail, Buffer size %llu\n",
				sizeof(struct TXS_CB_LIST_ENTRY));
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		return NDIS_STATUS_RESOURCES;
	}
	os_zero_mem(pElem, sizeof(struct TXS_CB_LIST_ENTRY));
	list_add_tail(&pElem->list, &pTxsCbList->EntryHead);

	++pTxsCbList->EntryCnt;

	pElem->pid = *pid;
	pElem->Callback = Callback;
	pElem->pAd = pAd;
	RTMP_GetCurrentSystemTick(&pElem->enq_timestamp);
	NdisReleaseSpinLock(&(pTxsCbList->Lock));
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS TxsRemoveCbEntry(struct physical_device *ph_dev, UINT8 pid)
{
	struct TXS_CB_LIST *pTxsCbList = NULL;
	struct TXS_CB_LIST_ENTRY *elem = NULL;
	struct TXS_CB_LIST_ENTRY *temp_elem = NULL;

	if (ph_dev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"(Remove txs callback entry failed: ph_dev is NULL)\n");
		return NDIS_STATUS_FAILURE;
	}

	pTxsCbList = &ph_dev->TxsCbList;

	NdisAcquireSpinLock(&(pTxsCbList->Lock));
	if (list_empty(&pTxsCbList->EntryHead)) {
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"(error:: TxsCbList is empty)\n");
		return NDIS_STATUS_FAILURE;
	}

	list_for_each_entry_safe(elem, temp_elem, &pTxsCbList->EntryHead, list) {
		if (elem->pid == pid) {
			--pTxsCbList->EntryCnt;
			ReleasePid(pTxsCbList, pid);
			list_del(&elem->list);
			os_free_mem(elem);
			NdisReleaseSpinLock(&(pTxsCbList->Lock));
			return NDIS_STATUS_SUCCESS;
		}
	}
	NdisReleaseSpinLock(&(pTxsCbList->Lock));
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"(Remove txs callback entry failed: entry not found)\n");
	return NDIS_STATUS_FAILURE;
}

/*Callback and pAd will be returned to caller*/
NDIS_STATUS TxsRetrieveCbEntry(
	struct TXS_CB_LIST *pTxsCbList,
	UINT8 pid,
	ULONG tr_delay,
	struct txs_callback_info_t **Callback,
	RTMP_ADAPTER **pAd)
{
	struct TXS_CB_LIST_ENTRY *elem = NULL;
	struct TXS_CB_LIST_ENTRY *temp_elem = NULL;
	ULONG cur_timestamp, cb_entry_dur_jiffies, txs_tr_delay_jiffies;

	RTMP_GetCurrentSystemTick(&cur_timestamp);
	NdisAcquireSpinLock(&(pTxsCbList->Lock));
	if (list_empty(&pTxsCbList->EntryHead)) {
		NdisReleaseSpinLock(&(pTxsCbList->Lock));
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"(error:: TxsCbList is empty)\n");
		return NDIS_STATUS_FAILURE;
	}

	list_for_each_entry_safe(elem, temp_elem, &pTxsCbList->EntryHead, list) {
		if (elem->pid == pid) {
			*Callback = elem->Callback;
			cb_entry_dur_jiffies = cur_timestamp - elem->enq_timestamp;
			txs_tr_delay_jiffies = TR_DELAY_TO_JIFFIES(tr_delay);

			MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
					"(T2- T1 = %lu jiffies)\n", cb_entry_dur_jiffies);
			MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
					"(tr_delay = %lu us = %lu jiffies)\n", tr_delay, txs_tr_delay_jiffies);
			if (cb_entry_dur_jiffies < txs_tr_delay_jiffies) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"(error:: Entry timestamp of pid %d invalid)\n", pid);
				NdisReleaseSpinLock(&(pTxsCbList->Lock));
				return NDIS_STATUS_FAILURE;
			}
			--pTxsCbList->EntryCnt;
			*pAd = elem->pAd;
			ReleasePid(pTxsCbList, pid);
			list_del(&elem->list);
			os_free_mem(elem);
			NdisReleaseSpinLock(&(pTxsCbList->Lock));
			return NDIS_STATUS_SUCCESS;
		}
	}

	NdisReleaseSpinLock(&(pTxsCbList->Lock));
	return NDIS_STATUS_FAILURE;
}

VOID TxsReceiveHandler(struct physical_device *ph_dev, struct txs_info_t txs_info)
{
	RTMP_ADAPTER *pAd = NULL;
	struct txs_callback_info_t *Callback = NULL;
	UINT8 pid = txs_info.pid;
	NDIS_STATUS Status;

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Received txs result for pid = %d, txs_sts = %d\n",
			pid, txs_info.txs_sts);

	Status = TxsRetrieveCbEntry(&ph_dev->TxsCbList, pid, txs_info.tr_delay, &Callback, &pAd);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"(error:: Txs Callback of PID %d not found)\n", pid);
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&(Callback->txs_info), sizeof(struct txs_info_t));
	if (Callback->txs_info == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"Fail to allocate mem for callback->txs_info\n");
		if (Callback->callback_arg)
			os_free_mem(Callback->callback_arg);
		return;
	}
	os_zero_mem(Callback->txs_info, sizeof(struct txs_info_t));
	os_move_mem(Callback->txs_info, &txs_info, sizeof(struct txs_info_t));

	/* enqueue message */
	MlmeEnqueue(pAd, TXS_STATE_MACHINE, TXS_FSM_WAIT_TXS_RES,
		sizeof(struct txs_callback_info_t), (PVOID)Callback, 0);
	RTMP_MLME_HANDLER(pAd);
	os_free_mem(Callback);
}

VOID mlme_wait_txs_action(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct txs_callback_info_t *Callback = NULL;
	int ret_val = 0;

	Callback = (struct txs_callback_info_t *)&elem->Msg;
	ret_val = Callback->callback_func(Callback->callback_arg, Callback->txs_info);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
			"Callback dequeued,  txs_sts = %d\n",
			Callback->txs_info->txs_sts);

	if (ret_val != 0)
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"Callback returned nonzero value,  ret_val = %d, txs_sts = %d\n",
			ret_val, Callback->txs_info->txs_sts);
	if (Callback->callback_arg)
		os_free_mem(Callback->callback_arg);
	os_free_mem(Callback->txs_info);
}

VOID WaitTxsTOPeriodicHandler(IN struct physical_device *physical_dev)
{
	RTMP_ADAPTER *pAd = NULL;
	struct TXS_CB_LIST *pTxsCbList = &physical_dev->TxsCbList;
	struct TXS_CB_LIST_ENTRY *elem = NULL;
	struct TXS_CB_LIST_ENTRY *next_elem = NULL;
	struct txs_callback_info_t *Callback = NULL;
	ULONG cur_timestamp;

	NdisAcquireSpinLock(&(pTxsCbList->Lock));
	list_for_each_entry_safe(elem, next_elem, &pTxsCbList->EntryHead, list) {
		Callback = elem->Callback;
		if (Callback->timeout_cnt == TXS_TMR_DISABLED) {
			continue;
		} else if (Callback->timeout_cnt == 0) {
			ULONG tr_delay;

			RTMP_GetCurrentSystemTick(&cur_timestamp);
			tr_delay = jiffies_to_usecs(cur_timestamp - elem->enq_timestamp);
			ReleasePid(pTxsCbList, elem->pid);
			--pTxsCbList->EntryCnt;
			pAd = elem->pAd;
			os_alloc_mem(pAd, (UCHAR **)&Callback->txs_info, sizeof(struct txs_info_t));
			if (Callback->txs_info == NULL) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					"Fail to allocate mem for callback->txs_info\n");
				if (Callback->callback_arg)
					os_free_mem(Callback->callback_arg);
				os_free_mem(Callback);
				list_del(&elem->list);
				os_free_mem(elem);
				continue;
			}
			os_zero_mem(Callback->txs_info, sizeof(struct txs_info_t));
			Callback->txs_info->txs_sts = TXS_STS_TO;
			Callback->txs_info->pid = elem->pid;
			Callback->txs_info->tr_delay = tr_delay;
			MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_NOTICE,
					"Txs Callback of PID %d timeout, txs_sts = %d\n",
					Callback->txs_info->pid, Callback->txs_info->txs_sts);

			/* enqueue message */
			MlmeEnqueue(pAd, TXS_STATE_MACHINE, TXS_FSM_WAIT_TXS_RES,
				sizeof(struct txs_callback_info_t), (PVOID)Callback, 0);
			RTMP_MLME_HANDLER(pAd);
			os_free_mem(Callback);
			list_del(&elem->list);
			os_free_mem(elem);
		} else {
			--Callback->timeout_cnt;
		}
	}
	NdisReleaseSpinLock(&(pTxsCbList->Lock));
}
/** @} */
/** @} */
