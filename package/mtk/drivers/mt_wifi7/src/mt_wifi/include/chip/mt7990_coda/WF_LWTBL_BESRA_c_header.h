#ifndef __WF_LWTBL_REGS_H__
#define __WF_LWTBL_REGS_H__
/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
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

#define PACKING
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD peer_link_address_47_32_  : 16; // 15- 0
        FIELD muar                      :  6; // 21-16
        FIELD rca1                      :  1; // 22-22
        FIELD kid                       :  2; // 24-23
        FIELD rcid                      :  1; // 25-25
        FIELD band                      :  2; // 27-26
        FIELD rv                        :  1; // 28-28
        FIELD rca2                      :  1; // 29-29
        FIELD wpi_flag                  :  1; // 30-30
        FIELD rsvd_31_31                :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_INFO_DW00, *PREG_WF_LWTBL_PEER_INFO_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD peer_link_address_31_0_   : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_INFO_DW01, *PREG_WF_LWTBL_PEER_INFO_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD aid                       : 12; // 11- 0
        FIELD gid_su                    :  1; // 12-12
        FIELD spp_en                    :  1; // 13-13
        FIELD wpi_even                  :  1; // 14-14
        FIELD aad_om                    :  1; // 15-15
        FIELD cipher_suit_pgtk          :  5; // 20-16
        FIELD fd                        :  1; // 21-21
        FIELD td                        :  1; // 22-22
        FIELD sw                        :  1; // 23-23
        FIELD ul                        :  1; // 24-24
        FIELD tx_ps                     :  1; // 25-25
        FIELD qos                       :  1; // 26-26
        FIELD ht                        :  1; // 27-27
        FIELD vht                       :  1; // 28-28
        FIELD he                        :  1; // 29-29
        FIELD eht                       :  1; // 30-30
        FIELD mesh                      :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW02, *PREG_WF_LWTBL_PEER_CAP_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD wmm_q                     :  2; //  1- 0
        FIELD eht_sig_mcs               :  2; //  3- 2
        FIELD hdrt_mode                 :  1; //  4- 4
        FIELD beam_chg                  :  1; //  5- 5
        FIELD eht_ltf_sym_num_opt       :  2; //  7- 6
        FIELD pfmu_idx                  :  8; // 15- 8
        FIELD ulpf_idx                  :  8; // 23-16
        FIELD ribf                      :  1; // 24-24
        FIELD ulpf                      :  1; // 25-25
        FIELD bypass_txsmm              :  1; // 26-26
        FIELD tbf_ht                    :  1; // 27-27
        FIELD tbf_vht                   :  1; // 28-28
        FIELD tbf_he                    :  1; // 29-29
        FIELD tbf_eht                   :  1; // 30-30
        FIELD ign_fbk                   :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW03, *PREG_WF_LWTBL_PEER_CAP_DW03; // DW3

typedef PACKING union
{
    PACKING struct
    {
        FIELD negotiated_winsize0       :  3; //  2- 0
        FIELD negotiated_winsize1       :  3; //  5- 3
        FIELD negotiated_winsize2       :  3; //  8- 6
        FIELD negotiated_winsize3       :  3; // 11- 9
        FIELD negotiated_winsize4       :  3; // 14-12
        FIELD negotiated_winsize5       :  3; // 17-15
        FIELD negotiated_winsize6       :  3; // 20-18
        FIELD negotiated_winsize7       :  3; // 23-21
        FIELD pe                        :  2; // 25-24
        FIELD dis_rhtr                  :  1; // 26-26
        FIELD ldpc_ht                   :  1; // 27-27
        FIELD ldpc_vht                  :  1; // 28-28
        FIELD ldpc_he                   :  1; // 29-29
        FIELD ldpc_eht                  :  1; // 30-30
        FIELD ba_mode                   :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW04, *PREG_WF_LWTBL_PEER_CAP_DW04; // DW4

typedef PACKING union
{
    PACKING struct
    {
        FIELD af                        :  3; //  2- 0
        FIELD af_he                     :  2; //  4- 3
        FIELD rts                       :  1; //  5- 5
        FIELD smps                      :  1; //  6- 6
        FIELD dyn_bw                    :  1; //  7- 7
        FIELD mmss                      :  3; // 10- 8
        FIELD usr                       :  1; // 11-11
        FIELD sr_r                      :  3; // 14-12
        FIELD sr_abort                  :  1; // 15-15
        FIELD tx_power_offset           :  6; // 21-16
        FIELD ltf_eht                   :  2; // 23-22
        FIELD gi_eht                    :  2; // 25-24
        FIELD doppl                     :  1; // 26-26
        FIELD txop_ps_cap               :  1; // 27-27
        FIELD du_i_psm                  :  1; // 28-28
        FIELD i_psm                     :  1; // 29-29
        FIELD psm                       :  1; // 30-30
        FIELD skip_tx                   :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW05, *PREG_WF_LWTBL_PEER_CAP_DW05; // DW5

typedef PACKING union
{
    PACKING struct
    {
        FIELD cbrn                      :  3; //  2- 0
        FIELD dbnss_en                  :  1; //  3- 3
        FIELD baf_en                    :  1; //  4- 4
        FIELD rdgba                     :  1; //  5- 5
        FIELD r                         :  1; //  6- 6
        FIELD spe_idx                   :  5; // 11- 7
        FIELD g2                        :  1; // 12-12
        FIELD g4                        :  1; // 13-13
        FIELD g8                        :  1; // 14-14
        FIELD g16                       :  1; // 15-15
        FIELD g2_ltf                    :  2; // 17-16
        FIELD g4_ltf                    :  2; // 19-18
        FIELD g8_ltf                    :  2; // 21-20
        FIELD g16_ltf                   :  2; // 23-22
        FIELD g2_he                     :  2; // 25-24
        FIELD g4_he                     :  2; // 27-26
        FIELD g8_he                     :  2; // 29-28
        FIELD g16_he                    :  2; // 31-30
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW06, *PREG_WF_LWTBL_PEER_CAP_DW06; // DW6

typedef PACKING union
{
    PACKING struct
    {
        FIELD ba_win_size0              :  4; //  3- 0
        FIELD ba_win_size1              :  4; //  7- 4
        FIELD ba_win_size2              :  4; // 11- 8
        FIELD ba_win_size3              :  4; // 15-12
        FIELD ba_win_size4              :  4; // 19-16
        FIELD ba_win_size5              :  4; // 23-20
        FIELD ba_win_size6              :  4; // 27-24
        FIELD ba_win_size7              :  4; // 31-28
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW07, *PREG_WF_LWTBL_PEER_CAP_DW07; // DW7

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac0_rts_fail_cnt          :  5; //  4- 0
        FIELD ac1_rts_fail_cnt          :  5; //  9- 5
        FIELD ac2_rts_fail_cnt          :  5; // 14-10
        FIELD ac3_rts_fail_cnt          :  5; // 19-15
        FIELD partial_aid               :  9; // 28-20
        FIELD rsvd_30_29                :  2; // 30-29
        FIELD chk_per                   :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW08, *PREG_WF_LWTBL_PEER_CAP_DW08; // DW8

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_avg_mpdu_size          : 14; // 13- 0
        FIELD rsvd_14_14                :  1; // 14-14
        FIELD pritx_sw_mode             :  1; // 15-15
        FIELD pritx_ersu                :  1; // 16-16
        FIELD pritx_plr                 :  1; // 17-17
        FIELD pritx_dcm                 :  1; // 18-18
        FIELD pritx_er106t              :  1; // 19-19
        FIELD fcap                      :  3; // 22-20
        FIELD mpdu_fail_cnt             :  3; // 25-23
        FIELD mpdu_ok_cnt               :  3; // 28-26
        FIELD rate_idx                  :  3; // 31-29
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_PEER_CAP_DW09, *PREG_WF_LWTBL_PEER_CAP_DW09; // DW9

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate1                     : 15; // 14- 0
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD rate2                     : 15; // 30-16
        FIELD rsvd_31_31                :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_TABLE_DW10, *PREG_WF_LWTBL_AUTO_RATE_TABLE_DW10; // DW10

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate3                     : 15; // 14- 0
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD rate4                     : 15; // 30-16
        FIELD rsvd_31_31                :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_TABLE_DW11, *PREG_WF_LWTBL_AUTO_RATE_TABLE_DW11; // DW11

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate5                     : 15; // 14- 0
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD rate6                     : 15; // 30-16
        FIELD rsvd_31_31                :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_TABLE_DW12, *PREG_WF_LWTBL_AUTO_RATE_TABLE_DW12; // DW12

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate7                     : 15; // 14- 0
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD rate8                     : 15; // 30-16
        FIELD rsvd_31_31                :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_TABLE_DW13, *PREG_WF_LWTBL_AUTO_RATE_TABLE_DW13; // DW13

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate1_tx_cnt              : 16; // 15- 0
        FIELD rate1_fail_cnt            : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_11_00                : 12; // 11- 0
        FIELD cipher_suit_igtk          :  2; // 13-12
        FIELD cipher_suit_bigtk         :  2; // 15-14
        FIELD rsvd_31_16                : 16; // 31-16
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_CTR_DW14, *PREG_WF_LWTBL_AUTO_RATE_CTR_DW14; // DW14

typedef PACKING union
{
    PACKING struct
    {
        FIELD rate2_ok_cnt              : 16; // 15- 0
        FIELD rate3_ok_cnt              : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_CTR_DW15, *PREG_WF_LWTBL_AUTO_RATE_CTR_DW15; // DW15

typedef PACKING union
{
    PACKING struct
    {
        FIELD current_bw_tx_cnt         : 16; // 15- 0
        FIELD current_bw_fail_cnt       : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_CTR_DW16, *PREG_WF_LWTBL_AUTO_RATE_CTR_DW16; // DW16

typedef PACKING union
{
    PACKING struct
    {
        FIELD other_bw_tx_cnt           : 16; // 15- 0
        FIELD other_bw_fail_cnt         : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_AUTO_RATE_CTR_DW17, *PREG_WF_LWTBL_AUTO_RATE_CTR_DW17; // DW17

typedef PACKING union
{
    PACKING struct
    {
        FIELD rts_ok_cnt                : 16; // 15- 0
        FIELD rts_fail_cnt              : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_PPDU_CTR_DW18, *PREG_WF_LWTBL_PPDU_CTR_DW18; // DW18

typedef PACKING union
{
    PACKING struct
    {
        FIELD data_retry_cnt            : 16; // 15- 0
        FIELD mgnt_retry_cnt            : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_PPDU_CTR_DW19, *PREG_WF_LWTBL_PPDU_CTR_DW19; // DW19

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac0_ctt_cdt_crb__ctt      : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW20, *PREG_WF_LWTBL_ADM_CTR_DW20; // DW20

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac0_ctb_crt_ctb__crt      : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW21, *PREG_WF_LWTBL_ADM_CTR_DW21; // DW21

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac1_ctt_cdt_crb__ctmc     : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW22, *PREG_WF_LWTBL_ADM_CTR_DW22; // DW22

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac1_ctb_crt_ctb__crmc     : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW23, *PREG_WF_LWTBL_ADM_CTR_DW23; // DW23

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac2_ctt_cdt_crb__ctdb     : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW24, *PREG_WF_LWTBL_ADM_CTR_DW24; // DW24

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac2_ctb_crt_ctb__crdb     : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW25, *PREG_WF_LWTBL_ADM_CTR_DW25; // DW25

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac3_ctt_cdt_crb__ctodb    : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW26, *PREG_WF_LWTBL_ADM_CTR_DW26; // DW26

typedef PACKING union
{
    PACKING struct
    {
        FIELD ac3_ctb_crt_ctb__         : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_LWTBL_ADM_CTR_DW27, *PREG_WF_LWTBL_ADM_CTR_DW27; // DW27

typedef PACKING union
{
    PACKING struct
    {
        FIELD related_idx0              : 12; // 11- 0
        FIELD related_band0             :  2; // 13-12
        FIELD primary_mld_band          :  2; // 15-14
        FIELD related_idx1              : 12; // 27-16
        FIELD related_band1             :  2; // 29-28
        FIELD secondary_mld_band        :  2; // 31-30
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_MLO_INFO_DW28, *PREG_WF_LWTBL_MLO_INFO_DW28; // DW28

typedef PACKING union
{
    PACKING struct
    {
        FIELD dispatch_policy0          :  2; //  1- 0
        FIELD dispatch_policy1          :  2; //  3- 2
        FIELD dispatch_policy2          :  2; //  5- 4
        FIELD dispatch_policy3          :  2; //  7- 6
        FIELD dispatch_policy4          :  2; //  9- 8
        FIELD dispatch_policy5          :  2; // 11-10
        FIELD dispatch_policy6          :  2; // 13-12
        FIELD dispatch_policy7          :  2; // 15-14
        FIELD own_mld_id                :  6; // 21-16
        FIELD emlsr0                    :  1; // 22-22
        FIELD emlmr0                    :  1; // 23-23
        FIELD emlsr1                    :  1; // 24-24
        FIELD emlmr1                    :  1; // 25-25
        FIELD emlsr2                    :  1; // 26-26
        FIELD emlmr2                    :  1; // 27-27
        FIELD rsvd_28_28                :  1; // 28-28
        FIELD str_bitmap                :  3; // 31-29
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_MLO_INFO_DW29, *PREG_WF_LWTBL_MLO_INFO_DW29; // DW29

typedef PACKING union
{
    PACKING struct
    {
        FIELD dispatch_order            :  7; //  6- 0
        FIELD dispatch_ratio            :  7; // 13- 7
        FIELD rsvd_15_14                :  2; // 15-14
        FIELD link_mgf                  : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_MLO_INFO_DW30, *PREG_WF_LWTBL_MLO_INFO_DW30; // DW30

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_22_00                : 23; // 22- 0
        FIELD bftx_tb                   :  1; // 23-23
        FIELD drop                      :  1; // 24-24
        FIELD cascad                    :  1; // 25-25
        FIELD all_ack                   :  1; // 26-26
        FIELD mpdu_size                 :  2; // 28-27
        FIELD rxd_dup_mode              :  2; // 30-29
        FIELD ack_en                    :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_RESP_INFO_DW31, *PREG_WF_LWTBL_RESP_INFO_DW31; // DW31

typedef PACKING union
{
    PACKING struct
    {
        FIELD om_info                   : 12; // 11- 0
        FIELD om_info_eht               :  4; // 15-12
        FIELD rxd_dup_for_om_chg        :  1; // 16-16
        FIELD rxd_dup_white_list        : 12; // 28-17
        FIELD rsvd_31_29                :  3; // 31-29
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_RX_DUP_INFO_DW32, *PREG_WF_LWTBL_RX_DUP_INFO_DW32; // DW32

typedef PACKING union
{
    PACKING struct
    {
        FIELD user_rssi                 :  9; //  8- 0
        FIELD user_snr                  :  6; // 14- 9
        FIELD rsvd_15_15                :  1; // 15-15
        FIELD rapid_reaction_rate       : 12; // 27-16
        FIELD rsvd_29_28                :  2; // 29-28
        FIELD ht_amsdu                  :  1; // 30-30
        FIELD amsdu_cross_lg            :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_RX_STAT_DW33, *PREG_WF_LWTBL_RX_STAT_DW33; // DW33

typedef PACKING union
{
    PACKING struct
    {
        FIELD resp_rcpi0                :  8; //  7- 0
        FIELD resp_rcpi1                :  8; // 15- 8
        FIELD resp_rcpi2                :  8; // 23-16
        FIELD resp_rcpi3                :  8; // 31-24
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_RX_STAT_DW34, *PREG_WF_LWTBL_RX_STAT_DW34; // DW34

typedef PACKING union
{
    PACKING struct
    {
        FIELD snr_rx0                   :  6; //  5- 0
        FIELD snr_rx1                   :  6; // 11- 6
        FIELD snr_rx2                   :  6; // 17-12
        FIELD snr_rx3                   :  6; // 23-18
        FIELD rsvd_31_24                :  8; // 31-24
    } Bits;
    UINT32 Raw;
} REG_WF_LWTBL_RX_STAT_DW35, *PREG_WF_LWTBL_RX_STAT_DW35; // DW35

typedef volatile PACKING struct
{
    REG_WF_LWTBL_PEER_INFO_DW00                  WF_LWTBL_PEER_INFO_DW00                 ; // 0x0000
    REG_WF_LWTBL_PEER_INFO_DW01                  WF_LWTBL_PEER_INFO_DW01                 ; // 0x0004
    REG_WF_LWTBL_PEER_CAP_DW02                   WF_LWTBL_PEER_CAP_DW02                  ; // 0x0008
    REG_WF_LWTBL_PEER_CAP_DW03                   WF_LWTBL_PEER_CAP_DW03                  ; // 0x000c
    REG_WF_LWTBL_PEER_CAP_DW04                   WF_LWTBL_PEER_CAP_DW04                  ; // 0x0010
    REG_WF_LWTBL_PEER_CAP_DW05                   WF_LWTBL_PEER_CAP_DW05                  ; // 0x0014
    REG_WF_LWTBL_PEER_CAP_DW06                   WF_LWTBL_PEER_CAP_DW06                  ; // 0x0018
    REG_WF_LWTBL_PEER_CAP_DW07                   WF_LWTBL_PEER_CAP_DW07                  ; // 0x001c
    REG_WF_LWTBL_PEER_CAP_DW08                   WF_LWTBL_PEER_CAP_DW08                  ; // 0x0020
    REG_WF_LWTBL_PEER_CAP_DW09                   WF_LWTBL_PEER_CAP_DW09                  ; // 0x0024
    REG_WF_LWTBL_AUTO_RATE_TABLE_DW10            WF_LWTBL_AUTO_RATE_TABLE_DW10           ; // 0x0028
    REG_WF_LWTBL_AUTO_RATE_TABLE_DW11            WF_LWTBL_AUTO_RATE_TABLE_DW11           ; // 0x002c
    REG_WF_LWTBL_AUTO_RATE_TABLE_DW12            WF_LWTBL_AUTO_RATE_TABLE_DW12           ; // 0x0030
    REG_WF_LWTBL_AUTO_RATE_TABLE_DW13            WF_LWTBL_AUTO_RATE_TABLE_DW13           ; // 0x0034
    REG_WF_LWTBL_AUTO_RATE_CTR_DW14              WF_LWTBL_AUTO_RATE_CTR_DW14             ; // 0x0038
    REG_WF_LWTBL_AUTO_RATE_CTR_DW15              WF_LWTBL_AUTO_RATE_CTR_DW15             ; // 0x003c
    REG_WF_LWTBL_AUTO_RATE_CTR_DW16              WF_LWTBL_AUTO_RATE_CTR_DW16             ; // 0x0040
    REG_WF_LWTBL_AUTO_RATE_CTR_DW17              WF_LWTBL_AUTO_RATE_CTR_DW17             ; // 0x0044
    REG_WF_LWTBL_PPDU_CTR_DW18                   WF_LWTBL_PPDU_CTR_DW18                  ; // 0x0048
    REG_WF_LWTBL_PPDU_CTR_DW19                   WF_LWTBL_PPDU_CTR_DW19                  ; // 0x004c
    REG_WF_LWTBL_ADM_CTR_DW20                    WF_LWTBL_ADM_CTR_DW20                   ; // 0x0050
    REG_WF_LWTBL_ADM_CTR_DW21                    WF_LWTBL_ADM_CTR_DW21                   ; // 0x0054
    REG_WF_LWTBL_ADM_CTR_DW22                    WF_LWTBL_ADM_CTR_DW22                   ; // 0x0058
    REG_WF_LWTBL_ADM_CTR_DW23                    WF_LWTBL_ADM_CTR_DW23                   ; // 0x005c
    REG_WF_LWTBL_ADM_CTR_DW24                    WF_LWTBL_ADM_CTR_DW24                   ; // 0x0060
    REG_WF_LWTBL_ADM_CTR_DW25                    WF_LWTBL_ADM_CTR_DW25                   ; // 0x0064
    REG_WF_LWTBL_ADM_CTR_DW26                    WF_LWTBL_ADM_CTR_DW26                   ; // 0x0068
    REG_WF_LWTBL_ADM_CTR_DW27                    WF_LWTBL_ADM_CTR_DW27                   ; // 0x006c
    REG_WF_LWTBL_MLO_INFO_DW28                   WF_LWTBL_MLO_INFO_DW28                  ; // 0x0070
    REG_WF_LWTBL_MLO_INFO_DW29                   WF_LWTBL_MLO_INFO_DW29                  ; // 0x0074
    REG_WF_LWTBL_MLO_INFO_DW30                   WF_LWTBL_MLO_INFO_DW30                  ; // 0x0078
    REG_WF_LWTBL_RESP_INFO_DW31                  WF_LWTBL_RESP_INFO_DW31                 ; // 0x007c
    REG_WF_LWTBL_RX_DUP_INFO_DW32                WF_LWTBL_RX_DUP_INFO_DW32               ; // 0x0080
    REG_WF_LWTBL_RX_STAT_DW33                    WF_LWTBL_RX_STAT_DW33                   ; // 0x0084
    REG_WF_LWTBL_RX_STAT_DW34                    WF_LWTBL_RX_STAT_DW34                   ; // 0x0088
    REG_WF_LWTBL_RX_STAT_DW35                    WF_LWTBL_RX_STAT_DW35                   ; // 0x008c
}WF_LWTBL_REGS, *PWF_LWTBL_REGS;
extern PWF_LWTBL_REGS g_WF_LWTBL_BASE;

#define WF_LWTBL_BASE (g_WF_LWTBL_BASE)
#define WF_LWTBL_PEER_INFO_DW00                  INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_INFO_DW00                 ) // 0x0000
#define WF_LWTBL_PEER_INFO_DW01                  INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_INFO_DW01                 ) // 0x0004
#define WF_LWTBL_PEER_CAP_DW02                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW02                  ) // 0x0008
#define WF_LWTBL_PEER_CAP_DW03                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW03                  ) // 0x000c
#define WF_LWTBL_PEER_CAP_DW04                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW04                  ) // 0x0010
#define WF_LWTBL_PEER_CAP_DW05                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW05                  ) // 0x0014
#define WF_LWTBL_PEER_CAP_DW06                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW06                  ) // 0x0018
#define WF_LWTBL_PEER_CAP_DW07                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW07                  ) // 0x001c
#define WF_LWTBL_PEER_CAP_DW08                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW08                  ) // 0x0020
#define WF_LWTBL_PEER_CAP_DW09                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PEER_CAP_DW09                  ) // 0x0024
#define WF_LWTBL_AUTO_RATE_TABLE_DW10            INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_TABLE_DW10           ) // 0x0028
#define WF_LWTBL_AUTO_RATE_TABLE_DW11            INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_TABLE_DW11           ) // 0x002c
#define WF_LWTBL_AUTO_RATE_TABLE_DW12            INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_TABLE_DW12           ) // 0x0030
#define WF_LWTBL_AUTO_RATE_TABLE_DW13            INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_TABLE_DW13           ) // 0x0034
#define WF_LWTBL_AUTO_RATE_CTR_DW14              INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_CTR_DW14             ) // 0x0038
#define WF_LWTBL_AUTO_RATE_CTR_DW15              INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_CTR_DW15             ) // 0x003c
#define WF_LWTBL_AUTO_RATE_CTR_DW16              INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_CTR_DW16             ) // 0x0040
#define WF_LWTBL_AUTO_RATE_CTR_DW17              INREG32(&WF_LWTBL_BASE->WF_LWTBL_AUTO_RATE_CTR_DW17             ) // 0x0044
#define WF_LWTBL_PPDU_CTR_DW18                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PPDU_CTR_DW18                  ) // 0x0048
#define WF_LWTBL_PPDU_CTR_DW19                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_PPDU_CTR_DW19                  ) // 0x004c
#define WF_LWTBL_ADM_CTR_DW20                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW20                   ) // 0x0050
#define WF_LWTBL_ADM_CTR_DW21                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW21                   ) // 0x0054
#define WF_LWTBL_ADM_CTR_DW22                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW22                   ) // 0x0058
#define WF_LWTBL_ADM_CTR_DW23                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW23                   ) // 0x005c
#define WF_LWTBL_ADM_CTR_DW24                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW24                   ) // 0x0060
#define WF_LWTBL_ADM_CTR_DW25                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW25                   ) // 0x0064
#define WF_LWTBL_ADM_CTR_DW26                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW26                   ) // 0x0068
#define WF_LWTBL_ADM_CTR_DW27                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_ADM_CTR_DW27                   ) // 0x006c
#define WF_LWTBL_MLO_INFO_DW28                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_MLO_INFO_DW28                  ) // 0x0070
#define WF_LWTBL_MLO_INFO_DW29                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_MLO_INFO_DW29                  ) // 0x0074
#define WF_LWTBL_MLO_INFO_DW30                   INREG32(&WF_LWTBL_BASE->WF_LWTBL_MLO_INFO_DW30                  ) // 0x0078
#define WF_LWTBL_RESP_INFO_DW31                  INREG32(&WF_LWTBL_BASE->WF_LWTBL_RESP_INFO_DW31                 ) // 0x007c
#define WF_LWTBL_RX_DUP_INFO_DW32                INREG32(&WF_LWTBL_BASE->WF_LWTBL_RX_DUP_INFO_DW32               ) // 0x0080
#define WF_LWTBL_RX_STAT_DW33                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_RX_STAT_DW33                   ) // 0x0084
#define WF_LWTBL_RX_STAT_DW34                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_RX_STAT_DW34                   ) // 0x0088
#define WF_LWTBL_RX_STAT_DW35                    INREG32(&WF_LWTBL_BASE->WF_LWTBL_RX_STAT_DW35                   ) // 0x008c

#endif // REG_BASE_C_MODULE

#define WF_LWTBL_PEER_LINK_ADDRESS_47_32__DW                        0 
#define WF_LWTBL_PEER_LINK_ADDRESS_47_32__ADDR                      0 
#define WF_LWTBL_PEER_LINK_ADDRESS_47_32__MASK                      0x0000ffff // 15- 0
#define WF_LWTBL_PEER_LINK_ADDRESS_47_32__SHIFT                     0 
#define WF_LWTBL_MUAR_DW                                            0 
#define WF_LWTBL_MUAR_ADDR                                          0 
#define WF_LWTBL_MUAR_MASK                                          0x003f0000 // 21-16
#define WF_LWTBL_MUAR_SHIFT                                         16
#define WF_LWTBL_RCA1_DW                                            0 
#define WF_LWTBL_RCA1_ADDR                                          0 
#define WF_LWTBL_RCA1_MASK                                          0x00400000 // 22-22
#define WF_LWTBL_RCA1_SHIFT                                         22
#define WF_LWTBL_KID_DW                                             0 
#define WF_LWTBL_KID_ADDR                                           0 
#define WF_LWTBL_KID_MASK                                           0x01800000 // 24-23
#define WF_LWTBL_KID_SHIFT                                          23
#define WF_LWTBL_RCID_DW                                            0 
#define WF_LWTBL_RCID_ADDR                                          0 
#define WF_LWTBL_RCID_MASK                                          0x02000000 // 25-25
#define WF_LWTBL_RCID_SHIFT                                         25
#define WF_LWTBL_BAND_DW                                            0 
#define WF_LWTBL_BAND_ADDR                                          0 
#define WF_LWTBL_BAND_MASK                                          0x0c000000 // 27-26
#define WF_LWTBL_BAND_SHIFT                                         26
#define WF_LWTBL_RV_DW                                              0 
#define WF_LWTBL_RV_ADDR                                            0 
#define WF_LWTBL_RV_MASK                                            0x10000000 // 28-28
#define WF_LWTBL_RV_SHIFT                                           28
#define WF_LWTBL_RCA2_DW                                            0 
#define WF_LWTBL_RCA2_ADDR                                          0 
#define WF_LWTBL_RCA2_MASK                                          0x20000000 // 29-29
#define WF_LWTBL_RCA2_SHIFT                                         29
#define WF_LWTBL_WPI_FLAG_DW                                        0 
#define WF_LWTBL_WPI_FLAG_ADDR                                      0 
#define WF_LWTBL_WPI_FLAG_MASK                                      0x40000000 // 30-30
#define WF_LWTBL_WPI_FLAG_SHIFT                                     30
#define WF_LWTBL_PEER_LINK_ADDRESS_31_0__DW                         1 
#define WF_LWTBL_PEER_LINK_ADDRESS_31_0__ADDR                       4 
#define WF_LWTBL_PEER_LINK_ADDRESS_31_0__MASK                       0xffffffff // 31- 0
#define WF_LWTBL_PEER_LINK_ADDRESS_31_0__SHIFT                      0 
#define WF_LWTBL_AID_DW                                             2 
#define WF_LWTBL_AID_ADDR                                           8 
#define WF_LWTBL_AID_MASK                                           0x00000fff // 11- 0
#define WF_LWTBL_AID_SHIFT                                          0 
#define WF_LWTBL_GID_SU_DW                                          2 
#define WF_LWTBL_GID_SU_ADDR                                        8 
#define WF_LWTBL_GID_SU_MASK                                        0x00001000 // 12-12
#define WF_LWTBL_GID_SU_SHIFT                                       12
#define WF_LWTBL_SPP_EN_DW                                          2 
#define WF_LWTBL_SPP_EN_ADDR                                        8 
#define WF_LWTBL_SPP_EN_MASK                                        0x00002000 // 13-13
#define WF_LWTBL_SPP_EN_SHIFT                                       13
#define WF_LWTBL_WPI_EVEN_DW                                        2 
#define WF_LWTBL_WPI_EVEN_ADDR                                      8 
#define WF_LWTBL_WPI_EVEN_MASK                                      0x00004000 // 14-14
#define WF_LWTBL_WPI_EVEN_SHIFT                                     14
#define WF_LWTBL_AAD_OM_DW                                          2 
#define WF_LWTBL_AAD_OM_ADDR                                        8 
#define WF_LWTBL_AAD_OM_MASK                                        0x00008000 // 15-15
#define WF_LWTBL_AAD_OM_SHIFT                                       15
#define WF_LWTBL_CIPHER_SUIT_PGTK_DW                                2 
#define WF_LWTBL_CIPHER_SUIT_PGTK_ADDR                              8 
#define WF_LWTBL_CIPHER_SUIT_PGTK_MASK                              0x001f0000 // 20-16
#define WF_LWTBL_CIPHER_SUIT_PGTK_SHIFT                             16
#define WF_LWTBL_FD_DW                                              2 
#define WF_LWTBL_FD_ADDR                                            8 
#define WF_LWTBL_FD_MASK                                            0x00200000 // 21-21
#define WF_LWTBL_FD_SHIFT                                           21
#define WF_LWTBL_TD_DW                                              2 
#define WF_LWTBL_TD_ADDR                                            8 
#define WF_LWTBL_TD_MASK                                            0x00400000 // 22-22
#define WF_LWTBL_TD_SHIFT                                           22
#define WF_LWTBL_SW_DW                                              2 
#define WF_LWTBL_SW_ADDR                                            8 
#define WF_LWTBL_SW_MASK                                            0x00800000 // 23-23
#define WF_LWTBL_SW_SHIFT                                           23
#define WF_LWTBL_UL_DW                                              2 
#define WF_LWTBL_UL_ADDR                                            8 
#define WF_LWTBL_UL_MASK                                            0x01000000 // 24-24
#define WF_LWTBL_UL_SHIFT                                           24
#define WF_LWTBL_TX_PS_DW                                           2 
#define WF_LWTBL_TX_PS_ADDR                                         8 
#define WF_LWTBL_TX_PS_MASK                                         0x02000000 // 25-25
#define WF_LWTBL_TX_PS_SHIFT                                        25
#define WF_LWTBL_QOS_DW                                             2 
#define WF_LWTBL_QOS_ADDR                                           8 
#define WF_LWTBL_QOS_MASK                                           0x04000000 // 26-26
#define WF_LWTBL_QOS_SHIFT                                          26
#define WF_LWTBL_HT_DW                                              2 
#define WF_LWTBL_HT_ADDR                                            8 
#define WF_LWTBL_HT_MASK                                            0x08000000 // 27-27
#define WF_LWTBL_HT_SHIFT                                           27
#define WF_LWTBL_VHT_DW                                             2 
#define WF_LWTBL_VHT_ADDR                                           8 
#define WF_LWTBL_VHT_MASK                                           0x10000000 // 28-28
#define WF_LWTBL_VHT_SHIFT                                          28
#define WF_LWTBL_HE_DW                                              2 
#define WF_LWTBL_HE_ADDR                                            8 
#define WF_LWTBL_HE_MASK                                            0x20000000 // 29-29
#define WF_LWTBL_HE_SHIFT                                           29
#define WF_LWTBL_EHT_DW                                             2 
#define WF_LWTBL_EHT_ADDR                                           8 
#define WF_LWTBL_EHT_MASK                                           0x40000000 // 30-30
#define WF_LWTBL_EHT_SHIFT                                          30
#define WF_LWTBL_MESH_DW                                            2 
#define WF_LWTBL_MESH_ADDR                                          8 
#define WF_LWTBL_MESH_MASK                                          0x80000000 // 31-31
#define WF_LWTBL_MESH_SHIFT                                         31
#define WF_LWTBL_WMM_Q_DW                                           3 
#define WF_LWTBL_WMM_Q_ADDR                                         12
#define WF_LWTBL_WMM_Q_MASK                                         0x00000003 //  1- 0
#define WF_LWTBL_WMM_Q_SHIFT                                        0 
#define WF_LWTBL_EHT_SIG_MCS_DW                                     3 
#define WF_LWTBL_EHT_SIG_MCS_ADDR                                   12
#define WF_LWTBL_EHT_SIG_MCS_MASK                                   0x0000000c //  3- 2
#define WF_LWTBL_EHT_SIG_MCS_SHIFT                                  2 
#define WF_LWTBL_HDRT_MODE_DW                                       3 
#define WF_LWTBL_HDRT_MODE_ADDR                                     12
#define WF_LWTBL_HDRT_MODE_MASK                                     0x00000010 //  4- 4
#define WF_LWTBL_HDRT_MODE_SHIFT                                    4 
#define WF_LWTBL_BEAM_CHG_DW                                        3 
#define WF_LWTBL_BEAM_CHG_ADDR                                      12
#define WF_LWTBL_BEAM_CHG_MASK                                      0x00000020 //  5- 5
#define WF_LWTBL_BEAM_CHG_SHIFT                                     5 
#define WF_LWTBL_EHT_LTF_SYM_NUM_OPT_DW                             3 
#define WF_LWTBL_EHT_LTF_SYM_NUM_OPT_ADDR                           12
#define WF_LWTBL_EHT_LTF_SYM_NUM_OPT_MASK                           0x000000c0 //  7- 6
#define WF_LWTBL_EHT_LTF_SYM_NUM_OPT_SHIFT                          6 
#define WF_LWTBL_PFMU_IDX_DW                                        3 
#define WF_LWTBL_PFMU_IDX_ADDR                                      12
#define WF_LWTBL_PFMU_IDX_MASK                                      0x0000ff00 // 15- 8
#define WF_LWTBL_PFMU_IDX_SHIFT                                     8 
#define WF_LWTBL_ULPF_IDX_DW                                        3 
#define WF_LWTBL_ULPF_IDX_ADDR                                      12
#define WF_LWTBL_ULPF_IDX_MASK                                      0x00ff0000 // 23-16
#define WF_LWTBL_ULPF_IDX_SHIFT                                     16
#define WF_LWTBL_RIBF_DW                                            3 
#define WF_LWTBL_RIBF_ADDR                                          12
#define WF_LWTBL_RIBF_MASK                                          0x01000000 // 24-24
#define WF_LWTBL_RIBF_SHIFT                                         24
#define WF_LWTBL_ULPF_DW                                            3 
#define WF_LWTBL_ULPF_ADDR                                          12
#define WF_LWTBL_ULPF_MASK                                          0x02000000 // 25-25
#define WF_LWTBL_ULPF_SHIFT                                         25
#define WF_LWTBL_BYPASS_TXSMM_DW                                    3 
#define WF_LWTBL_BYPASS_TXSMM_ADDR                                  12
#define WF_LWTBL_BYPASS_TXSMM_MASK                                  0x04000000 // 26-26
#define WF_LWTBL_BYPASS_TXSMM_SHIFT                                 26
#define WF_LWTBL_TBF_HT_DW                                          3 
#define WF_LWTBL_TBF_HT_ADDR                                        12
#define WF_LWTBL_TBF_HT_MASK                                        0x08000000 // 27-27
#define WF_LWTBL_TBF_HT_SHIFT                                       27
#define WF_LWTBL_TBF_VHT_DW                                         3 
#define WF_LWTBL_TBF_VHT_ADDR                                       12
#define WF_LWTBL_TBF_VHT_MASK                                       0x10000000 // 28-28
#define WF_LWTBL_TBF_VHT_SHIFT                                      28
#define WF_LWTBL_TBF_HE_DW                                          3 
#define WF_LWTBL_TBF_HE_ADDR                                        12
#define WF_LWTBL_TBF_HE_MASK                                        0x20000000 // 29-29
#define WF_LWTBL_TBF_HE_SHIFT                                       29
#define WF_LWTBL_TBF_EHT_DW                                         3 
#define WF_LWTBL_TBF_EHT_ADDR                                       12
#define WF_LWTBL_TBF_EHT_MASK                                       0x40000000 // 30-30
#define WF_LWTBL_TBF_EHT_SHIFT                                      30
#define WF_LWTBL_IGN_FBK_DW                                         3 
#define WF_LWTBL_IGN_FBK_ADDR                                       12
#define WF_LWTBL_IGN_FBK_MASK                                       0x80000000 // 31-31
#define WF_LWTBL_IGN_FBK_SHIFT                                      31
#define WF_LWTBL_NEGOTIATED_WINSIZE0_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE0_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE0_MASK                           0x00000007 //  2- 0
#define WF_LWTBL_NEGOTIATED_WINSIZE0_SHIFT                          0 
#define WF_LWTBL_NEGOTIATED_WINSIZE1_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE1_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE1_MASK                           0x00000038 //  5- 3
#define WF_LWTBL_NEGOTIATED_WINSIZE1_SHIFT                          3 
#define WF_LWTBL_NEGOTIATED_WINSIZE2_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE2_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE2_MASK                           0x000001c0 //  8- 6
#define WF_LWTBL_NEGOTIATED_WINSIZE2_SHIFT                          6 
#define WF_LWTBL_NEGOTIATED_WINSIZE3_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE3_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE3_MASK                           0x00000e00 // 11- 9
#define WF_LWTBL_NEGOTIATED_WINSIZE3_SHIFT                          9 
#define WF_LWTBL_NEGOTIATED_WINSIZE4_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE4_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE4_MASK                           0x00007000 // 14-12
#define WF_LWTBL_NEGOTIATED_WINSIZE4_SHIFT                          12
#define WF_LWTBL_NEGOTIATED_WINSIZE5_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE5_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE5_MASK                           0x00038000 // 17-15
#define WF_LWTBL_NEGOTIATED_WINSIZE5_SHIFT                          15
#define WF_LWTBL_NEGOTIATED_WINSIZE6_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE6_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE6_MASK                           0x001c0000 // 20-18
#define WF_LWTBL_NEGOTIATED_WINSIZE6_SHIFT                          18
#define WF_LWTBL_NEGOTIATED_WINSIZE7_DW                             4 
#define WF_LWTBL_NEGOTIATED_WINSIZE7_ADDR                           16
#define WF_LWTBL_NEGOTIATED_WINSIZE7_MASK                           0x00e00000 // 23-21
#define WF_LWTBL_NEGOTIATED_WINSIZE7_SHIFT                          21
#define WF_LWTBL_PE_DW                                              4 
#define WF_LWTBL_PE_ADDR                                            16
#define WF_LWTBL_PE_MASK                                            0x03000000 // 25-24
#define WF_LWTBL_PE_SHIFT                                           24
#define WF_LWTBL_DIS_RHTR_DW                                        4 
#define WF_LWTBL_DIS_RHTR_ADDR                                      16
#define WF_LWTBL_DIS_RHTR_MASK                                      0x04000000 // 26-26
#define WF_LWTBL_DIS_RHTR_SHIFT                                     26
#define WF_LWTBL_LDPC_HT_DW                                         4 
#define WF_LWTBL_LDPC_HT_ADDR                                       16
#define WF_LWTBL_LDPC_HT_MASK                                       0x08000000 // 27-27
#define WF_LWTBL_LDPC_HT_SHIFT                                      27
#define WF_LWTBL_LDPC_VHT_DW                                        4 
#define WF_LWTBL_LDPC_VHT_ADDR                                      16
#define WF_LWTBL_LDPC_VHT_MASK                                      0x10000000 // 28-28
#define WF_LWTBL_LDPC_VHT_SHIFT                                     28
#define WF_LWTBL_LDPC_HE_DW                                         4 
#define WF_LWTBL_LDPC_HE_ADDR                                       16
#define WF_LWTBL_LDPC_HE_MASK                                       0x20000000 // 29-29
#define WF_LWTBL_LDPC_HE_SHIFT                                      29
#define WF_LWTBL_LDPC_EHT_DW                                        4 
#define WF_LWTBL_LDPC_EHT_ADDR                                      16
#define WF_LWTBL_LDPC_EHT_MASK                                      0x40000000 // 30-30
#define WF_LWTBL_LDPC_EHT_SHIFT                                     30
#define WF_LWTBL_BA_MODE_DW                                         4 
#define WF_LWTBL_BA_MODE_ADDR                                       16
#define WF_LWTBL_BA_MODE_MASK                                       0x80000000 // 31-31
#define WF_LWTBL_BA_MODE_SHIFT                                      31
#define WF_LWTBL_AF_DW                                              5 
#define WF_LWTBL_AF_ADDR                                            20
#define WF_LWTBL_AF_MASK                                            0x00000007 //  2- 0
#define WF_LWTBL_AF_SHIFT                                           0 
#define WF_LWTBL_AF_HE_DW                                           5 
#define WF_LWTBL_AF_HE_ADDR                                         20
#define WF_LWTBL_AF_HE_MASK                                         0x00000018 //  4- 3
#define WF_LWTBL_AF_HE_SHIFT                                        3 
#define WF_LWTBL_RTS_DW                                             5 
#define WF_LWTBL_RTS_ADDR                                           20
#define WF_LWTBL_RTS_MASK                                           0x00000020 //  5- 5
#define WF_LWTBL_RTS_SHIFT                                          5 
#define WF_LWTBL_SMPS_DW                                            5 
#define WF_LWTBL_SMPS_ADDR                                          20
#define WF_LWTBL_SMPS_MASK                                          0x00000040 //  6- 6
#define WF_LWTBL_SMPS_SHIFT                                         6 
#define WF_LWTBL_DYN_BW_DW                                          5 
#define WF_LWTBL_DYN_BW_ADDR                                        20
#define WF_LWTBL_DYN_BW_MASK                                        0x00000080 //  7- 7
#define WF_LWTBL_DYN_BW_SHIFT                                       7 
#define WF_LWTBL_MMSS_DW                                            5 
#define WF_LWTBL_MMSS_ADDR                                          20
#define WF_LWTBL_MMSS_MASK                                          0x00000700 // 10- 8
#define WF_LWTBL_MMSS_SHIFT                                         8 
#define WF_LWTBL_USR_DW                                             5 
#define WF_LWTBL_USR_ADDR                                           20
#define WF_LWTBL_USR_MASK                                           0x00000800 // 11-11
#define WF_LWTBL_USR_SHIFT                                          11
#define WF_LWTBL_SR_R_DW                                            5 
#define WF_LWTBL_SR_R_ADDR                                          20
#define WF_LWTBL_SR_R_MASK                                          0x00007000 // 14-12
#define WF_LWTBL_SR_R_SHIFT                                         12
#define WF_LWTBL_SR_ABORT_DW                                        5 
#define WF_LWTBL_SR_ABORT_ADDR                                      20
#define WF_LWTBL_SR_ABORT_MASK                                      0x00008000 // 15-15
#define WF_LWTBL_SR_ABORT_SHIFT                                     15
#define WF_LWTBL_TX_POWER_OFFSET_DW                                 5 
#define WF_LWTBL_TX_POWER_OFFSET_ADDR                               20
#define WF_LWTBL_TX_POWER_OFFSET_MASK                               0x003f0000 // 21-16
#define WF_LWTBL_TX_POWER_OFFSET_SHIFT                              16
#define WF_LWTBL_LTF_EHT_DW                                         5 
#define WF_LWTBL_LTF_EHT_ADDR                                       20
#define WF_LWTBL_LTF_EHT_MASK                                       0x00c00000 // 23-22
#define WF_LWTBL_LTF_EHT_SHIFT                                      22
#define WF_LWTBL_GI_EHT_DW                                          5 
#define WF_LWTBL_GI_EHT_ADDR                                        20
#define WF_LWTBL_GI_EHT_MASK                                        0x03000000 // 25-24
#define WF_LWTBL_GI_EHT_SHIFT                                       24
#define WF_LWTBL_DOPPL_DW                                           5 
#define WF_LWTBL_DOPPL_ADDR                                         20
#define WF_LWTBL_DOPPL_MASK                                         0x04000000 // 26-26
#define WF_LWTBL_DOPPL_SHIFT                                        26
#define WF_LWTBL_TXOP_PS_CAP_DW                                     5 
#define WF_LWTBL_TXOP_PS_CAP_ADDR                                   20
#define WF_LWTBL_TXOP_PS_CAP_MASK                                   0x08000000 // 27-27
#define WF_LWTBL_TXOP_PS_CAP_SHIFT                                  27
#define WF_LWTBL_DU_I_PSM_DW                                        5 
#define WF_LWTBL_DU_I_PSM_ADDR                                      20
#define WF_LWTBL_DU_I_PSM_MASK                                      0x10000000 // 28-28
#define WF_LWTBL_DU_I_PSM_SHIFT                                     28
#define WF_LWTBL_I_PSM_DW                                           5 
#define WF_LWTBL_I_PSM_ADDR                                         20
#define WF_LWTBL_I_PSM_MASK                                         0x20000000 // 29-29
#define WF_LWTBL_I_PSM_SHIFT                                        29
#define WF_LWTBL_PSM_DW                                             5 
#define WF_LWTBL_PSM_ADDR                                           20
#define WF_LWTBL_PSM_MASK                                           0x40000000 // 30-30
#define WF_LWTBL_PSM_SHIFT                                          30
#define WF_LWTBL_SKIP_TX_DW                                         5 
#define WF_LWTBL_SKIP_TX_ADDR                                       20
#define WF_LWTBL_SKIP_TX_MASK                                       0x80000000 // 31-31
#define WF_LWTBL_SKIP_TX_SHIFT                                      31
#define WF_LWTBL_CBRN_DW                                            6 
#define WF_LWTBL_CBRN_ADDR                                          24
#define WF_LWTBL_CBRN_MASK                                          0x00000007 //  2- 0
#define WF_LWTBL_CBRN_SHIFT                                         0 
#define WF_LWTBL_DBNSS_EN_DW                                        6 
#define WF_LWTBL_DBNSS_EN_ADDR                                      24
#define WF_LWTBL_DBNSS_EN_MASK                                      0x00000008 //  3- 3
#define WF_LWTBL_DBNSS_EN_SHIFT                                     3 
#define WF_LWTBL_BAF_EN_DW                                          6 
#define WF_LWTBL_BAF_EN_ADDR                                        24
#define WF_LWTBL_BAF_EN_MASK                                        0x00000010 //  4- 4
#define WF_LWTBL_BAF_EN_SHIFT                                       4 
#define WF_LWTBL_RDGBA_DW                                           6 
#define WF_LWTBL_RDGBA_ADDR                                         24
#define WF_LWTBL_RDGBA_MASK                                         0x00000020 //  5- 5
#define WF_LWTBL_RDGBA_SHIFT                                        5 
#define WF_LWTBL_R_DW                                               6 
#define WF_LWTBL_R_ADDR                                             24
#define WF_LWTBL_R_MASK                                             0x00000040 //  6- 6
#define WF_LWTBL_R_SHIFT                                            6 
#define WF_LWTBL_SPE_IDX_DW                                         6 
#define WF_LWTBL_SPE_IDX_ADDR                                       24
#define WF_LWTBL_SPE_IDX_MASK                                       0x00000f80 // 11- 7
#define WF_LWTBL_SPE_IDX_SHIFT                                      7 
#define WF_LWTBL_G2_DW                                              6 
#define WF_LWTBL_G2_ADDR                                            24
#define WF_LWTBL_G2_MASK                                            0x00001000 // 12-12
#define WF_LWTBL_G2_SHIFT                                           12
#define WF_LWTBL_G4_DW                                              6 
#define WF_LWTBL_G4_ADDR                                            24
#define WF_LWTBL_G4_MASK                                            0x00002000 // 13-13
#define WF_LWTBL_G4_SHIFT                                           13
#define WF_LWTBL_G8_DW                                              6 
#define WF_LWTBL_G8_ADDR                                            24
#define WF_LWTBL_G8_MASK                                            0x00004000 // 14-14
#define WF_LWTBL_G8_SHIFT                                           14
#define WF_LWTBL_G16_DW                                             6 
#define WF_LWTBL_G16_ADDR                                           24
#define WF_LWTBL_G16_MASK                                           0x00008000 // 15-15
#define WF_LWTBL_G16_SHIFT                                          15
#define WF_LWTBL_G2_LTF_DW                                          6 
#define WF_LWTBL_G2_LTF_ADDR                                        24
#define WF_LWTBL_G2_LTF_MASK                                        0x00030000 // 17-16
#define WF_LWTBL_G2_LTF_SHIFT                                       16
#define WF_LWTBL_G4_LTF_DW                                          6 
#define WF_LWTBL_G4_LTF_ADDR                                        24
#define WF_LWTBL_G4_LTF_MASK                                        0x000c0000 // 19-18
#define WF_LWTBL_G4_LTF_SHIFT                                       18
#define WF_LWTBL_G8_LTF_DW                                          6 
#define WF_LWTBL_G8_LTF_ADDR                                        24
#define WF_LWTBL_G8_LTF_MASK                                        0x00300000 // 21-20
#define WF_LWTBL_G8_LTF_SHIFT                                       20
#define WF_LWTBL_G16_LTF_DW                                         6 
#define WF_LWTBL_G16_LTF_ADDR                                       24
#define WF_LWTBL_G16_LTF_MASK                                       0x00c00000 // 23-22
#define WF_LWTBL_G16_LTF_SHIFT                                      22
#define WF_LWTBL_G2_HE_DW                                           6 
#define WF_LWTBL_G2_HE_ADDR                                         24
#define WF_LWTBL_G2_HE_MASK                                         0x03000000 // 25-24
#define WF_LWTBL_G2_HE_SHIFT                                        24
#define WF_LWTBL_G4_HE_DW                                           6 
#define WF_LWTBL_G4_HE_ADDR                                         24
#define WF_LWTBL_G4_HE_MASK                                         0x0c000000 // 27-26
#define WF_LWTBL_G4_HE_SHIFT                                        26
#define WF_LWTBL_G8_HE_DW                                           6 
#define WF_LWTBL_G8_HE_ADDR                                         24
#define WF_LWTBL_G8_HE_MASK                                         0x30000000 // 29-28
#define WF_LWTBL_G8_HE_SHIFT                                        28
#define WF_LWTBL_G16_HE_DW                                          6 
#define WF_LWTBL_G16_HE_ADDR                                        24
#define WF_LWTBL_G16_HE_MASK                                        0xc0000000 // 31-30
#define WF_LWTBL_G16_HE_SHIFT                                       30
#define WF_LWTBL_BA_WIN_SIZE0_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE0_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE0_MASK                                  0x0000000f //  3- 0
#define WF_LWTBL_BA_WIN_SIZE0_SHIFT                                 0 
#define WF_LWTBL_BA_WIN_SIZE1_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE1_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE1_MASK                                  0x000000f0 //  7- 4
#define WF_LWTBL_BA_WIN_SIZE1_SHIFT                                 4 
#define WF_LWTBL_BA_WIN_SIZE2_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE2_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE2_MASK                                  0x00000f00 // 11- 8
#define WF_LWTBL_BA_WIN_SIZE2_SHIFT                                 8 
#define WF_LWTBL_BA_WIN_SIZE3_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE3_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE3_MASK                                  0x0000f000 // 15-12
#define WF_LWTBL_BA_WIN_SIZE3_SHIFT                                 12
#define WF_LWTBL_BA_WIN_SIZE4_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE4_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE4_MASK                                  0x000f0000 // 19-16
#define WF_LWTBL_BA_WIN_SIZE4_SHIFT                                 16
#define WF_LWTBL_BA_WIN_SIZE5_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE5_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE5_MASK                                  0x00f00000 // 23-20
#define WF_LWTBL_BA_WIN_SIZE5_SHIFT                                 20
#define WF_LWTBL_BA_WIN_SIZE6_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE6_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE6_MASK                                  0x0f000000 // 27-24
#define WF_LWTBL_BA_WIN_SIZE6_SHIFT                                 24
#define WF_LWTBL_BA_WIN_SIZE7_DW                                    7 
#define WF_LWTBL_BA_WIN_SIZE7_ADDR                                  28
#define WF_LWTBL_BA_WIN_SIZE7_MASK                                  0xf0000000 // 31-28
#define WF_LWTBL_BA_WIN_SIZE7_SHIFT                                 28
#define WF_LWTBL_AC0_RTS_FAIL_CNT_DW                                8 
#define WF_LWTBL_AC0_RTS_FAIL_CNT_ADDR                              32
#define WF_LWTBL_AC0_RTS_FAIL_CNT_MASK                              0x0000001f //  4- 0
#define WF_LWTBL_AC0_RTS_FAIL_CNT_SHIFT                             0 
#define WF_LWTBL_AC1_RTS_FAIL_CNT_DW                                8 
#define WF_LWTBL_AC1_RTS_FAIL_CNT_ADDR                              32
#define WF_LWTBL_AC1_RTS_FAIL_CNT_MASK                              0x000003e0 //  9- 5
#define WF_LWTBL_AC1_RTS_FAIL_CNT_SHIFT                             5 
#define WF_LWTBL_AC2_RTS_FAIL_CNT_DW                                8 
#define WF_LWTBL_AC2_RTS_FAIL_CNT_ADDR                              32
#define WF_LWTBL_AC2_RTS_FAIL_CNT_MASK                              0x00007c00 // 14-10
#define WF_LWTBL_AC2_RTS_FAIL_CNT_SHIFT                             10
#define WF_LWTBL_AC3_RTS_FAIL_CNT_DW                                8 
#define WF_LWTBL_AC3_RTS_FAIL_CNT_ADDR                              32
#define WF_LWTBL_AC3_RTS_FAIL_CNT_MASK                              0x000f8000 // 19-15
#define WF_LWTBL_AC3_RTS_FAIL_CNT_SHIFT                             15
#define WF_LWTBL_PARTIAL_AID_DW                                     8 
#define WF_LWTBL_PARTIAL_AID_ADDR                                   32
#define WF_LWTBL_PARTIAL_AID_MASK                                   0x1ff00000 // 28-20
#define WF_LWTBL_PARTIAL_AID_SHIFT                                  20
#define WF_LWTBL_CHK_PER_DW                                         8 
#define WF_LWTBL_CHK_PER_ADDR                                       32
#define WF_LWTBL_CHK_PER_MASK                                       0x80000000 // 31-31
#define WF_LWTBL_CHK_PER_SHIFT                                      31
#define WF_LWTBL_RX_AVG_MPDU_SIZE_DW                                9 
#define WF_LWTBL_RX_AVG_MPDU_SIZE_ADDR                              36
#define WF_LWTBL_RX_AVG_MPDU_SIZE_MASK                              0x00003fff // 13- 0
#define WF_LWTBL_RX_AVG_MPDU_SIZE_SHIFT                             0 
#define WF_LWTBL_PRITX_SW_MODE_DW                                   9 
#define WF_LWTBL_PRITX_SW_MODE_ADDR                                 36
#define WF_LWTBL_PRITX_SW_MODE_MASK                                 0x00008000 // 15-15
#define WF_LWTBL_PRITX_SW_MODE_SHIFT                                15
#define WF_LWTBL_PRITX_ERSU_DW                                      9 
#define WF_LWTBL_PRITX_ERSU_ADDR                                    36
#define WF_LWTBL_PRITX_ERSU_MASK                                    0x00010000 // 16-16
#define WF_LWTBL_PRITX_ERSU_SHIFT                                   16
#define WF_LWTBL_PRITX_PLR_DW                                       9 
#define WF_LWTBL_PRITX_PLR_ADDR                                     36
#define WF_LWTBL_PRITX_PLR_MASK                                     0x00020000 // 17-17
#define WF_LWTBL_PRITX_PLR_SHIFT                                    17
#define WF_LWTBL_PRITX_DCM_DW                                       9 
#define WF_LWTBL_PRITX_DCM_ADDR                                     36
#define WF_LWTBL_PRITX_DCM_MASK                                     0x00040000 // 18-18
#define WF_LWTBL_PRITX_DCM_SHIFT                                    18
#define WF_LWTBL_PRITX_ER106T_DW                                    9 
#define WF_LWTBL_PRITX_ER106T_ADDR                                  36
#define WF_LWTBL_PRITX_ER106T_MASK                                  0x00080000 // 19-19
#define WF_LWTBL_PRITX_ER106T_SHIFT                                 19
#define WF_LWTBL_FCAP_DW                                            9 
#define WF_LWTBL_FCAP_ADDR                                          36
#define WF_LWTBL_FCAP_MASK                                          0x00700000 // 22-20
#define WF_LWTBL_FCAP_SHIFT                                         20
#define WF_LWTBL_MPDU_FAIL_CNT_DW                                   9 
#define WF_LWTBL_MPDU_FAIL_CNT_ADDR                                 36
#define WF_LWTBL_MPDU_FAIL_CNT_MASK                                 0x03800000 // 25-23
#define WF_LWTBL_MPDU_FAIL_CNT_SHIFT                                23
#define WF_LWTBL_MPDU_OK_CNT_DW                                     9 
#define WF_LWTBL_MPDU_OK_CNT_ADDR                                   36
#define WF_LWTBL_MPDU_OK_CNT_MASK                                   0x1c000000 // 28-26
#define WF_LWTBL_MPDU_OK_CNT_SHIFT                                  26
#define WF_LWTBL_RATE_IDX_DW                                        9 
#define WF_LWTBL_RATE_IDX_ADDR                                      36
#define WF_LWTBL_RATE_IDX_MASK                                      0xe0000000 // 31-29
#define WF_LWTBL_RATE_IDX_SHIFT                                     29
#define WF_LWTBL_RATE1_DW                                           10
#define WF_LWTBL_RATE1_ADDR                                         40
#define WF_LWTBL_RATE1_MASK                                         0x00007fff // 14- 0
#define WF_LWTBL_RATE1_SHIFT                                        0 
#define WF_LWTBL_RATE2_DW                                           10
#define WF_LWTBL_RATE2_ADDR                                         40
#define WF_LWTBL_RATE2_MASK                                         0x7fff0000 // 30-16
#define WF_LWTBL_RATE2_SHIFT                                        16
#define WF_LWTBL_RATE3_DW                                           11
#define WF_LWTBL_RATE3_ADDR                                         44
#define WF_LWTBL_RATE3_MASK                                         0x00007fff // 14- 0
#define WF_LWTBL_RATE3_SHIFT                                        0 
#define WF_LWTBL_RATE4_DW                                           11
#define WF_LWTBL_RATE4_ADDR                                         44
#define WF_LWTBL_RATE4_MASK                                         0x7fff0000 // 30-16
#define WF_LWTBL_RATE4_SHIFT                                        16
#define WF_LWTBL_RATE5_DW                                           12
#define WF_LWTBL_RATE5_ADDR                                         48
#define WF_LWTBL_RATE5_MASK                                         0x00007fff // 14- 0
#define WF_LWTBL_RATE5_SHIFT                                        0 
#define WF_LWTBL_RATE6_DW                                           12
#define WF_LWTBL_RATE6_ADDR                                         48
#define WF_LWTBL_RATE6_MASK                                         0x7fff0000 // 30-16
#define WF_LWTBL_RATE6_SHIFT                                        16
#define WF_LWTBL_RATE7_DW                                           13
#define WF_LWTBL_RATE7_ADDR                                         52
#define WF_LWTBL_RATE7_MASK                                         0x00007fff // 14- 0
#define WF_LWTBL_RATE7_SHIFT                                        0 
#define WF_LWTBL_RATE8_DW                                           13
#define WF_LWTBL_RATE8_ADDR                                         52
#define WF_LWTBL_RATE8_MASK                                         0x7fff0000 // 30-16
#define WF_LWTBL_RATE8_SHIFT                                        16
#define WF_LWTBL_RATE1_TX_CNT_DW                                    14
#define WF_LWTBL_RATE1_TX_CNT_ADDR                                  56
#define WF_LWTBL_RATE1_TX_CNT_MASK                                  0x0000ffff // 15- 0
#define WF_LWTBL_RATE1_TX_CNT_SHIFT                                 0 
#define WF_LWTBL_CIPHER_SUIT_IGTK_DW                                14
#define WF_LWTBL_CIPHER_SUIT_IGTK_ADDR                              56
#define WF_LWTBL_CIPHER_SUIT_IGTK_MASK                              0x00003000 // 13-12
#define WF_LWTBL_CIPHER_SUIT_IGTK_SHIFT                             12
#define WF_LWTBL_CIPHER_SUIT_BIGTK_DW                               14
#define WF_LWTBL_CIPHER_SUIT_BIGTK_ADDR                             56
#define WF_LWTBL_CIPHER_SUIT_BIGTK_MASK                             0x0000c000 // 15-14
#define WF_LWTBL_CIPHER_SUIT_BIGTK_SHIFT                            14
#define WF_LWTBL_RATE1_FAIL_CNT_DW                                  14
#define WF_LWTBL_RATE1_FAIL_CNT_ADDR                                56
#define WF_LWTBL_RATE1_FAIL_CNT_MASK                                0xffff0000 // 31-16
#define WF_LWTBL_RATE1_FAIL_CNT_SHIFT                               16
#define WF_LWTBL_RATE2_OK_CNT_DW                                    15
#define WF_LWTBL_RATE2_OK_CNT_ADDR                                  60
#define WF_LWTBL_RATE2_OK_CNT_MASK                                  0x0000ffff // 15- 0
#define WF_LWTBL_RATE2_OK_CNT_SHIFT                                 0 
#define WF_LWTBL_RATE3_OK_CNT_DW                                    15
#define WF_LWTBL_RATE3_OK_CNT_ADDR                                  60
#define WF_LWTBL_RATE3_OK_CNT_MASK                                  0xffff0000 // 31-16
#define WF_LWTBL_RATE3_OK_CNT_SHIFT                                 16
#define WF_LWTBL_CURRENT_BW_TX_CNT_DW                               16
#define WF_LWTBL_CURRENT_BW_TX_CNT_ADDR                             64
#define WF_LWTBL_CURRENT_BW_TX_CNT_MASK                             0x0000ffff // 15- 0
#define WF_LWTBL_CURRENT_BW_TX_CNT_SHIFT                            0 
#define WF_LWTBL_CURRENT_BW_FAIL_CNT_DW                             16
#define WF_LWTBL_CURRENT_BW_FAIL_CNT_ADDR                           64
#define WF_LWTBL_CURRENT_BW_FAIL_CNT_MASK                           0xffff0000 // 31-16
#define WF_LWTBL_CURRENT_BW_FAIL_CNT_SHIFT                          16
#define WF_LWTBL_OTHER_BW_TX_CNT_DW                                 17
#define WF_LWTBL_OTHER_BW_TX_CNT_ADDR                               68
#define WF_LWTBL_OTHER_BW_TX_CNT_MASK                               0x0000ffff // 15- 0
#define WF_LWTBL_OTHER_BW_TX_CNT_SHIFT                              0 
#define WF_LWTBL_OTHER_BW_FAIL_CNT_DW                               17
#define WF_LWTBL_OTHER_BW_FAIL_CNT_ADDR                             68
#define WF_LWTBL_OTHER_BW_FAIL_CNT_MASK                             0xffff0000 // 31-16
#define WF_LWTBL_OTHER_BW_FAIL_CNT_SHIFT                            16
#define WF_LWTBL_RTS_OK_CNT_DW                                      18
#define WF_LWTBL_RTS_OK_CNT_ADDR                                    72
#define WF_LWTBL_RTS_OK_CNT_MASK                                    0x0000ffff // 15- 0
#define WF_LWTBL_RTS_OK_CNT_SHIFT                                   0 
#define WF_LWTBL_RTS_FAIL_CNT_DW                                    18
#define WF_LWTBL_RTS_FAIL_CNT_ADDR                                  72
#define WF_LWTBL_RTS_FAIL_CNT_MASK                                  0xffff0000 // 31-16
#define WF_LWTBL_RTS_FAIL_CNT_SHIFT                                 16
#define WF_LWTBL_DATA_RETRY_CNT_DW                                  19
#define WF_LWTBL_DATA_RETRY_CNT_ADDR                                76
#define WF_LWTBL_DATA_RETRY_CNT_MASK                                0x0000ffff // 15- 0
#define WF_LWTBL_DATA_RETRY_CNT_SHIFT                               0 
#define WF_LWTBL_MGNT_RETRY_CNT_DW                                  19
#define WF_LWTBL_MGNT_RETRY_CNT_ADDR                                76
#define WF_LWTBL_MGNT_RETRY_CNT_MASK                                0xffff0000 // 31-16
#define WF_LWTBL_MGNT_RETRY_CNT_SHIFT                               16
#define WF_LWTBL_AC0_CTT_CDT_CRB__CTT_DW                            20
#define WF_LWTBL_AC0_CTT_CDT_CRB__CTT_ADDR                          80
#define WF_LWTBL_AC0_CTT_CDT_CRB__CTT_MASK                          0xffffffff // 31- 0
#define WF_LWTBL_AC0_CTT_CDT_CRB__CTT_SHIFT                         0 
#define WF_LWTBL_AC0_CTB_CRT_CTB__CRT_DW                            21
#define WF_LWTBL_AC0_CTB_CRT_CTB__CRT_ADDR                          84
#define WF_LWTBL_AC0_CTB_CRT_CTB__CRT_MASK                          0xffffffff // 31- 0
#define WF_LWTBL_AC0_CTB_CRT_CTB__CRT_SHIFT                         0 
#define WF_LWTBL_AC1_CTT_CDT_CRB__CTMC_DW                           22
#define WF_LWTBL_AC1_CTT_CDT_CRB__CTMC_ADDR                         88
#define WF_LWTBL_AC1_CTT_CDT_CRB__CTMC_MASK                         0xffffffff // 31- 0
#define WF_LWTBL_AC1_CTT_CDT_CRB__CTMC_SHIFT                        0 
#define WF_LWTBL_AC1_CTB_CRT_CTB__CRMC_DW                           23
#define WF_LWTBL_AC1_CTB_CRT_CTB__CRMC_ADDR                         92
#define WF_LWTBL_AC1_CTB_CRT_CTB__CRMC_MASK                         0xffffffff // 31- 0
#define WF_LWTBL_AC1_CTB_CRT_CTB__CRMC_SHIFT                        0 
#define WF_LWTBL_AC2_CTT_CDT_CRB__CTDB_DW                           24
#define WF_LWTBL_AC2_CTT_CDT_CRB__CTDB_ADDR                         96
#define WF_LWTBL_AC2_CTT_CDT_CRB__CTDB_MASK                         0xffffffff // 31- 0
#define WF_LWTBL_AC2_CTT_CDT_CRB__CTDB_SHIFT                        0 
#define WF_LWTBL_AC2_CTB_CRT_CTB__CRDB_DW                           25
#define WF_LWTBL_AC2_CTB_CRT_CTB__CRDB_ADDR                         100
#define WF_LWTBL_AC2_CTB_CRT_CTB__CRDB_MASK                         0xffffffff // 31- 0
#define WF_LWTBL_AC2_CTB_CRT_CTB__CRDB_SHIFT                        0 
#define WF_LWTBL_AC3_CTT_CDT_CRB__CTODB_DW                          26
#define WF_LWTBL_AC3_CTT_CDT_CRB__CTODB_ADDR                        104
#define WF_LWTBL_AC3_CTT_CDT_CRB__CTODB_MASK                        0xffffffff // 31- 0
#define WF_LWTBL_AC3_CTT_CDT_CRB__CTODB_SHIFT                       0 
#define WF_LWTBL_AC3_CTB_CRT_CTB___DW                               27
#define WF_LWTBL_AC3_CTB_CRT_CTB___ADDR                             108
#define WF_LWTBL_AC3_CTB_CRT_CTB___MASK                             0xffffffff // 31- 0
#define WF_LWTBL_AC3_CTB_CRT_CTB___SHIFT                            0 
#define WF_LWTBL_RELATED_IDX0_DW                                    28
#define WF_LWTBL_RELATED_IDX0_ADDR                                  112
#define WF_LWTBL_RELATED_IDX0_MASK                                  0x00000fff // 11- 0
#define WF_LWTBL_RELATED_IDX0_SHIFT                                 0 
#define WF_LWTBL_RELATED_BAND0_DW                                   28
#define WF_LWTBL_RELATED_BAND0_ADDR                                 112
#define WF_LWTBL_RELATED_BAND0_MASK                                 0x00003000 // 13-12
#define WF_LWTBL_RELATED_BAND0_SHIFT                                12
#define WF_LWTBL_PRIMARY_MLD_BAND_DW                                28
#define WF_LWTBL_PRIMARY_MLD_BAND_ADDR                              112
#define WF_LWTBL_PRIMARY_MLD_BAND_MASK                              0x0000c000 // 15-14
#define WF_LWTBL_PRIMARY_MLD_BAND_SHIFT                             14
#define WF_LWTBL_RELATED_IDX1_DW                                    28
#define WF_LWTBL_RELATED_IDX1_ADDR                                  112
#define WF_LWTBL_RELATED_IDX1_MASK                                  0x0fff0000 // 27-16
#define WF_LWTBL_RELATED_IDX1_SHIFT                                 16
#define WF_LWTBL_RELATED_BAND1_DW                                   28
#define WF_LWTBL_RELATED_BAND1_ADDR                                 112
#define WF_LWTBL_RELATED_BAND1_MASK                                 0x30000000 // 29-28
#define WF_LWTBL_RELATED_BAND1_SHIFT                                28
#define WF_LWTBL_SECONDARY_MLD_BAND_DW                              28
#define WF_LWTBL_SECONDARY_MLD_BAND_ADDR                            112
#define WF_LWTBL_SECONDARY_MLD_BAND_MASK                            0xc0000000 // 31-30
#define WF_LWTBL_SECONDARY_MLD_BAND_SHIFT                           30
#define WF_LWTBL_DISPATCH_POLICY0_DW                                29
#define WF_LWTBL_DISPATCH_POLICY0_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY0_MASK                              0x00000003 //  1- 0
#define WF_LWTBL_DISPATCH_POLICY0_SHIFT                             0 
#define WF_LWTBL_DISPATCH_POLICY1_DW                                29
#define WF_LWTBL_DISPATCH_POLICY1_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY1_MASK                              0x0000000c //  3- 2
#define WF_LWTBL_DISPATCH_POLICY1_SHIFT                             2 
#define WF_LWTBL_DISPATCH_POLICY2_DW                                29
#define WF_LWTBL_DISPATCH_POLICY2_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY2_MASK                              0x00000030 //  5- 4
#define WF_LWTBL_DISPATCH_POLICY2_SHIFT                             4 
#define WF_LWTBL_DISPATCH_POLICY3_DW                                29
#define WF_LWTBL_DISPATCH_POLICY3_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY3_MASK                              0x000000c0 //  7- 6
#define WF_LWTBL_DISPATCH_POLICY3_SHIFT                             6 
#define WF_LWTBL_DISPATCH_POLICY4_DW                                29
#define WF_LWTBL_DISPATCH_POLICY4_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY4_MASK                              0x00000300 //  9- 8
#define WF_LWTBL_DISPATCH_POLICY4_SHIFT                             8 
#define WF_LWTBL_DISPATCH_POLICY5_DW                                29
#define WF_LWTBL_DISPATCH_POLICY5_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY5_MASK                              0x00000c00 // 11-10
#define WF_LWTBL_DISPATCH_POLICY5_SHIFT                             10
#define WF_LWTBL_DISPATCH_POLICY6_DW                                29
#define WF_LWTBL_DISPATCH_POLICY6_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY6_MASK                              0x00003000 // 13-12
#define WF_LWTBL_DISPATCH_POLICY6_SHIFT                             12
#define WF_LWTBL_DISPATCH_POLICY7_DW                                29
#define WF_LWTBL_DISPATCH_POLICY7_ADDR                              116
#define WF_LWTBL_DISPATCH_POLICY7_MASK                              0x0000c000 // 15-14
#define WF_LWTBL_DISPATCH_POLICY7_SHIFT                             14
#define WF_LWTBL_OWN_MLD_ID_DW                                      29
#define WF_LWTBL_OWN_MLD_ID_ADDR                                    116
#define WF_LWTBL_OWN_MLD_ID_MASK                                    0x003f0000 // 21-16
#define WF_LWTBL_OWN_MLD_ID_SHIFT                                   16
#define WF_LWTBL_EMLSR0_DW                                          29
#define WF_LWTBL_EMLSR0_ADDR                                        116
#define WF_LWTBL_EMLSR0_MASK                                        0x00400000 // 22-22
#define WF_LWTBL_EMLSR0_SHIFT                                       22
#define WF_LWTBL_EMLMR0_DW                                          29
#define WF_LWTBL_EMLMR0_ADDR                                        116
#define WF_LWTBL_EMLMR0_MASK                                        0x00800000 // 23-23
#define WF_LWTBL_EMLMR0_SHIFT                                       23
#define WF_LWTBL_EMLSR1_DW                                          29
#define WF_LWTBL_EMLSR1_ADDR                                        116
#define WF_LWTBL_EMLSR1_MASK                                        0x01000000 // 24-24
#define WF_LWTBL_EMLSR1_SHIFT                                       24
#define WF_LWTBL_EMLMR1_DW                                          29
#define WF_LWTBL_EMLMR1_ADDR                                        116
#define WF_LWTBL_EMLMR1_MASK                                        0x02000000 // 25-25
#define WF_LWTBL_EMLMR1_SHIFT                                       25
#define WF_LWTBL_EMLSR2_DW                                          29
#define WF_LWTBL_EMLSR2_ADDR                                        116
#define WF_LWTBL_EMLSR2_MASK                                        0x04000000 // 26-26
#define WF_LWTBL_EMLSR2_SHIFT                                       26
#define WF_LWTBL_EMLMR2_DW                                          29
#define WF_LWTBL_EMLMR2_ADDR                                        116
#define WF_LWTBL_EMLMR2_MASK                                        0x08000000 // 27-27
#define WF_LWTBL_EMLMR2_SHIFT                                       27
#define WF_LWTBL_STR_BITMAP_DW                                      29
#define WF_LWTBL_STR_BITMAP_ADDR                                    116
#define WF_LWTBL_STR_BITMAP_MASK                                    0xe0000000 // 31-29
#define WF_LWTBL_STR_BITMAP_SHIFT                                   29
#define WF_LWTBL_DISPATCH_ORDER_DW                                  30
#define WF_LWTBL_DISPATCH_ORDER_ADDR                                120
#define WF_LWTBL_DISPATCH_ORDER_MASK                                0x0000007f //  6- 0
#define WF_LWTBL_DISPATCH_ORDER_SHIFT                               0 
#define WF_LWTBL_DISPATCH_RATIO_DW                                  30
#define WF_LWTBL_DISPATCH_RATIO_ADDR                                120
#define WF_LWTBL_DISPATCH_RATIO_MASK                                0x00003f80 // 13- 7
#define WF_LWTBL_DISPATCH_RATIO_SHIFT                               7 
#define WF_LWTBL_LINK_MGF_DW                                        30
#define WF_LWTBL_LINK_MGF_ADDR                                      120
#define WF_LWTBL_LINK_MGF_MASK                                      0xffff0000 // 31-16
#define WF_LWTBL_LINK_MGF_SHIFT                                     16
#define WF_LWTBL_BFTX_TB_DW                                         31
#define WF_LWTBL_BFTX_TB_ADDR                                       124
#define WF_LWTBL_BFTX_TB_MASK                                       0x00800000 // 23-23
#define WF_LWTBL_BFTX_TB_SHIFT                                      23
#define WF_LWTBL_DROP_DW                                            31
#define WF_LWTBL_DROP_ADDR                                          124
#define WF_LWTBL_DROP_MASK                                          0x01000000 // 24-24
#define WF_LWTBL_DROP_SHIFT                                         24
#define WF_LWTBL_CASCAD_DW                                          31
#define WF_LWTBL_CASCAD_ADDR                                        124
#define WF_LWTBL_CASCAD_MASK                                        0x02000000 // 25-25
#define WF_LWTBL_CASCAD_SHIFT                                       25
#define WF_LWTBL_ALL_ACK_DW                                         31
#define WF_LWTBL_ALL_ACK_ADDR                                       124
#define WF_LWTBL_ALL_ACK_MASK                                       0x04000000 // 26-26
#define WF_LWTBL_ALL_ACK_SHIFT                                      26
#define WF_LWTBL_MPDU_SIZE_DW                                       31
#define WF_LWTBL_MPDU_SIZE_ADDR                                     124
#define WF_LWTBL_MPDU_SIZE_MASK                                     0x18000000 // 28-27
#define WF_LWTBL_MPDU_SIZE_SHIFT                                    27
#define WF_LWTBL_RXD_DUP_MODE_DW                                    31
#define WF_LWTBL_RXD_DUP_MODE_ADDR                                  124
#define WF_LWTBL_RXD_DUP_MODE_MASK                                  0x60000000 // 30-29
#define WF_LWTBL_RXD_DUP_MODE_SHIFT                                 29
#define WF_LWTBL_ACK_EN_DW                                          31
#define WF_LWTBL_ACK_EN_ADDR                                        124
#define WF_LWTBL_ACK_EN_MASK                                        0x80000000 // 31-31
#define WF_LWTBL_ACK_EN_SHIFT                                       31
#define WF_LWTBL_OM_INFO_DW                                         32
#define WF_LWTBL_OM_INFO_ADDR                                       128
#define WF_LWTBL_OM_INFO_MASK                                       0x00000fff // 11- 0
#define WF_LWTBL_OM_INFO_SHIFT                                      0 
#define WF_LWTBL_OM_INFO_EHT_DW                                     32
#define WF_LWTBL_OM_INFO_EHT_ADDR                                   128
#define WF_LWTBL_OM_INFO_EHT_MASK                                   0x0000f000 // 15-12
#define WF_LWTBL_OM_INFO_EHT_SHIFT                                  12
#define WF_LWTBL_RXD_DUP_FOR_OM_CHG_DW                              32
#define WF_LWTBL_RXD_DUP_FOR_OM_CHG_ADDR                            128
#define WF_LWTBL_RXD_DUP_FOR_OM_CHG_MASK                            0x00010000 // 16-16
#define WF_LWTBL_RXD_DUP_FOR_OM_CHG_SHIFT                           16
#define WF_LWTBL_RXD_DUP_WHITE_LIST_DW                              32
#define WF_LWTBL_RXD_DUP_WHITE_LIST_ADDR                            128
#define WF_LWTBL_RXD_DUP_WHITE_LIST_MASK                            0x1ffe0000 // 28-17
#define WF_LWTBL_RXD_DUP_WHITE_LIST_SHIFT                           17
#define WF_LWTBL_USER_RSSI_DW                                       33
#define WF_LWTBL_USER_RSSI_ADDR                                     132
#define WF_LWTBL_USER_RSSI_MASK                                     0x000001ff //  8- 0
#define WF_LWTBL_USER_RSSI_SHIFT                                    0 
#define WF_LWTBL_USER_SNR_DW                                        33
#define WF_LWTBL_USER_SNR_ADDR                                      132
#define WF_LWTBL_USER_SNR_MASK                                      0x00007e00 // 14- 9
#define WF_LWTBL_USER_SNR_SHIFT                                     9 
#define WF_LWTBL_RAPID_REACTION_RATE_DW                             33
#define WF_LWTBL_RAPID_REACTION_RATE_ADDR                           132
#define WF_LWTBL_RAPID_REACTION_RATE_MASK                           0x0fff0000 // 27-16
#define WF_LWTBL_RAPID_REACTION_RATE_SHIFT                          16
#define WF_LWTBL_HT_AMSDU_DW                                        33
#define WF_LWTBL_HT_AMSDU_ADDR                                      132
#define WF_LWTBL_HT_AMSDU_MASK                                      0x40000000 // 30-30
#define WF_LWTBL_HT_AMSDU_SHIFT                                     30
#define WF_LWTBL_AMSDU_CROSS_LG_DW                                  33
#define WF_LWTBL_AMSDU_CROSS_LG_ADDR                                132
#define WF_LWTBL_AMSDU_CROSS_LG_MASK                                0x80000000 // 31-31
#define WF_LWTBL_AMSDU_CROSS_LG_SHIFT                               31
#define WF_LWTBL_RESP_RCPI0_DW                                      34
#define WF_LWTBL_RESP_RCPI0_ADDR                                    136
#define WF_LWTBL_RESP_RCPI0_MASK                                    0x000000ff //  7- 0
#define WF_LWTBL_RESP_RCPI0_SHIFT                                   0 
#define WF_LWTBL_RESP_RCPI1_DW                                      34
#define WF_LWTBL_RESP_RCPI1_ADDR                                    136
#define WF_LWTBL_RESP_RCPI1_MASK                                    0x0000ff00 // 15- 8
#define WF_LWTBL_RESP_RCPI1_SHIFT                                   8 
#define WF_LWTBL_RESP_RCPI2_DW                                      34
#define WF_LWTBL_RESP_RCPI2_ADDR                                    136
#define WF_LWTBL_RESP_RCPI2_MASK                                    0x00ff0000 // 23-16
#define WF_LWTBL_RESP_RCPI2_SHIFT                                   16
#define WF_LWTBL_RESP_RCPI3_DW                                      34
#define WF_LWTBL_RESP_RCPI3_ADDR                                    136
#define WF_LWTBL_RESP_RCPI3_MASK                                    0xff000000 // 31-24
#define WF_LWTBL_RESP_RCPI3_SHIFT                                   24
#define WF_LWTBL_SNR_RX0_DW                                         35
#define WF_LWTBL_SNR_RX0_ADDR                                       140
#define WF_LWTBL_SNR_RX0_MASK                                       0x0000003f //  5- 0
#define WF_LWTBL_SNR_RX0_SHIFT                                      0 
#define WF_LWTBL_SNR_RX1_DW                                         35
#define WF_LWTBL_SNR_RX1_ADDR                                       140
#define WF_LWTBL_SNR_RX1_MASK                                       0x00000fc0 // 11- 6
#define WF_LWTBL_SNR_RX1_SHIFT                                      6 
#define WF_LWTBL_SNR_RX2_DW                                         35
#define WF_LWTBL_SNR_RX2_ADDR                                       140
#define WF_LWTBL_SNR_RX2_MASK                                       0x0003f000 // 17-12
#define WF_LWTBL_SNR_RX2_SHIFT                                      12
#define WF_LWTBL_SNR_RX3_DW                                         35
#define WF_LWTBL_SNR_RX3_ADDR                                       140
#define WF_LWTBL_SNR_RX3_MASK                                       0x00fc0000 // 23-18
#define WF_LWTBL_SNR_RX3_SHIFT                                      18

#define WF_LWTBL_GET_PEER_LINK_ADDRESS_47_32_(reg32)                                                        READ_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_47_32_)
#define WF_LWTBL_GET_MUAR(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_MUAR)
#define WF_LWTBL_GET_RCA1(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_RCA1)
#define WF_LWTBL_GET_KID(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_KID)
#define WF_LWTBL_GET_RCID(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_RCID)
#define WF_LWTBL_GET_BAND(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_BAND)
#define WF_LWTBL_GET_RV(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_RV)
#define WF_LWTBL_GET_RCA2(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_RCA2)
#define WF_LWTBL_GET_WPI_FLAG(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_WPI_FLAG)
#define WF_LWTBL_GET_PEER_LINK_ADDRESS_31_0_(reg32)                                                         READ_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_31_0_)
#define WF_LWTBL_GET_AID(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_AID)
#define WF_LWTBL_GET_GID_SU(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_GID_SU)
#define WF_LWTBL_GET_SPP_EN(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_SPP_EN)
#define WF_LWTBL_GET_WPI_EVEN(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_WPI_EVEN)
#define WF_LWTBL_GET_AAD_OM(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_AAD_OM)
#define WF_LWTBL_GET_CIPHER_SUIT_PGTK(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_PGTK)
#define WF_LWTBL_GET_FD(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_FD)
#define WF_LWTBL_GET_TD(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_TD)
#define WF_LWTBL_GET_SW(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_SW)
#define WF_LWTBL_GET_UL(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_UL)
#define WF_LWTBL_GET_TX_PS(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_TX_PS)
#define WF_LWTBL_GET_QOS(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_QOS)
#define WF_LWTBL_GET_HT(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_HT)
#define WF_LWTBL_GET_VHT(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_VHT)
#define WF_LWTBL_GET_HE(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_HE)
#define WF_LWTBL_GET_EHT(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_EHT)
#define WF_LWTBL_GET_MESH(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_MESH)
#define WF_LWTBL_GET_WMM_Q(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_WMM_Q)
#define WF_LWTBL_GET_EHT_SIG_MCS(reg32)                                                                     READ_FIELD((reg32), WF_LWTBL_EHT_SIG_MCS)
#define WF_LWTBL_GET_HDRT_MODE(reg32)                                                                       READ_FIELD((reg32), WF_LWTBL_HDRT_MODE)
#define WF_LWTBL_GET_BEAM_CHG(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_BEAM_CHG)
#define WF_LWTBL_GET_EHT_LTF_SYM_NUM_OPT(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_EHT_LTF_SYM_NUM_OPT)
#define WF_LWTBL_GET_PFMU_IDX(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_PFMU_IDX)
#define WF_LWTBL_GET_ULPF_IDX(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_ULPF_IDX)
#define WF_LWTBL_GET_RIBF(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_RIBF)
#define WF_LWTBL_GET_ULPF(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_ULPF)
#define WF_LWTBL_GET_BYPASS_TXSMM(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BYPASS_TXSMM)
#define WF_LWTBL_GET_TBF_HT(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_TBF_HT)
#define WF_LWTBL_GET_TBF_VHT(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_TBF_VHT)
#define WF_LWTBL_GET_TBF_HE(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_TBF_HE)
#define WF_LWTBL_GET_TBF_EHT(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_TBF_EHT)
#define WF_LWTBL_GET_IGN_FBK(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_IGN_FBK)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE0(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE0)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE1(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE1)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE2(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE2)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE3(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE3)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE4(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE4)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE5(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE5)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE6(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE6)
#define WF_LWTBL_GET_NEGOTIATED_WINSIZE7(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE7)
#define WF_LWTBL_GET_PE(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_PE)
#define WF_LWTBL_GET_DIS_RHTR(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_DIS_RHTR)
#define WF_LWTBL_GET_LDPC_HT(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_LDPC_HT)
#define WF_LWTBL_GET_LDPC_VHT(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_LDPC_VHT)
#define WF_LWTBL_GET_LDPC_HE(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_LDPC_HE)
#define WF_LWTBL_GET_LDPC_EHT(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_LDPC_EHT)
#define WF_LWTBL_GET_BA_MODE(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_BA_MODE)
#define WF_LWTBL_GET_AF(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_AF)
#define WF_LWTBL_GET_AF_HE(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_AF_HE)
#define WF_LWTBL_GET_RTS(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_RTS)
#define WF_LWTBL_GET_SMPS(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_SMPS)
#define WF_LWTBL_GET_DYN_BW(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_DYN_BW)
#define WF_LWTBL_GET_MMSS(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_MMSS)
#define WF_LWTBL_GET_USR(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_USR)
#define WF_LWTBL_GET_SR_R(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_SR_R)
#define WF_LWTBL_GET_SR_ABORT(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_SR_ABORT)
#define WF_LWTBL_GET_TX_POWER_OFFSET(reg32)                                                                 READ_FIELD((reg32), WF_LWTBL_TX_POWER_OFFSET)
#define WF_LWTBL_GET_LTF_EHT(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_LTF_EHT)
#define WF_LWTBL_GET_GI_EHT(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_GI_EHT)
#define WF_LWTBL_GET_DOPPL(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_DOPPL)
#define WF_LWTBL_GET_TXOP_PS_CAP(reg32)                                                                     READ_FIELD((reg32), WF_LWTBL_TXOP_PS_CAP)
#define WF_LWTBL_GET_DU_I_PSM(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_DU_I_PSM)
#define WF_LWTBL_GET_I_PSM(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_I_PSM)
#define WF_LWTBL_GET_PSM(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_PSM)
#define WF_LWTBL_GET_SKIP_TX(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SKIP_TX)
#define WF_LWTBL_GET_CBRN(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_CBRN)
#define WF_LWTBL_GET_DBNSS_EN(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_DBNSS_EN)
#define WF_LWTBL_GET_BAF_EN(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_BAF_EN)
#define WF_LWTBL_GET_RDGBA(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RDGBA)
#define WF_LWTBL_GET_R(reg32)                                                                               READ_FIELD((reg32), WF_LWTBL_R)
#define WF_LWTBL_GET_SPE_IDX(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SPE_IDX)
#define WF_LWTBL_GET_G2(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_G2)
#define WF_LWTBL_GET_G4(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_G4)
#define WF_LWTBL_GET_G8(reg32)                                                                              READ_FIELD((reg32), WF_LWTBL_G8)
#define WF_LWTBL_GET_G16(reg32)                                                                             READ_FIELD((reg32), WF_LWTBL_G16)
#define WF_LWTBL_GET_G2_LTF(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_G2_LTF)
#define WF_LWTBL_GET_G4_LTF(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_G4_LTF)
#define WF_LWTBL_GET_G8_LTF(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_G8_LTF)
#define WF_LWTBL_GET_G16_LTF(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_G16_LTF)
#define WF_LWTBL_GET_G2_HE(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_G2_HE)
#define WF_LWTBL_GET_G4_HE(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_G4_HE)
#define WF_LWTBL_GET_G8_HE(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_G8_HE)
#define WF_LWTBL_GET_G16_HE(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_G16_HE)
#define WF_LWTBL_GET_BA_WIN_SIZE0(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE0)
#define WF_LWTBL_GET_BA_WIN_SIZE1(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE1)
#define WF_LWTBL_GET_BA_WIN_SIZE2(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE2)
#define WF_LWTBL_GET_BA_WIN_SIZE3(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE3)
#define WF_LWTBL_GET_BA_WIN_SIZE4(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE4)
#define WF_LWTBL_GET_BA_WIN_SIZE5(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE5)
#define WF_LWTBL_GET_BA_WIN_SIZE6(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE6)
#define WF_LWTBL_GET_BA_WIN_SIZE7(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE7)
#define WF_LWTBL_GET_AC0_RTS_FAIL_CNT(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_AC0_RTS_FAIL_CNT)
#define WF_LWTBL_GET_AC1_RTS_FAIL_CNT(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_AC1_RTS_FAIL_CNT)
#define WF_LWTBL_GET_AC2_RTS_FAIL_CNT(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_AC2_RTS_FAIL_CNT)
#define WF_LWTBL_GET_AC3_RTS_FAIL_CNT(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_AC3_RTS_FAIL_CNT)
#define WF_LWTBL_GET_PARTIAL_AID(reg32)                                                                     READ_FIELD((reg32), WF_LWTBL_PARTIAL_AID)
#define WF_LWTBL_GET_CHK_PER(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_CHK_PER)
#define WF_LWTBL_GET_RX_AVG_MPDU_SIZE(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_RX_AVG_MPDU_SIZE)
#define WF_LWTBL_GET_PRITX_SW_MODE(reg32)                                                                   READ_FIELD((reg32), WF_LWTBL_PRITX_SW_MODE)
#define WF_LWTBL_GET_PRITX_ERSU(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_PRITX_ERSU)
#define WF_LWTBL_GET_PRITX_PLR(reg32)                                                                       READ_FIELD((reg32), WF_LWTBL_PRITX_PLR)
#define WF_LWTBL_GET_PRITX_DCM(reg32)                                                                       READ_FIELD((reg32), WF_LWTBL_PRITX_DCM)
#define WF_LWTBL_GET_PRITX_ER106T(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_PRITX_ER106T)
#define WF_LWTBL_GET_FCAP(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_FCAP)
#define WF_LWTBL_GET_MPDU_FAIL_CNT(reg32)                                                                   READ_FIELD((reg32), WF_LWTBL_MPDU_FAIL_CNT)
#define WF_LWTBL_GET_MPDU_OK_CNT(reg32)                                                                     READ_FIELD((reg32), WF_LWTBL_MPDU_OK_CNT)
#define WF_LWTBL_GET_RATE_IDX(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_RATE_IDX)
#define WF_LWTBL_GET_RATE1(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE1)
#define WF_LWTBL_GET_RATE2(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE2)
#define WF_LWTBL_GET_RATE3(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE3)
#define WF_LWTBL_GET_RATE4(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE4)
#define WF_LWTBL_GET_RATE5(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE5)
#define WF_LWTBL_GET_RATE6(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE6)
#define WF_LWTBL_GET_RATE7(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE7)
#define WF_LWTBL_GET_RATE8(reg32)                                                                           READ_FIELD((reg32), WF_LWTBL_RATE8)
#define WF_LWTBL_GET_RATE1_TX_CNT(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RATE1_TX_CNT)
#define WF_LWTBL_GET_CIPHER_SUIT_IGTK(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_IGTK)
#define WF_LWTBL_GET_CIPHER_SUIT_BIGTK(reg32)                                                               READ_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_BIGTK)
#define WF_LWTBL_GET_RATE1_FAIL_CNT(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_RATE1_FAIL_CNT)
#define WF_LWTBL_GET_RATE2_OK_CNT(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RATE2_OK_CNT)
#define WF_LWTBL_GET_RATE3_OK_CNT(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RATE3_OK_CNT)
#define WF_LWTBL_GET_CURRENT_BW_TX_CNT(reg32)                                                               READ_FIELD((reg32), WF_LWTBL_CURRENT_BW_TX_CNT)
#define WF_LWTBL_GET_CURRENT_BW_FAIL_CNT(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_CURRENT_BW_FAIL_CNT)
#define WF_LWTBL_GET_OTHER_BW_TX_CNT(reg32)                                                                 READ_FIELD((reg32), WF_LWTBL_OTHER_BW_TX_CNT)
#define WF_LWTBL_GET_OTHER_BW_FAIL_CNT(reg32)                                                               READ_FIELD((reg32), WF_LWTBL_OTHER_BW_FAIL_CNT)
#define WF_LWTBL_GET_RTS_OK_CNT(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_RTS_OK_CNT)
#define WF_LWTBL_GET_RTS_FAIL_CNT(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RTS_FAIL_CNT)
#define WF_LWTBL_GET_DATA_RETRY_CNT(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_DATA_RETRY_CNT)
#define WF_LWTBL_GET_MGNT_RETRY_CNT(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_MGNT_RETRY_CNT)
#define WF_LWTBL_GET_AC0_CTT_CDT_CRB__CTT(reg32)                                                            READ_FIELD((reg32), WF_LWTBL_AC0_CTT_CDT_CRB__CTT)
#define WF_LWTBL_GET_AC0_CTB_CRT_CTB__CRT(reg32)                                                            READ_FIELD((reg32), WF_LWTBL_AC0_CTB_CRT_CTB__CRT)
#define WF_LWTBL_GET_AC1_CTT_CDT_CRB__CTMC(reg32)                                                           READ_FIELD((reg32), WF_LWTBL_AC1_CTT_CDT_CRB__CTMC)
#define WF_LWTBL_GET_AC1_CTB_CRT_CTB__CRMC(reg32)                                                           READ_FIELD((reg32), WF_LWTBL_AC1_CTB_CRT_CTB__CRMC)
#define WF_LWTBL_GET_AC2_CTT_CDT_CRB__CTDB(reg32)                                                           READ_FIELD((reg32), WF_LWTBL_AC2_CTT_CDT_CRB__CTDB)
#define WF_LWTBL_GET_AC2_CTB_CRT_CTB__CRDB(reg32)                                                           READ_FIELD((reg32), WF_LWTBL_AC2_CTB_CRT_CTB__CRDB)
#define WF_LWTBL_GET_AC3_CTT_CDT_CRB__CTODB(reg32)                                                          READ_FIELD((reg32), WF_LWTBL_AC3_CTT_CDT_CRB__CTODB)
#define WF_LWTBL_GET_AC3_CTB_CRT_CTB__(reg32)                                                               READ_FIELD((reg32), WF_LWTBL_AC3_CTB_CRT_CTB__)
#define WF_LWTBL_GET_RELATED_IDX0(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RELATED_IDX0)
#define WF_LWTBL_GET_RELATED_BAND0(reg32)                                                                   READ_FIELD((reg32), WF_LWTBL_RELATED_BAND0)
#define WF_LWTBL_GET_PRIMARY_MLD_BAND(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_PRIMARY_MLD_BAND)
#define WF_LWTBL_GET_RELATED_IDX1(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RELATED_IDX1)
#define WF_LWTBL_GET_RELATED_BAND1(reg32)                                                                   READ_FIELD((reg32), WF_LWTBL_RELATED_BAND1)
#define WF_LWTBL_GET_SECONDARY_MLD_BAND(reg32)                                                              READ_FIELD((reg32), WF_LWTBL_SECONDARY_MLD_BAND)
#define WF_LWTBL_GET_DISPATCH_POLICY0(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY0)
#define WF_LWTBL_GET_DISPATCH_POLICY1(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY1)
#define WF_LWTBL_GET_DISPATCH_POLICY2(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY2)
#define WF_LWTBL_GET_DISPATCH_POLICY3(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY3)
#define WF_LWTBL_GET_DISPATCH_POLICY4(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY4)
#define WF_LWTBL_GET_DISPATCH_POLICY5(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY5)
#define WF_LWTBL_GET_DISPATCH_POLICY6(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY6)
#define WF_LWTBL_GET_DISPATCH_POLICY7(reg32)                                                                READ_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY7)
#define WF_LWTBL_GET_OWN_MLD_ID(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_OWN_MLD_ID)
#define WF_LWTBL_GET_EMLSR0(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLSR0)
#define WF_LWTBL_GET_EMLMR0(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLMR0)
#define WF_LWTBL_GET_EMLSR1(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLSR1)
#define WF_LWTBL_GET_EMLMR1(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLMR1)
#define WF_LWTBL_GET_EMLSR2(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLSR2)
#define WF_LWTBL_GET_EMLMR2(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_EMLMR2)
#define WF_LWTBL_GET_STR_BITMAP(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_STR_BITMAP)
#define WF_LWTBL_GET_DISPATCH_ORDER(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_DISPATCH_ORDER)
#define WF_LWTBL_GET_DISPATCH_RATIO(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_DISPATCH_RATIO)
#define WF_LWTBL_GET_LINK_MGF(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_LINK_MGF)
#define WF_LWTBL_GET_BFTX_TB(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_BFTX_TB)
#define WF_LWTBL_GET_DROP(reg32)                                                                            READ_FIELD((reg32), WF_LWTBL_DROP)
#define WF_LWTBL_GET_CASCAD(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_CASCAD)
#define WF_LWTBL_GET_ALL_ACK(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_ALL_ACK)
#define WF_LWTBL_GET_MPDU_SIZE(reg32)                                                                       READ_FIELD((reg32), WF_LWTBL_MPDU_SIZE)
#define WF_LWTBL_GET_RXD_DUP_MODE(reg32)                                                                    READ_FIELD((reg32), WF_LWTBL_RXD_DUP_MODE)
#define WF_LWTBL_GET_ACK_EN(reg32)                                                                          READ_FIELD((reg32), WF_LWTBL_ACK_EN)
#define WF_LWTBL_GET_OM_INFO(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_OM_INFO)
#define WF_LWTBL_GET_OM_INFO_EHT(reg32)                                                                     READ_FIELD((reg32), WF_LWTBL_OM_INFO_EHT)
#define WF_LWTBL_GET_RXD_DUP_FOR_OM_CHG(reg32)                                                              READ_FIELD((reg32), WF_LWTBL_RXD_DUP_FOR_OM_CHG)
#define WF_LWTBL_GET_RXD_DUP_WHITE_LIST(reg32)                                                              READ_FIELD((reg32), WF_LWTBL_RXD_DUP_WHITE_LIST)
#define WF_LWTBL_GET_USER_RSSI(reg32)                                                                       READ_FIELD((reg32), WF_LWTBL_USER_RSSI)
#define WF_LWTBL_GET_USER_SNR(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_USER_SNR)
#define WF_LWTBL_GET_RAPID_REACTION_RATE(reg32)                                                             READ_FIELD((reg32), WF_LWTBL_RAPID_REACTION_RATE)
#define WF_LWTBL_GET_HT_AMSDU(reg32)                                                                        READ_FIELD((reg32), WF_LWTBL_HT_AMSDU)
#define WF_LWTBL_GET_AMSDU_CROSS_LG(reg32)                                                                  READ_FIELD((reg32), WF_LWTBL_AMSDU_CROSS_LG)
#define WF_LWTBL_GET_RESP_RCPI0(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_RESP_RCPI0)
#define WF_LWTBL_GET_RESP_RCPI1(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_RESP_RCPI1)
#define WF_LWTBL_GET_RESP_RCPI2(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_RESP_RCPI2)
#define WF_LWTBL_GET_RESP_RCPI3(reg32)                                                                      READ_FIELD((reg32), WF_LWTBL_RESP_RCPI3)
#define WF_LWTBL_GET_SNR_RX0(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SNR_RX0)
#define WF_LWTBL_GET_SNR_RX1(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SNR_RX1)
#define WF_LWTBL_GET_SNR_RX2(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SNR_RX2)
#define WF_LWTBL_GET_SNR_RX3(reg32)                                                                         READ_FIELD((reg32), WF_LWTBL_SNR_RX3)

#define WF_LWTBL_SET_PEER_LINK_ADDRESS_47_32_(reg32, val32)                                                 WRITE_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_47_32_, val32)
#define WF_LWTBL_SET_MUAR(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_MUAR, val32)
#define WF_LWTBL_SET_RCA1(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_RCA1, val32)
#define WF_LWTBL_SET_KID(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_KID, val32)
#define WF_LWTBL_SET_RCID(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_RCID, val32)
#define WF_LWTBL_SET_BAND(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_BAND, val32)
#define WF_LWTBL_SET_RV(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_RV, val32)
#define WF_LWTBL_SET_RCA2(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_RCA2, val32)
#define WF_LWTBL_SET_WPI_FLAG(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_WPI_FLAG, val32)
#define WF_LWTBL_SET_PEER_LINK_ADDRESS_31_0_(reg32, val32)                                                  WRITE_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_31_0_, val32)
#define WF_LWTBL_SET_AID(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_AID, val32)
#define WF_LWTBL_SET_GID_SU(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_GID_SU, val32)
#define WF_LWTBL_SET_SPP_EN(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_SPP_EN, val32)
#define WF_LWTBL_SET_WPI_EVEN(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_WPI_EVEN, val32)
#define WF_LWTBL_SET_AAD_OM(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_AAD_OM, val32)
#define WF_LWTBL_SET_CIPHER_SUIT_PGTK(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_PGTK, val32)
#define WF_LWTBL_SET_FD(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_FD, val32)
#define WF_LWTBL_SET_TD(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_TD, val32)
#define WF_LWTBL_SET_SW(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_SW, val32)
#define WF_LWTBL_SET_UL(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_UL, val32)
#define WF_LWTBL_SET_TX_PS(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_TX_PS, val32)
#define WF_LWTBL_SET_QOS(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_QOS, val32)
#define WF_LWTBL_SET_HT(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_HT, val32)
#define WF_LWTBL_SET_VHT(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_VHT, val32)
#define WF_LWTBL_SET_HE(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_HE, val32)
#define WF_LWTBL_SET_EHT(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_EHT, val32)
#define WF_LWTBL_SET_MESH(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_MESH, val32)
#define WF_LWTBL_SET_WMM_Q(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_WMM_Q, val32)
#define WF_LWTBL_SET_EHT_SIG_MCS(reg32, val32)                                                              WRITE_FIELD((reg32), WF_LWTBL_EHT_SIG_MCS, val32)
#define WF_LWTBL_SET_HDRT_MODE(reg32, val32)                                                                WRITE_FIELD((reg32), WF_LWTBL_HDRT_MODE, val32)
#define WF_LWTBL_SET_BEAM_CHG(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_BEAM_CHG, val32)
#define WF_LWTBL_SET_EHT_LTF_SYM_NUM_OPT(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_EHT_LTF_SYM_NUM_OPT, val32)
#define WF_LWTBL_SET_PFMU_IDX(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_PFMU_IDX, val32)
#define WF_LWTBL_SET_ULPF_IDX(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_ULPF_IDX, val32)
#define WF_LWTBL_SET_RIBF(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_RIBF, val32)
#define WF_LWTBL_SET_ULPF(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_ULPF, val32)
#define WF_LWTBL_SET_BYPASS_TXSMM(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BYPASS_TXSMM, val32)
#define WF_LWTBL_SET_TBF_HT(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_TBF_HT, val32)
#define WF_LWTBL_SET_TBF_VHT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_TBF_VHT, val32)
#define WF_LWTBL_SET_TBF_HE(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_TBF_HE, val32)
#define WF_LWTBL_SET_TBF_EHT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_TBF_EHT, val32)
#define WF_LWTBL_SET_IGN_FBK(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_IGN_FBK, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE0(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE0, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE1(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE1, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE2(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE2, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE3(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE3, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE4(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE4, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE5(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE5, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE6(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE6, val32)
#define WF_LWTBL_SET_NEGOTIATED_WINSIZE7(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE7, val32)
#define WF_LWTBL_SET_PE(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_PE, val32)
#define WF_LWTBL_SET_DIS_RHTR(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_DIS_RHTR, val32)
#define WF_LWTBL_SET_LDPC_HT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_LDPC_HT, val32)
#define WF_LWTBL_SET_LDPC_VHT(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_LDPC_VHT, val32)
#define WF_LWTBL_SET_LDPC_HE(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_LDPC_HE, val32)
#define WF_LWTBL_SET_LDPC_EHT(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_LDPC_EHT, val32)
#define WF_LWTBL_SET_BA_MODE(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_BA_MODE, val32)
#define WF_LWTBL_SET_AF(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_AF, val32)
#define WF_LWTBL_SET_AF_HE(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_AF_HE, val32)
#define WF_LWTBL_SET_RTS(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_RTS, val32)
#define WF_LWTBL_SET_SMPS(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_SMPS, val32)
#define WF_LWTBL_SET_DYN_BW(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_DYN_BW, val32)
#define WF_LWTBL_SET_MMSS(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_MMSS, val32)
#define WF_LWTBL_SET_USR(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_USR, val32)
#define WF_LWTBL_SET_SR_R(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_SR_R, val32)
#define WF_LWTBL_SET_SR_ABORT(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_SR_ABORT, val32)
#define WF_LWTBL_SET_TX_POWER_OFFSET(reg32, val32)                                                          WRITE_FIELD((reg32), WF_LWTBL_TX_POWER_OFFSET, val32)
#define WF_LWTBL_SET_LTF_EHT(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_LTF_EHT, val32)
#define WF_LWTBL_SET_GI_EHT(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_GI_EHT, val32)
#define WF_LWTBL_SET_DOPPL(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_DOPPL, val32)
#define WF_LWTBL_SET_TXOP_PS_CAP(reg32, val32)                                                              WRITE_FIELD((reg32), WF_LWTBL_TXOP_PS_CAP, val32)
#define WF_LWTBL_SET_DU_I_PSM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_DU_I_PSM, val32)
#define WF_LWTBL_SET_I_PSM(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_I_PSM, val32)
#define WF_LWTBL_SET_PSM(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_PSM, val32)
#define WF_LWTBL_SET_SKIP_TX(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SKIP_TX, val32)
#define WF_LWTBL_SET_CBRN(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_CBRN, val32)
#define WF_LWTBL_SET_DBNSS_EN(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_DBNSS_EN, val32)
#define WF_LWTBL_SET_BAF_EN(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_BAF_EN, val32)
#define WF_LWTBL_SET_RDGBA(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RDGBA, val32)
#define WF_LWTBL_SET_R(reg32, val32)                                                                        WRITE_FIELD((reg32), WF_LWTBL_R, val32)
#define WF_LWTBL_SET_SPE_IDX(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SPE_IDX, val32)
#define WF_LWTBL_SET_G2(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_G2, val32)
#define WF_LWTBL_SET_G4(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_G4, val32)
#define WF_LWTBL_SET_G8(reg32, val32)                                                                       WRITE_FIELD((reg32), WF_LWTBL_G8, val32)
#define WF_LWTBL_SET_G16(reg32, val32)                                                                      WRITE_FIELD((reg32), WF_LWTBL_G16, val32)
#define WF_LWTBL_SET_G2_LTF(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_G2_LTF, val32)
#define WF_LWTBL_SET_G4_LTF(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_G4_LTF, val32)
#define WF_LWTBL_SET_G8_LTF(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_G8_LTF, val32)
#define WF_LWTBL_SET_G16_LTF(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_G16_LTF, val32)
#define WF_LWTBL_SET_G2_HE(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_G2_HE, val32)
#define WF_LWTBL_SET_G4_HE(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_G4_HE, val32)
#define WF_LWTBL_SET_G8_HE(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_G8_HE, val32)
#define WF_LWTBL_SET_G16_HE(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_G16_HE, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE0(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE0, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE1(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE1, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE2(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE2, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE3(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE3, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE4(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE4, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE5(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE5, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE6(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE6, val32)
#define WF_LWTBL_SET_BA_WIN_SIZE7(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE7, val32)
#define WF_LWTBL_SET_AC0_RTS_FAIL_CNT(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_AC0_RTS_FAIL_CNT, val32)
#define WF_LWTBL_SET_AC1_RTS_FAIL_CNT(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_AC1_RTS_FAIL_CNT, val32)
#define WF_LWTBL_SET_AC2_RTS_FAIL_CNT(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_AC2_RTS_FAIL_CNT, val32)
#define WF_LWTBL_SET_AC3_RTS_FAIL_CNT(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_AC3_RTS_FAIL_CNT, val32)
#define WF_LWTBL_SET_PARTIAL_AID(reg32, val32)                                                              WRITE_FIELD((reg32), WF_LWTBL_PARTIAL_AID, val32)
#define WF_LWTBL_SET_CHK_PER(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_CHK_PER, val32)
#define WF_LWTBL_SET_RX_AVG_MPDU_SIZE(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_RX_AVG_MPDU_SIZE, val32)
#define WF_LWTBL_SET_PRITX_SW_MODE(reg32, val32)                                                            WRITE_FIELD((reg32), WF_LWTBL_PRITX_SW_MODE, val32)
#define WF_LWTBL_SET_PRITX_ERSU(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_PRITX_ERSU, val32)
#define WF_LWTBL_SET_PRITX_PLR(reg32, val32)                                                                WRITE_FIELD((reg32), WF_LWTBL_PRITX_PLR, val32)
#define WF_LWTBL_SET_PRITX_DCM(reg32, val32)                                                                WRITE_FIELD((reg32), WF_LWTBL_PRITX_DCM, val32)
#define WF_LWTBL_SET_PRITX_ER106T(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_PRITX_ER106T, val32)
#define WF_LWTBL_SET_FCAP(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_FCAP, val32)
#define WF_LWTBL_SET_MPDU_FAIL_CNT(reg32, val32)                                                            WRITE_FIELD((reg32), WF_LWTBL_MPDU_FAIL_CNT, val32)
#define WF_LWTBL_SET_MPDU_OK_CNT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_LWTBL_MPDU_OK_CNT, val32)
#define WF_LWTBL_SET_RATE_IDX(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_RATE_IDX, val32)
#define WF_LWTBL_SET_RATE1(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE1, val32)
#define WF_LWTBL_SET_RATE2(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE2, val32)
#define WF_LWTBL_SET_RATE3(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE3, val32)
#define WF_LWTBL_SET_RATE4(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE4, val32)
#define WF_LWTBL_SET_RATE5(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE5, val32)
#define WF_LWTBL_SET_RATE6(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE6, val32)
#define WF_LWTBL_SET_RATE7(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE7, val32)
#define WF_LWTBL_SET_RATE8(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_LWTBL_RATE8, val32)
#define WF_LWTBL_SET_RATE1_TX_CNT(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RATE1_TX_CNT, val32)
#define WF_LWTBL_SET_CIPHER_SUIT_IGTK(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_IGTK, val32)
#define WF_LWTBL_SET_CIPHER_SUIT_BIGTK(reg32, val32)                                                        WRITE_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_BIGTK, val32)
#define WF_LWTBL_SET_RATE1_FAIL_CNT(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_RATE1_FAIL_CNT, val32)
#define WF_LWTBL_SET_RATE2_OK_CNT(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RATE2_OK_CNT, val32)
#define WF_LWTBL_SET_RATE3_OK_CNT(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RATE3_OK_CNT, val32)
#define WF_LWTBL_SET_CURRENT_BW_TX_CNT(reg32, val32)                                                        WRITE_FIELD((reg32), WF_LWTBL_CURRENT_BW_TX_CNT, val32)
#define WF_LWTBL_SET_CURRENT_BW_FAIL_CNT(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_CURRENT_BW_FAIL_CNT, val32)
#define WF_LWTBL_SET_OTHER_BW_TX_CNT(reg32, val32)                                                          WRITE_FIELD((reg32), WF_LWTBL_OTHER_BW_TX_CNT, val32)
#define WF_LWTBL_SET_OTHER_BW_FAIL_CNT(reg32, val32)                                                        WRITE_FIELD((reg32), WF_LWTBL_OTHER_BW_FAIL_CNT, val32)
#define WF_LWTBL_SET_RTS_OK_CNT(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_RTS_OK_CNT, val32)
#define WF_LWTBL_SET_RTS_FAIL_CNT(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RTS_FAIL_CNT, val32)
#define WF_LWTBL_SET_DATA_RETRY_CNT(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_DATA_RETRY_CNT, val32)
#define WF_LWTBL_SET_MGNT_RETRY_CNT(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_MGNT_RETRY_CNT, val32)
#define WF_LWTBL_SET_AC0_CTT_CDT_CRB__CTT(reg32, val32)                                                     WRITE_FIELD((reg32), WF_LWTBL_AC0_CTT_CDT_CRB__CTT, val32)
#define WF_LWTBL_SET_AC0_CTB_CRT_CTB__CRT(reg32, val32)                                                     WRITE_FIELD((reg32), WF_LWTBL_AC0_CTB_CRT_CTB__CRT, val32)
#define WF_LWTBL_SET_AC1_CTT_CDT_CRB__CTMC(reg32, val32)                                                    WRITE_FIELD((reg32), WF_LWTBL_AC1_CTT_CDT_CRB__CTMC, val32)
#define WF_LWTBL_SET_AC1_CTB_CRT_CTB__CRMC(reg32, val32)                                                    WRITE_FIELD((reg32), WF_LWTBL_AC1_CTB_CRT_CTB__CRMC, val32)
#define WF_LWTBL_SET_AC2_CTT_CDT_CRB__CTDB(reg32, val32)                                                    WRITE_FIELD((reg32), WF_LWTBL_AC2_CTT_CDT_CRB__CTDB, val32)
#define WF_LWTBL_SET_AC2_CTB_CRT_CTB__CRDB(reg32, val32)                                                    WRITE_FIELD((reg32), WF_LWTBL_AC2_CTB_CRT_CTB__CRDB, val32)
#define WF_LWTBL_SET_AC3_CTT_CDT_CRB__CTODB(reg32, val32)                                                   WRITE_FIELD((reg32), WF_LWTBL_AC3_CTT_CDT_CRB__CTODB, val32)
#define WF_LWTBL_SET_AC3_CTB_CRT_CTB__(reg32, val32)                                                        WRITE_FIELD((reg32), WF_LWTBL_AC3_CTB_CRT_CTB__, val32)
#define WF_LWTBL_SET_RELATED_IDX0(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RELATED_IDX0, val32)
#define WF_LWTBL_SET_RELATED_BAND0(reg32, val32)                                                            WRITE_FIELD((reg32), WF_LWTBL_RELATED_BAND0, val32)
#define WF_LWTBL_SET_PRIMARY_MLD_BAND(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_PRIMARY_MLD_BAND, val32)
#define WF_LWTBL_SET_RELATED_IDX1(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RELATED_IDX1, val32)
#define WF_LWTBL_SET_RELATED_BAND1(reg32, val32)                                                            WRITE_FIELD((reg32), WF_LWTBL_RELATED_BAND1, val32)
#define WF_LWTBL_SET_SECONDARY_MLD_BAND(reg32, val32)                                                       WRITE_FIELD((reg32), WF_LWTBL_SECONDARY_MLD_BAND, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY0(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY0, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY1(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY1, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY2(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY2, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY3(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY3, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY4(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY4, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY5(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY5, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY6(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY6, val32)
#define WF_LWTBL_SET_DISPATCH_POLICY7(reg32, val32)                                                         WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY7, val32)
#define WF_LWTBL_SET_OWN_MLD_ID(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_OWN_MLD_ID, val32)
#define WF_LWTBL_SET_EMLSR0(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLSR0, val32)
#define WF_LWTBL_SET_EMLMR0(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLMR0, val32)
#define WF_LWTBL_SET_EMLSR1(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLSR1, val32)
#define WF_LWTBL_SET_EMLMR1(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLMR1, val32)
#define WF_LWTBL_SET_EMLSR2(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLSR2, val32)
#define WF_LWTBL_SET_EMLMR2(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_EMLMR2, val32)
#define WF_LWTBL_SET_STR_BITMAP(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_STR_BITMAP, val32)
#define WF_LWTBL_SET_DISPATCH_ORDER(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_ORDER, val32)
#define WF_LWTBL_SET_DISPATCH_RATIO(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_DISPATCH_RATIO, val32)
#define WF_LWTBL_SET_LINK_MGF(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_LINK_MGF, val32)
#define WF_LWTBL_SET_BFTX_TB(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_BFTX_TB, val32)
#define WF_LWTBL_SET_DROP(reg32, val32)                                                                     WRITE_FIELD((reg32), WF_LWTBL_DROP, val32)
#define WF_LWTBL_SET_CASCAD(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_CASCAD, val32)
#define WF_LWTBL_SET_ALL_ACK(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_ALL_ACK, val32)
#define WF_LWTBL_SET_MPDU_SIZE(reg32, val32)                                                                WRITE_FIELD((reg32), WF_LWTBL_MPDU_SIZE, val32)
#define WF_LWTBL_SET_RXD_DUP_MODE(reg32, val32)                                                             WRITE_FIELD((reg32), WF_LWTBL_RXD_DUP_MODE, val32)
#define WF_LWTBL_SET_ACK_EN(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_LWTBL_ACK_EN, val32)
#define WF_LWTBL_SET_OM_INFO(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_OM_INFO, val32)
#define WF_LWTBL_SET_OM_INFO_EHT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_LWTBL_OM_INFO_EHT, val32)
#define WF_LWTBL_SET_RXD_DUP_FOR_OM_CHG(reg32, val32)                                                       WRITE_FIELD((reg32), WF_LWTBL_RXD_DUP_FOR_OM_CHG, val32)
#define WF_LWTBL_SET_RXD_DUP_WHITE_LIST(reg32, val32)                                                       WRITE_FIELD((reg32), WF_LWTBL_RXD_DUP_WHITE_LIST, val32)
#define WF_LWTBL_SET_USER_RSSI(reg32, val32)                                                                WRITE_FIELD((reg32), WF_LWTBL_USER_RSSI, val32)
#define WF_LWTBL_SET_USER_SNR(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_USER_SNR, val32)
#define WF_LWTBL_SET_RAPID_REACTION_RATE(reg32, val32)                                                      WRITE_FIELD((reg32), WF_LWTBL_RAPID_REACTION_RATE, val32)
#define WF_LWTBL_SET_HT_AMSDU(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_LWTBL_HT_AMSDU, val32)
#define WF_LWTBL_SET_AMSDU_CROSS_LG(reg32, val32)                                                           WRITE_FIELD((reg32), WF_LWTBL_AMSDU_CROSS_LG, val32)
#define WF_LWTBL_SET_RESP_RCPI0(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_RESP_RCPI0, val32)
#define WF_LWTBL_SET_RESP_RCPI1(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_RESP_RCPI1, val32)
#define WF_LWTBL_SET_RESP_RCPI2(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_RESP_RCPI2, val32)
#define WF_LWTBL_SET_RESP_RCPI3(reg32, val32)                                                               WRITE_FIELD((reg32), WF_LWTBL_RESP_RCPI3, val32)
#define WF_LWTBL_SET_SNR_RX0(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SNR_RX0, val32)
#define WF_LWTBL_SET_SNR_RX1(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SNR_RX1, val32)
#define WF_LWTBL_SET_SNR_RX2(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SNR_RX2, val32)
#define WF_LWTBL_SET_SNR_RX3(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_LWTBL_SNR_RX3, val32)

#define WF_LWTBL_CLR_PEER_LINK_ADDRESS_47_32_(reg32)                                                        CLEAR_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_47_32_)
#define WF_LWTBL_CLR_MUAR(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_MUAR)
#define WF_LWTBL_CLR_RCA1(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_RCA1)
#define WF_LWTBL_CLR_KID(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_KID)
#define WF_LWTBL_CLR_RCID(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_RCID)
#define WF_LWTBL_CLR_BAND(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_BAND)
#define WF_LWTBL_CLR_RV(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_RV)
#define WF_LWTBL_CLR_RCA2(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_RCA2)
#define WF_LWTBL_CLR_WPI_FLAG(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_WPI_FLAG)
#define WF_LWTBL_CLR_PEER_LINK_ADDRESS_31_0_(reg32)                                                         CLEAR_FIELD((reg32), WF_LWTBL_PEER_LINK_ADDRESS_31_0_)
#define WF_LWTBL_CLR_AID(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_AID)
#define WF_LWTBL_CLR_GID_SU(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_GID_SU)
#define WF_LWTBL_CLR_SPP_EN(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_SPP_EN)
#define WF_LWTBL_CLR_WPI_EVEN(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_WPI_EVEN)
#define WF_LWTBL_CLR_AAD_OM(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_AAD_OM)
#define WF_LWTBL_CLR_CIPHER_SUIT_PGTK(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_PGTK)
#define WF_LWTBL_CLR_FD(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_FD)
#define WF_LWTBL_CLR_TD(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_TD)
#define WF_LWTBL_CLR_SW(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_SW)
#define WF_LWTBL_CLR_UL(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_UL)
#define WF_LWTBL_CLR_TX_PS(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_TX_PS)
#define WF_LWTBL_CLR_QOS(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_QOS)
#define WF_LWTBL_CLR_HT(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_HT)
#define WF_LWTBL_CLR_VHT(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_VHT)
#define WF_LWTBL_CLR_HE(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_HE)
#define WF_LWTBL_CLR_EHT(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_EHT)
#define WF_LWTBL_CLR_MESH(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_MESH)
#define WF_LWTBL_CLR_WMM_Q(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_WMM_Q)
#define WF_LWTBL_CLR_EHT_SIG_MCS(reg32)                                                                     CLEAR_FIELD((reg32), WF_LWTBL_EHT_SIG_MCS)
#define WF_LWTBL_CLR_HDRT_MODE(reg32)                                                                       CLEAR_FIELD((reg32), WF_LWTBL_HDRT_MODE)
#define WF_LWTBL_CLR_BEAM_CHG(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_BEAM_CHG)
#define WF_LWTBL_CLR_EHT_LTF_SYM_NUM_OPT(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_EHT_LTF_SYM_NUM_OPT)
#define WF_LWTBL_CLR_PFMU_IDX(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_PFMU_IDX)
#define WF_LWTBL_CLR_ULPF_IDX(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_ULPF_IDX)
#define WF_LWTBL_CLR_RIBF(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_RIBF)
#define WF_LWTBL_CLR_ULPF(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_ULPF)
#define WF_LWTBL_CLR_BYPASS_TXSMM(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BYPASS_TXSMM)
#define WF_LWTBL_CLR_TBF_HT(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_TBF_HT)
#define WF_LWTBL_CLR_TBF_VHT(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_TBF_VHT)
#define WF_LWTBL_CLR_TBF_HE(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_TBF_HE)
#define WF_LWTBL_CLR_TBF_EHT(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_TBF_EHT)
#define WF_LWTBL_CLR_IGN_FBK(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_IGN_FBK)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE0(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE0)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE1(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE1)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE2(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE2)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE3(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE3)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE4(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE4)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE5(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE5)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE6(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE6)
#define WF_LWTBL_CLR_NEGOTIATED_WINSIZE7(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_NEGOTIATED_WINSIZE7)
#define WF_LWTBL_CLR_PE(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_PE)
#define WF_LWTBL_CLR_DIS_RHTR(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_DIS_RHTR)
#define WF_LWTBL_CLR_LDPC_HT(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_LDPC_HT)
#define WF_LWTBL_CLR_LDPC_VHT(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_LDPC_VHT)
#define WF_LWTBL_CLR_LDPC_HE(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_LDPC_HE)
#define WF_LWTBL_CLR_LDPC_EHT(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_LDPC_EHT)
#define WF_LWTBL_CLR_BA_MODE(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_BA_MODE)
#define WF_LWTBL_CLR_AF(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_AF)
#define WF_LWTBL_CLR_AF_HE(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_AF_HE)
#define WF_LWTBL_CLR_RTS(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_RTS)
#define WF_LWTBL_CLR_SMPS(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_SMPS)
#define WF_LWTBL_CLR_DYN_BW(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_DYN_BW)
#define WF_LWTBL_CLR_MMSS(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_MMSS)
#define WF_LWTBL_CLR_USR(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_USR)
#define WF_LWTBL_CLR_SR_R(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_SR_R)
#define WF_LWTBL_CLR_SR_ABORT(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_SR_ABORT)
#define WF_LWTBL_CLR_TX_POWER_OFFSET(reg32)                                                                 CLEAR_FIELD((reg32), WF_LWTBL_TX_POWER_OFFSET)
#define WF_LWTBL_CLR_LTF_EHT(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_LTF_EHT)
#define WF_LWTBL_CLR_GI_EHT(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_GI_EHT)
#define WF_LWTBL_CLR_DOPPL(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_DOPPL)
#define WF_LWTBL_CLR_TXOP_PS_CAP(reg32)                                                                     CLEAR_FIELD((reg32), WF_LWTBL_TXOP_PS_CAP)
#define WF_LWTBL_CLR_DU_I_PSM(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_DU_I_PSM)
#define WF_LWTBL_CLR_I_PSM(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_I_PSM)
#define WF_LWTBL_CLR_PSM(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_PSM)
#define WF_LWTBL_CLR_SKIP_TX(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SKIP_TX)
#define WF_LWTBL_CLR_CBRN(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_CBRN)
#define WF_LWTBL_CLR_DBNSS_EN(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_DBNSS_EN)
#define WF_LWTBL_CLR_BAF_EN(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_BAF_EN)
#define WF_LWTBL_CLR_RDGBA(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RDGBA)
#define WF_LWTBL_CLR_R(reg32)                                                                               CLEAR_FIELD((reg32), WF_LWTBL_R)
#define WF_LWTBL_CLR_SPE_IDX(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SPE_IDX)
#define WF_LWTBL_CLR_G2(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_G2)
#define WF_LWTBL_CLR_G4(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_G4)
#define WF_LWTBL_CLR_G8(reg32)                                                                              CLEAR_FIELD((reg32), WF_LWTBL_G8)
#define WF_LWTBL_CLR_G16(reg32)                                                                             CLEAR_FIELD((reg32), WF_LWTBL_G16)
#define WF_LWTBL_CLR_G2_LTF(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_G2_LTF)
#define WF_LWTBL_CLR_G4_LTF(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_G4_LTF)
#define WF_LWTBL_CLR_G8_LTF(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_G8_LTF)
#define WF_LWTBL_CLR_G16_LTF(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_G16_LTF)
#define WF_LWTBL_CLR_G2_HE(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_G2_HE)
#define WF_LWTBL_CLR_G4_HE(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_G4_HE)
#define WF_LWTBL_CLR_G8_HE(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_G8_HE)
#define WF_LWTBL_CLR_G16_HE(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_G16_HE)
#define WF_LWTBL_CLR_BA_WIN_SIZE0(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE0)
#define WF_LWTBL_CLR_BA_WIN_SIZE1(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE1)
#define WF_LWTBL_CLR_BA_WIN_SIZE2(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE2)
#define WF_LWTBL_CLR_BA_WIN_SIZE3(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE3)
#define WF_LWTBL_CLR_BA_WIN_SIZE4(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE4)
#define WF_LWTBL_CLR_BA_WIN_SIZE5(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE5)
#define WF_LWTBL_CLR_BA_WIN_SIZE6(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE6)
#define WF_LWTBL_CLR_BA_WIN_SIZE7(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_BA_WIN_SIZE7)
#define WF_LWTBL_CLR_AC0_RTS_FAIL_CNT(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_AC0_RTS_FAIL_CNT)
#define WF_LWTBL_CLR_AC1_RTS_FAIL_CNT(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_AC1_RTS_FAIL_CNT)
#define WF_LWTBL_CLR_AC2_RTS_FAIL_CNT(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_AC2_RTS_FAIL_CNT)
#define WF_LWTBL_CLR_AC3_RTS_FAIL_CNT(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_AC3_RTS_FAIL_CNT)
#define WF_LWTBL_CLR_PARTIAL_AID(reg32)                                                                     CLEAR_FIELD((reg32), WF_LWTBL_PARTIAL_AID)
#define WF_LWTBL_CLR_CHK_PER(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_CHK_PER)
#define WF_LWTBL_CLR_RX_AVG_MPDU_SIZE(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_RX_AVG_MPDU_SIZE)
#define WF_LWTBL_CLR_PRITX_SW_MODE(reg32)                                                                   CLEAR_FIELD((reg32), WF_LWTBL_PRITX_SW_MODE)
#define WF_LWTBL_CLR_PRITX_ERSU(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_PRITX_ERSU)
#define WF_LWTBL_CLR_PRITX_PLR(reg32)                                                                       CLEAR_FIELD((reg32), WF_LWTBL_PRITX_PLR)
#define WF_LWTBL_CLR_PRITX_DCM(reg32)                                                                       CLEAR_FIELD((reg32), WF_LWTBL_PRITX_DCM)
#define WF_LWTBL_CLR_PRITX_ER106T(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_PRITX_ER106T)
#define WF_LWTBL_CLR_FCAP(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_FCAP)
#define WF_LWTBL_CLR_MPDU_FAIL_CNT(reg32)                                                                   CLEAR_FIELD((reg32), WF_LWTBL_MPDU_FAIL_CNT)
#define WF_LWTBL_CLR_MPDU_OK_CNT(reg32)                                                                     CLEAR_FIELD((reg32), WF_LWTBL_MPDU_OK_CNT)
#define WF_LWTBL_CLR_RATE_IDX(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_RATE_IDX)
#define WF_LWTBL_CLR_RATE1(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE1)
#define WF_LWTBL_CLR_RATE2(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE2)
#define WF_LWTBL_CLR_RATE3(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE3)
#define WF_LWTBL_CLR_RATE4(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE4)
#define WF_LWTBL_CLR_RATE5(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE5)
#define WF_LWTBL_CLR_RATE6(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE6)
#define WF_LWTBL_CLR_RATE7(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE7)
#define WF_LWTBL_CLR_RATE8(reg32)                                                                           CLEAR_FIELD((reg32), WF_LWTBL_RATE8)
#define WF_LWTBL_CLR_RATE1_TX_CNT(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RATE1_TX_CNT)
#define WF_LWTBL_CLR_CIPHER_SUIT_IGTK(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_IGTK)
#define WF_LWTBL_CLR_CIPHER_SUIT_BIGTK(reg32)                                                               CLEAR_FIELD((reg32), WF_LWTBL_CIPHER_SUIT_BIGTK)
#define WF_LWTBL_CLR_RATE1_FAIL_CNT(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_RATE1_FAIL_CNT)
#define WF_LWTBL_CLR_RATE2_OK_CNT(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RATE2_OK_CNT)
#define WF_LWTBL_CLR_RATE3_OK_CNT(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RATE3_OK_CNT)
#define WF_LWTBL_CLR_CURRENT_BW_TX_CNT(reg32)                                                               CLEAR_FIELD((reg32), WF_LWTBL_CURRENT_BW_TX_CNT)
#define WF_LWTBL_CLR_CURRENT_BW_FAIL_CNT(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_CURRENT_BW_FAIL_CNT)
#define WF_LWTBL_CLR_OTHER_BW_TX_CNT(reg32)                                                                 CLEAR_FIELD((reg32), WF_LWTBL_OTHER_BW_TX_CNT)
#define WF_LWTBL_CLR_OTHER_BW_FAIL_CNT(reg32)                                                               CLEAR_FIELD((reg32), WF_LWTBL_OTHER_BW_FAIL_CNT)
#define WF_LWTBL_CLR_RTS_OK_CNT(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_RTS_OK_CNT)
#define WF_LWTBL_CLR_RTS_FAIL_CNT(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RTS_FAIL_CNT)
#define WF_LWTBL_CLR_DATA_RETRY_CNT(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_DATA_RETRY_CNT)
#define WF_LWTBL_CLR_MGNT_RETRY_CNT(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_MGNT_RETRY_CNT)
#define WF_LWTBL_CLR_AC0_CTT_CDT_CRB__CTT(reg32)                                                            CLEAR_FIELD((reg32), WF_LWTBL_AC0_CTT_CDT_CRB__CTT)
#define WF_LWTBL_CLR_AC0_CTB_CRT_CTB__CRT(reg32)                                                            CLEAR_FIELD((reg32), WF_LWTBL_AC0_CTB_CRT_CTB__CRT)
#define WF_LWTBL_CLR_AC1_CTT_CDT_CRB__CTMC(reg32)                                                           CLEAR_FIELD((reg32), WF_LWTBL_AC1_CTT_CDT_CRB__CTMC)
#define WF_LWTBL_CLR_AC1_CTB_CRT_CTB__CRMC(reg32)                                                           CLEAR_FIELD((reg32), WF_LWTBL_AC1_CTB_CRT_CTB__CRMC)
#define WF_LWTBL_CLR_AC2_CTT_CDT_CRB__CTDB(reg32)                                                           CLEAR_FIELD((reg32), WF_LWTBL_AC2_CTT_CDT_CRB__CTDB)
#define WF_LWTBL_CLR_AC2_CTB_CRT_CTB__CRDB(reg32)                                                           CLEAR_FIELD((reg32), WF_LWTBL_AC2_CTB_CRT_CTB__CRDB)
#define WF_LWTBL_CLR_AC3_CTT_CDT_CRB__CTODB(reg32)                                                          CLEAR_FIELD((reg32), WF_LWTBL_AC3_CTT_CDT_CRB__CTODB)
#define WF_LWTBL_CLR_AC3_CTB_CRT_CTB__(reg32)                                                               CLEAR_FIELD((reg32), WF_LWTBL_AC3_CTB_CRT_CTB__)
#define WF_LWTBL_CLR_RELATED_IDX0(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RELATED_IDX0)
#define WF_LWTBL_CLR_RELATED_BAND0(reg32)                                                                   CLEAR_FIELD((reg32), WF_LWTBL_RELATED_BAND0)
#define WF_LWTBL_CLR_PRIMARY_MLD_BAND(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_PRIMARY_MLD_BAND)
#define WF_LWTBL_CLR_RELATED_IDX1(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RELATED_IDX1)
#define WF_LWTBL_CLR_RELATED_BAND1(reg32)                                                                   CLEAR_FIELD((reg32), WF_LWTBL_RELATED_BAND1)
#define WF_LWTBL_CLR_SECONDARY_MLD_BAND(reg32)                                                              CLEAR_FIELD((reg32), WF_LWTBL_SECONDARY_MLD_BAND)
#define WF_LWTBL_CLR_DISPATCH_POLICY0(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY0)
#define WF_LWTBL_CLR_DISPATCH_POLICY1(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY1)
#define WF_LWTBL_CLR_DISPATCH_POLICY2(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY2)
#define WF_LWTBL_CLR_DISPATCH_POLICY3(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY3)
#define WF_LWTBL_CLR_DISPATCH_POLICY4(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY4)
#define WF_LWTBL_CLR_DISPATCH_POLICY5(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY5)
#define WF_LWTBL_CLR_DISPATCH_POLICY6(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY6)
#define WF_LWTBL_CLR_DISPATCH_POLICY7(reg32)                                                                CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_POLICY7)
#define WF_LWTBL_CLR_OWN_MLD_ID(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_OWN_MLD_ID)
#define WF_LWTBL_CLR_EMLSR0(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLSR0)
#define WF_LWTBL_CLR_EMLMR0(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLMR0)
#define WF_LWTBL_CLR_EMLSR1(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLSR1)
#define WF_LWTBL_CLR_EMLMR1(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLMR1)
#define WF_LWTBL_CLR_EMLSR2(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLSR2)
#define WF_LWTBL_CLR_EMLMR2(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_EMLMR2)
#define WF_LWTBL_CLR_STR_BITMAP(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_STR_BITMAP)
#define WF_LWTBL_CLR_DISPATCH_ORDER(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_ORDER)
#define WF_LWTBL_CLR_DISPATCH_RATIO(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_DISPATCH_RATIO)
#define WF_LWTBL_CLR_LINK_MGF(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_LINK_MGF)
#define WF_LWTBL_CLR_BFTX_TB(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_BFTX_TB)
#define WF_LWTBL_CLR_DROP(reg32)                                                                            CLEAR_FIELD((reg32), WF_LWTBL_DROP)
#define WF_LWTBL_CLR_CASCAD(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_CASCAD)
#define WF_LWTBL_CLR_ALL_ACK(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_ALL_ACK)
#define WF_LWTBL_CLR_MPDU_SIZE(reg32)                                                                       CLEAR_FIELD((reg32), WF_LWTBL_MPDU_SIZE)
#define WF_LWTBL_CLR_RXD_DUP_MODE(reg32)                                                                    CLEAR_FIELD((reg32), WF_LWTBL_RXD_DUP_MODE)
#define WF_LWTBL_CLR_ACK_EN(reg32)                                                                          CLEAR_FIELD((reg32), WF_LWTBL_ACK_EN)
#define WF_LWTBL_CLR_OM_INFO(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_OM_INFO)
#define WF_LWTBL_CLR_OM_INFO_EHT(reg32)                                                                     CLEAR_FIELD((reg32), WF_LWTBL_OM_INFO_EHT)
#define WF_LWTBL_CLR_RXD_DUP_FOR_OM_CHG(reg32)                                                              CLEAR_FIELD((reg32), WF_LWTBL_RXD_DUP_FOR_OM_CHG)
#define WF_LWTBL_CLR_RXD_DUP_WHITE_LIST(reg32)                                                              CLEAR_FIELD((reg32), WF_LWTBL_RXD_DUP_WHITE_LIST)
#define WF_LWTBL_CLR_USER_RSSI(reg32)                                                                       CLEAR_FIELD((reg32), WF_LWTBL_USER_RSSI)
#define WF_LWTBL_CLR_USER_SNR(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_USER_SNR)
#define WF_LWTBL_CLR_RAPID_REACTION_RATE(reg32)                                                             CLEAR_FIELD((reg32), WF_LWTBL_RAPID_REACTION_RATE)
#define WF_LWTBL_CLR_HT_AMSDU(reg32)                                                                        CLEAR_FIELD((reg32), WF_LWTBL_HT_AMSDU)
#define WF_LWTBL_CLR_AMSDU_CROSS_LG(reg32)                                                                  CLEAR_FIELD((reg32), WF_LWTBL_AMSDU_CROSS_LG)
#define WF_LWTBL_CLR_RESP_RCPI0(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_RESP_RCPI0)
#define WF_LWTBL_CLR_RESP_RCPI1(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_RESP_RCPI1)
#define WF_LWTBL_CLR_RESP_RCPI2(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_RESP_RCPI2)
#define WF_LWTBL_CLR_RESP_RCPI3(reg32)                                                                      CLEAR_FIELD((reg32), WF_LWTBL_RESP_RCPI3)
#define WF_LWTBL_CLR_SNR_RX0(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SNR_RX0)
#define WF_LWTBL_CLR_SNR_RX1(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SNR_RX1)
#define WF_LWTBL_CLR_SNR_RX2(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SNR_RX2)
#define WF_LWTBL_CLR_SNR_RX3(reg32)                                                                         CLEAR_FIELD((reg32), WF_LWTBL_SNR_RX3)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_LWTBL_REGS_H__
