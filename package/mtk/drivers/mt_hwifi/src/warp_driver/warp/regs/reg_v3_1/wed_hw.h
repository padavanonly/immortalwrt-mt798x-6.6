
#ifndef __WED0_REGS_H__
#define __WED0_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
//
//                     WED0 CR Definitions
//
//****************************************************************************

#define WED_ID_ADDR                                       0x00000000
#define WED_REV_ID_ADDR                                   0x00000004
#define WED_MOD_RST_ADDR                                  0x00000008
#define WED_CTRL_ADDR                                     0x0000000C
#define WED_AXI_CTRL_ADDR                                 0x00000010
#define WED_LOOPBACK_ADDR                                 0x00000014
#define WED_CTRL2_ADDR                                    0x0000001C
#define WED_EX_INT_STA_ADDR                               0x00000020
#define WED_EX_INT_MSK_ADDR                               0x00000028
#define WED_EX_INT_MSK1_ADDR                              0x0000002C
#define WED_EX_INT_MSK2_ADDR                              0x00000030
#define WED_EX_INT_MSK3_ADDR                              0x00000034
#define WED_EX_INT_STA_POST_MSK0_ADDR                     0x00000040
#define WED_EX_INT_STA_POST_MSK1_ADDR                     0x00000044
#define WED_EX_INT_STA_POST_MSK2_ADDR                     0x00000048
#define WED_EX_INT_STA_POST_MSK3_ADDR                     0x0000004C
#define WED_IRQ_MON_ADDR                                  0x00000050
#define WED_ERR_MON_ADDR                                  0x00000054
#define WED_ST_ADDR                                       0x00000060
#define WED_WPDMA_ST_ADDR                                 0x00000064
#define WED_WDMA_ST_ADDR                                  0x00000068
#define WED_BM_ST_ADDR                                    0x0000006C
#define WED_TX_BM_BASE_ADDR                               0x00000084
#define WED_TX_BM_INIT_PTR_ADDR                           0x00000088
#define WED_TX_BM_BLEN_ADDR                               0x0000008C
#define WED_TX_BM_STS_ADDR                                0x00000090
#define WED_TX_BM_PTR_ADDR                                0x00000094
#define WED_TX_BM_RANGE_CFG_ADDR                          0x00000098
#define WED_TX_BM_ERR_STS_ADDR                            0x000000A4
#define WED_TX_TKID_CTRL_ADDR                             0x000000C0
#define WED_TX_TKID_SPR_ADDR                              0x000000C4
#define WED_TX_TKID_TKID_ADDR                             0x000000C8
#define WED_TX_BM_INIT_HEAD_PTR_ADDR                      0x000000CC
#define WED_TX_TKID_INTF_ADDR                             0x000000DC
#define WED_TX_TKID_DYN_TH_ADDR                           0x000000E0
#define WED_TX_TKID_STS_ADDR                              0x000000E4
#define WED_TX_TKID_RECYC_ADDR                            0x000000E8
#define WED_TX_TKID_ALI_ADDR                              0x000000EC
#define WED_TX_TKID_VB_FREE_0_31_ADDR                     0x000000F0
#define WED_TX_TKID_VB_FREE_32_63_ADDR                    0x000000F4
#define WED_TX_TKID_VB_USED_0_31_ADDR                     0x000000F8
#define WED_TX_TKID_VB_USED_32_63_ADDR                    0x000000FC
#define WED_TXD_DW0_ADDR                                  0x00000100
#define WED_TXD_DW1_ADDR                                  0x00000104
#define WED_TXD_DW2_ADDR                                  0x00000108
#define WED_TXD_DW3_ADDR                                  0x0000010C
#define WED_TXD_DW4_ADDR                                  0x00000110
#define WED_TXD_DW5_ADDR                                  0x00000114
#define WED_TXD_DW6_ADDR                                  0x00000118
#define WED_TXD_DW7_ADDR                                  0x0000011C
#define WED_TXP_DW0_ADDR                                  0x00000120
#define WED_TXP_DW1_ADDR                                  0x00000124
#define WED_TXDP_CTRL_ADDR                                0x00000130
#define WED_DBG_CTRL_ADDR                                 0x00000180
#define WED_DBG_BUS_ADDR_L_ADDR                           0x00000184
#define WED_DBG_BUS_ADDR_H_ADDR                           0x00000188
#define WED_DBG_BUS_MON_ADDR                              0x0000018C
#define WED_DBG_PRB0_ADDR                                 0x00000190
#define WED_DBG_PRB1_ADDR                                 0x00000194
#define WED_DBG_PRB2_ADDR                                 0x00000198
#define WED_DBG_PRB3_ADDR                                 0x0000019C
#define WED_TX_TKID_FREE_TKID_IN_ADDR                     0x000001B0
#define WED_TX_TKID_FREE_TKID_LDDRAM_ADDR                 0x000001B4
#define WED_TX_TKID_FREE_TKID_LDSRAM_ADDR                 0x000001B8
#define WED_TX_TKID_FREE_TKID_RECYCLE_ADDR                0x000001BC
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_ADDR              0x000001C0
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_ADDR           0x000001C4
#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_ADDR           0x000001C8
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_ADDR         0x000001CC
#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_ADDR           0x000001D4
#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_ADDR        0x000001D8
#define WED_PMTR_CTRL_ADDR                                0x000001DC
#define WED_PMTR_TGT_ADDR                                 0x000001E0
#define WED_PMTR_TGT_ST_ADDR                              0x000001E4
#define WED_PMTR_LTCY_MAX0_1_ADDR                         0x000001E8
#define WED_PMTR_LTCY_MAX2_3_ADDR                         0x000001EC
#define WED_PMTR_LTCY_ACC0_ADDR                           0x000001F0
#define WED_PMTR_LTCY_ACC1_ADDR                           0x000001F4
#define WED_PMTR_LTCY_ACC2_ADDR                           0x000001F8
#define WED_PMTR_LTCY_ACC3_ADDR                           0x000001FC
#define WED_INT_STA_ADDR                                  0x00000200
#define WED_INT_MSK_ADDR                                  0x00000204
#define WED_GLO_CFG_ADDR                                  0x00000208
#define WED_RST_IDX_ADDR                                  0x0000020C
#define WED_DLY_INT_CFG_ADDR                              0x00000210
#define WED_DLY_INT_CFG1_ADDR                             0x00000214
#define WED_INT_CTRL_ADDR                                 0x00000218
#define WED_SPR_ADDR                                      0x0000021C
#define WED_INT_MSK1_ADDR                                 0x00000224
#define WED_INT_MSK2_ADDR                                 0x00000228
#define WED_INT_MSK3_ADDR                                 0x0000022C
#define WED_INT_STA_POST_MSK0_ADDR                        0x00000230
#define WED_INT_STA_POST_MSK1_ADDR                        0x00000234
#define WED_INT_STA_POST_MSK2_ADDR                        0x00000238
#define WED_INT_STA_POST_MSK3_ADDR                        0x0000023C
#define WED_SPR_0X24C_ADDR                                0x0000024C
#define WED_SPR_RO_ZERO_ADDR                              0x00000258
#define WED_SPR_0X25C_ADDR                                0x0000025C
#define WED_TX0_MIB_ADDR                                  0x000002A0
#define WED_TX1_MIB_ADDR                                  0x000002A4
#define WED_TX_COHERENT_MIB_ADDR                          0x000002D0
#define WED_RX0_MIB_ADDR                                  0x000002E0
#define WED_RX1_MIB_ADDR                                  0x000002E4
#define WED_RX_DMA_MIB_0_ADDR                             0x000002F0
#define WED_RX_DMA_MIB_1_ADDR                             0x000002F4
#define WED_RX_DMA_ERR_ADDR                               0x000002F8
#define WED_TX0_CTRL0_ADDR                                0x00000300
#define WED_TX0_CTRL1_ADDR                                0x00000304
#define WED_TX0_CTRL2_ADDR                                0x00000308
#define WED_TX0_CTRL3_ADDR                                0x0000030C
#define WED_TX1_CTRL0_ADDR                                0x00000310
#define WED_TX1_CTRL1_ADDR                                0x00000314
#define WED_TX1_CTRL2_ADDR                                0x00000318
#define WED_TX1_CTRL3_ADDR                                0x0000031C
#define WED_HIFTXD_BASE00_L_ADDR                          0x00000320
#define WED_HIFTXD_BASE01_L_ADDR                          0x00000324
#define WED_HIFTXD_BASE02_L_ADDR                          0x00000328
#define WED_HIFTXD_BASE03_L_ADDR                          0x0000032C
#define WED_HIFTXD_BASE04_L_ADDR                          0x00000330
#define WED_HIFTXD_BASE05_L_ADDR                          0x00000334
#define WED_HIFTXD_BASE06_L_ADDR                          0x00000338
#define WED_HIFTXD_BASE07_L_ADDR                          0x0000033C
#define WED_HIFTXD_BASE08_L_ADDR                          0x00000340
#define WED_HIFTXD_BASE09_L_ADDR                          0x00000344
#define WED_HIFTXD_BASE10_L_ADDR                          0x00000348
#define WED_HIFTXD_BASE11_L_ADDR                          0x0000034C
#define WED_HIFTXD_BASE12_L_ADDR                          0x00000350
#define WED_HIFTXD_BASE13_L_ADDR                          0x00000354
#define WED_HIFTXD_BASE14_L_ADDR                          0x00000358
#define WED_HIFTXD_BASE15_L_ADDR                          0x0000035C
#define WED_HIFTXD_BASE16_L_ADDR                          0x00000360
#define WED_HIFTXD_BASE17_L_ADDR                          0x00000364
#define WED_HIFTXD_BASE18_L_ADDR                          0x00000368
#define WED_HIFTXD_BASE19_L_ADDR                          0x0000036C
#define WED_HIFTXD_BASE20_L_ADDR                          0x00000370
#define WED_HIFTXD_BASE21_L_ADDR                          0x00000374
#define WED_HIFTXD_BASE22_L_ADDR                          0x00000378
#define WED_HIFTXD_BASE23_L_ADDR                          0x0000037C
#define WED_HIFTXD_BASE24_L_ADDR                          0x00000380
#define WED_HIFTXD_BASE25_L_ADDR                          0x00000384
#define WED_HIFTXD_BASE26_L_ADDR                          0x00000388
#define WED_HIFTXD_BASE27_L_ADDR                          0x0000038C
#define WED_HIFTXD_BASE28_L_ADDR                          0x00000390
#define WED_HIFTXD_BASE29_L_ADDR                          0x00000394
#define WED_HIFTXD_BASE30_L_ADDR                          0x00000398
#define WED_HIFTXD_BASE31_L_ADDR                          0x0000039C
#define WED_HIFTXD_BASE_00_07_H_ADDR                      0x000003A0
#define WED_HIFTXD_BASE_08_15_H_ADDR                      0x000003A4
#define WED_HIFTXD_BASE_16_23_H_ADDR                      0x000003A8
#define WED_HIFTXD_BASE_24_31_H_ADDR                      0x000003AC
#define WED_SCR0_ADDR                                     0x000003C0
#define WED_SCR1_ADDR                                     0x000003C4
#define WED_SCR2_ADDR                                     0x000003C8
#define WED_SCR3_ADDR                                     0x000003CC
#define WED_SCR4_ADDR                                     0x000003D0
#define WED_SCR5_ADDR                                     0x000003D4
#define WED_SCR6_ADDR                                     0x000003D8
#define WED_SCR7_ADDR                                     0x000003DC
#define WED_RX0_CTRL0_ADDR                                0x00000400
#define WED_RX0_CTRL1_ADDR                                0x00000404
#define WED_RX0_CTRL2_ADDR                                0x00000408
#define WED_RX0_CTRL3_ADDR                                0x0000040C
#define WED_RX1_CTRL0_ADDR                                0x00000410
#define WED_RX1_CTRL1_ADDR                                0x00000414
#define WED_RX1_CTRL2_ADDR                                0x00000418
#define WED_RX1_CTRL3_ADDR                                0x0000041C
#define WED_RX_BASE_PTR_0_ADDR                            0x00000420
#define WED_RX_MAX_CNT_0_ADDR                             0x00000424
#define WED_RX_CRX_IDX_0_ADDR                             0x00000428
#define WED_RX_DRX_IDX_0_ADDR                             0x0000042C
#define WED_RX_BASE_PTR_1_ADDR                            0x00000430
#define WED_RX_MAX_CNT_1_ADDR                             0x00000434
#define WED_RX_CRX_IDX_1_ADDR                             0x00000438
#define WED_RX_DRX_IDX_1_ADDR                             0x0000043C
#define WED_WPDMA_INT_STA_REC_ADDR                        0x00000500
#define WED_WPDMA_INT_TRIG_ADDR                           0x00000504
#define WED_WPDMA_GLO_CFG_ADDR                            0x00000508
#define WED_WPDMA_RST_IDX_ADDR                            0x0000050C
#define WED_WPDMA_BUS_CFG_ADDR                            0x00000510
#define WED_WPDMA_TX_DLY_CIDX_ADDR                        0x00000514
#define WED_WPDMA_CTRL_ADDR                               0x00000518
#define WED_WPDMA_FORCE_PROC_ADDR                         0x0000051C
#define WED_WPDMA_INT_CTRL_ADDR                           0x00000520
#define WED_WPDMA_INT_MSK_ADDR                            0x00000524
#define WED_WPDMA_INT_CLR_ADDR                            0x00000528
#define WED_WPDMA_INT_MON_ADDR                            0x0000052C
#define WED_WPDMA_INT_CTRL_TX_ADDR                        0x00000530
#define WED_WPDMA_INT_CTRL_RX_ADDR                        0x00000534
#define WED_WPDMA_INT_CTRL_TX_FREE_ADDR                   0x00000538
#define WED_WPDMA_SPR_ADDR                                0x0000053C
#define WED_PCIE_MSIS_TRIG_ADDR                           0x00000540
#define WED_PCIE_MSIS_REC_ADDR                            0x00000544
#define WED_PCIE_MSIS_CLR_ADDR                            0x00000548
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_ADDR                 0x0000054C
#define WED_PCIE_INTS_CLR_ADDR                            0x00000550
#define WED_PCIE_EP_INTS_CLR_ADDR                         0x00000554
#define WED_PCIE_CFG_ADDR_H_ADDR                          0x0000055C
#define WED_PCIE_CFG_ADDR_INTS_ADDR                       0x00000560
#define WED_PCIE_CFG_ADDR_INTM_ADDR                       0x00000564
#define WED_PCIE_CFG_ADDR_MSIS_ADDR                       0x00000568
#define WED_PCIE_CFG_ADDR_INTS_EP_ADDR                    0x0000056C
#define WED_PCIE_INTS_TRIG_ADDR                           0x00000570
#define WED_PCIE_INTS_REC_ADDR                            0x00000574
#define WED_PCIE_INTM_REC_ADDR                            0x00000578
#define WED_PCIE_INT_CTRL_ADDR                            0x0000057C
#define WED_WPDMA_CFG_ADDR_INTS_ADDR                      0x00000580
#define WED_WPDMA_CFG_ADDR_INTM_ADDR                      0x00000584
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_ADDR                  0x00000588
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_ADDR             0x0000058C
#define WED_WPDMA_CFG_ADDR_INT_H_ADDR                     0x00000590
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_ADDR             0x00000594
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_H_ADDR                0x00000598
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H_ADDR           0x0000059C
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_ADDR                  0x000005A0
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_ADDR             0x000005A4
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_H_ADDR                0x000005A8
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H_ADDR           0x000005AC
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_ADDR                  0x000005B0
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_H_ADDR                0x000005B4
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H_ADDR           0x000005B8
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_ADDR             0x000005BC
#define WED_WPDMA_TX0_MIB_ADDR                            0x000005C0
#define WED_WPDMA_TX1_MIB_ADDR                            0x000005C4
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H_ADDR           0x000005C8
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_ADDR                  0x000005CC
#define WED_WPDMA_TX_COHERENT_MIB_ADDR                    0x000005D0
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_H_ADDR                0x000005D4
#define WED_WPDMA_RX0_MIB_ADDR                            0x000005E0
#define WED_WPDMA_RX1_MIB_ADDR                            0x000005E4
#define WED_WPDMA_RX_COHERENT_MIB_ADDR                    0x000005F0
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_ADDR              0x000005F8
#define WED_WPDMA_TX0_CTRL0_ADDR                          0x00000600
#define WED_WPDMA_TX0_CTRL1_ADDR                          0x00000604
#define WED_WPDMA_TX0_CTRL2_ADDR                          0x00000608
#define WED_WPDMA_TX0_CTRL3_ADDR                          0x0000060C
#define WED_WPDMA_TX1_CTRL0_ADDR                          0x00000610
#define WED_WPDMA_TX1_CTRL1_ADDR                          0x00000614
#define WED_WPDMA_TX1_CTRL2_ADDR                          0x00000618
#define WED_WPDMA_TX1_CTRL3_ADDR                          0x0000061C
#define RRO_RX_D_DRV_MON_R0_WRAP_ADDR                     0x00000640
#define RRO_RX_D_DRV_MON_R1_WRAP_ADDR                     0x00000644
#define RRO_RX_D_DRV_MON_R2_WRAP_ADDR                     0x00000648
#define RRO_RX_D_DRV_MON0_ADDR                            0x0000064C
#define RRO_RX_D_DRV_MON1_ADDR                            0x00000650
#define RRO_RX_D_DRV_MON2_ADDR                            0x00000654
#define RRO_RX_D_DRV_ERR_COMP0_ADDR                       0x00000658
#define RRO_RX_D_DRV_ERR_COMP1_ADDR                       0x0000065C
#define RRO_RX_D_DRV_ERR_COMP2_ADDR                       0x00000660
#define RRO_RX_D_DRV_ERR_COMP3_ADDR                       0x00000664
#define RRO_RX_D_DRV_LEAGLE_START_ADDR                    0x00000668
#define RRO_RX_D_DRV_LEAGLE_END_ADDR                      0x0000066C
#define RRO_RX_PG_DRV_MON_R0_WRAP_ADDR                    0x00000670
#define RRO_RX_PG_DRV_MON_R1_WRAP_ADDR                    0x00000674
#define RRO_RX_PG_DRV_MON_R2_WRAP_ADDR                    0x00000678
#define RRO_RX_PG_DRV_MON0_ADDR                           0x0000067C
#define RRO_RX_PG_DRV_MON1_ADDR                           0x00000680
#define RRO_RX_PG_DRV_MON2_ADDR                           0x00000684
#define RRO_RX_PG_DRV_ERR_COMP0_ADDR                      0x00000688
#define RRO_RX_PG_DRV_ERR_COMP1_ADDR                      0x0000068C
#define RRO_RX_PG_DRV_ERR_COMP2_ADDR                      0x00000690
#define RRO_RX_PG_DRV_ERR_COMP3_ADDR                      0x00000694
#define RRO_RX_PG_DRV_LEAGLE_START_ADDR                   0x00000698
#define RRO_RX_PG_DRV_LEAGLE_END_ADDR                     0x0000069C
#define RRO_RX_D_DRV_ERR_SWITCH_ADDR_ADDR                 0x000006A0
#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR_ADDR                0x000006A4
#define RRO_RX_D_DRV_IN_MIB_ADDR                          0x000006A8
#define RRO_RX_D_DRV_OU_MIB_ADDR                          0x000006AC
#define RRO_RX_D_DRV_DESC_MIB_ADDR                        0x000006B0
#define RRO_RX_PG_DRV_IN_MIB_ADDR                         0x000006B4
#define RRO_RX_PG_DRV_OU_MIB_ADDR                         0x000006B8
#define RRO_RX_PG_DRV_DESC_MIB_ADDR                       0x000006BC
#define RRO_RX_D_DRV_LOOPBACK_INDEX_ADDR                  0x000006C0
#define WED_WPDMA_RX0_CTRL0_ADDR                          0x00000700
#define WED_WPDMA_RX0_CTRL1_ADDR                          0x00000704
#define WED_WPDMA_RX0_CTRL2_ADDR                          0x00000708
#define WED_WPDMA_RX0_CTRL3_ADDR                          0x0000070C
#define WED_WPDMA_RX1_CTRL0_ADDR                          0x00000710
#define WED_WPDMA_RX1_CTRL1_ADDR                          0x00000714
#define WED_WPDMA_RX1_CTRL2_ADDR                          0x00000718
#define WED_WPDMA_RX1_CTRL3_ADDR                          0x0000071C
#define WED_WPDMA_RX_D_INFO_ADDR                          0x00000758
#define WED_WDMA_TX0_BASE_ADDR                            0x00000800
#define WED_WDMA_TX0_CNT_ADDR                             0x00000804
#define WED_WDMA_TX0_CTX_IDX_ADDR                         0x00000808
#define WED_WDMA_TX0_DTX_IDX_ADDR                         0x0000080C
#define WED_WDMA_TX0_MIB_0_ADDR                           0x00000810
#define WED_WPDMA_RRO3_1_RX_D_RX0_BASE_ADDR               0x00000814
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_ADDR                0x00000818
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_ADDR            0x0000081C
#define WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_ADDR            0x00000820
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR                0x00000824
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_ADDR                0x00000828
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_ADDR           0x0000082C
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_ADDR           0x00000830
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDR      0x00000834
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H_ADDR    0x00000838
#define WED_WPDMA_RRO3_1_RX_D_RX0_MIB_ADDR                0x0000083C
#define WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_ADDR      0x00000840
#define WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB_ADDR        0x00000844
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR                0x00000848
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ADDR                0x0000084C
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR               0x00000850
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_ADDR        0x00000854
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_ADDR        0x00000858
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR          0x0000085C
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_ADDR            0x00000860
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDR    0x00000864
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H_ADDR  0x00000868
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ADDR            0x0000086C
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1_ADDR            0x00000870
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2_ADDR            0x00000874
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR            0x00000878
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4_ADDR            0x0000087C
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ADDR            0x00000880
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_ADDR                 0x00000884
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDR    0x00000888
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H_ADDR  0x0000088C
#define WED_WDMA_TX0_MIB_1_ADDR                           0x00000890
#define WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT_ADDR  0x00000894
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB_ADDR      0x000008B0
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB_ADDR      0x000008B4
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_ADDR           0x000008B8
#define WED_RRO_CTL_ADDR                                  0x000008FC
#define WED_WDMA_RX0_BASE_ADDR                            0x00000900
#define WED_WDMA_RX0_CNT_ADDR                             0x00000904
#define WED_WDMA_RX0_CRX_IDX_ADDR                         0x00000908
#define WED_WDMA_RX0_DRX_IDX_ADDR                         0x0000090C
#define WED_WDMA_RX1_BASE_ADDR                            0x00000910
#define WED_WDMA_RX1_CNT_ADDR                             0x00000914
#define WED_WDMA_RX1_CRX_IDX_ADDR                         0x00000918
#define WED_WDMA_RX1_DRX_IDX_ADDR                         0x0000091C
#define WED_WDMA_RX0_THRES_CFG_ADDR                       0x00000940
#define WED_WDMA_RX1_THRES_CFG_ADDR                       0x00000944
#define WED_WDMA_RX_PREF_CFG_ADDR                         0x00000950
#define WED_WDMA_RX_PREF_RX0_SIDX_ADDR                    0x00000954
#define WED_WDMA_RX_PREF_RX1_SIDX_ADDR                    0x00000958
#define WED_WDMA_RX_PREF_FIFO_CFG_ADDR                    0x0000095C
#define WED_WDMA_RX_CIDX_WR_CFG_ADDR                      0x00000960
#define WED_WDMA_PREF_MIB_COHERENT_CNT_ADDR               0x00000964
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_ADDR          0x00000970
#define WED_WDMA_TX_DRV_TIME_DELAY_CFG_ADDR               0x00000974
#define WED_WDMA_TX_DRV_TIME_DELAY_MON_ADDR               0x00000978
#define WED_WDMA_TX_DRV_ACCU_DELAY_CFG_ADDR               0x0000097C
#define WED_WDMA_TX_DRV_ACCU_DELAY_MON_ADDR               0x00000980
#define WED_WDMA_INFO_ADDR                                0x00000A00
#define WED_WDMA_GLO_CFG_ADDR                             0x00000A04
#define WED_WDMA_RST_IDX_ADDR                             0x00000A08
#define WED_WDMA_LOAD_IDX_ADDR                            0x00000A0C
#define WED_WDMA_LOAD_DRV_IDX_ADDR                        0x00000A10
#define WED_WDMA_LOAD_CRX_IDX_ADDR                        0x00000A14
#define WED_WDMA_SPR_ADDR                                 0x00000A1C
#define WED_WDMA_INT_STA_REC_ADDR                         0x00000A20
#define WED_WDMA_INT_CLR_ADDR                             0x00000A24
#define WED_WDMA_INT_TRIG_ADDR                            0x00000A28
#define WED_WDMA_INT_CTRL_ADDR                            0x00000A2C
#define WED_WDMA_INT_MON_ADDR                             0x00000A3C
#define WED_WDMA_TX_DRV_PMTR_1_ADDR                       0x00000A40
#define WED_WDMA_TX_DRV_PMTR_2_ADDR                       0x00000A44
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR               0x00000A48
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_2_ADDR               0x00000A4C
#define WED_WDMA_CFG_BASE_ADDR                            0x00000AA0
#define WED_WDMA_OFST0_ADDR                               0x00000AA4
#define WED_WDMA_OFST1_ADDR                               0x00000AA8
#define WED_WDMA_CFG_BASE_H_ADDR                          0x00000AAC
#define WED_WDMA_RX0_MIB_ADDR                             0x00000AE0
#define WED_WDMA_RX1_MIB_ADDR                             0x00000AE4
#define WED_WDMA_RX0_RECYCLE_MIB_ADDR                     0x00000AE8
#define WED_WDMA_RX1_RECYCLE_MIB_ADDR                     0x00000AEC
#define WED_WDMA_RX0_PROCESSED_MIB_ADDR                   0x00000AF0
#define WED_WDMA_RX1_PROCESSED_MIB_ADDR                   0x00000AF4
#define WED_WDMA_RX_COHERENT_MIB_ADDR                     0x00000AF8
#define WED_RTQM_GLO_CFG_ADDR                             0x00000B00
#define WED_RTQM_RST_ADDR                                 0x00000B04
#define WED_RTQM_IGRS_FIFO_CFG_ADDR                       0x00000B08
#define WED_RTQM_PFDBK_FIFO_CFG_ADDR                      0x00000B0C
#define WED_RTQM_PREF_FIFO_CFG_ADDR                       0x00000B10
#define WED_RTQM_IGRS0_CFG0_ADDR                          0x00000B14
#define WED_RTQM_IGRS0_CFG1_ADDR                          0x00000B18
#define WED_RTQM_IGRS0_I2HW_DMAD_CNT_ADDR                 0x00000B1C
#define WED_RTQM_IGRS0_I2H0_DMAD_CNT_ADDR                 0x00000B20
#define WED_RTQM_IGRS0_I2H1_DMAD_CNT_ADDR                 0x00000B24
#define WED_RTQM_IGRS0_I2HW_PKT_CNT_ADDR                  0x00000B28
#define WED_RTQM_IGRS0_I2H0_PKT_CNT_ADDR                  0x00000B2C
#define WED_RTQM_IGRS0_I2H1_PKT_CNT_ADDR                  0x00000B30
#define WED_RTQM_IGRS0_FDROP_CNT_ADDR                     0x00000B34
#define WED_RTQM_IGRS0_DBG_ADDR                           0x00000B38
#define WED_RTQM_IGRS1_CFG0_ADDR                          0x00000B3C
#define WED_RTQM_IGRS1_CFG1_ADDR                          0x00000B40
#define WED_RTQM_IGRS1_I2HW_DMAD_CNT_ADDR                 0x00000B44
#define WED_RTQM_IGRS1_I2H0_DMAD_CNT_ADDR                 0x00000B48
#define WED_RTQM_IGRS1_I2H1_DMAD_CNT_ADDR                 0x00000B4C
#define WED_RTQM_IGRS1_I2HW_PKT_CNT_ADDR                  0x00000B50
#define WED_RTQM_IGRS1_I2H0_PKT_CNT_ADDR                  0x00000B54
#define WED_RTQM_IGRS1_I2H1_PKT_CNT_ADDR                  0x00000B58
#define WED_RTQM_IGRS1_FDROP_CNT_ADDR                     0x00000B5C
#define WED_RTQM_IGRS1_DBG_ADDR                           0x00000B60
#define WED_RTQM_IGRS2_CFG0_ADDR                          0x00000B64
#define WED_RTQM_IGRS2_CFG1_ADDR                          0x00000B68
#define WED_RTQM_IGRS2_I2HW_DMAD_CNT_ADDR                 0x00000B6C
#define WED_RTQM_IGRS2_I2H0_DMAD_CNT_ADDR                 0x00000B70
#define WED_RTQM_IGRS2_I2H1_DMAD_CNT_ADDR                 0x00000B74
#define WED_RTQM_IGRS2_I2HW_PKT_CNT_ADDR                  0x00000B78
#define WED_RTQM_IGRS2_I2H0_PKT_CNT_ADDR                  0x00000B7C
#define WED_RTQM_IGRS2_I2H1_PKT_CNT_ADDR                  0x00000B80
#define WED_RTQM_IGRS2_FDROP_CNT_ADDR                     0x00000B84
#define WED_RTQM_IGRS2_DBG_ADDR                           0x00000B88
#define WED_RTQM_IGRS3_CFG0_ADDR                          0x00000B8C
#define WED_RTQM_IGRS3_CFG1_ADDR                          0x00000B90
#define WED_RTQM_IGRS3_I2HW_DMAD_CNT_ADDR                 0x00000B94
#define WED_RTQM_IGRS3_I2H0_DMAD_CNT_ADDR                 0x00000B98
#define WED_RTQM_IGRS3_I2H1_DMAD_CNT_ADDR                 0x00000B9C
#define WED_RTQM_IGRS3_I2HW_PKT_CNT_ADDR                  0x00000BA0
#define WED_RTQM_IGRS3_I2H0_PKT_CNT_ADDR                  0x00000BA4
#define WED_RTQM_IGRS3_I2H1_PKT_CNT_ADDR                  0x00000BA8
#define WED_RTQM_IGRS3_FDROP_CNT_ADDR                     0x00000BAC
#define WED_RTQM_IGRS3_DBG_ADDR                           0x00000BB0
#define WED_RTQM_EGRS_WEIGHT_ADDR                         0x00000BB4
#define WED_RTQM_ENQ_CFG0_ADDR                            0x00000BB8
#define WED_RTQM_ENQ_I2Q_DMAD_CNT_ADDR                    0x00000BBC
#define WED_RTQM_ENQ_I2N_DMAD_CNT_ADDR                    0x00000BC0
#define WED_RTQM_ENQ_I2Q_PKT_CNT_ADDR                     0x00000BC4
#define WED_RTQM_ENQ_I2N_PKT_CNT_ADDR                     0x00000BC8
#define WED_RTQM_ENQ_USED_ENTRY_CNT_ADDR                  0x00000BCC
#define WED_RTQM_ENQ_ERR_CNT_ADDR                         0x00000BD0
#define WED_RTQM_DEQ_CFG0_ADDR                            0x00000BD4
#define WED_RTQM_DEQ_DQ_DMAD_CNT_ADDR                     0x00000BD8
#define WED_RTQM_DEQ_Q2I_DMAD_CNT_ADDR                    0x00000BDC
#define WED_RTQM_DEQ_DQ_PKT_CNT_ADDR                      0x00000BE0
#define WED_RTQM_DEQ_Q2I_PKT_CNT_ADDR                     0x00000BE4
#define WED_RTQM_DEQ_USED_PFDBK_CNT_ADDR                  0x00000BE8
#define WED_RTQM_DEQ_ERR_CNT_ADDR                         0x00000BEC
#define WED_RTQM_DEQ_DBG_ADDR                             0x00000BF0
#define WED_RTQM_ERR_STS_ADDR                             0x00000BF4
#define WED_RTQM_QUEUE_CFG0_ADDR                          0x00000BF8
#define WED_RTQM_QUEUE_ACC_CFG_ADDR                       0x00000BFC
#define WED_RTQM_DELAY_CFG_ADDR                           0x00000C00
#define WED_RTQM_DELAY_DBG_ADDR                           0x00000C04
#define WED_RTQM_QUEUE_ACC_WR0_ADDR                       0x00000C10
#define WED_RTQM_QUEUE_ACC_WR1_ADDR                       0x00000C14
#define WED_RTQM_QUEUE_ACC_WR2_ADDR                       0x00000C18
#define WED_RTQM_QUEUE_ACC_WR3_ADDR                       0x00000C1C
#define WED_RTQM_QUEUE_ACC_WR4_ADDR                       0x00000C20
#define WED_RTQM_QUEUE_ACC_RD0_ADDR                       0x00000C24
#define WED_RTQM_QUEUE_ACC_RD1_ADDR                       0x00000C28
#define WED_RTQM_QUEUE_ACC_RD2_ADDR                       0x00000C2C
#define WED_RTQM_QUEUE_ACC_RD3_ADDR                       0x00000C30
#define WED_RTQM_QUEUE_ACC_RD4_ADDR                       0x00000C34
#define WED_RTQM_QUEUE_CNT_ADDR                           0x00000C38
#define WED_RTQM_DBG_CFG_ADDR                             0x00000C3C
#define WED_RTQM_QUEUE_DBG_DEPTH_ADDR                     0x00000C40
#define WED_RTQM_QUEUE_DBG_CFG_ADDR                       0x00000C44
#define WED_RTQM_QUEUE_DBG_THRES_CFG_ADDR                 0x00000C48
#define WED_RTQM_QUEUE_DBG_THRES_ADDR                     0x00000C4C
#define WED_RTQM_QUEUE_PEAK_MON_ADDR                      0x00000C50
#define WED_RTQM_DBG_MON0_ADDR                            0x00000C60
#define WED_RTQM_DBG_MON1_ADDR                            0x00000C64
#define WED_RTQM_DBG_MON2_ADDR                            0x00000C68
#define WED_RTQM_DBG_MON3_ADDR                            0x00000C6C
#define WED_RTQM_DBG_ENQ_DATA0_ADDR                       0x00000C70
#define WED_RTQM_DBG_ENQ_DATA1_ADDR                       0x00000C74
#define WED_RTQM_DBG_ENQ_DATA2_ADDR                       0x00000C78
#define WED_RTQM_DBG_ENQ_DATA3_ADDR                       0x00000C7C
#define WED_RTQM_AGE_CFG0_ADDR                            0x00000C88
#define WED_RTQM_AGE_CFG1_ADDR                            0x00000C90
#define WED_RTQM_AGE_FIFO_CFG_ADDR                        0x00000C94
#define WED_RTQM_AGE_PKT_CNT_ADDR                         0x00000C98
#define WED_RTQM_PFDBK_CNT_ADDR                           0x00000C9C
#define WED_TX_BM_DYN_ALLOC_TH_ADDR                       0x00000D00
#define WED_TX_BM_DYN_ALLOC_CFG_ADDR                      0x00000D04
#define WED_TX_BM_ADD_BASE_ADDR                           0x00000D08
#define WED_TX_BM_ADD_PTR_ADDR                            0x00000D0C
#define WED_TX_BM_TOTAL_DMAD_IDX_ADDR                     0x00000D10
#define WED_TX_BM_ADD_BASE_H_ADDR                         0x00000D14
#define WED_RX_BM_RX_DMAD_ADDR                            0x00000D80
#define WED_RX_BM_BASE_ADDR                               0x00000D84
#define WED_RX_BM_INIT_PTR_ADDR                           0x00000D88
#define WED_RX_BM_PTR_ADDR                                0x00000D8C
#define WED_RX_BM_STS_ADDR                                0x00000D94
#define WED_RX_BM_INTF2_ADDR                              0x00000D98
#define WED_RX_BM_INTF_ADDR                               0x00000D9C
#define WED_RX_BM_RANGE_CFG_ADDR                          0x00000DA0
#define WED_RX_BM_INIT_HEAD_PTR_ADDR                      0x00000DA4
#define WED_RX_BM_ERR_STS_ADDR                            0x00000DA8
#define WED_RX_BM_DYN_ALLOC_TH_ADDR                       0x00000DB4
#define WED_RX_BM_DYN_ALLOC_CFG_ADDR                      0x00000DB8
#define WED_RX_BM_ADD_BASE_ADDR                           0x00000DBC
#define WED_RX_BM_ADD_PTR_ADDR                            0x00000DC0
#define WED_RX_BM_TOTAL_DMAD_IDX_ADDR                     0x00000DC4
#define WED_RX_BM_ADD_BASE_H_ADDR                         0x00000DC8
#define RRO_MSDU_PG_RING0_CIDX_CFG0_ADDR                  0x00000DE8
#define RRO_MSDU_PG_RING0_CIDX_CFG1_ADDR                  0x00000DEC
#define RRO_MSDU_PG_RING1_CIDX_CFG0_ADDR                  0x00000DF0
#define RRO_MSDU_PG_RING1_CIDX_CFG1_ADDR                  0x00000DF4
#define RRO_MSDU_PG_RING2_CIDX_CFG0_ADDR                  0x00000DF8
#define RRO_MSDU_PG_RING2_CIDX_CFG1_ADDR                  0x00000DFC
#define RRO_IND_CMD_0_SIGNATURE_ADDR                      0x00000E00
#define IND_CMD_0_CTRL_0_ADDR                             0x00000E04
#define IND_CMD_0_CTRL_1_ADDR                             0x00000E08
#define IND_CMD_0_CTRL_2_ADDR                             0x00000E0C
#define RRO_CONF_0_ADDR                                   0x00000E10
#define RRO_CONF_1_ADDR                                   0x00000E14
#define ADDR_ELEM_CONF_0_ADDR                             0x00000E18
#define ADDR_ELEM_CONF_1_ADDR                             0x00000E1C
#define ADDR_ELEM_BASE_TBL_CONF_ADDR                      0x00000E20
#define ADDR_ELEM_BASE_TBL_WDATA_ADDR                     0x00000E24
#define ADDR_ELEM_BASE_TBL_RDATA_ADDR                     0x00000E28
#define RRO_MSDU_PG_RING2_CFG1_ADDR                       0x00000E2C
#define PN_CONF_0_ADDR                                    0x00000E30
#define PN_CONF_WDATA_L_ADDR                              0x00000E34
#define PN_CONF_WDATA_M_ADDR                              0x00000E38
#define PN_CONF_RDATA_L_ADDR                              0x00000E3C
#define PN_CONF_RDATA_M_ADDR                              0x00000E40
#define RRO_MSDU_PG_RING0_DIDX_CFG0_ADDR                  0x00000E44
#define RRO_MSDU_PG_RING0_DIDX_CFG1_ADDR                  0x00000E48
#define RRO_MSDU_PG_RING1_DIDX_CFG0_ADDR                  0x00000E4C
#define RRO_MSDU_PG_RING1_DIDX_CFG1_ADDR                  0x00000E50
#define RRO_MSDU_PG_RING2_DIDX_CFG0_ADDR                  0x00000E54
#define RRO_MSDU_PG_RING2_DIDX_CFG1_ADDR                  0x00000E58
#define RRO_MSDU_PG_0_CTRL0_ADDR                          0x00000E5C
#define RRO_MSDU_PG_0_CTRL1_ADDR                          0x00000E60
#define RRO_MSDU_PG_0_CTRL2_ADDR                          0x00000E64
#define RRO_MSDU_PG_1_CTRL0_ADDR                          0x00000E68
#define RRO_MSDU_PG_1_CTRL1_ADDR                          0x00000E6C
#define RRO_MSDU_PG_1_CTRL2_ADDR                          0x00000E70
#define RRO_MSDU_PG_2_CTRL0_ADDR                          0x00000E74
#define RRO_MSDU_PG_2_CTRL1_ADDR                          0x00000E78
#define RRO_MSDU_PG_2_CTRL2_ADDR                          0x00000E7C
#define RRO_RX_D_RX0_BASE_ADDR                            0x00000E80
#define RRO_RX_D_RX0_CNT_ADDR                             0x00000E84
#define RRO_RX_D_RX0_CRX_IDX_ADDR                         0x00000E88
#define RRO_RX_D_RX0_DRX_IDX_ADDR                         0x00000E8C
#define RRO_RX_D_RX1_BASE_ADDR                            0x00000E90
#define RRO_RX_D_RX1_CNT_ADDR                             0x00000E94
#define RRO_RX_D_RX1_CRX_IDX_ADDR                         0x00000E98
#define RRO_RX_D_RX1_DRX_IDX_ADDR                         0x00000E9C
#define RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR                0x00000EA0
#define RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR                0x00000EA4
#define RRO_RX_D_RING_CFG_ADDR_2_ADDR                     0x00000EA8
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR                  0x00000EAC
#define RRO_PG_BM_RX_DMAD_ADDR                            0x00000EB0
#define RRO_PG_BM_BASE_ADDR                               0x00000EB4
#define RRO_PG_BM_INIT_PTR_ADDR                           0x00000EB8
#define RRO_PG_BM_PTR_ADDR                                0x00000EBC
#define RRO_PG_BM_ADD_BASE_H_ADDR                         0x00000EC0
#define RRO_PG_BM_STS_ADDR                                0x00000EC4
#define RRO_PG_BM_RANGE_CFG_ADDR                          0x00000EC8
#define RRO_PG_BM_INTF_ADDR                               0x00000ECC
#define RRO_PG_BM_ERR_STS_ADDR                            0x00000ED0
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR                  0x00000ED4
#define RRO_PG_BM_DYN_ALLOC_TH_ADDR                       0x00000ED8
#define RRO_PG_BM_DYN_ALLOC_CFG_ADDR                      0x00000EDC
#define RRO_PG_BM_ADD_BASE_ADDR                           0x00000EE0
#define RRO_PG_BM_ADD_PTR_ADDR                            0x00000EE4
#define RRO_PG_BM_TOTAL_DMAD_IDX_ADDR                     0x00000EE8
#define WED_WPDMA_INT_CTRL_RRO_RX_ADDR                    0x00000EEC
#define RRO_PG_BM_INIT_HEAD_PTR_ADDR                      0x00000EF0
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR               0x00000EF4
#define RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR                0x00000EF8
#define RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR                0x00000EFC
#define WED_RRO_RX_HW_STS_ADDR                            0x00000F00
#define WED_RRO_RX_DBG0_ADDR                              0x00000F04
#define WED_RRO_RX_DBG1_ADDR                              0x00000F08
#define WED_RRO_RX_DBG2_ADDR                              0x00000F0C
#define WED_RRO_RX_DBG3_ADDR                              0x00000F10
#define WED_RRO_RX_DBG4_ADDR                              0x00000F14
#define WED_RX_IND_CMD_CNT0_ADDR                          0x00000F20
#define WED_RX_IND_CMD_CNT1_ADDR                          0x00000F24
#define WED_RX_IND_CMD_CNT2_ADDR                          0x00000F28
#define WED_RX_IND_CMD_CNT3_ADDR                          0x00000F2C
#define WED_RX_IND_CMD_CNT4_ADDR                          0x00000F30
#define WED_RX_IND_CMD_CNT5_ADDR                          0x00000F34
#define WED_RX_IND_CMD_CNT6_ADDR                          0x00000F38
#define WED_RX_IND_CMD_CNT7_ADDR                          0x00000F3C
#define WED_RX_IND_CMD_CNT8_ADDR                          0x00000F40
#define WED_RX_IND_CMD_CNT9_ADDR                          0x00000F44
#define WED_RX_ADDR_ELEM_CNT0_ADDR                        0x00000F48
#define WED_RX_ADDR_ELEM_CNT1_ADDR                        0x00000F4C
#define WED_RX_ADDR_ELEM_CNT2_ADDR                        0x00000F50
#define WED_RX_ADDR_ELEM_CNT3_ADDR                        0x00000F54
#define WED_RX_ADDR_ELEM_CNT4_ADDR                        0x00000F58
#define WED_RX_MSDU_PG_CNT1_ADDR                          0x00000F5C
#define WED_RX_MSDU_PG_CNT2_ADDR                          0x00000F60
#define WED_RX_MSDU_PG_CNT3_ADDR                          0x00000F64
#define WED_RX_MSDU_PG_CNT4_ADDR                          0x00000F68
#define WED_RX_MSDU_PG_CNT5_ADDR                          0x00000F6C
#define WED_RX_PN_CHK_CNT0_ADDR                           0x00000F70
#define WED_RX_IND_CMD_AXI0_ADDR                          0x00000F74
#define WED_RX_ADDR_ELEM_AXI0_ADDR                        0x00000F78
#define WED_RX_ADDR_ELEM_AXI1_ADDR                        0x00000F7C
#define WED_RX_ADDR_ELEM_AXI2_ADDR                        0x00000F80
#define WED_RX_ADDR_ELEM_AXI3_ADDR                        0x00000F84
#define WED_RX_ADDR_ELEM_AXI4_ADDR                        0x00000F88
#define WED_RX_MSDU_PG_AXI0_ADDR                          0x00000F8C
#define PN_CONF_1_ADDR                                    0x00000F90
#define PN_CONF_2_ADDR                                    0x00000F94
#define DONT_USE_BEHIND_THIS_ADDR_ADDR                    0x00000FBC

/* =====================================================================================

  ---WED_ID (0x15010000 + 0x000)---

    MODE[31..0]                  - (RO) WED ID Mode

 =====================================================================================*/
#define WED_ID_MODE_ADDR                                  WED_ID_ADDR
#define WED_ID_MODE_MASK                                  0xFFFFFFFF                // MODE[31..0]
#define WED_ID_MODE_SHFT                                  0

/* =====================================================================================

  ---WED_REV_ID (0x15010000 + 0x004)---

    ECO[7..0]                    - (RO) WED revision ECO
    BRANCH[15..8]                - (RO) WED revision branch
    MINOR[27..16]                - (RO) WED revision minor
    MAJOR[31..28]                - (RO) WED revision major

 =====================================================================================*/
#define WED_REV_ID_MAJOR_ADDR                             WED_REV_ID_ADDR
#define WED_REV_ID_MAJOR_MASK                             0xF0000000                // MAJOR[31..28]
#define WED_REV_ID_MAJOR_SHFT                             28
#define WED_REV_ID_MINOR_ADDR                             WED_REV_ID_ADDR
#define WED_REV_ID_MINOR_MASK                             0x0FFF0000                // MINOR[27..16]
#define WED_REV_ID_MINOR_SHFT                             16
#define WED_REV_ID_BRANCH_ADDR                            WED_REV_ID_ADDR
#define WED_REV_ID_BRANCH_MASK                            0x0000FF00                // BRANCH[15..8]
#define WED_REV_ID_BRANCH_SHFT                            8
#define WED_REV_ID_ECO_ADDR                               WED_REV_ID_ADDR
#define WED_REV_ID_ECO_MASK                               0x000000FF                // ECO[7..0]
#define WED_REV_ID_ECO_SHFT                               0

/* =====================================================================================

  ---WED_MOD_RST (0x15010000 + 0x008)---

    TX_BM[0]                     - (A0) Write 1 to reset WED Tx Buffer Manager and Tx TKID Manager, then this bit will return to 0 automatically.
    RX_BM[1]                     - (A0) Write 1 to reset WED Rx Buffer Manager, then this bit will return to 0 automatically.
    RX_PG_BM[2]                  - (A0) Write 1 to reset WED Rx Page Buffer Manager, then this bit will return to 0 automatically.
    RRO_RX_TO_PG[3]              - (A0) Write 1 to reset WED Rx RRO path from IND_CMD to INFO_PAGE/PN_CHK , then this bit will return to 0 automatically.
                                     To reset the rest of the Rx RRO path (RX_PG_BM/RX_BM/…), must reset following modules 1 by 1
    TX_FREE_AGT[4]               - (A0) Write 1 to reset WED Tx Free Agent, then this bit will return to 0 automatically.
    TX_TKID_ALI[5]               - (A0) Write 1 to reset WED Tx TKID Aligner, then this bit will return to 0 automatically.
    RESERVED6[6]                 - (RO) Reserved bits
    WPDMA_RRO3_1_RX_D_DRV[7]     - (A0) Write 1 to reset WED WPDMA Rx  RRO3.1 Driver Engine, then this bit will return to 0 automatically.
    WPDMA_TX_DRV[8]              - (A0) Write 1 to reset WED WPDMA Tx Driver Engine, then this bit will return to 0 automatically.
    WPDMA_RX_DRV[9]              - (A0) Write 1 to reset WED WPDMA Rx Control Path Driver Engine, then this bit will return to 0 automatically.
    WPDMA_RX_D_DRV[10]           - (A0) Write 1 to reset WED WPDMA Rx Data Path Driver Engine, then this bit will return to 0 automatically.
    WPDMA_INT_AGT[11]            - (A0) Write 1 to reset WED WPDMA Interrupt Agent, then this bit will return to 0 automatically.
    WED_TX_DMA[12]               - (A0) Write 1 to reset WED Tx DMA, then this bit will return to 0 automatically.
    WED_RX_DMA[13]               - (A0) Write 1 to reset WED Rx DMA, then this bit will return to 0 automatically.
    RESERVED14[15..14]           - (RO) Reserved bits
    WDMA_TX_DRV[16]              - (A0) Write 1 to reset WED WDMA Tx Driver Engine, then this bit will return to 0 automatically.
    WDMA_RX_DRV[17]              - (A0) Write 1 to reset WED WDMA Rx Driver Engine, then this bit will return to 0 automatically.
    RESERVED18[18]               - (RO) Reserved bits
    WDMA_INT_AGT[19]             - (A0) Write 1 to reset WED WDMA Interrupt Agent, then this bit will return to 0 automatically.
    RX_RRO_QM[20]                - (A0) Write 1 to reset WED Rx Re-Order Queue Manager, then this bit will return to 0 automatically.
    RX_ROUTE_QM[21]              - (A0) Write 1 to reset WED Rx Route Queue Manager, then this bit will return to 0 automatically.
    TX_PAO[22]                   - (A0) Write 1 to reset WED Tx PAO, then this bit will return to 0 automatically.
    RESERVED23[30..23]           - (RO) Reserved bits
    WED[31]                      - (A0) Whole WED reset.

 =====================================================================================*/
#define WED_MOD_RST_WED_ADDR                              WED_MOD_RST_ADDR
#define WED_MOD_RST_WED_MASK                              0x80000000                // WED[31]
#define WED_MOD_RST_WED_SHFT                              31
#define WED_MOD_RST_TX_PAO_ADDR                           WED_MOD_RST_ADDR
#define WED_MOD_RST_TX_PAO_MASK                           0x00400000                // TX_PAO[22]
#define WED_MOD_RST_TX_PAO_SHFT                           22
#define WED_MOD_RST_RX_ROUTE_QM_ADDR                      WED_MOD_RST_ADDR
#define WED_MOD_RST_RX_ROUTE_QM_MASK                      0x00200000                // RX_ROUTE_QM[21]
#define WED_MOD_RST_RX_ROUTE_QM_SHFT                      21
#define WED_MOD_RST_RX_RRO_QM_ADDR                        WED_MOD_RST_ADDR
#define WED_MOD_RST_RX_RRO_QM_MASK                        0x00100000                // RX_RRO_QM[20]
#define WED_MOD_RST_RX_RRO_QM_SHFT                        20
#define WED_MOD_RST_WDMA_INT_AGT_ADDR                     WED_MOD_RST_ADDR
#define WED_MOD_RST_WDMA_INT_AGT_MASK                     0x00080000                // WDMA_INT_AGT[19]
#define WED_MOD_RST_WDMA_INT_AGT_SHFT                     19
#define WED_MOD_RST_WDMA_RX_DRV_ADDR                      WED_MOD_RST_ADDR
#define WED_MOD_RST_WDMA_RX_DRV_MASK                      0x00020000                // WDMA_RX_DRV[17]
#define WED_MOD_RST_WDMA_RX_DRV_SHFT                      17
#define WED_MOD_RST_WDMA_TX_DRV_ADDR                      WED_MOD_RST_ADDR
#define WED_MOD_RST_WDMA_TX_DRV_MASK                      0x00010000                // WDMA_TX_DRV[16]
#define WED_MOD_RST_WDMA_TX_DRV_SHFT                      16
#define WED_MOD_RST_WED_RX_DMA_ADDR                       WED_MOD_RST_ADDR
#define WED_MOD_RST_WED_RX_DMA_MASK                       0x00002000                // WED_RX_DMA[13]
#define WED_MOD_RST_WED_RX_DMA_SHFT                       13
#define WED_MOD_RST_WED_TX_DMA_ADDR                       WED_MOD_RST_ADDR
#define WED_MOD_RST_WED_TX_DMA_MASK                       0x00001000                // WED_TX_DMA[12]
#define WED_MOD_RST_WED_TX_DMA_SHFT                       12
#define WED_MOD_RST_WPDMA_INT_AGT_ADDR                    WED_MOD_RST_ADDR
#define WED_MOD_RST_WPDMA_INT_AGT_MASK                    0x00000800                // WPDMA_INT_AGT[11]
#define WED_MOD_RST_WPDMA_INT_AGT_SHFT                    11
#define WED_MOD_RST_WPDMA_RX_D_DRV_ADDR                   WED_MOD_RST_ADDR
#define WED_MOD_RST_WPDMA_RX_D_DRV_MASK                   0x00000400                // WPDMA_RX_D_DRV[10]
#define WED_MOD_RST_WPDMA_RX_D_DRV_SHFT                   10
#define WED_MOD_RST_WPDMA_RX_DRV_ADDR                     WED_MOD_RST_ADDR
#define WED_MOD_RST_WPDMA_RX_DRV_MASK                     0x00000200                // WPDMA_RX_DRV[9]
#define WED_MOD_RST_WPDMA_RX_DRV_SHFT                     9
#define WED_MOD_RST_WPDMA_TX_DRV_ADDR                     WED_MOD_RST_ADDR
#define WED_MOD_RST_WPDMA_TX_DRV_MASK                     0x00000100                // WPDMA_TX_DRV[8]
#define WED_MOD_RST_WPDMA_TX_DRV_SHFT                     8
#define WED_MOD_RST_WPDMA_RRO3_1_RX_D_DRV_ADDR            WED_MOD_RST_ADDR
#define WED_MOD_RST_WPDMA_RRO3_1_RX_D_DRV_MASK            0x00000080                // WPDMA_RRO3_1_RX_D_DRV[7]
#define WED_MOD_RST_WPDMA_RRO3_1_RX_D_DRV_SHFT            7
#define WED_MOD_RST_TX_TKID_ALI_ADDR                      WED_MOD_RST_ADDR
#define WED_MOD_RST_TX_TKID_ALI_MASK                      0x00000020                // TX_TKID_ALI[5]
#define WED_MOD_RST_TX_TKID_ALI_SHFT                      5
#define WED_MOD_RST_TX_FREE_AGT_ADDR                      WED_MOD_RST_ADDR
#define WED_MOD_RST_TX_FREE_AGT_MASK                      0x00000010                // TX_FREE_AGT[4]
#define WED_MOD_RST_TX_FREE_AGT_SHFT                      4
#define WED_MOD_RST_RRO_RX_TO_PG_ADDR                     WED_MOD_RST_ADDR
#define WED_MOD_RST_RRO_RX_TO_PG_MASK                     0x00000008                // RRO_RX_TO_PG[3]
#define WED_MOD_RST_RRO_RX_TO_PG_SHFT                     3
#define WED_MOD_RST_RX_PG_BM_ADDR                         WED_MOD_RST_ADDR
#define WED_MOD_RST_RX_PG_BM_MASK                         0x00000004                // RX_PG_BM[2]
#define WED_MOD_RST_RX_PG_BM_SHFT                         2
#define WED_MOD_RST_RX_BM_ADDR                            WED_MOD_RST_ADDR
#define WED_MOD_RST_RX_BM_MASK                            0x00000002                // RX_BM[1]
#define WED_MOD_RST_RX_BM_SHFT                            1
#define WED_MOD_RST_TX_BM_ADDR                            WED_MOD_RST_ADDR
#define WED_MOD_RST_TX_BM_MASK                            0x00000001                // TX_BM[0]
#define WED_MOD_RST_TX_BM_SHFT                            0

/* =====================================================================================

  ---WED_CTRL (0x15010000 + 0x00C)---

    WPDMA_INT_AGT_EN[0]          - (RW) Enable WiFi card interrupt agent, to intercept the PCIe interrupt from WiFi WPDMA.
    WPDMA_INT_AGT_BUSY[1]        - (RO) WiFi card WPDMA interrupt agent is busy.
    WDMA_INT_AGT_EN[2]           - (RW) Enable Ethernet interrupt agent, to intercept the Ethernet interrupt from Frame Engine WDMA.
    WDMA_INT_AGT_BUSY[3]         - (RO) Ethernet WDMA interrupt agent is busy.
    RESERVED4[4]                 - (RO) Reserved bits
    WED_RX_IND_CMD_EN[5]         - (RW) Enable WED Rx IND_CMD.
    WED_RX_PG_BM_EN[6]           - (RW) Enable WED Rx Page Buffer Manager.
    WED_RX_PG_BM_BUSY[7]         - (RO) WED Rx Page Buffer Manager is busy.
    WED_TX_BM_EN[8]              - (RW) Enable WED Tx Buffer Manager and Tx TKID Manager.
    WED_TX_BM_BUSY[9]            - (RO) WED Tx Buffer Manager is busy.
    WED_TX_FREE_AGT_EN[10]       - (RW) Enable WED Tx Free Agent.
    WED_TX_FREE_AGT_BUSY[11]     - (RO) WED Tx Free Agent is busy.
    WED_RX_BM_EN[12]             - (RW) Enable WED Rx Buffer Manager.
    WED_RX_BM_BUSY[13]           - (RO) WED Rx Buffer Manager is busy.
    RESERVED14[14]               - (RO) Reserved bits
    WED_RX_RRO_QM_BUSY[15]       - (RO) WED Rx Re-Order Queue Manager is busy.
    WED_RX_ROUTE_QM_EN[16]       - (RW) Enable WED Rx Route Queue Manager.
    WED_RX_ROUTE_QM_BUSY[17]     - (RO) WED Rx Route Queue Manager is busy.
    WED_TX_TKID_BUSY[18]         - (RO) WED Tx TKID Manager is busy.
    RESERVED19[19]               - (RO) Reserved bits
    WED_TX_TKID_ALI_EN[20]       - (RW) Enable WED Tx Free Aligner.
    WED_TX_TKID_ALI_BUSY[21]     - (RO) WED Tx Free Aligner is busy.
    WED_TX_PAO_EN[22]            - (RW) Enable WED Tx PAO.
    WED_TX_PAO_BUSY[23]          - (RO) WED Tx PAO is busy.
    FINAL_DIDX_RD[24]            - (RW) Final Read DMA Index in ISR
    ETH_DMAD_FMT[25]             - (RW) ETH_DMAD format selection
                                     Must be 0x1, not support legacy 128b format anymore.
    RESERVED26[27..26]           - (RO) Reserved bits
    MIB_RD_CLR[28]               - (RW) MIB counter clear mode
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_CTRL_MIB_RD_CLR_ADDR                          WED_CTRL_ADDR
#define WED_CTRL_MIB_RD_CLR_MASK                          0x10000000                // MIB_RD_CLR[28]
#define WED_CTRL_MIB_RD_CLR_SHFT                          28
#define WED_CTRL_ETH_DMAD_FMT_ADDR                        WED_CTRL_ADDR
#define WED_CTRL_ETH_DMAD_FMT_MASK                        0x02000000                // ETH_DMAD_FMT[25]
#define WED_CTRL_ETH_DMAD_FMT_SHFT                        25
#define WED_CTRL_FINAL_DIDX_RD_ADDR                       WED_CTRL_ADDR
#define WED_CTRL_FINAL_DIDX_RD_MASK                       0x01000000                // FINAL_DIDX_RD[24]
#define WED_CTRL_FINAL_DIDX_RD_SHFT                       24
#define WED_CTRL_WED_TX_PAO_BUSY_ADDR                     WED_CTRL_ADDR
#define WED_CTRL_WED_TX_PAO_BUSY_MASK                     0x00800000                // WED_TX_PAO_BUSY[23]
#define WED_CTRL_WED_TX_PAO_BUSY_SHFT                     23
#define WED_CTRL_WED_TX_PAO_EN_ADDR                       WED_CTRL_ADDR
#define WED_CTRL_WED_TX_PAO_EN_MASK                       0x00400000                // WED_TX_PAO_EN[22]
#define WED_CTRL_WED_TX_PAO_EN_SHFT                       22
#define WED_CTRL_WED_TX_TKID_ALI_BUSY_ADDR                WED_CTRL_ADDR
#define WED_CTRL_WED_TX_TKID_ALI_BUSY_MASK                0x00200000                // WED_TX_TKID_ALI_BUSY[21]
#define WED_CTRL_WED_TX_TKID_ALI_BUSY_SHFT                21
#define WED_CTRL_WED_TX_TKID_ALI_EN_ADDR                  WED_CTRL_ADDR
#define WED_CTRL_WED_TX_TKID_ALI_EN_MASK                  0x00100000                // WED_TX_TKID_ALI_EN[20]
#define WED_CTRL_WED_TX_TKID_ALI_EN_SHFT                  20
#define WED_CTRL_WED_TX_TKID_BUSY_ADDR                    WED_CTRL_ADDR
#define WED_CTRL_WED_TX_TKID_BUSY_MASK                    0x00040000                // WED_TX_TKID_BUSY[18]
#define WED_CTRL_WED_TX_TKID_BUSY_SHFT                    18
#define WED_CTRL_WED_RX_ROUTE_QM_BUSY_ADDR                WED_CTRL_ADDR
#define WED_CTRL_WED_RX_ROUTE_QM_BUSY_MASK                0x00020000                // WED_RX_ROUTE_QM_BUSY[17]
#define WED_CTRL_WED_RX_ROUTE_QM_BUSY_SHFT                17
#define WED_CTRL_WED_RX_ROUTE_QM_EN_ADDR                  WED_CTRL_ADDR
#define WED_CTRL_WED_RX_ROUTE_QM_EN_MASK                  0x00010000                // WED_RX_ROUTE_QM_EN[16]
#define WED_CTRL_WED_RX_ROUTE_QM_EN_SHFT                  16
#define WED_CTRL_WED_RX_RRO_QM_BUSY_ADDR                  WED_CTRL_ADDR
#define WED_CTRL_WED_RX_RRO_QM_BUSY_MASK                  0x00008000                // WED_RX_RRO_QM_BUSY[15]
#define WED_CTRL_WED_RX_RRO_QM_BUSY_SHFT                  15
#define WED_CTRL_WED_RX_BM_BUSY_ADDR                      WED_CTRL_ADDR
#define WED_CTRL_WED_RX_BM_BUSY_MASK                      0x00002000                // WED_RX_BM_BUSY[13]
#define WED_CTRL_WED_RX_BM_BUSY_SHFT                      13
#define WED_CTRL_WED_RX_BM_EN_ADDR                        WED_CTRL_ADDR
#define WED_CTRL_WED_RX_BM_EN_MASK                        0x00001000                // WED_RX_BM_EN[12]
#define WED_CTRL_WED_RX_BM_EN_SHFT                        12
#define WED_CTRL_WED_TX_FREE_AGT_BUSY_ADDR                WED_CTRL_ADDR
#define WED_CTRL_WED_TX_FREE_AGT_BUSY_MASK                0x00000800                // WED_TX_FREE_AGT_BUSY[11]
#define WED_CTRL_WED_TX_FREE_AGT_BUSY_SHFT                11
#define WED_CTRL_WED_TX_FREE_AGT_EN_ADDR                  WED_CTRL_ADDR
#define WED_CTRL_WED_TX_FREE_AGT_EN_MASK                  0x00000400                // WED_TX_FREE_AGT_EN[10]
#define WED_CTRL_WED_TX_FREE_AGT_EN_SHFT                  10
#define WED_CTRL_WED_TX_BM_BUSY_ADDR                      WED_CTRL_ADDR
#define WED_CTRL_WED_TX_BM_BUSY_MASK                      0x00000200                // WED_TX_BM_BUSY[9]
#define WED_CTRL_WED_TX_BM_BUSY_SHFT                      9
#define WED_CTRL_WED_TX_BM_EN_ADDR                        WED_CTRL_ADDR
#define WED_CTRL_WED_TX_BM_EN_MASK                        0x00000100                // WED_TX_BM_EN[8]
#define WED_CTRL_WED_TX_BM_EN_SHFT                        8
#define WED_CTRL_WED_RX_PG_BM_BUSY_ADDR                   WED_CTRL_ADDR
#define WED_CTRL_WED_RX_PG_BM_BUSY_MASK                   0x00000080                // WED_RX_PG_BM_BUSY[7]
#define WED_CTRL_WED_RX_PG_BM_BUSY_SHFT                   7
#define WED_CTRL_WED_RX_PG_BM_EN_ADDR                     WED_CTRL_ADDR
#define WED_CTRL_WED_RX_PG_BM_EN_MASK                     0x00000040                // WED_RX_PG_BM_EN[6]
#define WED_CTRL_WED_RX_PG_BM_EN_SHFT                     6
#define WED_CTRL_WED_RX_IND_CMD_EN_ADDR                   WED_CTRL_ADDR
#define WED_CTRL_WED_RX_IND_CMD_EN_MASK                   0x00000020                // WED_RX_IND_CMD_EN[5]
#define WED_CTRL_WED_RX_IND_CMD_EN_SHFT                   5
#define WED_CTRL_WDMA_INT_AGT_BUSY_ADDR                   WED_CTRL_ADDR
#define WED_CTRL_WDMA_INT_AGT_BUSY_MASK                   0x00000008                // WDMA_INT_AGT_BUSY[3]
#define WED_CTRL_WDMA_INT_AGT_BUSY_SHFT                   3
#define WED_CTRL_WDMA_INT_AGT_EN_ADDR                     WED_CTRL_ADDR
#define WED_CTRL_WDMA_INT_AGT_EN_MASK                     0x00000004                // WDMA_INT_AGT_EN[2]
#define WED_CTRL_WDMA_INT_AGT_EN_SHFT                     2
#define WED_CTRL_WPDMA_INT_AGT_BUSY_ADDR                  WED_CTRL_ADDR
#define WED_CTRL_WPDMA_INT_AGT_BUSY_MASK                  0x00000002                // WPDMA_INT_AGT_BUSY[1]
#define WED_CTRL_WPDMA_INT_AGT_BUSY_SHFT                  1
#define WED_CTRL_WPDMA_INT_AGT_EN_ADDR                    WED_CTRL_ADDR
#define WED_CTRL_WPDMA_INT_AGT_EN_MASK                    0x00000001                // WPDMA_INT_AGT_EN[0]
#define WED_CTRL_WPDMA_INT_AGT_EN_SHFT                    0

/* =====================================================================================

  ---WED_AXI_CTRL (0x15010000 + 0x010)---

    CSR_CG_DISABLE[0]            - (RW) clock gating disable for AXI bus inside WED
    W_BUSY[1]                    - (RO) write channel busy signal
    R_BUSY[2]                    - (RO) read channel busy signal
    CSR_ERRMID_SET_RIRQ[3]       - (RO) csr_errmid_set_rirq
    CSR_ERRMID_SET_BIRQ[4]       - (RO) csr_errmid_set_birq
    UP0_NO_DUMMY_READ_EN[5]      - (RW) Enable WED bus upsizer0 no dummy read
    UP1_NO_DUMMY_READ_EN[6]      - (RW) Enable WED bus upsizer1 no dummy read
    RESERVED7[7]                 - (RO) Reserved bits
    SL0_AWTHRES[9..8]            - (RW)  xxx
    SL0_ARTHRES[11..10]          - (RW)  xxx
    UP0_WRAP_ERR_FLAG[12]        - (RO) Detects wrap error
    UP1_WRAP_ERR_FLAG[13]        - (RO) Detects wrap error
    RESERVED14[15..14]           - (RO) Reserved bits
    MI0_QOS_ON[16]               - (RW) Enable QoS function
    MI0_OUTSTD_EXTEND_EN[17]     - (RW) Auto adds extra outstanding for QoS command
    MI0_LOCK_ERR_FLAG[18]        - (RO) Detects lock error
    MI0_CTRL_UPD[19]             - (RO) Master interface control signal updated status
    MI1_QOS_ON[20]               - (RW) Enable QoS function
    MI1_OUTSTD_EXTEND_EN[21]     - (RW) Auto adds extra outstanding for QoS command
    MI1_LOCK_ERR_FLAG[22]        - (RO) Detects lock error
    MI1_CTRL_UPD[23]             - (RO) Master interface control signal updated status
    MI2_QOS_ON[24]               - (RW) Enable QoS function
    MI2_OUTSTD_EXTEND_EN[25]     - (RW) Auto adds extra outstanding for QoS command
    MI2_LOCK_ERR_FLAG[26]        - (RO) Detects lock error
    MI2_CTRL_UPD[27]             - (RO) Master interface control signal updated status
    MI3_QOS_ON[28]               - (RW) Enable QoS function
    MI3_OUTSTD_EXTEND_EN[29]     - (RW) Auto adds extra outstanding for QoS command
    MI3_LOCK_ERR_FLAG[30]        - (RO) Detects lock error
    MI3_CTRL_UPD[31]             - (RO) Master interface control signal updated status

 =====================================================================================*/
#define WED_AXI_CTRL_MI3_CTRL_UPD_ADDR                    WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI3_CTRL_UPD_MASK                    0x80000000                // MI3_CTRL_UPD[31]
#define WED_AXI_CTRL_MI3_CTRL_UPD_SHFT                    31
#define WED_AXI_CTRL_MI3_LOCK_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI3_LOCK_ERR_FLAG_MASK               0x40000000                // MI3_LOCK_ERR_FLAG[30]
#define WED_AXI_CTRL_MI3_LOCK_ERR_FLAG_SHFT               30
#define WED_AXI_CTRL_MI3_OUTSTD_EXTEND_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI3_OUTSTD_EXTEND_EN_MASK            0x20000000                // MI3_OUTSTD_EXTEND_EN[29]
#define WED_AXI_CTRL_MI3_OUTSTD_EXTEND_EN_SHFT            29
#define WED_AXI_CTRL_MI3_QOS_ON_ADDR                      WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI3_QOS_ON_MASK                      0x10000000                // MI3_QOS_ON[28]
#define WED_AXI_CTRL_MI3_QOS_ON_SHFT                      28
#define WED_AXI_CTRL_MI2_CTRL_UPD_ADDR                    WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI2_CTRL_UPD_MASK                    0x08000000                // MI2_CTRL_UPD[27]
#define WED_AXI_CTRL_MI2_CTRL_UPD_SHFT                    27
#define WED_AXI_CTRL_MI2_LOCK_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI2_LOCK_ERR_FLAG_MASK               0x04000000                // MI2_LOCK_ERR_FLAG[26]
#define WED_AXI_CTRL_MI2_LOCK_ERR_FLAG_SHFT               26
#define WED_AXI_CTRL_MI2_OUTSTD_EXTEND_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI2_OUTSTD_EXTEND_EN_MASK            0x02000000                // MI2_OUTSTD_EXTEND_EN[25]
#define WED_AXI_CTRL_MI2_OUTSTD_EXTEND_EN_SHFT            25
#define WED_AXI_CTRL_MI2_QOS_ON_ADDR                      WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI2_QOS_ON_MASK                      0x01000000                // MI2_QOS_ON[24]
#define WED_AXI_CTRL_MI2_QOS_ON_SHFT                      24
#define WED_AXI_CTRL_MI1_CTRL_UPD_ADDR                    WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI1_CTRL_UPD_MASK                    0x00800000                // MI1_CTRL_UPD[23]
#define WED_AXI_CTRL_MI1_CTRL_UPD_SHFT                    23
#define WED_AXI_CTRL_MI1_LOCK_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI1_LOCK_ERR_FLAG_MASK               0x00400000                // MI1_LOCK_ERR_FLAG[22]
#define WED_AXI_CTRL_MI1_LOCK_ERR_FLAG_SHFT               22
#define WED_AXI_CTRL_MI1_OUTSTD_EXTEND_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI1_OUTSTD_EXTEND_EN_MASK            0x00200000                // MI1_OUTSTD_EXTEND_EN[21]
#define WED_AXI_CTRL_MI1_OUTSTD_EXTEND_EN_SHFT            21
#define WED_AXI_CTRL_MI1_QOS_ON_ADDR                      WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI1_QOS_ON_MASK                      0x00100000                // MI1_QOS_ON[20]
#define WED_AXI_CTRL_MI1_QOS_ON_SHFT                      20
#define WED_AXI_CTRL_MI0_CTRL_UPD_ADDR                    WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI0_CTRL_UPD_MASK                    0x00080000                // MI0_CTRL_UPD[19]
#define WED_AXI_CTRL_MI0_CTRL_UPD_SHFT                    19
#define WED_AXI_CTRL_MI0_LOCK_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI0_LOCK_ERR_FLAG_MASK               0x00040000                // MI0_LOCK_ERR_FLAG[18]
#define WED_AXI_CTRL_MI0_LOCK_ERR_FLAG_SHFT               18
#define WED_AXI_CTRL_MI0_OUTSTD_EXTEND_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI0_OUTSTD_EXTEND_EN_MASK            0x00020000                // MI0_OUTSTD_EXTEND_EN[17]
#define WED_AXI_CTRL_MI0_OUTSTD_EXTEND_EN_SHFT            17
#define WED_AXI_CTRL_MI0_QOS_ON_ADDR                      WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_MI0_QOS_ON_MASK                      0x00010000                // MI0_QOS_ON[16]
#define WED_AXI_CTRL_MI0_QOS_ON_SHFT                      16
#define WED_AXI_CTRL_UP1_WRAP_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_UP1_WRAP_ERR_FLAG_MASK               0x00002000                // UP1_WRAP_ERR_FLAG[13]
#define WED_AXI_CTRL_UP1_WRAP_ERR_FLAG_SHFT               13
#define WED_AXI_CTRL_UP0_WRAP_ERR_FLAG_ADDR               WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_UP0_WRAP_ERR_FLAG_MASK               0x00001000                // UP0_WRAP_ERR_FLAG[12]
#define WED_AXI_CTRL_UP0_WRAP_ERR_FLAG_SHFT               12
#define WED_AXI_CTRL_SL0_ARTHRES_ADDR                     WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_SL0_ARTHRES_MASK                     0x00000C00                // SL0_ARTHRES[11..10]
#define WED_AXI_CTRL_SL0_ARTHRES_SHFT                     10
#define WED_AXI_CTRL_SL0_AWTHRES_ADDR                     WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_SL0_AWTHRES_MASK                     0x00000300                // SL0_AWTHRES[9..8]
#define WED_AXI_CTRL_SL0_AWTHRES_SHFT                     8
#define WED_AXI_CTRL_UP1_NO_DUMMY_READ_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_UP1_NO_DUMMY_READ_EN_MASK            0x00000040                // UP1_NO_DUMMY_READ_EN[6]
#define WED_AXI_CTRL_UP1_NO_DUMMY_READ_EN_SHFT            6
#define WED_AXI_CTRL_UP0_NO_DUMMY_READ_EN_ADDR            WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_UP0_NO_DUMMY_READ_EN_MASK            0x00000020                // UP0_NO_DUMMY_READ_EN[5]
#define WED_AXI_CTRL_UP0_NO_DUMMY_READ_EN_SHFT            5
#define WED_AXI_CTRL_CSR_ERRMID_SET_BIRQ_ADDR             WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_CSR_ERRMID_SET_BIRQ_MASK             0x00000010                // CSR_ERRMID_SET_BIRQ[4]
#define WED_AXI_CTRL_CSR_ERRMID_SET_BIRQ_SHFT             4
#define WED_AXI_CTRL_CSR_ERRMID_SET_RIRQ_ADDR             WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_CSR_ERRMID_SET_RIRQ_MASK             0x00000008                // CSR_ERRMID_SET_RIRQ[3]
#define WED_AXI_CTRL_CSR_ERRMID_SET_RIRQ_SHFT             3
#define WED_AXI_CTRL_R_BUSY_ADDR                          WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_R_BUSY_MASK                          0x00000004                // R_BUSY[2]
#define WED_AXI_CTRL_R_BUSY_SHFT                          2
#define WED_AXI_CTRL_W_BUSY_ADDR                          WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_W_BUSY_MASK                          0x00000002                // W_BUSY[1]
#define WED_AXI_CTRL_W_BUSY_SHFT                          1
#define WED_AXI_CTRL_CSR_CG_DISABLE_ADDR                  WED_AXI_CTRL_ADDR
#define WED_AXI_CTRL_CSR_CG_DISABLE_MASK                  0x00000001                // CSR_CG_DISABLE[0]
#define WED_AXI_CTRL_CSR_CG_DISABLE_SHFT                  0

/* =====================================================================================

  ---WED_LOOPBACK (0x15010000 + 0x014)---

    en[0]                        - (RW) loopbackup function enable
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_LOOPBACK_en_ADDR                              WED_LOOPBACK_ADDR
#define WED_LOOPBACK_en_MASK                              0x00000001                // en[0]
#define WED_LOOPBACK_en_SHFT                              0

/* =====================================================================================

  ---WED_CTRL2 (0x15010000 + 0x01C)---

    COHERENT_WAIT_MAXIMUM[7..0]  - (RW) The maximum count value for coherent wait counter in each module. If descriptor coherent problem occurs, module will wait until counter reach the maximum value (coherent_wait_maximum) and then reread the descriptor.
    RESERVED8[23..8]             - (RO) Reserved bits
    GLO_AWULTRA[25..24]          - (RW) Global AXI write ultra control force value
    RESERVED26[26]               - (RO) Reserved bits
    GLO_AWULTRA_FORCE[27]        - (RW) Global AXI write ultra force control, replace all master's awultra by GLO_AWULTRA
    GLO_ARULTRA[29..28]          - (RW) Global AXI read ultra control force value
    RESERVED30[30]               - (RO) Reserved bits
    GLO_ARULTRA_FORCE[31]        - (RW) Global AXI read ultra force control, replace all master's arultra by GLO_ARULTRA

 =====================================================================================*/
#define WED_CTRL2_GLO_ARULTRA_FORCE_ADDR                  WED_CTRL2_ADDR
#define WED_CTRL2_GLO_ARULTRA_FORCE_MASK                  0x80000000                // GLO_ARULTRA_FORCE[31]
#define WED_CTRL2_GLO_ARULTRA_FORCE_SHFT                  31
#define WED_CTRL2_GLO_ARULTRA_ADDR                        WED_CTRL2_ADDR
#define WED_CTRL2_GLO_ARULTRA_MASK                        0x30000000                // GLO_ARULTRA[29..28]
#define WED_CTRL2_GLO_ARULTRA_SHFT                        28
#define WED_CTRL2_GLO_AWULTRA_FORCE_ADDR                  WED_CTRL2_ADDR
#define WED_CTRL2_GLO_AWULTRA_FORCE_MASK                  0x08000000                // GLO_AWULTRA_FORCE[27]
#define WED_CTRL2_GLO_AWULTRA_FORCE_SHFT                  27
#define WED_CTRL2_GLO_AWULTRA_ADDR                        WED_CTRL2_ADDR
#define WED_CTRL2_GLO_AWULTRA_MASK                        0x03000000                // GLO_AWULTRA[25..24]
#define WED_CTRL2_GLO_AWULTRA_SHFT                        24
#define WED_CTRL2_COHERENT_WAIT_MAXIMUM_ADDR              WED_CTRL2_ADDR
#define WED_CTRL2_COHERENT_WAIT_MAXIMUM_MASK              0x000000FF                // COHERENT_WAIT_MAXIMUM[7..0]
#define WED_CTRL2_COHERENT_WAIT_MAXIMUM_SHFT              0

/* =====================================================================================

  ---WED_EX_INT_STA (0x15010000 + 0x020)---

    RESERVED0[0]                 - (RO) Reserved bits
    RRO3_1_RX_DIDX_FIN[1]        - (W1C) WED finished the checking of all RRO3.1 Rx data ring DMAD's Magic Count behind DMAD_index
    TX_DIDX_FIN0[2]              - (W1C) WED finished the checking of all Tx data ring 0 DMAD's DDONE behind DMAD_index
    TX_DIDX_FIN1[3]              - (W1C) WED finished the checking of all Tx data ring 1 DMAD's DDONE behind DMAD_index
    RESERVED4[7..4]              - (RO) Reserved bits
    TX_BM_LTH[8]                 - (W1C) WED Tx BM high available DMAD buffer threshold met, notify SW to sub (retrieve) Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) < l_buf_th ]
    TX_BM_HTH[9]                 - (W1C) WED Tx BM low available DMAD buffer threshold met, notify SW to add Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) > h_buf_th ]
    TX_TKID_LTH[10]              - (W1C) WED Tx TKID owned available free TKID amount is lower than low threshold value
    TX_TKID_HTH[11]              - (W1C) WED Tx TKID owned available free TKID amount is higher than high threshold value
    RX_PG_BM_L_BUF[12]           - (W1C) WED Rx PG BM high available DMAD buffer threshold met, notify SW to sub (retrieve) Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) < l_buf_th ]
    RX_PG_BM_H_BUF[13]           - (W1C) WED Rx PG BM low available DMAD buffer threshold met, notify SW to add Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) > h_buf_th ]
    RX_BM_L_BUF[14]              - (W1C) WED Rx BM high available DMAD buffer threshold met, notify SW to sub (retrieve) Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) < l_buf_th ]
    RX_BM_H_BUF[15]              - (W1C) WED Rx BM low available DMAD buffer threshold met, notify SW to add Rx DMAD buffer
                                     [ Sw_alloc - (tail_idx - head_idx) > h_buf_th ]
    RESERVED16[17..16]           - (RO) Reserved bits
    RX_DRV_COHERENT[18]          - (W1C) WED Rx Driver encounter coherence problem. The DMAD read from WDMA RxRing has DDONE=0 problem.
    RESERVED19[19]               - (RO) Reserved bits
    TX_FREE_DIDX_FIN0[20]        - (W1C) WED finished the checking of Tx free ring 0 all DMAD's DDONE behind DMAD_index
    TX_FREE_DIDX_FIN1[21]        - (W1C) WED finished the checking of Tx free ring 1 all DMAD's DDONE behind DMAD_index
    RESERVED22[23..22]           - (RO) Reserved bits
    RX_DRV_DMAD_RECYCLE[24]      - (W1C) WED RxDriver decides to recycle the descriptor
    RESERVED25[27..25]           - (RO) Reserved bits
    BUS_MON[28]                  - (W1C) WED bus monitor hit
    RESERVED29[29]               - (RO) Reserved bits
    ERR_MON[30]                  - (W1C) There are error events recorded in WED_ERR_MON
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WED_EX_INT_STA_ERR_MON_ADDR                       WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_ERR_MON_MASK                       0x40000000                // ERR_MON[30]
#define WED_EX_INT_STA_ERR_MON_SHFT                       30
#define WED_EX_INT_STA_BUS_MON_ADDR                       WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_BUS_MON_MASK                       0x10000000                // BUS_MON[28]
#define WED_EX_INT_STA_BUS_MON_SHFT                       28
#define WED_EX_INT_STA_RX_DRV_DMAD_RECYCLE_ADDR           WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_DRV_DMAD_RECYCLE_MASK           0x01000000                // RX_DRV_DMAD_RECYCLE[24]
#define WED_EX_INT_STA_RX_DRV_DMAD_RECYCLE_SHFT           24
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN1_ADDR             WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN1_MASK             0x00200000                // TX_FREE_DIDX_FIN1[21]
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN1_SHFT             21
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN0_ADDR             WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN0_MASK             0x00100000                // TX_FREE_DIDX_FIN0[20]
#define WED_EX_INT_STA_TX_FREE_DIDX_FIN0_SHFT             20
#define WED_EX_INT_STA_RX_DRV_COHERENT_ADDR               WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_DRV_COHERENT_MASK               0x00040000                // RX_DRV_COHERENT[18]
#define WED_EX_INT_STA_RX_DRV_COHERENT_SHFT               18
#define WED_EX_INT_STA_RX_BM_H_BUF_ADDR                   WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_BM_H_BUF_MASK                   0x00008000                // RX_BM_H_BUF[15]
#define WED_EX_INT_STA_RX_BM_H_BUF_SHFT                   15
#define WED_EX_INT_STA_RX_BM_L_BUF_ADDR                   WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_BM_L_BUF_MASK                   0x00004000                // RX_BM_L_BUF[14]
#define WED_EX_INT_STA_RX_BM_L_BUF_SHFT                   14
#define WED_EX_INT_STA_RX_PG_BM_H_BUF_ADDR                WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_PG_BM_H_BUF_MASK                0x00002000                // RX_PG_BM_H_BUF[13]
#define WED_EX_INT_STA_RX_PG_BM_H_BUF_SHFT                13
#define WED_EX_INT_STA_RX_PG_BM_L_BUF_ADDR                WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RX_PG_BM_L_BUF_MASK                0x00001000                // RX_PG_BM_L_BUF[12]
#define WED_EX_INT_STA_RX_PG_BM_L_BUF_SHFT                12
#define WED_EX_INT_STA_TX_TKID_HTH_ADDR                   WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_TKID_HTH_MASK                   0x00000800                // TX_TKID_HTH[11]
#define WED_EX_INT_STA_TX_TKID_HTH_SHFT                   11
#define WED_EX_INT_STA_TX_TKID_LTH_ADDR                   WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_TKID_LTH_MASK                   0x00000400                // TX_TKID_LTH[10]
#define WED_EX_INT_STA_TX_TKID_LTH_SHFT                   10
#define WED_EX_INT_STA_TX_BM_HTH_ADDR                     WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_BM_HTH_MASK                     0x00000200                // TX_BM_HTH[9]
#define WED_EX_INT_STA_TX_BM_HTH_SHFT                     9
#define WED_EX_INT_STA_TX_BM_LTH_ADDR                     WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_BM_LTH_MASK                     0x00000100                // TX_BM_LTH[8]
#define WED_EX_INT_STA_TX_BM_LTH_SHFT                     8
#define WED_EX_INT_STA_TX_DIDX_FIN1_ADDR                  WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_DIDX_FIN1_MASK                  0x00000008                // TX_DIDX_FIN1[3]
#define WED_EX_INT_STA_TX_DIDX_FIN1_SHFT                  3
#define WED_EX_INT_STA_TX_DIDX_FIN0_ADDR                  WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_TX_DIDX_FIN0_MASK                  0x00000004                // TX_DIDX_FIN0[2]
#define WED_EX_INT_STA_TX_DIDX_FIN0_SHFT                  2
#define WED_EX_INT_STA_RRO3_1_RX_DIDX_FIN_ADDR            WED_EX_INT_STA_ADDR
#define WED_EX_INT_STA_RRO3_1_RX_DIDX_FIN_MASK            0x00000002                // RRO3_1_RX_DIDX_FIN[1]
#define WED_EX_INT_STA_RRO3_1_RX_DIDX_FIN_SHFT            1

/* =====================================================================================

  ---WED_EX_INT_MSK (0x15010000 + 0x028)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[0]
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_MSK_EN_ADDR                            WED_EX_INT_MSK_ADDR
#define WED_EX_INT_MSK_EN_MASK                            0xFFFFFFFF                // EN[31..0]
#define WED_EX_INT_MSK_EN_SHFT                            0

/* =====================================================================================

  ---WED_EX_INT_MSK1 (0x15010000 + 0x02c)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[1]
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_MSK1_EN_ADDR                           WED_EX_INT_MSK1_ADDR
#define WED_EX_INT_MSK1_EN_MASK                           0xFFFFFFFF                // EN[31..0]
#define WED_EX_INT_MSK1_EN_SHFT                           0

/* =====================================================================================

  ---WED_EX_INT_MSK2 (0x15010000 + 0x030)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[2]
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_MSK2_EN_ADDR                           WED_EX_INT_MSK2_ADDR
#define WED_EX_INT_MSK2_EN_MASK                           0xFFFFFFFF                // EN[31..0]
#define WED_EX_INT_MSK2_EN_SHFT                           0

/* =====================================================================================

  ---WED_EX_INT_MSK3 (0x15010000 + 0x034)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[3]
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_MSK3_EN_ADDR                           WED_EX_INT_MSK3_ADDR
#define WED_EX_INT_MSK3_EN_MASK                           0xFFFFFFFF                // EN[31..0]
#define WED_EX_INT_MSK3_EN_SHFT                           0

/* =====================================================================================

  ---WED_EX_INT_STA_POST_MSK0 (0x15010000 + 0x040)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     WED_EX_INT_STA_POST_MSK# = WED_EX_INT_STA & WED_EX_INT_MSK#
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_STA_POST_MSK0_EVENT_ADDR               WED_EX_INT_STA_POST_MSK0_ADDR
#define WED_EX_INT_STA_POST_MSK0_EVENT_MASK               0xFFFFFFFF                // EVENT[31..0]
#define WED_EX_INT_STA_POST_MSK0_EVENT_SHFT               0

/* =====================================================================================

  ---WED_EX_INT_STA_POST_MSK1 (0x15010000 + 0x044)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_STA_POST_MSK1_EVENT_ADDR               WED_EX_INT_STA_POST_MSK1_ADDR
#define WED_EX_INT_STA_POST_MSK1_EVENT_MASK               0xFFFFFFFF                // EVENT[31..0]
#define WED_EX_INT_STA_POST_MSK1_EVENT_SHFT               0

/* =====================================================================================

  ---WED_EX_INT_STA_POST_MSK2 (0x15010000 + 0x048)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_STA_POST_MSK2_EVENT_ADDR               WED_EX_INT_STA_POST_MSK2_ADDR
#define WED_EX_INT_STA_POST_MSK2_EVENT_MASK               0xFFFFFFFF                // EVENT[31..0]
#define WED_EX_INT_STA_POST_MSK2_EVENT_SHFT               0

/* =====================================================================================

  ---WED_EX_INT_STA_POST_MSK3 (0x15010000 + 0x04c)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_EX_INT_STA

 =====================================================================================*/
#define WED_EX_INT_STA_POST_MSK3_EVENT_ADDR               WED_EX_INT_STA_POST_MSK3_ADDR
#define WED_EX_INT_STA_POST_MSK3_EVENT_MASK               0xFFFFFFFF                // EVENT[31..0]
#define WED_EX_INT_STA_POST_MSK3_EVENT_SHFT               0

/* =====================================================================================

  ---WED_IRQ_MON (0x15010000 + 0x050)---

    PCIE_IRQ[3..0]               - (RO) PCIe# INTx IRQ signal
    PCIE_MSI_IRQ[7..4]           - (RO) PCIe# MSI IRQ signal
    WDMA_IRQ[11..8]              - (RO) Frame Engine WDMA# IRQ signal
    RESERVED12[15..12]           - (RO) Reserved bits
    CONNSYS_IRQ[19..16]          - (RO) CONNSYS WFDMA# IRQ signal
    RESERVED20[27..20]           - (RO) Reserved bits
    WED_IRQ[31..28]              - (RO) WED IRQ signal

 =====================================================================================*/
#define WED_IRQ_MON_WED_IRQ_ADDR                          WED_IRQ_MON_ADDR
#define WED_IRQ_MON_WED_IRQ_MASK                          0xF0000000                // WED_IRQ[31..28]
#define WED_IRQ_MON_WED_IRQ_SHFT                          28
#define WED_IRQ_MON_CONNSYS_IRQ_ADDR                      WED_IRQ_MON_ADDR
#define WED_IRQ_MON_CONNSYS_IRQ_MASK                      0x000F0000                // CONNSYS_IRQ[19..16]
#define WED_IRQ_MON_CONNSYS_IRQ_SHFT                      16
#define WED_IRQ_MON_WDMA_IRQ_ADDR                         WED_IRQ_MON_ADDR
#define WED_IRQ_MON_WDMA_IRQ_MASK                         0x00000F00                // WDMA_IRQ[11..8]
#define WED_IRQ_MON_WDMA_IRQ_SHFT                         8
#define WED_IRQ_MON_PCIE_MSI_IRQ_ADDR                     WED_IRQ_MON_ADDR
#define WED_IRQ_MON_PCIE_MSI_IRQ_MASK                     0x000000F0                // PCIE_MSI_IRQ[7..4]
#define WED_IRQ_MON_PCIE_MSI_IRQ_SHFT                     4
#define WED_IRQ_MON_PCIE_IRQ_ADDR                         WED_IRQ_MON_ADDR
#define WED_IRQ_MON_PCIE_IRQ_MASK                         0x0000000F                // PCIE_IRQ[3..0]
#define WED_IRQ_MON_PCIE_IRQ_SHFT                         0

/* =====================================================================================

  ---WED_ERR_MON (0x15010000 + 0x054)---

    TF_LEN_ERR[0]                - (W1C) Tx free notify byte count mismatch with token count
    RESERVED1[1]                 - (RO) Reserved bits
    TX_TKID_FREE_ALREADY_FREE[2] - (W1C) There is a Tx free Token_ID which is already free
    RESERVED3[3]                 - (RO) Reserved bits
    TF_TKID_FIFO_INVLD[4]        - (W1C) There is a Tx free Token_ID FIFO output a word without valid Token_ID. (all not belongs to Ethernet)
    RESERVED5[6..5]              - (RO) Reserved bits
    TX_PAO_ERR[7]                - (W1C) WED Tx PAO error flag for debug
    RESERVED8[9..8]              - (RO) Reserved bits
    TX_BM_FREE_AT_EMPTY[10]      - (W1C) WED Tx BM DMAD free when DMAD FIFO is already empty, all should have been free/returned already.
    TX_BM_DMAD_RD_ERR[11]        - (W1C) WED Tx BM DMAD read condition error: AXI read [less/more] then request / AXI read in at dmad_full / AXI DMAD read over tail_idx
    RX_BM_FREE_AT_EMPTY[12]      - (W1C) WED Rx BM DMAD free when DMAD FIFO is already empty, all should have been free/returned already.
    RX_BM_DMAD_RD_ERR[13]        - (W1C) WED Rx BM DMAD read condition error: AXI read [less/more] then request / AXI read in at dmad_full / AXI DMAD read over tail_idx
    RESERVED14[15..14]           - (RO) Reserved bits
    RX_DRV_R_RESP_ERR[16]        - (W1C) WED Rx Driver module AXI read response error
    RX_DRV_W_RESP_ERR[17]        - (W1C) WED Rx Driver module AXI write response error
    RESERVED18[18]               - (RO) Reserved bits
    RX_DRV_INTI_WDMA_ENABLED[19] - (W1C) WED Rx Driver finds that WDMA Rx has already been enabled when RX Driver module is initializing WDMA RxRing.  This should not happened, check the SW programming sequence.
    RX_DRV_BM_DMAD_COHERENT[20]  - (W1C) The DMAD which RxDriver received from Buffer Management has DDONE=1 problem.(Should not happened)
    RX_DRV_BM_DMAD_ERROR[21]     - (W1C) The DMAD which RxDriver received from Buffer Management has wrong format(DDONE=1 or SDP0=0 or SDL0=0)
    TX_DMA_R_RESP_ERR[22]        - (W1C) WED Tx DMA AXI read response error
    TX_DMA_W_RESP_ERR[23]        - (W1C) WED Tx DMA AXI write response error
    WPDMA_RRO3_1_RX_D_DRV_ERR[24] - (W1C) WED WPDMA Rx RRO3.1 Data Driver Error.
                                     Register WED_WPDMA_RRO3_1_RX_D_ERR_STS shows detail error status.
                                     Should clear WED_WPDMA_RRO3_1_RX_D_ERR_STS first before clear this register field.
    RX_DRV_GET_BM_DMAD_SKIP[25]  - (W1C) WED RxDriver decides to skip the process that getting descriptor from BM due to BM cannot provide DMAD to RxDriver for certain time and HW counter reach the threshold (thus RxDriver will not increasing CRX too).
    RESERVED26[28..26]           - (RO) Reserved bits
    RX_ROUTE_QM_ERR[29]          - (W1C) WED Rx Route QM Error.
                                     Register WED_RTQM_ERR_STS shows detail error status.
                                     Should clear WED_RTQM_ERR_STS first before clear this register field.
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_ERR_MON_RX_ROUTE_QM_ERR_ADDR                  WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_ROUTE_QM_ERR_MASK                  0x20000000                // RX_ROUTE_QM_ERR[29]
#define WED_ERR_MON_RX_ROUTE_QM_ERR_SHFT                  29
#define WED_ERR_MON_RX_DRV_GET_BM_DMAD_SKIP_ADDR          WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_GET_BM_DMAD_SKIP_MASK          0x02000000                // RX_DRV_GET_BM_DMAD_SKIP[25]
#define WED_ERR_MON_RX_DRV_GET_BM_DMAD_SKIP_SHFT          25
#define WED_ERR_MON_WPDMA_RRO3_1_RX_D_DRV_ERR_ADDR        WED_ERR_MON_ADDR
#define WED_ERR_MON_WPDMA_RRO3_1_RX_D_DRV_ERR_MASK        0x01000000                // WPDMA_RRO3_1_RX_D_DRV_ERR[24]
#define WED_ERR_MON_WPDMA_RRO3_1_RX_D_DRV_ERR_SHFT        24
#define WED_ERR_MON_TX_DMA_W_RESP_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_DMA_W_RESP_ERR_MASK                0x00800000                // TX_DMA_W_RESP_ERR[23]
#define WED_ERR_MON_TX_DMA_W_RESP_ERR_SHFT                23
#define WED_ERR_MON_TX_DMA_R_RESP_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_DMA_R_RESP_ERR_MASK                0x00400000                // TX_DMA_R_RESP_ERR[22]
#define WED_ERR_MON_TX_DMA_R_RESP_ERR_SHFT                22
#define WED_ERR_MON_RX_DRV_BM_DMAD_ERROR_ADDR             WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_BM_DMAD_ERROR_MASK             0x00200000                // RX_DRV_BM_DMAD_ERROR[21]
#define WED_ERR_MON_RX_DRV_BM_DMAD_ERROR_SHFT             21
#define WED_ERR_MON_RX_DRV_BM_DMAD_COHERENT_ADDR          WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_BM_DMAD_COHERENT_MASK          0x00100000                // RX_DRV_BM_DMAD_COHERENT[20]
#define WED_ERR_MON_RX_DRV_BM_DMAD_COHERENT_SHFT          20
#define WED_ERR_MON_RX_DRV_INTI_WDMA_ENABLED_ADDR         WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_INTI_WDMA_ENABLED_MASK         0x00080000                // RX_DRV_INTI_WDMA_ENABLED[19]
#define WED_ERR_MON_RX_DRV_INTI_WDMA_ENABLED_SHFT         19
#define WED_ERR_MON_RX_DRV_W_RESP_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_W_RESP_ERR_MASK                0x00020000                // RX_DRV_W_RESP_ERR[17]
#define WED_ERR_MON_RX_DRV_W_RESP_ERR_SHFT                17
#define WED_ERR_MON_RX_DRV_R_RESP_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_DRV_R_RESP_ERR_MASK                0x00010000                // RX_DRV_R_RESP_ERR[16]
#define WED_ERR_MON_RX_DRV_R_RESP_ERR_SHFT                16
#define WED_ERR_MON_RX_BM_DMAD_RD_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_BM_DMAD_RD_ERR_MASK                0x00002000                // RX_BM_DMAD_RD_ERR[13]
#define WED_ERR_MON_RX_BM_DMAD_RD_ERR_SHFT                13
#define WED_ERR_MON_RX_BM_FREE_AT_EMPTY_ADDR              WED_ERR_MON_ADDR
#define WED_ERR_MON_RX_BM_FREE_AT_EMPTY_MASK              0x00001000                // RX_BM_FREE_AT_EMPTY[12]
#define WED_ERR_MON_RX_BM_FREE_AT_EMPTY_SHFT              12
#define WED_ERR_MON_TX_BM_DMAD_RD_ERR_ADDR                WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_BM_DMAD_RD_ERR_MASK                0x00000800                // TX_BM_DMAD_RD_ERR[11]
#define WED_ERR_MON_TX_BM_DMAD_RD_ERR_SHFT                11
#define WED_ERR_MON_TX_BM_FREE_AT_EMPTY_ADDR              WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_BM_FREE_AT_EMPTY_MASK              0x00000400                // TX_BM_FREE_AT_EMPTY[10]
#define WED_ERR_MON_TX_BM_FREE_AT_EMPTY_SHFT              10
#define WED_ERR_MON_TX_PAO_ERR_ADDR                       WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_PAO_ERR_MASK                       0x00000080                // TX_PAO_ERR[7]
#define WED_ERR_MON_TX_PAO_ERR_SHFT                       7
#define WED_ERR_MON_TF_TKID_FIFO_INVLD_ADDR               WED_ERR_MON_ADDR
#define WED_ERR_MON_TF_TKID_FIFO_INVLD_MASK               0x00000010                // TF_TKID_FIFO_INVLD[4]
#define WED_ERR_MON_TF_TKID_FIFO_INVLD_SHFT               4
#define WED_ERR_MON_TX_TKID_FREE_ALREADY_FREE_ADDR        WED_ERR_MON_ADDR
#define WED_ERR_MON_TX_TKID_FREE_ALREADY_FREE_MASK        0x00000004                // TX_TKID_FREE_ALREADY_FREE[2]
#define WED_ERR_MON_TX_TKID_FREE_ALREADY_FREE_SHFT        2
#define WED_ERR_MON_TF_LEN_ERR_ADDR                       WED_ERR_MON_ADDR
#define WED_ERR_MON_TF_LEN_ERR_MASK                       0x00000001                // TF_LEN_ERR[0]
#define WED_ERR_MON_TF_LEN_ERR_SHFT                       0

/* =====================================================================================

  ---WED_ST (0x15010000 + 0x060)---

    RX_ST[7..0]                  - (RO) WED RX state machine
    TX_ST[15..8]                 - (RO) WED TX state machine
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_ST_TX_ST_ADDR                                 WED_ST_ADDR
#define WED_ST_TX_ST_MASK                                 0x0000FF00                // TX_ST[15..8]
#define WED_ST_TX_ST_SHFT                                 8
#define WED_ST_RX_ST_ADDR                                 WED_ST_ADDR
#define WED_ST_RX_ST_MASK                                 0x000000FF                // RX_ST[7..0]
#define WED_ST_RX_ST_SHFT                                 0

/* =====================================================================================

  ---WED_WPDMA_ST (0x15010000 + 0x064)---

    RX_DRV_ST[7..0]              - (RO) WPDMA Rx Driver Module state machine
    TX_DRV_ST[15..8]             - (RO) WPDMA Tx Driver Module state machine
    INT_AGT_ST[23..16]           - (RO) WPDMA Interrupt Agent Module state machine
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_ST_INT_AGT_ST_ADDR                      WED_WPDMA_ST_ADDR
#define WED_WPDMA_ST_INT_AGT_ST_MASK                      0x00FF0000                // INT_AGT_ST[23..16]
#define WED_WPDMA_ST_INT_AGT_ST_SHFT                      16
#define WED_WPDMA_ST_TX_DRV_ST_ADDR                       WED_WPDMA_ST_ADDR
#define WED_WPDMA_ST_TX_DRV_ST_MASK                       0x0000FF00                // TX_DRV_ST[15..8]
#define WED_WPDMA_ST_TX_DRV_ST_SHFT                       8
#define WED_WPDMA_ST_RX_DRV_ST_ADDR                       WED_WPDMA_ST_ADDR
#define WED_WPDMA_ST_RX_DRV_ST_MASK                       0x000000FF                // RX_DRV_ST[7..0]
#define WED_WPDMA_ST_RX_DRV_ST_SHFT                       0

/* =====================================================================================

  ---WED_WDMA_ST (0x15010000 + 0x068)---

    RX_DRV_ST[7..0]              - (RO) WDMA Rx Driver Module state machine
    TX_DRV_ST[15..8]             - (RO) WDMA Tx Driver Module state machine
    INT_AGT_ST[23..16]           - (RO) WDMA Interrupt Agent Module state machine
    AXI_W_AFTER_AW_ST[25..24]    - (RO) State machine of AXI W after AW reschedule control module for Rx Driver Module
    RX_PREF_DMAD_ST[28..26]      - (RO) State machine of DMAD prefetch agent of WDMA Rx Driver Module
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_ST_RX_PREF_DMAD_ST_ADDR                  WED_WDMA_ST_ADDR
#define WED_WDMA_ST_RX_PREF_DMAD_ST_MASK                  0x1C000000                // RX_PREF_DMAD_ST[28..26]
#define WED_WDMA_ST_RX_PREF_DMAD_ST_SHFT                  26
#define WED_WDMA_ST_AXI_W_AFTER_AW_ST_ADDR                WED_WDMA_ST_ADDR
#define WED_WDMA_ST_AXI_W_AFTER_AW_ST_MASK                0x03000000                // AXI_W_AFTER_AW_ST[25..24]
#define WED_WDMA_ST_AXI_W_AFTER_AW_ST_SHFT                24
#define WED_WDMA_ST_INT_AGT_ST_ADDR                       WED_WDMA_ST_ADDR
#define WED_WDMA_ST_INT_AGT_ST_MASK                       0x00FF0000                // INT_AGT_ST[23..16]
#define WED_WDMA_ST_INT_AGT_ST_SHFT                       16
#define WED_WDMA_ST_TX_DRV_ST_ADDR                        WED_WDMA_ST_ADDR
#define WED_WDMA_ST_TX_DRV_ST_MASK                        0x0000FF00                // TX_DRV_ST[15..8]
#define WED_WDMA_ST_TX_DRV_ST_SHFT                        8
#define WED_WDMA_ST_RX_DRV_ST_ADDR                        WED_WDMA_ST_ADDR
#define WED_WDMA_ST_RX_DRV_ST_MASK                        0x000000FF                // RX_DRV_ST[7..0]
#define WED_WDMA_ST_RX_DRV_ST_SHFT                        0

/* =====================================================================================

  ---WED_BM_ST (0x15010000 + 0x06c)---

    RX_BM_ST[7..0]               - (RO) Rx Buffer Manager Module state machine
    TX_BM_ST[15..8]              - (RO) Tx Buffer Manager Module state machine
    RESERVED16[23..16]           - (RO) Reserved bits
    FREE_AGT_ST[31..24]          - (RO) Free Agent Module state machine

 =====================================================================================*/
#define WED_BM_ST_FREE_AGT_ST_ADDR                        WED_BM_ST_ADDR
#define WED_BM_ST_FREE_AGT_ST_MASK                        0xFF000000                // FREE_AGT_ST[31..24]
#define WED_BM_ST_FREE_AGT_ST_SHFT                        24
#define WED_BM_ST_TX_BM_ST_ADDR                           WED_BM_ST_ADDR
#define WED_BM_ST_TX_BM_ST_MASK                           0x0000FF00                // TX_BM_ST[15..8]
#define WED_BM_ST_TX_BM_ST_SHFT                           8
#define WED_BM_ST_RX_BM_ST_ADDR                           WED_BM_ST_ADDR
#define WED_BM_ST_RX_BM_ST_MASK                           0x000000FF                // RX_BM_ST[7..0]
#define WED_BM_ST_RX_BM_ST_SHFT                           0

/* =====================================================================================

  ---WED_TX_BM_BASE (0x15010000 + 0x084)---

    PTR[31..0]                   - (RW) Buffer Pool Base Address
                                     Note that the Tx BM Buffer Pool Base Address must be 4KB addressing aligned

 =====================================================================================*/
#define WED_TX_BM_BASE_PTR_ADDR                           WED_TX_BM_BASE_ADDR
#define WED_TX_BM_BASE_PTR_MASK                           0xFFFFFFFF                // PTR[31..0]
#define WED_TX_BM_BASE_PTR_SHFT                           0

/* =====================================================================================

  ---WED_TX_BM_INIT_PTR (0x15010000 + 0x088)---

    SW_TAIL_IDX[15..0]           - (RW) 1. Free DMAD ptr(numbers) allocated by SW at initial. The Rx DMAD FIFO is 64K, but this value is flexiable depending on SW's requested, the operation need for Tx without loss. The maximum value SW can set/prepare is 64k-1 (0xffff).
    INIT_SW_TAIL_HEAD_IDX[16]    - (A0) Tail_idx and head_idx will initiate to sw_tail_idx and init_head_idx at setting this bit.
                                     SW initiates tail_idx to sw_tail_idx at setting this bit. SW set this bit with sw_tail_idx below after it has allocated sw_tail_idx number of Rx DMAD buffer, before enable WED Tx Buffer Manager operation.
                                     init_head_idx is for DV verficaion purpose, it is recommended that SW does not need to do init_head_idx settings.
                                     This bit is auto cleared.
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_ADDR     WED_TX_BM_INIT_PTR_ADDR
#define WED_TX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK     0x00010000                // INIT_SW_TAIL_HEAD_IDX[16]
#define WED_TX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_SHFT     16
#define WED_TX_BM_INIT_PTR_SW_TAIL_IDX_ADDR               WED_TX_BM_INIT_PTR_ADDR
#define WED_TX_BM_INIT_PTR_SW_TAIL_IDX_MASK               0x0000FFFF                // SW_TAIL_IDX[15..0]
#define WED_TX_BM_INIT_PTR_SW_TAIL_IDX_SHFT               0

/* =====================================================================================

  ---WED_TX_BM_BLEN (0x15010000 + 0x08C)---

    BYTE_LEN[13..0]              - (RW) Free buffer byte length
                                     Configured free buffer length, WDMA Rx hardware driver module will fetch the length info to decide the payload length on one descriptor/buffer which is owned by WED Buffer Manager.
    RESERVED14[15..14]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) High bits of Buffer Pool Base Address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_BLEN_BASE_PTR_H_ADDR                    WED_TX_BM_BLEN_ADDR
#define WED_TX_BM_BLEN_BASE_PTR_H_MASK                    0x00FF0000                // BASE_PTR_H[23..16]
#define WED_TX_BM_BLEN_BASE_PTR_H_SHFT                    16
#define WED_TX_BM_BLEN_BYTE_LEN_ADDR                      WED_TX_BM_BLEN_ADDR
#define WED_TX_BM_BLEN_BYTE_LEN_MASK                      0x00003FFF                // BYTE_LEN[13..0]
#define WED_TX_BM_BLEN_BYTE_LEN_SHFT                      0

/* =====================================================================================

  ---WED_TX_BM_STS (0x15010000 + 0x090)---

    RESERVED0[15..0]             - (RO) Reserved bits
    DMAD_NUM[20..16]             - (RO) Number of valid free DMADs, ready for WDMA Rx driver module. Those free DMADs are picked out from buffer pool by Buffer manager.
    RESERVED21[23..21]           - (RO) Reserved bits
    DMAD_VLD[24]                 - (RO) Free DMAD is valid for WDMA Rx driver module
    DMAD_RD[25]                  - (RO) WDMA Rx driver module extracted a free DMAD
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_STS_DMAD_RD_ADDR                        WED_TX_BM_STS_ADDR
#define WED_TX_BM_STS_DMAD_RD_MASK                        0x02000000                // DMAD_RD[25]
#define WED_TX_BM_STS_DMAD_RD_SHFT                        25
#define WED_TX_BM_STS_DMAD_VLD_ADDR                       WED_TX_BM_STS_ADDR
#define WED_TX_BM_STS_DMAD_VLD_MASK                       0x01000000                // DMAD_VLD[24]
#define WED_TX_BM_STS_DMAD_VLD_SHFT                       24
#define WED_TX_BM_STS_DMAD_NUM_ADDR                       WED_TX_BM_STS_ADDR
#define WED_TX_BM_STS_DMAD_NUM_MASK                       0x001F0000                // DMAD_NUM[20..16]
#define WED_TX_BM_STS_DMAD_NUM_SHFT                       16

/* =====================================================================================

  ---WED_TX_BM_PTR (0x15010000 + 0x094)---

    TAIL_IDX[15..0]              - (RO) 1. tail_idx is updated to sw_tail_idx when init_sw_tail_idx of WED_TX_BM_INIT_PTR is set. This is the Free DMAD number SW allocates. WED_TX_BM will start Rx DMAD delivery and return, after wed_tx_bm is enabled, and this field != head_idx (0).
                                     2. it is advanced by HW each time when a DMAD is free/returned from WED_TX_TKID (original rx DMAD is exchanged with that of CPU ring, for CPU to handle rx pkt to it.) The tail_idx update should not run over head_idx
    HEAD_IDX[31..16]             - (RO) head_idx is incremented by HW each time a Rx DMAD is fetched by WED_TX_BM, which points to the index WED_TX_BM will fetch next.
                                     Note that head_idx will never exceed tail_idx. When they are equal, it means all rx DMAD allocated by SW are all passed to HW to Rx pkt and no one is free yet.

 =====================================================================================*/
#define WED_TX_BM_PTR_HEAD_IDX_ADDR                       WED_TX_BM_PTR_ADDR
#define WED_TX_BM_PTR_HEAD_IDX_MASK                       0xFFFF0000                // HEAD_IDX[31..16]
#define WED_TX_BM_PTR_HEAD_IDX_SHFT                       16
#define WED_TX_BM_PTR_TAIL_IDX_ADDR                       WED_TX_BM_PTR_ADDR
#define WED_TX_BM_PTR_TAIL_IDX_MASK                       0x0000FFFF                // TAIL_IDX[15..0]
#define WED_TX_BM_PTR_TAIL_IDX_SHFT                       0

/* =====================================================================================

  ---WED_TX_BM_RANGE_CFG (0x15010000 + 0x098)---

    SW_CFG_BUF_IDX[15..0]        - (RW) This value is the entries of the DMAD FIFO ring set by SW at initial.  The maximum value of tail_idx and head_idx that can be achieved is this value. The maximum value SW can set is 64K-1(0xFFFF).
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_RANGE_CFG_SW_CFG_BUF_IDX_ADDR           WED_TX_BM_RANGE_CFG_ADDR
#define WED_TX_BM_RANGE_CFG_SW_CFG_BUF_IDX_MASK           0x0000FFFF                // SW_CFG_BUF_IDX[15..0]
#define WED_TX_BM_RANGE_CFG_SW_CFG_BUF_IDX_SHFT           0

/* =====================================================================================

  ---WED_TX_BM_ERR_STS (0x15010000 + 0x0A4)---

    DMAD_AXI_RD_LESS[0]          - (RO) WED Tx Buffer Manager Error inidcation: DMAD AXI read transfer less than expected. For example, issue 8 transfers but rlast is  asserted earlier  than 8th transfer.
                                     Bit 3:0 will trigger 0x54 WED_ERR_MON [11] tx_bm_dmad_rd_err
    DMAD_AXI_RD_MORE[1]          - (RO) WED Tx Buffer Manager Error inidcation: DMAD AXI read transfer more than expected. For example, issue 8 transfers but rlast is  asserted later  than 8th transfer.
                                     Bit 3:0 will trigger  0x54 WED_ERR_MON [11] tx_bm_dmad_rd_err
    DMAD_FULL_AXI_RD[2]          - (RO) WED Tx Buffer Manager Error inidcation: DMAD AXI read while internal DMAD FIFO is already full.
                                     Bit 3:0 will trigger  0x54 WED_ERR_MON [11] tx_bm_dmad_rd_err
    DMAD_AXI_RD_OVER_TAIL[3]     - (RO) WED Tx Buffer Manager Error inidcation: DMAD AXI read while head_idx is reaching tail_idx. DMAD should be all read in already.
                                     Bit 3:0 will trigger  0x54 WED_ERR_MON [11] tx_bm_dmad_rd_err
    RESERVED4[6..4]              - (RO) Reserved bits
    FREE_AT_EMPTY[7]             - (RO) DMAD free while head_idx==tail_idx, all rx DMAD should already be free.
                                     Will trigger  0x54 WED_ERR_MON [10] tx_bm_free_at_empty
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_ERR_STS_FREE_AT_EMPTY_ADDR              WED_TX_BM_ERR_STS_ADDR
#define WED_TX_BM_ERR_STS_FREE_AT_EMPTY_MASK              0x00000080                // FREE_AT_EMPTY[7]
#define WED_TX_BM_ERR_STS_FREE_AT_EMPTY_SHFT              7
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_ADDR      WED_TX_BM_ERR_STS_ADDR
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_MASK      0x00000008                // DMAD_AXI_RD_OVER_TAIL[3]
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_SHFT      3
#define WED_TX_BM_ERR_STS_DMAD_FULL_AXI_RD_ADDR           WED_TX_BM_ERR_STS_ADDR
#define WED_TX_BM_ERR_STS_DMAD_FULL_AXI_RD_MASK           0x00000004                // DMAD_FULL_AXI_RD[2]
#define WED_TX_BM_ERR_STS_DMAD_FULL_AXI_RD_SHFT           2
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_MORE_ADDR           WED_TX_BM_ERR_STS_ADDR
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_MORE_MASK           0x00000002                // DMAD_AXI_RD_MORE[1]
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_MORE_SHFT           1
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_LESS_ADDR           WED_TX_BM_ERR_STS_ADDR
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_LESS_MASK           0x00000001                // DMAD_AXI_RD_LESS[0]
#define WED_TX_BM_ERR_STS_DMAD_AXI_RD_LESS_SHFT           0

/* =====================================================================================

  ---WED_TX_TKID_CTRL (0x15010000 + 0x0C0)---

    VLD_GRP_NUM[7..0]            - (RW) Buffer Pool total software allocated valid buffer number.
                                     (unit: 1 group comprises 128 buffers)
    RESERVED8[11..8]             - (RO) Reserved bits
    VB_PGSEL[12]                 - (RW) control CR WED_TX_TKID_VB_FREE_0_31/WED_TX_TKID_VB_FREE_32_63/WED_TX_TKID_VB_USED_0_31/WED_TX_TKID_VB_USED_32_63 display page, 128/64= 2 pages
    RESERVED13[13]               - (RO) Reserved bits
    FIFO_AVAIL[15..14]           - (RW) Token ID FIFO available condition
    RSV_GRP_NUM[23..16]          - (RW) Buffer Pool reserved buffer number for hardware, only buffers those over this reserved number will involve the dynamic buffer allocation calculation.
                                     (unit: 1 group comprises 128 buffers)
    RESERVED24[24]               - (RO) Reserved bits
    FORCE_DRAM[25]               - (RW)  xxx
    FREE_WAIT[26]                - (RW)  xxx
    RESERVED27[27]               - (RO) Reserved bits
    PAUSE[28]                    - (RW) Software pauses the TKID Manager during changing the "vld_num", for dynamic buffer allocation purpose
    SCAN[29]                     - (A0) Software forces the TKID Manager to scan TKID Valid Bit SRAM and update internal status.
                                     Write 1 to enable scan, and this bit will return to 0 automatically after finish.
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_CTRL_SCAN_ADDR                        WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_SCAN_MASK                        0x20000000                // SCAN[29]
#define WED_TX_TKID_CTRL_SCAN_SHFT                        29
#define WED_TX_TKID_CTRL_PAUSE_ADDR                       WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_PAUSE_MASK                       0x10000000                // PAUSE[28]
#define WED_TX_TKID_CTRL_PAUSE_SHFT                       28
#define WED_TX_TKID_CTRL_FREE_WAIT_ADDR                   WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_FREE_WAIT_MASK                   0x04000000                // FREE_WAIT[26]
#define WED_TX_TKID_CTRL_FREE_WAIT_SHFT                   26
#define WED_TX_TKID_CTRL_FORCE_DRAM_ADDR                  WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_FORCE_DRAM_MASK                  0x02000000                // FORCE_DRAM[25]
#define WED_TX_TKID_CTRL_FORCE_DRAM_SHFT                  25
#define WED_TX_TKID_CTRL_RSV_GRP_NUM_ADDR                 WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_RSV_GRP_NUM_MASK                 0x00FF0000                // RSV_GRP_NUM[23..16]
#define WED_TX_TKID_CTRL_RSV_GRP_NUM_SHFT                 16
#define WED_TX_TKID_CTRL_FIFO_AVAIL_ADDR                  WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_FIFO_AVAIL_MASK                  0x0000C000                // FIFO_AVAIL[15..14]
#define WED_TX_TKID_CTRL_FIFO_AVAIL_SHFT                  14
#define WED_TX_TKID_CTRL_VB_PGSEL_ADDR                    WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_VB_PGSEL_MASK                    0x00001000                // VB_PGSEL[12]
#define WED_TX_TKID_CTRL_VB_PGSEL_SHFT                    12
#define WED_TX_TKID_CTRL_VLD_GRP_NUM_ADDR                 WED_TX_TKID_CTRL_ADDR
#define WED_TX_TKID_CTRL_VLD_GRP_NUM_MASK                 0x000000FF                // VLD_GRP_NUM[7..0]
#define WED_TX_TKID_CTRL_VLD_GRP_NUM_SHFT                 0

/* =====================================================================================

  ---WED_TX_TKID_SPR (0x15010000 + 0x0C4)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_TX_TKID_SPR_RSV_REG_ADDR                      WED_TX_TKID_SPR_ADDR
#define WED_TX_TKID_SPR_RSV_REG_MASK                      0xFFFFFFFF                // RSV_REG[31..0]
#define WED_TX_TKID_SPR_RSV_REG_SHFT                      0

/* =====================================================================================

  ---WED_TX_TKID_TKID (0x15010000 + 0x0C8)---

    START_ID[15..0]              - (RW) Free TKID allocated token ID start number
    END_ID[31..16]               - (RW) Free TKID allocated token ID end number

 =====================================================================================*/
#define WED_TX_TKID_TKID_END_ID_ADDR                      WED_TX_TKID_TKID_ADDR
#define WED_TX_TKID_TKID_END_ID_MASK                      0xFFFF0000                // END_ID[31..16]
#define WED_TX_TKID_TKID_END_ID_SHFT                      16
#define WED_TX_TKID_TKID_START_ID_ADDR                    WED_TX_TKID_TKID_ADDR
#define WED_TX_TKID_TKID_START_ID_MASK                    0x0000FFFF                // START_ID[15..0]
#define WED_TX_TKID_TKID_START_ID_SHFT                    0

/* =====================================================================================

  ---WED_TX_BM_INIT_HEAD_PTR (0x15010000 + 0x0CC)---

    INIT_HEAD_IDX[15..0]         - (RW) Start buffer index, for verification purpose (wraparound test)
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_ADDR        WED_TX_BM_INIT_HEAD_PTR_ADDR
#define WED_TX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_MASK        0x0000FFFF                // INIT_HEAD_IDX[15..0]
#define WED_TX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_SHFT        0

/* =====================================================================================

  ---WED_TX_TKID_INTF (0x15010000 + 0x0DC)---

    FREE_TKID[15..0]             - (RO) Free Token_ID for TKID Manager
    FREE_TKFIFO_FDEP[25..16]     - (RO) Free depth of Tx Free Agent Token_ID FIFO
    RESERVED26[27..26]           - (RO) Reserved bits
    FREE_TKID_VLD[28]            - (RO) There is at least one valid Free token_ID for Buffer Manager, these free Token_IDs are extracted from WiFi card's Rx ring#1 "Tx Free Notify" messages.
    FREE_TKID_RD[29]             - (RO) TKID Manager extracted a free Token_ID from Tx Free Agent module
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_INTF_FREE_TKID_RD_ADDR                WED_TX_TKID_INTF_ADDR
#define WED_TX_TKID_INTF_FREE_TKID_RD_MASK                0x20000000                // FREE_TKID_RD[29]
#define WED_TX_TKID_INTF_FREE_TKID_RD_SHFT                29
#define WED_TX_TKID_INTF_FREE_TKID_VLD_ADDR               WED_TX_TKID_INTF_ADDR
#define WED_TX_TKID_INTF_FREE_TKID_VLD_MASK               0x10000000                // FREE_TKID_VLD[28]
#define WED_TX_TKID_INTF_FREE_TKID_VLD_SHFT               28
#define WED_TX_TKID_INTF_FREE_TKFIFO_FDEP_ADDR            WED_TX_TKID_INTF_ADDR
#define WED_TX_TKID_INTF_FREE_TKFIFO_FDEP_MASK            0x03FF0000                // FREE_TKFIFO_FDEP[25..16]
#define WED_TX_TKID_INTF_FREE_TKFIFO_FDEP_SHFT            16
#define WED_TX_TKID_INTF_FREE_TKID_ADDR                   WED_TX_TKID_INTF_ADDR
#define WED_TX_TKID_INTF_FREE_TKID_MASK                   0x0000FFFF                // FREE_TKID[15..0]
#define WED_TX_TKID_INTF_FREE_TKID_SHFT                   0

/* =====================================================================================

  ---WED_TX_TKID_DYN_TH (0x15010000 + 0x0E0)---

    LOW_GRP_NUM[7..0]            - (RW) WED assert interrupt to request more TKID from software, when WED Tx owned available free TKID amount is lower than this threshold value.
                                     (unit: 1 group comprises 128 buffers)
    RESERVED8[15..8]             - (RO) Reserved bits
    HI_GRP_NUM[23..16]           - (RW) WED asserts interrupt to return buffer to software, when WED Tx owned available "TRAILING" free TKID amount is higher than this threshold value.
                                     (unit: 1 group comprises 128 buffers)
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_DYN_TH_HI_GRP_NUM_ADDR                WED_TX_TKID_DYN_TH_ADDR
#define WED_TX_TKID_DYN_TH_HI_GRP_NUM_MASK                0x00FF0000                // HI_GRP_NUM[23..16]
#define WED_TX_TKID_DYN_TH_HI_GRP_NUM_SHFT                16
#define WED_TX_TKID_DYN_TH_LOW_GRP_NUM_ADDR               WED_TX_TKID_DYN_TH_ADDR
#define WED_TX_TKID_DYN_TH_LOW_GRP_NUM_MASK               0x000000FF                // LOW_GRP_NUM[7..0]
#define WED_TX_TKID_DYN_TH_LOW_GRP_NUM_SHFT               0

/* =====================================================================================

  ---WED_TX_TKID_STS (0x15010000 + 0x0E4)---

    FREE_NUM[15..0]              - (RU) Total Free Tx TKID number, not be selected yet
    FSM[27..16]                  - (RO) Tx TKID Manager Module state machine
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_STS_FSM_ADDR                          WED_TX_TKID_STS_ADDR
#define WED_TX_TKID_STS_FSM_MASK                          0x0FFF0000                // FSM[27..16]
#define WED_TX_TKID_STS_FSM_SHFT                          16
#define WED_TX_TKID_STS_FREE_NUM_ADDR                     WED_TX_TKID_STS_ADDR
#define WED_TX_TKID_STS_FREE_NUM_MASK                     0x0000FFFF                // FREE_NUM[15..0]
#define WED_TX_TKID_STS_FREE_NUM_SHFT                     0

/* =====================================================================================

  ---WED_TX_TKID_RECYC (0x15010000 + 0x0E8)---

    START_GRP_IDX[7..0]          - (RO) WED asserts interrupt to return buffer to software, software could recycle all buffers after this entry index.
                                     (unit: 1 group comprises 128 buffers)
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_RECYC_START_GRP_IDX_ADDR              WED_TX_TKID_RECYC_ADDR
#define WED_TX_TKID_RECYC_START_GRP_IDX_MASK              0x000000FF                // START_GRP_IDX[7..0]
#define WED_TX_TKID_RECYC_START_GRP_IDX_SHFT              0

/* =====================================================================================

  ---WED_TX_TKID_ALI (0x15010000 + 0x0EC)---

    RESERVED0[0]                 - (RO) Reserved bits
    FIFO_RST[1]                  - (A0) Reset internal FIFO and registers.
                                     Must disable the HW before reset it
    RESERVED2[3..2]              - (RO) Reserved bits
    FIFO_EMPTY[7..4]             - (RO) FIFO empty
                                     Bits [0] for FIFO0
                                     Bits [1] for FIFO1
                                     Bits [2] for FIFO2
                                     Bits [3] for FIFO3
    FIFO_FULL[11..8]             - (RO) FIFO full
                                     Bits [0] for FIFO0
                                     Bits [1] for FIFO1
                                     Bits [2] for FIFO2
                                     Bits [3] for FIFO3
    RESERVED12[15..12]           - (RO) Reserved bits
    FIFO_CNT[27..16]             - (RO) Used entry in each FIFO.
                                     Bits [2:0] for FIFO0
                                     Bits [5:3] for FIFO1
                                     Bits [8:6] for FIFO2
                                     Bits [11:9] for FIFO3
    FIFO_WR_ERR[28]              - (W1C) Write an full FIFO
    FIFO_RD_ERR[29]              - (W1C) Read an empty FIFO
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_TKID_ALI_FIFO_RD_ERR_ADDR                  WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_RD_ERR_MASK                  0x20000000                // FIFO_RD_ERR[29]
#define WED_TX_TKID_ALI_FIFO_RD_ERR_SHFT                  29
#define WED_TX_TKID_ALI_FIFO_WR_ERR_ADDR                  WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_WR_ERR_MASK                  0x10000000                // FIFO_WR_ERR[28]
#define WED_TX_TKID_ALI_FIFO_WR_ERR_SHFT                  28
#define WED_TX_TKID_ALI_FIFO_CNT_ADDR                     WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_CNT_MASK                     0x0FFF0000                // FIFO_CNT[27..16]
#define WED_TX_TKID_ALI_FIFO_CNT_SHFT                     16
#define WED_TX_TKID_ALI_FIFO_FULL_ADDR                    WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_FULL_MASK                    0x00000F00                // FIFO_FULL[11..8]
#define WED_TX_TKID_ALI_FIFO_FULL_SHFT                    8
#define WED_TX_TKID_ALI_FIFO_EMPTY_ADDR                   WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_EMPTY_MASK                   0x000000F0                // FIFO_EMPTY[7..4]
#define WED_TX_TKID_ALI_FIFO_EMPTY_SHFT                   4
#define WED_TX_TKID_ALI_FIFO_RST_ADDR                     WED_TX_TKID_ALI_ADDR
#define WED_TX_TKID_ALI_FIFO_RST_MASK                     0x00000002                // FIFO_RST[1]
#define WED_TX_TKID_ALI_FIFO_RST_SHFT                     1

/* =====================================================================================

  ---WED_TX_TKID_VB_FREE_0_31 (0x15010000 + 0x0F0)---

    GRP[31..0]                   - (RO) WED Tx TKID Manager internal bitmap usage record.
                                     1 bit mapping to a group comprises 128 TKIDs.
                                     E.g. bits[0] indicates the usage of TKID Bitmap entry #0~#127

 =====================================================================================*/
#define WED_TX_TKID_VB_FREE_0_31_GRP_ADDR                 WED_TX_TKID_VB_FREE_0_31_ADDR
#define WED_TX_TKID_VB_FREE_0_31_GRP_MASK                 0xFFFFFFFF                // GRP[31..0]
#define WED_TX_TKID_VB_FREE_0_31_GRP_SHFT                 0

/* =====================================================================================

  ---WED_TX_TKID_VB_FREE_32_63 (0x15010000 + 0x0F4)---

    GRP[31..0]                   - (RO) WED Tx TKID Manager internal bitmap usage record.
                                     1 bit mapping to a group comprises 128 TKIDs.
                                     E.g. bits[0] indicates the usage of TKID Bitmap entry #4096~#4223

 =====================================================================================*/
#define WED_TX_TKID_VB_FREE_32_63_GRP_ADDR                WED_TX_TKID_VB_FREE_32_63_ADDR
#define WED_TX_TKID_VB_FREE_32_63_GRP_MASK                0xFFFFFFFF                // GRP[31..0]
#define WED_TX_TKID_VB_FREE_32_63_GRP_SHFT                0

/* =====================================================================================

  ---WED_TX_TKID_VB_USED_0_31 (0x15010000 + 0x0F8)---

    GRP[31..0]                   - (RO) WED Tx TKID Manager internal bitmap usage record.
                                     1 bit mapping to a group comprises 128 TKIDs.
                                     E.g. bits[0] indicates the usage of TKID Bitmap entry #0~#127

 =====================================================================================*/
#define WED_TX_TKID_VB_USED_0_31_GRP_ADDR                 WED_TX_TKID_VB_USED_0_31_ADDR
#define WED_TX_TKID_VB_USED_0_31_GRP_MASK                 0xFFFFFFFF                // GRP[31..0]
#define WED_TX_TKID_VB_USED_0_31_GRP_SHFT                 0

/* =====================================================================================

  ---WED_TX_TKID_VB_USED_32_63 (0x15010000 + 0x0FC)---

    GRP[31..0]                   - (RO) WED Tx TKID Manager internal bitmap usage record.
                                     1 bit mapping to a group comprises 128 TKIDs.
                                     E.g. bits[0] indicates the usage of TKID Bitmap entry #4096~#4223

 =====================================================================================*/
#define WED_TX_TKID_VB_USED_32_63_GRP_ADDR                WED_TX_TKID_VB_USED_32_63_ADDR
#define WED_TX_TKID_VB_USED_32_63_GRP_MASK                0xFFFFFFFF                // GRP[31..0]
#define WED_TX_TKID_VB_USED_32_63_GRP_SHFT                0

/* =====================================================================================

  ---WED_TXD_DW0 (0x15010000 + 0x100)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW0

 =====================================================================================*/
#define WED_TXD_DW0_VALUE_ADDR                            WED_TXD_DW0_ADDR
#define WED_TXD_DW0_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW0_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW1 (0x15010000 + 0x104)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW1

 =====================================================================================*/
#define WED_TXD_DW1_VALUE_ADDR                            WED_TXD_DW1_ADDR
#define WED_TXD_DW1_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW1_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW2 (0x15010000 + 0x108)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW2

 =====================================================================================*/
#define WED_TXD_DW2_VALUE_ADDR                            WED_TXD_DW2_ADDR
#define WED_TXD_DW2_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW2_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW3 (0x15010000 + 0x10C)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW3

 =====================================================================================*/
#define WED_TXD_DW3_VALUE_ADDR                            WED_TXD_DW3_ADDR
#define WED_TXD_DW3_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW3_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW4 (0x15010000 + 0x110)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW4

 =====================================================================================*/
#define WED_TXD_DW4_VALUE_ADDR                            WED_TXD_DW4_ADDR
#define WED_TXD_DW4_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW4_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW5 (0x15010000 + 0x114)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW5

 =====================================================================================*/
#define WED_TXD_DW5_VALUE_ADDR                            WED_TXD_DW5_ADDR
#define WED_TXD_DW5_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW5_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW6 (0x15010000 + 0x118)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW6

 =====================================================================================*/
#define WED_TXD_DW6_VALUE_ADDR                            WED_TXD_DW6_ADDR
#define WED_TXD_DW6_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW6_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXD_DW7 (0x15010000 + 0x11C)---

    VALUE[31..0]                 - (RW) WED WPDMA Tx Driver module write value of TXD DW7

 =====================================================================================*/
#define WED_TXD_DW7_VALUE_ADDR                            WED_TXD_DW7_ADDR
#define WED_TXD_DW7_VALUE_MASK                            0xFFFFFFFF                // VALUE[31..0]
#define WED_TXD_DW7_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXP_DW0 (0x15010000 + 0x120)---

    VALUE[15..0]                 - (RW) WED WPDMA Tx Driver module write value of TXP DW0
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TXP_DW0_VALUE_ADDR                            WED_TXP_DW0_ADDR
#define WED_TXP_DW0_VALUE_MASK                            0x0000FFFF                // VALUE[15..0]
#define WED_TXP_DW0_VALUE_SHFT                            0

/* =====================================================================================

  ---WED_TXP_DW1 (0x15010000 + 0x124)---

    RESERVED0[15..0]             - (RO) Reserved bits
    VALUE[31..16]                - (RW) WED WPDMA Tx Driver module write value of TXP DW1

 =====================================================================================*/
#define WED_TXP_DW1_VALUE_ADDR                            WED_TXP_DW1_ADDR
#define WED_TXP_DW1_VALUE_MASK                            0xFFFF0000                // VALUE[31..16]
#define WED_TXP_DW1_VALUE_SHFT                            16

/* =====================================================================================

  ---WED_TXDP_CTRL (0x15010000 + 0x130)---

    DW_OVERWR[9..0]              - (RW) WED WPDMA Tx Driver module overwrite TXD/TXP control per double word, each bit control one DW overwrite access.
    WCID_BIT[10]                 - (RW) WED WPDMA Tx Driver module overwrite TXD WCID Field Bit Selection
    TXD_ADDR_SEL[11]             - (RW) WED WPDMA Tx Driver module, HIFTXD Address Selection
    HIFTXD_SIZE[15..12]          - (RW) HIFTXD size, for calculate HIFTXD physical address pointer
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TXDP_CTRL_HIFTXD_SIZE_ADDR                    WED_TXDP_CTRL_ADDR
#define WED_TXDP_CTRL_HIFTXD_SIZE_MASK                    0x0000F000                // HIFTXD_SIZE[15..12]
#define WED_TXDP_CTRL_HIFTXD_SIZE_SHFT                    12
#define WED_TXDP_CTRL_TXD_ADDR_SEL_ADDR                   WED_TXDP_CTRL_ADDR
#define WED_TXDP_CTRL_TXD_ADDR_SEL_MASK                   0x00000800                // TXD_ADDR_SEL[11]
#define WED_TXDP_CTRL_TXD_ADDR_SEL_SHFT                   11
#define WED_TXDP_CTRL_WCID_BIT_ADDR                       WED_TXDP_CTRL_ADDR
#define WED_TXDP_CTRL_WCID_BIT_MASK                       0x00000400                // WCID_BIT[10]
#define WED_TXDP_CTRL_WCID_BIT_SHFT                       10
#define WED_TXDP_CTRL_DW_OVERWR_ADDR                      WED_TXDP_CTRL_ADDR
#define WED_TXDP_CTRL_DW_OVERWR_MASK                      0x000003FF                // DW_OVERWR[9..0]
#define WED_TXDP_CTRL_DW_OVERWR_SHFT                      0

/* =====================================================================================

  ---WED_DBG_CTRL (0x15010000 + 0x180)---

    PRB_SEL[5..0]                - (RW) WED probe pin selection, each selected module's 128 bits probe signal will on WED_DBG_PRB0~3
    RESERVED6[7..6]              - (RO) Reserved bits
    PRB_DW_SEL_TO_CHIP[9..8]     - (RW) Select one of WED_DBG_PRB0~3 to output to chip top probe MUX
    RESERVED10[23..10]           - (RO) Reserved bits
    BUS_MON_AW_EN[24]            - (RW) Enable WED bus monitor AXI write address
    BUS_MON_AR_EN[25]            - (RW) Enable WED bus monitor AXI read address
    TX_BM_MEM_PGSEL[27..26]      - (RW) WED_TX_BM/WED_TX_TKID valid bit SRAM debug mode page select for APB read/write
    TX_TKID_BM_MEM_DBG[28]       - (RW) WED_TX_TKID/WED_TX_BM valid bit SRAM debug mode
                                     (SRAM data width is 128b/16B)
                                     TKID SRAM is seperated to 2 pages controlled by  tx_bm_mem_pgsel to select which page to access.
                                     Tx BM SRAM is seperated to 4 pages controlled by  tx_bm_mem_pgsel to select which page to access.
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_DBG_CTRL_TX_TKID_BM_MEM_DBG_ADDR              WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_TX_TKID_BM_MEM_DBG_MASK              0x10000000                // TX_TKID_BM_MEM_DBG[28]
#define WED_DBG_CTRL_TX_TKID_BM_MEM_DBG_SHFT              28
#define WED_DBG_CTRL_TX_BM_MEM_PGSEL_ADDR                 WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_TX_BM_MEM_PGSEL_MASK                 0x0C000000                // TX_BM_MEM_PGSEL[27..26]
#define WED_DBG_CTRL_TX_BM_MEM_PGSEL_SHFT                 26
#define WED_DBG_CTRL_BUS_MON_AR_EN_ADDR                   WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_BUS_MON_AR_EN_MASK                   0x02000000                // BUS_MON_AR_EN[25]
#define WED_DBG_CTRL_BUS_MON_AR_EN_SHFT                   25
#define WED_DBG_CTRL_BUS_MON_AW_EN_ADDR                   WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_BUS_MON_AW_EN_MASK                   0x01000000                // BUS_MON_AW_EN[24]
#define WED_DBG_CTRL_BUS_MON_AW_EN_SHFT                   24
#define WED_DBG_CTRL_PRB_DW_SEL_TO_CHIP_ADDR              WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_PRB_DW_SEL_TO_CHIP_MASK              0x00000300                // PRB_DW_SEL_TO_CHIP[9..8]
#define WED_DBG_CTRL_PRB_DW_SEL_TO_CHIP_SHFT              8
#define WED_DBG_CTRL_PRB_SEL_ADDR                         WED_DBG_CTRL_ADDR
#define WED_DBG_CTRL_PRB_SEL_MASK                         0x0000003F                // PRB_SEL[5..0]
#define WED_DBG_CTRL_PRB_SEL_SHFT                         0

/* =====================================================================================

  ---WED_DBG_BUS_ADDR_L (0x15010000 + 0x184)---

    ADDRESS[31..0]               - (RW) WED bus monitor for output awaddr & araddr low part

 =====================================================================================*/
#define WED_DBG_BUS_ADDR_L_ADDRESS_ADDR                   WED_DBG_BUS_ADDR_L_ADDR
#define WED_DBG_BUS_ADDR_L_ADDRESS_MASK                   0xFFFFFFFF                // ADDRESS[31..0]
#define WED_DBG_BUS_ADDR_L_ADDRESS_SHFT                   0

/* =====================================================================================

  ---WED_DBG_BUS_ADDR_H (0x15010000 + 0x188)---

    ADDRESS[7..0]                - (RW) WED bus monitor for output awaddr & araddr high part
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_DBG_BUS_ADDR_H_ADDRESS_ADDR                   WED_DBG_BUS_ADDR_H_ADDR
#define WED_DBG_BUS_ADDR_H_ADDRESS_MASK                   0x000000FF                // ADDRESS[7..0]
#define WED_DBG_BUS_ADDR_H_ADDRESS_SHFT                   0

/* =====================================================================================

  ---WED_DBG_BUS_MON (0x15010000 + 0x18C)---

    STATUS[31..0]                - (RO) WED AXI bus status
                                     [31:10] Reserved
                                     [9] wedaxi_sl0_r_busy
                                     [8] wedaxi_sl0_w_busy
                                     [7] wedaxi_mi2_r_busy
                                     [6] wedaxi_mi2_w_busy
                                     [5] wedaxi_mi2_errmid_set_birq
                                     [4] wedaxi_mi2_errmid_set_rirq
                                     [3] wedaxi_mi1_r_busy
                                     [2] wedaxi_mi1_w_busy
                                     [1] wedaxi_mi1_errmid_set_birq
                                     [0] wedaxi_mi1_errmid_set_rirq

 =====================================================================================*/
#define WED_DBG_BUS_MON_STATUS_ADDR                       WED_DBG_BUS_MON_ADDR
#define WED_DBG_BUS_MON_STATUS_MASK                       0xFFFFFFFF                // STATUS[31..0]
#define WED_DBG_BUS_MON_STATUS_SHFT                       0

/* =====================================================================================

  ---WED_DBG_PRB0 (0x15010000 + 0x190)---

    SIGNAL[31..0]                - (RO) WED probe selected signals #0

 =====================================================================================*/
#define WED_DBG_PRB0_SIGNAL_ADDR                          WED_DBG_PRB0_ADDR
#define WED_DBG_PRB0_SIGNAL_MASK                          0xFFFFFFFF                // SIGNAL[31..0]
#define WED_DBG_PRB0_SIGNAL_SHFT                          0

/* =====================================================================================

  ---WED_DBG_PRB1 (0x15010000 + 0x194)---

    SIGNAL[31..0]                - (RO) WED probe selected signals #1

 =====================================================================================*/
#define WED_DBG_PRB1_SIGNAL_ADDR                          WED_DBG_PRB1_ADDR
#define WED_DBG_PRB1_SIGNAL_MASK                          0xFFFFFFFF                // SIGNAL[31..0]
#define WED_DBG_PRB1_SIGNAL_SHFT                          0

/* =====================================================================================

  ---WED_DBG_PRB2 (0x15010000 + 0x198)---

    SIGNAL[31..0]                - (RO) WED probe selected signals #2

 =====================================================================================*/
#define WED_DBG_PRB2_SIGNAL_ADDR                          WED_DBG_PRB2_ADDR
#define WED_DBG_PRB2_SIGNAL_MASK                          0xFFFFFFFF                // SIGNAL[31..0]
#define WED_DBG_PRB2_SIGNAL_SHFT                          0

/* =====================================================================================

  ---WED_DBG_PRB3 (0x15010000 + 0x19c)---

    SIGNAL[31..0]                - (RO) WED probe selected signals #3

 =====================================================================================*/
#define WED_DBG_PRB3_SIGNAL_ADDR                          WED_DBG_PRB3_ADDR
#define WED_DBG_PRB3_SIGNAL_MASK                          0xFFFFFFFF                // SIGNAL[31..0]
#define WED_DBG_PRB3_SIGNAL_SHFT                          0

/* =====================================================================================

  ---WED_TX_TKID_FREE_TKID_IN (0x15010000 + 0x1b0)---

    CNT[31..0]                   - (RO) Counter the number of TKID input WED Tx TKID

 =====================================================================================*/
#define WED_TX_TKID_FREE_TKID_IN_CNT_ADDR                 WED_TX_TKID_FREE_TKID_IN_ADDR
#define WED_TX_TKID_FREE_TKID_IN_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_TX_TKID_FREE_TKID_IN_CNT_SHFT                 0

/* =====================================================================================

  ---WED_TX_TKID_FREE_TKID_LDDRAM (0x15010000 + 0x1b4)---

    CNT[31..0]                   - (RO) Counter the number of TKID which are required access DRAM

 =====================================================================================*/
#define WED_TX_TKID_FREE_TKID_LDDRAM_CNT_ADDR             WED_TX_TKID_FREE_TKID_LDDRAM_ADDR
#define WED_TX_TKID_FREE_TKID_LDDRAM_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_TX_TKID_FREE_TKID_LDDRAM_CNT_SHFT             0

/* =====================================================================================

  ---WED_TX_TKID_FREE_TKID_LDSRAM (0x15010000 + 0x1b8)---

    CNT[31..0]                   - (RO) Counter the number of TKID which are required access SRAM only

 =====================================================================================*/
#define WED_TX_TKID_FREE_TKID_LDSRAM_CNT_ADDR             WED_TX_TKID_FREE_TKID_LDSRAM_ADDR
#define WED_TX_TKID_FREE_TKID_LDSRAM_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_TX_TKID_FREE_TKID_LDSRAM_CNT_SHFT             0

/* =====================================================================================

  ---WED_TX_TKID_FREE_TKID_RECYCLE (0x15010000 + 0x1bc)---

    CNT[31..0]                   - (RO) Counter the number of TKID which are recycled by WED Tx TKID

 =====================================================================================*/
#define WED_TX_TKID_FREE_TKID_RECYCLE_CNT_ADDR            WED_TX_TKID_FREE_TKID_RECYCLE_ADDR
#define WED_TX_TKID_FREE_TKID_RECYCLE_CNT_MASK            0xFFFFFFFF                // CNT[31..0]
#define WED_TX_TKID_FREE_TKID_RECYCLE_CNT_SHFT            0

/* =====================================================================================

  ---WED_TX_FREE_TO_TX_TKID_TKID_MIB (0x15010000 + 0x1c0)---

    CNT[31..0]                   - (RC) Token_ID counter, from WED Tx Free Agent to Tx TKID Manager.

 =====================================================================================*/
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_CNT_ADDR          WED_TX_FREE_TO_TX_TKID_TKID_MIB_ADDR
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_CNT_MASK          0xFFFFFFFF                // CNT[31..0]
#define WED_TX_FREE_TO_TX_TKID_TKID_MIB_CNT_SHFT          0

/* =====================================================================================

  ---WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB (0x15010000 + 0x1c4)---

    CNT[31..0]                   - (RC) SKBID counter, from WED Tx Buffer Manager to WDMA Rx Driver Engine.

 =====================================================================================*/
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_CNT_ADDR       WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_ADDR
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB_CNT_SHFT       0

/* =====================================================================================

  ---WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB (0x15010000 + 0x1c8)---

    CNT[31..0]                   - (RC) DMAD read counter, RRO Rx Data Driver Engine read from WED Rx Buffer Manager.

 =====================================================================================*/
#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_ADDR       WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_ADDR
#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_RX_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_SHFT       0

/* =====================================================================================

  ---WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB (0x15010000 + 0x1cc)---

    CNT[31..0]                   - (RC) DMAD read counter, WPDMA Rx Data Driver Engine read from WED Rx Buffer Manager.

 =====================================================================================*/
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_CNT_ADDR     WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_ADDR
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_CNT_MASK     0xFFFFFFFF                // CNT[31..0]
#define WED_RX_BM_TO_WPDMA_RX_D_DRV_DMAD_MIB_CNT_SHFT     0

/* =====================================================================================

  ---WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB (0x15010000 + 0x1D4)---

    CNT[31..0]                   - (RC) Free DMAD counter from WED Tx TKID Manager to Tx Buffer Manager.

 =====================================================================================*/
#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_CNT_ADDR       WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_ADDR
#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_CNT_MASK       0xFFFFFFFF                // CNT[31..0]
#define WED_TX_TKID_TO_TX_BM_FREE_DMAD_MIB_CNT_SHFT       0

/* =====================================================================================

  ---WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB (0x15010000 + 0x1D8)---

    CNT[31..0]                   - (RC) DMAD read counter, RRO Rx Page Driver Engine read from WED Rx PG Buffer Manager.

 =====================================================================================*/
#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_ADDR    WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_ADDR
#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_MASK    0xFFFFFFFF                // CNT[31..0]
#define WED_RX_PG_BM_TO_RRO_RX_D_DRV_DMAD_MIB_CNT_SHFT    0

/* =====================================================================================

  ---WED_PMTR_CTRL (0x15010000 + 0x1DC)---

    RESERVED0[15..0]             - (RO) Reserved bits
    MAX_DIV[19..16]              - (RW) Divisor for latency maximum counter
    RESERVED20[23..20]           - (RO) Reserved bits
    ACC_DIV[27..24]              - (RW) Divisor for latency accumulated counter
    RESERVED28[30..28]           - (RO) Reserved bits
    EN[31]                       - (RW) Enable WED performance monitor

 =====================================================================================*/
#define WED_PMTR_CTRL_EN_ADDR                             WED_PMTR_CTRL_ADDR
#define WED_PMTR_CTRL_EN_MASK                             0x80000000                // EN[31]
#define WED_PMTR_CTRL_EN_SHFT                             31
#define WED_PMTR_CTRL_ACC_DIV_ADDR                        WED_PMTR_CTRL_ADDR
#define WED_PMTR_CTRL_ACC_DIV_MASK                        0x0F000000                // ACC_DIV[27..24]
#define WED_PMTR_CTRL_ACC_DIV_SHFT                        24
#define WED_PMTR_CTRL_MAX_DIV_ADDR                        WED_PMTR_CTRL_ADDR
#define WED_PMTR_CTRL_MAX_DIV_MASK                        0x000F0000                // MAX_DIV[19..16]
#define WED_PMTR_CTRL_MAX_DIV_SHFT                        16

/* =====================================================================================

  ---WED_PMTR_TGT (0x15010000 + 0X1E0)---

    MST0[4..0]                   - (RW)  xxx
    RESERVED5[7..5]              - (RO) Reserved bits
    MST1[12..8]                  - (RW)  xxx
    RESERVED13[15..13]           - (RO) Reserved bits
    MST2[20..16]                 - (RW)  xxx
    RESERVED21[23..21]           - (RO) Reserved bits
    MST3[28..24]                 - (RW) Select monitored target master
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PMTR_TGT_MST3_ADDR                            WED_PMTR_TGT_ADDR
#define WED_PMTR_TGT_MST3_MASK                            0x1F000000                // MST3[28..24]
#define WED_PMTR_TGT_MST3_SHFT                            24
#define WED_PMTR_TGT_MST2_ADDR                            WED_PMTR_TGT_ADDR
#define WED_PMTR_TGT_MST2_MASK                            0x001F0000                // MST2[20..16]
#define WED_PMTR_TGT_MST2_SHFT                            16
#define WED_PMTR_TGT_MST1_ADDR                            WED_PMTR_TGT_ADDR
#define WED_PMTR_TGT_MST1_MASK                            0x00001F00                // MST1[12..8]
#define WED_PMTR_TGT_MST1_SHFT                            8
#define WED_PMTR_TGT_MST0_ADDR                            WED_PMTR_TGT_ADDR
#define WED_PMTR_TGT_MST0_MASK                            0x0000001F                // MST0[4..0]
#define WED_PMTR_TGT_MST0_SHFT                            0

/* =====================================================================================

  ---WED_PMTR_TGT_ST (0x15010000 + 0X1E4)---

    MST0[7..0]                   - (RW)  xxx
    MST1[15..8]                  - (RW)  xxx
    MST2[23..16]                 - (RW)  xxx
    MST3[31..24]                 - (RW) Select the state of the monitored target master

 =====================================================================================*/
#define WED_PMTR_TGT_ST_MST3_ADDR                         WED_PMTR_TGT_ST_ADDR
#define WED_PMTR_TGT_ST_MST3_MASK                         0xFF000000                // MST3[31..24]
#define WED_PMTR_TGT_ST_MST3_SHFT                         24
#define WED_PMTR_TGT_ST_MST2_ADDR                         WED_PMTR_TGT_ST_ADDR
#define WED_PMTR_TGT_ST_MST2_MASK                         0x00FF0000                // MST2[23..16]
#define WED_PMTR_TGT_ST_MST2_SHFT                         16
#define WED_PMTR_TGT_ST_MST1_ADDR                         WED_PMTR_TGT_ST_ADDR
#define WED_PMTR_TGT_ST_MST1_MASK                         0x0000FF00                // MST1[15..8]
#define WED_PMTR_TGT_ST_MST1_SHFT                         8
#define WED_PMTR_TGT_ST_MST0_ADDR                         WED_PMTR_TGT_ST_ADDR
#define WED_PMTR_TGT_ST_MST0_MASK                         0x000000FF                // MST0[7..0]
#define WED_PMTR_TGT_ST_MST0_SHFT                         0

/* =====================================================================================

  ---WED_PMTR_LTCY_MAX0_1 (0x15010000 + 0X1E8)---

    MST0_CNT[15..0]              - (RC) Maximum latency record, unit is (cycle_count/2^WED_PMTR_CTRL.MAX_DIV)
    MST1_CNT[31..16]             - (RC) Maximum latency record, unit is (cycle_count/2^WED_PMTR_CTRL.MAX_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_MAX0_1_MST1_CNT_ADDR                WED_PMTR_LTCY_MAX0_1_ADDR
#define WED_PMTR_LTCY_MAX0_1_MST1_CNT_MASK                0xFFFF0000                // MST1_CNT[31..16]
#define WED_PMTR_LTCY_MAX0_1_MST1_CNT_SHFT                16
#define WED_PMTR_LTCY_MAX0_1_MST0_CNT_ADDR                WED_PMTR_LTCY_MAX0_1_ADDR
#define WED_PMTR_LTCY_MAX0_1_MST0_CNT_MASK                0x0000FFFF                // MST0_CNT[15..0]
#define WED_PMTR_LTCY_MAX0_1_MST0_CNT_SHFT                0

/* =====================================================================================

  ---WED_PMTR_LTCY_MAX2_3 (0x15010000 + 0X1EC)---

    MST2_CNT[15..0]              - (RC) Maximum latency record, unit is (cycle_count/2^WED_PMTR_CTRL.MAX_DIV)
    MST3_CNT[31..16]             - (RC) Maximum latency record, unit is (cycle_count/2^WED_PMTR_CTRL.MAX_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_MAX2_3_MST3_CNT_ADDR                WED_PMTR_LTCY_MAX2_3_ADDR
#define WED_PMTR_LTCY_MAX2_3_MST3_CNT_MASK                0xFFFF0000                // MST3_CNT[31..16]
#define WED_PMTR_LTCY_MAX2_3_MST3_CNT_SHFT                16
#define WED_PMTR_LTCY_MAX2_3_MST2_CNT_ADDR                WED_PMTR_LTCY_MAX2_3_ADDR
#define WED_PMTR_LTCY_MAX2_3_MST2_CNT_MASK                0x0000FFFF                // MST2_CNT[15..0]
#define WED_PMTR_LTCY_MAX2_3_MST2_CNT_SHFT                0

/* =====================================================================================

  ---WED_PMTR_LTCY_ACC0 (0x15010000 + 0X1F0)---

    CNT[31..0]                   - (RC) Accumulated latency count , unit is (cycle_count/2^WED_PMTR_CTRL.ACC_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_ACC0_CNT_ADDR                       WED_PMTR_LTCY_ACC0_ADDR
#define WED_PMTR_LTCY_ACC0_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_PMTR_LTCY_ACC0_CNT_SHFT                       0

/* =====================================================================================

  ---WED_PMTR_LTCY_ACC1 (0x15010000 + 0X1F4)---

    CNT[31..0]                   - (RC) Accumulated latency count , unit is (cycle_count/2^WED_PMTR_CTRL.ACC_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_ACC1_CNT_ADDR                       WED_PMTR_LTCY_ACC1_ADDR
#define WED_PMTR_LTCY_ACC1_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_PMTR_LTCY_ACC1_CNT_SHFT                       0

/* =====================================================================================

  ---WED_PMTR_LTCY_ACC2 (0x15010000 + 0X1F8)---

    CNT[31..0]                   - (RC) Accumulated latency count , unit is (cycle_count/2^WED_PMTR_CTRL.ACC_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_ACC2_CNT_ADDR                       WED_PMTR_LTCY_ACC2_ADDR
#define WED_PMTR_LTCY_ACC2_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_PMTR_LTCY_ACC2_CNT_SHFT                       0

/* =====================================================================================

  ---WED_PMTR_LTCY_ACC3 (0x15010000 + 0X1FC)---

    CNT[31..0]                   - (RC) Accumulated latency count , unit is (cycle_count/2^WED_PMTR_CTRL.ACC_DIV)

 =====================================================================================*/
#define WED_PMTR_LTCY_ACC3_CNT_ADDR                       WED_PMTR_LTCY_ACC3_ADDR
#define WED_PMTR_LTCY_ACC3_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_PMTR_LTCY_ACC3_CNT_SHFT                       0

/* =====================================================================================

  ---WED_INT_STA (0x15010000 + 0x200)---

    EVENT_0[0]                   - (W1C)  xxx
    EVENT_1[1]                   - (W1C)  xxx
    EVENT_2[2]                   - (W1C)  xxx
    EVENT_3[3]                   - (W1C)  xxx
    EVENT_4[4]                   - (W1C)  xxx
    EVENT_5[5]                   - (W1C)  xxx
    EVENT_6[6]                   - (W1C)  xxx
    EVENT_7[7]                   - (W1C)  xxx
    EVENT_8[8]                   - (W1C)  xxx
    EVENT_9[9]                   - (W1C)  xxx
    EVENT_10[10]                 - (W1C)  xxx
    EVENT_11[11]                 - (W1C)  xxx
    EVENT_12[12]                 - (W1C)  xxx
    EVENT_13[13]                 - (W1C)  xxx
    EVENT_14[14]                 - (W1C)  xxx
    EVENT_15[15]                 - (W1C)  xxx
    EVENT_16[16]                 - (W1C)  xxx
    EVENT_17[17]                 - (W1C)  xxx
    EVENT_18[18]                 - (W1C)  xxx
    EVENT_19[19]                 - (W1C)  xxx
    EVENT_20[20]                 - (W1C)  xxx
    EVENT_21[21]                 - (W1C)  xxx
    EVENT_22[22]                 - (W1C)  xxx
    EVENT_23[23]                 - (W1C)  xxx
    EVENT_24[24]                 - (W1C)  xxx
    EVENT_25[25]                 - (W1C)  xxx
    EVENT_26[26]                 - (W1C)  xxx
    EVENT_27[27]                 - (W1C)  xxx
    EVENT_28[28]                 - (W1C)  xxx
    EVENT_29[29]                 - (W1C)  xxx
    EVENT_30[30]                 - (W1C)  xxx
    EVENT_31[31]                 - (W1C) WiFi interrupt status, please refer WiFi DMA specification for each bit's interrupt event definition

 =====================================================================================*/
#define WED_INT_STA_EVENT_31_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_31_MASK                         0x80000000                // EVENT_31[31]
#define WED_INT_STA_EVENT_31_SHFT                         31
#define WED_INT_STA_EVENT_30_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_30_MASK                         0x40000000                // EVENT_30[30]
#define WED_INT_STA_EVENT_30_SHFT                         30
#define WED_INT_STA_EVENT_29_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_29_MASK                         0x20000000                // EVENT_29[29]
#define WED_INT_STA_EVENT_29_SHFT                         29
#define WED_INT_STA_EVENT_28_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_28_MASK                         0x10000000                // EVENT_28[28]
#define WED_INT_STA_EVENT_28_SHFT                         28
#define WED_INT_STA_EVENT_27_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_27_MASK                         0x08000000                // EVENT_27[27]
#define WED_INT_STA_EVENT_27_SHFT                         27
#define WED_INT_STA_EVENT_26_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_26_MASK                         0x04000000                // EVENT_26[26]
#define WED_INT_STA_EVENT_26_SHFT                         26
#define WED_INT_STA_EVENT_25_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_25_MASK                         0x02000000                // EVENT_25[25]
#define WED_INT_STA_EVENT_25_SHFT                         25
#define WED_INT_STA_EVENT_24_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_24_MASK                         0x01000000                // EVENT_24[24]
#define WED_INT_STA_EVENT_24_SHFT                         24
#define WED_INT_STA_EVENT_23_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_23_MASK                         0x00800000                // EVENT_23[23]
#define WED_INT_STA_EVENT_23_SHFT                         23
#define WED_INT_STA_EVENT_22_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_22_MASK                         0x00400000                // EVENT_22[22]
#define WED_INT_STA_EVENT_22_SHFT                         22
#define WED_INT_STA_EVENT_21_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_21_MASK                         0x00200000                // EVENT_21[21]
#define WED_INT_STA_EVENT_21_SHFT                         21
#define WED_INT_STA_EVENT_20_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_20_MASK                         0x00100000                // EVENT_20[20]
#define WED_INT_STA_EVENT_20_SHFT                         20
#define WED_INT_STA_EVENT_19_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_19_MASK                         0x00080000                // EVENT_19[19]
#define WED_INT_STA_EVENT_19_SHFT                         19
#define WED_INT_STA_EVENT_18_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_18_MASK                         0x00040000                // EVENT_18[18]
#define WED_INT_STA_EVENT_18_SHFT                         18
#define WED_INT_STA_EVENT_17_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_17_MASK                         0x00020000                // EVENT_17[17]
#define WED_INT_STA_EVENT_17_SHFT                         17
#define WED_INT_STA_EVENT_16_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_16_MASK                         0x00010000                // EVENT_16[16]
#define WED_INT_STA_EVENT_16_SHFT                         16
#define WED_INT_STA_EVENT_15_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_15_MASK                         0x00008000                // EVENT_15[15]
#define WED_INT_STA_EVENT_15_SHFT                         15
#define WED_INT_STA_EVENT_14_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_14_MASK                         0x00004000                // EVENT_14[14]
#define WED_INT_STA_EVENT_14_SHFT                         14
#define WED_INT_STA_EVENT_13_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_13_MASK                         0x00002000                // EVENT_13[13]
#define WED_INT_STA_EVENT_13_SHFT                         13
#define WED_INT_STA_EVENT_12_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_12_MASK                         0x00001000                // EVENT_12[12]
#define WED_INT_STA_EVENT_12_SHFT                         12
#define WED_INT_STA_EVENT_11_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_11_MASK                         0x00000800                // EVENT_11[11]
#define WED_INT_STA_EVENT_11_SHFT                         11
#define WED_INT_STA_EVENT_10_ADDR                         WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_10_MASK                         0x00000400                // EVENT_10[10]
#define WED_INT_STA_EVENT_10_SHFT                         10
#define WED_INT_STA_EVENT_9_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_9_MASK                          0x00000200                // EVENT_9[9]
#define WED_INT_STA_EVENT_9_SHFT                          9
#define WED_INT_STA_EVENT_8_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_8_MASK                          0x00000100                // EVENT_8[8]
#define WED_INT_STA_EVENT_8_SHFT                          8
#define WED_INT_STA_EVENT_7_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_7_MASK                          0x00000080                // EVENT_7[7]
#define WED_INT_STA_EVENT_7_SHFT                          7
#define WED_INT_STA_EVENT_6_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_6_MASK                          0x00000040                // EVENT_6[6]
#define WED_INT_STA_EVENT_6_SHFT                          6
#define WED_INT_STA_EVENT_5_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_5_MASK                          0x00000020                // EVENT_5[5]
#define WED_INT_STA_EVENT_5_SHFT                          5
#define WED_INT_STA_EVENT_4_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_4_MASK                          0x00000010                // EVENT_4[4]
#define WED_INT_STA_EVENT_4_SHFT                          4
#define WED_INT_STA_EVENT_3_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_3_MASK                          0x00000008                // EVENT_3[3]
#define WED_INT_STA_EVENT_3_SHFT                          3
#define WED_INT_STA_EVENT_2_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_2_MASK                          0x00000004                // EVENT_2[2]
#define WED_INT_STA_EVENT_2_SHFT                          2
#define WED_INT_STA_EVENT_1_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_1_MASK                          0x00000002                // EVENT_1[1]
#define WED_INT_STA_EVENT_1_SHFT                          1
#define WED_INT_STA_EVENT_0_ADDR                          WED_INT_STA_ADDR
#define WED_INT_STA_EVENT_0_MASK                          0x00000001                // EVENT_0[0]
#define WED_INT_STA_EVENT_0_SHFT                          0

/* =====================================================================================

  ---WED_INT_MSK (0x15010000 + 0x204)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[0]
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_MSK_EN_ADDR                               WED_INT_MSK_ADDR
#define WED_INT_MSK_EN_MASK                               0xFFFFFFFF                // EN[31..0]
#define WED_INT_MSK_EN_SHFT                               0

/* =====================================================================================

  ---WED_GLO_CFG (0x15010000 + 0x208)---

    TX_DMA_EN[0]                 - (RW) TX_DMA Enable
                                     1: Enable TX_DMA
                                     0: Disable TX_DMA
    TX_DMA_BUSY[1]               - (RO) TX_DMA Busy indicator
                                     1: TX_DMA is busy
                                     0: TX_DMA is not busy
    RX_DMA_EN[2]                 - (RW) RX_DMA Enable
                                     1: Enable RX_DMA
                                     0: Disable RX_DMA
    RX_DMA_BUSY[3]               - (RO) RX_DMA Busy indicator
                                     1: RX_DMA is busy
                                     0: RX_DMA is not busy
    RESERVED4[5..4]              - (RO) Reserved bits
    TX_WB_DDONE[6]               - (RW) Determine whether to write back DDONE
                                     0 :Disable TX_DMA writing back DDONE into TXD
                                     1 : Enable TX_DMA writing back DDONE into TXD
    BIG_ENDIAN[7]                - (RW) The endian mode selection. DMA applies the endian rule to convert payload and TX/RX information. DMA won't apply endian rule to register or descriptor.
                                     1: big endian.
                                     0: little endian.
    DESC_32B_E[8]                - (RW) Enable 32-Byte Descriptor Length
    WCOMPLETE_SEL[9]             - (RW) select the AXI wready or bvalid as write complete signal for TX DMA
    RESERVED10[10]               - (RO) Reserved bits
    RX_DMA_BURST_CFG[12..11]     - (RW) RX DMA burst length
    RESERVED13[28..13]           - (RO) Reserved bits
    BYTE_SWAP[29]                - (RW) Byte Swapping for TX/RX DMAD
                                     0: Not to swap (Endian of DMAD unchanged)
                                     1: Swap (Endian of DMAD reversed)
    CSR_CLKGATE_BYP[30]          - (RW) TX DMA Clock gated Bypass
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WED_GLO_CFG_CSR_CLKGATE_BYP_ADDR                  WED_GLO_CFG_ADDR
#define WED_GLO_CFG_CSR_CLKGATE_BYP_MASK                  0x40000000                // CSR_CLKGATE_BYP[30]
#define WED_GLO_CFG_CSR_CLKGATE_BYP_SHFT                  30
#define WED_GLO_CFG_BYTE_SWAP_ADDR                        WED_GLO_CFG_ADDR
#define WED_GLO_CFG_BYTE_SWAP_MASK                        0x20000000                // BYTE_SWAP[29]
#define WED_GLO_CFG_BYTE_SWAP_SHFT                        29
#define WED_GLO_CFG_RX_DMA_BURST_CFG_ADDR                 WED_GLO_CFG_ADDR
#define WED_GLO_CFG_RX_DMA_BURST_CFG_MASK                 0x00001800                // RX_DMA_BURST_CFG[12..11]
#define WED_GLO_CFG_RX_DMA_BURST_CFG_SHFT                 11
#define WED_GLO_CFG_WCOMPLETE_SEL_ADDR                    WED_GLO_CFG_ADDR
#define WED_GLO_CFG_WCOMPLETE_SEL_MASK                    0x00000200                // WCOMPLETE_SEL[9]
#define WED_GLO_CFG_WCOMPLETE_SEL_SHFT                    9
#define WED_GLO_CFG_DESC_32B_E_ADDR                       WED_GLO_CFG_ADDR
#define WED_GLO_CFG_DESC_32B_E_MASK                       0x00000100                // DESC_32B_E[8]
#define WED_GLO_CFG_DESC_32B_E_SHFT                       8
#define WED_GLO_CFG_BIG_ENDIAN_ADDR                       WED_GLO_CFG_ADDR
#define WED_GLO_CFG_BIG_ENDIAN_MASK                       0x00000080                // BIG_ENDIAN[7]
#define WED_GLO_CFG_BIG_ENDIAN_SHFT                       7
#define WED_GLO_CFG_TX_WB_DDONE_ADDR                      WED_GLO_CFG_ADDR
#define WED_GLO_CFG_TX_WB_DDONE_MASK                      0x00000040                // TX_WB_DDONE[6]
#define WED_GLO_CFG_TX_WB_DDONE_SHFT                      6
#define WED_GLO_CFG_RX_DMA_BUSY_ADDR                      WED_GLO_CFG_ADDR
#define WED_GLO_CFG_RX_DMA_BUSY_MASK                      0x00000008                // RX_DMA_BUSY[3]
#define WED_GLO_CFG_RX_DMA_BUSY_SHFT                      3
#define WED_GLO_CFG_RX_DMA_EN_ADDR                        WED_GLO_CFG_ADDR
#define WED_GLO_CFG_RX_DMA_EN_MASK                        0x00000004                // RX_DMA_EN[2]
#define WED_GLO_CFG_RX_DMA_EN_SHFT                        2
#define WED_GLO_CFG_TX_DMA_BUSY_ADDR                      WED_GLO_CFG_ADDR
#define WED_GLO_CFG_TX_DMA_BUSY_MASK                      0x00000002                // TX_DMA_BUSY[1]
#define WED_GLO_CFG_TX_DMA_BUSY_SHFT                      1
#define WED_GLO_CFG_TX_DMA_EN_ADDR                        WED_GLO_CFG_ADDR
#define WED_GLO_CFG_TX_DMA_EN_MASK                        0x00000001                // TX_DMA_EN[0]
#define WED_GLO_CFG_TX_DMA_EN_SHFT                        0

/* =====================================================================================

  ---WED_RST_IDX (0x15010000 + 0x20C)---

    DTX_IDX0[0]                  - (RW) Write 1 to reset to TX_DMA_IDX0 to 0 (Tx Data path, DMA index for Host CPU)
    DTX_IDX1[1]                  - (RW) Write 1 to reset to TX_DMA_IDX1 to 0 (Tx Data path, DMA index for Host CPU)
    RESERVED2[5..2]              - (RO) Reserved bits
    DRX_IDX0[6]                  - (RW) Write 1 to reset to RX_DMA_IDX0 to 0 (Rx Data path, DMA index for Host CPU)
    DRX_IDX1[7]                  - (RW) Write 1 to reset to RX_DMA_IDX1 to 0 (Rx Data path, DMA index for Host CPU)
    RESERVED8[29..8]             - (RO) Reserved bits
    WPDMA_DRX_IDX0[30]           - (RW) Write 1 to reset to RX_WPDMA_IDX0 to 0 (FreeDoneEvent Ring, mirrored DMA index for Host CPU)
    WPDMA_DRX_IDX1[31]           - (RW) Write 1 to reset to RX_WPDMA_IDX1 to 0 (FreeDoneEvent Ring, mirrored DMA index for Host CPU)

 =====================================================================================*/
#define WED_RST_IDX_WPDMA_DRX_IDX1_ADDR                   WED_RST_IDX_ADDR
#define WED_RST_IDX_WPDMA_DRX_IDX1_MASK                   0x80000000                // WPDMA_DRX_IDX1[31]
#define WED_RST_IDX_WPDMA_DRX_IDX1_SHFT                   31
#define WED_RST_IDX_WPDMA_DRX_IDX0_ADDR                   WED_RST_IDX_ADDR
#define WED_RST_IDX_WPDMA_DRX_IDX0_MASK                   0x40000000                // WPDMA_DRX_IDX0[30]
#define WED_RST_IDX_WPDMA_DRX_IDX0_SHFT                   30
#define WED_RST_IDX_DRX_IDX1_ADDR                         WED_RST_IDX_ADDR
#define WED_RST_IDX_DRX_IDX1_MASK                         0x00000080                // DRX_IDX1[7]
#define WED_RST_IDX_DRX_IDX1_SHFT                         7
#define WED_RST_IDX_DRX_IDX0_ADDR                         WED_RST_IDX_ADDR
#define WED_RST_IDX_DRX_IDX0_MASK                         0x00000040                // DRX_IDX0[6]
#define WED_RST_IDX_DRX_IDX0_SHFT                         6
#define WED_RST_IDX_DTX_IDX1_ADDR                         WED_RST_IDX_ADDR
#define WED_RST_IDX_DTX_IDX1_MASK                         0x00000002                // DTX_IDX1[1]
#define WED_RST_IDX_DTX_IDX1_SHFT                         1
#define WED_RST_IDX_DTX_IDX0_ADDR                         WED_RST_IDX_ADDR
#define WED_RST_IDX_DTX_IDX0_MASK                         0x00000001                // DTX_IDX0[0]
#define WED_RST_IDX_DTX_IDX0_SHFT                         0

/* =====================================================================================

  ---WED_DLY_INT_CFG (0x15010000 + 0x210)---

    RX_MAX_PTIME[7..0]           - (RW) Specified Max pending time for the internal RX_DONE_INT. When the pending time equal or greater RX_MAX_PTIME*20us or the number of pended RX_DONE_INT equal or greater than RX_MAX_PINT (see above), RX_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt time check
    RX_MAX_PINT[14..8]           - (RW) Specified Max number of pended interrupts.
                                     When the number of pended interrupts equal or greater than the value specified here or interrupt pending time reach the limit (See below), RX_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt count check
    RX_DLY_INT_EN[15]            - (RW) RX Delayed Interrupt Enable
    TX_MAX_PTIME[23..16]         - (RW) Specified Max pending time for the internal TX_DONE_INT. When the pending time equal or greater TX_MAX_PTIME*20us or the number of pended TX_DONE_INT equal or greater than TX_MAX_PINT (see above), TX_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt time check
    TX_MAX_PINT[30..24]          - (RW) Specified Max number of pended interrupts.
                                     When the number of pended interrupts equal or greater than the value specified here or interrupt pending time reach the limit (See below), TX_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt count check
    TX_DLY_INT_EN[31]            - (RW) TX Delayed Interrupt Enable

 =====================================================================================*/
#define WED_DLY_INT_CFG_TX_DLY_INT_EN_ADDR                WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_TX_DLY_INT_EN_MASK                0x80000000                // TX_DLY_INT_EN[31]
#define WED_DLY_INT_CFG_TX_DLY_INT_EN_SHFT                31
#define WED_DLY_INT_CFG_TX_MAX_PINT_ADDR                  WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_TX_MAX_PINT_MASK                  0x7F000000                // TX_MAX_PINT[30..24]
#define WED_DLY_INT_CFG_TX_MAX_PINT_SHFT                  24
#define WED_DLY_INT_CFG_TX_MAX_PTIME_ADDR                 WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_TX_MAX_PTIME_MASK                 0x00FF0000                // TX_MAX_PTIME[23..16]
#define WED_DLY_INT_CFG_TX_MAX_PTIME_SHFT                 16
#define WED_DLY_INT_CFG_RX_DLY_INT_EN_ADDR                WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_RX_DLY_INT_EN_MASK                0x00008000                // RX_DLY_INT_EN[15]
#define WED_DLY_INT_CFG_RX_DLY_INT_EN_SHFT                15
#define WED_DLY_INT_CFG_RX_MAX_PINT_ADDR                  WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_RX_MAX_PINT_MASK                  0x00007F00                // RX_MAX_PINT[14..8]
#define WED_DLY_INT_CFG_RX_MAX_PINT_SHFT                  8
#define WED_DLY_INT_CFG_RX_MAX_PTIME_ADDR                 WED_DLY_INT_CFG_ADDR
#define WED_DLY_INT_CFG_RX_MAX_PTIME_MASK                 0x000000FF                // RX_MAX_PTIME[7..0]
#define WED_DLY_INT_CFG_RX_MAX_PTIME_SHFT                 0

/* =====================================================================================

  ---WED_DLY_INT_CFG1 (0x15010000 + 0x214)---

    TX_FREE_MAX_PTIME[7..0]      - (RW) Specified Max pending time for the internal TX_FREE_DONE_INT. When the pending time equal or greater MAX_PTIME*20us or the number of pended TX_FREE_DONE_INT equal or greater than MAX_PINT (see above), TX_FREE_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt time check.
    TX_FREE_MAX_PINT[14..8]      - (RW) Specified Max number of pended interrupts.
                                     When the number of pended interrupts equal or greater than the value specified here or interrupt pending time reach the limit (See below), TX_FREE_DONE_INT is asserted to CPU.
                                     Set to 0 will disable pending interrupt count check
    TX_FREE_DLY_INT_EN[15]       - (RW) Tx Free Delayed Interrupt Enable
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_DLY_INT_CFG1_TX_FREE_DLY_INT_EN_ADDR          WED_DLY_INT_CFG1_ADDR
#define WED_DLY_INT_CFG1_TX_FREE_DLY_INT_EN_MASK          0x00008000                // TX_FREE_DLY_INT_EN[15]
#define WED_DLY_INT_CFG1_TX_FREE_DLY_INT_EN_SHFT          15
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PINT_ADDR            WED_DLY_INT_CFG1_ADDR
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PINT_MASK            0x00007F00                // TX_FREE_MAX_PINT[14..8]
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PINT_SHFT            8
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PTIME_ADDR           WED_DLY_INT_CFG1_ADDR
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PTIME_MASK           0x000000FF                // TX_FREE_MAX_PTIME[7..0]
#define WED_DLY_INT_CFG1_TX_FREE_MAX_PTIME_SHFT           0

/* =====================================================================================

  ---WED_INT_CTRL (0x15010000 + 0x218)---

    RX_DONE_ASSERT0[4..0]        - (RW) RX Queue#0 packet received interrupt bit selection
                                     Specify WED asserting which interrupt status bits to indicate host CPU.
    RESERVED5[7..5]              - (RO) Reserved bits
    RX_DONE_ASSERT1[12..8]       - (RW) RX Queue#1 packet received interrupt bit selection
                                     Specify WED asserting which interrupt status bits to indicate host CPU.
    RESERVED13[31..13]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_INT_CTRL_RX_DONE_ASSERT1_ADDR                 WED_INT_CTRL_ADDR
#define WED_INT_CTRL_RX_DONE_ASSERT1_MASK                 0x00001F00                // RX_DONE_ASSERT1[12..8]
#define WED_INT_CTRL_RX_DONE_ASSERT1_SHFT                 8
#define WED_INT_CTRL_RX_DONE_ASSERT0_ADDR                 WED_INT_CTRL_ADDR
#define WED_INT_CTRL_RX_DONE_ASSERT0_MASK                 0x0000001F                // RX_DONE_ASSERT0[4..0]
#define WED_INT_CTRL_RX_DONE_ASSERT0_SHFT                 0

/* =====================================================================================

  ---WED_SPR (0x15010000 + 0x21C)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_SPR_RSV_REG_ADDR                              WED_SPR_ADDR
#define WED_SPR_RSV_REG_MASK                              0xFFFFFFFF                // RSV_REG[31..0]
#define WED_SPR_RSV_REG_SHFT                              0

/* =====================================================================================

  ---WED_INT_MSK1 (0x15010000 + 0x224)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[1]
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_MSK1_EN_ADDR                              WED_INT_MSK1_ADDR
#define WED_INT_MSK1_EN_MASK                              0xFFFFFFFF                // EN[31..0]
#define WED_INT_MSK1_EN_SHFT                              0

/* =====================================================================================

  ---WED_INT_MSK2 (0x15010000 + 0x228)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[2]
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_MSK2_EN_ADDR                              WED_INT_MSK2_ADDR
#define WED_INT_MSK2_EN_MASK                              0xFFFFFFFF                // EN[31..0]
#define WED_INT_MSK2_EN_SHFT                              0

/* =====================================================================================

  ---WED_INT_MSK3 (0x15010000 + 0x22C)---

    EN[31..0]                    - (RW) Interrupt mask for WED_INT_STA, selected events will assert Interrupt signal bit[3]
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_MSK3_EN_ADDR                              WED_INT_MSK3_ADDR
#define WED_INT_MSK3_EN_MASK                              0xFFFFFFFF                // EN[31..0]
#define WED_INT_MSK3_EN_SHFT                              0

/* =====================================================================================

  ---WED_INT_STA_POST_MSK0 (0x15010000 + 0x230)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     WED_INT_STA_POST_MSK# = WED_INT_STA & WED_INT_MSK#
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_STA_POST_MSK0_EVENT_ADDR                  WED_INT_STA_POST_MSK0_ADDR
#define WED_INT_STA_POST_MSK0_EVENT_MASK                  0xFFFFFFFF                // EVENT[31..0]
#define WED_INT_STA_POST_MSK0_EVENT_SHFT                  0

/* =====================================================================================

  ---WED_INT_STA_POST_MSK1 (0x15010000 + 0x234)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_STA_POST_MSK1_EVENT_ADDR                  WED_INT_STA_POST_MSK1_ADDR
#define WED_INT_STA_POST_MSK1_EVENT_MASK                  0xFFFFFFFF                // EVENT[31..0]
#define WED_INT_STA_POST_MSK1_EVENT_SHFT                  0

/* =====================================================================================

  ---WED_INT_STA_POST_MSK2 (0x15010000 + 0x238)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_STA_POST_MSK2_EVENT_ADDR                  WED_INT_STA_POST_MSK2_ADDR
#define WED_INT_STA_POST_MSK2_EVENT_MASK                  0xFFFFFFFF                // EVENT[31..0]
#define WED_INT_STA_POST_MSK2_EVENT_SHFT                  0

/* =====================================================================================

  ---WED_INT_STA_POST_MSK3 (0x15010000 + 0x23C)---

    EVENT[31..0]                 - (RO) Post mask interrupt status.
                                     Bits definition all same as WED_INT_STA

 =====================================================================================*/
#define WED_INT_STA_POST_MSK3_EVENT_ADDR                  WED_INT_STA_POST_MSK3_ADDR
#define WED_INT_STA_POST_MSK3_EVENT_MASK                  0xFFFFFFFF                // EVENT[31..0]
#define WED_INT_STA_POST_MSK3_EVENT_SHFT                  0

/* =====================================================================================

  ---WED_SPR_0X24C (0x15010000 + 0x24C)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_SPR_0X24C_RSV_REG_ADDR                        WED_SPR_0X24C_ADDR
#define WED_SPR_0X24C_RSV_REG_MASK                        0xFFFFFFFF                // RSV_REG[31..0]
#define WED_SPR_0X24C_RSV_REG_SHFT                        0

/* =====================================================================================

  ---WED_SPR_RO_ZERO (0x15010000 + 0x258)---

    REG[31..0]                   - (RO) spare_cr

 =====================================================================================*/
#define WED_SPR_RO_ZERO_REG_ADDR                          WED_SPR_RO_ZERO_ADDR
#define WED_SPR_RO_ZERO_REG_MASK                          0xFFFFFFFF                // REG[31..0]
#define WED_SPR_RO_ZERO_REG_SHFT                          0

/* =====================================================================================

  ---WED_SPR_0X25C (0x15010000 + 0x25C)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_SPR_0X25C_RSV_REG_ADDR                        WED_SPR_0X25C_ADDR
#define WED_SPR_0X25C_RSV_REG_MASK                        0xFFFFFFFF                // RSV_REG[31..0]
#define WED_SPR_0X25C_RSV_REG_SHFT                        0

/* =====================================================================================

  ---WED_TX0_MIB (0x15010000 + 0x2a0)---

    PKT_CNT[31..0]               - (RC) WED Tx ring0 transmitted packet counter

 =====================================================================================*/
#define WED_TX0_MIB_PKT_CNT_ADDR                          WED_TX0_MIB_ADDR
#define WED_TX0_MIB_PKT_CNT_MASK                          0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_TX0_MIB_PKT_CNT_SHFT                          0

/* =====================================================================================

  ---WED_TX1_MIB (0x15010000 + 0x2a4)---

    PKT_CNT[31..0]               - (RC) WED Tx ring1 transmitted packet counter

 =====================================================================================*/
#define WED_TX1_MIB_PKT_CNT_ADDR                          WED_TX1_MIB_ADDR
#define WED_TX1_MIB_PKT_CNT_MASK                          0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_TX1_MIB_PKT_CNT_SHFT                          0

/* =====================================================================================

  ---WED_TX_COHERENT_MIB (0x15010000 + 0x2d0)---

    CNT[31..0]                   - (RC) WED Tx coherent event counter

 =====================================================================================*/
#define WED_TX_COHERENT_MIB_CNT_ADDR                      WED_TX_COHERENT_MIB_ADDR
#define WED_TX_COHERENT_MIB_CNT_MASK                      0xFFFFFFFF                // CNT[31..0]
#define WED_TX_COHERENT_MIB_CNT_SHFT                      0

/* =====================================================================================

  ---WED_RX0_MIB (0x15010000 + 0x2e0)---

    PKT_CNT[31..0]               - (RC) WED Tx Free ring0 received packet counter

 =====================================================================================*/
#define WED_RX0_MIB_PKT_CNT_ADDR                          WED_RX0_MIB_ADDR
#define WED_RX0_MIB_PKT_CNT_MASK                          0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_RX0_MIB_PKT_CNT_SHFT                          0

/* =====================================================================================

  ---WED_RX1_MIB (0x15010000 + 0x2e4)---

    PKT_CNT[31..0]               - (RC) WED Tx Free ring1 received packet counter

 =====================================================================================*/
#define WED_RX1_MIB_PKT_CNT_ADDR                          WED_RX1_MIB_ADDR
#define WED_RX1_MIB_PKT_CNT_MASK                          0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_RX1_MIB_PKT_CNT_SHFT                          0

/* =====================================================================================

  ---WED_RX_DMA_MIB_0 (0x15010000 + 0x2f0)---

    WR_CNT[31..0]                - (RC) FIFO write count

 =====================================================================================*/
#define WED_RX_DMA_MIB_0_WR_CNT_ADDR                      WED_RX_DMA_MIB_0_ADDR
#define WED_RX_DMA_MIB_0_WR_CNT_MASK                      0xFFFFFFFF                // WR_CNT[31..0]
#define WED_RX_DMA_MIB_0_WR_CNT_SHFT                      0

/* =====================================================================================

  ---WED_RX_DMA_MIB_1 (0x15010000 + 0x2f4)---

    RD_CNT[31..0]                - (RC) FIFO read count

 =====================================================================================*/
#define WED_RX_DMA_MIB_1_RD_CNT_ADDR                      WED_RX_DMA_MIB_1_ADDR
#define WED_RX_DMA_MIB_1_RD_CNT_MASK                      0xFFFFFFFF                // RD_CNT[31..0]
#define WED_RX_DMA_MIB_1_RD_CNT_SHFT                      0

/* =====================================================================================

  ---WED_RX_DMA_ERR (0x15010000 + 0x2f8)---

    CLR[0]                       - (A0) wed_rx_dma error status clear, write 1 clear, hw will write 0 if clear done.
                                     if error still exist, error status will not clear by this bit
    STS[31..1]                   - (RO) wed_rx_dma error status
                                     [1:0] rtqm write the disable ring fifo

 =====================================================================================*/
#define WED_RX_DMA_ERR_STS_ADDR                           WED_RX_DMA_ERR_ADDR
#define WED_RX_DMA_ERR_STS_MASK                           0xFFFFFFFE                // STS[31..1]
#define WED_RX_DMA_ERR_STS_SHFT                           1
#define WED_RX_DMA_ERR_CLR_ADDR                           WED_RX_DMA_ERR_ADDR
#define WED_RX_DMA_ERR_CLR_MASK                           0x00000001                // CLR[0]
#define WED_RX_DMA_ERR_CLR_SHFT                           0

/* =====================================================================================

  ---WED_TX0_CTRL0 (0x15010000 + 0x300)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TX_Ring0 (4-DWORD aligned address)

 =====================================================================================*/
#define WED_TX0_CTRL0_BASE_PTR_ADDR                       WED_TX0_CTRL0_ADDR
#define WED_TX0_CTRL0_BASE_PTR_MASK                       0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_TX0_CTRL0_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_TX0_CTRL1 (0x15010000 + 0x304)---

    MAX_CNT[11..0]               - (RW) The maximum number of TXD count in TXD_Ring0.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX0_CTRL1_BASE_PTR_H_ADDR                     WED_TX0_CTRL1_ADDR
#define WED_TX0_CTRL1_BASE_PTR_H_MASK                     0x00FF0000                // BASE_PTR_H[23..16]
#define WED_TX0_CTRL1_BASE_PTR_H_SHFT                     16
#define WED_TX0_CTRL1_MAX_CNT_ADDR                        WED_TX0_CTRL1_ADDR
#define WED_TX0_CTRL1_MAX_CNT_MASK                        0x00000FFF                // MAX_CNT[11..0]
#define WED_TX0_CTRL1_MAX_CNT_SHFT                        0

/* =====================================================================================

  ---WED_TX0_CTRL2 (0x15010000 + 0x308)---

    CPU_IDX[11..0]               - (RW) Point to the next TXD CPU wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX0_CTRL2_CPU_IDX_ADDR                        WED_TX0_CTRL2_ADDR
#define WED_TX0_CTRL2_CPU_IDX_MASK                        0x00000FFF                // CPU_IDX[11..0]
#define WED_TX0_CTRL2_CPU_IDX_SHFT                        0

/* =====================================================================================

  ---WED_TX0_CTRL3 (0x15010000 + 0x30c)---

    DMA_IDX[11..0]               - (RO) Point to the next TXD DMA wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX0_CTRL3_DMA_IDX_ADDR                        WED_TX0_CTRL3_ADDR
#define WED_TX0_CTRL3_DMA_IDX_MASK                        0x00000FFF                // DMA_IDX[11..0]
#define WED_TX0_CTRL3_DMA_IDX_SHFT                        0

/* =====================================================================================

  ---WED_TX1_CTRL0 (0x15010000 + 0x310)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TX_Ring1 (4-DWORD aligned address)

 =====================================================================================*/
#define WED_TX1_CTRL0_BASE_PTR_ADDR                       WED_TX1_CTRL0_ADDR
#define WED_TX1_CTRL0_BASE_PTR_MASK                       0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_TX1_CTRL0_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_TX1_CTRL1 (0x15010000 + 0x314)---

    MAX_CNT[11..0]               - (RW) The maximum number of TXD count in TXD_Ring1.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX1_CTRL1_BASE_PTR_H_ADDR                     WED_TX1_CTRL1_ADDR
#define WED_TX1_CTRL1_BASE_PTR_H_MASK                     0x00FF0000                // BASE_PTR_H[23..16]
#define WED_TX1_CTRL1_BASE_PTR_H_SHFT                     16
#define WED_TX1_CTRL1_MAX_CNT_ADDR                        WED_TX1_CTRL1_ADDR
#define WED_TX1_CTRL1_MAX_CNT_MASK                        0x00000FFF                // MAX_CNT[11..0]
#define WED_TX1_CTRL1_MAX_CNT_SHFT                        0

/* =====================================================================================

  ---WED_TX1_CTRL2 (0x15010000 + 0x318)---

    CPU_IDX[11..0]               - (RW) Point to the next TXD CPU wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX1_CTRL2_CPU_IDX_ADDR                        WED_TX1_CTRL2_ADDR
#define WED_TX1_CTRL2_CPU_IDX_MASK                        0x00000FFF                // CPU_IDX[11..0]
#define WED_TX1_CTRL2_CPU_IDX_SHFT                        0

/* =====================================================================================

  ---WED_TX1_CTRL3 (0x15010000 + 0x31c)---

    DMA_IDX[11..0]               - (RO) Point to the next TXD DMA wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX1_CTRL3_DMA_IDX_ADDR                        WED_TX1_CTRL3_ADDR
#define WED_TX1_CTRL3_DMA_IDX_MASK                        0x00000FFF                // DMA_IDX[11..0]
#define WED_TX1_CTRL3_DMA_IDX_SHFT                        0

/* =====================================================================================

  ---WED_HIFTXD_BASE00_L (0x15010000 + 0x320)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 00 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE00_L_PTR_ADDR                      WED_HIFTXD_BASE00_L_ADDR
#define WED_HIFTXD_BASE00_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE00_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE01_L (0x15010000 + 0x324)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 01 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE01_L_PTR_ADDR                      WED_HIFTXD_BASE01_L_ADDR
#define WED_HIFTXD_BASE01_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE01_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE02_L (0x15010000 + 0x328)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 02 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE02_L_PTR_ADDR                      WED_HIFTXD_BASE02_L_ADDR
#define WED_HIFTXD_BASE02_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE02_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE03_L (0x15010000 + 0x32c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 03 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE03_L_PTR_ADDR                      WED_HIFTXD_BASE03_L_ADDR
#define WED_HIFTXD_BASE03_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE03_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE04_L (0x15010000 + 0x330)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 04 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE04_L_PTR_ADDR                      WED_HIFTXD_BASE04_L_ADDR
#define WED_HIFTXD_BASE04_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE04_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE05_L (0x15010000 + 0x334)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 05 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE05_L_PTR_ADDR                      WED_HIFTXD_BASE05_L_ADDR
#define WED_HIFTXD_BASE05_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE05_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE06_L (0x15010000 + 0x338)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 06 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE06_L_PTR_ADDR                      WED_HIFTXD_BASE06_L_ADDR
#define WED_HIFTXD_BASE06_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE06_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE07_L (0x15010000 + 0x33c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 07 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE07_L_PTR_ADDR                      WED_HIFTXD_BASE07_L_ADDR
#define WED_HIFTXD_BASE07_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE07_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE08_L (0x15010000 + 0x340)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 08 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE08_L_PTR_ADDR                      WED_HIFTXD_BASE08_L_ADDR
#define WED_HIFTXD_BASE08_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE08_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE09_L (0x15010000 + 0x344)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 09 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE09_L_PTR_ADDR                      WED_HIFTXD_BASE09_L_ADDR
#define WED_HIFTXD_BASE09_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE09_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE10_L (0x15010000 + 0x348)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 10 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE10_L_PTR_ADDR                      WED_HIFTXD_BASE10_L_ADDR
#define WED_HIFTXD_BASE10_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE10_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE11_L (0x15010000 + 0x34c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 11 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE11_L_PTR_ADDR                      WED_HIFTXD_BASE11_L_ADDR
#define WED_HIFTXD_BASE11_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE11_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE12_L (0x15010000 + 0x350)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 12 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE12_L_PTR_ADDR                      WED_HIFTXD_BASE12_L_ADDR
#define WED_HIFTXD_BASE12_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE12_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE13_L (0x15010000 + 0x354)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 13 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE13_L_PTR_ADDR                      WED_HIFTXD_BASE13_L_ADDR
#define WED_HIFTXD_BASE13_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE13_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE14_L (0x15010000 + 0x358)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 14 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE14_L_PTR_ADDR                      WED_HIFTXD_BASE14_L_ADDR
#define WED_HIFTXD_BASE14_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE14_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE15_L (0x15010000 + 0x35c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 15 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE15_L_PTR_ADDR                      WED_HIFTXD_BASE15_L_ADDR
#define WED_HIFTXD_BASE15_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE15_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE16_L (0x15010000 + 0x360)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 16 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE16_L_PTR_ADDR                      WED_HIFTXD_BASE16_L_ADDR
#define WED_HIFTXD_BASE16_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE16_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE17_L (0x15010000 + 0x364)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 17 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE17_L_PTR_ADDR                      WED_HIFTXD_BASE17_L_ADDR
#define WED_HIFTXD_BASE17_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE17_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE18_L (0x15010000 + 0x368)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 18 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE18_L_PTR_ADDR                      WED_HIFTXD_BASE18_L_ADDR
#define WED_HIFTXD_BASE18_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE18_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE19_L (0x15010000 + 0x36c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 19 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE19_L_PTR_ADDR                      WED_HIFTXD_BASE19_L_ADDR
#define WED_HIFTXD_BASE19_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE19_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE20_L (0x15010000 + 0x370)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 20 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE20_L_PTR_ADDR                      WED_HIFTXD_BASE20_L_ADDR
#define WED_HIFTXD_BASE20_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE20_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE21_L (0x15010000 + 0x374)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 21 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE21_L_PTR_ADDR                      WED_HIFTXD_BASE21_L_ADDR
#define WED_HIFTXD_BASE21_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE21_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE22_L (0x15010000 + 0x378)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 22 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE22_L_PTR_ADDR                      WED_HIFTXD_BASE22_L_ADDR
#define WED_HIFTXD_BASE22_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE22_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE23_L (0x15010000 + 0x37c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 23 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE23_L_PTR_ADDR                      WED_HIFTXD_BASE23_L_ADDR
#define WED_HIFTXD_BASE23_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE23_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE24_L (0x15010000 + 0x380)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 24 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE24_L_PTR_ADDR                      WED_HIFTXD_BASE24_L_ADDR
#define WED_HIFTXD_BASE24_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE24_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE25_L (0x15010000 + 0x384)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 25 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE25_L_PTR_ADDR                      WED_HIFTXD_BASE25_L_ADDR
#define WED_HIFTXD_BASE25_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE25_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE26_L (0x15010000 + 0x388)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 26 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE26_L_PTR_ADDR                      WED_HIFTXD_BASE26_L_ADDR
#define WED_HIFTXD_BASE26_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE26_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE27_L (0x15010000 + 0x38c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 27 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE27_L_PTR_ADDR                      WED_HIFTXD_BASE27_L_ADDR
#define WED_HIFTXD_BASE27_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE27_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE28_L (0x15010000 + 0x390)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 28 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE28_L_PTR_ADDR                      WED_HIFTXD_BASE28_L_ADDR
#define WED_HIFTXD_BASE28_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE28_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE29_L (0x15010000 + 0x394)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 29 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE29_L_PTR_ADDR                      WED_HIFTXD_BASE29_L_ADDR
#define WED_HIFTXD_BASE29_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE29_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE30_L (0x15010000 + 0x398)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 30 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE30_L_PTR_ADDR                      WED_HIFTXD_BASE30_L_ADDR
#define WED_HIFTXD_BASE30_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE30_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE31_L (0x15010000 + 0x39c)---

    PTR[31..0]                   - (RW) HIFTXD base pointer 31 [31:0]

 =====================================================================================*/
#define WED_HIFTXD_BASE31_L_PTR_ADDR                      WED_HIFTXD_BASE31_L_ADDR
#define WED_HIFTXD_BASE31_L_PTR_MASK                      0xFFFFFFFF                // PTR[31..0]
#define WED_HIFTXD_BASE31_L_PTR_SHFT                      0

/* =====================================================================================

  ---WED_HIFTXD_BASE_00_07_H (0x15010000 + 0x3a0)---

    BASE07[3..0]                 - (RW) HIFTXD base pointer 07 [35:32]
    BASE06[7..4]                 - (RW) HIFTXD base pointer 06 [35:32]
    BASE05[11..8]                - (RW) HIFTXD base pointer 05 [35:32]
    BASE04[15..12]               - (RW) HIFTXD base pointer 04 [35:32]
    BASE03[19..16]               - (RW) HIFTXD base pointer 03 [35:32]
    BASE02[23..20]               - (RW) HIFTXD base pointer 02 [35:32]
    BASE01[27..24]               - (RW) HIFTXD base pointer 01 [35:32]
    BASE00[31..28]               - (RW) HIFTXD base pointer 00 [35:32]

 =====================================================================================*/
#define WED_HIFTXD_BASE_00_07_H_BASE00_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE00_MASK               0xF0000000                // BASE00[31..28]
#define WED_HIFTXD_BASE_00_07_H_BASE00_SHFT               28
#define WED_HIFTXD_BASE_00_07_H_BASE01_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE01_MASK               0x0F000000                // BASE01[27..24]
#define WED_HIFTXD_BASE_00_07_H_BASE01_SHFT               24
#define WED_HIFTXD_BASE_00_07_H_BASE02_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE02_MASK               0x00F00000                // BASE02[23..20]
#define WED_HIFTXD_BASE_00_07_H_BASE02_SHFT               20
#define WED_HIFTXD_BASE_00_07_H_BASE03_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE03_MASK               0x000F0000                // BASE03[19..16]
#define WED_HIFTXD_BASE_00_07_H_BASE03_SHFT               16
#define WED_HIFTXD_BASE_00_07_H_BASE04_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE04_MASK               0x0000F000                // BASE04[15..12]
#define WED_HIFTXD_BASE_00_07_H_BASE04_SHFT               12
#define WED_HIFTXD_BASE_00_07_H_BASE05_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE05_MASK               0x00000F00                // BASE05[11..8]
#define WED_HIFTXD_BASE_00_07_H_BASE05_SHFT               8
#define WED_HIFTXD_BASE_00_07_H_BASE06_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE06_MASK               0x000000F0                // BASE06[7..4]
#define WED_HIFTXD_BASE_00_07_H_BASE06_SHFT               4
#define WED_HIFTXD_BASE_00_07_H_BASE07_ADDR               WED_HIFTXD_BASE_00_07_H_ADDR
#define WED_HIFTXD_BASE_00_07_H_BASE07_MASK               0x0000000F                // BASE07[3..0]
#define WED_HIFTXD_BASE_00_07_H_BASE07_SHFT               0

/* =====================================================================================

  ---WED_HIFTXD_BASE_08_15_H (0x15010000 + 0x3a4)---

    BASE15[3..0]                 - (RW) HIFTXD base pointer 15 [35:32]
    BASE14[7..4]                 - (RW) HIFTXD base pointer 14 [35:32]
    BASE13[11..8]                - (RW) HIFTXD base pointer 13 [35:32]
    BASE12[15..12]               - (RW) HIFTXD base pointer 12 [35:32]
    BASE11[19..16]               - (RW) HIFTXD base pointer 11 [35:32]
    BASE10[23..20]               - (RW) HIFTXD base pointer 10 [35:32]
    BASE09[27..24]               - (RW) HIFTXD base pointer 09 [35:32]
    BASE08[31..28]               - (RW) HIFTXD base pointer 08 [35:32]

 =====================================================================================*/
#define WED_HIFTXD_BASE_08_15_H_BASE08_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE08_MASK               0xF0000000                // BASE08[31..28]
#define WED_HIFTXD_BASE_08_15_H_BASE08_SHFT               28
#define WED_HIFTXD_BASE_08_15_H_BASE09_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE09_MASK               0x0F000000                // BASE09[27..24]
#define WED_HIFTXD_BASE_08_15_H_BASE09_SHFT               24
#define WED_HIFTXD_BASE_08_15_H_BASE10_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE10_MASK               0x00F00000                // BASE10[23..20]
#define WED_HIFTXD_BASE_08_15_H_BASE10_SHFT               20
#define WED_HIFTXD_BASE_08_15_H_BASE11_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE11_MASK               0x000F0000                // BASE11[19..16]
#define WED_HIFTXD_BASE_08_15_H_BASE11_SHFT               16
#define WED_HIFTXD_BASE_08_15_H_BASE12_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE12_MASK               0x0000F000                // BASE12[15..12]
#define WED_HIFTXD_BASE_08_15_H_BASE12_SHFT               12
#define WED_HIFTXD_BASE_08_15_H_BASE13_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE13_MASK               0x00000F00                // BASE13[11..8]
#define WED_HIFTXD_BASE_08_15_H_BASE13_SHFT               8
#define WED_HIFTXD_BASE_08_15_H_BASE14_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE14_MASK               0x000000F0                // BASE14[7..4]
#define WED_HIFTXD_BASE_08_15_H_BASE14_SHFT               4
#define WED_HIFTXD_BASE_08_15_H_BASE15_ADDR               WED_HIFTXD_BASE_08_15_H_ADDR
#define WED_HIFTXD_BASE_08_15_H_BASE15_MASK               0x0000000F                // BASE15[3..0]
#define WED_HIFTXD_BASE_08_15_H_BASE15_SHFT               0

/* =====================================================================================

  ---WED_HIFTXD_BASE_16_23_H (0x15010000 + 0x3a8)---

    BASE23[3..0]                 - (RW) HIFTXD base pointer 23 [35:32]
    BASE22[7..4]                 - (RW) HIFTXD base pointer 22 [35:32]
    BASE21[11..8]                - (RW) HIFTXD base pointer 21 [35:32]
    BASE20[15..12]               - (RW) HIFTXD base pointer 20 [35:32]
    BASE19[19..16]               - (RW) HIFTXD base pointer 19 [35:32]
    BASE18[23..20]               - (RW) HIFTXD base pointer 18 [35:32]
    BASE17[27..24]               - (RW) HIFTXD base pointer 17 [35:32]
    BASE16[31..28]               - (RW) HIFTXD base pointer 16 [35:32]

 =====================================================================================*/
#define WED_HIFTXD_BASE_16_23_H_BASE16_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE16_MASK               0xF0000000                // BASE16[31..28]
#define WED_HIFTXD_BASE_16_23_H_BASE16_SHFT               28
#define WED_HIFTXD_BASE_16_23_H_BASE17_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE17_MASK               0x0F000000                // BASE17[27..24]
#define WED_HIFTXD_BASE_16_23_H_BASE17_SHFT               24
#define WED_HIFTXD_BASE_16_23_H_BASE18_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE18_MASK               0x00F00000                // BASE18[23..20]
#define WED_HIFTXD_BASE_16_23_H_BASE18_SHFT               20
#define WED_HIFTXD_BASE_16_23_H_BASE19_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE19_MASK               0x000F0000                // BASE19[19..16]
#define WED_HIFTXD_BASE_16_23_H_BASE19_SHFT               16
#define WED_HIFTXD_BASE_16_23_H_BASE20_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE20_MASK               0x0000F000                // BASE20[15..12]
#define WED_HIFTXD_BASE_16_23_H_BASE20_SHFT               12
#define WED_HIFTXD_BASE_16_23_H_BASE21_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE21_MASK               0x00000F00                // BASE21[11..8]
#define WED_HIFTXD_BASE_16_23_H_BASE21_SHFT               8
#define WED_HIFTXD_BASE_16_23_H_BASE22_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE22_MASK               0x000000F0                // BASE22[7..4]
#define WED_HIFTXD_BASE_16_23_H_BASE22_SHFT               4
#define WED_HIFTXD_BASE_16_23_H_BASE23_ADDR               WED_HIFTXD_BASE_16_23_H_ADDR
#define WED_HIFTXD_BASE_16_23_H_BASE23_MASK               0x0000000F                // BASE23[3..0]
#define WED_HIFTXD_BASE_16_23_H_BASE23_SHFT               0

/* =====================================================================================

  ---WED_HIFTXD_BASE_24_31_H (0x15010000 + 0x3ac)---

    BASE31[3..0]                 - (RW) HIFTXD base pointer 31 [35:32]
    BASE30[7..4]                 - (RW) HIFTXD base pointer 30 [35:32]
    BASE29[11..8]                - (RW) HIFTXD base pointer 29 [35:32]
    BASE28[15..12]               - (RW) HIFTXD base pointer 28 [35:32]
    BASE27[19..16]               - (RW) HIFTXD base pointer 27 [35:32]
    BASE26[23..20]               - (RW) HIFTXD base pointer 26 [35:32]
    BASE25[27..24]               - (RW) HIFTXD base pointer 25 [35:32]
    BASE24[31..28]               - (RW) HIFTXD base pointer 24 [35:32]

 =====================================================================================*/
#define WED_HIFTXD_BASE_24_31_H_BASE24_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE24_MASK               0xF0000000                // BASE24[31..28]
#define WED_HIFTXD_BASE_24_31_H_BASE24_SHFT               28
#define WED_HIFTXD_BASE_24_31_H_BASE25_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE25_MASK               0x0F000000                // BASE25[27..24]
#define WED_HIFTXD_BASE_24_31_H_BASE25_SHFT               24
#define WED_HIFTXD_BASE_24_31_H_BASE26_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE26_MASK               0x00F00000                // BASE26[23..20]
#define WED_HIFTXD_BASE_24_31_H_BASE26_SHFT               20
#define WED_HIFTXD_BASE_24_31_H_BASE27_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE27_MASK               0x000F0000                // BASE27[19..16]
#define WED_HIFTXD_BASE_24_31_H_BASE27_SHFT               16
#define WED_HIFTXD_BASE_24_31_H_BASE28_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE28_MASK               0x0000F000                // BASE28[15..12]
#define WED_HIFTXD_BASE_24_31_H_BASE28_SHFT               12
#define WED_HIFTXD_BASE_24_31_H_BASE29_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE29_MASK               0x00000F00                // BASE29[11..8]
#define WED_HIFTXD_BASE_24_31_H_BASE29_SHFT               8
#define WED_HIFTXD_BASE_24_31_H_BASE30_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE30_MASK               0x000000F0                // BASE30[7..4]
#define WED_HIFTXD_BASE_24_31_H_BASE30_SHFT               4
#define WED_HIFTXD_BASE_24_31_H_BASE31_ADDR               WED_HIFTXD_BASE_24_31_H_ADDR
#define WED_HIFTXD_BASE_24_31_H_BASE31_MASK               0x0000000F                // BASE31[3..0]
#define WED_HIFTXD_BASE_24_31_H_BASE31_SHFT               0

/* =====================================================================================

  ---WED_SCR0 (0x15010000 + 0x3c0)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR0_REG_ADDR                                 WED_SCR0_ADDR
#define WED_SCR0_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR0_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR1 (0x15010000 + 0x3c4)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR1_REG_ADDR                                 WED_SCR1_ADDR
#define WED_SCR1_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR1_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR2 (0x15010000 + 0x3c8)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR2_REG_ADDR                                 WED_SCR2_ADDR
#define WED_SCR2_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR2_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR3 (0x15010000 + 0x3cc)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR3_REG_ADDR                                 WED_SCR3_ADDR
#define WED_SCR3_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR3_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR4 (0x15010000 + 0x3d0)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR4_REG_ADDR                                 WED_SCR4_ADDR
#define WED_SCR4_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR4_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR5 (0x15010000 + 0x3d4)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR5_REG_ADDR                                 WED_SCR5_ADDR
#define WED_SCR5_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR5_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR6 (0x15010000 + 0x3d8)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR6_REG_ADDR                                 WED_SCR6_ADDR
#define WED_SCR6_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR6_REG_SHFT                                 0

/* =====================================================================================

  ---WED_SCR7 (0x15010000 + 0x3dc)---

    REG[31..0]                   - (RW) Scratchpad registers for programmer to hold data temporarily.

 =====================================================================================*/
#define WED_SCR7_REG_ADDR                                 WED_SCR7_ADDR
#define WED_SCR7_REG_MASK                                 0xFFFFFFFF                // REG[31..0]
#define WED_SCR7_REG_SHFT                                 0

/* =====================================================================================

  ---WED_RX0_CTRL0 (0x15010000 + 0x400)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TxFree RXD Ring #0. It should be a 4-DWORD aligned address

 =====================================================================================*/
#define WED_RX0_CTRL0_BASE_PTR_ADDR                       WED_RX0_CTRL0_ADDR
#define WED_RX0_CTRL0_BASE_PTR_MASK                       0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_RX0_CTRL0_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_RX0_CTRL1 (0x15010000 + 0x404)---

    MAX_CNT[11..0]               - (RW) The maximum number of RXD count in RXD Ring #0.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX0_CTRL1_BASE_PTR_H_ADDR                     WED_RX0_CTRL1_ADDR
#define WED_RX0_CTRL1_BASE_PTR_H_MASK                     0x00FF0000                // BASE_PTR_H[23..16]
#define WED_RX0_CTRL1_BASE_PTR_H_SHFT                     16
#define WED_RX0_CTRL1_MAX_CNT_ADDR                        WED_RX0_CTRL1_ADDR
#define WED_RX0_CTRL1_MAX_CNT_MASK                        0x00000FFF                // MAX_CNT[11..0]
#define WED_RX0_CTRL1_MAX_CNT_SHFT                        0

/* =====================================================================================

  ---WED_RX0_CTRL2 (0x15010000 + 0x408)---

    CPU_IDX[11..0]               - (RW) Point to the next RXD CPU wants to allocate to RXD Ring #0.
                                     WED hardware will copy this value to WiFi side WPDMA RX CPU_IDX
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX0_CTRL2_CPU_IDX_ADDR                        WED_RX0_CTRL2_ADDR
#define WED_RX0_CTRL2_CPU_IDX_MASK                        0x00000FFF                // CPU_IDX[11..0]
#define WED_RX0_CTRL2_CPU_IDX_SHFT                        0

/* =====================================================================================

  ---WED_RX0_CTRL3 (0x15010000 + 0x40c)---

    DMA_IDX[11..0]               - (RO) Point to the next RXD DMA wants to use in FDS Ring#2. It should be a 4-DWORD aligned address.
                                     WED hardware will sync the DMA_IDX of WiFi side WPDMA RX to this register after RX event/packet processing.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX0_CTRL3_DMA_IDX_ADDR                        WED_RX0_CTRL3_ADDR
#define WED_RX0_CTRL3_DMA_IDX_MASK                        0x00000FFF                // DMA_IDX[11..0]
#define WED_RX0_CTRL3_DMA_IDX_SHFT                        0

/* =====================================================================================

  ---WED_RX1_CTRL0 (0x15010000 + 0x410)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TxFree RXD Ring #1. It should be a 4-DWORD aligned address

 =====================================================================================*/
#define WED_RX1_CTRL0_BASE_PTR_ADDR                       WED_RX1_CTRL0_ADDR
#define WED_RX1_CTRL0_BASE_PTR_MASK                       0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_RX1_CTRL0_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_RX1_CTRL1 (0x15010000 + 0x414)---

    MAX_CNT[11..0]               - (RW) The maximum number of RXD count in RXD Ring #1.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX1_CTRL1_BASE_PTR_H_ADDR                     WED_RX1_CTRL1_ADDR
#define WED_RX1_CTRL1_BASE_PTR_H_MASK                     0x00FF0000                // BASE_PTR_H[23..16]
#define WED_RX1_CTRL1_BASE_PTR_H_SHFT                     16
#define WED_RX1_CTRL1_MAX_CNT_ADDR                        WED_RX1_CTRL1_ADDR
#define WED_RX1_CTRL1_MAX_CNT_MASK                        0x00000FFF                // MAX_CNT[11..0]
#define WED_RX1_CTRL1_MAX_CNT_SHFT                        0

/* =====================================================================================

  ---WED_RX1_CTRL2 (0x15010000 + 0x418)---

    CPU_IDX[11..0]               - (RW) Point to the next RXD CPU wants to allocate to RXD Ring #1.
                                     WED hardware will copy this value to WiFi side WPDMA RX CPU_IDX
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX1_CTRL2_CPU_IDX_ADDR                        WED_RX1_CTRL2_ADDR
#define WED_RX1_CTRL2_CPU_IDX_MASK                        0x00000FFF                // CPU_IDX[11..0]
#define WED_RX1_CTRL2_CPU_IDX_SHFT                        0

/* =====================================================================================

  ---WED_RX1_CTRL3 (0x15010000 + 0x41c)---

    DMA_IDX[11..0]               - (RO) Point to the next RXD DMA wants to use in FDS Ring#2. It should be a 4-DWORD aligned address.
                                     WED hardware will sync the DMA_IDX of WiFi side WPDMA RX to this register after RX event/packet processing.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX1_CTRL3_DMA_IDX_ADDR                        WED_RX1_CTRL3_ADDR
#define WED_RX1_CTRL3_DMA_IDX_MASK                        0x00000FFF                // DMA_IDX[11..0]
#define WED_RX1_CTRL3_DMA_IDX_SHFT                        0

/* =====================================================================================

  ---WED_RX_BASE_PTR_0 (0x15010000 + 0x420)---

    BASE_PTR[31..0]              - (RW) Point to the base address of RX Ring #0 (2-DW aligned address)

 =====================================================================================*/
#define WED_RX_BASE_PTR_0_BASE_PTR_ADDR                   WED_RX_BASE_PTR_0_ADDR
#define WED_RX_BASE_PTR_0_BASE_PTR_MASK                   0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_RX_BASE_PTR_0_BASE_PTR_SHFT                   0

/* =====================================================================================

  ---WED_RX_MAX_CNT_0 (0x15010000 + 0x424)---

    MAX_CNT[11..0]               - (RW) The maximum number of RXD count in RX Ring #0
                                     Set 0 is disable this ring, driver does not access Ring.
                                     Set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MAX_CNT_0_BASE_PTR_H_ADDR                  WED_RX_MAX_CNT_0_ADDR
#define WED_RX_MAX_CNT_0_BASE_PTR_H_MASK                  0x00FF0000                // BASE_PTR_H[23..16]
#define WED_RX_MAX_CNT_0_BASE_PTR_H_SHFT                  16
#define WED_RX_MAX_CNT_0_MAX_CNT_ADDR                     WED_RX_MAX_CNT_0_ADDR
#define WED_RX_MAX_CNT_0_MAX_CNT_MASK                     0x00000FFF                // MAX_CNT[11..0]
#define WED_RX_MAX_CNT_0_MAX_CNT_SHFT                     0

/* =====================================================================================

  ---WED_RX_CRX_IDX_0 (0x15010000 + 0x428)---

    CRX_IDX[11..0]               - (RW) Point to the next RXD CPU wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_CRX_IDX_0_CRX_IDX_ADDR                     WED_RX_CRX_IDX_0_ADDR
#define WED_RX_CRX_IDX_0_CRX_IDX_MASK                     0x00000FFF                // CRX_IDX[11..0]
#define WED_RX_CRX_IDX_0_CRX_IDX_SHFT                     0

/* =====================================================================================

  ---WED_RX_DRX_IDX_0 (0x15010000 + 0x42c)---

    DRX_IDX[11..0]               - (RO) Point to the next RXD DMA wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_DRX_IDX_0_DRX_IDX_ADDR                     WED_RX_DRX_IDX_0_ADDR
#define WED_RX_DRX_IDX_0_DRX_IDX_MASK                     0x00000FFF                // DRX_IDX[11..0]
#define WED_RX_DRX_IDX_0_DRX_IDX_SHFT                     0

/* =====================================================================================

  ---WED_RX_BASE_PTR_1 (0x15010000 + 0x430)---

    BASE_PTR[31..0]              - (RW) Point to the base address of RX Ring #1 (2-DW aligned address)

 =====================================================================================*/
#define WED_RX_BASE_PTR_1_BASE_PTR_ADDR                   WED_RX_BASE_PTR_1_ADDR
#define WED_RX_BASE_PTR_1_BASE_PTR_MASK                   0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_RX_BASE_PTR_1_BASE_PTR_SHFT                   0

/* =====================================================================================

  ---WED_RX_MAX_CNT_1 (0x15010000 + 0x434)---

    MAX_CNT[11..0]               - (RW) The maximum number of RXD count in RX Ring #1
                                     Set 0 is disable this ring, driver does not access Ring.
                                     Set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MAX_CNT_1_BASE_PTR_H_ADDR                  WED_RX_MAX_CNT_1_ADDR
#define WED_RX_MAX_CNT_1_BASE_PTR_H_MASK                  0x00FF0000                // BASE_PTR_H[23..16]
#define WED_RX_MAX_CNT_1_BASE_PTR_H_SHFT                  16
#define WED_RX_MAX_CNT_1_MAX_CNT_ADDR                     WED_RX_MAX_CNT_1_ADDR
#define WED_RX_MAX_CNT_1_MAX_CNT_MASK                     0x00000FFF                // MAX_CNT[11..0]
#define WED_RX_MAX_CNT_1_MAX_CNT_SHFT                     0

/* =====================================================================================

  ---WED_RX_CRX_IDX_1 (0x15010000 + 0x438)---

    CRX_IDX[11..0]               - (RW) Point to the next RXD CPU wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_CRX_IDX_1_CRX_IDX_ADDR                     WED_RX_CRX_IDX_1_ADDR
#define WED_RX_CRX_IDX_1_CRX_IDX_MASK                     0x00000FFF                // CRX_IDX[11..0]
#define WED_RX_CRX_IDX_1_CRX_IDX_SHFT                     0

/* =====================================================================================

  ---WED_RX_DRX_IDX_1 (0x15010000 + 0x43c)---

    DRX_IDX[11..0]               - (RO) Point to the next RXD DMA wants to use
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_DRX_IDX_1_DRX_IDX_ADDR                     WED_RX_DRX_IDX_1_ADDR
#define WED_RX_DRX_IDX_1_DRX_IDX_MASK                     0x00000FFF                // DRX_IDX[11..0]
#define WED_RX_DRX_IDX_1_DRX_IDX_SHFT                     0

/* =====================================================================================

  ---WED_WPDMA_INT_STA_REC (0x15010000 + 0x500)---

    INT_STA[31..0]               - (RO) Record the last WED read original WPDMA_INT_STA value from WiFi card.

 =====================================================================================*/
#define WED_WPDMA_INT_STA_REC_INT_STA_ADDR                WED_WPDMA_INT_STA_REC_ADDR
#define WED_WPDMA_INT_STA_REC_INT_STA_MASK                0xFFFFFFFF                // INT_STA[31..0]
#define WED_WPDMA_INT_STA_REC_INT_STA_SHFT                0

/* =====================================================================================

  ---WED_WPDMA_INT_TRIG (0x15010000 + 0x504)---

    INT_TRIG[31..0]              - (RW) Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA. Each bit's definition is same as WPDMA_INT_STA value of WiFi card.

 =====================================================================================*/
#define WED_WPDMA_INT_TRIG_INT_TRIG_ADDR                  WED_WPDMA_INT_TRIG_ADDR
#define WED_WPDMA_INT_TRIG_INT_TRIG_MASK                  0xFFFFFFFF                // INT_TRIG[31..0]
#define WED_WPDMA_INT_TRIG_INT_TRIG_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_GLO_CFG (0x15010000 + 0x508)---

    TX_DRV_EN[0]                 - (RW) Tx Driver Module Enable
    TX_DRV_BUSY[1]               - (RO) Tx Driver Module Busy indicator
    RX_DRV_EN[2]                 - (RW) Rx Driver Module Enable
    RX_DRV_BUSY[3]               - (RO) Rx Driver Module Busy indicator
    RX_DRV_RING0_PKT_PROC[4]     - (RW) Rx Driver Ring0 packet processing enable
                                     Must NOT modify this register during Rx Driver Module Enable(rx_drv_en=1), set 0 for debug.
    RX_DRV_RING1_PKT_PROC[5]     - (RW) Rx Driver Ring1 packet processing enable
                                     Must NOT modify this register during Rx Driver Module Enable(rx_drv_en=1), set 0 for debug.
    RX_DRV_RING0_CRX_SYNC[6]     - (RW) Rx Driver Ring0 CRX sync process enable
                                     Must NOT modify this register during Rx Driver Module Enable(rx_drv_en=1), set 0 for debug.
    RX_DRV_RING1_CRX_SYNC[7]     - (RW) Rx Driver Ring1 CRX sync process enable
                                     Must NOT modify this register during Rx Driver Module Enable(rx_drv_en=1), set 0 for debug.
    csr_tx_bready_always_equal_1_en[8] - (RW) in order to saving waitting time of bvalid from slave, so when enable =1, the WPDMA_TX_DRV bready response always =1.
    RESERVED9[10..9]             - (RO) Reserved bits
    RSV0[11]                     - (RW) Reserved
    RX_DRV_EVENT_PKT_FMT_VER[15..12] - (RW) Assign the event packet format version if Rx Driver module will not automatically identify the event packet format.
    RSV2[17..16]                 - (RW) Reserved
    RX_DRV_UNS_VER_FORCE_4[18]   - (RW) Force unrecognized free notify version to ver=4
    RX_DRV_UNSUPPORT_FMT_BYP[19] - (RW) The event packet will be bypassed if the version of event packet format is unsupported. WED only support the event packet which the format version is 0x0, 0x1 or 0x2
    RX_DRV_EVENT_PKT_FMT_CHK[20] - (RW) WPDMA Rx Driver module will automatically identify the event packet format version and decide how to parse it. If this feature is disabled, event packet format version applied by WPDMA Rx Driver module need to be assigned by SW.
    RX_DDONE2_WR[21]             - (RW) WPDMA Rx Drvier write DDONE2 bit of the Rx descriptor after the operation finished.
                                     0: Not to write DDONE2 bit
                                     1: Write DDONE2=1
    RSV1[23..22]                 - (RW) Reserved
    TX_TKID_KEEP[24]             - (RW) Keep the original Tx token_ID value when modifying TXD/TXP
                                     When total HW+SW allocated SKB over 8K, tx_tkid_keep must be 0
    TX_DDONE_CHK_LAST[25]        - (RW) DDONE check mechanism, when TX_DDONE_CHK=1
    TXD_VER[26]                  - (RW) Select TXD Version
    RESERVED27[27]               - (RO) Reserved bits
    TX_DMAD_DW3_ZERO[28]         - (RW) Not to preserve TX DMAD DW3
    BYTE_SWAP[29]                - (RW) Byte Swapping for TX/RX DMAD
                                     0: Not to swap (Endian of DMAD unchanged)
                                     1: Swap (Endian of DMAD reversed)
    TX_DDONE_CHK[30]             - (RW) Check the DDONE bit of the descriptor before tx_done release.
                                     0: Not check DDONE, save DRAM reading access for each descriptor, directly move CPU release index to as DMA index of WiFi card
                                     1: Read and check if DDONE=1, before move on CPU release index. WPDMA_GLO_CFG.tx_wb_ddone of WiFi card must also be enabled.
    RX_2B_OFFSET[31]             - (RW) RX PBF 2-byte Offset
                                     1: Skip the first two bytes of the RX PBF
                                     0: Not to skip the first two bytes of the RX PBF

 =====================================================================================*/
#define WED_WPDMA_GLO_CFG_RX_2B_OFFSET_ADDR               WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_2B_OFFSET_MASK               0x80000000                // RX_2B_OFFSET[31]
#define WED_WPDMA_GLO_CFG_RX_2B_OFFSET_SHFT               31
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_ADDR               WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_MASK               0x40000000                // TX_DDONE_CHK[30]
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_SHFT               30
#define WED_WPDMA_GLO_CFG_BYTE_SWAP_ADDR                  WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_BYTE_SWAP_MASK                  0x20000000                // BYTE_SWAP[29]
#define WED_WPDMA_GLO_CFG_BYTE_SWAP_SHFT                  29
#define WED_WPDMA_GLO_CFG_TX_DMAD_DW3_ZERO_ADDR           WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_DMAD_DW3_ZERO_MASK           0x10000000                // TX_DMAD_DW3_ZERO[28]
#define WED_WPDMA_GLO_CFG_TX_DMAD_DW3_ZERO_SHFT           28
#define WED_WPDMA_GLO_CFG_TXD_VER_ADDR                    WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TXD_VER_MASK                    0x04000000                // TXD_VER[26]
#define WED_WPDMA_GLO_CFG_TXD_VER_SHFT                    26
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_LAST_ADDR          WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_LAST_MASK          0x02000000                // TX_DDONE_CHK_LAST[25]
#define WED_WPDMA_GLO_CFG_TX_DDONE_CHK_LAST_SHFT          25
#define WED_WPDMA_GLO_CFG_TX_TKID_KEEP_ADDR               WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_TKID_KEEP_MASK               0x01000000                // TX_TKID_KEEP[24]
#define WED_WPDMA_GLO_CFG_TX_TKID_KEEP_SHFT               24
#define WED_WPDMA_GLO_CFG_RSV1_ADDR                       WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RSV1_MASK                       0x00C00000                // RSV1[23..22]
#define WED_WPDMA_GLO_CFG_RSV1_SHFT                       22
#define WED_WPDMA_GLO_CFG_RX_DDONE2_WR_ADDR               WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DDONE2_WR_MASK               0x00200000                // RX_DDONE2_WR[21]
#define WED_WPDMA_GLO_CFG_RX_DDONE2_WR_SHFT               21
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_CHK_ADDR   WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_CHK_MASK   0x00100000                // RX_DRV_EVENT_PKT_FMT_CHK[20]
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_CHK_SHFT   20
#define WED_WPDMA_GLO_CFG_RX_DRV_UNSUPPORT_FMT_BYP_ADDR   WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_UNSUPPORT_FMT_BYP_MASK   0x00080000                // RX_DRV_UNSUPPORT_FMT_BYP[19]
#define WED_WPDMA_GLO_CFG_RX_DRV_UNSUPPORT_FMT_BYP_SHFT   19
#define WED_WPDMA_GLO_CFG_RX_DRV_UNS_VER_FORCE_4_ADDR     WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_UNS_VER_FORCE_4_MASK     0x00040000                // RX_DRV_UNS_VER_FORCE_4[18]
#define WED_WPDMA_GLO_CFG_RX_DRV_UNS_VER_FORCE_4_SHFT     18
#define WED_WPDMA_GLO_CFG_RSV2_ADDR                       WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RSV2_MASK                       0x00030000                // RSV2[17..16]
#define WED_WPDMA_GLO_CFG_RSV2_SHFT                       16
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_VER_ADDR   WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_VER_MASK   0x0000F000                // RX_DRV_EVENT_PKT_FMT_VER[15..12]
#define WED_WPDMA_GLO_CFG_RX_DRV_EVENT_PKT_FMT_VER_SHFT   12
#define WED_WPDMA_GLO_CFG_RSV0_ADDR                       WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RSV0_MASK                       0x00000800                // RSV0[11]
#define WED_WPDMA_GLO_CFG_RSV0_SHFT                       11
#define WED_WPDMA_GLO_CFG_csr_tx_bready_always_equal_1_en_ADDR WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_csr_tx_bready_always_equal_1_en_MASK 0x00000100                // csr_tx_bready_always_equal_1_en[8]
#define WED_WPDMA_GLO_CFG_csr_tx_bready_always_equal_1_en_SHFT 8
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_CRX_SYNC_ADDR      WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_CRX_SYNC_MASK      0x00000080                // RX_DRV_RING1_CRX_SYNC[7]
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_CRX_SYNC_SHFT      7
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_CRX_SYNC_ADDR      WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_CRX_SYNC_MASK      0x00000040                // RX_DRV_RING0_CRX_SYNC[6]
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_CRX_SYNC_SHFT      6
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_PKT_PROC_ADDR      WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_PKT_PROC_MASK      0x00000020                // RX_DRV_RING1_PKT_PROC[5]
#define WED_WPDMA_GLO_CFG_RX_DRV_RING1_PKT_PROC_SHFT      5
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_PKT_PROC_ADDR      WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_PKT_PROC_MASK      0x00000010                // RX_DRV_RING0_PKT_PROC[4]
#define WED_WPDMA_GLO_CFG_RX_DRV_RING0_PKT_PROC_SHFT      4
#define WED_WPDMA_GLO_CFG_RX_DRV_BUSY_ADDR                WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_BUSY_MASK                0x00000008                // RX_DRV_BUSY[3]
#define WED_WPDMA_GLO_CFG_RX_DRV_BUSY_SHFT                3
#define WED_WPDMA_GLO_CFG_RX_DRV_EN_ADDR                  WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_RX_DRV_EN_MASK                  0x00000004                // RX_DRV_EN[2]
#define WED_WPDMA_GLO_CFG_RX_DRV_EN_SHFT                  2
#define WED_WPDMA_GLO_CFG_TX_DRV_BUSY_ADDR                WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_DRV_BUSY_MASK                0x00000002                // TX_DRV_BUSY[1]
#define WED_WPDMA_GLO_CFG_TX_DRV_BUSY_SHFT                1
#define WED_WPDMA_GLO_CFG_TX_DRV_EN_ADDR                  WED_WPDMA_GLO_CFG_ADDR
#define WED_WPDMA_GLO_CFG_TX_DRV_EN_MASK                  0x00000001                // TX_DRV_EN[0]
#define WED_WPDMA_GLO_CFG_TX_DRV_EN_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_RST_IDX (0x15010000 + 0x50C)---

    CTX_IDX0[0]                  - (RW) Write 1 to reset to TX_DRV_IDX0 to 0 (Tx data path, index for WFDMA)
    CTX_IDX1[1]                  - (RW) Write 1 to reset to TX_DRV_IDX1 to 0 (Tx data path, index for WFDMA)
    RESERVED2[15..2]             - (RO) Reserved bits
    CRX_IDX0[16]                 - (RW) If WiFi side WPDMA ring be reset, these related bits must also be reset.
                                     Write 1 to reset to RX_DRV_IDX0 to 0
                                     (FreeDoneEvent Ring, index for WFDMA)
    CRX_IDX1[17]                 - (RW) If WiFi side WPDMA ring be reset, these related bits must also be reset.
                                     Write 1 to reset to RX_DRV_IDX1 to 0
                                     (FreeDoneEvent Ring, index for WFDMA)
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RST_IDX_CRX_IDX1_ADDR                   WED_WPDMA_RST_IDX_ADDR
#define WED_WPDMA_RST_IDX_CRX_IDX1_MASK                   0x00020000                // CRX_IDX1[17]
#define WED_WPDMA_RST_IDX_CRX_IDX1_SHFT                   17
#define WED_WPDMA_RST_IDX_CRX_IDX0_ADDR                   WED_WPDMA_RST_IDX_ADDR
#define WED_WPDMA_RST_IDX_CRX_IDX0_MASK                   0x00010000                // CRX_IDX0[16]
#define WED_WPDMA_RST_IDX_CRX_IDX0_SHFT                   16
#define WED_WPDMA_RST_IDX_CTX_IDX1_ADDR                   WED_WPDMA_RST_IDX_ADDR
#define WED_WPDMA_RST_IDX_CTX_IDX1_MASK                   0x00000002                // CTX_IDX1[1]
#define WED_WPDMA_RST_IDX_CTX_IDX1_SHFT                   1
#define WED_WPDMA_RST_IDX_CTX_IDX0_ADDR                   WED_WPDMA_RST_IDX_ADDR
#define WED_WPDMA_RST_IDX_CTX_IDX0_MASK                   0x00000001                // CTX_IDX0[0]
#define WED_WPDMA_RST_IDX_CTX_IDX0_SHFT                   0

/* =====================================================================================

  ---WED_WPDMA_BUS_CFG (0x15010000 + 0x510)---

    TX_WR_BND_4KB_BST[0]         - (RW) Enable WPDMA_TX_DRV AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    TX_RD_BND_4KB_BST[1]         - (RW) Enable WPDMA_TX_DRV AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    RESERVED2[15..2]             - (RO) Reserved bits
    RX_WR_BND_4KB_BST[16]        - (RW) Enable WPDMA_RX_DRV(TxFreeDoneEvent) AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RX_RD_BND_4KB_BST[17]        - (RW) Enable WPDMA_RX_DRV(TxFreeDoneEvent) AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_BUS_CFG_RX_RD_BND_4KB_BST_ADDR          WED_WPDMA_BUS_CFG_ADDR
#define WED_WPDMA_BUS_CFG_RX_RD_BND_4KB_BST_MASK          0x00020000                // RX_RD_BND_4KB_BST[17]
#define WED_WPDMA_BUS_CFG_RX_RD_BND_4KB_BST_SHFT          17
#define WED_WPDMA_BUS_CFG_RX_WR_BND_4KB_BST_ADDR          WED_WPDMA_BUS_CFG_ADDR
#define WED_WPDMA_BUS_CFG_RX_WR_BND_4KB_BST_MASK          0x00010000                // RX_WR_BND_4KB_BST[16]
#define WED_WPDMA_BUS_CFG_RX_WR_BND_4KB_BST_SHFT          16
#define WED_WPDMA_BUS_CFG_TX_RD_BND_4KB_BST_ADDR          WED_WPDMA_BUS_CFG_ADDR
#define WED_WPDMA_BUS_CFG_TX_RD_BND_4KB_BST_MASK          0x00000002                // TX_RD_BND_4KB_BST[1]
#define WED_WPDMA_BUS_CFG_TX_RD_BND_4KB_BST_SHFT          1
#define WED_WPDMA_BUS_CFG_TX_WR_BND_4KB_BST_ADDR          WED_WPDMA_BUS_CFG_ADDR
#define WED_WPDMA_BUS_CFG_TX_WR_BND_4KB_BST_MASK          0x00000001                // TX_WR_BND_4KB_BST[0]
#define WED_WPDMA_BUS_CFG_TX_WR_BND_4KB_BST_SHFT          0

/* =====================================================================================

  ---WED_WPDMA_TX_DLY_CIDX (0x15010000 + 0x514)---

    RESERVED0[7..0]              - (RO) Reserved bits
    RING_LOWTH[15..8]            - (RW) Specified low threshold for DMA side CIDX leading DIDX. When the DMA transmit-able valid DMAD count equal or lower RING_LOWTH, WED asserts write CPU index transaction to WFDMA.
                                     Set to 0 will disable ring low threshold check
    MAX_PTIME[23..16]            - (RW) Specified Max pending time for the internal index. When the pending time equal or greater MAX_PTIME*1us or the number of pended index equal or greater than MAX_PIDX (see above), WED asserts write CPU index transaction to WFDMA.
                                     Set to 0 will disable pending index time check
    MAX_PIDX[30..24]             - (RW) Specified Max number of pended index.
                                     When the number of pended index equal or greater than the value specified here or index pending time reach the limit (See below), WED asserts write CPU index transaction to WFDMA.
                                     Set to 0 will disable pending index count check
    EN[31]                       - (RW) WED Delayed CPU index Write to WFDMA Tx

 =====================================================================================*/
#define WED_WPDMA_TX_DLY_CIDX_EN_ADDR                     WED_WPDMA_TX_DLY_CIDX_ADDR
#define WED_WPDMA_TX_DLY_CIDX_EN_MASK                     0x80000000                // EN[31]
#define WED_WPDMA_TX_DLY_CIDX_EN_SHFT                     31
#define WED_WPDMA_TX_DLY_CIDX_MAX_PIDX_ADDR               WED_WPDMA_TX_DLY_CIDX_ADDR
#define WED_WPDMA_TX_DLY_CIDX_MAX_PIDX_MASK               0x7F000000                // MAX_PIDX[30..24]
#define WED_WPDMA_TX_DLY_CIDX_MAX_PIDX_SHFT               24
#define WED_WPDMA_TX_DLY_CIDX_MAX_PTIME_ADDR              WED_WPDMA_TX_DLY_CIDX_ADDR
#define WED_WPDMA_TX_DLY_CIDX_MAX_PTIME_MASK              0x00FF0000                // MAX_PTIME[23..16]
#define WED_WPDMA_TX_DLY_CIDX_MAX_PTIME_SHFT              16
#define WED_WPDMA_TX_DLY_CIDX_RING_LOWTH_ADDR             WED_WPDMA_TX_DLY_CIDX_ADDR
#define WED_WPDMA_TX_DLY_CIDX_RING_LOWTH_MASK             0x0000FF00                // RING_LOWTH[15..8]
#define WED_WPDMA_TX_DLY_CIDX_RING_LOWTH_SHFT             8

/* =====================================================================================

  ---WED_WPDMA_CTRL (0x15010000 + 0x518)---

    SDL1_MAX[13..0]              - (RW) WiFi WPDMA SDL1 maximum length
    RESERVED14[30..14]           - (RO) Reserved bits
    SDL1_FIXED[31]               - (RW) WiFi WPDMA SDL1 by fixed value

 =====================================================================================*/
#define WED_WPDMA_CTRL_SDL1_FIXED_ADDR                    WED_WPDMA_CTRL_ADDR
#define WED_WPDMA_CTRL_SDL1_FIXED_MASK                    0x80000000                // SDL1_FIXED[31]
#define WED_WPDMA_CTRL_SDL1_FIXED_SHFT                    31
#define WED_WPDMA_CTRL_SDL1_MAX_ADDR                      WED_WPDMA_CTRL_ADDR
#define WED_WPDMA_CTRL_SDL1_MAX_MASK                      0x00003FFF                // SDL1_MAX[13..0]
#define WED_WPDMA_CTRL_SDL1_MAX_SHFT                      0

/* =====================================================================================

  ---WED_WPDMA_FORCE_PROC (0x15010000 + 0x51C)---

    TX_DONE0[0]                  - (A0) TX Queue#0 packet transmit process control
    TX_DONE1[1]                  - (A0) TX Queue#1 packet transmit process control
    RESERVED2[9..2]              - (RO) Reserved bits
    RRO3_1_RX_DONE[10]           - (A0) RRO3.1 RX Queue#0 interrupt control
    RESERVED11[11]               - (RO) Reserved bits
    RRO_RX_INT0[12]              - (A0) RRO RX Queue#0 interrupt control
    RRO_RX_INT1[13]              - (A0) RRO RX Queue#1 interrupt control
    RESERVED14[15..14]           - (RO) Reserved bits
    TX_FREE_DONE0[16]            - (A0) TX Free Queue#0 packet transmit process control
    TX_FREE_DONE1[17]            - (A0) TX Free Queue#1 packet transmit process control
    RESERVED18[23..18]           - (RO) Reserved bits
    MSDU_PG_INT0[24]             - (A0) MSDU PG Queue#0 interrupt control
    MSDU_PG_INT1[25]             - (A0) MSDU PG Queue#1 interrupt control
    MSDU_PG_INT2[26]             - (A0) MSDU PG Queue#2 interrupt control
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT2_ADDR            WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT2_MASK            0x04000000                // MSDU_PG_INT2[26]
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT2_SHFT            26
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT1_ADDR            WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT1_MASK            0x02000000                // MSDU_PG_INT1[25]
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT1_SHFT            25
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT0_ADDR            WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT0_MASK            0x01000000                // MSDU_PG_INT0[24]
#define WED_WPDMA_FORCE_PROC_MSDU_PG_INT0_SHFT            24
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE1_ADDR           WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE1_MASK           0x00020000                // TX_FREE_DONE1[17]
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE1_SHFT           17
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE0_ADDR           WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE0_MASK           0x00010000                // TX_FREE_DONE0[16]
#define WED_WPDMA_FORCE_PROC_TX_FREE_DONE0_SHFT           16
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT1_ADDR             WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT1_MASK             0x00002000                // RRO_RX_INT1[13]
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT1_SHFT             13
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT0_ADDR             WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT0_MASK             0x00001000                // RRO_RX_INT0[12]
#define WED_WPDMA_FORCE_PROC_RRO_RX_INT0_SHFT             12
#define WED_WPDMA_FORCE_PROC_RRO3_1_RX_DONE_ADDR          WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_RRO3_1_RX_DONE_MASK          0x00000400                // RRO3_1_RX_DONE[10]
#define WED_WPDMA_FORCE_PROC_RRO3_1_RX_DONE_SHFT          10
#define WED_WPDMA_FORCE_PROC_TX_DONE1_ADDR                WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_TX_DONE1_MASK                0x00000002                // TX_DONE1[1]
#define WED_WPDMA_FORCE_PROC_TX_DONE1_SHFT                1
#define WED_WPDMA_FORCE_PROC_TX_DONE0_ADDR                WED_WPDMA_FORCE_PROC_ADDR
#define WED_WPDMA_FORCE_PROC_TX_DONE0_MASK                0x00000001                // TX_DONE0[0]
#define WED_WPDMA_FORCE_PROC_TX_DONE0_SHFT                0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL (0x15010000 + 0x520)---

    MSK_ON_DLY_PRD[7..0]         - (RW) Delay period to Postpone turning on interrupt mask register
                                     delay period = prd * 16 * AXI_clock_period
    POLL_PRD[15..8]              - (RW) WiFi CONNSYS interrupt status polling period
                                     polling period = poll_prd * 16 * AXI_clock_period
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=1)
    SRC_SEL[17..16]              - (RW) WiFi CONNSYS interrupt signal source selection
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=1 and system comprises WiFi CONNSYS, amount of WED/CONNSYS depends on system configuration)
    RESERVED18[19..18]           - (RO) Reserved bits
    MSK_EN_POLA[20]              - (RW) WiFi WPDMA interrupt mask polarity to enable interrupt
    RESERVED21[21]               - (RO) Reserved bits
    SIG_SRC[22]                  - (RW) Interrupt signal source type
    MSK_ON_DLY[23]               - (RW) Postpone turning on interrupt mask register
    POLL_MODE[25..24]            - (RW) WiFi CONNSYS interrupt status polling mode
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=1)
    RESERVED26[27..26]           - (RO) Reserved bits
    MSK_EN[28]                   - (RW) WiFi WPDMA Interrupt Mask control
    RESERVED29[31..29]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_MSK_EN_ADDR                    WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_MSK_EN_MASK                    0x10000000                // MSK_EN[28]
#define WED_WPDMA_INT_CTRL_MSK_EN_SHFT                    28
#define WED_WPDMA_INT_CTRL_POLL_MODE_ADDR                 WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_POLL_MODE_MASK                 0x03000000                // POLL_MODE[25..24]
#define WED_WPDMA_INT_CTRL_POLL_MODE_SHFT                 24
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_ADDR                WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_MASK                0x00800000                // MSK_ON_DLY[23]
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_SHFT                23
#define WED_WPDMA_INT_CTRL_SIG_SRC_ADDR                   WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_SIG_SRC_MASK                   0x00400000                // SIG_SRC[22]
#define WED_WPDMA_INT_CTRL_SIG_SRC_SHFT                   22
#define WED_WPDMA_INT_CTRL_MSK_EN_POLA_ADDR               WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_MSK_EN_POLA_MASK               0x00100000                // MSK_EN_POLA[20]
#define WED_WPDMA_INT_CTRL_MSK_EN_POLA_SHFT               20
#define WED_WPDMA_INT_CTRL_SRC_SEL_ADDR                   WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_SRC_SEL_MASK                   0x00030000                // SRC_SEL[17..16]
#define WED_WPDMA_INT_CTRL_SRC_SEL_SHFT                   16
#define WED_WPDMA_INT_CTRL_POLL_PRD_ADDR                  WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_POLL_PRD_MASK                  0x0000FF00                // POLL_PRD[15..8]
#define WED_WPDMA_INT_CTRL_POLL_PRD_SHFT                  8
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_PRD_ADDR            WED_WPDMA_INT_CTRL_ADDR
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_PRD_MASK            0x000000FF                // MSK_ON_DLY_PRD[7..0]
#define WED_WPDMA_INT_CTRL_MSK_ON_DLY_PRD_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_INT_MSK (0x15010000 + 0x524)---

    VALUE[31..0]                 - (RW) The value for write-back to WiFi card WPDMA interrupt mask registers, after WED Interrupt Agent cleared WiFi card WPDMA interrupt status register

 =====================================================================================*/
#define WED_WPDMA_INT_MSK_VALUE_ADDR                      WED_WPDMA_INT_MSK_ADDR
#define WED_WPDMA_INT_MSK_VALUE_MASK                      0xFFFFFFFF                // VALUE[31..0]
#define WED_WPDMA_INT_MSK_VALUE_SHFT                      0

/* =====================================================================================

  ---WED_WPDMA_INT_CLR (0x15010000 + 0x528)---

    INT_CLR[31..0]               - (RW) Specify the WPDMA interrupt status bits which will be cleared by WED hardware, after WED reads out the interrupt status and the corresponding event bit is asserting.
                                     Each bit's definition is same as WiFi card's interrupt status register.

 =====================================================================================*/
#define WED_WPDMA_INT_CLR_INT_CLR_ADDR                    WED_WPDMA_INT_CLR_ADDR
#define WED_WPDMA_INT_CLR_INT_CLR_MASK                    0xFFFFFFFF                // INT_CLR[31..0]
#define WED_WPDMA_INT_CLR_INT_CLR_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_INT_MON (0x15010000 + 0x52C)---

    TX_DONE_INT0[0]              - (RO) TX Queue#0 packet transmit interrupt
    TX_DONE_INT1[1]              - (RO) TX Queue#1 packet transmit interrupt
    RESERVED2[6..2]              - (RO) Reserved bits
    RRO3_1_RX_DONE_INT[7]        - (RO) RRO3.1 RX Queue#0 packet receive interrupt
    RESERVED8[9..8]              - (RO) Reserved bits
    RRO_RX_INT0[10]              - (RO) RRO RX Queue#0 interrupt
    RRO_RX_INT1[11]              - (RO) RRO RX Queue#1 interrupt
    MSDU_PG_INT0[12]             - (RO) MSDU PG Queue#0 interrupt
    MSDU_PG_INT1[13]             - (RO) MSDU PG Queue#1 interrupt
    MSDU_PG_INT2[14]             - (RO) MSDU PG Queue#2 interrupt
    RESERVED15[15]               - (RO) Reserved bits
    TX_FREE_DONE_INT0[16]        - (RO) TX Free Queue#0 packet transmit interrupt
    TX_FREE_DONE_INT1[17]        - (RO) TX Free Queue#1 packet transmit interrupt
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT1_ADDR          WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT1_MASK          0x00020000                // TX_FREE_DONE_INT1[17]
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT1_SHFT          17
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT0_ADDR          WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT0_MASK          0x00010000                // TX_FREE_DONE_INT0[16]
#define WED_WPDMA_INT_MON_TX_FREE_DONE_INT0_SHFT          16
#define WED_WPDMA_INT_MON_MSDU_PG_INT2_ADDR               WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_MSDU_PG_INT2_MASK               0x00004000                // MSDU_PG_INT2[14]
#define WED_WPDMA_INT_MON_MSDU_PG_INT2_SHFT               14
#define WED_WPDMA_INT_MON_MSDU_PG_INT1_ADDR               WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_MSDU_PG_INT1_MASK               0x00002000                // MSDU_PG_INT1[13]
#define WED_WPDMA_INT_MON_MSDU_PG_INT1_SHFT               13
#define WED_WPDMA_INT_MON_MSDU_PG_INT0_ADDR               WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_MSDU_PG_INT0_MASK               0x00001000                // MSDU_PG_INT0[12]
#define WED_WPDMA_INT_MON_MSDU_PG_INT0_SHFT               12
#define WED_WPDMA_INT_MON_RRO_RX_INT1_ADDR                WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_RRO_RX_INT1_MASK                0x00000800                // RRO_RX_INT1[11]
#define WED_WPDMA_INT_MON_RRO_RX_INT1_SHFT                11
#define WED_WPDMA_INT_MON_RRO_RX_INT0_ADDR                WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_RRO_RX_INT0_MASK                0x00000400                // RRO_RX_INT0[10]
#define WED_WPDMA_INT_MON_RRO_RX_INT0_SHFT                10
#define WED_WPDMA_INT_MON_RRO3_1_RX_DONE_INT_ADDR         WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_RRO3_1_RX_DONE_INT_MASK         0x00000080                // RRO3_1_RX_DONE_INT[7]
#define WED_WPDMA_INT_MON_RRO3_1_RX_DONE_INT_SHFT         7
#define WED_WPDMA_INT_MON_TX_DONE_INT1_ADDR               WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_TX_DONE_INT1_MASK               0x00000002                // TX_DONE_INT1[1]
#define WED_WPDMA_INT_MON_TX_DONE_INT1_SHFT               1
#define WED_WPDMA_INT_MON_TX_DONE_INT0_ADDR               WED_WPDMA_INT_MON_ADDR
#define WED_WPDMA_INT_MON_TX_DONE_INT0_MASK               0x00000001                // TX_DONE_INT0[0]
#define WED_WPDMA_INT_MON_TX_DONE_INT0_SHFT               0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL_TX (0x15010000 + 0x530)---

    TX_DONE_EN0[0]               - (RW) TX Queue#0 packet transmit interrupt enable
    TX_DONE_CLR0[1]              - (RW) Clear TX Queue#0 packet transmit interrupt automatically after it has been received by agent.
    TX_DONE_TRIG0[6..2]          - (RW) TX Queue#0 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED7[7]                 - (RO) Reserved bits
    TX_DONE_EN1[8]               - (RW) TX Queue#1 packet transmit interrupt enable
    TX_DONE_CLR1[9]              - (RW) Clear TX Queue#1 packet transmit interrupt automatically after it has been received by agent.
    TX_DONE_TRIG1[14..10]        - (RW) TX Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED15[23..15]           - (RO) Reserved bits
    DLY_MERGE_EN[24]             - (RW) Enable the merged Tx delay interrupt for MT7615
    RESERVED25[25]               - (RO) Reserved bits
    DLY_MERGE_BSEL[30..26]       - (RW) TX packet transmit merged delayed interrupt bit selection
    DLY_MERGE_UNP_IMM[31]        - (RW) TX packet transmit merged delayed interrupt assert immediately for un-processed packet event.

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_UNP_IMM_ADDR      WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_UNP_IMM_MASK      0x80000000                // DLY_MERGE_UNP_IMM[31]
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_UNP_IMM_SHFT      31
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_BSEL_ADDR         WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_BSEL_MASK         0x7C000000                // DLY_MERGE_BSEL[30..26]
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_BSEL_SHFT         26
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_EN_ADDR           WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_EN_MASK           0x01000000                // DLY_MERGE_EN[24]
#define WED_WPDMA_INT_CTRL_TX_DLY_MERGE_EN_SHFT           24
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG1_ADDR          WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG1_MASK          0x00007C00                // TX_DONE_TRIG1[14..10]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG1_SHFT          10
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR1_ADDR           WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR1_MASK           0x00000200                // TX_DONE_CLR1[9]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR1_SHFT           9
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN1_ADDR            WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN1_MASK            0x00000100                // TX_DONE_EN1[8]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN1_SHFT            8
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG0_ADDR          WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG0_MASK          0x0000007C                // TX_DONE_TRIG0[6..2]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_TRIG0_SHFT          2
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR0_ADDR           WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR0_MASK           0x00000002                // TX_DONE_CLR0[1]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_CLR0_SHFT           1
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN0_ADDR            WED_WPDMA_INT_CTRL_TX_ADDR
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN0_MASK            0x00000001                // TX_DONE_EN0[0]
#define WED_WPDMA_INT_CTRL_TX_TX_DONE_EN0_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL_RX (0x15010000 + 0x534)---

    RX_DONE_EN0[0]               - (RW) RX Queue#0 packet transmit interrupt enable
    RX_DONE_CLR0[1]              - (RW) Clear RX Queue#0 packet transmit interrupt automatically after it has been received by agent.
    RX_DONE_TRIG0[6..2]          - (RW) RX Queue#0 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED7[7]                 - (RO) Reserved bits
    RX_DONE_EN1[8]               - (RW) RX Queue#1 packet transmit interrupt enable
    RX_DONE_CLR1[9]              - (RW) Clear RX Queue#1 packet transmit interrupt automatically after it has been received by agent.
    RX_DONE_TRIG1[14..10]        - (RW) RX Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RX_RRO3_1_DONE_EN[15]        - (RW) RRO3.1 RX Queue#1 packet transmit interrupt enable
    RX_RRO3_1_DONE_CLR[16]       - (RW) Clear RRO3.1 RX Queue#1 packet transmit interrupt automatically after it has been received by agent.
    RX_RRO3_1_DONE_TRIG[21..17]  - (RW) RRO3.1 RX Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED22[23..22]           - (RO) Reserved bits
    DLY_MERGE_EN[24]             - (RW) Enable the merged Rx + TX_free delay interrupt for MT7615
    RESERVED25[25]               - (RO) Reserved bits
    DLY_MERGE_BSEL[30..26]       - (RW) RX + TX_free packet transmit merged delayed interrupt bit selection
    DLY_MERGE_UNP_IMM[31]        - (RW) RX + TX_free packet transmit merged delayed interrupt assert immediately for un-processed packet event.

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_UNP_IMM_ADDR      WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_UNP_IMM_MASK      0x80000000                // DLY_MERGE_UNP_IMM[31]
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_UNP_IMM_SHFT      31
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_BSEL_ADDR         WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_BSEL_MASK         0x7C000000                // DLY_MERGE_BSEL[30..26]
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_BSEL_SHFT         26
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_EN_ADDR           WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_EN_MASK           0x01000000                // DLY_MERGE_EN[24]
#define WED_WPDMA_INT_CTRL_RX_DLY_MERGE_EN_SHFT           24
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_TRIG_ADDR    WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_TRIG_MASK    0x003E0000                // RX_RRO3_1_DONE_TRIG[21..17]
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_TRIG_SHFT    17
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_CLR_ADDR     WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_CLR_MASK     0x00010000                // RX_RRO3_1_DONE_CLR[16]
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_CLR_SHFT     16
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_EN_ADDR      WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_EN_MASK      0x00008000                // RX_RRO3_1_DONE_EN[15]
#define WED_WPDMA_INT_CTRL_RX_RX_RRO3_1_DONE_EN_SHFT      15
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG1_ADDR          WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG1_MASK          0x00007C00                // RX_DONE_TRIG1[14..10]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG1_SHFT          10
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR1_ADDR           WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR1_MASK           0x00000200                // RX_DONE_CLR1[9]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR1_SHFT           9
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN1_ADDR            WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN1_MASK            0x00000100                // RX_DONE_EN1[8]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN1_SHFT            8
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG0_ADDR          WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG0_MASK          0x0000007C                // RX_DONE_TRIG0[6..2]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_TRIG0_SHFT          2
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR0_ADDR           WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR0_MASK           0x00000002                // RX_DONE_CLR0[1]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_CLR0_SHFT           1
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN0_ADDR            WED_WPDMA_INT_CTRL_RX_ADDR
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN0_MASK            0x00000001                // RX_DONE_EN0[0]
#define WED_WPDMA_INT_CTRL_RX_RX_DONE_EN0_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL_TX_FREE (0x15010000 + 0x538)---

    TX_FREE_DONE_EN0[0]          - (RW) TX Free Queue#0 packet transmit interrupt enable
    TX_FREE_DONE_CLR0[1]         - (RW) Clear TX Free Queue#0 packet transmit interrupt automatically after it has been received by agent.
    TX_FREE_DONE_TRIG0[6..2]     - (RW) TX Free Queue#0 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED7[7]                 - (RO) Reserved bits
    TX_FREE_DONE_EN1[8]          - (RW) TX Free Queue#1 packet transmit interrupt enable
    TX_FREE_DONE_CLR1[9]         - (RW) Clear TX Free Queue#1 packet transmit interrupt automatically after it has been received by agent.
    TX_FREE_DONE_TRIG1[14..10]   - (RW) TX Free Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED15[31..15]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG1_ADDR WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG1_MASK 0x00007C00                // TX_FREE_DONE_TRIG1[14..10]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG1_SHFT 10
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR1_ADDR WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR1_MASK 0x00000200                // TX_FREE_DONE_CLR1[9]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR1_SHFT 9
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN1_ADDR  WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN1_MASK  0x00000100                // TX_FREE_DONE_EN1[8]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN1_SHFT  8
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG0_ADDR WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG0_MASK 0x0000007C                // TX_FREE_DONE_TRIG0[6..2]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_TRIG0_SHFT 2
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR0_ADDR WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR0_MASK 0x00000002                // TX_FREE_DONE_CLR0[1]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_CLR0_SHFT 1
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN0_ADDR  WED_WPDMA_INT_CTRL_TX_FREE_ADDR
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN0_MASK  0x00000001                // TX_FREE_DONE_EN0[0]
#define WED_WPDMA_INT_CTRL_TX_FREE_TX_FREE_DONE_EN0_SHFT  0

/* =====================================================================================

  ---WED_WPDMA_SPR (0x15010000 + 0x53C)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_WPDMA_SPR_RSV_REG_ADDR                        WED_WPDMA_SPR_ADDR
#define WED_WPDMA_SPR_RSV_REG_MASK                        0xFFFFFFFF                // RSV_REG[31..0]
#define WED_WPDMA_SPR_RSV_REG_SHFT                        0

/* =====================================================================================

  ---WED_PCIE_MSIS_TRIG (0x15010000 + 0x540)---

    CFG[31..0]                   - (RW) Specify the PCIe RC MSI function bits which will trigger WED hardware to handle the interrupt event from PCIe WiFi card

 =====================================================================================*/
#define WED_PCIE_MSIS_TRIG_CFG_ADDR                       WED_PCIE_MSIS_TRIG_ADDR
#define WED_PCIE_MSIS_TRIG_CFG_MASK                       0xFFFFFFFF                // CFG[31..0]
#define WED_PCIE_MSIS_TRIG_CFG_SHFT                       0

/* =====================================================================================

  ---WED_PCIE_MSIS_REC (0x15010000 + 0x544)---

    REG[31..0]                   - (RO) Record the last WED read original PCIE RC MSI function value from PCIe Controller

 =====================================================================================*/
#define WED_PCIE_MSIS_REC_REG_ADDR                        WED_PCIE_MSIS_REC_ADDR
#define WED_PCIE_MSIS_REC_REG_MASK                        0xFFFFFFFF                // REG[31..0]
#define WED_PCIE_MSIS_REC_REG_SHFT                        0

/* =====================================================================================

  ---WED_PCIE_MSIS_CLR (0x15010000 + 0x548)---

    CFG[31..0]                   - (RW) Specify the PCIe RC MSI function bits which will be cleared by WED hardware, after WED reads out the MSI function and the corresponding bit is asserting.

 =====================================================================================*/
#define WED_PCIE_MSIS_CLR_CFG_ADDR                        WED_PCIE_MSIS_CLR_ADDR
#define WED_PCIE_MSIS_CLR_CFG_MASK                        0xFFFFFFFF                // CFG[31..0]
#define WED_PCIE_MSIS_CLR_CFG_SHFT                        0

/* =====================================================================================

  ---WED_WPDMA_TX_DRV_MAGIC_COUNT (0x15010000 + 0x54C)---

    en[0]                        - (RW) magic count function enable for loopback,  , just work when loopback function enable, read ddone function reservation, but ddone check always pass
    mon[4..1]                    - (RO) magic count monitor
    RESERVED5[31..5]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_mon_ADDR             WED_WPDMA_TX_DRV_MAGIC_COUNT_ADDR
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_mon_MASK             0x0000001E                // mon[4..1]
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_mon_SHFT             1
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_en_ADDR              WED_WPDMA_TX_DRV_MAGIC_COUNT_ADDR
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_en_MASK              0x00000001                // en[0]
#define WED_WPDMA_TX_DRV_MAGIC_COUNT_en_SHFT              0

/* =====================================================================================

  ---WED_PCIE_INTS_CLR (0x15010000 + 0x550)---

    CFG[31..0]                   - (RW) Specify the PCIe RC Interrupt Status bits which will be cleared by WED hardware, after WED reads out the PCIe RC Interrupt Status and the corresponding bit is asserting.

 =====================================================================================*/
#define WED_PCIE_INTS_CLR_CFG_ADDR                        WED_PCIE_INTS_CLR_ADDR
#define WED_PCIE_INTS_CLR_CFG_MASK                        0xFFFFFFFF                // CFG[31..0]
#define WED_PCIE_INTS_CLR_CFG_SHFT                        0

/* =====================================================================================

  ---WED_PCIE_EP_INTS_CLR (0x15010000 + 0x554)---

    CFG[31..0]                   - (RW) Specify the write value to clearPCIe EP Interrupt Status register

 =====================================================================================*/
#define WED_PCIE_EP_INTS_CLR_CFG_ADDR                     WED_PCIE_EP_INTS_CLR_ADDR
#define WED_PCIE_EP_INTS_CLR_CFG_MASK                     0xFFFFFFFF                // CFG[31..0]
#define WED_PCIE_EP_INTS_CLR_CFG_SHFT                     0

/* =====================================================================================

  ---WED_PCIE_CFG_ADDR_H (0x15010000 + 0x55C)---

    RC_PTR[7..0]                 - (RW) The PCIe RC configuration register address high bits, belong to WED_PCIE_CFG_ADDR_INTS, WED_PCIE_CFG_ADDR_INTM, WED_PCIE_CFG_ADDR_MSIS
                                     e.g. The PCIe RC interrupt status register Address = {WED_PCIE_CFG_ADDR_H.RC_PTR, WED_PCIE_CFG_ADDR_INTS.PTR}
    RESERVED8[15..8]             - (RO) Reserved bits
    EP_PTR[23..16]               - (RW) The PCIe EO configuration register address high bits
                                     e.g. The PCIe EP interrupt status register Address = {WED_PCIE_CFG_ADDR_H.EP_PTR, WED_PCIE_CFG_ADDR_INTS_EP.PTR}
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PCIE_CFG_ADDR_H_EP_PTR_ADDR                   WED_PCIE_CFG_ADDR_H_ADDR
#define WED_PCIE_CFG_ADDR_H_EP_PTR_MASK                   0x00FF0000                // EP_PTR[23..16]
#define WED_PCIE_CFG_ADDR_H_EP_PTR_SHFT                   16
#define WED_PCIE_CFG_ADDR_H_RC_PTR_ADDR                   WED_PCIE_CFG_ADDR_H_ADDR
#define WED_PCIE_CFG_ADDR_H_RC_PTR_MASK                   0x000000FF                // RC_PTR[7..0]
#define WED_PCIE_CFG_ADDR_H_RC_PTR_SHFT                   0

/* =====================================================================================

  ---WED_PCIE_CFG_ADDR_INTS (0x15010000 + 0x560)---

    PTR[31..0]                   - (RW) The PCIE RC interrupt status registers address

 =====================================================================================*/
#define WED_PCIE_CFG_ADDR_INTS_PTR_ADDR                   WED_PCIE_CFG_ADDR_INTS_ADDR
#define WED_PCIE_CFG_ADDR_INTS_PTR_MASK                   0xFFFFFFFF                // PTR[31..0]
#define WED_PCIE_CFG_ADDR_INTS_PTR_SHFT                   0

/* =====================================================================================

  ---WED_PCIE_CFG_ADDR_INTM (0x15010000 + 0x564)---

    PTR[31..0]                   - (RW) The PCIE RC interrupt mask registers address

 =====================================================================================*/
#define WED_PCIE_CFG_ADDR_INTM_PTR_ADDR                   WED_PCIE_CFG_ADDR_INTM_ADDR
#define WED_PCIE_CFG_ADDR_INTM_PTR_MASK                   0xFFFFFFFF                // PTR[31..0]
#define WED_PCIE_CFG_ADDR_INTM_PTR_SHFT                   0

/* =====================================================================================

  ---WED_PCIE_CFG_ADDR_MSIS (0x15010000 + 0x568)---

    PTR[31..0]                   - (RW) The PCIE RC interrupt MSI function registers address

 =====================================================================================*/
#define WED_PCIE_CFG_ADDR_MSIS_PTR_ADDR                   WED_PCIE_CFG_ADDR_MSIS_ADDR
#define WED_PCIE_CFG_ADDR_MSIS_PTR_MASK                   0xFFFFFFFF                // PTR[31..0]
#define WED_PCIE_CFG_ADDR_MSIS_PTR_SHFT                   0

/* =====================================================================================

  ---WED_PCIE_CFG_ADDR_INTS_EP (0x15010000 + 0x56C)---

    PTR[31..0]                   - (RW) The PCIE EP interrupt status registers address

 =====================================================================================*/
#define WED_PCIE_CFG_ADDR_INTS_EP_PTR_ADDR                WED_PCIE_CFG_ADDR_INTS_EP_ADDR
#define WED_PCIE_CFG_ADDR_INTS_EP_PTR_MASK                0xFFFFFFFF                // PTR[31..0]
#define WED_PCIE_CFG_ADDR_INTS_EP_PTR_SHFT                0

/* =====================================================================================

  ---WED_PCIE_INTS_TRIG (0x15010000 + 0x570)---

    INTS_TRIG[31..0]             - (RW) Specify the PCIe interrupt status bits which will trigger WED hardware to handle the interrupt event from PCIe WiFi card

 =====================================================================================*/
#define WED_PCIE_INTS_TRIG_INTS_TRIG_ADDR                 WED_PCIE_INTS_TRIG_ADDR
#define WED_PCIE_INTS_TRIG_INTS_TRIG_MASK                 0xFFFFFFFF                // INTS_TRIG[31..0]
#define WED_PCIE_INTS_TRIG_INTS_TRIG_SHFT                 0

/* =====================================================================================

  ---WED_PCIE_INTS_REC (0x15010000 + 0x574)---

    INTS[31..0]                  - (RO) Record the last WED read original PCIE_INT_STA value from PCIe Controller

 =====================================================================================*/
#define WED_PCIE_INTS_REC_INTS_ADDR                       WED_PCIE_INTS_REC_ADDR
#define WED_PCIE_INTS_REC_INTS_MASK                       0xFFFFFFFF                // INTS[31..0]
#define WED_PCIE_INTS_REC_INTS_SHFT                       0

/* =====================================================================================

  ---WED_PCIE_INTM_REC (0x15010000 + 0x578)---

    INTM[31..0]                  - (RO) Record the last WED read original PCIE_INT_MASK value from PCIe Controller

 =====================================================================================*/
#define WED_PCIE_INTM_REC_INTM_ADDR                       WED_PCIE_INTM_REC_ADDR
#define WED_PCIE_INTM_REC_INTM_MASK                       0xFFFFFFFF                // INTM[31..0]
#define WED_PCIE_INTM_REC_INTM_SHFT                       0

/* =====================================================================================

  ---WED_PCIE_INT_CTRL (0x15010000 + 0x57C)---

    POLL_PRD[7..0]               - (RW) PCIe interrupt status polling period
                                     polling period = poll_prd * 16 * AXI_clock_period
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=0)
    RESERVED8[11..8]             - (RO) Reserved bits
    POLL_MODE[13..12]            - (RW) PCIe interrupt status polling mode
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=0)
    RESERVED14[15..14]           - (RO) Reserved bits
    SRC_SEL[17..16]              - (RW) PCIe interrupt signal source selection
                                     (Only valid when WED_WPDMA_INT_CTRL.SIG_SRC=0 and system comprises PCIe, amount of WED/PCIe depends on system configuration)
    RESERVED18[19..18]           - (RO) Reserved bits
    MSK_EN_POLA[20]              - (RW) PCIe interrupt mask polarity to enable interrupt
    RESERVED21[23..21]           - (RO) Reserved bits
    MSI_EN[24]                   - (RW) PCIe interrupt MSI processing control
    IRQ_MSI_SEL[25]              - (RW) PCIe IRQ signal selection
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_PCIE_INT_CTRL_IRQ_MSI_SEL_ADDR                WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_IRQ_MSI_SEL_MASK                0x02000000                // IRQ_MSI_SEL[25]
#define WED_PCIE_INT_CTRL_IRQ_MSI_SEL_SHFT                25
#define WED_PCIE_INT_CTRL_MSI_EN_ADDR                     WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_MSI_EN_MASK                     0x01000000                // MSI_EN[24]
#define WED_PCIE_INT_CTRL_MSI_EN_SHFT                     24
#define WED_PCIE_INT_CTRL_MSK_EN_POLA_ADDR                WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_MSK_EN_POLA_MASK                0x00100000                // MSK_EN_POLA[20]
#define WED_PCIE_INT_CTRL_MSK_EN_POLA_SHFT                20
#define WED_PCIE_INT_CTRL_SRC_SEL_ADDR                    WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_SRC_SEL_MASK                    0x00030000                // SRC_SEL[17..16]
#define WED_PCIE_INT_CTRL_SRC_SEL_SHFT                    16
#define WED_PCIE_INT_CTRL_POLL_MODE_ADDR                  WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_POLL_MODE_MASK                  0x00003000                // POLL_MODE[13..12]
#define WED_PCIE_INT_CTRL_POLL_MODE_SHFT                  12
#define WED_PCIE_INT_CTRL_POLL_PRD_ADDR                   WED_PCIE_INT_CTRL_ADDR
#define WED_PCIE_INT_CTRL_POLL_PRD_MASK                   0x000000FF                // POLL_PRD[7..0]
#define WED_PCIE_INT_CTRL_POLL_PRD_SHFT                   0

/* =====================================================================================

  ---WED_WPDMA_CFG_ADDR_INTS (0x15010000 + 0x580)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA interrupt status register address

 =====================================================================================*/
#define WED_WPDMA_CFG_ADDR_INTS_PTR_ADDR                  WED_WPDMA_CFG_ADDR_INTS_ADDR
#define WED_WPDMA_CFG_ADDR_INTS_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_ADDR_INTS_PTR_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_CFG_ADDR_INTM (0x15010000 + 0x584)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA interrupt mask register address

 =====================================================================================*/
#define WED_WPDMA_CFG_ADDR_INTM_PTR_ADDR                  WED_WPDMA_CFG_ADDR_INTM_ADDR
#define WED_WPDMA_CFG_ADDR_INTM_PTR_MASK                  0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_ADDR_INTM_PTR_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX0 (0x15010000 + 0x588)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx data ring#0 register starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_PTR_ADDR              WED_WPDMA_CFG_CIDX_ADDR_TX0_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_PTR_MASK              0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_PTR_SHFT              0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE (0x15010000 + 0x58C)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx free notify ring#0 register cpu index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_PTR_ADDR         WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_PTR_MASK         0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_PTR_SHFT         0

/* =====================================================================================

  ---WED_WPDMA_CFG_ADDR_INT_H (0x15010000 + 0x590)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA configuration register address[39:32]
                                     e.g. The WiFi card WPDMA interrupt status register address[39:0]={WED_WPDMA_CFG_ADDR_H.PTR,WED_WPDMA_CFG_ADDR_INTS.PTR}
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_ADDR_INT_H_PTR_ADDR                 WED_WPDMA_CFG_ADDR_INT_H_ADDR
#define WED_WPDMA_CFG_ADDR_INT_H_PTR_MASK                 0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_ADDR_INT_H_PTR_SHFT                 0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE (0x15010000 + 0x594)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx free notify ring#0 register dma index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_PTR_ADDR         WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_PTR_MASK         0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_PTR_SHFT         0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX0_H (0x15010000 + 0x598)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx data ring configuration register address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_H_PTR_ADDR            WED_WPDMA_CFG_CIDX_ADDR_TX0_H_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_H_PTR_MASK            0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_H_PTR_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H (0x15010000 + 0x59c)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx Free Notify ring configuration cpu index register address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H_PTR_ADDR       WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H_PTR_MASK       0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX0_FREE_H_PTR_SHFT       0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX1 (0x15010000 + 0x5A0)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx data ring#0 register cpu index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_PTR_ADDR              WED_WPDMA_CFG_CIDX_ADDR_TX1_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_PTR_MASK              0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_PTR_SHFT              0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE (0x15010000 + 0x5A4)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx free notify ring#0 register cpu index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_PTR_ADDR         WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_PTR_MASK         0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_PTR_SHFT         0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX1_H (0x15010000 + 0x5A8)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx data ring configuration register cpu index address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_H_PTR_ADDR            WED_WPDMA_CFG_CIDX_ADDR_TX1_H_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_H_PTR_MASK            0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_H_PTR_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H (0x15010000 + 0x5AC)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx Free Notify ring configuration register cpu index address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H_PTR_ADDR       WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H_ADDR
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H_PTR_MASK       0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_CIDX_ADDR_TX1_FREE_H_PTR_SHFT       0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX0 (0x15010000 + 0x5B0)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx dma index data ring#0 register starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_PTR_ADDR              WED_WPDMA_CFG_DIDX_ADDR_TX0_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_PTR_MASK              0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_PTR_SHFT              0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX0_H (0x15010000 + 0x5B4)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx dma index data ring configuration register address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_H_PTR_ADDR            WED_WPDMA_CFG_DIDX_ADDR_TX0_H_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_H_PTR_MASK            0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_H_PTR_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H (0x15010000 + 0x5B8)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx Free Notify ring configuration dma index register address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H_PTR_ADDR       WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H_PTR_MASK       0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX0_FREE_H_PTR_SHFT       0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE (0x15010000 + 0x5BC)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx free notify ring#0 register dma index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_PTR_ADDR         WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_PTR_MASK         0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_PTR_SHFT         0

/* =====================================================================================

  ---WED_WPDMA_TX0_MIB (0x15010000 + 0x5C0)---

    PKT_CNT[31..0]               - (RC) WED en-queued to WiFi card WPDMA Tx ring0 packet counter

 =====================================================================================*/
#define WED_WPDMA_TX0_MIB_PKT_CNT_ADDR                    WED_WPDMA_TX0_MIB_ADDR
#define WED_WPDMA_TX0_MIB_PKT_CNT_MASK                    0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_TX0_MIB_PKT_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_TX1_MIB (0x15010000 + 0x5C4)---

    PKT_CNT[31..0]               - (RC) WED en-queued to WiFi card WPDMA Tx ring1 packet counter

 =====================================================================================*/
#define WED_WPDMA_TX1_MIB_PKT_CNT_ADDR                    WED_WPDMA_TX1_MIB_ADDR
#define WED_WPDMA_TX1_MIB_PKT_CNT_MASK                    0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_TX1_MIB_PKT_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H (0x15010000 + 0x5C8)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx Free Notify ring configuration dma index register cpu index address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H_PTR_ADDR       WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H_PTR_MASK       0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_FREE_H_PTR_SHFT       0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX1 (0x15010000 + 0x5CC)---

    PTR[31..0]                   - (RW) The WiFi card WPDMA Tx data ring#0 register dma index starting address

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_PTR_ADDR              WED_WPDMA_CFG_DIDX_ADDR_TX1_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_PTR_MASK              0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_PTR_SHFT              0

/* =====================================================================================

  ---WED_WPDMA_TX_COHERENT_MIB (0x15010000 + 0x5D0)---

    CNT[31..0]                   - (RC) WED got WiFi card WPDMA Tx ring0+1 coherent event counter.
                                     WED Reads DMAD after DTX_IDX but gets DDONE=0.

 =====================================================================================*/
#define WED_WPDMA_TX_COHERENT_MIB_CNT_ADDR                WED_WPDMA_TX_COHERENT_MIB_ADDR
#define WED_WPDMA_TX_COHERENT_MIB_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define WED_WPDMA_TX_COHERENT_MIB_CNT_SHFT                0

/* =====================================================================================

  ---WED_WPDMA_CFG_DIDX_ADDR_TX1_H (0x15010000 + 0x5D4)---

    PTR[7..0]                    - (RW) The WiFi card WPDMA Tx data ring configuration register dma index address[39:32]
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_H_PTR_ADDR            WED_WPDMA_CFG_DIDX_ADDR_TX1_H_ADDR
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_H_PTR_MASK            0x000000FF                // PTR[7..0]
#define WED_WPDMA_CFG_DIDX_ADDR_TX1_H_PTR_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_RX0_MIB (0x15010000 + 0x5e0)---

    PKT_CNT[31..0]               - (RC) WED received from WiFi card WPDMA Rx ring0 packet counter

 =====================================================================================*/
#define WED_WPDMA_RX0_MIB_PKT_CNT_ADDR                    WED_WPDMA_RX0_MIB_ADDR
#define WED_WPDMA_RX0_MIB_PKT_CNT_MASK                    0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_RX0_MIB_PKT_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_RX1_MIB (0x15010000 + 0x5e4)---

    PKT_CNT[31..0]               - (RC) WED received from WiFi card WPDMA Rx ring1 packet counter

 =====================================================================================*/
#define WED_WPDMA_RX1_MIB_PKT_CNT_ADDR                    WED_WPDMA_RX1_MIB_ADDR
#define WED_WPDMA_RX1_MIB_PKT_CNT_MASK                    0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_RX1_MIB_PKT_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_RX_COHERENT_MIB (0x15010000 + 0x5F0)---

    CNT[31..0]                   - (RC) WED received from WiFi card WPDMA Rx ring1 coherent event counter.
                                     WED Reads DMAD after DTX_IDX but gets DDONE=0.

 =====================================================================================*/
#define WED_WPDMA_RX_COHERENT_MIB_CNT_ADDR                WED_WPDMA_RX_COHERENT_MIB_ADDR
#define WED_WPDMA_RX_COHERENT_MIB_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define WED_WPDMA_RX_COHERENT_MIB_CNT_SHFT                0

/* =====================================================================================

  ---WED_WPDMA_RX_EXTC_FREE_TKID_MIB (0x15010000 + 0x5F8)---

    CNT[31..0]                   - (RC) WED received from WiFi card WPDMA Rx ring1 Tx Free Notify, and extracted the Token_ID count which belongs to ethernet.
                                     WED_TX_BM_TKID.start_id <=  Token_ID <= WED_TX_BM_TKID.end_id

 =====================================================================================*/
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_CNT_ADDR          WED_WPDMA_RX_EXTC_FREE_TKID_MIB_ADDR
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_CNT_MASK          0xFFFFFFFF                // CNT[31..0]
#define WED_WPDMA_RX_EXTC_FREE_TKID_MIB_CNT_SHFT          0

/* =====================================================================================

  ---WED_WPDMA_TX0_CTRL0 (0x15010000 + 0x600)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TX_Ring0 (4-DWORD aligned address)

 =====================================================================================*/
#define WED_WPDMA_TX0_CTRL0_BASE_PTR_ADDR                 WED_WPDMA_TX0_CTRL0_ADDR
#define WED_WPDMA_TX0_CTRL0_BASE_PTR_MASK                 0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_WPDMA_TX0_CTRL0_BASE_PTR_SHFT                 0

/* =====================================================================================

  ---WED_WPDMA_TX0_CTRL1 (0x15010000 + 0x604)---

    MAX_CNT[11..0]               - (RW) The maximum number of DMAD count in TXD_Ring0.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX0_CTRL1_BASE_PTR_H_ADDR               WED_WPDMA_TX0_CTRL1_ADDR
#define WED_WPDMA_TX0_CTRL1_BASE_PTR_H_MASK               0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WPDMA_TX0_CTRL1_BASE_PTR_H_SHFT               16
#define WED_WPDMA_TX0_CTRL1_MAX_CNT_ADDR                  WED_WPDMA_TX0_CTRL1_ADDR
#define WED_WPDMA_TX0_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define WED_WPDMA_TX0_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_TX0_CTRL2 (0x15010000 + 0x608)---

    CPU_IDX[11..0]               - (RO) This CPU index is  already updated to WFDMA.
    RESERVED12[15..12]           - (RO) Reserved bits
    CPU_IDX_INT[27..16]          - (RO) Point to the next DMAD hardware driver agent wants to use, this index is controlled by WED hardware internally.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_INT_ADDR              WED_WPDMA_TX0_CTRL2_ADDR
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_INT_MASK              0x0FFF0000                // CPU_IDX_INT[27..16]
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_INT_SHFT              16
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_ADDR                  WED_WPDMA_TX0_CTRL2_ADDR
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define WED_WPDMA_TX0_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_TX0_CTRL3 (0x15010000 + 0x60c)---

    DMA_IDX_MIRO[11..0]          - (RO) Point to the next DMAD which DMA wants to use, the mirror of WiFi WFDMA.
    RESERVED12[15..12]           - (RO) Reserved bits
    RLS_IDX[27..16]              - (RO) Point to the next DMAD hardware driver agent wants to release, this index is controlled by WED hardware
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX0_CTRL3_RLS_IDX_ADDR                  WED_WPDMA_TX0_CTRL3_ADDR
#define WED_WPDMA_TX0_CTRL3_RLS_IDX_MASK                  0x0FFF0000                // RLS_IDX[27..16]
#define WED_WPDMA_TX0_CTRL3_RLS_IDX_SHFT                  16
#define WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO_ADDR             WED_WPDMA_TX0_CTRL3_ADDR
#define WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO_MASK             0x00000FFF                // DMA_IDX_MIRO[11..0]
#define WED_WPDMA_TX0_CTRL3_DMA_IDX_MIRO_SHFT             0

/* =====================================================================================

  ---WED_WPDMA_TX1_CTRL0 (0x15010000 + 0x610)---

    BASE_PTR[31..0]              - (RW) Point to the base address of TX_Ring0 (4-DWORD aligned address)

 =====================================================================================*/
#define WED_WPDMA_TX1_CTRL0_BASE_PTR_ADDR                 WED_WPDMA_TX1_CTRL0_ADDR
#define WED_WPDMA_TX1_CTRL0_BASE_PTR_MASK                 0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_WPDMA_TX1_CTRL0_BASE_PTR_SHFT                 0

/* =====================================================================================

  ---WED_WPDMA_TX1_CTRL1 (0x15010000 + 0x614)---

    MAX_CNT[11..0]               - (RW) The maximum number of DMAD count in TXD_Ring0.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX1_CTRL1_BASE_PTR_H_ADDR               WED_WPDMA_TX1_CTRL1_ADDR
#define WED_WPDMA_TX1_CTRL1_BASE_PTR_H_MASK               0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WPDMA_TX1_CTRL1_BASE_PTR_H_SHFT               16
#define WED_WPDMA_TX1_CTRL1_MAX_CNT_ADDR                  WED_WPDMA_TX1_CTRL1_ADDR
#define WED_WPDMA_TX1_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define WED_WPDMA_TX1_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_TX1_CTRL2 (0x15010000 + 0x618)---

    CPU_IDX[11..0]               - (RO) This CPU index is  already updated to WFDMA.
    RESERVED12[15..12]           - (RO) Reserved bits
    CPU_IDX_INT[27..16]          - (RO) Point to the next DMAD hardware driver agent wants to use, this index is controlled by WED hardware internally.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_INT_ADDR              WED_WPDMA_TX1_CTRL2_ADDR
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_INT_MASK              0x0FFF0000                // CPU_IDX_INT[27..16]
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_INT_SHFT              16
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_ADDR                  WED_WPDMA_TX1_CTRL2_ADDR
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define WED_WPDMA_TX1_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_TX1_CTRL3 (0x15010000 + 0x61c)---

    DMA_IDX_MIRO[11..0]          - (RO) Point to the next DMAD which DMA wants to use, the mirror of WiFi WFDMA.
    RESERVED12[15..12]           - (RO) Reserved bits
    RLS_IDX[27..16]              - (RO) Point to the next DMAD hardware driver agent wants to release, this index is controlled by WED hardware
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_TX1_CTRL3_RLS_IDX_ADDR                  WED_WPDMA_TX1_CTRL3_ADDR
#define WED_WPDMA_TX1_CTRL3_RLS_IDX_MASK                  0x0FFF0000                // RLS_IDX[27..16]
#define WED_WPDMA_TX1_CTRL3_RLS_IDX_SHFT                  16
#define WED_WPDMA_TX1_CTRL3_DMA_IDX_MIRO_ADDR             WED_WPDMA_TX1_CTRL3_ADDR
#define WED_WPDMA_TX1_CTRL3_DMA_IDX_MIRO_MASK             0x00000FFF                // DMA_IDX_MIRO[11..0]
#define WED_WPDMA_TX1_CTRL3_DMA_IDX_MIRO_SHFT             0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON_R0_WRAP (0x15010000 + 0x640)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring0)

 =====================================================================================*/
#define RRO_RX_D_DRV_MON_R0_WRAP_CNT_ADDR                 RRO_RX_D_DRV_MON_R0_WRAP_ADDR
#define RRO_RX_D_DRV_MON_R0_WRAP_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_MON_R0_WRAP_CNT_SHFT                 0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON_R1_WRAP (0x15010000 + 0x644)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring1)

 =====================================================================================*/
#define RRO_RX_D_DRV_MON_R1_WRAP_CNT_ADDR                 RRO_RX_D_DRV_MON_R1_WRAP_ADDR
#define RRO_RX_D_DRV_MON_R1_WRAP_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_MON_R1_WRAP_CNT_SHFT                 0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON_R2_WRAP (0x15010000 + 0x648)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring2)

 =====================================================================================*/
#define RRO_RX_D_DRV_MON_R2_WRAP_CNT_ADDR                 RRO_RX_D_DRV_MON_R2_WRAP_ADDR
#define RRO_RX_D_DRV_MON_R2_WRAP_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_MON_R2_WRAP_CNT_SHFT                 0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON0 (0x15010000 + 0x64c)---

    DATA[31..0]                  - (RO) Status monitor
                                     [29:19] : FSM (AXI)
                                     [18:0] : FSM

 =====================================================================================*/
#define RRO_RX_D_DRV_MON0_DATA_ADDR                       RRO_RX_D_DRV_MON0_ADDR
#define RRO_RX_D_DRV_MON0_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_MON0_DATA_SHFT                       0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON1 (0x15010000 + 0x650)---

    DATA[31..0]                  - (RO) Status monitor
                                     [31:0] : Write address when error trigger

 =====================================================================================*/
#define RRO_RX_D_DRV_MON1_DATA_ADDR                       RRO_RX_D_DRV_MON1_ADDR
#define RRO_RX_D_DRV_MON1_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_MON1_DATA_SHFT                       0

/* =====================================================================================

  ---RRO_RX_D_DRV_MON2 (0x15010000 + 0x654)---

    DATA[31..0]                  - (RO) Status monitor
                                     [24] : FIFO write
                                     [23] : FIFO read
                                     [22] : FIFO empty
                                     [21] : FIFO full
                                     [20:16] : FIFO count
                                     [13] : write_desc_cnt_error_flag
                                     [12] : erraddr_flag
                                     [11] : err3_flag
                                     [10] : err2_flag
                                     [9] : err1_flag
                                     [8] : err0_flag
                                     [7:0] : Write address(MSB) when error trigger

 =====================================================================================*/
#define RRO_RX_D_DRV_MON2_DATA_ADDR                       RRO_RX_D_DRV_MON2_ADDR
#define RRO_RX_D_DRV_MON2_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_MON2_DATA_SHFT                       0

/* =====================================================================================

  ---RRO_RX_D_DRV_ERR_COMP0 (0x15010000 + 0x658)---

    DATA[31..0]                  - (RW) Compare write DW0 data content
                                     if write data content match, it would trigger err0_flag

 =====================================================================================*/
#define RRO_RX_D_DRV_ERR_COMP0_DATA_ADDR                  RRO_RX_D_DRV_ERR_COMP0_ADDR
#define RRO_RX_D_DRV_ERR_COMP0_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_ERR_COMP0_DATA_SHFT                  0

/* =====================================================================================

  ---RRO_RX_D_DRV_ERR_COMP1 (0x15010000 + 0x65c)---

    DATA[31..0]                  - (RW) Compare write DW1 data content
                                     if write data content match, it would trigger err1_flag

 =====================================================================================*/
#define RRO_RX_D_DRV_ERR_COMP1_DATA_ADDR                  RRO_RX_D_DRV_ERR_COMP1_ADDR
#define RRO_RX_D_DRV_ERR_COMP1_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_ERR_COMP1_DATA_SHFT                  0

/* =====================================================================================

  ---RRO_RX_D_DRV_ERR_COMP2 (0x15010000 + 0x660)---

    DATA[31..0]                  - (RW) Compare write DW2 data content
                                     if write data content match, it would trigger err2_flag

 =====================================================================================*/
#define RRO_RX_D_DRV_ERR_COMP2_DATA_ADDR                  RRO_RX_D_DRV_ERR_COMP2_ADDR
#define RRO_RX_D_DRV_ERR_COMP2_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_ERR_COMP2_DATA_SHFT                  0

/* =====================================================================================

  ---RRO_RX_D_DRV_ERR_COMP3 (0x15010000 + 0x664)---

    DATA[31..0]                  - (RW) Compare write DW3 data content
                                     if write data content match, it would trigger err3_flag

 =====================================================================================*/
#define RRO_RX_D_DRV_ERR_COMP3_DATA_ADDR                  RRO_RX_D_DRV_ERR_COMP3_ADDR
#define RRO_RX_D_DRV_ERR_COMP3_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_D_DRV_ERR_COMP3_DATA_SHFT                  0

/* =====================================================================================

  ---RRO_RX_D_DRV_LEAGLE_START (0x15010000 + 0x668)---

    WADDR[31..0]                 - (RW) Compare write desc. address region is within start and end, monitor address is below 4GB

 =====================================================================================*/
#define RRO_RX_D_DRV_LEAGLE_START_WADDR_ADDR              RRO_RX_D_DRV_LEAGLE_START_ADDR
#define RRO_RX_D_DRV_LEAGLE_START_WADDR_MASK              0xFFFFFFFF                // WADDR[31..0]
#define RRO_RX_D_DRV_LEAGLE_START_WADDR_SHFT              0

/* =====================================================================================

  ---RRO_RX_D_DRV_LEAGLE_END (0x15010000 + 0x66C)---

    WADDR[31..0]                 - (RW) Compare write desc. address region is within start and end, monitor address is below 4GB

 =====================================================================================*/
#define RRO_RX_D_DRV_LEAGLE_END_WADDR_ADDR                RRO_RX_D_DRV_LEAGLE_END_ADDR
#define RRO_RX_D_DRV_LEAGLE_END_WADDR_MASK                0xFFFFFFFF                // WADDR[31..0]
#define RRO_RX_D_DRV_LEAGLE_END_WADDR_SHFT                0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON_R0_WRAP (0x15010000 + 0x670)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring0)

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON_R0_WRAP_CNT_ADDR                RRO_RX_PG_DRV_MON_R0_WRAP_ADDR
#define RRO_RX_PG_DRV_MON_R0_WRAP_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_MON_R0_WRAP_CNT_SHFT                0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON_R1_WRAP (0x15010000 + 0x674)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring1)

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON_R1_WRAP_CNT_ADDR                RRO_RX_PG_DRV_MON_R1_WRAP_ADDR
#define RRO_RX_PG_DRV_MON_R1_WRAP_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_MON_R1_WRAP_CNT_SHFT                0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON_R2_WRAP (0x15010000 + 0x678)---

    CNT[31..0]                   - (RO) This counter will count how many times it has wrapped (Ring2)

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON_R2_WRAP_CNT_ADDR                RRO_RX_PG_DRV_MON_R2_WRAP_ADDR
#define RRO_RX_PG_DRV_MON_R2_WRAP_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_MON_R2_WRAP_CNT_SHFT                0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON0 (0x15010000 + 0x67c)---

    DATA[31..0]                  - (RO) Status monitor
                                     [29:19] : FSM (AXI)
                                     [18:0] : FSM

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON0_DATA_ADDR                      RRO_RX_PG_DRV_MON0_ADDR
#define RRO_RX_PG_DRV_MON0_DATA_MASK                      0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_MON0_DATA_SHFT                      0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON1 (0x15010000 + 0x680)---

    DATA[31..0]                  - (RO) Status monitor
                                     [31:0] : Write address when error trigger

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON1_DATA_ADDR                      RRO_RX_PG_DRV_MON1_ADDR
#define RRO_RX_PG_DRV_MON1_DATA_MASK                      0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_MON1_DATA_SHFT                      0

/* =====================================================================================

  ---RRO_RX_PG_DRV_MON2 (0x15010000 + 0x684)---

    DATA[31..0]                  - (RO) Status monitor
                                     [24] : FIFO write
                                     [23] : FIFO read
                                     [22] : FIFO empty
                                     [21] : FIFO full
                                     [20:16] : FIFO count
                                     [13] : write_desc_cnt_error_flag
                                     [12] : erraddr_flag
                                     [11] : err3_flag
                                     [10] : err2_flag
                                     [9] : err1_flag
                                     [8] : err0_flag
                                     [7:0] : Write address(MSB) when error trigger

 =====================================================================================*/
#define RRO_RX_PG_DRV_MON2_DATA_ADDR                      RRO_RX_PG_DRV_MON2_ADDR
#define RRO_RX_PG_DRV_MON2_DATA_MASK                      0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_MON2_DATA_SHFT                      0

/* =====================================================================================

  ---RRO_RX_PG_DRV_ERR_COMP0 (0x15010000 + 0x688)---

    DATA[31..0]                  - (RW) Compare write DW0 data content
                                     if write data content match, it would trigger err0_flag

 =====================================================================================*/
#define RRO_RX_PG_DRV_ERR_COMP0_DATA_ADDR                 RRO_RX_PG_DRV_ERR_COMP0_ADDR
#define RRO_RX_PG_DRV_ERR_COMP0_DATA_MASK                 0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_ERR_COMP0_DATA_SHFT                 0

/* =====================================================================================

  ---RRO_RX_PG_DRV_ERR_COMP1 (0x15010000 + 0x68c)---

    DATA[31..0]                  - (RW) Compare write DW1 data content
                                     if write data content match, it would trigger err1_flag

 =====================================================================================*/
#define RRO_RX_PG_DRV_ERR_COMP1_DATA_ADDR                 RRO_RX_PG_DRV_ERR_COMP1_ADDR
#define RRO_RX_PG_DRV_ERR_COMP1_DATA_MASK                 0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_ERR_COMP1_DATA_SHFT                 0

/* =====================================================================================

  ---RRO_RX_PG_DRV_ERR_COMP2 (0x15010000 + 0x690)---

    DATA[31..0]                  - (RW) Compare write DW2 data content
                                     if write data content match, it would trigger err2_flag

 =====================================================================================*/
#define RRO_RX_PG_DRV_ERR_COMP2_DATA_ADDR                 RRO_RX_PG_DRV_ERR_COMP2_ADDR
#define RRO_RX_PG_DRV_ERR_COMP2_DATA_MASK                 0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_ERR_COMP2_DATA_SHFT                 0

/* =====================================================================================

  ---RRO_RX_PG_DRV_ERR_COMP3 (0x15010000 + 0x694)---

    DATA[31..0]                  - (RW) Compare write DW3 data content
                                     if write data content match, it would trigger err3_flag

 =====================================================================================*/
#define RRO_RX_PG_DRV_ERR_COMP3_DATA_ADDR                 RRO_RX_PG_DRV_ERR_COMP3_ADDR
#define RRO_RX_PG_DRV_ERR_COMP3_DATA_MASK                 0xFFFFFFFF                // DATA[31..0]
#define RRO_RX_PG_DRV_ERR_COMP3_DATA_SHFT                 0

/* =====================================================================================

  ---RRO_RX_PG_DRV_LEAGLE_START (0x15010000 + 0x698)---

    WADDR[31..0]                 - (RW) Compare write desc. address region is within start and end, monitor address is below 4GB

 =====================================================================================*/
#define RRO_RX_PG_DRV_LEAGLE_START_WADDR_ADDR             RRO_RX_PG_DRV_LEAGLE_START_ADDR
#define RRO_RX_PG_DRV_LEAGLE_START_WADDR_MASK             0xFFFFFFFF                // WADDR[31..0]
#define RRO_RX_PG_DRV_LEAGLE_START_WADDR_SHFT             0

/* =====================================================================================

  ---RRO_RX_PG_DRV_LEAGLE_END (0x15010000 + 0x69C)---

    WADDR[31..0]                 - (RW) Compare write desc. address region is within start and end, monitor address is below 4GB

 =====================================================================================*/
#define RRO_RX_PG_DRV_LEAGLE_END_WADDR_ADDR               RRO_RX_PG_DRV_LEAGLE_END_ADDR
#define RRO_RX_PG_DRV_LEAGLE_END_WADDR_MASK               0xFFFFFFFF                // WADDR[31..0]
#define RRO_RX_PG_DRV_LEAGLE_END_WADDR_SHFT               0

/* =====================================================================================

  ---RRO_RX_D_DRV_ERR_SWITCH_ADDR (0x15010000 + 0x6A0)---

    BASE[31..0]                  - (RW) Switch awaddr when detect error

 =====================================================================================*/
#define RRO_RX_D_DRV_ERR_SWITCH_ADDR_BASE_ADDR            RRO_RX_D_DRV_ERR_SWITCH_ADDR_ADDR
#define RRO_RX_D_DRV_ERR_SWITCH_ADDR_BASE_MASK            0xFFFFFFFF                // BASE[31..0]
#define RRO_RX_D_DRV_ERR_SWITCH_ADDR_BASE_SHFT            0

/* =====================================================================================

  ---RRO_RX_PG_DRV_ERR_SWITCH_ADDR (0x15010000 + 0x6A4)---

    BASE[31..0]                  - (RW) Switch awaddr when detect error

 =====================================================================================*/
#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR_BASE_ADDR           RRO_RX_PG_DRV_ERR_SWITCH_ADDR_ADDR
#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR_BASE_MASK           0xFFFFFFFF                // BASE[31..0]
#define RRO_RX_PG_DRV_ERR_SWITCH_ADDR_BASE_SHFT           0

/* =====================================================================================

  ---RRO_RX_D_DRV_IN_MIB (0x15010000 + 0x6A8)---

    CNT[31..0]                   - (RO) Counter for read free buffer from RXBM

 =====================================================================================*/
#define RRO_RX_D_DRV_IN_MIB_CNT_ADDR                      RRO_RX_D_DRV_IN_MIB_ADDR
#define RRO_RX_D_DRV_IN_MIB_CNT_MASK                      0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_IN_MIB_CNT_SHFT                      0

/* =====================================================================================

  ---RRO_RX_D_DRV_OU_MIB (0x15010000 + 0x6AC)---

    CNT[31..0]                   - (RO) Counter for read free buffer from FIFO

 =====================================================================================*/
#define RRO_RX_D_DRV_OU_MIB_CNT_ADDR                      RRO_RX_D_DRV_OU_MIB_ADDR
#define RRO_RX_D_DRV_OU_MIB_CNT_MASK                      0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_OU_MIB_CNT_SHFT                      0

/* =====================================================================================

  ---RRO_RX_D_DRV_DESC_MIB (0x15010000 + 0x6B0)---

    CNT[31..0]                   - (RO) Counter for write DMAD

 =====================================================================================*/
#define RRO_RX_D_DRV_DESC_MIB_CNT_ADDR                    RRO_RX_D_DRV_DESC_MIB_ADDR
#define RRO_RX_D_DRV_DESC_MIB_CNT_MASK                    0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_D_DRV_DESC_MIB_CNT_SHFT                    0

/* =====================================================================================

  ---RRO_RX_PG_DRV_IN_MIB (0x15010000 + 0x6B4)---

    CNT[31..0]                   - (RO) Counter for read free buffer from RXBM

 =====================================================================================*/
#define RRO_RX_PG_DRV_IN_MIB_CNT_ADDR                     RRO_RX_PG_DRV_IN_MIB_ADDR
#define RRO_RX_PG_DRV_IN_MIB_CNT_MASK                     0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_IN_MIB_CNT_SHFT                     0

/* =====================================================================================

  ---RRO_RX_PG_DRV_OU_MIB (0x15010000 + 0x6B8)---

    CNT[31..0]                   - (RO) Counter for read free buffer from FIFO

 =====================================================================================*/
#define RRO_RX_PG_DRV_OU_MIB_CNT_ADDR                     RRO_RX_PG_DRV_OU_MIB_ADDR
#define RRO_RX_PG_DRV_OU_MIB_CNT_MASK                     0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_OU_MIB_CNT_SHFT                     0

/* =====================================================================================

  ---RRO_RX_PG_DRV_DESC_MIB (0x15010000 + 0x6BC)---

    CNT[31..0]                   - (RO) Counter for write DMAD

 =====================================================================================*/
#define RRO_RX_PG_DRV_DESC_MIB_CNT_ADDR                   RRO_RX_PG_DRV_DESC_MIB_ADDR
#define RRO_RX_PG_DRV_DESC_MIB_CNT_MASK                   0xFFFFFFFF                // CNT[31..0]
#define RRO_RX_PG_DRV_DESC_MIB_CNT_SHFT                   0

/* =====================================================================================

  ---RRO_RX_D_DRV_LOOPBACK_INDEX (0x15010000 + 0x6C0)---

    CIDX[11..0]                  - (RW) In the loopback function, let rro_rx_d_drv axi write the location of CIDX and also axi read the location of DIDX.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_DRV_LOOPBACK_INDEX_CIDX_ADDR             RRO_RX_D_DRV_LOOPBACK_INDEX_ADDR
#define RRO_RX_D_DRV_LOOPBACK_INDEX_CIDX_MASK             0x00000FFF                // CIDX[11..0]
#define RRO_RX_D_DRV_LOOPBACK_INDEX_CIDX_SHFT             0

/* =====================================================================================

  ---WED_WPDMA_RX0_CTRL0 (0x15010000 + 0x700)---

    BASE_PTR[31..0]              - (RO) Point to the base address of RXD Ring #0. It should be a 4-DWORD aligned address
                                     This is the same address as WED_RX0_CTRL0

 =====================================================================================*/
#define WED_WPDMA_RX0_CTRL0_BASE_PTR_ADDR                 WED_WPDMA_RX0_CTRL0_ADDR
#define WED_WPDMA_RX0_CTRL0_BASE_PTR_MASK                 0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_WPDMA_RX0_CTRL0_BASE_PTR_SHFT                 0

/* =====================================================================================

  ---WED_WPDMA_RX0_CTRL1 (0x15010000 + 0x704)---

    MAX_CNT[11..0]               - (RO) The maximum number of RXD count in RXD Ring #0.
                                     This is the same value as WED_RX0_CTRL0
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RO) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX0_CTRL1_BASE_PTR_H_ADDR               WED_WPDMA_RX0_CTRL1_ADDR
#define WED_WPDMA_RX0_CTRL1_BASE_PTR_H_MASK               0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WPDMA_RX0_CTRL1_BASE_PTR_H_SHFT               16
#define WED_WPDMA_RX0_CTRL1_MAX_CNT_ADDR                  WED_WPDMA_RX0_CTRL1_ADDR
#define WED_WPDMA_RX0_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define WED_WPDMA_RX0_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_RX0_CTRL2 (0x15010000 + 0x708)---

    CPU_IDX[11..0]               - (RO) Point to the next RXD hardware driver agent wants to allocate to RXD Ring #0.
                                     WiFi side WPDMA RX CPU_IDX was configured by WED hardware rather than WiFi host driver
    RESERVED12[15..12]           - (RO) Reserved bits
    DRV_IDX[27..16]              - (RO) Point to the next RXD hardware driver agent wants to process of RXD Ring #0.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX0_CTRL2_DRV_IDX_ADDR                  WED_WPDMA_RX0_CTRL2_ADDR
#define WED_WPDMA_RX0_CTRL2_DRV_IDX_MASK                  0x0FFF0000                // DRV_IDX[27..16]
#define WED_WPDMA_RX0_CTRL2_DRV_IDX_SHFT                  16
#define WED_WPDMA_RX0_CTRL2_CPU_IDX_ADDR                  WED_WPDMA_RX0_CTRL2_ADDR
#define WED_WPDMA_RX0_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define WED_WPDMA_RX0_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_RX0_CTRL3 (0x15010000 + 0x70c)---

    DMA_IDX_MIRO[11..0]          - (RO) Point to the next RXD DMA wants to use in RXD Ring#0. It should be a 4-DWORD aligned address.
                                     Mirror of WiFi WPDMA RX DMA_IDX.
                                     WED hardware reads from WiFi side WPDMA RX DMA_IDX and puts the value in this register.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX0_CTRL3_DMA_IDX_MIRO_ADDR             WED_WPDMA_RX0_CTRL3_ADDR
#define WED_WPDMA_RX0_CTRL3_DMA_IDX_MIRO_MASK             0x00000FFF                // DMA_IDX_MIRO[11..0]
#define WED_WPDMA_RX0_CTRL3_DMA_IDX_MIRO_SHFT             0

/* =====================================================================================

  ---WED_WPDMA_RX1_CTRL0 (0x15010000 + 0x710)---

    BASE_PTR[31..0]              - (RO) Point to the base address of RXD Ring #1. It should be a 4-DWORD aligned address
                                     This is the same address as WED_RX1_CTRL0

 =====================================================================================*/
#define WED_WPDMA_RX1_CTRL0_BASE_PTR_ADDR                 WED_WPDMA_RX1_CTRL0_ADDR
#define WED_WPDMA_RX1_CTRL0_BASE_PTR_MASK                 0xFFFFFFFF                // BASE_PTR[31..0]
#define WED_WPDMA_RX1_CTRL0_BASE_PTR_SHFT                 0

/* =====================================================================================

  ---WED_WPDMA_RX1_CTRL1 (0x15010000 + 0x714)---

    MAX_CNT[11..0]               - (RO) The maximum number of RXD count in RXD Ring #1.
                                     This is the same value as WED_RX1_CTRL1
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RO) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX1_CTRL1_BASE_PTR_H_ADDR               WED_WPDMA_RX1_CTRL1_ADDR
#define WED_WPDMA_RX1_CTRL1_BASE_PTR_H_MASK               0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WPDMA_RX1_CTRL1_BASE_PTR_H_SHFT               16
#define WED_WPDMA_RX1_CTRL1_MAX_CNT_ADDR                  WED_WPDMA_RX1_CTRL1_ADDR
#define WED_WPDMA_RX1_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define WED_WPDMA_RX1_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_RX1_CTRL2 (0x15010000 + 0x718)---

    CPU_IDX[11..0]               - (RO) Point to the next RXD hardware driver agent wants to allocate to RXD Ring #1.
                                     WiFi side WPDMA RX CPU_IDX was configured by WED hardware rather than WiFi host driver
    RESERVED12[15..12]           - (RO) Reserved bits
    DRV_IDX[27..16]              - (RO) Point to the next RXD hardware driver agent wants to process of RXD Ring #1.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX1_CTRL2_DRV_IDX_ADDR                  WED_WPDMA_RX1_CTRL2_ADDR
#define WED_WPDMA_RX1_CTRL2_DRV_IDX_MASK                  0x0FFF0000                // DRV_IDX[27..16]
#define WED_WPDMA_RX1_CTRL2_DRV_IDX_SHFT                  16
#define WED_WPDMA_RX1_CTRL2_CPU_IDX_ADDR                  WED_WPDMA_RX1_CTRL2_ADDR
#define WED_WPDMA_RX1_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define WED_WPDMA_RX1_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---WED_WPDMA_RX1_CTRL3 (0x15010000 + 0x71c)---

    DMA_IDX_MIRO[11..0]          - (RO) Point to the next RXD DMA wants to use in RXD Ring#1. It should be a 4-DWORD aligned address.
                                     Mirror of WiFi WPDMA RX DMA_IDX.
                                     WED hardware reads from WiFi side WPDMA RX DMA_IDX and puts the value in this register.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX1_CTRL3_DMA_IDX_MIRO_ADDR             WED_WPDMA_RX1_CTRL3_ADDR
#define WED_WPDMA_RX1_CTRL3_DMA_IDX_MIRO_MASK             0x00000FFF                // DMA_IDX_MIRO[11..0]
#define WED_WPDMA_RX1_CTRL3_DMA_IDX_MIRO_SHFT             0

/* =====================================================================================

  ---WED_WPDMA_RX_D_INFO (0x15010000 + 0x758)---

    RESERVED0[7..0]              - (RO) Reserved bits
    RX_RING_NUM[15..8]           - (RO) Rx ring number
    BASE_PTR_WIDTH[23..16]       - (RO) Base pointer width, x
                                     base_addr[31:32-x] is shared with all ring base address. Only ring #0 base address [31:32-x] field Is write-able.
                                     [Note]: "0" means no bit of base address is shared.
    INDEX_WIDTH[27..24]          - (RO) Point to the next RX DMAD CPU wants to use
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RX_D_INFO_INDEX_WIDTH_ADDR              WED_WPDMA_RX_D_INFO_ADDR
#define WED_WPDMA_RX_D_INFO_INDEX_WIDTH_MASK              0x0F000000                // INDEX_WIDTH[27..24]
#define WED_WPDMA_RX_D_INFO_INDEX_WIDTH_SHFT              24
#define WED_WPDMA_RX_D_INFO_BASE_PTR_WIDTH_ADDR           WED_WPDMA_RX_D_INFO_ADDR
#define WED_WPDMA_RX_D_INFO_BASE_PTR_WIDTH_MASK           0x00FF0000                // BASE_PTR_WIDTH[23..16]
#define WED_WPDMA_RX_D_INFO_BASE_PTR_WIDTH_SHFT           16
#define WED_WPDMA_RX_D_INFO_RX_RING_NUM_ADDR              WED_WPDMA_RX_D_INFO_ADDR
#define WED_WPDMA_RX_D_INFO_RX_RING_NUM_MASK              0x0000FF00                // RX_RING_NUM[15..8]
#define WED_WPDMA_RX_D_INFO_RX_RING_NUM_SHFT              8

/* =====================================================================================

  ---WED_WDMA_TX0_BASE (0x15010000 + 0x800)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)

 =====================================================================================*/
#define WED_WDMA_TX0_BASE_PTR_ADDR                        WED_WDMA_TX0_BASE_ADDR
#define WED_WDMA_TX0_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define WED_WDMA_TX0_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---WED_WDMA_TX0_CNT (0x15010000 + 0x804)---

    MAX[11..0]                   - (RW) The maximum number of TXD count in RX Ring #0
                                     Set 0 or 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX0_CNT_BASE_PTR_H_ADDR                  WED_WDMA_TX0_CNT_ADDR
#define WED_WDMA_TX0_CNT_BASE_PTR_H_MASK                  0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WDMA_TX0_CNT_BASE_PTR_H_SHFT                  16
#define WED_WDMA_TX0_CNT_MAX_ADDR                         WED_WDMA_TX0_CNT_ADDR
#define WED_WDMA_TX0_CNT_MAX_MASK                         0x00000FFF                // MAX[11..0]
#define WED_WDMA_TX0_CNT_MAX_SHFT                         0

/* =====================================================================================

  ---WED_WDMA_TX0_CTX_IDX (0x15010000 + 0x808)---

    CTX_IDX[11..0]               - (RO) Point to the next TXD hardware driver agent wants to use, this index of WDMA is controlled by WED hardware.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX0_CTX_IDX_CTX_IDX_ADDR                 WED_WDMA_TX0_CTX_IDX_ADDR
#define WED_WDMA_TX0_CTX_IDX_CTX_IDX_MASK                 0x00000FFF                // CTX_IDX[11..0]
#define WED_WDMA_TX0_CTX_IDX_CTX_IDX_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_TX0_DTX_IDX (0x15010000 + 0x80c)---

    DTX_IDX[11..0]               - (RO) Point to the next TXD DMA wants to use, this is a mirrored value of WDMA TX_DRX_IDX.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX0_DTX_IDX_DTX_IDX_ADDR                 WED_WDMA_TX0_DTX_IDX_ADDR
#define WED_WDMA_TX0_DTX_IDX_DTX_IDX_MASK                 0x00000FFF                // DTX_IDX[11..0]
#define WED_WDMA_TX0_DTX_IDX_DTX_IDX_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_TX0_MIB_0 (0x15010000 + 0x810)---

    WR_CNT[31..0]                - (RC) FIFO write count

 =====================================================================================*/
#define WED_WDMA_TX0_MIB_0_WR_CNT_ADDR                    WED_WDMA_TX0_MIB_0_ADDR
#define WED_WDMA_TX0_MIB_0_WR_CNT_MASK                    0xFFFFFFFF                // WR_CNT[31..0]
#define WED_WDMA_TX0_MIB_0_WR_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_BASE (0x15010000 + 0x814)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_BASE_PTR_ADDR           WED_WPDMA_RRO3_1_RX_D_RX0_BASE_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_BASE_PTR_MASK           0xFFFFFFFF                // PTR[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_BASE_PTR_SHFT           0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_CNT (0x15010000 + 0x818)---

    MAX[15..0]                   - (RW) The maximum number of RX DMAD count in RX Ring #0
                                     Set 0 is disable this ring, driver does not access Ring, cpu index and dma index.
                                     Set 1 is illegal.
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_BASE_PTR_H_ADDR     WED_WPDMA_RRO3_1_RX_D_RX0_CNT_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_BASE_PTR_H_MASK     0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_BASE_PTR_H_SHFT     16
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_MAX_ADDR            WED_WPDMA_RRO3_1_RX_D_RX0_CNT_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_MAX_MASK            0x0000FFFF                // MAX[15..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_CNT_MAX_SHFT            0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX (0x15010000 + 0x81C)---

    CRX_IDX[15..0]               - (RO) Point to the next RX DMAD hardware driver agent wants to use, this index of WPDMA is controlled by WED hardware.
    DRV_IDX[31..16]              - (RO) Point to the next RX DMAD hardware driver agent wants to process, this index is controlled by WED hardware.

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_DRV_IDX_ADDR    WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_DRV_IDX_MASK    0xFFFF0000                // DRV_IDX[31..16]
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_DRV_IDX_SHFT    16
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_CRX_IDX_ADDR    WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_CRX_IDX_MASK    0x0000FFFF                // CRX_IDX[15..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_CRX_IDX_CRX_IDX_SHFT    0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX (0x15010000 + 0x820)---

    DRX_IDX_MIRO[15..0]          - (RO) Point to the next RX DMAD which DMA wants to use, this is a mirrored value of WPDMA RX_DRX_IDX.
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_ADDR WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_MASK 0x0000FFFF                // DRX_IDX_MIRO[15..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_GLO_CFG (0x15010000 + 0x824)---

    RX_DRV_EN[0]                 - (RW) Rx Driver Module Enable
    RX_DRV_BUSY[1]               - (RO) Rx Driver Module Busy indicator
    RXDRV_CLKGATE_BYP[2]         - (RW) Rx Driver Module clock gating Bypass
    FSM_RETURN_IDLE[3]           - (RW) Force RX Driver Finite State Machine return to IDLE as possible as it can. (Will not returned to IDLE when current state is related to bus access)
    RST_INIT_COMPLETE_FLAG[4]    - (RW) Reset the Rx Driver module initialize completed flag. So that when Rx Driver is enabled and FSM is IDLE state, it will initialize DMA again. Before set this register to 1, make sure Rx Driver is not enabled and not busy.
    INIT_PHASE_BYPASS[5]         - (RW) RX Driver module will initial WPDMA Rx Ring when Rx Driver HW module is enabled. If WPDMA Rx Ring is initialized by software, set this register to 1 to disable the HW initial function.
    INIT_PHASE_ENABLING_RXDMA_BYPASS[6] - (RW) When Rx Driver module finished the WPDMA Rx Ring initial sequence, Rx Driver module will turn on the WPDMA Rx Enable register automatically. If this register is set to 1, that Rx Driver module will not enable WPDMA Rx DMA utomatically after WPDMA Rx Ring initial sequence has finished.
    INIT_PHASE_RXEN_SEL[11..7]   - (RW) Bit location of RX DMA enable bit in WPDMA register field
                                     Value 0~31 means Bit 0 ~ Bit 31
    RXDRV_DISABLED_FSM_AUTO_IDLE[12] - (RW) When Rx Driver is disabled, RX Driver Finite State Machine return to IDLE automatically as possible as it can.(Even though the operation of Rx Driver module is not completed yet)
    DISABLE_IRQ_HANDLE_PROC[13]  - (RW) Disable the interrupt handle process, which is the main function of Rx Driver. If the function is disabled, Rx Driver module will not accept and handle any Rx Done interrupt provided by WED WPDMA interrupt agent. Usually, the function should not be disabled.
    DISABLE_DMAD_HANDLE_PROC[14] - (RW) Disable the basic DMAD handle process, which is the main function of Rx Driver. If the function is disabled, Rx Driver module will not manage the Frame Engine WPDMA Rx Ring. Usually, the function should not be disabled.
    WCOMPLETE_SEL[15]            - (RW) select the AXI wready or bvalid as write complete signal for Rx Driver module
    WAIT_COHERENT_EN[16]         - (RW) Enable the coherent wait and reread descriptor feature while Rx Driver coherent problem occurs (DMAD DDONE=0). If enabled, Rx Driver will wait until coherent wait counter reach the maximum count value (configurable) and then re-read the descriptor.
    AXI_W_AFTER_AW_EN[17]        - (RW) Enable the Rx Driver module AXI W channel scheduler to postpone the W channel access until AW channel has completed. Enable this feature may solve some AXI access problem, but also degrade the AXI write performance. Note that not all AXI Slave support this protocol.
    axi_w_after_aw_busy[18]      - (RO) modu.e axi_w_after_aw busy signal
    axi_w_after_aw_state[20..19] - (RO) modu.e axi_w_after_aw state monitor
    RESERVED21[31..21]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_state_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_state_MASK 0x00180000                // axi_w_after_aw_state[20..19]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_state_SHFT 19
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_busy_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_busy_MASK 0x00040000                // axi_w_after_aw_busy[18]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_axi_w_after_aw_busy_SHFT 18
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_AXI_W_AFTER_AW_EN_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_AXI_W_AFTER_AW_EN_MASK 0x00020000                // AXI_W_AFTER_AW_EN[17]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_AXI_W_AFTER_AW_EN_SHFT 17
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WAIT_COHERENT_EN_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WAIT_COHERENT_EN_MASK 0x00010000                // WAIT_COHERENT_EN[16]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WAIT_COHERENT_EN_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WCOMPLETE_SEL_ADDR  WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WCOMPLETE_SEL_MASK  0x00008000                // WCOMPLETE_SEL[15]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_WCOMPLETE_SEL_SHFT  15
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_MASK 0x00004000                // DISABLE_DMAD_HANDLE_PROC[14]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_SHFT 14
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_MASK 0x00002000                // DISABLE_IRQ_HANDLE_PROC[13]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_SHFT 13
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_MASK 0x00001000                // RXDRV_DISABLED_FSM_AUTO_IDLE[12]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_SHFT 12
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_RXEN_SEL_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_RXEN_SEL_MASK 0x00000F80                // INIT_PHASE_RXEN_SEL[11..7]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_RXEN_SEL_SHFT 7
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_MASK 0x00000040                // INIT_PHASE_ENABLING_RXDMA_BYPASS[6]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_SHFT 6
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_BYPASS_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_BYPASS_MASK 0x00000020                // INIT_PHASE_BYPASS[5]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_INIT_PHASE_BYPASS_SHFT 5
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RST_INIT_COMPLETE_FLAG_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RST_INIT_COMPLETE_FLAG_MASK 0x00000010                // RST_INIT_COMPLETE_FLAG[4]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RST_INIT_COMPLETE_FLAG_SHFT 4
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_FSM_RETURN_IDLE_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_FSM_RETURN_IDLE_MASK 0x00000008                // FSM_RETURN_IDLE[3]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_FSM_RETURN_IDLE_SHFT 3
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_CLKGATE_BYP_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_CLKGATE_BYP_MASK 0x00000004                // RXDRV_CLKGATE_BYP[2]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RXDRV_CLKGATE_BYP_SHFT 2
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_BUSY_ADDR    WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_BUSY_MASK    0x00000002                // RX_DRV_BUSY[1]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_BUSY_SHFT    1
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_ADDR      WED_WPDMA_RRO3_1_RX_D_GLO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_MASK      0x00000001                // RX_DRV_EN[0]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_RX_DRV_EN_SHFT      0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RST_IDX (0x15010000 + 0x828)---

    CRX_IDX0[0]                  - (RW) Write 1 to reset RX_DRV CRX_IDX0 to 0
    DRX_IDX_ALL[1]               - (RW) These 32bit registers are used to reset the index of WPDMA Rx Data Driver module inside of WED, will not reset the index logic on WPDMA
                                     Write 1 to reset RX_DRV DRX_IDX0 and DRX_IDX1 to 0
    DRV_IDX0[2]                  - (RW) Write 1 to reset RX_DRV DRV_IDX0 to 0
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRV_IDX0_ADDR       WED_WPDMA_RRO3_1_RX_D_RST_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRV_IDX0_MASK       0x00000004                // DRV_IDX0[2]
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRV_IDX0_SHFT       2
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRX_IDX_ALL_ADDR    WED_WPDMA_RRO3_1_RX_D_RST_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRX_IDX_ALL_MASK    0x00000002                // DRX_IDX_ALL[1]
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_DRX_IDX_ALL_SHFT    1
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_CRX_IDX0_ADDR       WED_WPDMA_RRO3_1_RX_D_RST_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_CRX_IDX0_MASK       0x00000001                // CRX_IDX0[0]
#define WED_WPDMA_RRO3_1_RX_D_RST_IDX_CRX_IDX0_SHFT       0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX (0x15010000 + 0x82C)---

    DRV_IDX0[15..0]              - (RW) The data to be overwrite into DRV_IDX1
    LOAD_DRV_IDX0[16]            - (RW) Write 1 to overwrite the Rx Driver DRV_IDX1
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_LOAD_DRV_IDX0_ADDR WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_LOAD_DRV_IDX0_MASK 0x00010000                // LOAD_DRV_IDX0[16]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_LOAD_DRV_IDX0_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_DRV_IDX0_ADDR  WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_DRV_IDX0_MASK  0x0000FFFF                // DRV_IDX0[15..0]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRV_IDX_DRV_IDX0_SHFT  0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX (0x15010000 + 0x830)---

    CRX_IDX0[15..0]              - (RW) The data to be overwrite into CRX_IDX1
    LOAD_CRX_IDX0[16]            - (RW) Write 1 to overwrite the Rx Driver CRX_IDX1
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_LOAD_CRX_IDX0_ADDR WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_LOAD_CRX_IDX0_MASK 0x00010000                // LOAD_CRX_IDX0[16]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_LOAD_CRX_IDX0_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_CRX_IDX0_ADDR  WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_CRX_IDX0_MASK  0x0000FFFF                // CRX_IDX0[15..0]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_CRX_IDX_CRX_IDX0_SHFT  0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR (0x15010000 + 0x834)---

    ADDRESS[31..0]               - (RW) WPDMA global configuration register address

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDRESS_MASK 0xFFFFFFFF                // ADDRESS[31..0]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H (0x15010000 + 0x838)---

    ADDRESS[7..0]                - (RW) Bit[39:32] of WPDMA global configuration register address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H_ADDR
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H_ADDRESS_MASK 0x000000FF                // ADDRESS[7..0]
#define WED_WPDMA_RRO3_1_RX_D_GLO_CFG_BASE_ADDR_H_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_MIB (0x15010000 + 0x83C)---

    PKT_CNT[31..0]               - (RC) WPDMA Rx Data ring0 packet counter (the counter will not be increased if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_MIB_PKT_CNT_ADDR        WED_WPDMA_RRO3_1_RX_D_RX0_MIB_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_MIB_PKT_CNT_MASK        0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_MIB_PKT_CNT_SHFT        0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB (0x15010000 + 0x840)---

    PKT_CNT[31..0]               - (RC) WPDMA Rx data ring0 all packet processed counter (the counter counts all packet processed by Rx Driver, even if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_PKT_CNT_ADDR WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_PKT_CNT_MASK 0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_PROCESSED_MIB_PKT_CNT_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB (0x15010000 + 0x844)---

    CNT[31..0]                   - (RC) WPDMA Rx Data Driver Rx coherent event counter

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB_CNT_ADDR    WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB_CNT_MASK    0xFFFFFFFF                // CNT[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX_COHERENT_MIB_CNT_SHFT    0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_ERR_STS (0x15010000 + 0x848)---

    RX_DRV_R_RESP_ERR[0]         - (W1C) WPDMA Rx Data Driver module AXI read response error
    RX_DRV_W_RESP_ERR[1]         - (W1C) WPDMA Rx Data Driver module AXI write response error
    RX_DRV_COHERENT[2]           - (W1C) WPDMA Rx Data Driver encounter coherence problem. The DMAD read from WPDMA Rx Ring has DDONE=0 problem.
    RX_DRV_INTI_WDMA_ENABLED[3]  - (W1C) WPDMA Rx Data Driver finds that WPDMA Rx has already been enabled when RX Driver module is initializing WPDMA Rx Data Ring.  This should not happened, check the programming sequence.
    RESERVED4[7..4]              - (RO) Reserved bits
    RX_DRV_PREF_R_RESP_ERR[8]    - (W1C) WPDMA Rx Data Driver prefetch agent AXI read response error
    RX_DRV_PREF_DMAD_ERR[9]      - (W1C) WPDMA Rx Data Driver get a DMAD with wrong DDONE value from prefetch agent when DMAD prefetch is enabled.
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_DMAD_ERR_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_DMAD_ERR_MASK 0x00000200                // RX_DRV_PREF_DMAD_ERR[9]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_DMAD_ERR_SHFT 9
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_R_RESP_ERR_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_R_RESP_ERR_MASK 0x00000100                // RX_DRV_PREF_R_RESP_ERR[8]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_PREF_R_RESP_ERR_SHFT 8
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_INTI_WDMA_ENABLED_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_INTI_WDMA_ENABLED_MASK 0x00000008                // RX_DRV_INTI_WDMA_ENABLED[3]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_INTI_WDMA_ENABLED_SHFT 3
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_COHERENT_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_COHERENT_MASK 0x00000004                // RX_DRV_COHERENT[2]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_COHERENT_SHFT 2
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_W_RESP_ERR_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_W_RESP_ERR_MASK 0x00000002                // RX_DRV_W_RESP_ERR[1]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_W_RESP_ERR_SHFT 1
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_R_RESP_ERR_ADDR WED_WPDMA_RRO3_1_RX_D_ERR_STS_ADDR
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_R_RESP_ERR_MASK 0x00000001                // RX_DRV_R_RESP_ERR[0]
#define WED_WPDMA_RRO3_1_RX_D_ERR_STS_RX_DRV_R_RESP_ERR_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_BUS_CFG (0x15010000 + 0x84C)---

    WR_BND_4KB_BST[0]            - (RW) Enable AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RD_BND_4KB_BST[1]            - (RW) Enable AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    RESERVED2[7..2]              - (RO) Reserved bits
    AWULTRA[9..8]                - (RW) AXI write ultra control
    RESERVED10[11..10]           - (RO) Reserved bits
    ARULTRA[13..12]              - (RW) AXI read ultra control
    RESERVED14[31..14]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ARULTRA_ADDR        WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ARULTRA_MASK        0x00003000                // ARULTRA[13..12]
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ARULTRA_SHFT        12
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_AWULTRA_ADDR        WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_AWULTRA_MASK        0x00000300                // AWULTRA[9..8]
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_AWULTRA_SHFT        8
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_RD_BND_4KB_BST_ADDR WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_RD_BND_4KB_BST_MASK 0x00000002                // RD_BND_4KB_BST[1]
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_RD_BND_4KB_BST_SHFT 1
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_WR_BND_4KB_BST_ADDR WED_WPDMA_RRO3_1_RX_D_BUS_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_WR_BND_4KB_BST_MASK 0x00000001                // WR_BND_4KB_BST[0]
#define WED_WPDMA_RRO3_1_RX_D_BUS_CFG_WR_BND_4KB_BST_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_PREF_CFG (0x15010000 + 0x850)---

    ENABLE[0]                    - (RW) Enable prefetch agent
    BUSY[1]                      - (RO) Prefetch agent busy
    DMAD_SIZE[2]                 - (RW) Size of each DMAD
    MAGIC_COUNT_CHK[3]           - (RW) Enable MAGIC COUNT Check.
                                     The DMAD with wrong MAGIC COUNT and all DMAD afterward will be dropped. Prefetch agent will read DMAD with wrong MAGIC COUNT later.
    RESERVED4[4]                 - (RO) Reserved bits
    WR_BND_4KB_BST[5]            - (RW) Enable AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RD_BND_4KB_BST[6]            - (RW) Enable AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    RESERVED7[7]                 - (RO) Reserved bits
    BURST_SIZE[12..8]            - (RW) Prefetch Bus Burst Size
                                     Bus burst size. Unit is 8bytes. Maximum value is 16 (128bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (128bytes) or less than size of 1 DMAD.
    RESERVED13[15..13]           - (RO) Reserved bits
    LOW_THRES[20..16]            - (RW) Prefetch FIFO Low Threshold.
                                     Ring which data byte in prefetch FIFO under low threshold has higher priority than others.
                                     Unit is 8bytes. Maximum is 16 (128bytes). Set the register to 0 will disable the low threshold mechanism. Recommend low threshold is size of 1 or 2 DMAD.
    RESERVED21[21]               - (RO) Reserved bits
    MAGIC_COUNT_MONITER[25..22]  - (RO) Prefetch magic count moniter
    MAGIC_COUNT_OW_VALUE[29..26] - (RW) Prefetch magic count overwrite value, will overwrite by magic_count_ow
    MAGIC_COUNT_OW[30]           - (A0) Prefetch magic count overwrite trigger bit,
                                     set 1 to overwrite the magic count from magic_count_ow_value, this bit will autoclear by HW when overwrite done
    RESERVED31[31]               - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_MASK 0x40000000                // MAGIC_COUNT_OW[30]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_SHFT 30
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_VALUE_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_VALUE_MASK 0x3C000000                // MAGIC_COUNT_OW_VALUE[29..26]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_OW_VALUE_SHFT 26
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_MONITER_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_MONITER_MASK 0x03C00000                // MAGIC_COUNT_MONITER[25..22]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_MONITER_SHFT 22
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_LOW_THRES_ADDR     WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_LOW_THRES_MASK     0x001F0000                // LOW_THRES[20..16]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_LOW_THRES_SHFT     16
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BURST_SIZE_ADDR    WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BURST_SIZE_MASK    0x00001F00                // BURST_SIZE[12..8]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BURST_SIZE_SHFT    8
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_RD_BND_4KB_BST_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_RD_BND_4KB_BST_MASK 0x00000040                // RD_BND_4KB_BST[6]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_RD_BND_4KB_BST_SHFT 6
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_WR_BND_4KB_BST_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_WR_BND_4KB_BST_MASK 0x00000020                // WR_BND_4KB_BST[5]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_WR_BND_4KB_BST_SHFT 5
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_CHK_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_CHK_MASK 0x00000008                // MAGIC_COUNT_CHK[3]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_MAGIC_COUNT_CHK_SHFT 3
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_DMAD_SIZE_ADDR     WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_DMAD_SIZE_MASK     0x00000004                // DMAD_SIZE[2]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_DMAD_SIZE_SHFT     2
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BUSY_ADDR          WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BUSY_MASK          0x00000002                // BUSY[1]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_BUSY_SHFT          1
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_ADDR        WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_MASK        0x00000001                // ENABLE[0]
#define WED_WPDMA_RRO3_1_RX_D_PREF_CFG_ENABLE_SHFT        0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0 (0x15010000 + 0x854)---

    START_IDX[15..0]             - (RO) Point to the next DMAD that prefetch agent wants to read
    IDX_OW_VAL[31..16]           - (RW) The index will be overwrite by this register field

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_IDX_OW_VAL_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_IDX_OW_VAL_MASK 0xFFFF0000                // IDX_OW_VAL[31..16]
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_IDX_OW_VAL_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_START_IDX_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_START_IDX_MASK 0x0000FFFF                // START_IDX[15..0]
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_0_START_IDX_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1 (0x15010000 + 0x858)---

    IDX_CLR[0]                   - (RW) Clear the index
    IDX_OW[1]                    - (RW) Overwrite the index
    RESERVED2[31..2]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_OW_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_OW_MASK 0x00000002                // IDX_OW[1]
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_OW_SHFT 1
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_CLR_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_CLR_MASK 0x00000001                // IDX_CLR[0]
#define WED_WPDMA_RRO3_1_RX_D_PREF_RX0_SIDX_1_IDX_CLR_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG (0x15010000 + 0x85C)---

    RING0_CLEAR[0]               - (RW) Clear Ring0 FIFO
    RING0_FULL[1]                - (RO) Ring0 FIFO Full
    RING0_EMPTY[2]               - (RO) Ring0 FIFO Empty
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[8..4]         - (RO) Used Ring0 FIFO entry
    RESERVED9[9]                 - (RO) Reserved bits
    RING0_FREE_CNT[14..10]       - (RO) Free Ring0 FIFO entry
    RESERVED15[31..15]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FREE_CNT_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FREE_CNT_MASK 0x00007C00                // RING0_FREE_CNT[14..10]
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FREE_CNT_SHFT 10
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_USED_CNT_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_USED_CNT_MASK 0x000001F0                // RING0_USED_CNT[8..4]
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_USED_CNT_SHFT 4
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_EMPTY_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_EMPTY_MASK 0x00000004                // RING0_EMPTY[2]
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_EMPTY_SHFT 2
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FULL_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FULL_MASK 0x00000002                // RING0_FULL[1]
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_FULL_SHFT 1
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_CLEAR_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_CLEAR_MASK 0x00000001                // RING0_CLEAR[0]
#define WED_WPDMA_RRO3_1_RX_D_PREF_FIFO_CFG_RING0_CLEAR_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG (0x15010000 + 0x860)---

    CIDX_DLY_THRES_EN[0]         - (RW) Enable CIDX write delay  threshold
                                     Write CIDX when DIDX-CIDX < Threshold
    CIDX_DLY_CNT_CLR[1]          - (RW) Clear CIDX delay counter
    CIDX_DLY_CNT_MAX[9..2]       - (RW) CIDX write delay max count
                                     If CIDX_DLY_CNT_MAX=0,  then write delay feature will be disabled.
    CIDX_DLY_THRES[25..10]       - (RW) CIDX write delay threshold
                                     If CIDX_DLY_THRES_EN=1 but the threshold=0, then write delay feature will be disabled
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_ADDR WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_MASK 0x03FFFC00                // CIDX_DLY_THRES[25..10]
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_SHFT 10
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_ADDR WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_MASK 0x000003FC                // CIDX_DLY_CNT_MAX[9..2]
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_SHFT 2
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_ADDR WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_MASK 0x00000002                // CIDX_DLY_CNT_CLR[1]
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_SHFT 1
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_EN_ADDR WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_ADDR
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_EN_MASK 0x00000001                // CIDX_DLY_THRES_EN[0]
#define WED_WPDMA_RRO3_1_RX_D_CIDX_WR_CFG_CIDX_DLY_THRES_EN_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR (0x15010000 + 0x864)---

    ADDRESS[31..0]               - (RW) WPDMA Rx ring0 configuration register CIDX start address

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDRESS_MASK 0xFFFFFFFF                // ADDRESS[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H (0x15010000 + 0x868)---

    ADDRESS[7..0]                - (RW) Bit[39:32] of WPDMA Rx ring0 configuration register start address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H_ADDRESS_MASK 0x000000FF                // ADDRESS[7..0]
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_CIDX_ADDR_H_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0 (0x15010000 + 0x86C)---

    ENABLE[0]                    - (RW) Enable Range Check
    ERR[1]                       - (W1C) ERROR Address Data Valid
    ERR_CLR[2]                   - (RW) ERROR Address Data Clear
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_CLR_ADDR    WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_CLR_MASK    0x00000004                // ERR_CLR[2]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_CLR_SHFT    2
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_ADDR        WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_MASK        0x00000002                // ERR[1]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ERR_SHFT        1
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ENABLE_ADDR     WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ENABLE_MASK     0x00000001                // ENABLE[0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG0_ENABLE_SHFT     0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1 (0x15010000 + 0x870)---

    ADDR_MAX_31_0[31..0]         - (RW) Max Address Range [31:0]

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1_ADDR_MAX_31_0_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1_ADDR_MAX_31_0_MASK 0xFFFFFFFF                // ADDR_MAX_31_0[31..0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG1_ADDR_MAX_31_0_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2 (0x15010000 + 0x874)---

    ADDR_MIN_31_0[31..0]         - (RW) Min Address Range [31:0]

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2_ADDR_MIN_31_0_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2_ADDR_MIN_31_0_MASK 0xFFFFFFFF                // ADDR_MIN_31_0[31..0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG2_ADDR_MIN_31_0_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3 (0x15010000 + 0x878)---

    ADDR_MAX_47_32[15..0]        - (RW) Max Address Range [47:32]
    ADDR_MIN_47_32[31..16]       - (RW) Min Address Range [47:32]

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MIN_47_32_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MIN_47_32_MASK 0xFFFF0000                // ADDR_MIN_47_32[31..16]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MIN_47_32_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MAX_47_32_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MAX_47_32_MASK 0x0000FFFF                // ADDR_MAX_47_32[15..0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG3_ADDR_MAX_47_32_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4 (0x15010000 + 0x87C)---

    ERR_ADDR_31_0[31..0]         - (RO) ERROR Address Data [31:0]

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4_ERR_ADDR_31_0_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4_ERR_ADDR_31_0_MASK 0xFFFFFFFF                // ERR_ADDR_31_0[31..0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG4_ERR_ADDR_31_0_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5 (0x15010000 + 0x880)---

    ERR_ADDR_47_16[15..0]        - (RO) ERROR Address Data [47:32]
    ERR_LEN[19..16]              - (RO) ERROR Length Data
    RESERVED20[31..20]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_LEN_ADDR    WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_LEN_MASK    0x000F0000                // ERR_LEN[19..16]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_LEN_SHFT    16
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_ADDR_47_16_ADDR WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ADDR
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_ADDR_47_16_MASK 0x0000FFFF                // ERR_ADDR_47_16[15..0]
#define WED_WPDMA_RRO3_1_RX_D_AW_RAN_DBG5_ERR_ADDR_47_16_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_MAGIC_COUNT (0x15010000 + 0x884)---

    ENABLE[0]                    - (RW) Enable WPDMA RRO3.1 Rx Driver Magic Count Check
    OW[1]                        - (A0) Write 1 to overwrite the Magic count
    OW_VALUE[5..2]               - (RW) overwrite value to Magic Count when trigger ow
    MONITER[9..6]                - (RO) Moniter Magic Count
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_MONITER_ADDR         WED_WPDMA_RRO3_1_MAGIC_COUNT_ADDR
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_MONITER_MASK         0x000003C0                // MONITER[9..6]
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_MONITER_SHFT         6
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_VALUE_ADDR        WED_WPDMA_RRO3_1_MAGIC_COUNT_ADDR
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_VALUE_MASK        0x0000003C                // OW_VALUE[5..2]
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_VALUE_SHFT        2
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_ADDR              WED_WPDMA_RRO3_1_MAGIC_COUNT_ADDR
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_MASK              0x00000002                // OW[1]
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_OW_SHFT              1
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_ENABLE_ADDR          WED_WPDMA_RRO3_1_MAGIC_COUNT_ADDR
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_ENABLE_MASK          0x00000001                // ENABLE[0]
#define WED_WPDMA_RRO3_1_MAGIC_COUNT_ENABLE_SHFT          0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR (0x15010000 + 0x888)---

    ADDRESS[31..0]               - (RW) WPDMA Rx ring0 configuration register DIDX start address

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDRESS_MASK 0xFFFFFFFF                // ADDRESS[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H (0x15010000 + 0x88C)---

    ADDRESS[7..0]                - (RW) Bit[39:32] of WPDMA Rx ring0 configuration register start address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H_ADDRESS_ADDR WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H_ADDRESS_MASK 0x000000FF                // ADDRESS[7..0]
#define WED_WPDMA_RRO3_1_RX_D_RING0_CFG_DIDX_ADDR_H_ADDRESS_SHFT 0

/* =====================================================================================

  ---WED_WDMA_TX0_MIB_1 (0x15010000 + 0x890)---

    RD_CNT[31..0]                - (RC) FIFO read count

 =====================================================================================*/
#define WED_WDMA_TX0_MIB_1_RD_CNT_ADDR                    WED_WDMA_TX0_MIB_1_ADDR
#define WED_WDMA_TX0_MIB_1_RD_CNT_MASK                    0xFFFFFFFF                // RD_CNT[31..0]
#define WED_WDMA_TX0_MIB_1_RD_CNT_SHFT                    0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT (0x15010000 + 0x894)---

    all[31..0]                   - (RC) wpdma rro3p1 prefetch coherent count

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT_all_ADDR WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT_ADDR
#define WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT_all_MASK 0xFFFFFFFF                // all[31..0]
#define WED_WPDMA_RRO3_1_RX_D_PREF_MIB_COHERENT_CNT_all_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB (0x15010000 + 0x8B0)---

    DMAD_CNT[31..0]              - (RC) WPDMA Rx Data ring0 ring_info == 0 dmad counter (the counter will not be increased if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB_DMAD_CNT_ADDR WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB_DMAD_CNT_MASK 0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO0_MIB_DMAD_CNT_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB (0x15010000 + 0x8B4)---

    DMAD_CNT[31..0]              - (RC) WPDMA Rx Data ring0 ring_info == 1 dmad counter (the counter will not be increased if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB_DMAD_CNT_ADDR WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB_ADDR
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB_DMAD_CNT_MASK 0xFFFFFFFF                // DMAD_CNT[31..0]
#define WED_WPDMA_RRO3_1_RX_D_RX0_RINGINFO1_MIB_DMAD_CNT_SHFT 0

/* =====================================================================================

  ---WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX (0x15010000 + 0x8B8)---

    DRX_IDX0[15..0]              - (RW) The data to be overwrite into DRX_IDX1
    LOAD_DRX_IDX0[16]            - (RW) Write 1 to overwrite the Rx Driver DRX_IDX1
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_LOAD_DRX_IDX0_ADDR WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_LOAD_DRX_IDX0_MASK 0x00010000                // LOAD_DRX_IDX0[16]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_LOAD_DRX_IDX0_SHFT 16
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_DRX_IDX0_ADDR  WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_ADDR
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_DRX_IDX0_MASK  0x0000FFFF                // DRX_IDX0[15..0]
#define WED_WPDMA_RRO3_1_RX_D_LOAD_DRX_IDX_DRX_IDX0_SHFT  0

/* =====================================================================================

  ---WED_RRO_CTL (0x15010000 + 0x8FC)---

    RRO_VER[0]                   - (RW) RRO version select
    RESERVED1[31..1]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_RRO_CTL_RRO_VER_ADDR                          WED_RRO_CTL_ADDR
#define WED_RRO_CTL_RRO_VER_MASK                          0x00000001                // RRO_VER[0]
#define WED_RRO_CTL_RRO_VER_SHFT                          0

/* =====================================================================================

  ---WED_WDMA_RX0_BASE (0x15010000 + 0x900)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)

 =====================================================================================*/
#define WED_WDMA_RX0_BASE_PTR_ADDR                        WED_WDMA_RX0_BASE_ADDR
#define WED_WDMA_RX0_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define WED_WDMA_RX0_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---WED_WDMA_RX0_CNT (0x15010000 + 0x904)---

    MAX[15..0]                   - (RW) The maximum number of RXD count in RX Ring #0
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RX0_CNT_BASE_PTR_H_ADDR                  WED_WDMA_RX0_CNT_ADDR
#define WED_WDMA_RX0_CNT_BASE_PTR_H_MASK                  0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WDMA_RX0_CNT_BASE_PTR_H_SHFT                  16
#define WED_WDMA_RX0_CNT_MAX_ADDR                         WED_WDMA_RX0_CNT_ADDR
#define WED_WDMA_RX0_CNT_MAX_MASK                         0x0000FFFF                // MAX[15..0]
#define WED_WDMA_RX0_CNT_MAX_SHFT                         0

/* =====================================================================================

  ---WED_WDMA_RX0_CRX_IDX (0x15010000 + 0x908)---

    CRX_IDX[15..0]               - (RO) Point to the next RXD hardware driver agent wants to use, this index of WDMA is controlled by WED hardware.
    DRV_IDX[31..16]              - (RO) Point to the next RXD hardware driver agent wants to process of RXD Ring #0, this index is controlled by WED hardware.

 =====================================================================================*/
#define WED_WDMA_RX0_CRX_IDX_DRV_IDX_ADDR                 WED_WDMA_RX0_CRX_IDX_ADDR
#define WED_WDMA_RX0_CRX_IDX_DRV_IDX_MASK                 0xFFFF0000                // DRV_IDX[31..16]
#define WED_WDMA_RX0_CRX_IDX_DRV_IDX_SHFT                 16
#define WED_WDMA_RX0_CRX_IDX_CRX_IDX_ADDR                 WED_WDMA_RX0_CRX_IDX_ADDR
#define WED_WDMA_RX0_CRX_IDX_CRX_IDX_MASK                 0x0000FFFF                // CRX_IDX[15..0]
#define WED_WDMA_RX0_CRX_IDX_CRX_IDX_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_RX0_DRX_IDX (0x15010000 + 0x90c)---

    DRX_IDX_MIRO[15..0]          - (RO) Point to the next RXD DMA wants to use, this is a mirrored value of WDMA RX_DRX_IDX.
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RX0_DRX_IDX_DRX_IDX_MIRO_ADDR            WED_WDMA_RX0_DRX_IDX_ADDR
#define WED_WDMA_RX0_DRX_IDX_DRX_IDX_MIRO_MASK            0x0000FFFF                // DRX_IDX_MIRO[15..0]
#define WED_WDMA_RX0_DRX_IDX_DRX_IDX_MIRO_SHFT            0

/* =====================================================================================

  ---WED_WDMA_RX1_BASE (0x15010000 + 0x910)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #1 (4-DW aligned address)

 =====================================================================================*/
#define WED_WDMA_RX1_BASE_PTR_ADDR                        WED_WDMA_RX1_BASE_ADDR
#define WED_WDMA_RX1_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define WED_WDMA_RX1_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---WED_WDMA_RX1_CNT (0x15010000 + 0x914)---

    MAX[15..0]                   - (RW) The maximum number of RXD count in RX Ring #1
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of base address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RX1_CNT_BASE_PTR_H_ADDR                  WED_WDMA_RX1_CNT_ADDR
#define WED_WDMA_RX1_CNT_BASE_PTR_H_MASK                  0x00FF0000                // BASE_PTR_H[23..16]
#define WED_WDMA_RX1_CNT_BASE_PTR_H_SHFT                  16
#define WED_WDMA_RX1_CNT_MAX_ADDR                         WED_WDMA_RX1_CNT_ADDR
#define WED_WDMA_RX1_CNT_MAX_MASK                         0x0000FFFF                // MAX[15..0]
#define WED_WDMA_RX1_CNT_MAX_SHFT                         0

/* =====================================================================================

  ---WED_WDMA_RX1_CRX_IDX (0x15010000 + 0x918)---

    CRX_IDX[15..0]               - (RO) Point to the next RXD hardware driver agent wants to use, this index of WDMA is controlled by WED hardware.
    DRV_IDX[31..16]              - (RO) Point to the next RXD hardware driver agent wants to process of RXD Ring #1, this index is controlled by WED hardware.

 =====================================================================================*/
#define WED_WDMA_RX1_CRX_IDX_DRV_IDX_ADDR                 WED_WDMA_RX1_CRX_IDX_ADDR
#define WED_WDMA_RX1_CRX_IDX_DRV_IDX_MASK                 0xFFFF0000                // DRV_IDX[31..16]
#define WED_WDMA_RX1_CRX_IDX_DRV_IDX_SHFT                 16
#define WED_WDMA_RX1_CRX_IDX_CRX_IDX_ADDR                 WED_WDMA_RX1_CRX_IDX_ADDR
#define WED_WDMA_RX1_CRX_IDX_CRX_IDX_MASK                 0x0000FFFF                // CRX_IDX[15..0]
#define WED_WDMA_RX1_CRX_IDX_CRX_IDX_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_RX1_DRX_IDX (0x15010000 + 0x91c)---

    DRX_IDX_MIRO[15..0]          - (RO) Point to the next RXD DMA wants to use, this is a mirrored value of WDMA RX_DRX_IDX.
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RX1_DRX_IDX_DRX_IDX_MIRO_ADDR            WED_WDMA_RX1_DRX_IDX_ADDR
#define WED_WDMA_RX1_DRX_IDX_DRX_IDX_MIRO_MASK            0x0000FFFF                // DRX_IDX_MIRO[15..0]
#define WED_WDMA_RX1_DRX_IDX_DRX_IDX_MIRO_SHFT            0

/* =====================================================================================

  ---WED_WDMA_RX0_THRES_CFG (0x15010000 + 0x940)---

    WAIT_BM_CNT_MAX[15..0]       - (RW) When enable the dynamic skip feature for DMAD prepare phase. If Rx Driver cannot get new descriptor from Buffer Management (BM) module, Rx Driver will wait BM to prepare new descriptor until internal counter==wait_bm_cnt_max, then skip the DMAD prepare phase. Note that if wait_bm_cnt_max==0, then RxDriver will skip the phase immediately.
                                     unit: clock cycle or 20us(decided by CR setting)
    DRX_CRX_DISTANCE_THRES[31..16] - (RW) The distance tolerance threshold of this Ring's DRX and CRX. The Distance between DRX and CRX is defined as "distance=DRX_IDX-CRX_IDX". This affects the behavior of descriptor recycle function and descriptor prepare phase skip function.
                                     The threshold must always larger than 2 and lower than RING_MAX_CNT-2:
                                     2<Threshold<RING_MAX_CNT-2

 =====================================================================================*/
#define WED_WDMA_RX0_THRES_CFG_DRX_CRX_DISTANCE_THRES_ADDR WED_WDMA_RX0_THRES_CFG_ADDR
#define WED_WDMA_RX0_THRES_CFG_DRX_CRX_DISTANCE_THRES_MASK 0xFFFF0000                // DRX_CRX_DISTANCE_THRES[31..16]
#define WED_WDMA_RX0_THRES_CFG_DRX_CRX_DISTANCE_THRES_SHFT 16
#define WED_WDMA_RX0_THRES_CFG_WAIT_BM_CNT_MAX_ADDR       WED_WDMA_RX0_THRES_CFG_ADDR
#define WED_WDMA_RX0_THRES_CFG_WAIT_BM_CNT_MAX_MASK       0x0000FFFF                // WAIT_BM_CNT_MAX[15..0]
#define WED_WDMA_RX0_THRES_CFG_WAIT_BM_CNT_MAX_SHFT       0

/* =====================================================================================

  ---WED_WDMA_RX1_THRES_CFG (0x15010000 + 0x944)---

    WAIT_BM_CNT_MAX[15..0]       - (RW) When enable the dynamic skip feature for DMAD prepare phase. If Rx Driver cannot get new descriptor from Buffer Management(BM) module, Rx Driver will wait BM to prepare new descriptor until internal counter==wait_bm_cnt_max, then skip the DMAD prepare phase. Note that if wait_bm_cnt_max==0, then RxDriver will skip the phase immediately.
                                     unit: clock cycle or 20us(decided by CR setting)
    DRX_CRX_DISTANCE_THRES[31..16] - (RW) The distance tolerance threshold of this Ring's DRX and CRX. The Distance between DRX and CRX is defined as "distance=DRX_IDX-CRX_IDX". This affects the behavior of descriptor recycle function and descriptor prepare phase skip function.
                                     The threshold must always larger than 2 and lower than RING_MAX_CNT-2:
                                     2<Threshold<RING_MAX_CNT-2

 =====================================================================================*/
#define WED_WDMA_RX1_THRES_CFG_DRX_CRX_DISTANCE_THRES_ADDR WED_WDMA_RX1_THRES_CFG_ADDR
#define WED_WDMA_RX1_THRES_CFG_DRX_CRX_DISTANCE_THRES_MASK 0xFFFF0000                // DRX_CRX_DISTANCE_THRES[31..16]
#define WED_WDMA_RX1_THRES_CFG_DRX_CRX_DISTANCE_THRES_SHFT 16
#define WED_WDMA_RX1_THRES_CFG_WAIT_BM_CNT_MAX_ADDR       WED_WDMA_RX1_THRES_CFG_ADDR
#define WED_WDMA_RX1_THRES_CFG_WAIT_BM_CNT_MAX_MASK       0x0000FFFF                // WAIT_BM_CNT_MAX[15..0]
#define WED_WDMA_RX1_THRES_CFG_WAIT_BM_CNT_MAX_SHFT       0

/* =====================================================================================

  ---WED_WDMA_RX_PREF_CFG (0x15010000 + 0x950)---

    ENABLE[0]                    - (RW) Enable prefetch agent
    BUSY[1]                      - (RO) Prefetch agent busy
    DMAD_SIZE[2]                 - (RW) Size of each DMAD
    DDONE_CHK[3]                 - (RW) Enable DDONE Check.
                                     The DMAD with wrong DDONE and all DMAD afterward will be dropped. Prefetch agent will read DMAD with wrong DDONE later.
    DDONE_POLARITY[4]            - (RW) DDONE polarity of DDONE check
    WR_BND_4KB_BST[5]            - (RW) Enable AXI 4KB boundary write burst handler.
                                     Handle AXI write request which is crossing 4KB boundary by HW.
    RD_BND_4KB_BST[6]            - (RW) Enable AXI 4KB boundary read burst handler.
                                     Handle AXI read request which is crossing 4KB boundary by HW.
    RESERVED7[7]                 - (RO) Reserved bits
    BURST_SIZE[12..8]            - (RW) Prefetch Bus Burst Size
                                     Bus burst size. Unit is 8bytes. Maximum value is 16 (128bytes). Minimum value is size of 1 DMAD. Must not larger than 16 (128bytes) or less than size of 1 DMAD.
    R_RESP_ERR[13]               - (W1C) AXI read response error
    DMAD_ERR[14]                 - (W1C) Receive DMAD with wrong DDONE value
    RESERVED15[15]               - (RO) Reserved bits
    LOW_THRES[21..16]            - (RW) Prefetch FIFO Low Threshold.
                                     Ring which data byte in prefetch FIFO under low threshold has higher priority than others.
                                     Unit is 8bytes. Maximum is 32 (256bytes). Set the register to 0 will disable the low threshold mechanism. Recommend low threshold is size of 1 or 2 DMAD.
    RX0_SIDX_OW[22]              - (RW) Overwrite the ring 0 start index
    RX1_SIDX_OW[23]              - (RW) Overwrite the ring 1 start index
    RX0_SIDX_CLR[24]             - (RW) Clear the ring 0 start index
    RX1_SIDX_CLR[25]             - (RW) Clear the ring 1 start index
    DDONE2_EN[26]                - (RW) Enable DDONE2 HW
                                     Must not enable/disable DDONE2 HW during prefetch HW enabled
    DDONE2_BUSY[27]              - (RO) DDONE2 HW Busy
                                     Any data in DDONE2 HW
    DDONE2_CLR_MET[28]           - (RW) DDONE2 HW DW CNT Clear Method
    DDONE2_CLR[29]               - (A0) Clear DDONE2 HW
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_ADDR              WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_MASK              0x20000000                // DDONE2_CLR[29]
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_SHFT              29
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_MET_ADDR          WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_MET_MASK          0x10000000                // DDONE2_CLR_MET[28]
#define WED_WDMA_RX_PREF_CFG_DDONE2_CLR_MET_SHFT          28
#define WED_WDMA_RX_PREF_CFG_DDONE2_BUSY_ADDR             WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE2_BUSY_MASK             0x08000000                // DDONE2_BUSY[27]
#define WED_WDMA_RX_PREF_CFG_DDONE2_BUSY_SHFT             27
#define WED_WDMA_RX_PREF_CFG_DDONE2_EN_ADDR               WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE2_EN_MASK               0x04000000                // DDONE2_EN[26]
#define WED_WDMA_RX_PREF_CFG_DDONE2_EN_SHFT               26
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_CLR_ADDR            WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_CLR_MASK            0x02000000                // RX1_SIDX_CLR[25]
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_CLR_SHFT            25
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_CLR_ADDR            WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_CLR_MASK            0x01000000                // RX0_SIDX_CLR[24]
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_CLR_SHFT            24
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_OW_ADDR             WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_OW_MASK             0x00800000                // RX1_SIDX_OW[23]
#define WED_WDMA_RX_PREF_CFG_RX1_SIDX_OW_SHFT             23
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_OW_ADDR             WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_OW_MASK             0x00400000                // RX0_SIDX_OW[22]
#define WED_WDMA_RX_PREF_CFG_RX0_SIDX_OW_SHFT             22
#define WED_WDMA_RX_PREF_CFG_LOW_THRES_ADDR               WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_LOW_THRES_MASK               0x003F0000                // LOW_THRES[21..16]
#define WED_WDMA_RX_PREF_CFG_LOW_THRES_SHFT               16
#define WED_WDMA_RX_PREF_CFG_DMAD_ERR_ADDR                WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DMAD_ERR_MASK                0x00004000                // DMAD_ERR[14]
#define WED_WDMA_RX_PREF_CFG_DMAD_ERR_SHFT                14
#define WED_WDMA_RX_PREF_CFG_R_RESP_ERR_ADDR              WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_R_RESP_ERR_MASK              0x00002000                // R_RESP_ERR[13]
#define WED_WDMA_RX_PREF_CFG_R_RESP_ERR_SHFT              13
#define WED_WDMA_RX_PREF_CFG_BURST_SIZE_ADDR              WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_BURST_SIZE_MASK              0x00001F00                // BURST_SIZE[12..8]
#define WED_WDMA_RX_PREF_CFG_BURST_SIZE_SHFT              8
#define WED_WDMA_RX_PREF_CFG_RD_BND_4KB_BST_ADDR          WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_RD_BND_4KB_BST_MASK          0x00000040                // RD_BND_4KB_BST[6]
#define WED_WDMA_RX_PREF_CFG_RD_BND_4KB_BST_SHFT          6
#define WED_WDMA_RX_PREF_CFG_WR_BND_4KB_BST_ADDR          WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_WR_BND_4KB_BST_MASK          0x00000020                // WR_BND_4KB_BST[5]
#define WED_WDMA_RX_PREF_CFG_WR_BND_4KB_BST_SHFT          5
#define WED_WDMA_RX_PREF_CFG_DDONE_POLARITY_ADDR          WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE_POLARITY_MASK          0x00000010                // DDONE_POLARITY[4]
#define WED_WDMA_RX_PREF_CFG_DDONE_POLARITY_SHFT          4
#define WED_WDMA_RX_PREF_CFG_DDONE_CHK_ADDR               WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DDONE_CHK_MASK               0x00000008                // DDONE_CHK[3]
#define WED_WDMA_RX_PREF_CFG_DDONE_CHK_SHFT               3
#define WED_WDMA_RX_PREF_CFG_DMAD_SIZE_ADDR               WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_DMAD_SIZE_MASK               0x00000004                // DMAD_SIZE[2]
#define WED_WDMA_RX_PREF_CFG_DMAD_SIZE_SHFT               2
#define WED_WDMA_RX_PREF_CFG_BUSY_ADDR                    WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_BUSY_MASK                    0x00000002                // BUSY[1]
#define WED_WDMA_RX_PREF_CFG_BUSY_SHFT                    1
#define WED_WDMA_RX_PREF_CFG_ENABLE_ADDR                  WED_WDMA_RX_PREF_CFG_ADDR
#define WED_WDMA_RX_PREF_CFG_ENABLE_MASK                  0x00000001                // ENABLE[0]
#define WED_WDMA_RX_PREF_CFG_ENABLE_SHFT                  0

/* =====================================================================================

  ---WED_WDMA_RX_PREF_RX0_SIDX (0x15010000 + 0x954)---

    START_IDX[15..0]             - (RO) Point to the next DMAD that prefetch agent wants to read
    IDX_OW_VAL[31..16]           - (RW) The index will be overwrite by this register field

 =====================================================================================*/
#define WED_WDMA_RX_PREF_RX0_SIDX_IDX_OW_VAL_ADDR         WED_WDMA_RX_PREF_RX0_SIDX_ADDR
#define WED_WDMA_RX_PREF_RX0_SIDX_IDX_OW_VAL_MASK         0xFFFF0000                // IDX_OW_VAL[31..16]
#define WED_WDMA_RX_PREF_RX0_SIDX_IDX_OW_VAL_SHFT         16
#define WED_WDMA_RX_PREF_RX0_SIDX_START_IDX_ADDR          WED_WDMA_RX_PREF_RX0_SIDX_ADDR
#define WED_WDMA_RX_PREF_RX0_SIDX_START_IDX_MASK          0x0000FFFF                // START_IDX[15..0]
#define WED_WDMA_RX_PREF_RX0_SIDX_START_IDX_SHFT          0

/* =====================================================================================

  ---WED_WDMA_RX_PREF_RX1_SIDX (0x15010000 + 0x958)---

    START_IDX[15..0]             - (RO) Point to the next DMAD that prefetch agent wants to read
    IDX_OW_VAL[31..16]           - (RW) The index will be overwrite by this register field

 =====================================================================================*/
#define WED_WDMA_RX_PREF_RX1_SIDX_IDX_OW_VAL_ADDR         WED_WDMA_RX_PREF_RX1_SIDX_ADDR
#define WED_WDMA_RX_PREF_RX1_SIDX_IDX_OW_VAL_MASK         0xFFFF0000                // IDX_OW_VAL[31..16]
#define WED_WDMA_RX_PREF_RX1_SIDX_IDX_OW_VAL_SHFT         16
#define WED_WDMA_RX_PREF_RX1_SIDX_START_IDX_ADDR          WED_WDMA_RX_PREF_RX1_SIDX_ADDR
#define WED_WDMA_RX_PREF_RX1_SIDX_START_IDX_MASK          0x0000FFFF                // START_IDX[15..0]
#define WED_WDMA_RX_PREF_RX1_SIDX_START_IDX_SHFT          0

/* =====================================================================================

  ---WED_WDMA_RX_PREF_FIFO_CFG (0x15010000 + 0x95c)---

    RING0_CLEAR[0]               - (RW) Clear Ring0 FIFO
    RING0_FULL[1]                - (RO) Ring0 FIFO Full
    RING0_EMPTY[2]               - (RO) Ring0 FIFO Empty
    RESERVED3[3]                 - (RO) Reserved bits
    RING0_USED_CNT[9..4]         - (RO) Used Ring0 FIFO entry
    RING0_FREE_CNT[15..10]       - (RO) Free Ring0 FIFO entry
    RING1_CLEAR[16]              - (RW) Clear Ring1 FIFO
    RING1_FULL[17]               - (RO) Ring1 FIFO Full
    RING1_EMPTY[18]              - (RO) Ring1 FIFO Empty
    RESERVED19[19]               - (RO) Reserved bits
    RING1_USED_CNT[25..20]       - (RO) Used Ring1 FIFO entry
    RING1_FREE_CNT[31..26]       - (RO) Free Ring1 FIFO entry

 =====================================================================================*/
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FREE_CNT_ADDR     WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FREE_CNT_MASK     0xFC000000                // RING1_FREE_CNT[31..26]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FREE_CNT_SHFT     26
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_USED_CNT_ADDR     WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_USED_CNT_MASK     0x03F00000                // RING1_USED_CNT[25..20]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_USED_CNT_SHFT     20
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_EMPTY_ADDR        WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_EMPTY_MASK        0x00040000                // RING1_EMPTY[18]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_EMPTY_SHFT        18
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FULL_ADDR         WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FULL_MASK         0x00020000                // RING1_FULL[17]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_FULL_SHFT         17
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_CLEAR_ADDR        WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_CLEAR_MASK        0x00010000                // RING1_CLEAR[16]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING1_CLEAR_SHFT        16
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FREE_CNT_ADDR     WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FREE_CNT_MASK     0x0000FC00                // RING0_FREE_CNT[15..10]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FREE_CNT_SHFT     10
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_USED_CNT_ADDR     WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_USED_CNT_MASK     0x000003F0                // RING0_USED_CNT[9..4]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_USED_CNT_SHFT     4
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_EMPTY_ADDR        WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_EMPTY_MASK        0x00000004                // RING0_EMPTY[2]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_EMPTY_SHFT        2
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FULL_ADDR         WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FULL_MASK         0x00000002                // RING0_FULL[1]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_FULL_SHFT         1
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_CLEAR_ADDR        WED_WDMA_RX_PREF_FIFO_CFG_ADDR
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_CLEAR_MASK        0x00000001                // RING0_CLEAR[0]
#define WED_WDMA_RX_PREF_FIFO_CFG_RING0_CLEAR_SHFT        0

/* =====================================================================================

  ---WED_WDMA_RX_CIDX_WR_CFG (0x15010000 + 0x960)---

    CIDX_DLY_THRES_EN[0]         - (RW) Enable CIDX write delay  threshold
                                     Write CIDX when DIDX-CIDX < Threshold
    RESERVED1[1]                 - (RO) Reserved bits
    CIDX_DLY_CNT_CLR[3..2]       - (RW) Clear CIDX delay counter
    RESERVED4[7..4]              - (RO) Reserved bits
    CIDX_DLY_CNT_MAX[15..8]      - (RW) CIDX write delay max count
                                     If CIDX_DLY_CNT_MAX=0,  then write delay feature will be disabled.
    CIDX_DLY_THRES[31..16]       - (RW) CIDX write delay threshold
                                     If CIDX_DLY_THRES_EN=1 but the threshold=0, then write delay feature will be disabled

 =====================================================================================*/
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_ADDR       WED_WDMA_RX_CIDX_WR_CFG_ADDR
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_MASK       0xFFFF0000                // CIDX_DLY_THRES[31..16]
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_SHFT       16
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_ADDR     WED_WDMA_RX_CIDX_WR_CFG_ADDR
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_MASK     0x0000FF00                // CIDX_DLY_CNT_MAX[15..8]
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_MAX_SHFT     8
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_ADDR     WED_WDMA_RX_CIDX_WR_CFG_ADDR
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_MASK     0x0000000C                // CIDX_DLY_CNT_CLR[3..2]
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_CNT_CLR_SHFT     2
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_EN_ADDR    WED_WDMA_RX_CIDX_WR_CFG_ADDR
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_EN_MASK    0x00000001                // CIDX_DLY_THRES_EN[0]
#define WED_WDMA_RX_CIDX_WR_CFG_CIDX_DLY_THRES_EN_SHFT    0

/* =====================================================================================

  ---WED_WDMA_PREF_MIB_COHERENT_CNT (0x15010000 + 0x964)---

    ALL[31..0]                   - (RC) wdma pref coherent mib count

 =====================================================================================*/
#define WED_WDMA_PREF_MIB_COHERENT_CNT_ALL_ADDR           WED_WDMA_PREF_MIB_COHERENT_CNT_ADDR
#define WED_WDMA_PREF_MIB_COHERENT_CNT_ALL_MASK           0xFFFFFFFF                // ALL[31..0]
#define WED_WDMA_PREF_MIB_COHERENT_CNT_ALL_SHFT           0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG (0x15010000 + 0x970)---

    cidx_time_delay_en[0]        - (RW) delay interrupt, delay updating cpu index after a certain period of time.
                                     wdma_tx_drv will update cpu index when (delay time == cidx_time_delay_value), delay time will count when (cidx != didx)  and reset when wdma_tx_drv update cidx. The unit is 1us.
                                     Can be enabled together with cidx_accu_delay_en
    cidx_accu_delay_en[1]        - (RW) delay interrupt, update cpu index after accumulating a config value of cpu indexes. wdma tx drv will update cpu index when (accumlation cidx  == cidx_accu_delay_value), delay time will count when (cidx != didx)  and reset when wdma_tx_drv update cidx.
                                     Can be enabled together with cidx_time_delay_en
    cidx_time_delay_run[2]       - (RO) time delay counter status
                                     0: stop
                                     1: running
    RESERVED3[31..3]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_run_ADDR WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_ADDR
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_run_MASK 0x00000004                // cidx_time_delay_run[2]
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_run_SHFT 2
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_accu_delay_en_ADDR WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_ADDR
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_accu_delay_en_MASK 0x00000002                // cidx_accu_delay_en[1]
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_accu_delay_en_SHFT 1
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_en_ADDR WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_ADDR
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_en_MASK 0x00000001                // cidx_time_delay_en[0]
#define WED_WDMA_TX_DRV_DELAY_INTERRUPT_CFG_cidx_time_delay_en_SHFT 0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_TIME_DELAY_CFG (0x15010000 + 0x974)---

    cidx_time_delay_value[31..0] - (RW) The unit is 1us.
                                     wdma_tx_drv will update cpu index when (delay time == cidx_time_delay_value), delay time will count when (cidx != didx)  and reset when wdma_tx_drv update cidx.

 =====================================================================================*/
#define WED_WDMA_TX_DRV_TIME_DELAY_CFG_cidx_time_delay_value_ADDR WED_WDMA_TX_DRV_TIME_DELAY_CFG_ADDR
#define WED_WDMA_TX_DRV_TIME_DELAY_CFG_cidx_time_delay_value_MASK 0xFFFFFFFF                // cidx_time_delay_value[31..0]
#define WED_WDMA_TX_DRV_TIME_DELAY_CFG_cidx_time_delay_value_SHFT 0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_TIME_DELAY_MON (0x15010000 + 0x978)---

    cidx_time_delay_count[31..0] - (RO) time delay counter monitor

 =====================================================================================*/
#define WED_WDMA_TX_DRV_TIME_DELAY_MON_cidx_time_delay_count_ADDR WED_WDMA_TX_DRV_TIME_DELAY_MON_ADDR
#define WED_WDMA_TX_DRV_TIME_DELAY_MON_cidx_time_delay_count_MASK 0xFFFFFFFF                // cidx_time_delay_count[31..0]
#define WED_WDMA_TX_DRV_TIME_DELAY_MON_cidx_time_delay_count_SHFT 0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_ACCU_DELAY_CFG (0x15010000 + 0x97c)---

    cidx_accu_delay_value[31..0] - (RW) wdma tx drv will update cpu index when (accumlation cidx  == cidx_accu_delay_value), delay time will count when (cidx != didx)  and reset when wdma_tx_drv update cidx.

 =====================================================================================*/
#define WED_WDMA_TX_DRV_ACCU_DELAY_CFG_cidx_accu_delay_value_ADDR WED_WDMA_TX_DRV_ACCU_DELAY_CFG_ADDR
#define WED_WDMA_TX_DRV_ACCU_DELAY_CFG_cidx_accu_delay_value_MASK 0xFFFFFFFF                // cidx_accu_delay_value[31..0]
#define WED_WDMA_TX_DRV_ACCU_DELAY_CFG_cidx_accu_delay_value_SHFT 0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_ACCU_DELAY_MON (0x15010000 + 0x980)---

    cidx_accu_delay_count[31..0] - (RO) accumulation delay count monitor

 =====================================================================================*/
#define WED_WDMA_TX_DRV_ACCU_DELAY_MON_cidx_accu_delay_count_ADDR WED_WDMA_TX_DRV_ACCU_DELAY_MON_ADDR
#define WED_WDMA_TX_DRV_ACCU_DELAY_MON_cidx_accu_delay_count_MASK 0xFFFFFFFF                // cidx_accu_delay_count[31..0]
#define WED_WDMA_TX_DRV_ACCU_DELAY_MON_cidx_accu_delay_count_SHFT 0

/* =====================================================================================

  ---WED_WDMA_INFO (0x15010000 + 0xa00)---

    TX_RING_NUM[7..0]            - (RO) Tx ring number
    RX_RING_NUM[15..8]           - (RO) Rx ring number
    BASE_PTR_WIDTH[23..16]       - (RO) Base pointer width, x
                                     base_addr[31:32-x] is shared with all ring base address. Only ring #0 base address [31:32-x] field Is write-able.
                                     [Note]: "0" means no bit of base address is shared.
    INDEX_WIDTH[27..24]          - (RO) Point to the next RXD CPU wants to use
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_INFO_INDEX_WIDTH_ADDR                    WED_WDMA_INFO_ADDR
#define WED_WDMA_INFO_INDEX_WIDTH_MASK                    0x0F000000                // INDEX_WIDTH[27..24]
#define WED_WDMA_INFO_INDEX_WIDTH_SHFT                    24
#define WED_WDMA_INFO_BASE_PTR_WIDTH_ADDR                 WED_WDMA_INFO_ADDR
#define WED_WDMA_INFO_BASE_PTR_WIDTH_MASK                 0x00FF0000                // BASE_PTR_WIDTH[23..16]
#define WED_WDMA_INFO_BASE_PTR_WIDTH_SHFT                 16
#define WED_WDMA_INFO_RX_RING_NUM_ADDR                    WED_WDMA_INFO_ADDR
#define WED_WDMA_INFO_RX_RING_NUM_MASK                    0x0000FF00                // RX_RING_NUM[15..8]
#define WED_WDMA_INFO_RX_RING_NUM_SHFT                    8
#define WED_WDMA_INFO_TX_RING_NUM_ADDR                    WED_WDMA_INFO_ADDR
#define WED_WDMA_INFO_TX_RING_NUM_MASK                    0x000000FF                // TX_RING_NUM[7..0]
#define WED_WDMA_INFO_TX_RING_NUM_SHFT                    0

/* =====================================================================================

  ---WED_WDMA_GLO_CFG (0x15010000 + 0xa04)---

    TX_DRV_EN[0]                 - (RW) Tx Driver Module Enable
    TX_DDONE_CHK[1]              - (RW) TX DDONE check
    RX_DRV_EN[2]                 - (RW) Rx Driver Module Enable
    RX_DRV_BUSY[3]               - (RO) Rx Driver Module Busy indicator
    RECYCLE_DESCRIPTOR_USE_TX_BM_EN[4] - (RW) Normal flow, set 1'd1.
                                     SER flow, set 1'd0, will drop all dmad.

                                     In SER flow need to "drop" all dmad, so can't use tx_bm buffer.
    RESERVED5[6..5]              - (RO) Reserved bits
    TX_WR_DONE_SEL[7]            - (RW) Tx Driver Module AXI write done criteria selection, set 1 is invaid
    TX_DDONE_CHK_LAST[8]         - (RW) TX DDONE check for last entry only
    LS_CHK_BYP[9]                - (RW) Bypass RX DMAD LS Field check
    RXDRV_DDONE2_CHK[10]         - (RW) RXDRV check DDONE2 field
                                     Enable to check DDONE2 field. This does NOT affect prefetch HW, which has another mechanism to check DDONE2.
    TX_DRV_BURST_CFG[12..11]     - (RW) TX DRV burst length
    RXDRV_DISABLED_FSM_AUTO_IDLE[13] - (RW) When RxDriver is disabled, RX Driver Finite State Machine return to IDLE automatically as possible as it can.(Even though the operation of Rx Driver module is not completed yet)
    DISABLE_IRQ_HANDLE_PROC[14]  - (RW) Disable the interrupt handle process, which is the main function of RxDriver. If the function is disabled, RxDriver  module will not accept and handle any Rx Done interrupt provided by WED WDMA interrupt agent. Usually, the function should not be disabled.
    DISABLE_DMAD_HANDLE_PROC[15] - (RW) Disable the basic DMAD handle process, which is the main function of RxDriver. If the function is disabled, RxDriver  module will not manage the Frame Engine WDMA RxRing. Usually, the function should not be disabled.
    WCOMPLETE_SEL[16]            - (RW) select the AXI wready or bvalid as write complete signal for Rx Driver module
    INIT_PHASE_ENABLING_RXDMA_BYPASS[17] - (RW) When RxDriver module finished the WDMA RxRing initial sequence, RxDriver module will turn on the WDMA Rx Enable register automatically. If this register is set to 1,  that RxDriver module will not enable WDMA Rx automatically after WDMA RxRing initial sequence has finished.
    INIT_PHASE_BYPASS[18]        - (RW) RX Driver module will initial WDMA RxRing when RxDriver HW module is enabled. If WDMA RxRing is initialized by software, set this register to 1 to disable the HW initial function.
    FSM_RETURN_IDLE[19]          - (RW) Force RX Driver Finite State Machine return to IDLE as possible as it can. (Will not returned to IDLE when current state is related to bus access)
    WAIT_COHERENT_EN[20]         - (RW) Enable the coherent wait and reread descriptor feature while Rx Driver coherent problem occurs(RxRing DDONE=0). If enabled, Rx Driver will wait until coherent wait counter reach the maximum count value (configurable) and then  reread the descriptor.
    AXI_W_AFTER_AW_EN[21]        - (RW) Enable the Rx Driver module AXI W channel scheduler to postpone the W channel access until AW channel has completed. Enable this feature may solve some AXI access problem, but also degrade the AXI write performance. Note that not all AXI Slave support this protocol. If AXI master access the slave device which is NOT support this feature may cause bus hang.
    IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE[22] - (RW) When Rx Driver descriptor supply when idle is enabled, Rx Driver will write 1 descriptor at a time. This feature should be ALWAYS ENABLED.
    IDLE_STATE_DMAD_SUPPLY_EN[23] - (RW) Enable descriptor supply when Rx Driver is idle and CRX_IDX!=(DRV_IDX-1). When RxDrvire is idle and the CRX_IDX!=(DRV_IDX-1), RxDriver will trying to prepare new descriptor for WDMA RxRing, in other words, trying to increase CRX_IDX until CRX_IDX==(DRV_IDX-1). This feature must be enabled when the RxDriver dynamic skip DMAD prepare feature is also enabled.
    DYNAMIC_SKIP_DMAD_PREPARE[24] - (RW) Enable dynamic skip for DMAD prepare phase. When this function is enabled, and RxDriver cannot get new descriptor from Buffer Management (decided by HW counter). RxDriver will skip the process and not prepare new descriptor for RxRing. In other words, the CRX_IDX will not increase and causing CRX_IDX!=(DRV_IDX-1). If (DRX_IDX-CRX_IDX) > distance tolerance threshold, then the dynamic skip DMAD feature will be ignored and RxDriver will always wait BM to provide new descriptor, thus prevent the dynamic skip feature let CRX==DRX and WDMA RxRing stop. When this function enabled, make sure that RxDriver will supply new descriptor when RxDriver is idle (enable the related function).
    DYNAMIC_DMAD_RECYCLE[25]     - (RW) Enable dynamic descriptor recycling when RxDriver cannot get new descriptor from Buffer Management, and (DRX_IDX-CRX_IDX) > threshold. This prevent DRX catch up CRX which will cause WDMA RxRing stop. Note that when descriptor has recycled, also means that the packet is dropped. When this function enabled, the distance tolerance threshold between CRX_IDX and DRX_IDX must has proper setting.
    RST_INIT_COMPLETE_FLAG[26]   - (RW) Reset the Rx Driver module initialize completed flag. So that when Rx Driver is enabled and FSM is IDLE state, it will initialize DMA again. Before set this register to 1, make sure RxDriver is not enabled and not busy.
    DYNAMIC_SKIP_DMAD_CNT_SRC[27] - (RW) Decides the dynamic skip counter reference signal source for HW counter which providing the information that how long BM cannot provide DMAD to RxDriver. If the register set to 0, the counter will counting with RxDriver module clk. If the register is set to 1, the counter will counting with 20us pulse signal.
    DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD[28] - (RW) When dynamic_skip_dmad_prepare is enabled and dynamic_dmad_recycle is disabled, (1)if recycle threshold is reached and this register is set to 1, then RxDriver will wait until BM can provide new DMAD; (2)if recycle threshold is reached and this register is set to 0, then RxDriver will wait BM for new DMAD until HW timer reach timer threshold
    TX_DRV_DTX_DLY_DIS[29]       - (RW) When enabled, the dtx request will be delayed a lot
    RXDRV_CLKGATE_BYP[30]        - (RW) Rx Driver Module clock gating Bypass
    CSR_IDLE_2_CIDX_INCREASE_EN[31] - (RW) The enable of C-index increase if in IDLE state with free buffer
                                     State = IDLE  to State GET_NEW_RX_DESC

 =====================================================================================*/
#define WED_WDMA_GLO_CFG_CSR_IDLE_2_CIDX_INCREASE_EN_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_CSR_IDLE_2_CIDX_INCREASE_EN_MASK 0x80000000                // CSR_IDLE_2_CIDX_INCREASE_EN[31]
#define WED_WDMA_GLO_CFG_CSR_IDLE_2_CIDX_INCREASE_EN_SHFT 31
#define WED_WDMA_GLO_CFG_RXDRV_CLKGATE_BYP_ADDR           WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RXDRV_CLKGATE_BYP_MASK           0x40000000                // RXDRV_CLKGATE_BYP[30]
#define WED_WDMA_GLO_CFG_RXDRV_CLKGATE_BYP_SHFT           30
#define WED_WDMA_GLO_CFG_TX_DRV_DTX_DLY_DIS_ADDR          WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_DRV_DTX_DLY_DIS_MASK          0x20000000                // TX_DRV_DTX_DLY_DIS[29]
#define WED_WDMA_GLO_CFG_TX_DRV_DTX_DLY_DIS_SHFT          29
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_MASK 0x10000000                // DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD[28]
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_THRES_REACH_METHOD_SHFT 28
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_CNT_SRC_ADDR   WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_CNT_SRC_MASK   0x08000000                // DYNAMIC_SKIP_DMAD_CNT_SRC[27]
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_CNT_SRC_SHFT   27
#define WED_WDMA_GLO_CFG_RST_INIT_COMPLETE_FLAG_ADDR      WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RST_INIT_COMPLETE_FLAG_MASK      0x04000000                // RST_INIT_COMPLETE_FLAG[26]
#define WED_WDMA_GLO_CFG_RST_INIT_COMPLETE_FLAG_SHFT      26
#define WED_WDMA_GLO_CFG_DYNAMIC_DMAD_RECYCLE_ADDR        WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DYNAMIC_DMAD_RECYCLE_MASK        0x02000000                // DYNAMIC_DMAD_RECYCLE[25]
#define WED_WDMA_GLO_CFG_DYNAMIC_DMAD_RECYCLE_SHFT        25
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_ADDR   WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_MASK   0x01000000                // DYNAMIC_SKIP_DMAD_PREPARE[24]
#define WED_WDMA_GLO_CFG_DYNAMIC_SKIP_DMAD_PREPARE_SHFT   24
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_EN_ADDR   WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_EN_MASK   0x00800000                // IDLE_STATE_DMAD_SUPPLY_EN[23]
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_EN_SHFT   23
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE_MASK 0x00400000                // IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE[22]
#define WED_WDMA_GLO_CFG_IDLE_STATE_DMAD_SUPPLY_SINGLE_DMAD_WRITE_SHFT 22
#define WED_WDMA_GLO_CFG_AXI_W_AFTER_AW_EN_ADDR           WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_AXI_W_AFTER_AW_EN_MASK           0x00200000                // AXI_W_AFTER_AW_EN[21]
#define WED_WDMA_GLO_CFG_AXI_W_AFTER_AW_EN_SHFT           21
#define WED_WDMA_GLO_CFG_WAIT_COHERENT_EN_ADDR            WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_WAIT_COHERENT_EN_MASK            0x00100000                // WAIT_COHERENT_EN[20]
#define WED_WDMA_GLO_CFG_WAIT_COHERENT_EN_SHFT            20
#define WED_WDMA_GLO_CFG_FSM_RETURN_IDLE_ADDR             WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_FSM_RETURN_IDLE_MASK             0x00080000                // FSM_RETURN_IDLE[19]
#define WED_WDMA_GLO_CFG_FSM_RETURN_IDLE_SHFT             19
#define WED_WDMA_GLO_CFG_INIT_PHASE_BYPASS_ADDR           WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_INIT_PHASE_BYPASS_MASK           0x00040000                // INIT_PHASE_BYPASS[18]
#define WED_WDMA_GLO_CFG_INIT_PHASE_BYPASS_SHFT           18
#define WED_WDMA_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_MASK 0x00020000                // INIT_PHASE_ENABLING_RXDMA_BYPASS[17]
#define WED_WDMA_GLO_CFG_INIT_PHASE_ENABLING_RXDMA_BYPASS_SHFT 17
#define WED_WDMA_GLO_CFG_WCOMPLETE_SEL_ADDR               WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_WCOMPLETE_SEL_MASK               0x00010000                // WCOMPLETE_SEL[16]
#define WED_WDMA_GLO_CFG_WCOMPLETE_SEL_SHFT               16
#define WED_WDMA_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_ADDR    WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_MASK    0x00008000                // DISABLE_DMAD_HANDLE_PROC[15]
#define WED_WDMA_GLO_CFG_DISABLE_DMAD_HANDLE_PROC_SHFT    15
#define WED_WDMA_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_ADDR     WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_MASK     0x00004000                // DISABLE_IRQ_HANDLE_PROC[14]
#define WED_WDMA_GLO_CFG_DISABLE_IRQ_HANDLE_PROC_SHFT     14
#define WED_WDMA_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_MASK 0x00002000                // RXDRV_DISABLED_FSM_AUTO_IDLE[13]
#define WED_WDMA_GLO_CFG_RXDRV_DISABLED_FSM_AUTO_IDLE_SHFT 13
#define WED_WDMA_GLO_CFG_TX_DRV_BURST_CFG_ADDR            WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_DRV_BURST_CFG_MASK            0x00001800                // TX_DRV_BURST_CFG[12..11]
#define WED_WDMA_GLO_CFG_TX_DRV_BURST_CFG_SHFT            11
#define WED_WDMA_GLO_CFG_RXDRV_DDONE2_CHK_ADDR            WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RXDRV_DDONE2_CHK_MASK            0x00000400                // RXDRV_DDONE2_CHK[10]
#define WED_WDMA_GLO_CFG_RXDRV_DDONE2_CHK_SHFT            10
#define WED_WDMA_GLO_CFG_LS_CHK_BYP_ADDR                  WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_LS_CHK_BYP_MASK                  0x00000200                // LS_CHK_BYP[9]
#define WED_WDMA_GLO_CFG_LS_CHK_BYP_SHFT                  9
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_LAST_ADDR           WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_LAST_MASK           0x00000100                // TX_DDONE_CHK_LAST[8]
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_LAST_SHFT           8
#define WED_WDMA_GLO_CFG_TX_WR_DONE_SEL_ADDR              WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_WR_DONE_SEL_MASK              0x00000080                // TX_WR_DONE_SEL[7]
#define WED_WDMA_GLO_CFG_TX_WR_DONE_SEL_SHFT              7
#define WED_WDMA_GLO_CFG_RECYCLE_DESCRIPTOR_USE_TX_BM_EN_ADDR WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RECYCLE_DESCRIPTOR_USE_TX_BM_EN_MASK 0x00000010                // RECYCLE_DESCRIPTOR_USE_TX_BM_EN[4]
#define WED_WDMA_GLO_CFG_RECYCLE_DESCRIPTOR_USE_TX_BM_EN_SHFT 4
#define WED_WDMA_GLO_CFG_RX_DRV_BUSY_ADDR                 WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RX_DRV_BUSY_MASK                 0x00000008                // RX_DRV_BUSY[3]
#define WED_WDMA_GLO_CFG_RX_DRV_BUSY_SHFT                 3
#define WED_WDMA_GLO_CFG_RX_DRV_EN_ADDR                   WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_RX_DRV_EN_MASK                   0x00000004                // RX_DRV_EN[2]
#define WED_WDMA_GLO_CFG_RX_DRV_EN_SHFT                   2
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_ADDR                WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_MASK                0x00000002                // TX_DDONE_CHK[1]
#define WED_WDMA_GLO_CFG_TX_DDONE_CHK_SHFT                1
#define WED_WDMA_GLO_CFG_TX_DRV_EN_ADDR                   WED_WDMA_GLO_CFG_ADDR
#define WED_WDMA_GLO_CFG_TX_DRV_EN_MASK                   0x00000001                // TX_DRV_EN[0]
#define WED_WDMA_GLO_CFG_TX_DRV_EN_SHFT                   0

/* =====================================================================================

  ---WED_WDMA_RST_IDX (0x15010000 + 0xa08)---

    RESERVED0[14..0]             - (RO) Reserved bits
    DTX_IDX0[15]                 - (RW) Write 1 to reset WED_WDMA_TX_DRV DTX_IDX0 to 0
    CRX_IDX0[16]                 - (RW) Write 1 to reset WED_WDMA_RX_DRV CRX_IDX0 to 0
    CRX_IDX1[17]                 - (RW) Write 1 to reset WED_WDMA_RX_DRV CRX_IDX1 to 0
    CTX_IDX0[18]                 - (RW) Write 1 to reset WED_WDMA_TX_DRV CTX_IDX0 to 0
    CTX_IDX1[19]                 - (RW) Write 1 to reset WED_WDMA_TX_DRV CTX_IDX1 to 0
                                     Not used, WED_WDMA_TX_DRV not has ring#1
    DRX_IDX_ALL[20]              - (RW) Write 1 to reset WED_WDMA_RX_DRV DRX_IDX0 and DRX_IDX1 to 0
    RESERVED21[23..21]           - (RO) Reserved bits
    DRV_IDX0[24]                 - (RW) Write 1 to reset WED_WDMA_RX_DRV DRV_IDX0 to 0
    DRV_IDX1[25]                 - (RW) These registers are used to reset the index of WDMA Driver module inside of WED, not reset the index logic on WDMA
                                     Write 1 to reset WED_WDMA_RX_DRV DRV_IDX1 to 0
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_RST_IDX_DRV_IDX1_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_DRV_IDX1_MASK                    0x02000000                // DRV_IDX1[25]
#define WED_WDMA_RST_IDX_DRV_IDX1_SHFT                    25
#define WED_WDMA_RST_IDX_DRV_IDX0_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_DRV_IDX0_MASK                    0x01000000                // DRV_IDX0[24]
#define WED_WDMA_RST_IDX_DRV_IDX0_SHFT                    24
#define WED_WDMA_RST_IDX_DRX_IDX_ALL_ADDR                 WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_DRX_IDX_ALL_MASK                 0x00100000                // DRX_IDX_ALL[20]
#define WED_WDMA_RST_IDX_DRX_IDX_ALL_SHFT                 20
#define WED_WDMA_RST_IDX_CTX_IDX1_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_CTX_IDX1_MASK                    0x00080000                // CTX_IDX1[19]
#define WED_WDMA_RST_IDX_CTX_IDX1_SHFT                    19
#define WED_WDMA_RST_IDX_CTX_IDX0_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_CTX_IDX0_MASK                    0x00040000                // CTX_IDX0[18]
#define WED_WDMA_RST_IDX_CTX_IDX0_SHFT                    18
#define WED_WDMA_RST_IDX_CRX_IDX1_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_CRX_IDX1_MASK                    0x00020000                // CRX_IDX1[17]
#define WED_WDMA_RST_IDX_CRX_IDX1_SHFT                    17
#define WED_WDMA_RST_IDX_CRX_IDX0_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_CRX_IDX0_MASK                    0x00010000                // CRX_IDX0[16]
#define WED_WDMA_RST_IDX_CRX_IDX0_SHFT                    16
#define WED_WDMA_RST_IDX_DTX_IDX0_ADDR                    WED_WDMA_RST_IDX_ADDR
#define WED_WDMA_RST_IDX_DTX_IDX0_MASK                    0x00008000                // DTX_IDX0[15]
#define WED_WDMA_RST_IDX_DTX_IDX0_SHFT                    15

/* =====================================================================================

  ---WED_WDMA_LOAD_IDX (0x15010000 + 0xa0c)---

    DRV_IDX0[0]                  - (RW) Write 1 to overwrite the Rx Driver DRV_IDX0
    DRV_IDX1[1]                  - (RW) Write 1 to overwrite the Rx Driver DRV_IDX1
    RESERVED2[7..2]              - (RO) Reserved bits
    CRX_IDX0[8]                  - (RW) Write 1 to overwrite the Rx Driver CRX_IDX0
    CRX_IDX1[9]                  - (RW) Write 1 to overwrite the Rx Driver CRX_IDX1
    RESERVED10[31..10]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_LOAD_IDX_CRX_IDX1_ADDR                   WED_WDMA_LOAD_IDX_ADDR
#define WED_WDMA_LOAD_IDX_CRX_IDX1_MASK                   0x00000200                // CRX_IDX1[9]
#define WED_WDMA_LOAD_IDX_CRX_IDX1_SHFT                   9
#define WED_WDMA_LOAD_IDX_CRX_IDX0_ADDR                   WED_WDMA_LOAD_IDX_ADDR
#define WED_WDMA_LOAD_IDX_CRX_IDX0_MASK                   0x00000100                // CRX_IDX0[8]
#define WED_WDMA_LOAD_IDX_CRX_IDX0_SHFT                   8
#define WED_WDMA_LOAD_IDX_DRV_IDX1_ADDR                   WED_WDMA_LOAD_IDX_ADDR
#define WED_WDMA_LOAD_IDX_DRV_IDX1_MASK                   0x00000002                // DRV_IDX1[1]
#define WED_WDMA_LOAD_IDX_DRV_IDX1_SHFT                   1
#define WED_WDMA_LOAD_IDX_DRV_IDX0_ADDR                   WED_WDMA_LOAD_IDX_ADDR
#define WED_WDMA_LOAD_IDX_DRV_IDX0_MASK                   0x00000001                // DRV_IDX0[0]
#define WED_WDMA_LOAD_IDX_DRV_IDX0_SHFT                   0

/* =====================================================================================

  ---WED_WDMA_LOAD_DRV_IDX (0x15010000 + 0xa10)---

    DRV_IDX0[15..0]              - (RW) The data to be overwrite into DRV_IDX0
    DRV_IDX1[31..16]             - (RW) The data to be overwrite into DRV_IDX1

 =====================================================================================*/
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX1_ADDR               WED_WDMA_LOAD_DRV_IDX_ADDR
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX1_MASK               0xFFFF0000                // DRV_IDX1[31..16]
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX1_SHFT               16
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX0_ADDR               WED_WDMA_LOAD_DRV_IDX_ADDR
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX0_MASK               0x0000FFFF                // DRV_IDX0[15..0]
#define WED_WDMA_LOAD_DRV_IDX_DRV_IDX0_SHFT               0

/* =====================================================================================

  ---WED_WDMA_LOAD_CRX_IDX (0x15010000 + 0xa14)---

    CRX_IDX0[15..0]              - (RW) The data to be overwrite into CRX_IDX0
    CRX_IDX1[31..16]             - (RW) The data to be overwrite into CRX_IDX1

 =====================================================================================*/
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX1_ADDR               WED_WDMA_LOAD_CRX_IDX_ADDR
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX1_MASK               0xFFFF0000                // CRX_IDX1[31..16]
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX1_SHFT               16
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX0_ADDR               WED_WDMA_LOAD_CRX_IDX_ADDR
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX0_MASK               0x0000FFFF                // CRX_IDX0[15..0]
#define WED_WDMA_LOAD_CRX_IDX_CRX_IDX0_SHFT               0

/* =====================================================================================

  ---WED_WDMA_SPR (0x15010000 + 0xa1c)---

    RSV_REG[31..0]               - (RW) Spare reserved registers

 =====================================================================================*/
#define WED_WDMA_SPR_RSV_REG_ADDR                         WED_WDMA_SPR_ADDR
#define WED_WDMA_SPR_RSV_REG_MASK                         0xFFFFFFFF                // RSV_REG[31..0]
#define WED_WDMA_SPR_RSV_REG_SHFT                         0

/* =====================================================================================

  ---WED_WDMA_INT_STA_REC (0x15010000 + 0xa20)---

    TX_DONE0[0]                  - (RO) Tx ring #0 packet transmit interrupt
    TX_DONE1[1]                  - (RO) Tx ring #1 packet transmit interrupt
    TX_DONE2[2]                  - (RO) Tx ring #2 packet transmit interrupt
    TX_DONE3[3]                  - (RO) Tx ring #3 packet transmit interrupt
    RESERVED4[15..4]             - (RO) Reserved bits
    RX_DONE0[16]                 - (RO) Rx ring #0 packet receive interrupt
    RX_DONE1[17]                 - (RO) Rx ring #1 packet receive interrupt
    RESERVED18[27..18]           - (RO) Reserved bits
    TX_DLY_INT[28]               - (RO) Summary of the whole WDMA Tx related interrupts.
    TX_COHERENT[29]              - (RO) TX_DMA finds data coherent event while checking DDONE bit.
    RX_DLY_INT[30]               - (RO) Summary of the whole WDMA Rx related interrupts.
    RX_COHERENT[31]              - (RO) These 32bit registers record the last WED read original WDMA_INT_STA value from WDMA
                                     RX_DMA finds data coherent event while checking DDONE bit.

 =====================================================================================*/
#define WED_WDMA_INT_STA_REC_RX_COHERENT_ADDR             WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_RX_COHERENT_MASK             0x80000000                // RX_COHERENT[31]
#define WED_WDMA_INT_STA_REC_RX_COHERENT_SHFT             31
#define WED_WDMA_INT_STA_REC_RX_DLY_INT_ADDR              WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_RX_DLY_INT_MASK              0x40000000                // RX_DLY_INT[30]
#define WED_WDMA_INT_STA_REC_RX_DLY_INT_SHFT              30
#define WED_WDMA_INT_STA_REC_TX_COHERENT_ADDR             WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_COHERENT_MASK             0x20000000                // TX_COHERENT[29]
#define WED_WDMA_INT_STA_REC_TX_COHERENT_SHFT             29
#define WED_WDMA_INT_STA_REC_TX_DLY_INT_ADDR              WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_DLY_INT_MASK              0x10000000                // TX_DLY_INT[28]
#define WED_WDMA_INT_STA_REC_TX_DLY_INT_SHFT              28
#define WED_WDMA_INT_STA_REC_RX_DONE1_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_RX_DONE1_MASK                0x00020000                // RX_DONE1[17]
#define WED_WDMA_INT_STA_REC_RX_DONE1_SHFT                17
#define WED_WDMA_INT_STA_REC_RX_DONE0_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_RX_DONE0_MASK                0x00010000                // RX_DONE0[16]
#define WED_WDMA_INT_STA_REC_RX_DONE0_SHFT                16
#define WED_WDMA_INT_STA_REC_TX_DONE3_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_DONE3_MASK                0x00000008                // TX_DONE3[3]
#define WED_WDMA_INT_STA_REC_TX_DONE3_SHFT                3
#define WED_WDMA_INT_STA_REC_TX_DONE2_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_DONE2_MASK                0x00000004                // TX_DONE2[2]
#define WED_WDMA_INT_STA_REC_TX_DONE2_SHFT                2
#define WED_WDMA_INT_STA_REC_TX_DONE1_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_DONE1_MASK                0x00000002                // TX_DONE1[1]
#define WED_WDMA_INT_STA_REC_TX_DONE1_SHFT                1
#define WED_WDMA_INT_STA_REC_TX_DONE0_ADDR                WED_WDMA_INT_STA_REC_ADDR
#define WED_WDMA_INT_STA_REC_TX_DONE0_MASK                0x00000001                // TX_DONE0[0]
#define WED_WDMA_INT_STA_REC_TX_DONE0_SHFT                0

/* =====================================================================================

  ---WED_WDMA_INT_CLR (0x15010000 + 0xa24)---

    INT_CLR[31..0]               - (RW) Specify the WDMA interrupt status bits which will be cleared by WED hardware, after WED reads out the interrupt status and the corresponding event bit is asserting.
                                     Each bit's definition is same as WED_WDMA_INT_STA_REC
                                     Only supported:
                                     [17] rx_done1
                                     [16] rx_done0
                                     [1] tx_done1
                                     [0] tx_done0
                                     Other bits are no effect

 =====================================================================================*/
#define WED_WDMA_INT_CLR_INT_CLR_ADDR                     WED_WDMA_INT_CLR_ADDR
#define WED_WDMA_INT_CLR_INT_CLR_MASK                     0xFFFFFFFF                // INT_CLR[31..0]
#define WED_WDMA_INT_CLR_INT_CLR_SHFT                     0

/* =====================================================================================

  ---WED_WDMA_INT_TRIG (0x15010000 + 0xa28)---

    INT_TRIG[31..0]              - (RW) Specify the WDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from Ethernet WDMA
                                     Each bit's definition is same as WED_WDMA_INT_STA_REC
                                     Only supported:
                                     [17] rx_done1
                                     [16] rx_done0
                                     [1] tx_done1
                                     [0] tx_done0
                                     Other bits are no effect

 =====================================================================================*/
#define WED_WDMA_INT_TRIG_INT_TRIG_ADDR                   WED_WDMA_INT_TRIG_ADDR
#define WED_WDMA_INT_TRIG_INT_TRIG_MASK                   0xFFFFFFFF                // INT_TRIG[31..0]
#define WED_WDMA_INT_TRIG_INT_TRIG_SHFT                   0

/* =====================================================================================

  ---WED_WDMA_INT_CTRL (0x15010000 + 0xA2C)---

    POLL_PRD[7..0]               - (RW) WDMA interrupt status polling period
                                     polling period = poll_prd * 16 * AXI_clock_period
    RESERVED8[11..8]             - (RO) Reserved bits
    POLL_MODE[13..12]            - (RW) WDMA interrupt status polling mode
    RESERVED14[15..14]           - (RO) Reserved bits
    SRC_SEL[17..16]              - (RW) WDMA interrupt signal source selection
                                     (Amount of WED/WDMA depends on system configuration)
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_INT_CTRL_SRC_SEL_ADDR                    WED_WDMA_INT_CTRL_ADDR
#define WED_WDMA_INT_CTRL_SRC_SEL_MASK                    0x00030000                // SRC_SEL[17..16]
#define WED_WDMA_INT_CTRL_SRC_SEL_SHFT                    16
#define WED_WDMA_INT_CTRL_POLL_MODE_ADDR                  WED_WDMA_INT_CTRL_ADDR
#define WED_WDMA_INT_CTRL_POLL_MODE_MASK                  0x00003000                // POLL_MODE[13..12]
#define WED_WDMA_INT_CTRL_POLL_MODE_SHFT                  12
#define WED_WDMA_INT_CTRL_POLL_PRD_ADDR                   WED_WDMA_INT_CTRL_ADDR
#define WED_WDMA_INT_CTRL_POLL_PRD_MASK                   0x000000FF                // POLL_PRD[7..0]
#define WED_WDMA_INT_CTRL_POLL_PRD_SHFT                   0

/* =====================================================================================

  ---WED_WDMA_INT_MON (0x15010000 + 0xA3C)---

    RESERVED0[15..0]             - (RO) Reserved bits
    RX_DONE_INT0[16]             - (RO) Rx ring #0 packet receive interrupt event
    RX_DONE_INT1[17]             - (RO) Rx ring #1 packet receive interrupt event
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_INT_MON_RX_DONE_INT1_ADDR                WED_WDMA_INT_MON_ADDR
#define WED_WDMA_INT_MON_RX_DONE_INT1_MASK                0x00020000                // RX_DONE_INT1[17]
#define WED_WDMA_INT_MON_RX_DONE_INT1_SHFT                17
#define WED_WDMA_INT_MON_RX_DONE_INT0_ADDR                WED_WDMA_INT_MON_ADDR
#define WED_WDMA_INT_MON_RX_DONE_INT0_MASK                0x00010000                // RX_DONE_INT0[16]
#define WED_WDMA_INT_MON_RX_DONE_INT0_SHFT                16

/* =====================================================================================

  ---WED_WDMA_TX_DRV_PMTR_1 (0x15010000 + 0xa40)---

    CONFIG[1..0]                 - (RW) Decide monitor
                                     0: monitor axi write  DRAM
                                     1: monitor axi read  DRAM
                                     2: monitor axi write CR
                                     3: monitor axi read CR
    EN_TRIG[2]                   - (A0) set 1 to trigger pmtr enable, this bit will clear by HW.
    TIMES[5..3]                  - (RW) The quantity used to calculate the average. 2^times
    AVERAGE_RDY[6]               - (RO) average ready
    AVERAGE[27..7]               - (RO) The average latency in TIMES axi command. Unit is ns
    CLEAR[28]                    - (A0) clear the result of PMTR. This bit will clear by HW
    latency_error[29]            - (RO) If one axi command latency is over 16384ns, latency_error will be set 1. And this axi command do not caucalate in average.
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX_DRV_PMTR_1_latency_error_ADDR         WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_latency_error_MASK         0x20000000                // latency_error[29]
#define WED_WDMA_TX_DRV_PMTR_1_latency_error_SHFT         29
#define WED_WDMA_TX_DRV_PMTR_1_CLEAR_ADDR                 WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_CLEAR_MASK                 0x10000000                // CLEAR[28]
#define WED_WDMA_TX_DRV_PMTR_1_CLEAR_SHFT                 28
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_ADDR               WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_MASK               0x0FFFFF80                // AVERAGE[27..7]
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_SHFT               7
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_RDY_ADDR           WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_RDY_MASK           0x00000040                // AVERAGE_RDY[6]
#define WED_WDMA_TX_DRV_PMTR_1_AVERAGE_RDY_SHFT           6
#define WED_WDMA_TX_DRV_PMTR_1_TIMES_ADDR                 WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_TIMES_MASK                 0x00000038                // TIMES[5..3]
#define WED_WDMA_TX_DRV_PMTR_1_TIMES_SHFT                 3
#define WED_WDMA_TX_DRV_PMTR_1_EN_TRIG_ADDR               WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_EN_TRIG_MASK               0x00000004                // EN_TRIG[2]
#define WED_WDMA_TX_DRV_PMTR_1_EN_TRIG_SHFT               2
#define WED_WDMA_TX_DRV_PMTR_1_CONFIG_ADDR                WED_WDMA_TX_DRV_PMTR_1_ADDR
#define WED_WDMA_TX_DRV_PMTR_1_CONFIG_MASK                0x00000003                // CONFIG[1..0]
#define WED_WDMA_TX_DRV_PMTR_1_CONFIG_SHFT                0

/* =====================================================================================

  ---WED_WDMA_TX_DRV_PMTR_2 (0x15010000 + 0xa44)---

    max_latency[13..0]           - (RO) The max latency in one en_trigger. Unit is 1ns
    RESERVED14[31..14]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_TX_DRV_PMTR_2_max_latency_ADDR           WED_WDMA_TX_DRV_PMTR_2_ADDR
#define WED_WDMA_TX_DRV_PMTR_2_max_latency_MASK           0x00003FFF                // max_latency[13..0]
#define WED_WDMA_TX_DRV_PMTR_2_max_latency_SHFT           0

/* =====================================================================================

  ---WED_WPDMA_RRO3P1_RX_DRV_PMTR_1 (0x15010000 + 0xa48)---

    CONFIG[0]                    - (RW) Decide monitor
                                     0: monitor axi write WPDMA CR
                                     1: monitor axi read WPDMA CR
    RESERVED1[1]                 - (RO) Reserved bits
    EN_TRIG[2]                   - (A0) set 1 to trigger pmtr enable, this bit will clear by HW.
    TIMES[5..3]                  - (RW) The quantity used to calculate the average. 2^times
    AVERAGE_RDY[6]               - (RO) average ready
    AVERAGE[27..7]               - (RO) The average latency in TIMES axi command. Unit is ns
    CLEAR[28]                    - (A0) clear the result of PMTR. This bit will clear by HW
    latency_error[29]            - (RO) If one axi command latency is over 16384ns, latency_error will be set 1. And this axi command do not caucalate in average.
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_latency_error_ADDR WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_latency_error_MASK 0x20000000                // latency_error[29]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_latency_error_SHFT 29
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CLEAR_ADDR         WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CLEAR_MASK         0x10000000                // CLEAR[28]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CLEAR_SHFT         28
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_ADDR       WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_MASK       0x0FFFFF80                // AVERAGE[27..7]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_SHFT       7
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_RDY_ADDR   WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_RDY_MASK   0x00000040                // AVERAGE_RDY[6]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_AVERAGE_RDY_SHFT   6
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_TIMES_ADDR         WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_TIMES_MASK         0x00000038                // TIMES[5..3]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_TIMES_SHFT         3
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_EN_TRIG_ADDR       WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_EN_TRIG_MASK       0x00000004                // EN_TRIG[2]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_EN_TRIG_SHFT       2
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CONFIG_ADDR        WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CONFIG_MASK        0x00000001                // CONFIG[0]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_1_CONFIG_SHFT        0

/* =====================================================================================

  ---WED_WPDMA_RRO3P1_RX_DRV_PMTR_2 (0x15010000 + 0xa4c)---

    max_latency[13..0]           - (RO) The max latency in one en_trigger. Unit is 1ns
    RESERVED14[31..14]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_2_max_latency_ADDR   WED_WPDMA_RRO3P1_RX_DRV_PMTR_2_ADDR
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_2_max_latency_MASK   0x00003FFF                // max_latency[13..0]
#define WED_WPDMA_RRO3P1_RX_DRV_PMTR_2_max_latency_SHFT   0

/* =====================================================================================

  ---WED_WDMA_CFG_BASE (0x15010000 + 0xaa0)---

    PTR[31..0]                   - (RW) The WDMA configuration registers physical base address

 =====================================================================================*/
#define WED_WDMA_CFG_BASE_PTR_ADDR                        WED_WDMA_CFG_BASE_ADDR
#define WED_WDMA_CFG_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define WED_WDMA_CFG_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---WED_WDMA_OFST0 (0x15010000 + 0xaa4)---

    INTS[15..0]                  - (RW) The offset of WDMA interrupt status registers
    GLO_CFG[31..16]              - (RW) The offset of WDMA global configuration registers

 =====================================================================================*/
#define WED_WDMA_OFST0_GLO_CFG_ADDR                       WED_WDMA_OFST0_ADDR
#define WED_WDMA_OFST0_GLO_CFG_MASK                       0xFFFF0000                // GLO_CFG[31..16]
#define WED_WDMA_OFST0_GLO_CFG_SHFT                       16
#define WED_WDMA_OFST0_INTS_ADDR                          WED_WDMA_OFST0_ADDR
#define WED_WDMA_OFST0_INTS_MASK                          0x0000FFFF                // INTS[15..0]
#define WED_WDMA_OFST0_INTS_SHFT                          0

/* =====================================================================================

  ---WED_WDMA_OFST1 (0x15010000 + 0xaa8)---

    TX0_CTRL[15..0]              - (RW) The offset of WDMA TX_BASE_PTR_0 registers in Ethernet Frame Engine WDMA
    RX0_CTRL[31..16]             - (RW) The offset of WDMA RX_BASE_PTR_0 registers in Ethernet Frame Engine WDMA

 =====================================================================================*/
#define WED_WDMA_OFST1_RX0_CTRL_ADDR                      WED_WDMA_OFST1_ADDR
#define WED_WDMA_OFST1_RX0_CTRL_MASK                      0xFFFF0000                // RX0_CTRL[31..16]
#define WED_WDMA_OFST1_RX0_CTRL_SHFT                      16
#define WED_WDMA_OFST1_TX0_CTRL_ADDR                      WED_WDMA_OFST1_ADDR
#define WED_WDMA_OFST1_TX0_CTRL_MASK                      0x0000FFFF                // TX0_CTRL[15..0]
#define WED_WDMA_OFST1_TX0_CTRL_SHFT                      0

/* =====================================================================================

  ---WED_WDMA_CFG_BASE_H (0x15010000 + 0xaac)---

    PTR[7..0]                    - (RW) Bit[39:32] of the WDMA configuration registers physical base address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_WDMA_CFG_BASE_H_PTR_ADDR                      WED_WDMA_CFG_BASE_H_ADDR
#define WED_WDMA_CFG_BASE_H_PTR_MASK                      0x000000FF                // PTR[7..0]
#define WED_WDMA_CFG_BASE_H_PTR_SHFT                      0

/* =====================================================================================

  ---WED_WDMA_RX0_MIB (0x15010000 + 0xae0)---

    PKT_CNT[31..0]               - (RC) WED received from Ethernet WDMA Rx ring0 packet counter (the counter will not be increased if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WDMA_RX0_MIB_PKT_CNT_ADDR                     WED_WDMA_RX0_MIB_ADDR
#define WED_WDMA_RX0_MIB_PKT_CNT_MASK                     0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WDMA_RX0_MIB_PKT_CNT_SHFT                     0

/* =====================================================================================

  ---WED_WDMA_RX1_MIB (0x15010000 + 0xae4)---

    PKT_CNT[31..0]               - (RC) WED received from Ethernet WDMA Rx ring1 packet counter(the counter will not be increased if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WDMA_RX1_MIB_PKT_CNT_ADDR                     WED_WDMA_RX1_MIB_ADDR
#define WED_WDMA_RX1_MIB_PKT_CNT_MASK                     0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WDMA_RX1_MIB_PKT_CNT_SHFT                     0

/* =====================================================================================

  ---WED_WDMA_RX0_RECYCLE_MIB (0x15010000 + 0xae8)---

    CNT[31..0]                   - (RC) Ethernet WDMA Rx ring0 descriptor recycled by WED Rx Driver counter(For WED design, Expect 1 descriptor=1 packet)

 =====================================================================================*/
#define WED_WDMA_RX0_RECYCLE_MIB_CNT_ADDR                 WED_WDMA_RX0_RECYCLE_MIB_ADDR
#define WED_WDMA_RX0_RECYCLE_MIB_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_WDMA_RX0_RECYCLE_MIB_CNT_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_RX1_RECYCLE_MIB (0x15010000 + 0xaec)---

    CNT[31..0]                   - (RC) Ethernet WDMA Rx ring1 descriptor recycled by WED Rx Driver counter(For WED design, Expect 1 descriptor=1 packet)

 =====================================================================================*/
#define WED_WDMA_RX1_RECYCLE_MIB_CNT_ADDR                 WED_WDMA_RX1_RECYCLE_MIB_ADDR
#define WED_WDMA_RX1_RECYCLE_MIB_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_WDMA_RX1_RECYCLE_MIB_CNT_SHFT                 0

/* =====================================================================================

  ---WED_WDMA_RX0_PROCESSED_MIB (0x15010000 + 0xaf0)---

    PKT_CNT[31..0]               - (RC) Ethernet WDMA Rx ring0 all packet processed by Rx Driver counter (the counter counts all packet processed by Rx Driver, even if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WDMA_RX0_PROCESSED_MIB_PKT_CNT_ADDR           WED_WDMA_RX0_PROCESSED_MIB_ADDR
#define WED_WDMA_RX0_PROCESSED_MIB_PKT_CNT_MASK           0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WDMA_RX0_PROCESSED_MIB_PKT_CNT_SHFT           0

/* =====================================================================================

  ---WED_WDMA_RX1_PROCESSED_MIB (0x15010000 + 0xaf4)---

    PKT_CNT[31..0]               - (RC) Ethernet WDMA Rx ring1 all packet processed by Rx Driver counter (the counter counts all packet processed by Rx Driver, even if the descriptor has recycled and packet drop)

 =====================================================================================*/
#define WED_WDMA_RX1_PROCESSED_MIB_PKT_CNT_ADDR           WED_WDMA_RX1_PROCESSED_MIB_ADDR
#define WED_WDMA_RX1_PROCESSED_MIB_PKT_CNT_MASK           0xFFFFFFFF                // PKT_CNT[31..0]
#define WED_WDMA_RX1_PROCESSED_MIB_PKT_CNT_SHFT           0

/* =====================================================================================

  ---WED_WDMA_RX_COHERENT_MIB (0x15010000 + 0xaf8)---

    CNT[31..0]                   - (RC) Rx Driver Rx Coherent event counter

 =====================================================================================*/
#define WED_WDMA_RX_COHERENT_MIB_CNT_ADDR                 WED_WDMA_RX_COHERENT_MIB_ADDR
#define WED_WDMA_RX_COHERENT_MIB_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_WDMA_RX_COHERENT_MIB_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_GLO_CFG (0x15010000 + 0xb00)---

    ENABLE[8..0]                 - (RW) Rx Route QM enables.
                                     Each bit represents enable of different HW. Set 1 to enable, set 0 to disable.
                                     [0] Global Enable
                                     [1] Enable Enqueue HW
                                     [2] Enable Dequeue HW
                                     [3] Enable Queue Free Entry Prefetch HW
                                     [7:4] Enable IGRS3~0
                                     [8] Enable Queue Entry Ageout HW
    RESERVED9[15..9]             - (RO) Reserved bits
    STATUS_BUSY[16]              - (RO) Global Busy (Packet Transferring)
    STATUS_ENQUEUE_BUSY[17]      - (RO) Enqueue HW Busy
    STATUS_DEQUEUE_BUSY[18]      - (RO) Dequeue HW Busy
    STATUS_ENTRY_PREFETCHED[19]  - (RO) Entry Prefetched (At least 1 free entry prefetched)
    STATUS_ENTRY_ENQUEUED[20]    - (RO) Entry Enqueued (At least 1 DMAD enqueued)
    STATUS_ENTRY_ALL_PREFETCHABLE[21] - (RO) Queue entry all prefetchable
    STATUS_QUEUE_NOT_EMPTY[22]   - (RO) Queue not empty (At least 1 DMAD enqueued)
    STATUS_QUEUE_FULL[23]        - (RO) Queue Full
    STATUS_IGRS0_BUSY[24]        - (RO) IGRS0 Busy (Data Transferring)
    STATUS_IGRS1_BUSY[25]        - (RO) IGRS1 Busy (Data Transferring)
    STATUS_IGRS2_BUSY[26]        - (RO) IGRS2 Busy (Data Transferring)
    STATUS_IGRS3_BUSY[27]        - (RO) IGRS3 Busy (Data Transferring)
    STATUS_AGE_OUT_BUSY[28]      - (RO) Queue Ageout HW Busy
    RESERVED29[30..29]           - (RO) Reserved bits
    CG_DIS[31]                   - (RW) Disable clock gating (clock free run).

 =====================================================================================*/
#define WED_RTQM_GLO_CFG_CG_DIS_ADDR                      WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_CG_DIS_MASK                      0x80000000                // CG_DIS[31]
#define WED_RTQM_GLO_CFG_CG_DIS_SHFT                      31
#define WED_RTQM_GLO_CFG_STATUS_AGE_OUT_BUSY_ADDR         WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_AGE_OUT_BUSY_MASK         0x10000000                // STATUS_AGE_OUT_BUSY[28]
#define WED_RTQM_GLO_CFG_STATUS_AGE_OUT_BUSY_SHFT         28
#define WED_RTQM_GLO_CFG_STATUS_IGRS3_BUSY_ADDR           WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_IGRS3_BUSY_MASK           0x08000000                // STATUS_IGRS3_BUSY[27]
#define WED_RTQM_GLO_CFG_STATUS_IGRS3_BUSY_SHFT           27
#define WED_RTQM_GLO_CFG_STATUS_IGRS2_BUSY_ADDR           WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_IGRS2_BUSY_MASK           0x04000000                // STATUS_IGRS2_BUSY[26]
#define WED_RTQM_GLO_CFG_STATUS_IGRS2_BUSY_SHFT           26
#define WED_RTQM_GLO_CFG_STATUS_IGRS1_BUSY_ADDR           WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_IGRS1_BUSY_MASK           0x02000000                // STATUS_IGRS1_BUSY[25]
#define WED_RTQM_GLO_CFG_STATUS_IGRS1_BUSY_SHFT           25
#define WED_RTQM_GLO_CFG_STATUS_IGRS0_BUSY_ADDR           WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_IGRS0_BUSY_MASK           0x01000000                // STATUS_IGRS0_BUSY[24]
#define WED_RTQM_GLO_CFG_STATUS_IGRS0_BUSY_SHFT           24
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_FULL_ADDR           WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_FULL_MASK           0x00800000                // STATUS_QUEUE_FULL[23]
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_FULL_SHFT           23
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_NOT_EMPTY_ADDR      WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_NOT_EMPTY_MASK      0x00400000                // STATUS_QUEUE_NOT_EMPTY[22]
#define WED_RTQM_GLO_CFG_STATUS_QUEUE_NOT_EMPTY_SHFT      22
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ALL_PREFETCHABLE_ADDR WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ALL_PREFETCHABLE_MASK 0x00200000                // STATUS_ENTRY_ALL_PREFETCHABLE[21]
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ALL_PREFETCHABLE_SHFT 21
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ENQUEUED_ADDR       WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ENQUEUED_MASK       0x00100000                // STATUS_ENTRY_ENQUEUED[20]
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_ENQUEUED_SHFT       20
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_PREFETCHED_ADDR     WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_PREFETCHED_MASK     0x00080000                // STATUS_ENTRY_PREFETCHED[19]
#define WED_RTQM_GLO_CFG_STATUS_ENTRY_PREFETCHED_SHFT     19
#define WED_RTQM_GLO_CFG_STATUS_DEQUEUE_BUSY_ADDR         WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_DEQUEUE_BUSY_MASK         0x00040000                // STATUS_DEQUEUE_BUSY[18]
#define WED_RTQM_GLO_CFG_STATUS_DEQUEUE_BUSY_SHFT         18
#define WED_RTQM_GLO_CFG_STATUS_ENQUEUE_BUSY_ADDR         WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_ENQUEUE_BUSY_MASK         0x00020000                // STATUS_ENQUEUE_BUSY[17]
#define WED_RTQM_GLO_CFG_STATUS_ENQUEUE_BUSY_SHFT         17
#define WED_RTQM_GLO_CFG_STATUS_BUSY_ADDR                 WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_STATUS_BUSY_MASK                 0x00010000                // STATUS_BUSY[16]
#define WED_RTQM_GLO_CFG_STATUS_BUSY_SHFT                 16
#define WED_RTQM_GLO_CFG_ENABLE_ADDR                      WED_RTQM_GLO_CFG_ADDR
#define WED_RTQM_GLO_CFG_ENABLE_MASK                      0x000001FF                // ENABLE[8..0]
#define WED_RTQM_GLO_CFG_ENABLE_SHFT                      0

/* =====================================================================================

  ---WED_RTQM_RST (0x15010000 + 0xb04)---

    RST_QUEUE[0]                 - (RW) reset queue. Set 1 to active reset, set 0 to release reset.
    RST_ENQUEUE[1]               - (RW) reset enqueue. Set 1 to active reset, set 0 to release reset.
    RST_DEQUEUE[2]               - (RW) reset dequeue. Set 1 to active reset, set 0 to release reset.
    RST_PFDBK_FIFO_ARB[3]        - (RW) reset ppe feedback fifo arbiter. Set 1 to active reset, set 0 to release reset.
    RST_IGRS0[4]                 - (RW) reset IGRS0. Set 1 to active reset, set 0 to release reset.
    RST_IGRS1[5]                 - (RW) reset IGRS1. Set 1 to active reset, set 0 to release reset.
    RST_IGRS2[6]                 - (RW) reset IGRS2. Set 1 to active reset, set 0 to release reset.
    RST_IGRS3[7]                 - (RW) reset IGRS3. Set 1 to active reset, set 0 to release reset.
    RST_EGRS0[8]                 - (RW) reset EGRS0. Set 1 to active reset, set 0 to release reset.
    RST_EGRS1[9]                 - (RW) reset EGRS1. Set 1 to active reset, set 0 to release reset.
    RST_EGRS2[10]                - (RW) reset EGRS2. Set 1 to active reset, set 0 to release reset.
    RST_EGRS3[11]                - (RW) reset EGRS3. Set 1 to active reset, set 0 to release reset.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_RST_RST_EGRS3_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_EGRS3_MASK                       0x00000800                // RST_EGRS3[11]
#define WED_RTQM_RST_RST_EGRS3_SHFT                       11
#define WED_RTQM_RST_RST_EGRS2_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_EGRS2_MASK                       0x00000400                // RST_EGRS2[10]
#define WED_RTQM_RST_RST_EGRS2_SHFT                       10
#define WED_RTQM_RST_RST_EGRS1_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_EGRS1_MASK                       0x00000200                // RST_EGRS1[9]
#define WED_RTQM_RST_RST_EGRS1_SHFT                       9
#define WED_RTQM_RST_RST_EGRS0_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_EGRS0_MASK                       0x00000100                // RST_EGRS0[8]
#define WED_RTQM_RST_RST_EGRS0_SHFT                       8
#define WED_RTQM_RST_RST_IGRS3_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_IGRS3_MASK                       0x00000080                // RST_IGRS3[7]
#define WED_RTQM_RST_RST_IGRS3_SHFT                       7
#define WED_RTQM_RST_RST_IGRS2_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_IGRS2_MASK                       0x00000040                // RST_IGRS2[6]
#define WED_RTQM_RST_RST_IGRS2_SHFT                       6
#define WED_RTQM_RST_RST_IGRS1_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_IGRS1_MASK                       0x00000020                // RST_IGRS1[5]
#define WED_RTQM_RST_RST_IGRS1_SHFT                       5
#define WED_RTQM_RST_RST_IGRS0_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_IGRS0_MASK                       0x00000010                // RST_IGRS0[4]
#define WED_RTQM_RST_RST_IGRS0_SHFT                       4
#define WED_RTQM_RST_RST_PFDBK_FIFO_ARB_ADDR              WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_PFDBK_FIFO_ARB_MASK              0x00000008                // RST_PFDBK_FIFO_ARB[3]
#define WED_RTQM_RST_RST_PFDBK_FIFO_ARB_SHFT              3
#define WED_RTQM_RST_RST_DEQUEUE_ADDR                     WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_DEQUEUE_MASK                     0x00000004                // RST_DEQUEUE[2]
#define WED_RTQM_RST_RST_DEQUEUE_SHFT                     2
#define WED_RTQM_RST_RST_ENQUEUE_ADDR                     WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_ENQUEUE_MASK                     0x00000002                // RST_ENQUEUE[1]
#define WED_RTQM_RST_RST_ENQUEUE_SHFT                     1
#define WED_RTQM_RST_RST_QUEUE_ADDR                       WED_RTQM_RST_ADDR
#define WED_RTQM_RST_RST_QUEUE_MASK                       0x00000001                // RST_QUEUE[0]
#define WED_RTQM_RST_RST_QUEUE_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_IGRS_FIFO_CFG (0x15010000 + 0xb08)---

    IGRS0_CNT[4..0]              - (RO) IGRS0 FIFO Used entry
    IGRS0_CLEAR[5]               - (RW) Clear IGRS0 FIFO
    IGRS0_FULL[6]                - (RO) IGRS0 FIFO full
    IGRS0_EMPTY[7]               - (RO) IGRS0 FIFO empty
    IGRS1_CNT[12..8]             - (RO) IGRS1 FIFO Used entry
    IGRS1_CLEAR[13]              - (RW) Clear IGRS1 FIFO
    IGRS1_FULL[14]               - (RO) IGRS1 FIFO full
    IGRS1_EMPTY[15]              - (RO) IGRS1 FIFO empty
    IGRS2_CNT[20..16]            - (RO) IGRS2 FIFO Used entry
    IGRS2_CLEAR[21]              - (RW) Clear IGRS2 FIFO
    IGRS2_FULL[22]               - (RO) IGRS2 FIFO full
    IGRS2_EMPTY[23]              - (RO) IGRS2 FIFO empty
    IGRS3_CNT[28..24]            - (RO) IGRS3 FIFO Used entry
    IGRS3_CLEAR[29]              - (RW) Clear IGRS3 FIFO
    IGRS3_FULL[30]               - (RO) IGRS3 FIFO full
    IGRS3_EMPTY[31]              - (RO) IGRS3 FIFO empty

 =====================================================================================*/
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_EMPTY_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_EMPTY_MASK           0x80000000                // IGRS3_EMPTY[31]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_EMPTY_SHFT           31
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_FULL_ADDR            WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_FULL_MASK            0x40000000                // IGRS3_FULL[30]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_FULL_SHFT            30
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CLEAR_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CLEAR_MASK           0x20000000                // IGRS3_CLEAR[29]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CLEAR_SHFT           29
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CNT_ADDR             WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CNT_MASK             0x1F000000                // IGRS3_CNT[28..24]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS3_CNT_SHFT             24
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_EMPTY_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_EMPTY_MASK           0x00800000                // IGRS2_EMPTY[23]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_EMPTY_SHFT           23
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_FULL_ADDR            WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_FULL_MASK            0x00400000                // IGRS2_FULL[22]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_FULL_SHFT            22
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CLEAR_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CLEAR_MASK           0x00200000                // IGRS2_CLEAR[21]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CLEAR_SHFT           21
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CNT_ADDR             WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CNT_MASK             0x001F0000                // IGRS2_CNT[20..16]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS2_CNT_SHFT             16
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_EMPTY_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_EMPTY_MASK           0x00008000                // IGRS1_EMPTY[15]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_EMPTY_SHFT           15
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_FULL_ADDR            WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_FULL_MASK            0x00004000                // IGRS1_FULL[14]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_FULL_SHFT            14
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CLEAR_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CLEAR_MASK           0x00002000                // IGRS1_CLEAR[13]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CLEAR_SHFT           13
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CNT_ADDR             WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CNT_MASK             0x00001F00                // IGRS1_CNT[12..8]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS1_CNT_SHFT             8
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_EMPTY_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_EMPTY_MASK           0x00000080                // IGRS0_EMPTY[7]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_EMPTY_SHFT           7
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_FULL_ADDR            WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_FULL_MASK            0x00000040                // IGRS0_FULL[6]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_FULL_SHFT            6
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CLEAR_ADDR           WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CLEAR_MASK           0x00000020                // IGRS0_CLEAR[5]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CLEAR_SHFT           5
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CNT_ADDR             WED_RTQM_IGRS_FIFO_CFG_ADDR
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CNT_MASK             0x0000001F                // IGRS0_CNT[4..0]
#define WED_RTQM_IGRS_FIFO_CFG_IGRS0_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_PFDBK_FIFO_CFG (0x15010000 + 0xb0c)---

    P2Q_CNT[10..0]               - (RO) PPE2RTQM PFDBK FIFO Used entry
    RESERVED11[11]               - (RO) Reserved bits
    P2Q_GFFDIS[12]               - (RW) Disable PPE2RTQM PFDBK FIFO Internal Fast Path
    P2Q_CLEAR[13]                - (RW) Clear PPE2RTQM PFDBK FIFO
    P2Q_FULL[14]                 - (RO) PPE2RTQM PFDBK FIFO full
    P2Q_EMPTY[15]                - (RO) PPE2RTQM PFDBK FIFO empty
    A2Q_CNT[20..16]              - (RO) Ageout Dummy PFDBK FIFO Used entry
    RESERVED21[23..21]           - (RO) Reserved bits
    A2Q_CLEAR[24]                - (RW) Clear Ageout Dummy PFDBK FIFO
    A2Q_FULL[25]                 - (RO) Ageout Dummy PFDBK FIFO full
    A2Q_EMPTY[26]                - (RO) Ageout Dummy PFDBK FIFO empty
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_EMPTY_ADDR            WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_EMPTY_MASK            0x04000000                // A2Q_EMPTY[26]
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_EMPTY_SHFT            26
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_FULL_ADDR             WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_FULL_MASK             0x02000000                // A2Q_FULL[25]
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_FULL_SHFT             25
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CLEAR_ADDR            WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CLEAR_MASK            0x01000000                // A2Q_CLEAR[24]
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CLEAR_SHFT            24
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CNT_ADDR              WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CNT_MASK              0x001F0000                // A2Q_CNT[20..16]
#define WED_RTQM_PFDBK_FIFO_CFG_A2Q_CNT_SHFT              16
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_EMPTY_ADDR            WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_EMPTY_MASK            0x00008000                // P2Q_EMPTY[15]
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_EMPTY_SHFT            15
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_FULL_ADDR             WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_FULL_MASK             0x00004000                // P2Q_FULL[14]
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_FULL_SHFT             14
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CLEAR_ADDR            WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CLEAR_MASK            0x00002000                // P2Q_CLEAR[13]
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CLEAR_SHFT            13
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_GFFDIS_ADDR           WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_GFFDIS_MASK           0x00001000                // P2Q_GFFDIS[12]
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_GFFDIS_SHFT           12
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CNT_ADDR              WED_RTQM_PFDBK_FIFO_CFG_ADDR
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CNT_MASK              0x000007FF                // P2Q_CNT[10..0]
#define WED_RTQM_PFDBK_FIFO_CFG_P2Q_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_PREF_FIFO_CFG (0x15010000 + 0xb10)---

    RESERVED0[0]                 - (RO) Reserved bits
    PREF_0_CLEAR[1]              - (RW) Clear PREF_0 FIFO
    PREF_0_FULL[2]               - (RO) PREF_0 FIFO full
    PREF_0_EMPTY[3]              - (RO) PREF_0 FIFO empty
    RESERVED4[4]                 - (RO) Reserved bits
    PREF_1_CLEAR[5]              - (RW) Clear PREF_1 FIFO
    PREF_1_FULL[6]               - (RO) PREF_1 FIFO full
    PREF_1_EMPTY[7]              - (RO) PREF_1 FIFO empty
    RESERVED8[8]                 - (RO) Reserved bits
    PREF_2_CLEAR[9]              - (RW) Clear PREF_2 FIFO
    PREF_2_FULL[10]              - (RO) PREF_2 FIFO full
    PREF_2_EMPTY[11]             - (RO) PREF_2 FIFO empty
    RESERVED12[12]               - (RO) Reserved bits
    PREF_3_CLEAR[13]             - (RW) Clear PREF_3 FIFO
    PREF_3_FULL[14]              - (RO) PREF_3 FIFO full
    PREF_3_EMPTY[15]             - (RO) PREF_3 FIFO empty
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_EMPTY_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_EMPTY_MASK          0x00008000                // PREF_3_EMPTY[15]
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_EMPTY_SHFT          15
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_FULL_ADDR           WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_FULL_MASK           0x00004000                // PREF_3_FULL[14]
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_FULL_SHFT           14
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_CLEAR_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_CLEAR_MASK          0x00002000                // PREF_3_CLEAR[13]
#define WED_RTQM_PREF_FIFO_CFG_PREF_3_CLEAR_SHFT          13
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_EMPTY_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_EMPTY_MASK          0x00000800                // PREF_2_EMPTY[11]
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_EMPTY_SHFT          11
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_FULL_ADDR           WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_FULL_MASK           0x00000400                // PREF_2_FULL[10]
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_FULL_SHFT           10
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_CLEAR_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_CLEAR_MASK          0x00000200                // PREF_2_CLEAR[9]
#define WED_RTQM_PREF_FIFO_CFG_PREF_2_CLEAR_SHFT          9
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_EMPTY_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_EMPTY_MASK          0x00000080                // PREF_1_EMPTY[7]
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_EMPTY_SHFT          7
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_FULL_ADDR           WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_FULL_MASK           0x00000040                // PREF_1_FULL[6]
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_FULL_SHFT           6
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_CLEAR_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_CLEAR_MASK          0x00000020                // PREF_1_CLEAR[5]
#define WED_RTQM_PREF_FIFO_CFG_PREF_1_CLEAR_SHFT          5
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_EMPTY_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_EMPTY_MASK          0x00000008                // PREF_0_EMPTY[3]
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_EMPTY_SHFT          3
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_FULL_ADDR           WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_FULL_MASK           0x00000004                // PREF_0_FULL[2]
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_FULL_SHFT           2
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_CLEAR_ADDR          WED_RTQM_PREF_FIFO_CFG_ADDR
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_CLEAR_MASK          0x00000002                // PREF_0_CLEAR[1]
#define WED_RTQM_PREF_FIFO_CFG_PREF_0_CLEAR_SHFT          1

/* =====================================================================================

  ---WED_RTQM_IGRS0_CFG0 (0x15010000 + 0xb14)---

    DEST_FORCE[2..0]             - (RW) Force path control
    FORCE_DROP[3]                - (RW) Force Drop all DMAD
    DEST_CHK_V0[6..4]            - (RW) DMAD version 0 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] TO_HOST==1
                                     [2] TO_HOST_A==1
    RESERVED7[7]                 - (RO) Reserved bits
    DEST_CHK_V1[15..8]           - (RW) DMAD version 1 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] UN==1
                                     [2] C==1
                                     [3] TO_HOST==1
                                     [4] CS_STATUS[0]==1
                                     [5] CS_STATUS[1]==1
                                     [6] CS_STATUS[2]==1
                                     [7] CS_STATUS[3]==1
    SCH_REQ_ANY_TIME[16]         - (RW) Assert scheduler request signal any time
    RESERVED17[17]               - (RO) Reserved bits
    DMAD_CLR_PPE_FLD[19..18]     - (RW) Clear DMAD PPE Related Field
                                     [0] Enable, clear DMAD PPE_VLD/PPE_ENTRY/CRSN Field to 0
                                     [1] Clear all DMAD (if set to 0, clear 1st DMAD for scatter/gather packet)
    DMAD_MOD_EN[26..20]          - (RW) Enable DMAD modification feature
                                     [0] PPE_VLD
                                     [1] PPE_ENTRY
                                     [2] CRSN
                                     [3] DDONE
                                     [4] LS
                                     [5] RING_INFO
                                     [6] TO_HOST
    DMAD_MOD_METHOD[27]          - (RW) modification method of DMAD modification feature
    HOST_RING_SRC[29..28]        - (RW) To host path ring info source selection
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_IGRS0_CFG0_HOST_RING_SRC_ADDR            WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_HOST_RING_SRC_MASK            0x30000000                // HOST_RING_SRC[29..28]
#define WED_RTQM_IGRS0_CFG0_HOST_RING_SRC_SHFT            28
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_METHOD_ADDR          WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_METHOD_MASK          0x08000000                // DMAD_MOD_METHOD[27]
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_METHOD_SHFT          27
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_EN_ADDR              WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_EN_MASK              0x07F00000                // DMAD_MOD_EN[26..20]
#define WED_RTQM_IGRS0_CFG0_DMAD_MOD_EN_SHFT              20
#define WED_RTQM_IGRS0_CFG0_DMAD_CLR_PPE_FLD_ADDR         WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DMAD_CLR_PPE_FLD_MASK         0x000C0000                // DMAD_CLR_PPE_FLD[19..18]
#define WED_RTQM_IGRS0_CFG0_DMAD_CLR_PPE_FLD_SHFT         18
#define WED_RTQM_IGRS0_CFG0_SCH_REQ_ANY_TIME_ADDR         WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_SCH_REQ_ANY_TIME_MASK         0x00010000                // SCH_REQ_ANY_TIME[16]
#define WED_RTQM_IGRS0_CFG0_SCH_REQ_ANY_TIME_SHFT         16
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V1_ADDR              WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V1_MASK              0x0000FF00                // DEST_CHK_V1[15..8]
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V1_SHFT              8
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V0_ADDR              WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V0_MASK              0x00000070                // DEST_CHK_V0[6..4]
#define WED_RTQM_IGRS0_CFG0_DEST_CHK_V0_SHFT              4
#define WED_RTQM_IGRS0_CFG0_FORCE_DROP_ADDR               WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_FORCE_DROP_MASK               0x00000008                // FORCE_DROP[3]
#define WED_RTQM_IGRS0_CFG0_FORCE_DROP_SHFT               3
#define WED_RTQM_IGRS0_CFG0_DEST_FORCE_ADDR               WED_RTQM_IGRS0_CFG0_ADDR
#define WED_RTQM_IGRS0_CFG0_DEST_FORCE_MASK               0x00000007                // DEST_FORCE[2..0]
#define WED_RTQM_IGRS0_CFG0_DEST_FORCE_SHFT               0

/* =====================================================================================

  ---WED_RTQM_IGRS0_CFG1 (0x15010000 + 0xb18)---

    RESERVED0[5..0]              - (RO) Reserved bits
    DMAD_MOD_TO_HOST[6]          - (RW) DMAD TO_HOST field value of DMAD modification feature
    DMAD_MOD_RING_INFO[8..7]     - (RW) DMAD RING_INFO field value of DMAD modification feature
    DMAD_MOD_LS[9]               - (RW) DMAD LS field value of DMAD modification feature
    DMAD_MOD_DDONE[10]           - (RW) DMAD DDONE field value of DMAD modification feature
    DMAD_MOD_CRSN[15..11]        - (RW) DMAD CRSN field value of DMAD modification feature
    DMAD_MOD_PPE_ENTRY[30..16]   - (RW) DMAD PPE_ENTRY field value of DMAD modification feature
    DMAD_MOD_PPE_VLD[31]         - (RW) DMAD PPE_VLD field value of DMAD modification feature

 =====================================================================================*/
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_VLD_ADDR         WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_VLD_MASK         0x80000000                // DMAD_MOD_PPE_VLD[31]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_VLD_SHFT         31
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_ENTRY_ADDR       WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_ENTRY_MASK       0x7FFF0000                // DMAD_MOD_PPE_ENTRY[30..16]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_PPE_ENTRY_SHFT       16
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_CRSN_ADDR            WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_CRSN_MASK            0x0000F800                // DMAD_MOD_CRSN[15..11]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_CRSN_SHFT            11
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_DDONE_ADDR           WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_DDONE_MASK           0x00000400                // DMAD_MOD_DDONE[10]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_DDONE_SHFT           10
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_LS_ADDR              WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_LS_MASK              0x00000200                // DMAD_MOD_LS[9]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_LS_SHFT              9
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_RING_INFO_ADDR       WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_RING_INFO_MASK       0x00000180                // DMAD_MOD_RING_INFO[8..7]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_RING_INFO_SHFT       7
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_TO_HOST_ADDR         WED_RTQM_IGRS0_CFG1_ADDR
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_TO_HOST_MASK         0x00000040                // DMAD_MOD_TO_HOST[6]
#define WED_RTQM_IGRS0_CFG1_DMAD_MOD_TO_HOST_SHFT         6

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2HW_DMAD_CNT (0x15010000 + 0xb1c)---

    CNT[31..0]                   - (RC) IGRS to HW DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2HW_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS0_I2HW_DMAD_CNT_ADDR
#define WED_RTQM_IGRS0_I2HW_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2HW_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2H0_DMAD_CNT (0x15010000 + 0xb20)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2H0_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS0_I2H0_DMAD_CNT_ADDR
#define WED_RTQM_IGRS0_I2H0_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2H0_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2H1_DMAD_CNT (0x15010000 + 0xb24)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2H1_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS0_I2H1_DMAD_CNT_ADDR
#define WED_RTQM_IGRS0_I2H1_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2H1_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2HW_PKT_CNT (0x15010000 + 0xb28)---

    CNT[31..0]                   - (RC) IGRS to RX_BM Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2HW_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS0_I2HW_PKT_CNT_ADDR
#define WED_RTQM_IGRS0_I2HW_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2HW_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2H0_PKT_CNT (0x15010000 + 0xb2c)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2H0_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS0_I2H0_PKT_CNT_ADDR
#define WED_RTQM_IGRS0_I2H0_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2H0_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS0_I2H1_PKT_CNT (0x15010000 + 0xb30)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_I2H1_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS0_I2H1_PKT_CNT_ADDR
#define WED_RTQM_IGRS0_I2H1_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_I2H1_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS0_FDROP_CNT (0x15010000 + 0xb34)---

    CNT[31..0]                   - (RC) IGRS Force Drop DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS0_FDROP_CNT_CNT_ADDR                 WED_RTQM_IGRS0_FDROP_CNT_ADDR
#define WED_RTQM_IGRS0_FDROP_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS0_FDROP_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_IGRS0_DBG (0x15010000 + 0xb38)---

    CURR_DMAD_EOP_MON[0]         - (RO) Current DMAD EOP
    CURR_DMAD_SOP_MON[1]         - (RO) Current DMAD SOP
    NEXT_DMAD_SOP_MON[2]         - (RO) Next DMAD SOP
    XOP_CLR[3]                   - (A0) Clear current DMAD SOP
    SCH_RLS_SET[4]               - (RW) Force scheduler release signal
                                     Set scheduler release signal to 1.
    SCH_RLS_MANUAL[5]            - (A0) Assert scheduler release signal manually
                                     Assert scheduler release signal 1 times manually.
    SCH_REQ_MON[8..6]            - (RO) Scheduler request signal monitor
    SCH_RESULT_MON[11..9]        - (RO) Scheduler result signal monitor
    SCH_RESULT_RLS_MON[14..12]   - (RO) Scheduler result release signal monitor
    SEQ_ID_ENABLE[15]            - (RW) Insert DMAD debug sequence ID
                                     Insert sequence ID to DMAD PPE_ENTRY field.
    SEQ_ID_MON[30..16]           - (RO) DMAD debug sequence ID monitor
    SEQ_ID_CLR[31]               - (A0) Clear DMAD debug sequence ID

 =====================================================================================*/
#define WED_RTQM_IGRS0_DBG_SEQ_ID_CLR_ADDR                WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SEQ_ID_CLR_MASK                0x80000000                // SEQ_ID_CLR[31]
#define WED_RTQM_IGRS0_DBG_SEQ_ID_CLR_SHFT                31
#define WED_RTQM_IGRS0_DBG_SEQ_ID_MON_ADDR                WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SEQ_ID_MON_MASK                0x7FFF0000                // SEQ_ID_MON[30..16]
#define WED_RTQM_IGRS0_DBG_SEQ_ID_MON_SHFT                16
#define WED_RTQM_IGRS0_DBG_SEQ_ID_ENABLE_ADDR             WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SEQ_ID_ENABLE_MASK             0x00008000                // SEQ_ID_ENABLE[15]
#define WED_RTQM_IGRS0_DBG_SEQ_ID_ENABLE_SHFT             15
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_RLS_MON_ADDR        WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_RLS_MON_MASK        0x00007000                // SCH_RESULT_RLS_MON[14..12]
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_RLS_MON_SHFT        12
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_MON_ADDR            WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_MON_MASK            0x00000E00                // SCH_RESULT_MON[11..9]
#define WED_RTQM_IGRS0_DBG_SCH_RESULT_MON_SHFT            9
#define WED_RTQM_IGRS0_DBG_SCH_REQ_MON_ADDR               WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SCH_REQ_MON_MASK               0x000001C0                // SCH_REQ_MON[8..6]
#define WED_RTQM_IGRS0_DBG_SCH_REQ_MON_SHFT               6
#define WED_RTQM_IGRS0_DBG_SCH_RLS_MANUAL_ADDR            WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SCH_RLS_MANUAL_MASK            0x00000020                // SCH_RLS_MANUAL[5]
#define WED_RTQM_IGRS0_DBG_SCH_RLS_MANUAL_SHFT            5
#define WED_RTQM_IGRS0_DBG_SCH_RLS_SET_ADDR               WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_SCH_RLS_SET_MASK               0x00000010                // SCH_RLS_SET[4]
#define WED_RTQM_IGRS0_DBG_SCH_RLS_SET_SHFT               4
#define WED_RTQM_IGRS0_DBG_XOP_CLR_ADDR                   WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_XOP_CLR_MASK                   0x00000008                // XOP_CLR[3]
#define WED_RTQM_IGRS0_DBG_XOP_CLR_SHFT                   3
#define WED_RTQM_IGRS0_DBG_NEXT_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_NEXT_DMAD_SOP_MON_MASK         0x00000004                // NEXT_DMAD_SOP_MON[2]
#define WED_RTQM_IGRS0_DBG_NEXT_DMAD_SOP_MON_SHFT         2
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_SOP_MON_MASK         0x00000002                // CURR_DMAD_SOP_MON[1]
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_SOP_MON_SHFT         1
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_EOP_MON_ADDR         WED_RTQM_IGRS0_DBG_ADDR
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_EOP_MON_MASK         0x00000001                // CURR_DMAD_EOP_MON[0]
#define WED_RTQM_IGRS0_DBG_CURR_DMAD_EOP_MON_SHFT         0

/* =====================================================================================

  ---WED_RTQM_IGRS1_CFG0 (0x15010000 + 0xb3c)---

    DEST_FORCE[2..0]             - (RW) Force path control
    FORCE_DROP[3]                - (RW) Force Drop all DMAD
    DEST_CHK_V0[6..4]            - (RW) DMAD ver 0 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] TO_HOST==1
                                     [2] TO_HOST_A==1
    RESERVED7[7]                 - (RO) Reserved bits
    DEST_CHK_V1[15..8]           - (RW) DMAD ver 1 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] UN==1
                                     [2] C==1
                                     [3] TO_HOST==1
                                     [4] CS_STATUS[0]==1
                                     [5] CS_STATUS[1]==1
                                     [6] CS_STATUS[2]==1
                                     [7] CS_STATUS[3]==1
    SCH_REQ_ANY_TIME[16]         - (RW) Assert scheduler request signal any time
    RESERVED17[17]               - (RO) Reserved bits
    DMAD_CLR_PPE_FLD[19..18]     - (RW) Clear DMAD PPE Related Field
                                     [0] Enable, clear DMAD PPE_VLD/PPE_ENTRY/CRSN Field to 0
                                     [1] Clear all DMAD (if set to 0, clear 1st DMAD for scatter/gather packet)
    DMAD_MOD_EN[26..20]          - (RW) Enable DMAD modification feature
                                     [0] PPE_VLD
                                     [1] PPE_ENTRY
                                     [2] CRSN
                                     [3] DDONE
                                     [4] LS
                                     [5] RING_INFO
                                     [6] TO_HOST
    DMAD_MOD_METHOD[27]          - (RW) modification method of DMAD modification feature
    HOST_RING_SRC[29..28]        - (RW) To host path ring info source selection
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_IGRS1_CFG0_HOST_RING_SRC_ADDR            WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_HOST_RING_SRC_MASK            0x30000000                // HOST_RING_SRC[29..28]
#define WED_RTQM_IGRS1_CFG0_HOST_RING_SRC_SHFT            28
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_METHOD_ADDR          WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_METHOD_MASK          0x08000000                // DMAD_MOD_METHOD[27]
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_METHOD_SHFT          27
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_EN_ADDR              WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_EN_MASK              0x07F00000                // DMAD_MOD_EN[26..20]
#define WED_RTQM_IGRS1_CFG0_DMAD_MOD_EN_SHFT              20
#define WED_RTQM_IGRS1_CFG0_DMAD_CLR_PPE_FLD_ADDR         WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DMAD_CLR_PPE_FLD_MASK         0x000C0000                // DMAD_CLR_PPE_FLD[19..18]
#define WED_RTQM_IGRS1_CFG0_DMAD_CLR_PPE_FLD_SHFT         18
#define WED_RTQM_IGRS1_CFG0_SCH_REQ_ANY_TIME_ADDR         WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_SCH_REQ_ANY_TIME_MASK         0x00010000                // SCH_REQ_ANY_TIME[16]
#define WED_RTQM_IGRS1_CFG0_SCH_REQ_ANY_TIME_SHFT         16
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V1_ADDR              WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V1_MASK              0x0000FF00                // DEST_CHK_V1[15..8]
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V1_SHFT              8
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V0_ADDR              WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V0_MASK              0x00000070                // DEST_CHK_V0[6..4]
#define WED_RTQM_IGRS1_CFG0_DEST_CHK_V0_SHFT              4
#define WED_RTQM_IGRS1_CFG0_FORCE_DROP_ADDR               WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_FORCE_DROP_MASK               0x00000008                // FORCE_DROP[3]
#define WED_RTQM_IGRS1_CFG0_FORCE_DROP_SHFT               3
#define WED_RTQM_IGRS1_CFG0_DEST_FORCE_ADDR               WED_RTQM_IGRS1_CFG0_ADDR
#define WED_RTQM_IGRS1_CFG0_DEST_FORCE_MASK               0x00000007                // DEST_FORCE[2..0]
#define WED_RTQM_IGRS1_CFG0_DEST_FORCE_SHFT               0

/* =====================================================================================

  ---WED_RTQM_IGRS1_CFG1 (0x15010000 + 0xb40)---

    RESERVED0[5..0]              - (RO) Reserved bits
    DMAD_MOD_TO_HOST[6]          - (RW) DMAD TO_HOST field value of DMAD modification feature
    DMAD_MOD_RING_INFO[8..7]     - (RW) DMAD RING_INFO field value of DMAD modification feature
    DMAD_MOD_LS[9]               - (RW) DMAD LS field value of DMAD modification feature
    DMAD_MOD_DDONE[10]           - (RW) DMAD DDONE field value of DMAD modification feature
    DMAD_MOD_CRSN[15..11]        - (RW) DMAD CRSN field value of DMAD modification feature
    DMAD_MOD_PPE_ENTRY[30..16]   - (RW) DMAD PPE_ENTRY field value of DMAD modification feature
    DMAD_MOD_PPE_VLD[31]         - (RW) DMAD PPE_VLD field value of DMAD modification feature

 =====================================================================================*/
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_VLD_ADDR         WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_VLD_MASK         0x80000000                // DMAD_MOD_PPE_VLD[31]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_VLD_SHFT         31
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_ENTRY_ADDR       WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_ENTRY_MASK       0x7FFF0000                // DMAD_MOD_PPE_ENTRY[30..16]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_PPE_ENTRY_SHFT       16
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_CRSN_ADDR            WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_CRSN_MASK            0x0000F800                // DMAD_MOD_CRSN[15..11]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_CRSN_SHFT            11
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_DDONE_ADDR           WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_DDONE_MASK           0x00000400                // DMAD_MOD_DDONE[10]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_DDONE_SHFT           10
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_LS_ADDR              WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_LS_MASK              0x00000200                // DMAD_MOD_LS[9]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_LS_SHFT              9
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_RING_INFO_ADDR       WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_RING_INFO_MASK       0x00000180                // DMAD_MOD_RING_INFO[8..7]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_RING_INFO_SHFT       7
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_TO_HOST_ADDR         WED_RTQM_IGRS1_CFG1_ADDR
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_TO_HOST_MASK         0x00000040                // DMAD_MOD_TO_HOST[6]
#define WED_RTQM_IGRS1_CFG1_DMAD_MOD_TO_HOST_SHFT         6

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2HW_DMAD_CNT (0x15010000 + 0xb44)---

    CNT[31..0]                   - (RC) IGRS to HW DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2HW_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS1_I2HW_DMAD_CNT_ADDR
#define WED_RTQM_IGRS1_I2HW_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2HW_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2H0_DMAD_CNT (0x15010000 + 0xb48)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2H0_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS1_I2H0_DMAD_CNT_ADDR
#define WED_RTQM_IGRS1_I2H0_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2H0_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2H1_DMAD_CNT (0x15010000 + 0xb4c)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2H1_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS1_I2H1_DMAD_CNT_ADDR
#define WED_RTQM_IGRS1_I2H1_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2H1_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2HW_PKT_CNT (0x15010000 + 0xb50)---

    CNT[31..0]                   - (RC) IGRS to RX BM Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2HW_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS1_I2HW_PKT_CNT_ADDR
#define WED_RTQM_IGRS1_I2HW_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2HW_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2H0_PKT_CNT (0x15010000 + 0xb54)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2H0_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS1_I2H0_PKT_CNT_ADDR
#define WED_RTQM_IGRS1_I2H0_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2H0_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS1_I2H1_PKT_CNT (0x15010000 + 0xb58)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_I2H1_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS1_I2H1_PKT_CNT_ADDR
#define WED_RTQM_IGRS1_I2H1_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_I2H1_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS1_FDROP_CNT (0x15010000 + 0xb5c)---

    CNT[31..0]                   - (RC) IGRS Force Drop DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS1_FDROP_CNT_CNT_ADDR                 WED_RTQM_IGRS1_FDROP_CNT_ADDR
#define WED_RTQM_IGRS1_FDROP_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS1_FDROP_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_IGRS1_DBG (0x15010000 + 0xb60)---

    CURR_DMAD_EOP_MON[0]         - (RO) Current DMAD EOP
    CURR_DMAD_SOP_MON[1]         - (RO) Current DMAD SOP
    NEXT_DMAD_SOP_MON[2]         - (RO) Next DMAD SOP
    XOP_CLR[3]                   - (A0) Clear current DMAD SOP
    SCH_RLS_SET[4]               - (RW) Force scheduler release signal
                                     Set scheduler release signal to 1.
    SCH_RLS_MANUAL[5]            - (A0) Assert scheduler release signal manually
                                     Assert scheduler release signal 1 times manually.
    SCH_REQ_MON[8..6]            - (RO) Scheduler request signal monitor
    SCH_RESULT_MON[11..9]        - (RO) Scheduler result signal monitor
    SCH_RESULT_RLS_MON[14..12]   - (RO) Scheduler result release signal monitor
    SEQ_ID_ENABLE[15]            - (RW) Insert DMAD debug sequence ID
                                     Insert sequence ID to DMAD PPE_ENTRY field.
    SEQ_ID_MON[30..16]           - (RO) DMAD debug sequence ID monitor
    SEQ_ID_CLR[31]               - (A0) Clear DMAD debug sequence ID

 =====================================================================================*/
#define WED_RTQM_IGRS1_DBG_SEQ_ID_CLR_ADDR                WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SEQ_ID_CLR_MASK                0x80000000                // SEQ_ID_CLR[31]
#define WED_RTQM_IGRS1_DBG_SEQ_ID_CLR_SHFT                31
#define WED_RTQM_IGRS1_DBG_SEQ_ID_MON_ADDR                WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SEQ_ID_MON_MASK                0x7FFF0000                // SEQ_ID_MON[30..16]
#define WED_RTQM_IGRS1_DBG_SEQ_ID_MON_SHFT                16
#define WED_RTQM_IGRS1_DBG_SEQ_ID_ENABLE_ADDR             WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SEQ_ID_ENABLE_MASK             0x00008000                // SEQ_ID_ENABLE[15]
#define WED_RTQM_IGRS1_DBG_SEQ_ID_ENABLE_SHFT             15
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_RLS_MON_ADDR        WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_RLS_MON_MASK        0x00007000                // SCH_RESULT_RLS_MON[14..12]
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_RLS_MON_SHFT        12
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_MON_ADDR            WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_MON_MASK            0x00000E00                // SCH_RESULT_MON[11..9]
#define WED_RTQM_IGRS1_DBG_SCH_RESULT_MON_SHFT            9
#define WED_RTQM_IGRS1_DBG_SCH_REQ_MON_ADDR               WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SCH_REQ_MON_MASK               0x000001C0                // SCH_REQ_MON[8..6]
#define WED_RTQM_IGRS1_DBG_SCH_REQ_MON_SHFT               6
#define WED_RTQM_IGRS1_DBG_SCH_RLS_MANUAL_ADDR            WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SCH_RLS_MANUAL_MASK            0x00000020                // SCH_RLS_MANUAL[5]
#define WED_RTQM_IGRS1_DBG_SCH_RLS_MANUAL_SHFT            5
#define WED_RTQM_IGRS1_DBG_SCH_RLS_SET_ADDR               WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_SCH_RLS_SET_MASK               0x00000010                // SCH_RLS_SET[4]
#define WED_RTQM_IGRS1_DBG_SCH_RLS_SET_SHFT               4
#define WED_RTQM_IGRS1_DBG_XOP_CLR_ADDR                   WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_XOP_CLR_MASK                   0x00000008                // XOP_CLR[3]
#define WED_RTQM_IGRS1_DBG_XOP_CLR_SHFT                   3
#define WED_RTQM_IGRS1_DBG_NEXT_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_NEXT_DMAD_SOP_MON_MASK         0x00000004                // NEXT_DMAD_SOP_MON[2]
#define WED_RTQM_IGRS1_DBG_NEXT_DMAD_SOP_MON_SHFT         2
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_SOP_MON_MASK         0x00000002                // CURR_DMAD_SOP_MON[1]
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_SOP_MON_SHFT         1
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_EOP_MON_ADDR         WED_RTQM_IGRS1_DBG_ADDR
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_EOP_MON_MASK         0x00000001                // CURR_DMAD_EOP_MON[0]
#define WED_RTQM_IGRS1_DBG_CURR_DMAD_EOP_MON_SHFT         0

/* =====================================================================================

  ---WED_RTQM_IGRS2_CFG0 (0x15010000 + 0xb64)---

    DEST_FORCE[2..0]             - (RW) Force path control
    FORCE_DROP[3]                - (RW) Force Drop all DMAD
    DEST_CHK_V0[6..4]            - (RW) DMAD ver 0 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] TO_HOST==1
                                     [2] TO_HOST_A==1
    RESERVED7[7]                 - (RO) Reserved bits
    DEST_CHK_V1[15..8]           - (RW) DMAD ver 1 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] UN==1
                                     [2] C==1
                                     [3] TO_HOST==1
                                     [4] CS_STATUS[0]==1
                                     [5] CS_STATUS[1]==1
                                     [6] CS_STATUS[2]==1
                                     [7] CS_STATUS[3]==1
    SCH_REQ_ANY_TIME[16]         - (RW) Assert scheduler request signal any time
    RESERVED17[17]               - (RO) Reserved bits
    DMAD_CLR_PPE_FLD[19..18]     - (RW) Clear DMAD PPE Related Field
                                     [0] Enable, clear DMAD PPE_VLD/PPE_ENTRY/CRSN Field to 0
                                     [1] Clear all DMAD (if set to 0, clear 1st DMAD for scatter/gather packet)
    DMAD_MOD_EN[26..20]          - (RW) Enable DMAD modification feature
                                     [0] PPE_VLD
                                     [1] PPE_ENTRY
                                     [2] CRSN
                                     [3] DDONE
                                     [4] LS
                                     [5] RING_INFO
                                     [6] TO_HOST
    DMAD_MOD_METHOD[27]          - (RW) modification method of DMAD modification feature
    HOST_RING_SRC[29..28]        - (RW) To host path ring info source selection
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_IGRS2_CFG0_HOST_RING_SRC_ADDR            WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_HOST_RING_SRC_MASK            0x30000000                // HOST_RING_SRC[29..28]
#define WED_RTQM_IGRS2_CFG0_HOST_RING_SRC_SHFT            28
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_METHOD_ADDR          WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_METHOD_MASK          0x08000000                // DMAD_MOD_METHOD[27]
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_METHOD_SHFT          27
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_EN_ADDR              WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_EN_MASK              0x07F00000                // DMAD_MOD_EN[26..20]
#define WED_RTQM_IGRS2_CFG0_DMAD_MOD_EN_SHFT              20
#define WED_RTQM_IGRS2_CFG0_DMAD_CLR_PPE_FLD_ADDR         WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DMAD_CLR_PPE_FLD_MASK         0x000C0000                // DMAD_CLR_PPE_FLD[19..18]
#define WED_RTQM_IGRS2_CFG0_DMAD_CLR_PPE_FLD_SHFT         18
#define WED_RTQM_IGRS2_CFG0_SCH_REQ_ANY_TIME_ADDR         WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_SCH_REQ_ANY_TIME_MASK         0x00010000                // SCH_REQ_ANY_TIME[16]
#define WED_RTQM_IGRS2_CFG0_SCH_REQ_ANY_TIME_SHFT         16
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V1_ADDR              WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V1_MASK              0x0000FF00                // DEST_CHK_V1[15..8]
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V1_SHFT              8
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V0_ADDR              WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V0_MASK              0x00000070                // DEST_CHK_V0[6..4]
#define WED_RTQM_IGRS2_CFG0_DEST_CHK_V0_SHFT              4
#define WED_RTQM_IGRS2_CFG0_FORCE_DROP_ADDR               WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_FORCE_DROP_MASK               0x00000008                // FORCE_DROP[3]
#define WED_RTQM_IGRS2_CFG0_FORCE_DROP_SHFT               3
#define WED_RTQM_IGRS2_CFG0_DEST_FORCE_ADDR               WED_RTQM_IGRS2_CFG0_ADDR
#define WED_RTQM_IGRS2_CFG0_DEST_FORCE_MASK               0x00000007                // DEST_FORCE[2..0]
#define WED_RTQM_IGRS2_CFG0_DEST_FORCE_SHFT               0

/* =====================================================================================

  ---WED_RTQM_IGRS2_CFG1 (0x15010000 + 0xb68)---

    RESERVED0[5..0]              - (RO) Reserved bits
    DMAD_MOD_TO_HOST[6]          - (RW) DMAD TO_HOST field value of DMAD modification feature
    DMAD_MOD_RING_INFO[8..7]     - (RW) DMAD RING_INFO field value of DMAD modification feature
    DMAD_MOD_LS[9]               - (RW) DMAD LS field value of DMAD modification feature
    DMAD_MOD_DDONE[10]           - (RW) DMAD DDONE field value of DMAD modification feature
    DMAD_MOD_CRSN[15..11]        - (RW) DMAD CRSN field value of DMAD modification feature
    DMAD_MOD_PPE_ENTRY[30..16]   - (RW) DMAD PPE_ENTRY field value of DMAD modification feature
    DMAD_MOD_PPE_VLD[31]         - (RW) DMAD PPE_VLD field value of DMAD modification feature

 =====================================================================================*/
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_VLD_ADDR         WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_VLD_MASK         0x80000000                // DMAD_MOD_PPE_VLD[31]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_VLD_SHFT         31
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_ENTRY_ADDR       WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_ENTRY_MASK       0x7FFF0000                // DMAD_MOD_PPE_ENTRY[30..16]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_PPE_ENTRY_SHFT       16
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_CRSN_ADDR            WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_CRSN_MASK            0x0000F800                // DMAD_MOD_CRSN[15..11]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_CRSN_SHFT            11
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_DDONE_ADDR           WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_DDONE_MASK           0x00000400                // DMAD_MOD_DDONE[10]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_DDONE_SHFT           10
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_LS_ADDR              WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_LS_MASK              0x00000200                // DMAD_MOD_LS[9]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_LS_SHFT              9
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_RING_INFO_ADDR       WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_RING_INFO_MASK       0x00000180                // DMAD_MOD_RING_INFO[8..7]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_RING_INFO_SHFT       7
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_TO_HOST_ADDR         WED_RTQM_IGRS2_CFG1_ADDR
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_TO_HOST_MASK         0x00000040                // DMAD_MOD_TO_HOST[6]
#define WED_RTQM_IGRS2_CFG1_DMAD_MOD_TO_HOST_SHFT         6

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2HW_DMAD_CNT (0x15010000 + 0xb6c)---

    CNT[31..0]                   - (RC) IGRS to HW DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2HW_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS2_I2HW_DMAD_CNT_ADDR
#define WED_RTQM_IGRS2_I2HW_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2HW_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2H0_DMAD_CNT (0x15010000 + 0xb70)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2H0_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS2_I2H0_DMAD_CNT_ADDR
#define WED_RTQM_IGRS2_I2H0_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2H0_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2H1_DMAD_CNT (0x15010000 + 0xb74)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2H1_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS2_I2H1_DMAD_CNT_ADDR
#define WED_RTQM_IGRS2_I2H1_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2H1_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2HW_PKT_CNT (0x15010000 + 0xb78)---

    CNT[31..0]                   - (RC) IGRS to RX BM Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2HW_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS2_I2HW_PKT_CNT_ADDR
#define WED_RTQM_IGRS2_I2HW_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2HW_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2H0_PKT_CNT (0x15010000 + 0xb7c)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2H0_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS2_I2H0_PKT_CNT_ADDR
#define WED_RTQM_IGRS2_I2H0_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2H0_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS2_I2H1_PKT_CNT (0x15010000 + 0xb80)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_I2H1_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS2_I2H1_PKT_CNT_ADDR
#define WED_RTQM_IGRS2_I2H1_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_I2H1_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS2_FDROP_CNT (0x15010000 + 0xb84)---

    CNT[31..0]                   - (RC) IGRS Force Drop DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS2_FDROP_CNT_CNT_ADDR                 WED_RTQM_IGRS2_FDROP_CNT_ADDR
#define WED_RTQM_IGRS2_FDROP_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS2_FDROP_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_IGRS2_DBG (0x15010000 + 0xb88)---

    CURR_DMAD_EOP_MON[0]         - (RO) Current DMAD EOP
    CURR_DMAD_SOP_MON[1]         - (RO) Current DMAD SOP
    NEXT_DMAD_SOP_MON[2]         - (RO) Next DMAD SOP
    XOP_CLR[3]                   - (A0) Clear current DMAD SOP
    SCH_RLS_SET[4]               - (RW) Force scheduler release signal
                                     Set scheduler release signal to 1.
    SCH_RLS_MANUAL[5]            - (A0) Assert scheduler release signal manually
                                     Assert scheduler release signal 1 times manually.
    SCH_REQ_MON[8..6]            - (RO) Scheduler request signal monitor
    SCH_RESULT_MON[11..9]        - (RO) Scheduler result signal monitor
    SCH_RESULT_RLS_MON[14..12]   - (RO) Scheduler result release signal monitor
    SEQ_ID_ENABLE[15]            - (RW) Insert DMAD debug sequence ID
                                     Insert sequence ID to DMAD PPE_ENTRY field.
    SEQ_ID_MON[30..16]           - (RO) DMAD debug sequence ID monitor
    SEQ_ID_CLR[31]               - (A0) Clear DMAD debug sequence ID

 =====================================================================================*/
#define WED_RTQM_IGRS2_DBG_SEQ_ID_CLR_ADDR                WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SEQ_ID_CLR_MASK                0x80000000                // SEQ_ID_CLR[31]
#define WED_RTQM_IGRS2_DBG_SEQ_ID_CLR_SHFT                31
#define WED_RTQM_IGRS2_DBG_SEQ_ID_MON_ADDR                WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SEQ_ID_MON_MASK                0x7FFF0000                // SEQ_ID_MON[30..16]
#define WED_RTQM_IGRS2_DBG_SEQ_ID_MON_SHFT                16
#define WED_RTQM_IGRS2_DBG_SEQ_ID_ENABLE_ADDR             WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SEQ_ID_ENABLE_MASK             0x00008000                // SEQ_ID_ENABLE[15]
#define WED_RTQM_IGRS2_DBG_SEQ_ID_ENABLE_SHFT             15
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_RLS_MON_ADDR        WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_RLS_MON_MASK        0x00007000                // SCH_RESULT_RLS_MON[14..12]
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_RLS_MON_SHFT        12
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_MON_ADDR            WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_MON_MASK            0x00000E00                // SCH_RESULT_MON[11..9]
#define WED_RTQM_IGRS2_DBG_SCH_RESULT_MON_SHFT            9
#define WED_RTQM_IGRS2_DBG_SCH_REQ_MON_ADDR               WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SCH_REQ_MON_MASK               0x000001C0                // SCH_REQ_MON[8..6]
#define WED_RTQM_IGRS2_DBG_SCH_REQ_MON_SHFT               6
#define WED_RTQM_IGRS2_DBG_SCH_RLS_MANUAL_ADDR            WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SCH_RLS_MANUAL_MASK            0x00000020                // SCH_RLS_MANUAL[5]
#define WED_RTQM_IGRS2_DBG_SCH_RLS_MANUAL_SHFT            5
#define WED_RTQM_IGRS2_DBG_SCH_RLS_SET_ADDR               WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_SCH_RLS_SET_MASK               0x00000010                // SCH_RLS_SET[4]
#define WED_RTQM_IGRS2_DBG_SCH_RLS_SET_SHFT               4
#define WED_RTQM_IGRS2_DBG_XOP_CLR_ADDR                   WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_XOP_CLR_MASK                   0x00000008                // XOP_CLR[3]
#define WED_RTQM_IGRS2_DBG_XOP_CLR_SHFT                   3
#define WED_RTQM_IGRS2_DBG_NEXT_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_NEXT_DMAD_SOP_MON_MASK         0x00000004                // NEXT_DMAD_SOP_MON[2]
#define WED_RTQM_IGRS2_DBG_NEXT_DMAD_SOP_MON_SHFT         2
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_SOP_MON_MASK         0x00000002                // CURR_DMAD_SOP_MON[1]
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_SOP_MON_SHFT         1
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_EOP_MON_ADDR         WED_RTQM_IGRS2_DBG_ADDR
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_EOP_MON_MASK         0x00000001                // CURR_DMAD_EOP_MON[0]
#define WED_RTQM_IGRS2_DBG_CURR_DMAD_EOP_MON_SHFT         0

/* =====================================================================================

  ---WED_RTQM_IGRS3_CFG0 (0x15010000 + 0xb8c)---

    DEST_FORCE[2..0]             - (RW) Force path control
    FORCE_DROP[3]                - (RW) Force Drop all DMAD
    DEST_CHK_V0[6..4]            - (RW) DMAD ver 0 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] TO_HOST==1
                                     [2] TO_HOST_A==1
    RESERVED7[7]                 - (RO) Reserved bits
    DEST_CHK_V1[15..8]           - (RW) DMAD ver 1 destination to host check conditions
                                     [0] scatter-gather packet
                                     [1] UN==1
                                     [2] C==1
                                     [3] TO_HOST==1
                                     [4] CS_STATUS[0]==1
                                     [5] CS_STATUS[1]==1
                                     [6] CS_STATUS[2]==1
                                     [7] CS_STATUS[3]==1
    SCH_REQ_ANY_TIME[16]         - (RW) Assert scheduler request signal any time
    RESERVED17[17]               - (RO) Reserved bits
    DMAD_CLR_PPE_FLD[19..18]     - (RW) Clear DMAD PPE Related Field
                                     [0] Enable, clear DMAD PPE_VLD/PPE_ENTRY/CRSN Field to 0
                                     [1] Clear all DMAD (if set to 0, clear 1st DMAD for scatter/gather packet)
    DMAD_MOD_EN[26..20]          - (RW) Enable DMAD modification feature
                                     [0] PPE_VLD
                                     [1] PPE_ENTRY
                                     [2] CRSN
                                     [3] DDONE
                                     [4] LS
                                     [5] RING_INFO
                                     [6] TO_HOST
    DMAD_MOD_METHOD[27]          - (RW) modification method of DMAD modification feature
    HOST_RING_SRC[29..28]        - (RW) To host path ring info source selection
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_IGRS3_CFG0_HOST_RING_SRC_ADDR            WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_HOST_RING_SRC_MASK            0x30000000                // HOST_RING_SRC[29..28]
#define WED_RTQM_IGRS3_CFG0_HOST_RING_SRC_SHFT            28
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_METHOD_ADDR          WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_METHOD_MASK          0x08000000                // DMAD_MOD_METHOD[27]
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_METHOD_SHFT          27
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_EN_ADDR              WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_EN_MASK              0x07F00000                // DMAD_MOD_EN[26..20]
#define WED_RTQM_IGRS3_CFG0_DMAD_MOD_EN_SHFT              20
#define WED_RTQM_IGRS3_CFG0_DMAD_CLR_PPE_FLD_ADDR         WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DMAD_CLR_PPE_FLD_MASK         0x000C0000                // DMAD_CLR_PPE_FLD[19..18]
#define WED_RTQM_IGRS3_CFG0_DMAD_CLR_PPE_FLD_SHFT         18
#define WED_RTQM_IGRS3_CFG0_SCH_REQ_ANY_TIME_ADDR         WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_SCH_REQ_ANY_TIME_MASK         0x00010000                // SCH_REQ_ANY_TIME[16]
#define WED_RTQM_IGRS3_CFG0_SCH_REQ_ANY_TIME_SHFT         16
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V1_ADDR              WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V1_MASK              0x0000FF00                // DEST_CHK_V1[15..8]
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V1_SHFT              8
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V0_ADDR              WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V0_MASK              0x00000070                // DEST_CHK_V0[6..4]
#define WED_RTQM_IGRS3_CFG0_DEST_CHK_V0_SHFT              4
#define WED_RTQM_IGRS3_CFG0_FORCE_DROP_ADDR               WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_FORCE_DROP_MASK               0x00000008                // FORCE_DROP[3]
#define WED_RTQM_IGRS3_CFG0_FORCE_DROP_SHFT               3
#define WED_RTQM_IGRS3_CFG0_DEST_FORCE_ADDR               WED_RTQM_IGRS3_CFG0_ADDR
#define WED_RTQM_IGRS3_CFG0_DEST_FORCE_MASK               0x00000007                // DEST_FORCE[2..0]
#define WED_RTQM_IGRS3_CFG0_DEST_FORCE_SHFT               0

/* =====================================================================================

  ---WED_RTQM_IGRS3_CFG1 (0x15010000 + 0xb90)---

    RESERVED0[5..0]              - (RO) Reserved bits
    DMAD_MOD_TO_HOST[6]          - (RW) DMAD TO_HOST field value of DMAD modification feature
    DMAD_MOD_RING_INFO[8..7]     - (RW) DMAD RING_INFO field value of DMAD modification feature
    DMAD_MOD_LS[9]               - (RW) DMAD LS field value of DMAD modification feature
    DMAD_MOD_DDONE[10]           - (RW) DMAD DDONE field value of DMAD modification feature
    DMAD_MOD_CRSN[15..11]        - (RW) DMAD CRSN field value of DMAD modification feature
    DMAD_MOD_PPE_ENTRY[30..16]   - (RW) DMAD PPE_ENTRY field value of DMAD modification feature
    DMAD_MOD_PPE_VLD[31]         - (RW) DMAD PPE_VLD field value of DMAD modification feature

 =====================================================================================*/
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_VLD_ADDR         WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_VLD_MASK         0x80000000                // DMAD_MOD_PPE_VLD[31]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_VLD_SHFT         31
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_ENTRY_ADDR       WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_ENTRY_MASK       0x7FFF0000                // DMAD_MOD_PPE_ENTRY[30..16]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_PPE_ENTRY_SHFT       16
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_CRSN_ADDR            WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_CRSN_MASK            0x0000F800                // DMAD_MOD_CRSN[15..11]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_CRSN_SHFT            11
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_DDONE_ADDR           WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_DDONE_MASK           0x00000400                // DMAD_MOD_DDONE[10]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_DDONE_SHFT           10
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_LS_ADDR              WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_LS_MASK              0x00000200                // DMAD_MOD_LS[9]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_LS_SHFT              9
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_RING_INFO_ADDR       WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_RING_INFO_MASK       0x00000180                // DMAD_MOD_RING_INFO[8..7]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_RING_INFO_SHFT       7
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_TO_HOST_ADDR         WED_RTQM_IGRS3_CFG1_ADDR
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_TO_HOST_MASK         0x00000040                // DMAD_MOD_TO_HOST[6]
#define WED_RTQM_IGRS3_CFG1_DMAD_MOD_TO_HOST_SHFT         6

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2HW_DMAD_CNT (0x15010000 + 0xb94)---

    CNT[31..0]                   - (RC) IGRS to HW DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2HW_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS3_I2HW_DMAD_CNT_ADDR
#define WED_RTQM_IGRS3_I2HW_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2HW_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2H0_DMAD_CNT (0x15010000 + 0xb98)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2H0_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS3_I2H0_DMAD_CNT_ADDR
#define WED_RTQM_IGRS3_I2H0_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2H0_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2H1_DMAD_CNT (0x15010000 + 0xb9c)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2H1_DMAD_CNT_CNT_ADDR             WED_RTQM_IGRS3_I2H1_DMAD_CNT_ADDR
#define WED_RTQM_IGRS3_I2H1_DMAD_CNT_CNT_MASK             0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2H1_DMAD_CNT_CNT_SHFT             0

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2HW_PKT_CNT (0x15010000 + 0xba0)---

    CNT[31..0]                   - (RC) IGRS to HW Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2HW_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS3_I2HW_PKT_CNT_ADDR
#define WED_RTQM_IGRS3_I2HW_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2HW_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2H0_PKT_CNT (0x15010000 + 0xba4)---

    CNT[31..0]                   - (RC) IGRS to Host Ring0 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2H0_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS3_I2H0_PKT_CNT_ADDR
#define WED_RTQM_IGRS3_I2H0_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2H0_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS3_I2H1_PKT_CNT (0x15010000 + 0xba8)---

    CNT[31..0]                   - (RC) IGRS to Host Ring1 Packet Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_I2H1_PKT_CNT_CNT_ADDR              WED_RTQM_IGRS3_I2H1_PKT_CNT_ADDR
#define WED_RTQM_IGRS3_I2H1_PKT_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_I2H1_PKT_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_IGRS3_FDROP_CNT (0x15010000 + 0xbac)---

    CNT[31..0]                   - (RC) IGRS Force Drop DMAD Counter

 =====================================================================================*/
#define WED_RTQM_IGRS3_FDROP_CNT_CNT_ADDR                 WED_RTQM_IGRS3_FDROP_CNT_ADDR
#define WED_RTQM_IGRS3_FDROP_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_IGRS3_FDROP_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_IGRS3_DBG (0x15010000 + 0xbb0)---

    CURR_DMAD_EOP_MON[0]         - (RO) Current DMAD EOP
    CURR_DMAD_SOP_MON[1]         - (RO) Current DMAD SOP
    NEXT_DMAD_SOP_MON[2]         - (RO) Next DMAD SOP
    XOP_CLR[3]                   - (A0) Clear current DMAD SOP
    SCH_RLS_SET[4]               - (RW) Force scheduler release signal
                                     Set scheduler release signal to 1.
    SCH_RLS_MANUAL[5]            - (A0) Assert scheduler release signal manually
                                     Assert scheduler release signal 1 times manually.
    SCH_REQ_MON[8..6]            - (RO) Scheduler request signal monitor
    SCH_RESULT_MON[11..9]        - (RO) Scheduler result signal monitor
    SCH_RESULT_RLS_MON[14..12]   - (RO) Scheduler result release signal monitor
    SEQ_ID_ENABLE[15]            - (RW) Insert DMAD debug sequence ID
                                     Insert sequence ID to DMAD PPE_ENTRY field.
    SEQ_ID_MON[30..16]           - (RO) DMAD debug sequence ID monitor
    SEQ_ID_CLR[31]               - (A0) Clear DMAD debug sequence ID

 =====================================================================================*/
#define WED_RTQM_IGRS3_DBG_SEQ_ID_CLR_ADDR                WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SEQ_ID_CLR_MASK                0x80000000                // SEQ_ID_CLR[31]
#define WED_RTQM_IGRS3_DBG_SEQ_ID_CLR_SHFT                31
#define WED_RTQM_IGRS3_DBG_SEQ_ID_MON_ADDR                WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SEQ_ID_MON_MASK                0x7FFF0000                // SEQ_ID_MON[30..16]
#define WED_RTQM_IGRS3_DBG_SEQ_ID_MON_SHFT                16
#define WED_RTQM_IGRS3_DBG_SEQ_ID_ENABLE_ADDR             WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SEQ_ID_ENABLE_MASK             0x00008000                // SEQ_ID_ENABLE[15]
#define WED_RTQM_IGRS3_DBG_SEQ_ID_ENABLE_SHFT             15
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_RLS_MON_ADDR        WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_RLS_MON_MASK        0x00007000                // SCH_RESULT_RLS_MON[14..12]
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_RLS_MON_SHFT        12
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_MON_ADDR            WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_MON_MASK            0x00000E00                // SCH_RESULT_MON[11..9]
#define WED_RTQM_IGRS3_DBG_SCH_RESULT_MON_SHFT            9
#define WED_RTQM_IGRS3_DBG_SCH_REQ_MON_ADDR               WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SCH_REQ_MON_MASK               0x000001C0                // SCH_REQ_MON[8..6]
#define WED_RTQM_IGRS3_DBG_SCH_REQ_MON_SHFT               6
#define WED_RTQM_IGRS3_DBG_SCH_RLS_MANUAL_ADDR            WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SCH_RLS_MANUAL_MASK            0x00000020                // SCH_RLS_MANUAL[5]
#define WED_RTQM_IGRS3_DBG_SCH_RLS_MANUAL_SHFT            5
#define WED_RTQM_IGRS3_DBG_SCH_RLS_SET_ADDR               WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_SCH_RLS_SET_MASK               0x00000010                // SCH_RLS_SET[4]
#define WED_RTQM_IGRS3_DBG_SCH_RLS_SET_SHFT               4
#define WED_RTQM_IGRS3_DBG_XOP_CLR_ADDR                   WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_XOP_CLR_MASK                   0x00000008                // XOP_CLR[3]
#define WED_RTQM_IGRS3_DBG_XOP_CLR_SHFT                   3
#define WED_RTQM_IGRS3_DBG_NEXT_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_NEXT_DMAD_SOP_MON_MASK         0x00000004                // NEXT_DMAD_SOP_MON[2]
#define WED_RTQM_IGRS3_DBG_NEXT_DMAD_SOP_MON_SHFT         2
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_SOP_MON_ADDR         WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_SOP_MON_MASK         0x00000002                // CURR_DMAD_SOP_MON[1]
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_SOP_MON_SHFT         1
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_EOP_MON_ADDR         WED_RTQM_IGRS3_DBG_ADDR
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_EOP_MON_MASK         0x00000001                // CURR_DMAD_EOP_MON[0]
#define WED_RTQM_IGRS3_DBG_CURR_DMAD_EOP_MON_SHFT         0

/* =====================================================================================

  ---WED_RTQM_EGRS_WEIGHT (0x15010000 + 0xbb4)---

    EGRS0_ITF0_WEIGHT[1..0]      - (RW) Weight of EGRS0 Interface 0
    EGRS0_ITF1_WEIGHT[3..2]      - (RW) Weight of EGRS0 Interface 1
    EGRS0_ITF2_WEIGHT[5..4]      - (RW) Weight of EGRS0 Interface 2
    EGRS0_ITF3_WEIGHT[7..6]      - (RW) Weight of EGRS0 Interface 3
    EGRS1_ITF0_WEIGHT[9..8]      - (RW) Weight of EGRS1 Interface 0
    EGRS1_ITF1_WEIGHT[11..10]    - (RW) Weight of EGRS1 Interface 1
    EGRS1_ITF2_WEIGHT[13..12]    - (RW) Weight of EGRS1 Interface 2
    EGRS1_ITF3_WEIGHT[15..14]    - (RW) Weight of EGRS1 Interface 3
    EGRS2_ITF0_WEIGHT[17..16]    - (RW) Weight of EGRS2 Interface 0
    EGRS2_ITF1_WEIGHT[19..18]    - (RW) Weight of EGRS2 Interface 1
    EGRS2_ITF2_WEIGHT[21..20]    - (RW) Weight of EGRS2 Interface 2
    EGRS2_ITF3_WEIGHT[23..22]    - (RW) Weight of EGRS2 Interface 3
    EGRS3_ITF0_WEIGHT[25..24]    - (RW) Weight of EGRS3 Interface 0
    EGRS3_ITF1_WEIGHT[27..26]    - (RW) Weight of EGRS3 Interface 1
    EGRS3_ITF2_WEIGHT[29..28]    - (RW) Weight of EGRS3 Interface 2
    EGRS3_ITF3_WEIGHT[31..30]    - (RW) Weight of EGRS3 Interface 3

 =====================================================================================*/
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF3_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF3_WEIGHT_MASK       0xC0000000                // EGRS3_ITF3_WEIGHT[31..30]
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF3_WEIGHT_SHFT       30
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF2_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF2_WEIGHT_MASK       0x30000000                // EGRS3_ITF2_WEIGHT[29..28]
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF2_WEIGHT_SHFT       28
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF1_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF1_WEIGHT_MASK       0x0C000000                // EGRS3_ITF1_WEIGHT[27..26]
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF1_WEIGHT_SHFT       26
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF0_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF0_WEIGHT_MASK       0x03000000                // EGRS3_ITF0_WEIGHT[25..24]
#define WED_RTQM_EGRS_WEIGHT_EGRS3_ITF0_WEIGHT_SHFT       24
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF3_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF3_WEIGHT_MASK       0x00C00000                // EGRS2_ITF3_WEIGHT[23..22]
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF3_WEIGHT_SHFT       22
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF2_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF2_WEIGHT_MASK       0x00300000                // EGRS2_ITF2_WEIGHT[21..20]
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF2_WEIGHT_SHFT       20
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF1_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF1_WEIGHT_MASK       0x000C0000                // EGRS2_ITF1_WEIGHT[19..18]
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF1_WEIGHT_SHFT       18
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF0_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF0_WEIGHT_MASK       0x00030000                // EGRS2_ITF0_WEIGHT[17..16]
#define WED_RTQM_EGRS_WEIGHT_EGRS2_ITF0_WEIGHT_SHFT       16
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF3_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF3_WEIGHT_MASK       0x0000C000                // EGRS1_ITF3_WEIGHT[15..14]
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF3_WEIGHT_SHFT       14
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF2_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF2_WEIGHT_MASK       0x00003000                // EGRS1_ITF2_WEIGHT[13..12]
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF2_WEIGHT_SHFT       12
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF1_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF1_WEIGHT_MASK       0x00000C00                // EGRS1_ITF1_WEIGHT[11..10]
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF1_WEIGHT_SHFT       10
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF0_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF0_WEIGHT_MASK       0x00000300                // EGRS1_ITF0_WEIGHT[9..8]
#define WED_RTQM_EGRS_WEIGHT_EGRS1_ITF0_WEIGHT_SHFT       8
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF3_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF3_WEIGHT_MASK       0x000000C0                // EGRS0_ITF3_WEIGHT[7..6]
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF3_WEIGHT_SHFT       6
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF2_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF2_WEIGHT_MASK       0x00000030                // EGRS0_ITF2_WEIGHT[5..4]
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF2_WEIGHT_SHFT       4
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF1_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF1_WEIGHT_MASK       0x0000000C                // EGRS0_ITF1_WEIGHT[3..2]
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF1_WEIGHT_SHFT       2
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF0_WEIGHT_ADDR       WED_RTQM_EGRS_WEIGHT_ADDR
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF0_WEIGHT_MASK       0x00000003                // EGRS0_ITF0_WEIGHT[1..0]
#define WED_RTQM_EGRS_WEIGHT_EGRS0_ITF0_WEIGHT_SHFT       0

/* =====================================================================================

  ---WED_RTQM_ENQ_CFG0 (0x15010000 + 0xbb8)---

    FORCE_DROP[0]                - (RW) Force Drop all DMAD
    ENQ_ERR_NO_ENQ[1]            - (RW) Drop enqueue error DMAD and not to overwrite target entry in SRAM
    ENQ_ERR_AUTO_FREE[2]         - (RW) Set target entry to free when enqueue error occurs
    DMAD_TKID_INS[3]             - (RW) Insert WiFi DMAD TKID to NETSYS DMAD
                                     Insert WiFi DMAD TKID to NETSYS DMAD RSV4 field
    SEQ_ID_ENABLE[4]             - (RW) Insert NETSYS DMAD debug sequence ID
                                     Insert sequence ID to NETSYS DMAD RES6 field.
    SEQ_ID_CLR[5]                - (A0) Clear NETSYS DMAD debug sequence ID
    RXDMAD_OFST_UNIT[7..6]       - (RW) Unit of WiFi Rx DMAD Head Offset Field
    TXDMAD_OFLD[11..8]           - (RW) NETSYS WDMA Tx DMAD Offload Field
                                     Usually it is 0.
    TXDMAD_FPORT[15..12]         - (RW) NETSYS WDMA Tx DMAD Force Port Field
                                     Set this register field to Netsys PPE Port of this WED
    SEQ_ID_MON[31..16]           - (RO) NETSYS DMAD debug sequence ID monitor

 =====================================================================================*/
#define WED_RTQM_ENQ_CFG0_SEQ_ID_MON_ADDR                 WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_SEQ_ID_MON_MASK                 0xFFFF0000                // SEQ_ID_MON[31..16]
#define WED_RTQM_ENQ_CFG0_SEQ_ID_MON_SHFT                 16
#define WED_RTQM_ENQ_CFG0_TXDMAD_FPORT_ADDR               WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_TXDMAD_FPORT_MASK               0x0000F000                // TXDMAD_FPORT[15..12]
#define WED_RTQM_ENQ_CFG0_TXDMAD_FPORT_SHFT               12
#define WED_RTQM_ENQ_CFG0_TXDMAD_OFLD_ADDR                WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_TXDMAD_OFLD_MASK                0x00000F00                // TXDMAD_OFLD[11..8]
#define WED_RTQM_ENQ_CFG0_TXDMAD_OFLD_SHFT                8
#define WED_RTQM_ENQ_CFG0_RXDMAD_OFST_UNIT_ADDR           WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_RXDMAD_OFST_UNIT_MASK           0x000000C0                // RXDMAD_OFST_UNIT[7..6]
#define WED_RTQM_ENQ_CFG0_RXDMAD_OFST_UNIT_SHFT           6
#define WED_RTQM_ENQ_CFG0_SEQ_ID_CLR_ADDR                 WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_SEQ_ID_CLR_MASK                 0x00000020                // SEQ_ID_CLR[5]
#define WED_RTQM_ENQ_CFG0_SEQ_ID_CLR_SHFT                 5
#define WED_RTQM_ENQ_CFG0_SEQ_ID_ENABLE_ADDR              WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_SEQ_ID_ENABLE_MASK              0x00000010                // SEQ_ID_ENABLE[4]
#define WED_RTQM_ENQ_CFG0_SEQ_ID_ENABLE_SHFT              4
#define WED_RTQM_ENQ_CFG0_DMAD_TKID_INS_ADDR              WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_DMAD_TKID_INS_MASK              0x00000008                // DMAD_TKID_INS[3]
#define WED_RTQM_ENQ_CFG0_DMAD_TKID_INS_SHFT              3
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_AUTO_FREE_ADDR          WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_AUTO_FREE_MASK          0x00000004                // ENQ_ERR_AUTO_FREE[2]
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_AUTO_FREE_SHFT          2
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_NO_ENQ_ADDR             WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_NO_ENQ_MASK             0x00000002                // ENQ_ERR_NO_ENQ[1]
#define WED_RTQM_ENQ_CFG0_ENQ_ERR_NO_ENQ_SHFT             1
#define WED_RTQM_ENQ_CFG0_FORCE_DROP_ADDR                 WED_RTQM_ENQ_CFG0_ADDR
#define WED_RTQM_ENQ_CFG0_FORCE_DROP_MASK                 0x00000001                // FORCE_DROP[0]
#define WED_RTQM_ENQ_CFG0_FORCE_DROP_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_ENQ_I2Q_DMAD_CNT (0x15010000 + 0xbbc)---

    CNT[31..0]                   - (RC) IGRS enqueue to SRAM DMAD counter

 =====================================================================================*/
#define WED_RTQM_ENQ_I2Q_DMAD_CNT_CNT_ADDR                WED_RTQM_ENQ_I2Q_DMAD_CNT_ADDR
#define WED_RTQM_ENQ_I2Q_DMAD_CNT_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_I2Q_DMAD_CNT_CNT_SHFT                0

/* =====================================================================================

  ---WED_RTQM_ENQ_I2N_DMAD_CNT (0x15010000 + 0xbc0)---

    CNT[31..0]                   - (RC) IGRS enqueue to NETSYS DMAD counter

 =====================================================================================*/
#define WED_RTQM_ENQ_I2N_DMAD_CNT_CNT_ADDR                WED_RTQM_ENQ_I2N_DMAD_CNT_ADDR
#define WED_RTQM_ENQ_I2N_DMAD_CNT_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_I2N_DMAD_CNT_CNT_SHFT                0

/* =====================================================================================

  ---WED_RTQM_ENQ_I2Q_PKT_CNT (0x15010000 + 0xbc4)---

    CNT[31..0]                   - (RC) IGRS enqueue to SRAM packet counter

 =====================================================================================*/
#define WED_RTQM_ENQ_I2Q_PKT_CNT_CNT_ADDR                 WED_RTQM_ENQ_I2Q_PKT_CNT_ADDR
#define WED_RTQM_ENQ_I2Q_PKT_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_I2Q_PKT_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_ENQ_I2N_PKT_CNT (0x15010000 + 0xbc8)---

    CNT[31..0]                   - (RC) IGRS enqueue to NETSYS packet counter

 =====================================================================================*/
#define WED_RTQM_ENQ_I2N_PKT_CNT_CNT_ADDR                 WED_RTQM_ENQ_I2N_PKT_CNT_ADDR
#define WED_RTQM_ENQ_I2N_PKT_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_I2N_PKT_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_ENQ_USED_ENTRY_CNT (0x15010000 + 0xbcc)---

    CNT[31..0]                   - (RC) Enqueue HW used entry counter

 =====================================================================================*/
#define WED_RTQM_ENQ_USED_ENTRY_CNT_CNT_ADDR              WED_RTQM_ENQ_USED_ENTRY_CNT_ADDR
#define WED_RTQM_ENQ_USED_ENTRY_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_USED_ENTRY_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_ENQ_ERR_CNT (0x15010000 + 0xbd0)---

    CNT[31..0]                   - (RC) Enqueue error DMAD counter

 =====================================================================================*/
#define WED_RTQM_ENQ_ERR_CNT_CNT_ADDR                     WED_RTQM_ENQ_ERR_CNT_ADDR
#define WED_RTQM_ENQ_ERR_CNT_CNT_MASK                     0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_ENQ_ERR_CNT_CNT_SHFT                     0

/* =====================================================================================

  ---WED_RTQM_DEQ_CFG0 (0x15010000 + 0xbd4)---

    FORCE_DROP[0]                - (RW) Force Drop all DMAD
    DMAD_PVLD_UPD[1]             - (RW) Update WiFi DMAD PPE_VLD field.
                                     Update WiFi DMAD PPE_VLD=1 for those packet passing through PPE.
    SG_DMAD_PFDBK_MERGE[2]       - (RW) Merge PPE feedback to all WiFi DMAD during scatter/gather.
                                     When enabled, merge PPE feedback to all DMAD during scatter/gather.
    SG_DMAD_PFDBK_DEFAULT[3]     - (RW) Default value of PPE feedback related field
                                     If the field is not going to be merged with PPE feedback, keeps value not changed.
    DMAD_PFDBK_MERGE_DIS[4]      - (RW) Disable PPE feedback merge feature
    DEQ_ERR_INS_ADDR[5]          - (RW) Insert dequeue address to DMAD
                                     Replace DMAD PPE_ENTRY with dequeue address.
    DEQ_ERR_DEST[7..6]           - (RW) Dequeue destination when error occurs
    DEQ_FSM[8]                   - (RO) Dequeue HW FSM
    DEQ_FSM_FORCE_IDLE[9]        - (RW) Force Dequeue HW FSM to IDLE
    DMAD_MOD_EN[15..10]          - (RW) Enable DMAD modification feature
                                     [0] PPE_VLD
                                     [1] CRSN
                                     [2] DDONE
                                     [3] LS
                                     [4] RING_INFO
                                     [5] TO_HOST
    DMAD_MOD_RING_INFO[17..16]   - (RW) DMAD RING_INFO field value of DMAD modification feature
    DMAD_MOD_LS[18]              - (RW) DMAD LS field value of DMAD modification feature
    DMAD_MOD_DDONE[19]           - (RW) DMAD DDONE field value of DMAD modification feature
    DMAD_MOD_CRSN[24..20]        - (RW) DMAD CRSN field value of DMAD modification feature
    DMAD_MOD_PPE_VLD[25]         - (RW) DMAD PPE_VLD field value of DMAD modification feature
    DMAD_MOD_TO_HOST[26]         - (RW) DMAD TO_HOST field value of DMAD modification feature
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_TO_HOST_ADDR           WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_TO_HOST_MASK           0x04000000                // DMAD_MOD_TO_HOST[26]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_TO_HOST_SHFT           26
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_PPE_VLD_ADDR           WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_PPE_VLD_MASK           0x02000000                // DMAD_MOD_PPE_VLD[25]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_PPE_VLD_SHFT           25
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_CRSN_ADDR              WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_CRSN_MASK              0x01F00000                // DMAD_MOD_CRSN[24..20]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_CRSN_SHFT              20
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_DDONE_ADDR             WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_DDONE_MASK             0x00080000                // DMAD_MOD_DDONE[19]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_DDONE_SHFT             19
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_LS_ADDR                WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_LS_MASK                0x00040000                // DMAD_MOD_LS[18]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_LS_SHFT                18
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_RING_INFO_ADDR         WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_RING_INFO_MASK         0x00030000                // DMAD_MOD_RING_INFO[17..16]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_RING_INFO_SHFT         16
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_EN_ADDR                WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_EN_MASK                0x0000FC00                // DMAD_MOD_EN[15..10]
#define WED_RTQM_DEQ_CFG0_DMAD_MOD_EN_SHFT                10
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_FORCE_IDLE_ADDR         WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_FORCE_IDLE_MASK         0x00000200                // DEQ_FSM_FORCE_IDLE[9]
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_FORCE_IDLE_SHFT         9
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_ADDR                    WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_MASK                    0x00000100                // DEQ_FSM[8]
#define WED_RTQM_DEQ_CFG0_DEQ_FSM_SHFT                    8
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_DEST_ADDR               WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_DEST_MASK               0x000000C0                // DEQ_ERR_DEST[7..6]
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_DEST_SHFT               6
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_INS_ADDR_ADDR           WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_INS_ADDR_MASK           0x00000020                // DEQ_ERR_INS_ADDR[5]
#define WED_RTQM_DEQ_CFG0_DEQ_ERR_INS_ADDR_SHFT           5
#define WED_RTQM_DEQ_CFG0_DMAD_PFDBK_MERGE_DIS_ADDR       WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_PFDBK_MERGE_DIS_MASK       0x00000010                // DMAD_PFDBK_MERGE_DIS[4]
#define WED_RTQM_DEQ_CFG0_DMAD_PFDBK_MERGE_DIS_SHFT       4
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_DEFAULT_ADDR      WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_DEFAULT_MASK      0x00000008                // SG_DMAD_PFDBK_DEFAULT[3]
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_DEFAULT_SHFT      3
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_MERGE_ADDR        WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_MERGE_MASK        0x00000004                // SG_DMAD_PFDBK_MERGE[2]
#define WED_RTQM_DEQ_CFG0_SG_DMAD_PFDBK_MERGE_SHFT        2
#define WED_RTQM_DEQ_CFG0_DMAD_PVLD_UPD_ADDR              WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_DMAD_PVLD_UPD_MASK              0x00000002                // DMAD_PVLD_UPD[1]
#define WED_RTQM_DEQ_CFG0_DMAD_PVLD_UPD_SHFT              1
#define WED_RTQM_DEQ_CFG0_FORCE_DROP_ADDR                 WED_RTQM_DEQ_CFG0_ADDR
#define WED_RTQM_DEQ_CFG0_FORCE_DROP_MASK                 0x00000001                // FORCE_DROP[0]
#define WED_RTQM_DEQ_CFG0_FORCE_DROP_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_DEQ_DQ_DMAD_CNT (0x15010000 + 0xbd8)---

    CNT[31..0]                   - (RC) Dequeue from SRAM DMAD counter

 =====================================================================================*/
#define WED_RTQM_DEQ_DQ_DMAD_CNT_CNT_ADDR                 WED_RTQM_DEQ_DQ_DMAD_CNT_ADDR
#define WED_RTQM_DEQ_DQ_DMAD_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_DQ_DMAD_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_DEQ_Q2I_DMAD_CNT (0x15010000 + 0xbdc)---

    CNT[31..0]                   - (RC) Queue dequeue to IGRS DMAD counter

 =====================================================================================*/
#define WED_RTQM_DEQ_Q2I_DMAD_CNT_CNT_ADDR                WED_RTQM_DEQ_Q2I_DMAD_CNT_ADDR
#define WED_RTQM_DEQ_Q2I_DMAD_CNT_CNT_MASK                0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_Q2I_DMAD_CNT_CNT_SHFT                0

/* =====================================================================================

  ---WED_RTQM_DEQ_DQ_PKT_CNT (0x15010000 + 0xbe0)---

    CNT[31..0]                   - (RC) Dequeue from SRAM packet counter

 =====================================================================================*/
#define WED_RTQM_DEQ_DQ_PKT_CNT_CNT_ADDR                  WED_RTQM_DEQ_DQ_PKT_CNT_ADDR
#define WED_RTQM_DEQ_DQ_PKT_CNT_CNT_MASK                  0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_DQ_PKT_CNT_CNT_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_DEQ_Q2I_PKT_CNT (0x15010000 + 0xbe4)---

    CNT[31..0]                   - (RC) Queue dequeue to IGRS packet counter

 =====================================================================================*/
#define WED_RTQM_DEQ_Q2I_PKT_CNT_CNT_ADDR                 WED_RTQM_DEQ_Q2I_PKT_CNT_ADDR
#define WED_RTQM_DEQ_Q2I_PKT_CNT_CNT_MASK                 0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_Q2I_PKT_CNT_CNT_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_DEQ_USED_PFDBK_CNT (0x15010000 + 0xbe8)---

    CNT[31..0]                   - (RC) Dequeue HW used PPE feedback counter

 =====================================================================================*/
#define WED_RTQM_DEQ_USED_PFDBK_CNT_CNT_ADDR              WED_RTQM_DEQ_USED_PFDBK_CNT_ADDR
#define WED_RTQM_DEQ_USED_PFDBK_CNT_CNT_MASK              0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_USED_PFDBK_CNT_CNT_SHFT              0

/* =====================================================================================

  ---WED_RTQM_DEQ_ERR_CNT (0x15010000 + 0xbec)---

    CNT[31..0]                   - (RC) Dequeue error DMAD counter

 =====================================================================================*/
#define WED_RTQM_DEQ_ERR_CNT_CNT_ADDR                     WED_RTQM_DEQ_ERR_CNT_ADDR
#define WED_RTQM_DEQ_ERR_CNT_CNT_MASK                     0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_DEQ_ERR_CNT_CNT_SHFT                     0

/* =====================================================================================

  ---WED_RTQM_DEQ_DBG (0x15010000 + 0xbf0)---

    CURR_DMAD_EOP_MON[0]         - (RO) Current DMAD EOP
    CURR_DMAD_SOP_MON[1]         - (RO) Current DMAD SOP
    NEXT_DMAD_SOP_MON[2]         - (RO) Next DMAD SOP
    XOP_CLR[3]                   - (A0) Clear current DMAD SOP
    RESERVED4[5..4]              - (RO) Reserved bits
    DEQ_ERR_MOD_EN[9..6]         - (RW) Modify DMAD field when error occurs
                                     [0] CRSN
                                     [1] PPE_ENTRY
                                     [2] PPE_VLD
                                     [3] LS
    DEQ_ERR_MOD_LS[10]           - (RW) DMAD LS field of dequeue error DMAD modification feature
    DEQ_ERR_MOD_CRSN[15..11]     - (RW) DMAD CRSN field of dequeue error DMAD modification feature
    DEQ_ERR_MOD_PPE_ENTRY[30..16] - (RW) DMAD PPE_ENTRY value of dequeue error DMAD modification feature
    DEQ_ERR_MOD_PPE_VLD[31]      - (RW) DMAD PPE_VLD value of dequeue error DMAD modification feature

 =====================================================================================*/
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_VLD_ADDR         WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_VLD_MASK         0x80000000                // DEQ_ERR_MOD_PPE_VLD[31]
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_VLD_SHFT         31
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_ENTRY_ADDR       WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_ENTRY_MASK       0x7FFF0000                // DEQ_ERR_MOD_PPE_ENTRY[30..16]
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_PPE_ENTRY_SHFT       16
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_CRSN_ADDR            WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_CRSN_MASK            0x0000F800                // DEQ_ERR_MOD_CRSN[15..11]
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_CRSN_SHFT            11
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_LS_ADDR              WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_LS_MASK              0x00000400                // DEQ_ERR_MOD_LS[10]
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_LS_SHFT              10
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_EN_ADDR              WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_EN_MASK              0x000003C0                // DEQ_ERR_MOD_EN[9..6]
#define WED_RTQM_DEQ_DBG_DEQ_ERR_MOD_EN_SHFT              6
#define WED_RTQM_DEQ_DBG_XOP_CLR_ADDR                     WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_XOP_CLR_MASK                     0x00000008                // XOP_CLR[3]
#define WED_RTQM_DEQ_DBG_XOP_CLR_SHFT                     3
#define WED_RTQM_DEQ_DBG_NEXT_DMAD_SOP_MON_ADDR           WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_NEXT_DMAD_SOP_MON_MASK           0x00000004                // NEXT_DMAD_SOP_MON[2]
#define WED_RTQM_DEQ_DBG_NEXT_DMAD_SOP_MON_SHFT           2
#define WED_RTQM_DEQ_DBG_CURR_DMAD_SOP_MON_ADDR           WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_CURR_DMAD_SOP_MON_MASK           0x00000002                // CURR_DMAD_SOP_MON[1]
#define WED_RTQM_DEQ_DBG_CURR_DMAD_SOP_MON_SHFT           1
#define WED_RTQM_DEQ_DBG_CURR_DMAD_EOP_MON_ADDR           WED_RTQM_DEQ_DBG_ADDR
#define WED_RTQM_DEQ_DBG_CURR_DMAD_EOP_MON_MASK           0x00000001                // CURR_DMAD_EOP_MON[0]
#define WED_RTQM_DEQ_DBG_CURR_DMAD_EOP_MON_SHFT           0

/* =====================================================================================

  ---WED_RTQM_ERR_STS (0x15010000 + 0xbf4)---

    IGRS0_FIFO_W_ERR[0]          - (W1C) Try to write IGRS0 FIFO when it is full
    IGRS0_FIFO_R_ERR[1]          - (W1C) Try to read IGRS0 FIFO when it is empty
    IGRS1_FIFO_W_ERR[2]          - (W1C) Try to write IGRS1 FIFO when it is full
    IGRS1_FIFO_R_ERR[3]          - (W1C) Try to read IGRS1 FIFO when it is empty
    IGRS2_FIFO_W_ERR[4]          - (W1C) Try to write IGRS2 FIFO when it is full
    IGRS2_FIFO_R_ERR[5]          - (W1C) Try to read IGRS2 FIFO when it is empty
    IGRS3_FIFO_W_ERR[6]          - (W1C) Try to write IGRS3 FIFO when it is full
    IGRS3_FIFO_R_ERR[7]          - (W1C) Try to read IGRS3 FIFO when it is empty
    ENQ_ERR[8]                   - (W1C) Enqueue error, try to enqueue a entry with invalid status
    DEQ_ERR[9]                   - (W1C) Dequeue error, try to dequeue a entry which is not enqueued before
    PFDBK_FIFO_W_ERR[10]         - (W1C) Try to write PPE Feedback FIFO when it is full
    PFDBK_FIFO_R_ERR[11]         - (W1C) Try to read PPE Feedback FIFO when it is empty
    PFDBK_AGE_FIFO_W_ERR[12]     - (W1C) Try to write Dummy PPE Feedback FIFO when it is full
    PFDBK_AGE_FIFO_R_ERR[13]     - (W1C) Try to read Dummy PPE Feedback FIFO when it is empty
    QFREE_SRCH_FIFO_W_ERR[14]    - (W1C) Try to write Queue Free Entry Search FIFO when it is full
    QFREE_SRCH_FIFO_R_ERR[15]    - (W1C) Try to read Queue Free Entry Search FIFO when it is empty
    AGE_SRCH_FIFO_W_ERR[16]      - (W1C) Try to write Ageout Entry Search FIFO when it is full
    AGE_SRCH_FIFO_R_ERR[17]      - (W1C) Try to read Ageout Entry Search FIFO when it is empty
    RRO_ITF_FIFO_W_ERR[18]       - (W1C) Try to write RRO2RTQM Interface FIFO when it is full
    PN_ITF_FIFO_W_ERR[19]        - (W1C) Try to write PN2RTQM Interface FIFO when it is full
    CR_ITF_FIFO_W_ERR[20]        - (W1C) Try to write CR2RTQM Interface FIFO when it is full
    Q2BM_ITF_FIFO_R_ERR[21]      - (W1C) Try to read RTQM2RXBM Interface FIFO when it is empty
    Q2H0_ITF_FIFO_W_ERR[22]      - (W1C) Try to write RTQM2HOST Ring 0 Interface FIFO when it is full
    Q2H1_ITF_FIFO_W_ERR[23]      - (W1C) Try to write RTQM2HOST Ring 1 Interface FIFO when it is full
    PFDBK_ITF_FIFO_R_ERR[24]     - (W1C) Try to read PPE Feedback Interface FIFO when it is empty
    Q2N_ITF_FIFO_W_ERR[25]       - (W1C) Try to write RTQM2NETSYS Interface FIFO when it is full
    RRO3_1_ITF_FIFO_W_ERR[26]    - (W1C) Try to write RRO3.12NETSYS Interface FIFO when it is full
    RESERVED27[31..27]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_ERR_STS_RRO3_1_ITF_FIFO_W_ERR_ADDR       WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_RRO3_1_ITF_FIFO_W_ERR_MASK       0x04000000                // RRO3_1_ITF_FIFO_W_ERR[26]
#define WED_RTQM_ERR_STS_RRO3_1_ITF_FIFO_W_ERR_SHFT       26
#define WED_RTQM_ERR_STS_Q2N_ITF_FIFO_W_ERR_ADDR          WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_Q2N_ITF_FIFO_W_ERR_MASK          0x02000000                // Q2N_ITF_FIFO_W_ERR[25]
#define WED_RTQM_ERR_STS_Q2N_ITF_FIFO_W_ERR_SHFT          25
#define WED_RTQM_ERR_STS_PFDBK_ITF_FIFO_R_ERR_ADDR        WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PFDBK_ITF_FIFO_R_ERR_MASK        0x01000000                // PFDBK_ITF_FIFO_R_ERR[24]
#define WED_RTQM_ERR_STS_PFDBK_ITF_FIFO_R_ERR_SHFT        24
#define WED_RTQM_ERR_STS_Q2H1_ITF_FIFO_W_ERR_ADDR         WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_Q2H1_ITF_FIFO_W_ERR_MASK         0x00800000                // Q2H1_ITF_FIFO_W_ERR[23]
#define WED_RTQM_ERR_STS_Q2H1_ITF_FIFO_W_ERR_SHFT         23
#define WED_RTQM_ERR_STS_Q2H0_ITF_FIFO_W_ERR_ADDR         WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_Q2H0_ITF_FIFO_W_ERR_MASK         0x00400000                // Q2H0_ITF_FIFO_W_ERR[22]
#define WED_RTQM_ERR_STS_Q2H0_ITF_FIFO_W_ERR_SHFT         22
#define WED_RTQM_ERR_STS_Q2BM_ITF_FIFO_R_ERR_ADDR         WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_Q2BM_ITF_FIFO_R_ERR_MASK         0x00200000                // Q2BM_ITF_FIFO_R_ERR[21]
#define WED_RTQM_ERR_STS_Q2BM_ITF_FIFO_R_ERR_SHFT         21
#define WED_RTQM_ERR_STS_CR_ITF_FIFO_W_ERR_ADDR           WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_CR_ITF_FIFO_W_ERR_MASK           0x00100000                // CR_ITF_FIFO_W_ERR[20]
#define WED_RTQM_ERR_STS_CR_ITF_FIFO_W_ERR_SHFT           20
#define WED_RTQM_ERR_STS_PN_ITF_FIFO_W_ERR_ADDR           WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PN_ITF_FIFO_W_ERR_MASK           0x00080000                // PN_ITF_FIFO_W_ERR[19]
#define WED_RTQM_ERR_STS_PN_ITF_FIFO_W_ERR_SHFT           19
#define WED_RTQM_ERR_STS_RRO_ITF_FIFO_W_ERR_ADDR          WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_RRO_ITF_FIFO_W_ERR_MASK          0x00040000                // RRO_ITF_FIFO_W_ERR[18]
#define WED_RTQM_ERR_STS_RRO_ITF_FIFO_W_ERR_SHFT          18
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_R_ERR_ADDR         WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_R_ERR_MASK         0x00020000                // AGE_SRCH_FIFO_R_ERR[17]
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_R_ERR_SHFT         17
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_W_ERR_ADDR         WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_W_ERR_MASK         0x00010000                // AGE_SRCH_FIFO_W_ERR[16]
#define WED_RTQM_ERR_STS_AGE_SRCH_FIFO_W_ERR_SHFT         16
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_R_ERR_ADDR       WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_R_ERR_MASK       0x00008000                // QFREE_SRCH_FIFO_R_ERR[15]
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_R_ERR_SHFT       15
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_W_ERR_ADDR       WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_W_ERR_MASK       0x00004000                // QFREE_SRCH_FIFO_W_ERR[14]
#define WED_RTQM_ERR_STS_QFREE_SRCH_FIFO_W_ERR_SHFT       14
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_R_ERR_ADDR        WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_R_ERR_MASK        0x00002000                // PFDBK_AGE_FIFO_R_ERR[13]
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_R_ERR_SHFT        13
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_W_ERR_ADDR        WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_W_ERR_MASK        0x00001000                // PFDBK_AGE_FIFO_W_ERR[12]
#define WED_RTQM_ERR_STS_PFDBK_AGE_FIFO_W_ERR_SHFT        12
#define WED_RTQM_ERR_STS_PFDBK_FIFO_R_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PFDBK_FIFO_R_ERR_MASK            0x00000800                // PFDBK_FIFO_R_ERR[11]
#define WED_RTQM_ERR_STS_PFDBK_FIFO_R_ERR_SHFT            11
#define WED_RTQM_ERR_STS_PFDBK_FIFO_W_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_PFDBK_FIFO_W_ERR_MASK            0x00000400                // PFDBK_FIFO_W_ERR[10]
#define WED_RTQM_ERR_STS_PFDBK_FIFO_W_ERR_SHFT            10
#define WED_RTQM_ERR_STS_DEQ_ERR_ADDR                     WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_DEQ_ERR_MASK                     0x00000200                // DEQ_ERR[9]
#define WED_RTQM_ERR_STS_DEQ_ERR_SHFT                     9
#define WED_RTQM_ERR_STS_ENQ_ERR_ADDR                     WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_ENQ_ERR_MASK                     0x00000100                // ENQ_ERR[8]
#define WED_RTQM_ERR_STS_ENQ_ERR_SHFT                     8
#define WED_RTQM_ERR_STS_IGRS3_FIFO_R_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS3_FIFO_R_ERR_MASK            0x00000080                // IGRS3_FIFO_R_ERR[7]
#define WED_RTQM_ERR_STS_IGRS3_FIFO_R_ERR_SHFT            7
#define WED_RTQM_ERR_STS_IGRS3_FIFO_W_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS3_FIFO_W_ERR_MASK            0x00000040                // IGRS3_FIFO_W_ERR[6]
#define WED_RTQM_ERR_STS_IGRS3_FIFO_W_ERR_SHFT            6
#define WED_RTQM_ERR_STS_IGRS2_FIFO_R_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS2_FIFO_R_ERR_MASK            0x00000020                // IGRS2_FIFO_R_ERR[5]
#define WED_RTQM_ERR_STS_IGRS2_FIFO_R_ERR_SHFT            5
#define WED_RTQM_ERR_STS_IGRS2_FIFO_W_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS2_FIFO_W_ERR_MASK            0x00000010                // IGRS2_FIFO_W_ERR[4]
#define WED_RTQM_ERR_STS_IGRS2_FIFO_W_ERR_SHFT            4
#define WED_RTQM_ERR_STS_IGRS1_FIFO_R_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS1_FIFO_R_ERR_MASK            0x00000008                // IGRS1_FIFO_R_ERR[3]
#define WED_RTQM_ERR_STS_IGRS1_FIFO_R_ERR_SHFT            3
#define WED_RTQM_ERR_STS_IGRS1_FIFO_W_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS1_FIFO_W_ERR_MASK            0x00000004                // IGRS1_FIFO_W_ERR[2]
#define WED_RTQM_ERR_STS_IGRS1_FIFO_W_ERR_SHFT            2
#define WED_RTQM_ERR_STS_IGRS0_FIFO_R_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS0_FIFO_R_ERR_MASK            0x00000002                // IGRS0_FIFO_R_ERR[1]
#define WED_RTQM_ERR_STS_IGRS0_FIFO_R_ERR_SHFT            1
#define WED_RTQM_ERR_STS_IGRS0_FIFO_W_ERR_ADDR            WED_RTQM_ERR_STS_ADDR
#define WED_RTQM_ERR_STS_IGRS0_FIFO_W_ERR_MASK            0x00000001                // IGRS0_FIFO_W_ERR[0]
#define WED_RTQM_ERR_STS_IGRS0_FIFO_W_ERR_SHFT            0

/* =====================================================================================

  ---WED_RTQM_QUEUE_CFG0 (0x15010000 + 0xbf8)---

    ENQ_ERR_CHK[3..0]            - (RW) Enqueue error check
                                     [0] flag_fetchable
                                     [1] flag_enqueue
                                     [2] flag_link_head
                                     [3] flag_age
    DEQ_ERR_CHK[7..4]            - (RW) Dequeue error check
                                     [0] flag_fetchable
                                     [1] flag_enqueue
                                     [2] flag_link_head
                                     [3] flag_age
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_QUEUE_CFG0_DEQ_ERR_CHK_ADDR              WED_RTQM_QUEUE_CFG0_ADDR
#define WED_RTQM_QUEUE_CFG0_DEQ_ERR_CHK_MASK              0x000000F0                // DEQ_ERR_CHK[7..4]
#define WED_RTQM_QUEUE_CFG0_DEQ_ERR_CHK_SHFT              4
#define WED_RTQM_QUEUE_CFG0_ENQ_ERR_CHK_ADDR              WED_RTQM_QUEUE_CFG0_ADDR
#define WED_RTQM_QUEUE_CFG0_ENQ_ERR_CHK_MASK              0x0000000F                // ENQ_ERR_CHK[3..0]
#define WED_RTQM_QUEUE_CFG0_ENQ_ERR_CHK_SHFT              0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_CFG (0x15010000 + 0xbfc)---

    WR[0]                        - (A0) Issue write operation
                                     Must not read and write at same time
                                     Issue a write operation. The bit reset to 0 automatically after operation finished.
    RD[1]                        - (A0) Issue read operation
                                     Must not read and write at same time
                                     Issue a read operation. The bit reset to 0 automatically after operation finished.
    RESERVED2[3..2]              - (RO) Reserved bits
    FLAG_ENQUEUE_SET_ALL[4]      - (RW) Set all FLAG_ENQUEUE to 1
    FLAG_FETCHABLE_SET_ALL[5]    - (RW) Set all FLAG_FETCHABLE to 1
    FLAG_AGE_SET_ALL[6]          - (RW) Set all FLAG_AGE to 1
    FLAG_LINK_HEAD_SET_ALL[7]    - (RW) Set all FLAG_LINK_HEAD to 1
    FLAG_ENQUEUE_CLR_ALL[8]      - (RW) Clear all FLAG_ENQUEUE to 0
    FLAG_FETCHABLE_CLR_ALL[9]    - (RW) Clear all FLAG_FETCHABLE to 0
    FLAG_AGE_CLR_ALL[10]         - (RW) Clear all FLAG_AGE to 0
    FLAG_LINK_HEAD_CLR_ALL[11]   - (RW) Clear all FLAG_LINK_HEAD to 0
    WR_CMD_ERR[21..12]           - (RO) Write operation errors
                                     [0] Clear FLAG_AGE Error
                                     [1] Set FLAG_AGE Error
                                     [2] Clear FLAG_LINK_HEAD Error
                                     [3] Set FLAG_LINK_HEAD Error
                                     [4] Clear FLAG_FETCHABLE Error
                                     [5] Set FLAG_FETCHABLE Error
                                     [6] Clear FLAG_ENQUEUE Error
                                     [7] Set FLAG_ENQUEUE Error
                                     [8] SRAM Write Error
                                     [9] Reserved
    ADDRESS[31..22]              - (RW) Specify address for read/write operation
                                     route qm depth = 512

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_CFG_ADDRESS_ADDR               WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_ADDRESS_MASK               0xFFC00000                // ADDRESS[31..22]
#define WED_RTQM_QUEUE_ACC_CFG_ADDRESS_SHFT               22
#define WED_RTQM_QUEUE_ACC_CFG_WR_CMD_ERR_ADDR            WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_WR_CMD_ERR_MASK            0x003FF000                // WR_CMD_ERR[21..12]
#define WED_RTQM_QUEUE_ACC_CFG_WR_CMD_ERR_SHFT            12
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_CLR_ALL_ADDR WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_CLR_ALL_MASK 0x00000800                // FLAG_LINK_HEAD_CLR_ALL[11]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_CLR_ALL_SHFT 11
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_CLR_ALL_ADDR      WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_CLR_ALL_MASK      0x00000400                // FLAG_AGE_CLR_ALL[10]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_CLR_ALL_SHFT      10
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_CLR_ALL_ADDR WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_CLR_ALL_MASK 0x00000200                // FLAG_FETCHABLE_CLR_ALL[9]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_CLR_ALL_SHFT 9
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_CLR_ALL_ADDR  WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_CLR_ALL_MASK  0x00000100                // FLAG_ENQUEUE_CLR_ALL[8]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_CLR_ALL_SHFT  8
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_SET_ALL_ADDR WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_SET_ALL_MASK 0x00000080                // FLAG_LINK_HEAD_SET_ALL[7]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_LINK_HEAD_SET_ALL_SHFT 7
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_SET_ALL_ADDR      WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_SET_ALL_MASK      0x00000040                // FLAG_AGE_SET_ALL[6]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_AGE_SET_ALL_SHFT      6
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_SET_ALL_ADDR WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_SET_ALL_MASK 0x00000020                // FLAG_FETCHABLE_SET_ALL[5]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_FETCHABLE_SET_ALL_SHFT 5
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_SET_ALL_ADDR  WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_SET_ALL_MASK  0x00000010                // FLAG_ENQUEUE_SET_ALL[4]
#define WED_RTQM_QUEUE_ACC_CFG_FLAG_ENQUEUE_SET_ALL_SHFT  4
#define WED_RTQM_QUEUE_ACC_CFG_RD_ADDR                    WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_RD_MASK                    0x00000002                // RD[1]
#define WED_RTQM_QUEUE_ACC_CFG_RD_SHFT                    1
#define WED_RTQM_QUEUE_ACC_CFG_WR_ADDR                    WED_RTQM_QUEUE_ACC_CFG_ADDR
#define WED_RTQM_QUEUE_ACC_CFG_WR_MASK                    0x00000001                // WR[0]
#define WED_RTQM_QUEUE_ACC_CFG_WR_SHFT                    0

/* =====================================================================================

  ---WED_RTQM_DELAY_CFG (0x15010000 + 0xc00)---

    timer_delay_en[0]            - (RW) Route qm to WDMA_TX_DRV DMAD delay enable. It will invalid when efuse timer_delay_en=1
                                     If enabled, route_qm will add DMAD delay to WDMA_TX_DRV.
                                     The delay time = timer_delay_tick_div * (timer_delay_mod + timer_delay_cycle )
    timer_delay_tick_div[3..1]   - (RW) Route qm to WDMA_TX_DRV DMAD delay tick interval select.
    timer_delay_cycle[11..4]     - (RW) Route qm to WDMA_TX_DRV DMAD delay cycle of tick.
    timer_delay_mod[14..12]      - (RW) Route qm to WDMA_TX_DRV DMAD delay base cycle mode. It will invalid when efuse timer_delay_en=1
    timer_delay_val_mon[23..15]  - (RO) Route qm to WDMA_TX_DRV DMAD delay value monitor
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_DELAY_CFG_timer_delay_val_mon_ADDR       WED_RTQM_DELAY_CFG_ADDR
#define WED_RTQM_DELAY_CFG_timer_delay_val_mon_MASK       0x00FF8000                // timer_delay_val_mon[23..15]
#define WED_RTQM_DELAY_CFG_timer_delay_val_mon_SHFT       15
#define WED_RTQM_DELAY_CFG_timer_delay_mod_ADDR           WED_RTQM_DELAY_CFG_ADDR
#define WED_RTQM_DELAY_CFG_timer_delay_mod_MASK           0x00007000                // timer_delay_mod[14..12]
#define WED_RTQM_DELAY_CFG_timer_delay_mod_SHFT           12
#define WED_RTQM_DELAY_CFG_timer_delay_cycle_ADDR         WED_RTQM_DELAY_CFG_ADDR
#define WED_RTQM_DELAY_CFG_timer_delay_cycle_MASK         0x00000FF0                // timer_delay_cycle[11..4]
#define WED_RTQM_DELAY_CFG_timer_delay_cycle_SHFT         4
#define WED_RTQM_DELAY_CFG_timer_delay_tick_div_ADDR      WED_RTQM_DELAY_CFG_ADDR
#define WED_RTQM_DELAY_CFG_timer_delay_tick_div_MASK      0x0000000E                // timer_delay_tick_div[3..1]
#define WED_RTQM_DELAY_CFG_timer_delay_tick_div_SHFT      1
#define WED_RTQM_DELAY_CFG_timer_delay_en_ADDR            WED_RTQM_DELAY_CFG_ADDR
#define WED_RTQM_DELAY_CFG_timer_delay_en_MASK            0x00000001                // timer_delay_en[0]
#define WED_RTQM_DELAY_CFG_timer_delay_en_SHFT            0

/* =====================================================================================

  ---WED_RTQM_DELAY_DBG (0x15010000 + 0xc04)---

    timer_delay_en_fuse_mon[0]   - (RO) timer delay en fuse setting monitor
    timer_delay_mod_fuse_mon[3..1] - (RO) timer delay mod fuse setting monitor
    timer_delay_en_after_mux_mon[4] - (RO) timer delay en after mux monitor
    timer_delay_mod_after_mux_mon[7..5] - (RO) timer delay mod after mux monitor
    timer_delay_total_cycle_mon[16..8] - (RO) timer delay total cycle monitor
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_DELAY_DBG_timer_delay_total_cycle_mon_ADDR WED_RTQM_DELAY_DBG_ADDR
#define WED_RTQM_DELAY_DBG_timer_delay_total_cycle_mon_MASK 0x0001FF00                // timer_delay_total_cycle_mon[16..8]
#define WED_RTQM_DELAY_DBG_timer_delay_total_cycle_mon_SHFT 8
#define WED_RTQM_DELAY_DBG_timer_delay_mod_after_mux_mon_ADDR WED_RTQM_DELAY_DBG_ADDR
#define WED_RTQM_DELAY_DBG_timer_delay_mod_after_mux_mon_MASK 0x000000E0                // timer_delay_mod_after_mux_mon[7..5]
#define WED_RTQM_DELAY_DBG_timer_delay_mod_after_mux_mon_SHFT 5
#define WED_RTQM_DELAY_DBG_timer_delay_en_after_mux_mon_ADDR WED_RTQM_DELAY_DBG_ADDR
#define WED_RTQM_DELAY_DBG_timer_delay_en_after_mux_mon_MASK 0x00000010                // timer_delay_en_after_mux_mon[4]
#define WED_RTQM_DELAY_DBG_timer_delay_en_after_mux_mon_SHFT 4
#define WED_RTQM_DELAY_DBG_timer_delay_mod_fuse_mon_ADDR  WED_RTQM_DELAY_DBG_ADDR
#define WED_RTQM_DELAY_DBG_timer_delay_mod_fuse_mon_MASK  0x0000000E                // timer_delay_mod_fuse_mon[3..1]
#define WED_RTQM_DELAY_DBG_timer_delay_mod_fuse_mon_SHFT  1
#define WED_RTQM_DELAY_DBG_timer_delay_en_fuse_mon_ADDR   WED_RTQM_DELAY_DBG_ADDR
#define WED_RTQM_DELAY_DBG_timer_delay_en_fuse_mon_MASK   0x00000001                // timer_delay_en_fuse_mon[0]
#define WED_RTQM_DELAY_DBG_timer_delay_en_fuse_mon_SHFT   0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_WR0 (0x15010000 + 0xc10)---

    DATA[31..0]                  - (RW) SRAM Write Data [31:0]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_WR0_DATA_ADDR                  WED_RTQM_QUEUE_ACC_WR0_ADDR
#define WED_RTQM_QUEUE_ACC_WR0_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_WR0_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_WR1 (0x15010000 + 0xc14)---

    DATA[31..0]                  - (RW) SRAM Write Data [63:32]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_WR1_DATA_ADDR                  WED_RTQM_QUEUE_ACC_WR1_ADDR
#define WED_RTQM_QUEUE_ACC_WR1_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_WR1_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_WR2 (0x15010000 + 0xc18)---

    DATA[31..0]                  - (RW) SRAM Write Data [95:64]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_WR2_DATA_ADDR                  WED_RTQM_QUEUE_ACC_WR2_ADDR
#define WED_RTQM_QUEUE_ACC_WR2_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_WR2_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_WR3 (0x15010000 + 0xc1c)---

    DATA[31..0]                  - (RW) SRAM Write Data [127:96]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_WR3_DATA_ADDR                  WED_RTQM_QUEUE_ACC_WR3_ADDR
#define WED_RTQM_QUEUE_ACC_WR3_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_WR3_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_WR4 (0x15010000 + 0xc20)---

    LINK[9..0]                   - (RW) SRAM Link List Data Field
                                     route qm depth = 512
    DATA_RSV[15..10]             - (RW) SRAM Write Data [133:128]
    WR_CMD[25..16]               - (RW) Write command for write operation
                                     [0] Clear FLAG_AGE
                                     [1] Set FLAG_AGE
                                     [2] Clear FLAG_LINK_HEAD
                                     [3] Set FLAG_LINK_HEAD
                                     [4] Clear FLAG_FETCHABLE
                                     [5] Set FLAG_FETCHABLE
                                     [6] Clear FLAG_ENQUEUE
                                     [7] Set FLAG_ENQUEUE
                                     [8] SRAM Write
                                     [9] Reserved
    RESERVED26[31..26]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_WR4_WR_CMD_ADDR                WED_RTQM_QUEUE_ACC_WR4_ADDR
#define WED_RTQM_QUEUE_ACC_WR4_WR_CMD_MASK                0x03FF0000                // WR_CMD[25..16]
#define WED_RTQM_QUEUE_ACC_WR4_WR_CMD_SHFT                16
#define WED_RTQM_QUEUE_ACC_WR4_DATA_RSV_ADDR              WED_RTQM_QUEUE_ACC_WR4_ADDR
#define WED_RTQM_QUEUE_ACC_WR4_DATA_RSV_MASK              0x0000FC00                // DATA_RSV[15..10]
#define WED_RTQM_QUEUE_ACC_WR4_DATA_RSV_SHFT              10
#define WED_RTQM_QUEUE_ACC_WR4_LINK_ADDR                  WED_RTQM_QUEUE_ACC_WR4_ADDR
#define WED_RTQM_QUEUE_ACC_WR4_LINK_MASK                  0x000003FF                // LINK[9..0]
#define WED_RTQM_QUEUE_ACC_WR4_LINK_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_RD0 (0x15010000 + 0xc24)---

    DATA[31..0]                  - (RO) SRAM Read Data [31:0]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_RD0_DATA_ADDR                  WED_RTQM_QUEUE_ACC_RD0_ADDR
#define WED_RTQM_QUEUE_ACC_RD0_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_RD0_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_RD1 (0x15010000 + 0xc28)---

    DATA[31..0]                  - (RO) SRAM Read Data [63:32]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_RD1_DATA_ADDR                  WED_RTQM_QUEUE_ACC_RD1_ADDR
#define WED_RTQM_QUEUE_ACC_RD1_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_RD1_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_RD2 (0x15010000 + 0xc2c)---

    DATA[31..0]                  - (RO) SRAM Read Data [95:64]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_RD2_DATA_ADDR                  WED_RTQM_QUEUE_ACC_RD2_ADDR
#define WED_RTQM_QUEUE_ACC_RD2_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_RD2_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_RD3 (0x15010000 + 0xc30)---

    DATA[31..0]                  - (RO) SRAM Read Data [127:96]

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_RD3_DATA_ADDR                  WED_RTQM_QUEUE_ACC_RD3_ADDR
#define WED_RTQM_QUEUE_ACC_RD3_DATA_MASK                  0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_QUEUE_ACC_RD3_DATA_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_ACC_RD4 (0x15010000 + 0xc34)---

    LINK[9..0]                   - (RO) SRAM Link List Data Field
                                     route qm depth = 512
    DATA_RSV[15..10]             - (RO) SRAM Read Data [133:128]
    FLAGS[19..16]                - (RO) Queue Entry Flags
                                     [0] FLAG_AGE
                                     [1] FLAG_LINK_HEAD
                                     [2] FLAG_FETCHABLE
                                     [3] FLAG_ENQUEUE
    RESERVED20[31..20]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_QUEUE_ACC_RD4_FLAGS_ADDR                 WED_RTQM_QUEUE_ACC_RD4_ADDR
#define WED_RTQM_QUEUE_ACC_RD4_FLAGS_MASK                 0x000F0000                // FLAGS[19..16]
#define WED_RTQM_QUEUE_ACC_RD4_FLAGS_SHFT                 16
#define WED_RTQM_QUEUE_ACC_RD4_DATA_RSV_ADDR              WED_RTQM_QUEUE_ACC_RD4_ADDR
#define WED_RTQM_QUEUE_ACC_RD4_DATA_RSV_MASK              0x0000FC00                // DATA_RSV[15..10]
#define WED_RTQM_QUEUE_ACC_RD4_DATA_RSV_SHFT              10
#define WED_RTQM_QUEUE_ACC_RD4_LINK_ADDR                  WED_RTQM_QUEUE_ACC_RD4_ADDR
#define WED_RTQM_QUEUE_ACC_RD4_LINK_MASK                  0x000003FF                // LINK[9..0]
#define WED_RTQM_QUEUE_ACC_RD4_LINK_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_QUEUE_CNT (0x15010000 + 0xc38)---

    CNT[31..0]                   - (RO) Queue Used Entry Counter

 =====================================================================================*/
#define WED_RTQM_QUEUE_CNT_CNT_ADDR                       WED_RTQM_QUEUE_CNT_ADDR
#define WED_RTQM_QUEUE_CNT_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_QUEUE_CNT_CNT_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_DBG_CFG (0x15010000 + 0xc3c)---

    DBG_MON_SEL[7..0]            - (RW) RTQM Debug Monitor Selection
    DBG_ENQ_WR[8]                - (A0) RTQM Debug Enqueue Interface DMAD FIFO Write
                                     Set 1 to write ENQ DMAD Data to FIFO. Clear to 0 automatically at write done.
                                     Must check DBG_ENQ_VLD==1 before write FIFO (prevent overflow)
    DBG_ENQ_VLD[9]               - (RO) RTQM Debug Enqueue Interface DMAD FIFO Valid
    PPE_FDBK_DROP[10]            - (RW) Drop PPE Feedback.
    RESERVED11[11]               - (RO) Reserved bits
    ENQ_DDONE_CTRL[13..12]       - (RW) Enqueue HW to NETSYS DMAD DDONE Field Control
    ENQ_DDONE_DW1_CTRL[15..14]   - (RW) Enqueue HW to NETSYS DMAD DDONE_DW1 Field Control
    ENQ_DDONE_DW2_CTRL[17..16]   - (RW) Enqueue HW to NETSYS DMAD DDONE_DW2 Field Control
    ENQ_DDONE2_CTRL[19..18]      - (RW) Enqueue HW to NETSYS DMAD DDONE2 Field Control
    RESERVED20[31..20]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_DBG_CFG_ENQ_DDONE2_CTRL_ADDR             WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_ENQ_DDONE2_CTRL_MASK             0x000C0000                // ENQ_DDONE2_CTRL[19..18]
#define WED_RTQM_DBG_CFG_ENQ_DDONE2_CTRL_SHFT             18
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW2_CTRL_ADDR          WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW2_CTRL_MASK          0x00030000                // ENQ_DDONE_DW2_CTRL[17..16]
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW2_CTRL_SHFT          16
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW1_CTRL_ADDR          WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW1_CTRL_MASK          0x0000C000                // ENQ_DDONE_DW1_CTRL[15..14]
#define WED_RTQM_DBG_CFG_ENQ_DDONE_DW1_CTRL_SHFT          14
#define WED_RTQM_DBG_CFG_ENQ_DDONE_CTRL_ADDR              WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_ENQ_DDONE_CTRL_MASK              0x00003000                // ENQ_DDONE_CTRL[13..12]
#define WED_RTQM_DBG_CFG_ENQ_DDONE_CTRL_SHFT              12
#define WED_RTQM_DBG_CFG_PPE_FDBK_DROP_ADDR               WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_PPE_FDBK_DROP_MASK               0x00000400                // PPE_FDBK_DROP[10]
#define WED_RTQM_DBG_CFG_PPE_FDBK_DROP_SHFT               10
#define WED_RTQM_DBG_CFG_DBG_ENQ_VLD_ADDR                 WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_DBG_ENQ_VLD_MASK                 0x00000200                // DBG_ENQ_VLD[9]
#define WED_RTQM_DBG_CFG_DBG_ENQ_VLD_SHFT                 9
#define WED_RTQM_DBG_CFG_DBG_ENQ_WR_ADDR                  WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_DBG_ENQ_WR_MASK                  0x00000100                // DBG_ENQ_WR[8]
#define WED_RTQM_DBG_CFG_DBG_ENQ_WR_SHFT                  8
#define WED_RTQM_DBG_CFG_DBG_MON_SEL_ADDR                 WED_RTQM_DBG_CFG_ADDR
#define WED_RTQM_DBG_CFG_DBG_MON_SEL_MASK                 0x000000FF                // DBG_MON_SEL[7..0]
#define WED_RTQM_DBG_CFG_DBG_MON_SEL_SHFT                 0

/* =====================================================================================

  ---WED_RTQM_QUEUE_DBG_DEPTH (0x15010000 + 0xc40)---

    LIMIT_EN[0]                  - (RW) Set enable to limit the route qm queue's depth.
                                     The fetchable flag will be always set 0, means queue always is not empty.
    LIMIT_VALUE[11..1]           - (RW) route qm queue's depth limitation value.
                                     Max value:512, over 512 is unvalid
                                     e.g.  If set 16, route qm queue MEM just can use 0~15.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_VALUE_ADDR         WED_RTQM_QUEUE_DBG_DEPTH_ADDR
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_VALUE_MASK         0x00000FFE                // LIMIT_VALUE[11..1]
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_VALUE_SHFT         1
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_EN_ADDR            WED_RTQM_QUEUE_DBG_DEPTH_ADDR
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_EN_MASK            0x00000001                // LIMIT_EN[0]
#define WED_RTQM_QUEUE_DBG_DEPTH_LIMIT_EN_SHFT            0

/* =====================================================================================

  ---WED_RTQM_QUEUE_DBG_CFG (0x15010000 + 0xc44)---

    CNT_OVER_THRES_CLR[0]        - (A0) set 1 to clear queue cnt over threshold counter
    CNT_MAX_CLR[1]               - (A0) set 1 to clear queue max value
    RESERVED2[31..2]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_QUEUE_DBG_CFG_CNT_MAX_CLR_ADDR           WED_RTQM_QUEUE_DBG_CFG_ADDR
#define WED_RTQM_QUEUE_DBG_CFG_CNT_MAX_CLR_MASK           0x00000002                // CNT_MAX_CLR[1]
#define WED_RTQM_QUEUE_DBG_CFG_CNT_MAX_CLR_SHFT           1
#define WED_RTQM_QUEUE_DBG_CFG_CNT_OVER_THRES_CLR_ADDR    WED_RTQM_QUEUE_DBG_CFG_ADDR
#define WED_RTQM_QUEUE_DBG_CFG_CNT_OVER_THRES_CLR_MASK    0x00000001                // CNT_OVER_THRES_CLR[0]
#define WED_RTQM_QUEUE_DBG_CFG_CNT_OVER_THRES_CLR_SHFT    0

/* =====================================================================================

  ---WED_RTQM_QUEUE_DBG_THRES_CFG (0x15010000 + 0xc48)---

    VALUE[31..0]                 - (RW) QUEUE threshold.
                                     WED_RTQM_QUEUE_DBG_THRES.OVER_CNT will increase when DMAD enqueue and the queue using count is over the threshold.

 =====================================================================================*/
#define WED_RTQM_QUEUE_DBG_THRES_CFG_VALUE_ADDR           WED_RTQM_QUEUE_DBG_THRES_CFG_ADDR
#define WED_RTQM_QUEUE_DBG_THRES_CFG_VALUE_MASK           0xFFFFFFFF                // VALUE[31..0]
#define WED_RTQM_QUEUE_DBG_THRES_CFG_VALUE_SHFT           0

/* =====================================================================================

  ---WED_RTQM_QUEUE_DBG_THRES (0x15010000 + 0xc4c)---

    OVER_CNT[31..0]              - (RO) Will increase when DMAD enqueue and the queue using count is over the threshold.

 =====================================================================================*/
#define WED_RTQM_QUEUE_DBG_THRES_OVER_CNT_ADDR            WED_RTQM_QUEUE_DBG_THRES_ADDR
#define WED_RTQM_QUEUE_DBG_THRES_OVER_CNT_MASK            0xFFFFFFFF                // OVER_CNT[31..0]
#define WED_RTQM_QUEUE_DBG_THRES_OVER_CNT_SHFT            0

/* =====================================================================================

  ---WED_RTQM_QUEUE_PEAK_MON (0x15010000 + 0xc50)---

    VALUE[31..0]                 - (RO) Monit the the peak of route_qm queue using count

 =====================================================================================*/
#define WED_RTQM_QUEUE_PEAK_MON_VALUE_ADDR                WED_RTQM_QUEUE_PEAK_MON_ADDR
#define WED_RTQM_QUEUE_PEAK_MON_VALUE_MASK                0xFFFFFFFF                // VALUE[31..0]
#define WED_RTQM_QUEUE_PEAK_MON_VALUE_SHFT                0

/* =====================================================================================

  ---WED_RTQM_DBG_MON0 (0x15010000 + 0xc60)---

    DATA[31..0]                  - (RU) RTQM Debug Monitor Data

 =====================================================================================*/
#define WED_RTQM_DBG_MON0_DATA_ADDR                       WED_RTQM_DBG_MON0_ADDR
#define WED_RTQM_DBG_MON0_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_DBG_MON0_DATA_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_DBG_MON1 (0x15010000 + 0xc64)---

    DATA[31..0]                  - (RU) RTQM Debug Monitor Data

 =====================================================================================*/
#define WED_RTQM_DBG_MON1_DATA_ADDR                       WED_RTQM_DBG_MON1_ADDR
#define WED_RTQM_DBG_MON1_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_DBG_MON1_DATA_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_DBG_MON2 (0x15010000 + 0xc68)---

    DATA[31..0]                  - (RU) RTQM Debug Monitor Data

 =====================================================================================*/
#define WED_RTQM_DBG_MON2_DATA_ADDR                       WED_RTQM_DBG_MON2_ADDR
#define WED_RTQM_DBG_MON2_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_DBG_MON2_DATA_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_DBG_MON3 (0x15010000 + 0xc6c)---

    DATA[31..0]                  - (RU) RTQM Debug Monitor Data

 =====================================================================================*/
#define WED_RTQM_DBG_MON3_DATA_ADDR                       WED_RTQM_DBG_MON3_ADDR
#define WED_RTQM_DBG_MON3_DATA_MASK                       0xFFFFFFFF                // DATA[31..0]
#define WED_RTQM_DBG_MON3_DATA_SHFT                       0

/* =====================================================================================

  ---WED_RTQM_DBG_ENQ_DATA0 (0x15010000 + 0xc70)---

    DMAD[31..0]                  - (RW) RTQM Debug Enqueue DMAD Data
                                     DMAD[31:0]

 =====================================================================================*/
#define WED_RTQM_DBG_ENQ_DATA0_DMAD_ADDR                  WED_RTQM_DBG_ENQ_DATA0_ADDR
#define WED_RTQM_DBG_ENQ_DATA0_DMAD_MASK                  0xFFFFFFFF                // DMAD[31..0]
#define WED_RTQM_DBG_ENQ_DATA0_DMAD_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_DBG_ENQ_DATA1 (0x15010000 + 0xc74)---

    DMAD[31..0]                  - (RW) RTQM Debug Enqueue DMAD Data
                                     DMAD[63:32]

 =====================================================================================*/
#define WED_RTQM_DBG_ENQ_DATA1_DMAD_ADDR                  WED_RTQM_DBG_ENQ_DATA1_ADDR
#define WED_RTQM_DBG_ENQ_DATA1_DMAD_MASK                  0xFFFFFFFF                // DMAD[31..0]
#define WED_RTQM_DBG_ENQ_DATA1_DMAD_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_DBG_ENQ_DATA2 (0x15010000 + 0xc78)---

    DMAD[31..0]                  - (RW) RTQM Debug Enqueue DMAD Data
                                     DMAD[95:64]

 =====================================================================================*/
#define WED_RTQM_DBG_ENQ_DATA2_DMAD_ADDR                  WED_RTQM_DBG_ENQ_DATA2_ADDR
#define WED_RTQM_DBG_ENQ_DATA2_DMAD_MASK                  0xFFFFFFFF                // DMAD[31..0]
#define WED_RTQM_DBG_ENQ_DATA2_DMAD_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_DBG_ENQ_DATA3 (0x15010000 + 0xc7c)---

    DMAD[31..0]                  - (RW) RTQM Debug Enqueue DMAD Data
                                     DMAD[127:96]

 =====================================================================================*/
#define WED_RTQM_DBG_ENQ_DATA3_DMAD_ADDR                  WED_RTQM_DBG_ENQ_DATA3_ADDR
#define WED_RTQM_DBG_ENQ_DATA3_DMAD_MASK                  0xFFFFFFFF                // DMAD[31..0]
#define WED_RTQM_DBG_ENQ_DATA3_DMAD_SHFT                  0

/* =====================================================================================

  ---WED_RTQM_AGE_CFG0 (0x15010000 + 0xc88)---

    FSM[1..0]                    - (RO) Ageout HW FSM
    FSM_FORCE_IDLE[2]            - (RW) Force Ageout HW FSM to IDLE
    FLUSH_EN[3]                  - (A0) Flush Mode Enable
    RESERVED4[7..4]              - (RO) Reserved bits
    DFDBK_CSRN[12..8]            - (RW) Dummy PFDBK CSRN Field
    RESERVED13[15..13]           - (RO) Reserved bits
    DFDBK_PPE_ENTRY[30..16]      - (RW) Dummy PFDBK PPE Entry Field
    DFDBK_TO_HOST[31]            - (RW) Dummy PFDBK To Host Field

 =====================================================================================*/
#define WED_RTQM_AGE_CFG0_DFDBK_TO_HOST_ADDR              WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_DFDBK_TO_HOST_MASK              0x80000000                // DFDBK_TO_HOST[31]
#define WED_RTQM_AGE_CFG0_DFDBK_TO_HOST_SHFT              31
#define WED_RTQM_AGE_CFG0_DFDBK_PPE_ENTRY_ADDR            WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_DFDBK_PPE_ENTRY_MASK            0x7FFF0000                // DFDBK_PPE_ENTRY[30..16]
#define WED_RTQM_AGE_CFG0_DFDBK_PPE_ENTRY_SHFT            16
#define WED_RTQM_AGE_CFG0_DFDBK_CSRN_ADDR                 WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_DFDBK_CSRN_MASK                 0x00001F00                // DFDBK_CSRN[12..8]
#define WED_RTQM_AGE_CFG0_DFDBK_CSRN_SHFT                 8
#define WED_RTQM_AGE_CFG0_FLUSH_EN_ADDR                   WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_FLUSH_EN_MASK                   0x00000008                // FLUSH_EN[3]
#define WED_RTQM_AGE_CFG0_FLUSH_EN_SHFT                   3
#define WED_RTQM_AGE_CFG0_FSM_FORCE_IDLE_ADDR             WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_FSM_FORCE_IDLE_MASK             0x00000004                // FSM_FORCE_IDLE[2]
#define WED_RTQM_AGE_CFG0_FSM_FORCE_IDLE_SHFT             2
#define WED_RTQM_AGE_CFG0_FSM_ADDR                        WED_RTQM_AGE_CFG0_ADDR
#define WED_RTQM_AGE_CFG0_FSM_MASK                        0x00000003                // FSM[1..0]
#define WED_RTQM_AGE_CFG0_FSM_SHFT                        0

/* =====================================================================================

  ---WED_RTQM_AGE_CFG1 (0x15010000 + 0xc90)---

    AGE_TIME[31..0]              - (RW) Age Out Duration
                                     Duration = (AGE_TIME+1)*20us. Each tick is 20us.
                                     Timer is disabled if AGE_TIME=0.

 =====================================================================================*/
#define WED_RTQM_AGE_CFG1_AGE_TIME_ADDR                   WED_RTQM_AGE_CFG1_ADDR
#define WED_RTQM_AGE_CFG1_AGE_TIME_MASK                   0xFFFFFFFF                // AGE_TIME[31..0]
#define WED_RTQM_AGE_CFG1_AGE_TIME_SHFT                   0

/* =====================================================================================

  ---WED_RTQM_AGE_FIFO_CFG (0x15010000 + 0xc94)---

    RESERVED0[0]                 - (RO) Reserved bits
    PREF_0_CLEAR[1]              - (RW) Clear PREF_0 FIFO
    PREF_0_FULL[2]               - (RO) PREF_0 FIFO full
    PREF_0_EMPTY[3]              - (RO) PREF_0 FIFO empty
    RESERVED4[4]                 - (RO) Reserved bits
    PREF_1_CLEAR[5]              - (RW) Clear PREF_1 FIFO
    PREF_1_FULL[6]               - (RO) PREF_1 FIFO full
    PREF_1_EMPTY[7]              - (RO) PREF_1 FIFO empty
    RESERVED8[8]                 - (RO) Reserved bits
    PREF_2_CLEAR[9]              - (RW) Clear PREF_2 FIFO
    PREF_2_FULL[10]              - (RO) PREF_2 FIFO full
    PREF_2_EMPTY[11]             - (RO) PREF_2 FIFO empty
    RESERVED12[12]               - (RO) Reserved bits
    PREF_3_CLEAR[13]             - (RW) Clear PREF_3 FIFO
    PREF_3_FULL[14]              - (RO) PREF_3 FIFO full
    PREF_3_EMPTY[15]             - (RO) PREF_3 FIFO empty
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_EMPTY_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_EMPTY_MASK           0x00008000                // PREF_3_EMPTY[15]
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_EMPTY_SHFT           15
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_FULL_ADDR            WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_FULL_MASK            0x00004000                // PREF_3_FULL[14]
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_FULL_SHFT            14
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_CLEAR_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_CLEAR_MASK           0x00002000                // PREF_3_CLEAR[13]
#define WED_RTQM_AGE_FIFO_CFG_PREF_3_CLEAR_SHFT           13
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_EMPTY_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_EMPTY_MASK           0x00000800                // PREF_2_EMPTY[11]
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_EMPTY_SHFT           11
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_FULL_ADDR            WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_FULL_MASK            0x00000400                // PREF_2_FULL[10]
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_FULL_SHFT            10
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_CLEAR_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_CLEAR_MASK           0x00000200                // PREF_2_CLEAR[9]
#define WED_RTQM_AGE_FIFO_CFG_PREF_2_CLEAR_SHFT           9
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_EMPTY_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_EMPTY_MASK           0x00000080                // PREF_1_EMPTY[7]
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_EMPTY_SHFT           7
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_FULL_ADDR            WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_FULL_MASK            0x00000040                // PREF_1_FULL[6]
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_FULL_SHFT            6
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_CLEAR_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_CLEAR_MASK           0x00000020                // PREF_1_CLEAR[5]
#define WED_RTQM_AGE_FIFO_CFG_PREF_1_CLEAR_SHFT           5
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_EMPTY_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_EMPTY_MASK           0x00000008                // PREF_0_EMPTY[3]
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_EMPTY_SHFT           3
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_FULL_ADDR            WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_FULL_MASK            0x00000004                // PREF_0_FULL[2]
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_FULL_SHFT            2
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_CLEAR_ADDR           WED_RTQM_AGE_FIFO_CFG_ADDR
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_CLEAR_MASK           0x00000002                // PREF_0_CLEAR[1]
#define WED_RTQM_AGE_FIFO_CFG_PREF_0_CLEAR_SHFT           1

/* =====================================================================================

  ---WED_RTQM_AGE_PKT_CNT (0x15010000 + 0xc98)---

    CNT[31..0]                   - (RC) Ageout packet counter

 =====================================================================================*/
#define WED_RTQM_AGE_PKT_CNT_CNT_ADDR                     WED_RTQM_AGE_PKT_CNT_ADDR
#define WED_RTQM_AGE_PKT_CNT_CNT_MASK                     0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_AGE_PKT_CNT_CNT_SHFT                     0

/* =====================================================================================

  ---WED_RTQM_PFDBK_CNT (0x15010000 + 0xc9c)---

    CNT[31..0]                   - (RC) PPE Feedback Interface Read Counter

 =====================================================================================*/
#define WED_RTQM_PFDBK_CNT_CNT_ADDR                       WED_RTQM_PFDBK_CNT_ADDR
#define WED_RTQM_PFDBK_CNT_CNT_MASK                       0xFFFFFFFF                // CNT[31..0]
#define WED_RTQM_PFDBK_CNT_CNT_SHFT                       0

/* =====================================================================================

  ---WED_TX_BM_DYN_ALLOC_TH (0x15010000 + 0xD00)---

    L_BUF_TH[15..0]              - (RW) WED TX Buffer Manager SW Dynamic Allocation Low Threshold for buffer adding    [Sw_alloc - (tail_idx - head_idx) < l_buf_th]
    H_BUF_TH[31..16]             - (RW) WED TX Buffer Manager SW Dynamic Allocation High Threshold for buffer retrieving    [Sw_alloc - (tail_idx - head_idx) > h_buf_th]

 =====================================================================================*/
#define WED_TX_BM_DYN_ALLOC_TH_H_BUF_TH_ADDR              WED_TX_BM_DYN_ALLOC_TH_ADDR
#define WED_TX_BM_DYN_ALLOC_TH_H_BUF_TH_MASK              0xFFFF0000                // H_BUF_TH[31..16]
#define WED_TX_BM_DYN_ALLOC_TH_H_BUF_TH_SHFT              16
#define WED_TX_BM_DYN_ALLOC_TH_L_BUF_TH_ADDR              WED_TX_BM_DYN_ALLOC_TH_ADDR
#define WED_TX_BM_DYN_ALLOC_TH_L_BUF_TH_MASK              0x0000FFFF                // L_BUF_TH[15..0]
#define WED_TX_BM_DYN_ALLOC_TH_L_BUF_TH_SHFT              0

/* =====================================================================================

  ---WED_TX_BM_DYN_ALLOC_CFG (0x15010000 + 0xD04)---

    SW_ADDSUB_IDX_CNT[15..0]     - (RW) WED TX Buffer Manager SW Add/Subtract DMAD buffer amount
    SW_SUB_BUF_REQ[16]           - (A0) WED TX Buffer Manager SW Subtract DMAD buffer Request
                                     Req is cleared when HW SUB process done (HW_ADD_SUB_ACK=1)
    SW_ADD_BUF_REQ[17]           - (A0) WED TX Buffer Manager SW Add DMAD buffer Request (Add>Sub when both set)
                                     Req is cleared when HW ADD process done (HW_ADD_SUB_ACK=1)
    SW_SUB_RDY[18]               - (A0) For Sub process, HW set HW_SUB_PAUSE (b31) to notify SW to retrieve DMSAD buffer, SW set this bit when it finishes DMAD content copy for retrieve.
    RESERVED19[27..19]           - (RO) Reserved bits
    SW_SUB_UNDERFLOW_ERR[28]     - (W1C) SW add RX DMAD count underflow (<=0)
                                     The subtrating process will not be executed, and this error bit is W1C
    SW_ADD_OVERFLOW_ERR[29]      - (W1C) (1) sw_tail_idx is larger than wed_rx_bm_sw_cfg_buf_idx
                                     Buffer manager will not execute fetch and free DMAD processes.
                                     (2) SW add RXDMAD count overflow (>wed_rx_bm_sw_cfg_buf_idx)
                                     The adding process will not be executed.
                                     This error bit is W1C.
    HW_ADDSUB_ACK[30]            - (RO) HW signal ADD/SUB done, return to normal DMAD ring fifo operation.
    HW_SUB_pause[31]             - (RO) At SW subtrate buffer process, when HW tail_idx-head_idx >= SW_ADDSUB_IDX_CNT, HW set this bit to inform SW, Buffer is ready for retrieve.
                                     HW_SUB_pause is cleared after SW set SW_SUB_RDY (b18), indicating SW has copied out the DMAD for retrieving, HW can over-write the DMAD index content

 =====================================================================================*/
#define WED_TX_BM_DYN_ALLOC_CFG_HW_SUB_pause_ADDR         WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_HW_SUB_pause_MASK         0x80000000                // HW_SUB_pause[31]
#define WED_TX_BM_DYN_ALLOC_CFG_HW_SUB_pause_SHFT         31
#define WED_TX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_ADDR        WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_MASK        0x40000000                // HW_ADDSUB_ACK[30]
#define WED_TX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_SHFT        30
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_ADDR  WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_MASK  0x20000000                // SW_ADD_OVERFLOW_ERR[29]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_SHFT  29
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_ADDR WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_MASK 0x10000000                // SW_SUB_UNDERFLOW_ERR[28]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_SHFT 28
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_ADDR           WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_MASK           0x00040000                // SW_SUB_RDY[18]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_SHFT           18
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_ADDR       WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_MASK       0x00020000                // SW_ADD_BUF_REQ[17]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_SHFT       17
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_ADDR       WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_MASK       0x00010000                // SW_SUB_BUF_REQ[16]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_SHFT       16
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_ADDR    WED_TX_BM_DYN_ALLOC_CFG_ADDR
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_MASK    0x0000FFFF                // SW_ADDSUB_IDX_CNT[15..0]
#define WED_TX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_SHFT    0

/* =====================================================================================

  ---WED_TX_BM_ADD_BASE (0x15010000 + 0xD08)---

    PTR[31..0]                   - (RW) SW ADD_IDX Buffer Base Address

 =====================================================================================*/
#define WED_TX_BM_ADD_BASE_PTR_ADDR                       WED_TX_BM_ADD_BASE_ADDR
#define WED_TX_BM_ADD_BASE_PTR_MASK                       0xFFFFFFFF                // PTR[31..0]
#define WED_TX_BM_ADD_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_TX_BM_ADD_PTR (0x15010000 + 0xD0C)---

    SW_ADD_IDX_REMAIN[15..0]     - (RO) SW_ADD_IDX_CNT Remaining = SW_ADD_IDX_CNT - head2_idx
    HEAD2_IDX[31..16]            - (RO) head2_idx is incremented by HW each time a Rx DMAD is fetched from WED_TX_BM_ADD_BASE by WED_TX_BM, which points to the index WED_TX_BM will fetch next, at SW add Rx buffer stage.
                                     max. head2_idx =  SW_ADD_IDX_CNT

 =====================================================================================*/
#define WED_TX_BM_ADD_PTR_HEAD2_IDX_ADDR                  WED_TX_BM_ADD_PTR_ADDR
#define WED_TX_BM_ADD_PTR_HEAD2_IDX_MASK                  0xFFFF0000                // HEAD2_IDX[31..16]
#define WED_TX_BM_ADD_PTR_HEAD2_IDX_SHFT                  16
#define WED_TX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_ADDR          WED_TX_BM_ADD_PTR_ADDR
#define WED_TX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_MASK          0x0000FFFF                // SW_ADD_IDX_REMAIN[15..0]
#define WED_TX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_SHFT          0

/* =====================================================================================

  ---WED_TX_BM_TOTAL_DMAD_IDX (0x15010000 + 0xD10)---

    SW_TOTAL_RX_BM_CNT[15..0]    - (RO) The total amount of Rx DMAD count after SW init/add/subtrate, max 64K-1
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_ADDR  WED_TX_BM_TOTAL_DMAD_IDX_ADDR
#define WED_TX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_MASK  0x0000FFFF                // SW_TOTAL_RX_BM_CNT[15..0]
#define WED_TX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_SHFT  0

/* =====================================================================================

  ---WED_TX_BM_ADD_BASE_H (0x15010000 + 0xD14)---

    PTR[7..0]                    - (RW) High bits of SW ADD_IDX Buffer Base Address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_TX_BM_ADD_BASE_H_PTR_ADDR                     WED_TX_BM_ADD_BASE_H_ADDR
#define WED_TX_BM_ADD_BASE_H_PTR_MASK                     0x000000FF                // PTR[7..0]
#define WED_TX_BM_ADD_BASE_H_PTR_SHFT                     0

/* =====================================================================================

  ---WED_RX_BM_RX_DMAD (0x15010000 + 0xD80)---

    SDL0[13..0]                  - (RW) Rx DMAD field: sdl0 , for WED_RX_BM to cat with reduced Rx DMAD read from DRAM to form a complete one
    RESERVED14[15..14]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of Buffer Pool Base Address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_RX_DMAD_BASE_PTR_H_ADDR                 WED_RX_BM_RX_DMAD_ADDR
#define WED_RX_BM_RX_DMAD_BASE_PTR_H_MASK                 0x00FF0000                // BASE_PTR_H[23..16]
#define WED_RX_BM_RX_DMAD_BASE_PTR_H_SHFT                 16
#define WED_RX_BM_RX_DMAD_SDL0_ADDR                       WED_RX_BM_RX_DMAD_ADDR
#define WED_RX_BM_RX_DMAD_SDL0_MASK                       0x00003FFF                // SDL0[13..0]
#define WED_RX_BM_RX_DMAD_SDL0_SHFT                       0

/* =====================================================================================

  ---WED_RX_BM_BASE (0x15010000 + 0xD84)---

    PTR[31..0]                   - (RW) Buffer Pool Base Address
                                     Note that the Rx BM Buffer Pool Base Address must be 4KB addressing aligned

 =====================================================================================*/
#define WED_RX_BM_BASE_PTR_ADDR                           WED_RX_BM_BASE_ADDR
#define WED_RX_BM_BASE_PTR_MASK                           0xFFFFFFFF                // PTR[31..0]
#define WED_RX_BM_BASE_PTR_SHFT                           0

/* =====================================================================================

  ---WED_RX_BM_INIT_PTR (0x15010000 + 0xD88)---

    SW_TAIL_IDX[15..0]           - (RW) 1. Free DMAD ptr(numbers) allocated by SW at initial. The Rx DMAD FIFO is 64K, but this value is flexiable depending on SW's requested, the operation need for Rx without loss. The maximum value SW can set/prepare is 64k-1 (0xffff).
    INIT_SW_TAIL_HEAD_IDX[16]    - (A0) Tail_idx and head_idx will initiate to sw_tail_idx and init_head_idx at setting this bit.
                                     SW initiates tail_idx to sw_tail_idx at setting this bit. SW set this bit with sw_tail_idx below after it has allocated sw_tail_idx number of Rx DMAD buffer, before enable WED Tx Buffer Manager operation.
                                     init_head_idx is for DV verficaion purpose, it is recommended that SW does not need to do init_head_idx settings.
                                     This bit is auto cleared.
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_ADDR     WED_RX_BM_INIT_PTR_ADDR
#define WED_RX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK     0x00010000                // INIT_SW_TAIL_HEAD_IDX[16]
#define WED_RX_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_SHFT     16
#define WED_RX_BM_INIT_PTR_SW_TAIL_IDX_ADDR               WED_RX_BM_INIT_PTR_ADDR
#define WED_RX_BM_INIT_PTR_SW_TAIL_IDX_MASK               0x0000FFFF                // SW_TAIL_IDX[15..0]
#define WED_RX_BM_INIT_PTR_SW_TAIL_IDX_SHFT               0

/* =====================================================================================

  ---WED_RX_BM_PTR (0x15010000 + 0xD8C)---

    TAIL_IDX[15..0]              - (RO) 1. tail_idx is updated to sw_tail_idx when init_sw_tail_idx of WED_RX_BM_INIT_PTR is set. This is the Free DMAD number SW allocates. WED_RX_BM will start Rx DMAD delivery and return, after wed_rx_bm is enabled, and this field != head_idx (0).
                                     2. it is advanced by HW each time when a DMAD is free/returned from RX_ROUTE_QM or WED_RX_DMA (original rx DMAD is exchanged with that of CPU ring, for CPU to handle rx pkt to it.) The tail_idx update should not run over head_idx
    HEAD_IDX[31..16]             - (RO) head_idx is incremented by HW each time a Rx DMAD is fetched by WED_RX_BM, which points to the index WED_RX_BM will fetch next.
                                     Note that head_idx will never exceed tail_idx. When they are equal, it means all rx DMAD allocated by SW are all passed to HW to Rx pkt and no one is free yet.

 =====================================================================================*/
#define WED_RX_BM_PTR_HEAD_IDX_ADDR                       WED_RX_BM_PTR_ADDR
#define WED_RX_BM_PTR_HEAD_IDX_MASK                       0xFFFF0000                // HEAD_IDX[31..16]
#define WED_RX_BM_PTR_HEAD_IDX_SHFT                       16
#define WED_RX_BM_PTR_TAIL_IDX_ADDR                       WED_RX_BM_PTR_ADDR
#define WED_RX_BM_PTR_TAIL_IDX_MASK                       0x0000FFFF                // TAIL_IDX[15..0]
#define WED_RX_BM_PTR_TAIL_IDX_SHFT                       0

/* =====================================================================================

  ---WED_RX_BM_STS (0x15010000 + 0xD94)---

    RESERVED0[7..0]              - (RO) Reserved bits
    DMAD_NUM[12..8]              - (RO) Number of valid free DMADs in common dmad FIFO, ready for WPDMA/RRO_RX_D_DRV modules. Those free DMADs are picked out from buffer pool by Buffer manager.
    RESERVED13[15..13]           - (RO) Reserved bits
    DMAD0_NUM[17..16]            - (RO) Number of valid free DMADs in arbitrated dmad small FIFO, ready for WPDMA_RX_D_DRV module for rx legacy.
    RESERVED18[19..18]           - (RO) Reserved bits
    DMAD0_VLD[20]                - (RO) Free DMAD is valid for WPDMA_RX_D_DRV module for rx legacy
    DMAD0_RD[21]                 - (RO) WPDMA_RX_D_DRV module for rx legacy extracted a free DMAD
    RESERVED22[23..22]           - (RO) Reserved bits
    DMAD1_NUM[25..24]            - (RO) Number of valid free DMADs in arbitrated dmad small FIFO, ready for RRO_RX_D_DRV module for rx 3.0.
    RESERVED26[27..26]           - (RO) Reserved bits
    DMAD1_VLD[28]                - (RO) Free DMAD is valid for RRO_RX_D_DRV module for rx 3.0
    DMAD1_RD[29]                 - (RO) RRO_RX_D_DRV module for rx 3.0 extracted a free DMAD
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_STS_DMAD1_RD_ADDR                       WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD1_RD_MASK                       0x20000000                // DMAD1_RD[29]
#define WED_RX_BM_STS_DMAD1_RD_SHFT                       29
#define WED_RX_BM_STS_DMAD1_VLD_ADDR                      WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD1_VLD_MASK                      0x10000000                // DMAD1_VLD[28]
#define WED_RX_BM_STS_DMAD1_VLD_SHFT                      28
#define WED_RX_BM_STS_DMAD1_NUM_ADDR                      WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD1_NUM_MASK                      0x03000000                // DMAD1_NUM[25..24]
#define WED_RX_BM_STS_DMAD1_NUM_SHFT                      24
#define WED_RX_BM_STS_DMAD0_RD_ADDR                       WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD0_RD_MASK                       0x00200000                // DMAD0_RD[21]
#define WED_RX_BM_STS_DMAD0_RD_SHFT                       21
#define WED_RX_BM_STS_DMAD0_VLD_ADDR                      WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD0_VLD_MASK                      0x00100000                // DMAD0_VLD[20]
#define WED_RX_BM_STS_DMAD0_VLD_SHFT                      20
#define WED_RX_BM_STS_DMAD0_NUM_ADDR                      WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD0_NUM_MASK                      0x00030000                // DMAD0_NUM[17..16]
#define WED_RX_BM_STS_DMAD0_NUM_SHFT                      16
#define WED_RX_BM_STS_DMAD_NUM_ADDR                       WED_RX_BM_STS_ADDR
#define WED_RX_BM_STS_DMAD_NUM_MASK                       0x00001F00                // DMAD_NUM[12..8]
#define WED_RX_BM_STS_DMAD_NUM_SHFT                       8

/* =====================================================================================

  ---WED_RX_BM_INTF2 (0x15010000 + 0xD98)---

    DMAD_EXCHG[15..0]            - (RO) Exchange DMAD Token_ID for Buffer Manager
    RESERVED16[27..16]           - (RO) Reserved bits
    DMAD_EXCHG_VLD[28]           - (RO) There is at least one valid exchange DMAD token_ID for Buffer Manager, the Token_IDs requested for exchage are returned by  WED Rx DMA module, exchanged from CPU Rx ring after CPU rx pkt from DMAD given by WED RX Buffer Manager.
    DMAD_EXCHG_RD[29]            - (RO) Buffer Manager extracted an exchange DMAD Token_ID from WED Rx DMA module
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_INTF2_DMAD_EXCHG_RD_ADDR                WED_RX_BM_INTF2_ADDR
#define WED_RX_BM_INTF2_DMAD_EXCHG_RD_MASK                0x20000000                // DMAD_EXCHG_RD[29]
#define WED_RX_BM_INTF2_DMAD_EXCHG_RD_SHFT                29
#define WED_RX_BM_INTF2_DMAD_EXCHG_VLD_ADDR               WED_RX_BM_INTF2_ADDR
#define WED_RX_BM_INTF2_DMAD_EXCHG_VLD_MASK               0x10000000                // DMAD_EXCHG_VLD[28]
#define WED_RX_BM_INTF2_DMAD_EXCHG_VLD_SHFT               28
#define WED_RX_BM_INTF2_DMAD_EXCHG_ADDR                   WED_RX_BM_INTF2_ADDR
#define WED_RX_BM_INTF2_DMAD_EXCHG_MASK                   0x0000FFFF                // DMAD_EXCHG[15..0]
#define WED_RX_BM_INTF2_DMAD_EXCHG_SHFT                   0

/* =====================================================================================

  ---WED_RX_BM_INTF (0x15010000 + 0xD9C)---

    DMAD_FREE[15..0]             - (RO) Free Token_ID for Buffer Manager
    RESERVED16[27..16]           - (RO) Reserved bits
    DMAD_FREE_VLD[28]            - (RO) There is at least one valid Free token_ID for Buffer Manager, these free Token_IDs are returned by  Rx Route Queue Manager module, after the corresponding WiFi Rx pkt has been sent to Ethernet.
    DMAD_FREE_RD[29]             - (RO) Buffer Manager extracted a free Token_ID from Rx Route Queue Manager module
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_INTF_DMAD_FREE_RD_ADDR                  WED_RX_BM_INTF_ADDR
#define WED_RX_BM_INTF_DMAD_FREE_RD_MASK                  0x20000000                // DMAD_FREE_RD[29]
#define WED_RX_BM_INTF_DMAD_FREE_RD_SHFT                  29
#define WED_RX_BM_INTF_DMAD_FREE_VLD_ADDR                 WED_RX_BM_INTF_ADDR
#define WED_RX_BM_INTF_DMAD_FREE_VLD_MASK                 0x10000000                // DMAD_FREE_VLD[28]
#define WED_RX_BM_INTF_DMAD_FREE_VLD_SHFT                 28
#define WED_RX_BM_INTF_DMAD_FREE_ADDR                     WED_RX_BM_INTF_ADDR
#define WED_RX_BM_INTF_DMAD_FREE_MASK                     0x0000FFFF                // DMAD_FREE[15..0]
#define WED_RX_BM_INTF_DMAD_FREE_SHFT                     0

/* =====================================================================================

  ---WED_RX_BM_RANGE_CFG (0x15010000 + 0xDA0)---

    SW_CFG_BUF_IDX[15..0]        - (RW) This value is the entries of the DMAD FIFO ring set by SW at initial.  The maximum value of tail_idx and head_idx that can be achieved is this value. The maximum value SW can set is 64K-1(0xFFFF).
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_RANGE_CFG_SW_CFG_BUF_IDX_ADDR           WED_RX_BM_RANGE_CFG_ADDR
#define WED_RX_BM_RANGE_CFG_SW_CFG_BUF_IDX_MASK           0x0000FFFF                // SW_CFG_BUF_IDX[15..0]
#define WED_RX_BM_RANGE_CFG_SW_CFG_BUF_IDX_SHFT           0

/* =====================================================================================

  ---WED_RX_BM_INIT_HEAD_PTR (0x15010000 + 0xDA4)---

    INIT_HEAD_IDX[15..0]         - (RW) Start buffer index, for verification purpose (wraparound test)
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_ADDR        WED_RX_BM_INIT_HEAD_PTR_ADDR
#define WED_RX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_MASK        0x0000FFFF                // INIT_HEAD_IDX[15..0]
#define WED_RX_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_SHFT        0

/* =====================================================================================

  ---WED_RX_BM_ERR_STS (0x15010000 + 0xDA8)---

    DMAD_AXI_RD_LESS[0]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read transfer less than expected. For example, issue 8 transfers but rlast is  asserted earlier  than 8th transfer.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_AXI_RD_MORE[1]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read transfer more than expected. For example, issue 8 transfers but rlast is  asserted later  than 8th transfer.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_FULL_AXI_RD[2]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read while internal DMAD FIFO is already full.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_AXI_RD_OVER_TAIL[3]     - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read while head_idx is reaching tail_idx. DMAD should be all read in already.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    RESERVED4[6..4]              - (RO) Reserved bits
    FREE_AT_EMPTY[7]             - (RO) DMAD free while head_idx==tail_idx, all rx DMAD should already be free.
                                     Will trigger 0x20 WED_EX_INT_STA [12] rx_bm_free_at_empty
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_ERR_STS_FREE_AT_EMPTY_ADDR              WED_RX_BM_ERR_STS_ADDR
#define WED_RX_BM_ERR_STS_FREE_AT_EMPTY_MASK              0x00000080                // FREE_AT_EMPTY[7]
#define WED_RX_BM_ERR_STS_FREE_AT_EMPTY_SHFT              7
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_ADDR      WED_RX_BM_ERR_STS_ADDR
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_MASK      0x00000008                // DMAD_AXI_RD_OVER_TAIL[3]
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_SHFT      3
#define WED_RX_BM_ERR_STS_DMAD_FULL_AXI_RD_ADDR           WED_RX_BM_ERR_STS_ADDR
#define WED_RX_BM_ERR_STS_DMAD_FULL_AXI_RD_MASK           0x00000004                // DMAD_FULL_AXI_RD[2]
#define WED_RX_BM_ERR_STS_DMAD_FULL_AXI_RD_SHFT           2
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_MORE_ADDR           WED_RX_BM_ERR_STS_ADDR
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_MORE_MASK           0x00000002                // DMAD_AXI_RD_MORE[1]
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_MORE_SHFT           1
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_LESS_ADDR           WED_RX_BM_ERR_STS_ADDR
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_LESS_MASK           0x00000001                // DMAD_AXI_RD_LESS[0]
#define WED_RX_BM_ERR_STS_DMAD_AXI_RD_LESS_SHFT           0

/* =====================================================================================

  ---WED_RX_BM_DYN_ALLOC_TH (0x15010000 + 0xDB4)---

    L_BUF_TH[15..0]              - (RW) WED RX Buffer Manager SW Dynamic Allocation Low Threshold for buffer adding    [Sw_alloc - (tail_idx - head_idx) < l_buf_th]
    H_BUF_TH[31..16]             - (RW) WED RX Buffer Manager SW Dynamic Allocation High Threshold for buffer retrieving    [Sw_alloc - (tail_idx - head_idx) > h_buf_th]

 =====================================================================================*/
#define WED_RX_BM_DYN_ALLOC_TH_H_BUF_TH_ADDR              WED_RX_BM_DYN_ALLOC_TH_ADDR
#define WED_RX_BM_DYN_ALLOC_TH_H_BUF_TH_MASK              0xFFFF0000                // H_BUF_TH[31..16]
#define WED_RX_BM_DYN_ALLOC_TH_H_BUF_TH_SHFT              16
#define WED_RX_BM_DYN_ALLOC_TH_L_BUF_TH_ADDR              WED_RX_BM_DYN_ALLOC_TH_ADDR
#define WED_RX_BM_DYN_ALLOC_TH_L_BUF_TH_MASK              0x0000FFFF                // L_BUF_TH[15..0]
#define WED_RX_BM_DYN_ALLOC_TH_L_BUF_TH_SHFT              0

/* =====================================================================================

  ---WED_RX_BM_DYN_ALLOC_CFG (0x15010000 + 0xDB8)---

    SW_ADDSUB_IDX_CNT[15..0]     - (RW) WED RX Buffer Manager SW Add/Subtract DMAD buffer amount
    SW_SUB_BUF_REQ[16]           - (A0) WED RX Buffer Manager SW Subtract DMAD buffer Request
                                     Req is cleared when HW SUB process done (HW_ADD_SUB_ACK=1)
    SW_ADD_BUF_REQ[17]           - (A0) WED RX Buffer Manager SW Add DMAD buffer Request (Add>Sub when both set)
                                     Req is cleared when HW ADD process done (HW_ADD_SUB_ACK=1)
    SW_SUB_RDY[18]               - (A0) For Sub process, HW set HW_SUB_PAUSE (b31) to notify SW to retrieve DMSAD buffer, SW set this bit when it finishes DMAD content copy for retrieve.
    RESERVED19[27..19]           - (RO) Reserved bits
    SW_SUB_UNDERFLOW_ERR[28]     - (W1C) SW add RX DMAD count underflow (<=0)
                                     The subtrating process will not be executed, and this error bit is W1C
    SW_ADD_OVERFLOW_ERR[29]      - (W1C) (1) sw_tail_idx is larger than wed_rx_bm_sw_cfg_buf_idx
                                     Buffer manager will not execute fetch and free DMAD processes.
                                     (2) SW add RXDMAD count overflow (>wed_rx_bm_sw_cfg_buf_idx)
                                     The adding process will not be executed.
                                     This error bit is W1C.
    HW_ADDSUB_ACK[30]            - (RO) HW signal ADD/SUB done, return to normal DMAD ring fifo operation.
    HW_SUB_pause[31]             - (RO) At SW subtrate buffer process, when HW tail_idx-head_idx >= SW_ADDSUB_IDX_CNT, HW set this bit to inform SW, Buffer is ready for retrieve.
                                     HW_SUB_pause is cleared after SW set SW_SUB_RDY (b18), indicating SW has copied out the DMAD for retrieving, HW can over-write the DMAD index content

 =====================================================================================*/
#define WED_RX_BM_DYN_ALLOC_CFG_HW_SUB_pause_ADDR         WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_HW_SUB_pause_MASK         0x80000000                // HW_SUB_pause[31]
#define WED_RX_BM_DYN_ALLOC_CFG_HW_SUB_pause_SHFT         31
#define WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_ADDR        WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_MASK        0x40000000                // HW_ADDSUB_ACK[30]
#define WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_SHFT        30
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_ADDR  WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_MASK  0x20000000                // SW_ADD_OVERFLOW_ERR[29]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_SHFT  29
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_ADDR WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_MASK 0x10000000                // SW_SUB_UNDERFLOW_ERR[28]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_SHFT 28
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_ADDR           WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_MASK           0x00040000                // SW_SUB_RDY[18]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_RDY_SHFT           18
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_ADDR       WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_MASK       0x00020000                // SW_ADD_BUF_REQ[17]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_SHFT       17
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_ADDR       WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_MASK       0x00010000                // SW_SUB_BUF_REQ[16]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_SHFT       16
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_ADDR    WED_RX_BM_DYN_ALLOC_CFG_ADDR
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_MASK    0x0000FFFF                // SW_ADDSUB_IDX_CNT[15..0]
#define WED_RX_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_SHFT    0

/* =====================================================================================

  ---WED_RX_BM_ADD_BASE (0x15010000 + 0xDBC)---

    PTR[31..0]                   - (RW) SW ADD_IDX Buffer Base Address

 =====================================================================================*/
#define WED_RX_BM_ADD_BASE_PTR_ADDR                       WED_RX_BM_ADD_BASE_ADDR
#define WED_RX_BM_ADD_BASE_PTR_MASK                       0xFFFFFFFF                // PTR[31..0]
#define WED_RX_BM_ADD_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---WED_RX_BM_ADD_PTR (0x15010000 + 0xDC0)---

    SW_ADD_IDX_REMAIN[15..0]     - (RO) SW_ADD_IDX_CNT Remaining = SW_ADD_IDX_CNT - head2_idx
    HEAD2_IDX[31..16]            - (RO) head2_idx is incremented by HW each time a Rx DMAD is fetched from WED_RX_BM_ADD_BASE by WED_RX_BM, which points to the index WED_RX_BM will fetch next, at SW add Rx buffer stage.
                                     max. head2_idx =  SW_ADD_IDX_CNT

 =====================================================================================*/
#define WED_RX_BM_ADD_PTR_HEAD2_IDX_ADDR                  WED_RX_BM_ADD_PTR_ADDR
#define WED_RX_BM_ADD_PTR_HEAD2_IDX_MASK                  0xFFFF0000                // HEAD2_IDX[31..16]
#define WED_RX_BM_ADD_PTR_HEAD2_IDX_SHFT                  16
#define WED_RX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_ADDR          WED_RX_BM_ADD_PTR_ADDR
#define WED_RX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_MASK          0x0000FFFF                // SW_ADD_IDX_REMAIN[15..0]
#define WED_RX_BM_ADD_PTR_SW_ADD_IDX_REMAIN_SHFT          0

/* =====================================================================================

  ---WED_RX_BM_TOTAL_DMAD_IDX (0x15010000 + 0xDC4)---

    SW_TOTAL_RX_BM_CNT[15..0]    - (RO) The total amount of Rx DMAD count after SW init/add/subtrate, max 64K-1
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_ADDR  WED_RX_BM_TOTAL_DMAD_IDX_ADDR
#define WED_RX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_MASK  0x0000FFFF                // SW_TOTAL_RX_BM_CNT[15..0]
#define WED_RX_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_SHFT  0

/* =====================================================================================

  ---WED_RX_BM_ADD_BASE_H (0x15010000 + 0xDC8)---

    PTR[7..0]                    - (RW) High Bits of SW ADD_IDX Buffer Base Address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_BM_ADD_BASE_H_PTR_ADDR                     WED_RX_BM_ADD_BASE_H_ADDR
#define WED_RX_BM_ADD_BASE_H_PTR_MASK                     0x000000FF                // PTR[7..0]
#define WED_RX_BM_ADD_BASE_H_PTR_SHFT                     0

/* =====================================================================================

  ---RRO_MSDU_PG_RING0_CIDX_CFG0 (0x15010000 + 0xDE8)---

    MSDU_PG_ADDR_R0_L[31..0]     - (RW) The CR cpu index address of RRO MSDU PAGE RING 0 - low

 =====================================================================================*/
#define RRO_MSDU_PG_RING0_CIDX_CFG0_MSDU_PG_ADDR_R0_L_ADDR RRO_MSDU_PG_RING0_CIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING0_CIDX_CFG0_MSDU_PG_ADDR_R0_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R0_L[31..0]
#define RRO_MSDU_PG_RING0_CIDX_CFG0_MSDU_PG_ADDR_R0_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING0_CIDX_CFG1 (0x15010000 + 0xDEC)---

    MSDU_PG_ADDR_R0_M[3..0]      - (RW) The CR cpu index address of RRO MSDU PAGE RING 0 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING0_CIDX_CFG1_MSDU_PG_ADDR_R0_M_ADDR RRO_MSDU_PG_RING0_CIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING0_CIDX_CFG1_MSDU_PG_ADDR_R0_M_MASK 0x0000000F                // MSDU_PG_ADDR_R0_M[3..0]
#define RRO_MSDU_PG_RING0_CIDX_CFG1_MSDU_PG_ADDR_R0_M_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING1_CIDX_CFG0 (0x15010000 + 0xDF0)---

    MSDU_PG_ADDR_R1_L[31..0]     - (RW) The CR cpu index address of RRO MSDU PAGE RING 1- low

 =====================================================================================*/
#define RRO_MSDU_PG_RING1_CIDX_CFG0_MSDU_PG_ADDR_R1_L_ADDR RRO_MSDU_PG_RING1_CIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING1_CIDX_CFG0_MSDU_PG_ADDR_R1_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R1_L[31..0]
#define RRO_MSDU_PG_RING1_CIDX_CFG0_MSDU_PG_ADDR_R1_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING1_CIDX_CFG1 (0x15010000 + 0xDF4)---

    MSDU_PG_ADDR_R1_M[3..0]      - (RW) The CR cpu index address of RRO MSDU PAGE RING 1 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING1_CIDX_CFG1_MSDU_PG_ADDR_R1_M_ADDR RRO_MSDU_PG_RING1_CIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING1_CIDX_CFG1_MSDU_PG_ADDR_R1_M_MASK 0x0000000F                // MSDU_PG_ADDR_R1_M[3..0]
#define RRO_MSDU_PG_RING1_CIDX_CFG1_MSDU_PG_ADDR_R1_M_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING2_CIDX_CFG0 (0x15010000 + 0xDF8)---

    MSDU_PG_ADDR_R2_L[31..0]     - (RW) The CR cpu index address of RRO MSDU PAGE RING 2 - low

 =====================================================================================*/
#define RRO_MSDU_PG_RING2_CIDX_CFG0_MSDU_PG_ADDR_R2_L_ADDR RRO_MSDU_PG_RING2_CIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING2_CIDX_CFG0_MSDU_PG_ADDR_R2_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R2_L[31..0]
#define RRO_MSDU_PG_RING2_CIDX_CFG0_MSDU_PG_ADDR_R2_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING2_CIDX_CFG1 (0x15010000 + 0xDFC)---

    MSDU_PG_ADDR_R2_M[3..0]      - (RW) The CR cpu index address of RRO MSDU PAGE RING 2 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING2_CIDX_CFG1_MSDU_PG_ADDR_R2_M_ADDR RRO_MSDU_PG_RING2_CIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CIDX_CFG1_MSDU_PG_ADDR_R2_M_MASK 0x0000000F                // MSDU_PG_ADDR_R2_M[3..0]
#define RRO_MSDU_PG_RING2_CIDX_CFG1_MSDU_PG_ADDR_R2_M_SHFT 0

/* =====================================================================================

  ---RRO_IND_CMD_0_SIGNATURE (0x15010000 + 0xE00)---

    DMA_IDX[11..0]               - (RW) The dma index of RRO indicate command ring 0 which is assigned by RRO
    RESERVED12[15..12]           - (RO) Reserved bits
    SW_PROC_IDX[27..16]          - (RW) The SW assigning initial/start processing index of RRO indicate command ring 0, following is updated by WED
    MAGIC_CNT[30..28]            - (RW) The magic count of RRO indicate command ring 0 which is controlled by RRO
    VLD[31]                      - (A0) This bit is expected to be set to 1 by RRO

 =====================================================================================*/
#define RRO_IND_CMD_0_SIGNATURE_VLD_ADDR                  RRO_IND_CMD_0_SIGNATURE_ADDR
#define RRO_IND_CMD_0_SIGNATURE_VLD_MASK                  0x80000000                // VLD[31]
#define RRO_IND_CMD_0_SIGNATURE_VLD_SHFT                  31
#define RRO_IND_CMD_0_SIGNATURE_MAGIC_CNT_ADDR            RRO_IND_CMD_0_SIGNATURE_ADDR
#define RRO_IND_CMD_0_SIGNATURE_MAGIC_CNT_MASK            0x70000000                // MAGIC_CNT[30..28]
#define RRO_IND_CMD_0_SIGNATURE_MAGIC_CNT_SHFT            28
#define RRO_IND_CMD_0_SIGNATURE_SW_PROC_IDX_ADDR          RRO_IND_CMD_0_SIGNATURE_ADDR
#define RRO_IND_CMD_0_SIGNATURE_SW_PROC_IDX_MASK          0x0FFF0000                // SW_PROC_IDX[27..16]
#define RRO_IND_CMD_0_SIGNATURE_SW_PROC_IDX_SHFT          16
#define RRO_IND_CMD_0_SIGNATURE_DMA_IDX_ADDR              RRO_IND_CMD_0_SIGNATURE_ADDR
#define RRO_IND_CMD_0_SIGNATURE_DMA_IDX_MASK              0x00000FFF                // DMA_IDX[11..0]
#define RRO_IND_CMD_0_SIGNATURE_DMA_IDX_SHFT              0

/* =====================================================================================

  ---IND_CMD_0_CTRL_0 (0x15010000 + 0xE04)---

    PROC_IDX[11..0]              - (RO) The processing index of RRO indicate command ring 0 which is controlled by WED
    RESERVED12[15..12]           - (RO) Reserved bits
    PREFETCH_IND_CMD_FREE_CNT[19..16] - (RO) The prefetch free count of indicate commands
    RESERVED20[27..20]           - (RO) Reserved bits
    MAGIC_CNT[30..28]            - (RO) The magic count of RRO indicate command ring 0 which is controlled by WED
    INIT_MAGIC_CNT_PROC_IDX[31]  - (RO) 0: Not update MAGIC_CNT by RRO indicate command signature
                                     1: Update MAGIC_CNT by RRO indicate command, it will set to 0 after updating is complete

 =====================================================================================*/
#define IND_CMD_0_CTRL_0_INIT_MAGIC_CNT_PROC_IDX_ADDR     IND_CMD_0_CTRL_0_ADDR
#define IND_CMD_0_CTRL_0_INIT_MAGIC_CNT_PROC_IDX_MASK     0x80000000                // INIT_MAGIC_CNT_PROC_IDX[31]
#define IND_CMD_0_CTRL_0_INIT_MAGIC_CNT_PROC_IDX_SHFT     31
#define IND_CMD_0_CTRL_0_MAGIC_CNT_ADDR                   IND_CMD_0_CTRL_0_ADDR
#define IND_CMD_0_CTRL_0_MAGIC_CNT_MASK                   0x70000000                // MAGIC_CNT[30..28]
#define IND_CMD_0_CTRL_0_MAGIC_CNT_SHFT                   28
#define IND_CMD_0_CTRL_0_PREFETCH_IND_CMD_FREE_CNT_ADDR   IND_CMD_0_CTRL_0_ADDR
#define IND_CMD_0_CTRL_0_PREFETCH_IND_CMD_FREE_CNT_MASK   0x000F0000                // PREFETCH_IND_CMD_FREE_CNT[19..16]
#define IND_CMD_0_CTRL_0_PREFETCH_IND_CMD_FREE_CNT_SHFT   16
#define IND_CMD_0_CTRL_0_PROC_IDX_ADDR                    IND_CMD_0_CTRL_0_ADDR
#define IND_CMD_0_CTRL_0_PROC_IDX_MASK                    0x00000FFF                // PROC_IDX[11..0]
#define IND_CMD_0_CTRL_0_PROC_IDX_SHFT                    0

/* =====================================================================================

  ---IND_CMD_0_CTRL_1 (0x15010000 + 0xE08)---

    RESERVED0[3..0]              - (RO) Reserved bits
    RRO_IND_CMD_BASE_L[31..4]    - (RW) The base address of RRO indicate command ring 0
                                     IND_CMD_0_CTRL_1[3:0] is RSV and read 0, for RRO_IND_CMD_BASE is 16B aligned

 =====================================================================================*/
#define IND_CMD_0_CTRL_1_RRO_IND_CMD_BASE_L_ADDR          IND_CMD_0_CTRL_1_ADDR
#define IND_CMD_0_CTRL_1_RRO_IND_CMD_BASE_L_MASK          0xFFFFFFF0                // RRO_IND_CMD_BASE_L[31..4]
#define IND_CMD_0_CTRL_1_RRO_IND_CMD_BASE_L_SHFT          4

/* =====================================================================================

  ---IND_CMD_0_CTRL_2 (0x15010000 + 0xE0C)---

    RESERVED0[3..0]              - (RO) Reserved bits
    MAX_CNT_11_4[11..4]          - (RW) The max count [11:4] of RRO indicate command ring 0, max_cnt must be multiples of 16, which means [3:0] must be 0
    RESERVED12[15..12]           - (RO) Reserved bits
    RRO_IND_CMD_BASE_M[19..16]   - (RW) The base address of RRO indicate command ring 0
    RESERVED20[31..20]           - (RO) Reserved bits

 =====================================================================================*/
#define IND_CMD_0_CTRL_2_RRO_IND_CMD_BASE_M_ADDR          IND_CMD_0_CTRL_2_ADDR
#define IND_CMD_0_CTRL_2_RRO_IND_CMD_BASE_M_MASK          0x000F0000                // RRO_IND_CMD_BASE_M[19..16]
#define IND_CMD_0_CTRL_2_RRO_IND_CMD_BASE_M_SHFT          16
#define IND_CMD_0_CTRL_2_MAX_CNT_11_4_ADDR                IND_CMD_0_CTRL_2_ADDR
#define IND_CMD_0_CTRL_2_MAX_CNT_11_4_MASK                0x00000FF0                // MAX_CNT_11_4[11..4]
#define IND_CMD_0_CTRL_2_MAX_CNT_11_4_SHFT                4

/* =====================================================================================

  ---RRO_CONF_0 (0x15010000 + 0xE10)---

    ACK_SN_BASE_0_L[31..0]       - (RW) The base address of RRO CR ACK_SN 0

 =====================================================================================*/
#define RRO_CONF_0_ACK_SN_BASE_0_L_ADDR                   RRO_CONF_0_ADDR
#define RRO_CONF_0_ACK_SN_BASE_0_L_MASK                   0xFFFFFFFF                // ACK_SN_BASE_0_L[31..0]
#define RRO_CONF_0_ACK_SN_BASE_0_L_SHFT                   0

/* =====================================================================================

  ---RRO_CONF_1 (0x15010000 + 0xE14)---

    PARTICULAR_SE_ID[11..0]      - (RW) The particular session ID
    RESERVED12[15..12]           - (RO) Reserved bits
    ACK_SN_BASE_0_M[19..16]      - (RW) The base address of RRO CR ACK_SN 0
    RESERVED20[28..20]           - (RO) Reserved bits
    MAX_WIN_SZ[31..29]           - (RW) The maximum window size
                                     Only support 32, 64, 128, 256, 512, and 1024.
                                     0: 32, 1: 64, 2:128, 3: 256, 4: 512, 5: 1024

 =====================================================================================*/
#define RRO_CONF_1_MAX_WIN_SZ_ADDR                        RRO_CONF_1_ADDR
#define RRO_CONF_1_MAX_WIN_SZ_MASK                        0xE0000000                // MAX_WIN_SZ[31..29]
#define RRO_CONF_1_MAX_WIN_SZ_SHFT                        29
#define RRO_CONF_1_ACK_SN_BASE_0_M_ADDR                   RRO_CONF_1_ADDR
#define RRO_CONF_1_ACK_SN_BASE_0_M_MASK                   0x000F0000                // ACK_SN_BASE_0_M[19..16]
#define RRO_CONF_1_ACK_SN_BASE_0_M_SHFT                   16
#define RRO_CONF_1_PARTICULAR_SE_ID_ADDR                  RRO_CONF_1_ADDR
#define RRO_CONF_1_PARTICULAR_SE_ID_MASK                  0x00000FFF                // PARTICULAR_SE_ID[11..0]
#define RRO_CONF_1_PARTICULAR_SE_ID_SHFT                  0

/* =====================================================================================

  ---ADDR_ELEM_CONF_0 (0x15010000 + 0xE18)---

    PARTICULAR_SE_ID_ADDR_BASE_L[31..0] - (RW) The base address of Particular SE_ID of ADDR_ARRAY

 =====================================================================================*/
#define ADDR_ELEM_CONF_0_PARTICULAR_SE_ID_ADDR_BASE_L_ADDR ADDR_ELEM_CONF_0_ADDR
#define ADDR_ELEM_CONF_0_PARTICULAR_SE_ID_ADDR_BASE_L_MASK 0xFFFFFFFF                // PARTICULAR_SE_ID_ADDR_BASE_L[31..0]
#define ADDR_ELEM_CONF_0_PARTICULAR_SE_ID_ADDR_BASE_L_SHFT 0

/* =====================================================================================

  ---ADDR_ELEM_CONF_1 (0x15010000 + 0xE1C)---

    PARTICULAR_SE_ID_ADDR_BASE_M[3..0] - (RW) The base address of Particular SE_ID of ADDR_ARRAY
    RESERVED4[7..4]              - (RO) Reserved bits
    CLR_ADDR_ELEM_WSTRB[15..8]   - (RW) WED_RX_PN_CHK pn_chk_fail count acccumulated till Rd Clear
    PREFETCH_ADDR_ELEM_FREE_CNT[19..16] - (RO) The prefetch free count of ADDR_ELEMs
    RESERVED20[29..20]           - (RO) Reserved bits
    CLR_ADDR_ELEM_AXI_OP[31..30] - (RW) CLR_ADDR_ELEM AXI write operation mode:
                                     0: wait for the correct BRESP then do next run ind_cmd fetch
                                     1: wait for any BRESP of this channel-clr_addr_elem/ack_sn, then do next run ind_cmd fetch
                                     2: no waiting BRESP, do next run ind_cmd fetch directly after wlast
                                     Note that 1, 2 may encounter bus racing issue while geting quicker turn around time

 =====================================================================================*/
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_AXI_OP_ADDR        ADDR_ELEM_CONF_1_ADDR
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_AXI_OP_MASK        0xC0000000                // CLR_ADDR_ELEM_AXI_OP[31..30]
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_AXI_OP_SHFT        30
#define ADDR_ELEM_CONF_1_PREFETCH_ADDR_ELEM_FREE_CNT_ADDR ADDR_ELEM_CONF_1_ADDR
#define ADDR_ELEM_CONF_1_PREFETCH_ADDR_ELEM_FREE_CNT_MASK 0x000F0000                // PREFETCH_ADDR_ELEM_FREE_CNT[19..16]
#define ADDR_ELEM_CONF_1_PREFETCH_ADDR_ELEM_FREE_CNT_SHFT 16
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_WSTRB_ADDR         ADDR_ELEM_CONF_1_ADDR
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_WSTRB_MASK         0x0000FF00                // CLR_ADDR_ELEM_WSTRB[15..8]
#define ADDR_ELEM_CONF_1_CLR_ADDR_ELEM_WSTRB_SHFT         8
#define ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M_ADDR ADDR_ELEM_CONF_1_ADDR
#define ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M_MASK 0x0000000F                // PARTICULAR_SE_ID_ADDR_BASE_M[3..0]
#define ADDR_ELEM_CONF_1_PARTICULAR_SE_ID_ADDR_BASE_M_SHFT 0

/* =====================================================================================

  ---ADDR_ELEM_BASE_TBL_CONF (0x15010000 + 0xE20)---

    BASE_TBL_OFST[6..0]          - (RW) ADDR_ELEM Base Addr TBL offset - Session ID[9:3]- to Rd/Wr, 8 session IDs are put to 1 base addr range (max. 64KB: 8x1024 x8B)
    RESERVED7[27..7]             - (RO) Reserved bits
    RD_RDY[28]                   - (RO) 0: ADDR_ELEM_BASE_TBL Read is not done
                                     1: ADDR_ELEM_BASE_TBL Read is done
    WR_RDY[29]                   - (RO) 0: ADDR_ELEM_BASE_TBL Write is not done
                                     1: ADDR_ELEM_BASE_TBL Write is done
    RD[30]                       - (A0) 1: read data to ADDR_ELEM_BASE_TBL
                                     HW clear to 0
    WR[31]                       - (A0) 1: write data to ADDR_ELEM_BASE_TBL
                                     HW clear to 0

 =====================================================================================*/
#define ADDR_ELEM_BASE_TBL_CONF_WR_ADDR                   ADDR_ELEM_BASE_TBL_CONF_ADDR
#define ADDR_ELEM_BASE_TBL_CONF_WR_MASK                   0x80000000                // WR[31]
#define ADDR_ELEM_BASE_TBL_CONF_WR_SHFT                   31
#define ADDR_ELEM_BASE_TBL_CONF_RD_ADDR                   ADDR_ELEM_BASE_TBL_CONF_ADDR
#define ADDR_ELEM_BASE_TBL_CONF_RD_MASK                   0x40000000                // RD[30]
#define ADDR_ELEM_BASE_TBL_CONF_RD_SHFT                   30
#define ADDR_ELEM_BASE_TBL_CONF_WR_RDY_ADDR               ADDR_ELEM_BASE_TBL_CONF_ADDR
#define ADDR_ELEM_BASE_TBL_CONF_WR_RDY_MASK               0x20000000                // WR_RDY[29]
#define ADDR_ELEM_BASE_TBL_CONF_WR_RDY_SHFT               29
#define ADDR_ELEM_BASE_TBL_CONF_RD_RDY_ADDR               ADDR_ELEM_BASE_TBL_CONF_ADDR
#define ADDR_ELEM_BASE_TBL_CONF_RD_RDY_MASK               0x10000000                // RD_RDY[28]
#define ADDR_ELEM_BASE_TBL_CONF_RD_RDY_SHFT               28
#define ADDR_ELEM_BASE_TBL_CONF_BASE_TBL_OFST_ADDR        ADDR_ELEM_BASE_TBL_CONF_ADDR
#define ADDR_ELEM_BASE_TBL_CONF_BASE_TBL_OFST_MASK        0x0000007F                // BASE_TBL_OFST[6..0]
#define ADDR_ELEM_BASE_TBL_CONF_BASE_TBL_OFST_SHFT        0

/* =====================================================================================

  ---ADDR_ELEM_BASE_TBL_WDATA (0x15010000 + 0xE24)---

    W_DATA[31..0]                - (RW) ADDR_ELEM base addr TBL Write DATA

 =====================================================================================*/
#define ADDR_ELEM_BASE_TBL_WDATA_W_DATA_ADDR              ADDR_ELEM_BASE_TBL_WDATA_ADDR
#define ADDR_ELEM_BASE_TBL_WDATA_W_DATA_MASK              0xFFFFFFFF                // W_DATA[31..0]
#define ADDR_ELEM_BASE_TBL_WDATA_W_DATA_SHFT              0

/* =====================================================================================

  ---ADDR_ELEM_BASE_TBL_RDATA (0x15010000 + 0xE28)---

    R_DATA[31..0]                - (RO) ADDR_ELEM base addr TBL Read DATA

 =====================================================================================*/
#define ADDR_ELEM_BASE_TBL_RDATA_R_DATA_ADDR              ADDR_ELEM_BASE_TBL_RDATA_ADDR
#define ADDR_ELEM_BASE_TBL_RDATA_R_DATA_MASK              0xFFFFFFFF                // R_DATA[31..0]
#define ADDR_ELEM_BASE_TBL_RDATA_R_DATA_SHFT              0

/* =====================================================================================

  ---RRO_MSDU_PG_RING2_CFG1 (0x15010000 + 0xE2C)---

    RESERVED0[7..0]              - (RO) Reserved bits
    POLL_CYCLE[15..8]            - (RW) Polling mode period (cycle x 1us)
    R0_INT_FLAG[16]              - (RO) Service R0 interrupt now
    R1_INT_FLAG[17]              - (RO) Service R1 interrupt now
    R2_INT_FLAG[18]              - (RO) Service R2 interrupt now
    FORCE_IDLE_WHEN_DRV_ERROR[19] - (RW) Force return idle state when detect error
    INIT_COMPLETE_FLAG[20]       - (RO) Initialization is done
    POLL_FLAG[21]                - (RO) Serive polling
    BUSY_FLAG[22]                - (RO) DRV is busy
    ERR_LAT_OPT[23]              - (RW) Latch awaddr when detect error
    DMAD_DBG_EN[24]              - (RW) Add dmad seq count for debug
    DRV_DBG_CLR[25]              - (A0) DRV DBG SW reset
    DRV_CLR[26]                  - (A0) DRV SW reset, force to reset state
    INIT_FLAG_CLR[27]            - (A0) Clear initialization flag
    FORCE_RETURN_IDLE[28]        - (RW) Force return idle state
    FORCE_IDLE_WHEN_DRV_DISABLE[29] - (RW) Force return idle state when set DRV_EN to 1'b0
    INT_PROC_DISABLE[30]         - (RW) Disable initialization procedure
    DRV_EN[31]                   - (RW) RRO MSDU PG DRV Enable

 =====================================================================================*/
#define RRO_MSDU_PG_RING2_CFG1_DRV_EN_ADDR                RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_DRV_EN_MASK                0x80000000                // DRV_EN[31]
#define RRO_MSDU_PG_RING2_CFG1_DRV_EN_SHFT                31
#define RRO_MSDU_PG_RING2_CFG1_INT_PROC_DISABLE_ADDR      RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_INT_PROC_DISABLE_MASK      0x40000000                // INT_PROC_DISABLE[30]
#define RRO_MSDU_PG_RING2_CFG1_INT_PROC_DISABLE_SHFT      30
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_DISABLE_ADDR RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_DISABLE_MASK 0x20000000                // FORCE_IDLE_WHEN_DRV_DISABLE[29]
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_DISABLE_SHFT 29
#define RRO_MSDU_PG_RING2_CFG1_FORCE_RETURN_IDLE_ADDR     RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_FORCE_RETURN_IDLE_MASK     0x10000000                // FORCE_RETURN_IDLE[28]
#define RRO_MSDU_PG_RING2_CFG1_FORCE_RETURN_IDLE_SHFT     28
#define RRO_MSDU_PG_RING2_CFG1_INIT_FLAG_CLR_ADDR         RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_INIT_FLAG_CLR_MASK         0x08000000                // INIT_FLAG_CLR[27]
#define RRO_MSDU_PG_RING2_CFG1_INIT_FLAG_CLR_SHFT         27
#define RRO_MSDU_PG_RING2_CFG1_DRV_CLR_ADDR               RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_DRV_CLR_MASK               0x04000000                // DRV_CLR[26]
#define RRO_MSDU_PG_RING2_CFG1_DRV_CLR_SHFT               26
#define RRO_MSDU_PG_RING2_CFG1_DRV_DBG_CLR_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_DRV_DBG_CLR_MASK           0x02000000                // DRV_DBG_CLR[25]
#define RRO_MSDU_PG_RING2_CFG1_DRV_DBG_CLR_SHFT           25
#define RRO_MSDU_PG_RING2_CFG1_DMAD_DBG_EN_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_DMAD_DBG_EN_MASK           0x01000000                // DMAD_DBG_EN[24]
#define RRO_MSDU_PG_RING2_CFG1_DMAD_DBG_EN_SHFT           24
#define RRO_MSDU_PG_RING2_CFG1_ERR_LAT_OPT_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_ERR_LAT_OPT_MASK           0x00800000                // ERR_LAT_OPT[23]
#define RRO_MSDU_PG_RING2_CFG1_ERR_LAT_OPT_SHFT           23
#define RRO_MSDU_PG_RING2_CFG1_BUSY_FLAG_ADDR             RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_BUSY_FLAG_MASK             0x00400000                // BUSY_FLAG[22]
#define RRO_MSDU_PG_RING2_CFG1_BUSY_FLAG_SHFT             22
#define RRO_MSDU_PG_RING2_CFG1_POLL_FLAG_ADDR             RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_POLL_FLAG_MASK             0x00200000                // POLL_FLAG[21]
#define RRO_MSDU_PG_RING2_CFG1_POLL_FLAG_SHFT             21
#define RRO_MSDU_PG_RING2_CFG1_INIT_COMPLETE_FLAG_ADDR    RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_INIT_COMPLETE_FLAG_MASK    0x00100000                // INIT_COMPLETE_FLAG[20]
#define RRO_MSDU_PG_RING2_CFG1_INIT_COMPLETE_FLAG_SHFT    20
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_ERROR_ADDR RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_ERROR_MASK 0x00080000                // FORCE_IDLE_WHEN_DRV_ERROR[19]
#define RRO_MSDU_PG_RING2_CFG1_FORCE_IDLE_WHEN_DRV_ERROR_SHFT 19
#define RRO_MSDU_PG_RING2_CFG1_R2_INT_FLAG_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_R2_INT_FLAG_MASK           0x00040000                // R2_INT_FLAG[18]
#define RRO_MSDU_PG_RING2_CFG1_R2_INT_FLAG_SHFT           18
#define RRO_MSDU_PG_RING2_CFG1_R1_INT_FLAG_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_R1_INT_FLAG_MASK           0x00020000                // R1_INT_FLAG[17]
#define RRO_MSDU_PG_RING2_CFG1_R1_INT_FLAG_SHFT           17
#define RRO_MSDU_PG_RING2_CFG1_R0_INT_FLAG_ADDR           RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_R0_INT_FLAG_MASK           0x00010000                // R0_INT_FLAG[16]
#define RRO_MSDU_PG_RING2_CFG1_R0_INT_FLAG_SHFT           16
#define RRO_MSDU_PG_RING2_CFG1_POLL_CYCLE_ADDR            RRO_MSDU_PG_RING2_CFG1_ADDR
#define RRO_MSDU_PG_RING2_CFG1_POLL_CYCLE_MASK            0x0000FF00                // POLL_CYCLE[15..8]
#define RRO_MSDU_PG_RING2_CFG1_POLL_CYCLE_SHFT            8

/* =====================================================================================

  ---PN_CONF_0 (0x15010000 + 0xE30)---

    SE_ID[11..0]                 - (RW) Session ID, only [9:0] is used for Jaguar
    RESERVED12[27..12]           - (RO) Reserved bits
    PN_RD_RDY[28]                - (RO) 0: PN TBL Read is not done
                                     1: PN TBL Read is done
    PN_WR_RDY[29]                - (RO) 0: PN TBL Write is not done
                                     1: PN TBL Write is done
    PN_RD[30]                    - (A0) 1: read data to PN Table of session id
                                     HW clear to 0
    PN_WR[31]                    - (A0) 1: write data to PN Table of session id
                                     HW clear to 0

 =====================================================================================*/
#define PN_CONF_0_PN_WR_ADDR                              PN_CONF_0_ADDR
#define PN_CONF_0_PN_WR_MASK                              0x80000000                // PN_WR[31]
#define PN_CONF_0_PN_WR_SHFT                              31
#define PN_CONF_0_PN_RD_ADDR                              PN_CONF_0_ADDR
#define PN_CONF_0_PN_RD_MASK                              0x40000000                // PN_RD[30]
#define PN_CONF_0_PN_RD_SHFT                              30
#define PN_CONF_0_PN_WR_RDY_ADDR                          PN_CONF_0_ADDR
#define PN_CONF_0_PN_WR_RDY_MASK                          0x20000000                // PN_WR_RDY[29]
#define PN_CONF_0_PN_WR_RDY_SHFT                          29
#define PN_CONF_0_PN_RD_RDY_ADDR                          PN_CONF_0_ADDR
#define PN_CONF_0_PN_RD_RDY_MASK                          0x10000000                // PN_RD_RDY[28]
#define PN_CONF_0_PN_RD_RDY_SHFT                          28
#define PN_CONF_0_SE_ID_ADDR                              PN_CONF_0_ADDR
#define PN_CONF_0_SE_ID_MASK                              0x00000FFF                // SE_ID[11..0]
#define PN_CONF_0_SE_ID_SHFT                              0

/* =====================================================================================

  ---PN_CONF_WDATA_L (0x15010000 + 0xE34)---

    PN_31_0[31..0]               - (RW) Packet number

 =====================================================================================*/
#define PN_CONF_WDATA_L_PN_31_0_ADDR                      PN_CONF_WDATA_L_ADDR
#define PN_CONF_WDATA_L_PN_31_0_MASK                      0xFFFFFFFF                // PN_31_0[31..0]
#define PN_CONF_WDATA_L_PN_31_0_SHFT                      0

/* =====================================================================================

  ---PN_CONF_WDATA_M (0x15010000 + 0xE38)---

    PN_47_32[15..0]              - (RW) Packet number
    NEED_CHECK_PN[16]            - (RW) Need to check pn of session ID or not
    IS_FIRST[17]                 - (RW) If this bit is 1, WED will update last pn anyway and set this field to 0
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define PN_CONF_WDATA_M_IS_FIRST_ADDR                     PN_CONF_WDATA_M_ADDR
#define PN_CONF_WDATA_M_IS_FIRST_MASK                     0x00020000                // IS_FIRST[17]
#define PN_CONF_WDATA_M_IS_FIRST_SHFT                     17
#define PN_CONF_WDATA_M_NEED_CHECK_PN_ADDR                PN_CONF_WDATA_M_ADDR
#define PN_CONF_WDATA_M_NEED_CHECK_PN_MASK                0x00010000                // NEED_CHECK_PN[16]
#define PN_CONF_WDATA_M_NEED_CHECK_PN_SHFT                16
#define PN_CONF_WDATA_M_PN_47_32_ADDR                     PN_CONF_WDATA_M_ADDR
#define PN_CONF_WDATA_M_PN_47_32_MASK                     0x0000FFFF                // PN_47_32[15..0]
#define PN_CONF_WDATA_M_PN_47_32_SHFT                     0

/* =====================================================================================

  ---PN_CONF_RDATA_L (0x15010000 + 0xE3C)---

    PN_31_0[31..0]               - (RO) Packet number

 =====================================================================================*/
#define PN_CONF_RDATA_L_PN_31_0_ADDR                      PN_CONF_RDATA_L_ADDR
#define PN_CONF_RDATA_L_PN_31_0_MASK                      0xFFFFFFFF                // PN_31_0[31..0]
#define PN_CONF_RDATA_L_PN_31_0_SHFT                      0

/* =====================================================================================

  ---PN_CONF_RDATA_M (0x15010000 + 0xE40)---

    PN_47_32[15..0]              - (RO) Packet number
    NEED_CHECK_PN[16]            - (RO) Need to check pn of session ID or not
    IS_FIRST[17]                 - (RO) If this bit is 1, WED will update last pn anyway and set this field to 0
    RESERVED18[31..18]           - (RO) Reserved bits

 =====================================================================================*/
#define PN_CONF_RDATA_M_IS_FIRST_ADDR                     PN_CONF_RDATA_M_ADDR
#define PN_CONF_RDATA_M_IS_FIRST_MASK                     0x00020000                // IS_FIRST[17]
#define PN_CONF_RDATA_M_IS_FIRST_SHFT                     17
#define PN_CONF_RDATA_M_NEED_CHECK_PN_ADDR                PN_CONF_RDATA_M_ADDR
#define PN_CONF_RDATA_M_NEED_CHECK_PN_MASK                0x00010000                // NEED_CHECK_PN[16]
#define PN_CONF_RDATA_M_NEED_CHECK_PN_SHFT                16
#define PN_CONF_RDATA_M_PN_47_32_ADDR                     PN_CONF_RDATA_M_ADDR
#define PN_CONF_RDATA_M_PN_47_32_MASK                     0x0000FFFF                // PN_47_32[15..0]
#define PN_CONF_RDATA_M_PN_47_32_SHFT                     0

/* =====================================================================================

  ---RRO_MSDU_PG_RING0_DIDX_CFG0 (0x15010000 + 0xE44)---

    MSDU_PG_ADDR_R0_L[31..0]     - (RW) The CR dma index address of RRO MSDU PAGE RING 0 - low

 =====================================================================================*/
#define RRO_MSDU_PG_RING0_DIDX_CFG0_MSDU_PG_ADDR_R0_L_ADDR RRO_MSDU_PG_RING0_DIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING0_DIDX_CFG0_MSDU_PG_ADDR_R0_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R0_L[31..0]
#define RRO_MSDU_PG_RING0_DIDX_CFG0_MSDU_PG_ADDR_R0_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING0_DIDX_CFG1 (0x15010000 + 0xE48)---

    MSDU_PG_ADDR_R0_M[3..0]      - (RW) The CR dma index address of RRO MSDU PAGE RING 0 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING0_DIDX_CFG1_MSDU_PG_ADDR_R0_M_ADDR RRO_MSDU_PG_RING0_DIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING0_DIDX_CFG1_MSDU_PG_ADDR_R0_M_MASK 0x0000000F                // MSDU_PG_ADDR_R0_M[3..0]
#define RRO_MSDU_PG_RING0_DIDX_CFG1_MSDU_PG_ADDR_R0_M_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING1_DIDX_CFG0 (0x15010000 + 0xE4C)---

    MSDU_PG_ADDR_R1_L[31..0]     - (RW) The CR dma index address of RRO MSDU PAGE RING 1- low

 =====================================================================================*/
#define RRO_MSDU_PG_RING1_DIDX_CFG0_MSDU_PG_ADDR_R1_L_ADDR RRO_MSDU_PG_RING1_DIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING1_DIDX_CFG0_MSDU_PG_ADDR_R1_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R1_L[31..0]
#define RRO_MSDU_PG_RING1_DIDX_CFG0_MSDU_PG_ADDR_R1_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING1_DIDX_CFG1 (0x15010000 + 0xE50)---

    MSDU_PG_ADDR_R1_M[3..0]      - (RW) The CR dma index address of RRO MSDU PAGE RING 1 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING1_DIDX_CFG1_MSDU_PG_ADDR_R1_M_ADDR RRO_MSDU_PG_RING1_DIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING1_DIDX_CFG1_MSDU_PG_ADDR_R1_M_MASK 0x0000000F                // MSDU_PG_ADDR_R1_M[3..0]
#define RRO_MSDU_PG_RING1_DIDX_CFG1_MSDU_PG_ADDR_R1_M_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING2_DIDX_CFG0 (0x15010000 + 0xE54)---

    MSDU_PG_ADDR_R2_L[31..0]     - (RW) The CR dma index address of RRO MSDU PAGE RING 2 - low

 =====================================================================================*/
#define RRO_MSDU_PG_RING2_DIDX_CFG0_MSDU_PG_ADDR_R2_L_ADDR RRO_MSDU_PG_RING2_DIDX_CFG0_ADDR
#define RRO_MSDU_PG_RING2_DIDX_CFG0_MSDU_PG_ADDR_R2_L_MASK 0xFFFFFFFF                // MSDU_PG_ADDR_R2_L[31..0]
#define RRO_MSDU_PG_RING2_DIDX_CFG0_MSDU_PG_ADDR_R2_L_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_RING2_DIDX_CFG1 (0x15010000 + 0xE58)---

    MSDU_PG_ADDR_R2_M[3..0]      - (RW) The CR dma index address of RRO MSDU PAGE RING 2 - med
    RESERVED4[31..4]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_RING2_DIDX_CFG1_MSDU_PG_ADDR_R2_M_ADDR RRO_MSDU_PG_RING2_DIDX_CFG1_ADDR
#define RRO_MSDU_PG_RING2_DIDX_CFG1_MSDU_PG_ADDR_R2_M_MASK 0x0000000F                // MSDU_PG_ADDR_R2_M[3..0]
#define RRO_MSDU_PG_RING2_DIDX_CFG1_MSDU_PG_ADDR_R2_M_SHFT 0

/* =====================================================================================

  ---RRO_MSDU_PG_0_CTRL0 (0x15010000 + 0xE5C)---

    BASE_PTR_L[31..0]            - (RW) Point to the base address of RRO MSDU_PAGE Ring #0.

 =====================================================================================*/
#define RRO_MSDU_PG_0_CTRL0_BASE_PTR_L_ADDR               RRO_MSDU_PG_0_CTRL0_ADDR
#define RRO_MSDU_PG_0_CTRL0_BASE_PTR_L_MASK               0xFFFFFFFF                // BASE_PTR_L[31..0]
#define RRO_MSDU_PG_0_CTRL0_BASE_PTR_L_SHFT               0

/* =====================================================================================

  ---RRO_MSDU_PG_0_CTRL1 (0x15010000 + 0xE60)---

    MAX_CNT[11..0]               - (RW) The maximum number of page count in RRO MSDU_PAGE Ring #0.
                                     Set 0 is Disable this ring, driver does not access Ring, cpu index and dma index.
                                     set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_M[23..16]           - (RW) Point to the base address of RRO MSDU_PAGE Ring #0.
    RESERVED24[27..24]           - (RO) Reserved bits
    MAGIC_CNT[31..28]            - (RO) The magic count of RRO MSDU_PAGE Ring #0.

 =====================================================================================*/
#define RRO_MSDU_PG_0_CTRL1_MAGIC_CNT_ADDR                RRO_MSDU_PG_0_CTRL1_ADDR
#define RRO_MSDU_PG_0_CTRL1_MAGIC_CNT_MASK                0xF0000000                // MAGIC_CNT[31..28]
#define RRO_MSDU_PG_0_CTRL1_MAGIC_CNT_SHFT                28
#define RRO_MSDU_PG_0_CTRL1_BASE_PTR_M_ADDR               RRO_MSDU_PG_0_CTRL1_ADDR
#define RRO_MSDU_PG_0_CTRL1_BASE_PTR_M_MASK               0x00FF0000                // BASE_PTR_M[23..16]
#define RRO_MSDU_PG_0_CTRL1_BASE_PTR_M_SHFT               16
#define RRO_MSDU_PG_0_CTRL1_MAX_CNT_ADDR                  RRO_MSDU_PG_0_CTRL1_ADDR
#define RRO_MSDU_PG_0_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define RRO_MSDU_PG_0_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---RRO_MSDU_PG_0_CTRL2 (0x15010000 + 0xE64)---

    CPU_IDX[11..0]               - (RO) Point to the next RXD hardware driver agent wants to allocate to MSDU_PAGE address  Ring #0.
    RESERVED12[15..12]           - (RO) Reserved bits
    DMA_IDX_MIRO[27..16]         - (RO) Mirror of RRO MSDU_PAGE Ring #0 DMA_IDX.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_0_CTRL2_DMA_IDX_MIRO_ADDR             RRO_MSDU_PG_0_CTRL2_ADDR
#define RRO_MSDU_PG_0_CTRL2_DMA_IDX_MIRO_MASK             0x0FFF0000                // DMA_IDX_MIRO[27..16]
#define RRO_MSDU_PG_0_CTRL2_DMA_IDX_MIRO_SHFT             16
#define RRO_MSDU_PG_0_CTRL2_CPU_IDX_ADDR                  RRO_MSDU_PG_0_CTRL2_ADDR
#define RRO_MSDU_PG_0_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define RRO_MSDU_PG_0_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---RRO_MSDU_PG_1_CTRL0 (0x15010000 + 0xE68)---

    BASE_PTR_L[31..0]            - (RW) Point to the base address of RRO MSDU_PAGE Ring #1.

 =====================================================================================*/
#define RRO_MSDU_PG_1_CTRL0_BASE_PTR_L_ADDR               RRO_MSDU_PG_1_CTRL0_ADDR
#define RRO_MSDU_PG_1_CTRL0_BASE_PTR_L_MASK               0xFFFFFFFF                // BASE_PTR_L[31..0]
#define RRO_MSDU_PG_1_CTRL0_BASE_PTR_L_SHFT               0

/* =====================================================================================

  ---RRO_MSDU_PG_1_CTRL1 (0x15010000 + 0xE6C)---

    MAX_CNT[11..0]               - (RW) The maximum number of page count in RRO MSDU_PAGE Ring #1.
                                     Set 0 is Disable this ring, driver does not access Ring, cpu index and dma index.
                                     set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_M[23..16]           - (RW) Point to the base address of RRO MSDU_PAGE Ring #1.
    RESERVED24[27..24]           - (RO) Reserved bits
    MAGIC_CNT[31..28]            - (RO) The magic count of RRO MSDU_PAGE Ring #1.

 =====================================================================================*/
#define RRO_MSDU_PG_1_CTRL1_MAGIC_CNT_ADDR                RRO_MSDU_PG_1_CTRL1_ADDR
#define RRO_MSDU_PG_1_CTRL1_MAGIC_CNT_MASK                0xF0000000                // MAGIC_CNT[31..28]
#define RRO_MSDU_PG_1_CTRL1_MAGIC_CNT_SHFT                28
#define RRO_MSDU_PG_1_CTRL1_BASE_PTR_M_ADDR               RRO_MSDU_PG_1_CTRL1_ADDR
#define RRO_MSDU_PG_1_CTRL1_BASE_PTR_M_MASK               0x00FF0000                // BASE_PTR_M[23..16]
#define RRO_MSDU_PG_1_CTRL1_BASE_PTR_M_SHFT               16
#define RRO_MSDU_PG_1_CTRL1_MAX_CNT_ADDR                  RRO_MSDU_PG_1_CTRL1_ADDR
#define RRO_MSDU_PG_1_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define RRO_MSDU_PG_1_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---RRO_MSDU_PG_1_CTRL2 (0x15010000 + 0xE70)---

    CPU_IDX[11..0]               - (RO) Point to the next RXD hardware driver agent wants to allocate to MSDU_PAGE address  Ring #1.
    RESERVED12[15..12]           - (RO) Reserved bits
    DMA_IDX_MIRO[27..16]         - (RO) Mirror of RRO MSDU_PAGE Ring #1 DMA_IDX.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_MSDU_PG_1_CTRL2_DMA_IDX_MIRO_ADDR             RRO_MSDU_PG_1_CTRL2_ADDR
#define RRO_MSDU_PG_1_CTRL2_DMA_IDX_MIRO_MASK             0x0FFF0000                // DMA_IDX_MIRO[27..16]
#define RRO_MSDU_PG_1_CTRL2_DMA_IDX_MIRO_SHFT             16
#define RRO_MSDU_PG_1_CTRL2_CPU_IDX_ADDR                  RRO_MSDU_PG_1_CTRL2_ADDR
#define RRO_MSDU_PG_1_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define RRO_MSDU_PG_1_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---RRO_MSDU_PG_2_CTRL0 (0x15010000 + 0xE74)---

    BASE_PTR_L[31..0]            - (RW) Point to the base address of RRO MSDU_PAGE Ring #2.

 =====================================================================================*/
#define RRO_MSDU_PG_2_CTRL0_BASE_PTR_L_ADDR               RRO_MSDU_PG_2_CTRL0_ADDR
#define RRO_MSDU_PG_2_CTRL0_BASE_PTR_L_MASK               0xFFFFFFFF                // BASE_PTR_L[31..0]
#define RRO_MSDU_PG_2_CTRL0_BASE_PTR_L_SHFT               0

/* =====================================================================================

  ---RRO_MSDU_PG_2_CTRL1 (0x15010000 + 0xE78)---

    MAX_CNT[11..0]               - (RW) The maximum number of page count in RRO MSDU_PAGE Ring #2.
                                     Set 0 is Disable this ring, driver does not access Ring, cpu index and dma index.
                                     set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    BASE_PTR_M[23..16]           - (RW) Point to the base address of RRO MSDU_PAGE Ring #2.
    RESERVED24[27..24]           - (RO) Reserved bits
    MAGIC_CNT[31..28]            - (RO) The magic count of RRO MSDU_PAGE Ring #2.

 =====================================================================================*/
#define RRO_MSDU_PG_2_CTRL1_MAGIC_CNT_ADDR                RRO_MSDU_PG_2_CTRL1_ADDR
#define RRO_MSDU_PG_2_CTRL1_MAGIC_CNT_MASK                0xF0000000                // MAGIC_CNT[31..28]
#define RRO_MSDU_PG_2_CTRL1_MAGIC_CNT_SHFT                28
#define RRO_MSDU_PG_2_CTRL1_BASE_PTR_M_ADDR               RRO_MSDU_PG_2_CTRL1_ADDR
#define RRO_MSDU_PG_2_CTRL1_BASE_PTR_M_MASK               0x00FF0000                // BASE_PTR_M[23..16]
#define RRO_MSDU_PG_2_CTRL1_BASE_PTR_M_SHFT               16
#define RRO_MSDU_PG_2_CTRL1_MAX_CNT_ADDR                  RRO_MSDU_PG_2_CTRL1_ADDR
#define RRO_MSDU_PG_2_CTRL1_MAX_CNT_MASK                  0x00000FFF                // MAX_CNT[11..0]
#define RRO_MSDU_PG_2_CTRL1_MAX_CNT_SHFT                  0

/* =====================================================================================

  ---RRO_MSDU_PG_2_CTRL2 (0x15010000 + 0xE7C)---

    CPU_IDX[11..0]               - (RO) Point to the next RXD hardware driver agent wants to allocate to MSDU_PAGE address  Ring #2.
    RESERVED12[15..12]           - (RO) Reserved bits
    DMA_IDX_MIRO[27..16]         - (RO) Mirror of RRO MSDU_PAGE Ring #2 DMA_IDX.
    RESERVED28[30..28]           - (RO) Reserved bits
    BURST_MODE_DISABLE[31]       - (RW) Disable burst re-fill DMAD mode, re-fill one DMAD, move one CIDX

 =====================================================================================*/
#define RRO_MSDU_PG_2_CTRL2_BURST_MODE_DISABLE_ADDR       RRO_MSDU_PG_2_CTRL2_ADDR
#define RRO_MSDU_PG_2_CTRL2_BURST_MODE_DISABLE_MASK       0x80000000                // BURST_MODE_DISABLE[31]
#define RRO_MSDU_PG_2_CTRL2_BURST_MODE_DISABLE_SHFT       31
#define RRO_MSDU_PG_2_CTRL2_DMA_IDX_MIRO_ADDR             RRO_MSDU_PG_2_CTRL2_ADDR
#define RRO_MSDU_PG_2_CTRL2_DMA_IDX_MIRO_MASK             0x0FFF0000                // DMA_IDX_MIRO[27..16]
#define RRO_MSDU_PG_2_CTRL2_DMA_IDX_MIRO_SHFT             16
#define RRO_MSDU_PG_2_CTRL2_CPU_IDX_ADDR                  RRO_MSDU_PG_2_CTRL2_ADDR
#define RRO_MSDU_PG_2_CTRL2_CPU_IDX_MASK                  0x00000FFF                // CPU_IDX[11..0]
#define RRO_MSDU_PG_2_CTRL2_CPU_IDX_SHFT                  0

/* =====================================================================================

  ---RRO_RX_D_RX0_BASE (0x15010000 + 0xE80)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)(LSB)

 =====================================================================================*/
#define RRO_RX_D_RX0_BASE_PTR_ADDR                        RRO_RX_D_RX0_BASE_ADDR
#define RRO_RX_D_RX0_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define RRO_RX_D_RX0_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---RRO_RX_D_RX0_CNT (0x15010000 + 0xE84)---

    MAX[11..0]                   - (RW) The maximum number of RX DMAD count in RX Ring #0
                                     Set 0 is disable this ring, driver does not access Ring, cpu index and dma index.
                                     Set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    PTR_H[23..16]                - (RW) Point to the base address of RX Ring #0 (4-DW aligned address)(MSB)
    RESERVED24[27..24]           - (RO) Reserved bits
    MAGIC_CNT[31..28]            - (RO) The MAGIC_CNT used when WED reloads RXDMAD to WFDMA.
                                     Advance MAGIC_CNT after WED updates RRO_RX_D_RX0_CRX_IDX to 0x0.

 =====================================================================================*/
#define RRO_RX_D_RX0_CNT_MAGIC_CNT_ADDR                   RRO_RX_D_RX0_CNT_ADDR
#define RRO_RX_D_RX0_CNT_MAGIC_CNT_MASK                   0xF0000000                // MAGIC_CNT[31..28]
#define RRO_RX_D_RX0_CNT_MAGIC_CNT_SHFT                   28
#define RRO_RX_D_RX0_CNT_PTR_H_ADDR                       RRO_RX_D_RX0_CNT_ADDR
#define RRO_RX_D_RX0_CNT_PTR_H_MASK                       0x00FF0000                // PTR_H[23..16]
#define RRO_RX_D_RX0_CNT_PTR_H_SHFT                       16
#define RRO_RX_D_RX0_CNT_MAX_ADDR                         RRO_RX_D_RX0_CNT_ADDR
#define RRO_RX_D_RX0_CNT_MAX_MASK                         0x00000FFF                // MAX[11..0]
#define RRO_RX_D_RX0_CNT_MAX_SHFT                         0

/* =====================================================================================

  ---RRO_RX_D_RX0_CRX_IDX (0x15010000 + 0xE88)---

    CRX_IDX[11..0]               - (RO) Point to the next RX DMAD hardware driver agent wants to use, this index of WPDMA is controlled by WED hardware.
    RESERVED12[15..12]           - (RO) Reserved bits
    DRV_IDX[27..16]              - (RO) Point to the next RX DMAD hardware driver agent wants to process, this index is controlled by WED hardware.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RX0_CRX_IDX_DRV_IDX_ADDR                 RRO_RX_D_RX0_CRX_IDX_ADDR
#define RRO_RX_D_RX0_CRX_IDX_DRV_IDX_MASK                 0x0FFF0000                // DRV_IDX[27..16]
#define RRO_RX_D_RX0_CRX_IDX_DRV_IDX_SHFT                 16
#define RRO_RX_D_RX0_CRX_IDX_CRX_IDX_ADDR                 RRO_RX_D_RX0_CRX_IDX_ADDR
#define RRO_RX_D_RX0_CRX_IDX_CRX_IDX_MASK                 0x00000FFF                // CRX_IDX[11..0]
#define RRO_RX_D_RX0_CRX_IDX_CRX_IDX_SHFT                 0

/* =====================================================================================

  ---RRO_RX_D_RX0_DRX_IDX (0x15010000 + 0xE8C)---

    DRX_IDX_MIRO[11..0]          - (RO) Point to the next RX DMAD which DMA wants to use, this is a mirrored value of WPDMA RX_DRX_IDX.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_ADDR            RRO_RX_D_RX0_DRX_IDX_ADDR
#define RRO_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_MASK            0x00000FFF                // DRX_IDX_MIRO[11..0]
#define RRO_RX_D_RX0_DRX_IDX_DRX_IDX_MIRO_SHFT            0

/* =====================================================================================

  ---RRO_RX_D_RX1_BASE (0x15010000 + 0xE90)---

    PTR[31..0]                   - (RW) Point to the base address of RX Ring #1 (4-DW aligned address)(LSB)

 =====================================================================================*/
#define RRO_RX_D_RX1_BASE_PTR_ADDR                        RRO_RX_D_RX1_BASE_ADDR
#define RRO_RX_D_RX1_BASE_PTR_MASK                        0xFFFFFFFF                // PTR[31..0]
#define RRO_RX_D_RX1_BASE_PTR_SHFT                        0

/* =====================================================================================

  ---RRO_RX_D_RX1_CNT (0x15010000 + 0xE94)---

    MAX[11..0]                   - (RW) The maximum number of RX DMAD count in RX Ring #1
                                     Set 0 is disable this ring, driver does not access Ring, cpu index and dma index.
                                     Set 1 is illegal.
    RESERVED12[15..12]           - (RO) Reserved bits
    PTR_H[23..16]                - (RW) Point to the base address of RX Ring #1 (4-DW aligned address)(MSB)
    RESERVED24[27..24]           - (RO) Reserved bits
    MAGIC_CNT[31..28]            - (RO) The MAGIC_CNT used when WED reloads RXDMAD to WFDMA.
                                     Advance MAGIC_CNT after WED updates RRO_RX_D_RX1_CRX_IDX to 0x0.

 =====================================================================================*/
#define RRO_RX_D_RX1_CNT_MAGIC_CNT_ADDR                   RRO_RX_D_RX1_CNT_ADDR
#define RRO_RX_D_RX1_CNT_MAGIC_CNT_MASK                   0xF0000000                // MAGIC_CNT[31..28]
#define RRO_RX_D_RX1_CNT_MAGIC_CNT_SHFT                   28
#define RRO_RX_D_RX1_CNT_PTR_H_ADDR                       RRO_RX_D_RX1_CNT_ADDR
#define RRO_RX_D_RX1_CNT_PTR_H_MASK                       0x00FF0000                // PTR_H[23..16]
#define RRO_RX_D_RX1_CNT_PTR_H_SHFT                       16
#define RRO_RX_D_RX1_CNT_MAX_ADDR                         RRO_RX_D_RX1_CNT_ADDR
#define RRO_RX_D_RX1_CNT_MAX_MASK                         0x00000FFF                // MAX[11..0]
#define RRO_RX_D_RX1_CNT_MAX_SHFT                         0

/* =====================================================================================

  ---RRO_RX_D_RX1_CRX_IDX (0x15010000 + 0xE98)---

    CRX_IDX[11..0]               - (RO) Point to the next RX DMAD hardware driver agent wants to use, this index of WPDMA is controlled by WED hardware.
    RESERVED12[15..12]           - (RO) Reserved bits
    DRV_IDX[27..16]              - (RO) Point to the next RX DMAD hardware driver agent wants to process, this index is controlled by WED hardware.
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RX1_CRX_IDX_DRV_IDX_ADDR                 RRO_RX_D_RX1_CRX_IDX_ADDR
#define RRO_RX_D_RX1_CRX_IDX_DRV_IDX_MASK                 0x0FFF0000                // DRV_IDX[27..16]
#define RRO_RX_D_RX1_CRX_IDX_DRV_IDX_SHFT                 16
#define RRO_RX_D_RX1_CRX_IDX_CRX_IDX_ADDR                 RRO_RX_D_RX1_CRX_IDX_ADDR
#define RRO_RX_D_RX1_CRX_IDX_CRX_IDX_MASK                 0x00000FFF                // CRX_IDX[11..0]
#define RRO_RX_D_RX1_CRX_IDX_CRX_IDX_SHFT                 0

/* =====================================================================================

  ---RRO_RX_D_RX1_DRX_IDX (0x15010000 + 0xE9C)---

    DRX_IDX_MIRO[11..0]          - (RO) Point to the next RX DMAD which DMA wants to use, this is a mirrored value of WPDMA RX_DRX_IDX.
    RESERVED12[31..12]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RX1_DRX_IDX_DRX_IDX_MIRO_ADDR            RRO_RX_D_RX1_DRX_IDX_ADDR
#define RRO_RX_D_RX1_DRX_IDX_DRX_IDX_MIRO_MASK            0x00000FFF                // DRX_IDX_MIRO[11..0]
#define RRO_RX_D_RX1_DRX_IDX_DRX_IDX_MIRO_SHFT            0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_CIDX_ADDR_0 (0x15010000 + 0xEA0)---

    ADDR_RRO_R0_L[31..0]         - (RW) WPDMA RRO Rx ring0 configuration cpu index register start address

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR_RRO_R0_L_ADDR  RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR
#define RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR_RRO_R0_L_MASK  0xFFFFFFFF                // ADDR_RRO_R0_L[31..0]
#define RRO_RX_D_RING_CFG_CIDX_ADDR_0_ADDR_RRO_R0_L_SHFT  0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_CIDX_ADDR_1 (0x15010000 + 0xEA4)---

    ADDR_RRO_R1_L[31..0]         - (RW) WPDMA RRO Rx ring1 configuration cpu index register start address

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR_RRO_R1_L_ADDR  RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR
#define RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR_RRO_R1_L_MASK  0xFFFFFFFF                // ADDR_RRO_R1_L[31..0]
#define RRO_RX_D_RING_CFG_CIDX_ADDR_1_ADDR_RRO_R1_L_SHFT  0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_ADDR_2 (0x15010000 + 0xEA8)---

    RESERVED0[7..0]              - (RO) Reserved bits
    POLL_CYCLE[15..8]            - (RW) Polling mode period (cycle x 1us)
    R0_INT_FLAG[16]              - (RO) Service R0 interrupt now
    R1_INT_FLAG[17]              - (RO) Service R1 interrupt now
    BURST_MODE_DISABLE[18]       - (RW) Disable burst re-fill DMAD mode, re-fill one DMAD, move one CIDX
    FORCE_IDLE_WHEN_DRV_ERROR[19] - (RW) Force return idle state when detect error
    INIT_COMPLETE_FLAG[20]       - (RO) Initialization is done
    POLL_FLAG[21]                - (RO) Serive polling
    BUSY_FLAG[22]                - (RO) DRV is busy
    ERR_LAT_OPT[23]              - (RW) Latch awaddr when detect error
    DMAD_DBG_EN[24]              - (RW) Add dmad seq count for debug
    DRV_DBG_CLR[25]              - (A0) DRV DBG SW reset
    DRV_CLR[26]                  - (A0) DRV SW reset, force to reset state
    INIT_FLAG_CLR[27]            - (A0) Clear initialization flag
    FORCE_RETURN_IDLE[28]        - (RW) Force return idle state
    FORCE_IDLE_WHEN_DRV_DISABLE[29] - (RW) Force return idle state when set DRV_EN to 1'b0
    INT_PROC_DISABLE[30]         - (RW) Disable initialization procedure
    DRV_EN[31]                   - (RW) WPDMA RRO RX D DRV Enable

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_ADDR              RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_MASK              0x80000000                // DRV_EN[31]
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_EN_SHFT              31
#define RRO_RX_D_RING_CFG_ADDR_2_INT_PROC_DISABLE_ADDR    RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_INT_PROC_DISABLE_MASK    0x40000000                // INT_PROC_DISABLE[30]
#define RRO_RX_D_RING_CFG_ADDR_2_INT_PROC_DISABLE_SHFT    30
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_DISABLE_ADDR RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_DISABLE_MASK 0x20000000                // FORCE_IDLE_WHEN_DRV_DISABLE[29]
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_DISABLE_SHFT 29
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_RETURN_IDLE_ADDR   RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_RETURN_IDLE_MASK   0x10000000                // FORCE_RETURN_IDLE[28]
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_RETURN_IDLE_SHFT   28
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_FLAG_CLR_ADDR       RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_FLAG_CLR_MASK       0x08000000                // INIT_FLAG_CLR[27]
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_FLAG_CLR_SHFT       27
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_CLR_ADDR             RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_CLR_MASK             0x04000000                // DRV_CLR[26]
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_CLR_SHFT             26
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_DBG_CLR_ADDR         RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_DBG_CLR_MASK         0x02000000                // DRV_DBG_CLR[25]
#define RRO_RX_D_RING_CFG_ADDR_2_DRV_DBG_CLR_SHFT         25
#define RRO_RX_D_RING_CFG_ADDR_2_DMAD_DBG_EN_ADDR         RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_DMAD_DBG_EN_MASK         0x01000000                // DMAD_DBG_EN[24]
#define RRO_RX_D_RING_CFG_ADDR_2_DMAD_DBG_EN_SHFT         24
#define RRO_RX_D_RING_CFG_ADDR_2_ERR_LAT_OPT_ADDR         RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_ERR_LAT_OPT_MASK         0x00800000                // ERR_LAT_OPT[23]
#define RRO_RX_D_RING_CFG_ADDR_2_ERR_LAT_OPT_SHFT         23
#define RRO_RX_D_RING_CFG_ADDR_2_BUSY_FLAG_ADDR           RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_BUSY_FLAG_MASK           0x00400000                // BUSY_FLAG[22]
#define RRO_RX_D_RING_CFG_ADDR_2_BUSY_FLAG_SHFT           22
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_FLAG_ADDR           RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_FLAG_MASK           0x00200000                // POLL_FLAG[21]
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_FLAG_SHFT           21
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_COMPLETE_FLAG_ADDR  RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_COMPLETE_FLAG_MASK  0x00100000                // INIT_COMPLETE_FLAG[20]
#define RRO_RX_D_RING_CFG_ADDR_2_INIT_COMPLETE_FLAG_SHFT  20
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_ERROR_ADDR RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_ERROR_MASK 0x00080000                // FORCE_IDLE_WHEN_DRV_ERROR[19]
#define RRO_RX_D_RING_CFG_ADDR_2_FORCE_IDLE_WHEN_DRV_ERROR_SHFT 19
#define RRO_RX_D_RING_CFG_ADDR_2_BURST_MODE_DISABLE_ADDR  RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_BURST_MODE_DISABLE_MASK  0x00040000                // BURST_MODE_DISABLE[18]
#define RRO_RX_D_RING_CFG_ADDR_2_BURST_MODE_DISABLE_SHFT  18
#define RRO_RX_D_RING_CFG_ADDR_2_R1_INT_FLAG_ADDR         RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_R1_INT_FLAG_MASK         0x00020000                // R1_INT_FLAG[17]
#define RRO_RX_D_RING_CFG_ADDR_2_R1_INT_FLAG_SHFT         17
#define RRO_RX_D_RING_CFG_ADDR_2_R0_INT_FLAG_ADDR         RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_R0_INT_FLAG_MASK         0x00010000                // R0_INT_FLAG[16]
#define RRO_RX_D_RING_CFG_ADDR_2_R0_INT_FLAG_SHFT         16
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_CYCLE_ADDR          RRO_RX_D_RING_CFG_ADDR_2_ADDR
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_CYCLE_MASK          0x0000FF00                // POLL_CYCLE[15..8]
#define RRO_RX_D_RING_CFG_ADDR_2_POLL_CYCLE_SHFT          8

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_CIDX_ADDR (0x15010000 + 0xEAC)---

    ADDR_RRO_R0_M[3..0]          - (RW) WPDMA RRO Rx ring0 configuration cpu index register start address
    ADDR_RRO_R1_M[7..4]          - (RW) WPDMA RRO Rx ring1 configuration cpu index register start address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R1_M_ADDR    RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R1_M_MASK    0x000000F0                // ADDR_RRO_R1_M[7..4]
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R1_M_SHFT    4
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R0_M_ADDR    RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R0_M_MASK    0x0000000F                // ADDR_RRO_R0_M[3..0]
#define RRO_RX_D_RING_CFG_CIDX_ADDR_ADDR_RRO_R0_M_SHFT    0

/* =====================================================================================

  ---RRO_PG_BM_RX_DMAD (0x15010000 + 0xEB0)---

    SDL0[13..0]                  - (RW) Rx DMAD field: sdl0 , for WED_RX_BM to cat with reduced Rx DMAD read from DRAM to form a complete one
    RESERVED14[15..14]           - (RO) Reserved bits
    BASE_PTR_H[23..16]           - (RW) Bit[39:32] of Rx Page Buffer Pool Base Address
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_RX_DMAD_BASE_PTR_H_ADDR                 RRO_PG_BM_RX_DMAD_ADDR
#define RRO_PG_BM_RX_DMAD_BASE_PTR_H_MASK                 0x00FF0000                // BASE_PTR_H[23..16]
#define RRO_PG_BM_RX_DMAD_BASE_PTR_H_SHFT                 16
#define RRO_PG_BM_RX_DMAD_SDL0_ADDR                       RRO_PG_BM_RX_DMAD_ADDR
#define RRO_PG_BM_RX_DMAD_SDL0_MASK                       0x00003FFF                // SDL0[13..0]
#define RRO_PG_BM_RX_DMAD_SDL0_SHFT                       0

/* =====================================================================================

  ---RRO_PG_BM_BASE (0x15010000 + 0xEB4)---

    PTR[31..0]                   - (RW) Rx Page Buffer Pool Base Address [31:0]
                                     Note that the Rx PG_BM Buffer Pool Base Address must be 4KB addressing aligned

 =====================================================================================*/
#define RRO_PG_BM_BASE_PTR_ADDR                           RRO_PG_BM_BASE_ADDR
#define RRO_PG_BM_BASE_PTR_MASK                           0xFFFFFFFF                // PTR[31..0]
#define RRO_PG_BM_BASE_PTR_SHFT                           0

/* =====================================================================================

  ---RRO_PG_BM_INIT_PTR (0x15010000 + 0xEB8)---

    SW_TAIL_IDX[15..0]           - (RW) 1. Free DMAD ptr(numbers) allocated by SW at initial. The Rx DMAD FIFO is 64K, but this value is flexiable depending on SW's requested, the operation need for Rx without loss. The maximum value SW can set/prepare is 64k-1 (0xffff).
    INIT_SW_TAIL_HEAD_IDX[16]    - (A0) Tail_idx and head_idx will initiate to sw_tail_idx and init_head_idx at setting this bit.
                                     SW initiates tail_idx to sw_tail_idx at setting this bit. SW set this bit with sw_tail_idx below after it has allocated sw_tail_idx number of Rx DMAD buffer, before enable WED Tx Buffer Manager operation.
                                     init_head_idx is for DV verficaion purpose, it is recommended that SW does not need to do init_head_idx settings.
                                     This bit is auto cleared.
    RESERVED17[31..17]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_ADDR     RRO_PG_BM_INIT_PTR_ADDR
#define RRO_PG_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_MASK     0x00010000                // INIT_SW_TAIL_HEAD_IDX[16]
#define RRO_PG_BM_INIT_PTR_INIT_SW_TAIL_HEAD_IDX_SHFT     16
#define RRO_PG_BM_INIT_PTR_SW_TAIL_IDX_ADDR               RRO_PG_BM_INIT_PTR_ADDR
#define RRO_PG_BM_INIT_PTR_SW_TAIL_IDX_MASK               0x0000FFFF                // SW_TAIL_IDX[15..0]
#define RRO_PG_BM_INIT_PTR_SW_TAIL_IDX_SHFT               0

/* =====================================================================================

  ---RRO_PG_BM_PTR (0x15010000 + 0xEBC)---

    TAIL_IDX[15..0]              - (RO) 1. tail_idx is updated to sw_tail_idx when init_sw_tail_idx of WED_RX_BM_INIT_PTR is set. This is the Free DMAD number SW allocates. WED_RX_BM will start Rx DMAD delivery and return, after wed_rx_bm is enabled, and this field != head_idx (0).
                                     2. it is advanced by HW each time when a DMAD is free/returned from RX_ROUTE_QM or WED_RX_DMA (original rx DMAD is exchanged with that of CPU ring, for CPU to handle rx pkt to it.) The tail_idx update should not run over head_idx
    HEAD_IDX[31..16]             - (RO) head_idx is incremented by HW each time a Rx DMAD is fetched by WED_RX_BM, which points to the index WED_RX_BM will fetch next.
                                     Note that head_idx will never exceed tail_idx. When they are equal, it means all rx DMAD allocated by SW are all passed to HW to Rx pkt and no one is free yet.

 =====================================================================================*/
#define RRO_PG_BM_PTR_HEAD_IDX_ADDR                       RRO_PG_BM_PTR_ADDR
#define RRO_PG_BM_PTR_HEAD_IDX_MASK                       0xFFFF0000                // HEAD_IDX[31..16]
#define RRO_PG_BM_PTR_HEAD_IDX_SHFT                       16
#define RRO_PG_BM_PTR_TAIL_IDX_ADDR                       RRO_PG_BM_PTR_ADDR
#define RRO_PG_BM_PTR_TAIL_IDX_MASK                       0x0000FFFF                // TAIL_IDX[15..0]
#define RRO_PG_BM_PTR_TAIL_IDX_SHFT                       0

/* =====================================================================================

  ---RRO_PG_BM_ADD_BASE_H (0x15010000 + 0xEC0)---

    PTR[7..0]                    - (RW) High Bits of SW ADD_IDX Buffer Base Address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_ADD_BASE_H_PTR_ADDR                     RRO_PG_BM_ADD_BASE_H_ADDR
#define RRO_PG_BM_ADD_BASE_H_PTR_MASK                     0x000000FF                // PTR[7..0]
#define RRO_PG_BM_ADD_BASE_H_PTR_SHFT                     0

/* =====================================================================================

  ---RRO_PG_BM_STS (0x15010000 + 0xEC4)---

    RESERVED0[7..0]              - (RO) Reserved bits
    DMAD_NUM[12..8]              - (RO) Number of valid free page DMADs in common dmad FIFO, ready for RRO_RX_D_DRV module. Those free page DMADs are picked out from buffer pool by Buffer manager.
    RESERVED13[15..13]           - (RO) Reserved bits
    DMAD0_NUM[17..16]            - (RO) Number of valid free page DMADs in arbitrated dmad small FIFO, ready for RRO_RX_D_DRV module for page.
    RESERVED18[19..18]           - (RO) Reserved bits
    DMAD0_VLD[20]                - (RO) Free page DMAD is valid for RRO_RX_D_DRV module for page
    DMAD0_RD[21]                 - (RO) RRO_RX_D_DRV module for page extracted a free page DMAD
    RESERVED22[31..22]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_STS_DMAD0_RD_ADDR                       RRO_PG_BM_STS_ADDR
#define RRO_PG_BM_STS_DMAD0_RD_MASK                       0x00200000                // DMAD0_RD[21]
#define RRO_PG_BM_STS_DMAD0_RD_SHFT                       21
#define RRO_PG_BM_STS_DMAD0_VLD_ADDR                      RRO_PG_BM_STS_ADDR
#define RRO_PG_BM_STS_DMAD0_VLD_MASK                      0x00100000                // DMAD0_VLD[20]
#define RRO_PG_BM_STS_DMAD0_VLD_SHFT                      20
#define RRO_PG_BM_STS_DMAD0_NUM_ADDR                      RRO_PG_BM_STS_ADDR
#define RRO_PG_BM_STS_DMAD0_NUM_MASK                      0x00030000                // DMAD0_NUM[17..16]
#define RRO_PG_BM_STS_DMAD0_NUM_SHFT                      16
#define RRO_PG_BM_STS_DMAD_NUM_ADDR                       RRO_PG_BM_STS_ADDR
#define RRO_PG_BM_STS_DMAD_NUM_MASK                       0x00001F00                // DMAD_NUM[12..8]
#define RRO_PG_BM_STS_DMAD_NUM_SHFT                       8

/* =====================================================================================

  ---RRO_PG_BM_RANGE_CFG (0x15010000 + 0xEC8)---

    SW_CFG_BUF_IDX[15..0]        - (RW) This value is the entries of the DMAD FIFO ring set by SW at initial.  The maximum value of tail_idx and head_idx that can be achieved is this value. The maximum value SW can set is 64K-1(0xFFFF).
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_RANGE_CFG_SW_CFG_BUF_IDX_ADDR           RRO_PG_BM_RANGE_CFG_ADDR
#define RRO_PG_BM_RANGE_CFG_SW_CFG_BUF_IDX_MASK           0x0000FFFF                // SW_CFG_BUF_IDX[15..0]
#define RRO_PG_BM_RANGE_CFG_SW_CFG_BUF_IDX_SHFT           0

/* =====================================================================================

  ---RRO_PG_BM_INTF (0x15010000 + 0xECC)---

    DMAD_FREE[15..0]             - (RO) Free Token_ID for Buffer Manager
    RESERVED16[27..16]           - (RO) Reserved bits
    DMAD_FREE_VLD[28]            - (RO) There is at least one valid Free token_ID for Buffer Manager, these free Token_IDs are returned by  Rx Route Queue Manager module, after the corresponding WiFi Rx pkt has been sent to Ethernet.
    DMAD_FREE_RD[29]             - (RO) Buffer Manager extracted a free Token_ID from Rx Route Queue Manager module
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_INTF_DMAD_FREE_RD_ADDR                  RRO_PG_BM_INTF_ADDR
#define RRO_PG_BM_INTF_DMAD_FREE_RD_MASK                  0x20000000                // DMAD_FREE_RD[29]
#define RRO_PG_BM_INTF_DMAD_FREE_RD_SHFT                  29
#define RRO_PG_BM_INTF_DMAD_FREE_VLD_ADDR                 RRO_PG_BM_INTF_ADDR
#define RRO_PG_BM_INTF_DMAD_FREE_VLD_MASK                 0x10000000                // DMAD_FREE_VLD[28]
#define RRO_PG_BM_INTF_DMAD_FREE_VLD_SHFT                 28
#define RRO_PG_BM_INTF_DMAD_FREE_ADDR                     RRO_PG_BM_INTF_ADDR
#define RRO_PG_BM_INTF_DMAD_FREE_MASK                     0x0000FFFF                // DMAD_FREE[15..0]
#define RRO_PG_BM_INTF_DMAD_FREE_SHFT                     0

/* =====================================================================================

  ---RRO_PG_BM_ERR_STS (0x15010000 + 0xED0)---

    DMAD_AXI_RD_LESS[0]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read transfer less than expected. For example, issue 8 transfers but rlast is  asserted earlier  than 8th transfer.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_AXI_RD_MORE[1]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read transfer more than expected. For example, issue 8 transfers but rlast is  asserted later  than 8th transfer.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_FULL_AXI_RD[2]          - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read while internal DMAD FIFO is already full.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    DMAD_AXI_RD_OVER_TAIL[3]     - (RO) WED Rx Buffer Manager Error inidcation: DMAD AXI read while head_idx is reaching tail_idx. DMAD should be all read in already.
                                     Bit 3:0 will trigger 0x20 WED_EX_INT_STA [13] rx_bm_dmad_rd_err
    RESERVED4[6..4]              - (RO) Reserved bits
    FREE_AT_EMPTY[7]             - (RO) DMAD free while head_idx==tail_idx, all rx DMAD should already be free.
                                     Will trigger 0x20 WED_EX_INT_STA [12] rx_bm_free_at_empty
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_ERR_STS_FREE_AT_EMPTY_ADDR              RRO_PG_BM_ERR_STS_ADDR
#define RRO_PG_BM_ERR_STS_FREE_AT_EMPTY_MASK              0x00000080                // FREE_AT_EMPTY[7]
#define RRO_PG_BM_ERR_STS_FREE_AT_EMPTY_SHFT              7
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_ADDR      RRO_PG_BM_ERR_STS_ADDR
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_MASK      0x00000008                // DMAD_AXI_RD_OVER_TAIL[3]
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_OVER_TAIL_SHFT      3
#define RRO_PG_BM_ERR_STS_DMAD_FULL_AXI_RD_ADDR           RRO_PG_BM_ERR_STS_ADDR
#define RRO_PG_BM_ERR_STS_DMAD_FULL_AXI_RD_MASK           0x00000004                // DMAD_FULL_AXI_RD[2]
#define RRO_PG_BM_ERR_STS_DMAD_FULL_AXI_RD_SHFT           2
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_MORE_ADDR           RRO_PG_BM_ERR_STS_ADDR
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_MORE_MASK           0x00000002                // DMAD_AXI_RD_MORE[1]
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_MORE_SHFT           1
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_LESS_ADDR           RRO_PG_BM_ERR_STS_ADDR
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_LESS_MASK           0x00000001                // DMAD_AXI_RD_LESS[0]
#define RRO_PG_BM_ERR_STS_DMAD_AXI_RD_LESS_SHFT           0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_DIDX_ADDR (0x15010000 + 0xED4)---

    ADDR_RRO_R0_M[3..0]          - (RW) WPDMA RRO Rx ring0 configuration dma index register start address
    ADDR_RRO_R1_M[7..4]          - (RW) WPDMA RRO Rx ring1 configuration dma index register start address
    RESERVED8[31..8]             - (RO) Reserved bits

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R1_M_ADDR    RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R1_M_MASK    0x000000F0                // ADDR_RRO_R1_M[7..4]
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R1_M_SHFT    4
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R0_M_ADDR    RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R0_M_MASK    0x0000000F                // ADDR_RRO_R0_M[3..0]
#define RRO_RX_D_RING_CFG_DIDX_ADDR_ADDR_RRO_R0_M_SHFT    0

/* =====================================================================================

  ---RRO_PG_BM_DYN_ALLOC_TH (0x15010000 + 0xED8)---

    L_BUF_TH[15..0]              - (RW) WED RX Buffer Manager SW Dynamic Allocation Low Threshold for buffer adding    [Sw_alloc - (tail_idx - head_idx) < l_buf_th]
    H_BUF_TH[31..16]             - (RW) WED RX Buffer Manager SW Dynamic Allocation High Threshold for buffer retrieving    [Sw_alloc - (tail_idx - head_idx) > h_buf_th]

 =====================================================================================*/
#define RRO_PG_BM_DYN_ALLOC_TH_H_BUF_TH_ADDR              RRO_PG_BM_DYN_ALLOC_TH_ADDR
#define RRO_PG_BM_DYN_ALLOC_TH_H_BUF_TH_MASK              0xFFFF0000                // H_BUF_TH[31..16]
#define RRO_PG_BM_DYN_ALLOC_TH_H_BUF_TH_SHFT              16
#define RRO_PG_BM_DYN_ALLOC_TH_L_BUF_TH_ADDR              RRO_PG_BM_DYN_ALLOC_TH_ADDR
#define RRO_PG_BM_DYN_ALLOC_TH_L_BUF_TH_MASK              0x0000FFFF                // L_BUF_TH[15..0]
#define RRO_PG_BM_DYN_ALLOC_TH_L_BUF_TH_SHFT              0

/* =====================================================================================

  ---RRO_PG_BM_DYN_ALLOC_CFG (0x15010000 + 0xEDC)---

    SW_ADDSUB_IDX_CNT[15..0]     - (RW) WED RX Buffer Manager SW Add/Subtract DMAD buffer amount
    SW_SUB_BUF_REQ[16]           - (A0) WED RX Buffer Manager SW Subtract DMAD buffer Request
                                     Req is cleared when HW SUB process done (HW_ADD_SUB_ACK=1)
    SW_ADD_BUF_REQ[17]           - (A0) WED RX Buffer Manager SW Add DMAD buffer Request (Add>Sub when both set)
                                     Req is cleared when HW ADD process done (HW_ADD_SUB_ACK=1)
    SW_SUB_RDY[18]               - (A0) For Sub process, HW set HW_SUB_PAUSE (b31) to notify SW to retrieve DMSAD buffer, SW set this bit when it finishes DMAD content copy for retrieve.
    RESERVED19[27..19]           - (RO) Reserved bits
    SW_SUB_UNDERFLOW_ERR[28]     - (W1C) SW add RX DMAD count underflow (<=0)
                                     The subtrating process will not be executed, and this error bit is W1C
    SW_ADD_OVERFLOW_ERR[29]      - (W1C) (1) sw_tail_idx is larger than wed_rx_bm_sw_cfg_buf_idx
                                     Buffer manager will not execute fetch and free DMAD processes.
                                     (2) SW add RXDMAD count overflow (>wed_rx_bm_sw_cfg_buf_idx)
                                     The adding process will not be executed.
                                     This error bit is W1C.
    HW_ADDSUB_ACK[30]            - (RO) HW signal ADD/SUB done, return to normal DMAD ring fifo operation.
    HW_SUB_pause[31]             - (RO) At SW subtrate buffer process, when HW tail_idx-head_idx >= SW_ADDSUB_IDX_CNT, HW set this bit to inform SW, Buffer is ready for retrieve.
                                     HW_SUB_pause is cleared after SW set SW_SUB_RDY (b18), indicating SW has copied out the DMAD for retrieving, HW can over-write the DMAD index content

 =====================================================================================*/
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_SUB_pause_ADDR         RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_SUB_pause_MASK         0x80000000                // HW_SUB_pause[31]
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_SUB_pause_SHFT         31
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_ADDR        RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_MASK        0x40000000                // HW_ADDSUB_ACK[30]
#define RRO_PG_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK_SHFT        30
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_ADDR  RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_MASK  0x20000000                // SW_ADD_OVERFLOW_ERR[29]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_OVERFLOW_ERR_SHFT  29
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_ADDR RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_MASK 0x10000000                // SW_SUB_UNDERFLOW_ERR[28]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_UNDERFLOW_ERR_SHFT 28
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_RDY_ADDR           RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_RDY_MASK           0x00040000                // SW_SUB_RDY[18]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_RDY_SHFT           18
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_ADDR       RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_MASK       0x00020000                // SW_ADD_BUF_REQ[17]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ_SHFT       17
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_ADDR       RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_MASK       0x00010000                // SW_SUB_BUF_REQ[16]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ_SHFT       16
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_ADDR    RRO_PG_BM_DYN_ALLOC_CFG_ADDR
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_MASK    0x0000FFFF                // SW_ADDSUB_IDX_CNT[15..0]
#define RRO_PG_BM_DYN_ALLOC_CFG_SW_ADDSUB_IDX_CNT_SHFT    0

/* =====================================================================================

  ---RRO_PG_BM_ADD_BASE (0x15010000 + 0xEE0)---

    PTR[31..0]                   - (RW) SW ADD_IDX Buffer Base Address

 =====================================================================================*/
#define RRO_PG_BM_ADD_BASE_PTR_ADDR                       RRO_PG_BM_ADD_BASE_ADDR
#define RRO_PG_BM_ADD_BASE_PTR_MASK                       0xFFFFFFFF                // PTR[31..0]
#define RRO_PG_BM_ADD_BASE_PTR_SHFT                       0

/* =====================================================================================

  ---RRO_PG_BM_ADD_PTR (0x15010000 + 0xEE4)---

    SW_ADD_IDX_REMAIN[15..0]     - (RO) SW_ADD_IDX_CNT Remaining = SW_ADD_IDX_CNT - head2_idx
    HEAD2_IDX[31..16]            - (RO) head2_idx is incremented by HW each time a Rx DMAD is fetched from WED_RX_BM_ADD_BASE by WED_RX_BM, which points to the index WED_RX_BM will fetch next, at SW add Rx buffer stage.
                                     max. head2_idx =  SW_ADD_IDX_CNT

 =====================================================================================*/
#define RRO_PG_BM_ADD_PTR_HEAD2_IDX_ADDR                  RRO_PG_BM_ADD_PTR_ADDR
#define RRO_PG_BM_ADD_PTR_HEAD2_IDX_MASK                  0xFFFF0000                // HEAD2_IDX[31..16]
#define RRO_PG_BM_ADD_PTR_HEAD2_IDX_SHFT                  16
#define RRO_PG_BM_ADD_PTR_SW_ADD_IDX_REMAIN_ADDR          RRO_PG_BM_ADD_PTR_ADDR
#define RRO_PG_BM_ADD_PTR_SW_ADD_IDX_REMAIN_MASK          0x0000FFFF                // SW_ADD_IDX_REMAIN[15..0]
#define RRO_PG_BM_ADD_PTR_SW_ADD_IDX_REMAIN_SHFT          0

/* =====================================================================================

  ---RRO_PG_BM_TOTAL_DMAD_IDX (0x15010000 + 0xEE8)---

    SW_TOTAL_RX_BM_CNT[15..0]    - (RO) The total amount of Rx DMAD count after SW init/add/subtrate, max 64K-1
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_ADDR  RRO_PG_BM_TOTAL_DMAD_IDX_ADDR
#define RRO_PG_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_MASK  0x0000FFFF                // SW_TOTAL_RX_BM_CNT[15..0]
#define RRO_PG_BM_TOTAL_DMAD_IDX_SW_TOTAL_RX_BM_CNT_SHFT  0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL_RRO_RX (0x15010000 + 0xEEC)---

    RRO_RX_DONE_EN0[0]           - (RW) RRO RX Queue#0 packet transmit interrupt enable
    RRO_RX_DONE_CLR0[1]          - (RW) Clear RRO RX Queue#0 packet transmit interrupt automatically after it has been received by agent.
    RRO_RX_DONE_TRIG0[6..2]      - (RW) RRO RX Queue#0 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED7[7]                 - (RO) Reserved bits
    RRO_RX_DONE_EN1[8]           - (RW) RRO RX Queue#1 packet transmit interrupt enable
    RRO_RX_DONE_CLR1[9]          - (RW) Clear RRO RX Queue#1 packet transmit interrupt automatically after it has been received by agent.
    RRO_RX_DONE_TRIG1[14..10]    - (RW) RRO RX Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED15[31..15]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG1_ADDR  WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG1_MASK  0x00007C00                // RRO_RX_DONE_TRIG1[14..10]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG1_SHFT  10
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR1_ADDR   WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR1_MASK   0x00000200                // RRO_RX_DONE_CLR1[9]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR1_SHFT   9
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN1_ADDR    WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN1_MASK    0x00000100                // RRO_RX_DONE_EN1[8]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN1_SHFT    8
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG0_ADDR  WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG0_MASK  0x0000007C                // RRO_RX_DONE_TRIG0[6..2]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_TRIG0_SHFT  2
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR0_ADDR   WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR0_MASK   0x00000002                // RRO_RX_DONE_CLR0[1]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_CLR0_SHFT   1
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN0_ADDR    WED_WPDMA_INT_CTRL_RRO_RX_ADDR
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN0_MASK    0x00000001                // RRO_RX_DONE_EN0[0]
#define WED_WPDMA_INT_CTRL_RRO_RX_RRO_RX_DONE_EN0_SHFT    0

/* =====================================================================================

  ---RRO_PG_BM_INIT_HEAD_PTR (0x15010000 + 0xEF0)---

    INIT_HEAD_IDX[15..0]         - (RW) Start buffer index, for verification purpose (wraparound test)
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define RRO_PG_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_ADDR        RRO_PG_BM_INIT_HEAD_PTR_ADDR
#define RRO_PG_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_MASK        0x0000FFFF                // INIT_HEAD_IDX[15..0]
#define RRO_PG_BM_INIT_HEAD_PTR_INIT_HEAD_IDX_SHFT        0

/* =====================================================================================

  ---WED_WPDMA_INT_CTRL_RRO_MSDU_PG (0x15010000 + 0xEF4)---

    RRO_PG_DONE_EN0[0]           - (RW) RRO MSDU PG Queue#0 packet transmit interrupt enable
    RRO_PG_DONE_CLR0[1]          - (RW) Clear RRO MSDU PG Queue#0 packet transmit interrupt automatically after it has been received by agent.
    RRO_PG_DONE_TRIG0[6..2]      - (RW) RRO MSDU PG Queue#0 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED7[7]                 - (RO) Reserved bits
    RRO_PG_DONE_EN1[8]           - (RW) RRO MSDU PG Queue#1 packet transmit interrupt enable
    RRO_PG_DONE_CLR1[9]          - (RW) Clear RRO MSDU PG Queue#1 packet transmit interrupt automatically after it has been received by agent.
    RRO_PG_DONE_TRIG1[14..10]    - (RW) RRO MSDU PG Queue#1 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED15[15]               - (RO) Reserved bits
    RRO_PG_DONE_EN2[16]          - (RW) RRO MSDU PG Queue#2 packet transmit interrupt enable
    RRO_PG_DONE_CLR2[17]         - (RW) Clear RRO MSDU PG Queue#2 packet transmit interrupt automatically after it has been received by agent.
    RRO_PG_DONE_TRIG2[22..18]    - (RW) RRO MSDU PG Queue#2 packet transmit interrupt bit selection
                                     Specify the WPDMA interrupt status bits which will trigger WED hardware to handle the interrupt event from WiFi card's WPDMA.
    RESERVED23[31..23]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG2_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG2_MASK 0x007C0000                // RRO_PG_DONE_TRIG2[22..18]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG2_SHFT 18
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR2_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR2_MASK 0x00020000                // RRO_PG_DONE_CLR2[17]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR2_SHFT 17
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN2_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN2_MASK 0x00010000                // RRO_PG_DONE_EN2[16]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN2_SHFT 16
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG1_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG1_MASK 0x00007C00                // RRO_PG_DONE_TRIG1[14..10]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG1_SHFT 10
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR1_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR1_MASK 0x00000200                // RRO_PG_DONE_CLR1[9]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR1_SHFT 9
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN1_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN1_MASK 0x00000100                // RRO_PG_DONE_EN1[8]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN1_SHFT 8
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG0_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG0_MASK 0x0000007C                // RRO_PG_DONE_TRIG0[6..2]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_TRIG0_SHFT 2
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR0_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR0_MASK 0x00000002                // RRO_PG_DONE_CLR0[1]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_CLR0_SHFT 1
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN0_ADDR WED_WPDMA_INT_CTRL_RRO_MSDU_PG_ADDR
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN0_MASK 0x00000001                // RRO_PG_DONE_EN0[0]
#define WED_WPDMA_INT_CTRL_RRO_MSDU_PG_RRO_PG_DONE_EN0_SHFT 0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_DIDX_ADDR_0 (0x15010000 + 0xEF8)---

    ADDR_RRO_R0_L[31..0]         - (RW) WPDMA RRO Rx ring0 configuration dma index register start address

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR_RRO_R0_L_ADDR  RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR
#define RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR_RRO_R0_L_MASK  0xFFFFFFFF                // ADDR_RRO_R0_L[31..0]
#define RRO_RX_D_RING_CFG_DIDX_ADDR_0_ADDR_RRO_R0_L_SHFT  0

/* =====================================================================================

  ---RRO_RX_D_RING_CFG_DIDX_ADDR_1 (0x15010000 + 0xEFC)---

    ADDR_RRO_R1_L[31..0]         - (RW) WPDMA RRO Rx ring1 configuration dma index register start address

 =====================================================================================*/
#define RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR_RRO_R1_L_ADDR  RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR
#define RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR_RRO_R1_L_MASK  0xFFFFFFFF                // ADDR_RRO_R1_L[31..0]
#define RRO_RX_D_RING_CFG_DIDX_ADDR_1_ADDR_RRO_R1_L_SHFT  0

/* =====================================================================================

  ---WED_RRO_RX_HW_STS (0x15010000 + 0xF00)---

    RX_IND_CMD_BUSY[0]           - (RO) WED_RX_IND_CMD is busy
    RX_ADDR_ELEM_BUSY[1]         - (RO) WED_RX_ADDR_ELEM is busy
    RX_MSDU_INFO_PG_BUSY[2]      - (RO) WED_RX_MSDU_INFO_PG is busy
    RX_PN_CHK_BUSY[3]            - (RO) WED_RX_PN_CHK is busy
    RESERVED4[7..4]              - (RO) Reserved bits
    FETCH_IND_CMD_CS[9..8]       - (RO) WED_RX_IND_CMD fetch_ind_cmd FSM
    IND2ADDR_ELEM_CS[12..10]     - (RO) WED_RX_ADDR_ELEM ind2addr_elem FSM
    CLR_ADDR_ACKSN_CS[15..13]    - (RO) WED_RX_ADDR_ELEM clr_addr_acksn FSM
    MSDU_INFO_PG_CS[17..16]      - (RO) WED_RX_MSDU_INFO_PG msdu_info_pg FSM
    PN_CHK_CS[20..18]            - (RO) WED_RX_PN_CHK pn_chk FSM
    RESERVED21[31..21]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RRO_RX_HW_STS_PN_CHK_CS_ADDR                  WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_PN_CHK_CS_MASK                  0x001C0000                // PN_CHK_CS[20..18]
#define WED_RRO_RX_HW_STS_PN_CHK_CS_SHFT                  18
#define WED_RRO_RX_HW_STS_MSDU_INFO_PG_CS_ADDR            WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_MSDU_INFO_PG_CS_MASK            0x00030000                // MSDU_INFO_PG_CS[17..16]
#define WED_RRO_RX_HW_STS_MSDU_INFO_PG_CS_SHFT            16
#define WED_RRO_RX_HW_STS_CLR_ADDR_ACKSN_CS_ADDR          WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_CLR_ADDR_ACKSN_CS_MASK          0x0000E000                // CLR_ADDR_ACKSN_CS[15..13]
#define WED_RRO_RX_HW_STS_CLR_ADDR_ACKSN_CS_SHFT          13
#define WED_RRO_RX_HW_STS_IND2ADDR_ELEM_CS_ADDR           WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_IND2ADDR_ELEM_CS_MASK           0x00001C00                // IND2ADDR_ELEM_CS[12..10]
#define WED_RRO_RX_HW_STS_IND2ADDR_ELEM_CS_SHFT           10
#define WED_RRO_RX_HW_STS_FETCH_IND_CMD_CS_ADDR           WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_FETCH_IND_CMD_CS_MASK           0x00000300                // FETCH_IND_CMD_CS[9..8]
#define WED_RRO_RX_HW_STS_FETCH_IND_CMD_CS_SHFT           8
#define WED_RRO_RX_HW_STS_RX_PN_CHK_BUSY_ADDR             WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_RX_PN_CHK_BUSY_MASK             0x00000008                // RX_PN_CHK_BUSY[3]
#define WED_RRO_RX_HW_STS_RX_PN_CHK_BUSY_SHFT             3
#define WED_RRO_RX_HW_STS_RX_MSDU_INFO_PG_BUSY_ADDR       WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_RX_MSDU_INFO_PG_BUSY_MASK       0x00000004                // RX_MSDU_INFO_PG_BUSY[2]
#define WED_RRO_RX_HW_STS_RX_MSDU_INFO_PG_BUSY_SHFT       2
#define WED_RRO_RX_HW_STS_RX_ADDR_ELEM_BUSY_ADDR          WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_RX_ADDR_ELEM_BUSY_MASK          0x00000002                // RX_ADDR_ELEM_BUSY[1]
#define WED_RRO_RX_HW_STS_RX_ADDR_ELEM_BUSY_SHFT          1
#define WED_RRO_RX_HW_STS_RX_IND_CMD_BUSY_ADDR            WED_RRO_RX_HW_STS_ADDR
#define WED_RRO_RX_HW_STS_RX_IND_CMD_BUSY_MASK            0x00000001                // RX_IND_CMD_BUSY[0]
#define WED_RRO_RX_HW_STS_RX_IND_CMD_BUSY_SHFT            0

/* =====================================================================================

  ---WED_RRO_RX_DBG0 (0x15010000 + 0xF04)---

    IND_CMD_WA_B64[3..0]         - (RO) WED_RX_IND_CMD rx_ind_cmd_mem wa (64b data)
    IND_CMD_RA[7..4]             - (RO) WED_RX_IND_CMD rx_ind_cmd_mem ra
    ADDR_ELEM_WA_B64[11..8]      - (RO) WED_RX_ADDR_ELEM rx_addr_elem_mem wa (64b data)
    ADDR_ELEM_RA[15..12]         - (RO) WED_RX_ADDR_ELEM rx_addr_elem_mem ra
    RX_PG_MEM_WA[19..16]         - (RO) WED_RX_MSDU_INFO_PG rx_page_mem wa
    RX_PG_MEM_RA[23..20]         - (RO) WED_RX_MSDU_INFO_PG rx_page_mem ra
    rx_ind_cmd[29..24]           - (RO) WED_RX_IND_CMD to WED_RX_ADDR_ELEM rx_ind_cmd
    rx_ind_cmd_rd[30]            - (RO) WED_RX_IND_CMD to WED_RX_ADDR_ELEM rx_ind_cmd_rd
    rx_ind_cmd_vld[31]           - (RO) WED_RX_IND_CMD to WED_RX_ADDR_ELEM rx_ind_cmd_vld

 =====================================================================================*/
#define WED_RRO_RX_DBG0_rx_ind_cmd_vld_ADDR               WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_rx_ind_cmd_vld_MASK               0x80000000                // rx_ind_cmd_vld[31]
#define WED_RRO_RX_DBG0_rx_ind_cmd_vld_SHFT               31
#define WED_RRO_RX_DBG0_rx_ind_cmd_rd_ADDR                WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_rx_ind_cmd_rd_MASK                0x40000000                // rx_ind_cmd_rd[30]
#define WED_RRO_RX_DBG0_rx_ind_cmd_rd_SHFT                30
#define WED_RRO_RX_DBG0_rx_ind_cmd_ADDR                   WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_rx_ind_cmd_MASK                   0x3F000000                // rx_ind_cmd[29..24]
#define WED_RRO_RX_DBG0_rx_ind_cmd_SHFT                   24
#define WED_RRO_RX_DBG0_RX_PG_MEM_RA_ADDR                 WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_RX_PG_MEM_RA_MASK                 0x00F00000                // RX_PG_MEM_RA[23..20]
#define WED_RRO_RX_DBG0_RX_PG_MEM_RA_SHFT                 20
#define WED_RRO_RX_DBG0_RX_PG_MEM_WA_ADDR                 WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_RX_PG_MEM_WA_MASK                 0x000F0000                // RX_PG_MEM_WA[19..16]
#define WED_RRO_RX_DBG0_RX_PG_MEM_WA_SHFT                 16
#define WED_RRO_RX_DBG0_ADDR_ELEM_RA_ADDR                 WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_ADDR_ELEM_RA_MASK                 0x0000F000                // ADDR_ELEM_RA[15..12]
#define WED_RRO_RX_DBG0_ADDR_ELEM_RA_SHFT                 12
#define WED_RRO_RX_DBG0_ADDR_ELEM_WA_B64_ADDR             WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_ADDR_ELEM_WA_B64_MASK             0x00000F00                // ADDR_ELEM_WA_B64[11..8]
#define WED_RRO_RX_DBG0_ADDR_ELEM_WA_B64_SHFT             8
#define WED_RRO_RX_DBG0_IND_CMD_RA_ADDR                   WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_IND_CMD_RA_MASK                   0x000000F0                // IND_CMD_RA[7..4]
#define WED_RRO_RX_DBG0_IND_CMD_RA_SHFT                   4
#define WED_RRO_RX_DBG0_IND_CMD_WA_B64_ADDR               WED_RRO_RX_DBG0_ADDR
#define WED_RRO_RX_DBG0_IND_CMD_WA_B64_MASK               0x0000000F                // IND_CMD_WA_B64[3..0]
#define WED_RRO_RX_DBG0_IND_CMD_WA_B64_SHFT               0

/* =====================================================================================

  ---WED_RRO_RX_DBG1 (0x15010000 + 0xF08)---

    parti_addr_elem[13..0]       - (RO) WED_RX_ADDR_ELEM to WED_RX_MSDU_INFO_PAGE parti_addr_elem[13:0]
    parti_addr_elem_rd[14]       - (RO) WED_RX_ADDR_ELEM to WED_RX_MSDU_INFO_PAGE parti_addr_elem_rd
    parti_addr_elem_vld[15]      - (RO) WED_RX_ADDR_ELEM to WED_RX_MSDU_INFO_PAGE parti_addr_elem_vld
    acksn_last_cnt[27..16]       - (RO) ack sn last number, should be the equal to ind_cmd_proc_idx, which is ind_cmd total process number
    addr_elem_wr_align_err[28]   - (RO) addr_elem AXI rd -> wr to addr_elem mem alignment error
    addr_elem_aw_err[29]         - (RO) addr_elem rd proc error -
                                     when signature_fail on last addr_elem read, the fetch_addr_elem_cnt_rem can't be equal to axi_avail_inc[1:0] because axi_avail_inc must be fewer than axi_rd_cnt
    RESERVED30[31..30]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RRO_RX_DBG1_addr_elem_aw_err_ADDR             WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_addr_elem_aw_err_MASK             0x20000000                // addr_elem_aw_err[29]
#define WED_RRO_RX_DBG1_addr_elem_aw_err_SHFT             29
#define WED_RRO_RX_DBG1_addr_elem_wr_align_err_ADDR       WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_addr_elem_wr_align_err_MASK       0x10000000                // addr_elem_wr_align_err[28]
#define WED_RRO_RX_DBG1_addr_elem_wr_align_err_SHFT       28
#define WED_RRO_RX_DBG1_acksn_last_cnt_ADDR               WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_acksn_last_cnt_MASK               0x0FFF0000                // acksn_last_cnt[27..16]
#define WED_RRO_RX_DBG1_acksn_last_cnt_SHFT               16
#define WED_RRO_RX_DBG1_parti_addr_elem_vld_ADDR          WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_parti_addr_elem_vld_MASK          0x00008000                // parti_addr_elem_vld[15]
#define WED_RRO_RX_DBG1_parti_addr_elem_vld_SHFT          15
#define WED_RRO_RX_DBG1_parti_addr_elem_rd_ADDR           WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_parti_addr_elem_rd_MASK           0x00004000                // parti_addr_elem_rd[14]
#define WED_RRO_RX_DBG1_parti_addr_elem_rd_SHFT           14
#define WED_RRO_RX_DBG1_parti_addr_elem_ADDR              WED_RRO_RX_DBG1_ADDR
#define WED_RRO_RX_DBG1_parti_addr_elem_MASK              0x00003FFF                // parti_addr_elem[13..0]
#define WED_RRO_RX_DBG1_parti_addr_elem_SHFT              0

/* =====================================================================================

  ---WED_RRO_RX_DBG2 (0x15010000 + 0xF0C)---

    hif_rxd_ext[13..0]           - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PN_CHK hif_rxd_ext[13:0]
    hif_rxd_ext_rd[14]           - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PN_CHK hif_rxd_ext_rd
    hif_rxd_ext_vld[15]          - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PN_CHK hif_rxd_ext_vld
    rx_pg_dmad_free[29..16]      - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PG_BM rx_pg_dmad_free[13:0]
    rx_pg_dmad_free_rd[30]       - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PG_BM rx_pg_dmad_free_rd
    rx_pg_dmad_free_vld[31]      - (RO) WED_RX_MSDU_INFO_PAGE to WED_RX_PG_BM rx_pg_dmad_free_vld

 =====================================================================================*/
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_vld_ADDR          WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_vld_MASK          0x80000000                // rx_pg_dmad_free_vld[31]
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_vld_SHFT          31
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_rd_ADDR           WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_rd_MASK           0x40000000                // rx_pg_dmad_free_rd[30]
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_rd_SHFT           30
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_ADDR              WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_MASK              0x3FFF0000                // rx_pg_dmad_free[29..16]
#define WED_RRO_RX_DBG2_rx_pg_dmad_free_SHFT              16
#define WED_RRO_RX_DBG2_hif_rxd_ext_vld_ADDR              WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_hif_rxd_ext_vld_MASK              0x00008000                // hif_rxd_ext_vld[15]
#define WED_RRO_RX_DBG2_hif_rxd_ext_vld_SHFT              15
#define WED_RRO_RX_DBG2_hif_rxd_ext_rd_ADDR               WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_hif_rxd_ext_rd_MASK               0x00004000                // hif_rxd_ext_rd[14]
#define WED_RRO_RX_DBG2_hif_rxd_ext_rd_SHFT               14
#define WED_RRO_RX_DBG2_hif_rxd_ext_ADDR                  WED_RRO_RX_DBG2_ADDR
#define WED_RRO_RX_DBG2_hif_rxd_ext_MASK                  0x00003FFF                // hif_rxd_ext[13..0]
#define WED_RRO_RX_DBG2_hif_rxd_ext_SHFT                  0

/* =====================================================================================

  ---WED_RRO_RX_DBG3 (0x15010000 + 0xF10)---

    RESERVED0[15..0]             - (RO) Reserved bits
    rx_msdu2rtqm_wdata[29..16]   - (RO) WED_RX_PN_CHK to RX_ROUTE_QM rx_msdu2rtqm_wdata[13:0]
    rx_msdu2rtqm_rdy[30]         - (RO) WED_RX_PN_CHK to RX_ROUTE_QM rx_msdu2rtqm_rdy
    rx_msdu2rtqm_wr[31]          - (RO) WED_RX_PN_CHK to RX_ROUTE_QM rx_msdu2rtqm_wr

 =====================================================================================*/
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wr_ADDR              WED_RRO_RX_DBG3_ADDR
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wr_MASK              0x80000000                // rx_msdu2rtqm_wr[31]
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wr_SHFT              31
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_rdy_ADDR             WED_RRO_RX_DBG3_ADDR
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_rdy_MASK             0x40000000                // rx_msdu2rtqm_rdy[30]
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_rdy_SHFT             30
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wdata_ADDR           WED_RRO_RX_DBG3_ADDR
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wdata_MASK           0x3FFF0000                // rx_msdu2rtqm_wdata[29..16]
#define WED_RRO_RX_DBG3_rx_msdu2rtqm_wdata_SHFT           16

/* =====================================================================================

  ---WED_RRO_RX_DBG4 (0x15010000 + 0xF14)---

    clr_addr_acksn_wdata[31..0]  - (RO) WED_RX_ADDR_ELEM clr_addr/acksn wdata[31:0]

 =====================================================================================*/
#define WED_RRO_RX_DBG4_clr_addr_acksn_wdata_ADDR         WED_RRO_RX_DBG4_ADDR
#define WED_RRO_RX_DBG4_clr_addr_acksn_wdata_MASK         0xFFFFFFFF                // clr_addr_acksn_wdata[31..0]
#define WED_RRO_RX_DBG4_clr_addr_acksn_wdata_SHFT         0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT0 (0x15010000 + 0xF20)---

    total_ind_cmd_cnt[27..0]     - (RC) WED_RX_IND_CMD total ind_cmd count acccumulated till Rd Clear
    RESERVED28[28]               - (RO) Reserved bits
    dbg_cnt_rdclr_en[29]         - (RW) reset all debug counters'value to 0
    all_dbg_cnt_rst[30]          - (RW) reset all debug counters'value to 0
    dbg_cnt_en[31]               - (RW) WED RRO rx path debug counter enable
                                     0: stop/disable accumulation (maintains current value)
                                     1.: start/enable accumulation

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT0_dbg_cnt_en_ADDR               WED_RX_IND_CMD_CNT0_ADDR
#define WED_RX_IND_CMD_CNT0_dbg_cnt_en_MASK               0x80000000                // dbg_cnt_en[31]
#define WED_RX_IND_CMD_CNT0_dbg_cnt_en_SHFT               31
#define WED_RX_IND_CMD_CNT0_all_dbg_cnt_rst_ADDR          WED_RX_IND_CMD_CNT0_ADDR
#define WED_RX_IND_CMD_CNT0_all_dbg_cnt_rst_MASK          0x40000000                // all_dbg_cnt_rst[30]
#define WED_RX_IND_CMD_CNT0_all_dbg_cnt_rst_SHFT          30
#define WED_RX_IND_CMD_CNT0_dbg_cnt_rdclr_en_ADDR         WED_RX_IND_CMD_CNT0_ADDR
#define WED_RX_IND_CMD_CNT0_dbg_cnt_rdclr_en_MASK         0x20000000                // dbg_cnt_rdclr_en[29]
#define WED_RX_IND_CMD_CNT0_dbg_cnt_rdclr_en_SHFT         29
#define WED_RX_IND_CMD_CNT0_total_ind_cmd_cnt_ADDR        WED_RX_IND_CMD_CNT0_ADDR
#define WED_RX_IND_CMD_CNT0_total_ind_cmd_cnt_MASK        0x0FFFFFFF                // total_ind_cmd_cnt[27..0]
#define WED_RX_IND_CMD_CNT0_total_ind_cmd_cnt_SHFT        0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT1 (0x15010000 + 0xF24)---

    ind_cmd_fetch1_cnt[27..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 1 count acccumulated till Rd Clear
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT1_ind_cmd_fetch1_cnt_ADDR       WED_RX_IND_CMD_CNT1_ADDR
#define WED_RX_IND_CMD_CNT1_ind_cmd_fetch1_cnt_MASK       0x0FFFFFFF                // ind_cmd_fetch1_cnt[27..0]
#define WED_RX_IND_CMD_CNT1_ind_cmd_fetch1_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT2 (0x15010000 + 0xF28)---

    ind_cmd_fetch2_cnt[27..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 2 count acccumulated till Rd Clear
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT2_ind_cmd_fetch2_cnt_ADDR       WED_RX_IND_CMD_CNT2_ADDR
#define WED_RX_IND_CMD_CNT2_ind_cmd_fetch2_cnt_MASK       0x0FFFFFFF                // ind_cmd_fetch2_cnt[27..0]
#define WED_RX_IND_CMD_CNT2_ind_cmd_fetch2_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT3 (0x15010000 + 0xF2C)---

    ind_cmd_fetch3_cnt[27..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 3 count acccumulated till Rd Clear
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT3_ind_cmd_fetch3_cnt_ADDR       WED_RX_IND_CMD_CNT3_ADDR
#define WED_RX_IND_CMD_CNT3_ind_cmd_fetch3_cnt_MASK       0x0FFFFFFF                // ind_cmd_fetch3_cnt[27..0]
#define WED_RX_IND_CMD_CNT3_ind_cmd_fetch3_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT4 (0x15010000 + 0xF30)---

    ind_cmd_fetch4_cnt[27..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 4 count acccumulated till Rd Clear
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT4_ind_cmd_fetch4_cnt_ADDR       WED_RX_IND_CMD_CNT4_ADDR
#define WED_RX_IND_CMD_CNT4_ind_cmd_fetch4_cnt_MASK       0x0FFFFFFF                // ind_cmd_fetch4_cnt[27..0]
#define WED_RX_IND_CMD_CNT4_ind_cmd_fetch4_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT5 (0x15010000 + 0xF34)---

    ind_cmd_fetch5_cnt[23..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 5 count acccumulated till Rd Clear
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT5_ind_cmd_fetch5_cnt_ADDR       WED_RX_IND_CMD_CNT5_ADDR
#define WED_RX_IND_CMD_CNT5_ind_cmd_fetch5_cnt_MASK       0x00FFFFFF                // ind_cmd_fetch5_cnt[23..0]
#define WED_RX_IND_CMD_CNT5_ind_cmd_fetch5_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT6 (0x15010000 + 0xF38)---

    ind_cmd_fetch6_cnt[23..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 6 count acccumulated till Rd Clear
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT6_ind_cmd_fetch6_cnt_ADDR       WED_RX_IND_CMD_CNT6_ADDR
#define WED_RX_IND_CMD_CNT6_ind_cmd_fetch6_cnt_MASK       0x00FFFFFF                // ind_cmd_fetch6_cnt[23..0]
#define WED_RX_IND_CMD_CNT6_ind_cmd_fetch6_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT7 (0x15010000 + 0xF3C)---

    ind_cmd_fetch7_cnt[23..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 7 count acccumulated till Rd Clear
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT7_ind_cmd_fetch7_cnt_ADDR       WED_RX_IND_CMD_CNT7_ADDR
#define WED_RX_IND_CMD_CNT7_ind_cmd_fetch7_cnt_MASK       0x00FFFFFF                // ind_cmd_fetch7_cnt[23..0]
#define WED_RX_IND_CMD_CNT7_ind_cmd_fetch7_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT8 (0x15010000 + 0xF40)---

    ind_cmd_fetch8_cnt[23..0]    - (RC) WED_RX_IND_CMD ind_cmd fetch 8 count acccumulated till Rd Clear
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT8_ind_cmd_fetch8_cnt_ADDR       WED_RX_IND_CMD_CNT8_ADDR
#define WED_RX_IND_CMD_CNT8_ind_cmd_fetch8_cnt_MASK       0x00FFFFFF                // ind_cmd_fetch8_cnt[23..0]
#define WED_RX_IND_CMD_CNT8_ind_cmd_fetch8_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_IND_CMD_CNT9 (0x15010000 + 0xF44)---

    magic_cnt_fail_cnt[15..0]    - (RC) WED_RX_IND_CMD magic_cnt_fail count acccumulated till Rd Clear
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_IND_CMD_CNT9_magic_cnt_fail_cnt_ADDR       WED_RX_IND_CMD_CNT9_ADDR
#define WED_RX_IND_CMD_CNT9_magic_cnt_fail_cnt_MASK       0x0000FFFF                // magic_cnt_fail_cnt[15..0]
#define WED_RX_IND_CMD_CNT9_magic_cnt_fail_cnt_SHFT       0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_CNT0 (0x15010000 + 0xF48)---

    total_addr_elem_cnt[31..0]   - (RC) WED_RX_ADDR_ELEM total addr_elem count acccumulated till Rd Clear

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_CNT0_total_addr_elem_cnt_ADDR    WED_RX_ADDR_ELEM_CNT0_ADDR
#define WED_RX_ADDR_ELEM_CNT0_total_addr_elem_cnt_MASK    0xFFFFFFFF                // total_addr_elem_cnt[31..0]
#define WED_RX_ADDR_ELEM_CNT0_total_addr_elem_cnt_SHFT    0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_CNT1 (0x15010000 + 0xF4C)---

    total_sig_fail_cnt[15..0]    - (RC) WED_RX_ADDR_ELEM total signature_fail count acccumulated till Rd Clear
    total_1st_sig_fail_cnt[31..16] - (RC) WED_RX_ADDR_ELEM total clr_addr_elem times (signature_fail per run, < 8 addr_elems) acccumulated till Rd Clear

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_CNT1_total_1st_sig_fail_cnt_ADDR WED_RX_ADDR_ELEM_CNT1_ADDR
#define WED_RX_ADDR_ELEM_CNT1_total_1st_sig_fail_cnt_MASK 0xFFFF0000                // total_1st_sig_fail_cnt[31..16]
#define WED_RX_ADDR_ELEM_CNT1_total_1st_sig_fail_cnt_SHFT 16
#define WED_RX_ADDR_ELEM_CNT1_total_sig_fail_cnt_ADDR     WED_RX_ADDR_ELEM_CNT1_ADDR
#define WED_RX_ADDR_ELEM_CNT1_total_sig_fail_cnt_MASK     0x0000FFFF                // total_sig_fail_cnt[15..0]
#define WED_RX_ADDR_ELEM_CNT1_total_sig_fail_cnt_SHFT     0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_CNT2 (0x15010000 + 0xF50)---

    clr_addr_cnt[19..0]          - (RC) WED_RX_ADDR_ELEM clr_addr_elem count acccumulated till Rd Clear, add 1 each addr_elem cleared
    RESERVED20[31..20]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_CNT2_clr_addr_cnt_ADDR           WED_RX_ADDR_ELEM_CNT2_ADDR
#define WED_RX_ADDR_ELEM_CNT2_clr_addr_cnt_MASK           0x000FFFFF                // clr_addr_cnt[19..0]
#define WED_RX_ADDR_ELEM_CNT2_clr_addr_cnt_SHFT           0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_CNT3 (0x15010000 + 0xF54)---

    acksn_cnt[27..0]             - (RC) WED_RX_ADDR_ELEM ack_sn count acccumulated till Rd Clear (1 per run, <8 addr_elems)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_CNT3_acksn_cnt_ADDR              WED_RX_ADDR_ELEM_CNT3_ADDR
#define WED_RX_ADDR_ELEM_CNT3_acksn_cnt_MASK              0x0FFFFFFF                // acksn_cnt[27..0]
#define WED_RX_ADDR_ELEM_CNT3_acksn_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_CNT4 (0x15010000 + 0xF58)---

    acksn_last_cnt[27..0]        - (RC) WED_RX_ADDR_ELEM ack_sn with last count acccumulated till Rd Clear
                                     ie. To update RRO CPU_IDX count, should be the same as ind_cmd proc_idx
                                     (RRO cpu_idx starts from max_cnt-1)
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_CNT4_acksn_last_cnt_ADDR         WED_RX_ADDR_ELEM_CNT4_ADDR
#define WED_RX_ADDR_ELEM_CNT4_acksn_last_cnt_MASK         0x0FFFFFFF                // acksn_last_cnt[27..0]
#define WED_RX_ADDR_ELEM_CNT4_acksn_last_cnt_SHFT         0

/* =====================================================================================

  ---WED_RX_MSDU_PG_CNT1 (0x15010000 + 0xF5C)---

    pg_cnt1_cnt[27..0]           - (RC) WED_RX_MSDU_PG per addr_elem page_cnt=1 count acccumulated till Rd Clr
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MSDU_PG_CNT1_pg_cnt1_cnt_ADDR              WED_RX_MSDU_PG_CNT1_ADDR
#define WED_RX_MSDU_PG_CNT1_pg_cnt1_cnt_MASK              0x0FFFFFFF                // pg_cnt1_cnt[27..0]
#define WED_RX_MSDU_PG_CNT1_pg_cnt1_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_MSDU_PG_CNT2 (0x15010000 + 0xF60)---

    pg_cnt2_cnt[23..0]           - (RC) WED_RX_MSDU_PG per addr_elem page_cnt=2 count acccumulated till Rd Clr
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MSDU_PG_CNT2_pg_cnt2_cnt_ADDR              WED_RX_MSDU_PG_CNT2_ADDR
#define WED_RX_MSDU_PG_CNT2_pg_cnt2_cnt_MASK              0x00FFFFFF                // pg_cnt2_cnt[23..0]
#define WED_RX_MSDU_PG_CNT2_pg_cnt2_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_MSDU_PG_CNT3 (0x15010000 + 0xF64)---

    pg_cnt3_cnt[23..0]           - (RC) WED_RX_MSDU_PG per addr_elem page_cnt=3 count acccumulated till Rd Clr
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MSDU_PG_CNT3_pg_cnt3_cnt_ADDR              WED_RX_MSDU_PG_CNT3_ADDR
#define WED_RX_MSDU_PG_CNT3_pg_cnt3_cnt_MASK              0x00FFFFFF                // pg_cnt3_cnt[23..0]
#define WED_RX_MSDU_PG_CNT3_pg_cnt3_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_MSDU_PG_CNT4 (0x15010000 + 0xF68)---

    pg_cnt4_cnt[23..0]           - (RC) WED_RX_MSDU_PG per addr_elem page_cnt=4 count acccumulated till Rd Clr
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MSDU_PG_CNT4_pg_cnt4_cnt_ADDR              WED_RX_MSDU_PG_CNT4_ADDR
#define WED_RX_MSDU_PG_CNT4_pg_cnt4_cnt_MASK              0x00FFFFFF                // pg_cnt4_cnt[23..0]
#define WED_RX_MSDU_PG_CNT4_pg_cnt4_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_MSDU_PG_CNT5 (0x15010000 + 0xF6C)---

    pg_cnt5_cnt[23..0]           - (RC) WED_RX_MSDU_PG per addr_elem page_cnt>=5 count acccumulated till Rd Clr
    RESERVED24[31..24]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_MSDU_PG_CNT5_pg_cnt5_cnt_ADDR              WED_RX_MSDU_PG_CNT5_ADDR
#define WED_RX_MSDU_PG_CNT5_pg_cnt5_cnt_MASK              0x00FFFFFF                // pg_cnt5_cnt[23..0]
#define WED_RX_MSDU_PG_CNT5_pg_cnt5_cnt_SHFT              0

/* =====================================================================================

  ---WED_RX_PN_CHK_CNT0 (0x15010000 + 0xF70)---

    pn_chk_fail_cnt[15..0]       - (RC) WED_RX_PN_CHK pn_chk_fail count acccumulated till Rd Clear
    RESERVED16[31..16]           - (RO) Reserved bits

 =====================================================================================*/
#define WED_RX_PN_CHK_CNT0_pn_chk_fail_cnt_ADDR           WED_RX_PN_CHK_CNT0_ADDR
#define WED_RX_PN_CHK_CNT0_pn_chk_fail_cnt_MASK           0x0000FFFF                // pn_chk_fail_cnt[15..0]
#define WED_RX_PN_CHK_CNT0_pn_chk_fail_cnt_SHFT           0

/* =====================================================================================

  ---WED_RX_IND_CMD_AXI0 (0x15010000 + 0xF74)---

    araddr_reg[31..0]            - (RU) WED_RX_IND_CMD AXI rd ADDR latch

 =====================================================================================*/
#define WED_RX_IND_CMD_AXI0_araddr_reg_ADDR               WED_RX_IND_CMD_AXI0_ADDR
#define WED_RX_IND_CMD_AXI0_araddr_reg_MASK               0xFFFFFFFF                // araddr_reg[31..0]
#define WED_RX_IND_CMD_AXI0_araddr_reg_SHFT               0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_AXI0 (0x15010000 + 0xF78)---

    araddr_reg[31..0]            - (RU) WED_RX_ADDR_ELEM AXI rd ADDR latch

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_AXI0_araddr_reg_ADDR             WED_RX_ADDR_ELEM_AXI0_ADDR
#define WED_RX_ADDR_ELEM_AXI0_araddr_reg_MASK             0xFFFFFFFF                // araddr_reg[31..0]
#define WED_RX_ADDR_ELEM_AXI0_araddr_reg_SHFT             0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_AXI1 (0x15010000 + 0xF7C)---

    awaddr_reg[31..0]            - (RU) WED_RX_ADDR_ELEM AXI wr ADDR latch

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_AXI1_awaddr_reg_ADDR             WED_RX_ADDR_ELEM_AXI1_ADDR
#define WED_RX_ADDR_ELEM_AXI1_awaddr_reg_MASK             0xFFFFFFFF                // awaddr_reg[31..0]
#define WED_RX_ADDR_ELEM_AXI1_awaddr_reg_SHFT             0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_AXI2 (0x15010000 + 0xF80)---

    awaddr_clr_addr_lat[31..0]   - (RU) WED_RX_ADDR_ELEM clr_addr_elem AXI wr ADDR latch

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_AXI2_awaddr_clr_addr_lat_ADDR    WED_RX_ADDR_ELEM_AXI2_ADDR
#define WED_RX_ADDR_ELEM_AXI2_awaddr_clr_addr_lat_MASK    0xFFFFFFFF                // awaddr_clr_addr_lat[31..0]
#define WED_RX_ADDR_ELEM_AXI2_awaddr_clr_addr_lat_SHFT    0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_AXI3 (0x15010000 + 0xF84)---

    wdata0[31..0]                - (RU) WED_RX_ADDR_ELEM AXI wdata [31:0]

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_AXI3_wdata0_ADDR                 WED_RX_ADDR_ELEM_AXI3_ADDR
#define WED_RX_ADDR_ELEM_AXI3_wdata0_MASK                 0xFFFFFFFF                // wdata0[31..0]
#define WED_RX_ADDR_ELEM_AXI3_wdata0_SHFT                 0

/* =====================================================================================

  ---WED_RX_ADDR_ELEM_AXI4 (0x15010000 + 0xF88)---

    wdata1[31..0]                - (RU) WED_RX_ADDR_ELEM AXI wdata [63:32]

 =====================================================================================*/
#define WED_RX_ADDR_ELEM_AXI4_wdata1_ADDR                 WED_RX_ADDR_ELEM_AXI4_ADDR
#define WED_RX_ADDR_ELEM_AXI4_wdata1_MASK                 0xFFFFFFFF                // wdata1[31..0]
#define WED_RX_ADDR_ELEM_AXI4_wdata1_SHFT                 0

/* =====================================================================================

  ---WED_RX_MSDU_PG_AXI0 (0x15010000 + 0xF8C)---

    araddr_reg[31..0]            - (RU) WED_RX_MSDU_PG AXI rd ADDR latch

 =====================================================================================*/
#define WED_RX_MSDU_PG_AXI0_araddr_reg_ADDR               WED_RX_MSDU_PG_AXI0_ADDR
#define WED_RX_MSDU_PG_AXI0_araddr_reg_MASK               0xFFFFFFFF                // araddr_reg[31..0]
#define WED_RX_MSDU_PG_AXI0_araddr_reg_SHFT               0

/* =====================================================================================

  ---PN_CONF_1 (0x15010000 + 0xF90)---

    sdl0_check_val[13..0]        - (RW) When pn_chk receive sdl0 <= sdl_val, force to host
    sdl0_check_force_to_host_en[14] - (RW) When pn_chk receive sdl0 <= sdl_val, force to host
    paticular_id_force_to_host_en[15] - (RW) Destination of pn chk receive paticular session id
    enable[16]                   - (RW) if set 0, state will stuck at IDLE
    chk_sdp_en[17]               - (RW) check pn chk receive sdp value
    RESERVED18[23..18]           - (RO) Reserved bits
    chk_sdp_35_32[27..24]        - (RW) chk sdp[35:32]
    RESERVED28[31..28]           - (RO) Reserved bits

 =====================================================================================*/
#define PN_CONF_1_chk_sdp_35_32_ADDR                      PN_CONF_1_ADDR
#define PN_CONF_1_chk_sdp_35_32_MASK                      0x0F000000                // chk_sdp_35_32[27..24]
#define PN_CONF_1_chk_sdp_35_32_SHFT                      24
#define PN_CONF_1_chk_sdp_en_ADDR                         PN_CONF_1_ADDR
#define PN_CONF_1_chk_sdp_en_MASK                         0x00020000                // chk_sdp_en[17]
#define PN_CONF_1_chk_sdp_en_SHFT                         17
#define PN_CONF_1_enable_ADDR                             PN_CONF_1_ADDR
#define PN_CONF_1_enable_MASK                             0x00010000                // enable[16]
#define PN_CONF_1_enable_SHFT                             16
#define PN_CONF_1_paticular_id_force_to_host_en_ADDR      PN_CONF_1_ADDR
#define PN_CONF_1_paticular_id_force_to_host_en_MASK      0x00008000                // paticular_id_force_to_host_en[15]
#define PN_CONF_1_paticular_id_force_to_host_en_SHFT      15
#define PN_CONF_1_sdl0_check_force_to_host_en_ADDR        PN_CONF_1_ADDR
#define PN_CONF_1_sdl0_check_force_to_host_en_MASK        0x00004000                // sdl0_check_force_to_host_en[14]
#define PN_CONF_1_sdl0_check_force_to_host_en_SHFT        14
#define PN_CONF_1_sdl0_check_val_ADDR                     PN_CONF_1_ADDR
#define PN_CONF_1_sdl0_check_val_MASK                     0x00003FFF                // sdl0_check_val[13..0]
#define PN_CONF_1_sdl0_check_val_SHFT                     0

/* =====================================================================================

  ---PN_CONF_2 (0x15010000 + 0xF94)---

    chk_sdp_31_0[31..0]          - (RW) chk sdp[31:0]

 =====================================================================================*/
#define PN_CONF_2_chk_sdp_31_0_ADDR                       PN_CONF_2_ADDR
#define PN_CONF_2_chk_sdp_31_0_MASK                       0xFFFFFFFF                // chk_sdp_31_0[31..0]
#define PN_CONF_2_chk_sdp_31_0_SHFT                       0

/* =====================================================================================

  ---DONT_USE_BEHIND_THIS_ADDR (0x15010000 + 0xFBC)---

    value[31..0]                 - (RW) 0xFC0~0xFFF reserved for TMBIST configuration

 =====================================================================================*/
#define DONT_USE_BEHIND_THIS_ADDR_value_ADDR              DONT_USE_BEHIND_THIS_ADDR_ADDR
#define DONT_USE_BEHIND_THIS_ADDR_value_MASK              0xFFFFFFFF                // value[31..0]
#define DONT_USE_BEHIND_THIS_ADDR_value_SHFT              0

#ifdef __cplusplus
}
#endif

#endif // __WED0_REGS_H__
