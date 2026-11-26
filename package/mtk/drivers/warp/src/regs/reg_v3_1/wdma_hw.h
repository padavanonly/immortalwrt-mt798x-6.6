
#ifndef __WDMA0_V3_REGS_H__
#define __WDMA0_V3_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WDMA0_V3 CR Definitions
//
//****************************************************************************

#define WDMA_TX_BASE_PTR_0_ADDR                        0x00000000
#define WDMA_TX_MAX_CNT_0_ADDR                         0x00000004
#define WDMA_TX_CTX_IDX_0_ADDR                         0x00000008
#define WDMA_TX_DTX_IDX_0_ADDR                         0x0000000c
#define WDMA_TX_BASE_PTR_1_ADDR                        0x00000010
#define WDMA_TX_MAX_CNT_1_ADDR                         0x00000014
#define WDMA_TX_CTX_IDX_1_ADDR                         0x00000018
#define WDMA_TX_DTX_IDX_1_ADDR                         0x0000001c
#define WDMA_TX_BASE_PTR_2_ADDR                        0x00000020
#define WDMA_TX_MAX_CNT_2_ADDR                         0x00000024
#define WDMA_TX_CTX_IDX_2_ADDR                         0x00000028
#define WDMA_TX_DTX_IDX_2_ADDR                         0x0000002c
#define WDMA_TX_BASE_PTR_3_ADDR                        0x00000030
#define WDMA_TX_MAX_CNT_3_ADDR                         0x00000034
#define WDMA_TX_CTX_IDX_3_ADDR                         0x00000038
#define WDMA_TX_DTX_IDX_3_ADDR                         0x0000003c
#define WDMA_RX_BASE_PTR_0_ADDR                        0x00000100
#define WDMA_RX_MAX_CNT_0_ADDR                         0x00000104
#define WDMA_RX_CRX_IDX_0_ADDR                         0x00000108
#define WDMA_RX_DRX_IDX_0_ADDR                         0x0000010c
#define WDMA_RX_BASE_PTR_1_ADDR                        0x00000110
#define WDMA_RX_MAX_CNT_1_ADDR                         0x00000114
#define WDMA_RX_CRX_IDX_1_ADDR                         0x00000118
#define WDMA_RX_DRX_IDX_1_ADDR                         0x0000011c
#define WDMA_INFO_ADDR                                 0x00000200
#define WDMA_GLO_CFG0_ADDR                             0x00000204
#define WDMA_RST_IDX_ADDR                              0x00000208
#define WDMA_FREEQ_THRES_ADDR                          0x00000210
#define WDMA_INT_STATUS_ADDR                           0x00000220
#define WDMA_INT_MASK_ADDR                             0x00000228
#define WDMA_ERR_INT_STATUS_ADDR                       0x00000230
#define WDMA_GLO_CFG1_ADDR                             0x00000234
#define WDMA_TX_XDMA_FIFO_CFG0_ADDR                    0x00000238
#define WDMA_RX_XDMA_FIFO_CFG0_ADDR                    0x0000023c
#define WDMA_INT_STS_GRP0_ADDR                         0x00000240
#define WDMA_INT_STS_GRP1_ADDR                         0x00000244
#define WDMA_INT_STS_GRP2_ADDR                         0x00000248
#define WDMA_INT_STS_GRP3_ADDR                         0x0000024c
#define WDMA_INT_GRP1_ADDR                             0x00000250
#define WDMA_INT_GRP2_ADDR                             0x00000254
#define WDMA_INT_GRP3_ADDR                             0x00000258
#define WDMA_BUS_CFG_ADDR                              0x00000260
#define WDMA_ULTRA_CFG_ADDR                            0x00000264
#define WDMA_XFER_CNT_CFG1_ADDR                        0x00000268
#define WDMA_SDL_CFG_ADDR                              0x00000270
#define WDMA_IDLE_MASK_ADDR                            0x00000274
#define WDMA_SCH_Q01_CFG_ADDR                          0x00000280
#define WDMA_SCH_Q23_CFG_ADDR                          0x00000284
#define WDMA_INT_STATUS_0_ADDR                         0x00000290
#define WDMA_INT_MASK_0_ADDR                           0x00000294
#define WDMA_INT_STATUS_1_ADDR                         0x00000298
#define WDMA_INT_MASK_1_ADDR                           0x0000029c
#define WDMA_INT_STATUS_2_ADDR                         0x000002a0
#define WDMA_INT_MASK_2_ADDR                           0x000002a4
#define WDMA_INT_STATUS_3_ADDR                         0x000002a8
#define WDMA_INT_MASK_3_ADDR                           0x000002ac
#define WDMA_TX_DELAY_INT_CFG_0_ADDR                   0x000002b0
#define WDMA_TX_DELAY_INT_CFG_1_ADDR                   0x000002b4
#define WDMA_RX_DELAY_INT_CFG_0_ADDR                   0x000002c0
#define WDMA_RX_FC_CFG_0_ADDR                          0x000002c8
#define WDMA_RX_FC_CFG_1_ADDR                          0x000002cc
#define WDMA_PREF_TX_CFG_ADDR                          0x000002d0
#define WDMA_PREF_TX_FIFO_CFG0_ADDR                    0x000002d4
#define WDMA_PREF_TX_FIFO_CFG1_ADDR                    0x000002d8
#define WDMA_PREF_RX_CFG_ADDR                          0x000002dc
#define WDMA_PREF_RX_FIFO_CFG0_ADDR                    0x000002e0
#define WDMA_PREF_SIDX_CFG_ADDR                        0x000002e4
#define WDMA_PREF_SIDX_MON_ADDR                        0x000002e8
#define WDMA_PREF_SIDX_OW_ADDR                         0x000002ec
#define WDMA_XFER_CNT_CFG_ADDR                         0x000002f0
#define WDMA_XFER_CNT_MON0_ADDR                        0x000002f4
#define WDMA_XFER_CNT_MON1_ADDR                        0x000002f8
#define WDMA_RX_ULTRA_CFG_ADDR                         0x000002fc
#define WDMA_WRBK_TX_CFG_ADDR                          0x00000300
#define WDMA_WRBK_TX_FIFO_CFG0_ADDR                    0x00000304
#define WDMA_WRBK_TX_FIFO_CFG1_ADDR                    0x00000308
#define WDMA_WRBK_TX_FIFO_CFG2_ADDR                    0x0000030c
#define WDMA_WRBK_TX_FIFO_CFG3_ADDR                    0x00000310
#define WDMA_WRBK_RX_CFG_ADDR                          0x00000344
#define WDMA_WRBK_RX_FIFO_CFG0_ADDR                    0x00000348
#define WDMA_WRBK_RX_FIFO_CFG1_ADDR                    0x0000034c
#define WDMA_WRBK_SIDX_CFG_ADDR                        0x00000388
#define WDMA_WRBK_SIDX_MON_ADDR                        0x00000390
#define WDMA_WRBK_SIDX_OW_ADDR                         0x00000394
#define WDMA_DBG_CFG_ADDR                              0x00000398
#define WDMA_TX_DBG_MON_0_ADDR                         0x0000039c
#define WDMA_TX_DBG_MON_1_ADDR                         0x000003a0
#define WDMA_TX_DBG_MON_2_ADDR                         0x000003a4
#define WDMA_TX_DBG_MON_3_ADDR                         0x000003a8
#define WDMA_TX_DBG_MON_4_ADDR                         0x000003ac
#define WDMA_TX_DBG_MON_5_ADDR                         0x000003b0
#define WDMA_TX_DBG_MON_6_ADDR                         0x000003b4
#define WDMA_TX_DBG_MON_7_ADDR                         0x000003b8
#define WDMA_TX_DBG_MON_8_ADDR                         0x000003bc
#define WDMA_RX_DBG_MON_0_ADDR                         0x000003c0
#define WDMA_RX_DBG_MON_1_ADDR                         0x000003c4
#define WDMA_RX_DBG_MON_2_ADDR                         0x000003c8
#define WDMA_RX_DBG_MON_3_ADDR                         0x000003cc
#define WDMA_RX_DBG_MON_4_ADDR                         0x000003d0
#define WDMA_RX_DBG_MON_5_ADDR                         0x000003d4
#define WDMA_RX_DBG_MON_6_ADDR                         0x000003d8
#define WDMA_RX_DBG_MON_7_ADDR                         0x000003dc
#define WDMA_RX_DBG_MON_8_ADDR                         0x000003e0
#define WDMA_RX_MULTI_ID_CFG_ADDR                      0x000003e4
#define WDMA_RX_MULTI_ID_BLK_STS_ADDR                  0x000003e8
#define WDMA_RX_MULTI_ID_DATA_STS0_ADDR                0x000003ec
#define WDMA_RX_MULTI_ID_DATA_STS1_ADDR                0x000003f0
#define WDMA_RX_MULTI_ID_DATA_STS2_ADDR                0x000003f4
#define WDMA_RX_MULTI_ID_DATA_STS3_ADDR                0x000003f8
#define WDMA_RX_MULTI_ID_DBG_MON_ADDR                  0x000003fc

/* =====================================================================================

  ---DMA_TX_BASE_PTR_0 (0x15104800 + 0x0000)---

    TX_BASE_PTR[31..0]           - (RW) Point to the base address of TX Ring #0 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_TX_BASE_PTR_0_TX_BASE_PTR_ADDR            WDMA_TX_BASE_PTR_0_ADDR
#define WDMA_TX_BASE_PTR_0_TX_BASE_PTR_MASK            0xFFFFFFFF                // TX_BASE_PTR[31..0]
#define WDMA_TX_BASE_PTR_0_TX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_TX_MAX_CNT_0 (0x15104800 + 0x0004)---

    TX_MAX_CNT[15..0]            - (RW) The maximum number of TXD count in TX Ring #0
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_MAX_CNT_0_TX_MAX_CNT_ADDR              WDMA_TX_MAX_CNT_0_ADDR
#define WDMA_TX_MAX_CNT_0_TX_MAX_CNT_MASK              0x0000FFFF                // TX_MAX_CNT[15..0]
#define WDMA_TX_MAX_CNT_0_TX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_TX_CTX_IDX_0 (0x15104800 + 0x0008)---

    TX_CTX_IDX[15..0]            - (RW) Point to the next TXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_CTX_IDX_0_TX_CTX_IDX_ADDR              WDMA_TX_CTX_IDX_0_ADDR
#define WDMA_TX_CTX_IDX_0_TX_CTX_IDX_MASK              0x0000FFFF                // TX_CTX_IDX[15..0]
#define WDMA_TX_CTX_IDX_0_TX_CTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_DTX_IDX_0 (0x15104800 + 0x000c)---

    TX_DTX_IDX[15..0]            - (RO) Point to the next TXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_DTX_IDX_0_TX_DTX_IDX_ADDR              WDMA_TX_DTX_IDX_0_ADDR
#define WDMA_TX_DTX_IDX_0_TX_DTX_IDX_MASK              0x0000FFFF                // TX_DTX_IDX[15..0]
#define WDMA_TX_DTX_IDX_0_TX_DTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_BASE_PTR_1 (0x15104800 + 0x0010)---

    TX_BASE_PTR[31..0]           - (RW) Point to the base address of TX Ring #1 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_TX_BASE_PTR_1_TX_BASE_PTR_ADDR            WDMA_TX_BASE_PTR_1_ADDR
#define WDMA_TX_BASE_PTR_1_TX_BASE_PTR_MASK            0xFFFFFFFF                // TX_BASE_PTR[31..0]
#define WDMA_TX_BASE_PTR_1_TX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_TX_MAX_CNT_1 (0x15104800 + 0x0014)---

    TX_MAX_CNT[15..0]            - (RW) The maximum number of TXD count in TX Ring #1
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_MAX_CNT_1_TX_MAX_CNT_ADDR              WDMA_TX_MAX_CNT_1_ADDR
#define WDMA_TX_MAX_CNT_1_TX_MAX_CNT_MASK              0x0000FFFF                // TX_MAX_CNT[15..0]
#define WDMA_TX_MAX_CNT_1_TX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_TX_CTX_IDX_1 (0x15104800 + 0x0018)---

    TX_CTX_IDX[15..0]            - (RW) Point to the next TXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_CTX_IDX_1_TX_CTX_IDX_ADDR              WDMA_TX_CTX_IDX_1_ADDR
#define WDMA_TX_CTX_IDX_1_TX_CTX_IDX_MASK              0x0000FFFF                // TX_CTX_IDX[15..0]
#define WDMA_TX_CTX_IDX_1_TX_CTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_DTX_IDX_1 (0x15104800 + 0x001c)---

    TX_DTX_IDX[15..0]            - (RO) Point to the next TXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_DTX_IDX_1_TX_DTX_IDX_ADDR              WDMA_TX_DTX_IDX_1_ADDR
#define WDMA_TX_DTX_IDX_1_TX_DTX_IDX_MASK              0x0000FFFF                // TX_DTX_IDX[15..0]
#define WDMA_TX_DTX_IDX_1_TX_DTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_BASE_PTR_2 (0x15104800 + 0x0020)---

    TX_BASE_PTR[31..0]           - (RW) Point to the base address of TX Ring #2 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_TX_BASE_PTR_2_TX_BASE_PTR_ADDR            WDMA_TX_BASE_PTR_2_ADDR
#define WDMA_TX_BASE_PTR_2_TX_BASE_PTR_MASK            0xFFFFFFFF                // TX_BASE_PTR[31..0]
#define WDMA_TX_BASE_PTR_2_TX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_TX_MAX_CNT_2 (0x15104800 + 0x0024)---

    TX_MAX_CNT[15..0]            - (RW) The maximum number of TXD count in TX Ring #2
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_MAX_CNT_2_TX_MAX_CNT_ADDR              WDMA_TX_MAX_CNT_2_ADDR
#define WDMA_TX_MAX_CNT_2_TX_MAX_CNT_MASK              0x0000FFFF                // TX_MAX_CNT[15..0]
#define WDMA_TX_MAX_CNT_2_TX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_TX_CTX_IDX_2 (0x15104800 + 0x0028)---

    TX_CTX_IDX[15..0]            - (RW) Point to the next TXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_CTX_IDX_2_TX_CTX_IDX_ADDR              WDMA_TX_CTX_IDX_2_ADDR
#define WDMA_TX_CTX_IDX_2_TX_CTX_IDX_MASK              0x0000FFFF                // TX_CTX_IDX[15..0]
#define WDMA_TX_CTX_IDX_2_TX_CTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_DTX_IDX_2 (0x15104800 + 0x002c)---

    TX_DTX_IDX[15..0]            - (RO) Point to the next TXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_DTX_IDX_2_TX_DTX_IDX_ADDR              WDMA_TX_DTX_IDX_2_ADDR
#define WDMA_TX_DTX_IDX_2_TX_DTX_IDX_MASK              0x0000FFFF                // TX_DTX_IDX[15..0]
#define WDMA_TX_DTX_IDX_2_TX_DTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_BASE_PTR_3 (0x15104800 + 0x0030)---

    TX_BASE_PTR[31..0]           - (RW) Point to the base address of TX Ring #3 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_TX_BASE_PTR_3_TX_BASE_PTR_ADDR            WDMA_TX_BASE_PTR_3_ADDR
#define WDMA_TX_BASE_PTR_3_TX_BASE_PTR_MASK            0xFFFFFFFF                // TX_BASE_PTR[31..0]
#define WDMA_TX_BASE_PTR_3_TX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_TX_MAX_CNT_3 (0x15104800 + 0x0034)---

    TX_MAX_CNT[15..0]            - (RW) The maximum number of TXD count in TX Ring #3
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_MAX_CNT_3_TX_MAX_CNT_ADDR              WDMA_TX_MAX_CNT_3_ADDR
#define WDMA_TX_MAX_CNT_3_TX_MAX_CNT_MASK              0x0000FFFF                // TX_MAX_CNT[15..0]
#define WDMA_TX_MAX_CNT_3_TX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_TX_CTX_IDX_3 (0x15104800 + 0x0038)---

    TX_CTX_IDX[15..0]            - (RW) Point to the next TXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_CTX_IDX_3_TX_CTX_IDX_ADDR              WDMA_TX_CTX_IDX_3_ADDR
#define WDMA_TX_CTX_IDX_3_TX_CTX_IDX_MASK              0x0000FFFF                // TX_CTX_IDX[15..0]
#define WDMA_TX_CTX_IDX_3_TX_CTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_TX_DTX_IDX_3 (0x15104800 + 0x003c)---

    TX_DTX_IDX[15..0]            - (RO) Point to the next TXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_DTX_IDX_3_TX_DTX_IDX_ADDR              WDMA_TX_DTX_IDX_3_ADDR
#define WDMA_TX_DTX_IDX_3_TX_DTX_IDX_MASK              0x0000FFFF                // TX_DTX_IDX[15..0]
#define WDMA_TX_DTX_IDX_3_TX_DTX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_RX_BASE_PTR_0 (0x15104800 + 0x0100)---

    RX_BASE_PTR[31..0]           - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_RX_BASE_PTR_0_RX_BASE_PTR_ADDR            WDMA_RX_BASE_PTR_0_ADDR
#define WDMA_RX_BASE_PTR_0_RX_BASE_PTR_MASK            0xFFFFFFFF                // RX_BASE_PTR[31..0]
#define WDMA_RX_BASE_PTR_0_RX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_RX_MAX_CNT_0 (0x15104800 + 0x0104)---

    RX_MAX_CNT[15..0]            - (RW) The maximum number of RXD count in RX Ring #0
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_MAX_CNT_0_RX_MAX_CNT_ADDR              WDMA_RX_MAX_CNT_0_ADDR
#define WDMA_RX_MAX_CNT_0_RX_MAX_CNT_MASK              0x0000FFFF                // RX_MAX_CNT[15..0]
#define WDMA_RX_MAX_CNT_0_RX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_RX_CRX_IDX_0 (0x15104800 + 0x0108)---

    RX_CRX_IDX[15..0]            - (RW) Point to the next RXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_CRX_IDX_0_RX_CRX_IDX_ADDR              WDMA_RX_CRX_IDX_0_ADDR
#define WDMA_RX_CRX_IDX_0_RX_CRX_IDX_MASK              0x0000FFFF                // RX_CRX_IDX[15..0]
#define WDMA_RX_CRX_IDX_0_RX_CRX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_RX_DRX_IDX_0 (0x15104800 + 0x010c)---

    RX_DRX_IDX[15..0]            - (RO) Point to the next RXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_DRX_IDX_0_RX_DRX_IDX_ADDR              WDMA_RX_DRX_IDX_0_ADDR
#define WDMA_RX_DRX_IDX_0_RX_DRX_IDX_MASK              0x0000FFFF                // RX_DRX_IDX[15..0]
#define WDMA_RX_DRX_IDX_0_RX_DRX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_RX_BASE_PTR_1 (0x15104800 + 0x0110)---

    RX_BASE_PTR[31..0]           - (RW) Point to the base address of RX Ring #1 (4-DW aligned address)

 =====================================================================================*/
#define WDMA_RX_BASE_PTR_1_RX_BASE_PTR_ADDR            WDMA_RX_BASE_PTR_1_ADDR
#define WDMA_RX_BASE_PTR_1_RX_BASE_PTR_MASK            0xFFFFFFFF                // RX_BASE_PTR[31..0]
#define WDMA_RX_BASE_PTR_1_RX_BASE_PTR_SHFT            0

/* =====================================================================================

  ---DMA_RX_MAX_CNT_1 (0x15104800 + 0x0114)---

    RX_MAX_CNT[15..0]            - (RW) The maximum number of RXD count in RX Ring #1
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_MAX_CNT_1_RX_MAX_CNT_ADDR              WDMA_RX_MAX_CNT_1_ADDR
#define WDMA_RX_MAX_CNT_1_RX_MAX_CNT_MASK              0x0000FFFF                // RX_MAX_CNT[15..0]
#define WDMA_RX_MAX_CNT_1_RX_MAX_CNT_SHFT              0

/* =====================================================================================

  ---DMA_RX_CRX_IDX_1 (0x15104800 + 0x0118)---

    RX_CRX_IDX[15..0]            - (RW) Point to the next RXD CPU wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_CRX_IDX_1_RX_CRX_IDX_ADDR              WDMA_RX_CRX_IDX_1_ADDR
#define WDMA_RX_CRX_IDX_1_RX_CRX_IDX_MASK              0x0000FFFF                // RX_CRX_IDX[15..0]
#define WDMA_RX_CRX_IDX_1_RX_CRX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_RX_DRX_IDX_1 (0x15104800 + 0x011c)---

    RX_DRX_IDX[15..0]            - (RO) Point to the next RXD DMA wants to use
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_DRX_IDX_1_RX_DRX_IDX_ADDR              WDMA_RX_DRX_IDX_1_ADDR
#define WDMA_RX_DRX_IDX_1_RX_DRX_IDX_MASK              0x0000FFFF                // RX_DRX_IDX[15..0]
#define WDMA_RX_DRX_IDX_1_RX_DRX_IDX_SHFT              0

/* =====================================================================================

  ---DMA_INFO (0x15104800 + 0x0200)---

    TX_RING_NUM[7..0]            - (RO) TX ring number
    RX_RING_NUM[15..8]           - (RO) RX ring number
    BASE_PTR_WIDTH[23..16]       - (RO) Base pointer width, x
                                     Base_addr[31:32-x] is shared with all ring base address. Only ring #0 base address[31:32-x] field Is writable.
                                     [note]: "0" means no bit of base_address is shared.
    INDEX_WIDTH[27..24]          - (RO) Ring index width
                                     0 means 16bits, 1~15 means 1~15bits
    DMA_REVISION[31..28]         - (RO) WDMA0 revision

 =====================================================================================*/
#define WDMA_INFO_DMA_REVISION_ADDR                    WDMA_INFO_ADDR
#define WDMA_INFO_DMA_REVISION_MASK                    0xF0000000                // DMA_REVISION[31..28]
#define WDMA_INFO_DMA_REVISION_SHFT                    28
#define WDMA_INFO_INDEX_WIDTH_ADDR                     WDMA_INFO_ADDR
#define WDMA_INFO_INDEX_WIDTH_MASK                     0x0F000000                // INDEX_WIDTH[27..24]
#define WDMA_INFO_INDEX_WIDTH_SHFT                     24
#define WDMA_INFO_BASE_PTR_WIDTH_ADDR                  WDMA_INFO_ADDR
#define WDMA_INFO_BASE_PTR_WIDTH_MASK                  0x00FF0000                // BASE_PTR_WIDTH[23..16]
#define WDMA_INFO_BASE_PTR_WIDTH_SHFT                  16
#define WDMA_INFO_RX_RING_NUM_ADDR                     WDMA_INFO_ADDR
#define WDMA_INFO_RX_RING_NUM_MASK                     0x0000FF00                // RX_RING_NUM[15..8]
#define WDMA_INFO_RX_RING_NUM_SHFT                     8
#define WDMA_INFO_TX_RING_NUM_ADDR                     WDMA_INFO_ADDR
#define WDMA_INFO_TX_RING_NUM_MASK                     0x000000FF                // TX_RING_NUM[7..0]
#define WDMA_INFO_TX_RING_NUM_SHFT                     0

/* =====================================================================================

  ---DMA_GLO_CFG0 (0x15104800 + 0x0204)---

    TX_DMA_EN[0]                 - (RW)  xxx
    TX_DMA_BUSY[1]               - (RO) Indicate that xDMA is dealing with packet from memory to frame engine
    RX_DMA_EN[2]                 - (RW)  xxx
    RX_DMA_BUSY[3]               - (RO) Indicate that xDMA is dealing with packet from frame engine to memory
    TX_BURST_4KB_BND_EN[4]       - (RW) AXI read cross 4kb
    RX_BURST_4KB_BND_EN[5]       - (RW) AXI write cross 4kb
    TX_WB_DDONE[6]               - (RW)  xxx
    PAYLOAD_BYTE_SWAP[7]         - (RW) Big endian
    RESERVED8[8]                 - (RO) Reserved bits
    TX_CHK_DDONE[9]              - (RW) Defeature
    TX_SCH_RST[10]               - (A0) Reset scheduler
    DMA_BT_SIZE[13..11]          - (RW) Burst size of WDMA
    OTSD_THRES[17..14]           - (RW) AXI outstanding capability of WDMA
                                     0 : No support outstanding
                                     1 : Support 1 AXI outstanding
                                     2: Support 2 AXI outstanding
                                     ..
                                     15: Support 15 AXI outstanding
    CDM_FCNT_THRES[21..18]       - (RW) Guard band protection from memory to network interface
                                     0: No guard band
                                     2: 16B
                                     ..
                                     15: 240B
    RESERVED22[23..22]           - (RO) Reserved bits
    LB_MODE[24]                  - (RW) Power correlation
    PAYLOAD_BYTE_SWAP_SEL[25]    - (RW) Defeature
    DMAD_BYTE_SWAP_SEL[26]       - (RW) Defeature
    PKT_WCOMP[27]                - (RW) RX_DMA checks WDONE or WCOMPLETE when writing packet into DRAM
    DEC_WCOMP[28]                - (RW) DMA checks WDONE or WCOMPLETE when updating descriptor into DRAM
    DMAD_BYTE_SWAP[29]           - (RW) Swaps byte
    RESERVED30[30]               - (RO) Reserved bits
    RX_2B_OFFSET[31]             - (RW) Defeature

 =====================================================================================*/
#define WDMA_GLO_CFG0_RX_2B_OFFSET_ADDR                WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_RX_2B_OFFSET_MASK                0x80000000                // RX_2B_OFFSET[31]
#define WDMA_GLO_CFG0_RX_2B_OFFSET_SHFT                31
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_ADDR              WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_MASK              0x20000000                // DMAD_BYTE_SWAP[29]
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_SHFT              29
#define WDMA_GLO_CFG0_DEC_WCOMP_ADDR                   WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_DEC_WCOMP_MASK                   0x10000000                // DEC_WCOMP[28]
#define WDMA_GLO_CFG0_DEC_WCOMP_SHFT                   28
#define WDMA_GLO_CFG0_PKT_WCOMP_ADDR                   WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_PKT_WCOMP_MASK                   0x08000000                // PKT_WCOMP[27]
#define WDMA_GLO_CFG0_PKT_WCOMP_SHFT                   27
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_SEL_ADDR          WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_SEL_MASK          0x04000000                // DMAD_BYTE_SWAP_SEL[26]
#define WDMA_GLO_CFG0_DMAD_BYTE_SWAP_SEL_SHFT          26
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_SEL_ADDR       WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_SEL_MASK       0x02000000                // PAYLOAD_BYTE_SWAP_SEL[25]
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_SEL_SHFT       25
#define WDMA_GLO_CFG0_LB_MODE_ADDR                     WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_LB_MODE_MASK                     0x01000000                // LB_MODE[24]
#define WDMA_GLO_CFG0_LB_MODE_SHFT                     24
#define WDMA_GLO_CFG0_CDM_FCNT_THRES_ADDR              WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_CDM_FCNT_THRES_MASK              0x003C0000                // CDM_FCNT_THRES[21..18]
#define WDMA_GLO_CFG0_CDM_FCNT_THRES_SHFT              18
#define WDMA_GLO_CFG0_OTSD_THRES_ADDR                  WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_OTSD_THRES_MASK                  0x0003C000                // OTSD_THRES[17..14]
#define WDMA_GLO_CFG0_OTSD_THRES_SHFT                  14
#define WDMA_GLO_CFG0_DMA_BT_SIZE_ADDR                 WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_DMA_BT_SIZE_MASK                 0x00003800                // DMA_BT_SIZE[13..11]
#define WDMA_GLO_CFG0_DMA_BT_SIZE_SHFT                 11
#define WDMA_GLO_CFG0_TX_SCH_RST_ADDR                  WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_SCH_RST_MASK                  0x00000400                // TX_SCH_RST[10]
#define WDMA_GLO_CFG0_TX_SCH_RST_SHFT                  10
#define WDMA_GLO_CFG0_TX_CHK_DDONE_ADDR                WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_CHK_DDONE_MASK                0x00000200                // TX_CHK_DDONE[9]
#define WDMA_GLO_CFG0_TX_CHK_DDONE_SHFT                9
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_ADDR           WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_MASK           0x00000080                // PAYLOAD_BYTE_SWAP[7]
#define WDMA_GLO_CFG0_PAYLOAD_BYTE_SWAP_SHFT           7
#define WDMA_GLO_CFG0_TX_WB_DDONE_ADDR                 WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_WB_DDONE_MASK                 0x00000040                // TX_WB_DDONE[6]
#define WDMA_GLO_CFG0_TX_WB_DDONE_SHFT                 6
#define WDMA_GLO_CFG0_RX_BURST_4KB_BND_EN_ADDR         WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_RX_BURST_4KB_BND_EN_MASK         0x00000020                // RX_BURST_4KB_BND_EN[5]
#define WDMA_GLO_CFG0_RX_BURST_4KB_BND_EN_SHFT         5
#define WDMA_GLO_CFG0_TX_BURST_4KB_BND_EN_ADDR         WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_BURST_4KB_BND_EN_MASK         0x00000010                // TX_BURST_4KB_BND_EN[4]
#define WDMA_GLO_CFG0_TX_BURST_4KB_BND_EN_SHFT         4
#define WDMA_GLO_CFG0_RX_DMA_BUSY_ADDR                 WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_RX_DMA_BUSY_MASK                 0x00000008                // RX_DMA_BUSY[3]
#define WDMA_GLO_CFG0_RX_DMA_BUSY_SHFT                 3
#define WDMA_GLO_CFG0_RX_DMA_EN_ADDR                   WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_RX_DMA_EN_MASK                   0x00000004                // RX_DMA_EN[2]
#define WDMA_GLO_CFG0_RX_DMA_EN_SHFT                   2
#define WDMA_GLO_CFG0_TX_DMA_BUSY_ADDR                 WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_DMA_BUSY_MASK                 0x00000002                // TX_DMA_BUSY[1]
#define WDMA_GLO_CFG0_TX_DMA_BUSY_SHFT                 1
#define WDMA_GLO_CFG0_TX_DMA_EN_ADDR                   WDMA_GLO_CFG0_ADDR
#define WDMA_GLO_CFG0_TX_DMA_EN_MASK                   0x00000001                // TX_DMA_EN[0]
#define WDMA_GLO_CFG0_TX_DMA_EN_SHFT                   0

/* =====================================================================================

  ---DMA_RST_IDX (0x15104800 + 0x0208)---

    RST_DTX_IDX0[0]              - (A0) Write 1 to reset TX_DMA TX_IDX0 to 0
    RST_DTX_IDX1[1]              - (A0) Write 1 to reset TX_DMA TX_IDX1 to 0
    RST_DTX_IDX2[2]              - (A0) Write 1 to reset TX_DMA TX_IDX2 to 0
    RST_DTX_IDX3[3]              - (A0) Write 1 to reset TX_DMA TX_IDX3 to 0
    RESERVED4[15..4]             - (RO) Reserved bits
    RST_DRX_IDX0[16]             - (A0) Write 1 to reset RX_DMA RX_IDX0 to 0
    RST_DRX_IDX1[17]             - (A0) Write 1 to reset RX_DMA RX_IDX1 to 0
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RST_IDX_RST_DRX_IDX1_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DRX_IDX1_MASK                 0x00020000                // RST_DRX_IDX1[17]
#define WDMA_RST_IDX_RST_DRX_IDX1_SHFT                 17
#define WDMA_RST_IDX_RST_DRX_IDX0_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DRX_IDX0_MASK                 0x00010000                // RST_DRX_IDX0[16]
#define WDMA_RST_IDX_RST_DRX_IDX0_SHFT                 16
#define WDMA_RST_IDX_RST_DTX_IDX3_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DTX_IDX3_MASK                 0x00000008                // RST_DTX_IDX3[3]
#define WDMA_RST_IDX_RST_DTX_IDX3_SHFT                 3
#define WDMA_RST_IDX_RST_DTX_IDX2_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DTX_IDX2_MASK                 0x00000004                // RST_DTX_IDX2[2]
#define WDMA_RST_IDX_RST_DTX_IDX2_SHFT                 2
#define WDMA_RST_IDX_RST_DTX_IDX1_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DTX_IDX1_MASK                 0x00000002                // RST_DTX_IDX1[1]
#define WDMA_RST_IDX_RST_DTX_IDX1_SHFT                 1
#define WDMA_RST_IDX_RST_DTX_IDX0_ADDR                 WDMA_RST_IDX_ADDR
#define WDMA_RST_IDX_RST_DTX_IDX0_MASK                 0x00000001                // RST_DTX_IDX0[0]
#define WDMA_RST_IDX_RST_DTX_IDX0_SHFT                 0

/* =====================================================================================

  ---DMA_FREEQ_THRES (0x15104800 + 0x0210)---

    FREEQ_THRES[3..0]            - (RW) RX free queue threshold
                                     WDMA will stop DMA interface when left RX descriptors reach this threshold
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_FREEQ_THRES_FREEQ_THRES_ADDR              WDMA_FREEQ_THRES_ADDR
#define WDMA_FREEQ_THRES_FREEQ_THRES_MASK              0x0000000F                // FREEQ_THRES[3..0]
#define WDMA_FREEQ_THRES_FREEQ_THRES_SHFT              0

/* =====================================================================================

  ---DMA_INT_STATUS (0x15104800 + 0x0220)---

    TX_DONE_INT0[0]              - (W1C) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (W1C) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (W1C) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (W1C) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (W1C) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (W1C) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (W1C) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (W1C) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (W1C) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (W1C) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (W1C) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (W1C) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (W1C) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (W1C) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (W1C) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (W1C) RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WDMA_INT_STATUS_RX_COHERENT_ADDR               WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_COHERENT_MASK               0x80000000                // RX_COHERENT[31]
#define WDMA_INT_STATUS_RX_COHERENT_SHFT               31
#define WDMA_INT_STATUS_RX_DLY_INT_ADDR                WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_DLY_INT_MASK                0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_STATUS_RX_DLY_INT_SHFT                30
#define WDMA_INT_STATUS_TX_COHERENT_ADDR               WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_COHERENT_MASK               0x20000000                // TX_COHERENT[29]
#define WDMA_INT_STATUS_TX_COHERENT_SHFT               29
#define WDMA_INT_STATUS_TX_DLY_INT_ADDR                WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DLY_INT_MASK                0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_STATUS_TX_DLY_INT_SHFT                28
#define WDMA_INT_STATUS_RX_DONE_DLY_INT1_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_DONE_DLY_INT1_MASK          0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_STATUS_RX_DONE_DLY_INT1_SHFT          21
#define WDMA_INT_STATUS_RX_DONE_DLY_INT0_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_DONE_DLY_INT0_MASK          0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_STATUS_RX_DONE_DLY_INT0_SHFT          20
#define WDMA_INT_STATUS_RX_DONE_INT1_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_DONE_INT1_MASK              0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_STATUS_RX_DONE_INT1_SHFT              17
#define WDMA_INT_STATUS_RX_DONE_INT0_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_RX_DONE_INT0_MASK              0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_STATUS_RX_DONE_INT0_SHFT              16
#define WDMA_INT_STATUS_TX_DONE_DLY_INT3_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_DLY_INT3_MASK          0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_STATUS_TX_DONE_DLY_INT3_SHFT          11
#define WDMA_INT_STATUS_TX_DONE_DLY_INT2_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_DLY_INT2_MASK          0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_STATUS_TX_DONE_DLY_INT2_SHFT          10
#define WDMA_INT_STATUS_TX_DONE_DLY_INT1_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_DLY_INT1_MASK          0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_STATUS_TX_DONE_DLY_INT1_SHFT          9
#define WDMA_INT_STATUS_TX_DONE_DLY_INT0_ADDR          WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_DLY_INT0_MASK          0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_STATUS_TX_DONE_DLY_INT0_SHFT          8
#define WDMA_INT_STATUS_TX_DONE_INT3_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_INT3_MASK              0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_STATUS_TX_DONE_INT3_SHFT              3
#define WDMA_INT_STATUS_TX_DONE_INT2_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_INT2_MASK              0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_STATUS_TX_DONE_INT2_SHFT              2
#define WDMA_INT_STATUS_TX_DONE_INT1_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_INT1_MASK              0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_STATUS_TX_DONE_INT1_SHFT              1
#define WDMA_INT_STATUS_TX_DONE_INT0_ADDR              WDMA_INT_STATUS_ADDR
#define WDMA_INT_STATUS_TX_DONE_INT0_MASK              0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_STATUS_TX_DONE_INT0_SHFT              0

/* =====================================================================================

  ---DMA_INT_MASK (0x15104800 + 0x0228)---

    TX_DONE_INT0[0]              - (RW) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RW) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (RW) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (RW) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (RW) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (RW) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (RW) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (RW) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RW) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (RW) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (RW) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (RW) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RW) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (RW) Interrupt enable for TX_DMA data coherent event
    RX_DLY_INT[30]               - (RW) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (RW) Interrupt enable for RX_DMA data coherent event

 =====================================================================================*/
#define WDMA_INT_MASK_RX_COHERENT_ADDR                 WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_COHERENT_MASK                 0x80000000                // RX_COHERENT[31]
#define WDMA_INT_MASK_RX_COHERENT_SHFT                 31
#define WDMA_INT_MASK_RX_DLY_INT_ADDR                  WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_DLY_INT_MASK                  0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_MASK_RX_DLY_INT_SHFT                  30
#define WDMA_INT_MASK_TX_COHERENT_ADDR                 WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_COHERENT_MASK                 0x20000000                // TX_COHERENT[29]
#define WDMA_INT_MASK_TX_COHERENT_SHFT                 29
#define WDMA_INT_MASK_TX_DLY_INT_ADDR                  WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DLY_INT_MASK                  0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_MASK_TX_DLY_INT_SHFT                  28
#define WDMA_INT_MASK_RX_DONE_DLY_INT1_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_DONE_DLY_INT1_MASK            0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_MASK_RX_DONE_DLY_INT1_SHFT            21
#define WDMA_INT_MASK_RX_DONE_DLY_INT0_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_DONE_DLY_INT0_MASK            0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_MASK_RX_DONE_DLY_INT0_SHFT            20
#define WDMA_INT_MASK_RX_DONE_INT1_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_DONE_INT1_MASK                0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_MASK_RX_DONE_INT1_SHFT                17
#define WDMA_INT_MASK_RX_DONE_INT0_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_RX_DONE_INT0_MASK                0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_MASK_RX_DONE_INT0_SHFT                16
#define WDMA_INT_MASK_TX_DONE_DLY_INT3_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_DLY_INT3_MASK            0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_MASK_TX_DONE_DLY_INT3_SHFT            11
#define WDMA_INT_MASK_TX_DONE_DLY_INT2_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_DLY_INT2_MASK            0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_MASK_TX_DONE_DLY_INT2_SHFT            10
#define WDMA_INT_MASK_TX_DONE_DLY_INT1_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_DLY_INT1_MASK            0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_MASK_TX_DONE_DLY_INT1_SHFT            9
#define WDMA_INT_MASK_TX_DONE_DLY_INT0_ADDR            WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_DLY_INT0_MASK            0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_MASK_TX_DONE_DLY_INT0_SHFT            8
#define WDMA_INT_MASK_TX_DONE_INT3_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_INT3_MASK                0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_MASK_TX_DONE_INT3_SHFT                3
#define WDMA_INT_MASK_TX_DONE_INT2_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_INT2_MASK                0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_MASK_TX_DONE_INT2_SHFT                2
#define WDMA_INT_MASK_TX_DONE_INT1_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_INT1_MASK                0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_MASK_TX_DONE_INT1_SHFT                1
#define WDMA_INT_MASK_TX_DONE_INT0_ADDR                WDMA_INT_MASK_ADDR
#define WDMA_INT_MASK_TX_DONE_INT0_MASK                0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_MASK_TX_DONE_INT0_SHFT                0

/* =====================================================================================

  ---DMA_ERR_INT_STATUS (0x15104800 + 0x0230)---

    TX_AXI_BURST_WR_ERR_RESP[0]  - (W1C) AXI write error response(never happen)
    TX_AXI_BURST_RD_ERR_RESP[1]  - (W1C) AXI read error response
                                     When multi id disable, check TX_AXI_BURST_RD_ERR_RESP
                                     When multi id enable(default), check DMA_RX_MULTI_ID_CFG.AXI_ERR_RESP
    TX_AXI_PREF_WR_ERR_RESP[2]   - (W1C) AXI write error response(never happen)
    TX_AXI_PREF_RD_ERR_RESP[3]   - (W1C) AXI read error response
    TX_AXI_WRBK_WR_ERR_RESP[4]   - (W1C) AXI write error response
    TX_AXI_WRBK_RD_ERR_RESP[5]   - (W1C) AXI read error response(never happen)
    RESERVED6[7..6]              - (RO) Reserved bits
    RX_AXI_BURST_WR_ERR_RESP[8]  - (W1C) AXI write error response
    RX_AXI_BURST_RD_ERR_RESP[9]  - (W1C) AXI read error response(never happen)
    RX_AXI_PREF_WR_ERR_RESP[10]  - (W1C) AXI write error response(never happen)
    RX_AXI_PREF_RD_ERR_RESP[11]  - (W1C) AXI read error response
    RX_AXI_WRBK_WR_ERR_RESP[12]  - (W1C) AXI write error response
    RX_AXI_WRBK_RD_ERR_RESP[13]  - (W1C) AXI read error response(never happen)
    RX_CMD_EOF_ERR_RESP[14]      - (W1C) CDM eof asserts but CMD FIFO.eof not asserts
    RX_CDM_EOF_ERR_RESP[15]      - (W1C) CMD FIFO.eof asserts, but CDM EOF not asserts
    TX_PREF_RD_UD_ERR_RESP[16]   - (W1C) TX prefetch FIFO read underflow error
    TX_PREF_WR_OF_ERR_RESP[17]   - (W1C) TX prefetch FIFO write overflow error
    TX_WRBK_RD_UD_ERR_RESP[18]   - (W1C) TX writeback FIFO read underflow error
    TX_WRBK_WR_OF_ERR_RESP[19]   - (W1C) TX writeback FIFO write overflow error
    RX_PREF_RD_UD_ERR_RESP[20]   - (W1C) RX prefetch FIFO read underflow error
    RX_PREF_WR_OF_ERR_RESP[21]   - (W1C) RX prefetch FIFO write overflow error
    RX_WRBK_RD_UD_ERR_RESP[22]   - (W1C) RX writeback FIFO read underflow error
    RX_WRBK_WR_OF_ERR_RESP[23]   - (W1C) RX writeback FIFO write overflow error
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_ERR_INT_STATUS_RX_WRBK_WR_OF_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_WRBK_WR_OF_ERR_RESP_MASK 0x00800000                // RX_WRBK_WR_OF_ERR_RESP[23]
#define WDMA_ERR_INT_STATUS_RX_WRBK_WR_OF_ERR_RESP_SHFT 23
#define WDMA_ERR_INT_STATUS_RX_WRBK_RD_UD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_WRBK_RD_UD_ERR_RESP_MASK 0x00400000                // RX_WRBK_RD_UD_ERR_RESP[22]
#define WDMA_ERR_INT_STATUS_RX_WRBK_RD_UD_ERR_RESP_SHFT 22
#define WDMA_ERR_INT_STATUS_RX_PREF_WR_OF_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_PREF_WR_OF_ERR_RESP_MASK 0x00200000                // RX_PREF_WR_OF_ERR_RESP[21]
#define WDMA_ERR_INT_STATUS_RX_PREF_WR_OF_ERR_RESP_SHFT 21
#define WDMA_ERR_INT_STATUS_RX_PREF_RD_UD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_PREF_RD_UD_ERR_RESP_MASK 0x00100000                // RX_PREF_RD_UD_ERR_RESP[20]
#define WDMA_ERR_INT_STATUS_RX_PREF_RD_UD_ERR_RESP_SHFT 20
#define WDMA_ERR_INT_STATUS_TX_WRBK_WR_OF_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_WRBK_WR_OF_ERR_RESP_MASK 0x00080000                // TX_WRBK_WR_OF_ERR_RESP[19]
#define WDMA_ERR_INT_STATUS_TX_WRBK_WR_OF_ERR_RESP_SHFT 19
#define WDMA_ERR_INT_STATUS_TX_WRBK_RD_UD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_WRBK_RD_UD_ERR_RESP_MASK 0x00040000                // TX_WRBK_RD_UD_ERR_RESP[18]
#define WDMA_ERR_INT_STATUS_TX_WRBK_RD_UD_ERR_RESP_SHFT 18
#define WDMA_ERR_INT_STATUS_TX_PREF_WR_OF_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_PREF_WR_OF_ERR_RESP_MASK 0x00020000                // TX_PREF_WR_OF_ERR_RESP[17]
#define WDMA_ERR_INT_STATUS_TX_PREF_WR_OF_ERR_RESP_SHFT 17
#define WDMA_ERR_INT_STATUS_TX_PREF_RD_UD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_PREF_RD_UD_ERR_RESP_MASK 0x00010000                // TX_PREF_RD_UD_ERR_RESP[16]
#define WDMA_ERR_INT_STATUS_TX_PREF_RD_UD_ERR_RESP_SHFT 16
#define WDMA_ERR_INT_STATUS_RX_CDM_EOF_ERR_RESP_ADDR   WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_CDM_EOF_ERR_RESP_MASK   0x00008000                // RX_CDM_EOF_ERR_RESP[15]
#define WDMA_ERR_INT_STATUS_RX_CDM_EOF_ERR_RESP_SHFT   15
#define WDMA_ERR_INT_STATUS_RX_CMD_EOF_ERR_RESP_ADDR   WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_CMD_EOF_ERR_RESP_MASK   0x00004000                // RX_CMD_EOF_ERR_RESP[14]
#define WDMA_ERR_INT_STATUS_RX_CMD_EOF_ERR_RESP_SHFT   14
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_RD_ERR_RESP_MASK 0x00002000                // RX_AXI_WRBK_RD_ERR_RESP[13]
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_RD_ERR_RESP_SHFT 13
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_WR_ERR_RESP_MASK 0x00001000                // RX_AXI_WRBK_WR_ERR_RESP[12]
#define WDMA_ERR_INT_STATUS_RX_AXI_WRBK_WR_ERR_RESP_SHFT 12
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_RD_ERR_RESP_MASK 0x00000800                // RX_AXI_PREF_RD_ERR_RESP[11]
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_RD_ERR_RESP_SHFT 11
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_WR_ERR_RESP_MASK 0x00000400                // RX_AXI_PREF_WR_ERR_RESP[10]
#define WDMA_ERR_INT_STATUS_RX_AXI_PREF_WR_ERR_RESP_SHFT 10
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_RD_ERR_RESP_MASK 0x00000200                // RX_AXI_BURST_RD_ERR_RESP[9]
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_RD_ERR_RESP_SHFT 9
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_WR_ERR_RESP_MASK 0x00000100                // RX_AXI_BURST_WR_ERR_RESP[8]
#define WDMA_ERR_INT_STATUS_RX_AXI_BURST_WR_ERR_RESP_SHFT 8
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_RD_ERR_RESP_MASK 0x00000020                // TX_AXI_WRBK_RD_ERR_RESP[5]
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_RD_ERR_RESP_SHFT 5
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_WR_ERR_RESP_MASK 0x00000010                // TX_AXI_WRBK_WR_ERR_RESP[4]
#define WDMA_ERR_INT_STATUS_TX_AXI_WRBK_WR_ERR_RESP_SHFT 4
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_RD_ERR_RESP_MASK 0x00000008                // TX_AXI_PREF_RD_ERR_RESP[3]
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_RD_ERR_RESP_SHFT 3
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_WR_ERR_RESP_MASK 0x00000004                // TX_AXI_PREF_WR_ERR_RESP[2]
#define WDMA_ERR_INT_STATUS_TX_AXI_PREF_WR_ERR_RESP_SHFT 2
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_RD_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_RD_ERR_RESP_MASK 0x00000002                // TX_AXI_BURST_RD_ERR_RESP[1]
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_RD_ERR_RESP_SHFT 1
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_WR_ERR_RESP_ADDR WDMA_ERR_INT_STATUS_ADDR
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_WR_ERR_RESP_MASK 0x00000001                // TX_AXI_BURST_WR_ERR_RESP[0]
#define WDMA_ERR_INT_STATUS_TX_AXI_BURST_WR_ERR_RESP_SHFT 0

/* =====================================================================================

  ---DMA_GLO_CFG1 (0x15104800 + 0x0234)---

    TX_BST_THRES[3..0]           - (RW) Burst threshold
    RX_BST_THRES[7..4]           - (RW) Burst threshold
    TX_SEG1_EN[8]                - (RW) Enable to support TX two segment
    RX_SCATHER_BYPASS[9]         - (RW) Bypasss RX scatter/scatter
    RX_JBP_ID_EN[10]             - (RW) Enable tagging ID for RX scatter/gather
    RX_MERGE_ID_EN[11]           - (RW) Enable to merge AXI ID with two AXI masters
    RESERVED12[12]               - (RO) Reserved bits
    TX_PREF_CHK_DDONE2_EN[13]    - (RW) Enable to check DDONE2
    TX_PREF_CHK_DDONE2_BUSY[14]  - (RO) State of checking ddone2
    TX_PREF_CHK_DDONE2_CLR[15]   - (A0) Clear prefetch checks ddone2 counters
    TX_PREF_CHK_DDONE2_METHOD_CLR[16] - (RW) Clear prefetch checks ddone2 method
    RX_PREF_CHK_DDONE2_EN[17]    - (RW) Enable to check DDONE2
    RX_PREF_CHK_DDONE2_BUSY[18]  - (RO) State of checking ddone2
    RX_PREF_CHK_DDONE2_CLR[19]   - (A0) Clear prefetch checks ddone2 counters
    RX_PREF_CHK_DDONE2_METHOD_CLR[20] - (RW) Clear prefetch checks ddone2 method
    PREF_CHK_DDONE_DW0[21]       - (RW) Enable to check DDONE of DW0(bit0~63)
    PREF_CHK_DDONE_DW1[22]       - (RW) Enable to check DDONE of DW1(bit64~127)
    PREF_CHK_DDONE_DW2[23]       - (RW) Enable to check DDONE of DW2(bit128~191)
    PREF_CHK_DDONE_DW3[24]       - (RW) Enable to check DDONE of DW3(bit192~255)
    PREF_CHK_DDONE_POL[25]       - (RW) check DW0~DW3 DDONE polarity selection
                                     0 : 0 is correct ddone value
                                     1 : 1 is correct ddone value
    FLUSH_EN[26]                 - (RW) Enable flush function
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_GLO_CFG1_FLUSH_EN_ADDR                    WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_FLUSH_EN_MASK                    0x04000000                // FLUSH_EN[26]
#define WDMA_GLO_CFG1_FLUSH_EN_SHFT                    26
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_POL_ADDR          WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_POL_MASK          0x02000000                // PREF_CHK_DDONE_POL[25]
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_POL_SHFT          25
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW3_ADDR          WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW3_MASK          0x01000000                // PREF_CHK_DDONE_DW3[24]
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW3_SHFT          24
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW2_ADDR          WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW2_MASK          0x00800000                // PREF_CHK_DDONE_DW2[23]
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW2_SHFT          23
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW1_ADDR          WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW1_MASK          0x00400000                // PREF_CHK_DDONE_DW1[22]
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW1_SHFT          22
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW0_ADDR          WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW0_MASK          0x00200000                // PREF_CHK_DDONE_DW0[21]
#define WDMA_GLO_CFG1_PREF_CHK_DDONE_DW0_SHFT          21
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_METHOD_CLR_ADDR WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_METHOD_CLR_MASK 0x00100000                // RX_PREF_CHK_DDONE2_METHOD_CLR[20]
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_METHOD_CLR_SHFT 20
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_CLR_ADDR      WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_CLR_MASK      0x00080000                // RX_PREF_CHK_DDONE2_CLR[19]
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_CLR_SHFT      19
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_BUSY_ADDR     WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_BUSY_MASK     0x00040000                // RX_PREF_CHK_DDONE2_BUSY[18]
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_BUSY_SHFT     18
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_EN_ADDR       WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_EN_MASK       0x00020000                // RX_PREF_CHK_DDONE2_EN[17]
#define WDMA_GLO_CFG1_RX_PREF_CHK_DDONE2_EN_SHFT       17
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_METHOD_CLR_ADDR WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_METHOD_CLR_MASK 0x00010000                // TX_PREF_CHK_DDONE2_METHOD_CLR[16]
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_METHOD_CLR_SHFT 16
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_CLR_ADDR      WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_CLR_MASK      0x00008000                // TX_PREF_CHK_DDONE2_CLR[15]
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_CLR_SHFT      15
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_BUSY_ADDR     WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_BUSY_MASK     0x00004000                // TX_PREF_CHK_DDONE2_BUSY[14]
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_BUSY_SHFT     14
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_EN_ADDR       WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_EN_MASK       0x00002000                // TX_PREF_CHK_DDONE2_EN[13]
#define WDMA_GLO_CFG1_TX_PREF_CHK_DDONE2_EN_SHFT       13
#define WDMA_GLO_CFG1_RX_MERGE_ID_EN_ADDR              WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_MERGE_ID_EN_MASK              0x00000800                // RX_MERGE_ID_EN[11]
#define WDMA_GLO_CFG1_RX_MERGE_ID_EN_SHFT              11
#define WDMA_GLO_CFG1_RX_JBP_ID_EN_ADDR                WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_JBP_ID_EN_MASK                0x00000400                // RX_JBP_ID_EN[10]
#define WDMA_GLO_CFG1_RX_JBP_ID_EN_SHFT                10
#define WDMA_GLO_CFG1_RX_SCATHER_BYPASS_ADDR           WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_SCATHER_BYPASS_MASK           0x00000200                // RX_SCATHER_BYPASS[9]
#define WDMA_GLO_CFG1_RX_SCATHER_BYPASS_SHFT           9
#define WDMA_GLO_CFG1_TX_SEG1_EN_ADDR                  WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_SEG1_EN_MASK                  0x00000100                // TX_SEG1_EN[8]
#define WDMA_GLO_CFG1_TX_SEG1_EN_SHFT                  8
#define WDMA_GLO_CFG1_RX_BST_THRES_ADDR                WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_RX_BST_THRES_MASK                0x000000F0                // RX_BST_THRES[7..4]
#define WDMA_GLO_CFG1_RX_BST_THRES_SHFT                4
#define WDMA_GLO_CFG1_TX_BST_THRES_ADDR                WDMA_GLO_CFG1_ADDR
#define WDMA_GLO_CFG1_TX_BST_THRES_MASK                0x0000000F                // TX_BST_THRES[3..0]
#define WDMA_GLO_CFG1_TX_BST_THRES_SHFT                0

/* =====================================================================================

  ---DMA_TX_XDMA_FIFO_CFG0 (0x15104800 + 0x0238)---

    TX_PAR_FIFO_CLEAR[0]         - (RW) Clear parser FIFO
    TX_PAR_FIFO_EMPTY[1]         - (RO) Parser FIFO empty
    TX_PAR_FIFO_FULL[2]          - (RO) Parser FIFO full
    RESERVED3[3]                 - (RO) Reserved bits
    TX_CMD_FIFO_CLEAR[4]         - (RW) Clear CMD FIFO
    TX_CMD_FIFO_EMPTY[5]         - (RO) CMD FIFO empty
    TX_CMD_FIFO_FULL[6]          - (RO) CMD FIFO full
    RESERVED7[7]                 - (RO) Reserved bits
    TX_DMAD_FIFO_CLEAR[8]        - (RW) Clear DMAD FIFO
    TX_DMAD_FIFO_EMPTY[9]        - (RO) DMAD FIFO empty
    TX_DMAD_FIFO_FULL[10]        - (RO) DMAD FIFO full
    RESERVED11[11]               - (RO) Reserved bits
    TX_ARR_FIFO_CLEAR[12]        - (RW) Clear Arrange FIFO
    TX_ARR_FIFO_EMPTY[13]        - (RO) Arrange FIFO empty
    TX_ARR_FIFO_FULL[14]         - (RO) Arrange FIFO full
    TX_ALN_FIFO_CLEAR[15]        - (RW) Clear Align FIFO
    TX_ALN_FIFO_EMPTY[16]        - (RO) Align FIFO empty
    TX_ALN_FIFO_FULL[17]         - (RO) Align FIFO full
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_FULL_ADDR   WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_FULL_MASK   0x00020000                // TX_ALN_FIFO_FULL[17]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_FULL_SHFT   17
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_EMPTY_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_EMPTY_MASK  0x00010000                // TX_ALN_FIFO_EMPTY[16]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_EMPTY_SHFT  16
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_CLEAR_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_CLEAR_MASK  0x00008000                // TX_ALN_FIFO_CLEAR[15]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ALN_FIFO_CLEAR_SHFT  15
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_FULL_ADDR   WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_FULL_MASK   0x00004000                // TX_ARR_FIFO_FULL[14]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_FULL_SHFT   14
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_EMPTY_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_EMPTY_MASK  0x00002000                // TX_ARR_FIFO_EMPTY[13]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_EMPTY_SHFT  13
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_CLEAR_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_CLEAR_MASK  0x00001000                // TX_ARR_FIFO_CLEAR[12]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_ARR_FIFO_CLEAR_SHFT  12
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_FULL_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_FULL_MASK  0x00000400                // TX_DMAD_FIFO_FULL[10]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_FULL_SHFT  10
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_EMPTY_ADDR WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_EMPTY_MASK 0x00000200                // TX_DMAD_FIFO_EMPTY[9]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_EMPTY_SHFT 9
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_CLEAR_ADDR WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_CLEAR_MASK 0x00000100                // TX_DMAD_FIFO_CLEAR[8]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_DMAD_FIFO_CLEAR_SHFT 8
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_FULL_ADDR   WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_FULL_MASK   0x00000040                // TX_CMD_FIFO_FULL[6]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_FULL_SHFT   6
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_EMPTY_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_EMPTY_MASK  0x00000020                // TX_CMD_FIFO_EMPTY[5]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_EMPTY_SHFT  5
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_CLEAR_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_CLEAR_MASK  0x00000010                // TX_CMD_FIFO_CLEAR[4]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_CMD_FIFO_CLEAR_SHFT  4
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_FULL_ADDR   WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_FULL_MASK   0x00000004                // TX_PAR_FIFO_FULL[2]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_FULL_SHFT   2
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_EMPTY_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_EMPTY_MASK  0x00000002                // TX_PAR_FIFO_EMPTY[1]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_EMPTY_SHFT  1
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_CLEAR_ADDR  WDMA_TX_XDMA_FIFO_CFG0_ADDR
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_CLEAR_MASK  0x00000001                // TX_PAR_FIFO_CLEAR[0]
#define WDMA_TX_XDMA_FIFO_CFG0_TX_PAR_FIFO_CLEAR_SHFT  0

/* =====================================================================================

  ---DMA_RX_XDMA_FIFO_CFG0 (0x15104800 + 0x023c)---

    RX_PAR_FIFO_CLEAR[0]         - (RW) Clear parser FIFO
    RX_PAR_FIFO_EMPTY[1]         - (RO) Parser FIFO empty
    RX_PAR_FIFO_FULL[2]          - (RO) Parser FIFO full
    RESERVED3[3]                 - (RO) Reserved bits
    RX_CMD_FIFO_CLEAR[4]         - (RW) Clear CMD FIFO
    RX_CMD_FIFO_EMPTY[5]         - (RO) CMD FIFO empty
    RX_CMD_FIFO_FULL[6]          - (RO) CMD FIFO full
    RESERVED7[7]                 - (RO) Reserved bits
    RX_DMAD_FIFO_CLEAR[8]        - (RW) Clear DMAD FIFO
    RX_DMAD_FIFO_EMPTY[9]        - (RO) DMAD FIFO empty
    RX_DMAD_FIFO_FULL[10]        - (RO) DMAD FIFO full
    RESERVED11[11]               - (RO) Reserved bits
    RX_ARR_FIFO_CLEAR[12]        - (RW) Clear Arrange FIFO
    RX_ARR_FIFO_EMPTY[13]        - (RO) Arrange FIFO empty
    RX_ARR_FIFO_FULL[14]         - (RO) Arrange FIFO full
    RX_LEN_FIFO_CLEAR[15]        - (RW) Clear len FIFO
    RX_LEN_FIFO_EMPTY[16]        - (RO) Len FIFO empty
    RX_LEN_FIFO_FULL[17]         - (RO) Len FIFO full
    RX_WID_FIFO_CLEAR[18]        - (RW) Clear len FIFO
    RX_WID_FIFO_EMPTY[19]        - (RO) WID FIFO empty
    RX_WID_FIFO_FULL[20]         - (RO) WID FIFO full
    RX_BID_FIFO_CLEAR[21]        - (RW) Clear BID FIFO
    RX_BID_FIFO_EMPTY[22]        - (RO) BID FIFO empty
    RX_BID_FIFO_FULL[23]         - (RO) BID FIFO empty
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_FULL_MASK   0x00800000                // RX_BID_FIFO_FULL[23]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_FULL_SHFT   23
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_EMPTY_MASK  0x00400000                // RX_BID_FIFO_EMPTY[22]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_EMPTY_SHFT  22
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_CLEAR_MASK  0x00200000                // RX_BID_FIFO_CLEAR[21]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_BID_FIFO_CLEAR_SHFT  21
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_FULL_MASK   0x00100000                // RX_WID_FIFO_FULL[20]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_FULL_SHFT   20
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_EMPTY_MASK  0x00080000                // RX_WID_FIFO_EMPTY[19]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_EMPTY_SHFT  19
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_CLEAR_MASK  0x00040000                // RX_WID_FIFO_CLEAR[18]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_WID_FIFO_CLEAR_SHFT  18
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_FULL_MASK   0x00020000                // RX_LEN_FIFO_FULL[17]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_FULL_SHFT   17
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_EMPTY_MASK  0x00010000                // RX_LEN_FIFO_EMPTY[16]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_EMPTY_SHFT  16
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_CLEAR_MASK  0x00008000                // RX_LEN_FIFO_CLEAR[15]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_LEN_FIFO_CLEAR_SHFT  15
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_FULL_MASK   0x00004000                // RX_ARR_FIFO_FULL[14]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_FULL_SHFT   14
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_EMPTY_MASK  0x00002000                // RX_ARR_FIFO_EMPTY[13]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_EMPTY_SHFT  13
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_CLEAR_MASK  0x00001000                // RX_ARR_FIFO_CLEAR[12]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_ARR_FIFO_CLEAR_SHFT  12
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_FULL_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_FULL_MASK  0x00000400                // RX_DMAD_FIFO_FULL[10]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_FULL_SHFT  10
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_EMPTY_ADDR WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_EMPTY_MASK 0x00000200                // RX_DMAD_FIFO_EMPTY[9]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_EMPTY_SHFT 9
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_CLEAR_ADDR WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_CLEAR_MASK 0x00000100                // RX_DMAD_FIFO_CLEAR[8]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_DMAD_FIFO_CLEAR_SHFT 8
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_FULL_MASK   0x00000040                // RX_CMD_FIFO_FULL[6]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_FULL_SHFT   6
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_EMPTY_MASK  0x00000020                // RX_CMD_FIFO_EMPTY[5]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_EMPTY_SHFT  5
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_CLEAR_MASK  0x00000010                // RX_CMD_FIFO_CLEAR[4]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_CMD_FIFO_CLEAR_SHFT  4
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_FULL_ADDR   WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_FULL_MASK   0x00000004                // RX_PAR_FIFO_FULL[2]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_FULL_SHFT   2
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_EMPTY_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_EMPTY_MASK  0x00000002                // RX_PAR_FIFO_EMPTY[1]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_EMPTY_SHFT  1
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_CLEAR_ADDR  WDMA_RX_XDMA_FIFO_CFG0_ADDR
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_CLEAR_MASK  0x00000001                // RX_PAR_FIFO_CLEAR[0]
#define WDMA_RX_XDMA_FIFO_CFG0_RX_PAR_FIFO_CLEAR_SHFT  0

/* =====================================================================================

  ---DMA_INT_STS_GRP0 (0x15104800 + 0x0240)---

    DMA_INT_STS_GRP0[31..0]      - (RO) Interrupt group 0 status, this information already "and" with "~WDMA_INT_GRP1 & ~WDMA_INT_GRP2"
                                     Each bit definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_STS_GRP0_DMA_INT_STS_GRP0_ADDR        WDMA_INT_STS_GRP0_ADDR
#define WDMA_INT_STS_GRP0_DMA_INT_STS_GRP0_MASK        0xFFFFFFFF                // DMA_INT_STS_GRP0[31..0]
#define WDMA_INT_STS_GRP0_DMA_INT_STS_GRP0_SHFT        0

/* =====================================================================================

  ---DMA_INT_STS_GRP1 (0x15104800 + 0x0244)---

    DMA_INT_STS_GRP1[31..0]      - (RO) Interrupt group 1 status, this information already "and" with "WDMA_INT_GRP1"
                                     Each bit definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_STS_GRP1_DMA_INT_STS_GRP1_ADDR        WDMA_INT_STS_GRP1_ADDR
#define WDMA_INT_STS_GRP1_DMA_INT_STS_GRP1_MASK        0xFFFFFFFF                // DMA_INT_STS_GRP1[31..0]
#define WDMA_INT_STS_GRP1_DMA_INT_STS_GRP1_SHFT        0

/* =====================================================================================

  ---DMA_INT_STS_GRP2 (0x15104800 + 0x0248)---

    DMA_INT_STS_GRP2[31..0]      - (RO) Interrupt group 2 status, this information already "and" with "WDMA_INT_GRP2"
                                     Each bit definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_STS_GRP2_DMA_INT_STS_GRP2_ADDR        WDMA_INT_STS_GRP2_ADDR
#define WDMA_INT_STS_GRP2_DMA_INT_STS_GRP2_MASK        0xFFFFFFFF                // DMA_INT_STS_GRP2[31..0]
#define WDMA_INT_STS_GRP2_DMA_INT_STS_GRP2_SHFT        0

/* =====================================================================================

  ---DMA_INT_STS_GRP3 (0x15104800 + 0x024c)---

    DMA_INT_STS_GRP3[31..0]      - (RO) Interrupt group 3 status, this information already "and" with "WDMA_INT_GRP3"
                                     Each bit definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_STS_GRP3_DMA_INT_STS_GRP3_ADDR        WDMA_INT_STS_GRP3_ADDR
#define WDMA_INT_STS_GRP3_DMA_INT_STS_GRP3_MASK        0xFFFFFFFF                // DMA_INT_STS_GRP3[31..0]
#define WDMA_INT_STS_GRP3_DMA_INT_STS_GRP3_SHFT        0

/* =====================================================================================

  ---DMA_INT_GRP1 (0x15104800 + 0x0250)---

    DMA_INT_GRP1[31..0]          - (RW) Interrupt group 1 assignment.
                                     Each bit's definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_GRP1_DMA_INT_GRP1_ADDR                WDMA_INT_GRP1_ADDR
#define WDMA_INT_GRP1_DMA_INT_GRP1_MASK                0xFFFFFFFF                // DMA_INT_GRP1[31..0]
#define WDMA_INT_GRP1_DMA_INT_GRP1_SHFT                0

/* =====================================================================================

  ---DMA_INT_GRP2 (0x15104800 + 0x0254)---

    DMA_INT_GRP2[31..0]          - (RW) Interrupt group 2 assignment.
                                     Each bit's definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_GRP2_DMA_INT_GRP2_ADDR                WDMA_INT_GRP2_ADDR
#define WDMA_INT_GRP2_DMA_INT_GRP2_MASK                0xFFFFFFFF                // DMA_INT_GRP2[31..0]
#define WDMA_INT_GRP2_DMA_INT_GRP2_SHFT                0

/* =====================================================================================

  ---DMA_INT_GRP3 (0x15104800 + 0x0258)---

    DMA_INT_GRP3[31..0]          - (RW) Interrupt group 3 assignment.
                                     Each bit's definition is same as "WDMA_INT_STATUS"

 =====================================================================================*/
#define WDMA_INT_GRP3_DMA_INT_GRP3_ADDR                WDMA_INT_GRP3_ADDR
#define WDMA_INT_GRP3_DMA_INT_GRP3_MASK                0xFFFFFFFF                // DMA_INT_GRP3[31..0]
#define WDMA_INT_GRP3_DMA_INT_GRP3_SHFT                0

/* =====================================================================================

  ---DMA_BUS_CFG (0x15104800 + 0x0260)---

    AXI_CG_DISABLE[0]            - (RW) Disable clock gating for xDMA arbiter
    RESERVED1[1]                 - (RO) Reserved bits
    AXI_QOS_ON[2]                - (RW) Enable QoS function(Enable CBIP ultra feature)
    AXI_OUTSTANDING_EXTEND[3]    - (RW) Auto adds extra outstanding for QoS command(Enable CBIP ultra feature)
    AXI_ERRMID_SET_BIRQ[4]       - (RO) Detects error MID from slave in B channel(from arbiter)
    AXI_ERRMID_SET_RIRQ[5]       - (RO) Detects error MID from slave in R channel(from arbiter)
    AXI_LOCK_ERROR[6]            - (RO) Detects lock error(arbiter)
    RESERVED7[7]                 - (RO) Reserved bits
    AXI_W_BUSY[8]                - (RO) Write transaction not completes(from arbiter)
    AXI_R_BUSY[9]                - (RO) Read transaction not completes(from arbiter)
    AXI_CTRL_UPDATED[10]         - (RO) Master interface control signal updated status(from arbiter)
    RESERVED11[11]               - (RO) Reserved bits
    AXI_ULTRA_TXDMA[13..12]      - (RW) AXI ULTRA for TX_DMA AW Channel
    AXI_ULTRA_RXDMA[15..14]      - (RW) AXI ULTRA for RX_DMA AR Channel
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_BUS_CFG_AXI_ULTRA_RXDMA_ADDR              WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_ULTRA_RXDMA_MASK              0x0000C000                // AXI_ULTRA_RXDMA[15..14]
#define WDMA_BUS_CFG_AXI_ULTRA_RXDMA_SHFT              14
#define WDMA_BUS_CFG_AXI_ULTRA_TXDMA_ADDR              WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_ULTRA_TXDMA_MASK              0x00003000                // AXI_ULTRA_TXDMA[13..12]
#define WDMA_BUS_CFG_AXI_ULTRA_TXDMA_SHFT              12
#define WDMA_BUS_CFG_AXI_CTRL_UPDATED_ADDR             WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_CTRL_UPDATED_MASK             0x00000400                // AXI_CTRL_UPDATED[10]
#define WDMA_BUS_CFG_AXI_CTRL_UPDATED_SHFT             10
#define WDMA_BUS_CFG_AXI_R_BUSY_ADDR                   WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_R_BUSY_MASK                   0x00000200                // AXI_R_BUSY[9]
#define WDMA_BUS_CFG_AXI_R_BUSY_SHFT                   9
#define WDMA_BUS_CFG_AXI_W_BUSY_ADDR                   WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_W_BUSY_MASK                   0x00000100                // AXI_W_BUSY[8]
#define WDMA_BUS_CFG_AXI_W_BUSY_SHFT                   8
#define WDMA_BUS_CFG_AXI_LOCK_ERROR_ADDR               WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_LOCK_ERROR_MASK               0x00000040                // AXI_LOCK_ERROR[6]
#define WDMA_BUS_CFG_AXI_LOCK_ERROR_SHFT               6
#define WDMA_BUS_CFG_AXI_ERRMID_SET_RIRQ_ADDR          WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_ERRMID_SET_RIRQ_MASK          0x00000020                // AXI_ERRMID_SET_RIRQ[5]
#define WDMA_BUS_CFG_AXI_ERRMID_SET_RIRQ_SHFT          5
#define WDMA_BUS_CFG_AXI_ERRMID_SET_BIRQ_ADDR          WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_ERRMID_SET_BIRQ_MASK          0x00000010                // AXI_ERRMID_SET_BIRQ[4]
#define WDMA_BUS_CFG_AXI_ERRMID_SET_BIRQ_SHFT          4
#define WDMA_BUS_CFG_AXI_OUTSTANDING_EXTEND_ADDR       WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_OUTSTANDING_EXTEND_MASK       0x00000008                // AXI_OUTSTANDING_EXTEND[3]
#define WDMA_BUS_CFG_AXI_OUTSTANDING_EXTEND_SHFT       3
#define WDMA_BUS_CFG_AXI_QOS_ON_ADDR                   WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_QOS_ON_MASK                   0x00000004                // AXI_QOS_ON[2]
#define WDMA_BUS_CFG_AXI_QOS_ON_SHFT                   2
#define WDMA_BUS_CFG_AXI_CG_DISABLE_ADDR               WDMA_BUS_CFG_ADDR
#define WDMA_BUS_CFG_AXI_CG_DISABLE_MASK               0x00000001                // AXI_CG_DISABLE[0]
#define WDMA_BUS_CFG_AXI_CG_DISABLE_SHFT               0

/* =====================================================================================

  ---DMA_ULTRA_CFG (0x15104800 + 0x0264)---

    AXI_PREULTRA_THRES[10..0]    - (RW) AXI Pre-ultra threshold.
                                     If CDM TX free FIFO count bigger than AXI_PREULTRA_THRES, will trigger AXI Pre-ultra transaction when AXI_PREULTRA_EN = 1.
    RESERVED11[14..11]           - (RO) Reserved bits
    AXI_PREULTRA_EN[15]          - (RW) Enable AXI Pre-ultra transaction
    AXI_ULTRA_THRES[26..16]      - (RW) AXI Ultra threshold.
                                     If CDM TX free FIFO count bigger than AXI_ULTRA_THRES, will trigger AXI Ultra transaction when AXI_ULTRA_EN = 1.
    RESERVED27[30..27]           - (RO) Reserved bits
    AXI_ULTRA_EN[31]             - (RW) Enable AXI Ultra transaction

 =====================================================================================*/
#define WDMA_ULTRA_CFG_AXI_ULTRA_EN_ADDR               WDMA_ULTRA_CFG_ADDR
#define WDMA_ULTRA_CFG_AXI_ULTRA_EN_MASK               0x80000000                // AXI_ULTRA_EN[31]
#define WDMA_ULTRA_CFG_AXI_ULTRA_EN_SHFT               31
#define WDMA_ULTRA_CFG_AXI_ULTRA_THRES_ADDR            WDMA_ULTRA_CFG_ADDR
#define WDMA_ULTRA_CFG_AXI_ULTRA_THRES_MASK            0x07FF0000                // AXI_ULTRA_THRES[26..16]
#define WDMA_ULTRA_CFG_AXI_ULTRA_THRES_SHFT            16
#define WDMA_ULTRA_CFG_AXI_PREULTRA_EN_ADDR            WDMA_ULTRA_CFG_ADDR
#define WDMA_ULTRA_CFG_AXI_PREULTRA_EN_MASK            0x00008000                // AXI_PREULTRA_EN[15]
#define WDMA_ULTRA_CFG_AXI_PREULTRA_EN_SHFT            15
#define WDMA_ULTRA_CFG_AXI_PREULTRA_THRES_ADDR         WDMA_ULTRA_CFG_ADDR
#define WDMA_ULTRA_CFG_AXI_PREULTRA_THRES_MASK         0x000007FF                // AXI_PREULTRA_THRES[10..0]
#define WDMA_ULTRA_CFG_AXI_PREULTRA_THRES_SHFT         0

/* =====================================================================================

  ---DMA_XFER_CNT_CFG1 (0x15104800 + 0x0268)---

    MAX_PKT_NUM[15..0]           - (RW)  xxx
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_XFER_CNT_CFG1_MAX_PKT_NUM_ADDR            WDMA_XFER_CNT_CFG1_ADDR
#define WDMA_XFER_CNT_CFG1_MAX_PKT_NUM_MASK            0x0000FFFF                // MAX_PKT_NUM[15..0]
#define WDMA_XFER_CNT_CFG1_MAX_PKT_NUM_SHFT            0

/* =====================================================================================

  ---DMA_SDL_CFG (0x15104800 + 0x0270)---

    SDL[15..0]                   - (RW) RX_DMA read SDL value
    SDL_EN[16]                   - (RW) Enable RX_DMA read SDL from CR
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_SDL_CFG_SDL_EN_ADDR                       WDMA_SDL_CFG_ADDR
#define WDMA_SDL_CFG_SDL_EN_MASK                       0x00010000                // SDL_EN[16]
#define WDMA_SDL_CFG_SDL_EN_SHFT                       16
#define WDMA_SDL_CFG_SDL_ADDR                          WDMA_SDL_CFG_ADDR
#define WDMA_SDL_CFG_SDL_MASK                          0x0000FFFF                // SDL[15..0]
#define WDMA_SDL_CFG_SDL_SHFT                          0

/* =====================================================================================

  ---DMA_IDLE_MASK (0x15104800 + 0x0274)---

    IDLE_MASK[2..0]              - (RW) Idle signal mask
    RESERVED3[3]                 - (RO) Reserved bits
    AXI_IDLE[4]                  - (RO) AXM idle status
    DMA_IDLE[5]                  - (RO) DMA idle status
    RESERVED6[31..6]             - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_IDLE_MASK_DMA_IDLE_ADDR                   WDMA_IDLE_MASK_ADDR
#define WDMA_IDLE_MASK_DMA_IDLE_MASK                   0x00000020                // DMA_IDLE[5]
#define WDMA_IDLE_MASK_DMA_IDLE_SHFT                   5
#define WDMA_IDLE_MASK_AXI_IDLE_ADDR                   WDMA_IDLE_MASK_ADDR
#define WDMA_IDLE_MASK_AXI_IDLE_MASK                   0x00000010                // AXI_IDLE[4]
#define WDMA_IDLE_MASK_AXI_IDLE_SHFT                   4
#define WDMA_IDLE_MASK_IDLE_MASK_ADDR                  WDMA_IDLE_MASK_ADDR
#define WDMA_IDLE_MASK_IDLE_MASK_MASK                  0x00000007                // IDLE_MASK[2..0]
#define WDMA_IDLE_MASK_IDLE_MASK_SHFT                  0

/* =====================================================================================

  ---DMA_SCH_Q01_CFG (0x15104800 + 0x0280)---

    MAX_RATE0[9..0]              - (RW) Define the limited Max rate for queue if MAX_RATE_ULMT is 0.
                                     The value specified represents the amount of 4-byte quota to be added into the queue #1 bucket per 125us.
                                     For example: If 512 is programmed, then the max rate limited is :
                                     512 * 4 bytes/125us = 16.384M bytes/sec or 131Mbps
    MIN_RATE_RATIO0[11..10]      - (RW) Define the guaranteed Min rate based on MAX_RATE
    MAX_WEIGHT0[13..12]          - (RW) Define the auto-reload bucket size if MAX_RATE_ULMT is set to 1. It is also served as excess bandwidth allocation ratio for servicing queue
    MAX_RATE_ULMT0[14]           - (RW) Maximum rate limitation
    MAX_BKT_SIZE0[15]            - (RW) Maximum bucket size
    MAX_RATE1[25..16]            - (RW) Define the limited Max rate for queue if MAX_RATE_ULMT is 0.
                                     The value specified represents the amount of 4-byte quota to be added into the queue #1 bucket per 125us.
                                     For example: If 512 is programmed, then the max rate limited is :
                                     512 * 4 bytes/125us = 16.384M bytes/sec or 131Mbps
    MIN_RATE_RATIO1[27..26]      - (RW) Define the guaranteed Min rate based on MAX_RATE
    MAX_WEIGHT1[29..28]          - (RW) Define the auto-reload bucket size if MAX_RATE_ULMT is set to 1. It is also served as excess bandwidth allocation ratio for servicing queue
    MAX_RATE_ULMT1[30]           - (RW) Maximum rate limitation
    MAX_BKT_SIZE1[31]            - (RW) Maximum bucket size

 =====================================================================================*/
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_ADDR            WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_MASK            0x80000000                // MAX_BKT_SIZE1[31]
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_SHFT            31
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_ADDR           WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_MASK           0x40000000                // MAX_RATE_ULMT1[30]
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_SHFT           30
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT1_ADDR              WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT1_MASK              0x30000000                // MAX_WEIGHT1[29..28]
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT1_SHFT              28
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_ADDR          WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_MASK          0x0C000000                // MIN_RATE_RATIO1[27..26]
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_SHFT          26
#define WDMA_SCH_Q01_CFG_MAX_RATE1_ADDR                WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_RATE1_MASK                0x03FF0000                // MAX_RATE1[25..16]
#define WDMA_SCH_Q01_CFG_MAX_RATE1_SHFT                16
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_ADDR            WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_MASK            0x00008000                // MAX_BKT_SIZE0[15]
#define WDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_SHFT            15
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_ADDR           WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_MASK           0x00004000                // MAX_RATE_ULMT0[14]
#define WDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_SHFT           14
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT0_ADDR              WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT0_MASK              0x00003000                // MAX_WEIGHT0[13..12]
#define WDMA_SCH_Q01_CFG_MAX_WEIGHT0_SHFT              12
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_ADDR          WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_MASK          0x00000C00                // MIN_RATE_RATIO0[11..10]
#define WDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_SHFT          10
#define WDMA_SCH_Q01_CFG_MAX_RATE0_ADDR                WDMA_SCH_Q01_CFG_ADDR
#define WDMA_SCH_Q01_CFG_MAX_RATE0_MASK                0x000003FF                // MAX_RATE0[9..0]
#define WDMA_SCH_Q01_CFG_MAX_RATE0_SHFT                0

/* =====================================================================================

  ---DMA_SCH_Q23_CFG (0x15104800 + 0x0284)---

    MAX_RATE2[9..0]              - (RW) Define the limited Max rate for queue if MAX_RATE_ULMT is 0.
                                     The value specified represents the amount of 4-byte quota to be added into the queue #1 bucket per 125us.
                                     For example: If 512 is programmed, then the max rate limited is :
                                     512 * 4 bytes/125us = 16.384M bytes/sec or 131Mbps
    MIN_RATE_RATIO2[11..10]      - (RW) Define the guaranteed Min rate based on MAX_RATE
    MAX_WEIGHT2[13..12]          - (RW) Define the auto-reload bucket size if MAX_RATE_ULMT is set to 1. It is also served as excess bandwidth allocation ratio for servicing queue
    MAX_RATE_ULMT2[14]           - (RW) Maximum rate limitation
    MAX_BKT_SIZE2[15]            - (RW) Maximum bucket size
    MAX_RATE3[25..16]            - (RW) Define the limited Max rate for queue if MAX_RATE_ULMT is 0.
                                     The value specified represents the amount of 4-byte quota to be added into the queue #1 bucket per 125us.
                                     For example: If 512 is programmed, then the max rate limited is :
                                     512 * 4 bytes/125us = 16.384M bytes/sec or 131Mbps
    MIN_RATE_RATIO3[27..26]      - (RW) Define the guaranteed Min rate based on MAX_RATE
    MAX_WEIGHT3[29..28]          - (RW) Define the auto-reload bucket size if MAX_RATE_ULMT is set to 1. It is also served as excess bandwidth allocation ratio for servicing queue
    MAX_RATE_ULMT3[30]           - (RW) Maximum rate limitation
    MAX_BKT_SIZE3[31]            - (RW) Maximum bucket size

 =====================================================================================*/
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE3_ADDR            WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE3_MASK            0x80000000                // MAX_BKT_SIZE3[31]
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE3_SHFT            31
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT3_ADDR           WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT3_MASK           0x40000000                // MAX_RATE_ULMT3[30]
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT3_SHFT           30
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT3_ADDR              WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT3_MASK              0x30000000                // MAX_WEIGHT3[29..28]
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT3_SHFT              28
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO3_ADDR          WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO3_MASK          0x0C000000                // MIN_RATE_RATIO3[27..26]
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO3_SHFT          26
#define WDMA_SCH_Q23_CFG_MAX_RATE3_ADDR                WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_RATE3_MASK                0x03FF0000                // MAX_RATE3[25..16]
#define WDMA_SCH_Q23_CFG_MAX_RATE3_SHFT                16
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE2_ADDR            WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE2_MASK            0x00008000                // MAX_BKT_SIZE2[15]
#define WDMA_SCH_Q23_CFG_MAX_BKT_SIZE2_SHFT            15
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT2_ADDR           WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT2_MASK           0x00004000                // MAX_RATE_ULMT2[14]
#define WDMA_SCH_Q23_CFG_MAX_RATE_ULMT2_SHFT           14
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT2_ADDR              WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT2_MASK              0x00003000                // MAX_WEIGHT2[13..12]
#define WDMA_SCH_Q23_CFG_MAX_WEIGHT2_SHFT              12
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO2_ADDR          WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO2_MASK          0x00000C00                // MIN_RATE_RATIO2[11..10]
#define WDMA_SCH_Q23_CFG_MIN_RATE_RATIO2_SHFT          10
#define WDMA_SCH_Q23_CFG_MAX_RATE2_ADDR                WDMA_SCH_Q23_CFG_ADDR
#define WDMA_SCH_Q23_CFG_MAX_RATE2_MASK                0x000003FF                // MAX_RATE2[9..0]
#define WDMA_SCH_Q23_CFG_MAX_RATE2_SHFT                0

/* =====================================================================================

  ---DMA_INT_STATUS_0 (0x15104800 + 0x0290)---

    TX_DONE_INT0[0]              - (W1C) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (W1C) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (W1C) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (W1C) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (W1C) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (W1C) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (W1C) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (W1C) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (W1C) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (W1C) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (W1C) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (W1C) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (W1C) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (W1C) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (W1C) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (W1C) RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WDMA_INT_STATUS_0_RX_COHERENT_ADDR             WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_COHERENT_MASK             0x80000000                // RX_COHERENT[31]
#define WDMA_INT_STATUS_0_RX_COHERENT_SHFT             31
#define WDMA_INT_STATUS_0_RX_DLY_INT_ADDR              WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_DLY_INT_MASK              0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_STATUS_0_RX_DLY_INT_SHFT              30
#define WDMA_INT_STATUS_0_TX_COHERENT_ADDR             WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_COHERENT_MASK             0x20000000                // TX_COHERENT[29]
#define WDMA_INT_STATUS_0_TX_COHERENT_SHFT             29
#define WDMA_INT_STATUS_0_TX_DLY_INT_ADDR              WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DLY_INT_MASK              0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_STATUS_0_TX_DLY_INT_SHFT              28
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT1_MASK        0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT1_SHFT        21
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT0_MASK        0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_STATUS_0_RX_DONE_DLY_INT0_SHFT        20
#define WDMA_INT_STATUS_0_RX_DONE_INT1_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_DONE_INT1_MASK            0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_STATUS_0_RX_DONE_INT1_SHFT            17
#define WDMA_INT_STATUS_0_RX_DONE_INT0_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_RX_DONE_INT0_MASK            0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_STATUS_0_RX_DONE_INT0_SHFT            16
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT3_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT3_MASK        0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT3_SHFT        11
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT2_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT2_MASK        0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT2_SHFT        10
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT1_MASK        0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT1_SHFT        9
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT0_MASK        0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_STATUS_0_TX_DONE_DLY_INT0_SHFT        8
#define WDMA_INT_STATUS_0_TX_DONE_INT3_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_INT3_MASK            0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_STATUS_0_TX_DONE_INT3_SHFT            3
#define WDMA_INT_STATUS_0_TX_DONE_INT2_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_INT2_MASK            0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_STATUS_0_TX_DONE_INT2_SHFT            2
#define WDMA_INT_STATUS_0_TX_DONE_INT1_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_INT1_MASK            0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_STATUS_0_TX_DONE_INT1_SHFT            1
#define WDMA_INT_STATUS_0_TX_DONE_INT0_ADDR            WDMA_INT_STATUS_0_ADDR
#define WDMA_INT_STATUS_0_TX_DONE_INT0_MASK            0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_STATUS_0_TX_DONE_INT0_SHFT            0

/* =====================================================================================

  ---DMA_INT_MASK_0 (0x15104800 + 0x0294)---

    TX_DONE_INT0[0]              - (RW) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RW) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (RW) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (RW) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (RW) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (RW) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (RW) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (RW) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RW) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (RW) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (RW) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (RW) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RW) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (RW) Interrupt enable for TX_DMA data coherent event
    RX_DLY_INT[30]               - (RW) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (RW) Interrupt enable for RX_DMA data coherent event

 =====================================================================================*/
#define WDMA_INT_MASK_0_RX_COHERENT_ADDR               WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_COHERENT_MASK               0x80000000                // RX_COHERENT[31]
#define WDMA_INT_MASK_0_RX_COHERENT_SHFT               31
#define WDMA_INT_MASK_0_RX_DLY_INT_ADDR                WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_DLY_INT_MASK                0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_MASK_0_RX_DLY_INT_SHFT                30
#define WDMA_INT_MASK_0_TX_COHERENT_ADDR               WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_COHERENT_MASK               0x20000000                // TX_COHERENT[29]
#define WDMA_INT_MASK_0_TX_COHERENT_SHFT               29
#define WDMA_INT_MASK_0_TX_DLY_INT_ADDR                WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DLY_INT_MASK                0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_MASK_0_TX_DLY_INT_SHFT                28
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT1_MASK          0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT1_SHFT          21
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT0_MASK          0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_MASK_0_RX_DONE_DLY_INT0_SHFT          20
#define WDMA_INT_MASK_0_RX_DONE_INT1_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_DONE_INT1_MASK              0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_MASK_0_RX_DONE_INT1_SHFT              17
#define WDMA_INT_MASK_0_RX_DONE_INT0_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_RX_DONE_INT0_MASK              0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_MASK_0_RX_DONE_INT0_SHFT              16
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT3_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT3_MASK          0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT3_SHFT          11
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT2_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT2_MASK          0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT2_SHFT          10
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT1_MASK          0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT1_SHFT          9
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT0_MASK          0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_MASK_0_TX_DONE_DLY_INT0_SHFT          8
#define WDMA_INT_MASK_0_TX_DONE_INT3_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_INT3_MASK              0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_MASK_0_TX_DONE_INT3_SHFT              3
#define WDMA_INT_MASK_0_TX_DONE_INT2_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_INT2_MASK              0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_MASK_0_TX_DONE_INT2_SHFT              2
#define WDMA_INT_MASK_0_TX_DONE_INT1_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_INT1_MASK              0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_MASK_0_TX_DONE_INT1_SHFT              1
#define WDMA_INT_MASK_0_TX_DONE_INT0_ADDR              WDMA_INT_MASK_0_ADDR
#define WDMA_INT_MASK_0_TX_DONE_INT0_MASK              0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_MASK_0_TX_DONE_INT0_SHFT              0

/* =====================================================================================

  ---DMA_INT_STATUS_1 (0x15104800 + 0x0298)---

    TX_DONE_INT0[0]              - (W1C) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (W1C) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (W1C) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (W1C) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (W1C) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (W1C) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (W1C) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (W1C) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (W1C) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (W1C) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (W1C) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (W1C) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (W1C) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (W1C) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (W1C) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (W1C) RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WDMA_INT_STATUS_1_RX_COHERENT_ADDR             WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_COHERENT_MASK             0x80000000                // RX_COHERENT[31]
#define WDMA_INT_STATUS_1_RX_COHERENT_SHFT             31
#define WDMA_INT_STATUS_1_RX_DLY_INT_ADDR              WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_DLY_INT_MASK              0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_STATUS_1_RX_DLY_INT_SHFT              30
#define WDMA_INT_STATUS_1_TX_COHERENT_ADDR             WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_COHERENT_MASK             0x20000000                // TX_COHERENT[29]
#define WDMA_INT_STATUS_1_TX_COHERENT_SHFT             29
#define WDMA_INT_STATUS_1_TX_DLY_INT_ADDR              WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DLY_INT_MASK              0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_STATUS_1_TX_DLY_INT_SHFT              28
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT1_MASK        0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT1_SHFT        21
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT0_MASK        0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_STATUS_1_RX_DONE_DLY_INT0_SHFT        20
#define WDMA_INT_STATUS_1_RX_DONE_INT1_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_DONE_INT1_MASK            0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_STATUS_1_RX_DONE_INT1_SHFT            17
#define WDMA_INT_STATUS_1_RX_DONE_INT0_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_RX_DONE_INT0_MASK            0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_STATUS_1_RX_DONE_INT0_SHFT            16
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT3_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT3_MASK        0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT3_SHFT        11
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT2_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT2_MASK        0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT2_SHFT        10
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT1_MASK        0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT1_SHFT        9
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT0_MASK        0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_STATUS_1_TX_DONE_DLY_INT0_SHFT        8
#define WDMA_INT_STATUS_1_TX_DONE_INT3_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_INT3_MASK            0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_STATUS_1_TX_DONE_INT3_SHFT            3
#define WDMA_INT_STATUS_1_TX_DONE_INT2_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_INT2_MASK            0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_STATUS_1_TX_DONE_INT2_SHFT            2
#define WDMA_INT_STATUS_1_TX_DONE_INT1_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_INT1_MASK            0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_STATUS_1_TX_DONE_INT1_SHFT            1
#define WDMA_INT_STATUS_1_TX_DONE_INT0_ADDR            WDMA_INT_STATUS_1_ADDR
#define WDMA_INT_STATUS_1_TX_DONE_INT0_MASK            0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_STATUS_1_TX_DONE_INT0_SHFT            0

/* =====================================================================================

  ---DMA_INT_MASK_1 (0x15104800 + 0x029c)---

    TX_DONE_INT0[0]              - (RW) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RW) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (RW) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (RW) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (RW) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (RW) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (RW) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (RW) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RW) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (RW) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (RW) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (RW) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RW) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (RW) Interrupt enable for TX_DMA data coherent event
    RX_DLY_INT[30]               - (RW) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (RW) Interrupt enable for RX_DMA data coherent event

 =====================================================================================*/
#define WDMA_INT_MASK_1_RX_COHERENT_ADDR               WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_COHERENT_MASK               0x80000000                // RX_COHERENT[31]
#define WDMA_INT_MASK_1_RX_COHERENT_SHFT               31
#define WDMA_INT_MASK_1_RX_DLY_INT_ADDR                WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_DLY_INT_MASK                0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_MASK_1_RX_DLY_INT_SHFT                30
#define WDMA_INT_MASK_1_TX_COHERENT_ADDR               WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_COHERENT_MASK               0x20000000                // TX_COHERENT[29]
#define WDMA_INT_MASK_1_TX_COHERENT_SHFT               29
#define WDMA_INT_MASK_1_TX_DLY_INT_ADDR                WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DLY_INT_MASK                0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_MASK_1_TX_DLY_INT_SHFT                28
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT1_MASK          0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT1_SHFT          21
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT0_MASK          0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_MASK_1_RX_DONE_DLY_INT0_SHFT          20
#define WDMA_INT_MASK_1_RX_DONE_INT1_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_DONE_INT1_MASK              0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_MASK_1_RX_DONE_INT1_SHFT              17
#define WDMA_INT_MASK_1_RX_DONE_INT0_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_RX_DONE_INT0_MASK              0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_MASK_1_RX_DONE_INT0_SHFT              16
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT3_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT3_MASK          0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT3_SHFT          11
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT2_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT2_MASK          0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT2_SHFT          10
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT1_MASK          0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT1_SHFT          9
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT0_MASK          0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_MASK_1_TX_DONE_DLY_INT0_SHFT          8
#define WDMA_INT_MASK_1_TX_DONE_INT3_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_INT3_MASK              0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_MASK_1_TX_DONE_INT3_SHFT              3
#define WDMA_INT_MASK_1_TX_DONE_INT2_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_INT2_MASK              0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_MASK_1_TX_DONE_INT2_SHFT              2
#define WDMA_INT_MASK_1_TX_DONE_INT1_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_INT1_MASK              0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_MASK_1_TX_DONE_INT1_SHFT              1
#define WDMA_INT_MASK_1_TX_DONE_INT0_ADDR              WDMA_INT_MASK_1_ADDR
#define WDMA_INT_MASK_1_TX_DONE_INT0_MASK              0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_MASK_1_TX_DONE_INT0_SHFT              0

/* =====================================================================================

  ---DMA_INT_STATUS_2 (0x15104800 + 0x02a0)---

    TX_DONE_INT0[0]              - (W1C) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (W1C) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (W1C) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (W1C) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (W1C) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (W1C) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (W1C) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (W1C) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (W1C) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (W1C) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (W1C) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (W1C) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (W1C) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (W1C) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (W1C) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (W1C) RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WDMA_INT_STATUS_2_RX_COHERENT_ADDR             WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_COHERENT_MASK             0x80000000                // RX_COHERENT[31]
#define WDMA_INT_STATUS_2_RX_COHERENT_SHFT             31
#define WDMA_INT_STATUS_2_RX_DLY_INT_ADDR              WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_DLY_INT_MASK              0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_STATUS_2_RX_DLY_INT_SHFT              30
#define WDMA_INT_STATUS_2_TX_COHERENT_ADDR             WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_COHERENT_MASK             0x20000000                // TX_COHERENT[29]
#define WDMA_INT_STATUS_2_TX_COHERENT_SHFT             29
#define WDMA_INT_STATUS_2_TX_DLY_INT_ADDR              WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DLY_INT_MASK              0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_STATUS_2_TX_DLY_INT_SHFT              28
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT1_MASK        0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT1_SHFT        21
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT0_MASK        0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_STATUS_2_RX_DONE_DLY_INT0_SHFT        20
#define WDMA_INT_STATUS_2_RX_DONE_INT1_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_DONE_INT1_MASK            0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_STATUS_2_RX_DONE_INT1_SHFT            17
#define WDMA_INT_STATUS_2_RX_DONE_INT0_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_RX_DONE_INT0_MASK            0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_STATUS_2_RX_DONE_INT0_SHFT            16
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT3_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT3_MASK        0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT3_SHFT        11
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT2_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT2_MASK        0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT2_SHFT        10
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT1_MASK        0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT1_SHFT        9
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT0_MASK        0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_STATUS_2_TX_DONE_DLY_INT0_SHFT        8
#define WDMA_INT_STATUS_2_TX_DONE_INT3_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_INT3_MASK            0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_STATUS_2_TX_DONE_INT3_SHFT            3
#define WDMA_INT_STATUS_2_TX_DONE_INT2_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_INT2_MASK            0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_STATUS_2_TX_DONE_INT2_SHFT            2
#define WDMA_INT_STATUS_2_TX_DONE_INT1_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_INT1_MASK            0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_STATUS_2_TX_DONE_INT1_SHFT            1
#define WDMA_INT_STATUS_2_TX_DONE_INT0_ADDR            WDMA_INT_STATUS_2_ADDR
#define WDMA_INT_STATUS_2_TX_DONE_INT0_MASK            0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_STATUS_2_TX_DONE_INT0_SHFT            0

/* =====================================================================================

  ---DMA_INT_MASK_2 (0x15104800 + 0x02a4)---

    TX_DONE_INT0[0]              - (RW) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RW) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (RW) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (RW) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (RW) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (RW) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (RW) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (RW) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RW) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (RW) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (RW) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (RW) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RW) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (RW) Interrupt enable for TX_DMA data coherent event
    RX_DLY_INT[30]               - (RW) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (RW) Interrupt enable for RX_DMA data coherent event

 =====================================================================================*/
#define WDMA_INT_MASK_2_RX_COHERENT_ADDR               WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_COHERENT_MASK               0x80000000                // RX_COHERENT[31]
#define WDMA_INT_MASK_2_RX_COHERENT_SHFT               31
#define WDMA_INT_MASK_2_RX_DLY_INT_ADDR                WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_DLY_INT_MASK                0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_MASK_2_RX_DLY_INT_SHFT                30
#define WDMA_INT_MASK_2_TX_COHERENT_ADDR               WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_COHERENT_MASK               0x20000000                // TX_COHERENT[29]
#define WDMA_INT_MASK_2_TX_COHERENT_SHFT               29
#define WDMA_INT_MASK_2_TX_DLY_INT_ADDR                WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DLY_INT_MASK                0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_MASK_2_TX_DLY_INT_SHFT                28
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT1_MASK          0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT1_SHFT          21
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT0_MASK          0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_MASK_2_RX_DONE_DLY_INT0_SHFT          20
#define WDMA_INT_MASK_2_RX_DONE_INT1_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_DONE_INT1_MASK              0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_MASK_2_RX_DONE_INT1_SHFT              17
#define WDMA_INT_MASK_2_RX_DONE_INT0_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_RX_DONE_INT0_MASK              0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_MASK_2_RX_DONE_INT0_SHFT              16
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT3_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT3_MASK          0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT3_SHFT          11
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT2_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT2_MASK          0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT2_SHFT          10
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT1_MASK          0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT1_SHFT          9
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT0_MASK          0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_MASK_2_TX_DONE_DLY_INT0_SHFT          8
#define WDMA_INT_MASK_2_TX_DONE_INT3_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_INT3_MASK              0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_MASK_2_TX_DONE_INT3_SHFT              3
#define WDMA_INT_MASK_2_TX_DONE_INT2_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_INT2_MASK              0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_MASK_2_TX_DONE_INT2_SHFT              2
#define WDMA_INT_MASK_2_TX_DONE_INT1_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_INT1_MASK              0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_MASK_2_TX_DONE_INT1_SHFT              1
#define WDMA_INT_MASK_2_TX_DONE_INT0_ADDR              WDMA_INT_MASK_2_ADDR
#define WDMA_INT_MASK_2_TX_DONE_INT0_MASK              0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_MASK_2_TX_DONE_INT0_SHFT              0

/* =====================================================================================

  ---DMA_INT_STATUS_3 (0x15104800 + 0x02a8)---

    TX_DONE_INT0[0]              - (W1C) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (W1C) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (W1C) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (W1C) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (W1C) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (W1C) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (W1C) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (W1C) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (W1C) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (W1C) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (W1C) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (W1C) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (W1C) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (W1C) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (W1C) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (W1C) RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WDMA_INT_STATUS_3_RX_COHERENT_ADDR             WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_COHERENT_MASK             0x80000000                // RX_COHERENT[31]
#define WDMA_INT_STATUS_3_RX_COHERENT_SHFT             31
#define WDMA_INT_STATUS_3_RX_DLY_INT_ADDR              WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_DLY_INT_MASK              0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_STATUS_3_RX_DLY_INT_SHFT              30
#define WDMA_INT_STATUS_3_TX_COHERENT_ADDR             WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_COHERENT_MASK             0x20000000                // TX_COHERENT[29]
#define WDMA_INT_STATUS_3_TX_COHERENT_SHFT             29
#define WDMA_INT_STATUS_3_TX_DLY_INT_ADDR              WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DLY_INT_MASK              0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_STATUS_3_TX_DLY_INT_SHFT              28
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT1_MASK        0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT1_SHFT        21
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT0_MASK        0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_STATUS_3_RX_DONE_DLY_INT0_SHFT        20
#define WDMA_INT_STATUS_3_RX_DONE_INT1_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_DONE_INT1_MASK            0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_STATUS_3_RX_DONE_INT1_SHFT            17
#define WDMA_INT_STATUS_3_RX_DONE_INT0_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_RX_DONE_INT0_MASK            0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_STATUS_3_RX_DONE_INT0_SHFT            16
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT3_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT3_MASK        0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT3_SHFT        11
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT2_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT2_MASK        0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT2_SHFT        10
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT1_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT1_MASK        0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT1_SHFT        9
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT0_ADDR        WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT0_MASK        0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_STATUS_3_TX_DONE_DLY_INT0_SHFT        8
#define WDMA_INT_STATUS_3_TX_DONE_INT3_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_INT3_MASK            0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_STATUS_3_TX_DONE_INT3_SHFT            3
#define WDMA_INT_STATUS_3_TX_DONE_INT2_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_INT2_MASK            0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_STATUS_3_TX_DONE_INT2_SHFT            2
#define WDMA_INT_STATUS_3_TX_DONE_INT1_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_INT1_MASK            0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_STATUS_3_TX_DONE_INT1_SHFT            1
#define WDMA_INT_STATUS_3_TX_DONE_INT0_ADDR            WDMA_INT_STATUS_3_ADDR
#define WDMA_INT_STATUS_3_TX_DONE_INT0_MASK            0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_STATUS_3_TX_DONE_INT0_SHFT            0

/* =====================================================================================

  ---DMA_INT_MASK_3 (0x15104800 + 0x02ac)---

    TX_DONE_INT0[0]              - (RW) TX ring #0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RW) TX ring #1 packet transmit interrupt
    TX_DONE_INT2[2]              - (RW) TX ring #2 packet transmit interrupt
    TX_DONE_INT3[3]              - (RW) TX ring #3 packet transmit interrupt
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_DONE_DLY_INT0[8]          - (RW) TX ring #0 packet transmit delay interrupt
    TX_DONE_DLY_INT1[9]          - (RW) TX ring #1 packet transmit delay interrupt
    TX_DONE_DLY_INT2[10]         - (RW) TX ring #2 packet transmit delay interrupt
    TX_DONE_DLY_INT3[11]         - (RW) TX ring #3 packet transmit delay interrupt
    RESERVED12[15..12]           - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RW) RX ring #0 packet receive interrupt
    RX_DONE_INT1[17]             - (RW) RX ring #1 packet receive interrupt
    RESERVED18[19..18]           - (RO) Reserved bits
    RX_DONE_DLY_INT0[20]         - (RW) RX ring #0 packet receive delay interrupt
    RX_DONE_DLY_INT1[21]         - (RW) RX ring #1 packet receive delay interrupt
    RESERVED22[27..22]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RW) Summary of the whole WDMA TX related interrupts.
    TX_COHERENT[29]              - (RW) Interrupt enable for TX_DMA data coherent event
    RX_DLY_INT[30]               - (RW) Summary of the whole WDMA RX related interrupts.
    RX_COHERENT[31]              - (RW) Interrupt enable for RX_DMA data coherent event

 =====================================================================================*/
#define WDMA_INT_MASK_3_RX_COHERENT_ADDR               WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_COHERENT_MASK               0x80000000                // RX_COHERENT[31]
#define WDMA_INT_MASK_3_RX_COHERENT_SHFT               31
#define WDMA_INT_MASK_3_RX_DLY_INT_ADDR                WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_DLY_INT_MASK                0x40000000                // RX_DLY_INT[30]
#define WDMA_INT_MASK_3_RX_DLY_INT_SHFT                30
#define WDMA_INT_MASK_3_TX_COHERENT_ADDR               WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_COHERENT_MASK               0x20000000                // TX_COHERENT[29]
#define WDMA_INT_MASK_3_TX_COHERENT_SHFT               29
#define WDMA_INT_MASK_3_TX_DLY_INT_ADDR                WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DLY_INT_MASK                0x10000000                // TX_DLY_INT[28]
#define WDMA_INT_MASK_3_TX_DLY_INT_SHFT                28
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT1_MASK          0x00200000                // RX_DONE_DLY_INT1[21]
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT1_SHFT          21
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT0_MASK          0x00100000                // RX_DONE_DLY_INT0[20]
#define WDMA_INT_MASK_3_RX_DONE_DLY_INT0_SHFT          20
#define WDMA_INT_MASK_3_RX_DONE_INT1_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_DONE_INT1_MASK              0x00020000                // RX_DONE_INT1[17]
#define WDMA_INT_MASK_3_RX_DONE_INT1_SHFT              17
#define WDMA_INT_MASK_3_RX_DONE_INT0_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_RX_DONE_INT0_MASK              0x00010000                // RX_DONE_INT0[16]
#define WDMA_INT_MASK_3_RX_DONE_INT0_SHFT              16
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT3_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT3_MASK          0x00000800                // TX_DONE_DLY_INT3[11]
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT3_SHFT          11
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT2_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT2_MASK          0x00000400                // TX_DONE_DLY_INT2[10]
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT2_SHFT          10
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT1_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT1_MASK          0x00000200                // TX_DONE_DLY_INT1[9]
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT1_SHFT          9
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT0_ADDR          WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT0_MASK          0x00000100                // TX_DONE_DLY_INT0[8]
#define WDMA_INT_MASK_3_TX_DONE_DLY_INT0_SHFT          8
#define WDMA_INT_MASK_3_TX_DONE_INT3_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_INT3_MASK              0x00000008                // TX_DONE_INT3[3]
#define WDMA_INT_MASK_3_TX_DONE_INT3_SHFT              3
#define WDMA_INT_MASK_3_TX_DONE_INT2_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_INT2_MASK              0x00000004                // TX_DONE_INT2[2]
#define WDMA_INT_MASK_3_TX_DONE_INT2_SHFT              2
#define WDMA_INT_MASK_3_TX_DONE_INT1_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_INT1_MASK              0x00000002                // TX_DONE_INT1[1]
#define WDMA_INT_MASK_3_TX_DONE_INT1_SHFT              1
#define WDMA_INT_MASK_3_TX_DONE_INT0_ADDR              WDMA_INT_MASK_3_ADDR
#define WDMA_INT_MASK_3_TX_DONE_INT0_MASK              0x00000001                // TX_DONE_INT0[0]
#define WDMA_INT_MASK_3_TX_DONE_INT0_SHFT              0

/* =====================================================================================

  ---DMA_TX_DELAY_INT_CFG_0 (0x15104800 + 0x02b0)---

    RING0_TXMAX_PTIME[7..0]      - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING0_TXMAX_PINT[14..8]      - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING0_TXDLY_INT_EN[15]       - (RW) Enable delay interrupt mechanism for Tx Ring 0
    RING1_TXMAX_PTIME[23..16]    - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING1_TXMAX_PINT[30..24]     - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING1_TXDLY_INT_EN[31]       - (RW) Enable delay interrupt mechanism for Tx Ring 1

 =====================================================================================*/
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXDLY_INT_EN_ADDR WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXDLY_INT_EN_MASK 0x80000000                // RING1_TXDLY_INT_EN[31]
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXDLY_INT_EN_SHFT 31
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PINT_ADDR  WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PINT_MASK  0x7F000000                // RING1_TXMAX_PINT[30..24]
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PINT_SHFT  24
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PTIME_ADDR WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PTIME_MASK 0x00FF0000                // RING1_TXMAX_PTIME[23..16]
#define WDMA_TX_DELAY_INT_CFG_0_RING1_TXMAX_PTIME_SHFT 16
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXDLY_INT_EN_ADDR WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXDLY_INT_EN_MASK 0x00008000                // RING0_TXDLY_INT_EN[15]
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXDLY_INT_EN_SHFT 15
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PINT_ADDR  WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PINT_MASK  0x00007F00                // RING0_TXMAX_PINT[14..8]
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PINT_SHFT  8
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PTIME_ADDR WDMA_TX_DELAY_INT_CFG_0_ADDR
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PTIME_MASK 0x000000FF                // RING0_TXMAX_PTIME[7..0]
#define WDMA_TX_DELAY_INT_CFG_0_RING0_TXMAX_PTIME_SHFT 0

/* =====================================================================================

  ---DMA_TX_DELAY_INT_CFG_1 (0x15104800 + 0x02b4)---

    RING2_TXMAX_PTIME[7..0]      - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING2_TXMAX_PINT[14..8]      - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING2_TXDLY_INT_EN[15]       - (RW) Enable delay interrupt mechanism for Tx Ring 2
    RING3_TXMAX_PTIME[23..16]    - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING3_TXMAX_PINT[30..24]     - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING3_TXDLY_INT_EN[31]       - (RW) Enable delay interrupt mechanism for Tx Ring 3

 =====================================================================================*/
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXDLY_INT_EN_ADDR WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXDLY_INT_EN_MASK 0x80000000                // RING3_TXDLY_INT_EN[31]
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXDLY_INT_EN_SHFT 31
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PINT_ADDR  WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PINT_MASK  0x7F000000                // RING3_TXMAX_PINT[30..24]
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PINT_SHFT  24
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PTIME_ADDR WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PTIME_MASK 0x00FF0000                // RING3_TXMAX_PTIME[23..16]
#define WDMA_TX_DELAY_INT_CFG_1_RING3_TXMAX_PTIME_SHFT 16
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXDLY_INT_EN_ADDR WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXDLY_INT_EN_MASK 0x00008000                // RING2_TXDLY_INT_EN[15]
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXDLY_INT_EN_SHFT 15
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PINT_ADDR  WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PINT_MASK  0x00007F00                // RING2_TXMAX_PINT[14..8]
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PINT_SHFT  8
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PTIME_ADDR WDMA_TX_DELAY_INT_CFG_1_ADDR
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PTIME_MASK 0x000000FF                // RING2_TXMAX_PTIME[7..0]
#define WDMA_TX_DELAY_INT_CFG_1_RING2_TXMAX_PTIME_SHFT 0

/* =====================================================================================

  ---DMA_RX_DELAY_INT_CFG_0 (0x15104800 + 0x02c0)---

    RING0_RXMAX_PTIME[7..0]      - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING0_RXMAX_PINT[14..8]      - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING0_RXDLY_INT_EN[15]       - (RW) Enable delay interrupt mechanism for Rx Ring 0
    RING1_RXMAX_PTIME[23..16]    - (RW) Specified Max.  pended time
                                     [Note] reset to 0 can disable pending interrupt time check.
    RING1_RXMAX_PINT[30..24]     - (RW) Specified Max. number of pended interrupts
                                     [Note] reset to 0 can disable pending interrupt count check.
    RING1_RXDLY_INT_EN[31]       - (RW) Enable delay interrupt mechanism for Rx Ring 1

 =====================================================================================*/
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXDLY_INT_EN_ADDR WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXDLY_INT_EN_MASK 0x80000000                // RING1_RXDLY_INT_EN[31]
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXDLY_INT_EN_SHFT 31
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PINT_ADDR  WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PINT_MASK  0x7F000000                // RING1_RXMAX_PINT[30..24]
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PINT_SHFT  24
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PTIME_ADDR WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PTIME_MASK 0x00FF0000                // RING1_RXMAX_PTIME[23..16]
#define WDMA_RX_DELAY_INT_CFG_0_RING1_RXMAX_PTIME_SHFT 16
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXDLY_INT_EN_ADDR WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXDLY_INT_EN_MASK 0x00008000                // RING0_RXDLY_INT_EN[15]
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXDLY_INT_EN_SHFT 15
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PINT_ADDR  WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PINT_MASK  0x00007F00                // RING0_RXMAX_PINT[14..8]
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PINT_SHFT  8
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PTIME_ADDR WDMA_RX_DELAY_INT_CFG_0_ADDR
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PTIME_MASK 0x000000FF                // RING0_RXMAX_PTIME[7..0]
#define WDMA_RX_DELAY_INT_CFG_0_RING0_RXMAX_PTIME_SHFT 0

/* =====================================================================================

  ---DMA_RX_FC_CFG_0 (0x15104800 + 0x02c8)---

    RING0_FC_ASRT_THRES[11..0]   - (RW) Specified PFC assert threshold.
                                     Enable PFC when free DMAD count < threshold.
                                     Threshold = 0 means PFC disabled
    RESERVED12[15..12]           - (RO) Reserved bits
    RING0_FC_DASRT_THRES[27..16] - (RW) Specified PFC deassert threshold.
                                     Disable PFC when free DMAD count > threshold.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_FC_CFG_0_RING0_FC_DASRT_THRES_ADDR     WDMA_RX_FC_CFG_0_ADDR
#define WDMA_RX_FC_CFG_0_RING0_FC_DASRT_THRES_MASK     0x0FFF0000                // RING0_FC_DASRT_THRES[27..16]
#define WDMA_RX_FC_CFG_0_RING0_FC_DASRT_THRES_SHFT     16
#define WDMA_RX_FC_CFG_0_RING0_FC_ASRT_THRES_ADDR      WDMA_RX_FC_CFG_0_ADDR
#define WDMA_RX_FC_CFG_0_RING0_FC_ASRT_THRES_MASK      0x00000FFF                // RING0_FC_ASRT_THRES[11..0]
#define WDMA_RX_FC_CFG_0_RING0_FC_ASRT_THRES_SHFT      0

/* =====================================================================================

  ---DMA_RX_FC_CFG_1 (0x15104800 + 0x02cc)---

    RING1_FC_ASRT_THRES[11..0]   - (RW) Specified PFC assert threshold.
                                     Enable PFC when free DMAD count < threshold.
                                     Threshold = 0 means PFC disabled.
    RESERVED12[15..12]           - (RO) Reserved bits
    RING1_FC_DASRT_THRES[27..16] - (RW) Specified PFC deassert threshold.
                                     Disable PFC when free DMAD count > threshold.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_FC_CFG_1_RING1_FC_DASRT_THRES_ADDR     WDMA_RX_FC_CFG_1_ADDR
#define WDMA_RX_FC_CFG_1_RING1_FC_DASRT_THRES_MASK     0x0FFF0000                // RING1_FC_DASRT_THRES[27..16]
#define WDMA_RX_FC_CFG_1_RING1_FC_DASRT_THRES_SHFT     16
#define WDMA_RX_FC_CFG_1_RING1_FC_ASRT_THRES_ADDR      WDMA_RX_FC_CFG_1_ADDR
#define WDMA_RX_FC_CFG_1_RING1_FC_ASRT_THRES_MASK      0x00000FFF                // RING1_FC_ASRT_THRES[11..0]
#define WDMA_RX_FC_CFG_1_RING1_FC_ASRT_THRES_SHFT      0

/* =====================================================================================

  ---DMA_PREF_TX_CFG (0x15104800 + 0x02d0)---

    PREF_EN[0]                   - (RW) Prefetch Enable
    BUSY[1]                      - (RO) Prefetch agent busy
    DMAD_SIZE[2]                 - (RW) Size of each DMAD
    DDONE_CHK[3]                 - (RW) Enable DDONE Check.
                                     The DMAD with wrong DDONE and all DMAD afterward (in same burst) will be dropped. Prefetch agent will retry to prefetch these DMAD later.
    DDONE_POLARITY[4]            - (RW) DDONE polarity of DDONE check
    WR_BND_4KB_BST[5]            - (RW) Enable AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RD_BND_4KB_BST[6]            - (RW) Enable AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    AXI_RRESP_ERR[7]             - (W1C) Prefetch agent AXI Read Respond Error
    BURST_SIZE[12..8]            - (RW) Prefetch Bus Burst Size
                                     Bus burst size. Unit is 8bytes. Maximum value is 16 (128bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (128bytes) or less than size of 1 DMAD.
    CURR_STATE[15..13]           - (RO) Prefetch agent FSM
    LOW_THRES[21..16]            - (RW) Prefetch FIFO Low Threshold.
                                     Ring which data byte in prefetch FIFO under low threshold has higher priority than others.
                                     Unit is 8bytes. Maximum is 32 (256bytes). Set the register to 0 will disable the low threshold mechanism. Recommend low threshold is size of 1 or 2 DMAD.
    AXI_ULTRA[23..22]            - (RW) AXI ULTRA for AW/AR
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_PREF_TX_CFG_AXI_ULTRA_ADDR                WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_AXI_ULTRA_MASK                0x00C00000                // AXI_ULTRA[23..22]
#define WDMA_PREF_TX_CFG_AXI_ULTRA_SHFT                22
#define WDMA_PREF_TX_CFG_LOW_THRES_ADDR                WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_LOW_THRES_MASK                0x003F0000                // LOW_THRES[21..16]
#define WDMA_PREF_TX_CFG_LOW_THRES_SHFT                16
#define WDMA_PREF_TX_CFG_CURR_STATE_ADDR               WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_CURR_STATE_MASK               0x0000E000                // CURR_STATE[15..13]
#define WDMA_PREF_TX_CFG_CURR_STATE_SHFT               13
#define WDMA_PREF_TX_CFG_BURST_SIZE_ADDR               WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_BURST_SIZE_MASK               0x00001F00                // BURST_SIZE[12..8]
#define WDMA_PREF_TX_CFG_BURST_SIZE_SHFT               8
#define WDMA_PREF_TX_CFG_AXI_RRESP_ERR_ADDR            WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_AXI_RRESP_ERR_MASK            0x00000080                // AXI_RRESP_ERR[7]
#define WDMA_PREF_TX_CFG_AXI_RRESP_ERR_SHFT            7
#define WDMA_PREF_TX_CFG_RD_BND_4KB_BST_ADDR           WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_RD_BND_4KB_BST_MASK           0x00000040                // RD_BND_4KB_BST[6]
#define WDMA_PREF_TX_CFG_RD_BND_4KB_BST_SHFT           6
#define WDMA_PREF_TX_CFG_WR_BND_4KB_BST_ADDR           WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_WR_BND_4KB_BST_MASK           0x00000020                // WR_BND_4KB_BST[5]
#define WDMA_PREF_TX_CFG_WR_BND_4KB_BST_SHFT           5
#define WDMA_PREF_TX_CFG_DDONE_POLARITY_ADDR           WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_DDONE_POLARITY_MASK           0x00000010                // DDONE_POLARITY[4]
#define WDMA_PREF_TX_CFG_DDONE_POLARITY_SHFT           4
#define WDMA_PREF_TX_CFG_DDONE_CHK_ADDR                WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_DDONE_CHK_MASK                0x00000008                // DDONE_CHK[3]
#define WDMA_PREF_TX_CFG_DDONE_CHK_SHFT                3
#define WDMA_PREF_TX_CFG_DMAD_SIZE_ADDR                WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_DMAD_SIZE_MASK                0x00000004                // DMAD_SIZE[2]
#define WDMA_PREF_TX_CFG_DMAD_SIZE_SHFT                2
#define WDMA_PREF_TX_CFG_BUSY_ADDR                     WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_BUSY_MASK                     0x00000002                // BUSY[1]
#define WDMA_PREF_TX_CFG_BUSY_SHFT                     1
#define WDMA_PREF_TX_CFG_PREF_EN_ADDR                  WDMA_PREF_TX_CFG_ADDR
#define WDMA_PREF_TX_CFG_PREF_EN_MASK                  0x00000001                // PREF_EN[0]
#define WDMA_PREF_TX_CFG_PREF_EN_SHFT                  0

/* =====================================================================================

  ---DMA_PREF_TX_FIFO_CFG0 (0x15104800 + 0x02d4)---

    RING0_CLEAR[0]               - (RW) Clear Ring0 FIFO
    RING0_FULL[1]                - (RO) Ring0 FIFO Full
    RING0_EMPTY[2]               - (RO) Ring0 FIFO Empty
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[9..4]         - (RO) Ring0 FIFO Used entry
    RING0_FREE_CNT[15..10]       - (RO) Ring0 FIFO Free entry
    RING1_CLEAR[16]              - (RW) Clear Ring1 FIFO
    RING1_FULL[17]               - (RO) Ring1 FIFO Full
    RING1_EMPTY[18]              - (RO) Ring1 FIFO Empty
    RESERVED19[19]               - (RO) Reserved bits
    RING1_USED_CNT[25..20]       - (RO) Ring1 FIFO Used entry
    RING1_FREE_CNT[31..26]       - (RO) Ring1 FIFO Free entry

 =====================================================================================*/
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FREE_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FREE_CNT_MASK     0xFC000000                // RING1_FREE_CNT[31..26]
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FREE_CNT_SHFT     26
#define WDMA_PREF_TX_FIFO_CFG0_RING1_USED_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING1_USED_CNT_MASK     0x03F00000                // RING1_USED_CNT[25..20]
#define WDMA_PREF_TX_FIFO_CFG0_RING1_USED_CNT_SHFT     20
#define WDMA_PREF_TX_FIFO_CFG0_RING1_EMPTY_ADDR        WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING1_EMPTY_MASK        0x00040000                // RING1_EMPTY[18]
#define WDMA_PREF_TX_FIFO_CFG0_RING1_EMPTY_SHFT        18
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FULL_ADDR         WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FULL_MASK         0x00020000                // RING1_FULL[17]
#define WDMA_PREF_TX_FIFO_CFG0_RING1_FULL_SHFT         17
#define WDMA_PREF_TX_FIFO_CFG0_RING1_CLEAR_ADDR        WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING1_CLEAR_MASK        0x00010000                // RING1_CLEAR[16]
#define WDMA_PREF_TX_FIFO_CFG0_RING1_CLEAR_SHFT        16
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FREE_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FREE_CNT_MASK     0x0000FC00                // RING0_FREE_CNT[15..10]
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FREE_CNT_SHFT     10
#define WDMA_PREF_TX_FIFO_CFG0_RING0_USED_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING0_USED_CNT_MASK     0x000003F0                // RING0_USED_CNT[9..4]
#define WDMA_PREF_TX_FIFO_CFG0_RING0_USED_CNT_SHFT     4
#define WDMA_PREF_TX_FIFO_CFG0_RING0_EMPTY_ADDR        WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING0_EMPTY_MASK        0x00000004                // RING0_EMPTY[2]
#define WDMA_PREF_TX_FIFO_CFG0_RING0_EMPTY_SHFT        2
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FULL_ADDR         WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FULL_MASK         0x00000002                // RING0_FULL[1]
#define WDMA_PREF_TX_FIFO_CFG0_RING0_FULL_SHFT         1
#define WDMA_PREF_TX_FIFO_CFG0_RING0_CLEAR_ADDR        WDMA_PREF_TX_FIFO_CFG0_ADDR
#define WDMA_PREF_TX_FIFO_CFG0_RING0_CLEAR_MASK        0x00000001                // RING0_CLEAR[0]
#define WDMA_PREF_TX_FIFO_CFG0_RING0_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_PREF_TX_FIFO_CFG1 (0x15104800 + 0x02d8)---

    RING2_CLEAR[0]               - (RW) Clear Ring2 FIFO
    RING2_FULL[1]                - (RO) Ring2 FIFO Full
    RING2_EMPTY[2]               - (RO) Ring2 FIFO Empty
    RESERVED3[3]                 - (RO) Reserved bits
    RING2_USED_CNT[9..4]         - (RO) Ring2 FIFO Used entry
    RING2_FREE_CNT[15..10]       - (RO) Ring2 FIFO Free entry
    RING3_CLEAR[16]              - (RW) Clear Ring3 FIFO
    RING3_FULL[17]               - (RO) Ring3 FIFO Full
    RING3_EMPTY[18]              - (RO) Ring3 FIFO Empty
    RESERVED19[19]               - (RO) Reserved bits
    RING3_USED_CNT[25..20]       - (RO) Ring3 FIFO Used entry
    RING3_FREE_CNT[31..26]       - (RO) Ring3 FIFO Free entry

 =====================================================================================*/
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FREE_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FREE_CNT_MASK     0xFC000000                // RING3_FREE_CNT[31..26]
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FREE_CNT_SHFT     26
#define WDMA_PREF_TX_FIFO_CFG1_RING3_USED_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING3_USED_CNT_MASK     0x03F00000                // RING3_USED_CNT[25..20]
#define WDMA_PREF_TX_FIFO_CFG1_RING3_USED_CNT_SHFT     20
#define WDMA_PREF_TX_FIFO_CFG1_RING3_EMPTY_ADDR        WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING3_EMPTY_MASK        0x00040000                // RING3_EMPTY[18]
#define WDMA_PREF_TX_FIFO_CFG1_RING3_EMPTY_SHFT        18
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FULL_ADDR         WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FULL_MASK         0x00020000                // RING3_FULL[17]
#define WDMA_PREF_TX_FIFO_CFG1_RING3_FULL_SHFT         17
#define WDMA_PREF_TX_FIFO_CFG1_RING3_CLEAR_ADDR        WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING3_CLEAR_MASK        0x00010000                // RING3_CLEAR[16]
#define WDMA_PREF_TX_FIFO_CFG1_RING3_CLEAR_SHFT        16
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FREE_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FREE_CNT_MASK     0x0000FC00                // RING2_FREE_CNT[15..10]
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FREE_CNT_SHFT     10
#define WDMA_PREF_TX_FIFO_CFG1_RING2_USED_CNT_ADDR     WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING2_USED_CNT_MASK     0x000003F0                // RING2_USED_CNT[9..4]
#define WDMA_PREF_TX_FIFO_CFG1_RING2_USED_CNT_SHFT     4
#define WDMA_PREF_TX_FIFO_CFG1_RING2_EMPTY_ADDR        WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING2_EMPTY_MASK        0x00000004                // RING2_EMPTY[2]
#define WDMA_PREF_TX_FIFO_CFG1_RING2_EMPTY_SHFT        2
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FULL_ADDR         WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FULL_MASK         0x00000002                // RING2_FULL[1]
#define WDMA_PREF_TX_FIFO_CFG1_RING2_FULL_SHFT         1
#define WDMA_PREF_TX_FIFO_CFG1_RING2_CLEAR_ADDR        WDMA_PREF_TX_FIFO_CFG1_ADDR
#define WDMA_PREF_TX_FIFO_CFG1_RING2_CLEAR_MASK        0x00000001                // RING2_CLEAR[0]
#define WDMA_PREF_TX_FIFO_CFG1_RING2_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_PREF_RX_CFG (0x15104800 + 0x02dc)---

    PREF_EN[0]                   - (RW) Prefetch Enable
    BUSY[1]                      - (RO) Prefetch agent busy
    DMAD_SIZE[2]                 - (RW) Size of each DMAD
    DDONE_CHK[3]                 - (RW) Enable DDONE Check.
                                     The DMAD with wrong DDONE and all DMAD afterward (in same burst) will be dropped. Prefetch agent will retry to prefetch these DMAD later.
    DDONE_POLARITY[4]            - (RW) DDONE polarity of DDONE check
    WR_BND_4KB_BST[5]            - (RW) Enable AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RD_BND_4KB_BST[6]            - (RW) Enable AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    AXI_RRESP_ERR[7]             - (W1C) Prefetch agent AXI Read Respond Error
    BURST_SIZE[12..8]            - (RW) Prefetch Bus Burst Size
                                     Bus burst size. Unit is 8bytes. Maximum value is 16 (128bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (128bytes) or less than size of 1 DMAD.
    CURR_STATE[15..13]           - (RO) Prefetch agent FSM
    LOW_THRES[21..16]            - (RW) Prefetch FIFO Low Threshold.
                                     Ring which data byte in prefetch FIFO under low threshold has higher priority than others.
                                     Unit is 8bytes. Maximum is 32 (256bytes). Set the register to 0 will disable the low threshold mechanism. Recommend low threshold is size of 1 or 2 DMAD.
    AXI_ULTRA[23..22]            - (RW) AXI ULTRA for AW/AR
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_PREF_RX_CFG_AXI_ULTRA_ADDR                WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_AXI_ULTRA_MASK                0x00C00000                // AXI_ULTRA[23..22]
#define WDMA_PREF_RX_CFG_AXI_ULTRA_SHFT                22
#define WDMA_PREF_RX_CFG_LOW_THRES_ADDR                WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_LOW_THRES_MASK                0x003F0000                // LOW_THRES[21..16]
#define WDMA_PREF_RX_CFG_LOW_THRES_SHFT                16
#define WDMA_PREF_RX_CFG_CURR_STATE_ADDR               WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_CURR_STATE_MASK               0x0000E000                // CURR_STATE[15..13]
#define WDMA_PREF_RX_CFG_CURR_STATE_SHFT               13
#define WDMA_PREF_RX_CFG_BURST_SIZE_ADDR               WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_BURST_SIZE_MASK               0x00001F00                // BURST_SIZE[12..8]
#define WDMA_PREF_RX_CFG_BURST_SIZE_SHFT               8
#define WDMA_PREF_RX_CFG_AXI_RRESP_ERR_ADDR            WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_AXI_RRESP_ERR_MASK            0x00000080                // AXI_RRESP_ERR[7]
#define WDMA_PREF_RX_CFG_AXI_RRESP_ERR_SHFT            7
#define WDMA_PREF_RX_CFG_RD_BND_4KB_BST_ADDR           WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_RD_BND_4KB_BST_MASK           0x00000040                // RD_BND_4KB_BST[6]
#define WDMA_PREF_RX_CFG_RD_BND_4KB_BST_SHFT           6
#define WDMA_PREF_RX_CFG_WR_BND_4KB_BST_ADDR           WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_WR_BND_4KB_BST_MASK           0x00000020                // WR_BND_4KB_BST[5]
#define WDMA_PREF_RX_CFG_WR_BND_4KB_BST_SHFT           5
#define WDMA_PREF_RX_CFG_DDONE_POLARITY_ADDR           WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_DDONE_POLARITY_MASK           0x00000010                // DDONE_POLARITY[4]
#define WDMA_PREF_RX_CFG_DDONE_POLARITY_SHFT           4
#define WDMA_PREF_RX_CFG_DDONE_CHK_ADDR                WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_DDONE_CHK_MASK                0x00000008                // DDONE_CHK[3]
#define WDMA_PREF_RX_CFG_DDONE_CHK_SHFT                3
#define WDMA_PREF_RX_CFG_DMAD_SIZE_ADDR                WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_DMAD_SIZE_MASK                0x00000004                // DMAD_SIZE[2]
#define WDMA_PREF_RX_CFG_DMAD_SIZE_SHFT                2
#define WDMA_PREF_RX_CFG_BUSY_ADDR                     WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_BUSY_MASK                     0x00000002                // BUSY[1]
#define WDMA_PREF_RX_CFG_BUSY_SHFT                     1
#define WDMA_PREF_RX_CFG_PREF_EN_ADDR                  WDMA_PREF_RX_CFG_ADDR
#define WDMA_PREF_RX_CFG_PREF_EN_MASK                  0x00000001                // PREF_EN[0]
#define WDMA_PREF_RX_CFG_PREF_EN_SHFT                  0

/* =====================================================================================

  ---DMA_PREF_RX_FIFO_CFG0 (0x15104800 + 0x02e0)---

    RING0_CLEAR[0]               - (RW) Clear Ring0 FIFO
    RING0_FULL[1]                - (RO) Ring0 FIFO Full
    RING0_EMPTY[2]               - (RO) Ring0 FIFO Empty
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[9..4]         - (RO) Ring0 FIFO Used entry
    RING0_FREE_CNT[15..10]       - (RO) Ring0 FIFO Free entry
    RING1_CLEAR[16]              - (RW) Clear Ring1 FIFO
    RING1_FULL[17]               - (RO) Ring1 FIFO Full
    RING1_EMPTY[18]              - (RO) Ring1 FIFO Empty
    RESERVED19[19]               - (RO) Reserved bits
    RING1_USED_CNT[25..20]       - (RO) Ring1 FIFO Used entry
    RING1_FREE_CNT[31..26]       - (RO) Ring1 FIFO Free entry

 =====================================================================================*/
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FREE_CNT_ADDR     WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FREE_CNT_MASK     0xFC000000                // RING1_FREE_CNT[31..26]
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FREE_CNT_SHFT     26
#define WDMA_PREF_RX_FIFO_CFG0_RING1_USED_CNT_ADDR     WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING1_USED_CNT_MASK     0x03F00000                // RING1_USED_CNT[25..20]
#define WDMA_PREF_RX_FIFO_CFG0_RING1_USED_CNT_SHFT     20
#define WDMA_PREF_RX_FIFO_CFG0_RING1_EMPTY_ADDR        WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING1_EMPTY_MASK        0x00040000                // RING1_EMPTY[18]
#define WDMA_PREF_RX_FIFO_CFG0_RING1_EMPTY_SHFT        18
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FULL_ADDR         WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FULL_MASK         0x00020000                // RING1_FULL[17]
#define WDMA_PREF_RX_FIFO_CFG0_RING1_FULL_SHFT         17
#define WDMA_PREF_RX_FIFO_CFG0_RING1_CLEAR_ADDR        WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING1_CLEAR_MASK        0x00010000                // RING1_CLEAR[16]
#define WDMA_PREF_RX_FIFO_CFG0_RING1_CLEAR_SHFT        16
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FREE_CNT_ADDR     WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FREE_CNT_MASK     0x0000FC00                // RING0_FREE_CNT[15..10]
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FREE_CNT_SHFT     10
#define WDMA_PREF_RX_FIFO_CFG0_RING0_USED_CNT_ADDR     WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING0_USED_CNT_MASK     0x000003F0                // RING0_USED_CNT[9..4]
#define WDMA_PREF_RX_FIFO_CFG0_RING0_USED_CNT_SHFT     4
#define WDMA_PREF_RX_FIFO_CFG0_RING0_EMPTY_ADDR        WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING0_EMPTY_MASK        0x00000004                // RING0_EMPTY[2]
#define WDMA_PREF_RX_FIFO_CFG0_RING0_EMPTY_SHFT        2
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FULL_ADDR         WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FULL_MASK         0x00000002                // RING0_FULL[1]
#define WDMA_PREF_RX_FIFO_CFG0_RING0_FULL_SHFT         1
#define WDMA_PREF_RX_FIFO_CFG0_RING0_CLEAR_ADDR        WDMA_PREF_RX_FIFO_CFG0_ADDR
#define WDMA_PREF_RX_FIFO_CFG0_RING0_CLEAR_MASK        0x00000001                // RING0_CLEAR[0]
#define WDMA_PREF_RX_FIFO_CFG0_RING0_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_PREF_SIDX_CFG (0x15104800 + 0x02e4)---

    TX_RING0_SIDX_CLR[0]         - (RW) Clear the index
    TX_RING1_SIDX_CLR[1]         - (RW) Clear the index
    TX_RING2_SIDX_CLR[2]         - (RW) Clear the index
    TX_RING3_SIDX_CLR[3]         - (RW) Clear the index
    RX_RING0_SIDX_CLR[4]         - (RW) Clear the index
    RX_RING1_SIDX_CLR[5]         - (RW) Clear the index
    RESERVED6[7..6]              - (RO) Reserved bits
    TX_RING0_SIDX_OW[8]          - (RW) Overwrite the index
    TX_RING1_SIDX_OW[9]          - (RW) Overwrite the index
    TX_RING2_SIDX_OW[10]         - (RW) Overwrite the index
    TX_RING3_SIDX_OW[11]         - (RW) Overwrite the index
    RX_RING0_SIDX_OW[12]         - (RW) Overwrite the index
    RX_RING1_SIDX_OW[13]         - (RW) Overwrite the index
    RESERVED14[15..14]           - (RO) Reserved bits
    MON_SEL[18..16]              - (RW) Select start index show in monitor
    RESERVED19[31..19]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_PREF_SIDX_CFG_MON_SEL_ADDR                WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_MON_SEL_MASK                0x00070000                // MON_SEL[18..16]
#define WDMA_PREF_SIDX_CFG_MON_SEL_SHFT                16
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_OW_MASK       0x00002000                // RX_RING1_SIDX_OW[13]
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_OW_SHFT       13
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_OW_MASK       0x00001000                // RX_RING0_SIDX_OW[12]
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_OW_SHFT       12
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_OW_MASK       0x00000800                // TX_RING3_SIDX_OW[11]
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_OW_SHFT       11
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_OW_MASK       0x00000400                // TX_RING2_SIDX_OW[10]
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_OW_SHFT       10
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_OW_MASK       0x00000200                // TX_RING1_SIDX_OW[9]
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_OW_SHFT       9
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_OW_ADDR       WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_OW_MASK       0x00000100                // TX_RING0_SIDX_OW[8]
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_OW_SHFT       8
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_CLR_MASK      0x00000020                // RX_RING1_SIDX_CLR[5]
#define WDMA_PREF_SIDX_CFG_RX_RING1_SIDX_CLR_SHFT      5
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_CLR_MASK      0x00000010                // RX_RING0_SIDX_CLR[4]
#define WDMA_PREF_SIDX_CFG_RX_RING0_SIDX_CLR_SHFT      4
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_CLR_MASK      0x00000008                // TX_RING3_SIDX_CLR[3]
#define WDMA_PREF_SIDX_CFG_TX_RING3_SIDX_CLR_SHFT      3
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_CLR_MASK      0x00000004                // TX_RING2_SIDX_CLR[2]
#define WDMA_PREF_SIDX_CFG_TX_RING2_SIDX_CLR_SHFT      2
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_CLR_MASK      0x00000002                // TX_RING1_SIDX_CLR[1]
#define WDMA_PREF_SIDX_CFG_TX_RING1_SIDX_CLR_SHFT      1
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_CLR_ADDR      WDMA_PREF_SIDX_CFG_ADDR
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_CLR_MASK      0x00000001                // TX_RING0_SIDX_CLR[0]
#define WDMA_PREF_SIDX_CFG_TX_RING0_SIDX_CLR_SHFT      0

/* =====================================================================================

  ---DMA_PREF_SIDX_MON (0x15104800 + 0x02e8)---

    START_IDX[15..0]             - (RO) Point to the next DMAD that prefetch agent wants to read
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_PREF_SIDX_MON_START_IDX_ADDR              WDMA_PREF_SIDX_MON_ADDR
#define WDMA_PREF_SIDX_MON_START_IDX_MASK              0x0000FFFF                // START_IDX[15..0]
#define WDMA_PREF_SIDX_MON_START_IDX_SHFT              0

/* =====================================================================================

  ---DMA_PREF_SIDX_OW (0x15104800 + 0x02ec)---

    START_IDX_OW_VAL[15..0]      - (RW) Overwrite the index with this register field
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_PREF_SIDX_OW_START_IDX_OW_VAL_ADDR        WDMA_PREF_SIDX_OW_ADDR
#define WDMA_PREF_SIDX_OW_START_IDX_OW_VAL_MASK        0x0000FFFF                // START_IDX_OW_VAL[15..0]
#define WDMA_PREF_SIDX_OW_START_IDX_OW_VAL_SHFT        0

/* =====================================================================================

  ---DMA_XFER_CNT_CFG (0x15104800 + 0x02f0)---

    RESERVED0[3..0]              - (RO) Reserved bits
    BYTE_CNT_UNIT[6..4]          - (RW) Byte Counter Unit
                                     Unit is 2^CNT_UNIT.
                                     Set to 0 means unit = 1 Byte; 1 means unit = 2 Byte; 2 means unit = 4 Byte …
    CLR_CTRL[7]                  - (RW) Monitor Clear Mode
    BYTE_CNT_SAT[8]              - (RW) Packet Counter Saturation Mode
    PKT_CNT_SAT[9]               - (RW) Byte Counter Saturation Mode
    RESERVED10[27..10]           - (RO) Reserved bits
    MON_SEL[31..28]              - (RW) Select monitor

 =====================================================================================*/
#define WDMA_XFER_CNT_CFG_MON_SEL_ADDR                 WDMA_XFER_CNT_CFG_ADDR
#define WDMA_XFER_CNT_CFG_MON_SEL_MASK                 0xF0000000                // MON_SEL[31..28]
#define WDMA_XFER_CNT_CFG_MON_SEL_SHFT                 28
#define WDMA_XFER_CNT_CFG_PKT_CNT_SAT_ADDR             WDMA_XFER_CNT_CFG_ADDR
#define WDMA_XFER_CNT_CFG_PKT_CNT_SAT_MASK             0x00000200                // PKT_CNT_SAT[9]
#define WDMA_XFER_CNT_CFG_PKT_CNT_SAT_SHFT             9
#define WDMA_XFER_CNT_CFG_BYTE_CNT_SAT_ADDR            WDMA_XFER_CNT_CFG_ADDR
#define WDMA_XFER_CNT_CFG_BYTE_CNT_SAT_MASK            0x00000100                // BYTE_CNT_SAT[8]
#define WDMA_XFER_CNT_CFG_BYTE_CNT_SAT_SHFT            8
#define WDMA_XFER_CNT_CFG_CLR_CTRL_ADDR                WDMA_XFER_CNT_CFG_ADDR
#define WDMA_XFER_CNT_CFG_CLR_CTRL_MASK                0x00000080                // CLR_CTRL[7]
#define WDMA_XFER_CNT_CFG_CLR_CTRL_SHFT                7
#define WDMA_XFER_CNT_CFG_BYTE_CNT_UNIT_ADDR           WDMA_XFER_CNT_CFG_ADDR
#define WDMA_XFER_CNT_CFG_BYTE_CNT_UNIT_MASK           0x00000070                // BYTE_CNT_UNIT[6..4]
#define WDMA_XFER_CNT_CFG_BYTE_CNT_UNIT_SHFT           4

/* =====================================================================================

  ---DMA_XFER_CNT_MON0 (0x15104800 + 0x02f4)---

    MON[31..0]                   - (RU) DMA Ring0 Transfer Counter

 =====================================================================================*/
#define WDMA_XFER_CNT_MON0_MON_ADDR                    WDMA_XFER_CNT_MON0_ADDR
#define WDMA_XFER_CNT_MON0_MON_MASK                    0xFFFFFFFF                // MON[31..0]
#define WDMA_XFER_CNT_MON0_MON_SHFT                    0

/* =====================================================================================

  ---DMA_XFER_CNT_MON1 (0x15104800 + 0x02f8)---

    MON[31..0]                   - (RU) DMA Ring1 Transfer Counter

 =====================================================================================*/
#define WDMA_XFER_CNT_MON1_MON_ADDR                    WDMA_XFER_CNT_MON1_ADDR
#define WDMA_XFER_CNT_MON1_MON_MASK                    0xFFFFFFFF                // MON[31..0]
#define WDMA_XFER_CNT_MON1_MON_SHFT                    0

/* =====================================================================================

  ---DMA_RX_ULTRA_CFG (0x15104800 + 0x02fc)---

    AXI_PREULTRA_THRES[10..0]    - (RW) AXI Pre-ultra threshold.
                                     If CDM RX write FIFO count bigger than AXI_PREULTRA_THRES, will trigger AXI Pre-ultra transaction when AXI_PREULTRA_EN = 1.
    RESERVED11[14..11]           - (RO) Reserved bits
    AXI_PREULTRA_EN[15]          - (RW) Enable AXI Ultra transaction for RXDMA burst control which issuing AW
    AXI_ULTRA_THRES[26..16]      - (RW) AXI Ultra threshold.
                                     If CDM RX write FIFO count bigger than AXI_ULTRA_THRES, will trigger AXI Ultra transaction when AXI_ULTRA_EN = 1.
    RESERVED27[30..27]           - (RO) Reserved bits
    AXI_ULTRA_EN[31]             - (RW) Enable AXI Ultra transaction for RXDMA burst control which issuing AW

 =====================================================================================*/
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_EN_ADDR            WDMA_RX_ULTRA_CFG_ADDR
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_EN_MASK            0x80000000                // AXI_ULTRA_EN[31]
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_EN_SHFT            31
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_THRES_ADDR         WDMA_RX_ULTRA_CFG_ADDR
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_THRES_MASK         0x07FF0000                // AXI_ULTRA_THRES[26..16]
#define WDMA_RX_ULTRA_CFG_AXI_ULTRA_THRES_SHFT         16
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_EN_ADDR         WDMA_RX_ULTRA_CFG_ADDR
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_EN_MASK         0x00008000                // AXI_PREULTRA_EN[15]
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_EN_SHFT         15
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_THRES_ADDR      WDMA_RX_ULTRA_CFG_ADDR
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_THRES_MASK      0x000007FF                // AXI_PREULTRA_THRES[10..0]
#define WDMA_RX_ULTRA_CFG_AXI_PREULTRA_THRES_SHFT      0

/* =====================================================================================

  ---DMA_WRBK_TX_CFG (0x15104800 + 0x0300)---

    BUSY[0]                      - (RO) Writeback agent busy
    DMAD_SIZE[1]                 - (RW) Size of each DMAD
    WR_BND_4KB_BST[2]            - (RW) Enable write transaction across 4kb boundary
    RD_BND_4KB_BST[3]            - (RW) Enable read transaction across 4kb boundary
    AXI_ULTRA[5..4]              - (RW) AXI ULTRA for writeback AW
    BURST_SIZE[10..6]            - (RW) Writeback Bus Burst Size
                                     Bus burst size. Unit is 128bits(16bytes). Maximum value is 16 (256bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (256bytes) or less than size of 1 DMAD.
    CURR_STATE[13..11]           - (RO) Writeback agent FSM
    WRBK_THRES[19..14]           - (RW) Writeback threshold, bit width is related to WRBK FIFO depth
                                     The minimum vaule is 2 which means 1 DMAD
                                     note : value 0 is unavailable
    RESERVED20[20]               - (RO) Reserved bits
    FLUSH_TIMER_EN[21]           - (RW) Enable writeback flush timer
    MAX_PENDING_TIME[29..22]     - (RW) Flush timer asserts interrupt while reaching max pending time
                                     If sets MAX_PENDING_TIME = 0 which means disable timer
    WRBK_EN[30]                  - (RW) Enable writeback
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_TX_CFG_WRBK_EN_ADDR                  WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_WRBK_EN_MASK                  0x40000000                // WRBK_EN[30]
#define WDMA_WRBK_TX_CFG_WRBK_EN_SHFT                  30
#define WDMA_WRBK_TX_CFG_MAX_PENDING_TIME_ADDR         WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_MAX_PENDING_TIME_MASK         0x3FC00000                // MAX_PENDING_TIME[29..22]
#define WDMA_WRBK_TX_CFG_MAX_PENDING_TIME_SHFT         22
#define WDMA_WRBK_TX_CFG_FLUSH_TIMER_EN_ADDR           WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_FLUSH_TIMER_EN_MASK           0x00200000                // FLUSH_TIMER_EN[21]
#define WDMA_WRBK_TX_CFG_FLUSH_TIMER_EN_SHFT           21
#define WDMA_WRBK_TX_CFG_WRBK_THRES_ADDR               WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_WRBK_THRES_MASK               0x000FC000                // WRBK_THRES[19..14]
#define WDMA_WRBK_TX_CFG_WRBK_THRES_SHFT               14
#define WDMA_WRBK_TX_CFG_CURR_STATE_ADDR               WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_CURR_STATE_MASK               0x00003800                // CURR_STATE[13..11]
#define WDMA_WRBK_TX_CFG_CURR_STATE_SHFT               11
#define WDMA_WRBK_TX_CFG_BURST_SIZE_ADDR               WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_BURST_SIZE_MASK               0x000007C0                // BURST_SIZE[10..6]
#define WDMA_WRBK_TX_CFG_BURST_SIZE_SHFT               6
#define WDMA_WRBK_TX_CFG_AXI_ULTRA_ADDR                WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_AXI_ULTRA_MASK                0x00000030                // AXI_ULTRA[5..4]
#define WDMA_WRBK_TX_CFG_AXI_ULTRA_SHFT                4
#define WDMA_WRBK_TX_CFG_RD_BND_4KB_BST_ADDR           WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_RD_BND_4KB_BST_MASK           0x00000008                // RD_BND_4KB_BST[3]
#define WDMA_WRBK_TX_CFG_RD_BND_4KB_BST_SHFT           3
#define WDMA_WRBK_TX_CFG_WR_BND_4KB_BST_ADDR           WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_WR_BND_4KB_BST_MASK           0x00000004                // WR_BND_4KB_BST[2]
#define WDMA_WRBK_TX_CFG_WR_BND_4KB_BST_SHFT           2
#define WDMA_WRBK_TX_CFG_DMAD_SIZE_ADDR                WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_DMAD_SIZE_MASK                0x00000002                // DMAD_SIZE[1]
#define WDMA_WRBK_TX_CFG_DMAD_SIZE_SHFT                1
#define WDMA_WRBK_TX_CFG_BUSY_ADDR                     WDMA_WRBK_TX_CFG_ADDR
#define WDMA_WRBK_TX_CFG_BUSY_MASK                     0x00000001                // BUSY[0]
#define WDMA_WRBK_TX_CFG_BUSY_SHFT                     0

/* =====================================================================================

  ---DMA_WRBK_TX_FIFO_CFG0 (0x15104800 + 0x0304)---

    RING0_CLEAR[0]               - (RW)  xxx
    RING0_FULL[1]                - (RO)  xxx
    RING0_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING0_FREE_CNT[25..20]       - (RO) TX ring 0 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FREE_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG0_ADDR
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FREE_CNT_MASK     0x03F00000                // RING0_FREE_CNT[25..20]
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FREE_CNT_SHFT     20
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_USED_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG0_ADDR
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_USED_CNT_MASK     0x000003F0                // RING0_USED_CNT[9..4]
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_USED_CNT_SHFT     4
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_EMPTY_ADDR        WDMA_WRBK_TX_FIFO_CFG0_ADDR
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_EMPTY_MASK        0x00000004                // RING0_EMPTY[2]
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_EMPTY_SHFT        2
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FULL_ADDR         WDMA_WRBK_TX_FIFO_CFG0_ADDR
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FULL_MASK         0x00000002                // RING0_FULL[1]
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_FULL_SHFT         1
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_CLEAR_ADDR        WDMA_WRBK_TX_FIFO_CFG0_ADDR
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_CLEAR_MASK        0x00000001                // RING0_CLEAR[0]
#define WDMA_WRBK_TX_FIFO_CFG0_RING0_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_TX_FIFO_CFG1 (0x15104800 + 0x0308)---

    RING1_CLEAR[0]               - (RW)  xxx
    RING1_FULL[1]                - (RO)  xxx
    RING1_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING1_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING1_FREE_CNT[25..20]       - (RO) TX ring 1 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FREE_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG1_ADDR
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FREE_CNT_MASK     0x03F00000                // RING1_FREE_CNT[25..20]
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FREE_CNT_SHFT     20
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_USED_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG1_ADDR
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_USED_CNT_MASK     0x000003F0                // RING1_USED_CNT[9..4]
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_USED_CNT_SHFT     4
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_EMPTY_ADDR        WDMA_WRBK_TX_FIFO_CFG1_ADDR
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_EMPTY_MASK        0x00000004                // RING1_EMPTY[2]
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_EMPTY_SHFT        2
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FULL_ADDR         WDMA_WRBK_TX_FIFO_CFG1_ADDR
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FULL_MASK         0x00000002                // RING1_FULL[1]
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_FULL_SHFT         1
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_CLEAR_ADDR        WDMA_WRBK_TX_FIFO_CFG1_ADDR
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_CLEAR_MASK        0x00000001                // RING1_CLEAR[0]
#define WDMA_WRBK_TX_FIFO_CFG1_RING1_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_TX_FIFO_CFG2 (0x15104800 + 0x030c)---

    RING2_CLEAR[0]               - (RW)  xxx
    RING2_FULL[1]                - (RO)  xxx
    RING2_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING2_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING2_FREE_CNT[25..20]       - (RO) TX ring 2 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FREE_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG2_ADDR
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FREE_CNT_MASK     0x03F00000                // RING2_FREE_CNT[25..20]
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FREE_CNT_SHFT     20
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_USED_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG2_ADDR
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_USED_CNT_MASK     0x000003F0                // RING2_USED_CNT[9..4]
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_USED_CNT_SHFT     4
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_EMPTY_ADDR        WDMA_WRBK_TX_FIFO_CFG2_ADDR
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_EMPTY_MASK        0x00000004                // RING2_EMPTY[2]
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_EMPTY_SHFT        2
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FULL_ADDR         WDMA_WRBK_TX_FIFO_CFG2_ADDR
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FULL_MASK         0x00000002                // RING2_FULL[1]
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_FULL_SHFT         1
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_CLEAR_ADDR        WDMA_WRBK_TX_FIFO_CFG2_ADDR
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_CLEAR_MASK        0x00000001                // RING2_CLEAR[0]
#define WDMA_WRBK_TX_FIFO_CFG2_RING2_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_TX_FIFO_CFG3 (0x15104800 + 0x0310)---

    RING3_CLEAR[0]               - (RW)  xxx
    RING3_FULL[1]                - (RO)  xxx
    RING3_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING3_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING3_FREE_CNT[25..20]       - (RO) TX ring 3 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FREE_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG3_ADDR
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FREE_CNT_MASK     0x03F00000                // RING3_FREE_CNT[25..20]
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FREE_CNT_SHFT     20
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_USED_CNT_ADDR     WDMA_WRBK_TX_FIFO_CFG3_ADDR
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_USED_CNT_MASK     0x000003F0                // RING3_USED_CNT[9..4]
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_USED_CNT_SHFT     4
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_EMPTY_ADDR        WDMA_WRBK_TX_FIFO_CFG3_ADDR
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_EMPTY_MASK        0x00000004                // RING3_EMPTY[2]
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_EMPTY_SHFT        2
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FULL_ADDR         WDMA_WRBK_TX_FIFO_CFG3_ADDR
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FULL_MASK         0x00000002                // RING3_FULL[1]
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_FULL_SHFT         1
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_CLEAR_ADDR        WDMA_WRBK_TX_FIFO_CFG3_ADDR
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_CLEAR_MASK        0x00000001                // RING3_CLEAR[0]
#define WDMA_WRBK_TX_FIFO_CFG3_RING3_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_RX_CFG (0x15104800 + 0x0344)---

    BUSY[0]                      - (RO) Writeback agent busy
    DMAD_SIZE[1]                 - (RW) Size of each DMAD
    WR_BND_4KB_BST[2]            - (RW) Enable write transaction across 4kb boundary
    RD_BND_4KB_BST[3]            - (RW) Enable read transaction across 4kb boundary
    AXI_ULTRA[5..4]              - (RW) AXI ULTRA for writeback AW
    BURST_SIZE[10..6]            - (RW) Writeback Bus Burst Size
                                     Bus burst size. Unit is 128bits(16bytes). Maximum value is 16 (256bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (256bytes) or less than size of 1 DMAD.
    CURR_STATE[13..11]           - (RO) Writeback agent FSM
    WRBK_THRES[19..14]           - (RW) Writeback threshold, bit width is related to WRBK FIFO depth
                                     The minimum vaule is 2 which means 1 DMAD
                                     note : value 0 is unavailable
    RESERVED20[20]               - (RO) Reserved bits
    FLUSH_TIMER_EN[21]           - (RW) Enable writeback flush timer
    MAX_PENDING_TIME[29..22]     - (RW) Flush timer asserts interrupt while reaching max pending time
                                     If sets MAX_PENDING_TIME = 0 which means disable timer
    WRBK_EN[30]                  - (RW) Enable writeback
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_RX_CFG_WRBK_EN_ADDR                  WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_WRBK_EN_MASK                  0x40000000                // WRBK_EN[30]
#define WDMA_WRBK_RX_CFG_WRBK_EN_SHFT                  30
#define WDMA_WRBK_RX_CFG_MAX_PENDING_TIME_ADDR         WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_MAX_PENDING_TIME_MASK         0x3FC00000                // MAX_PENDING_TIME[29..22]
#define WDMA_WRBK_RX_CFG_MAX_PENDING_TIME_SHFT         22
#define WDMA_WRBK_RX_CFG_FLUSH_TIMER_EN_ADDR           WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_FLUSH_TIMER_EN_MASK           0x00200000                // FLUSH_TIMER_EN[21]
#define WDMA_WRBK_RX_CFG_FLUSH_TIMER_EN_SHFT           21
#define WDMA_WRBK_RX_CFG_WRBK_THRES_ADDR               WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_WRBK_THRES_MASK               0x000FC000                // WRBK_THRES[19..14]
#define WDMA_WRBK_RX_CFG_WRBK_THRES_SHFT               14
#define WDMA_WRBK_RX_CFG_CURR_STATE_ADDR               WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_CURR_STATE_MASK               0x00003800                // CURR_STATE[13..11]
#define WDMA_WRBK_RX_CFG_CURR_STATE_SHFT               11
#define WDMA_WRBK_RX_CFG_BURST_SIZE_ADDR               WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_BURST_SIZE_MASK               0x000007C0                // BURST_SIZE[10..6]
#define WDMA_WRBK_RX_CFG_BURST_SIZE_SHFT               6
#define WDMA_WRBK_RX_CFG_AXI_ULTRA_ADDR                WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_AXI_ULTRA_MASK                0x00000030                // AXI_ULTRA[5..4]
#define WDMA_WRBK_RX_CFG_AXI_ULTRA_SHFT                4
#define WDMA_WRBK_RX_CFG_RD_BND_4KB_BST_ADDR           WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_RD_BND_4KB_BST_MASK           0x00000008                // RD_BND_4KB_BST[3]
#define WDMA_WRBK_RX_CFG_RD_BND_4KB_BST_SHFT           3
#define WDMA_WRBK_RX_CFG_WR_BND_4KB_BST_ADDR           WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_WR_BND_4KB_BST_MASK           0x00000004                // WR_BND_4KB_BST[2]
#define WDMA_WRBK_RX_CFG_WR_BND_4KB_BST_SHFT           2
#define WDMA_WRBK_RX_CFG_DMAD_SIZE_ADDR                WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_DMAD_SIZE_MASK                0x00000002                // DMAD_SIZE[1]
#define WDMA_WRBK_RX_CFG_DMAD_SIZE_SHFT                1
#define WDMA_WRBK_RX_CFG_BUSY_ADDR                     WDMA_WRBK_RX_CFG_ADDR
#define WDMA_WRBK_RX_CFG_BUSY_MASK                     0x00000001                // BUSY[0]
#define WDMA_WRBK_RX_CFG_BUSY_SHFT                     0

/* =====================================================================================

  ---DMA_WRBK_RX_FIFO_CFG0 (0x15104800 + 0x0348)---

    RING0_CLEAR[0]               - (RW)  xxx
    RING0_FULL[1]                - (RO)  xxx
    RING0_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING0_FREE_CNT[25..20]       - (RO) RX ring 0 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FREE_CNT_ADDR     WDMA_WRBK_RX_FIFO_CFG0_ADDR
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FREE_CNT_MASK     0x03F00000                // RING0_FREE_CNT[25..20]
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FREE_CNT_SHFT     20
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_USED_CNT_ADDR     WDMA_WRBK_RX_FIFO_CFG0_ADDR
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_USED_CNT_MASK     0x000003F0                // RING0_USED_CNT[9..4]
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_USED_CNT_SHFT     4
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_EMPTY_ADDR        WDMA_WRBK_RX_FIFO_CFG0_ADDR
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_EMPTY_MASK        0x00000004                // RING0_EMPTY[2]
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_EMPTY_SHFT        2
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FULL_ADDR         WDMA_WRBK_RX_FIFO_CFG0_ADDR
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FULL_MASK         0x00000002                // RING0_FULL[1]
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_FULL_SHFT         1
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_CLEAR_ADDR        WDMA_WRBK_RX_FIFO_CFG0_ADDR
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_CLEAR_MASK        0x00000001                // RING0_CLEAR[0]
#define WDMA_WRBK_RX_FIFO_CFG0_RING0_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_RX_FIFO_CFG1 (0x15104800 + 0x034c)---

    RING1_CLEAR[0]               - (RW)  xxx
    RING1_FULL[1]                - (RO)  xxx
    RING1_EMPTY[2]               - (RO)  xxx
    RESERVED3[3]                 - (RO) Reserved bits
    RING1_USED_CNT[9..4]         - (RO)  xxx
    RESERVED10[19..10]           - (RO) Reserved bits
    RING1_FREE_CNT[25..20]       - (RO) RX ring 1 configuration
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FREE_CNT_ADDR     WDMA_WRBK_RX_FIFO_CFG1_ADDR
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FREE_CNT_MASK     0x03F00000                // RING1_FREE_CNT[25..20]
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FREE_CNT_SHFT     20
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_USED_CNT_ADDR     WDMA_WRBK_RX_FIFO_CFG1_ADDR
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_USED_CNT_MASK     0x000003F0                // RING1_USED_CNT[9..4]
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_USED_CNT_SHFT     4
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_EMPTY_ADDR        WDMA_WRBK_RX_FIFO_CFG1_ADDR
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_EMPTY_MASK        0x00000004                // RING1_EMPTY[2]
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_EMPTY_SHFT        2
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FULL_ADDR         WDMA_WRBK_RX_FIFO_CFG1_ADDR
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FULL_MASK         0x00000002                // RING1_FULL[1]
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_FULL_SHFT         1
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_CLEAR_ADDR        WDMA_WRBK_RX_FIFO_CFG1_ADDR
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_CLEAR_MASK        0x00000001                // RING1_CLEAR[0]
#define WDMA_WRBK_RX_FIFO_CFG1_RING1_CLEAR_SHFT        0

/* =====================================================================================

  ---DMA_WRBK_SIDX_CFG (0x15104800 + 0x0388)---

    TX_RING0_SIDX_CLR[0]         - (RW) Clear the index
    TX_RING1_SIDX_CLR[1]         - (RW) Clear the index
    TX_RING2_SIDX_CLR[2]         - (RW) Clear the index
    TX_RING3_SIDX_CLR[3]         - (RW) Clear the index
    RX_RING0_SIDX_CLR[4]         - (RW) Clear the index
    RX_RING1_SIDX_CLR[5]         - (RW) Clear the index
    RESERVED6[7..6]              - (RO) Reserved bits
    TX_RING0_SIDX_OW[8]          - (RW) Overwrite the index
    TX_RING1_SIDX_OW[9]          - (RW) Overwrite the index
    TX_RING2_SIDX_OW[10]         - (RW) Overwrite the index
    TX_RING3_SIDX_OW[11]         - (RW) Overwrite the index
    RX_RING0_SIDX_OW[12]         - (RW) Overwrite the index
    RX_RING1_SIDX_OW[13]         - (RW) Overwrite the index
    RESERVED14[15..14]           - (RO) Reserved bits
    MON_SEL[18..16]              - (RW) Select start index show in monitor
    RESERVED19[31..19]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_SIDX_CFG_MON_SEL_ADDR                WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_MON_SEL_MASK                0x00070000                // MON_SEL[18..16]
#define WDMA_WRBK_SIDX_CFG_MON_SEL_SHFT                16
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_OW_MASK       0x00002000                // RX_RING1_SIDX_OW[13]
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_OW_SHFT       13
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_OW_MASK       0x00001000                // RX_RING0_SIDX_OW[12]
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_OW_SHFT       12
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_OW_MASK       0x00000800                // TX_RING3_SIDX_OW[11]
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_OW_SHFT       11
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_OW_MASK       0x00000400                // TX_RING2_SIDX_OW[10]
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_OW_SHFT       10
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_OW_MASK       0x00000200                // TX_RING1_SIDX_OW[9]
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_OW_SHFT       9
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_OW_ADDR       WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_OW_MASK       0x00000100                // TX_RING0_SIDX_OW[8]
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_OW_SHFT       8
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_CLR_MASK      0x00000020                // RX_RING1_SIDX_CLR[5]
#define WDMA_WRBK_SIDX_CFG_RX_RING1_SIDX_CLR_SHFT      5
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_CLR_MASK      0x00000010                // RX_RING0_SIDX_CLR[4]
#define WDMA_WRBK_SIDX_CFG_RX_RING0_SIDX_CLR_SHFT      4
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_CLR_MASK      0x00000008                // TX_RING3_SIDX_CLR[3]
#define WDMA_WRBK_SIDX_CFG_TX_RING3_SIDX_CLR_SHFT      3
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_CLR_MASK      0x00000004                // TX_RING2_SIDX_CLR[2]
#define WDMA_WRBK_SIDX_CFG_TX_RING2_SIDX_CLR_SHFT      2
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_CLR_MASK      0x00000002                // TX_RING1_SIDX_CLR[1]
#define WDMA_WRBK_SIDX_CFG_TX_RING1_SIDX_CLR_SHFT      1
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_CLR_ADDR      WDMA_WRBK_SIDX_CFG_ADDR
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_CLR_MASK      0x00000001                // TX_RING0_SIDX_CLR[0]
#define WDMA_WRBK_SIDX_CFG_TX_RING0_SIDX_CLR_SHFT      0

/* =====================================================================================

  ---DMA_WRBK_SIDX_MON (0x15104800 + 0x0390)---

    START_IDX[15..0]             - (RO) Point to the next DMAD that writeback agent wants to write
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_SIDX_MON_START_IDX_ADDR              WDMA_WRBK_SIDX_MON_ADDR
#define WDMA_WRBK_SIDX_MON_START_IDX_MASK              0x0000FFFF                // START_IDX[15..0]
#define WDMA_WRBK_SIDX_MON_START_IDX_SHFT              0

/* =====================================================================================

  ---DMA_WRBK_SIDX_OW (0x15104800 + 0x0394)---

    START_IDX_OW_VAL[15..0]      - (RW) Overwrite the index with this register field
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_WRBK_SIDX_OW_START_IDX_OW_VAL_ADDR        WDMA_WRBK_SIDX_OW_ADDR
#define WDMA_WRBK_SIDX_OW_START_IDX_OW_VAL_MASK        0x0000FFFF                // START_IDX_OW_VAL[15..0]
#define WDMA_WRBK_SIDX_OW_START_IDX_OW_VAL_SHFT        0

/* =====================================================================================

  ---DMA_DBG_CFG (0x15104800 + 0x0398)---

    debug_sig_sel[3..0]          - (RW) Select debug  signals show in monitor
    debug_sig_en[4]              - (RW) Enable debug monitor/probe
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_DBG_CFG_debug_sig_en_ADDR                 WDMA_DBG_CFG_ADDR
#define WDMA_DBG_CFG_debug_sig_en_MASK                 0x00000010                // debug_sig_en[4]
#define WDMA_DBG_CFG_debug_sig_en_SHFT                 4
#define WDMA_DBG_CFG_debug_sig_sel_ADDR                WDMA_DBG_CFG_ADDR
#define WDMA_DBG_CFG_debug_sig_sel_MASK                0x0000000F                // debug_sig_sel[3..0]
#define WDMA_DBG_CFG_debug_sig_sel_SHFT                0

/* =====================================================================================

  ---DMA_TX_DBG_MON_0 (0x15104800 + 0x039c)---

    debug_monitor_0[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_0_debug_monitor_0_ADDR         WDMA_TX_DBG_MON_0_ADDR
#define WDMA_TX_DBG_MON_0_debug_monitor_0_MASK         0xFFFFFFFF                // debug_monitor_0[31..0]
#define WDMA_TX_DBG_MON_0_debug_monitor_0_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_1 (0x15104800 + 0x03a0)---

    debug_monitor_1[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_1_debug_monitor_1_ADDR         WDMA_TX_DBG_MON_1_ADDR
#define WDMA_TX_DBG_MON_1_debug_monitor_1_MASK         0xFFFFFFFF                // debug_monitor_1[31..0]
#define WDMA_TX_DBG_MON_1_debug_monitor_1_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_2 (0x15104800 + 0x03a4)---

    debug_monitor_2[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_2_debug_monitor_2_ADDR         WDMA_TX_DBG_MON_2_ADDR
#define WDMA_TX_DBG_MON_2_debug_monitor_2_MASK         0xFFFFFFFF                // debug_monitor_2[31..0]
#define WDMA_TX_DBG_MON_2_debug_monitor_2_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_3 (0x15104800 + 0x03a8)---

    debug_monitor_3[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_3_debug_monitor_3_ADDR         WDMA_TX_DBG_MON_3_ADDR
#define WDMA_TX_DBG_MON_3_debug_monitor_3_MASK         0xFFFFFFFF                // debug_monitor_3[31..0]
#define WDMA_TX_DBG_MON_3_debug_monitor_3_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_4 (0x15104800 + 0x03ac)---

    debug_monitor_4[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_4_debug_monitor_4_ADDR         WDMA_TX_DBG_MON_4_ADDR
#define WDMA_TX_DBG_MON_4_debug_monitor_4_MASK         0xFFFFFFFF                // debug_monitor_4[31..0]
#define WDMA_TX_DBG_MON_4_debug_monitor_4_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_5 (0x15104800 + 0x03b0)---

    debug_monitor_5[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_5_debug_monitor_5_ADDR         WDMA_TX_DBG_MON_5_ADDR
#define WDMA_TX_DBG_MON_5_debug_monitor_5_MASK         0xFFFFFFFF                // debug_monitor_5[31..0]
#define WDMA_TX_DBG_MON_5_debug_monitor_5_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_6 (0x15104800 + 0x03b4)---

    debug_monitor_6[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_6_debug_monitor_6_ADDR         WDMA_TX_DBG_MON_6_ADDR
#define WDMA_TX_DBG_MON_6_debug_monitor_6_MASK         0xFFFFFFFF                // debug_monitor_6[31..0]
#define WDMA_TX_DBG_MON_6_debug_monitor_6_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_7 (0x15104800 + 0x03b8)---

    debug_monitor_7[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_7_debug_monitor_7_ADDR         WDMA_TX_DBG_MON_7_ADDR
#define WDMA_TX_DBG_MON_7_debug_monitor_7_MASK         0xFFFFFFFF                // debug_monitor_7[31..0]
#define WDMA_TX_DBG_MON_7_debug_monitor_7_SHFT         0

/* =====================================================================================

  ---DMA_TX_DBG_MON_8 (0x15104800 + 0x03bc)---

    debug_monitor_8[31..0]       - (RO) TX internal signal

 =====================================================================================*/
#define WDMA_TX_DBG_MON_8_debug_monitor_8_ADDR         WDMA_TX_DBG_MON_8_ADDR
#define WDMA_TX_DBG_MON_8_debug_monitor_8_MASK         0xFFFFFFFF                // debug_monitor_8[31..0]
#define WDMA_TX_DBG_MON_8_debug_monitor_8_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_0 (0x15104800 + 0x03c0)---

    debug_monitor_0[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_0_debug_monitor_0_ADDR         WDMA_RX_DBG_MON_0_ADDR
#define WDMA_RX_DBG_MON_0_debug_monitor_0_MASK         0xFFFFFFFF                // debug_monitor_0[31..0]
#define WDMA_RX_DBG_MON_0_debug_monitor_0_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_1 (0x15104800 + 0x03c4)---

    debug_monitor_1[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_1_debug_monitor_1_ADDR         WDMA_RX_DBG_MON_1_ADDR
#define WDMA_RX_DBG_MON_1_debug_monitor_1_MASK         0xFFFFFFFF                // debug_monitor_1[31..0]
#define WDMA_RX_DBG_MON_1_debug_monitor_1_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_2 (0x15104800 + 0x03c8)---

    debug_monitor_2[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_2_debug_monitor_2_ADDR         WDMA_RX_DBG_MON_2_ADDR
#define WDMA_RX_DBG_MON_2_debug_monitor_2_MASK         0xFFFFFFFF                // debug_monitor_2[31..0]
#define WDMA_RX_DBG_MON_2_debug_monitor_2_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_3 (0x15104800 + 0x03cc)---

    debug_monitor_3[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_3_debug_monitor_3_ADDR         WDMA_RX_DBG_MON_3_ADDR
#define WDMA_RX_DBG_MON_3_debug_monitor_3_MASK         0xFFFFFFFF                // debug_monitor_3[31..0]
#define WDMA_RX_DBG_MON_3_debug_monitor_3_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_4 (0x15104800 + 0x03d0)---

    debug_monitor_4[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_4_debug_monitor_4_ADDR         WDMA_RX_DBG_MON_4_ADDR
#define WDMA_RX_DBG_MON_4_debug_monitor_4_MASK         0xFFFFFFFF                // debug_monitor_4[31..0]
#define WDMA_RX_DBG_MON_4_debug_monitor_4_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_5 (0x15104800 + 0x03d4)---

    debug_monitor_5[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_5_debug_monitor_5_ADDR         WDMA_RX_DBG_MON_5_ADDR
#define WDMA_RX_DBG_MON_5_debug_monitor_5_MASK         0xFFFFFFFF                // debug_monitor_5[31..0]
#define WDMA_RX_DBG_MON_5_debug_monitor_5_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_6 (0x15104800 + 0x03d8)---

    debug_monitor_6[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_6_debug_monitor_6_ADDR         WDMA_RX_DBG_MON_6_ADDR
#define WDMA_RX_DBG_MON_6_debug_monitor_6_MASK         0xFFFFFFFF                // debug_monitor_6[31..0]
#define WDMA_RX_DBG_MON_6_debug_monitor_6_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_7 (0x15104800 + 0x03dc)---

    debug_monitor_7[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_7_debug_monitor_7_ADDR         WDMA_RX_DBG_MON_7_ADDR
#define WDMA_RX_DBG_MON_7_debug_monitor_7_MASK         0xFFFFFFFF                // debug_monitor_7[31..0]
#define WDMA_RX_DBG_MON_7_debug_monitor_7_SHFT         0

/* =====================================================================================

  ---DMA_RX_DBG_MON_8 (0x15104800 + 0x03e0)---

    debug_monitor_8[31..0]       - (RO) RX internal signal

 =====================================================================================*/
#define WDMA_RX_DBG_MON_8_debug_monitor_8_ADDR         WDMA_RX_DBG_MON_8_ADDR
#define WDMA_RX_DBG_MON_8_debug_monitor_8_MASK         0xFFFFFFFF                // debug_monitor_8[31..0]
#define WDMA_RX_DBG_MON_8_debug_monitor_8_SHFT         0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_CFG (0x15104800 + 0x03e4)---

    ENABLE[0]                    - (RW) Enable DMA RX multiple ID feature
                                     Must not disable during busy
    BUSY[1]                      - (RO) Busy
                                     Must not disable HW during busy
    RESERVED2[3..2]              - (RO) Reserved bits
    EN_BLOCK_RESV[4]             - (RW) Enable multiple ID HW block reservation subfunction
    EN_BLOCK_FREE[5]             - (RW) Enable multiple ID HW block free subfunction
    EN_DATA_WR[6]                - (RW) Enable multiple ID HW data write subfunction
    EN_DATA_RD[7]                - (RW) Enable multiple ID HW data read subfunction
    RREADY_ALWAYS_RDY[8]         - (RW) Multiple ID HW BUS AXI RREADY always be 1
    RESERVED9[11..9]             - (RO) Reserved bits
    AXI_RRESP_ERR[12]            - (W1C) AXI RRESP Error
    RESERVED13[15..13]           - (RO) Reserved bits
    DBG_MON_SEL[19..16]          - (RW) Debug Monitor Selection
    DBG_MON_SEL_FLAG[23..20]     - (RW) Flag monitor selection of debug monitor
                                     Select flags monitor if DBG_MON_SEL sets to Flag Monitor
    DBG_MON_SEL_DATA_CTRL[28..24] - (RW) Data control monitor selection of debug monitor
                                     Select Data control monitor if DBG_MON_SEL sets to Data Control Monitor
                                     0~15: head index of data queue 0~15
                                     16~31: tail index of data queue 0~15
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_DATA_CTRL_ADDR WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_DATA_CTRL_MASK 0x1F000000                // DBG_MON_SEL_DATA_CTRL[28..24]
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_DATA_CTRL_SHFT 24
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_FLAG_ADDR     WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_FLAG_MASK     0x00F00000                // DBG_MON_SEL_FLAG[23..20]
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_FLAG_SHFT     20
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_ADDR          WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_MASK          0x000F0000                // DBG_MON_SEL[19..16]
#define WDMA_RX_MULTI_ID_CFG_DBG_MON_SEL_SHFT          16
#define WDMA_RX_MULTI_ID_CFG_AXI_RRESP_ERR_ADDR        WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_AXI_RRESP_ERR_MASK        0x00001000                // AXI_RRESP_ERR[12]
#define WDMA_RX_MULTI_ID_CFG_AXI_RRESP_ERR_SHFT        12
#define WDMA_RX_MULTI_ID_CFG_RREADY_ALWAYS_RDY_ADDR    WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_RREADY_ALWAYS_RDY_MASK    0x00000100                // RREADY_ALWAYS_RDY[8]
#define WDMA_RX_MULTI_ID_CFG_RREADY_ALWAYS_RDY_SHFT    8
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_RD_ADDR           WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_RD_MASK           0x00000080                // EN_DATA_RD[7]
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_RD_SHFT           7
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_WR_ADDR           WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_WR_MASK           0x00000040                // EN_DATA_WR[6]
#define WDMA_RX_MULTI_ID_CFG_EN_DATA_WR_SHFT           6
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_FREE_ADDR        WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_FREE_MASK        0x00000020                // EN_BLOCK_FREE[5]
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_FREE_SHFT        5
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_RESV_ADDR        WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_RESV_MASK        0x00000010                // EN_BLOCK_RESV[4]
#define WDMA_RX_MULTI_ID_CFG_EN_BLOCK_RESV_SHFT        4
#define WDMA_RX_MULTI_ID_CFG_BUSY_ADDR                 WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_BUSY_MASK                 0x00000002                // BUSY[1]
#define WDMA_RX_MULTI_ID_CFG_BUSY_SHFT                 1
#define WDMA_RX_MULTI_ID_CFG_ENABLE_ADDR               WDMA_RX_MULTI_ID_CFG_ADDR
#define WDMA_RX_MULTI_ID_CFG_ENABLE_MASK               0x00000001                // ENABLE[0]
#define WDMA_RX_MULTI_ID_CFG_ENABLE_SHFT               0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_BLK_STS (0x15104800 + 0x03e8)---

    BLOCK_FULL[0]                - (RO) Block queue full
    BLOCK_EMPTY[1]               - (RO) Block queue empty
    BLOCK_OVERFLOW[2]            - (W1C) Block queue control HW detects overflow
    BLOCK_UNDERFLOW[3]           - (W1C) Block queue control HW detects underflow
    RESERVED4[7..4]              - (RO) Reserved bits
    BLOCK_HEAD_IDX[11..8]        - (RO) Block queue head index
    BLOCK_TAIL_IDX[15..12]       - (RO) Block queue tail index
    BLOCK_HEAD_IDX_CLR[16]       - (RW) Clear block queue head index
    BLOCK_HEAD_IDX_INC[17]       - (A0) Increase block queue head index
    BLOCK_HEAD_IDX_OW[18]        - (RW) Overwrite block queue head index
    RESERVED19[19]               - (RO) Reserved bits
    BLOCK_HEAD_IDX_OW_VAL[23..20] - (RW) Index value for block queue head index overwrite operation
    BLOCK_TAIL_IDX_CLR[24]       - (RW) Clear block queue tail index
    BLOCK_TAIL_IDX_INC[25]       - (A0) Increase block queue tail index
    BLOCK_TAIL_IDX_OW[26]        - (RW) Overwrite block queue tail index
    RESERVED27[27]               - (RO) Reserved bits
    BLOCK_TAIL_IDX_OW_VAL[31..28] - (RW) Index value for block queue tail index overwrite operation

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_VAL_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_VAL_MASK 0xF0000000                // BLOCK_TAIL_IDX_OW_VAL[31..28]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_VAL_SHFT 28
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_MASK 0x04000000                // BLOCK_TAIL_IDX_OW[26]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_OW_SHFT 26
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_INC_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_INC_MASK 0x02000000                // BLOCK_TAIL_IDX_INC[25]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_INC_SHFT 25
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_CLR_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_CLR_MASK 0x01000000                // BLOCK_TAIL_IDX_CLR[24]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_CLR_SHFT 24
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_VAL_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_VAL_MASK 0x00F00000                // BLOCK_HEAD_IDX_OW_VAL[23..20]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_VAL_SHFT 20
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_MASK 0x00040000                // BLOCK_HEAD_IDX_OW[18]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_OW_SHFT 18
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_INC_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_INC_MASK 0x00020000                // BLOCK_HEAD_IDX_INC[17]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_INC_SHFT 17
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_CLR_ADDR WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_CLR_MASK 0x00010000                // BLOCK_HEAD_IDX_CLR[16]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_CLR_SHFT 16
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_ADDR   WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_MASK   0x0000F000                // BLOCK_TAIL_IDX[15..12]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_TAIL_IDX_SHFT   12
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_ADDR   WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_MASK   0x00000F00                // BLOCK_HEAD_IDX[11..8]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_HEAD_IDX_SHFT   8
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_UNDERFLOW_ADDR  WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_UNDERFLOW_MASK  0x00000008                // BLOCK_UNDERFLOW[3]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_UNDERFLOW_SHFT  3
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_OVERFLOW_ADDR   WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_OVERFLOW_MASK   0x00000004                // BLOCK_OVERFLOW[2]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_OVERFLOW_SHFT   2
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_EMPTY_ADDR      WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_EMPTY_MASK      0x00000002                // BLOCK_EMPTY[1]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_EMPTY_SHFT      1
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_FULL_ADDR       WDMA_RX_MULTI_ID_BLK_STS_ADDR
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_FULL_MASK       0x00000001                // BLOCK_FULL[0]
#define WDMA_RX_MULTI_ID_BLK_STS_BLOCK_FULL_SHFT       0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_DATA_STS0 (0x15104800 + 0x03ec)---

    DATA_FULL[15..0]             - (RO) Full signal of data queue 0~15
    DATA_EMPTY[31..16]           - (RO) Empty signal of data queue 0~15

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_EMPTY_ADDR     WDMA_RX_MULTI_ID_DATA_STS0_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_EMPTY_MASK     0xFFFF0000                // DATA_EMPTY[31..16]
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_EMPTY_SHFT     16
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_FULL_ADDR      WDMA_RX_MULTI_ID_DATA_STS0_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_FULL_MASK      0x0000FFFF                // DATA_FULL[15..0]
#define WDMA_RX_MULTI_ID_DATA_STS0_DATA_FULL_SHFT      0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_DATA_STS1 (0x15104800 + 0x03f0)---

    DATA0_OV[0]                  - (W1C) Overflow signal of data queue 0
    DATA1_OV[1]                  - (W1C) Overflow signal of data queue 1
    DATA2_OV[2]                  - (W1C) Overflow signal of data queue 2
    DATA3_OV[3]                  - (W1C) Overflow signal of data queue 3
    DATA4_OV[4]                  - (W1C) Overflow signal of data queue 4
    DATA5_OV[5]                  - (W1C) Overflow signal of data queue 5
    DATA6_OV[6]                  - (W1C) Overflow signal of data queue 6
    DATA7_OV[7]                  - (W1C) Overflow signal of data queue 7
    DATA8_OV[8]                  - (W1C) Overflow signal of data queue 8
    DATA9_OV[9]                  - (W1C) Overflow signal of data queue 9
    DATA10_OV[10]                - (W1C) Overflow signal of data queue 10
    DATA11_OV[11]                - (W1C) Overflow signal of data queue 11
    DATA12_OV[12]                - (W1C) Overflow signal of data queue 12
    DATA13_OV[13]                - (W1C) Overflow signal of data queue 13
    DATA14_OV[14]                - (W1C) Overflow signal of data queue 14
    DATA15_OV[15]                - (W1C) Overflow signal of data queue 15
    DATA0_UD[16]                 - (W1C) Underflow signal of data queue 0
    DATA1_UD[17]                 - (W1C) Underflow signal of data queue 1
    DATA2_UD[18]                 - (W1C) Underflow signal of data queue 2
    DATA3_UD[19]                 - (W1C) Underflow signal of data queue 3
    DATA4_UD[20]                 - (W1C) Underflow signal of data queue 4
    DATA5_UD[21]                 - (W1C) Underflow signal of data queue 5
    DATA6_UD[22]                 - (W1C) Underflow signal of data queue 6
    DATA7_UD[23]                 - (W1C) Underflow signal of data queue 7
    DATA8_UD[24]                 - (W1C) Underflow signal of data queue 8
    DATA9_UD[25]                 - (W1C) Underflow signal of data queue 9
    DATA10_UD[26]                - (W1C) Underflow signal of data queue 10
    DATA11_UD[27]                - (W1C) Underflow signal of data queue 11
    DATA12_UD[28]                - (W1C) Underflow signal of data queue 12
    DATA13_UD[29]                - (W1C) Underflow signal of data queue 13
    DATA14_UD[30]                - (W1C) Underflow signal of data queue 14
    DATA15_UD[31]                - (W1C) Underflow signal of data queue 15

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_UD_MASK      0x80000000                // DATA15_UD[31]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_UD_SHFT      31
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_UD_MASK      0x40000000                // DATA14_UD[30]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_UD_SHFT      30
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_UD_MASK      0x20000000                // DATA13_UD[29]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_UD_SHFT      29
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_UD_MASK      0x10000000                // DATA12_UD[28]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_UD_SHFT      28
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_UD_MASK      0x08000000                // DATA11_UD[27]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_UD_SHFT      27
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_UD_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_UD_MASK      0x04000000                // DATA10_UD[26]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_UD_SHFT      26
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_UD_MASK       0x02000000                // DATA9_UD[25]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_UD_SHFT       25
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_UD_MASK       0x01000000                // DATA8_UD[24]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_UD_SHFT       24
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_UD_MASK       0x00800000                // DATA7_UD[23]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_UD_SHFT       23
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_UD_MASK       0x00400000                // DATA6_UD[22]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_UD_SHFT       22
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_UD_MASK       0x00200000                // DATA5_UD[21]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_UD_SHFT       21
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_UD_MASK       0x00100000                // DATA4_UD[20]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_UD_SHFT       20
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_UD_MASK       0x00080000                // DATA3_UD[19]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_UD_SHFT       19
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_UD_MASK       0x00040000                // DATA2_UD[18]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_UD_SHFT       18
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_UD_MASK       0x00020000                // DATA1_UD[17]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_UD_SHFT       17
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_UD_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_UD_MASK       0x00010000                // DATA0_UD[16]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_UD_SHFT       16
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_OV_MASK      0x00008000                // DATA15_OV[15]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA15_OV_SHFT      15
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_OV_MASK      0x00004000                // DATA14_OV[14]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA14_OV_SHFT      14
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_OV_MASK      0x00002000                // DATA13_OV[13]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA13_OV_SHFT      13
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_OV_MASK      0x00001000                // DATA12_OV[12]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA12_OV_SHFT      12
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_OV_MASK      0x00000800                // DATA11_OV[11]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA11_OV_SHFT      11
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_OV_ADDR      WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_OV_MASK      0x00000400                // DATA10_OV[10]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA10_OV_SHFT      10
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_OV_MASK       0x00000200                // DATA9_OV[9]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA9_OV_SHFT       9
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_OV_MASK       0x00000100                // DATA8_OV[8]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA8_OV_SHFT       8
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_OV_MASK       0x00000080                // DATA7_OV[7]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA7_OV_SHFT       7
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_OV_MASK       0x00000040                // DATA6_OV[6]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA6_OV_SHFT       6
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_OV_MASK       0x00000020                // DATA5_OV[5]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA5_OV_SHFT       5
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_OV_MASK       0x00000010                // DATA4_OV[4]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA4_OV_SHFT       4
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_OV_MASK       0x00000008                // DATA3_OV[3]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA3_OV_SHFT       3
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_OV_MASK       0x00000004                // DATA2_OV[2]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA2_OV_SHFT       2
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_OV_MASK       0x00000002                // DATA1_OV[1]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA1_OV_SHFT       1
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_OV_ADDR       WDMA_RX_MULTI_ID_DATA_STS1_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_OV_MASK       0x00000001                // DATA0_OV[0]
#define WDMA_RX_MULTI_ID_DATA_STS1_DATA0_OV_SHFT       0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_DATA_STS2 (0x15104800 + 0x03f4)---

    DATA_HEAD_IDX_CLR[15..0]     - (RW) Clear head index for data queue 0~15
    DATA_TAIL_IDX_CLR[31..16]    - (RW) Clear tail index for data queue 0~15

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_TAIL_IDX_CLR_ADDR WDMA_RX_MULTI_ID_DATA_STS2_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_TAIL_IDX_CLR_MASK 0xFFFF0000                // DATA_TAIL_IDX_CLR[31..16]
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_TAIL_IDX_CLR_SHFT 16
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_HEAD_IDX_CLR_ADDR WDMA_RX_MULTI_ID_DATA_STS2_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_HEAD_IDX_CLR_MASK 0x0000FFFF                // DATA_HEAD_IDX_CLR[15..0]
#define WDMA_RX_MULTI_ID_DATA_STS2_DATA_HEAD_IDX_CLR_SHFT 0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_DATA_STS3 (0x15104800 + 0x03f8)---

    IDX_OW[0]                    - (RW) Overwrite data queue index according to DBG_MON_SEL_DATA_CTRL
    IDX_INC[1]                   - (A0) Increase data queue index according to DBG_MON_SEL_DATA_CTRL
    RESERVED2[3..2]              - (RO) Reserved bits
    IDX_OW_VAL[8..4]             - (RW) Index value for data queue index overwrite operation
    RESERVED9[15..9]             - (RO) Reserved bits
    FLAG_DLAST_CLR[31..16]       - (RW) Clear last data flag for data queue 0~15

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_DATA_STS3_FLAG_DLAST_CLR_ADDR WDMA_RX_MULTI_ID_DATA_STS3_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS3_FLAG_DLAST_CLR_MASK 0xFFFF0000                // FLAG_DLAST_CLR[31..16]
#define WDMA_RX_MULTI_ID_DATA_STS3_FLAG_DLAST_CLR_SHFT 16
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_VAL_ADDR     WDMA_RX_MULTI_ID_DATA_STS3_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_VAL_MASK     0x000001F0                // IDX_OW_VAL[8..4]
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_VAL_SHFT     4
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_INC_ADDR        WDMA_RX_MULTI_ID_DATA_STS3_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_INC_MASK        0x00000002                // IDX_INC[1]
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_INC_SHFT        1
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_ADDR         WDMA_RX_MULTI_ID_DATA_STS3_ADDR
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_MASK         0x00000001                // IDX_OW[0]
#define WDMA_RX_MULTI_ID_DATA_STS3_IDX_OW_SHFT         0

/* =====================================================================================

  ---DMA_RX_MULTI_ID_DBG_MON (0x15104800 + 0x03fc)---

    DBG_MON[31..0]               - (RO) Debug Monitor
                                     Data control flag monitor

 =====================================================================================*/
#define WDMA_RX_MULTI_ID_DBG_MON_DBG_MON_ADDR          WDMA_RX_MULTI_ID_DBG_MON_ADDR
#define WDMA_RX_MULTI_ID_DBG_MON_DBG_MON_MASK          0xFFFFFFFF                // DBG_MON[31..0]
#define WDMA_RX_MULTI_ID_DBG_MON_DBG_MON_SHFT          0

#ifdef __cplusplus
}
#endif

#endif // __WDMA0_V3_REGS_H__
