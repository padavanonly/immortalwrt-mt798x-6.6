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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"
#ifdef DBG_AMSDU
DECLARE_TIMER_FUNCTION(amsdu_history_exec);

VOID amsdu_history_exec(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	UINT32 i;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	STA_TR_ENTRY *tr_entry = NULL;

	for (i = 0; IS_WCID_VALID(pAd, i); i++) {
		tr_entry = tr_entry_get(pAd, i);

		if (!IS_ENTRY_NONE(tr_entry)) {
			tr_entry->amsdu_1_rec[pAd->dbg_time_slot] = tr_entry->amsdu_1;
			tr_entry->amsdu_1 = 0;
			tr_entry->amsdu_2_rec[pAd->dbg_time_slot] = tr_entry->amsdu_2;
			tr_entry->amsdu_2 = 0;
			tr_entry->amsdu_3_rec[pAd->dbg_time_slot] = tr_entry->amsdu_3;
			tr_entry->amsdu_3 = 0;
			tr_entry->amsdu_4_rec[pAd->dbg_time_slot] = tr_entry->amsdu_4;
			tr_entry->amsdu_4 = 0;
			tr_entry->amsdu_5_rec[pAd->dbg_time_slot] = tr_entry->amsdu_5;
			tr_entry->amsdu_5 = 0;
			tr_entry->amsdu_6_rec[pAd->dbg_time_slot] = tr_entry->amsdu_6;
			tr_entry->amsdu_6 = 0;
			tr_entry->amsdu_7_rec[pAd->dbg_time_slot] = tr_entry->amsdu_7;
			tr_entry->amsdu_7 = 0;
			tr_entry->amsdu_8_rec[pAd->dbg_time_slot] = tr_entry->amsdu_8;
			tr_entry->amsdu_8 = 0;
		}
	}
	pAd->dbg_time_slot++;
	pAd->dbg_time_slot = pAd->dbg_time_slot % TIME_SLOT_NUMS;

}
BUILD_TIMER_FUNCTION(amsdu_history_exec);
#endif

static INT ge_sta_clean_queue(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	UINT16 idx, wcid_start, wcid_end;
	STA_TR_ENTRY *tr_entry;
	ULONG IrqFlags;
	PNDIS_PACKET pPacket;
	struct _QUEUE_ENTRY *pEntry;
	struct _QUEUE_HEADER *pQueue;

	if (wcid == WCID_ALL) {
		wcid_start = 0;
		wcid_end = WTBL_MAX_NUM(pAd) - 1;
	} else {
		if (IS_WCID_VALID(pAd, wcid))
			wcid_start = wcid_end = wcid;
		else {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"Invalid WCID[%d]\n", wcid);
			return FALSE;
		}
	}

	for (wcid = wcid_start; wcid <= wcid_end; wcid++) {
		tr_entry = tr_entry_get(pAd, wcid);

		if (IS_ENTRY_NONE(tr_entry))
			continue;

		for (idx = 0; idx < WMM_QUE_NUM; idx++) {
			RTMP_IRQ_LOCK(&tr_entry->txq_lock[idx], IrqFlags);
			pQueue = &tr_entry->tx_queue[idx];

			while (pQueue->Head) {
				pEntry = RemoveHeadQueue(pQueue);
				TR_ENQ_COUNT_DEC(tr_entry);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				if (pPacket)
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
			RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[idx], IrqFlags);
		}

		RTMP_IRQ_LOCK(&tr_entry->ps_queue_lock, IrqFlags);
		pQueue = &tr_entry->ps_queue;

		while (pQueue->Head) {
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);

			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

		RTMP_IRQ_UNLOCK(&tr_entry->ps_queue_lock, IrqFlags);
	}
	return NDIS_STATUS_SUCCESS;
}

static INT ge_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret;
	struct _QUEUE_HEADER *que;
	struct _QUEUE_ENTRY *entry;
	NDIS_PACKET *pkt;
	UCHAR i;
#ifdef DBG_AMSDU
	BOOLEAN cancelled;
#endif

#ifdef CONFIG_TX_DELAY
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	BOOLEAN que_agg_timer_cancelled;
#endif
	struct tx_swq_fifo *fifo_swq;


#ifdef DBG_AMSDU
	RTMPReleaseTimer(&pAd->amsdu_history_timer, &cancelled);
#endif

#ifdef CONFIG_TX_DELAY
	RTMPReleaseTimer(&tr_ctl->tx_delay_ctl.que_agg_timer, &que_agg_timer_cancelled);
#endif

	OS_SEM_LOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
	que = PD_GET_QM_MGMT_SWQ(pAd->physical_dev, 0);

	while (que->Head) {
		entry = RemoveHeadQueue(que);
		pkt = QUEUE_ENTRY_TO_PACKET(entry);

		if (pkt)
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	OS_SEM_UNLOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
	NdisFreeSpinLock(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));

	OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	que = PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev);

	while (que->Head) {
		entry = RemoveHeadQueue(que);
		pkt = QUEUE_ENTRY_TO_PACKET(entry);

		if (pkt)
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	NdisFreeSpinLock(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

	ret = ge_sta_clean_queue(pAd, WCID_ALL);

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		NdisFreeSpinLock(PD_GET_QM_TX_SWQ_LOCK(pAd->physical_dev, hc_get_hw_band_idx(pAd), i));

	fifo_swq = PD_GET_QM_TX_SWQ_PER_BAND(pAd->physical_dev, hc_get_hw_band_idx(pAd));
	if (fifo_swq)
		os_free_mem(fifo_swq);


	return ret;
}

static INT ge_enq_mgmtq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		tr_cnt->wlan_state_non_valid_drop++;
		goto error;
	}

	if (PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, band_idx) >= MGMT_QUE_MAX_NUMS) {
		tr_cnt->tx_sw_mgmtq_drop++;
		goto error;
	}

	OS_SEM_LOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));
	InsertTailQueue(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, band_idx), PACKET_TO_QUEUE_ENTRY(pkt));
	OS_SEM_UNLOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));

	qm_ops->schedule_tx_que(pAd, band_idx);

	return NDIS_STATUS_SUCCESS;

error:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT ge_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
	STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, wcid);
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	INT ret;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		tr_cnt->wlan_state_non_valid_drop++;
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (!RTMP_GET_PACKET_HIGH_PRIO(pkt)) {
		if (pAd->TxSwQueue[q_idx].Number >= pAd->TxSwQMaxLen) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			qm_ops->schedule_tx_que(pAd, band_idx);
			return NDIS_STATUS_FAILURE;
		}

		ret = ge_enq_req(pAd, pkt, q_idx, tr_entry, NULL);
		if (ret != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			qm_ops->schedule_tx_que(pAd, band_idx);
			return NDIS_STATUS_FAILURE;
		}
	} else {
		OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

		if (PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev) >= HIGH_PRIO_QUE_MAX_NUMS) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
			return NDIS_STATUS_FAILURE;

		}

		InsertTailQueue(PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev), PACKET_TO_QUEUE_ENTRY(pkt));
		OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	}
	qm_ops->schedule_tx_que(pAd, band_idx);

	return NDIS_STATUS_SUCCESS;
}

static inline VOID ge_enq_pkt(struct _QUEUE_HEADER *pkt_que, NDIS_SPIN_LOCK *queue_lock, PNDIS_PACKET pkt)
{
	if (queue_lock)
		OS_SEM_LOCK(queue_lock);
	InsertTailQueue(pkt_que, PACKET_TO_QUEUE_ENTRY(pkt));
	if (queue_lock)
		OS_SEM_UNLOCK(queue_lock);
}

static INT ge_enq_psq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt)
{
	ge_enq_pkt(&tr_entry->ps_queue, &tr_entry->ps_queue_lock, pkt);

	return NDIS_STATUS_SUCCESS;
}

INT ge_enq_delayq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt)
{
	ge_enq_pkt(&tr_entry->delay_queue, &tr_entry->delay_queue_lock, pkt);

	return NDIS_STATUS_SUCCESS;
}

static INT ge_schedule_tx_que(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	UINT i;
	UINT16 deq_wcid = 0;
	struct tx_swq_fifo *fifo_swq;
	uint32_t cpu_id, cpu_idx, cpu_num;

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, idx, i);
		deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];

		if (deq_wcid != 0)
			break;
	}

	if ((deq_wcid != 0) || (PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, idx) > 0) ||
		(PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev) > 0)) {
		cpu_id = smp_processor_id();
		cpu_num = num_online_cpus();
		cpu_idx = (cpu_id + 1) % cpu_num;

		if (tm_ops->schedule_task_async_on)
			tm_ops->schedule_task_async_on(pAd, cpu_idx, TX_DEQ_TASK, idx);
		else
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
	}

	return NDIS_STATUS_SUCCESS;
}

static VOID ge_sta_dump_queue(RTMP_ADAPTER *pAd, UINT16 wcid, enum PACKET_TYPE pkt_type, UCHAR qidx)
{
	unsigned long IrqFlags;
	struct _QUEUE_ENTRY *entry;
	INT cnt = 0;
	STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, wcid);

	if (tr_entry == NULL) {
		MTWF_PRINT("%s():Invalid entry(%p) or qidx(%d)\n", __func__, tr_entry, qidx);
		return;
	}

	OS_SEM_LOCK(&tr_entry->ps_sync_lock);
	MTWF_PRINT("\nDump TxQ[%d] of TR_ENTRY(ID:%d,\
				MAC:"MACSTR"),\
				enq_cap = %d, ps_state = %s\n",
				qidx, tr_entry->wcid, MAC2STR(tr_entry->Addr),
				tr_entry->enq_cap,
				tr_entry->ps_state == PWR_ACTIVE ? "PWR_ACTIVE" : "PWR_SAVE");

	OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);

	if (pkt_type == TX_DATA) {
		switch (qidx) {
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
			RTMP_IRQ_LOCK(&tr_entry->txq_lock[qidx], IrqFlags);
			entry = tr_entry->tx_queue[qidx].Head;

			MTWF_PRINT("\nDump Entry %s\n",	entry == NULL ? "Empty" : "HasEntry");

			while (entry != NULL) {
				MTWF_PRINT(" 0x%p ", entry);
				cnt++;
				entry = entry->Next;

				if (entry == NULL)
					MTWF_PRINT("\n");

				if (cnt > tr_entry->tx_queue[qidx].Number) {
					MTWF_PRINT("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, tr_entry->tx_queue[qidx].Number);
				}
			};

			RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[qidx], IrqFlags);
			break;
		default:
			MTWF_PRINT("unknown q_idx = %d\n", qidx);
			break;
		}
	} else if (pkt_type == TX_DATA_PS) {
		OS_SEM_LOCK(&tr_entry->ps_queue_lock);

		entry = tr_entry->ps_queue.Head;

		MTWF_PRINT("\nDump Entry %s\n",	entry == NULL ? "Empty" : "HasEntry");

		while (entry != NULL) {
			MTWF_PRINT(" 0x%p ", entry);
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_PRINT("\n");

			if (cnt > tr_entry->ps_queue.Number) {
				MTWF_PRINT("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, tr_entry->ps_queue.Number);
			}
		};

		OS_SEM_UNLOCK(&tr_entry->ps_queue_lock);
	} else if (pkt_type == TX_DATA_HIGH_PRIO) {

		OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

		entry = PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev)->Head;

		MTWF_PRINT("\nDump Entry %s\n", entry == NULL ? "Empty" : "HasEntry");

		while (entry != NULL) {
			MTWF_PRINT(" 0x%p ", entry);
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_PRINT("\n");

			if (cnt > PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev)) {
				MTWF_PRINT("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev));
			}
		};

		OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

	} else if (pkt_type == TX_MGMT) {

		OS_SEM_LOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
		entry = PD_GET_QM_MGMT_SWQ(pAd->physical_dev, 0)->Head;

		MTWF_PRINT("\nDump Entry %s\n", entry == NULL ? "Empty" : "HasEntry");

		while (entry != NULL) {
			MTWF_PRINT(" 0x%p ", entry);
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_PRINT("\n");

			if (cnt > PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, 0)) {
				MTWF_PRINT("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, 0));
			}
		};

		OS_SEM_UNLOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
	}
}

/*
 * management queue
 * power saving queue
 * data queue
 * high priority queue
 */
static INT32 ge_dump_all_sw_queue(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	struct _QUEUE_ENTRY *entry;
	INT cnt = 0;
	INT i, j;
	STA_TR_ENTRY *tr_entry = NULL;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	MTWF_PRINT("tx flow control[%d]:%d\n", hc_get_hw_band_idx(pAd), tx_flow_check_if_blocked(pAd));

	MTWF_PRINT("max_probe_num:%d\n", cap->ProbeRspMaxNum);
	MTWF_PRINT("txprobe_rsp_cnt_per_s[%d]:%d\n", hc_get_hw_band_idx(pAd), pAd->probe_rsp_cnt_per_s);
	MTWF_PRINT("bn_probe_rsp_drop_cnt[%d]:%d\n", hc_get_hw_band_idx(pAd), pAd->bn_probe_rsp_drop_cnt);

	/* management sw queue */
	OS_SEM_LOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
	entry = PD_GET_QM_MGMT_SWQ(pAd->physical_dev, 0)->Head;

	MTWF_PRINT("\nDump management queue Entry %s\n",
			entry == NULL ? "Empty" : "HasEntry");

	while (entry != NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " 0x%p ", entry);
		cnt++;
		entry = entry->Next;

		if (entry == NULL)
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "\n");

		if (cnt > PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, 0)) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
					 "Buggy here? entry number(%d) not equal!\n",
					 PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, 0));
		}
	};

	OS_SEM_UNLOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, 0));
	MTWF_PRINT("Count of management Entry = %d\n", cnt);

	/* high prority queue */
	cnt = 0;
	OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

	entry = PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev)->Head;

	MTWF_PRINT("\nDump high prority queue Entry %s\n",
			entry == NULL ? "Empty" : "HasEntry");

	while (entry != NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " 0x%p ", entry);
		cnt++;
		entry = entry->Next;

		if (entry == NULL)
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "\n");

		if (cnt > PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev)) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
					"Buggy here? entry number(%d) not equal!\n",
					PD_GET_QM_HIGH_PRI_SWQ_LEN(pAd->physical_dev));
		}
	};

	OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	MTWF_PRINT("Count of high prority queue Entry = %d\n", cnt);

	/* per sta queue */
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

		/*Skip the invalid Category to indicate/un-used entry*/
		if (pEntry->EntryType == ENTRY_CAT_NONE)
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		tr_entry = tr_entry_get(pAd, i);

		OS_SEM_LOCK(&tr_entry->ps_sync_lock);
		MTWF_PRINT("\nDump TR_ENTRY(ID:%d,\
				MAC:"MACSTR"),\
				enq_cap = %d, ps_state = %s\n",
				tr_entry->wcid, MAC2STR(tr_entry->Addr),
				tr_entry->enq_cap,
				tr_entry->ps_state == PWR_ACTIVE ? "PWR_ACTIVE" : "PWR_SAVE");
		OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);

		cnt = 0;
		OS_SEM_LOCK(&tr_entry->ps_queue_lock);

		entry = tr_entry->ps_queue.Head;

		MTWF_PRINT("\nDump wcid(%d) power saving queue Entry %s\n",
				i, entry == NULL ? "Empty" : "HasEntry");

		while (entry != NULL) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " 0x%p ", entry);
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "\n");

			if (cnt > tr_entry->ps_queue.Number) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
						 "Buggy here? entry number(%d) not equal!\n",
						  tr_entry->ps_queue.Number);
			}
		};

		OS_SEM_UNLOCK(&tr_entry->ps_queue_lock);
		MTWF_PRINT("Count of wcid(%d) power saving Entry = %d\n", i, cnt);


		for (j = 0; j < WMM_QUE_NUM; j++) {
			cnt = 0;
			OS_SEM_LOCK(&tr_entry->txq_lock[j]);
			entry = tr_entry->tx_queue[j].Head;

			MTWF_PRINT("\nDump wcid(%d), qidx(%d) data queue Entry %s\n",
					i, j, entry == NULL ? "Empty" : "HasEntry");

			while (entry != NULL) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " 0x%p ", entry);
				cnt++;
				entry = entry->Next;

				if (entry == NULL)
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "\n");

				if (cnt > tr_entry->tx_queue[j].Number) {
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
						 "Buggy here? entry number(%d) not equal!\n",
						 tr_entry->tx_queue[j].Number);
				}
			};

			OS_SEM_UNLOCK(&tr_entry->txq_lock[j]);
			MTWF_PRINT("Count of wcid(%d), qidx(%d) data Entry = %d\n",	i, j, cnt);

		}

	}

	return NDIS_STATUS_SUCCESS;
}

VOID ge_tx_swq_dump(RTMP_ADAPTER *pAd, INT qidx)
{
	ULONG IrqFlags;
	UINT deq_id, enq_id, cnt = 0;
	struct tx_swq_fifo *fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, hc_get_hw_band_idx(pAd), qidx);

	RTMP_IRQ_LOCK(&fifo_swq->swq_lock, IrqFlags);
	deq_id = fifo_swq->deqIdx;
	enq_id = fifo_swq->enqIdx;
	MTWF_PRINT("\nDump TxSwQ[%d]: DeqIdx=%d, EnqIdx=%d, %s\n",
			  qidx, deq_id, enq_id,
			  (fifo_swq->swq[deq_id] == 0 ? "Empty" : "HasEntry"));

	for (; deq_id != enq_id; (deq_id =  (deq_id == (TX_SWQ_FIFO_LEN - 1) ? 0 : deq_id + 1))) {
		MTWF_PRINT(" %d ", fifo_swq->swq[deq_id]);
		cnt++;

		if (cnt > TX_SWQ_FIFO_LEN) {
			MTWF_PRINT("%s(): Buggy here? force break! deq_id=%d, enq_id=%d\n",
					  __func__, deq_id, enq_id);
		}

	}

	RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, IrqFlags);
	MTWF_PRINT("\n");
}

INLINE BOOLEAN ge_get_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	return PD_GET_QM_TX_SWQ(pAd->physical_dev, hc_get_hw_band_idx(pAd), q_idx)->q_state;
}

INLINE INT ge_set_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx, BOOLEAN state)
{
	PD_GET_QM_TX_SWQ(pAd->physical_dev, hc_get_hw_band_idx(pAd), q_idx)->q_state = state;
	return NDIS_STATUS_SUCCESS;
}

INLINE UINT32 ge_get_swq_free_num(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	UINT cap_cnt = 0;
	struct tx_swq_fifo *fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, hc_get_hw_band_idx(pAd), q_idx);
	INT enq_idx = 0, deq_idx = 0;

	enq_idx = fifo_swq->enqIdx;
	deq_idx = fifo_swq->deqIdx;

	cap_cnt = (enq_idx > deq_idx) ? (TX_SWQ_FIFO_LEN - enq_idx + deq_idx)
			: (deq_idx - enq_idx);

	return cap_cnt;
}

UINT32 ge_check_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	BOOLEAN swq_state = ge_get_swq_state(pAd, q_idx);
	UINT swq_free_num = ge_get_swq_free_num(pAd, q_idx);
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if ((swq_state == TX_QUE_HIGH) &&
		(swq_free_num >= PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, q_idx)->low_water_mark)) {
		return TX_QUE_HIGH_TO_HIGH;
	} else if ((swq_state == TX_QUE_HIGH) &&
		(swq_free_num < PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, q_idx)->low_water_mark)) {
		return TX_QUE_HIGH_TO_LOW;
	} else if ((swq_state == TX_QUE_LOW) &&
		(swq_free_num > PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, q_idx)->high_water_mark)) {
		return TX_QUE_LOW_TO_HIGH;
	} else if ((swq_state == TX_QUE_LOW) &&
		(swq_free_num <= PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, q_idx)->high_water_mark)) {
		return TX_QUE_LOW_TO_LOW;
	} else {
		MTWF_PRINT("%s: unknow state %d, q number = %d",
		__func__, swq_state, swq_free_num);
		return TX_RING_UNKNOW_CHANGE;
	}
}

INT ge_enq_req(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qidx,
	STA_TR_ENTRY *tr_entry, struct _QUEUE_HEADER *pPktQueue)
{
	unsigned long irq_flags_swq = 0, irq_flags_txq = 0;
	BOOLEAN enq_done = FALSE;
	INT enq_idx = 0;
	struct tx_swq_fifo *fifo_swq;
	UINT16 occupied_wcid = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	ASSERT(qidx < WMM_QUE_NUM);
	ASSERT((tr_entry->wcid != 0));
	fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, qidx);
	RTMP_IRQ_LOCK(&fifo_swq->swq_lock, irq_flags_swq);
	if ((tr_entry->enqCount > SQ_ENQ_NORMAL_MAX)
		&& (tr_entry->tx_queue[qidx].Number > SQ_ENQ_RESERVE_PERAC)) {
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
		tr_cnt->tx_sw_dataq_drop++;
		goto enq_end;
	}

	enq_idx = fifo_swq->enqIdx;

	if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap)) {

		RTMP_IRQ_LOCK(&tr_entry->txq_lock[qidx], irq_flags_txq);
		TR_ENQ_COUNT_INC(tr_entry);
		InsertTailQueueAc(pAd, tr_entry, &tr_entry->tx_queue[qidx],
						  PACKET_TO_QUEUE_ENTRY(pkt));

		RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[qidx], irq_flags_txq);

		fifo_swq->swq[enq_idx] = tr_entry->wcid;
		INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
		if (fifo_swq->swq[fifo_swq->enqIdx] != 0) {
			/* Stop device first to avoid drop packets when detect SWQ full, not execute on WMM case */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
				!tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx))
				tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, TRUE, qidx);
		}
		enq_done = TRUE;
	} else {
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
		tr_cnt->tx_sw_dataq_drop++;

		goto enq_end;
	}

enq_end:
	RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, irq_flags_swq);
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			 "EnqPkt(%p) for WCID(%d) to tx_swq[%d].swq[%d] %s\n",
			  pkt, tr_entry->wcid, qidx, enq_idx,
			  (enq_done ? "success" : "fail"));

	if (enq_done == FALSE) {
#ifdef DBG_DIAGNOSE
#ifdef DBG_TXQ_DEPTH
		if ((pAd->DiagStruct.inited) && (pAd->DiagStruct.wcid == tr_entry->wcid))
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].enq_fall_cnt[qidx]++;
#endif

#endif /* DBG_DIAGNOSE */

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				 "\t FailedCause =>OccupiedWCID:%d,EnqCap:%d\n",
				  occupied_wcid, tr_entry->enq_cap);

	}
	return enq_done;
}


INT ge_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info)
{
	CHAR deq_qid = 0, start_q, end_q;
	UINT16 deq_wcid;
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry = NULL;
	unsigned long IrqFlags = 0;
	unsigned int quota = 0;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (!info->inited) {
		if (info->target_que < WMM_QUE_NUM) {
			info->start_q = info->target_que;
			info->end_q = info->target_que;
		} else {
			info->start_q = (WMM_QUE_NUM - 1);
			info->end_q = 0;
		}

		info->cur_q = info->start_q;

		/*
		 * a. for specific wcid, quota number "cnt" stored in info->pkt_cnt and shared by 4 ac queue
		 * b. for all wcid, quota stored in info->pkt_cnt and info->q_max_cnt[ac_index] and each ac has quota number "cnt"
		 *    shared by all wcid
		 */
		if (IS_WCID_VALID(pAd, info->target_wcid)) {
			info->pkt_cnt = cnt;
			info->full_qid[0] = FALSE;
			info->full_qid[1] = FALSE;
			info->full_qid[2] = FALSE;
			info->full_qid[3] = FALSE;
		} else {
			info->q_max_cnt[0] = cnt;
			info->q_max_cnt[1] = cnt;
			info->q_max_cnt[2] = cnt;
			info->q_max_cnt[3] = cnt;
		}

		info->inited = 1;
	}

	start_q = info->cur_q;
	end_q = info->end_q;

	/*
	 * decide cur_wcid and cur_que under info->pkt_cnt > 0 condition for specific wcid
	 * cur_wcid = info->target_wcid
	 * cur_que = deq_qid
	 * deq_que has two value, one come from info->target_que for specific ac queue,
	 * another go to check if tr_entry[deq_qid].number > 0 from highest priority
	 * to lowest priority ac queue for all ac queue
	 */
	if (IS_WCID_VALID(pAd, info->target_wcid)) {
		if (info->pkt_cnt <= 0) {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		deq_wcid = info->target_wcid;

		if (info->target_que >= WMM_QUE_NUM) {
			tr_entry = tr_entry_get(pAd, deq_wcid);

			for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
				if (info->full_qid[deq_qid] == FALSE && tr_entry->tx_queue[deq_qid].Number)
					break;
			}
		} else if (info->full_qid[info->target_que] == FALSE)
			deq_qid = info->target_que;
		else {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		if (deq_qid >= 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
		} else
			info->status = NDIS_STATUS_FAILURE;

		goto done;
	}

	/*
	 * decide cur_wcid and cur_que for all wcid
	 * cur_wcid = deq_wcid
	 * deq_wcid need to check tx_swq_fifo from highest priority to lowest priority ac queues
	 * and come from tx_swq_fifo.swq[tx_deq_fifo.deqIdx]
	 * cur_que = deq_qid upon found a wcid
	 */
	for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
		fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, deq_qid);
		RTMP_IRQ_LOCK(&fifo_swq->swq_lock, IrqFlags);
		{
			deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];
			quota = info->q_max_cnt[deq_qid];
		}
		RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, IrqFlags);

		if (deq_wcid == 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
					 "tx_swq[%d] emtpy!\n", deq_qid);
			info->q_max_cnt[deq_qid] = 0;
			continue;
		}

		if (info->q_max_cnt[deq_qid] > 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
			info->pkt_cnt = quota;
			break;
		}
	}

	if (deq_qid < end_q) {
		info->cur_q = deq_qid;
		info->status = NDIS_STATUS_FAILURE;
	}

done:
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			 "DeqReq %s, Start/End/Cur Queue=%d/%d/%d\n",
			  (info->status == NDIS_STATUS_SUCCESS ? "success" : "fail"),
			  info->start_q, info->end_q, info->cur_q);

	if (info->status == NDIS_STATUS_SUCCESS) {
		tr_entry = tr_entry_get(pAd, info->cur_wcid);
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				 "\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d, q_max_cnt=%d, QueuedNum=%d\n",
				  info->cur_wcid, info->cur_q, info->pkt_cnt, info->q_max_cnt[deq_qid],
				  tr_entry->tx_queue[info->cur_q].Number);
	} else {
		info->status = NDIS_STATUS_FAILURE;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				 "\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d\n",
				  info->cur_wcid, info->cur_q, info->pkt_cnt);
	}

	return TRUE;
}


static INT ge_deq_report(RTMP_ADAPTER *pAd, struct dequeue_info *info)
{
	UINT tx_cnt = info->deq_pkt_cnt, qidx = info->cur_q;
	struct tx_swq_fifo *fifo_swq;
	unsigned long IrqFlags = 0;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (qidx >= WMM_QUE_NUM) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			"Invalid AC Queue Index\n");
		return FALSE;
	}

	fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, qidx);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			 "Success DeQ(QId=%d) for WCID(%d), PktCnt=%d, TxSWQDeQ/EnQ ID=%d/%d\n",
			  info->cur_q, info->cur_wcid, info->deq_pkt_cnt,
			  fifo_swq->deqIdx, fifo_swq->enqIdx);

	if (tx_cnt > 0) {
		RTMP_IRQ_LOCK(&fifo_swq->swq_lock, IrqFlags);

		do {
			if (fifo_swq->swq[fifo_swq->deqIdx]  == info->cur_wcid) {
				fifo_swq->swq[fifo_swq->deqIdx] = 0;
				INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
				tx_cnt--;
			} else
				break;
		} while (tx_cnt != 0);

		if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx) &&
			ge_get_swq_free_num(pAd, qidx) > fifo_swq->high_water_mark)
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, FALSE, qidx);

		RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, IrqFlags);

		if (info->q_max_cnt[qidx] > info->deq_pkt_cnt)
			info->q_max_cnt[qidx] -= info->deq_pkt_cnt;
		else
			info->q_max_cnt[qidx] = 0;

		if (IS_WCID_VALID(pAd, info->target_wcid))
			info->pkt_cnt -= info->deq_pkt_cnt;

		/* ge_tx_swq_dump(pAd, qidx); */
		/* rtmp_sta_txq_dump(pAd, &pAd->MacTab.tr_entry[info->wcid], qidx); */
	}

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			 "After DeqReport, tx_swq D/EQIdx=%d/%d, deq_info.q_max_cnt/pkt_cnt=%d/%d\n",
			  fifo_swq->deqIdx, fifo_swq->enqIdx, info->q_max_cnt[qidx], info->pkt_cnt);

	return TRUE;
}

static BOOLEAN check_amsdu_limit(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, PNDIS_PACKET pkt)
{
	MAC_TABLE_ENTRY *pEntry = tx_blk->pMacEntry;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UINT32 total_len = tx_blk->TotalFrameLen + GET_OS_PKT_LEN(pkt);

	/*
	 * limitation rule:
	 * a. limit by A-MSDU size
	 * b. limit by A-MSDU number if amsdu_fix turn on
	 */
	if (tx_blk->TotalFrameNum < cap->hw_max_amsdu_nums) {

		if (tr_ctl->amsdu_fix) {
			if (tx_blk->TotalFrameNum < tr_ctl->amsdu_fix_num &&
				total_len <= pEntry->amsdu_limit_len_adjust)
				return TRUE;
		} else {

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"current total frame len = %d, pkt_len = %d, amsdu_limit_len_adjust = %d\n",
				 tx_blk->TotalFrameLen, GET_OS_PKT_LEN(pkt), pEntry->amsdu_limit_len_adjust);

			if (total_len <= pEntry->amsdu_limit_len_adjust)
				return TRUE;
		}
	}

	return FALSE;
}

#ifdef RANDOM_PKT_GEN
static UINT32 randomvalueforqidx;
static VOID random_write_resource_idx(RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RandomTxCtrl != 0) {
		pTxBlk->lmac_qidx = randomvalueforqidx % (cap->qos.WmmHwNum * 4);
		randomvalueforqidx += 7;
		if (pTxBlk->lmac_qidx < cap->qos.WmmHwNum * 4)
			pTxBlk->resource_idx = Qidmapping[pTxBlk->lmac_qidx];
	}
}
#endif

#ifdef CONFIG_TX_DELAY
DECLARE_TIMER_FUNCTION(que_agg_timeout);

VOID que_agg_timeout(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)FunctionContext;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

	if (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, 0)) {
		qm_ops->schedule_tx_que(pAd, 0);
		tx_delay_ctl->force_deq = TRUE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
	}
}
BUILD_TIMER_FUNCTION(que_agg_timeout);

static BOOLEAN ge_tx_deq_delay(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UCHAR q_idx)
{
	NDIS_PACKET *pkt = NULL;
	unsigned long flags = 0;
	struct _QUEUE_HEADER *que;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	if ((tx_delay_ctl->que_agg_en) && (!tx_delay_ctl->force_deq)) {
		RTMP_IRQ_LOCK(&tr_entry->txq_lock[q_idx], flags);
		que = &tr_entry->tx_queue[q_idx];

		if (que->Head)
			pkt = QUEUE_ENTRY_TO_PACKET(que->Head);

		RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[q_idx], flags);

		if ((que->Number > 0) &&
			(que->Number < tx_delay_ctl->tx_process_batch_cnt) &&
			(pkt) &&
			(GET_OS_PKT_LEN(pkt) >= tx_delay_ctl->min_pkt_len) &&
			(GET_OS_PKT_LEN(pkt) <= tx_delay_ctl->max_pkt_len)) {

			if (!is_udp_packet(pAd, pkt)) {
				if (!tx_delay_ctl->que_agg_timer_running) {
				RTMPSetTimer(&tx_delay_ctl->que_agg_timer, tx_delay_ctl->que_agg_timeout_value / 1000);
				tx_delay_ctl->que_agg_timer_running = TRUE;
				}

				return TRUE;
			}
		}
	}
	return FALSE;
}
#endif

#ifdef HWIFI_SUPPORT
static inline struct mtk_mac_dev *
pad_to_mac_dev(struct _RTMP_ADAPTER *ad)
{
	struct os_cookie *handle = ad->OS_Cookie;

	return handle->mac_dev;
}
#endif

INT deq_packet_gatter(RTMP_ADAPTER *pAd, struct dequeue_info *deq_info, struct _TX_BLK *pTxBlk)
{
	STA_TR_ENTRY *tr_entry;
	struct _QUEUE_ENTRY *qEntry = NULL;
	PNDIS_PACKET pPacket;
	struct _QUEUE_HEADER *pQueue;
	UCHAR q_idx = deq_info->cur_q;
	UINT16 wcid = deq_info->cur_wcid;
	struct wifi_dev *wdev = NULL;
	unsigned long IrqFlags = 0;
	INT ret = 0;
#ifdef HWIFI_SUPPORT
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);
#endif /* HWIFI_SUPPORT */

	tr_entry = tr_entry_get(pAd, wcid);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG, "deq_info->wcid=%d, qidx=%d!\n",
			wcid, q_idx);

	deq_info->deq_pkt_cnt = 0;

	RTMP_IRQ_LOCK(&tr_entry->txq_lock[q_idx], IrqFlags);

	do {
#ifdef HWIFI_SUPPORT
		struct mtk_mac_sta *sta;
		struct mtk_mac_txq *txq;
#endif /* HWIFI_SUPPORT */

		pQueue = &tr_entry->tx_queue[q_idx];
dequeue:
		qEntry = pQueue->Head;

		if (qEntry != NULL) {
			qEntry = RemoveHeadQueue(pQueue);
			TR_ENQ_COUNT_DEC(tr_entry);
			pPacket = QUEUE_ENTRY_TO_PACKET(qEntry);
			ASSERT(RTMP_GET_PACKET_WCID(pPacket) == wcid);

			if (pTxBlk->TotalFrameNum == 0)
				wdev = wdev_search_by_pkt(pAd, pPacket);

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
					 "GetPacket, wcid=%d, deq_pkt_cnt=%d, TotalFrameNum=%d, TotalFrameLen = %d\n",
					  wcid, deq_info->deq_pkt_cnt, pTxBlk->TotalFrameNum, pTxBlk->TotalFrameLen);

			pTxBlk->TxFrameType = tx_pkt_classification(pAd, pPacket, pTxBlk);

			if (pTxBlk->TxFrameType == TX_AMSDU_FRAME) {
				if (pTxBlk->TotalFrameNum > 0) {
					if ((!is_amsdu_capable(pPacket))
						|| !check_amsdu_limit(pAd, pTxBlk, pPacket)) {
						InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
						TR_ENQ_COUNT_INC(tr_entry);
						goto start_kick;
					}
				}
			}

#ifdef HWIFI_SUPPORT
			sta = tr_entry->mac_sta;

			if (!sta) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				goto start_kick;
			}

			pTxBlk->QueIdx = q_idx;

			if (pTxBlk->UserPriority < TXQ_NUM)
				txq = sta->txq[pTxBlk->UserPriority];
			else
				txq = sta->txq[0];

			ret = mac_dev->ops->tx_check_resource(&mac_dev->hw, txq, TX_LEGACY_RESOURCE);
#endif

			if (!ret) {
				InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
				TR_ENQ_COUNT_INC(tr_entry);

				/*
				 * Because of tx resource is not enough for this q_idx,
				 * set deque quota of this q_idx to 0 to let deq request can
				 * service next q_idx which may have tx resource
				 */
				if (IS_WCID_VALID(pAd, deq_info->target_wcid))
					deq_info->full_qid[q_idx] = TRUE;
				else
					deq_info->q_max_cnt[q_idx] = 0;

#ifdef DBG_DIAGNOSE
				if (pAd->DiagStruct.inited && pAd->DiagStruct.wcid == pTxBlk->Wcid) {
					struct dbg_diag_info *diag_info;

					diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
#ifdef DBG_TXQ_DEPTH
					diag_info->deq_fail_no_resource_cnt[QueIdx]++;
#endif
				}

#endif

				goto start_kick;
			}

			pTxBlk->TotalFrameNum++;
			/* The real fragment number maybe vary */
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);

			if (pTxBlk->TotalFrameNum == 1) {
				pTxBlk->pPacket = pPacket;
				pTxBlk->wdev = wdev;
				pTxBlk->tr_entry = tr_entry;
				pTxBlk->Wcid = wcid;
			}

			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
		} else {

			/*
			 * use to clear wcid of fifo_swq->swq[fifo_swq->deqIdx] to 0,
			 * that may happen when previos de-queue more than one packet
			 */
			if (pTxBlk->TxPacketList.Number == 0) {
				deq_info->deq_pkt_cnt++;
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
						"Try deQ a empty Q. pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
						 pTxBlk->TxPacketList.Number, deq_info->pkt_cnt);
				break;
			}
		}

		if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
			pQueue->Head) {
			goto dequeue;
		}

start_kick:

		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME) {
			if (pTxBlk->TxPacketList.Number == 1)
				pTxBlk->TxFrameType = TX_LEGACY_FRAME;
#ifdef DBG_AMSDU
			if (pTxBlk->TxPacketList.Number == 1)
				tr_entry->amsdu_1++;
			else if (pTxBlk->TxPacketList.Number == 2)
				tr_entry->amsdu_2++;
			else if (pTxBlk->TxPacketList.Number == 3)
				tr_entry->amsdu_3++;
			else if (pTxBlk->TxPacketList.Number == 4)
				tr_entry->amsdu_4++;
			else if (pTxBlk->TxPacketList.Number == 5)
				tr_entry->amsdu_5++;
			else if (pTxBlk->TxPacketList.Number == 6)
				tr_entry->amsdu_6++;
			else if (pTxBlk->TxPacketList.Number == 7)
				tr_entry->amsdu_7++;
			else if (pTxBlk->TxPacketList.Number == 8)
				tr_entry->amsdu_8++;
#endif
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
			"pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
			pTxBlk->TxPacketList.Number, deq_info->pkt_cnt);
		break;
	} while (0);

	RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[q_idx], IrqFlags);

	if (pTxBlk->TxPacketList.Number > 0)
		deq_info->deq_pkt_cnt += pTxBlk->TxPacketList.Number;

	return NDIS_STATUS_SUCCESS;

}

static NDIS_PACKET *get_high_prio_pkt(RTMP_ADAPTER *pAd)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	q_entry = RemoveHeadQueue(PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev));
	OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static NDIS_PACKET *first_high_prio_pkt(RTMP_ADAPTER *pAd)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SEM_LOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	q_entry = PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev)->Head;
	OS_SEM_UNLOCK(PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static INT32 ge_deq_high_prio_pkt(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk)
{
	struct wifi_dev *wdev;
	NDIS_PACKET *pkt = NULL;
	INT32 ret = 0;
	UINT16 wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef HWIFI_SUPPORT
		struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);
		struct mtk_mac_sta *sta;
		struct mtk_mac_txq *txq;
#endif /* HWIFI_SUPPORT */

	do {
		pkt = first_high_prio_pkt(pAd);

		if (!pkt) {
			return NDIS_STATUS_FAILURE;
		}

		wdev = wdev_search_by_pkt(pAd, pkt);
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, TX_DATA_HIGH_PRIO, tx_blk->QueIdx);
#ifdef RANDOM_PKT_GEN
		random_write_resource_idx(pAd, tx_blk);
#endif
		wcid = RTMP_GET_PACKET_WCID(pkt);
		tr_entry = tr_entry_get(pAd, wcid);
		sta = tr_entry->mac_sta;

		if (!sta)
			return NDIS_STATUS_FAILURE;

		if (tx_blk->UserPriority < TXQ_NUM)
			txq = sta->txq[tx_blk->UserPriority];
		else
			txq = sta->txq[0];

		ret = mac_dev->ops->tx_check_resource(&mac_dev->hw, txq, TX_HIGHPRIO_RESOURCE);
		if (!ret)
			return NDIS_STATUS_FAILURE;
		pkt = get_high_prio_pkt(pAd);
		if (pkt == NULL)
			continue;

		wcid = RTMP_GET_PACKET_WCID(pkt);

		if (!IS_WCID_VALID(pAd, wcid)) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				 "WCID is invalid\n");
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		if (wdev) {
			tx_blk->wdev = wdev;
		} else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		tr_entry = tr_entry_get(pAd, wcid);

		TX_BLK_SET_FLAG(tx_blk, fTX_HIGH_PRIO);
		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		tx_blk->Wcid = wcid;
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		break;
	} while (1);

	if (tx_blk->TxFrameType == TX_AMSDU_FRAME) {
		if (tx_blk->TxPacketList.Number == 1)
			tx_blk->TxFrameType = TX_LEGACY_FRAME;
	}

	return NDIS_STATUS_SUCCESS;
}

static inline NDIS_PACKET *ge_deq_pkt(struct _QUEUE_HEADER *pkt_que, NDIS_SPIN_LOCK *queue_lock)
{
	struct _QUEUE_ENTRY *q_entry;

	if (queue_lock)
		OS_SEM_LOCK(queue_lock);
	q_entry = RemoveHeadQueue(pkt_que);
	if (queue_lock)
		OS_SEM_UNLOCK(queue_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static NDIS_PACKET *ge_get_psq_pkt(RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry)
{
	return ge_deq_pkt(&tr_entry->ps_queue, &tr_entry->ps_queue_lock);
}

static NDIS_PACKET *get_mgmt_pkt(RTMP_ADAPTER *pAd)
{
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	return ge_deq_pkt(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, band_idx),
		PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));
}

NDIS_PACKET *ge_deq_delayq_pkt(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry)
{
	return ge_deq_pkt(&tr_entry->delay_queue, &tr_entry->delay_queue_lock);
}

static NDIS_PACKET *first_mgmt_pkt(RTMP_ADAPTER *pAd)
{
	struct _QUEUE_ENTRY *q_entry;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	OS_SEM_LOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));
	q_entry = PD_GET_QM_MGMT_SWQ(pAd->physical_dev, band_idx)->Head;
	OS_SEM_UNLOCK(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static INT32 ge_deq_mgmt_pkt(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk)
{
	struct wifi_dev *wdev;
	NDIS_PACKET *pkt = NULL;
	UINT16 wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef HWIFI_SUPPORT
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);
	struct mtk_mac_bss *bss;
	struct mtk_mac_sta *sta;
	struct mtk_mac_txq *txq;
#endif /* HWIFI_SUPPORT */
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	do {
		if (PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, band_idx) == 0)
			return NDIS_STATUS_FAILURE;

		pkt = first_mgmt_pkt(pAd);

		if (!pkt)
			return NDIS_STATUS_FAILURE;

		wdev = wdev_search_by_pkt(pAd, pkt);
		if (wdev == NULL) {
			RELEASE_NDIS_PACKET(pAd, get_mgmt_pkt(pAd), NDIS_STATUS_FAILURE);
			continue;
		}
		wcid = RTMP_GET_PACKET_WCID(pkt);

		if (!IS_WCID_VALID(pAd, wcid)) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				 "WCID is invalid\n");
			RELEASE_NDIS_PACKET(pAd, get_mgmt_pkt(pAd), NDIS_STATUS_FAILURE);
			continue;
		}
		tr_entry = tr_entry_get(pAd, wcid);
#ifdef HWIFI_SUPPORT
		mt_rcu_read_lock();
		sta = tr_entry->mac_sta;
		if (!sta) {
			bss = wdev->pHObj;
			if (!bss) {
				mt_rcu_read_unlock();
				RELEASE_NDIS_PACKET(pAd, get_mgmt_pkt(pAd), NDIS_STATUS_FAILURE);
				continue;
			}

			sta = bss->bmc_sta;
			if (!sta) {
				mt_rcu_read_unlock();
				RELEASE_NDIS_PACKET(pAd, get_mgmt_pkt(pAd), NDIS_STATUS_FAILURE);
				continue;
			}
		}

		txq = sta->txq[MGMT_TXQ];
		if (!mac_dev->ops->tx_check_resource(&mac_dev->hw, txq, TX_HIGHPRIO_RESOURCE)) {
			mt_rcu_read_unlock();
			return NDIS_STATUS_FAILURE;
		}

		mt_rcu_read_unlock();
#endif /* HWIFI_SUPPORT */

		pkt = get_mgmt_pkt(pAd);
		if (pkt == NULL)
			continue;

		tx_blk->wdev = wdev;
		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		tx_blk->Wcid = wcid;
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		break;
	} while (1);

	return NDIS_STATUS_SUCCESS;
}

static inline INT32 ge_deq_data_pkt_v2_process(
	RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, struct _QUEUE_HEADER *pTxPacketList)
{
	PNDIS_PACKET pPacket;
	struct _QUEUE_ENTRY *qEntry = NULL;
	CHAR q_idx = 0;
	INT ret = NDIS_STATUS_FAILURE;

	if (pTxPacketList->Head) {
		qEntry = RemoveHeadQueue(pTxPacketList);
		pPacket = QUEUE_ENTRY_TO_PACKET(qEntry);
		q_idx = RTMP_GET_PACKET_QUEIDX(pPacket);
		pTxBlk->QueIdx = q_idx;
		pTxBlk->wdev = wdev_search_by_pkt(pAd, pPacket);
		/* sanity check & correct the wrong wdev , when peer STA connect from one band to the other */
		pTxBlk->tr_entry = tr_entry_get(pAd, RTMP_GET_PACKET_WCID(pPacket));

		if (pTxBlk->tr_entry->wdev != pTxBlk->wdev) {
			pTxBlk->wdev = pTxBlk->tr_entry->wdev;
		}

		pTxBlk->resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, pTxBlk->wdev, TX_DATA, q_idx);
		pTxBlk->TxFrameType = tx_pkt_classification(pAd, pPacket, pTxBlk);
		pTxBlk->TotalFrameNum = 1;
		/* The real fragment number maybe vary */
		pTxBlk->TotalFragNum = RTMP_GET_PACKET_FRAGMENTS(pPacket);
		pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pPacket);

		pTxBlk->pPacket = pPacket;
		pTxBlk->HeaderBuf = hif_get_tx_buf(pAd->hdev_ctrl, pTxBlk,
			pTxBlk->resource_idx, pTxBlk->TxFrameType);
		InsertTailQueue(&pTxBlk->TxPacketList, qEntry);
		ret = NDIS_STATUS_SUCCESS;
	}
	return ret;
}

/* Support HW AMSDU only, doesn't support SW AMSDU                        */
/* It can simplify dequeue flow without taken SW AMSDU into consideration */
static INT32 ge_deq_data_pkt_v2(
	RTMP_ADAPTER *pAd,
	INT32 max_cnt,
	struct dequeue_info *info,
	struct _QUEUE_HEADER *pTxPacketList)
{
#ifdef CUT_THROUGH
	INT ret = 0;
#endif
	CHAR deq_qid, start_q, end_q, resource_idx;
	UINT16 deq_wcid = 0;
	unsigned long IrqFlags = 0;
	UINT16 *deq_quota = NULL;
	UINT32 deq_pkt_cnt = 0;
	UINT32 free_num = 0, free_token = 0;
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry = NULL;
	struct _QUEUE_ENTRY *qEntry = NULL;
	PNDIS_PACKET pPacket;
	struct _QUEUE_HEADER *pQueue;
	struct _RTMP_CHIP_CAP *cap;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

	if (!info->inited) {
		info->q_max_cnt[0] = max_cnt;
		info->q_max_cnt[1] = max_cnt;
		info->q_max_cnt[2] = max_cnt;
		info->q_max_cnt[3] = max_cnt;

		info->inited = 1;
	}

	start_q = (WMM_QUE_NUM - 1);
	end_q = 0;

	for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
		fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, deq_qid);

		/* swq empty case */
		if (fifo_swq->swq[fifo_swq->deqIdx] == 0)
			continue;

		deq_pkt_cnt = 0;
		deq_quota = &info->q_max_cnt[deq_qid];

		/* make sure useless wdev in this function */
		resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, NULL, TX_DATA, deq_qid);
#ifdef CUT_THROUGH
		/* check resource first, go to next queue if no resource in this queue */
		ret = mt_ct_get_hw_resource_free_num(pAd, NULL, resource_idx, &free_num, &free_token);
		if (ret == NDIS_STATUS_RESOURCES) {
			/* if tx ring is full, re-check pdma status */
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"hwifi is enabled, shall not happen, please check!\n");
			ret = mt_ct_get_hw_resource_free_num(pAd, NULL, resource_idx, &free_num, &free_token);
			pci_dec_resource_full_cnt(pAd, resource_idx);

			if (!ret)
				asic_set_resource_state(pAd, resource_idx, TX_RING_HIGH);
		}
		if (ret) {
			/*
			* Because of tx resource is not enough for this q_idx,
			* set deque quota of this q_idx to 0 to let deq request can
			* service next q_idx which may have tx resource
			*/
			*deq_quota = 0;
			continue;
		}
#endif /*CUT_THROUGH*/
		/* deq_packet_gatter */
		/* dequeue until run out of deq_quota or not enough resource */
		while ((*deq_quota) > 0 && deq_pkt_cnt < free_num && pTxPacketList->Number < free_token) {
			/* get deq wcid from swq */
			deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];

			if (deq_wcid == 0) {
				/* swq empty case */
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
						 "tx_swq[%d] emtpy!\n", deq_qid);
				if (!deq_pkt_cnt)
					*deq_quota = 0;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"deq_info:cur_wcid=%d, cur_qidx=%d, pkt_cnt=%d\n",
				deq_wcid, deq_qid, deq_pkt_cnt);

			tr_entry = tr_entry_get(pAd, deq_wcid);

#ifdef CONFIG_TX_DELAY
			if (IS_TX_DELAY_SW_MODE(cap))
				if (ge_tx_deq_delay(pAd, tr_entry, deq_qid))
					break;
#endif

			/* start to dequeue */
			pQueue = &tr_entry->tx_queue[deq_qid];
			qEntry = NULL;
			RTMP_IRQ_LOCK(&tr_entry->txq_lock[deq_qid], IrqFlags);
			if (pQueue->Head) {
				qEntry = RemoveHeadQueue(pQueue);
				TR_ENQ_COUNT_DEC(tr_entry);
			}
			RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[deq_qid], IrqFlags);
			if (qEntry) {
				pPacket = QUEUE_ENTRY_TO_PACKET(qEntry);
				ASSERT(RTMP_GET_PACKET_WCID(pPacket) == deq_wcid);
				InsertTailQueue(pTxPacketList, qEntry);
				deq_pkt_cnt++;
				(*deq_quota)--;
			}
			RTMP_IRQ_LOCK(&fifo_swq->swq_lock, IrqFlags);
			{
			fifo_swq->swq[fifo_swq->deqIdx] = 0;
			INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
			}
			RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, IrqFlags);
		}
		/* deq report */
		if (deq_pkt_cnt) {
			RTMP_IRQ_LOCK(&fifo_swq->swq_lock, IrqFlags);
			if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, deq_qid) &&
				ge_get_swq_free_num(pAd, deq_qid) > fifo_swq->high_water_mark) {
				tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, FALSE, deq_qid);
			}
			RTMP_IRQ_UNLOCK(&fifo_swq->swq_lock, IrqFlags);
		}
	}

	return pTxPacketList->Number;
}

static INT32 ge_deq_data_pkt(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info)
{
	INT ret = NDIS_STATUS_SUCCESS;

	ge_deq_req(pAd, max_cnt, info);

	/* wait for next schedule period to service de-queue pkt */
	if (info->status == NDIS_STATUS_FAILURE)
		return NDIS_STATUS_FAILURE;

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
		"deq_info:cur_wcid=%d, cur_qidx=%d, pkt_cnt=%d, pkt_bytes=%d\n",
		info->cur_wcid, info->cur_q, info->pkt_cnt, info->pkt_bytes);

	ret = deq_packet_gatter(pAd, info, tx_blk);

	if (!ret) {
		ge_deq_report(pAd, info);
	}
	return ret;
}

VOID RTMPRxDataDeqOffloadToOtherCPU(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);

	tm_ops->schedule_task(pAd, RX_DEQ_TASK, 0);
}

VOID ge_tx_pkt_deq_func(RTMP_ADAPTER *pAd, UINT8 idx)
{
	BOOLEAN need_schedule = TRUE;
	INT Count = 0;
	struct _TX_BLK TxBlk, *pTxBlk = &TxBlk;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *wdev_ops;
	struct dequeue_info deq_info = {0};
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	INT32 max_cnt = tr_ctl->max_tx_process;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
#ifdef HWIFI_SUPPORT
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);
#endif

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

#ifdef CONFIG_TX_DELAY
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
#endif


	if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE)) {
		pAd->tx_schedule_run = 0;
		return;
	}

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
		pAd->tx_schedule_run = 0;
		return;
	}
#endif /* ERR_RECOVERY */

	deq_info.target_wcid = WCID_ALL;
	deq_info.target_que = WMM_NUM_OF_AC;


	while (need_schedule) {
		NdisZeroMemory((UCHAR *)pTxBlk, sizeof(struct _TX_BLK));
		if (!ge_deq_mgmt_pkt(pAd, pTxBlk))
			goto pkt_handle;

		if (!ge_deq_high_prio_pkt(pAd, pTxBlk))
			goto pkt_handle;

		if (qm_ops->deq_data_pkt(pAd, pTxBlk, max_cnt, &deq_info)) {
			need_schedule = FALSE;
			break;
		}
pkt_handle:
		if (pTxBlk->TotalFrameNum) {
			if (!(pTxBlk->wdev)) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
					"assert wdev null\n");
				break;
			}
			wdev = pTxBlk->wdev;
			wdev_ops = wdev->wdev_ops;
			wdev_ops->tx_pkt_handle(pAd, wdev, pTxBlk);
			Count += pTxBlk->TotalFrameNum;
		}
	}
#ifdef DBG_DEQUE
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
			 "DeQueueRule:WCID[%d], Que[%d]\n",
			  deq_info.target_wcid, deq_info.target_que);

	if (pAd->DiagStruct.inited) {
		struct dbg_diag_info *diag_info;

		diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
		diag_info->deq_called++;
		diag_info->deq_round += round;

		if (Count < 8)
			diag_info->deq_cnt[Count]++;
		else
			diag_info->deq_cnt[8]++;
	}
#endif /* DBG_DEQUE */


#ifdef HWIFI_SUPPORT
	if (mac_dev->ops->tx_kick)
		mac_dev->ops->tx_kick(&mac_dev->hw);
#endif

	if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, idx) == TRUE)
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	pAd->tx_schedule_run = 0;

#ifdef CONFIG_TX_DELAY
	tx_delay_ctl->force_deq = FALSE;
#endif
}

static INT ge_bss_clean_queue(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	INT sta_idx;
	INT qidx;
	struct _STA_TR_ENTRY *tr_entry;
	/*TODO: add check de-queue task idle*/
	qm_leave_queue_pkt(wdev, PD_GET_QM_MGMT_SWQ(ad->physical_dev, 0),
		PD_GET_QM_MGMT_SWQ_LOCK(ad->physical_dev, 0));
	/*leave per sta/ac queue*/
	for (sta_idx = 0; IS_WCID_VALID(ad, sta_idx); sta_idx++) {

		tr_entry = tr_entry_get(ad, sta_idx);

		if (tr_entry->wdev != wdev)
			continue;

		for (qidx = 0; qidx < WMM_QUE_NUM ; qidx++) {
			qm_leave_queue_pkt(wdev, &tr_entry->tx_queue[qidx], &tr_entry->txq_lock[qidx]);
	}
	}
	return NDIS_STATUS_SUCCESS;
}

static INT ge_qm_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	struct tx_swq_fifo *fifo_swq;

	NdisAllocateSpinLock(pAd, PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, band_idx));
	InitializeQueueHeader(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, band_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_HIGH_PRI_SWQ_LOCK(pAd->physical_dev));
	InitializeQueueHeader(PD_GET_QM_HIGH_PRI_SWQ(pAd->physical_dev));
	pAd->tx_schedule_run = 0;

	pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;

	/* init fifo swq by dynamically allocate */
	os_alloc_mem(NULL, (UCHAR **)&fifo_swq, WMM_NUM_OF_AC * sizeof(struct tx_swq_fifo));

	if (!fifo_swq) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"os_alloc_mem p_fifo_swq[%d] fail\n", band_idx);
	}

	PD_SET_QM_TX_SWQ_PER_BAND(pAd->physical_dev, band_idx, fifo_swq);

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		NdisAllocateSpinLock(pAd, PD_GET_QM_TX_SWQ_LOCK(pAd->physical_dev, band_idx, i));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		fifo_swq = PD_GET_QM_TX_SWQ(pAd->physical_dev, band_idx, i);
		NdisZeroMemory(fifo_swq, sizeof(struct tx_swq_fifo));
		fifo_swq->low_water_mark = 5;
		fifo_swq->high_water_mark = TX_SWQ_FIFO_LEN>>2;
	}


#ifdef DBG_AMSDU
	pAd->dbg_time_slot = 0;
	RTMPInitTimer(pAd, &pAd->amsdu_history_timer, GET_TIMER_FUNCTION(amsdu_history_exec), pAd, TRUE);
	RTMPSetTimer(&pAd->amsdu_history_timer, 1000);
#endif
	tr_ctl->amsdu_fix_num = 8;
	tr_ctl->amsdu_fix = TRUE;
	tr_ctl->max_tx_process = cap->max_tx_process ? cap->max_tx_process : MAX_TX_PROCESS;

#ifdef IP_ASSEMBLY
	for (i = 0; i < WMM_NUM_OF_AC; i++)
		DlListInit(&pAd->assebQueue[i]);
#endif

	return NDIS_STATUS_SUCCESS;
}

VOID RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN in_hwIRQ,
	IN UCHAR QIdx,
	IN INT wcid,
	IN INT max_cnt)
{
}

#ifdef CONFIG_TX_DELAY
static INT ge_tx_delay_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	if (cap->tx_delay_support) {
		tx_delay_ctl->force_deq = FALSE;
		tx_delay_ctl->que_agg_en = FALSE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
		chip_tx_deley_parm_init(pAd->hdev_ctrl, cap->tx_delay_mode, tx_delay_ctl);

		if (IS_TX_DELAY_SW_MODE(cap)) {
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
				MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
						TX_DELAY_MODE_ARG1_TX_BATCH_CNT, tx_delay_ctl->tx_process_batch_cnt);

				MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
						TX_DELAY_MODE_ARG1_TX_DELAY_TIMEOUT_US, tx_delay_ctl->que_agg_timeout_value);

				MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
						TX_DELAY_MODE_ARG1_PKT_LENGTHS, tx_delay_ctl->min_pkt_len);
			} else {
				RTMPInitTimer(pAd, &tx_delay_ctl->que_agg_timer, GET_TIMER_FUNCTION(que_agg_timeout), pAd, FALSE);
			}
		}
	} else {
		tx_delay_ctl->que_agg_en = FALSE;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif


struct qm_ops ge_qm_ops = {
	.init_perband = ge_qm_init,
	.exit_perband = ge_qm_exit,
	.enq_mgmtq_pkt = ge_enq_mgmtq_pkt,
	.enq_dataq_pkt = ge_enq_dataq_pkt,
	.deq_tx_pkt = ge_tx_pkt_deq_func,
	.get_psq_pkt = ge_get_psq_pkt,
	.enq_psq_pkt = ge_enq_psq_pkt,
	.enq_delayq_pkt = ge_enq_delayq_pkt,
	.deq_delayq_pkt = ge_deq_delayq_pkt,
	.schedule_tx_que = ge_schedule_tx_que,
	.sta_clean_queue = ge_sta_clean_queue,
	.sta_dump_queue = ge_sta_dump_queue,
	.bss_clean_queue = ge_bss_clean_queue,
	.dump_all_sw_queue = ge_dump_all_sw_queue,
	.deq_data_pkt = ge_deq_data_pkt,
	.deq_data_pkt_v2 = ge_deq_data_pkt_v2,
#ifdef CONFIG_TX_DELAY
	.tx_delay_init = ge_tx_delay_init,
#endif
};

extern struct qm_ops fp_qm_ops;

VOID qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue, NDIS_SPIN_LOCK *lock)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _QUEUE_HEADER local_q;
	struct _QUEUE_ENTRY *q_entry;
	NDIS_PACKET *pkt;

	InitializeQueueHeader(&local_q);
	OS_SEM_LOCK(lock);

	/*remove entry owned by wdev*/
	do {
		q_entry = RemoveHeadQueue(queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WDEV(pkt) == wdev->wdev_idx)
			RELEASE_NDIS_PACKET(ad, pkt, NDIS_STATUS_SUCCESS);
		else
			InsertTailQueue(&local_q, q_entry);
	} while (1);

	/*re-enqueue other entries*/
	do {
		q_entry = RemoveHeadQueue(&local_q);

		if (!q_entry)
			break;

		InsertTailQueue(queue, q_entry);
	} while (1);

	OS_SEM_UNLOCK(lock);
}

static INT qm_for_wsys_notify_handle(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct wsys_notify_info *info = data;
	struct _RTMP_ADAPTER *ad = ne->priv;
	struct wifi_dev *wdev = info->wdev;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(ad->physical_dev);
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *)info->v;

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
		"event_id: %d, wdev=%d\n", event_id, info->wdev->wdev_idx);

	switch (event_id) {
	case WSYS_NOTIFY_CLOSE:
		if (qm_ops->bss_clean_queue)
			qm_ops->bss_clean_queue(ad, wdev);
		break;
	case WSYS_NOTIFY_DISCONNT_ACT:
		if (qm_ops->sta_clean_queue)
			qm_ops->sta_clean_queue(ad, tr_entry->wcid);
		break;
	case WSYS_NOTIFY_OPEN:
	case WSYS_NOTIFY_CONNT_ACT:
	case WSYS_NOTIFY_LINKUP:
	case WSYS_NOTIFY_LINKDOWN:
	case WSYS_NOTIFY_STA_UPDATE:
	default:
		break;
	}
	return ret;
}

static INT qm_notify_register(struct _RTMP_ADAPTER *ad)
{
	INT ret;
	struct notify_entry *ne = PD_GET_QM_WSYS_NE(ad->physical_dev, hc_get_hw_band_idx(ad));

	/*fill notify entry for wifi system chain*/
	ne->notify_call = qm_for_wsys_notify_handle;
	ne->priority = WSYS_NOTIFY_PRIORITY_QM;
	ne->priv = ad;
	/*register wifi system notify chain*/
	ret = register_wsys_notifier(ad->WifiSysInfo, ne);
	return ret;
}

static INT qm_notify_unregister(struct _RTMP_ADAPTER *ad)
{
	INT ret;
	struct notify_entry *ne = PD_GET_QM_WSYS_NE(ad->physical_dev, hc_get_hw_band_idx(ad));

	/*register wifi system notify chain*/
	ret = unregister_wsys_notifier(ad->WifiSysInfo, ne);
	return ret;
}


INT qm_init(VOID *ph_dev)
{
	RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(ph_dev);
	struct qm_ops *qm_ops = NULL;
	INT ret = NDIS_STATUS_SUCCESS;

	if (cap->qm == GENERIC_QM) {
		PD_SET_QM_OPS(ph_dev, &ge_qm_ops);
	} else if (cap->qm == FAST_PATH_QM) {
		PD_SET_QM_OPS(ph_dev, &fp_qm_ops);
	}

	qm_ops = PD_GET_QM_OPS(ph_dev);
	if (qm_ops->init_physical_dev)
		ret = qm_ops->init_physical_dev(ph_dev);

	return ret;
}

INT qm_exit(VOID *ph_dev)
{
	INT ret = NDIS_STATUS_SUCCESS;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(ph_dev);

	if (qm_ops->exit_physical_dev)
		ret = qm_ops->exit_physical_dev(ph_dev);

	return ret;
}

INT qm_init_perband(RTMP_ADAPTER *pAd)
{
	struct qm_ops *qm_ops = NULL;
	INT ret;

	qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	ret = qm_ops->init_perband(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

#ifdef CONFIG_TX_DELAY
	ret = qm_ops->tx_delay_init(pAd);
#endif
	/*register qm related notify chain*/
	qm_notify_register(pAd);
	return ret;
}

INT qm_exit_perband(RTMP_ADAPTER *pAd)
{
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	INT ret;

	/*unregister qm related notify chain*/
	qm_notify_unregister(pAd);
	ret = qm_ops->exit_perband(pAd);
	return ret;
}

struct _QUEUE_ENTRY *remove_queue_head(struct _QUEUE_HEADER *que_h)
{
	struct _QUEUE_ENTRY *head, *next;

	head = que_h->Head;

	if (head) {
		next = head->Next;
		head->Next = NULL;
		que_h->Head = next;
		if (!next)
			que_h->Tail = NULL;
		que_h->Number--;
	}

	return head;
}
EXPORT_SYMBOL(remove_queue_head);
