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

#ifndef __TR_BA_H__
#define __TR_BA_H__

#define REORDERING_PACKET_TIMEOUT_IN_MS		(100)
#define MAX_REORDERING_PACKET_TIMEOUT_IN_MS	(1500)

#define IS_BA_WAITING(_BAEntry) ((_BAEntry) && (_BAEntry)->WaitWM == TRUE)

struct reordering_mpdu;

struct reordering_list {
	struct reordering_mpdu *next;
	struct reordering_mpdu *tail;
	int qlen;
};
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
#define BUF_FREE_ID_SIZE (MAX_LEN_OF_BA_REC_TABLE)
#define BUF_FREE_ID_INVALID (MAX_LEN_OF_BA_REC_TABLE)
#if defined(DOT11_EHT_BE)
#define CHIP_BA_WINSIZE 1024
#elif defined(DOT11_HE_AX)
#define CHIP_BA_WINSIZE 256
#else
#define CHIP_BA_WINSIZE 64
#endif
struct ba_buf {
	u16 id;
	struct reordering_mpdu *buf[CHIP_BA_WINSIZE];
};

struct reordering_mpdu {
	struct reordering_mpdu *next;
	//struct reordering_list AmsduList;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR OpMode;
	UCHAR band;
};
#else
struct reordering_mpdu {
	struct reordering_mpdu *next;
	struct reordering_list AmsduList;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR OpMode;
	UCHAR band;
};
#endif

struct reordering_mpdu_pool {
	PVOID mem;
	NDIS_SPIN_LOCK lock;
	struct reordering_list freelist;
};


enum REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Initialization,
	Recipient_Established,
	Recipient_Offload,
};

enum ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
};

struct BA_INFO {
	BOOLEAN AutoTest;
	USHORT TxBitmap;
	USHORT RxBitmap;
	USHORT TxAmsduBitmap;
	USHORT TxAutoBitmap;
	USHORT DeclineBitmap;
	USHORT PolicyNotSupBitmap;
	USHORT RecWcidArray[NUM_OF_TID];
	USHORT OriWcidArray[NUM_OF_TID];
};

struct BA_ORI_ENTRY {
	struct _MAC_TABLE_ENTRY *pEntry;
	UCHAR TID;
	UINT16 BAWinSize;
	UCHAR Token;
	UCHAR amsdu_cap;
	/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	enum ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
};

struct ba_rec_debug {
	UINT16 sn;
	UINT8 amsdu;
#define BA_DATA 0
#define BA_BAR 1
	UINT8 type;
	USHORT last_in_seq;
	UCHAR ta[MAC_ADDR_LEN];
	UCHAR ra[MAC_ADDR_LEN];
};

struct BA_REC_ENTRY {
	struct _MAC_TABLE_ENTRY *pEntry;
	BOOLEAN check_amsdu_miss;
	UINT8 PreviousAmsduState;
	UINT16 PreviousSN;
	UINT16 PreviousReorderCase;
	enum REC_BLOCKACK_STATUS REC_BA_Status;
	USHORT LastIndSeq;
	NDIS_SPIN_LOCK RxReRingLock;
	UINT16 BAWinSize;
	ULONG LastIndSeqAtTimer;
	ULONG drop_dup_pkts;
	ULONG drop_old_pkts;
	ULONG drop_unknown_state_pkts;
	ULONG ba_sn_large_win_end;
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	INT16 stored_mpdu_num;
	u16 reorder_buf_id;
	u16 head_sn;
	u16 head_index;
	struct reordering_mpdu **reorder_buf;
#else
	struct reordering_list list;
#endif
	struct reordering_mpdu *CurMpdu;
#define STEP_ONE 0
#define REPEAT 1
#define OLDPKT 2
#define WITHIN 3
#define SURPASS 4
	UCHAR TID;
	UCHAR Session_id;
#define ADDBA_POSTPONE 1
#define DELBA_POSTPONE 2
	UCHAR Postpone_Action;
	UCHAR WaitWM;
	UINT8 RetryCnt;
	USHORT TimeOutValue;
#define BA_REC_DBG_SIZE 256
	struct ba_rec_debug *ba_rec_dbg;
	UINT32 ba_rec_dbg_idx;
	BOOLEAN CCMP_ICV_ERROR;
	BOOLEAN HAS_ICV_ERROR;
	ULONG RRO_RESET_CNT;
	ULONG icv_err_cnt;
	ULONG flag_err_cnt;
} ____cacheline_aligned;

enum {
	SN_HISTORY = (1 << 0),
	SN_RECORD_BASIC = (1 << 1),
	SN_RECORD_MAC = (1 << 2),
	SN_DUMP_WITHIN = (1 << 3),
	SN_DUMP_SURPASS = (1 << 4),
	SN_DUMP_OLD = (1 << 5),
	SN_DUMP_DUP = (1 << 6),
	SN_DUMP_STEPONE = (1 << 7),
	SN_DUMP_BAR = (1 << 8),
};

struct ba_control {
	struct BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
	struct BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	NDIS_SPIN_LOCK BATabLock;
	struct reordering_mpdu_pool mpdu_blk_pool[CFG_WIFI_RAM_BAND_NUM];
#define BA_TIMEOUT_BITMAP_LEN (MAX_LEN_OF_BA_REC_TABLE/32 + 1)
	BOOLEAN ba_timeout_check;
	UINT32 ba_timeout_bitmap[BA_TIMEOUT_BITMAP_LEN];
#ifdef RX_RPS_SUPPORT
	BOOLEAN ba_timeout_check_per_cpu[NR_CPUS];
	UINT32 ba_timeout_bitmap_per_cpu[NR_CPUS][BA_TIMEOUT_BITMAP_LEN];
#endif
	ULONG numAsRecipient;
	ULONG numAsOriginator;
	ULONG numDoneOriginator; /* count Done Originator sessions */
	ULONG dbg_flag;
	u8 rx_ba_disable;
	RALINK_TIMER_STRUCT FlushTimer;
	atomic_t SetFlushTimer;
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	struct ba_buf free_ba_buf[BUF_FREE_ID_SIZE];
	u32 free_ba_buf_id[BUF_FREE_ID_SIZE];
#endif
};

#endif
