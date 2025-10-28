// generate by structure_parser.py at 2022-07-05 12:31:09
#ifndef __WF_RX_DESCRIPTOR_G2_REGS_H__
#define __WF_RX_DESCRIPTOR_G2_REGS_H__
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
// ----------------- WF_RX_DESCRIPTOR_G2 Bit Field Definitions -----------------

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
        FIELD timestamp                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW00, *PREG_WF_RX_DESCRIPTOR_G2_GROUP2_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD crc                       : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW01, *PREG_WF_RX_DESCRIPTOR_G2_GROUP2_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW02, *PREG_WF_RX_DESCRIPTOR_G2_GROUP2_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW03, *PREG_WF_RX_DESCRIPTOR_G2_GROUP2_DW03; // DW3

// ----------------- WF_RX_DESCRIPTOR_G2 Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR_G2 Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW00          WF_RX_DESCRIPTOR_G2_GROUP2_DW00         ; // 0x0000
    REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW01          WF_RX_DESCRIPTOR_G2_GROUP2_DW01         ; // 0x0004
    REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW02          WF_RX_DESCRIPTOR_G2_GROUP2_DW02         ; // 0x0008
    REG_WF_RX_DESCRIPTOR_G2_GROUP2_DW03          WF_RX_DESCRIPTOR_G2_GROUP2_DW03         ; // 0x000c
} WF_RX_DESCRIPTOR_GROUP2_REGS, *PWF_RX_DESCRIPTOR_GROUP2_REGS;// sw manually fixed
// ----------------- WF_RX_DESCRIPTOR_G2 Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR_G2 C Macro Definitions   -----------------
// sw manually fixed
extern PWF_RX_DESCRIPTOR_GROUP2_REGS g_WF_RX_DESCRIPTOR_G2_BASE;

#define WF_RX_DESCRIPTOR_GROUP2_BASE (g_WF_RX_DESCRIPTOR_G2_BASE)
#define WF_RX_DESCRIPTOR_GROUP2_DW00          INREG32(&WF_RX_DESCRIPTOR_GROUP2_BASE->WF_RX_DESCRIPTOR_G2_GROUP2_DW00         ) // 0x0000
#define WF_RX_DESCRIPTOR_GROUP2_DW01          INREG32(&WF_RX_DESCRIPTOR_GROUP2_BASE->WF_RX_DESCRIPTOR_G2_GROUP2_DW01         ) // 0x0004
#define WF_RX_DESCRIPTOR_GROUP2_DW02          INREG32(&WF_RX_DESCRIPTOR_GROUP2_BASE->WF_RX_DESCRIPTOR_G2_GROUP2_DW02         ) // 0x0008
#define WF_RX_DESCRIPTOR_GROUP2_DW03          INREG32(&WF_RX_DESCRIPTOR_GROUP2_BASE->WF_RX_DESCRIPTOR_G2_GROUP2_DW03         ) // 0x000c

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_G2_TIMESTAMP_DW                                       0
#define WF_RX_DESCRIPTOR_G2_TIMESTAMP_ADDR                                     0
#define WF_RX_DESCRIPTOR_G2_TIMESTAMP_MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G2_TIMESTAMP_SHIFT                                    0
// DW1
#define WF_RX_DESCRIPTOR_G2_CRC_DW                                             1
#define WF_RX_DESCRIPTOR_G2_CRC_ADDR                                           4
#define WF_RX_DESCRIPTOR_G2_CRC_MASK                                           0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G2_CRC_SHIFT                                          0
// DW2
// DW3

// DW0
#define HAL_RX_STATUS_GET_TIMESTAMP(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_G2_TIMESTAMP)
// DW1
#define HAL_RX_STATUS_GET_CRC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G2_CRC)
// DW2
// DW3

// DW0
#define HAL_RX_STATUS_SET_TIMESTAMP(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G2_TIMESTAMP, val32)
// DW1
#define HAL_RX_STATUS_SET_CRC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G2_CRC, val32)
// DW2
// DW3

// DW0
#define HAL_RX_STATUS_CLR_TIMESTAMP(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G2_TIMESTAMP)
// DW1
#define HAL_RX_STATUS_CLR_CRC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G2_CRC)
// DW2
// DW3

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_G2_REGS_H__
