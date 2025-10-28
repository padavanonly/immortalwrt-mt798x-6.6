// generate by structure_parser.py at 2022-07-05 12:31:09
#ifndef __WF_RX_DESCRIPTOR_G1_REGS_H__
#define __WF_RX_DESCRIPTOR_G1_REGS_H__
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
// ----------------- WF_RX_DESCRIPTOR_G1 Bit Field Definitions -----------------

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
        FIELD pn_31_0_                  : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW00, *PREG_WF_RX_DESCRIPTOR_G1_GROUP1_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_63_32_                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW01, *PREG_WF_RX_DESCRIPTOR_G1_GROUP1_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_95_64_                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW02, *PREG_WF_RX_DESCRIPTOR_G1_GROUP1_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_127_96_                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW03, *PREG_WF_RX_DESCRIPTOR_G1_GROUP1_DW03; // DW3

// ----------------- WF_RX_DESCRIPTOR_G1 Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR_G1 Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW00          WF_RX_DESCRIPTOR_G1_GROUP1_DW00         ; // 0x0000
    REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW01          WF_RX_DESCRIPTOR_G1_GROUP1_DW01         ; // 0x0004
    REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW02          WF_RX_DESCRIPTOR_G1_GROUP1_DW02         ; // 0x0008
    REG_WF_RX_DESCRIPTOR_G1_GROUP1_DW03          WF_RX_DESCRIPTOR_G1_GROUP1_DW03         ; // 0x000c
} WF_RX_DESCRIPTOR_GROUP1_REGS, *PWF_RX_DESCRIPTOR_GROUP1_REGS;// sw manually fixed
// ----------------- WF_RX_DESCRIPTOR_G1 Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR_G1 C Macro Definitions   -----------------
// sw manually fixed
extern PWF_RX_DESCRIPTOR_GROUP1_REGS g_WF_RX_DESCRIPTOR_G1_BASE;

#define WF_RX_DESCRIPTOR_GROUP1_BASE (g_WF_RX_DESCRIPTOR_G1_BASE)
#define WF_RX_DESCRIPTOR_GROUP1_DW00          INREG32(&WF_RX_DESCRIPTOR_GROUP1_BASE->WF_RX_DESCRIPTOR_G1_GROUP1_DW00         ) // 0x0000
#define WF_RX_DESCRIPTOR_GROUP1_DW01          INREG32(&WF_RX_DESCRIPTOR_GROUP1_BASE->WF_RX_DESCRIPTOR_G1_GROUP1_DW01         ) // 0x0004
#define WF_RX_DESCRIPTOR_GROUP1_DW02          INREG32(&WF_RX_DESCRIPTOR_GROUP1_BASE->WF_RX_DESCRIPTOR_G1_GROUP1_DW02         ) // 0x0008
#define WF_RX_DESCRIPTOR_GROUP1_DW03          INREG32(&WF_RX_DESCRIPTOR_GROUP1_BASE->WF_RX_DESCRIPTOR_G1_GROUP1_DW03         ) // 0x000c

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_G1_PN_31_0__DW                                        0
#define WF_RX_DESCRIPTOR_G1_PN_31_0__ADDR                                      0
#define WF_RX_DESCRIPTOR_G1_PN_31_0__MASK                                      0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G1_PN_31_0__SHIFT                                     0
// DW1
#define WF_RX_DESCRIPTOR_G1_PN_63_32__DW                                       1
#define WF_RX_DESCRIPTOR_G1_PN_63_32__ADDR                                     4
#define WF_RX_DESCRIPTOR_G1_PN_63_32__MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G1_PN_63_32__SHIFT                                    0
// DW2
#define WF_RX_DESCRIPTOR_G1_PN_95_64__DW                                       2
#define WF_RX_DESCRIPTOR_G1_PN_95_64__ADDR                                     8
#define WF_RX_DESCRIPTOR_G1_PN_95_64__MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G1_PN_95_64__SHIFT                                    0
// DW3
#define WF_RX_DESCRIPTOR_G1_PN_127_96__DW                                      3
#define WF_RX_DESCRIPTOR_G1_PN_127_96__ADDR                                    12
#define WF_RX_DESCRIPTOR_G1_PN_127_96__MASK                                    0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G1_PN_127_96__SHIFT                                   0

// DW0
#define HAL_RX_STATUS_GET_PN_31_0_(reg32)                                                                   READ_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_31_0_)
// DW1
#define HAL_RX_STATUS_GET_PN_63_32_(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_63_32_)
// DW2
#define HAL_RX_STATUS_GET_PN_95_64_(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_95_64_)
// DW3
#define HAL_RX_STATUS_GET_PN_127_96_(reg32)                                                                 READ_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_127_96_)

// DW0
#define HAL_RX_STATUS_SET_PN_31_0_(reg32, val32)                                                            WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_31_0_, val32)
// DW1
#define HAL_RX_STATUS_SET_PN_63_32_(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_63_32_, val32)
// DW2
#define HAL_RX_STATUS_SET_PN_95_64_(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_95_64_, val32)
// DW3
#define HAL_RX_STATUS_SET_PN_127_96_(reg32, val32)                                                          WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_127_96_, val32)

// DW0
#define HAL_RX_STATUS_CLR_PN_31_0_(reg32)                                                                   CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_31_0_)
// DW1
#define HAL_RX_STATUS_CLR_PN_63_32_(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_63_32_)
// DW2
#define HAL_RX_STATUS_CLR_PN_95_64_(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_95_64_)
// DW3
#define HAL_RX_STATUS_CLR_PN_127_96_(reg32)                                                                 CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G1_PN_127_96_)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_G1_REGS_H__
