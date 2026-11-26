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
	mt_txbf_cal.h
*/


#ifndef _RT_TXBF_CAL_H_
#define _RT_TXBF_CAL_H_

#define TX_PATH_2   2
#define TX_PATH_3   3
#define TX_PATH_4   4
#define TX_PATH_5   5
#define TX_PATH_4_BIN 15

#define GROUP_0     0
#define CH_001      1
#define CH_008      8
#define CH_014      14
#define GROUP_1     1
#define CH_036      36
#define CH_044      44
#define GROUP_2     2
#define CH_052      52
#define CH_060      60
#define CH_064      64
#define GROUP_3     3
#define CH_068      68
#define CH_084      84
#define CH_092      92
#define GROUP_4     5
#define CH_100      100
#define CH_104      104
#define CH_112      112
#define GROUP_5     5
#define CH_116      116
#define CH_124      124
#define CH_128      128
#define GROUP_6     6
#define CH_132      132
#define CH_136      136
#define CH_144      144
#define GROUP_7     7
#define CH_149      149
#define CH_153      153
#define CH_161      161
#define GROUP_8     8
#define CH_165      165
#define CH_173      173
#define CH_181      181

#define GROUP_L     0
#define GROUP_M     1
#define GROUP_H     2

#define CLEAN_ALL   0
#define CLEAN_2G    1
#define CLEAN_5G    2
#define CLEAN_6G    3

#define IBF_PHASE_EEPROM_START      0xC00

#define IBF_LNA_PHASE_G0_ADDR       IBF_PHASE_EEPROM_START
#define IBF_LNA_PHASE_G1_ADDR       (IBF_LNA_PHASE_G0_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G2_ADDR       (IBF_LNA_PHASE_G1_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G3_ADDR       (IBF_LNA_PHASE_G2_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G4_ADDR       (IBF_LNA_PHASE_G3_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G5_ADDR       (IBF_LNA_PHASE_G4_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G6_ADDR       (IBF_LNA_PHASE_G5_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G7_ADDR       (IBF_LNA_PHASE_G6_ADDR  + 0x2E)
#define IBF_LNA_PHASE_G8_ADDR       (IBF_LNA_PHASE_G7_ADDR  + 0x2E)


enum IBF_PHASE_E2P_UPDATE_TYPE {
	IBF_PHASE_ONE_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_ERASE,
	IBF_PHASE_ALL_GROUP_READ_FROM_E2P
};

enum IBF_PHASE_CAL_TYPE {
	IBF_PHASE_CAL_NOTHING,
	IBF_PHASE_CAL_NORMAL,
	IBF_PHASE_CAL_VERIFY,
	IBF_PHASE_CAL_NORMAL_INSTRUMENT,

	IBF_PHASE_CAL_VERIFY_INSTRUMENT

};

enum IBF_PHASE_STATUS_INSTRUMENT {
	STATUS_EBF_INVALID,
	STATUS_IBF_INVALID,
	STATUS_OTHER_ISSUE,
	STATUS_DONE
};

struct IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC0_MH;
	UINT8 ucC1_MH;
	UINT8 ucC2_MH;
	UINT8 ucC3_MH;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC3_UH;
};

struct IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R0_UL;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R1_UL;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R2_UL;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_R3_UL;
	UINT8 ucG0_R2_UH_SX2;
	UINT8 ucG0_R2_H_SX2;
	UINT8 ucG0_R2_M_SX2;
	UINT8 ucG0_R2_L_SX2;
	UINT8 ucG0_R2_UL_SX2;
	UINT8 ucG0_R3_UH_SX2;
	UINT8 ucG0_R3_H_SX2;
	UINT8 ucG0_R3_M_SX2;
	UINT8 ucG0_R3_L_SX2;
	UINT8 ucG0_R3_UL_SX2;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
	UINT8 ucG0_M_T2_H_SX2;
	UINT8 ucG0_R0_Reserved;
	UINT8 ucG0_R1_Reserved;
	UINT8 ucG0_R2_Reserved;
	UINT8 ucG0_R3_Reserved;
	UINT8 ucG0_R2_SX2_Reserved;
	UINT8 ucG0_R3_SX2_Reserved;
};

struct IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_MH;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R0_UL;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_MH;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R1_UL;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_MH;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R2_UL;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_MH;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R3_UL;
	UINT8 ucGx_R2_UH_SX2;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R2_UL_SX2;
	UINT8 ucGx_R3_UH_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_R3_UL_SX2;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T2_H_SX2;
	UINT8 ucGx_R0_Reserved;
	UINT8 ucGx_R1_Reserved;
	UINT8 ucGx_R2_Reserved;
	UINT8 ucGx_R3_Reserved;
	UINT8 ucGx_R2_SX2_Reserved;
	UINT8 ucGx_R3_SX2_Reserved;
};

VOID iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);

VOID eBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);

VOID iBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);

#endif /* _RT_TXBF_CAL_H_ */
