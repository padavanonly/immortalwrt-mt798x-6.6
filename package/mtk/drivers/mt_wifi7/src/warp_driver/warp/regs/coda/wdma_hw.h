#ifndef _WDMA_HW_H_
#define _WDMA_HW_H_

#ifndef REG_FLD
#define REG_FLD(width, shift) (shift)
#endif /*REG_FLD*/

/*
 * WDMA0 Base: 0x1B102800
 * WDMA1 Base: 0x1B102C00
 */
#define WDMA_TX_BASE_PTR_0 0x000000000
#define WDMA_TX_MAX_CNT_0  0x000000004
#define WDMA_TX_CTX_IDX_0  0x000000008
#define WDMA_TX_DTX_IDX_0  0x00000000C
#define WDMA_TX_BASE_PTR_1 0x000000010
#define WDMA_TX_MAX_CNT_1  0x000000014
#define WDMA_TX_CTX_IDX_1  0x000000018
#define WDMA_TX_DTX_IDX_1  0x00000001C
#define WDMA_TX_BASE_PTR_2 0x000000020
#define WDMA_TX_MAX_CNT_2  0x000000024
#define WDMA_TX_CTX_IDX_2  0x000000028
#define WDMA_TX_DTX_IDX_2  0x00000002C
#define WDMA_TX_BASE_PTR_3 0x000000030
#define WDMA_TX_MAX_CNT_3  0x000000034
#define WDMA_TX_CTX_IDX_3  0x000000038
#define WDMA_TX_DTX_IDX_3  0x00000003C
#define WDMA_RX_BASE_PTR_0 0x000000100
#define WDMA_RX_MAX_CNT_0  0x000000104
#define WDMA_RX_CRX_IDX_0  0x000000108
#define WDMA_RX_DRX_IDX_0  0x00000010C
#define WDMA_RX_BASE_PTR_1 0x000000110
#define WDMA_RX_MAX_CNT_1  0x000000114
#define WDMA_RX_CRX_IDX_1  0x000000118
#define WDMA_RX_DRX_IDX_1  0x00000011C
#define WDMA_INFO          0x000000200
#define WDMA_GLO_CFG       0x000000204
#define WDMA_RST_IDX       0x000000208
#define WDMA_DELAY_INT_CFG 0x00000020C
#define WDMA_FREEQ_THRES   0x000000210
#define WDMA_INT_STATUS    0x000000220
#define WDMA_INT_MASK      0x000000228
#define WDMA_TX_DBG_MON0   0x000000230
#define WDMA_TX_DBG_MON1   0x000000234
#define WDMA_RX_DBG_MON0   0x000000238
#define WDMA_RX_DBG_MON1   0x00000023C
#define WDMA_INT_STS_GRP0  0x000000240
#define WDMA_INT_STS_GRP1  0x000000244
#define WDMA_INT_STS_GRP2  0x000000248
#define WDMA_INT_GRP1      0x000000250
#define WDMA_INT_GRP2      0x000000254
#define WDMA_SCH_Q01_CFG   0x000000280
#define WDMA_SCH_Q23_CFG   0x000000284

#define WDMA_TX_BASE_PTR_0_FLD_TX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_TX_MAX_CNT_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_MAX_CNT_0_FLD_TX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_TX_CTX_IDX_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_CTX_IDX_0_FLD_TX_CTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_DTX_IDX_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_DTX_IDX_0_FLD_TX_DTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_BASE_PTR_1_FLD_TX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_TX_MAX_CNT_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_MAX_CNT_1_FLD_TX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_TX_CTX_IDX_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_CTX_IDX_1_FLD_TX_CTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_DTX_IDX_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_DTX_IDX_1_FLD_TX_DTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_BASE_PTR_2_FLD_TX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_TX_MAX_CNT_2_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_MAX_CNT_2_FLD_TX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_TX_CTX_IDX_2_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_CTX_IDX_2_FLD_TX_CTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_DTX_IDX_2_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_DTX_IDX_2_FLD_TX_DTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_BASE_PTR_3_FLD_TX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_TX_MAX_CNT_3_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_MAX_CNT_3_FLD_TX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_TX_CTX_IDX_3_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_CTX_IDX_3_FLD_TX_CTX_IDX                      REG_FLD(12, 0)
#define WDMA_TX_DTX_IDX_3_FLD_REV                             REG_FLD(20, 12)
#define WDMA_TX_DTX_IDX_3_FLD_TX_DTX_IDX                      REG_FLD(12, 0)
#define WDMA_RX_BASE_PTR_0_FLD_RX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_RX_MAX_CNT_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_MAX_CNT_0_FLD_RX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_RX_CRX_IDX_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_CRX_IDX_0_FLD_RX_CRX_IDX                      REG_FLD(12, 0)
#define WDMA_RX_DRX_IDX_0_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_DRX_IDX_0_FLD_RX_DRX_IDX                      REG_FLD(12, 0)
#define WDMA_RX_BASE_PTR_1_FLD_RX_BASE_PTR                    REG_FLD(32, 0)
#define WDMA_RX_MAX_CNT_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_MAX_CNT_1_FLD_RX_MAX_CNT                      REG_FLD(12, 0)
#define WDMA_RX_CRX_IDX_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_CRX_IDX_1_FLD_RX_CRX_IDX                      REG_FLD(12, 0)
#define WDMA_RX_DRX_IDX_1_FLD_REV                             REG_FLD(20, 12)
#define WDMA_RX_DRX_IDX_1_FLD_RX_DRX_IDX                      REG_FLD(12, 0)
#define WDMA_INFO_FLD_REV                                     REG_FLD(4, 28)
#define WDMA_INFO_FLD_INDEX_WIDTH                             REG_FLD(4, 24)
#define WDMA_INFO_FLD_BASE_PTR_WIDTH                          REG_FLD(8, 16)
#define WDMA_INFO_FLD_RX_RING_NUM                             REG_FLD(8, 8)
#define WDMA_INFO_FLD_TX_RING_NUM                             REG_FLD(8, 0)
#define WDMA_GLO_CFG_FLD_RX_2B_OFFSET                         REG_FLD(1, 31)
#define WDMA_GLO_CFG_FLD_CSR_CLKGATE_BYP                      REG_FLD(1, 30)
#define WDMA_GLO_CFG_FLD_BYTE_SWAP                            REG_FLD(1, 29)
#define WDMA_GLO_CFG_FLD_RX_INFO1_PRESERVE                    REG_FLD(1, 28)
#define WDMA_GLO_CFG_FLD_RX_INFO2_PRESERVE                    REG_FLD(1, 27)
#define WDMA_GLO_CFG_FLD_RX_INFO3_PRESERVE                    REG_FLD(1, 26)
#define WDMA_GLO_CFG_FLD_RX_WCID_SEL                          REG_FLD(1, 25)
#define WDMA_GLO_CFG_FLD_RX_WIFI_EN                           REG_FLD(1, 24)
#define WDMA_GLO_CFG_FLD_REV0                                 REG_FLD(13, 11)
#define WDMA_GLO_CFG_FLD_MULTI_EN                             REG_FLD(1, 10)
#define WDMA_GLO_CFG_FLD_EXT_FIFO_EN                          REG_FLD(1, 9)
#define WDMA_GLO_CFG_FLD_DESC_32B_E                           REG_FLD(1, 8)
#define WDMA_GLO_CFG_FLD_BIG_ENDIAN                           REG_FLD(1, 7)
#define WDMA_GLO_CFG_FLD_TX_WB_DDONE                          REG_FLD(1, 6)
#define WDMA_GLO_CFG_FLD_WDMA_BT_SIZE                         REG_FLD(2, 4)
#define WDMA_GLO_CFG_FLD_RX_DMA_BUSY                          REG_FLD(1, 3)
#define WDMA_GLO_CFG_FLD_RX_DMA_EN                            REG_FLD(1, 2)
#define WDMA_GLO_CFG_FLD_TX_DMA_BUSY                          REG_FLD(1, 1)
#define WDMA_GLO_CFG_FLD_TX_DMA_EN                            REG_FLD(1, 0)
#define WDMA_RST_IDX_FLD_REV0                                 REG_FLD(14, 18)
#define WDMA_RST_IDX_FLD_RST_DRX_IDX1                         REG_FLD(1, 17)
#define WDMA_RST_IDX_FLD_RST_DRX_IDX0                         REG_FLD(1, 16)
#define WDMA_RST_IDX_FLD_REV1                                 REG_FLD(12, 4)
#define WDMA_RST_IDX_FLD_RST_DTX_IDX3                         REG_FLD(1, 3)
#define WDMA_RST_IDX_FLD_RST_DTX_IDX2                         REG_FLD(1, 2)
#define WDMA_RST_IDX_FLD_RST_DTX_IDX1                         REG_FLD(1, 1)
#define WDMA_RST_IDX_FLD_RST_DTX_IDX0                         REG_FLD(1, 0)
#define WDMA_DELAY_INT_CFG_FLD_TXDLY_INT_EN                   REG_FLD(1, 31)
#define WDMA_DELAY_INT_CFG_FLD_TXMAX_PINT                     REG_FLD(7, 24)
#define WDMA_DELAY_INT_CFG_FLD_TXMAX_PTIME                    REG_FLD(8, 16)
#define WDMA_DELAY_INT_CFG_FLD_RXDLY_INT_EN                   REG_FLD(1, 15)
#define WDMA_DELAY_INT_CFG_FLD_RXMAX_PINT                     REG_FLD(7, 8)
#define WDMA_DELAY_INT_CFG_FLD_RXMAX_PTIME                    REG_FLD(8, 0)
#define WDMA_FREEQ_THRES_FLD_REV1                             REG_FLD(28, 4)
#define WDMA_FREEQ_THRES_FLD_FREEQ_THRES                      REG_FLD(4, 0)
#define WDMA_INT_STATUS_FLD_RX_COHERENT                       REG_FLD(1, 31)
#define WDMA_INT_STATUS_FLD_RX_DLY_INT                        REG_FLD(1, 30)
#define WDMA_INT_STATUS_FLD_TX_COHERENT                       REG_FLD(1, 29)
#define WDMA_INT_STATUS_FLD_TX_DLY_INT                        REG_FLD(1, 28)
#define WDMA_INT_STATUS_FLD_REV0                              REG_FLD(10, 18)
#define WDMA_INT_STATUS_FLD_RX_DONE_INT1                      REG_FLD(1, 17)
#define WDMA_INT_STATUS_FLD_RX_DONE_INT0                      REG_FLD(1, 16)
#define WDMA_INT_STATUS_FLD_REV1                              REG_FLD(12, 4)
#define WDMA_INT_STATUS_FLD_TX_DONE_INT3                      REG_FLD(1, 3)
#define WDMA_INT_STATUS_FLD_TX_DONE_INT2                      REG_FLD(1, 2)
#define WDMA_INT_STATUS_FLD_TX_DONE_INT1                      REG_FLD(1, 1)
#define WDMA_INT_STATUS_FLD_TX_DONE_INT0                      REG_FLD(1, 0)
#define WDMA_INT_MASK_FLD_RX_COHERENT                         REG_FLD(1, 31)
#define WDMA_INT_MASK_FLD_RX_DLY_INT                          REG_FLD(1, 30)
#define WDMA_INT_MASK_FLD_TX_COHERENT                         REG_FLD(1, 29)
#define WDMA_INT_MASK_FLD_TX_DLY_INT                          REG_FLD(1, 28)
#define WDMA_INT_MASK_FLD_REV0                                REG_FLD(10, 18)
#define WDMA_INT_MASK_FLD_RX_DONE_INT1                        REG_FLD(1, 17)
#define WDMA_INT_MASK_FLD_RX_DONE_INT0                        REG_FLD(1, 16)
#define WDMA_INT_MASK_FLD_REV1                                REG_FLD(12, 4)
#define WDMA_INT_MASK_FLD_TX_DONE_INT3                        REG_FLD(1, 3)
#define WDMA_INT_MASK_FLD_TX_DONE_INT2                        REG_FLD(1, 2)
#define WDMA_INT_MASK_FLD_TX_DONE_INT1                        REG_FLD(1, 1)
#define WDMA_INT_MASK_FLD_TX_DONE_INT0                        REG_FLD(1, 0)
#define WDMA_TX_DBG_MON0_FLD_MONITOR                          REG_FLD(32, 0)
#define WDMA_TX_DBG_MON1_FLD_MONITOR                          REG_FLD(32, 0)
#define WDMA_RX_DBG_MON0_FLD_MONITOR                          REG_FLD(32, 0)
#define WDMA_RX_DBG_MON1_FLD_MONITOR                          REG_FLD(32, 0)
#define WDMA_INT_STS_GRP0_FLD_WDMA_INT_STS_GRP0               REG_FLD(32, 0)
#define WDMA_INT_STS_GRP1_FLD_WDMA_INT_STS_GRP1               REG_FLD(32, 0)
#define WDMA_INT_STS_GRP2_FLD_WDMA_INT_STS_GRP2               REG_FLD(32, 0)
#define WDMA_INT_GRP1_FLD_WDMA_INT_GRP1                       REG_FLD(32, 0)
#define WDMA_INT_GRP2_FLD_WDMA_INT_GRP2                       REG_FLD(32, 0)
#define WDMA_SCH_Q01_CFG_FLD_MAX_BKT_SIZE1                    REG_FLD(1, 31)
#define WDMA_SCH_Q01_CFG_FLD_MAX_RATE_ULMT1                   REG_FLD(1, 30)
#define WDMA_SCH_Q01_CFG_FLD_MAX_WEIGHT1                      REG_FLD(2, 28)
#define WDMA_SCH_Q01_CFG_FLD_MIN_RATE_RATIO1                  REG_FLD(2, 26)
#define WDMA_SCH_Q01_CFG_FLD_MAX_RATE1                        REG_FLD(10, 16)
#define WDMA_SCH_Q01_CFG_FLD_MAX_BKT_SIZE0                    REG_FLD(1, 15)
#define WDMA_SCH_Q01_CFG_FLD_MAX_RATE_ULMT0                   REG_FLD(1, 14)
#define WDMA_SCH_Q01_CFG_FLD_MAX_WEIGHT0                      REG_FLD(2, 12)
#define WDMA_SCH_Q01_CFG_FLD_MIN_RATE_RATIO0                  REG_FLD(2, 10)
#define WDMA_SCH_Q01_CFG_FLD_MAX_RATE0                        REG_FLD(10, 0)
#define WDMA_SCH_Q23_CFG_FLD_MAX_BKT_SIZE3                    REG_FLD(1, 31)
#define WDMA_SCH_Q23_CFG_FLD_MAX_RATE_ULMT3                   REG_FLD(1, 30)
#define WDMA_SCH_Q23_CFG_FLD_MAX_WEIGHT3                      REG_FLD(2, 28)
#define WDMA_SCH_Q23_CFG_FLD_MIN_RATE_RATIO3                  REG_FLD(2, 26)
#define WDMA_SCH_Q23_CFG_FLD_MAX_RATE3                        REG_FLD(10, 16)
#define WDMA_SCH_Q23_CFG_FLD_MAX_BKT_SIZE2                    REG_FLD(1, 15)
#define WDMA_SCH_Q23_CFG_FLD_MAX_RATE_ULMT2                   REG_FLD(1, 14)
#define WDMA_SCH_Q23_CFG_FLD_MAX_WEIGHT2                      REG_FLD(2, 12)
#define WDMA_SCH_Q23_CFG_FLD_MIN_RATE_RATIO2                  REG_FLD(2, 10)
#define WDMA_SCH_Q23_CFG_FLD_MAX_RATE2                        REG_FLD(10, 0)

#endif /*_WDMA_HW_H_*/
