// generate by structure_parser.py at 2022-07-05 12:31:09
#ifndef __WF_RX_DESCRIPTOR_G0_REGS_H__
#define __WF_RX_DESCRIPTOR_G0_REGS_H__
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
// ----------------- WF_RX_DESCRIPTOR_G0 Bit Field Definitions -----------------

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
        FIELD rx_byte_count             : 16; // 15- 0
        FIELD cosr                      :  1; // 16-16
        FIELD rsvd_26_17                : 10; // 26-17
        FIELD packet_type               :  5; // 31-27
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW00, *PREG_WF_RX_DESCRIPTOR_G0_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD mld_id                    : 12; // 11- 0
        FIELD rsvd_15_12                :  4; // 15-12
        FIELD group_vld                 :  5; // 20-16
        FIELD kid                       :  2; // 22-21
        FIELD cm                        :  1; // 23-23
        FIELD clm                       :  1; // 24-24
        FIELD i                         :  1; // 25-25
        FIELD t                         :  1; // 26-26
        FIELD bn                        :  2; // 28-27
        FIELD bipn_fail                 :  1; // 29-29
        FIELD rsvd_31_30                :  2; // 31-30
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW01, *PREG_WF_RX_DESCRIPTOR_G0_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD bssid                     :  6; //  5- 0
        FIELD rsvd_06_06                :  1; //  6- 6
        FIELD h                         :  1; //  7- 7
        FIELD header_length             :  5; // 12- 8
        FIELD ho                        :  3; // 15-13
        FIELD sec_mode                  :  5; // 20-16
        FIELD mubar                     :  1; // 21-21
        FIELD swbit                     :  1; // 22-22
        FIELD daf                       :  1; // 23-23
        FIELD el                        :  1; // 24-24
        FIELD htf                       :  1; // 25-25
        FIELD intf                      :  1; // 26-26
        FIELD frag                      :  1; // 27-27
        FIELD nul                       :  1; // 28-28
        FIELD ndata                     :  1; // 29-29
        FIELD namp                      :  1; // 30-30
        FIELD bf_rpt                    :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW02, *PREG_WF_RX_DESCRIPTOR_G0_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxv_sn                    :  8; //  7- 0
        FIELD ch_frequency              :  8; // 15- 8
        FIELD a1_type                   :  2; // 17-16
        FIELD htc                       :  1; // 18-18
        FIELD tcl                       :  1; // 19-19
        FIELD bbm                       :  1; // 20-20
        FIELD bu                        :  1; // 21-21
        FIELD co_ant                    :  1; // 22-22
        FIELD bf_cqi                    :  1; // 23-23
        FIELD fc                        :  1; // 24-24
        FIELD rsvd_25_25                :  1; // 25-25
        FIELD ip                        :  1; // 26-26
        FIELD ut                        :  1; // 27-27
        FIELD rsvd_29_28                :  2; // 29-28
        FIELD smesh                     :  1; // 30-30
        FIELD vlan                      :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW03, *PREG_WF_RX_DESCRIPTOR_G0_DW03; // DW3

typedef PACKING union
{
    PACKING struct
    {
        FIELD pf                        :  2; //  1- 0
        FIELD mac                       :  1; //  2- 2
        FIELD tid                       :  4; //  6- 3
        FIELD ether_type_offset         :  8; // 14- 7
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD pse_fid                   : 13; // 28-16
        FIELD rsvd_31_29                :  3; // 31-29
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW04, *PREG_WF_RX_DESCRIPTOR_G0_DW04; // DW4

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_DW05, *PREG_WF_RX_DESCRIPTOR_G0_DW05; // DW5

typedef PACKING union
{
    PACKING struct
    {
        FIELD cls_bitmap_31_0_          : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_PF_DW06, *PREG_WF_RX_DESCRIPTOR_G0_PF_DW06; // DW6

typedef PACKING union
{
    PACKING struct
    {
        FIELD cls_bitmap_33_32_         :  2; //  1- 0
        FIELD rsvd_18_02                : 17; // 18- 2
        FIELD dp                        :  1; // 19-19
        FIELD cls                       :  1; // 20-20
        FIELD ofld                      :  2; // 22-21
        FIELD mgc                       :  1; // 23-23
        FIELD wol                       :  5; // 28-24
        FIELD pf_mode                   :  1; // 29-29
        FIELD pf_sts                    :  2; // 31-30
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_G0_PF_DW07, *PREG_WF_RX_DESCRIPTOR_G0_PF_DW07; // DW7

// ----------------- WF_RX_DESCRIPTOR_G0 Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR_G0 Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_G0_DW00                 WF_RX_DESCRIPTOR_G0_DW00                ; // 0x0000
    REG_WF_RX_DESCRIPTOR_G0_DW01                 WF_RX_DESCRIPTOR_G0_DW01                ; // 0x0004
    REG_WF_RX_DESCRIPTOR_G0_DW02                 WF_RX_DESCRIPTOR_G0_DW02                ; // 0x0008
    REG_WF_RX_DESCRIPTOR_G0_DW03                 WF_RX_DESCRIPTOR_G0_DW03                ; // 0x000c
    REG_WF_RX_DESCRIPTOR_G0_DW04                 WF_RX_DESCRIPTOR_G0_DW04                ; // 0x0010
    REG_WF_RX_DESCRIPTOR_G0_DW05                 WF_RX_DESCRIPTOR_G0_DW05                ; // 0x0014
    REG_WF_RX_DESCRIPTOR_G0_PF_DW06              WF_RX_DESCRIPTOR_G0_PF_DW06             ; // 0x0018
    REG_WF_RX_DESCRIPTOR_G0_PF_DW07              WF_RX_DESCRIPTOR_G0_PF_DW07             ; // 0x001c
} WF_RX_DESCRIPTOR_REGS, *PWF_RX_DESCRIPTOR_REGS; // sw manually fixed
// ----------------- WF_RX_DESCRIPTOR_G0 Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR_G0 C Macro Definitions   -----------------
// sw manually fixed
extern PWF_RX_DESCRIPTOR_REGS g_WF_RX_DESCRIPTOR_G0_BASE;

#define WF_RX_DESCRIPTOR_GROUP0_BASE (g_WF_RX_DESCRIPTOR_G0_BASE)
#define WF_RX_DESCRIPTOR_GROUP0_DW00                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW00                ) // 0x0000
#define WF_RX_DESCRIPTOR_GROUP0_DW01                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW01                ) // 0x0004
#define WF_RX_DESCRIPTOR_GROUP0_DW02                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW02                ) // 0x0008
#define WF_RX_DESCRIPTOR_GROUP0_DW03                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW03                ) // 0x000c
#define WF_RX_DESCRIPTOR_GROUP0_DW04                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW04                ) // 0x0010
#define WF_RX_DESCRIPTOR_GROUP0_DW05                 INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_DW05                ) // 0x0014
#define WF_RX_DESCRIPTOR_GROUP0_PF_DW06              INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_PF_DW06             ) // 0x0018
#define WF_RX_DESCRIPTOR_GROUP0_PF_DW07              INREG32(&WF_RX_DESCRIPTOR_GROUP0_BASE->WF_RX_DESCRIPTOR_G0_PF_DW07             ) // 0x001c

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT_DW                                   0
#define WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT_ADDR                                 0
#define WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT_MASK                                 0x0000ffff // 15- 0
#define WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT_SHIFT                                0
#define WF_RX_DESCRIPTOR_G0_COSR_DW                                            0
#define WF_RX_DESCRIPTOR_G0_COSR_ADDR                                          0
#define WF_RX_DESCRIPTOR_G0_COSR_MASK                                          0x00010000 // 16-16
#define WF_RX_DESCRIPTOR_G0_COSR_SHIFT                                         16
#define WF_RX_DESCRIPTOR_G0_PACKET_TYPE_DW                                     0
#define WF_RX_DESCRIPTOR_G0_PACKET_TYPE_ADDR                                   0
#define WF_RX_DESCRIPTOR_G0_PACKET_TYPE_MASK                                   0xf8000000 // 31-27
#define WF_RX_DESCRIPTOR_G0_PACKET_TYPE_SHIFT                                  27
// DW1
#define WF_RX_DESCRIPTOR_G0_MLD_ID_DW                                          1
#define WF_RX_DESCRIPTOR_G0_MLD_ID_ADDR                                        4
#define WF_RX_DESCRIPTOR_G0_MLD_ID_MASK                                        0x00000fff // 11- 0
#define WF_RX_DESCRIPTOR_G0_MLD_ID_SHIFT                                       0
#define WF_RX_DESCRIPTOR_G0_GROUP_VLD_DW                                       1
#define WF_RX_DESCRIPTOR_G0_GROUP_VLD_ADDR                                     4
#define WF_RX_DESCRIPTOR_G0_GROUP_VLD_MASK                                     0x001f0000 // 20-16
#define WF_RX_DESCRIPTOR_G0_GROUP_VLD_SHIFT                                    16
#define WF_RX_DESCRIPTOR_G0_KID_DW                                             1
#define WF_RX_DESCRIPTOR_G0_KID_ADDR                                           4
#define WF_RX_DESCRIPTOR_G0_KID_MASK                                           0x00600000 // 22-21
#define WF_RX_DESCRIPTOR_G0_KID_SHIFT                                          21
#define WF_RX_DESCRIPTOR_G0_CM_DW                                              1
#define WF_RX_DESCRIPTOR_G0_CM_ADDR                                            4
#define WF_RX_DESCRIPTOR_G0_CM_MASK                                            0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_G0_CM_SHIFT                                           23
#define WF_RX_DESCRIPTOR_G0_CLM_DW                                             1
#define WF_RX_DESCRIPTOR_G0_CLM_ADDR                                           4
#define WF_RX_DESCRIPTOR_G0_CLM_MASK                                           0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_G0_CLM_SHIFT                                          24
#define WF_RX_DESCRIPTOR_G0_I_DW                                               1
#define WF_RX_DESCRIPTOR_G0_I_ADDR                                             4
#define WF_RX_DESCRIPTOR_G0_I_MASK                                             0x02000000 // 25-25
#define WF_RX_DESCRIPTOR_G0_I_SHIFT                                            25
#define WF_RX_DESCRIPTOR_G0_T_DW                                               1
#define WF_RX_DESCRIPTOR_G0_T_ADDR                                             4
#define WF_RX_DESCRIPTOR_G0_T_MASK                                             0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_G0_T_SHIFT                                            26
#define WF_RX_DESCRIPTOR_G0_BN_DW                                              1
#define WF_RX_DESCRIPTOR_G0_BN_ADDR                                            4
#define WF_RX_DESCRIPTOR_G0_BN_MASK                                            0x18000000 // 28-27
#define WF_RX_DESCRIPTOR_G0_BN_SHIFT                                           27
#define WF_RX_DESCRIPTOR_G0_BIPN_FAIL_DW                                       1
#define WF_RX_DESCRIPTOR_G0_BIPN_FAIL_ADDR                                     4
#define WF_RX_DESCRIPTOR_G0_BIPN_FAIL_MASK                                     0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_G0_BIPN_FAIL_SHIFT                                    29
// DW2
#define WF_RX_DESCRIPTOR_G0_BSSID_DW                                           2
#define WF_RX_DESCRIPTOR_G0_BSSID_ADDR                                         8
#define WF_RX_DESCRIPTOR_G0_BSSID_MASK                                         0x0000003f //  5- 0
#define WF_RX_DESCRIPTOR_G0_BSSID_SHIFT                                        0
#define WF_RX_DESCRIPTOR_G0_H_DW                                               2
#define WF_RX_DESCRIPTOR_G0_H_ADDR                                             8
#define WF_RX_DESCRIPTOR_G0_H_MASK                                             0x00000080 //  7- 7
#define WF_RX_DESCRIPTOR_G0_H_SHIFT                                            7
#define WF_RX_DESCRIPTOR_G0_HEADER_LENGTH_DW                                   2
#define WF_RX_DESCRIPTOR_G0_HEADER_LENGTH_ADDR                                 8
#define WF_RX_DESCRIPTOR_G0_HEADER_LENGTH_MASK                                 0x00001f00 // 12- 8
#define WF_RX_DESCRIPTOR_G0_HEADER_LENGTH_SHIFT                                8
#define WF_RX_DESCRIPTOR_G0_HO_DW                                              2
#define WF_RX_DESCRIPTOR_G0_HO_ADDR                                            8
#define WF_RX_DESCRIPTOR_G0_HO_MASK                                            0x0000e000 // 15-13
#define WF_RX_DESCRIPTOR_G0_HO_SHIFT                                           13
#define WF_RX_DESCRIPTOR_G0_SEC_MODE_DW                                        2
#define WF_RX_DESCRIPTOR_G0_SEC_MODE_ADDR                                      8
#define WF_RX_DESCRIPTOR_G0_SEC_MODE_MASK                                      0x001f0000 // 20-16
#define WF_RX_DESCRIPTOR_G0_SEC_MODE_SHIFT                                     16
#define WF_RX_DESCRIPTOR_G0_MUBAR_DW                                           2
#define WF_RX_DESCRIPTOR_G0_MUBAR_ADDR                                         8
#define WF_RX_DESCRIPTOR_G0_MUBAR_MASK                                         0x00200000 // 21-21
#define WF_RX_DESCRIPTOR_G0_MUBAR_SHIFT                                        21
#define WF_RX_DESCRIPTOR_G0_SWBIT_DW                                           2
#define WF_RX_DESCRIPTOR_G0_SWBIT_ADDR                                         8
#define WF_RX_DESCRIPTOR_G0_SWBIT_MASK                                         0x00400000 // 22-22
#define WF_RX_DESCRIPTOR_G0_SWBIT_SHIFT                                        22
#define WF_RX_DESCRIPTOR_G0_DAF_DW                                             2
#define WF_RX_DESCRIPTOR_G0_DAF_ADDR                                           8
#define WF_RX_DESCRIPTOR_G0_DAF_MASK                                           0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_G0_DAF_SHIFT                                          23
#define WF_RX_DESCRIPTOR_G0_EL_DW                                              2
#define WF_RX_DESCRIPTOR_G0_EL_ADDR                                            8
#define WF_RX_DESCRIPTOR_G0_EL_MASK                                            0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_G0_EL_SHIFT                                           24
#define WF_RX_DESCRIPTOR_G0_HTF_DW                                             2
#define WF_RX_DESCRIPTOR_G0_HTF_ADDR                                           8
#define WF_RX_DESCRIPTOR_G0_HTF_MASK                                           0x02000000 // 25-25
#define WF_RX_DESCRIPTOR_G0_HTF_SHIFT                                          25
#define WF_RX_DESCRIPTOR_G0_INTF_DW                                            2
#define WF_RX_DESCRIPTOR_G0_INTF_ADDR                                          8
#define WF_RX_DESCRIPTOR_G0_INTF_MASK                                          0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_G0_INTF_SHIFT                                         26
#define WF_RX_DESCRIPTOR_G0_FRAG_DW                                            2
#define WF_RX_DESCRIPTOR_G0_FRAG_ADDR                                          8
#define WF_RX_DESCRIPTOR_G0_FRAG_MASK                                          0x08000000 // 27-27
#define WF_RX_DESCRIPTOR_G0_FRAG_SHIFT                                         27
#define WF_RX_DESCRIPTOR_G0_NUL_DW                                             2
#define WF_RX_DESCRIPTOR_G0_NUL_ADDR                                           8
#define WF_RX_DESCRIPTOR_G0_NUL_MASK                                           0x10000000 // 28-28
#define WF_RX_DESCRIPTOR_G0_NUL_SHIFT                                          28
#define WF_RX_DESCRIPTOR_G0_NDATA_DW                                           2
#define WF_RX_DESCRIPTOR_G0_NDATA_ADDR                                         8
#define WF_RX_DESCRIPTOR_G0_NDATA_MASK                                         0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_G0_NDATA_SHIFT                                        29
#define WF_RX_DESCRIPTOR_G0_NAMP_DW                                            2
#define WF_RX_DESCRIPTOR_G0_NAMP_ADDR                                          8
#define WF_RX_DESCRIPTOR_G0_NAMP_MASK                                          0x40000000 // 30-30
#define WF_RX_DESCRIPTOR_G0_NAMP_SHIFT                                         30
#define WF_RX_DESCRIPTOR_G0_BF_RPT_DW                                          2
#define WF_RX_DESCRIPTOR_G0_BF_RPT_ADDR                                        8
#define WF_RX_DESCRIPTOR_G0_BF_RPT_MASK                                        0x80000000 // 31-31
#define WF_RX_DESCRIPTOR_G0_BF_RPT_SHIFT                                       31
// DW3
#define WF_RX_DESCRIPTOR_G0_RXV_SN_DW                                          3
#define WF_RX_DESCRIPTOR_G0_RXV_SN_ADDR                                        12
#define WF_RX_DESCRIPTOR_G0_RXV_SN_MASK                                        0x000000ff //  7- 0
#define WF_RX_DESCRIPTOR_G0_RXV_SN_SHIFT                                       0
#define WF_RX_DESCRIPTOR_G0_CH_FREQUENCY_DW                                    3
#define WF_RX_DESCRIPTOR_G0_CH_FREQUENCY_ADDR                                  12
#define WF_RX_DESCRIPTOR_G0_CH_FREQUENCY_MASK                                  0x0000ff00 // 15- 8
#define WF_RX_DESCRIPTOR_G0_CH_FREQUENCY_SHIFT                                 8
#define WF_RX_DESCRIPTOR_G0_A1_TYPE_DW                                         3
#define WF_RX_DESCRIPTOR_G0_A1_TYPE_ADDR                                       12
#define WF_RX_DESCRIPTOR_G0_A1_TYPE_MASK                                       0x00030000 // 17-16
#define WF_RX_DESCRIPTOR_G0_A1_TYPE_SHIFT                                      16
#define WF_RX_DESCRIPTOR_G0_HTC_DW                                             3
#define WF_RX_DESCRIPTOR_G0_HTC_ADDR                                           12
#define WF_RX_DESCRIPTOR_G0_HTC_MASK                                           0x00040000 // 18-18
#define WF_RX_DESCRIPTOR_G0_HTC_SHIFT                                          18
#define WF_RX_DESCRIPTOR_G0_TCL_DW                                             3
#define WF_RX_DESCRIPTOR_G0_TCL_ADDR                                           12
#define WF_RX_DESCRIPTOR_G0_TCL_MASK                                           0x00080000 // 19-19
#define WF_RX_DESCRIPTOR_G0_TCL_SHIFT                                          19
#define WF_RX_DESCRIPTOR_G0_BBM_DW                                             3
#define WF_RX_DESCRIPTOR_G0_BBM_ADDR                                           12
#define WF_RX_DESCRIPTOR_G0_BBM_MASK                                           0x00100000 // 20-20
#define WF_RX_DESCRIPTOR_G0_BBM_SHIFT                                          20
#define WF_RX_DESCRIPTOR_G0_BU_DW                                              3
#define WF_RX_DESCRIPTOR_G0_BU_ADDR                                            12
#define WF_RX_DESCRIPTOR_G0_BU_MASK                                            0x00200000 // 21-21
#define WF_RX_DESCRIPTOR_G0_BU_SHIFT                                           21
#define WF_RX_DESCRIPTOR_G0_CO_ANT_DW                                          3
#define WF_RX_DESCRIPTOR_G0_CO_ANT_ADDR                                        12
#define WF_RX_DESCRIPTOR_G0_CO_ANT_MASK                                        0x00400000 // 22-22
#define WF_RX_DESCRIPTOR_G0_CO_ANT_SHIFT                                       22
#define WF_RX_DESCRIPTOR_G0_BF_CQI_DW                                          3
#define WF_RX_DESCRIPTOR_G0_BF_CQI_ADDR                                        12
#define WF_RX_DESCRIPTOR_G0_BF_CQI_MASK                                        0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_G0_BF_CQI_SHIFT                                       23
#define WF_RX_DESCRIPTOR_G0_FC_DW                                              3
#define WF_RX_DESCRIPTOR_G0_FC_ADDR                                            12
#define WF_RX_DESCRIPTOR_G0_FC_MASK                                            0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_G0_FC_SHIFT                                           24
#define WF_RX_DESCRIPTOR_G0_IP_DW                                              3
#define WF_RX_DESCRIPTOR_G0_IP_ADDR                                            12
#define WF_RX_DESCRIPTOR_G0_IP_MASK                                            0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_G0_IP_SHIFT                                           26
#define WF_RX_DESCRIPTOR_G0_UT_DW                                              3
#define WF_RX_DESCRIPTOR_G0_UT_ADDR                                            12
#define WF_RX_DESCRIPTOR_G0_UT_MASK                                            0x08000000 // 27-27
#define WF_RX_DESCRIPTOR_G0_UT_SHIFT                                           27
#define WF_RX_DESCRIPTOR_G0_SMESH_DW                                           3
#define WF_RX_DESCRIPTOR_G0_SMESH_ADDR                                         12
#define WF_RX_DESCRIPTOR_G0_SMESH_MASK                                         0x40000000 // 30-30
#define WF_RX_DESCRIPTOR_G0_SMESH_SHIFT                                        30
#define WF_RX_DESCRIPTOR_G0_VLAN_DW                                            3
#define WF_RX_DESCRIPTOR_G0_VLAN_ADDR                                          12
#define WF_RX_DESCRIPTOR_G0_VLAN_MASK                                          0x80000000 // 31-31
#define WF_RX_DESCRIPTOR_G0_VLAN_SHIFT                                         31
// DW4
#define WF_RX_DESCRIPTOR_G0_PF_DW                                              4
#define WF_RX_DESCRIPTOR_G0_PF_ADDR                                            16
#define WF_RX_DESCRIPTOR_G0_PF_MASK                                            0x00000003 //  1- 0
#define WF_RX_DESCRIPTOR_G0_PF_SHIFT                                           0
#define WF_RX_DESCRIPTOR_G0_MAC_DW                                             4
#define WF_RX_DESCRIPTOR_G0_MAC_ADDR                                           16
#define WF_RX_DESCRIPTOR_G0_MAC_MASK                                           0x00000004 //  2- 2
#define WF_RX_DESCRIPTOR_G0_MAC_SHIFT                                          2
#define WF_RX_DESCRIPTOR_G0_TID_DW                                             4
#define WF_RX_DESCRIPTOR_G0_TID_ADDR                                           16
#define WF_RX_DESCRIPTOR_G0_TID_MASK                                           0x00000078 //  6- 3
#define WF_RX_DESCRIPTOR_G0_TID_SHIFT                                          3
#define WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET_DW                               4
#define WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET_ADDR                             16
#define WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET_MASK                             0x00007f80 // 14- 7
#define WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET_SHIFT                            7
#define WF_RX_DESCRIPTOR_G0_PSE_FID_DW                                         4
#define WF_RX_DESCRIPTOR_G0_PSE_FID_ADDR                                       16
#define WF_RX_DESCRIPTOR_G0_PSE_FID_MASK                                       0x1fff0000 // 28-16
#define WF_RX_DESCRIPTOR_G0_PSE_FID_SHIFT                                      16
// DW5
// DW6
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0__DW                                6
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0__ADDR                              24
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0__MASK                              0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0__SHIFT                             0
// DW7
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32__DW                               7
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32__ADDR                             28
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32__MASK                             0x00000003 //  1- 0
#define WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32__SHIFT                            0
#define WF_RX_DESCRIPTOR_G0_DP_DW                                              7
#define WF_RX_DESCRIPTOR_G0_DP_ADDR                                            28
#define WF_RX_DESCRIPTOR_G0_DP_MASK                                            0x00080000 // 19-19
#define WF_RX_DESCRIPTOR_G0_DP_SHIFT                                           19
#define WF_RX_DESCRIPTOR_G0_CLS_DW                                             7
#define WF_RX_DESCRIPTOR_G0_CLS_ADDR                                           28
#define WF_RX_DESCRIPTOR_G0_CLS_MASK                                           0x00100000 // 20-20
#define WF_RX_DESCRIPTOR_G0_CLS_SHIFT                                          20
#define WF_RX_DESCRIPTOR_G0_OFLD_DW                                            7
#define WF_RX_DESCRIPTOR_G0_OFLD_ADDR                                          28
#define WF_RX_DESCRIPTOR_G0_OFLD_MASK                                          0x00600000 // 22-21
#define WF_RX_DESCRIPTOR_G0_OFLD_SHIFT                                         21
#define WF_RX_DESCRIPTOR_G0_MGC_DW                                             7
#define WF_RX_DESCRIPTOR_G0_MGC_ADDR                                           28
#define WF_RX_DESCRIPTOR_G0_MGC_MASK                                           0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_G0_MGC_SHIFT                                          23
#define WF_RX_DESCRIPTOR_G0_WOL_DW                                             7
#define WF_RX_DESCRIPTOR_G0_WOL_ADDR                                           28
#define WF_RX_DESCRIPTOR_G0_WOL_MASK                                           0x1f000000 // 28-24
#define WF_RX_DESCRIPTOR_G0_WOL_SHIFT                                          24
#define WF_RX_DESCRIPTOR_G0_PF_MODE_DW                                         7
#define WF_RX_DESCRIPTOR_G0_PF_MODE_ADDR                                       28
#define WF_RX_DESCRIPTOR_G0_PF_MODE_MASK                                       0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_G0_PF_MODE_SHIFT                                      29
#define WF_RX_DESCRIPTOR_G0_PF_STS_DW                                          7
#define WF_RX_DESCRIPTOR_G0_PF_STS_ADDR                                        28
#define WF_RX_DESCRIPTOR_G0_PF_STS_MASK                                        0xc0000000 // 31-30
#define WF_RX_DESCRIPTOR_G0_PF_STS_SHIFT                                       30

// DW0
#define HAL_RX_STATUS_GET_RX_BYTE_COUNT(reg32)                                                              READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT)
#define HAL_RX_STATUS_GET_COSR(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_COSR)
#define HAL_RX_STATUS_GET_PACKET_TYPE(reg32)                                                                READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PACKET_TYPE)
// DW1
#define HAL_RX_STATUS_GET_MLD_ID(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MLD_ID)
#define HAL_RX_STATUS_GET_GROUP_VLD(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_GROUP_VLD)
#define HAL_RX_STATUS_GET_KID(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_KID)
#define HAL_RX_STATUS_GET_CM(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CM)
#define HAL_RX_STATUS_GET_CLM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLM)
#define HAL_RX_STATUS_GET_I(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_I)
#define HAL_RX_STATUS_GET_T(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_T)
#define HAL_RX_STATUS_GET_BN(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BN)
#define HAL_RX_STATUS_GET_BIPN_FAIL(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BIPN_FAIL)
// DW2
#define HAL_RX_STATUS_GET_BSSID(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BSSID)
#define HAL_RX_STATUS_GET_H(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_H)
#define HAL_RX_STATUS_GET_HEADER_LENGTH(reg32)                                                              READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HEADER_LENGTH)
#define HAL_RX_STATUS_GET_HO(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HO)
#define HAL_RX_STATUS_GET_SEC_MODE(reg32)                                                                   READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SEC_MODE)
#define HAL_RX_STATUS_GET_MUBAR(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MUBAR)
#define HAL_RX_STATUS_GET_SWBIT(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SWBIT)
#define HAL_RX_STATUS_GET_DAF(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DAF)
#define HAL_RX_STATUS_GET_EL(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_EL)
#define HAL_RX_STATUS_GET_HTF(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTF)
#define HAL_RX_STATUS_GET_INTF(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_INTF)
#define HAL_RX_STATUS_GET_FRAG(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FRAG)
#define HAL_RX_STATUS_GET_NUL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NUL)
#define HAL_RX_STATUS_GET_NDATA(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NDATA)
#define HAL_RX_STATUS_GET_NAMP(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NAMP)
#define HAL_RX_STATUS_GET_BF_RPT(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_RPT)
// DW3
#define HAL_RX_STATUS_GET_RXV_SN(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RXV_SN)
#define HAL_RX_STATUS_GET_CH_FREQUENCY(reg32)                                                               READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CH_FREQUENCY)
#define HAL_RX_STATUS_GET_A1_TYPE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_A1_TYPE)
#define HAL_RX_STATUS_GET_HTC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTC)
#define HAL_RX_STATUS_GET_TCL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TCL)
#define HAL_RX_STATUS_GET_BBM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BBM)
#define HAL_RX_STATUS_GET_BU(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BU)
#define HAL_RX_STATUS_GET_CO_ANT(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CO_ANT)
#define HAL_RX_STATUS_GET_BF_CQI(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_CQI)
#define HAL_RX_STATUS_GET_FC(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FC)
#define HAL_RX_STATUS_GET_IP(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_IP)
#define HAL_RX_STATUS_GET_UT(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_UT)
#define HAL_RX_STATUS_GET_SMESH(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SMESH)
#define HAL_RX_STATUS_GET_VLAN(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_VLAN)
// DW4
#define HAL_RX_STATUS_GET_PF(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF)
#define HAL_RX_STATUS_GET_MAC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MAC)
#define HAL_RX_STATUS_GET_TID(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TID)
#define HAL_RX_STATUS_GET_ETHER_TYPE_OFFSET(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET)
#define HAL_RX_STATUS_GET_PSE_FID(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PSE_FID)
// DW5
// DW6
#define HAL_RX_STATUS_GET_CLS_BITMAP_31_0_(reg32)                                                           READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0_)
// DW7
#define HAL_RX_STATUS_GET_CLS_BITMAP_33_32_(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32_)
#define HAL_RX_STATUS_GET_DP(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DP)
#define HAL_RX_STATUS_GET_CLS(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS)
#define HAL_RX_STATUS_GET_OFLD(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_OFLD)
#define HAL_RX_STATUS_GET_MGC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MGC)
#define HAL_RX_STATUS_GET_WOL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_WOL)
#define HAL_RX_STATUS_GET_PF_MODE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_MODE)
#define HAL_RX_STATUS_GET_PF_STS(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_STS)

// DW0
#define HAL_RX_STATUS_SET_RX_BYTE_COUNT(reg32, val32)                                                       WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT, val32)
#define HAL_RX_STATUS_SET_COSR(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_COSR, val32)
#define HAL_RX_STATUS_SET_PACKET_TYPE(reg32, val32)                                                         WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PACKET_TYPE, val32)
// DW1
#define HAL_RX_STATUS_SET_MLD_ID(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MLD_ID, val32)
#define HAL_RX_STATUS_SET_GROUP_VLD(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_GROUP_VLD, val32)
#define HAL_RX_STATUS_SET_KID(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_KID, val32)
#define HAL_RX_STATUS_SET_CM(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CM, val32)
#define HAL_RX_STATUS_SET_CLM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLM, val32)
#define HAL_RX_STATUS_SET_I(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_I, val32)
#define HAL_RX_STATUS_SET_T(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_T, val32)
#define HAL_RX_STATUS_SET_BN(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BN, val32)
#define HAL_RX_STATUS_SET_BIPN_FAIL(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BIPN_FAIL, val32)
// DW2
#define HAL_RX_STATUS_SET_BSSID(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BSSID, val32)
#define HAL_RX_STATUS_SET_H(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_H, val32)
#define HAL_RX_STATUS_SET_HEADER_LENGTH(reg32, val32)                                                       WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HEADER_LENGTH, val32)
#define HAL_RX_STATUS_SET_HO(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HO, val32)
#define HAL_RX_STATUS_SET_SEC_MODE(reg32, val32)                                                            WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SEC_MODE, val32)
#define HAL_RX_STATUS_SET_MUBAR(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MUBAR, val32)
#define HAL_RX_STATUS_SET_SWBIT(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SWBIT, val32)
#define HAL_RX_STATUS_SET_DAF(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DAF, val32)
#define HAL_RX_STATUS_SET_EL(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_EL, val32)
#define HAL_RX_STATUS_SET_HTF(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTF, val32)
#define HAL_RX_STATUS_SET_INTF(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_INTF, val32)
#define HAL_RX_STATUS_SET_FRAG(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FRAG, val32)
#define HAL_RX_STATUS_SET_NUL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NUL, val32)
#define HAL_RX_STATUS_SET_NDATA(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NDATA, val32)
#define HAL_RX_STATUS_SET_NAMP(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NAMP, val32)
#define HAL_RX_STATUS_SET_BF_RPT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_RPT, val32)
// DW3
#define HAL_RX_STATUS_SET_RXV_SN(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RXV_SN, val32)
#define HAL_RX_STATUS_SET_CH_FREQUENCY(reg32, val32)                                                        WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CH_FREQUENCY, val32)
#define HAL_RX_STATUS_SET_A1_TYPE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_A1_TYPE, val32)
#define HAL_RX_STATUS_SET_HTC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTC, val32)
#define HAL_RX_STATUS_SET_TCL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TCL, val32)
#define HAL_RX_STATUS_SET_BBM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BBM, val32)
#define HAL_RX_STATUS_SET_BU(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BU, val32)
#define HAL_RX_STATUS_SET_CO_ANT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CO_ANT, val32)
#define HAL_RX_STATUS_SET_BF_CQI(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_CQI, val32)
#define HAL_RX_STATUS_SET_FC(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FC, val32)
#define HAL_RX_STATUS_SET_IP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_IP, val32)
#define HAL_RX_STATUS_SET_UT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_UT, val32)
#define HAL_RX_STATUS_SET_SMESH(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SMESH, val32)
#define HAL_RX_STATUS_SET_VLAN(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_VLAN, val32)
// DW4
#define HAL_RX_STATUS_SET_PF(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF, val32)
#define HAL_RX_STATUS_SET_MAC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MAC, val32)
#define HAL_RX_STATUS_SET_TID(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TID, val32)
#define HAL_RX_STATUS_SET_ETHER_TYPE_OFFSET(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET, val32)
#define HAL_RX_STATUS_SET_PSE_FID(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PSE_FID, val32)
// DW5
// DW6
#define HAL_RX_STATUS_SET_CLS_BITMAP_31_0_(reg32, val32)                                                    WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0_, val32)
// DW7
#define HAL_RX_STATUS_SET_CLS_BITMAP_33_32_(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32_, val32)
#define HAL_RX_STATUS_SET_DP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DP, val32)
#define HAL_RX_STATUS_SET_CLS(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS, val32)
#define HAL_RX_STATUS_SET_OFLD(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_OFLD, val32)
#define HAL_RX_STATUS_SET_MGC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MGC, val32)
#define HAL_RX_STATUS_SET_WOL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_WOL, val32)
#define HAL_RX_STATUS_SET_PF_MODE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_MODE, val32)
#define HAL_RX_STATUS_SET_PF_STS(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_STS, val32)

// DW0
#define HAL_RX_STATUS_CLR_RX_BYTE_COUNT(reg32)                                                              CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RX_BYTE_COUNT)
#define HAL_RX_STATUS_CLR_COSR(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_COSR)
#define HAL_RX_STATUS_CLR_PACKET_TYPE(reg32)                                                                CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PACKET_TYPE)
// DW1
#define HAL_RX_STATUS_CLR_MLD_ID(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MLD_ID)
#define HAL_RX_STATUS_CLR_GROUP_VLD(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_GROUP_VLD)
#define HAL_RX_STATUS_CLR_KID(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_KID)
#define HAL_RX_STATUS_CLR_CM(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CM)
#define HAL_RX_STATUS_CLR_CLM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLM)
#define HAL_RX_STATUS_CLR_I(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_I)
#define HAL_RX_STATUS_CLR_T(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_T)
#define HAL_RX_STATUS_CLR_BN(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BN)
#define HAL_RX_STATUS_CLR_BIPN_FAIL(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BIPN_FAIL)
// DW2
#define HAL_RX_STATUS_CLR_BSSID(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BSSID)
#define HAL_RX_STATUS_CLR_H(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_H)
#define HAL_RX_STATUS_CLR_HEADER_LENGTH(reg32)                                                              CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HEADER_LENGTH)
#define HAL_RX_STATUS_CLR_HO(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HO)
#define HAL_RX_STATUS_CLR_SEC_MODE(reg32)                                                                   CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SEC_MODE)
#define HAL_RX_STATUS_CLR_MUBAR(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MUBAR)
#define HAL_RX_STATUS_CLR_SWBIT(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SWBIT)
#define HAL_RX_STATUS_CLR_DAF(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DAF)
#define HAL_RX_STATUS_CLR_EL(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_EL)
#define HAL_RX_STATUS_CLR_HTF(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTF)
#define HAL_RX_STATUS_CLR_INTF(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_INTF)
#define HAL_RX_STATUS_CLR_FRAG(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FRAG)
#define HAL_RX_STATUS_CLR_NUL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NUL)
#define HAL_RX_STATUS_CLR_NDATA(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NDATA)
#define HAL_RX_STATUS_CLR_NAMP(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_NAMP)
#define HAL_RX_STATUS_CLR_BF_RPT(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_RPT)
// DW3
#define HAL_RX_STATUS_CLR_RXV_SN(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_RXV_SN)
#define HAL_RX_STATUS_CLR_CH_FREQUENCY(reg32)                                                               CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CH_FREQUENCY)
#define HAL_RX_STATUS_CLR_A1_TYPE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_A1_TYPE)
#define HAL_RX_STATUS_CLR_HTC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_HTC)
#define HAL_RX_STATUS_CLR_TCL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TCL)
#define HAL_RX_STATUS_CLR_BBM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BBM)
#define HAL_RX_STATUS_CLR_BU(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BU)
#define HAL_RX_STATUS_CLR_CO_ANT(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CO_ANT)
#define HAL_RX_STATUS_CLR_BF_CQI(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_BF_CQI)
#define HAL_RX_STATUS_CLR_FC(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_FC)
#define HAL_RX_STATUS_CLR_IP(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_IP)
#define HAL_RX_STATUS_CLR_UT(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_UT)
#define HAL_RX_STATUS_CLR_SMESH(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_SMESH)
#define HAL_RX_STATUS_CLR_VLAN(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_VLAN)
// DW4
#define HAL_RX_STATUS_CLR_PF(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF)
#define HAL_RX_STATUS_CLR_MAC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MAC)
#define HAL_RX_STATUS_CLR_TID(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_TID)
#define HAL_RX_STATUS_CLR_ETHER_TYPE_OFFSET(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_ETHER_TYPE_OFFSET)
#define HAL_RX_STATUS_CLR_PSE_FID(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PSE_FID)
// DW5
// DW6
#define HAL_RX_STATUS_CLR_CLS_BITMAP_31_0_(reg32)                                                           CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_31_0_)
// DW7
#define HAL_RX_STATUS_CLR_CLS_BITMAP_33_32_(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS_BITMAP_33_32_)
#define HAL_RX_STATUS_CLR_DP(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_DP)
#define HAL_RX_STATUS_CLR_CLS(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_CLS)
#define HAL_RX_STATUS_CLR_OFLD(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_OFLD)
#define HAL_RX_STATUS_CLR_MGC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_MGC)
#define HAL_RX_STATUS_CLR_WOL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_WOL)
#define HAL_RX_STATUS_CLR_PF_MODE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_MODE)
#define HAL_RX_STATUS_CLR_PF_STS(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_G0_PF_STS)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_G0_REGS_H__
