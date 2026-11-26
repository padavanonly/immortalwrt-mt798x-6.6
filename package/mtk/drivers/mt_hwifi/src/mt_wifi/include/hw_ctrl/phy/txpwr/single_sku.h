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
	single_sku.h
*/

#ifndef __CMM_SINGLE_SKU_H__
#define __CMM_SINGLE_SKU_H__

/*******************************************************************************
 *	INCLUDED FILES
 ******************************************************************************/

/*******************************************************************************
 *	DEFINITIONS
 ******************************************************************************/

/** buffer size allocated for power limit table */
/* use Sku_sizeof[sku_tbl_idx] & Backoff_sizeof[sku_tbl_idx]  for new chips */
#define MAX_POWER_LIMIT_BUFFER_SIZE    42000

/* Debug log color */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define CH_G_BAND    0
#define CH_A_BAND    1
#define CH_6G_BAND   2

#define	SINGLE_SKU_TABLE_LENGTH		(SINGLE_SKU_TABLE_CCK_LENGTH+SINGLE_SKU_TABLE_OFDM_LENGTH+(SINGLE_SKU_TABLE_HT_LENGTH*2)+SINGLE_SKU_TABLE_VHT_LENGTH)

#define SINGLE_SKU_TABLE_EFFUSE_ADDRESS 0x12C

#define SINGLE_SKU_TABLE_CCK_LENGTH			4
#define SINGLE_SKU_TABLE_OFDM_LENGTH		8
#define SINGLE_SKU_TABLE_HT_20_LENGTH		8
#define SINGLE_SKU_TABLE_HT_40_LENGTH		9
#define SINGLE_SKU_TABLE_VHT_LENGTH			12
#define SINGLE_SKU_TABLE_HE_LENGTH			12
#define SINGLE_SKU_TABLE_EHT_LENGTH			16
#define SINGLE_SKU_TABLE_TYPE_NUM			31
#define TABLE_PARSE_TYPE_NUM				2

#define BF_BACKOFF_CCK_LENGTH				5
#define BF_BACKOFF_OFDM_OFF_LENGTH			5
#define BF_BACKOFF_OFDM_ON_LENGTH			4
#define BF_BACKOFF_LENGTH					15
#define BF_BACKOFF_TYPE_NUM					35

/* 0: None, 1: CH_G_BAND, 2: CH_A_BAND, 3: CH_G_BAND and CH_A_BAND */
#define TABLE_NO_PARSE						0
#define TABLE_PARSE_G_BAND					BIT(0)
#define TABLE_PARSE_A_BAND					BIT(1)
#define TABLE_PARSE_G_A_BAND				  BITS(0, 1)

#define SINGLE_SKU_TABLE_TX_OFFSET_NUM  3
#define SINGLE_SKU_TABLE_NSS_OFFSET_NUM 4

#define SKUTABLE_1					  1
#define SKUTABLE_2					  2
#define SKUTABLE_3					  3
#define SKUTABLE_4					  4
#define SKUTABLE_5					  5
#define SKUTABLE_6					  6
#define SKUTABLE_7					  7
#define SKUTABLE_8					  8
#define SKUTABLE_9					  9
#define SKUTABLE_10					10
#define SKUTABLE_11					11
#define SKUTABLE_12					12
#define SKUTABLE_13					13
#define SKUTABLE_14					14
#define SKUTABLE_15					15
#define SKUTABLE_16					16
#define SKUTABLE_17					17
#define SKUTABLE_18					18
#define SKUTABLE_19					19
#define SKUTABLE_20					20
#define TABLE_SIZE					21
#define AFC_STD_PWR_SKUTABLE_IDX		20


#define VHT20_OFFSET					0
#define VHT40_OFFSET					7
#define VHT80_OFFSET				   14
#define VHT160C_OFFSET				 21

/* PHY Mode */
#define SKU_CCK_OFFSET				  0
#define SKU_OFDM_OFFSET				 2
#define SKU_HT_OFFSET				   7
#define SKU_VHT_OFFSET				 21

/* MCS Rate */
#define SKU_CCK_RATE_M01				0
#define SKU_CCK_RATE_M23				1

#define SKU_OFDM_RATE_M01			   0
#define SKU_OFDM_RATE_M23			   1
#define SKU_OFDM_RATE_M45			   2
#define SKU_OFDM_RATE_M6				3
#define SKU_OFDM_RATE_M7				4

#define SKU_HT_RATE_M0				  0
#define SKU_HT_RATE_M32				 1
#define SKU_HT_RATE_M12				 2
#define SKU_HT_RATE_M34				 3
#define SKU_HT_RATE_M5				  4
#define SKU_HT_RATE_M6				  5
#define SKU_HT_RATE_M7				  6

#define SKU_VHT_RATE_M0				 0
#define SKU_VHT_RATE_M12				1
#define SKU_VHT_RATE_M34				2
#define SKU_VHT_RATE_M56				3
#define SKU_VHT_RATE_M7				 4
#define SKU_VHT_RATE_M8				 5
#define SKU_VHT_RATE_M9				 6

/*******************************************************************************
 *	MACRO
 ******************************************************************************/

#define SINGLE_SKU_TABLE_FILE_NAME	"/etc/wireless/mediatek/mt7615e-sku.dat"
#define BF_SKU_TABLE_FILE_NAME		"/etc/wireless/mediatek/mt7615e-sku-bf.dat"

/*******************************************************************************
 *	TYPES
 ******************************************************************************/

typedef enum _POWER_LIMIT_TABLE {
	POWER_LIMIT_TABLE_TYPE_SKU = 0,
	POWER_LIMIT_TABLE_TYPE_BACKOFF,
	POWER_LIMIT_TABLE_TYPE_NUM
} POWER_LIMIT_TABLE, *P_POWER_LIMIT_TABLE;

typedef struct _CH_POWER_V0 {
	UINT8  *Channel;
	UINT8  band;
	UINT8  u1PwrLimitCCK[SINGLE_SKU_TABLE_CCK_LENGTH];
	UINT8  u1PwrLimitOFDM[SINGLE_SKU_TABLE_OFDM_LENGTH];
	UINT8  u1PwrLimitHT20[SINGLE_SKU_TABLE_HT_20_LENGTH];
	UINT8  u1PwrLimitHT40[SINGLE_SKU_TABLE_HT_40_LENGTH];
	UINT8  u1PwrLimitVHT20[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT40[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT80[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT160[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitHE26[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE52[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE106[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE242[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE484[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE996[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitHE996X2[SINGLE_SKU_TABLE_HE_LENGTH];
	UINT8  u1PwrLimitEHT26[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT52[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT106[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT242[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT484[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996X2[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996X4[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT26_52[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT26_106[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT484_242[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996_484[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996_484_242[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996X2_484[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996X3[SINGLE_SKU_TABLE_EHT_LENGTH];
	UINT8  u1PwrLimitEHT996X3_484[SINGLE_SKU_TABLE_EHT_LENGTH];	
} CH_POWER_V0, *P_CH_POWER_V0;

typedef struct _CH_POWER_V1 {
	DL_LIST  List;
	UINT8  u1StartChannel;
	UINT8  u1ChNum;
	PUINT8 pu1ChList;
	UINT8  u1ChBand;
	PUINT8 pu1PwrLimit;
} CH_POWER_V1, *P_CH_POWER_V1;

typedef enum _ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE {
	POWER_LIMIT_LINK_LIST = 0,
	POWER_LIMIT_RAW_DATA_LENGTH,
	POWER_LIMIT_RAW_DATA_OFFSET,
	POWER_LIMIT_DATA_LENGTH,
	POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD,
	POWER_LIMIT_PARAMETER_INSTANCE_TYPE_NUM
} ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE, *P_ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE;

enum SKU_1_TXPWR_START_OFFSET {
	SKU_1_CCK_IDX = 0,
	SKU_1_OFDM_IDX = 4,
	SKU_1_HT20_IDX = 12,
	SKU_1_HT40_IDX = 20,
	SKU_1_VHT20_IDX = 29,
	SKU_1_VHT40_IDX = 41,
	SKU_1_VHT80_IDX = 53,
	SKU_1_VHT160_IDX = 65,
	SKU_1_HE26_IDX = 77,
	SKU_1_HE52_IDX = 89,
	SKU_1_HE106_IDX = 101,
	SKU_1_HE242_IDX = 113,
	SKU_1_HE484_IDX = 125,
	SKU_1_HE996_IDX = 137,
	SKU_1_HE996X2_IDX = 149,
	SKU_1_EHT26_IDX = 161,
	SKU_1_EHT52_IDX = 177,
	SKU_1_EHT106_IDX = 193,
	SKU_1_EHT242_IDX = 209,
	SKU_1_EHT484_IDX = 225,
	SKU_1_EHT996_IDX = 241,
	SKU_1_EHT996X2_IDX = 257,
	SKU_1_EHT996X4_IDX = 273,
	SKU_1_EHT26_52_IDX = 289,
	SKU_1_EHT26_106_IDX = 305,
	SKU_1_EHT484_242_IDX = 321,
	SKU_1_EHT996_484_IDX = 337,
	SKU_1_EHT996_484_242_IDX = 353,
	SKU_1_EHT996X2_484_IDX = 369,
	SKU_1_EHT996X3_IDX = 385,
	SKU_1_EHT996X3_484_IDX = 401,
	SKU_1_END_IDX = 417
};

enum SKU_2_TXPWR_START_OFFSET {
	SKU_2_CCK_BF_OFF_IDX = 0,
	SKU_2_OFDM_BF_OFF_IDX = 5,
	SKU_2_OFDM_BF_ON_IDX = 10,
	SKU_2_BW20_BF_OFF_IDX = 164,
	SKU_2_BW20_BF_ON_IDX = 179,
	SKU_2_BW40_BF_OFF_IDX = 194,
	SKU_2_BW40_BF_ON_IDX = 209,
	SKU_2_BW80_BF_OFF_IDX = 254,
	SKU_2_BW80_BF_ON_IDX = 269,
	SKU_2_BW160_BF_OFF_IDX = 344,
	SKU_2_BW160_BF_ON_IDX = 359,
	SKU_2_BW320_BF_OFF_IDX = 464,
	SKU_2_BW320_BF_ON_IDX = 479,
};

/*******************************************************************************
 *	GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *	FUNCTION PROTOTYPES
 ******************************************************************************/

NDIS_STATUS
MtPwrLimitLoadParamHandle(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitUnloadParamHandle(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitParse(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pi1PwrLimitNewCh,
	UINT8 u1ChBand,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitSimilarCheck(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pi1PwrLimitStartCh,
	PUINT8 pi1PwrLimitNewCh,
	BOOLEAN *pfgSameContent,
	UINT8 u1ChBand,
	UINT8 u1Type
	);

NDIS_STATUS
MtShowPwrLimitTable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type,
	UINT8 u1DebugLevel
	);

VOID
MtPwrLimitTblChProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1ChannelBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel
	);

NDIS_STATUS
MtPwrGetPwrLimitInstanceSku(
	struct _RTMP_ADAPTER *pAd,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID * ppvBuffer
	);

NDIS_STATUS
MtPwrGetPwrLimitInstanceBackoff(
	struct _RTMP_ADAPTER *pAd,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID *ppvBuffer);

NDIS_STATUS
MtPwrGetPwrLimitInstance(
	struct _RTMP_ADAPTER *pAd,
	POWER_LIMIT_TABLE u1Type,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID *ppvBuffer
	);

NDIS_STATUS
MtPowerLimitFormatTrans(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pu1Value,
	PCHAR pcRawData
	);

CHAR
SKUTxPwrOffsetGet(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx,
	UINT8 ucBW,
	UINT8 ucPhymode,
	UINT8 ucMCS,
	UINT8 ucNss,
	BOOLEAN fgSE
	);

NDIS_STATUS
MtPwrFillLimitParam(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ChBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel,
	VOID  *pi1PwrLimitParam,
	UINT8 u1Type
	);

NDIS_STATUS
MtPowerLimitFormatTrans(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pu1Value,
	PCHAR pcRawData
	);

#endif /*__CMM_SINGLE_SKU_H__*/
