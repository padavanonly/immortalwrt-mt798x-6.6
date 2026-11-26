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

#ifndef __QM_H__
#define __QM_H__

#define MCAST_WCID_TO_REMOVE 0

enum PACKET_TYPE;
struct _STA_TR_ENTRY;
struct dequeue_info;
struct _TX_BLK;

enum {
	TX_QUE_LOW,
	TX_QUE_HIGH,
};

enum {
	TX_QUE_HIGH_TO_HIGH,
	TX_QUE_HIGH_TO_LOW,
	TX_QUE_LOW_TO_LOW,
	TX_QUE_LOW_TO_HIGH,
	TX_QUE_UNKNOW_CHANGE,
};

enum queue_mm {
	GENERIC_QM,
	FAST_PATH_QM
};

struct qm_ctl {
	UINT32 total_psq_cnt;
} ____cacheline_aligned;

#define IS_GE_QM(_qm) (_qm == GENERIC_QM)

/**
 * @init: qm resource initialization
 * @exit: qm resource exit
 * @enq_mgmt_pkt: en-queue packet to management queue operation
 * @enq_data_pkt: en-queue packet to data queue operation
 * @deq_tx_pkt: de-queue packet from sw queue
 * @get_psq_pkt: get packet from power saving queue operation
 * @enq_psq_pkt: en-queue packet to power saving queue operation
 * @schedule_tx_que: schedule job that may use thread, worker, or tasklet to dequeue tx queue and service packet
 * @sta_clean_queue: cleanup resource inside queue per station
 * @sta_dump_queue: dump resource inside queue per station per queue index
 * @dump_all_sw_queue: dump all sw queue information
 * @deq_data_pkt: for fair queue dequeue packet
 * @tx_flow_ctl: enable/disable sw queue flow control
 */

struct qm_ops {
	/* INIT/EXIT */
	INT (*init_physical_dev)(VOID *ph_dev);
	INT (*exit_physical_dev)(VOID *ph_dev);
	INT (*init_perband)(struct _RTMP_ADAPTER *pAd);
	INT (*exit_perband)(struct _RTMP_ADAPTER *pAd);
	INT (*sta_clean_queue)(struct _RTMP_ADAPTER *pAd, UINT16 wcid);
	VOID (*sta_dump_queue)(struct _RTMP_ADAPTER *pAd, UINT16 wcid, enum PACKET_TYPE pkt_type, UCHAR q_idx);
	INT (*bss_clean_queue)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	INT (*dump_all_sw_queue)(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);

	/* TX */
	INT (*enq_mgmtq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt) ____cacheline_aligned;
	INT (*enq_dataq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx);
	VOID (*deq_tx_pkt)(struct _RTMP_ADAPTER *pAd, UINT8 idx);
	struct tx_delay_control *(*get_qm_delay_ctl)(struct _RTMP_ADAPTER *pAd, UINT8 idx);
	BOOLEAN (*tx_deq_delay)(struct _RTMP_ADAPTER *pAd, UINT8 idx);
	NDIS_PACKET *(*get_psq_pkt)(struct _RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry);
	INT (*enq_psq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt);
	INT (*enq_delayq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt);
	NDIS_PACKET *(*deq_delayq_pkt)(struct _RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry);
	INT (*schedule_tx_que)(struct _RTMP_ADAPTER *pAd, UINT8 idx);
	INT (*schedule_tx_que_on)(struct _RTMP_ADAPTER *pAd, int cpu, UINT8 idx);
	INT32 (*deq_data_pkt)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info);
	INT32 (*deq_data_pkt_v2)(struct _RTMP_ADAPTER * pAd, INT32 max_cnt, struct dequeue_info * info, struct _QUEUE_HEADER * pTxPacketList);
	/* RX */
	INT (*enq_rx_dataq_pkt)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);
#ifdef CONFIG_TX_DELAY
	INT (*tx_delay_init)(struct _RTMP_ADAPTER *pAd);
#endif
	INT (*tx_flow_ctl)(struct _RTMP_ADAPTER *pAd, BOOLEAN en);
} ____cacheline_aligned;

#ifdef MEMORY_OPTIMIZATION
#define MGMT_QUE_MAX_NUMS 128
#define HIGH_PRIO_QUE_MAX_NUMS 128
#else
#define MGMT_QUE_MAX_NUMS 512
#define HIGH_PRIO_QUE_MAX_NUMS 512
#endif
#define TX_DATA_QUE_MAX_NUMS 6144

#define RX_DATA_QUE_MAX_NUMS 4096

enum pkt_tx_status {
	PKT_SUCCESS = 0,
	INVALID_PKT_LEN = 1,
	INVALID_TR_WCID = 2,
	INVALID_TR_ENTRY = 3,
	INVALID_WDEV = 4,
	INVALID_ETH_TYPE = 5,
	DROP_PORT_SECURE = 6,
	DROP_PSQ_FULL = 7,
	DROP_TXQ_FULL = 8,
	DROP_TX_JAM = 9,
	DROP_TXQ_ENQ_FAIL = 10,
};

struct reason_id_str {
	INT id;
	RTMP_STRING *code_str;
};

#define InitializeQueueHeader(QueueHeader)              \
	{                                                       \
		(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
		(QueueHeader)->Number = 0;                          \
	}

#define RemoveHeadQueue(QueueHeader)					\
	(QueueHeader)->Head;						\
	do {								\
		struct _QUEUE_ENTRY *pNext;                             \
		if ((QueueHeader)->Head != NULL) {			\
			pNext = (QueueHeader)->Head->Next;		\
			(QueueHeader)->Head->Next = NULL;		\
			(QueueHeader)->Head = pNext;			\
			(QueueHeader)->Number--;			\
			if ((QueueHeader)->Number == 0) {		\
				(QueueHeader)->Head = NULL;		\
				(QueueHeader)->Tail = NULL;		\
			}						\
		}							\
	} while (0)

#define RemoveTailQueue(QueueHeader)					\
	(QueueHeader)->Tail;						\
	do {								\
		struct _QUEUE_ENTRY *pNext;				\
		if ((QueueHeader)->Head != NULL) {			\
			pNext = (QueueHeader)->Head;			\
			(QueueHeader)->Tail->Next = NULL;		\
			if (pNext->Next) {				\
				while (pNext->Next != (QueueHeader)->Tail) { \
					pNext = pNext->Next;		\
				}					\
				(QueueHeader)->Tail = pNext;		\
				pNext->Next = NULL;			\
			}						\
			(QueueHeader)->Number--;			\
			if ((QueueHeader)->Number == 0) {		\
				(QueueHeader)->Head = NULL;		\
				(QueueHeader)->Tail = NULL;		\
			}						\
		}							\
	} while (0)


#define InsertHeadQueue(QueueHeader, QueueEntry)            \
	{                                                           \
		((struct _QUEUE_ENTRY *)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (struct _QUEUE_ENTRY *)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (struct _QUEUE_ENTRY *)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
	}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
	{                                                               \
		((struct _QUEUE_ENTRY *)QueueEntry)->Next = NULL;                    \
		if ((QueueHeader)->Tail)                                    \
			(QueueHeader)->Tail->Next = (struct _QUEUE_ENTRY *)(QueueEntry); \
		else                                                        \
			(QueueHeader)->Head = (struct _QUEUE_ENTRY *)(QueueEntry);       \
		(QueueHeader)->Tail = (struct _QUEUE_ENTRY *)(QueueEntry);           \
		(QueueHeader)->Number++;                                    \
	}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)			\
	{																		\
		((struct _QUEUE_ENTRY *)QueueEntry)->Next = NULL;							\
		if ((QueueHeader)->Tail)											\
			(QueueHeader)->Tail->Next = (struct _QUEUE_ENTRY *)(QueueEntry);			\
		else																\
			(QueueHeader)->Head = (struct _QUEUE_ENTRY *)(QueueEntry);				\
		(QueueHeader)->Tail = (struct _QUEUE_ENTRY *)(QueueEntry);					\
		(QueueHeader)->Number++;											\
	}

#define PickFromQueue(QueueHeader, QueuePrevEntry, QueueEntry)                \
	do {\
		struct _QUEUE_ENTRY *pHead;	\
		if ((QueueHeader)->Head != (struct _QUEUE_ENTRY *)QueueEntry) {	\
			((struct _QUEUE_ENTRY *)QueuePrevEntry)->Next = ((struct _QUEUE_ENTRY *)QueueEntry)->Next;	\
			((struct _QUEUE_ENTRY *)QueueEntry)->Next = NULL;	\
			if (((QueueHeader)->Tail) == (struct _QUEUE_ENTRY *)QueueEntry) {	\
				(QueueHeader)->Tail = (struct _QUEUE_ENTRY *)QueuePrevEntry;	\
				(QueueHeader)->Tail->Next = (struct _QUEUE_ENTRY *)(QueuePrevEntry);	\
				((struct _QUEUE_ENTRY *)QueuePrevEntry)->Next = NULL;	\
			}	\
			(QueueHeader)->Number--; \
		} else {	\
			pHead = RemoveHeadQueue(QueueHeader); \
			((struct _QUEUE_ENTRY *)QueueEntry)->Next = NULL; \
		}	\
	} while (0);


#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags)				\
	do {													\
		if (bIntContext == FALSE)						\
			RTMP_IRQ_LOCK((lock), IrqFlags);		\
	} while (0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
	do {													\
		if (bIntContext == FALSE)						\
			RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
	} while (0)


INT qm_init(VOID *ph_dev);
INT qm_exit(VOID *ph_dev);
INT qm_init_perband(struct _RTMP_ADAPTER *pAd);
INT qm_exit_perband(struct _RTMP_ADAPTER *pAd);
VOID ge_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd, UINT8 idx);
VOID RTMPDeQueuePacket(struct _RTMP_ADAPTER *pAd, BOOLEAN bIntContext, UCHAR QueIdx, INT wcid, INT Max_Tx_Packets);
INT ge_enq_req(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qidx, struct _STA_TR_ENTRY *tr_entry, struct _QUEUE_HEADER *pPktQueue);
VOID ge_tx_swq_dump(struct _RTMP_ADAPTER *pAd, INT qidx);
VOID qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue, NDIS_SPIN_LOCK *lock);
INT deq_packet_gatter(struct _RTMP_ADAPTER *pAd, struct dequeue_info *deq_info, struct _TX_BLK *pTxBlk);
VOID ge_rx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);
VOID RTMPRxDataDeqOffloadToOtherCPU(struct _RTMP_ADAPTER *pAd);
NDIS_PACKET *ge_deq_delayq_pkt(struct _RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry);
INT ge_enq_delayq_pkt(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt);
#endif
