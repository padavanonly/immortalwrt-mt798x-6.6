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
/****************************************************************************
 ****************************************************************************

    Abstract:

 */

#ifndef __SCS_H__
#define __SCS_H__

#include "rt_config.h"

#ifdef SMART_CARRIER_SENSE_SUPPORT
#define TriggerTrafficeTh							250000/*2M*/
#define MAX_LEN_OF_SCS_BSS_TABLE					6
#define MinRssiTolerance				10
#define ThTolerance                                 10
#define PdBlkEnabeOffset							19
#define RTSDropCntEnabeOffset						0
#define RTSDropRdClrEnabeOffset						8
#define RtsDropCountOffset							16
#define RTSDropCountMask							0x0000ffff
#define PdBlkEnabeOffsetB1							25
#define PdBlkOfmdThMask								0x1ff
#define PdBlkOfmdThOffset							20
#define PdBlkOfmdThOffsetB1							16 /* Band1 */
#define PdBlkCckThMask								0xff
#define PdBlkCckThOffset							1
#define PdBlkCck1RThOffset							24
#define PdBlkOfmdThDefault							0x13c
#define PdBlkCckThDefault							0x92 /*-110dBm*/
#define BandNum										1/*MT7615 HW not support DBDC/MT7622 only Band0*/
#define BssNumScs										4 /* BssNum coflicts with STA Mode definition */
#define BssOffset									0x10
#define RtsRtyCountOffset							16
#define RtsCountMask								0x0000ffff
#define TxTrafficTh									9
/* #define BandOffset		0x200 */
#define OneStep										2/* dB */
#define FastInitTh									0xa6/* -90dBm */
#define FastInitThOfdm								0x14c/* -90dBm */
#define SCS_STA_NUM									4

#define FalseCcaUpBondDefault						500
#define FalseCcaLowBondDefault						50
#define CckFixedRssiBondDefault						184/* -72dBm. -72+256 */
#define OfdmFixedRssiBondDefault					368/* -72dBm. (-72*2)+512 */

enum {
	SCS_DISABLE,
	SCS_ENABLE,
};

enum {
	PD_BLOCKING_OFF,
	PD_BLOCKING_ON,
};

typedef struct _SMART_CARRIER_SENSE_CTRL {
	BOOL	SCSEnable;	/* 0:Disable, 1:Enable */
	UINT8	SCSStatus;/* 0: Normal, 1:Low_gain */
	CHAR	SCSMinRssi;
	UINT32	SCSTrafficThreshold; /* Traffic Threshold */
	UINT32	OneSecTxByteCount;
	UINT32	OneSecRxByteCount;
	INT32	CckPdBlkTh;
	INT32	OfdmPdBlkTh;
	INT32	SCSMinRssiTolerance;
	INT32	SCSThTolerance;
	UCHAR	OfdmPdSupport;
	BOOL	ForceScsOff;
	UINT32	RtsCount;
	UINT32	RtsRtyCount;
	/* MT7622  support Band0*/
	UINT32	RTS_MPDU_DROP_CNT;
	UINT32	Retry_MPDU_DROP_CNT;
	UINT32	LTO_MPDU_DROP_CNT;
	/*------------------------*/
	UINT32	CckFalseCcaCount;
	UINT32	OfdmFalseCcaCount;
	UINT16	CckFalseCcaUpBond;
	UINT16	CckFalseCcaLowBond;
	UINT16	OfdmFalseCcaUpBond;
	UINT16	OfdmFalseCcaLowBond;
	INT32	CckFixedRssiBond;
	INT32	OfdmFixedRssiBond;
	BOOL	PDreset;
	/*SCSGen6_for_MT7915 MT7986 MT7916 MT7981*/
	UINT16	LastETput;
	UINT16	ActiveSTAIdx;
} SMART_CARRIER_SENSE_CTRL, *PSMART_CARRIER_SENSE_CTRL;

typedef struct _SMART_CARRIER_SENSE_CTRL_GEN2_T {
    UINT_8    u1SCSEnable;
    INT_8     u1SCSMinRssi;
    UINT_32   u4OneSecTxByteCount;
    UINT_32   u4OneSecRxByteCount;
    UINT_16   u2CckPdBlkTh;
    UINT_16   u2OfdmPdBlkTh;
    UINT_16   u2SCSMinRssiTolerance;
    UINT_16   u2CckPdThrMax;
    UINT_16   u2OfdmPdThrMax;
    UINT_16   u2CckPdThrMin;
    UINT_16   u2OfdmPdThrMin;

    UINT_16   u2IniAvgTput[SCS_STA_NUM];
    UINT_16   u2LastTputDiff[SCS_STA_NUM];
    UINT_16   u2LastAvgTput[SCS_STA_NUM];
    UINT_16   u2LastMaxTput[SCS_STA_NUM];
    UINT_16   u2LastMinTput[SCS_STA_NUM];
    UINT_16   u2LastTputIdx[SCS_STA_NUM];
    BOOL      fgLastTputDone[SCS_STA_NUM];
    UINT_16   u2CurAvgTput[SCS_STA_NUM];
    UINT_16   u2CurTputIdx[SCS_STA_NUM];
    UINT_8    u1TputPeriodScaleBit[SCS_STA_NUM];

    UINT_8    u1LastActiveSTA;
    UINT_8    u1ContinuousActiveSTAZeroCnt;
    UINT_8    u1ChannelBusyTh;
    BOOL      fgChBusy;
    UINT_8    u1MyTxRxTh;
    BOOL      fgPDreset;

    UINT_32   u4ChannelBusyTime;
    UINT_32   u4MyTxAirtime;
    UINT_32   u4MyRxAirtime;
    UINT_32   u4OBSSAirtime;
} SMART_CARRIER_SENSE_CTRL_GEN2_T, *P_SMART_CARRIER_SENSE_CTRL_GEN2_T;

typedef struct _SCS_GLO_CHECK {
	UINT_32 u4Addr;
	BOOLEAN fgError;
} SCS_GLO_CHECK, *P_SCS_GLO_CHECK;

typedef struct _DRV_SCS_GLO {
	SCS_GLO_CHECK rscsband[2];
} DRV_SCS_GLO, *P_DRV_SCS_GLO;

typedef struct _SCS_GLO_INFO {
	UINT_32 u4Addr;
	UINT_32 u4Size;
} SCS_GLO_INFO, *P_SCS_GLO_INFO;

typedef struct _EVENT_SCS_GLO {
	SCS_GLO_INFO rscsband[2];
} EVENT_SCS_GLO, *P_EVENT_SCS_GLO;

VOID SCS_init(RTMP_ADAPTER *pAd);
VOID SmartCarrierSense_Gen6(RTMP_ADAPTER *pAd);

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventSCSGetGloAddrHandler(struct cmd_msg *msg, char *rsp_payload);
#endif /* WIFI_UNIFIED_COMMAND */
#endif /*SMART_CARRIER_SENSE_SUPPORT*/

#endif /* __SCS_H__ */

