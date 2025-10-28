#ifndef __WF_TX_DESCRIPTOR_REGS_H__
#define __WF_TX_DESCRIPTOR_REGS_H__

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
        FIELD tx_byte_count             : 16; // 15- 0
        FIELD ether_type_offset         :  7; // 22-16
        FIELD pkt_ft                    :  2; // 24-23
        FIELD q_idx                     :  7; // 31-25
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW00, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD mld_id                    : 12; // 11- 0
        FIELD tgid                      :  2; // 13-12
        FIELD hf                        :  2; // 15-14
        FIELD header_length             :  5; // 20-16
        FIELD tid_mgmt_type             :  4; // 24-21
        FIELD om                        :  6; // 30-25
        FIELD fr                        :  1; // 31-31
    } Bits;
    PACKING struct
    {
        FIELD mld_id                    : 12; // 11- 0
        FIELD tgid                      :  2; // 13-12
        FIELD hf                        :  2; // 15-14
        FIELD rsvd_16_16                :  1; // 16-16
        FIELD eosp                      :  1; // 17-17
        FIELD ams                       :  1; // 18-18
        FIELD rsvd_20_19                :  2; // 20-19
        FIELD tid_mgmt_type             :  4; // 24-21
        FIELD om                        :  6; // 30-25
        FIELD fr                        :  1; // 31-31
    } Bits1;
    PACKING struct
    {
        FIELD mld_id                    : 12; // 11- 0
        FIELD tgid                      :  2; // 13-12
        FIELD hf                        :  2; // 15-14
        FIELD mrd                       :  1; // 16-16
        FIELD eosp                      :  1; // 17-17
        FIELD rmvl                      :  1; // 18-18
        FIELD vlan                      :  1; // 19-19
        FIELD etyp                      :  1; // 20-20
        FIELD tid_mgmt_type             :  4; // 24-21
        FIELD om                        :  6; // 30-25
        FIELD fr                        :  1; // 31-31
    } Bits2;
    PACKING struct
    {
        FIELD mld_id                    : 12; // 11- 0
        FIELD tgid                      :  2; // 13-12
        FIELD hf                        :  2; // 15-14
        FIELD rsvd_20_16                :  5; // 20-16
        FIELD tid_mgmt_type             :  4; // 24-21
        FIELD om                        :  6; // 30-25
        FIELD fr                        :  1; // 31-31
    } Bits3;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW01, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD subtype                   :  4; //  3- 0
        FIELD ftype                     :  2; //  5- 4
        FIELD bf_type                   :  3; //  8- 6
        FIELD om_map                    :  1; //  9- 9
        FIELD header_padding            :  2; // 11-10
        FIELD du                        :  1; // 12-12
        FIELD he                        :  1; // 13-13
        FIELD frag                      :  2; // 15-14
        FIELD remaining_tx_time         : 10; // 25-16
        FIELD power_offset              :  6; // 31-26
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW02, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD na                        :  1; //  0- 0
        FIELD pf                        :  1; //  1- 1
        FIELD emrd                      :  1; //  2- 2
        FIELD eeosp                     :  1; //  3- 3
        FIELD bm                        :  1; //  4- 4
        FIELD hw_amsdu_cap              :  1; //  5- 5
        FIELD tx_count                  :  5; // 10- 6
        FIELD remaining_tx_count        :  5; // 15-11
        FIELD sn                        : 12; // 27-16
        FIELD ba_dis                    :  1; // 28-28
        FIELD pm                        :  1; // 29-29
        FIELD pn_vld                    :  1; // 30-30
        FIELD sn_vld                    :  1; // 31-31
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW03, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW03; // DW3

typedef PACKING union
{
    PACKING struct
    {
        FIELD pn_31_0_                  : 32; // 31- 0
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW04, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW04; // DW4

typedef PACKING union
{
    PACKING struct
    {
        FIELD pid                       :  8; //  7- 0
        FIELD txsfm                     :  1; //  8- 8
        FIELD txs2m                     :  1; //  9- 9
        FIELD txs2h                     :  1; // 10-10
        FIELD ocp                       :  1; // 11-11
        FIELD fbcz                      :  1; // 12-12
        FIELD bypass_rbb                :  1; // 13-13
        FIELD bypass_tbb                :  1; // 14-14
        FIELD fl                        :  1; // 15-15
        FIELD pn_47_32_                 : 16; // 31-16
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW05, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW05; // DW5

typedef PACKING union
{
    PACKING struct
    {
        FIELD mld                       :  1; //  0- 0
        FIELD amsdu_cap_utxb            :  1; //  1- 1
        FIELD das                       :  1; //  2- 2
        FIELD dis_mat                   :  1; //  3- 3
        FIELD rsvd_09_04                :  6; //  9- 4
        FIELD timestamp_offset_idx      :  5; // 14-10
        FIELD timestamp_offset_en       :  1; // 15-15
        FIELD fixed_rate_idx            :  6; // 21-16
        FIELD bw                        :  4; // 25-22
        FIELD rsvd_27_26                :  2; // 27-26
        FIELD vta                       :  1; // 28-28
        FIELD rsvd_29_29                :  1; // 29-29
        FIELD src                       :  2; // 31-30
    } Bits;
    PACKING struct
    {
        FIELD mld                       :  1; //  0- 0
        FIELD amsdu_cap_utxb            :  1; //  1- 1
        FIELD das                       :  1; //  2- 2
        FIELD dis_mat                   :  1; //  3- 3
        FIELD rsvd_09_04                :  6; //  9- 4
        FIELD msdu_count                :  6; // 15-10
        FIELD fixed_rate_idx            :  6; // 21-16
        FIELD bw                        :  4; // 25-22
        FIELD rsvd_27_26                :  2; // 27-26
        FIELD vta                       :  1; // 28-28
        FIELD rsvd_29_29                :  1; // 29-29
        FIELD src                       :  2; // 31-30
    } Bits1;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits2;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW06, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW06; // DW6

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw_tx_time                : 10; //  9- 0
        FIELD rsvd_11_10                :  2; // 11-10
        FIELD ici                       :  1; // 12-12
        FIELD rts                       :  1; // 13-13
        FIELD dis_dly_tx                :  1; // 14-14
        FIELD ut                        :  1; // 15-15
        FIELD rsvd_21_16                :  6; // 21-16
        FIELD ctxd_cnt                  :  4; // 25-22
        FIELD ctxd                      :  1; // 26-26
        FIELD hm                        :  1; // 27-27
        FIELD rsvd_28_28                :  1; // 28-28
        FIELD ip                        :  1; // 29-29
        FIELD txd_len                   :  2; // 31-30
    } Bits;
    PACKING struct
    {
        FIELD rsvd_31_00                : 32; // 31- 0
    } Bits1;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW07, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW07; // DW7

typedef PACKING union
{
    PACKING struct
    {
        FIELD token0                    : 16; // 15- 0
        FIELD token1                    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW08, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW08; // DW8

typedef PACKING union
{
    PACKING struct
    {
        FIELD token2                    : 16; // 15- 0
        FIELD token3                    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW09, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW09; // DW9

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp0                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW10, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW10; // DW10

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp0                      : 16; // 15- 0
        FIELD txp1                      : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW11, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW11; // DW11

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp1                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW12, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW12; // DW12

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp2                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW13, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW13; // DW13

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp2                      : 16; // 15- 0
        FIELD txp3                      : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW14, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW14; // DW14

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp3                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE1_DW15, *PREG_WF_TX_DESCRIPTOR_PAGE1_DW15; // DW15

typedef PACKING union
{
    PACKING struct
    {
        FIELD token4                    : 16; // 15- 0
        FIELD token5                    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW16, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW16; // DW16

typedef PACKING union
{
    PACKING struct
    {
        FIELD token6                    : 16; // 15- 0
        FIELD token7                    : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW17, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW17; // DW17

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp4                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW18, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW18; // DW18

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp4                      : 16; // 15- 0
        FIELD txp5                      : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW19, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW19; // DW19

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp5                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW20, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW20; // DW20

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp6                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW21, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW21; // DW21

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp6                      : 16; // 15- 0
        FIELD txp7                      : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW22, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW22; // DW22

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp7                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW23, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW23; // DW23

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp8                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW24, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW24; // DW24

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp8                      : 16; // 15- 0
        FIELD txp9                      : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW25, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW25; // DW25

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp9                      : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW26, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW26; // DW26

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp10                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW27, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW27; // DW27

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp10                     : 16; // 15- 0
        FIELD txp11                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW28, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW28; // DW28

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp11                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW29, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW29; // DW29

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp12                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW30, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW30; // DW30

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp12                     : 16; // 15- 0
        FIELD txp13_len                 : 12; // 27-16
        FIELD txp13_addr_33_32_         :  2; // 29-28
        FIELD txp13_src                 :  1; // 30-30
        FIELD txp13_ml                  :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE2_DW31, *PREG_WF_TX_DESCRIPTOR_PAGE2_DW31; // DW31

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp13_addr_31_0_          : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW32, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW32; // DW32

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp14                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW33, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW33; // DW33

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp14                     : 16; // 15- 0
        FIELD txp15                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW34, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW34; // DW34

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp15                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW35, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW35; // DW35

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp16                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW36, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW36; // DW36

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp16                     : 16; // 15- 0
        FIELD txp17                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW37, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW37; // DW37

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp17                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW38, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW38; // DW38

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp18                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW39, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW39; // DW39

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp18                     : 16; // 15- 0
        FIELD txp19                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW40, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW40; // DW40

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp19                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW41, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW41; // DW41

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp20                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW42, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW42; // DW42

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp20                     : 16; // 15- 0
        FIELD txp21                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW43, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW43; // DW43

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp21                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW44, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW44; // DW44

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp22                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW45, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW45; // DW45

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp22                     : 16; // 15- 0
        FIELD txp23                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW46, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW46; // DW46

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp23                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE3_DW47, *PREG_WF_TX_DESCRIPTOR_PAGE3_DW47; // DW47

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp24                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW48, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW48; // DW48

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp24                     : 16; // 15- 0
        FIELD txp25                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW49, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW49; // DW49

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp25                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW50, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW50; // DW50

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp26                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW51, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW51; // DW51

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp26                     : 16; // 15- 0
        FIELD txp27                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW52, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW52; // DW52

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp27                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW53, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW53; // DW53

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp28                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW54, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW54; // DW54

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp28                     : 16; // 15- 0
        FIELD txp29                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW55, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW55; // DW55

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp29                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW56, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW56; // DW56

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp30                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW57, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW57; // DW57

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp30                     : 16; // 15- 0
        FIELD txp31                     : 16; // 31-16
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW58, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW58; // DW58

typedef PACKING union
{
    PACKING struct
    {
        FIELD txp31                     : 32; // 31- 0
    } Bits;
    UINT32 Raw;
} REG_WF_TX_DESCRIPTOR_PAGE4_DW59, *PREG_WF_TX_DESCRIPTOR_PAGE4_DW59; // DW59

typedef volatile PACKING struct
{
    REG_WF_TX_DESCRIPTOR_PAGE1_DW00              WF_TX_DESCRIPTOR_PAGE1_DW00             ; // 0x0000
    REG_WF_TX_DESCRIPTOR_PAGE1_DW01              WF_TX_DESCRIPTOR_PAGE1_DW01             ; // 0x0004
    REG_WF_TX_DESCRIPTOR_PAGE1_DW02              WF_TX_DESCRIPTOR_PAGE1_DW02             ; // 0x0008
    REG_WF_TX_DESCRIPTOR_PAGE1_DW03              WF_TX_DESCRIPTOR_PAGE1_DW03             ; // 0x000c
    REG_WF_TX_DESCRIPTOR_PAGE1_DW04              WF_TX_DESCRIPTOR_PAGE1_DW04             ; // 0x0010
    REG_WF_TX_DESCRIPTOR_PAGE1_DW05              WF_TX_DESCRIPTOR_PAGE1_DW05             ; // 0x0014
    REG_WF_TX_DESCRIPTOR_PAGE1_DW06              WF_TX_DESCRIPTOR_PAGE1_DW06             ; // 0x0018
    REG_WF_TX_DESCRIPTOR_PAGE1_DW07              WF_TX_DESCRIPTOR_PAGE1_DW07             ; // 0x001c
    REG_WF_TX_DESCRIPTOR_PAGE1_DW08              WF_TX_DESCRIPTOR_PAGE1_DW08             ; // 0x0020
    REG_WF_TX_DESCRIPTOR_PAGE1_DW09              WF_TX_DESCRIPTOR_PAGE1_DW09             ; // 0x0024
    REG_WF_TX_DESCRIPTOR_PAGE1_DW10              WF_TX_DESCRIPTOR_PAGE1_DW10             ; // 0x0028
    REG_WF_TX_DESCRIPTOR_PAGE1_DW11              WF_TX_DESCRIPTOR_PAGE1_DW11             ; // 0x002c
    REG_WF_TX_DESCRIPTOR_PAGE1_DW12              WF_TX_DESCRIPTOR_PAGE1_DW12             ; // 0x0030
    REG_WF_TX_DESCRIPTOR_PAGE1_DW13              WF_TX_DESCRIPTOR_PAGE1_DW13             ; // 0x0034
    REG_WF_TX_DESCRIPTOR_PAGE1_DW14              WF_TX_DESCRIPTOR_PAGE1_DW14             ; // 0x0038
    REG_WF_TX_DESCRIPTOR_PAGE1_DW15              WF_TX_DESCRIPTOR_PAGE1_DW15             ; // 0x003c
    REG_WF_TX_DESCRIPTOR_PAGE2_DW16              WF_TX_DESCRIPTOR_PAGE2_DW16             ; // 0x0040
    REG_WF_TX_DESCRIPTOR_PAGE2_DW17              WF_TX_DESCRIPTOR_PAGE2_DW17             ; // 0x0044
    REG_WF_TX_DESCRIPTOR_PAGE2_DW18              WF_TX_DESCRIPTOR_PAGE2_DW18             ; // 0x0048
    REG_WF_TX_DESCRIPTOR_PAGE2_DW19              WF_TX_DESCRIPTOR_PAGE2_DW19             ; // 0x004c
    REG_WF_TX_DESCRIPTOR_PAGE2_DW20              WF_TX_DESCRIPTOR_PAGE2_DW20             ; // 0x0050
    REG_WF_TX_DESCRIPTOR_PAGE2_DW21              WF_TX_DESCRIPTOR_PAGE2_DW21             ; // 0x0054
    REG_WF_TX_DESCRIPTOR_PAGE2_DW22              WF_TX_DESCRIPTOR_PAGE2_DW22             ; // 0x0058
    REG_WF_TX_DESCRIPTOR_PAGE2_DW23              WF_TX_DESCRIPTOR_PAGE2_DW23             ; // 0x005c
    REG_WF_TX_DESCRIPTOR_PAGE2_DW24              WF_TX_DESCRIPTOR_PAGE2_DW24             ; // 0x0060
    REG_WF_TX_DESCRIPTOR_PAGE2_DW25              WF_TX_DESCRIPTOR_PAGE2_DW25             ; // 0x0064
    REG_WF_TX_DESCRIPTOR_PAGE2_DW26              WF_TX_DESCRIPTOR_PAGE2_DW26             ; // 0x0068
    REG_WF_TX_DESCRIPTOR_PAGE2_DW27              WF_TX_DESCRIPTOR_PAGE2_DW27             ; // 0x006c
    REG_WF_TX_DESCRIPTOR_PAGE2_DW28              WF_TX_DESCRIPTOR_PAGE2_DW28             ; // 0x0070
    REG_WF_TX_DESCRIPTOR_PAGE2_DW29              WF_TX_DESCRIPTOR_PAGE2_DW29             ; // 0x0074
    REG_WF_TX_DESCRIPTOR_PAGE2_DW30              WF_TX_DESCRIPTOR_PAGE2_DW30             ; // 0x0078
    REG_WF_TX_DESCRIPTOR_PAGE2_DW31              WF_TX_DESCRIPTOR_PAGE2_DW31             ; // 0x007c
    REG_WF_TX_DESCRIPTOR_PAGE3_DW32              WF_TX_DESCRIPTOR_PAGE3_DW32             ; // 0x0080
    REG_WF_TX_DESCRIPTOR_PAGE3_DW33              WF_TX_DESCRIPTOR_PAGE3_DW33             ; // 0x0084
    REG_WF_TX_DESCRIPTOR_PAGE3_DW34              WF_TX_DESCRIPTOR_PAGE3_DW34             ; // 0x0088
    REG_WF_TX_DESCRIPTOR_PAGE3_DW35              WF_TX_DESCRIPTOR_PAGE3_DW35             ; // 0x008c
    REG_WF_TX_DESCRIPTOR_PAGE3_DW36              WF_TX_DESCRIPTOR_PAGE3_DW36             ; // 0x0090
    REG_WF_TX_DESCRIPTOR_PAGE3_DW37              WF_TX_DESCRIPTOR_PAGE3_DW37             ; // 0x0094
    REG_WF_TX_DESCRIPTOR_PAGE3_DW38              WF_TX_DESCRIPTOR_PAGE3_DW38             ; // 0x0098
    REG_WF_TX_DESCRIPTOR_PAGE3_DW39              WF_TX_DESCRIPTOR_PAGE3_DW39             ; // 0x009c
    REG_WF_TX_DESCRIPTOR_PAGE3_DW40              WF_TX_DESCRIPTOR_PAGE3_DW40             ; // 0x00a0
    REG_WF_TX_DESCRIPTOR_PAGE3_DW41              WF_TX_DESCRIPTOR_PAGE3_DW41             ; // 0x00a4
    REG_WF_TX_DESCRIPTOR_PAGE3_DW42              WF_TX_DESCRIPTOR_PAGE3_DW42             ; // 0x00a8
    REG_WF_TX_DESCRIPTOR_PAGE3_DW43              WF_TX_DESCRIPTOR_PAGE3_DW43             ; // 0x00ac
    REG_WF_TX_DESCRIPTOR_PAGE3_DW44              WF_TX_DESCRIPTOR_PAGE3_DW44             ; // 0x00b0
    REG_WF_TX_DESCRIPTOR_PAGE3_DW45              WF_TX_DESCRIPTOR_PAGE3_DW45             ; // 0x00b4
    REG_WF_TX_DESCRIPTOR_PAGE3_DW46              WF_TX_DESCRIPTOR_PAGE3_DW46             ; // 0x00b8
    REG_WF_TX_DESCRIPTOR_PAGE3_DW47              WF_TX_DESCRIPTOR_PAGE3_DW47             ; // 0x00bc
    REG_WF_TX_DESCRIPTOR_PAGE4_DW48              WF_TX_DESCRIPTOR_PAGE4_DW48             ; // 0x00c0
    REG_WF_TX_DESCRIPTOR_PAGE4_DW49              WF_TX_DESCRIPTOR_PAGE4_DW49             ; // 0x00c4
    REG_WF_TX_DESCRIPTOR_PAGE4_DW50              WF_TX_DESCRIPTOR_PAGE4_DW50             ; // 0x00c8
    REG_WF_TX_DESCRIPTOR_PAGE4_DW51              WF_TX_DESCRIPTOR_PAGE4_DW51             ; // 0x00cc
    REG_WF_TX_DESCRIPTOR_PAGE4_DW52              WF_TX_DESCRIPTOR_PAGE4_DW52             ; // 0x00d0
    REG_WF_TX_DESCRIPTOR_PAGE4_DW53              WF_TX_DESCRIPTOR_PAGE4_DW53             ; // 0x00d4
    REG_WF_TX_DESCRIPTOR_PAGE4_DW54              WF_TX_DESCRIPTOR_PAGE4_DW54             ; // 0x00d8
    REG_WF_TX_DESCRIPTOR_PAGE4_DW55              WF_TX_DESCRIPTOR_PAGE4_DW55             ; // 0x00dc
    REG_WF_TX_DESCRIPTOR_PAGE4_DW56              WF_TX_DESCRIPTOR_PAGE4_DW56             ; // 0x00e0
    REG_WF_TX_DESCRIPTOR_PAGE4_DW57              WF_TX_DESCRIPTOR_PAGE4_DW57             ; // 0x00e4
    REG_WF_TX_DESCRIPTOR_PAGE4_DW58              WF_TX_DESCRIPTOR_PAGE4_DW58             ; // 0x00e8
    REG_WF_TX_DESCRIPTOR_PAGE4_DW59              WF_TX_DESCRIPTOR_PAGE4_DW59             ; // 0x00ec
} WF_TX_DESCRIPTOR_REGS, *PWF_TX_DESCRIPTOR_REGS;
extern PWF_TX_DESCRIPTOR_REGS g_WF_TX_DESCRIPTOR_BASE;

#define WF_TX_DESCRIPTOR_BASE (g_WF_TX_DESCRIPTOR_BASE)
#define WF_TX_DESCRIPTOR_PAGE1_DW00              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW00             ) // 0x0000
#define WF_TX_DESCRIPTOR_PAGE1_DW01              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW01             ) // 0x0004
#define WF_TX_DESCRIPTOR_PAGE1_DW02              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW02             ) // 0x0008
#define WF_TX_DESCRIPTOR_PAGE1_DW03              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW03             ) // 0x000c
#define WF_TX_DESCRIPTOR_PAGE1_DW04              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW04             ) // 0x0010
#define WF_TX_DESCRIPTOR_PAGE1_DW05              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW05             ) // 0x0014
#define WF_TX_DESCRIPTOR_PAGE1_DW06              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW06             ) // 0x0018
#define WF_TX_DESCRIPTOR_PAGE1_DW07              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW07             ) // 0x001c
#define WF_TX_DESCRIPTOR_PAGE1_DW08              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW08             ) // 0x0020
#define WF_TX_DESCRIPTOR_PAGE1_DW09              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW09             ) // 0x0024
#define WF_TX_DESCRIPTOR_PAGE1_DW10              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW10             ) // 0x0028
#define WF_TX_DESCRIPTOR_PAGE1_DW11              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW11             ) // 0x002c
#define WF_TX_DESCRIPTOR_PAGE1_DW12              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW12             ) // 0x0030
#define WF_TX_DESCRIPTOR_PAGE1_DW13              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW13             ) // 0x0034
#define WF_TX_DESCRIPTOR_PAGE1_DW14              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW14             ) // 0x0038
#define WF_TX_DESCRIPTOR_PAGE1_DW15              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE1_DW15             ) // 0x003c
#define WF_TX_DESCRIPTOR_PAGE2_DW16              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW16             ) // 0x0040
#define WF_TX_DESCRIPTOR_PAGE2_DW17              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW17             ) // 0x0044
#define WF_TX_DESCRIPTOR_PAGE2_DW18              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW18             ) // 0x0048
#define WF_TX_DESCRIPTOR_PAGE2_DW19              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW19             ) // 0x004c
#define WF_TX_DESCRIPTOR_PAGE2_DW20              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW20             ) // 0x0050
#define WF_TX_DESCRIPTOR_PAGE2_DW21              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW21             ) // 0x0054
#define WF_TX_DESCRIPTOR_PAGE2_DW22              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW22             ) // 0x0058
#define WF_TX_DESCRIPTOR_PAGE2_DW23              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW23             ) // 0x005c
#define WF_TX_DESCRIPTOR_PAGE2_DW24              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW24             ) // 0x0060
#define WF_TX_DESCRIPTOR_PAGE2_DW25              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW25             ) // 0x0064
#define WF_TX_DESCRIPTOR_PAGE2_DW26              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW26             ) // 0x0068
#define WF_TX_DESCRIPTOR_PAGE2_DW27              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW27             ) // 0x006c
#define WF_TX_DESCRIPTOR_PAGE2_DW28              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW28             ) // 0x0070
#define WF_TX_DESCRIPTOR_PAGE2_DW29              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW29             ) // 0x0074
#define WF_TX_DESCRIPTOR_PAGE2_DW30              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW30             ) // 0x0078
#define WF_TX_DESCRIPTOR_PAGE2_DW31              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE2_DW31             ) // 0x007c
#define WF_TX_DESCRIPTOR_PAGE3_DW32              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW32             ) // 0x0080
#define WF_TX_DESCRIPTOR_PAGE3_DW33              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW33             ) // 0x0084
#define WF_TX_DESCRIPTOR_PAGE3_DW34              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW34             ) // 0x0088
#define WF_TX_DESCRIPTOR_PAGE3_DW35              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW35             ) // 0x008c
#define WF_TX_DESCRIPTOR_PAGE3_DW36              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW36             ) // 0x0090
#define WF_TX_DESCRIPTOR_PAGE3_DW37              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW37             ) // 0x0094
#define WF_TX_DESCRIPTOR_PAGE3_DW38              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW38             ) // 0x0098
#define WF_TX_DESCRIPTOR_PAGE3_DW39              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW39             ) // 0x009c
#define WF_TX_DESCRIPTOR_PAGE3_DW40              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW40             ) // 0x00a0
#define WF_TX_DESCRIPTOR_PAGE3_DW41              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW41             ) // 0x00a4
#define WF_TX_DESCRIPTOR_PAGE3_DW42              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW42             ) // 0x00a8
#define WF_TX_DESCRIPTOR_PAGE3_DW43              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW43             ) // 0x00ac
#define WF_TX_DESCRIPTOR_PAGE3_DW44              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW44             ) // 0x00b0
#define WF_TX_DESCRIPTOR_PAGE3_DW45              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW45             ) // 0x00b4
#define WF_TX_DESCRIPTOR_PAGE3_DW46              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW46             ) // 0x00b8
#define WF_TX_DESCRIPTOR_PAGE3_DW47              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE3_DW47             ) // 0x00bc
#define WF_TX_DESCRIPTOR_PAGE4_DW48              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW48             ) // 0x00c0
#define WF_TX_DESCRIPTOR_PAGE4_DW49              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW49             ) // 0x00c4
#define WF_TX_DESCRIPTOR_PAGE4_DW50              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW50             ) // 0x00c8
#define WF_TX_DESCRIPTOR_PAGE4_DW51              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW51             ) // 0x00cc
#define WF_TX_DESCRIPTOR_PAGE4_DW52              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW52             ) // 0x00d0
#define WF_TX_DESCRIPTOR_PAGE4_DW53              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW53             ) // 0x00d4
#define WF_TX_DESCRIPTOR_PAGE4_DW54              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW54             ) // 0x00d8
#define WF_TX_DESCRIPTOR_PAGE4_DW55              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW55             ) // 0x00dc
#define WF_TX_DESCRIPTOR_PAGE4_DW56              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW56             ) // 0x00e0
#define WF_TX_DESCRIPTOR_PAGE4_DW57              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW57             ) // 0x00e4
#define WF_TX_DESCRIPTOR_PAGE4_DW58              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW58             ) // 0x00e8
#define WF_TX_DESCRIPTOR_PAGE4_DW59              INREG32(&WF_TX_DESCRIPTOR_BASE->WF_TX_DESCRIPTOR_PAGE4_DW59             ) // 0x00ec

#endif // REG_BASE_C_MODULE

#define WF_TX_DESCRIPTOR_TX_BYTE_COUNT_DW                                   0
#define WF_TX_DESCRIPTOR_TX_BYTE_COUNT_ADDR                                 0
#define WF_TX_DESCRIPTOR_TX_BYTE_COUNT_MASK                                 0x0000ffff // 15- 0
#define WF_TX_DESCRIPTOR_TX_BYTE_COUNT_SHIFT                                0
#define WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET_DW                               0
#define WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET_ADDR                             0
#define WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET_MASK                             0x007f0000 // 22-16
#define WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET_SHIFT                            16
#define WF_TX_DESCRIPTOR_PKT_FT_DW                                          0
#define WF_TX_DESCRIPTOR_PKT_FT_ADDR                                        0
#define WF_TX_DESCRIPTOR_PKT_FT_MASK                                        0x01800000 // 24-23
#define WF_TX_DESCRIPTOR_PKT_FT_SHIFT                                       23
#define WF_TX_DESCRIPTOR_Q_IDX_DW                                           0
#define WF_TX_DESCRIPTOR_Q_IDX_ADDR                                         0
#define WF_TX_DESCRIPTOR_Q_IDX_MASK                                         0xfe000000 // 31-25
#define WF_TX_DESCRIPTOR_Q_IDX_SHIFT                                        25
#define WF_TX_DESCRIPTOR_MLD_ID_DW                                          1
#define WF_TX_DESCRIPTOR_MLD_ID_ADDR                                        4
#define WF_TX_DESCRIPTOR_MLD_ID_MASK                                        0x00000fff // 11- 0
#define WF_TX_DESCRIPTOR_MLD_ID_SHIFT                                       0
#define WF_TX_DESCRIPTOR_TGID_DW                                            1
#define WF_TX_DESCRIPTOR_TGID_ADDR                                          4
#define WF_TX_DESCRIPTOR_TGID_MASK                                          0x00003000 // 13-12
#define WF_TX_DESCRIPTOR_TGID_SHIFT                                         12
#define WF_TX_DESCRIPTOR_HF_DW                                              1
#define WF_TX_DESCRIPTOR_HF_ADDR                                            4
#define WF_TX_DESCRIPTOR_HF_MASK                                            0x0000c000 // 15-14
#define WF_TX_DESCRIPTOR_HF_SHIFT                                           14
#define WF_TX_DESCRIPTOR_HEADER_LENGTH_DW                                   1
#define WF_TX_DESCRIPTOR_HEADER_LENGTH_ADDR                                 4
#define WF_TX_DESCRIPTOR_HEADER_LENGTH_MASK                                 0x001f0000 // 20-16
#define WF_TX_DESCRIPTOR_HEADER_LENGTH_SHIFT                                16
#define WF_TX_DESCRIPTOR_MRD_DW                                             1
#define WF_TX_DESCRIPTOR_MRD_ADDR                                           4
#define WF_TX_DESCRIPTOR_MRD_MASK                                           0x00010000 // 16-16
#define WF_TX_DESCRIPTOR_MRD_SHIFT                                          16
#define WF_TX_DESCRIPTOR_EOSP_DW                                            1
#define WF_TX_DESCRIPTOR_EOSP_ADDR                                          4
#define WF_TX_DESCRIPTOR_EOSP_MASK                                          0x00020000 // 17-17
#define WF_TX_DESCRIPTOR_EOSP_SHIFT                                         17
#define WF_TX_DESCRIPTOR_EOSP_DW                                            1
#define WF_TX_DESCRIPTOR_EOSP_ADDR                                          4
#define WF_TX_DESCRIPTOR_EOSP_MASK                                          0x00020000 // 17-17
#define WF_TX_DESCRIPTOR_EOSP_SHIFT                                         17
#define WF_TX_DESCRIPTOR_AMS_DW                                             1
#define WF_TX_DESCRIPTOR_AMS_ADDR                                           4
#define WF_TX_DESCRIPTOR_AMS_MASK                                           0x00040000 // 18-18
#define WF_TX_DESCRIPTOR_AMS_SHIFT                                          18
#define WF_TX_DESCRIPTOR_RMVL_DW                                            1
#define WF_TX_DESCRIPTOR_RMVL_ADDR                                          4
#define WF_TX_DESCRIPTOR_RMVL_MASK                                          0x00040000 // 18-18
#define WF_TX_DESCRIPTOR_RMVL_SHIFT                                         18
#define WF_TX_DESCRIPTOR_VLAN_DW                                            1
#define WF_TX_DESCRIPTOR_VLAN_ADDR                                          4
#define WF_TX_DESCRIPTOR_VLAN_MASK                                          0x00080000 // 19-19
#define WF_TX_DESCRIPTOR_VLAN_SHIFT                                         19
#define WF_TX_DESCRIPTOR_ETYP_DW                                            1
#define WF_TX_DESCRIPTOR_ETYP_ADDR                                          4
#define WF_TX_DESCRIPTOR_ETYP_MASK                                          0x00100000 // 20-20
#define WF_TX_DESCRIPTOR_ETYP_SHIFT                                         20
#define WF_TX_DESCRIPTOR_TID_MGMT_TYPE_DW                                   1
#define WF_TX_DESCRIPTOR_TID_MGMT_TYPE_ADDR                                 4
#define WF_TX_DESCRIPTOR_TID_MGMT_TYPE_MASK                                 0x01e00000 // 24-21
#define WF_TX_DESCRIPTOR_TID_MGMT_TYPE_SHIFT                                21
#define WF_TX_DESCRIPTOR_OM_DW                                              1
#define WF_TX_DESCRIPTOR_OM_ADDR                                            4
#define WF_TX_DESCRIPTOR_OM_MASK                                            0x7e000000 // 30-25
#define WF_TX_DESCRIPTOR_OM_SHIFT                                           25
#define WF_TX_DESCRIPTOR_FR_DW                                              1
#define WF_TX_DESCRIPTOR_FR_ADDR                                            4
#define WF_TX_DESCRIPTOR_FR_MASK                                            0x80000000 // 31-31
#define WF_TX_DESCRIPTOR_FR_SHIFT                                           31
#define WF_TX_DESCRIPTOR_SUBTYPE_DW                                         2
#define WF_TX_DESCRIPTOR_SUBTYPE_ADDR                                       8
#define WF_TX_DESCRIPTOR_SUBTYPE_MASK                                       0x0000000f //  3- 0
#define WF_TX_DESCRIPTOR_SUBTYPE_SHIFT                                      0
#define WF_TX_DESCRIPTOR_FTYPE_DW                                           2
#define WF_TX_DESCRIPTOR_FTYPE_ADDR                                         8
#define WF_TX_DESCRIPTOR_FTYPE_MASK                                         0x00000030 //  5- 4
#define WF_TX_DESCRIPTOR_FTYPE_SHIFT                                        4
#define WF_TX_DESCRIPTOR_BF_TYPE_DW                                         2
#define WF_TX_DESCRIPTOR_BF_TYPE_ADDR                                       8
#define WF_TX_DESCRIPTOR_BF_TYPE_MASK                                       0x000001c0 //  8- 6
#define WF_TX_DESCRIPTOR_BF_TYPE_SHIFT                                      6
#define WF_TX_DESCRIPTOR_OM_MAP_DW                                          2
#define WF_TX_DESCRIPTOR_OM_MAP_ADDR                                        8
#define WF_TX_DESCRIPTOR_OM_MAP_MASK                                        0x00000200 //  9- 9
#define WF_TX_DESCRIPTOR_OM_MAP_SHIFT                                       9
#define WF_TX_DESCRIPTOR_HEADER_PADDING_DW                                  2
#define WF_TX_DESCRIPTOR_HEADER_PADDING_ADDR                                8
#define WF_TX_DESCRIPTOR_HEADER_PADDING_MASK                                0x00000c00 // 11-10
#define WF_TX_DESCRIPTOR_HEADER_PADDING_SHIFT                               10
#define WF_TX_DESCRIPTOR_DU_DW                                              2
#define WF_TX_DESCRIPTOR_DU_ADDR                                            8
#define WF_TX_DESCRIPTOR_DU_MASK                                            0x00001000 // 12-12
#define WF_TX_DESCRIPTOR_DU_SHIFT                                           12
#define WF_TX_DESCRIPTOR_HE_DW                                              2
#define WF_TX_DESCRIPTOR_HE_ADDR                                            8
#define WF_TX_DESCRIPTOR_HE_MASK                                            0x00002000 // 13-13
#define WF_TX_DESCRIPTOR_HE_SHIFT                                           13
#define WF_TX_DESCRIPTOR_FRAG_DW                                            2
#define WF_TX_DESCRIPTOR_FRAG_ADDR                                          8
#define WF_TX_DESCRIPTOR_FRAG_MASK                                          0x0000c000 // 15-14
#define WF_TX_DESCRIPTOR_FRAG_SHIFT                                         14
#define WF_TX_DESCRIPTOR_REMAINING_TX_TIME_DW                               2
#define WF_TX_DESCRIPTOR_REMAINING_TX_TIME_ADDR                             8
#define WF_TX_DESCRIPTOR_REMAINING_TX_TIME_MASK                             0x03ff0000 // 25-16
#define WF_TX_DESCRIPTOR_REMAINING_TX_TIME_SHIFT                            16
#define WF_TX_DESCRIPTOR_POWER_OFFSET_DW                                    2
#define WF_TX_DESCRIPTOR_POWER_OFFSET_ADDR                                  8
#define WF_TX_DESCRIPTOR_POWER_OFFSET_MASK                                  0xfc000000 // 31-26
#define WF_TX_DESCRIPTOR_POWER_OFFSET_SHIFT                                 26
#define WF_TX_DESCRIPTOR_NA_DW                                              3
#define WF_TX_DESCRIPTOR_NA_ADDR                                            12
#define WF_TX_DESCRIPTOR_NA_MASK                                            0x00000001 //  0- 0
#define WF_TX_DESCRIPTOR_NA_SHIFT                                           0
#define WF_TX_DESCRIPTOR_PF_DW                                              3
#define WF_TX_DESCRIPTOR_PF_ADDR                                            12
#define WF_TX_DESCRIPTOR_PF_MASK                                            0x00000002 //  1- 1
#define WF_TX_DESCRIPTOR_PF_SHIFT                                           1
#define WF_TX_DESCRIPTOR_EMRD_DW                                            3
#define WF_TX_DESCRIPTOR_EMRD_ADDR                                          12
#define WF_TX_DESCRIPTOR_EMRD_MASK                                          0x00000004 //  2- 2
#define WF_TX_DESCRIPTOR_EMRD_SHIFT                                         2
#define WF_TX_DESCRIPTOR_EEOSP_DW                                           3
#define WF_TX_DESCRIPTOR_EEOSP_ADDR                                         12
#define WF_TX_DESCRIPTOR_EEOSP_MASK                                         0x00000008 //  3- 3
#define WF_TX_DESCRIPTOR_EEOSP_SHIFT                                        3
#define WF_TX_DESCRIPTOR_BM_DW                                              3
#define WF_TX_DESCRIPTOR_BM_ADDR                                            12
#define WF_TX_DESCRIPTOR_BM_MASK                                            0x00000010 //  4- 4
#define WF_TX_DESCRIPTOR_BM_SHIFT                                           4
#define WF_TX_DESCRIPTOR_HW_AMSDU_CAP_DW                                    3
#define WF_TX_DESCRIPTOR_HW_AMSDU_CAP_ADDR                                  12
#define WF_TX_DESCRIPTOR_HW_AMSDU_CAP_MASK                                  0x00000020 //  5- 5
#define WF_TX_DESCRIPTOR_HW_AMSDU_CAP_SHIFT                                 5
#define WF_TX_DESCRIPTOR_TX_COUNT_DW                                        3
#define WF_TX_DESCRIPTOR_TX_COUNT_ADDR                                      12
#define WF_TX_DESCRIPTOR_TX_COUNT_MASK                                      0x000007c0 // 10- 6
#define WF_TX_DESCRIPTOR_TX_COUNT_SHIFT                                     6
#define WF_TX_DESCRIPTOR_REMAINING_TX_COUNT_DW                              3
#define WF_TX_DESCRIPTOR_REMAINING_TX_COUNT_ADDR                            12
#define WF_TX_DESCRIPTOR_REMAINING_TX_COUNT_MASK                            0x0000f800 // 15-11
#define WF_TX_DESCRIPTOR_REMAINING_TX_COUNT_SHIFT                           11
#define WF_TX_DESCRIPTOR_SN_DW                                              3
#define WF_TX_DESCRIPTOR_SN_ADDR                                            12
#define WF_TX_DESCRIPTOR_SN_MASK                                            0x0fff0000 // 27-16
#define WF_TX_DESCRIPTOR_SN_SHIFT                                           16
#define WF_TX_DESCRIPTOR_BA_DIS_DW                                          3
#define WF_TX_DESCRIPTOR_BA_DIS_ADDR                                        12
#define WF_TX_DESCRIPTOR_BA_DIS_MASK                                        0x10000000 // 28-28
#define WF_TX_DESCRIPTOR_BA_DIS_SHIFT                                       28
#define WF_TX_DESCRIPTOR_PM_DW                                              3
#define WF_TX_DESCRIPTOR_PM_ADDR                                            12
#define WF_TX_DESCRIPTOR_PM_MASK                                            0x20000000 // 29-29
#define WF_TX_DESCRIPTOR_PM_SHIFT                                           29
#define WF_TX_DESCRIPTOR_PN_VLD_DW                                          3
#define WF_TX_DESCRIPTOR_PN_VLD_ADDR                                        12
#define WF_TX_DESCRIPTOR_PN_VLD_MASK                                        0x40000000 // 30-30
#define WF_TX_DESCRIPTOR_PN_VLD_SHIFT                                       30
#define WF_TX_DESCRIPTOR_SN_VLD_DW                                          3
#define WF_TX_DESCRIPTOR_SN_VLD_ADDR                                        12
#define WF_TX_DESCRIPTOR_SN_VLD_MASK                                        0x80000000 // 31-31
#define WF_TX_DESCRIPTOR_SN_VLD_SHIFT                                       31
#define WF_TX_DESCRIPTOR_PN_31_0__DW                                        4
#define WF_TX_DESCRIPTOR_PN_31_0__ADDR                                      16
#define WF_TX_DESCRIPTOR_PN_31_0__MASK                                      0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_PN_31_0__SHIFT                                     0
#define WF_TX_DESCRIPTOR_PID_DW                                             5
#define WF_TX_DESCRIPTOR_PID_ADDR                                           20
#define WF_TX_DESCRIPTOR_PID_MASK                                           0x000000ff //  7- 0
#define WF_TX_DESCRIPTOR_PID_SHIFT                                          0
#define WF_TX_DESCRIPTOR_TXSFM_DW                                           5
#define WF_TX_DESCRIPTOR_TXSFM_ADDR                                         20
#define WF_TX_DESCRIPTOR_TXSFM_MASK                                         0x00000100 //  8- 8
#define WF_TX_DESCRIPTOR_TXSFM_SHIFT                                        8
#define WF_TX_DESCRIPTOR_TXS2M_DW                                           5
#define WF_TX_DESCRIPTOR_TXS2M_ADDR                                         20
#define WF_TX_DESCRIPTOR_TXS2M_MASK                                         0x00000200 //  9- 9
#define WF_TX_DESCRIPTOR_TXS2M_SHIFT                                        9
#define WF_TX_DESCRIPTOR_TXS2H_DW                                           5
#define WF_TX_DESCRIPTOR_TXS2H_ADDR                                         20
#define WF_TX_DESCRIPTOR_TXS2H_MASK                                         0x00000400 // 10-10
#define WF_TX_DESCRIPTOR_TXS2H_SHIFT                                        10
#define WF_TX_DESCRIPTOR_OCP_DW                                             5
#define WF_TX_DESCRIPTOR_OCP_ADDR                                           20
#define WF_TX_DESCRIPTOR_OCP_MASK                                           0x00000800 // 11-11
#define WF_TX_DESCRIPTOR_OCP_SHIFT                                          11
#define WF_TX_DESCRIPTOR_FBCZ_DW                                            5
#define WF_TX_DESCRIPTOR_FBCZ_ADDR                                          20
#define WF_TX_DESCRIPTOR_FBCZ_MASK                                          0x00001000 // 12-12
#define WF_TX_DESCRIPTOR_FBCZ_SHIFT                                         12
#define WF_TX_DESCRIPTOR_BYPASS_RBB_DW                                      5
#define WF_TX_DESCRIPTOR_BYPASS_RBB_ADDR                                    20
#define WF_TX_DESCRIPTOR_BYPASS_RBB_MASK                                    0x00002000 // 13-13
#define WF_TX_DESCRIPTOR_BYPASS_RBB_SHIFT                                   13
#define WF_TX_DESCRIPTOR_BYPASS_TBB_DW                                      5
#define WF_TX_DESCRIPTOR_BYPASS_TBB_ADDR                                    20
#define WF_TX_DESCRIPTOR_BYPASS_TBB_MASK                                    0x00004000 // 14-14
#define WF_TX_DESCRIPTOR_BYPASS_TBB_SHIFT                                   14
#define WF_TX_DESCRIPTOR_FL_DW                                              5
#define WF_TX_DESCRIPTOR_FL_ADDR                                            20
#define WF_TX_DESCRIPTOR_FL_MASK                                            0x00008000 // 15-15
#define WF_TX_DESCRIPTOR_FL_SHIFT                                           15
#define WF_TX_DESCRIPTOR_PN_47_32__DW                                       5
#define WF_TX_DESCRIPTOR_PN_47_32__ADDR                                     20
#define WF_TX_DESCRIPTOR_PN_47_32__MASK                                     0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_PN_47_32__SHIFT                                    16
#define WF_TX_DESCRIPTOR_MLD_DW                                             6
#define WF_TX_DESCRIPTOR_MLD_ADDR                                           24
#define WF_TX_DESCRIPTOR_MLD_MASK                                           0x00000001 //  0- 0
#define WF_TX_DESCRIPTOR_MLD_SHIFT                                          0
#define WF_TX_DESCRIPTOR_MLD_DW                                             6
#define WF_TX_DESCRIPTOR_MLD_ADDR                                           24
#define WF_TX_DESCRIPTOR_MLD_MASK                                           0x00000001 //  0- 0
#define WF_TX_DESCRIPTOR_MLD_SHIFT                                          0
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_DW                                  6
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_ADDR                                24
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_MASK                                0x00000002 //  1- 1
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_SHIFT                               1
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_DW                                  6
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_ADDR                                24
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_MASK                                0x00000002 //  1- 1
#define WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB_SHIFT                               1
#define WF_TX_DESCRIPTOR_DAS_DW                                             6
#define WF_TX_DESCRIPTOR_DAS_ADDR                                           24
#define WF_TX_DESCRIPTOR_DAS_MASK                                           0x00000004 //  2- 2
#define WF_TX_DESCRIPTOR_DAS_SHIFT                                          2
#define WF_TX_DESCRIPTOR_DAS_DW                                             6
#define WF_TX_DESCRIPTOR_DAS_ADDR                                           24
#define WF_TX_DESCRIPTOR_DAS_MASK                                           0x00000004 //  2- 2
#define WF_TX_DESCRIPTOR_DAS_SHIFT                                          2
#define WF_TX_DESCRIPTOR_DIS_MAT_DW                                         6
#define WF_TX_DESCRIPTOR_DIS_MAT_ADDR                                       24
#define WF_TX_DESCRIPTOR_DIS_MAT_MASK                                       0x00000008 //  3- 3
#define WF_TX_DESCRIPTOR_DIS_MAT_SHIFT                                      3
#define WF_TX_DESCRIPTOR_DIS_MAT_DW                                         6
#define WF_TX_DESCRIPTOR_DIS_MAT_ADDR                                       24
#define WF_TX_DESCRIPTOR_DIS_MAT_MASK                                       0x00000008 //  3- 3
#define WF_TX_DESCRIPTOR_DIS_MAT_SHIFT                                      3
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX_DW                            6
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX_ADDR                          24
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX_MASK                          0x00007c00 // 14-10
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX_SHIFT                         10
#define WF_TX_DESCRIPTOR_MSDU_COUNT_DW                                      6
#define WF_TX_DESCRIPTOR_MSDU_COUNT_ADDR                                    24
#define WF_TX_DESCRIPTOR_MSDU_COUNT_MASK                                    0x0000fc00 // 15-10
#define WF_TX_DESCRIPTOR_MSDU_COUNT_SHIFT                                   10
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN_DW                             6
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN_ADDR                           24
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN_MASK                           0x00008000 // 15-15
#define WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN_SHIFT                          15
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_DW                                  6
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_ADDR                                24
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_MASK                                0x003f0000 // 21-16
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_SHIFT                               16
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_DW                                  6
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_ADDR                                24
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_MASK                                0x003f0000 // 21-16
#define WF_TX_DESCRIPTOR_FIXED_RATE_IDX_SHIFT                               16
#define WF_TX_DESCRIPTOR_BW_DW                                              6
#define WF_TX_DESCRIPTOR_BW_ADDR                                            24
#define WF_TX_DESCRIPTOR_BW_MASK                                            0x03c00000 // 25-22
#define WF_TX_DESCRIPTOR_BW_SHIFT                                           22
#define WF_TX_DESCRIPTOR_BW_DW                                              6
#define WF_TX_DESCRIPTOR_BW_ADDR                                            24
#define WF_TX_DESCRIPTOR_BW_MASK                                            0x03c00000 // 25-22
#define WF_TX_DESCRIPTOR_BW_SHIFT                                           22
#define WF_TX_DESCRIPTOR_VTA_DW                                             6
#define WF_TX_DESCRIPTOR_VTA_ADDR                                           24
#define WF_TX_DESCRIPTOR_VTA_MASK                                           0x10000000 // 28-28
#define WF_TX_DESCRIPTOR_VTA_SHIFT                                          28
#define WF_TX_DESCRIPTOR_VTA_DW                                             6
#define WF_TX_DESCRIPTOR_VTA_ADDR                                           24
#define WF_TX_DESCRIPTOR_VTA_MASK                                           0x10000000 // 28-28
#define WF_TX_DESCRIPTOR_VTA_SHIFT                                          28
#define WF_TX_DESCRIPTOR_SRC_DW                                             6
#define WF_TX_DESCRIPTOR_SRC_ADDR                                           24
#define WF_TX_DESCRIPTOR_SRC_MASK                                           0xc0000000 // 31-30
#define WF_TX_DESCRIPTOR_SRC_SHIFT                                          30
#define WF_TX_DESCRIPTOR_SRC_DW                                             6
#define WF_TX_DESCRIPTOR_SRC_ADDR                                           24
#define WF_TX_DESCRIPTOR_SRC_MASK                                           0xc0000000 // 31-30
#define WF_TX_DESCRIPTOR_SRC_SHIFT                                          30
#define WF_TX_DESCRIPTOR_SW_TX_TIME_DW                                      7
#define WF_TX_DESCRIPTOR_SW_TX_TIME_ADDR                                    28
#define WF_TX_DESCRIPTOR_SW_TX_TIME_MASK                                    0x000003ff //  9- 0
#define WF_TX_DESCRIPTOR_SW_TX_TIME_SHIFT                                   0
#define WF_TX_DESCRIPTOR_ICI_DW                                             7
#define WF_TX_DESCRIPTOR_ICI_ADDR                                           28
#define WF_TX_DESCRIPTOR_ICI_MASK                                           0x00001000 // 12-12
#define WF_TX_DESCRIPTOR_ICI_SHIFT                                          12
#define WF_TX_DESCRIPTOR_RTS_DW                                             7
#define WF_TX_DESCRIPTOR_RTS_ADDR                                           28
#define WF_TX_DESCRIPTOR_RTS_MASK                                           0x00002000 // 13-13
#define WF_TX_DESCRIPTOR_RTS_SHIFT                                          13
#define WF_TX_DESCRIPTOR_DIS_DLY_TX_DW                                      7
#define WF_TX_DESCRIPTOR_DIS_DLY_TX_ADDR                                    28
#define WF_TX_DESCRIPTOR_DIS_DLY_TX_MASK                                    0x00004000 // 14-14
#define WF_TX_DESCRIPTOR_DIS_DLY_TX_SHIFT                                   14
#define WF_TX_DESCRIPTOR_UT_DW                                              7
#define WF_TX_DESCRIPTOR_UT_ADDR                                            28
#define WF_TX_DESCRIPTOR_UT_MASK                                            0x00008000 // 15-15
#define WF_TX_DESCRIPTOR_UT_SHIFT                                           15
#define WF_TX_DESCRIPTOR_CTXD_CNT_DW                                        7
#define WF_TX_DESCRIPTOR_CTXD_CNT_ADDR                                      28
#define WF_TX_DESCRIPTOR_CTXD_CNT_MASK                                      0x03c00000 // 25-22
#define WF_TX_DESCRIPTOR_CTXD_CNT_SHIFT                                     22
#define WF_TX_DESCRIPTOR_CTXD_DW                                            7
#define WF_TX_DESCRIPTOR_CTXD_ADDR                                          28
#define WF_TX_DESCRIPTOR_CTXD_MASK                                          0x04000000 // 26-26
#define WF_TX_DESCRIPTOR_CTXD_SHIFT                                         26
#define WF_TX_DESCRIPTOR_HM_DW                                              7
#define WF_TX_DESCRIPTOR_HM_ADDR                                            28
#define WF_TX_DESCRIPTOR_HM_MASK                                            0x08000000 // 27-27
#define WF_TX_DESCRIPTOR_HM_SHIFT                                           27
#define WF_TX_DESCRIPTOR_IP_DW                                              7
#define WF_TX_DESCRIPTOR_IP_ADDR                                            28
#define WF_TX_DESCRIPTOR_IP_MASK                                            0x20000000 // 29-29
#define WF_TX_DESCRIPTOR_IP_SHIFT                                           29
#define WF_TX_DESCRIPTOR_TXD_LEN_DW                                         7
#define WF_TX_DESCRIPTOR_TXD_LEN_ADDR                                       28
#define WF_TX_DESCRIPTOR_TXD_LEN_MASK                                       0xc0000000 // 31-30
#define WF_TX_DESCRIPTOR_TXD_LEN_SHIFT                                      30
#define WF_TX_DESCRIPTOR_TOKEN0_DW                                          8
#define WF_TX_DESCRIPTOR_TOKEN0_ADDR                                        32
#define WF_TX_DESCRIPTOR_TOKEN0_MASK                                        0x0000ffff // 15- 0
#define WF_TX_DESCRIPTOR_TOKEN0_SHIFT                                       0
#define WF_TX_DESCRIPTOR_TOKEN1_DW                                          8
#define WF_TX_DESCRIPTOR_TOKEN1_ADDR                                        32
#define WF_TX_DESCRIPTOR_TOKEN1_MASK                                        0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TOKEN1_SHIFT                                       16
#define WF_TX_DESCRIPTOR_TOKEN2_DW                                          9
#define WF_TX_DESCRIPTOR_TOKEN2_ADDR                                        36
#define WF_TX_DESCRIPTOR_TOKEN2_MASK                                        0x0000ffff // 15- 0
#define WF_TX_DESCRIPTOR_TOKEN2_SHIFT                                       0
#define WF_TX_DESCRIPTOR_TOKEN3_DW                                          9
#define WF_TX_DESCRIPTOR_TOKEN3_ADDR                                        36
#define WF_TX_DESCRIPTOR_TOKEN3_MASK                                        0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TOKEN3_SHIFT                                       16
#define WF_TX_DESCRIPTOR_TXP0_DW                                            10
#define WF_TX_DESCRIPTOR_TXP0_ADDR                                          40
#define WF_TX_DESCRIPTOR_TXP0_MASK                                          0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP0_SHIFT                                         0
#define WF_TX_DESCRIPTOR_TXP1_DW                                            11
#define WF_TX_DESCRIPTOR_TXP1_ADDR                                          44
#define WF_TX_DESCRIPTOR_TXP1_MASK                                          0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP1_SHIFT                                         16
#define WF_TX_DESCRIPTOR_TXP2_DW                                            13
#define WF_TX_DESCRIPTOR_TXP2_ADDR                                          52
#define WF_TX_DESCRIPTOR_TXP2_MASK                                          0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP2_SHIFT                                         0
#define WF_TX_DESCRIPTOR_TXP3_DW                                            14
#define WF_TX_DESCRIPTOR_TXP3_ADDR                                          56
#define WF_TX_DESCRIPTOR_TXP3_MASK                                          0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP3_SHIFT                                         16
#define WF_TX_DESCRIPTOR_TOKEN4_DW                                          16
#define WF_TX_DESCRIPTOR_TOKEN4_ADDR                                        64
#define WF_TX_DESCRIPTOR_TOKEN4_MASK                                        0x0000ffff // 15- 0
#define WF_TX_DESCRIPTOR_TOKEN4_SHIFT                                       0
#define WF_TX_DESCRIPTOR_TOKEN5_DW                                          16
#define WF_TX_DESCRIPTOR_TOKEN5_ADDR                                        64
#define WF_TX_DESCRIPTOR_TOKEN5_MASK                                        0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TOKEN5_SHIFT                                       16
#define WF_TX_DESCRIPTOR_TOKEN6_DW                                          17
#define WF_TX_DESCRIPTOR_TOKEN6_ADDR                                        68
#define WF_TX_DESCRIPTOR_TOKEN6_MASK                                        0x0000ffff // 15- 0
#define WF_TX_DESCRIPTOR_TOKEN6_SHIFT                                       0
#define WF_TX_DESCRIPTOR_TOKEN7_DW                                          17
#define WF_TX_DESCRIPTOR_TOKEN7_ADDR                                        68
#define WF_TX_DESCRIPTOR_TOKEN7_MASK                                        0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TOKEN7_SHIFT                                       16
#define WF_TX_DESCRIPTOR_TXP4_DW                                            18
#define WF_TX_DESCRIPTOR_TXP4_ADDR                                          72
#define WF_TX_DESCRIPTOR_TXP4_MASK                                          0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP4_SHIFT                                         0
#define WF_TX_DESCRIPTOR_TXP5_DW                                            19
#define WF_TX_DESCRIPTOR_TXP5_ADDR                                          76
#define WF_TX_DESCRIPTOR_TXP5_MASK                                          0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP5_SHIFT                                         16
#define WF_TX_DESCRIPTOR_TXP6_DW                                            21
#define WF_TX_DESCRIPTOR_TXP6_ADDR                                          84
#define WF_TX_DESCRIPTOR_TXP6_MASK                                          0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP6_SHIFT                                         0
#define WF_TX_DESCRIPTOR_TXP7_DW                                            22
#define WF_TX_DESCRIPTOR_TXP7_ADDR                                          88
#define WF_TX_DESCRIPTOR_TXP7_MASK                                          0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP7_SHIFT                                         16
#define WF_TX_DESCRIPTOR_TXP8_DW                                            24
#define WF_TX_DESCRIPTOR_TXP8_ADDR                                          96
#define WF_TX_DESCRIPTOR_TXP8_MASK                                          0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP8_SHIFT                                         0
#define WF_TX_DESCRIPTOR_TXP9_DW                                            25
#define WF_TX_DESCRIPTOR_TXP9_ADDR                                          100
#define WF_TX_DESCRIPTOR_TXP9_MASK                                          0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP9_SHIFT                                         16
#define WF_TX_DESCRIPTOR_TXP10_DW                                           27
#define WF_TX_DESCRIPTOR_TXP10_ADDR                                         108
#define WF_TX_DESCRIPTOR_TXP10_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP10_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP11_DW                                           28
#define WF_TX_DESCRIPTOR_TXP11_ADDR                                         112
#define WF_TX_DESCRIPTOR_TXP11_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP11_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP12_DW                                           30
#define WF_TX_DESCRIPTOR_TXP12_ADDR                                         120
#define WF_TX_DESCRIPTOR_TXP12_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP12_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP13_LEN_DW                                       31
#define WF_TX_DESCRIPTOR_TXP13_LEN_ADDR                                     124
#define WF_TX_DESCRIPTOR_TXP13_LEN_MASK                                     0x0fff0000 // 27-16
#define WF_TX_DESCRIPTOR_TXP13_LEN_SHIFT                                    16
#define WF_TX_DESCRIPTOR_TXP13_ADDR_33_32__DW                               31
#define WF_TX_DESCRIPTOR_TXP13_ADDR_33_32__ADDR                             124
#define WF_TX_DESCRIPTOR_TXP13_ADDR_33_32__MASK                             0x30000000 // 29-28
#define WF_TX_DESCRIPTOR_TXP13_ADDR_33_32__SHIFT                            28
#define WF_TX_DESCRIPTOR_TXP13_SRC_DW                                       31
#define WF_TX_DESCRIPTOR_TXP13_SRC_ADDR                                     124
#define WF_TX_DESCRIPTOR_TXP13_SRC_MASK                                     0x40000000 // 30-30
#define WF_TX_DESCRIPTOR_TXP13_SRC_SHIFT                                    30
#define WF_TX_DESCRIPTOR_TXP13_ML_DW                                        31
#define WF_TX_DESCRIPTOR_TXP13_ML_ADDR                                      124
#define WF_TX_DESCRIPTOR_TXP13_ML_MASK                                      0x80000000 // 31-31
#define WF_TX_DESCRIPTOR_TXP13_ML_SHIFT                                     31
#define WF_TX_DESCRIPTOR_TXP13_ADDR_31_0__DW                                32
#define WF_TX_DESCRIPTOR_TXP13_ADDR_31_0__ADDR                              128
#define WF_TX_DESCRIPTOR_TXP13_ADDR_31_0__MASK                              0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP13_ADDR_31_0__SHIFT                             0
#define WF_TX_DESCRIPTOR_TXP14_DW                                           33
#define WF_TX_DESCRIPTOR_TXP14_ADDR                                         132
#define WF_TX_DESCRIPTOR_TXP14_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP14_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP15_DW                                           34
#define WF_TX_DESCRIPTOR_TXP15_ADDR                                         136
#define WF_TX_DESCRIPTOR_TXP15_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP15_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP16_DW                                           36
#define WF_TX_DESCRIPTOR_TXP16_ADDR                                         144
#define WF_TX_DESCRIPTOR_TXP16_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP16_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP17_DW                                           37
#define WF_TX_DESCRIPTOR_TXP17_ADDR                                         148
#define WF_TX_DESCRIPTOR_TXP17_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP17_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP18_DW                                           39
#define WF_TX_DESCRIPTOR_TXP18_ADDR                                         156
#define WF_TX_DESCRIPTOR_TXP18_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP18_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP19_DW                                           40
#define WF_TX_DESCRIPTOR_TXP19_ADDR                                         160
#define WF_TX_DESCRIPTOR_TXP19_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP19_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP20_DW                                           42
#define WF_TX_DESCRIPTOR_TXP20_ADDR                                         168
#define WF_TX_DESCRIPTOR_TXP20_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP20_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP21_DW                                           43
#define WF_TX_DESCRIPTOR_TXP21_ADDR                                         172
#define WF_TX_DESCRIPTOR_TXP21_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP21_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP22_DW                                           45
#define WF_TX_DESCRIPTOR_TXP22_ADDR                                         180
#define WF_TX_DESCRIPTOR_TXP22_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP22_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP23_DW                                           46
#define WF_TX_DESCRIPTOR_TXP23_ADDR                                         184
#define WF_TX_DESCRIPTOR_TXP23_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP23_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP24_DW                                           48
#define WF_TX_DESCRIPTOR_TXP24_ADDR                                         192
#define WF_TX_DESCRIPTOR_TXP24_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP24_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP25_DW                                           49
#define WF_TX_DESCRIPTOR_TXP25_ADDR                                         196
#define WF_TX_DESCRIPTOR_TXP25_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP25_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP26_DW                                           51
#define WF_TX_DESCRIPTOR_TXP26_ADDR                                         204
#define WF_TX_DESCRIPTOR_TXP26_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP26_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP27_DW                                           52
#define WF_TX_DESCRIPTOR_TXP27_ADDR                                         208
#define WF_TX_DESCRIPTOR_TXP27_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP27_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP28_DW                                           54
#define WF_TX_DESCRIPTOR_TXP28_ADDR                                         216
#define WF_TX_DESCRIPTOR_TXP28_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP28_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP29_DW                                           55
#define WF_TX_DESCRIPTOR_TXP29_ADDR                                         220
#define WF_TX_DESCRIPTOR_TXP29_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP29_SHIFT                                        16
#define WF_TX_DESCRIPTOR_TXP30_DW                                           57
#define WF_TX_DESCRIPTOR_TXP30_ADDR                                         228
#define WF_TX_DESCRIPTOR_TXP30_MASK                                         0xffffffff // 31- 0
#define WF_TX_DESCRIPTOR_TXP30_SHIFT                                        0
#define WF_TX_DESCRIPTOR_TXP31_DW                                           58
#define WF_TX_DESCRIPTOR_TXP31_ADDR                                         232
#define WF_TX_DESCRIPTOR_TXP31_MASK                                         0xffff0000 // 31-16
#define WF_TX_DESCRIPTOR_TXP31_SHIFT                                        16

#define HAL_TX_DESC_GET_TX_BYTE_COUNT(reg32)                                                                READ_FIELD((reg32), WF_TX_DESCRIPTOR_TX_BYTE_COUNT)
#define HAL_TX_DESC_GET_ETHER_TYPE_OFFSET(reg32)                                                            READ_FIELD((reg32), WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET)
#define HAL_TX_DESC_GET_PKT_FT(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_PKT_FT)
#define HAL_TX_DESC_GET_Q_IDX(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_Q_IDX)
#define HAL_TX_DESC_GET_MLD_ID(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_MLD_ID)
#define HAL_TX_DESC_GET_TGID(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TGID)
#define HAL_TX_DESC_GET_HF(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_HF)
#define HAL_TX_DESC_GET_HEADER_LENGTH(reg32)                                                                READ_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_LENGTH)
#define HAL_TX_DESC_GET_MRD(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_MRD)
#define HAL_TX_DESC_GET_EOSP(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP)
#define HAL_TX_DESC_GET_EOSP(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP)
#define HAL_TX_DESC_GET_AMS(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_AMS)
#define HAL_TX_DESC_GET_RMVL(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_RMVL)
#define HAL_TX_DESC_GET_VLAN(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_VLAN)
#define HAL_TX_DESC_GET_ETYP(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_ETYP)
#define HAL_TX_DESC_GET_TID_MGMT_TYPE(reg32)                                                                READ_FIELD((reg32), WF_TX_DESCRIPTOR_TID_MGMT_TYPE)
#define HAL_TX_DESC_GET_OM(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_OM)
#define HAL_TX_DESC_GET_FR(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_FR)
#define HAL_TX_DESC_GET_SUBTYPE(reg32)                                                                      READ_FIELD((reg32), WF_TX_DESCRIPTOR_SUBTYPE)
#define HAL_TX_DESC_GET_FTYPE(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_FTYPE)
#define HAL_TX_DESC_GET_BF_TYPE(reg32)                                                                      READ_FIELD((reg32), WF_TX_DESCRIPTOR_BF_TYPE)
#define HAL_TX_DESC_GET_OM_MAP(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_OM_MAP)
#define HAL_TX_DESC_GET_HEADER_PADDING(reg32)                                                               READ_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_PADDING)
#define HAL_TX_DESC_GET_DU(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_DU)
#define HAL_TX_DESC_GET_HE(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_HE)
#define HAL_TX_DESC_GET_FRAG(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_FRAG)
#define HAL_TX_DESC_GET_REMAINING_TX_TIME(reg32)                                                            READ_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_TIME)
#define HAL_TX_DESC_GET_POWER_OFFSET(reg32)                                                                 READ_FIELD((reg32), WF_TX_DESCRIPTOR_POWER_OFFSET)
#define HAL_TX_DESC_GET_NA(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_NA)
#define HAL_TX_DESC_GET_PF(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_PF)
#define HAL_TX_DESC_GET_EMRD(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_EMRD)
#define HAL_TX_DESC_GET_EEOSP(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_EEOSP)
#define HAL_TX_DESC_GET_BM(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_BM)
#define HAL_TX_DESC_GET_HW_AMSDU_CAP(reg32)                                                                 READ_FIELD((reg32), WF_TX_DESCRIPTOR_HW_AMSDU_CAP)
#define HAL_TX_DESC_GET_TX_COUNT(reg32)                                                                     READ_FIELD((reg32), WF_TX_DESCRIPTOR_TX_COUNT)
#define HAL_TX_DESC_GET_REMAINING_TX_COUNT(reg32)                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_COUNT)
#define HAL_TX_DESC_GET_SN(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_SN)
#define HAL_TX_DESC_GET_BA_DIS(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_BA_DIS)
#define HAL_TX_DESC_GET_PM(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_PM)
#define HAL_TX_DESC_GET_PN_VLD(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_PN_VLD)
#define HAL_TX_DESC_GET_SN_VLD(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_SN_VLD)
#define HAL_TX_DESC_GET_PN_31_0_(reg32)                                                                     READ_FIELD((reg32), WF_TX_DESCRIPTOR_PN_31_0_)
#define HAL_TX_DESC_GET_PID(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_PID)
#define HAL_TX_DESC_GET_TXSFM(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXSFM)
#define HAL_TX_DESC_GET_TXS2M(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2M)
#define HAL_TX_DESC_GET_TXS2H(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2H)
#define HAL_TX_DESC_GET_OCP(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_OCP)
#define HAL_TX_DESC_GET_FBCZ(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_FBCZ)
#define HAL_TX_DESC_GET_BYPASS_RBB(reg32)                                                                   READ_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_RBB)
#define HAL_TX_DESC_GET_BYPASS_TBB(reg32)                                                                   READ_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_TBB)
#define HAL_TX_DESC_GET_FL(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_FL)
#define HAL_TX_DESC_GET_PN_47_32_(reg32)                                                                    READ_FIELD((reg32), WF_TX_DESCRIPTOR_PN_47_32_)
#define HAL_TX_DESC_GET_MLD(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_MLD)
#define HAL_TX_DESC_GET_MLD(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_MLD)
#define HAL_TX_DESC_GET_AMSDU_CAP_UTXB(reg32)                                                               READ_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB)
#define HAL_TX_DESC_GET_AMSDU_CAP_UTXB(reg32)                                                               READ_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB)
#define HAL_TX_DESC_GET_DAS(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_DAS)
#define HAL_TX_DESC_GET_DAS(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_DAS)
#define HAL_TX_DESC_GET_DIS_MAT(reg32)                                                                      READ_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT)
#define HAL_TX_DESC_GET_DIS_MAT(reg32)                                                                      READ_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT)
#define HAL_TX_DESC_GET_TIMESTAMP_OFFSET_IDX(reg32)                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX)
#define HAL_TX_DESC_GET_MSDU_COUNT(reg32)                                                                   READ_FIELD((reg32), WF_TX_DESCRIPTOR_MSDU_COUNT)
#define HAL_TX_DESC_GET_TIMESTAMP_OFFSET_EN(reg32)                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN)
#define HAL_TX_DESC_GET_FIXED_RATE_IDX(reg32)                                                               READ_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX)
#define HAL_TX_DESC_GET_FIXED_RATE_IDX(reg32)                                                               READ_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX)
#define HAL_TX_DESC_GET_BW(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_BW)
#define HAL_TX_DESC_GET_BW(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_BW)
#define HAL_TX_DESC_GET_VTA(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_VTA)
#define HAL_TX_DESC_GET_VTA(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_VTA)
#define HAL_TX_DESC_GET_SRC(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_SRC)
#define HAL_TX_DESC_GET_SRC(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_SRC)
#define HAL_TX_DESC_GET_SW_TX_TIME(reg32)                                                                   READ_FIELD((reg32), WF_TX_DESCRIPTOR_SW_TX_TIME)
#define HAL_TX_DESC_GET_ICI(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_ICI)
#define HAL_TX_DESC_GET_RTS(reg32)                                                                          READ_FIELD((reg32), WF_TX_DESCRIPTOR_RTS)
#define HAL_TX_DESC_GET_DIS_DLY_TX(reg32)                                                                   READ_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_DLY_TX)
#define HAL_TX_DESC_GET_UT(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_UT)
#define HAL_TX_DESC_GET_CTXD_CNT(reg32)                                                                     READ_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD_CNT)
#define HAL_TX_DESC_GET_CTXD(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD)
#define HAL_TX_DESC_GET_HM(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_HM)
#define HAL_TX_DESC_GET_IP(reg32)                                                                           READ_FIELD((reg32), WF_TX_DESCRIPTOR_IP)
#define HAL_TX_DESC_GET_TXD_LEN(reg32)                                                                      READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXD_LEN)
#define HAL_TX_DESC_GET_TOKEN0(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN0)
#define HAL_TX_DESC_GET_TOKEN1(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN1)
#define HAL_TX_DESC_GET_TOKEN2(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN2)
#define HAL_TX_DESC_GET_TOKEN3(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN3)
#define HAL_TX_DESC_GET_TXP0(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP0)
#define HAL_TX_DESC_GET_TXP1(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP1)
#define HAL_TX_DESC_GET_TXP2(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP2)
#define HAL_TX_DESC_GET_TXP3(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP3)
#define HAL_TX_DESC_GET_TOKEN4(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN4)
#define HAL_TX_DESC_GET_TOKEN5(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN5)
#define HAL_TX_DESC_GET_TOKEN6(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN6)
#define HAL_TX_DESC_GET_TOKEN7(reg32)                                                                       READ_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN7)
#define HAL_TX_DESC_GET_TXP4(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP4)
#define HAL_TX_DESC_GET_TXP5(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP5)
#define HAL_TX_DESC_GET_TXP6(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP6)
#define HAL_TX_DESC_GET_TXP7(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP7)
#define HAL_TX_DESC_GET_TXP8(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP8)
#define HAL_TX_DESC_GET_TXP9(reg32)                                                                         READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP9)
#define HAL_TX_DESC_GET_TXP10(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP10)
#define HAL_TX_DESC_GET_TXP11(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP11)
#define HAL_TX_DESC_GET_TXP12(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP12)
#define HAL_TX_DESC_GET_TXP13_LEN(reg32)                                                                    READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_LEN)
#define HAL_TX_DESC_GET_TXP13_ADDR_33_32_(reg32)                                                            READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_33_32_)
#define HAL_TX_DESC_GET_TXP13_SRC(reg32)                                                                    READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_SRC)
#define HAL_TX_DESC_GET_TXP13_ML(reg32)                                                                     READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ML)
#define HAL_TX_DESC_GET_TXP13_ADDR_31_0_(reg32)                                                             READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_31_0_)
#define HAL_TX_DESC_GET_TXP14(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP14)
#define HAL_TX_DESC_GET_TXP15(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP15)
#define HAL_TX_DESC_GET_TXP16(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP16)
#define HAL_TX_DESC_GET_TXP17(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP17)
#define HAL_TX_DESC_GET_TXP18(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP18)
#define HAL_TX_DESC_GET_TXP19(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP19)
#define HAL_TX_DESC_GET_TXP20(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP20)
#define HAL_TX_DESC_GET_TXP21(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP21)
#define HAL_TX_DESC_GET_TXP22(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP22)
#define HAL_TX_DESC_GET_TXP23(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP23)
#define HAL_TX_DESC_GET_TXP24(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP24)
#define HAL_TX_DESC_GET_TXP25(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP25)
#define HAL_TX_DESC_GET_TXP26(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP26)
#define HAL_TX_DESC_GET_TXP27(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP27)
#define HAL_TX_DESC_GET_TXP28(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP28)
#define HAL_TX_DESC_GET_TXP29(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP29)
#define HAL_TX_DESC_GET_TXP30(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP30)
#define HAL_TX_DESC_GET_TXP31(reg32)                                                                        READ_FIELD((reg32), WF_TX_DESCRIPTOR_TXP31)

#define HAL_TX_DESC_SET_TX_BYTE_COUNT(reg32, val32)                                                         WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TX_BYTE_COUNT, val32)
#define HAL_TX_DESC_SET_ETHER_TYPE_OFFSET(reg32, val32)                                                     WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET, val32)
#define HAL_TX_DESC_SET_PKT_FT(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PKT_FT, val32)
#define HAL_TX_DESC_SET_Q_IDX(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_Q_IDX, val32)
#define HAL_TX_DESC_SET_MLD_ID(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_MLD_ID, val32)
#define HAL_TX_DESC_SET_TGID(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TGID, val32)
#define HAL_TX_DESC_SET_HF(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HF, val32)
#define HAL_TX_DESC_SET_HEADER_LENGTH(reg32, val32)                                                         WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_LENGTH, val32)
#define HAL_TX_DESC_SET_MRD(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_MRD, val32)
#define HAL_TX_DESC_SET_EOSP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP, val32)
#define HAL_TX_DESC_SET_EOSP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP, val32)
#define HAL_TX_DESC_SET_AMS(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_AMS, val32)
#define HAL_TX_DESC_SET_RMVL(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_RMVL, val32)
#define HAL_TX_DESC_SET_VLAN(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_VLAN, val32)
#define HAL_TX_DESC_SET_ETYP(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_ETYP, val32)
#define HAL_TX_DESC_SET_TID_MGMT_TYPE(reg32, val32)                                                         WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TID_MGMT_TYPE, val32)
#define HAL_TX_DESC_SET_OM(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_OM, val32)
#define HAL_TX_DESC_SET_FR(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FR, val32)
#define HAL_TX_DESC_SET_SUBTYPE(reg32, val32)                                                               WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SUBTYPE, val32)
#define HAL_TX_DESC_SET_FTYPE(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FTYPE, val32)
#define HAL_TX_DESC_SET_BF_TYPE(reg32, val32)                                                               WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BF_TYPE, val32)
#define HAL_TX_DESC_SET_OM_MAP(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_OM_MAP, val32)
#define HAL_TX_DESC_SET_HEADER_PADDING(reg32, val32)                                                        WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_PADDING, val32)
#define HAL_TX_DESC_SET_DU(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DU, val32)
#define HAL_TX_DESC_SET_HE(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HE, val32)
#define HAL_TX_DESC_SET_FRAG(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FRAG, val32)
#define HAL_TX_DESC_SET_REMAINING_TX_TIME(reg32, val32)                                                     WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_TIME, val32)
#define HAL_TX_DESC_SET_POWER_OFFSET(reg32, val32)                                                          WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_POWER_OFFSET, val32)
#define HAL_TX_DESC_SET_NA(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_NA, val32)
#define HAL_TX_DESC_SET_PF(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PF, val32)
#define HAL_TX_DESC_SET_EMRD(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_EMRD, val32)
#define HAL_TX_DESC_SET_EEOSP(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_EEOSP, val32)
#define HAL_TX_DESC_SET_BM(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BM, val32)
#define HAL_TX_DESC_SET_HW_AMSDU_CAP(reg32, val32)                                                          WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HW_AMSDU_CAP, val32)
#define HAL_TX_DESC_SET_TX_COUNT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TX_COUNT, val32)
#define HAL_TX_DESC_SET_REMAINING_TX_COUNT(reg32, val32)                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_COUNT, val32)
#define HAL_TX_DESC_SET_SN(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SN, val32)
#define HAL_TX_DESC_SET_BA_DIS(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BA_DIS, val32)
#define HAL_TX_DESC_SET_PM(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PM, val32)
#define HAL_TX_DESC_SET_PN_VLD(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PN_VLD, val32)
#define HAL_TX_DESC_SET_SN_VLD(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SN_VLD, val32)
#define HAL_TX_DESC_SET_PN_31_0_(reg32, val32)                                                              WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PN_31_0_, val32)
#define HAL_TX_DESC_SET_PID(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PID, val32)
#define HAL_TX_DESC_SET_TXSFM(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXSFM, val32)
#define HAL_TX_DESC_SET_TXS2M(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2M, val32)
#define HAL_TX_DESC_SET_TXS2H(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2H, val32)
#define HAL_TX_DESC_SET_OCP(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_OCP, val32)
#define HAL_TX_DESC_SET_FBCZ(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FBCZ, val32)
#define HAL_TX_DESC_SET_BYPASS_RBB(reg32, val32)                                                            WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_RBB, val32)
#define HAL_TX_DESC_SET_BYPASS_TBB(reg32, val32)                                                            WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_TBB, val32)
#define HAL_TX_DESC_SET_FL(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FL, val32)
#define HAL_TX_DESC_SET_PN_47_32_(reg32, val32)                                                             WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_PN_47_32_, val32)
#define HAL_TX_DESC_SET_MLD(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_MLD, val32)
#define HAL_TX_DESC_SET_MLD(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_MLD, val32)
#define HAL_TX_DESC_SET_AMSDU_CAP_UTXB(reg32, val32)                                                        WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB, val32)
#define HAL_TX_DESC_SET_AMSDU_CAP_UTXB(reg32, val32)                                                        WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB, val32)
#define HAL_TX_DESC_SET_DAS(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DAS, val32)
#define HAL_TX_DESC_SET_DAS(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DAS, val32)
#define HAL_TX_DESC_SET_DIS_MAT(reg32, val32)                                                               WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT, val32)
#define HAL_TX_DESC_SET_DIS_MAT(reg32, val32)                                                               WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT, val32)
#define HAL_TX_DESC_SET_TIMESTAMP_OFFSET_IDX(reg32, val32)                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX, val32)
#define HAL_TX_DESC_SET_MSDU_COUNT(reg32, val32)                                                            WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_MSDU_COUNT, val32)
#define HAL_TX_DESC_SET_TIMESTAMP_OFFSET_EN(reg32, val32)                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN, val32)
#define HAL_TX_DESC_SET_FIXED_RATE_IDX(reg32, val32)                                                        WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX, val32)
#define HAL_TX_DESC_SET_FIXED_RATE_IDX(reg32, val32)                                                        WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX, val32)
#define HAL_TX_DESC_SET_BW(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BW, val32)
#define HAL_TX_DESC_SET_BW(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_BW, val32)
#define HAL_TX_DESC_SET_VTA(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_VTA, val32)
#define HAL_TX_DESC_SET_VTA(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_VTA, val32)
#define HAL_TX_DESC_SET_SRC(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SRC, val32)
#define HAL_TX_DESC_SET_SRC(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SRC, val32)
#define HAL_TX_DESC_SET_SW_TX_TIME(reg32, val32)                                                            WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_SW_TX_TIME, val32)
#define HAL_TX_DESC_SET_ICI(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_ICI, val32)
#define HAL_TX_DESC_SET_RTS(reg32, val32)                                                                   WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_RTS, val32)
#define HAL_TX_DESC_SET_DIS_DLY_TX(reg32, val32)                                                            WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_DLY_TX, val32)
#define HAL_TX_DESC_SET_UT(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_UT, val32)
#define HAL_TX_DESC_SET_CTXD_CNT(reg32, val32)                                                              WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD_CNT, val32)
#define HAL_TX_DESC_SET_CTXD(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD, val32)
#define HAL_TX_DESC_SET_HM(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_HM, val32)
#define HAL_TX_DESC_SET_IP(reg32, val32)                                                                    WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_IP, val32)
#define HAL_TX_DESC_SET_TXD_LEN(reg32, val32)                                                               WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXD_LEN, val32)
#define HAL_TX_DESC_SET_TOKEN0(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN0, val32)
#define HAL_TX_DESC_SET_TOKEN1(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN1, val32)
#define HAL_TX_DESC_SET_TOKEN2(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN2, val32)
#define HAL_TX_DESC_SET_TOKEN3(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN3, val32)
#define HAL_TX_DESC_SET_TXP0(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP0, val32)
#define HAL_TX_DESC_SET_TXP1(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP1, val32)
#define HAL_TX_DESC_SET_TXP2(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP2, val32)
#define HAL_TX_DESC_SET_TXP3(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP3, val32)
#define HAL_TX_DESC_SET_TOKEN4(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN4, val32)
#define HAL_TX_DESC_SET_TOKEN5(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN5, val32)
#define HAL_TX_DESC_SET_TOKEN6(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN6, val32)
#define HAL_TX_DESC_SET_TOKEN7(reg32, val32)                                                                WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN7, val32)
#define HAL_TX_DESC_SET_TXP4(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP4, val32)
#define HAL_TX_DESC_SET_TXP5(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP5, val32)
#define HAL_TX_DESC_SET_TXP6(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP6, val32)
#define HAL_TX_DESC_SET_TXP7(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP7, val32)
#define HAL_TX_DESC_SET_TXP8(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP8, val32)
#define HAL_TX_DESC_SET_TXP9(reg32, val32)                                                                  WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP9, val32)
#define HAL_TX_DESC_SET_TXP10(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP10, val32)
#define HAL_TX_DESC_SET_TXP11(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP11, val32)
#define HAL_TX_DESC_SET_TXP12(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP12, val32)
#define HAL_TX_DESC_SET_TXP13_LEN(reg32, val32)                                                             WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_LEN, val32)
#define HAL_TX_DESC_SET_TXP13_ADDR_33_32_(reg32, val32)                                                     WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_33_32_, val32)
#define HAL_TX_DESC_SET_TXP13_SRC(reg32, val32)                                                             WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_SRC, val32)
#define HAL_TX_DESC_SET_TXP13_ML(reg32, val32)                                                              WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ML, val32)
#define HAL_TX_DESC_SET_TXP13_ADDR_31_0_(reg32, val32)                                                      WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_31_0_, val32)
#define HAL_TX_DESC_SET_TXP14(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP14, val32)
#define HAL_TX_DESC_SET_TXP15(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP15, val32)
#define HAL_TX_DESC_SET_TXP16(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP16, val32)
#define HAL_TX_DESC_SET_TXP17(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP17, val32)
#define HAL_TX_DESC_SET_TXP18(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP18, val32)
#define HAL_TX_DESC_SET_TXP19(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP19, val32)
#define HAL_TX_DESC_SET_TXP20(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP20, val32)
#define HAL_TX_DESC_SET_TXP21(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP21, val32)
#define HAL_TX_DESC_SET_TXP22(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP22, val32)
#define HAL_TX_DESC_SET_TXP23(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP23, val32)
#define HAL_TX_DESC_SET_TXP24(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP24, val32)
#define HAL_TX_DESC_SET_TXP25(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP25, val32)
#define HAL_TX_DESC_SET_TXP26(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP26, val32)
#define HAL_TX_DESC_SET_TXP27(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP27, val32)
#define HAL_TX_DESC_SET_TXP28(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP28, val32)
#define HAL_TX_DESC_SET_TXP29(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP29, val32)
#define HAL_TX_DESC_SET_TXP30(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP30, val32)
#define HAL_TX_DESC_SET_TXP31(reg32, val32)                                                                 WRITE_FIELD((reg32), WF_TX_DESCRIPTOR_TXP31, val32)

#define HAL_TX_DESC_CLR_TX_BYTE_COUNT(reg32)                                                                CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TX_BYTE_COUNT)
#define HAL_TX_DESC_CLR_ETHER_TYPE_OFFSET(reg32)                                                            CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_ETHER_TYPE_OFFSET)
#define HAL_TX_DESC_CLR_PKT_FT(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PKT_FT)
#define HAL_TX_DESC_CLR_Q_IDX(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_Q_IDX)
#define HAL_TX_DESC_CLR_MLD_ID(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_MLD_ID)
#define HAL_TX_DESC_CLR_TGID(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TGID)
#define HAL_TX_DESC_CLR_HF(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HF)
#define HAL_TX_DESC_CLR_HEADER_LENGTH(reg32)                                                                CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_LENGTH)
#define HAL_TX_DESC_CLR_MRD(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_MRD)
#define HAL_TX_DESC_CLR_EOSP(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP)
#define HAL_TX_DESC_CLR_EOSP(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_EOSP)
#define HAL_TX_DESC_CLR_AMS(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_AMS)
#define HAL_TX_DESC_CLR_RMVL(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_RMVL)
#define HAL_TX_DESC_CLR_VLAN(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_VLAN)
#define HAL_TX_DESC_CLR_ETYP(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_ETYP)
#define HAL_TX_DESC_CLR_TID_MGMT_TYPE(reg32)                                                                CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TID_MGMT_TYPE)
#define HAL_TX_DESC_CLR_OM(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_OM)
#define HAL_TX_DESC_CLR_FR(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FR)
#define HAL_TX_DESC_CLR_SUBTYPE(reg32)                                                                      CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SUBTYPE)
#define HAL_TX_DESC_CLR_FTYPE(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FTYPE)
#define HAL_TX_DESC_CLR_BF_TYPE(reg32)                                                                      CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BF_TYPE)
#define HAL_TX_DESC_CLR_OM_MAP(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_OM_MAP)
#define HAL_TX_DESC_CLR_HEADER_PADDING(reg32)                                                               CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HEADER_PADDING)
#define HAL_TX_DESC_CLR_DU(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DU)
#define HAL_TX_DESC_CLR_HE(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HE)
#define HAL_TX_DESC_CLR_FRAG(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FRAG)
#define HAL_TX_DESC_CLR_REMAINING_TX_TIME(reg32)                                                            CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_TIME)
#define HAL_TX_DESC_CLR_POWER_OFFSET(reg32)                                                                 CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_POWER_OFFSET)
#define HAL_TX_DESC_CLR_NA(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_NA)
#define HAL_TX_DESC_CLR_PF(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PF)
#define HAL_TX_DESC_CLR_EMRD(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_EMRD)
#define HAL_TX_DESC_CLR_EEOSP(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_EEOSP)
#define HAL_TX_DESC_CLR_BM(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BM)
#define HAL_TX_DESC_CLR_HW_AMSDU_CAP(reg32)                                                                 CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HW_AMSDU_CAP)
#define HAL_TX_DESC_CLR_TX_COUNT(reg32)                                                                     CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TX_COUNT)
#define HAL_TX_DESC_CLR_REMAINING_TX_COUNT(reg32)                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_REMAINING_TX_COUNT)
#define HAL_TX_DESC_CLR_SN(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SN)
#define HAL_TX_DESC_CLR_BA_DIS(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BA_DIS)
#define HAL_TX_DESC_CLR_PM(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PM)
#define HAL_TX_DESC_CLR_PN_VLD(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PN_VLD)
#define HAL_TX_DESC_CLR_SN_VLD(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SN_VLD)
#define HAL_TX_DESC_CLR_PN_31_0_(reg32)                                                                     CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PN_31_0_)
#define HAL_TX_DESC_CLR_PID(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PID)
#define HAL_TX_DESC_CLR_TXSFM(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXSFM)
#define HAL_TX_DESC_CLR_TXS2M(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2M)
#define HAL_TX_DESC_CLR_TXS2H(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXS2H)
#define HAL_TX_DESC_CLR_OCP(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_OCP)
#define HAL_TX_DESC_CLR_FBCZ(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FBCZ)
#define HAL_TX_DESC_CLR_BYPASS_RBB(reg32)                                                                   CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_RBB)
#define HAL_TX_DESC_CLR_BYPASS_TBB(reg32)                                                                   CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BYPASS_TBB)
#define HAL_TX_DESC_CLR_FL(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FL)
#define HAL_TX_DESC_CLR_PN_47_32_(reg32)                                                                    CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_PN_47_32_)
#define HAL_TX_DESC_CLR_MLD(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_MLD)
#define HAL_TX_DESC_CLR_MLD(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_MLD)
#define HAL_TX_DESC_CLR_AMSDU_CAP_UTXB(reg32)                                                               CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB)
#define HAL_TX_DESC_CLR_AMSDU_CAP_UTXB(reg32)                                                               CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_AMSDU_CAP_UTXB)
#define HAL_TX_DESC_CLR_DAS(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DAS)
#define HAL_TX_DESC_CLR_DAS(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DAS)
#define HAL_TX_DESC_CLR_DIS_MAT(reg32)                                                                      CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT)
#define HAL_TX_DESC_CLR_DIS_MAT(reg32)                                                                      CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_MAT)
#define HAL_TX_DESC_CLR_TIMESTAMP_OFFSET_IDX(reg32)                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_IDX)
#define HAL_TX_DESC_CLR_MSDU_COUNT(reg32)                                                                   CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_MSDU_COUNT)
#define HAL_TX_DESC_CLR_TIMESTAMP_OFFSET_EN(reg32)                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TIMESTAMP_OFFSET_EN)
#define HAL_TX_DESC_CLR_FIXED_RATE_IDX(reg32)                                                               CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX)
#define HAL_TX_DESC_CLR_FIXED_RATE_IDX(reg32)                                                               CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_FIXED_RATE_IDX)
#define HAL_TX_DESC_CLR_BW(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BW)
#define HAL_TX_DESC_CLR_BW(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_BW)
#define HAL_TX_DESC_CLR_VTA(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_VTA)
#define HAL_TX_DESC_CLR_VTA(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_VTA)
#define HAL_TX_DESC_CLR_SRC(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SRC)
#define HAL_TX_DESC_CLR_SRC(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SRC)
#define HAL_TX_DESC_CLR_SW_TX_TIME(reg32)                                                                   CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_SW_TX_TIME)
#define HAL_TX_DESC_CLR_ICI(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_ICI)
#define HAL_TX_DESC_CLR_RTS(reg32)                                                                          CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_RTS)
#define HAL_TX_DESC_CLR_DIS_DLY_TX(reg32)                                                                   CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_DIS_DLY_TX)
#define HAL_TX_DESC_CLR_UT(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_UT)
#define HAL_TX_DESC_CLR_CTXD_CNT(reg32)                                                                     CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD_CNT)
#define HAL_TX_DESC_CLR_CTXD(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_CTXD)
#define HAL_TX_DESC_CLR_HM(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_HM)
#define HAL_TX_DESC_CLR_IP(reg32)                                                                           CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_IP)
#define HAL_TX_DESC_CLR_TXD_LEN(reg32)                                                                      CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXD_LEN)
#define HAL_TX_DESC_CLR_TOKEN0(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN0)
#define HAL_TX_DESC_CLR_TOKEN1(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN1)
#define HAL_TX_DESC_CLR_TOKEN2(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN2)
#define HAL_TX_DESC_CLR_TOKEN3(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN3)
#define HAL_TX_DESC_CLR_TXP0(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP0)
#define HAL_TX_DESC_CLR_TXP1(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP1)
#define HAL_TX_DESC_CLR_TXP2(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP2)
#define HAL_TX_DESC_CLR_TXP3(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP3)
#define HAL_TX_DESC_CLR_TOKEN4(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN4)
#define HAL_TX_DESC_CLR_TOKEN5(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN5)
#define HAL_TX_DESC_CLR_TOKEN6(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN6)
#define HAL_TX_DESC_CLR_TOKEN7(reg32)                                                                       CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TOKEN7)
#define HAL_TX_DESC_CLR_TXP4(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP4)
#define HAL_TX_DESC_CLR_TXP5(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP5)
#define HAL_TX_DESC_CLR_TXP6(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP6)
#define HAL_TX_DESC_CLR_TXP7(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP7)
#define HAL_TX_DESC_CLR_TXP8(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP8)
#define HAL_TX_DESC_CLR_TXP9(reg32)                                                                         CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP9)
#define HAL_TX_DESC_CLR_TXP10(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP10)
#define HAL_TX_DESC_CLR_TXP11(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP11)
#define HAL_TX_DESC_CLR_TXP12(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP12)
#define HAL_TX_DESC_CLR_TXP13_LEN(reg32)                                                                    CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_LEN)
#define HAL_TX_DESC_CLR_TXP13_ADDR_33_32_(reg32)                                                            CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_33_32_)
#define HAL_TX_DESC_CLR_TXP13_SRC(reg32)                                                                    CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_SRC)
#define HAL_TX_DESC_CLR_TXP13_ML(reg32)                                                                     CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ML)
#define HAL_TX_DESC_CLR_TXP13_ADDR_31_0_(reg32)                                                             CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP13_ADDR_31_0_)
#define HAL_TX_DESC_CLR_TXP14(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP14)
#define HAL_TX_DESC_CLR_TXP15(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP15)
#define HAL_TX_DESC_CLR_TXP16(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP16)
#define HAL_TX_DESC_CLR_TXP17(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP17)
#define HAL_TX_DESC_CLR_TXP18(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP18)
#define HAL_TX_DESC_CLR_TXP19(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP19)
#define HAL_TX_DESC_CLR_TXP20(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP20)
#define HAL_TX_DESC_CLR_TXP21(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP21)
#define HAL_TX_DESC_CLR_TXP22(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP22)
#define HAL_TX_DESC_CLR_TXP23(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP23)
#define HAL_TX_DESC_CLR_TXP24(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP24)
#define HAL_TX_DESC_CLR_TXP25(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP25)
#define HAL_TX_DESC_CLR_TXP26(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP26)
#define HAL_TX_DESC_CLR_TXP27(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP27)
#define HAL_TX_DESC_CLR_TXP28(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP28)
#define HAL_TX_DESC_CLR_TXP29(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP29)
#define HAL_TX_DESC_CLR_TXP30(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP30)
#define HAL_TX_DESC_CLR_TXP31(reg32)                                                                        CLEAR_FIELD((reg32), WF_TX_DESCRIPTOR_TXP31)

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_TX_DESCRIPTOR_REGS_H__
