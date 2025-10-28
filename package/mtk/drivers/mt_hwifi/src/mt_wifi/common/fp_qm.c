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

static INT32 fp_tx_flow_ctl_init(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;
	UINT8 que_idx = hc_get_hw_band_idx(pAd);
	struct fp_tx_flow_control *flow_ctl = NULL;
	struct wifi_dev *wdev = NULL;

	flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(pAd->physical_dev);

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));

	flow_ctl->TxFlowBlockState[que_idx] = 0;

	while (1) {
		wdev = DlListFirst(&flow_ctl->TxBlockDevList[que_idx],
							struct wifi_dev, tx_block_list);

		if (!wdev)
			break;

		DlListDel(&wdev->tx_block_list);
		RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
	}

	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));

	return ret;
}

static INT32 fp_tx_flow_block(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
						UINT8 state, BOOLEAN block, UINT8 q_idx)
{
	INT32 ret = 0;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct fp_tx_flow_control *flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(pAd->physical_dev);
	struct wifi_dev *wdev_block = NULL;

	if (block && test_bit(0, &flow_ctl->flag)) {
		DlListForEach(wdev_block, &flow_ctl->TxBlockDevList[q_idx], struct wifi_dev, tx_block_list) {
			if (wdev_block == wdev)
				return ret;
		}
		RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
		tr_cnt->net_if_stop_cnt++;
		DlListAddTail(&flow_ctl->TxBlockDevList[q_idx], &wdev->tx_block_list);
		set_bit(state, &flow_ctl->TxFlowBlockState[q_idx]);
	} else {
		if (test_and_clear_bit(state, &flow_ctl->TxFlowBlockState[q_idx])) {
			while (1) {
				wdev_block = DlListFirst(&flow_ctl->TxBlockDevList[q_idx],
									struct wifi_dev, tx_block_list);

				if (!wdev_block)
					break;

				DlListDel(&wdev_block->tx_block_list);
				RTMP_OS_NETDEV_WAKE_QUEUE(wdev_block->if_dev);
			}
		}
	}

	return ret;
}

inline BOOLEAN fp_get_queue_state(struct _QUEUE_HEADER *que)
{
	return test_bit(0, &que->state);
}

inline INT fp_set_queue_state(struct _QUEUE_HEADER *que, BOOLEAN state)
{
	if (state == TX_QUE_LOW)
		set_bit(0, &que->state);
	else
		clear_bit(0, &que->state);

	return NDIS_STATUS_SUCCESS;
}

static inline NDIS_PACKET *fp_get_tx_data_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

#ifdef IGMP_SNOOPING_OFFLOAD
static inline NDIS_PACKET *fp_get_tx_bmc_data_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_BMC_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}
#endif

#ifdef HIGH_PRIO_QUEUE_SUPPORT
static inline NDIS_PACKET *fp_get_tx_highpri_data_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_HIGHPRI_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}
#endif

static inline NDIS_PACKET *fp_get_tx_mgmt_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = RemoveHeadQueue(PD_GET_QM_MGMT_POST_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static inline VOID fp_queue_deep_counting(RTMP_ADAPTER *pAd, struct tr_counter *tr_cnt, UINT8 idx)
{
	UINT32 queue_num = 0;
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT16 counting_range = qm_parm->max_data_que_num / (queue_deep_cnt_steps-1);

	queue_num += PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, idx);

	if (queue_num == 0)
		return;

	tr_cnt->queue_deep_cnt[queue_num/counting_range]++;
}


#ifdef DSCP_PRI_SUPPORT
INT8 get_dscp_mapped_priority(
		IN  PRTMP_ADAPTER pAd,
		IN  PNDIS_PACKET  pPkt)
{
	INT8 pri = -1;
	UINT8 dscpVal = 0;
	PUCHAR	pPktHdr = NULL;
	UINT16 protoType;
	struct wifi_dev *wdev;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return pri;

	protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));

	pPktHdr += LENGTH_802_3;

	if (protoType <= 1500) {
		/* 802.3, 802.3 LLC: DestMAC(6) + SrcMAC(6) + Length (2) + DSAP(1) + SSAP(1) + Control(1) + */
		/* if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.*/
		/*	=> + SNAP (5, OriginationID(3) + etherType(2)) */
		/* else */
		/*	=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it */
		if (pPktHdr[0] == 0xAA && pPktHdr[1] == 0xAA && pPktHdr[2] == 0x03) {
			pPktHdr += 6; /* Advance to type LLC 3byte + SNAP OriginationID 3 Byte*/
			protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr)));
		} else {
			return pri;
		}
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (protoType == ETH_TYPE_VLAN) {
		pPktHdr += 2; /* Skip the VLAN Header.*/
		protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr)));
	}

	switch (protoType) {
	case 0x0800:
		dscpVal = ((pPktHdr[1] & 0xfc) >> 2);
		break;
	case 0x86DD:
			dscpVal = (((pPktHdr[0] & 0x0f) << 2) | ((pPktHdr[1] & 0xc0) >> 6));
		break;
	default:
		return pri;
	}

	if (dscpVal <= 63) {
		UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPkt);

		wdev = get_wdev_by_idx(pAd, wdev_idx);
		if (!wdev)
			return pri;
		if ((wdev->func_idx) >= 0)
			pri = pAd->ApCfg.MBSSID[wdev->func_idx].dscp_pri_map[dscpVal];
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			"ApIdx:%d dscp value:%d local PRI:%d\n",
			wdev->func_idx, dscpVal, pri);
	}

	return pri;
}
#endif /*DSCP_PRI_SUPPORT*/

DECLARE_TIMER_FUNCTION(fp_que_agg_timeout);

VOID fp_que_agg_timeout(PVOID SystemSpecific1, PVOID func_context, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct tx_delay_control *tx_delay_ctl = (struct tx_delay_control *)func_context;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)tx_delay_ctl->priv;
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	UINT8 idx = tx_delay_ctl->idx;

	if (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, idx)) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
		tx_delay_ctl->force_deq = TRUE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
	}
}

BUILD_TIMER_FUNCTION(fp_que_agg_timeout);

static BOOLEAN fp_ge_tx_deq_delay(RTMP_ADAPTER *pAd, UINT8 idx)
{
	NDIS_PACKET *pkt = NULL;
	BOOLEAN is_udp;
	struct _QUEUE_HEADER *que;
	UINT32 q_nums = 0;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	if (!(tx_delay_ctl->que_agg_en) ||
		(tx_delay_ctl->force_deq) ||
		PD_GET_QM_MGMT_POST_SWQ_LEN(pAd->physical_dev, idx) > 0) {
		return FALSE;
	}

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));
	que = PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, idx);
	if (que->Head)
		pkt = QUEUE_ENTRY_TO_PACKET(que->Head);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));

	if (pkt) {
		q_nums = que->Number;
		is_udp = is_udp_packet(pAd, pkt);
	}

	if ((q_nums > 0) &&
		(q_nums < tx_delay_ctl->tx_process_batch_cnt) &&
		!is_udp) {

			if (!tx_delay_ctl->que_agg_timer_running) {
				RTMPSetTimer(&tx_delay_ctl->que_agg_timer, tx_delay_ctl->que_agg_timeout_value / 1000);
				tx_delay_ctl->que_agg_timer_running = TRUE;
			}

			return TRUE;
	}

	return FALSE;
}

static inline VOID fp_merge_post_que(
	struct _QUEUE_HEADER *que, struct _QUEUE_HEADER *post_que)
{
	if (post_que->Number == 0) {
		post_que->Head = que->Head;
		post_que->Tail = que->Tail;
		post_que->Number = que->Number;
	} else {
		if (que->Number) {
			post_que->Tail->Next = que->Head;
			post_que->Tail = que->Tail;
			post_que->Number += que->Number;
		}
	}

	post_que->state = que->state;
	que->Head = NULL;
	que->Tail = NULL;
	que->Number = 0;
}

static inline VOID fp_init_post_que(RTMP_ADAPTER *pAd, UINT8 idx)
{
	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, idx));
	fp_merge_post_que(
		PD_GET_QM_MGMT_SWQ(pAd->physical_dev, idx),
		PD_GET_QM_MGMT_POST_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, idx));

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
	fp_merge_post_que(
		PD_GET_QM_FP_SWQ(pAd->physical_dev, idx),
		PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));

#ifdef IGMP_SNOOPING_OFFLOAD
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, idx));
	fp_merge_post_que(
		PD_GET_QM_FP_BMC_SWQ(pAd->physical_dev, idx),
		PD_GET_QM_FP_POST_BMC_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, idx));
#endif

#ifdef HIGH_PRIO_QUEUE_SUPPORT
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
	fp_merge_post_que(
		PD_GET_QM_FP_HIGHPRI_SWQ(pAd->physical_dev, idx),
		PD_GET_QM_FP_POST_HIGHPRI_SWQ(pAd->physical_dev, idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
#endif
}

static inline NDIS_PACKET *fp_first_tx_mgmt(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = PD_GET_QM_MGMT_POST_SWQ(pAd->physical_dev, idx)->Head;
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static inline NDIS_PACKET *fp_first_tx_data(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, idx)->Head;
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

#ifdef IGMP_SNOOPING_OFFLOAD
static inline NDIS_PACKET *fp_first_tx_bmc_data(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = PD_GET_QM_FP_POST_BMC_SWQ(pAd->physical_dev, idx)->Head;
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}
#endif

#ifdef HIGH_PRIO_QUEUE_SUPPORT
static inline NDIS_PACKET *fp_first_tx_highpri_data(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _QUEUE_ENTRY *q_entry;

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
	q_entry = PD_GET_QM_FP_POST_HIGHPRI_SWQ(pAd->physical_dev, idx)->Head;
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}
#endif

static inline struct mtk_mac_dev *
pad_to_mac_dev(struct _RTMP_ADAPTER *ad)
{
	struct os_cookie *handle = ad->OS_Cookie;

	return handle->mac_dev;
}

#ifdef HWIFI_SUPPORT
VOID fp_hwifi_tx_pkt_deq_func(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct _TX_BLK blk, *tx_blk;
	NDIS_PACKET *pkt = NULL;
	UINT16 wcid = 0;
	INT ret = 0;
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev_ops *wdev_ops;
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT8 need_schedule = (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, idx) ? TRUE : FALSE);
	UINT16 tx_process_cnt = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	INT32 cpu_id;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(pAd);
	bool is_mgmt = false, legacy_resource = true;
	bool error = false;
#ifdef IGMP_SNOOPING_OFFLOAD
	bool is_bmc = false, bmc_resource = true;
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	bool is_highpri = false, highpri_resource = true;
#endif

	if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE))
		return;

	fp_init_post_que(pAd, idx);

	fp_queue_deep_counting(pAd, tr_cnt, idx);

	while (need_schedule &&
			(tx_process_cnt < qm_parm->max_tx_process_cnt) &&
			(legacy_resource
#ifdef IGMP_SNOOPING_OFFLOAD
			|| bmc_resource
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
			|| highpri_resource
#endif
			)) {
		struct mtk_mac_bss *bss;
		struct mtk_mac_sta *sta;
		struct mtk_mac_txq *txq;
		is_mgmt = false;
		pkt = NULL;
		error = false;
#ifdef HIGH_PRIO_QUEUE_SUPPORT
		is_highpri = false;
#endif
#ifdef IGMP_SNOOPING_OFFLOAD
		is_bmc = false;
#endif

		NdisZeroMemory((UCHAR *)&blk, sizeof(struct _TX_BLK));
		tx_blk = &blk;

		do {
#ifdef HIGH_PRIO_QUEUE_SUPPORT
			if (highpri_resource) {
				pkt = fp_first_tx_mgmt(pAd, idx);
				if (pkt) {
					is_mgmt = true;
					tx_blk->TxResourceType = TX_HIGHPRIO_RESOURCE;
					break;
				}

				pkt = fp_first_tx_highpri_data(pAd, idx);
				if (pkt) {
					is_highpri = true;
					tx_blk->TxResourceType = TX_HIGHPRIO_RESOURCE;
					break;
				}
			}
#else
			if (legacy_resource) {
				pkt = fp_first_tx_mgmt(pAd, idx);
				if (pkt) {
					is_mgmt = true;
					tx_blk->TxResourceType = TX_LEGACY_RESOURCE;
					break;
				}
			}
#endif

#ifdef IGMP_SNOOPING_OFFLOAD
			if (bmc_resource) {
				pkt = fp_first_tx_bmc_data(pAd, idx);
				if (pkt) {
					is_bmc = true;
					tx_blk->TxResourceType = TX_BMC_RESOURCE;
					break;
				}
			}
#endif
			if (legacy_resource) {
				pkt = fp_first_tx_data(pAd, idx);
				if (pkt) {
					tx_blk->TxResourceType = TX_LEGACY_RESOURCE;
					break;
				}
			}
		} while (0);

		if (!pkt)
			break;

		wdev = wdev_search_by_pkt(pAd, pkt);

		if (wdev != NULL) {
			wcid = RTMP_GET_PACKET_WCID(pkt);

			if (wcid >= wtbl_max_num) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
					"WCID is invalid\n");
				tr_cnt->tx_wcid_invalid++;
				error = true;
				goto error;
			}

			tr_entry = tr_entry_get(pAd, wcid);
			mt_rcu_read_lock();
			sta = tr_entry->mac_sta;
			if (is_mgmt) {
				if (!sta) {
					bss = wdev->pHObj;
					if (bss)
						sta = bss->bmc_sta;

					if (!sta) {
						tr_cnt->tx_invalid_mgmt_drop++;
						error = true;
						goto error;
					}
				}

				txq = sta->txq[MGMT_TXQ];
			} else {
				if (!sta) {
					tr_cnt->tx_invalid_data_drop++;
					error = true;
					goto error;
				}

				tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);

				if (tx_blk->UserPriority < TXQ_NUM)
					txq = sta->txq[tx_blk->UserPriority];
				else
					txq = sta->txq[0];

				bss = NULL;
			}

			if (!mac_dev->ops->tx_check_resource(&mac_dev->hw, txq, tx_blk->TxResourceType)) {
				mt_rcu_read_unlock();
				switch (tx_blk->TxResourceType) {
				case TX_LEGACY_RESOURCE:
					legacy_resource = false;
					break;
#ifdef HIGH_PRIO_QUEUE_SUPPORT
				case TX_HIGHPRIO_RESOURCE:
					highpri_resource = false;
					break;
#endif
#ifdef IGMP_SNOOPING_OFFLOAD
				case TX_BMC_RESOURCE:
					bmc_resource = false;
					break;
#endif
				default:
					break;
				}

				continue;
			}

			tx_blk->wdev = wdev;
		} else {
			tr_cnt->tx_invalid_wdev++;
			error = true;
			goto error;
		}
error:
		mt_rcu_read_unlock();

		if (is_mgmt)
			pkt = fp_get_tx_mgmt_element(pAd, idx);
#ifdef HIGH_PRIO_QUEUE_SUPPORT
		else if (is_highpri)
			pkt = fp_get_tx_highpri_data_element(pAd, idx);
#endif
#ifdef IGMP_SNOOPING_OFFLOAD
		else if (is_bmc)
			pkt = fp_get_tx_bmc_data_element(pAd, idx);
#endif
		else
			pkt = fp_get_tx_data_element(pAd, idx);

		if (pkt == NULL)
			continue;

		if (error) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		if (!is_mgmt) {
			/* only count data packets */
			cpu_id = smp_processor_id();

			if ((cpu_id < 4) && (cpu_id >= 0))
				tr_cnt->tx_deq_cpu_stat[cpu_id]++;
		}

#ifdef SW_CONNECT_SUPPORT
		if (RTMP_GET_PACKET_EAPOL(pkt)) {
			tr_entry->tx_deq_eap_cnt++;
		} else {
			tr_entry->tx_deq_data_cnt++;
			if (RTMP_GET_PACKET_ARP(pkt))
				tr_entry->tx_deq_arp_cnt++;
		}
#endif /* SW_CONNECT_SUPPORT */

		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		if (tx_blk->TxFrameType == TX_AMSDU_FRAME)
			tx_blk->TxFrameType = TX_LEGACY_FRAME;
		tx_blk->Wcid = wcid;
		tx_blk->pMacEntry = entry_get(pAd, wcid);

		if (RTMP_GET_PACKET_SN_VLD(pkt)) {
			tx_blk->sn = RTMP_GET_PACKET_SN(pkt);
			TX_BLK_SET_FLAG2(tx_blk, fTX_bSnVld);
		}

		wdev_ops = wdev->wdev_ops;
		ret = wdev_ops->tx_pkt_handle(pAd, wdev, tx_blk);

		tx_process_cnt++;
	}

	if (mac_dev->ops->tx_kick)
		mac_dev->ops->tx_kick(&mac_dev->hw);

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
	if ((PD_GET_QM_FP_SWQ_LEN(pAd->physical_dev, idx) + PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, idx))
			< (qm_parm->max_data_que_num  - 100)) {
		fp_tx_flow_block(pAd, NULL, 0, FALSE, idx);
	}
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
}
#endif /* HWIFI_SUPPORT */

#ifdef CONFIG_TX_DELAY
static struct tx_delay_control *fp_get_qm_delay_ctl(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	return &tr_ctl->tx_delay_ctl;
}

static INT fp_tx_delay_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tx_delay_control *tx_delay_ctl = NULL;
	UINT8 idx = hc_get_hw_band_idx(pAd);

	tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
	if (cap->tx_delay_support) {
		tx_delay_ctl->force_deq = FALSE;
		tx_delay_ctl->que_agg_en = FALSE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
		tx_delay_ctl->idx = idx;
		tx_delay_ctl->priv = pAd;
		chip_tx_deley_parm_init(pAd->hdev_ctrl, cap->tx_delay_mode, tx_delay_ctl);
		if (IS_TX_DELAY_SW_MODE(cap))
			RTMPInitTimer(pAd, &tx_delay_ctl->que_agg_timer,
					GET_TIMER_FUNCTION(fp_que_agg_timeout), tx_delay_ctl, FALSE);
	} else {
		tx_delay_ctl->que_agg_en = FALSE;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

static INT fp_enq_mgmtq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT8 idx;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		tr_cnt->wlan_state_non_valid_drop++;
		goto error;
	}

	idx = hc_get_hw_band_idx(pAd);

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, idx));
	if ((PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, idx) + PD_GET_QM_MGMT_POST_SWQ_LEN(pAd->physical_dev, idx))
			>= qm_parm->max_mgmt_que_num) {
		OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, idx));
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
		tr_cnt->tx_sw_mgmtq_drop++;
		goto error;
	}

	InsertTailQueue(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, idx), PACKET_TO_QUEUE_ENTRY(pkt));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, idx));

	tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	return NDIS_STATUS_SUCCESS;
error:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

INT32 fp_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	UINT8 idx;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	INT32 cpu_id;
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT32 data_qm_number;
	UINT32 cpu_idx, cpu_num;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		tr_cnt->wlan_state_non_valid_drop++;
		goto drop;
	}

	idx = hc_get_hw_band_idx(pAd);

#ifdef IGMP_SNOOPING_OFFLOAD
	if (RTMP_GET_PACKET_TXTYPE(pkt) == TX_BMC_FRAME) {
		OS_SPIN_LOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, idx));
		data_qm_number = PD_GET_QM_FP_BMC_SWQ_LEN(pAd->physical_dev, idx) +
						PD_GET_QM_FP_POST_BMC_SWQ_LEN(pAd->physical_dev, idx);

		if (data_qm_number >= qm_parm->max_data_que_num) {
			OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, idx));
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
			goto error;
		}
#ifdef DABS_QOS
		if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM)
			dabs_active_qos_by_ipaddr(pAd, pkt);
#endif
		MEM_DBG_PKT_RECORD(pkt, 1<<3);

		InsertTailQueue(PD_GET_QM_FP_BMC_SWQ(pAd->physical_dev, idx), PACKET_TO_QUEUE_ENTRY(pkt));
		OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, idx));
	} else
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	if (RTMP_GET_PACKET_HIGH_PRIO(pkt)) {
		OS_SPIN_LOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
		data_qm_number = PD_GET_QM_FP_HIGHPRI_SWQ_LEN(pAd->physical_dev, idx) +
						PD_GET_QM_FP_POST_HIGHPRI_SWQ_LEN(pAd->physical_dev, idx);
		if (data_qm_number >= qm_parm->max_highpri_que_num) {
			OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
			goto error;
		}
#ifdef DABS_QOS
		if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM)
			dabs_active_qos_by_ipaddr(pAd, pkt);
#endif
		MEM_DBG_PKT_RECORD(pkt, 1<<3);
		InsertTailQueue(PD_GET_QM_FP_HIGHPRI_SWQ(pAd->physical_dev, idx), PACKET_TO_QUEUE_ENTRY(pkt));
		OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, idx));
	} else
#endif
	{
		OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
		data_qm_number = PD_GET_QM_FP_SWQ_LEN(pAd->physical_dev, idx) +
						PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, idx);

		if (data_qm_number >= qm_parm->max_data_que_num) {
			fp_tx_flow_block(pAd, wdev, 0, TRUE, idx);

			if (data_qm_number >= qm_parm->max_data_que_num +
					qm_parm->extra_reserved_que_num) {
				OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
				tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
				goto error;
			}
		}
#ifdef DABS_QOS
		if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM)
			dabs_active_qos_by_ipaddr(pAd, pkt);
#endif
		MEM_DBG_PKT_RECORD(pkt, 1<<3);

		InsertTailQueue(PD_GET_QM_FP_SWQ(pAd->physical_dev, idx), PACKET_TO_QUEUE_ENTRY(pkt));
		OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, idx));
	}

	cpu_id = smp_processor_id();

	if ((cpu_id < 4) && (cpu_id >= 0))
		tr_cnt->tx_enq_cpu_stat[cpu_id]++;

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pkt)))
		pAd->tx_cnt.tx_pkt_enq_cnt[cpu_id]++;
#endif
	if (tm_ops->schedule_task_async_on) {
		cpu_num = num_online_cpus();
		cpu_idx = (cpu_id + 1) % cpu_num;
		tm_ops->schedule_task_async_on(pAd, cpu_idx, TX_DEQ_TASK, idx);
	}
	else
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	return NDIS_STATUS_SUCCESS;

error:
	tr_cnt->tx_sw_dataq_drop++;
drop:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT fp_qm_init_physical_dev(VOID *ph_dev)
{
	struct fp_qm *qm_parm = NULL;
	struct qm_ops *qm_ops = NULL;
	ULONG *pTxFlowBlockState = NULL;
	DL_LIST *pTxBlockDevList = NULL;
	struct fp_tx_flow_control *flow_ctl = NULL;
	UINT8 idx = 0;

	/* init qm parameter */
	qm_parm = PD_GET_QM_PARM(ph_dev);
#ifdef EXPERIMENTAL_FEATURES
	qm_parm->max_data_que_num = 8192;
	qm_parm->max_tx_process_cnt = 512;
#else
	qm_parm->max_data_que_num = 4096;
	qm_parm->max_tx_process_cnt = 8192;
#endif
	qm_parm->max_mgmt_que_num = 512;
	qm_parm->extra_reserved_que_num = 1024;
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	qm_parm->max_highpri_que_num = 512;
#endif
	/* reassign qm ops */
	qm_ops = PD_GET_QM_OPS(ph_dev);

	qm_ops->enq_dataq_pkt = fp_enq_dataq_pkt;
	qm_ops->tx_deq_delay = fp_ge_tx_deq_delay;
	qm_ops->deq_tx_pkt = fp_hwifi_tx_pkt_deq_func;

	/* init tx flow control */
	flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(ph_dev);
	os_alloc_mem(NULL, (UCHAR **)&pTxFlowBlockState, FP_QUE_NUM * sizeof(ULONG));

	if (!pTxFlowBlockState) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"%s os_alloc_mem pTxFlowBlockState fail\n", __func__);
		goto error1;
	}

	NdisZeroMemory(pTxFlowBlockState, FP_QUE_NUM * sizeof(ULONG));
	flow_ctl->TxFlowBlockState = pTxFlowBlockState;

	os_alloc_mem(NULL, (UCHAR **)&pTxBlockDevList, FP_QUE_NUM * sizeof(DL_LIST));

	if (!pTxBlockDevList) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"%s os_alloc_mem TxFlowBlockState fail\n", __func__);
		goto error0;
	}

	NdisZeroMemory(pTxBlockDevList, FP_QUE_NUM * sizeof(DL_LIST));

	flow_ctl->TxBlockDevList = pTxBlockDevList;
	set_bit(0, &flow_ctl->flag);

	for (idx = 0; idx < FP_QUE_NUM; idx++)
		DlListInit(&flow_ctl->TxBlockDevList[idx]);

	return NDIS_STATUS_SUCCESS;

error0:
	os_free_mem(pTxFlowBlockState);
error1:
	return NDIS_STATUS_FAILURE;

}

static INT fp_qm_exit_physical_dev(VOID *ph_dev)
{
	struct fp_tx_flow_control *flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(ph_dev);

	if (flow_ctl->TxFlowBlockState)
		os_free_mem(flow_ctl->TxFlowBlockState);

	if (flow_ctl->TxBlockDevList)
		os_free_mem(flow_ctl->TxBlockDevList);

	return NDIS_STATUS_SUCCESS;
}

static INT fp_qm_init_perband(RTMP_ADAPTER *pAd)
{
	UINT8 que_idx = hc_get_hw_band_idx(pAd);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct fp_qm *qm_parm = NULL;
#ifdef WHNAT_SUPPORT
	BOOLEAN multi_token_ques =
		pAd->CommonCfg.dbdc_mode &&
		!PD_GET_WHNAT_ENABLE(pAd->physical_dev) &&
		cap->multi_token_ques_per_band;
#else
	BOOLEAN multi_token_ques =
		pAd->CommonCfg.dbdc_mode &&
		cap->multi_token_ques_per_band;
#endif

	if (multi_token_ques) {
		qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);

		qm_parm->max_data_que_num = 2048;
		qm_parm->max_mgmt_que_num = 256;
		qm_parm->extra_reserved_que_num = 512;
	}

	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_SWQ(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, que_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_MGMT_POST_SWQ(pAd->physical_dev, que_idx));
#ifdef IGMP_SNOOPING_OFFLOAD
	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_BMC_SWQ(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_POST_BMC_SWQ(pAd->physical_dev, que_idx));
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisAllocateSpinLock(pAd, PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_HIGHPRI_SWQ(pAd->physical_dev, que_idx));
	InitializeQueueHeader(PD_GET_QM_FP_POST_HIGHPRI_SWQ(pAd->physical_dev, que_idx));
#endif
	fp_tx_flow_ctl_init(pAd);

	return NDIS_STATUS_SUCCESS;
}

static INT fp_qm_exit_perband(RTMP_ADAPTER *pAd)
{
	UINT8 que_idx = hc_get_hw_band_idx(pAd);
	struct _QUEUE_ENTRY *q_entry;
#ifdef CONFIG_TX_DELAY
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	BOOLEAN que_agg_timer_cancelled;
	struct tx_delay_control *tx_delay_ctl = NULL;
#endif
	struct wifi_dev *wdev = NULL;
	struct fp_tx_flow_control *flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(pAd->physical_dev);

#ifdef CONFIG_TX_DELAY
	tx_delay_ctl = &tr_ctl->tx_delay_ctl;
	RTMPReleaseTimer(&tx_delay_ctl->que_agg_timer, &que_agg_timer_cancelled);
#endif

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_MGMT_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_MGMT_POST_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));

	NdisFreeSpinLock(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_MGMT_POST_SWQ_LOCK(pAd->physical_dev, que_idx));

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_FP_POST_SWQ_LOCK(pAd->physical_dev, que_idx));

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));

	while (1) {
		wdev = DlListFirst(&flow_ctl->TxBlockDevList[que_idx], struct wifi_dev, tx_block_list);

		if (!wdev)
			break;

		DlListDel(&wdev->tx_block_list);
		RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
	}
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));

	NdisFreeSpinLock(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
#ifdef IGMP_SNOOPING_OFFLOAD
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_BMC_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_BMC_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_FP_POST_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_HIGHPRI_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	do {
		q_entry = RemoveHeadQueue(PD_GET_QM_FP_POST_HIGHPRI_SWQ(pAd->physical_dev, que_idx));

		if (!q_entry)
			break;

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
	} while (1);
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	NdisFreeSpinLock(PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
#endif
	return NDIS_STATUS_SUCCESS;
}

static INT fp_schedule_tx_que(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	UINT8 que_idx;

	que_idx = hc_get_hw_band_idx(pAd);

	if ((PD_GET_QM_FP_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_MGMT_POST_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#ifdef IGMP_SNOOPING_OFFLOAD
			|| (PD_GET_QM_FP_BMC_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_BMC_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
			|| (PD_GET_QM_FP_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#endif
			) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, que_idx);
	}

	return NDIS_STATUS_SUCCESS;
}

static INT fp_schedule_tx_que_on(RTMP_ADAPTER *pAd, int cpuid, UINT8 idx)
{
	struct tm_ops *tm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	UINT8 que_idx;

	que_idx = hc_get_hw_band_idx(pAd);

	if ((PD_GET_QM_FP_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_MGMT_POST_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#ifdef IGMP_SNOOPING_OFFLOAD
			|| (PD_GET_QM_FP_BMC_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_BMC_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
			|| (PD_GET_QM_FP_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
			|| (PD_GET_QM_FP_POST_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx) > 0)
#endif
			) {
		tm_ops->schedule_task_on(pAd, cpuid, TX_DEQ_TASK, que_idx);
	}

	return NDIS_STATUS_SUCCESS;
}

VOID fp_qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue,
						struct _QUEUE_HEADER *post_queue,
						NDIS_SPIN_LOCK *lock, NDIS_SPIN_LOCK *post_lock)
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

	OS_SEM_LOCK(post_lock);
	/*remove entry owned by wdev*/
	do {
		q_entry = RemoveHeadQueue(post_queue);

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

		InsertTailQueue(post_queue, q_entry);
	} while (1);

	OS_SEM_UNLOCK(post_lock);

	OS_SEM_UNLOCK(lock);
}

static INT fp_bss_clean_queue(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UINT8 que_idx = hc_get_hw_band_idx(ad);
	/*TODO: add check de-queue task idle*/
	fp_qm_leave_queue_pkt(wdev,
		PD_GET_QM_MGMT_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_POST_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_POST_SWQ_LOCK(ad->physical_dev, que_idx));
	fp_qm_leave_queue_pkt(wdev,
		PD_GET_QM_FP_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_SWQ_LOCK(ad->physical_dev, que_idx));
#ifdef IGMP_SNOOPING_OFFLOAD
	fp_qm_leave_queue_pkt(wdev,
		PD_GET_QM_FP_BMC_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_BMC_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_BMC_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_BMC_SWQ_LOCK(ad->physical_dev, que_idx));
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	fp_qm_leave_queue_pkt(wdev,
		PD_GET_QM_FP_HIGHPRI_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_HIGHPRI_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(ad->physical_dev, que_idx));
#endif

	return NDIS_STATUS_SUCCESS;
}

VOID fp_qm_sta_leave_queue_pkt(struct _STA_TR_ENTRY *tr_entry, struct _QUEUE_HEADER *queue,
						struct _QUEUE_HEADER *post_queue,
						NDIS_SPIN_LOCK *lock, NDIS_SPIN_LOCK *post_lock)
{
	struct _RTMP_ADAPTER *ad = tr_entry->wdev->sys_handle;
	struct _QUEUE_HEADER local_q;
	struct _QUEUE_ENTRY *q_entry;
	NDIS_PACKET *pkt;

	InitializeQueueHeader(&local_q);
	OS_SEM_LOCK(lock);

	/*remove entry owned by tr_entry*/
	do {
		q_entry = RemoveHeadQueue(queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WCID(pkt) == tr_entry->wcid)
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

	OS_SEM_LOCK(post_lock);
	/*remove entry owned by tr_entry*/
	do {
		q_entry = RemoveHeadQueue(post_queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WCID(pkt) == tr_entry->wcid)
			RELEASE_NDIS_PACKET(ad, pkt, NDIS_STATUS_SUCCESS);
		else
			InsertTailQueue(&local_q, q_entry);
	} while (1);

	/*re-enqueue other entries*/
	do {
		q_entry = RemoveHeadQueue(&local_q);

		if (!q_entry)
			break;

		InsertTailQueue(post_queue, q_entry);
	} while (1);

	OS_SEM_UNLOCK(post_lock);

	OS_SEM_UNLOCK(lock);
}

static INT fp_sta_clean_queue(struct _RTMP_ADAPTER *ad, UINT16 wcid)
{
	struct _STA_TR_ENTRY *tr_entry = NULL;
	UINT8 que_idx = 0;

	if (!IS_WCID_VALID(ad, wcid)) {
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"%s(): invalid wcid = %d\n", __func__, wcid);
		return NDIS_STATUS_INVALID_DATA;
	}

	tr_entry = tr_entry_get(ad, wcid);

	if (IS_ENTRY_NONE(tr_entry)) {
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"%s(): invalid tr_entry->EntryType = %08x\n", __func__, tr_entry->EntryType);
		return NDIS_STATUS_INVALID_DATA;
	}

	que_idx = hc_get_hw_band_idx(ad);

	fp_qm_sta_leave_queue_pkt(tr_entry,
		PD_GET_QM_MGMT_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_POST_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_MGMT_POST_SWQ_LOCK(ad->physical_dev, que_idx));
	fp_qm_sta_leave_queue_pkt(tr_entry,
		PD_GET_QM_FP_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_SWQ_LOCK(ad->physical_dev, que_idx));
#ifdef IGMP_SNOOPING_OFFLOAD
	fp_qm_sta_leave_queue_pkt(tr_entry,
		PD_GET_QM_FP_BMC_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_BMC_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_BMC_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_BMC_SWQ_LOCK(ad->physical_dev, que_idx));
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	fp_qm_sta_leave_queue_pkt(tr_entry,
		PD_GET_QM_FP_HIGHPRI_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_HIGHPRI_SWQ(ad->physical_dev, que_idx),
		PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(ad->physical_dev, que_idx),
		PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(ad->physical_dev, que_idx));
#endif

	return NDIS_STATUS_SUCCESS;
}

static INT32 fp_dump_all_sw_queue(RTMP_ADAPTER *pAd,  RTMP_STRING *arg)
{
	UINT8 que_idx;
	UINT8 value = 0;
	struct fp_tx_flow_control *flow_ctl = PD_GET_QM_FP_TX_FLOW_CTL(pAd->physical_dev);
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);

	if (arg && strlen(arg))
		value = (UCHAR)os_str_tol(arg, 0, 10);

	MTWF_PRINT("tx flow control:%d\n", test_bit(0, &flow_ctl->flag));

	que_idx = hc_get_hw_band_idx(pAd);

	MTWF_PRINT("max_mgmt_que_num:%d\n", qm_parm->max_mgmt_que_num);
	MTWF_PRINT("max_data_que_num:%d\n", qm_parm->max_data_que_num);
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	MTWF_PRINT("max_highpri_que_num:%d\n", qm_parm->max_highpri_que_num);
#endif
	MTWF_PRINT("max_probe_num:%d\n", cap->ProbeRspMaxNum);
	MTWF_PRINT("txprobe_rsp_cnt_per_s[%d]:%d\n", que_idx, pAd->probe_rsp_cnt_per_s);
	MTWF_PRINT("bn_probe_rsp_drop_cnt[%d]:%d\n", que_idx, pAd->bn_probe_rsp_drop_cnt);

	if (value) /* reset */
		pAd->bn_probe_rsp_drop_cnt = 0;

	OS_SPIN_LOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));
	MTWF_PRINT("mgmt_que[%d] number:%d\n", que_idx, PD_GET_QM_MGMT_SWQ_LEN(pAd->physical_dev, que_idx));
	MTWF_PRINT("mgmt_post_que[%d] number:%d\n", que_idx, PD_GET_QM_MGMT_POST_SWQ_LEN(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_MGMT_SWQ_LOCK(pAd->physical_dev, que_idx));

#ifdef IGMP_SNOOPING_OFFLOAD
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
	MTWF_PRINT("bmc_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_BMC_SWQ_LEN(pAd->physical_dev, que_idx));
	MTWF_PRINT("post_bmc_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_POST_BMC_SWQ_LEN(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_BMC_SWQ_LOCK(pAd->physical_dev, que_idx));
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	OS_SPIN_LOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
	MTWF_PRINT("highpri_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx));
	MTWF_PRINT("post_highpri_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_POST_HIGHPRI_SWQ_LEN(pAd->physical_dev, que_idx));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(pAd->physical_dev, que_idx));
#endif

	OS_SPIN_LOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));
	MTWF_PRINT("fp_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_SWQ_LEN(pAd->physical_dev, que_idx));
	MTWF_PRINT("fp_post_que[%d] number:%d\n", que_idx, PD_GET_QM_FP_POST_SWQ_LEN(pAd->physical_dev, que_idx));
	MTWF_PRINT("tx flow block state[%d]:%d\n", que_idx,
		test_bit(0, &flow_ctl->TxFlowBlockState[que_idx]));
	MTWF_PRINT("tx flow block dev number[%d]:%d\n", que_idx,
		DlListLen(&flow_ctl->TxBlockDevList[que_idx]));
	OS_SPIN_UNLOCK_BH(PD_GET_QM_FP_SWQ_LOCK(pAd->physical_dev, que_idx));

	return NDIS_STATUS_SUCCESS;
}


struct qm_ops fp_qm_ops = {
	.init_physical_dev = fp_qm_init_physical_dev,
	.exit_physical_dev = fp_qm_exit_physical_dev,
	.init_perband = fp_qm_init_perband,
	.exit_perband = fp_qm_exit_perband,
	.enq_mgmtq_pkt = fp_enq_mgmtq_pkt,
	.deq_tx_pkt = fp_hwifi_tx_pkt_deq_func,
	.enq_delayq_pkt = ge_enq_delayq_pkt,
	.deq_delayq_pkt = ge_deq_delayq_pkt,
	.dump_all_sw_queue = fp_dump_all_sw_queue,
	.schedule_tx_que = fp_schedule_tx_que,
	.schedule_tx_que_on = fp_schedule_tx_que_on,
	.bss_clean_queue = fp_bss_clean_queue,
	.sta_clean_queue = fp_sta_clean_queue,
#ifdef CONFIG_TX_DELAY
	.tx_delay_init = fp_tx_delay_init,
	.get_qm_delay_ctl = fp_get_qm_delay_ctl,
#endif
};
