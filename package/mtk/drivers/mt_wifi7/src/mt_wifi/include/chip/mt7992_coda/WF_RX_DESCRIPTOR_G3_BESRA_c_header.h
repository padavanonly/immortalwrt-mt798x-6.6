// generate by structure_parser.py at 2022-07-05 12:31:09
#ifndef __WF_RX_DESCRIPTOR_G3_REGS_H__
#define __WF_RX_DESCRIPTOR_G3_REGS_H__
/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef REG_BASE_C_MODULE
// ----------------- WF_RX_DESCRIPTOR_G3 Bit Field Definitions -----------------

#define PACKING
/* SW manually fixed */
//typedef unsigned int FIELD;
#ifndef FIELD
typedef unsigned int FIELD;
#endif


typedef PACKING union
{
    PACKING struct
    {
        FIELD p_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW00, *PREG_WF_RX_DESCRIPTOR_G3_GROUP3_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD p_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW01, *PREG_WF_RX_DESCRIPTOR_G3_GROUP3_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD dbw                       :  3; //  2- 0
        FIELD gi                        :  2; //  4- 3
        FIELD dcm                       :  1; //  5- 5
        FIELD num_rx                    :  3; //  8- 6
        FIELD stbc                      :  2; // 10- 9
        FIELD tx_mode                   :  4; // 14-11
        FIELD rsvd_31_15                : 17; // 31-15
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW02, *PREG_WF_RX_DESCRIPTOR_G3_GROUP3_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcpi                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW03, *PREG_WF_RX_DESCRIPTOR_G3_GROUP3_DW03; // DW3

// ----------------- WF_RX_DESCRIPTOR_G3 Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR_G3 Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW00          WF_RX_DESCRIPTOR_G3_GROUP3_DW00         ; // 0x0000
    REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW01          WF_RX_DESCRIPTOR_G3_GROUP3_DW01         ; // 0x0004
    REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW02          WF_RX_DESCRIPTOR_G3_GROUP3_DW02         ; // 0x0008
    REG_WF_RX_DESCRIPTOR_G3_GROUP3_DW03          WF_RX_DESCRIPTOR_G3_GROUP3_DW03         ; // 0x000c
} WF_RX_DESCRIPTOR_GROUP3_REGS, *PWF_RX_DESCRIPTOR_GROUP3_REGS;// sw manually fixed
// ----------------- WF_RX_DESCRIPTOR_G3 Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR_G3 C Macro Definitions   -----------------
// sw manually fixed
extern PWF_RX_DESCRIPTOR_GROUP3_REGS g_WF_RX_DESCRIPTOR_G3_BASE;

#define WF_RX_DESCRIPTOR_GROUP3_BASE (g_WF_RX_DESCRIPTOR_G3_BASE)
#define WF_RX_DESCRIPTOR_GROUP3_DW00          INREG32(&WF_RX_DESCRIPTOR_GROUP3_BASE->WF_RX_DESCRIPTOR_G3_GROUP3_DW00         ) // 0x0000
#define WF_RX_DESCRIPTOR_GROUP3_DW01          INREG32(&WF_RX_DESCRIPTOR_GROUP3_BASE->WF_RX_DESCRIPTOR_G3_GROUP3_DW01         ) // 0x0004
#define WF_RX_DESCRIPTOR_GROUP3_DW02          INREG32(&WF_RX_DESCRIPTOR_GROUP3_BASE->WF_RX_DESCRIPTOR_G3_GROUP3_DW02         ) // 0x0008
#define WF_RX_DESCRIPTOR_GROUP3_DW03          INREG32(&WF_RX_DESCRIPTOR_GROUP3_BASE->WF_RX_DESCRIPTOR_G3_GROUP3_DW03         ) // 0x000c

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_G3_P_RXV_DW                                           0
#define WF_RX_DESCRIPTOR_G3_P_RXV_ADDR                                         0
#define WF_RX_DESCRIPTOR_G3_P_RXV_MASK                                         0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G3_P_RXV_SHIFT                                        0
// DW1
// DO NOT process repeat field(p_rxv)
// DW2
#define WF_RX_DESCRIPTOR_G3_DBW_DW                                             2
#define WF_RX_DESCRIPTOR_G3_DBW_ADDR                                           8
#define WF_RX_DESCRIPTOR_G3_DBW_MASK                                           0x00000007 //  2- 0
#define WF_RX_DESCRIPTOR_G3_DBW_SHIFT                                          0
#define WF_RX_DESCRIPTOR_G3_GI_DW                                              2
#define WF_RX_DESCRIPTOR_G3_GI_ADDR                                            8
#define WF_RX_DESCRIPTOR_G3_GI_MASK                                            0x00000018 //  4- 3
#define WF_RX_DESCRIPTOR_G3_GI_SHIFT                                           3
#define WF_RX_DESCRIPTOR_G3_DCM_DW                                             2
#define WF_RX_DESCRIPTOR_G3_DCM_ADDR                                           8
#define WF_RX_DESCRIPTOR_G3_DCM_MASK                                           0x00000020 //  5- 5
#define WF_RX_DESCRIPTOR_G3_DCM_SHIFT                                          5
#define WF_RX_DESCRIPTOR_G3_NUM_RX_DW                                          2
#define WF_RX_DESCRIPTOR_G3_NUM_RX_ADDR                                        8
#define WF_RX_DESCRIPTOR_G3_NUM_RX_MASK                                        0x000001c0 //  8- 6
#define WF_RX_DESCRIPTOR_G3_NUM_RX_SHIFT                                       6
#define WF_RX_DESCRIPTOR_G3_STBC_DW                                            2
#define WF_RX_DESCRIPTOR_G3_STBC_ADDR                                          8
#define WF_RX_DESCRIPTOR_G3_STBC_MASK                                          0x00000600 // 10- 9
#define WF_RX_DESCRIPTOR_G3_STBC_SHIFT                                         9
#define WF_RX_DESCRIPTOR_G3_TX_MODE_DW                                         2
#define WF_RX_DESCRIPTOR_G3_TX_MODE_ADDR                                       8
#define WF_RX_DESCRIPTOR_G3_TX_MODE_MASK                                       0x00007800 // 14-11
#define WF_RX_DESCRIPTOR_G3_TX_MODE_SHIFT                                      11
// DW3
#define WF_RX_DESCRIPTOR_G3_RCPI_DW                                            3
#define WF_RX_DESCRIPTOR_G3_RCPI_ADDR                                          12
#define WF_RX_DESCRIPTOR_G3_RCPI_MASK                                          0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G3_RCPI_SHIFT                                         0

// DW0
#define HAL_RX_STATUS_GET_P_RXV(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_P_RXV)
// DW1
// DO NOT process repeat field(p_rxv)
// DW2
#define HAL_RX_STATUS_GET_DBW(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DBW)
#define HAL_RX_STATUS_GET_GI(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_GI)
#define HAL_RX_STATUS_GET_DCM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DCM)
#define HAL_RX_STATUS_GET_NUM_RX(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_NUM_RX)
#define HAL_RX_STATUS_GET_STBC(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_STBC)
#define HAL_RX_STATUS_GET_TX_MODE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_TX_MODE)
// DW3
#define HAL_RX_STATUS_GET_RCPI(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G3_RCPI)

// DW0
#define HAL_RX_STATUS_SET_P_RXV(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_P_RXV, val32)
// DW1
// DO NOT process repeat field(p_rxv)
// DW2
#define HAL_RX_STATUS_SET_DBW(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DBW, val32)
#define HAL_RX_STATUS_SET_GI(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_GI, val32)
#define HAL_RX_STATUS_SET_DCM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DCM, val32)
#define HAL_RX_STATUS_SET_NUM_RX(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_NUM_RX, val32)
#define HAL_RX_STATUS_SET_STBC(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_STBC, val32)
#define HAL_RX_STATUS_SET_TX_MODE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_TX_MODE, val32)
// DW3
#define HAL_RX_STATUS_SET_RCPI(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G3_RCPI, val32)

// DW0
#define HAL_RX_STATUS_CLR_P_RXV(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_P_RXV)
// DW1
// DO NOT process repeat field(p_rxv)
// DW2
#define HAL_RX_STATUS_CLR_DBW(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DBW)
#define HAL_RX_STATUS_CLR_GI(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_GI)
#define HAL_RX_STATUS_CLR_DCM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_DCM)
#define HAL_RX_STATUS_CLR_NUM_RX(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_NUM_RX)
#define HAL_RX_STATUS_CLR_STBC(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_STBC)
#define HAL_RX_STATUS_CLR_TX_MODE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_TX_MODE)
// DW3
#define HAL_RX_STATUS_CLR_RCPI(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G3_RCPI)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_G3_REGS_H__
