
#ifndef __PAO0_GATHER_ONLY_REGS_H__
#define __PAO0_GATHER_ONLY_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     PAO0_GATHER_ONLY CR Definitions
//
//****************************************************************************

#define WED_PAO_AMSDU_FIFO_ADDR               0x00001800
#define WED_PAO_AMSDU_FIFO_CFG_ADDR           0x00001804
#define WED_PAO_TIMER_ADDR                    0x00001808
#define WED_PAO_TIMER_JUMBO_ADDR              0x0000180C
#define WED_PAO_TX_BYTE_CNT_CFG0_ADDR         0x00001830
#define WED_PAO_WPDMAD_CFG0_ADDR              0x00001960
#define WED_PAO_WPDMAD_CFG1_ADDR              0x00001964
#define WED_PAO_HIFTXD_ADDR                   0x00001968
#define WED_PAO_MON_AMSDU_FIFO_ADDR           0x00001A30
#define WED_PAO_MON_AMSDU_FIFO_CNT0_ADDR      0x00001A34
#define WED_PAO_MON_AMSDU_FIFO_CNT1_ADDR      0x00001A38
#define WED_PAO_MON_AMSDU_FIFO_CNT2_ADDR      0x00001A3C
#define WED_PAO_MON_AMSDU_FIFO_CNT3_ADDR      0x00001A40
#define WED_PAO_MON_AMSDU_FIFO_CNT4_ADDR      0x00001A44
#define WED_PAO_MON_AMSDU_FIFO_CNT5_ADDR      0x00001A48
#define WED_PAO_MON_AMSDU_FIFO_CNT6_ADDR      0x00001A4C
#define WED_PAO_MON_AMSDU_FIFO_CNT7_ADDR      0x00001A50
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ADDR      0x00001A54
#define WED_PAO_MON_AMSDU_FIFO_CNT9_ADDR      0x00001A58
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_ADDR    0x00001A5C
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_ADDR    0x00001A60
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_ADDR    0x00001A64
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_ADDR    0x00001A68
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_ADDR    0x00001A6C
#define WED_PAO_MON_HIFTXD_FETCH_ADDR         0x00001E70
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_ADDR    0x00001E74
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_ADDR    0x00001E78
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_ADDR    0x00001E7C
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_ADDR    0x00001E80
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_ADDR 0x00001E90
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_ADDR 0x00001E94
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_ADDR 0x00001E98
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_ADDR 0x00001E9C
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_ADDR 0x00001EA0
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_ADDR 0x00001EA4
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_ADDR 0x00001EA8
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_ADDR 0x00001EAC
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_ADDR 0x00001EB0
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_ADDR 0x00001EB4
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_ADDR 0x00001EB8
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_ADDR 0x00001EBC
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_ADDR 0x00001EC0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_ADDR 0x00001EC4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_ADDR 0x00001EC8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_ADDR 0x00001ECC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_ADDR 0x00001ED0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_ADDR 0x00001ED4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_ADDR 0x00001ED8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_ADDR 0x00001EDC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_ADDR 0x00001EE0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_ADDR 0x00001EE4
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_ADDR 0x00001EE8
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_ADDR 0x00001EEC
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_ADDR 0x00001EF0
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_ADDR 0x00001EF4
#define WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB_ADDR 0x00001EF8
#define WED_PAO_AMSDU_FIFO_ERR_ADDR           0x00001FD0
#define WED_PAO_DBG_CFG_ADDR                  0x00001FE0
#define WED_PAO_DBG_CNT_ADDR                  0x00001FE4
#define WED_PAO_DBG_ADDR                      0x00001FE8

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO (0x15011800 + 0x000)---

    MIN_SDL_SIZE[9..0]           - (RW) IF SDL < MIN_SDL_SIZE, PAO change AMSDU_EN to 1'b0  (i.e.  By default could be merged packet SDL at least ETH header 14 bytes +  payload 1 byte)
    IS_PRIOR0_RING[10]           - (RW) Set IS_PRIOR=0 to WFDMA ring#
    IS_PRIOR1_RING[11]           - (RW) Set IS_PRIOR=1 to WFDMA ring#
    ETHER_TYPE_MRG[12]           - (RW) Change AMSDU_EN when EtherType<0x0600
    ETHER_TYPE_CFG[13]           - (RW) Change EtherType value when EtherType<0x0600
    IS_JUMBO_MRG[14]             - (RW) Change AMSDU_EN when jumbo gather/scatter
    ILLEGAL_TID_ERR_DROP_EN[15]  - (RW) When receive DMAD.TID > 7, then return DMAD back to TXBM
    JUMBO_TIMEOUT_DROP_EN[16]    - (RW) When receive jumbo DMAD, if the latter coming DMAD is waiting too long time, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_BUF_NUM_EXCEED_DROP_EN[17] - (RW) When receive jumbo DMAD, if the coming DMAD exceeds max, then return DMAD back to TXBM
    JUMBO_ID_ERR_1_DROP_EN[18]   - (RW) When receive jumbo DMAD, if the coming DMAD ID mismatch, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_ID_ERR_0_DROP_EN[19]   - (RW) When receive jumbo DMAD, if the coming DMAD ID mismatch, then drop current merged jumbo DMAD, for debug mode only
    JUMBO_ERR_STOP_EN[20]        - (RW) Stop AMSDU_FIFO FSM when Error occur, send clr to release FSM to IDLE, for debug mode only
    LATCH_HIFTXD_OFF[21]         - (RW) Latch the latest partial HIFTXD when dispatch to AMSDU_ENG
    DMAD_LAT_OPT[25..22]         - (RW) Latch DMAD when dispatch to AMSDU_ENG
    WARP3P1_DSCP_OPT_EN[26]      - (RW) WARP3.1 new added
                                     WARP3.1 DSCP option
    RESERVED27[27]               - (RO) Reserved bits
    WITH_PRT_PLD_MAX_BUF_NUM[31..28] - (RW) Set the maximum DMAD counts of RXDMAD.with_prt_pld=1

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_WITH_PRT_PLD_MAX_BUF_NUM_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_WITH_PRT_PLD_MAX_BUF_NUM_MASK 0xF0000000                // WITH_PRT_PLD_MAX_BUF_NUM[31..28]
#define WED_PAO_AMSDU_FIFO_WITH_PRT_PLD_MAX_BUF_NUM_SHFT 28
#define WED_PAO_AMSDU_FIFO_WARP3P1_DSCP_OPT_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_WARP3P1_DSCP_OPT_EN_MASK 0x04000000                // WARP3P1_DSCP_OPT_EN[26]
#define WED_PAO_AMSDU_FIFO_WARP3P1_DSCP_OPT_EN_SHFT 26
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_ADDR  WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_MASK  0x03C00000                // DMAD_LAT_OPT[25..22]
#define WED_PAO_AMSDU_FIFO_DMAD_LAT_OPT_SHFT  22
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_MASK 0x00200000                // LATCH_HIFTXD_OFF[21]
#define WED_PAO_AMSDU_FIFO_LATCH_HIFTXD_OFF_SHFT 21
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_MASK 0x00100000                // JUMBO_ERR_STOP_EN[20]
#define WED_PAO_AMSDU_FIFO_JUMBO_ERR_STOP_EN_SHFT 20
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_MASK 0x00080000                // JUMBO_ID_ERR_0_DROP_EN[19]
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_0_DROP_EN_SHFT 19
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_MASK 0x00040000                // JUMBO_ID_ERR_1_DROP_EN[18]
#define WED_PAO_AMSDU_FIFO_JUMBO_ID_ERR_1_DROP_EN_SHFT 18
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_MASK 0x00020000                // JUMBO_BUF_NUM_EXCEED_DROP_EN[17]
#define WED_PAO_AMSDU_FIFO_JUMBO_BUF_NUM_EXCEED_DROP_EN_SHFT 17
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_MASK 0x00010000                // JUMBO_TIMEOUT_DROP_EN[16]
#define WED_PAO_AMSDU_FIFO_JUMBO_TIMEOUT_DROP_EN_SHFT 16
#define WED_PAO_AMSDU_FIFO_ILLEGAL_TID_ERR_DROP_EN_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_ILLEGAL_TID_ERR_DROP_EN_MASK 0x00008000                // ILLEGAL_TID_ERR_DROP_EN[15]
#define WED_PAO_AMSDU_FIFO_ILLEGAL_TID_ERR_DROP_EN_SHFT 15
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_ADDR  WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_MASK  0x00004000                // IS_JUMBO_MRG[14]
#define WED_PAO_AMSDU_FIFO_IS_JUMBO_MRG_SHFT  14
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_MASK 0x00002000                // ETHER_TYPE_CFG[13]
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_CFG_SHFT 13
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_MASK 0x00001000                // ETHER_TYPE_MRG[12]
#define WED_PAO_AMSDU_FIFO_ETHER_TYPE_MRG_SHFT 12
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_MASK 0x00000800                // IS_PRIOR1_RING[11]
#define WED_PAO_AMSDU_FIFO_IS_PRIOR1_RING_SHFT 11
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_ADDR WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_MASK 0x00000400                // IS_PRIOR0_RING[10]
#define WED_PAO_AMSDU_FIFO_IS_PRIOR0_RING_SHFT 10
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_ADDR  WED_PAO_AMSDU_FIFO_ADDR
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_MASK  0x000003FF                // MIN_SDL_SIZE[9..0]
#define WED_PAO_AMSDU_FIFO_MIN_SDL_SIZE_SHFT  0

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO_CFG (0x15011800 + 0x004)---

    RESERVED0[27..0]             - (RO) Reserved bits
    WO_PRT_PLD_MAX_BUF_NUM[31..28] - (RW) Set the maximum DMAD counts of RXDMAD.with_prt_pld=0

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_CFG_WO_PRT_PLD_MAX_BUF_NUM_ADDR WED_PAO_AMSDU_FIFO_CFG_ADDR
#define WED_PAO_AMSDU_FIFO_CFG_WO_PRT_PLD_MAX_BUF_NUM_MASK 0xF0000000                // WO_PRT_PLD_MAX_BUF_NUM[31..28]
#define WED_PAO_AMSDU_FIFO_CFG_WO_PRT_PLD_MAX_BUF_NUM_SHFT 28

/* =====================================================================================

  ---WED_PAO_TIMER (0x15011800 + 0x008)---

    DIV[2..0]                    - (RW) Timer Tick Freq. DIV(x1 125M)
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TIMER_DIV_ADDR                WED_PAO_TIMER_ADDR
#define WED_PAO_TIMER_DIV_MASK                0x00000007                // DIV[2..0]
#define WED_PAO_TIMER_DIV_SHFT                0

/* =====================================================================================

  ---WED_PAO_TIMER_JUMBO (0x15011800 + 0x00C)---

    CYCLE[31..0]                 - (RW) Timer for AMSDU_FIFO (jumbo error detect), maximum cycle

 =====================================================================================*/
#define WED_PAO_TIMER_JUMBO_CYCLE_ADDR        WED_PAO_TIMER_JUMBO_ADDR
#define WED_PAO_TIMER_JUMBO_CYCLE_MASK        0xFFFFFFFF                // CYCLE[31..0]
#define WED_PAO_TIMER_JUMBO_CYCLE_SHFT        0

/* =====================================================================================

  ---WED_PAO_TX_BYTE_CNT_CFG0 (0x15011800 + 0x030)---

    RESERVED0[11..0]             - (RO) Reserved bits
    TXD_LEN[17..12]              - (RW) Constant for TX_BYTE_CNT calculation
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_ADDR WED_PAO_TX_BYTE_CNT_CFG0_ADDR
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_MASK 0x0003F000                // TXD_LEN[17..12]
#define WED_PAO_TX_BYTE_CNT_CFG0_TXD_LEN_SHFT 12

/* =====================================================================================

  ---WED_PAO_WPDMAD_CFG0 (0x15011800 + 0x160)---

    SDP1_ZERO[0]                 - (RW) WPDMA_TXDMAD.SDP1 force zero
    SDP0_ZERO[1]                 - (RW) WPDMA_TXDMAD.SDP0 force zero
    BURST[2]                     - (RW) WPDMA_TXDMAD.BURST value config
    LS1[3]                       - (RW) WPDMA_TXDMAD.LS1 value config
    LS0[4]                       - (RW) WPDMA_TXDMAD.LS0 value config
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_WPDMAD_CFG0_LS0_ADDR          WED_PAO_WPDMAD_CFG0_ADDR
#define WED_PAO_WPDMAD_CFG0_LS0_MASK          0x00000010                // LS0[4]
#define WED_PAO_WPDMAD_CFG0_LS0_SHFT          4
#define WED_PAO_WPDMAD_CFG0_LS1_ADDR          WED_PAO_WPDMAD_CFG0_ADDR
#define WED_PAO_WPDMAD_CFG0_LS1_MASK          0x00000008                // LS1[3]
#define WED_PAO_WPDMAD_CFG0_LS1_SHFT          3
#define WED_PAO_WPDMAD_CFG0_BURST_ADDR        WED_PAO_WPDMAD_CFG0_ADDR
#define WED_PAO_WPDMAD_CFG0_BURST_MASK        0x00000004                // BURST[2]
#define WED_PAO_WPDMAD_CFG0_BURST_SHFT        2
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_ADDR    WED_PAO_WPDMAD_CFG0_ADDR
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_MASK    0x00000002                // SDP0_ZERO[1]
#define WED_PAO_WPDMAD_CFG0_SDP0_ZERO_SHFT    1
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_ADDR    WED_PAO_WPDMAD_CFG0_ADDR
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_MASK    0x00000001                // SDP1_ZERO[0]
#define WED_PAO_WPDMAD_CFG0_SDP1_ZERO_SHFT    0

/* =====================================================================================

  ---WED_PAO_WPDMAD_CFG1 (0x15011800 + 0x164)---

    SDL1[13..0]                  - (RW) WPDMA_TXDMAD.SDL1 valud config.
    SDL0[27..14]                 - (RW) WPDMA_TXDMAD.SDL0 value config. Default=128 (HIF_TXD_V2 length)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_WPDMAD_CFG1_SDL0_ADDR         WED_PAO_WPDMAD_CFG1_ADDR
#define WED_PAO_WPDMAD_CFG1_SDL0_MASK         0x0FFFC000                // SDL0[27..14]
#define WED_PAO_WPDMAD_CFG1_SDL0_SHFT         14
#define WED_PAO_WPDMAD_CFG1_SDL1_ADDR         WED_PAO_WPDMAD_CFG1_ADDR
#define WED_PAO_WPDMAD_CFG1_SDL1_MASK         0x00003FFF                // SDL1[13..0]
#define WED_PAO_WPDMAD_CFG1_SDL1_SHFT         0

/* =====================================================================================

  ---WED_PAO_HIFTXD (0x15011800 + 0x168)---

    SIZE[3..0]                   - (RW) HIFTXD size, for calculate HIFTXD physical address pointer
    v1_opt_for_dv[4]             - (RW) HIFTXD_V1 option for DV using, fill USER_PRIORITY and IS_SP field
    wo_prt_pld_hif_version[5]    - (RW) This option is HIFTXD version format PAO will form when RXDMAD.WITH_PRT_PLD==0
    RESERVED6[9..6]              - (RO) Reserved bits
    IS_NON_CIPHER[10]            - (RW) HIFTXD.IS_NON_CIPHER value config.
    BC_MC_FLAG[12..11]           - (RW) HIFTXD.BC_MC_FLAG value config.
    IS_FR_HOST[13]               - (RW) HIFTXD.IS_FR_HOST value config.
    FORCE_LINK[14]               - (RW) HIFTXD.FORCE_LINK value config.
    SRC[16..15]                  - (RW) HIFTXD.SRC value config.
    HIF_VERSION[20..17]          - (RW) HIFTXD.HIF_VERSION value config.
    ETHER_TYPE_BSWAP[21]         - (RW) HIFTXD.ETHER_TYPE byte swap
    RESERVED22[31..22]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_ADDR  WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_MASK  0x00200000                // ETHER_TYPE_BSWAP[21]
#define WED_PAO_HIFTXD_ETHER_TYPE_BSWAP_SHFT  21
#define WED_PAO_HIFTXD_HIF_VERSION_ADDR       WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_HIF_VERSION_MASK       0x001E0000                // HIF_VERSION[20..17]
#define WED_PAO_HIFTXD_HIF_VERSION_SHFT       17
#define WED_PAO_HIFTXD_SRC_ADDR               WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_SRC_MASK               0x00018000                // SRC[16..15]
#define WED_PAO_HIFTXD_SRC_SHFT               15
#define WED_PAO_HIFTXD_FORCE_LINK_ADDR        WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_FORCE_LINK_MASK        0x00004000                // FORCE_LINK[14]
#define WED_PAO_HIFTXD_FORCE_LINK_SHFT        14
#define WED_PAO_HIFTXD_IS_FR_HOST_ADDR        WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_IS_FR_HOST_MASK        0x00002000                // IS_FR_HOST[13]
#define WED_PAO_HIFTXD_IS_FR_HOST_SHFT        13
#define WED_PAO_HIFTXD_BC_MC_FLAG_ADDR        WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_BC_MC_FLAG_MASK        0x00001800                // BC_MC_FLAG[12..11]
#define WED_PAO_HIFTXD_BC_MC_FLAG_SHFT        11
#define WED_PAO_HIFTXD_IS_NON_CIPHER_ADDR     WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_IS_NON_CIPHER_MASK     0x00000400                // IS_NON_CIPHER[10]
#define WED_PAO_HIFTXD_IS_NON_CIPHER_SHFT     10
#define WED_PAO_HIFTXD_wo_prt_pld_hif_version_ADDR WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_wo_prt_pld_hif_version_MASK 0x00000020                // wo_prt_pld_hif_version[5]
#define WED_PAO_HIFTXD_wo_prt_pld_hif_version_SHFT 5
#define WED_PAO_HIFTXD_v1_opt_for_dv_ADDR     WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_v1_opt_for_dv_MASK     0x00000010                // v1_opt_for_dv[4]
#define WED_PAO_HIFTXD_v1_opt_for_dv_SHFT     4
#define WED_PAO_HIFTXD_SIZE_ADDR              WED_PAO_HIFTXD_ADDR
#define WED_PAO_HIFTXD_SIZE_MASK              0x0000000F                // SIZE[3..0]
#define WED_PAO_HIFTXD_SIZE_SHFT              0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO (0x15011800 + 0x230)---

    FSM[8..0]                    - (RO) AMSDU_FIFO module FSM
    RESERVED9[31..9]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_FSM_ADDR       WED_PAO_MON_AMSDU_FIFO_ADDR
#define WED_PAO_MON_AMSDU_FIFO_FSM_MASK       0x000001FF                // FSM[8..0]
#define WED_PAO_MON_AMSDU_FIFO_FSM_SHFT       0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT0 (0x15011800 + 0x234)---

    DMAD[31..0]                  - (RO) AMSDU_FIFO DMAD counter, how many DMAD input

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_ADDR WED_PAO_MON_AMSDU_FIFO_CNT0_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_MASK 0xFFFFFFFF                // DMAD[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT0_DMAD_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT1 (0x15011800 + 0x238)---

    JUMBO_MSDU_CNT[31..0]        - (RO) AMSDU_FIFO MSDU counter, count jumbo MSDU (partial HIFTXD, BUF_NUM!=0, MSDU_CNT==1)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_ADDR WED_PAO_MON_AMSDU_FIFO_CNT1_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_MASK 0xFFFFFFFF                // JUMBO_MSDU_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT1_JUMBO_MSDU_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT2 (0x15011800 + 0x23C)---

    IS_SP_DMAD_CNT[31..0]        - (RO) AMSDU_FIFO DMAD counter, count IS_SP MSDU (partial HIFTXD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_ADDR WED_PAO_MON_AMSDU_FIFO_CNT2_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_MASK 0xFFFFFFFF                // IS_SP_DMAD_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT2_IS_SP_DMAD_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT3 (0x15011800 + 0x240)---

    AMSDU_EN_0_CNT0[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (original field value in WDMA Rx DMAD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_ADDR WED_PAO_MON_AMSDU_FIFO_CNT3_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_MASK 0xFFFFFFFF                // AMSDU_EN_0_CNT0[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT3_AMSDU_EN_0_CNT0_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT4 (0x15011800 + 0x244)---

    AMSDU_EN_0_CNT1[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if SDL< MIN_SDL, always check)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_ADDR WED_PAO_MON_AMSDU_FIFO_CNT4_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_MASK 0xFFFFFFFF                // AMSDU_EN_0_CNT1[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT4_AMSDU_EN_0_CNT1_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT5 (0x15011800 + 0x248)---

    AMSDU_EN_0_CNT2[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if ETHER_TYPE<16'h0600, has CR option)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_ADDR WED_PAO_MON_AMSDU_FIFO_CNT5_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_MASK 0xFFFFFFFF                // AMSDU_EN_0_CNT2[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT5_AMSDU_EN_0_CNT2_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT6 (0x15011800 + 0x24C)---

    AMSDU_EN_0_CNT3[31..0]       - (RO) AMSDU_FIFO DMAD counter, count AMSDU_EN=0 MSDU (partial HIFTXD) (overwrite if JUMBO_MSDU, has CR option)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_ADDR WED_PAO_MON_AMSDU_FIFO_CNT6_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_MASK 0xFFFFFFFF                // AMSDU_EN_0_CNT3[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT6_AMSDU_EN_0_CNT3_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT7 (0x15011800 + 0x250)---

    MSDU_CNT[31..0]              - (RO) AMSDU_FIFO MSDU counter, count MSDU (partial HIFTXD)

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_ADDR WED_PAO_MON_AMSDU_FIFO_CNT7_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_MASK 0xFFFFFFFF                // MSDU_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT7_MSDU_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT8 (0x15011800 + 0x254)---

    ETHER_TYPE_CNT[31..0]        - (RO) AMSDU_FIFO MSDU counter, count ETHER_TYPE<16'h0600

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_ADDR WED_PAO_MON_AMSDU_FIFO_CNT8_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_MASK 0xFFFFFFFF                // ETHER_TYPE_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT8_ETHER_TYPE_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_CNT9 (0x15011800 + 0x258)---

    DROP_MSDU_CNT[31..0]         - (RO) AMSDU_FIFO drop MSDU counter, count drop MSDU when buffer number exceed drop enable or illegal tid drop enable.

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_CNT9_DROP_MSDU_CNT_ADDR WED_PAO_MON_AMSDU_FIFO_CNT9_ADDR
#define WED_PAO_MON_AMSDU_FIFO_CNT9_DROP_MSDU_CNT_MASK 0xFFFFFFFF                // DROP_MSDU_CNT[31..0]
#define WED_PAO_MON_AMSDU_FIFO_CNT9_DROP_MSDU_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_0 (0x15011800 + 0x25C)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_ADDR WED_PAO_MON_AMSDU_FIFO_DMAD_0_ADDR
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_MASK 0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_0_DATA_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_1 (0x15011800 + 0x260)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_ADDR WED_PAO_MON_AMSDU_FIFO_DMAD_1_ADDR
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_MASK 0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_1_DATA_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_2 (0x15011800 + 0x264)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_ADDR WED_PAO_MON_AMSDU_FIFO_DMAD_2_ADDR
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_MASK 0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_2_DATA_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_3 (0x15011800 + 0x268)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_ADDR WED_PAO_MON_AMSDU_FIFO_DMAD_3_ADDR
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_MASK 0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_3_DATA_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_AMSDU_FIFO_DMAD_4 (0x15011800 + 0x26C)---

    DATA[31..0]                  - (RO) AMSDU_FIFO latch DMAD data

 =====================================================================================*/
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_ADDR WED_PAO_MON_AMSDU_FIFO_DMAD_4_ADDR
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_MASK 0xFFFFFFFF                // DATA[31..0]
#define WED_PAO_MON_AMSDU_FIFO_DMAD_4_DATA_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH (0x15011800 + 0x670)---

    STS[31..0]                   - (RO) PAO HIFTXD_FETCH status

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_STS_ADDR     WED_PAO_MON_HIFTXD_FETCH_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_STS_MASK     0xFFFFFFFF                // STS[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_STS_SHFT     0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT0 (0x15011800 + 0x674)---

    HIFTXD_IN_CNT[31..0]         - (RO) PAO HIFTX_FETCH input Partial HIFTXD count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_CNT0_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_MASK 0xFFFFFFFF                // HIFTXD_IN_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT0_HIFTXD_IN_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT1 (0x15011800 + 0x678)---

    HIFTXD_OU1_CNT[31..0]        - (RO) PAO HIFTX_FETCH output HIFTXD to Ring0 count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_CNT1_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_MASK 0xFFFFFFFF                // HIFTXD_OU1_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT1_HIFTXD_OU1_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT2 (0x15011800 + 0x67C)---

    HIFTXD_OU2_CNT[31..0]        - (RO) PAO HIFTX_FETCH output HIFTXD to Ring1 count

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_CNT2_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_MASK 0xFFFFFFFF                // HIFTXD_OU2_CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT2_HIFTXD_OU2_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_CNT3 (0x15011800 + 0x680)---

    CURQ_CNT[11..0]              - (RO) PAO QMEM output-queue entry count
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_CNT3_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_MASK 0x00000FFF                // CURQ_CNT[11..0]
#define WED_PAO_MON_HIFTXD_FETCH_CNT3_CURQ_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1 (0x15011800 + 0x690)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=1 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_1_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2 (0x15011800 + 0x694)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=2 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_2_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3 (0x15011800 + 0x698)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=3 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_3_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4 (0x15011800 + 0x69C)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=4 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_4_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5 (0x15011800 + 0x6A0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=5 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_5_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6 (0x15011800 + 0x6A4)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=6 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_6_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7 (0x15011800 + 0x6A8)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=7 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_7_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8 (0x15011800 + 0x6AC)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=8 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_8_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9 (0x15011800 + 0x6B0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=9 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_9_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10 (0x15011800 + 0x6B4)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=10 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_10_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11 (0x15011800 + 0x6B8)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=11 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_11_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12 (0x15011800 + 0x6BC)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=12 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_12_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13 (0x15011800 + 0x6C0)---

    CNT[31..0]                   - (RO) HIFTXD BUF_NUM=13 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_BUF_NUM_13_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1 (0x15011800 + 0x6C4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=1 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_1_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2 (0x15011800 + 0x6C8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=2 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_2_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3 (0x15011800 + 0x6CC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=3 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_3_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4 (0x15011800 + 0x6D0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=4 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_4_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5 (0x15011800 + 0x6D4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=5 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_5_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6 (0x15011800 + 0x6D8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=6 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_6_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7 (0x15011800 + 0x6DC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=7 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_7_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8 (0x15011800 + 0x6E0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=8 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_8_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9 (0x15011800 + 0x6E4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=9 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_9_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10 (0x15011800 + 0x6E8)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=10 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_10_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11 (0x15011800 + 0x6EC)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=11 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_11_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12 (0x15011800 + 0x6F0)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=12 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_12_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13 (0x15011800 + 0x6F4)---

    CNT[31..0]                   - (RO) HIFTXD MSDU_CNT=13 counter

 =====================================================================================*/
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_ADDR WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_ADDR
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_MON_HIFTXD_FETCH_MSDU_CNT_13_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB (0x15011800 + 0x6F8)---

    CNT[31..0]                   - (RC) Counter of DMAD from PAO to TXBM, when buffer number exceed drop enable or illegal tid drop enable.

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB_CNT_ADDR WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB_ADDR
#define WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB_CNT_MASK 0xFFFFFFFF                // CNT[31..0]
#define WED_PAO_AMSDU_FIFO_TO_TX_BM_DMAD_MIB_CNT_SHFT 0

/* =====================================================================================

  ---WED_PAO_AMSDU_FIFO_ERR (0x15011800 + 0x7D0)---

    JUMBO_BUF_NUM_ERR[0]         - (W1C) When receive jumbo (gather/scatter) MSDU, the merged DMADs exceed MAX, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_TIMEOUT_ERR[1]         - (W1C) When receive jumbo (gather/scatter) MSDU, the latter DMADs long time no comming, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_ID_ERR_0[2]            - (W1C) When receive jumbo (gather/scatter) MSDU, the JUMBO ID mismatch, HW will set this flag to 1'b1, SW write 1 clear
    JUMBO_ID_ERR_1[3]            - (W1C) When receive jumbo (gather/scatter) MSDU, the JUMBO ID mismatch, HW will set this flag to 1'b1, SW write 1 clear
    Illegal_TID_ERR[4]           - (W1C) When receive DMAD.TID > 7, HW will set this flag to 1'b1, SW write 1 clear
    RESERVED5[30..5]             - (RO) Reserved bits
    CLR[31]                      - (A0) Clear AMSDU_FIFO Error Status

 =====================================================================================*/
#define WED_PAO_AMSDU_FIFO_ERR_CLR_ADDR       WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_CLR_MASK       0x80000000                // CLR[31]
#define WED_PAO_AMSDU_FIFO_ERR_CLR_SHFT       31
#define WED_PAO_AMSDU_FIFO_ERR_Illegal_TID_ERR_ADDR WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_Illegal_TID_ERR_MASK 0x00000010                // Illegal_TID_ERR[4]
#define WED_PAO_AMSDU_FIFO_ERR_Illegal_TID_ERR_SHFT 4
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_ADDR WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_MASK 0x00000008                // JUMBO_ID_ERR_1[3]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_1_SHFT 3
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_ADDR WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_MASK 0x00000004                // JUMBO_ID_ERR_0[2]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_ID_ERR_0_SHFT 2
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_ADDR WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_MASK 0x00000002                // JUMBO_TIMEOUT_ERR[1]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_TIMEOUT_ERR_SHFT 1
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_ADDR WED_PAO_AMSDU_FIFO_ERR_ADDR
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_MASK 0x00000001                // JUMBO_BUF_NUM_ERR[0]
#define WED_PAO_AMSDU_FIFO_ERR_JUMBO_BUF_NUM_ERR_SHFT 0

/* =====================================================================================

  ---WED_PAO_DBG_CFG (0x15011800 + 0x7E0)---

    SEL[7..0]                    - (RW) PAO Debug Interface, PAO debug signal selection
    RESERVED8[11..8]             - (RO) Reserved bits
    MSEL[15..12]                 - (RW) PAO Debug Interface, PAO module selection
    PSEL[17..16]                 - (RW) PAO Debug Interface, PAO instance selection
    EN[18]                       - (RW) PAO Debug Interface, Enable
    RESERVED19[31..19]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_DBG_CFG_EN_ADDR               WED_PAO_DBG_CFG_ADDR
#define WED_PAO_DBG_CFG_EN_MASK               0x00040000                // EN[18]
#define WED_PAO_DBG_CFG_EN_SHFT               18
#define WED_PAO_DBG_CFG_PSEL_ADDR             WED_PAO_DBG_CFG_ADDR
#define WED_PAO_DBG_CFG_PSEL_MASK             0x00030000                // PSEL[17..16]
#define WED_PAO_DBG_CFG_PSEL_SHFT             16
#define WED_PAO_DBG_CFG_MSEL_ADDR             WED_PAO_DBG_CFG_ADDR
#define WED_PAO_DBG_CFG_MSEL_MASK             0x0000F000                // MSEL[15..12]
#define WED_PAO_DBG_CFG_MSEL_SHFT             12
#define WED_PAO_DBG_CFG_SEL_ADDR              WED_PAO_DBG_CFG_ADDR
#define WED_PAO_DBG_CFG_SEL_MASK              0x000000FF                // SEL[7..0]
#define WED_PAO_DBG_CFG_SEL_SHFT              0

/* =====================================================================================

  ---WED_PAO_DBG_CNT (0x15011800 + 0x7E4)---

    CLR[0]                       - (A0) PAO Debug Interface, clear counter
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_PAO_DBG_CNT_CLR_ADDR              WED_PAO_DBG_CNT_ADDR
#define WED_PAO_DBG_CNT_CLR_MASK              0x00000001                // CLR[0]
#define WED_PAO_DBG_CNT_CLR_SHFT              0

/* =====================================================================================

  ---WED_PAO_DBG (0x15011800 + 0x7E8)---

    INFO[31..0]                  - (RO) PAO Debug Interface, read debug info.

 =====================================================================================*/
#define WED_PAO_DBG_INFO_ADDR                 WED_PAO_DBG_ADDR
#define WED_PAO_DBG_INFO_MASK                 0xFFFFFFFF                // INFO[31..0]
#define WED_PAO_DBG_INFO_SHFT                 0

#ifdef __cplusplus
}
#endif

#endif // __PAO0_GATHER_ONLY_REGS_H__
