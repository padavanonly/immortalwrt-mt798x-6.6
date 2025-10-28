// generate by structure_parser.py at 2022-01-06 13:35:06
#ifndef __WF_RX_DESCRIPTOR_REGS_H__
#define __WF_RX_DESCRIPTOR_REGS_H__
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
// ----------------- WF_RX_DESCRIPTOR Bit Field Definitions -----------------

#define PACKING
typedef uint32_t UINT32;
typedef unsigned int FIELD;

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
} REG_WF_RX_DESCRIPTOR_DW00, *PREG_WF_RX_DESCRIPTOR_DW00; // DW0

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
} REG_WF_RX_DESCRIPTOR_DW01, *PREG_WF_RX_DESCRIPTOR_DW01; // DW1

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
} REG_WF_RX_DESCRIPTOR_DW02, *PREG_WF_RX_DESCRIPTOR_DW02; // DW2

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
} REG_WF_RX_DESCRIPTOR_DW03, *PREG_WF_RX_DESCRIPTOR_DW03; // DW3

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
} REG_WF_RX_DESCRIPTOR_DW04, *PREG_WF_RX_DESCRIPTOR_DW04; // DW4

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_DW05, *PREG_WF_RX_DESCRIPTOR_DW05; // DW5

typedef PACKING union
{
    PACKING struct
    {
        FIELD cls_bitmap_31_0_          : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_PF_DW06, *PREG_WF_RX_DESCRIPTOR_PF_DW06; // DW6

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
} REG_WF_RX_DESCRIPTOR_PF_DW07, *PREG_WF_RX_DESCRIPTOR_PF_DW07; // DW7

typedef PACKING union
{
    PACKING struct
    {
        FIELD frame_control_field       : 16; // 15- 0
        FIELD peer_mld_address_15_0_    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP4_DW08, *PREG_WF_RX_DESCRIPTOR_GROUP4_DW08; // DW8

typedef PACKING union
{
    PACKING struct
    {
        FIELD peer_mld_address_47_16_   : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP4_DW09, *PREG_WF_RX_DESCRIPTOR_GROUP4_DW09; // DW9

typedef PACKING union
{
    PACKING struct
    {
        FIELD fragment_number           :  4; //  3- 0
        FIELD sequence_number           : 12; // 15- 4
        FIELD qos_control_field         : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP4_DW10, *PREG_WF_RX_DESCRIPTOR_GROUP4_DW10; // DW10

typedef PACKING union
{
    PACKING struct
    {
        FIELD ht_control_field          : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP4_DW11, *PREG_WF_RX_DESCRIPTOR_GROUP4_DW11; // DW11

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_31_0_                  : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP1_DW12, *PREG_WF_RX_DESCRIPTOR_GROUP1_DW12; // DW12

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_63_32_                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP1_DW13, *PREG_WF_RX_DESCRIPTOR_GROUP1_DW13; // DW13

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_95_64_                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP1_DW14, *PREG_WF_RX_DESCRIPTOR_GROUP1_DW14; // DW14

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_127_96_                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP1_DW15, *PREG_WF_RX_DESCRIPTOR_GROUP1_DW15; // DW15

typedef PACKING union
{
    PACKING struct
    {
        FIELD timestamp                 : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP2_DW16, *PREG_WF_RX_DESCRIPTOR_GROUP2_DW16; // DW16

typedef PACKING union
{
    PACKING struct
    {
        FIELD crc                       : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP2_DW17, *PREG_WF_RX_DESCRIPTOR_GROUP2_DW17; // DW17

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP2_DW18, *PREG_WF_RX_DESCRIPTOR_GROUP2_DW18; // DW18

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP2_DW19, *PREG_WF_RX_DESCRIPTOR_GROUP2_DW19; // DW19

typedef PACKING union
{
    PACKING struct
    {
        FIELD p_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP3_DW20, *PREG_WF_RX_DESCRIPTOR_GROUP3_DW20; // DW20

typedef PACKING union
{
    PACKING struct
    {
        FIELD p_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP3_DW21, *PREG_WF_RX_DESCRIPTOR_GROUP3_DW21; // DW21

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
} REG_WF_RX_DESCRIPTOR_GROUP3_DW22, *PREG_WF_RX_DESCRIPTOR_GROUP3_DW22; // DW22

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcpi                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP3_DW23, *PREG_WF_RX_DESCRIPTOR_GROUP3_DW23; // DW23

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW24, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW24; // DW24

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW25, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW25; // DW25

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW26, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW26; // DW26

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW27, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW27; // DW27

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW28, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW28; // DW28

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW29, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW29; // DW29

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW30, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW30; // DW30

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW31, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW31; // DW31

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW32, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW32; // DW32

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW33, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW33; // DW33

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW34, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW34; // DW34

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW35, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW35; // DW35

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW36, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW36; // DW36

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW37, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW37; // DW37

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW38, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW38; // DW38

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW39, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW39; // DW39

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW40, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW40; // DW40

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW41, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW41; // DW41

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW42, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW42; // DW42

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW43, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW43; // DW43

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW44, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW44; // DW44

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rxv                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW45, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW45; // DW45

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW46, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW46; // DW46

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_RX_DESCRIPTOR_GROUP5_DW47, *PREG_WF_RX_DESCRIPTOR_GROUP5_DW47; // DW47

// ----------------- WF_RX_DESCRIPTOR Grouping Definitions  -----------------
// ----------------- WF_RX_DESCRIPTOR Register Definition   -----------------
typedef volatile PACKING struct
{
    REG_WF_RX_DESCRIPTOR_DW00                    WF_RX_DESCRIPTOR_DW00                   ; // 0x0000
    REG_WF_RX_DESCRIPTOR_DW01                    WF_RX_DESCRIPTOR_DW01                   ; // 0x0004
    REG_WF_RX_DESCRIPTOR_DW02                    WF_RX_DESCRIPTOR_DW02                   ; // 0x0008
    REG_WF_RX_DESCRIPTOR_DW03                    WF_RX_DESCRIPTOR_DW03                   ; // 0x000c
    REG_WF_RX_DESCRIPTOR_DW04                    WF_RX_DESCRIPTOR_DW04                   ; // 0x0010
    REG_WF_RX_DESCRIPTOR_DW05                    WF_RX_DESCRIPTOR_DW05                   ; // 0x0014
    REG_WF_RX_DESCRIPTOR_PF_DW06                 WF_RX_DESCRIPTOR_PF_DW06                ; // 0x0018
    REG_WF_RX_DESCRIPTOR_PF_DW07                 WF_RX_DESCRIPTOR_PF_DW07                ; // 0x001c
    REG_WF_RX_DESCRIPTOR_GROUP4_DW08             WF_RX_DESCRIPTOR_GROUP4_DW08            ; // 0x0020
    REG_WF_RX_DESCRIPTOR_GROUP4_DW09             WF_RX_DESCRIPTOR_GROUP4_DW09            ; // 0x0024
    REG_WF_RX_DESCRIPTOR_GROUP4_DW10             WF_RX_DESCRIPTOR_GROUP4_DW10            ; // 0x0028
    REG_WF_RX_DESCRIPTOR_GROUP4_DW11             WF_RX_DESCRIPTOR_GROUP4_DW11            ; // 0x002c
    REG_WF_RX_DESCRIPTOR_GROUP1_DW12             WF_RX_DESCRIPTOR_GROUP1_DW12            ; // 0x0030
    REG_WF_RX_DESCRIPTOR_GROUP1_DW13             WF_RX_DESCRIPTOR_GROUP1_DW13            ; // 0x0034
    REG_WF_RX_DESCRIPTOR_GROUP1_DW14             WF_RX_DESCRIPTOR_GROUP1_DW14            ; // 0x0038
    REG_WF_RX_DESCRIPTOR_GROUP1_DW15             WF_RX_DESCRIPTOR_GROUP1_DW15            ; // 0x003c
    REG_WF_RX_DESCRIPTOR_GROUP2_DW16             WF_RX_DESCRIPTOR_GROUP2_DW16            ; // 0x0040
    REG_WF_RX_DESCRIPTOR_GROUP2_DW17             WF_RX_DESCRIPTOR_GROUP2_DW17            ; // 0x0044
    REG_WF_RX_DESCRIPTOR_GROUP2_DW18             WF_RX_DESCRIPTOR_GROUP2_DW18            ; // 0x0048
    REG_WF_RX_DESCRIPTOR_GROUP2_DW19             WF_RX_DESCRIPTOR_GROUP2_DW19            ; // 0x004c
    REG_WF_RX_DESCRIPTOR_GROUP3_DW20             WF_RX_DESCRIPTOR_GROUP3_DW20            ; // 0x0050
    REG_WF_RX_DESCRIPTOR_GROUP3_DW21             WF_RX_DESCRIPTOR_GROUP3_DW21            ; // 0x0054
    REG_WF_RX_DESCRIPTOR_GROUP3_DW22             WF_RX_DESCRIPTOR_GROUP3_DW22            ; // 0x0058
    REG_WF_RX_DESCRIPTOR_GROUP3_DW23             WF_RX_DESCRIPTOR_GROUP3_DW23            ; // 0x005c
    REG_WF_RX_DESCRIPTOR_GROUP5_DW24             WF_RX_DESCRIPTOR_GROUP5_DW24            ; // 0x0060
    REG_WF_RX_DESCRIPTOR_GROUP5_DW25             WF_RX_DESCRIPTOR_GROUP5_DW25            ; // 0x0064
    REG_WF_RX_DESCRIPTOR_GROUP5_DW26             WF_RX_DESCRIPTOR_GROUP5_DW26            ; // 0x0068
    REG_WF_RX_DESCRIPTOR_GROUP5_DW27             WF_RX_DESCRIPTOR_GROUP5_DW27            ; // 0x006c
    REG_WF_RX_DESCRIPTOR_GROUP5_DW28             WF_RX_DESCRIPTOR_GROUP5_DW28            ; // 0x0070
    REG_WF_RX_DESCRIPTOR_GROUP5_DW29             WF_RX_DESCRIPTOR_GROUP5_DW29            ; // 0x0074
    REG_WF_RX_DESCRIPTOR_GROUP5_DW30             WF_RX_DESCRIPTOR_GROUP5_DW30            ; // 0x0078
    REG_WF_RX_DESCRIPTOR_GROUP5_DW31             WF_RX_DESCRIPTOR_GROUP5_DW31            ; // 0x007c
    REG_WF_RX_DESCRIPTOR_GROUP5_DW32             WF_RX_DESCRIPTOR_GROUP5_DW32            ; // 0x0080
    REG_WF_RX_DESCRIPTOR_GROUP5_DW33             WF_RX_DESCRIPTOR_GROUP5_DW33            ; // 0x0084
    REG_WF_RX_DESCRIPTOR_GROUP5_DW34             WF_RX_DESCRIPTOR_GROUP5_DW34            ; // 0x0088
    REG_WF_RX_DESCRIPTOR_GROUP5_DW35             WF_RX_DESCRIPTOR_GROUP5_DW35            ; // 0x008c
    REG_WF_RX_DESCRIPTOR_GROUP5_DW36             WF_RX_DESCRIPTOR_GROUP5_DW36            ; // 0x0090
    REG_WF_RX_DESCRIPTOR_GROUP5_DW37             WF_RX_DESCRIPTOR_GROUP5_DW37            ; // 0x0094
    REG_WF_RX_DESCRIPTOR_GROUP5_DW38             WF_RX_DESCRIPTOR_GROUP5_DW38            ; // 0x0098
    REG_WF_RX_DESCRIPTOR_GROUP5_DW39             WF_RX_DESCRIPTOR_GROUP5_DW39            ; // 0x009c
    REG_WF_RX_DESCRIPTOR_GROUP5_DW40             WF_RX_DESCRIPTOR_GROUP5_DW40            ; // 0x00a0
    REG_WF_RX_DESCRIPTOR_GROUP5_DW41             WF_RX_DESCRIPTOR_GROUP5_DW41            ; // 0x00a4
    REG_WF_RX_DESCRIPTOR_GROUP5_DW42             WF_RX_DESCRIPTOR_GROUP5_DW42            ; // 0x00a8
    REG_WF_RX_DESCRIPTOR_GROUP5_DW43             WF_RX_DESCRIPTOR_GROUP5_DW43            ; // 0x00ac
    REG_WF_RX_DESCRIPTOR_GROUP5_DW44             WF_RX_DESCRIPTOR_GROUP5_DW44            ; // 0x00b0
    REG_WF_RX_DESCRIPTOR_GROUP5_DW45             WF_RX_DESCRIPTOR_GROUP5_DW45            ; // 0x00b4
    REG_WF_RX_DESCRIPTOR_GROUP5_DW46             WF_RX_DESCRIPTOR_GROUP5_DW46            ; // 0x00b8
    REG_WF_RX_DESCRIPTOR_GROUP5_DW47             WF_RX_DESCRIPTOR_GROUP5_DW47            ; // 0x00bc
}WF_RX_DESCRIPTOR_REGS, *PWF_RX_DESCRIPTOR_REGS;
// ----------------- WF_RX_DESCRIPTOR Enum Definitions      -----------------
// ----------------- WF_RX_DESCRIPTOR C Macro Definitions   -----------------
extern PWF_RX_DESCRIPTOR_REGS g_WF_RX_DESCRIPTOR_BASE;

#define WF_RX_DESCRIPTOR_BASE (g_WF_RX_DESCRIPTOR_BASE)
#define WF_RX_DESCRIPTOR_DW00                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW00                   ) // 0x0000
#define WF_RX_DESCRIPTOR_DW01                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW01                   ) // 0x0004
#define WF_RX_DESCRIPTOR_DW02                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW02                   ) // 0x0008
#define WF_RX_DESCRIPTOR_DW03                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW03                   ) // 0x000c
#define WF_RX_DESCRIPTOR_DW04                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW04                   ) // 0x0010
#define WF_RX_DESCRIPTOR_DW05                    INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_DW05                   ) // 0x0014
#define WF_RX_DESCRIPTOR_PF_DW06                 INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_PF_DW06                ) // 0x0018
#define WF_RX_DESCRIPTOR_PF_DW07                 INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_PF_DW07                ) // 0x001c
#define WF_RX_DESCRIPTOR_GROUP4_DW08             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP4_DW08            ) // 0x0020
#define WF_RX_DESCRIPTOR_GROUP4_DW09             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP4_DW09            ) // 0x0024
#define WF_RX_DESCRIPTOR_GROUP4_DW10             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP4_DW10            ) // 0x0028
#define WF_RX_DESCRIPTOR_GROUP4_DW11             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP4_DW11            ) // 0x002c
#define WF_RX_DESCRIPTOR_GROUP1_DW12             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP1_DW12            ) // 0x0030
#define WF_RX_DESCRIPTOR_GROUP1_DW13             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP1_DW13            ) // 0x0034
#define WF_RX_DESCRIPTOR_GROUP1_DW14             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP1_DW14            ) // 0x0038
#define WF_RX_DESCRIPTOR_GROUP1_DW15             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP1_DW15            ) // 0x003c
#define WF_RX_DESCRIPTOR_GROUP2_DW16             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP2_DW16            ) // 0x0040
#define WF_RX_DESCRIPTOR_GROUP2_DW17             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP2_DW17            ) // 0x0044
#define WF_RX_DESCRIPTOR_GROUP2_DW18             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP2_DW18            ) // 0x0048
#define WF_RX_DESCRIPTOR_GROUP2_DW19             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP2_DW19            ) // 0x004c
#define WF_RX_DESCRIPTOR_GROUP3_DW20             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP3_DW20            ) // 0x0050
#define WF_RX_DESCRIPTOR_GROUP3_DW21             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP3_DW21            ) // 0x0054
#define WF_RX_DESCRIPTOR_GROUP3_DW22             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP3_DW22            ) // 0x0058
#define WF_RX_DESCRIPTOR_GROUP3_DW23             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP3_DW23            ) // 0x005c
#define WF_RX_DESCRIPTOR_GROUP5_DW24             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW24            ) // 0x0060
#define WF_RX_DESCRIPTOR_GROUP5_DW25             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW25            ) // 0x0064
#define WF_RX_DESCRIPTOR_GROUP5_DW26             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW26            ) // 0x0068
#define WF_RX_DESCRIPTOR_GROUP5_DW27             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW27            ) // 0x006c
#define WF_RX_DESCRIPTOR_GROUP5_DW28             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW28            ) // 0x0070
#define WF_RX_DESCRIPTOR_GROUP5_DW29             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW29            ) // 0x0074
#define WF_RX_DESCRIPTOR_GROUP5_DW30             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW30            ) // 0x0078
#define WF_RX_DESCRIPTOR_GROUP5_DW31             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW31            ) // 0x007c
#define WF_RX_DESCRIPTOR_GROUP5_DW32             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW32            ) // 0x0080
#define WF_RX_DESCRIPTOR_GROUP5_DW33             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW33            ) // 0x0084
#define WF_RX_DESCRIPTOR_GROUP5_DW34             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW34            ) // 0x0088
#define WF_RX_DESCRIPTOR_GROUP5_DW35             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW35            ) // 0x008c
#define WF_RX_DESCRIPTOR_GROUP5_DW36             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW36            ) // 0x0090
#define WF_RX_DESCRIPTOR_GROUP5_DW37             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW37            ) // 0x0094
#define WF_RX_DESCRIPTOR_GROUP5_DW38             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW38            ) // 0x0098
#define WF_RX_DESCRIPTOR_GROUP5_DW39             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW39            ) // 0x009c
#define WF_RX_DESCRIPTOR_GROUP5_DW40             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW40            ) // 0x00a0
#define WF_RX_DESCRIPTOR_GROUP5_DW41             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW41            ) // 0x00a4
#define WF_RX_DESCRIPTOR_GROUP5_DW42             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW42            ) // 0x00a8
#define WF_RX_DESCRIPTOR_GROUP5_DW43             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW43            ) // 0x00ac
#define WF_RX_DESCRIPTOR_GROUP5_DW44             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW44            ) // 0x00b0
#define WF_RX_DESCRIPTOR_GROUP5_DW45             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW45            ) // 0x00b4
#define WF_RX_DESCRIPTOR_GROUP5_DW46             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW46            ) // 0x00b8
#define WF_RX_DESCRIPTOR_GROUP5_DW47             INREG32(&WF_RX_DESCRIPTOR_BASE->WF_RX_DESCRIPTOR_GROUP5_DW47            ) // 0x00bc

#endif // REG_BASE_C_MODULE

// DW0
#define WF_RX_DESCRIPTOR_RX_BYTE_COUNT_DW                                   0 
#define WF_RX_DESCRIPTOR_RX_BYTE_COUNT_ADDR                                 0 
#define WF_RX_DESCRIPTOR_RX_BYTE_COUNT_MASK                                 0x0000ffff // 15- 0
#define WF_RX_DESCRIPTOR_RX_BYTE_COUNT_SHIFT                                0 
#define WF_RX_DESCRIPTOR_COSR_DW                                            0 
#define WF_RX_DESCRIPTOR_COSR_ADDR                                          0 
#define WF_RX_DESCRIPTOR_COSR_MASK                                          0x00010000 // 16-16
#define WF_RX_DESCRIPTOR_COSR_SHIFT                                         16
#define WF_RX_DESCRIPTOR_PACKET_TYPE_DW                                     0 
#define WF_RX_DESCRIPTOR_PACKET_TYPE_ADDR                                   0 
#define WF_RX_DESCRIPTOR_PACKET_TYPE_MASK                                   0xf8000000 // 31-27
#define WF_RX_DESCRIPTOR_PACKET_TYPE_SHIFT                                  27
// DW1
#define WF_RX_DESCRIPTOR_MLD_ID_DW                                          1 
#define WF_RX_DESCRIPTOR_MLD_ID_ADDR                                        4 
#define WF_RX_DESCRIPTOR_MLD_ID_MASK                                        0x00000fff // 11- 0
#define WF_RX_DESCRIPTOR_MLD_ID_SHIFT                                       0 
#define WF_RX_DESCRIPTOR_GROUP_VLD_DW                                       1 
#define WF_RX_DESCRIPTOR_GROUP_VLD_ADDR                                     4 
#define WF_RX_DESCRIPTOR_GROUP_VLD_MASK                                     0x001f0000 // 20-16
#define WF_RX_DESCRIPTOR_GROUP_VLD_SHIFT                                    16
#define WF_RX_DESCRIPTOR_KID_DW                                             1 
#define WF_RX_DESCRIPTOR_KID_ADDR                                           4 
#define WF_RX_DESCRIPTOR_KID_MASK                                           0x00600000 // 22-21
#define WF_RX_DESCRIPTOR_KID_SHIFT                                          21
#define WF_RX_DESCRIPTOR_CM_DW                                              1 
#define WF_RX_DESCRIPTOR_CM_ADDR                                            4 
#define WF_RX_DESCRIPTOR_CM_MASK                                            0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_CM_SHIFT                                           23
#define WF_RX_DESCRIPTOR_CLM_DW                                             1 
#define WF_RX_DESCRIPTOR_CLM_ADDR                                           4 
#define WF_RX_DESCRIPTOR_CLM_MASK                                           0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_CLM_SHIFT                                          24
#define WF_RX_DESCRIPTOR_I_DW                                               1 
#define WF_RX_DESCRIPTOR_I_ADDR                                             4 
#define WF_RX_DESCRIPTOR_I_MASK                                             0x02000000 // 25-25
#define WF_RX_DESCRIPTOR_I_SHIFT                                            25
#define WF_RX_DESCRIPTOR_T_DW                                               1 
#define WF_RX_DESCRIPTOR_T_ADDR                                             4 
#define WF_RX_DESCRIPTOR_T_MASK                                             0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_T_SHIFT                                            26
#define WF_RX_DESCRIPTOR_BN_DW                                              1 
#define WF_RX_DESCRIPTOR_BN_ADDR                                            4 
#define WF_RX_DESCRIPTOR_BN_MASK                                            0x18000000 // 28-27
#define WF_RX_DESCRIPTOR_BN_SHIFT                                           27
#define WF_RX_DESCRIPTOR_BIPN_FAIL_DW                                       1 
#define WF_RX_DESCRIPTOR_BIPN_FAIL_ADDR                                     4 
#define WF_RX_DESCRIPTOR_BIPN_FAIL_MASK                                     0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_BIPN_FAIL_SHIFT                                    29
// DW2
#define WF_RX_DESCRIPTOR_BSSID_DW                                           2 
#define WF_RX_DESCRIPTOR_BSSID_ADDR                                         8 
#define WF_RX_DESCRIPTOR_BSSID_MASK                                         0x0000003f //  5- 0
#define WF_RX_DESCRIPTOR_BSSID_SHIFT                                        0 
#define WF_RX_DESCRIPTOR_H_DW                                               2 
#define WF_RX_DESCRIPTOR_H_ADDR                                             8 
#define WF_RX_DESCRIPTOR_H_MASK                                             0x00000080 //  7- 7
#define WF_RX_DESCRIPTOR_H_SHIFT                                            7 
#define WF_RX_DESCRIPTOR_HEADER_LENGTH_DW                                   2 
#define WF_RX_DESCRIPTOR_HEADER_LENGTH_ADDR                                 8 
#define WF_RX_DESCRIPTOR_HEADER_LENGTH_MASK                                 0x00001f00 // 12- 8
#define WF_RX_DESCRIPTOR_HEADER_LENGTH_SHIFT                                8 
#define WF_RX_DESCRIPTOR_HO_DW                                              2 
#define WF_RX_DESCRIPTOR_HO_ADDR                                            8 
#define WF_RX_DESCRIPTOR_HO_MASK                                            0x0000e000 // 15-13
#define WF_RX_DESCRIPTOR_HO_SHIFT                                           13
#define WF_RX_DESCRIPTOR_SEC_MODE_DW                                        2 
#define WF_RX_DESCRIPTOR_SEC_MODE_ADDR                                      8 
#define WF_RX_DESCRIPTOR_SEC_MODE_MASK                                      0x001f0000 // 20-16
#define WF_RX_DESCRIPTOR_SEC_MODE_SHIFT                                     16
#define WF_RX_DESCRIPTOR_MUBAR_DW                                           2 
#define WF_RX_DESCRIPTOR_MUBAR_ADDR                                         8 
#define WF_RX_DESCRIPTOR_MUBAR_MASK                                         0x00200000 // 21-21
#define WF_RX_DESCRIPTOR_MUBAR_SHIFT                                        21
#define WF_RX_DESCRIPTOR_SWBIT_DW                                           2 
#define WF_RX_DESCRIPTOR_SWBIT_ADDR                                         8 
#define WF_RX_DESCRIPTOR_SWBIT_MASK                                         0x00400000 // 22-22
#define WF_RX_DESCRIPTOR_SWBIT_SHIFT                                        22
#define WF_RX_DESCRIPTOR_DAF_DW                                             2 
#define WF_RX_DESCRIPTOR_DAF_ADDR                                           8 
#define WF_RX_DESCRIPTOR_DAF_MASK                                           0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_DAF_SHIFT                                          23
#define WF_RX_DESCRIPTOR_EL_DW                                              2 
#define WF_RX_DESCRIPTOR_EL_ADDR                                            8 
#define WF_RX_DESCRIPTOR_EL_MASK                                            0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_EL_SHIFT                                           24
#define WF_RX_DESCRIPTOR_HTF_DW                                             2 
#define WF_RX_DESCRIPTOR_HTF_ADDR                                           8 
#define WF_RX_DESCRIPTOR_HTF_MASK                                           0x02000000 // 25-25
#define WF_RX_DESCRIPTOR_HTF_SHIFT                                          25
#define WF_RX_DESCRIPTOR_INTF_DW                                            2 
#define WF_RX_DESCRIPTOR_INTF_ADDR                                          8 
#define WF_RX_DESCRIPTOR_INTF_MASK                                          0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_INTF_SHIFT                                         26
#define WF_RX_DESCRIPTOR_FRAG_DW                                            2 
#define WF_RX_DESCRIPTOR_FRAG_ADDR                                          8 
#define WF_RX_DESCRIPTOR_FRAG_MASK                                          0x08000000 // 27-27
#define WF_RX_DESCRIPTOR_FRAG_SHIFT                                         27
#define WF_RX_DESCRIPTOR_NUL_DW                                             2 
#define WF_RX_DESCRIPTOR_NUL_ADDR                                           8 
#define WF_RX_DESCRIPTOR_NUL_MASK                                           0x10000000 // 28-28
#define WF_RX_DESCRIPTOR_NUL_SHIFT                                          28
#define WF_RX_DESCRIPTOR_NDATA_DW                                           2 
#define WF_RX_DESCRIPTOR_NDATA_ADDR                                         8 
#define WF_RX_DESCRIPTOR_NDATA_MASK                                         0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_NDATA_SHIFT                                        29
#define WF_RX_DESCRIPTOR_NAMP_DW                                            2 
#define WF_RX_DESCRIPTOR_NAMP_ADDR                                          8 
#define WF_RX_DESCRIPTOR_NAMP_MASK                                          0x40000000 // 30-30
#define WF_RX_DESCRIPTOR_NAMP_SHIFT                                         30
#define WF_RX_DESCRIPTOR_BF_RPT_DW                                          2 
#define WF_RX_DESCRIPTOR_BF_RPT_ADDR                                        8 
#define WF_RX_DESCRIPTOR_BF_RPT_MASK                                        0x80000000 // 31-31
#define WF_RX_DESCRIPTOR_BF_RPT_SHIFT                                       31
// DW3
#define WF_RX_DESCRIPTOR_RXV_SN_DW                                          3 
#define WF_RX_DESCRIPTOR_RXV_SN_ADDR                                        12
#define WF_RX_DESCRIPTOR_RXV_SN_MASK                                        0x000000ff //  7- 0
#define WF_RX_DESCRIPTOR_RXV_SN_SHIFT                                       0 
#define WF_RX_DESCRIPTOR_CH_FREQUENCY_DW                                    3 
#define WF_RX_DESCRIPTOR_CH_FREQUENCY_ADDR                                  12
#define WF_RX_DESCRIPTOR_CH_FREQUENCY_MASK                                  0x0000ff00 // 15- 8
#define WF_RX_DESCRIPTOR_CH_FREQUENCY_SHIFT                                 8 
#define WF_RX_DESCRIPTOR_A1_TYPE_DW                                         3 
#define WF_RX_DESCRIPTOR_A1_TYPE_ADDR                                       12
#define WF_RX_DESCRIPTOR_A1_TYPE_MASK                                       0x00030000 // 17-16
#define WF_RX_DESCRIPTOR_A1_TYPE_SHIFT                                      16
#define WF_RX_DESCRIPTOR_HTC_DW                                             3 
#define WF_RX_DESCRIPTOR_HTC_ADDR                                           12
#define WF_RX_DESCRIPTOR_HTC_MASK                                           0x00040000 // 18-18
#define WF_RX_DESCRIPTOR_HTC_SHIFT                                          18
#define WF_RX_DESCRIPTOR_TCL_DW                                             3 
#define WF_RX_DESCRIPTOR_TCL_ADDR                                           12
#define WF_RX_DESCRIPTOR_TCL_MASK                                           0x00080000 // 19-19
#define WF_RX_DESCRIPTOR_TCL_SHIFT                                          19
#define WF_RX_DESCRIPTOR_BBM_DW                                             3 
#define WF_RX_DESCRIPTOR_BBM_ADDR                                           12
#define WF_RX_DESCRIPTOR_BBM_MASK                                           0x00100000 // 20-20
#define WF_RX_DESCRIPTOR_BBM_SHIFT                                          20
#define WF_RX_DESCRIPTOR_BU_DW                                              3 
#define WF_RX_DESCRIPTOR_BU_ADDR                                            12
#define WF_RX_DESCRIPTOR_BU_MASK                                            0x00200000 // 21-21
#define WF_RX_DESCRIPTOR_BU_SHIFT                                           21
#define WF_RX_DESCRIPTOR_CO_ANT_DW                                          3 
#define WF_RX_DESCRIPTOR_CO_ANT_ADDR                                        12
#define WF_RX_DESCRIPTOR_CO_ANT_MASK                                        0x00400000 // 22-22
#define WF_RX_DESCRIPTOR_CO_ANT_SHIFT                                       22
#define WF_RX_DESCRIPTOR_BF_CQI_DW                                          3 
#define WF_RX_DESCRIPTOR_BF_CQI_ADDR                                        12
#define WF_RX_DESCRIPTOR_BF_CQI_MASK                                        0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_BF_CQI_SHIFT                                       23
#define WF_RX_DESCRIPTOR_FC_DW                                              3 
#define WF_RX_DESCRIPTOR_FC_ADDR                                            12
#define WF_RX_DESCRIPTOR_FC_MASK                                            0x01000000 // 24-24
#define WF_RX_DESCRIPTOR_FC_SHIFT                                           24
#define WF_RX_DESCRIPTOR_IP_DW                                              3 
#define WF_RX_DESCRIPTOR_IP_ADDR                                            12
#define WF_RX_DESCRIPTOR_IP_MASK                                            0x04000000 // 26-26
#define WF_RX_DESCRIPTOR_IP_SHIFT                                           26
#define WF_RX_DESCRIPTOR_UT_DW                                              3 
#define WF_RX_DESCRIPTOR_UT_ADDR                                            12
#define WF_RX_DESCRIPTOR_UT_MASK                                            0x08000000 // 27-27
#define WF_RX_DESCRIPTOR_UT_SHIFT                                           27
#define WF_RX_DESCRIPTOR_SMESH_DW                                           3 
#define WF_RX_DESCRIPTOR_SMESH_ADDR                                         12
#define WF_RX_DESCRIPTOR_SMESH_MASK                                         0x40000000 // 30-30
#define WF_RX_DESCRIPTOR_SMESH_SHIFT                                        30
#define WF_RX_DESCRIPTOR_VLAN_DW                                            3 
#define WF_RX_DESCRIPTOR_VLAN_ADDR                                          12
#define WF_RX_DESCRIPTOR_VLAN_MASK                                          0x80000000 // 31-31
#define WF_RX_DESCRIPTOR_VLAN_SHIFT                                         31
// DW4
#define WF_RX_DESCRIPTOR_PF_DW                                              4 
#define WF_RX_DESCRIPTOR_PF_ADDR                                            16
#define WF_RX_DESCRIPTOR_PF_MASK                                            0x00000003 //  1- 0
#define WF_RX_DESCRIPTOR_PF_SHIFT                                           0 
#define WF_RX_DESCRIPTOR_MAC_DW                                             4 
#define WF_RX_DESCRIPTOR_MAC_ADDR                                           16
#define WF_RX_DESCRIPTOR_MAC_MASK                                           0x00000004 //  2- 2
#define WF_RX_DESCRIPTOR_MAC_SHIFT                                          2 
#define WF_RX_DESCRIPTOR_TID_DW                                             4 
#define WF_RX_DESCRIPTOR_TID_ADDR                                           16
#define WF_RX_DESCRIPTOR_TID_MASK                                           0x00000078 //  6- 3
#define WF_RX_DESCRIPTOR_TID_SHIFT                                          3 
#define WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET_DW                               4 
#define WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET_ADDR                             16
#define WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET_MASK                             0x00007f80 // 14- 7
#define WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET_SHIFT                            7 
#define WF_RX_DESCRIPTOR_PSE_FID_DW                                         4 
#define WF_RX_DESCRIPTOR_PSE_FID_ADDR                                       16
#define WF_RX_DESCRIPTOR_PSE_FID_MASK                                       0x1fff0000 // 28-16
#define WF_RX_DESCRIPTOR_PSE_FID_SHIFT                                      16
// DW5
// DW6
#define WF_RX_DESCRIPTOR_CLS_BITMAP_31_0__DW                                6 
#define WF_RX_DESCRIPTOR_CLS_BITMAP_31_0__ADDR                              24
#define WF_RX_DESCRIPTOR_CLS_BITMAP_31_0__MASK                              0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_CLS_BITMAP_31_0__SHIFT                             0 
// DW7
#define WF_RX_DESCRIPTOR_CLS_BITMAP_33_32__DW                               7 
#define WF_RX_DESCRIPTOR_CLS_BITMAP_33_32__ADDR                             28
#define WF_RX_DESCRIPTOR_CLS_BITMAP_33_32__MASK                             0x00000003 //  1- 0
#define WF_RX_DESCRIPTOR_CLS_BITMAP_33_32__SHIFT                            0 
#define WF_RX_DESCRIPTOR_DP_DW                                              7 
#define WF_RX_DESCRIPTOR_DP_ADDR                                            28
#define WF_RX_DESCRIPTOR_DP_MASK                                            0x00080000 // 19-19
#define WF_RX_DESCRIPTOR_DP_SHIFT                                           19
#define WF_RX_DESCRIPTOR_CLS_DW                                             7 
#define WF_RX_DESCRIPTOR_CLS_ADDR                                           28
#define WF_RX_DESCRIPTOR_CLS_MASK                                           0x00100000 // 20-20
#define WF_RX_DESCRIPTOR_CLS_SHIFT                                          20
#define WF_RX_DESCRIPTOR_OFLD_DW                                            7 
#define WF_RX_DESCRIPTOR_OFLD_ADDR                                          28
#define WF_RX_DESCRIPTOR_OFLD_MASK                                          0x00600000 // 22-21
#define WF_RX_DESCRIPTOR_OFLD_SHIFT                                         21
#define WF_RX_DESCRIPTOR_MGC_DW                                             7 
#define WF_RX_DESCRIPTOR_MGC_ADDR                                           28
#define WF_RX_DESCRIPTOR_MGC_MASK                                           0x00800000 // 23-23
#define WF_RX_DESCRIPTOR_MGC_SHIFT                                          23
#define WF_RX_DESCRIPTOR_WOL_DW                                             7 
#define WF_RX_DESCRIPTOR_WOL_ADDR                                           28
#define WF_RX_DESCRIPTOR_WOL_MASK                                           0x1f000000 // 28-24
#define WF_RX_DESCRIPTOR_WOL_SHIFT                                          24
#define WF_RX_DESCRIPTOR_PF_MODE_DW                                         7 
#define WF_RX_DESCRIPTOR_PF_MODE_ADDR                                       28
#define WF_RX_DESCRIPTOR_PF_MODE_MASK                                       0x20000000 // 29-29
#define WF_RX_DESCRIPTOR_PF_MODE_SHIFT                                      29
#define WF_RX_DESCRIPTOR_PF_STS_DW                                          7 
#define WF_RX_DESCRIPTOR_PF_STS_ADDR                                        28
#define WF_RX_DESCRIPTOR_PF_STS_MASK                                        0xc0000000 // 31-30
#define WF_RX_DESCRIPTOR_PF_STS_SHIFT                                       30
// DW8
#define WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD_DW                             8 
#define WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD_ADDR                           32
#define WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD_MASK                           0x0000ffff // 15- 0
#define WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD_SHIFT                          0 
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0__DW                          8 
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0__ADDR                        32
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0__MASK                        0xffff0000 // 31-16
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0__SHIFT                       16
// DW9
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16__DW                         9 
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16__ADDR                       36
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16__MASK                       0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16__SHIFT                      0 
// DW10
#define WF_RX_DESCRIPTOR_FRAGMENT_NUMBER_DW                                 10
#define WF_RX_DESCRIPTOR_FRAGMENT_NUMBER_ADDR                               40
#define WF_RX_DESCRIPTOR_FRAGMENT_NUMBER_MASK                               0x0000000f //  3- 0
#define WF_RX_DESCRIPTOR_FRAGMENT_NUMBER_SHIFT                              0 
#define WF_RX_DESCRIPTOR_SEQUENCE_NUMBER_DW                                 10
#define WF_RX_DESCRIPTOR_SEQUENCE_NUMBER_ADDR                               40
#define WF_RX_DESCRIPTOR_SEQUENCE_NUMBER_MASK                               0x0000fff0 // 15- 4
#define WF_RX_DESCRIPTOR_SEQUENCE_NUMBER_SHIFT                              4 
#define WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD_DW                               10
#define WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD_ADDR                             40
#define WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD_MASK                             0xffff0000 // 31-16
#define WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD_SHIFT                            16
// DW11
#define WF_RX_DESCRIPTOR_HT_CONTROL_FIELD_DW                                11
#define WF_RX_DESCRIPTOR_HT_CONTROL_FIELD_ADDR                              44
#define WF_RX_DESCRIPTOR_HT_CONTROL_FIELD_MASK                              0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_HT_CONTROL_FIELD_SHIFT                             0 
// DW12
#define WF_RX_DESCRIPTOR_PN_31_0__DW                                        12
#define WF_RX_DESCRIPTOR_PN_31_0__ADDR                                      48
#define WF_RX_DESCRIPTOR_PN_31_0__MASK                                      0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_PN_31_0__SHIFT                                     0 
// DW13
#define WF_RX_DESCRIPTOR_PN_63_32__DW                                       13
#define WF_RX_DESCRIPTOR_PN_63_32__ADDR                                     52
#define WF_RX_DESCRIPTOR_PN_63_32__MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_PN_63_32__SHIFT                                    0 
// DW14
#define WF_RX_DESCRIPTOR_PN_95_64__DW                                       14
#define WF_RX_DESCRIPTOR_PN_95_64__ADDR                                     56
#define WF_RX_DESCRIPTOR_PN_95_64__MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_PN_95_64__SHIFT                                    0 
// DW15
#define WF_RX_DESCRIPTOR_PN_127_96__DW                                      15
#define WF_RX_DESCRIPTOR_PN_127_96__ADDR                                    60
#define WF_RX_DESCRIPTOR_PN_127_96__MASK                                    0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_PN_127_96__SHIFT                                   0 
// DW16
#define WF_RX_DESCRIPTOR_TIMESTAMP_DW                                       16
#define WF_RX_DESCRIPTOR_TIMESTAMP_ADDR                                     64
#define WF_RX_DESCRIPTOR_TIMESTAMP_MASK                                     0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_TIMESTAMP_SHIFT                                    0 
// DW17
#define WF_RX_DESCRIPTOR_CRC_DW                                             17
#define WF_RX_DESCRIPTOR_CRC_ADDR                                           68
#define WF_RX_DESCRIPTOR_CRC_MASK                                           0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_CRC_SHIFT                                          0 
// DW18
// DW19
// DW20
#define WF_RX_DESCRIPTOR_P_RXV_DW                                           20
#define WF_RX_DESCRIPTOR_P_RXV_ADDR                                         80
#define WF_RX_DESCRIPTOR_P_RXV_MASK                                         0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_P_RXV_SHIFT                                        0 
#define WF_RX_DESCRIPTOR_P_RXV_RX_RATE_DW				20
#define WF_RX_DESCRIPTOR_P_RXV_RX_RATE_ADDR				80
#define WF_RX_DESCRIPTOR_P_RXV_RX_RATE_MASK				0x0000007f // 0-6
#define WF_RX_DESCRIPTOR_P_RXV_RX_RATE_SHIFT				0
#define WF_RX_DESCRIPTOR_P_RXV_NSTS_DW					20
#define WF_RX_DESCRIPTOR_P_RXV_NSTS_ADDR				80
#define WF_RX_DESCRIPTOR_P_RXV_NSTS_MASK				0x00000780 // 7-10
#define WF_RX_DESCRIPTOR_P_RXV_NSTS_SHIFT				7
#define WF_RX_DESCRIPTOR_P_RXV_BEAMFORMED_DW				20
#define WF_RX_DESCRIPTOR_P_RXV_BEAMFORMED_ADDR				80
#define WF_RX_DESCRIPTOR_P_RXV_BEAMFORMED_MASK				0x00000800 // 11
#define WF_RX_DESCRIPTOR_P_RXV_BEAMFORMED_SHIFT				11
#define WF_RX_DESCRIPTOR_P_RXV_LDPC_DW					20
#define WF_RX_DESCRIPTOR_P_RXV_LDPC_ADDR				80
#define WF_RX_DESCRIPTOR_P_RXV_LDPC_MASK				0x00001000 // 12
#define WF_RX_DESCRIPTOR_P_RXV_LDPC_SHIFT				12
#define WF_RX_DESCRIPTOR_P_RXV_RU_ALLOC_DW				20
#define WF_RX_DESCRIPTOR_P_RXV_RU_ALLOC_ADDR				80
#define WF_RX_DESCRIPTOR_P_RXV_RU_ALLOC_MASK				0x7fc00000 // 22-30
#define WF_RX_DESCRIPTOR_P_RXV_RU_ALLOC_SHIFT				22
// DW21
// DO NOT process repeat field(p_rxv)
// DW22
#define WF_RX_DESCRIPTOR_DBW_DW                                             22
#define WF_RX_DESCRIPTOR_DBW_ADDR                                           88
#define WF_RX_DESCRIPTOR_DBW_MASK                                           0x00000007 //  2- 0
#define WF_RX_DESCRIPTOR_DBW_SHIFT                                          0 
#define WF_RX_DESCRIPTOR_GI_DW                                              22
#define WF_RX_DESCRIPTOR_GI_ADDR                                            88
#define WF_RX_DESCRIPTOR_GI_MASK                                            0x00000018 //  4- 3
#define WF_RX_DESCRIPTOR_GI_SHIFT                                           3 
#define WF_RX_DESCRIPTOR_DCM_DW                                             22
#define WF_RX_DESCRIPTOR_DCM_ADDR                                           88
#define WF_RX_DESCRIPTOR_DCM_MASK                                           0x00000020 //  5- 5
#define WF_RX_DESCRIPTOR_DCM_SHIFT                                          5 
#define WF_RX_DESCRIPTOR_NUM_RX_DW                                          22
#define WF_RX_DESCRIPTOR_NUM_RX_ADDR                                        88
#define WF_RX_DESCRIPTOR_NUM_RX_MASK                                        0x000001c0 //  8- 6
#define WF_RX_DESCRIPTOR_NUM_RX_SHIFT                                       6 
#define WF_RX_DESCRIPTOR_STBC_DW                                            22
#define WF_RX_DESCRIPTOR_STBC_ADDR                                          88
#define WF_RX_DESCRIPTOR_STBC_MASK                                          0x00000600 // 10- 9
#define WF_RX_DESCRIPTOR_STBC_SHIFT                                         9 
#define WF_RX_DESCRIPTOR_TX_MODE_DW                                         22
#define WF_RX_DESCRIPTOR_TX_MODE_ADDR                                       88
#define WF_RX_DESCRIPTOR_TX_MODE_MASK                                       0x00007800 // 14-11
#define WF_RX_DESCRIPTOR_TX_MODE_SHIFT                                      11
// DW23
#define WF_RX_DESCRIPTOR_RCPI_DW                                            23
#define WF_RX_DESCRIPTOR_RCPI_ADDR                                          92
#define WF_RX_DESCRIPTOR_RCPI_MASK                                          0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_RCPI_SHIFT                                         0 
// DW24
#define WF_RX_DESCRIPTOR_C_RXV_DW                                           24
#define WF_RX_DESCRIPTOR_C_RXV_ADDR                                         96
#define WF_RX_DESCRIPTOR_C_RXV_MASK                                         0xffffffff // 31- 0
#define WF_RX_DESCRIPTOR_C_RXV_SHIFT                                        0 
#define WF_RX_DESCRIPTOR_C_RXV_STBC_DW					24
#define WF_RX_DESCRIPTOR_C_RXV_STBC_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_STBC_MASK				0x00000003 //  1- 0
#define WF_RX_DESCRIPTOR_C_RXV_STBC_SHIFT				0
#define WF_RX_DESCRIPTOR_C_RXV_NESS_DW					24
#define WF_RX_DESCRIPTOR_C_RXV_NESS_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_NESS_MASK				0x0000000c // 2- 3
#define WF_RX_DESCRIPTOR_C_RXV_NESS_SHIFT				2
#define WF_RX_DESCRIPTOR_C_RXV_RX_MODE_DW				24
#define WF_RX_DESCRIPTOR_C_RXV_RX_MODE_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_RX_MODE_MASK				0x000000f0 // 4-7
#define WF_RX_DESCRIPTOR_C_RXV_RX_MODE_SHIFT				4
#define WF_RX_DESCRIPTOR_C_RXV_FR_MODE_DW				24
#define WF_RX_DESCRIPTOR_C_RXV_FR_MODE_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_FR_MODE_MASK				0x00003800 // 11-13
#define WF_RX_DESCRIPTOR_C_RXV_FR_MODE_SHIFT				11
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_PS_NOT_ALLOWED_DW			24
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_PS_NOT_ALLOWED_ADDR			96
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_PS_NOT_ALLOWED_MASK			0x00004000 // 14
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_PS_NOT_ALLOWED_SHIFT			14
#define WF_RX_DESCRIPTOR_C_RXV_SHORT_GI_DW				24
#define WF_RX_DESCRIPTOR_C_RXV_SHORT_GI_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_SHORT_GI_MASK				0x00030000 //16-17
#define WF_RX_DESCRIPTOR_C_RXV_SHORT_GI_SHIFT				16
#define WF_RX_DESCRIPTOR_C_RXV_NUM_USER_DW				24
#define WF_RX_DESCRIPTOR_C_RXV_NUM_USER_ADDR				96
#define WF_RX_DESCRIPTOR_C_RXV_NUM_USER_MASK				0x07f00000 //20-26
#define WF_RX_DESCRIPTOR_C_RXV_NUM_USER_SHIFT				20
#define WF_RX_DESCRIPTOR_C_RXV_LTF_DW					24
#define WF_RX_DESCRIPTOR_C_RXV_LTF_ADDR					96
#define WF_RX_DESCRIPTOR_C_RXV_LTF_MASK				0x18000000 //27-28
#define WF_RX_DESCRIPTOR_C_RXV_LTF_SHIFT				27
#define WF_RX_DESCRIPTOR_C_RXV_LDPC_EXTRA_OFDM_SYM_DW			24
#define WF_RX_DESCRIPTOR_C_RXV_LDPC_EXTRA_OFDM_SYM_ADDR			96
#define WF_RX_DESCRIPTOR_C_RXV_LDPC_EXTRA_OFDM_SYM_MASK			0x40000000 // 30
#define WF_RX_DESCRIPTOR_C_RXV_LDPC_EXTRA_OFDM_SYM_SHIFT		30
// DW25
#define WF_RX_DESCRIPTOR_C_RXV_PE_DIS_AMB_DW				25
#define WF_RX_DESCRIPTOR_C_RXV_PE_DIS_AMB_ADDR				100
#define WF_RX_DESCRIPTOR_C_RXV_PE_DIS_AMB_MASK				0x00000002 // 1
#define WF_RX_DESCRIPTOR_C_RXV_PE_DIS_AMB_SHIFT				1
#define WF_RX_DESCRIPTOR_C_RXV_UL_DL_DW					25
#define WF_RX_DESCRIPTOR_C_RXV_UL_DL_ADDR				100
#define WF_RX_DESCRIPTOR_C_RXV_UL_DL_MASK				0x00000004 // 2
#define WF_RX_DESCRIPTOR_C_RXV_UL_DL_SHIFT				2
#define WF_RX_DESCRIPTOR_C_RXV_PPDU_TYPE_COMP_DW			25
#define WF_RX_DESCRIPTOR_C_RXV_PPDU_TYPE_COMP_ADDR			100
#define WF_RX_DESCRIPTOR_C_RXV_PPDU_TYPE_COMP_MASK		0xc0000000 // 30-31
#define WF_RX_DESCRIPTOR_C_RXV_PPDU_TYPE_COMP_SHIFT		30

// DO NOT process repeat field(c_rxv)
// DW26
/* C-RXC Vector, 2nd Cycle */
#define WF_RX_DESCRIPTOR_C_RXV_GROUP_ID_DW				26
#define WF_RX_DESCRIPTOR_C_RXV_GROUP_ID_ADDR				104
#define WF_RX_DESCRIPTOR_C_RXV_GROUP_ID_MASK				0x0fc00000 // 22-27
#define WF_RX_DESCRIPTOR_C_RXV_GROUP_ID_SHIFT				22
#define WF_RX_DESCRIPTOR_C_RXV_NUM_RX_DW				26
#define WF_RX_DESCRIPTOR_C_RXV_NUM_RX_ADDR				104
#define WF_RX_DESCRIPTOR_C_RXV_NUM_RX_MASK				0x70000000 // 28-30
#define WF_RX_DESCRIPTOR_C_RXV_NUM_RX_SHIFT				28
// DO NOT process repeat field(c_rxv)
// DW27
// DO NOT process repeat field(c_rxv)
// DW28
#define WF_RX_DESCRIPTOR_C_RXV_PART_AID_DW				28
#define WF_RX_DESCRIPTOR_C_RXV_PART_AID_ADDR				112
#define WF_RX_DESCRIPTOR_C_RXV_PART_AID_MASK				0x0ffe0000	// 17-27
#define WF_RX_DESCRIPTOR_C_RXV_PART_AID_SHIFT				17
#define WF_RX_DESCRIPTOR_C_RXV_BEAM_CHANGE_DW				28
#define WF_RX_DESCRIPTOR_C_RXV_BEAM_CHANGE_ADDR				112
#define WF_RX_DESCRIPTOR_C_RXV_BEAM_CHANGE_MASK				0x20000000  // 29
#define WF_RX_DESCRIPTOR_C_RXV_BEAM_CHANGE_SHIFT			29
#define WF_RX_DESCRIPTOR_C_RXV_DCM_DW					28
#define WF_RX_DESCRIPTOR_C_RXV_DCM_ADDR					112
#define WF_RX_DESCRIPTOR_C_RXV_DCM_MASK					0x80000000 //31
#define WF_RX_DESCRIPTOR_C_RXV_DCM_SHIFT				31
// DO NOT process repeat field(c_rxv)
// DW29
#define WF_RX_DESCRIPTOR_C_RXV_DOPPLER_DW				29
#define WF_RX_DESCRIPTOR_C_RXV_DOPPLER_ADDR				116
#define WF_RX_DESCRIPTOR_C_RXV_DOPPLER_MASK				0x00000001 // 0
#define WF_RX_DESCRIPTOR_C_RXV_DOPPLER_SHIFT				0
#define WF_RX_DESCRIPTOR_C_RXV_BSS_COLOR_DW				29
#define WF_RX_DESCRIPTOR_C_RXV_BSS_COLOR_ADDR				116
#define WF_RX_DESCRIPTOR_C_RXV_BSS_COLOR_MASK				0x0000fc00// 10-15
#define WF_RX_DESCRIPTOR_C_RXV_BSS_COLOR_SHIFT				10
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_DW					29
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_ADDR				116
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_MASK				0x00fe0000// 17-23
#define WF_RX_DESCRIPTOR_C_RXV_TXOP_SHIFT				17
// DO NOT process repeat field(c_rxv)
// DW30
/* C-RXC Vector, 4th Cycle */
#define WF_RX_DESCRIPTOR_C_RXV_RCPI0_DW					30
#define WF_RX_DESCRIPTOR_C_RXV_RCPI0_ADDR				120
#define WF_RX_DESCRIPTOR_C_RXV_RCPI0_MASK				0x000000ff // 0-7
#define WF_RX_DESCRIPTOR_C_RXV_RCPI0_SHIFT				0
#define WF_RX_DESCRIPTOR_C_RXV_RCPI1_DW					30
#define WF_RX_DESCRIPTOR_C_RXV_RCPI1_ADDR				120
#define WF_RX_DESCRIPTOR_C_RXV_RCPI1_MASK				0x0000ff00	// 8-15
#define WF_RX_DESCRIPTOR_C_RXV_RCPI1_SHIFT				8
#define WF_RX_DESCRIPTOR_C_RXV_RCPI2_DW					30
#define WF_RX_DESCRIPTOR_C_RXV_RCPI2_ADDR				120
#define WF_RX_DESCRIPTOR_C_RXV_RCPI2_MASK				0x00ff0000 // 16-23
#define WF_RX_DESCRIPTOR_C_RXV_RCPI2_SHIFT				16
#define WF_RX_DESCRIPTOR_C_RXV_RCPI3_DW					30
#define WF_RX_DESCRIPTOR_C_RXV_RCPI3_ADDR				120
#define WF_RX_DESCRIPTOR_C_RXV_RCPI3_MASK				0xff000000 // 24-31
#define WF_RX_DESCRIPTOR_C_RXV_RCPI3_SHIFT				24
// DO NOT process repeat field(c_rxv)
// DW31
// DO NOT process repeat field(c_rxv)
// DW32
// DO NOT process repeat field(c_rxv)
// DW33
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE1_DW			33
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE1_ADDR			132
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE1_MASK			0x00000f00 // 8-11
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE1_SHIFT			8
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE2_DW			33
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE2_ADDR			132
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE2_MASK			0x0000f000 // 12-15
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE2_SHIFT			12
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE3_DW			33
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE3_ADDR			132
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE3_MASK			0x000f0000 // 16-19
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE3_SHIFT			16
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE4_DW			33
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE4_ADDR			132
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE4_MASK			0x00f00000 // 20-23
#define WF_RX_DESCRIPTOR_C_RXV_SPATIAL_REUSE4_SHIFT			20
// DO NOT process repeat field(c_rxv)
// DW34
// DO NOT process repeat field(c_rxv)
// DW35
// DO NOT process repeat field(c_rxv)
// DW36
/* C-RXC Vector, 7th Cycle */
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU0_DW				36
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU0_ADDR				144
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU0_MASK				0x000001ff // 0-8
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU0_SHIFT				0
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU1_DW				36
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU1_ADDR				144
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU1_MASK				0x0003fe00// 9 -17
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU1_SHIFT				9
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU2_DW				36
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU2_ADDR				144
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU2_MASK				0x07fc0000// 18-26
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU2_SHIFT				18
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_DW				36
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_ADDR				144
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_MASK				0xf1000000 // 27-31
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_SHIFT				27
// DO NOT process repeat field(c_rxv)
// DW37
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_1_DW				37
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_1_ADDR				148
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_1_MASK				0x0000000f // 0-3
#define WF_RX_DESCRIPTOR_C_RXV_SIGB_RU3_1_SHIFT				0
// DO NOT process repeat field(c_rxv)
// DW38
// DO NOT process repeat field(c_rxv)
// DW39
// DO NOT process repeat field(c_rxv)
// DW40
// DO NOT process repeat field(c_rxv)
// DW41
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_MCS_DW				41
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_MCS_ADDR				164
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_MCS_MASK				0x000c0000 // 18-19
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_MCS_SHIFT			18
#define WF_RX_DESCRIPTOR_C_RXV_EHT_LTF_SYM_NUM_DW			41
#define WF_RX_DESCRIPTOR_C_RXV_EHT_LTF_SYM_NUM_ADDR			164
#define WF_RX_DESCRIPTOR_C_RXV_EHT_LTF_SYM_NUM_MASK			0x00700000 // 20-22
#define WF_RX_DESCRIPTOR_C_RXV_EHT_LTF_SYM_NUM_SHIFT			20
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_SYM_NUM_DW			41
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_SYM_NUM_ADDR			164
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_SYM_NUM_MASK			0x7F80000 // 23-30
#define WF_RX_DESCRIPTOR_C_RXV_EHT_SIG_SYM_NUM_SHIFT			23

// DO NOT process repeat field(c_rxv)
// DW42
// DO NOT process repeat field(c_rxv)
// DW43
// DO NOT process repeat field(c_rxv)
// DW44
// DO NOT process repeat field(c_rxv)
// DW45
// DO NOT process repeat field(c_rxv)
// DW46
// DW47

// DW0
#define HAL_RX_STATUS_GET_RX_BYTE_COUNT(reg32)                                                              READ_FIELD((reg32), WF_RX_DESCRIPTOR_RX_BYTE_COUNT)
#define HAL_RX_STATUS_GET_COSR(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_COSR)
#define HAL_RX_STATUS_GET_PACKET_TYPE(reg32)                                                                READ_FIELD((reg32), WF_RX_DESCRIPTOR_PACKET_TYPE)
// DW1
#define HAL_RX_STATUS_GET_MLD_ID(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_MLD_ID)
#define HAL_RX_STATUS_GET_GROUP_VLD(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_GROUP_VLD)
#define HAL_RX_STATUS_GET_KID(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_KID)
#define HAL_RX_STATUS_GET_CM(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_CM)
#define HAL_RX_STATUS_GET_CLM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_CLM)
#define HAL_RX_STATUS_GET_I(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_I)
#define HAL_RX_STATUS_GET_T(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_T)
#define HAL_RX_STATUS_GET_BN(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_BN)
#define HAL_RX_STATUS_GET_BIPN_FAIL(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_BIPN_FAIL)
// DW2
#define HAL_RX_STATUS_GET_BSSID(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_BSSID)
#define HAL_RX_STATUS_GET_H(reg32)                                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_H)
#define HAL_RX_STATUS_GET_HEADER_LENGTH(reg32)                                                              READ_FIELD((reg32), WF_RX_DESCRIPTOR_HEADER_LENGTH)
#define HAL_RX_STATUS_GET_HO(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_HO)
#define HAL_RX_STATUS_GET_SEC_MODE(reg32)                                                                   READ_FIELD((reg32), WF_RX_DESCRIPTOR_SEC_MODE)
#define HAL_RX_STATUS_GET_MUBAR(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_MUBAR)
#define HAL_RX_STATUS_GET_SWBIT(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_SWBIT)
#define HAL_RX_STATUS_GET_DAF(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_DAF)
#define HAL_RX_STATUS_GET_EL(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_EL)
#define HAL_RX_STATUS_GET_HTF(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_HTF)
#define HAL_RX_STATUS_GET_INTF(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_INTF)
#define HAL_RX_STATUS_GET_FRAG(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_FRAG)
#define HAL_RX_STATUS_GET_NUL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_NUL)
#define HAL_RX_STATUS_GET_NDATA(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_NDATA)
#define HAL_RX_STATUS_GET_NAMP(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_NAMP)
#define HAL_RX_STATUS_GET_BF_RPT(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_BF_RPT)
// DW3
#define HAL_RX_STATUS_GET_RXV_SN(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_RXV_SN)
#define HAL_RX_STATUS_GET_CH_FREQUENCY(reg32)                                                               READ_FIELD((reg32), WF_RX_DESCRIPTOR_CH_FREQUENCY)
#define HAL_RX_STATUS_GET_A1_TYPE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_A1_TYPE)
#define HAL_RX_STATUS_GET_HTC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_HTC)
#define HAL_RX_STATUS_GET_TCL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_TCL)
#define HAL_RX_STATUS_GET_BBM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_BBM)
#define HAL_RX_STATUS_GET_BU(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_BU)
#define HAL_RX_STATUS_GET_CO_ANT(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_CO_ANT)
#define HAL_RX_STATUS_GET_BF_CQI(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_BF_CQI)
#define HAL_RX_STATUS_GET_FC(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_FC)
#define HAL_RX_STATUS_GET_IP(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_IP)
#define HAL_RX_STATUS_GET_UT(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_UT)
#define HAL_RX_STATUS_GET_SMESH(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_SMESH)
#define HAL_RX_STATUS_GET_VLAN(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_VLAN)
// DW4
#define HAL_RX_STATUS_GET_PF(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_PF)
#define HAL_RX_STATUS_GET_MAC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_MAC)
#define HAL_RX_STATUS_GET_TID(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_TID)
#define HAL_RX_STATUS_GET_ETHER_TYPE_OFFSET(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET)
#define HAL_RX_STATUS_GET_PSE_FID(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_PSE_FID)
// DW5
// DW6
#define HAL_RX_STATUS_GET_CLS_BITMAP_31_0_(reg32)                                                           READ_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_31_0_)
// DW7
#define HAL_RX_STATUS_GET_CLS_BITMAP_33_32_(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_33_32_)
#define HAL_RX_STATUS_GET_DP(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_DP)
#define HAL_RX_STATUS_GET_CLS(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_CLS)
#define HAL_RX_STATUS_GET_OFLD(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_OFLD)
#define HAL_RX_STATUS_GET_MGC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_MGC)
#define HAL_RX_STATUS_GET_WOL(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_WOL)
#define HAL_RX_STATUS_GET_PF_MODE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_PF_MODE)
#define HAL_RX_STATUS_GET_PF_STS(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_PF_STS)
// DW8
#define HAL_RX_STATUS_GET_FRAME_CONTROL_FIELD(reg32)                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD)
#define HAL_RX_STATUS_GET_PEER_MLD_ADDRESS_15_0_(reg32)                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0_)
// DW9
#define HAL_RX_STATUS_GET_PEER_MLD_ADDRESS_47_16_(reg32)                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16_)
// DW10
#define HAL_RX_STATUS_GET_FRAGMENT_NUMBER(reg32)                                                            READ_FIELD((reg32), WF_RX_DESCRIPTOR_FRAGMENT_NUMBER)
#define HAL_RX_STATUS_GET_SEQUENCE_NUMBER(reg32)                                                            READ_FIELD((reg32), WF_RX_DESCRIPTOR_SEQUENCE_NUMBER)
#define HAL_RX_STATUS_GET_QOS_CONTROL_FIELD(reg32)                                                          READ_FIELD((reg32), WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD)
// DW11
#define HAL_RX_STATUS_GET_HT_CONTROL_FIELD(reg32)                                                           READ_FIELD((reg32), WF_RX_DESCRIPTOR_HT_CONTROL_FIELD)
// DW12
#define HAL_RX_STATUS_GET_PN_31_0_(reg32)                                                                   READ_FIELD((reg32), WF_RX_DESCRIPTOR_PN_31_0_)
// DW13
#define HAL_RX_STATUS_GET_PN_63_32_(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_PN_63_32_)
// DW14
#define HAL_RX_STATUS_GET_PN_95_64_(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_PN_95_64_)
// DW15
#define HAL_RX_STATUS_GET_PN_127_96_(reg32)                                                                 READ_FIELD((reg32), WF_RX_DESCRIPTOR_PN_127_96_)
// DW16
#define HAL_RX_STATUS_GET_TIMESTAMP(reg32)                                                                  READ_FIELD((reg32), WF_RX_DESCRIPTOR_TIMESTAMP)
// DW17
#define HAL_RX_STATUS_GET_CRC(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_CRC)
// DW18
// DW19
// DW20
#define HAL_RX_STATUS_GET_P_RXV(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_P_RXV)
// DW21
// DO NOT process repeat field(p_rxv)
// DW22
#define HAL_RX_STATUS_GET_DBW(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_DBW)
#define HAL_RX_STATUS_GET_GI(reg32)                                                                         READ_FIELD((reg32), WF_RX_DESCRIPTOR_GI)
#define HAL_RX_STATUS_GET_DCM(reg32)                                                                        READ_FIELD((reg32), WF_RX_DESCRIPTOR_DCM)
#define HAL_RX_STATUS_GET_NUM_RX(reg32)                                                                     READ_FIELD((reg32), WF_RX_DESCRIPTOR_NUM_RX)
#define HAL_RX_STATUS_GET_STBC(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_STBC)
#define HAL_RX_STATUS_GET_TX_MODE(reg32)                                                                    READ_FIELD((reg32), WF_RX_DESCRIPTOR_TX_MODE)
// DW23
#define HAL_RX_STATUS_GET_RCPI(reg32)                                                                       READ_FIELD((reg32), WF_RX_DESCRIPTOR_RCPI)
// DW24
#define HAL_RX_STATUS_GET_C_RXV(reg32)                                                                      READ_FIELD((reg32), WF_RX_DESCRIPTOR_C_RXV)
// DW25
// DO NOT process repeat field(c_rxv)
// DW26
// DO NOT process repeat field(c_rxv)
// DW27
// DO NOT process repeat field(c_rxv)
// DW28
// DO NOT process repeat field(c_rxv)
// DW29
// DO NOT process repeat field(c_rxv)
// DW30
// DO NOT process repeat field(c_rxv)
// DW31
// DO NOT process repeat field(c_rxv)
// DW32
// DO NOT process repeat field(c_rxv)
// DW33
// DO NOT process repeat field(c_rxv)
// DW34
// DO NOT process repeat field(c_rxv)
// DW35
// DO NOT process repeat field(c_rxv)
// DW36
// DO NOT process repeat field(c_rxv)
// DW37
// DO NOT process repeat field(c_rxv)
// DW38
// DO NOT process repeat field(c_rxv)
// DW39
// DO NOT process repeat field(c_rxv)
// DW40
// DO NOT process repeat field(c_rxv)
// DW41
// DO NOT process repeat field(c_rxv)
// DW42
// DO NOT process repeat field(c_rxv)
// DW43
// DO NOT process repeat field(c_rxv)
// DW44
// DO NOT process repeat field(c_rxv)
// DW45
// DO NOT process repeat field(c_rxv)
// DW46
// DW47

// DW0
#define HAL_RX_STATUS_SET_RX_BYTE_COUNT(reg32, val32)                                                       WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_RX_BYTE_COUNT, val32)
#define HAL_RX_STATUS_SET_COSR(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_COSR, val32)
#define HAL_RX_STATUS_SET_PACKET_TYPE(reg32, val32)                                                         WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PACKET_TYPE, val32)
// DW1
#define HAL_RX_STATUS_SET_MLD_ID(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_MLD_ID, val32)
#define HAL_RX_STATUS_SET_GROUP_VLD(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_GROUP_VLD, val32)
#define HAL_RX_STATUS_SET_KID(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_KID, val32)
#define HAL_RX_STATUS_SET_CM(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CM, val32)
#define HAL_RX_STATUS_SET_CLM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CLM, val32)
#define HAL_RX_STATUS_SET_I(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_I, val32)
#define HAL_RX_STATUS_SET_T(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_T, val32)
#define HAL_RX_STATUS_SET_BN(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BN, val32)
#define HAL_RX_STATUS_SET_BIPN_FAIL(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BIPN_FAIL, val32)
// DW2
#define HAL_RX_STATUS_SET_BSSID(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BSSID, val32)
#define HAL_RX_STATUS_SET_H(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_H, val32)
#define HAL_RX_STATUS_SET_HEADER_LENGTH(reg32, val32)                                                       WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_HEADER_LENGTH, val32)
#define HAL_RX_STATUS_SET_HO(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_HO, val32)
#define HAL_RX_STATUS_SET_SEC_MODE(reg32, val32)                                                            WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_SEC_MODE, val32)
#define HAL_RX_STATUS_SET_MUBAR(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_MUBAR, val32)
#define HAL_RX_STATUS_SET_SWBIT(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_SWBIT, val32)
#define HAL_RX_STATUS_SET_DAF(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_DAF, val32)
#define HAL_RX_STATUS_SET_EL(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_EL, val32)
#define HAL_RX_STATUS_SET_HTF(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_HTF, val32)
#define HAL_RX_STATUS_SET_INTF(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_INTF, val32)
#define HAL_RX_STATUS_SET_FRAG(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_FRAG, val32)
#define HAL_RX_STATUS_SET_NUL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_NUL, val32)
#define HAL_RX_STATUS_SET_NDATA(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_NDATA, val32)
#define HAL_RX_STATUS_SET_NAMP(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_NAMP, val32)
#define HAL_RX_STATUS_SET_BF_RPT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BF_RPT, val32)
// DW3
#define HAL_RX_STATUS_SET_RXV_SN(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_RXV_SN, val32)
#define HAL_RX_STATUS_SET_CH_FREQUENCY(reg32, val32)                                                        WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CH_FREQUENCY, val32)
#define HAL_RX_STATUS_SET_A1_TYPE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_A1_TYPE, val32)
#define HAL_RX_STATUS_SET_HTC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_HTC, val32)
#define HAL_RX_STATUS_SET_TCL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_TCL, val32)
#define HAL_RX_STATUS_SET_BBM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BBM, val32)
#define HAL_RX_STATUS_SET_BU(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BU, val32)
#define HAL_RX_STATUS_SET_CO_ANT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CO_ANT, val32)
#define HAL_RX_STATUS_SET_BF_CQI(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_BF_CQI, val32)
#define HAL_RX_STATUS_SET_FC(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_FC, val32)
#define HAL_RX_STATUS_SET_IP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_IP, val32)
#define HAL_RX_STATUS_SET_UT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_UT, val32)
#define HAL_RX_STATUS_SET_SMESH(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_SMESH, val32)
#define HAL_RX_STATUS_SET_VLAN(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_VLAN, val32)
// DW4
#define HAL_RX_STATUS_SET_PF(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PF, val32)
#define HAL_RX_STATUS_SET_MAC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_MAC, val32)
#define HAL_RX_STATUS_SET_TID(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_TID, val32)
#define HAL_RX_STATUS_SET_ETHER_TYPE_OFFSET(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET, val32)
#define HAL_RX_STATUS_SET_PSE_FID(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PSE_FID, val32)
// DW5
// DW6
#define HAL_RX_STATUS_SET_CLS_BITMAP_31_0_(reg32, val32)                                                    WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_31_0_, val32)
// DW7
#define HAL_RX_STATUS_SET_CLS_BITMAP_33_32_(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_33_32_, val32)
#define HAL_RX_STATUS_SET_DP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_DP, val32)
#define HAL_RX_STATUS_SET_CLS(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CLS, val32)
#define HAL_RX_STATUS_SET_OFLD(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_OFLD, val32)
#define HAL_RX_STATUS_SET_MGC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_MGC, val32)
#define HAL_RX_STATUS_SET_WOL(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_WOL, val32)
#define HAL_RX_STATUS_SET_PF_MODE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PF_MODE, val32)
#define HAL_RX_STATUS_SET_PF_STS(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PF_STS, val32)
// DW8
#define HAL_RX_STATUS_SET_FRAME_CONTROL_FIELD(reg32, val32)                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD, val32)
#define HAL_RX_STATUS_SET_PEER_MLD_ADDRESS_15_0_(reg32, val32)                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0_, val32)
// DW9
#define HAL_RX_STATUS_SET_PEER_MLD_ADDRESS_47_16_(reg32, val32)                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16_, val32)
// DW10
#define HAL_RX_STATUS_SET_FRAGMENT_NUMBER(reg32, val32)                                                     WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_FRAGMENT_NUMBER, val32)
#define HAL_RX_STATUS_SET_SEQUENCE_NUMBER(reg32, val32)                                                     WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_SEQUENCE_NUMBER, val32)
#define HAL_RX_STATUS_SET_QOS_CONTROL_FIELD(reg32, val32)                                                   WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD, val32)
// DW11
#define HAL_RX_STATUS_SET_HT_CONTROL_FIELD(reg32, val32)                                                    WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_HT_CONTROL_FIELD, val32)
// DW12
#define HAL_RX_STATUS_SET_PN_31_0_(reg32, val32)                                                            WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PN_31_0_, val32)
// DW13
#define HAL_RX_STATUS_SET_PN_63_32_(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PN_63_32_, val32)
// DW14
#define HAL_RX_STATUS_SET_PN_95_64_(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PN_95_64_, val32)
// DW15
#define HAL_RX_STATUS_SET_PN_127_96_(reg32, val32)                                                          WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_PN_127_96_, val32)
// DW16
#define HAL_RX_STATUS_SET_TIMESTAMP(reg32, val32)                                                           WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_TIMESTAMP, val32)
// DW17
#define HAL_RX_STATUS_SET_CRC(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_CRC, val32)
// DW18
// DW19
// DW20
#define HAL_RX_STATUS_SET_P_RXV(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_P_RXV, val32)
// DW21
// DO NOT process repeat field(p_rxv)
// DW22
#define HAL_RX_STATUS_SET_DBW(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_DBW, val32)
#define HAL_RX_STATUS_SET_GI(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_GI, val32)
#define HAL_RX_STATUS_SET_DCM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_DCM, val32)
#define HAL_RX_STATUS_SET_NUM_RX(reg32, val32)                                                              WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_NUM_RX, val32)
#define HAL_RX_STATUS_SET_STBC(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_STBC, val32)
#define HAL_RX_STATUS_SET_TX_MODE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_TX_MODE, val32)
// DW23
#define HAL_RX_STATUS_SET_RCPI(reg32, val32)                                                                WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_RCPI, val32)
// DW24
#define HAL_RX_STATUS_SET_C_RXV(reg32, val32)                                                               WRITE_FIELD((reg32), WF_RX_DESCRIPTOR_C_RXV, val32)
// DW25
// DO NOT process repeat field(c_rxv)
// DW26
// DO NOT process repeat field(c_rxv)
// DW27
// DO NOT process repeat field(c_rxv)
// DW28
// DO NOT process repeat field(c_rxv)
// DW29
// DO NOT process repeat field(c_rxv)
// DW30
// DO NOT process repeat field(c_rxv)
// DW31
// DO NOT process repeat field(c_rxv)
// DW32
// DO NOT process repeat field(c_rxv)
// DW33
// DO NOT process repeat field(c_rxv)
// DW34
// DO NOT process repeat field(c_rxv)
// DW35
// DO NOT process repeat field(c_rxv)
// DW36
// DO NOT process repeat field(c_rxv)
// DW37
// DO NOT process repeat field(c_rxv)
// DW38
// DO NOT process repeat field(c_rxv)
// DW39
// DO NOT process repeat field(c_rxv)
// DW40
// DO NOT process repeat field(c_rxv)
// DW41
// DO NOT process repeat field(c_rxv)
// DW42
// DO NOT process repeat field(c_rxv)
// DW43
// DO NOT process repeat field(c_rxv)
// DW44
// DO NOT process repeat field(c_rxv)
// DW45
// DO NOT process repeat field(c_rxv)
// DW46
// DW47

// DW0
#define HAL_RX_STATUS_CLR_RX_BYTE_COUNT(reg32)                                                              CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_RX_BYTE_COUNT)
#define HAL_RX_STATUS_CLR_COSR(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_COSR)
#define HAL_RX_STATUS_CLR_PACKET_TYPE(reg32)                                                                CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PACKET_TYPE)
// DW1
#define HAL_RX_STATUS_CLR_MLD_ID(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_MLD_ID)
#define HAL_RX_STATUS_CLR_GROUP_VLD(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_GROUP_VLD)
#define HAL_RX_STATUS_CLR_KID(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_KID)
#define HAL_RX_STATUS_CLR_CM(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CM)
#define HAL_RX_STATUS_CLR_CLM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CLM)
#define HAL_RX_STATUS_CLR_I(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_I)
#define HAL_RX_STATUS_CLR_T(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_T)
#define HAL_RX_STATUS_CLR_BN(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BN)
#define HAL_RX_STATUS_CLR_BIPN_FAIL(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BIPN_FAIL)
// DW2
#define HAL_RX_STATUS_CLR_BSSID(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BSSID)
#define HAL_RX_STATUS_CLR_H(reg32)                                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_H)
#define HAL_RX_STATUS_CLR_HEADER_LENGTH(reg32)                                                              CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_HEADER_LENGTH)
#define HAL_RX_STATUS_CLR_HO(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_HO)
#define HAL_RX_STATUS_CLR_SEC_MODE(reg32)                                                                   CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_SEC_MODE)
#define HAL_RX_STATUS_CLR_MUBAR(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_MUBAR)
#define HAL_RX_STATUS_CLR_SWBIT(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_SWBIT)
#define HAL_RX_STATUS_CLR_DAF(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_DAF)
#define HAL_RX_STATUS_CLR_EL(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_EL)
#define HAL_RX_STATUS_CLR_HTF(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_HTF)
#define HAL_RX_STATUS_CLR_INTF(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_INTF)
#define HAL_RX_STATUS_CLR_FRAG(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_FRAG)
#define HAL_RX_STATUS_CLR_NUL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_NUL)
#define HAL_RX_STATUS_CLR_NDATA(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_NDATA)
#define HAL_RX_STATUS_CLR_NAMP(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_NAMP)
#define HAL_RX_STATUS_CLR_BF_RPT(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BF_RPT)
// DW3
#define HAL_RX_STATUS_CLR_RXV_SN(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_RXV_SN)
#define HAL_RX_STATUS_CLR_CH_FREQUENCY(reg32)                                                               CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CH_FREQUENCY)
#define HAL_RX_STATUS_CLR_A1_TYPE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_A1_TYPE)
#define HAL_RX_STATUS_CLR_HTC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_HTC)
#define HAL_RX_STATUS_CLR_TCL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_TCL)
#define HAL_RX_STATUS_CLR_BBM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BBM)
#define HAL_RX_STATUS_CLR_BU(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BU)
#define HAL_RX_STATUS_CLR_CO_ANT(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CO_ANT)
#define HAL_RX_STATUS_CLR_BF_CQI(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_BF_CQI)
#define HAL_RX_STATUS_CLR_FC(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_FC)
#define HAL_RX_STATUS_CLR_IP(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_IP)
#define HAL_RX_STATUS_CLR_UT(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_UT)
#define HAL_RX_STATUS_CLR_SMESH(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_SMESH)
#define HAL_RX_STATUS_CLR_VLAN(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_VLAN)
// DW4
#define HAL_RX_STATUS_CLR_PF(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PF)
#define HAL_RX_STATUS_CLR_MAC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_MAC)
#define HAL_RX_STATUS_CLR_TID(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_TID)
#define HAL_RX_STATUS_CLR_ETHER_TYPE_OFFSET(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_ETHER_TYPE_OFFSET)
#define HAL_RX_STATUS_CLR_PSE_FID(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PSE_FID)
// DW5
// DW6
#define HAL_RX_STATUS_CLR_CLS_BITMAP_31_0_(reg32)                                                           CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_31_0_)
// DW7
#define HAL_RX_STATUS_CLR_CLS_BITMAP_33_32_(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CLS_BITMAP_33_32_)
#define HAL_RX_STATUS_CLR_DP(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_DP)
#define HAL_RX_STATUS_CLR_CLS(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CLS)
#define HAL_RX_STATUS_CLR_OFLD(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_OFLD)
#define HAL_RX_STATUS_CLR_MGC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_MGC)
#define HAL_RX_STATUS_CLR_WOL(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_WOL)
#define HAL_RX_STATUS_CLR_PF_MODE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PF_MODE)
#define HAL_RX_STATUS_CLR_PF_STS(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PF_STS)
// DW8
#define HAL_RX_STATUS_CLR_FRAME_CONTROL_FIELD(reg32)                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_FRAME_CONTROL_FIELD)
#define HAL_RX_STATUS_CLR_PEER_MLD_ADDRESS_15_0_(reg32)                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_15_0_)
// DW9
#define HAL_RX_STATUS_CLR_PEER_MLD_ADDRESS_47_16_(reg32)                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PEER_MLD_ADDRESS_47_16_)
// DW10
#define HAL_RX_STATUS_CLR_FRAGMENT_NUMBER(reg32)                                                            CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_FRAGMENT_NUMBER)
#define HAL_RX_STATUS_CLR_SEQUENCE_NUMBER(reg32)                                                            CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_SEQUENCE_NUMBER)
#define HAL_RX_STATUS_CLR_QOS_CONTROL_FIELD(reg32)                                                          CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_QOS_CONTROL_FIELD)
// DW11
#define HAL_RX_STATUS_CLR_HT_CONTROL_FIELD(reg32)                                                           CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_HT_CONTROL_FIELD)
// DW12
#define HAL_RX_STATUS_CLR_PN_31_0_(reg32)                                                                   CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PN_31_0_)
// DW13
#define HAL_RX_STATUS_CLR_PN_63_32_(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PN_63_32_)
// DW14
#define HAL_RX_STATUS_CLR_PN_95_64_(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PN_95_64_)
// DW15
#define HAL_RX_STATUS_CLR_PN_127_96_(reg32)                                                                 CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_PN_127_96_)
// DW16
#define HAL_RX_STATUS_CLR_TIMESTAMP(reg32)                                                                  CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_TIMESTAMP)
// DW17
#define HAL_RX_STATUS_CLR_CRC(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_CRC)
// DW18
// DW19
// DW20
#define HAL_RX_STATUS_CLR_P_RXV(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_P_RXV)
// DW21
// DO NOT process repeat field(p_rxv)
// DW22
#define HAL_RX_STATUS_CLR_DBW(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_DBW)
#define HAL_RX_STATUS_CLR_GI(reg32)                                                                         CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_GI)
#define HAL_RX_STATUS_CLR_DCM(reg32)                                                                        CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_DCM)
#define HAL_RX_STATUS_CLR_NUM_RX(reg32)                                                                     CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_NUM_RX)
#define HAL_RX_STATUS_CLR_STBC(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_STBC)
#define HAL_RX_STATUS_CLR_TX_MODE(reg32)                                                                    CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_TX_MODE)
// DW23
#define HAL_RX_STATUS_CLR_RCPI(reg32)                                                                       CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_RCPI)
// DW24
#define HAL_RX_STATUS_CLR_C_RXV(reg32)                                                                      CLEAR_FIELD((reg32), WF_RX_DESCRIPTOR_C_RXV)
// DW25
// DO NOT process repeat field(c_rxv)
// DW26
// DO NOT process repeat field(c_rxv)
// DW27
// DO NOT process repeat field(c_rxv)
// DW28
// DO NOT process repeat field(c_rxv)
// DW29
// DO NOT process repeat field(c_rxv)
// DW30
// DO NOT process repeat field(c_rxv)
// DW31
// DO NOT process repeat field(c_rxv)
// DW32
// DO NOT process repeat field(c_rxv)
// DW33
// DO NOT process repeat field(c_rxv)
// DW34
// DO NOT process repeat field(c_rxv)
// DW35
// DO NOT process repeat field(c_rxv)
// DW36
// DO NOT process repeat field(c_rxv)
// DW37
// DO NOT process repeat field(c_rxv)
// DW38
// DO NOT process repeat field(c_rxv)
// DW39
// DO NOT process repeat field(c_rxv)
// DW40
// DO NOT process repeat field(c_rxv)
// DW41
// DO NOT process repeat field(c_rxv)
// DW42
// DO NOT process repeat field(c_rxv)
// DW43
// DO NOT process repeat field(c_rxv)
// DW44
// DO NOT process repeat field(c_rxv)
// DW45
// DO NOT process repeat field(c_rxv)
// DW46
// DW47

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_RX_DESCRIPTOR_REGS_H__
