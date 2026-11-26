// generate by structure_parser.py at 2022-07-05 12:31:09
#ifndef __WF_RX_DESCRIPTOR_G4_REGS_H__
#define __WF_RX_DESCRIPTOR_G4_REGS_H__
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
// ----------------- WF_RX_DESCRIPTOR_G4 Bit Field Definitions -----------------

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
        FIELD frame_control_field       : 16; // 15- 0
        FIELD peer_mld_address_15_0_    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW00, *PREG_WF_RX_DESCRIPTOR_G4_GROUP4_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD peer_mld_address_47_16_   : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW01, *PREG_WF_RX_DESCRIPTOR_G4_GROUP4_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD fragment_number           :  4; //  3- 0
        FIELD sequence_number           : 12; // 15- 4
        FIELD qos_control_field         : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW02, *PREG_WF_RX_DESCRIPTOR_G4_GROUP4_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD ht_control_field          : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW03, *PREG_WF_RX_DESCRIPTOR_G4_GROUP4_DW03; // DW3

// ----------------- WF_RX_DESCRIPTOR_G4 Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR_G4 Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW00          WF_RX_DESCRIPTOR_G4_GROUP4_DW00         ; // 0x0000
    REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW01          WF_RX_DESCRIPTOR_G4_GROUP4_DW01         ; // 0x0004
    REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW02          WF_RX_DESCRIPTOR_G4_GROUP4_DW02         ; // 0x0008
    REG_WF_RX_DESCRIPTOR_G4_GROUP4_DW03          WF_RX_DESCRIPTOR_G4_GROUP4_DW03         ; // 0x000c
} WF_RX_DESCRIPTOR_GROUP4_REGS, *PWF_RX_DESCRIPTOR_GROUP4_REGS;// sw manually fixed
// ----------------- WF_RX_DESCRIPTOR_G4 Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR_G4 C Macro Definitions   -----------------
// sw manually fixed
extern PWF_RX_DESCRIPTOR_GROUP4_REGS g_WF_RX_DESCRIPTOR_G4_BASE;

#define WF_RX_DESCRIPTOR_GROUP4_BASE (g_WF_RX_DESCRIPTOR_G4_BASE)
#define WF_RX_DESCRIPTOR_GROUP4_DW00          INREG32(&WF_RX_DESCRIPTOR_GROUP4_BASE->WF_RX_DESCRIPTOR_G4_GROUP4_DW00         ) // 0x0000
#define WF_RX_DESCRIPTOR_GROUP4_DW01          INREG32(&WF_RX_DESCRIPTOR_GROUP4_BASE->WF_RX_DESCRIPTOR_G4_GROUP4_DW01         ) // 0x0004
#define WF_RX_DESCRIPTOR_GROUP4_DW02          INREG32(&WF_RX_DESCRIPTOR_GROUP4_BASE->WF_RX_DESCRIPTOR_G4_GROUP4_DW02         ) // 0x0008
#define WF_RX_DESCRIPTOR_GROUP4_DW03          INREG32(&WF_RX_DESCRIPTOR_GROUP4_BASE->WF_RX_DESCRIPTOR_G4_GROUP4_DW03         ) // 0x000c

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD_DW                             0
#define WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD_ADDR                           0
#define WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD_MASK                           0x0000ffff // 15- 0
#define WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD_SHIFT                          0
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0__DW                          0
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0__ADDR                        0
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0__MASK                        0xffff0000 // 31-16
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0__SHIFT                       16
// DW1
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16__DW                         1
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16__ADDR                       4
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16__MASK                       0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16__SHIFT                      0
// DW2
#define WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER_DW                                 2
#define WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER_ADDR                               8
#define WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER_MASK                               0x0000000f //  3- 0
#define WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER_SHIFT                              0
#define WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER_DW                                 2
#define WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER_ADDR                               8
#define WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER_MASK                               0x0000fff0 // 15- 4
#define WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER_SHIFT                              4
#define WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD_DW                               2
#define WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD_ADDR                             8
#define WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD_MASK                             0xffff0000 // 31-16
#define WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD_SHIFT                            16
// DW3
#define WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD_DW                                3
#define WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD_ADDR                              12
#define WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD_MASK                              0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD_SHIFT                             0

// DW0
#define HAL_RX_STATUS_GET_FRAME_CONTROL_FIELD(reg32)                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD)
#define HAL_RX_STATUS_GET_PEER_MLD_ADDRESS_15_0_(reg32)                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0_)
// DW1
#define HAL_RX_STATUS_GET_PEER_MLD_ADDRESS_47_16_(reg32)                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16_)
// DW2
#define HAL_RX_STATUS_GET_FRAGMENT_NUMBER(reg32)                                                            READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER)
#define HAL_RX_STATUS_GET_SEQUENCE_NUMBER(reg32)                                                            READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER)
#define HAL_RX_STATUS_GET_QOS_CONTROL_FIELD(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD)
// DW3
#define HAL_RX_STATUS_GET_HT_CONTROL_FIELD(reg32)                                                           READ_FIELD((reg32), WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD)

// DW0
#define HAL_RX_STATUS_SET_FRAME_CONTROL_FIELD(reg32, val32)                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD, val32)
#define HAL_RX_STATUS_SET_PEER_MLD_ADDRESS_15_0_(reg32, val32)                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0_, val32)
// DW1
#define HAL_RX_STATUS_SET_PEER_MLD_ADDRESS_47_16_(reg32, val32)                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16_, val32)
// DW2
#define HAL_RX_STATUS_SET_FRAGMENT_NUMBER(reg32, val32)                                                     WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER, val32)
#define HAL_RX_STATUS_SET_SEQUENCE_NUMBER(reg32, val32)                                                     WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER, val32)
#define HAL_RX_STATUS_SET_QOS_CONTROL_FIELD(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD, val32)
// DW3
#define HAL_RX_STATUS_SET_HT_CONTROL_FIELD(reg32, val32)                                                    WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD, val32)

// DW0
#define HAL_RX_STATUS_CLR_FRAME_CONTROL_FIELD(reg32)                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAME_CONTROL_FIELD)
#define HAL_RX_STATUS_CLR_PEER_MLD_ADDRESS_15_0_(reg32)                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_15_0_)
// DW1
#define HAL_RX_STATUS_CLR_PEER_MLD_ADDRESS_47_16_(reg32)                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_PEER_MLD_ADDRESS_47_16_)
// DW2
#define HAL_RX_STATUS_CLR_FRAGMENT_NUMBER(reg32)                                                            CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_FRAGMENT_NUMBER)
#define HAL_RX_STATUS_CLR_SEQUENCE_NUMBER(reg32)                                                            CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_SEQUENCE_NUMBER)
#define HAL_RX_STATUS_CLR_QOS_CONTROL_FIELD(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_QOS_CONTROL_FIELD)
// DW3
#define HAL_RX_STATUS_CLR_HT_CONTROL_FIELD(reg32)                                                           CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G4_HT_CONTROL_FIELD)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_G4_REGS_H__
