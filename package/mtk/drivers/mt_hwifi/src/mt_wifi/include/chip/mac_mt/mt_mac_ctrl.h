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
	mt_mac_ctrl.h
*/

#ifndef __MT_MAC_CTRL_H__
#define __MT_MAC_CTRL_H__

/* TODO: shiang-usw,remove this */
#define HW_BEACON_OFFSET		0x0200
/* TODO: ---End */


/* TODO: shiang-usw, mark by shiang, need to remove it */
typedef union GNU_PACKED _TXWI_STRUC {
	UINT32 word;
} TXWI_STRUC;
/* TODO: --End */


struct _RTMP_ADAPTER;


#define TXS2MCU_AGGNUMS 31
#define TXS2HOST_AGGNUMS 31

enum {
	TXS_UNUSED,
	TXS_USED,
};

typedef struct _TXS_STATUS {
	UINT8 TxSPid;
	UINT8 State;
	UINT8 Type;
	UINT8 PktPid;
	UINT8 PktType;
	UINT8 PktSubType;
	UINT16 TxRate;
	UINT32 Priv;
} TXS_STATUS, *PTXS_STATYS;

#define TOTAL_PID_HASH_NUMS	0x10
#define TOTAL_PID_HASH_NUMS_PER_PKT_TYPE 0x8
#define TXS_WLAN_IDX_ALL 128
/*discuss with FW team, Host driver use PID from 0~127, FW use PID from 128 to 255 */
#define TXS_STATUS_NUM 128
typedef struct _TXS_CTL {
	/* TXS type hash table per pkt */
	DL_LIST TxSTypePerPkt[TOTAL_PID_HASH_NUMS];
	NDIS_SPIN_LOCK TxSTypePerPktLock[TOTAL_PID_HASH_NUMS];
	/* TXS type hash table per pkt type and subtype */
	DL_LIST TxSTypePerPktType[3][TOTAL_PID_HASH_NUMS_PER_PKT_TYPE];
	NDIS_SPIN_LOCK TxSTypePerPktTypeLock[3][TOTAL_PID_HASH_NUMS_PER_PKT_TYPE];
	ULONG TxS2McUStatusPerPkt;
	ULONG TxS2HostStatusPerPkt;
	ULONG TxS2McUStatusPerPktType[3];
	ULONG TxS2HostStatusPerPktType[3];
	ULONG TxSFormatPerPkt;
	ULONG TxSFormatPerPktType[3];
	UINT64 TxSStatusPerWlanIdx[2];
	ULONG TxSFailCount;
	UINT8 TxSPid;
	TXS_STATUS TxSStatus[TXS_STATUS_NUM];
	UINT32  TxSPidBitMap0_31;
	UINT32  TxSPidBitMap32_63;
	UINT32  TxSPidBitMap64_95;
	UINT32  TxSPidBitMap96_127;
} TXS_CTL, *PTXS_CTL;

typedef struct _TMR_CTRL_STRUCT {
	UCHAR HwTmrVersion;
	UCHAR TmrEnable;/* disable, initiator, responder */
	UCHAR TmrState; /* used to control CR enable/disable@initiator role. */
	UINT32 TmrCalResult;
	UCHAR TmrThroughold;
	UCHAR TmrIter;
} TMR_CTRL_STRUCT, *PTMR_CTRL_STRUCT;


#endif /* __MT_MAC_CTRL_H__ */
