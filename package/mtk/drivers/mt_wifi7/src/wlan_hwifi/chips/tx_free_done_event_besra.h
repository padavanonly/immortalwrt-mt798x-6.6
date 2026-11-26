// generate by structure_parser.py at 2021-02-02 14:46:47
#ifndef __WF_TX_FREE_DONE_EVENT_REGS_H__
#define __WF_TX_FREE_DONE_EVENT_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef REG_BASE_C_MODULE
// ----------------- WF_TX_FREE_DONE_EVENT Bit Field Definitions -----------------

#define PACKING
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_byte_count             : 16; // 15- 0
        FIELD msdu_id_count             : 10; // 25-16
        FIELD rsvd_26_26                :  1; // 26-26
        FIELD pkt_type                  :  5; // 31-27
    } Bits;
    UINT32 Raw;
} TFD_REG_DW00, *PREG_TFD_DW00; // DW0

typedef PACKING union
{
    PACKING struct
    {
        FIELD txd_count                 :  8; //  7- 0
        FIELD serial_id                 :  8; // 15- 8
        FIELD ver                       :  3; // 18-16
        FIELD rsvd_19_19                :  1; // 19-19
        FIELD pse_fid                   : 12; // 31-20
    } Bits;
    UINT32 Raw;
} TFD_REG_DW01, *PREG_TFD_DW01; // DW1

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsvd_11_00                : 12; // 11- 0
        FIELD wlan_id                   : 12; // 23-12
        FIELD qid                       :  7; // 30-24
        FIELD p                         :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_MPDU_DW02, *PREG_MPDU_DW02; // DW2

typedef PACKING union
{
    PACKING struct
    {
        FIELD transmit_delay            : 12; // 11- 0
        FIELD air_delay                 : 12; // 23-12
        FIELD tx_count                  :  5; // 28-24
        FIELD stat                      :  1; // 29-29
        FIELD h                         :  1; // 30-30
        FIELD p                         :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_MPDU_DW03, *PREG_MPDU_DW03; // DW3

typedef PACKING union
{
    PACKING struct
    {
        FIELD msdu_id0                  : 15; // 14- 0
        FIELD msdu_id1                  : 15; // 29-15
        FIELD h                         :  1; // 30-30
        FIELD p                         :  1; // 31-31
    } Bits;
    UINT32 Raw;
} REG_MSDU_DW04, *PREG_MSDU_DW04; // DW4

// ----------------- WF_TX_FREE_DONE_EVENT Grouping Definitions  -----------------
// ----------------- WF_TX_FREE_DONE_EVENT Register Definition   -----------------
typedef volatile PACKING struct
{
    TFD_REG_DW00             TFD_H_DW00                ; // 0x0000
    TFD_REG_DW01             TFD_H_DW01                ; // 0x0004
    REG_MPDU_DW02            MPDU_DW02           ; // 0x0008
    REG_MPDU_DW03            MPDU_DW03           ; // 0x000c
    REG_MSDU_DW04            MSDU_DW04           ; // 0x0010
}WF_TX_FREE_DONE_EVENT_REGS, *PWF_TX_FREE_DONE_EVENT_REGS;
// ----------------- WF_TX_FREE_DONE_EVENT Enum Definitions      -----------------
// ----------------- WF_TX_FREE_DONE_EVENT C Macro Definitions   -----------------
extern PWF_TX_FREE_DONE_EVENT_REGS g_WF_TX_FREE_DONE_EVENT_BASE;

#define WF_TX_FREE_DONE_EVENT_BASE                                          (g_WF_TX_FREE_DONE_EVENT_BASE)
#define TFD_DW00             INREG32(&WF_TX_FREE_DONE_EVENT_BASE->TFD_H_DW00          ) // 0x0000
#define TFD_DW01             INREG32(&WF_TX_FREE_DONE_EVENT_BASE->TFD_H_DW01          ) // 0x0004
#define MPDU_DW02            INREG32(&WF_TX_FREE_DONE_EVENT_BASE->MPDU_DW02           ) // 0x0008
#define MPDU_DW03            INREG32(&WF_TX_FREE_DONE_EVENT_BASE->MPDU_DW03           ) // 0x000c
#define MSDU_DW04            INREG32(&WF_TX_FREE_DONE_EVENT_BASE->MSDU_DW04           ) // 0x0010

#endif // REG_BASE_C_MODULE

// DW0
#define WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT_DW                                   0
#define WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT_ADDR                                 0
#define WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT_MASK                                 0x0000ffff // 15- 0
#define WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT_SHIFT                                0
#define WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT_DW                                   0
#define WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT_ADDR                                 0
#define WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT_MASK                                 0x03ff0000 // 25-16
#define WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT_SHIFT                                16
#define WF_TX_FREE_DONE_EVENT_PKT_TYPE_DW                                        0
#define WF_TX_FREE_DONE_EVENT_PKT_TYPE_ADDR                                      0
#define WF_TX_FREE_DONE_EVENT_PKT_TYPE_MASK                                      0xf8000000 // 31-27
#define WF_TX_FREE_DONE_EVENT_PKT_TYPE_SHIFT                                     27
// DW1
#define WF_TX_FREE_DONE_EVENT_TXD_COUNT_DW                                       1
#define WF_TX_FREE_DONE_EVENT_TXD_COUNT_ADDR                                     4
#define WF_TX_FREE_DONE_EVENT_TXD_COUNT_MASK                                     0x000000ff //  7- 0
#define WF_TX_FREE_DONE_EVENT_TXD_COUNT_SHIFT                                    0
#define WF_TX_FREE_DONE_EVENT_SERIAL_ID_DW                                       1
#define WF_TX_FREE_DONE_EVENT_SERIAL_ID_ADDR                                     4
#define WF_TX_FREE_DONE_EVENT_SERIAL_ID_MASK                                     0x0000ff00 // 15- 8
#define WF_TX_FREE_DONE_EVENT_SERIAL_ID_SHIFT                                    8
#define WF_TX_FREE_DONE_EVENT_VER_DW                                             1
#define WF_TX_FREE_DONE_EVENT_VER_ADDR                                           4
#define WF_TX_FREE_DONE_EVENT_VER_MASK                                           0x000f0000 // 19-16
#define WF_TX_FREE_DONE_EVENT_VER_SHIFT                                          16
#define WF_TX_FREE_DONE_EVENT_PSE_FID_DW                                         1
#define WF_TX_FREE_DONE_EVENT_PSE_FID_ADDR                                       4
#define WF_TX_FREE_DONE_EVENT_PSE_FID_MASK                                       0xfff00000 // 31-20
#define WF_TX_FREE_DONE_EVENT_PSE_FID_SHIFT                                      20
// DW2
#define WF_TX_FREE_DONE_EVENT_WLAN_ID_DW                                         2
#define WF_TX_FREE_DONE_EVENT_WLAN_ID_ADDR                                       8
#define WF_TX_FREE_DONE_EVENT_WLAN_ID_MASK                                       0x00fff000 // 23-12
#define WF_TX_FREE_DONE_EVENT_WLAN_ID_SHIFT                                      12
#define WF_TX_FREE_DONE_EVENT_QID_DW                                             2
#define WF_TX_FREE_DONE_EVENT_QID_ADDR                                           8
#define WF_TX_FREE_DONE_EVENT_QID_MASK                                           0x7f000000 // 30-24
#define WF_TX_FREE_DONE_EVENT_QID_SHIFT                                          24
#define WF_TX_FREE_DONE_EVENT_P2_DW                                              2
#define WF_TX_FREE_DONE_EVENT_P2_ADDR                                            8
#define WF_TX_FREE_DONE_EVENT_P2_MASK                                            0x80000000 // 31-31
#define WF_TX_FREE_DONE_EVENT_P2_SHIFT                                           31
// DW3
#define WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY_DW                                  3
#define WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY_ADDR                                12
#define WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY_MASK                                0x00000fff // 11- 0
#define WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY_SHIFT                               0
#define WF_TX_FREE_DONE_EVENT_AIR_DELAY_DW                                       3
#define WF_TX_FREE_DONE_EVENT_AIR_DELAY_ADDR                                     12
#define WF_TX_FREE_DONE_EVENT_AIR_DELAY_MASK                                     0x00fff000 // 23-12
#define WF_TX_FREE_DONE_EVENT_AIR_DELAY_SHIFT                                    12
#define WF_TX_FREE_DONE_EVENT_TX_COUNT_DW                                        3
#define WF_TX_FREE_DONE_EVENT_TX_COUNT_ADDR                                      12
#define WF_TX_FREE_DONE_EVENT_TX_COUNT_MASK                                      0x0f000000 // 27-24
#define WF_TX_FREE_DONE_EVENT_TX_COUNT_SHIFT                                     24
#define WF_TX_FREE_DONE_EVENT_STAT_DW                                            3
#define WF_TX_FREE_DONE_EVENT_STAT_ADDR                                          12
#define WF_TX_FREE_DONE_EVENT_STAT_MASK                                          0x30000000 // 29-28
#define WF_TX_FREE_DONE_EVENT_STAT_SHIFT                                         28
#define WF_TX_FREE_DONE_EVENT_H3_DW                                               3
#define WF_TX_FREE_DONE_EVENT_H3_ADDR                                            12
#define WF_TX_FREE_DONE_EVENT_H3_MASK                                            0x40000000 // 30-30
#define WF_TX_FREE_DONE_EVENT_H3_SHIFT                                           30
#define WF_TX_FREE_DONE_EVENT_P3_DW                                              3
#define WF_TX_FREE_DONE_EVENT_P3_ADDR                                            12
#define WF_TX_FREE_DONE_EVENT_P3_MASK                                            0x80000000 // 31-31
#define WF_TX_FREE_DONE_EVENT_P3_SHIFT                                           31
// DW4
#define WF_TX_FREE_DONE_EVENT_MSDU_ID0_DW                                        4
#define WF_TX_FREE_DONE_EVENT_MSDU_ID0_ADDR                                      16
#define WF_TX_FREE_DONE_EVENT_MSDU_ID0_MASK                                      0x00007fff // 14- 0
#define WF_TX_FREE_DONE_EVENT_MSDU_ID0_SHIFT                                     0
#define WF_TX_FREE_DONE_EVENT_MSDU_ID1_DW                                        4
#define WF_TX_FREE_DONE_EVENT_MSDU_ID1_ADDR                                      16
#define WF_TX_FREE_DONE_EVENT_MSDU_ID1_MASK                                      0x3fff8000 // 29-15
#define WF_TX_FREE_DONE_EVENT_MSDU_ID1_SHIFT                                     15
#define WF_TX_FREE_DONE_EVENT_H4_DW                                              4
#define WF_TX_FREE_DONE_EVENT_H4_ADDR                                            16
#define WF_TX_FREE_DONE_EVENT_H4_MASK                                            0x40000000 // 30-30
#define WF_TX_FREE_DONE_EVENT_H4_SHIFT                                            30
#define WF_TX_FREE_DONE_EVENT_P4_DW                                              4
#define WF_TX_FREE_DONE_EVENT_P4_ADDR                                            16
#define WF_TX_FREE_DONE_EVENT_P4_MASK                                            0x80000000 // 31-31
#define WF_TX_FREE_DONE_EVENT_P4_SHIFT                                           31
// v0 DW0
#define WF_TX_FREE_DONE_EVENT_V0_RX_BYTE_COUNT_DW                                0
#define WF_TX_FREE_DONE_EVENT_V0_RX_BYTE_COUNT_ADDR                              0
#define WF_TX_FREE_DONE_EVENT_V0_RX_BYTE_COUNT_MASK                              0x0000ffff // 15- 0
#define WF_TX_FREE_DONE_EVENT_V0_RX_BYTE_COUNT_SHIFT                             0
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID_COUNT_DW                                0
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID_COUNT_ADDR                              0
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID_COUNT_MASK                              0x007f0000 // 22-16
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID_COUNT_SHIFT                             16
#define WF_TX_FREE_DONE_EVENT_V0_PKT_TYPE_DW                                     0
#define WF_TX_FREE_DONE_EVENT_V0_PKT_TYPE_ADDR                                   0
#define WF_TX_FREE_DONE_EVENT_V0_PKT_TYPE_MASK                                   0xe0000000 // 31-29
#define WF_TX_FREE_DONE_EVENT_V0_PKT_TYPE_SHIFT                                  29

// v0 DW2
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_DW                                     2
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_ADDR                                   8
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_MASK                                   0x0000ffff // 15- 0
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0_SHIFT                                  0
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1_DW                                     2
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1_ADDR                                   8
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1_MASK                                   0xffff0000 // 31-16
#define WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1_SHIFT                                  16

// v1/v2 DW2
#define WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0_DW                                  2
#define WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0_ADDR                                8
#define WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0_MASK                                0x00007fff // 14- 0
#define WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0_SHIFT                               0
#define WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID_DW                                   2
#define WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID_ADDR                                 8
#define WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID_MASK                                 0x01ff8000 // 24-15
#define WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID_SHIFT                                15
#define WF_TX_FREE_DONE_EVENT_V1_V2_QID_DW                                       2
#define WF_TX_FREE_DONE_EVENT_V1_V2_QID_ADDR                                     8
#define WF_TX_FREE_DONE_EVENT_V1_V2_QID_MASK                                     0xfe000000 // 31-25
#define WF_TX_FREE_DONE_EVENT_V1_V2_QID_SHIFT                                    25


// DW0
#define HAL_TX_FREE_DONE_GET_RX_BYTE_COUNT(reg32)                   READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT)
#define HAL_TX_FREE_DONE_GET_MSDU_ID_COUNT(reg32)                   READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT)
#define HAL_TX_FREE_DONE_GET_PKT_TYPE(reg32)                        READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PKT_TYPE)
// DW1
#define HAL_TX_FREE_DONE_GET_TXD_COUNT(reg32)                       READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TXD_COUNT)
#define HAL_TX_FREE_DONE_GET_SERIAL_ID(reg32)                       READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_SERIAL_ID)
#define HAL_TX_FREE_DONE_GET_VER(reg32)                             READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_VER)
#define HAL_TX_FREE_DONE_GET_PSE_FID(reg32)                         READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PSE_FID)
// DW2
#define HAL_TX_FREE_DONE_GET_WLAN_ID(reg32)                         READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_WLAN_ID)
#define HAL_TX_FREE_DONE_GET_QID(reg32)                             READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_QID)
#define HAL_TX_FREE_DONE_GET_P(reg32)                               READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)
// DW3
#define HAL_TX_FREE_DONE_GET_TRANSMIT_DELAY(reg32)                  READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY)
#define HAL_TX_FREE_DONE_GET_AIR_DELAY(reg32)                       READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_AIR_DELAY)
#define HAL_TX_FREE_DONE_GET_TX_COUNT(reg32)                        READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TX_COUNT)
#define HAL_TX_FREE_DONE_GET_STAT(reg32)                            READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_STAT)
#define HAL_TX_FREE_DONE_GET_H(reg32)                               READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H)
#define HAL_TX_FREE_DONE_GET_P(reg32)                               READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)
// DW4
#define HAL_TX_FREE_DONE_GET_MSDU_ID0(reg32)                        READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID0)
#define HAL_TX_FREE_DONE_GET_MSDU_ID1(reg32)                        READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID1)
#define HAL_TX_FREE_DONE_GET_H(reg32)                               READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H)
#define HAL_TX_FREE_DONE_GET_P(reg32)                               READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)

// V0 DW2
#define HAL_TX_FREE_DONE_V0_GET_MSDU_ID0(reg32)                     READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0)
#define HAL_TX_FREE_DONE_V0_GET_MSDU_ID1(reg32)                     READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1)

// v1 v2 DW2
#define HAL_TX_FREE_DONE_V1_V2_GET_WLAN_ID(reg32)                   READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID)
#define HAL_TX_FREE_DONE_V1_V2_GET_QID(reg32)                       READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_QID)
#define HAL_TX_FREE_DONE_V1_V2_GET_MSDU_ID0(reg32)                  READ_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0)


// DW0
#define HAL_TX_FREE_DONE_SET_RX_BYTE_COUNT(reg32, val32)            WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT, val32)
#define HAL_TX_FREE_DONE_SET_MSDU_ID_COUNT(reg32, val32)            WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT, val32)
#define HAL_TX_FREE_DONE_SET_PKT_TYPE(reg32, val32)                 WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PKT_TYPE, val32)
// DW1
#define HAL_TX_FREE_DONE_SET_TXD_COUNT(reg32, val32)                WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TXD_COUNT, val32)
#define HAL_TX_FREE_DONE_SET_SERIAL_ID(reg32, val32)                WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_SERIAL_ID, val32)
#define HAL_TX_FREE_DONE_SET_VER(reg32, val32)                      WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_VER, val32)
#define HAL_TX_FREE_DONE_SET_PSE_FID(reg32, val32)                  WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PSE_FID, val32)
// DW2
#define HAL_TX_FREE_DONE_SET_WLAN_ID(reg32, val32)                  WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_WLAN_ID, val32)
#define HAL_TX_FREE_DONE_SET_QID(reg32, val32)                      WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_QID, val32)
#define HAL_TX_FREE_DONE_SET_P(reg32, val32)                        WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P, val32)
// DW3
#define HAL_TX_FREE_DONE_SET_TRANSMIT_DELAY(reg32, val32)           WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY, val32)
#define HAL_TX_FREE_DONE_SET_AIR_DELAY(reg32, val32)                WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_AIR_DELAY, val32)
#define HAL_TX_FREE_DONE_SET_TX_COUNT(reg32, val32)                 WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TX_COUNT, val32)
#define HAL_TX_FREE_DONE_SET_STAT(reg32, val32)                     WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_STAT, val32)
#define HAL_TX_FREE_DONE_SET_H(reg32, val32)                        WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H, val32)
#define HAL_TX_FREE_DONE_SET_P(reg32, val32)                        WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P, val32)
// DW4
#define HAL_TX_FREE_DONE_SET_MSDU_ID0(reg32, val32)                 WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID0, val32)
#define HAL_TX_FREE_DONE_SET_MSDU_ID1(reg32, val32)                 WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID1, val32)
#define HAL_TX_FREE_DONE_SET_H(reg32, val32)                        WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H, val32)
#define HAL_TX_FREE_DONE_SET_P(reg32, val32)                        WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P, val32)

// V0 DW2
#define HAL_TX_FREE_DONE_V0_SET_MSDU_ID0(reg32, val32)              WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V0_MSDU_ID0, val32)
#define HAL_TX_FREE_DONE_V0_SET_MSDU_ID1(reg32, val32)              WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V0_MSDU_ID1, val32)

//v1 v2 DW2
#define HAL_TX_FREE_DONE_V1_V2_SET_WLAN_ID(reg32, val32)            WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID, val32)
#define HAL_TX_FREE_DONE_V1_V2_SET_QID(reg32, val32)                WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_QID, val32)
#define HAL_TX_FREE_DONE_V1_V2_SET_MSDU_ID0(reg32, val32)           WRITE_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_MSDU_ID0, val32)

// DW0
#define HAL_TX_FREE_DONE_CLR_RX_BYTE_COUNT(reg32)                   CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_RX_BYTE_COUNT)
#define HAL_TX_FREE_DONE_CLR_MSDU_ID_COUNT(reg32)                   CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID_COUNT)
#define HAL_TX_FREE_DONE_CLR_PKT_TYPE(reg32)                        CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PKT_TYPE)
// DW1
#define HAL_TX_FREE_DONE_CLR_TXD_COUNT(reg32)                       CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TXD_COUNT)
#define HAL_TX_FREE_DONE_CLR_SERIAL_ID(reg32)                       CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_SERIAL_ID)
#define HAL_TX_FREE_DONE_CLR_VER(reg32)                             CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_VER)
#define HAL_TX_FREE_DONE_CLR_PSE_FID(reg32)                         CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_PSE_FID)
// DW2
#define HAL_TX_FREE_DONE_CLR_WLAN_ID(reg32)                         CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_WLAN_ID)
#define HAL_TX_FREE_DONE_CLR_QID(reg32)                             CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_QID)
#define HAL_TX_FREE_DONE_CLR_P(reg32)                               CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)
// DW3
#define HAL_TX_FREE_DONE_CLR_TRANSMIT_DELAY(reg32)                  CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TRANSMIT_DELAY)
#define HAL_TX_FREE_DONE_CLR_AIR_DELAY(reg32)                       CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_AIR_DELAY)
#define HAL_TX_FREE_DONE_CLR_TX_COUNT(reg32)                        CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_TX_COUNT)
#define HAL_TX_FREE_DONE_CLR_STAT(reg32)                            CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_STAT)
#define HAL_TX_FREE_DONE_CLR_H(reg32)                               CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H)
#define HAL_TX_FREE_DONE_CLR_P(reg32)                               CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)
// DW4
#define HAL_TX_FREE_DONE_CLR_MSDU_ID0(reg32)                        CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID0)
#define HAL_TX_FREE_DONE_CLR_MSDU_ID1(reg32)                        CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_MSDU_ID1)
#define HAL_TX_FREE_DONE_CLR_H(reg32)                               CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_H)
#define HAL_TX_FREE_DONE_CLR_P(reg32)                               CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_P)

// V0 DW2
#define HAL_TX_FREE_DONE_V0_CLR_MSDU_ID0(reg32)                     CLEAR_FIELD((reg32), WF_TX_FREE_DONE_V0_EVENT_MSDU_ID0)
#define HAL_TX_FREE_DONE_V0_CLR_MSDU_ID1(reg32)                     CLEAR_FIELD((reg32), WF_TX_FREE_DONE_V0_EVENT_MSDU_ID1)

// v1 v2 DW2
#define HAL_TX_FREE_DONE_V1_V2_CLR_WLAN_ID(reg32)                   CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_WLAN_ID)
#define HAL_TX_FREE_DONE_V1_V2_CLR_QID(reg32)                       CLEAR_FIELD((reg32), WF_TX_FREE_DONE_EVENT_V1_V2_QID)
#define HAL_TX_FREE_DONE_V1_V2_CLR_MSDU_ID0(reg32)                  CLEAR_FIELD((reg32), WF_TX_FREE_DONE_V1_V2_EVENT_MSDU_ID0)


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __WF_TX_FREE_DONE_EVENT_REGS_H__
